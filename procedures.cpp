/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "procedures.h"

Procedures::Procedures(QObject *parent) :
    QObject(parent)
{
    QPixmap pixmap("://rufus3.jpg");
    splash = new QSplashScreen(pixmap);
    splash->show();
    QTime dieTime= QTime::currentTime().addMSecs(1500);
     while (QTime::currentTime() < dieTime)
         QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
     splash->close();
     delete splash;

    lCPParDefaut    = "";
    lVilleParDefaut = "";
    rx              = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
    rxAdresse       = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9°, -]*");
    rxAVP           = QRegExp("[1,5|1.5|2|3|4|5|6|8|10|14|28|<28]");
    rxCot           = QRegExp("[xsA-Z0-9.+/]*");  // le x pour BZQKOO1x1.5 et le s pour Cs
    rxCP            = QRegExp("[0-9]{5}");
    rxIP            = QRegExp("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])");
    rxLogin         = QRegExp("[A-Za-z0-9]*");
    rxMail          = QRegExp("^[a-zA-Z][\\w\\.-]*[a-zA-Z0-9]@[a-zA-Z0-9][\\w\\.-]*[a-zA-Z0-9]\\.[a-zA-Z][a-zA-Z\\.]*[a-zA-Z]$");
    rxMdp           = QRegExp("^[a-zA-Z0-9]{5,15}$");
    rxMdpAdmin      = QRegExp("^[a-zA-Z0-9]{3,15}$");
    rxRecherche     = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-Z %-]*");
    rxTabac         = QRegExp("[0-9]{2}");
    rxTel           = QRegExp("[0-9 ]*");
    rxVille         = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
    gidUser         = -1;

    gnomFichIni     = QDir::homePath() + NOMFIC_INI;
    Init_Icones();
    QFile FichierIni(gnomFichIni);
    setMapDatas();
    gAppFont = QFont(POLICEPARDEFAUT);
    gAppFont.setPointSize(POINTPARDEFAUT);
    qApp->setFont(gAppFont);

    gdbOK           = false;
    if (!FichierIni.exists())
    {
        bool a = false;
        while (!a)
        {
            QString msg =       tr("Le fichier d'initialisation de l'application est absent");
            QString msgInfo =   tr("Le fichier d'initialisation\n\"") + gnomFichIni + "\"\n" + tr("n'existe pas.\n"
                                "Ce fichier est indispensable au bon fonctionnement de l'application.\n\n"
                                "Cette absence est normale si vous démarrez l'application pour la première fois.\n"
                                "Si c'est le cas, choisissez l'option \"Premier démarrage de Rufus\"\n\n"
                                "Si le logiciel fonctionnait déjà sur ce poste et que le fichier a été effacé par erreur:\n"
                                "1. Si vous disposez d'une sauvegarde du fichier, choisissez \"Restaurer le fichier à partir d'une sauvegarde\"\n"
                                "2. Sinon cliquez sur \"Reconstruire le fichier d'initialisation\" et suivez les étapes de la reconstruction."
                                " Il vous faudra alors compléter de nouveau"
                                " les renseignements concernant les appareils connectés au réseau ou à ce poste d'examen après"
                                " le démarrage complet du logiciel (Menu Edition/Paramètres).\n");
            gdbOK = a;
            a = VerifIni(msg, msgInfo, true, true, true, false, true, false);
        }
    }
    gsettingsIni    = new QSettings(gnomFichIni, QSettings::IniFormat);

    bool k                          = (gsettingsIni->value("BDD_POSTE/Active").toString() == "YES"
                                       && (gsettingsIni->value("BDD_POSTE/Port").toInt() == 3306
                                       || gsettingsIni->value("BDD_POSTE/Port").toInt() == 3307)
                                       )
                                    || (gsettingsIni->value("BDD_LOCAL/Active").toString() == "YES"
                                       && gsettingsIni->value("BDD_LOCAL/Serveur").toString() != ""
                                       && (gsettingsIni->value("BDD_LOCAL/Port").toInt() == 3306
                                       || gsettingsIni->value("BDD_LOCAL/Port").toInt() == 3307)
                                       )
                                    || (gsettingsIni->value("BDD_DISTANT/Active").toString() == "YES"
                                       && gsettingsIni->value("BDD_DISTANT/Serveur").toString() != ""
                                       && (gsettingsIni->value("BDD_DISTANT/Port").toInt() == 3306
                                       || gsettingsIni->value("BDD_DISTANT/Port").toInt() == 3307)
                                       );
   if (!k)
    {
        while (!k)
        {
            QString msg =       tr("Le fichier d'initialisation de l'application est corrompu\n");
            QString msgInfo =   tr("Le fichier d'initialisation\n\"") + gnomFichIni + "\"\n" +
                                tr("ne contient pas de renseignement valide\n"
                                "permettant la connexion à la base de données.\n\n"
                                "Ce fichier est indispensable au bon fonctionnement de l'application.\n\n"
                                "1. Si vous disposez d'une sauvegarde du fichier, choisissez \"Restaurer le fichier à partir d'une sauvegarde\"\n"
                                "2. Sinon cliquez sur \"Reconstruire le fichier d'initialisation\" et suivez les étapes de la reconstruction."
                                " Il vous faudra alors compléter de nouveau"
                                " les renseignements concernant les appareils connectés au réseau ou à ce poste d'examen après"
                                " le démarrage complet du logiciel (Menu Edition/Paramètres).\n");
            gdbOK = k;
            k = VerifIni(msg, msgInfo, false, true, true, false, false, false);
        }
    }
    else if (!FicheChoixConnexion())
   {
       initOK = false;
       return;
   }

    gnomImprimante  = "";

    Ouverture_Ports_Series();
    MesureRef               = "";
    dlgrefractionouverte    = false;
    initOK                  = true;
}

void Procedures::Test()
{
}

void Procedures::ab(int i)
{
    QString mess;
    if (i == 1) mess = "jusque là, ça va";
    else mess = "là, ça ne va pas";
    UpMessageBox::Watch(0, mess);
}

QMap<QString,QVariant>  Procedures::CalculAge(QDate ddn, QDate datedujour, QString Sexe)
{
    int         AnneeNaiss, MoisNaiss, JourNaiss;
    int         AnneeCurrent, MoisCurrent, JourCurrent;
    int         AgeAnnee, AgeMois;
    int         FormuleMoisJourNaissance, FormuleMoisJourAujourdhui;
    QString     img     = "silhouette";
    QString     formule = "";
    QDate       Naissance, Aujourdhui;

    Naissance                   = ddn;
    Aujourdhui                  = datedujour;
    AnneeNaiss                  = Naissance.toString("yyyy").toInt();
    MoisNaiss                   = Naissance.toString("MM").toInt();
    JourNaiss                   = Naissance.toString("dd").toInt();
    AnneeCurrent                = Aujourdhui.toString("yyyy").toInt();
    MoisCurrent                 = Aujourdhui.toString("MM").toInt();
    JourCurrent                 = Aujourdhui.toString("dd").toInt();
    FormuleMoisJourNaissance    = (MoisNaiss*100) + JourNaiss;
    FormuleMoisJourAujourdhui   = (MoisCurrent*100) + JourCurrent;
    AgeAnnee                    = AnneeCurrent - AnneeNaiss;
    AgeMois                     = MoisCurrent - MoisNaiss;
    if (FormuleMoisJourAujourdhui < FormuleMoisJourNaissance)   AgeAnnee --;
    if (JourNaiss > JourCurrent)                                AgeMois --;
    if (AgeMois < 0)                                            AgeMois = AgeMois + 12;
    switch (AgeAnnee) {
    case 0:
        if (Naissance.daysTo(Aujourdhui) > 31)
             Age["Total"]               = QString::number(AgeMois) + " mois";
        else Age["Total"]               = QString::number(Naissance.daysTo(Aujourdhui)) + " jours";
        break;
    case 1: case 2: case 3: case 4:
        Age["Total"]                    = QString::number(AgeAnnee) + " an";
        if (AgeAnnee > 1) Age["Total"]  = Age["Total"].toString() + "s";
        if (AgeMois > 0)  Age["Total"]  = Age["Total"].toString() + " " + QString::number(AgeMois) + " mois";
        break;
    default:
        Age["Total"]                    = QString::number(AgeAnnee) + " ans";
        break;
    }

    if (Sexe =="M")                     img = "man";
    if (Sexe =="F")                     img = "women";
    if (AgeAnnee < 16 && Sexe == "F")   img = "girl";
    if (AgeAnnee < 16 && Sexe == "M")   img = "boy";
    if (AgeAnnee < 8)                   img = "kid";
    if (AgeAnnee < 2)                   img = "baby";

    if (AgeAnnee < 11)                  formule = "l'enfant";
    else if (AgeAnnee < 18) {
        if (Sexe == "F")                formule = "la jeune";
        if (Sexe == "M")                formule = "le jeune";
    }
    else    {
        if (Sexe == "F")                formule = "madame";
        if (Sexe == "M")                formule = "monsieur";
    }

    Age["Annee"]    = AgeAnnee;
    Age["Mois"]     = AgeMois;
    Age["Icone"]    = img;
    Age["Formule"]  = formule;
    return Age;
}

QIcon   Procedures::CalcIconAge(QString img)
{
    QIcon icon = giconSilhouette;

    if (img == "man")           icon = giconMan;
    else if (img == "women")    icon = giconWomen;
    else if (img == "girl")     icon = giconGirl;
    else if (img == "boy")      icon = giconBoy;
    else if (img == "kid")      icon = giconKid;
    else if (img == "baby")     icon = giconBaby;

    return icon;
}

/*--------------------------------------------------------------------------------------------------------------
-- Choix d'une date ou d'une période ---------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
bool Procedures::ChoixDate()
{
    Dlg_ChxDate            = new dlg_choixdate();
    Dlg_ChxDate->setWindowTitle(tr("Choisir une période"));
    Dlg_ChxDate->restoreGeometry(gsettingsIni->value("PositionsFiches/PositionChoixDate").toByteArray());
    bool OK = false;
    if (Dlg_ChxDate->exec()> 0)
    {
        DateDebut = Dlg_ChxDate->ui->DepuisdateEdit->date();
        if (!Dlg_ChxDate->ui->JourradioButton->isChecked())
            DateFin = Dlg_ChxDate->ui->JusquAdateEdit->date();
        else
            DateFin = DateDebut;
        OK = true;
    }
    gsettingsIni->setValue("PositionsFiches/PositionChoixDate", Dlg_ChxDate->saveGeometry());
    delete Dlg_ChxDate;
    return OK;
}

QString Procedures::ConvertitEnHtml(QString Texte)
{
    QTextEdit textHtml;
    textHtml.setText(Texte);
    Texte = textHtml.toHtml();
    return Texte;
}

bool Procedures::EcritDansUnFichier(QString NomFichier, QString TexteFichier)
{
    //syntaxe = EcritDansUnFichier(QDir::homePath()+ "/Documents/test.txt", texte);
    QDir DirRssces;
    if (!DirRssces.exists(QDir::homePath() + "/Documents/Rufus/Ressources"))
        DirRssces.mkdir(QDir::homePath() + "/Documents/Rufus/Ressources");
    QFile testfile(NomFichier);
    if (!testfile.open(QIODevice::ReadWrite))
    {
        UpMessageBox::Watch(0, tr("Impossible d'ouvrir le fichier\n") + NomFichier);
        return false;
    }
    QTextStream out(&testfile);
    out << TexteFichier;
    return true;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- renvoie la valeur du dossier documents pour le type d'appareil concerné -----------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::getDossierDocuments(QString Appareil, int mode)
{
    QString Base;
    switch (mode) {
    case Procedures::Poste:
        Base = "BDD_POSTE";
        break;
    case Procedures::Distant:
        Base = "BDD_DISTANT";
        break;
    case Procedures::ReseauLocal:
        Base = "BDD_LOCAL";
        break;
    default:
        break;
    }
    return gsettingsIni->value(Base + "/DossiersDocuments/" + Appareil).toString();
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- Gestion du flag de mise à jour de la table patients -----------------------------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
int Procedures::GetflagPatients()
{
    QString req = "select MAJflagPatients from " NOM_TABLE_FLAGS;
    QSqlQuery quer(req,getDataBase());
    if (quer.size() > 0)
    {
        quer.first();
        return quer.value(0).toInt();
    }
    else return 0;
}

int Procedures::MAJflagPatients()
{
    QSqlQuery ("SET AUTOCOMMIT = 0;", getDataBase());
    QString lockrequete = "LOCK TABLES " NOM_TABLE_FLAGS " WRITE;";
    QSqlQuery lockquery (lockrequete, getDataBase());
    if (TraiteErreurRequete(lockquery,lockrequete, tr("Impossible de verrouiller ") + NOM_TABLE_FLAGS))
    {
        return -1;
        commit(getDataBase());
    }
    QString req = "select MAJflagPatients from " NOM_TABLE_FLAGS;
    QSqlQuery quer(req,getDataBase());
    int a = 0;
    QString MAJreq;
    if (quer.size() > 0)
    {
        quer.first();
        a = quer.value(0).toInt();
        MAJreq = "update " NOM_TABLE_FLAGS " set MAJflagPatients = " + QString::number(a+1);
    }
    else
        MAJreq = "insert into " NOM_TABLE_FLAGS " (MAJflagPatients) VALUES (1)";
    QSqlQuery (MAJreq,getDataBase());
    commit(getDataBase());
    return a;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- Gestion du flag de mise à jour de l'affichage du médecin traitant -----------------------------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
int Procedures::GetflagMG()
{
    QString req = "select MAJflagMG from " NOM_TABLE_FLAGS;
    QSqlQuery quer(req,getDataBase());
    if (quer.size() > 0)
    {
        quer.first();
        return quer.value(0).toInt();
    }
    else return 0;
}

int Procedures::MAJflagMG()
{
    QSqlQuery ("SET AUTOCOMMIT = 0;", getDataBase());
    QString lockrequete = "LOCK TABLES " NOM_TABLE_FLAGS " WRITE;";
    QSqlQuery lockquery (lockrequete, getDataBase());
    if (TraiteErreurRequete(lockquery,lockrequete, tr("Impossible de verrouiller ") + NOM_TABLE_FLAGS))
    {
        return -1;
        commit(getDataBase());
    }
    QString req = "select MAJflagMG from " NOM_TABLE_FLAGS;
    QSqlQuery quer(req,getDataBase());
    QString MAJreq = "insert into " NOM_TABLE_FLAGS " (MAJflagMG) VALUES (1)";
    int a = 0;
    if (quer.seek(0)) {
        a = quer.value(0).toInt();
        MAJreq = "update " NOM_TABLE_FLAGS " set MAJflagMG = " + QString::number(a+1);
    }
    QSqlQuery (MAJreq,getDataBase());
    commit(getDataBase());
    return a;
}

/*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------- GESTION DES VILLES ET DES CODES POSTAUX
géré par la classe villecpwidget
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

QStringList Procedures::getListeVilles()
{
    return gListVilles;
}

void Procedures::setListeVilles()
{
    gListVilles.clear();
    QString req = "SELECT distinct Ville FROM " NOM_TABLE_VILLES " ORDER BY Ville";
    QSqlQuery ChercheVillesQuery (req,getDataBase());
    if (!TraiteErreurRequete(ChercheVillesQuery,req, tr("Impossible de retrouver les villes!")))
    {
        for (int i = 0; i < ChercheVillesQuery.size(); i++)
        {
            ChercheVillesQuery.seek(i);
            gListVilles << ChercheVillesQuery.value(0).toString();
        }
    }
}

QStringList Procedures::getListeCP()
{
    return gListCP;
}

void Procedures::setListeCP()
{
    gListCP.clear();
    QString req = "SELECT distinct codePostal FROM " NOM_TABLE_VILLES " ORDER BY codePOstal";
    QSqlQuery ChercheCPQuery (req,getDataBase());
    if (!TraiteErreurRequete(ChercheCPQuery,req, tr("Impossible de retrouver les codes postaux!")))
    {
        for (int i = 0; i < ChercheCPQuery.size(); i++)
        {
            ChercheCPQuery.seek(i);
            gListCP << ChercheCPQuery.value(0).toString();
        }
    }
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Faire précéder l'apostrophe d'un caractère d'échappement pour les requêtes SQL - voir commentaire dans rufus.h --------------------
------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::CorrigeApostrophe(QString RechAp)
{
    RechAp.replace("\\","\\\\");
    return RechAp.replace("'","\\'");
}

void Procedures::EnChantier(QString msg)
{
    UpMessageBox msgbox;
    msgbox.setIconPixmap(QPixmap("://work-in-progress.png").scaledToWidth(150));
    UpSmallButton *OKBouton = new UpSmallButton();
    msgbox.setInformativeText(msg);
    msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
}

void Procedures::Message(QStringList listmsg, int pause, bool bottom)
{
    dlg_message(listmsg, pause, bottom);
}

void Procedures::Message(QString mess, int pause, bool bottom)
{
    QStringList listmsg;
    listmsg << mess;
    dlg_message(listmsg, pause, bottom);
}

void Procedures::Pause(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

/*------------------------------------------------------------------------------------------------------------------------------------
    -- Ecrire la première lettre de chaque Nom et Prénom en majucule ---------------------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::MajusculePremiereLettre(QString MajLettre, bool fin, bool Maj, bool lower)
{
    return fMAJPremiereLettre(MajLettre, fin, Maj, lower);
}

// ----------------------------------------------------------------------------------
// Modidife la taille de la police utilisée pour les widget d'une liste
// ----------------------------------------------------------------------------------
void Procedures::ModifTailleFont(QObject *obj, int siz, QFont font)
{
    font.setPointSize(font.pointSize() + siz);
    QWidget *widg = dynamic_cast<QWidget*>(obj);
    if (widg != Q_NULLPTR)
        widg->setFont(font);
    for (int i=0; i<obj->findChildren<QWidget*>().size(); i++)
    {
        //qDebug() << obj->findChildren<QWidget*>().at(i)->objectName();
        obj->findChildren<QWidget*>().at(i)->setFont(font);
    }
}


// ----------------------------------------------------------------------------------
// Retourne le nombre de lignes comm selectionnees
// ----------------------------------------------------------------------------------
int Procedures::Nombre_Mesure_Selected(QTreeWidget *Tree)
{
    int nbCommSelected = 0;
    for (int i =0 ; i < Tree->topLevelItemCount(); i++)
    {
        if (Tree->topLevelItem(i)->checkState(0) == Qt::Checked)
             nbCommSelected ++;
        if (Tree->topLevelItem(i)->childCount() > 0)
            for (int j = 0; j < Tree->topLevelItem(i)->childCount(); j++)
                if (Tree->topLevelItem(i)->child(j)->checkState(0) == Qt::Checked)
                    nbCommSelected ++;
    }
    return nbCommSelected;
}

// ----------------------------------------------------------------------------------
// Retourne le nombre de lignes comm selectionnees
// ----------------------------------------------------------------------------------
int Procedures::Nombre_Mesure_Selected(QTableWidget *Table, int col)
{
    int nbCommSelected = 0;
    for (int i =0 ; i < Table->rowCount(); i++)
    {
        if (Table->item(i,col)->checkState() == Qt::Checked)
             nbCommSelected ++;
    }
    return nbCommSelected;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation des icones --------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    Procedures::Init_Icones()
{
    giconCopy           .addFile(QStringLiteral("://copy.png"),                 QSize(),        QIcon::Normal, QIcon::Off);
    giconCut            .addFile(QStringLiteral("://cut.ico"),                  QSize(),        QIcon::Normal, QIcon::Off);
    giconInsert         .addFile(QStringLiteral("://ComPlus.png"),              QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconModify         .addFile(QStringLiteral("://edit.png"),                 QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconSuppr          .addFile(QStringLiteral("://ComMoins.png"),             QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconPaste          .addFile(QStringLiteral("://paste.ico"),                QSize(),        QIcon::Normal, QIcon::Off);

    giconAlcool         .addFile(QStringLiteral("://beer.png"),                 QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconAnnul          .addFile(QStringLiteral("://Supprime.png"),             QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconApres          .addFile(QStringLiteral("://button_blue_play.png"),     QSize(),        QIcon::Normal, QIcon::Off);
    giconAvant          .addFile(QStringLiteral("://button_blue_rear.png"),     QSize(),        QIcon::Normal, QIcon::Off);
    giconArchive        .addFile(QStringLiteral("://Archives.png"),             QSize(),        QIcon::Normal, QIcon::Off);
    giconAttente        .addFile(QStringLiteral("://attente2.png"),             QSize(),        QIcon::Normal, QIcon::Off);
    giconBaby           .addFile(QStringLiteral("://baby.png"),                 QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconBack           .addFile(QStringLiteral("://back.png"),                 QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconBackup         .addFile(QStringLiteral("://Backup.png"),               QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconBlackCheck     .addFile(QStringLiteral("://blackcheck.png"),           QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconBlockCenter    .addFile(QStringLiteral("://align-center.png"),         QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconBlockJustify   .addFile(QStringLiteral("://align-justify.png"),        QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconBlockLeft      .addFile(QStringLiteral("://align-left.png"),           QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconBlockRight     .addFile(QStringLiteral("://align-right.png"),          QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconBoy            .addFile(QStringLiteral("://boy.png"),                  QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconCheck          .addFile(QStringLiteral("://check.png"),                QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconCheckOblig     .addFile(QStringLiteral("://Check-icon.png"),           QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconClock          .addFile(QStringLiteral("://Clock.png"),                QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconComputer       .addFile(QStringLiteral("://Computer.png"),             QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconContact        .addFile(QStringLiteral("://contact.png"),              QSize(),        QIcon::Normal, QIcon::Off);
    giconCreer          .addFile(QStringLiteral("://toolbar_new_folder.png"),   QSize(),        QIcon::Normal, QIcon::Off);
    giconDate           .addFile(QStringLiteral("://calendar2.png"),            QSize(),        QIcon::Normal, QIcon::Off);
    giconDoctor         .addFile(QStringLiteral("://Doctor-icon.png"),          QSize(),        QIcon::Normal, QIcon::Off);
    giconErase          .addFile(QStringLiteral("://eraser.png"),               QSize(),        QIcon::Normal, QIcon::Off);
    giconEuro           .addFile(QStringLiteral("://euro.ico"),                 QSize(40,40),   QIcon::Normal, QIcon::Off);
    giconEuroCount      .addFile(QStringLiteral("://EuroCount.png"),            QSize(40,40),   QIcon::Normal, QIcon::Off);
    giconFamily         .addFile(QStringLiteral("://family.png"),               QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFermeAppuye    .addFile(QStringLiteral("://shutdowndown.png"),         QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFermeRelache   .addFile(QStringLiteral("://shutdown.png"),             QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFont           .addFile(QStringLiteral("://font.png"),                 QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFontmore       .addFile(QStringLiteral("://fontsizemore.ico"),         QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFontless       .addFile(QStringLiteral("://fontsizeless.ico"),         QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFontbold       .addFile(QStringLiteral("://fontbold.ico"),             QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFontitalic     .addFile(QStringLiteral("://fontitalic.ico"),           QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFontunderline  .addFile(QStringLiteral("://fontunderline.ico"),        QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconFontnormal     .addFile(QStringLiteral("://fontnormal.ico"),           QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconGirl           .addFile(QStringLiteral("://girl.png"),                 QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconHelp           .addFile(QStringLiteral("://help.png"),                 QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconImprime        .addFile(QStringLiteral("://Imprimer.png"),             QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconInformation    .addFile(QStringLiteral("://information.png"),          QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconInternet       .addFile(QStringLiteral("://internet.png"),             QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconListe          .addFile(QStringLiteral("://list_all_participants.png"),QSize(),        QIcon::Normal, QIcon::Off);
    giconLoupe          .addFile(QStringLiteral("://search.png"),               QSize(),        QIcon::Normal, QIcon::Off);
    giconLunettes       .addFile(QStringLiteral("://LunettesRondes.png"),       QSize(),        QIcon::Normal, QIcon::Off);
    giconMarteau        .addFile(QStringLiteral("://wrench-screwdriver.png"),   QSize(),        QIcon::Normal, QIcon::Off);
    giconMedoc          .addFile(QStringLiteral("://medoc.png"),                QSize(),        QIcon::Normal, QIcon::Off);
    giconMessage        .addFile(QStringLiteral("://3Dmessage.png"),            QSize(40,40),   QIcon::Normal, QIcon::Off);
    giconNetwork        .addFile(QStringLiteral("://network.png"),              QSize(40,40),   QIcon::Normal, QIcon::Off);
    giconNull           .addFile(QStringLiteral(""),                            QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconOK             .addFile(QStringLiteral("://start.png"),                QSize(),        QIcon::Normal, QIcon::Off);
    giconOKpushed       .addFile(QStringLiteral("://startpushed.png"),          QSize(),        QIcon::Normal, QIcon::Off);
    giconOups           .addFile(QStringLiteral("://damn-icon.png"),            QSize(45,45),   QIcon::Normal, QIcon::Off);
    giconPageApres      .addFile(QStringLiteral("://button_blue_ffw.png"),      QSize(),        QIcon::Normal, QIcon::Off);
    giconPageAvant      .addFile(QStringLiteral("://button_blue_rew.png"),      QSize(),        QIcon::Normal, QIcon::Off);
    giconPassword       .addFile(QStringLiteral("://password.ico"),             QSize(),        QIcon::Normal, QIcon::Off);
    giconPostit         .addFile(QStringLiteral("://Post-it.png"),              QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconPoubelle       .addFile(QStringLiteral("://trash.png"),                QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconQuestion       .addFile(QStringLiteral("://question.png"),             QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconRecopier       .addFile(QStringLiteral("://copy_v2.png"),              QSize(),        QIcon::Normal, QIcon::Off);
    giconReinit         .addFile(QStringLiteral("://reset.png"),                QSize(40,40),   QIcon::Normal, QIcon::Off);
    giconSide           .addFile(QStringLiteral("://right_left.png"),           QSize(),        QIcon::Normal, QIcon::Off);
    giconSunglasses     .addFile(QStringLiteral("://Sunglasses.png"),           QSize(),        QIcon::Normal, QIcon::Off);
    giconSmoking        .addFile(QStringLiteral("://No_smoking.png"),           QSize(),        QIcon::Normal, QIcon::Off);
    giconSortirDossier  .addFile(QStringLiteral("://metadoc.png"),              QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconStetho         .addFile(QStringLiteral("://stetho.png"),               QSize(),        QIcon::Normal, QIcon::Off);
    giconText           .addFile(QStringLiteral("://text.png"),                 QSize(),        QIcon::Normal, QIcon::Off);
    giconTri            .addFile(QStringLiteral("://tri.png"),                  QSize(),        QIcon::Normal, QIcon::Off);
    giconValide         .addFile(QStringLiteral("://Valide2.jpg"),              QSize(25,25),   QIcon::Normal, QIcon::Off);
    giconVitale         .addFile(QStringLiteral("://Vitale.jpg"),               QSize(),        QIcon::Normal, QIcon::Off);
    giconMan            .addFile(QStringLiteral("://man.png"),                  QSize(),        QIcon::Normal, QIcon::Off);
    giconWomen          .addFile(QStringLiteral("://women.png"),                QSize(30,30),   QIcon::Normal, QIcon::Off);
    giconKid            .addFile(QStringLiteral("://kid.png"),                  QSize(),        QIcon::Normal, QIcon::Off);
    giconSilhouette     .addFile(QStringLiteral("://silhouette.png"),           QSize(20,20),   QIcon::Normal, QIcon::Off);
    giconUpdate         .addFile(QStringLiteral("://update.png"),               QSize(20,20),   QIcon::Normal, QIcon::Off);
}

/*------------------------------------------------------------------------------------------------------------------------------------
-- Modifier la table Utilisateurs pour signifier aux autres utilisateurs que la salle d'attente vient d'être modifiée --------------------
------------------------------------------------------------------------------------------------------------------------------------*/
void Procedures::UpdVerrouSalDat()
{
    QString lockrequete = "LOCK TABLES " NOM_TABLE_USERSCONNECTES " WRITE;";
    QSqlQuery (lockrequete, db);
    QString req = "SELECT MAX(NewidModifSalDat) From " NOM_TABLE_USERSCONNECTES;
    QSqlQuery query (req, db);
    TraiteErreurRequete(query,req,"");
    QString Newid = "0";
    if (query.size() > 0)
    {
        query.first();
        Newid = QString::number(query.value(0).toInt() + 1);
    }
    req = "UPDATE " NOM_TABLE_USERSCONNECTES " SET NewiDModifSalDat = " + Newid;
    QSqlQuery query2 (req, db);
    TraiteErreurRequete(query2,req,"");
    QSqlQuery("unlock tables",db);
    emit UpdSalDat();                           //déclenche la MAJ de la salle d'attente
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Traite et affiche le signal d'erreur d'une requete -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::TraiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage)
{
    if (query.lastError().type() != QSqlError::NoError)
    {
        UpMessageBox::Watch(0, ErrorMessage, tr("\nErreur\n") + query.lastError().text() +  tr("\nrequete = ") + requete);
        EcritDansUnFichier(QDir::homePath()+ NOMFIC_TEST, requete);
        return true;
    }
    else return false;
}

// CZ 27082015 deb
/*---------------------------------------------------------------------------------
    Retourne  la String de la partie du document passée en paramètre
-----------------------------------------------------------------------------------*/
QString Procedures::RecupPartie(QString ficModele, QString partie)
{
int pos1 = -1;
int pos2 = -1;
    pos1 = ficModele.indexOf("<!--"+partie+"-->") + partie.length()+ 8;
    pos2 = ficModele.indexOf("<!--Fin"+partie+"-->") ;
    if (pos2 > pos1)
        return(ficModele.mid(pos1 , pos2 - pos1));
    else return "";
}

/*---------------------------------------------------------------------------------
    Effectue une sauvegarde immédiate de la base
-----------------------------------------------------------------------------------*/
bool Procedures::ImmediateBackup(bool full)
{
    QString req = "select NomPosteConnecte from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte <> '" + QHostInfo::localHostName().left(60) + "'";
    QSqlQuery postesquer(req,db);
    if (postesquer.size() > 0)
    {
        postesquer.first();
        UpMessageBox::Information(0, tr("Autres postes connectés!"),
                                     tr("Vous ne pouvez pas effectuer d'opération de sauvegarde/restauration sur la base de données"
                                     " si vous n'êtes pas le seul poste connecté.\n"
                                     "Le poste ") + postesquer.value(0).toString() + tr(" est aussi connecté"));
        return false;
    }

    QSqlQuery dirquer("select dirimagerie, DirBkup from " NOM_TABLE_PARAMSYSTEME, db);
    dirquer.first();
    QString NomDirStockageImagerie = dirquer.value(0).toString();
    QString NomDirDestination = dirquer.value(1).toString();
    if(!QDir(NomDirDestination).exists() || NomDirDestination == "")
    {
        if (UpMessageBox::Question(0,
                                   tr("Pas de destination"),
                                   NomDirDestination == ""?
                                   tr("Vous n'avez pas spécifié de dossier de destination\npour la sauvegarde\nVoulez-vous le faire maintenant?") :
                                   tr("Le dossier de destination de sauvegarde") + "\n" +  NomDirDestination + "\n" + tr("nest pas valide\nVoulez-vous choisir un autre dossier?"),
                                   UpDialog::ButtonCancel | UpDialog::ButtonOK,
                                   QStringList() << tr("Annuler") << tr("Choisir un dossier"))
            == UpSmallButton::STARTBUTTON)
        {
            QString dirSauv         = QFileDialog::getExistingDirectory(0,tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base\n"
                                                                        "Le nom de dossier ne doit pas contenir d'espace"), QDir::homePath());
            if (dirSauv == "")
                return false;
            if (dirSauv.contains(" "))
            {
                UpMessageBox::Watch(0, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
                return false;
            }
            NomDirDestination = dirSauv;
        }
        else return false;
    }
    bool OKbase, OKImages, OKVideos;
    if (full)
    {
        OKbase = true;
        OKImages = true;
        OKVideos = true;
    }
    else
    {
        AskBupRestore(false, NomDirStockageImagerie, NomDirDestination );
        if (gAskBupRestore->exec()==0)
            return false;
        QList<UpCheckBox*> listchk = gAskBupRestore->findChildren<UpCheckBox*>();
        for (int i= 0; i<listchk.size(); i++)
        {
            if (listchk.at(i)->accessibleDescription() == "base")
                OKbase = listchk.at(i)->isChecked();
            else if (listchk.at(i)->accessibleDescription() == "images")
                OKImages = listchk.at(i)->isChecked();
            else if (listchk.at(i)->accessibleDescription() == "videos")
                OKVideos = listchk.at(i)->isChecked();
        }
    }
    if (!OKbase && !OKImages && !OKVideos)
        return false;

    Message(tr("Sauvegarde en cours"),3000,false);
    ConnectTimers(false);

    bool result = true;
    if (OKbase)
    {
        QFile precBup(QDir::homePath() + SCRIPTBACKUPFILE);
        bool b = precBup.exists();
        DefinitScriptBackup(NomDirStockageImagerie, OKImages, OKVideos);
        QString msg = "sh " + QDir::homePath() + SCRIPTBACKUPFILE;
        QProcess dumpProcess(parent());
        dumpProcess.start(msg);
        dumpProcess.waitForFinished(1000000000);
        int  a = 99;
        if (dumpProcess.exitStatus() == QProcess::NormalExit)
            a = dumpProcess.exitCode();
        if (a == 0)
            msg = tr("Sauvegarde effectuée avec succès");
        else
            msg = tr("Incident pendant la sauvegarde");
        Message(msg,3000,false);
        if (b)
            DefinitScriptBackup(NomDirStockageImagerie);
        else
            QFile (QDir::homePath() + SCRIPTBACKUPFILE).remove();
        result = (a==0);
    }
    else
    {
        QString dest = NomDirDestination + "/" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmm");
        if (OKImages || OKVideos)
        {
            QDir dirdest;
            dirdest.mkdir(dest);
        }
        if (OKImages)
        {
            QString Msg = (tr("Sauvegarde des fichiers d'imagerie\n")
                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
            Message(Msg, 3000);
            QProcess::execute("cp -R " + NomDirStockageImagerie + NOMDIR_IMAGES + " " + dest);
            Message(tr("Fichiers d'imagerie restaurés!"), 3000, false);
        }
        if (OKVideos)
        {
            QString Msg = (tr("Sauvegarde des fichiers videos\n")
                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
            Message(Msg, 3000);
            QProcess::execute("cp -R " + NomDirStockageImagerie + NOMDIR_VIDEOS + " " + dest);
            Message(tr("Fichiers d'imagerie restaurés!"), 3000, false);
        }
    }
    ConnectTimers();
    UpMessageBox::Watch(0, tr("Sauvegarde terminée"));
    return result;
}

/*---------------------------------------------------------------------------------
    Retourne le corps du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::ImpressionCorps(QString text, bool ALD)
{
    QString Corps;
    QString nomModeleCorpsImpression;

    text = ConvertitEnHtml(text);
    if (ALD)
        nomModeleCorpsImpression = QDir::homePath() + NOMFIC_CORPSORDOALD;
    else
        nomModeleCorpsImpression = QDir::homePath() + NOMFIC_CORPSORDO;

    QFile qFile(nomModeleCorpsImpression);
    while (!qFile.open( QIODevice::ReadOnly ))
        if (!VerifRessources(nomModeleCorpsImpression))
            return QString();

    long file_len = qFile.size();
    QByteArray ba = qFile.readAll();
    ba.resize(file_len+1);
    ba.data()[file_len]=0;
    qFile.close ();
    Corps = ba;
    text.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:9pt");
    Corps.replace("{{TEXTE ORDO}}",text);
    Corps.replace("{{TEXTE ORDO HORS ALD}}"  , "");
    return Corps;
}

/*---------------------------------------------------------------------------------
    Retourne l'entête du document à imprimer
-----------------------------------------------------------------------------------*/
QMap<QString, QString> Procedures::ImpressionEntete(QDate date)
{
    QMap<QString, QString> EnteteMap;
    QString Entete;
    QString nomModeleEntete;
    int idparent = -1;
    bool rplct = false;
    // si le user est un remplaçant on essaie de savoir qui il remplace
    if (OtherUser["EnregHonoraires"].toInt()==3)
    {
        rplct = true;
        if (OtherUser["idUser"].toInt() == gidUserSuperViseur)
            idparent = gidUserParent;
        else
        {
            // le user est connecté, on cherche qui il remplace - son parent
            QString reqrp = "select userparent from " NOM_TABLE_USERSCONNECTES " where usersuperviseur = " + OtherUser["EnregHonoraires"].toString();
            QSqlQuery querrp(reqrp, getDataBase());
            if (querrp.size()>0)
            {
                querrp.first();
                idparent = querrp.value(0).toInt();
            }
            else
            {
                // le user n'est pas connecté on demande quel est son parent
                QSqlQuery soignquer("select soignant from " NOM_TABLE_UTILISATEURS " where iduser = " + OtherUser["idUser"].toString(), db);
                soignquer.first();
                QString req   = "select iduser, userlogin from " NOM_TABLE_UTILISATEURS
                        " where (userenreghonoraires = 1 or userenreghonoraires = 2)"
                        " and iduser <> " + OtherUser["idUser"].toString() +
                        " and soignant = " + soignquer.value(0).toString() +
                        " and userdesactive is null";
                //qDebug() << req;
                QSqlQuery quer(req, db);
                TraiteErreurRequete(quer,req);
                if (quer.size() == 1)
                {
                    quer       .first();
                    idparent   = quer.value(0).toInt();
                }
                else // on va demander qui est le soignant parent de ce remplaçant....
                {
                    gAskUser                = new UpDialog();
                    gAskUser                ->AjouteLayButtons();
                    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(gAskUser->layout());
                    QGroupBox*boxparent     = new QGroupBox();
                    globallay               ->insertWidget(0,boxparent);
                    boxparent               ->setAccessibleName("Parent");
                    QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + OtherUser["UserLogin"].toString() + "?";
                    boxparent               ->setTitle(lblUsrParent);

                    QFontMetrics fm         = QFontMetrics(qApp->font());
                    int hauteurligne        = fm.height()*1.6;
                    boxparent               ->setFixedHeight(((quer.size() + 1)*hauteurligne)+5);
                    QVBoxLayout *vbox       = new QVBoxLayout;
                    for (int i=0; i<quer.size(); i++)
                    {
                        quer        .seek(i);
                        QRadioButton *pradiobutt = new QRadioButton(boxparent);
                        pradiobutt  ->setText(quer.value(1).toString());
                        pradiobutt  ->setAccessibleName(quer.value(0).toString());
                        pradiobutt  ->setChecked(i==0);
                        vbox        ->addWidget(pradiobutt);
                    }
                    vbox                ->setContentsMargins(8,0,8,0);
                    boxparent           ->setLayout(vbox);
                    gAskUser                ->setModal(true);
                    globallay               ->setSizeConstraint(QLayout::SetFixedSize);
                    connect(gAskUser->OKButton,   SIGNAL(clicked(bool)),  gAskUser, SLOT(accept()));
                    gAskUser->exec();
                    QList<QRadioButton*> listbutt = boxparent->findChildren<QRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                            idparent = listbutt.at(j)->accessibleName().toInt();
                    delete gAskUser;
                }
            }
        }
    }
    for (int i = 1; i<3; i++)
    {
        if (i==1)
            nomModeleEntete = QDir::homePath() + NOMFIC_ENTETEORDO;
        else
            nomModeleEntete = QDir::homePath() + NOMFIC_ENTETEORDOALD;
        QFile qFileEnTete(nomModeleEntete);
        while (!qFileEnTete.open( QIODevice::ReadOnly ))
            if (!VerifRessources(nomModeleEntete))
                return QMap<QString, QString>();

        long        fileEnTete_len  = qFileEnTete.size();
        QByteArray  baEnTete        = qFileEnTete.readAll();
        baEnTete.resize(fileEnTete_len + 1);
        baEnTete.data()[fileEnTete_len] = 0;
        qFileEnTete.close ();
        Entete = baEnTete;
        if (rplct)
        {
            QString reqr = "select usertitre, usernom, userprenom, usernumps, usernumco from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(idparent);
            QSqlQuery querr (reqr,getDataBase());
            querr.first();
            if (querr.value(0).toString() != "")
                Entete.replace("{{TITREUSER}}"     , "<s>" + querr.value(0).toString() + " " + querr.value(2).toString() + " " + querr.value(1).toString() + "</s> "
                               "<font color=\"darkblue\">"
                               + tr ("remplacé par") + " "
                               + OtherUser["Titre"].toString() + " " + OtherUser["Prenom"].toString() + " " + OtherUser["Nom"].toString())
                               + "</font>";
            else
                Entete.replace("{{TITREUSER}}"     , "<s>" + querr.value(2).toString() + " " + querr.value(1).toString() + " </s><font color=\"red\">" + tr ("remplacé par") + " "
                               + OtherUser["Prenom"].toString() + " " + OtherUser["Nom"].toString()) + "</font>";
        }
        else
        {
            if (OtherUser["Titre"].toString() != "")
                Entete.replace("{{TITREUSER}}"     , OtherUser["Titre"].toString() + " " + OtherUser["Prenom"].toString() + " " + OtherUser["Nom"].toString());
            else
                Entete.replace("{{TITREUSER}}"     , OtherUser["Prenom"].toString() + " " + OtherUser["Nom"].toString());
        }
        if (OtherUser["NoSpecialite"].toInt() != 0)
            Entete.replace("{{SPECIALITE}}"    , OtherUser["NoSpecialite"].toString() + " " + OtherUser["Specialite"].toString());
        else
            Entete.replace("{{SPECIALITE}}"    , OtherUser["Specialite"].toString());
        QString adresse ="";
        int nlignesadresse = 0;
        if (OtherUser["NomLieu"].toString() != "")
        {
            nlignesadresse  ++;
            adresse         += OtherUser["NomLieu"].toString();
        }
        if (OtherUser["Adresse1"].toString() != "" || OtherUser["Adresse2"].toString() != "")
        {
            nlignesadresse  ++;
            if (nlignesadresse >0)
                adresse += "<br />";
            if (OtherUser["Adresse1"].toString() != "" && OtherUser["Adresse2"].toString() != "")
                adresse += OtherUser["Adresse1"].toString() + " - " + OtherUser["Adresse2"].toString();
            else
                adresse += OtherUser["Adresse1"].toString() + OtherUser["Adresse2"].toString();
        }
        Entete.replace("{{ADRESSE}}"       , adresse);
//        if (OtherUser["Adresse1"].toString() == "" && OtherUser["Adresse2"].toString() == "")
//            Entete.replace("{{ADRESSE}}"       , "<br />" + OtherUser["NomLieu"].toString());
//        else if (OtherUser["Adresse"].toString() =="")
//            Entete.replace("{{ADRESSE}}"       , OtherUser["NomLieu"].toString() + "<br />" + OtherUser["Adresse1"].toString());
//        else
//            Entete.replace("{{ADRESSE}}"       , OtherUser["NomLieu"].toString() + " - " + OtherUser["Adresse1"].toString() + "<br />" + OtherUser["Adresse2"].toString());
        Entete.replace("{{CPVILLE}}"           , OtherUser["CodePostal"].toString() + " " + OtherUser["Ville"].toString().toUpper());
        Entete.replace("{{TEL}}"               , "Tél. " + OtherUser["Telephone"].toString());
        if (nlignesadresse==2)
            Entete.replace("{{LIGNESARAJOUTER}}", "<span style=\"font-size:5pt;\"> <br /></span>");
        else
            Entete.replace("{{LIGNESARAJOUTER}}", "");

        QString NumSS = "";
        if (OtherUser["NumCO"].toString() != "")
        {
            NumSS =OtherUser["NumCO"].toString();
            if (OtherUser["NumPS"].toString() != "") NumSS += " - ";
        }
        if (OtherUser["NumPS"].toString() != "") NumSS += "RPPS " + OtherUser["NumPS"].toString();
        Entete.replace("{{NUMSS}}"             , NumSS);
        Entete.replace("{{DATE}}"              , OtherUser["Ville"].toString()  + tr(", le ") + date.toString(tr("d MMMM yyyy")));

        (i==1? EnteteMap["Norm"] = Entete : EnteteMap["ALD"] = Entete);
    }
    return EnteteMap;
}

/*---------------------------------------------------------------------------------
    Retourne le pied du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::ImpressionPied(bool lunettes, bool ALD)
{
    QString Pied;
    if (ALD)
        Pied = "<html><div align =\"center\"><table>{{DUPLI}}</table></div></html>";
    else
    {
        QString nomModelePied = QDir::homePath() + NOMFIC_PIEDPAGE;
        if (lunettes)
            nomModelePied = QDir::homePath() + NOMFIC_PIEDORDOLUNETTES;
        QFile   qFilePied(nomModelePied );
        while (!qFilePied.open( QIODevice::ReadOnly ))
            if (!VerifRessources(nomModelePied))
                return QString();
        long filePied_len = qFilePied.size();
        QByteArray baPied = qFilePied.readAll();
        baPied.resize(filePied_len + 1);
        baPied.data()[filePied_len] = 0;
        qFilePied.close ();
        if (OtherUser["AGA"].toBool())
            baPied.replace("{{AGA}}","Membre d'une association de gestion agréée - Le règlement des honoraires par chèque est accepté");
        else
            baPied.replace("{{AGA}}","");
        Pied = baPied;
    }
    return Pied;
}

bool Procedures::Imprime_Etat(QTextEdit *Etat, QString EnTete, QString Pied, int TaillePieddePage, int TailleEnTete, int TailleTopMarge,
                              bool AvecDupli, bool AvecPrevisu, bool AvecNumPage, bool AvecChoixImprimante)
{
    //AvecPrevisu = true;
    TextPrinter *TexteAImprimer = new TextPrinter();
    QString PiedDepart = Pied;
    TexteAImprimer->setFooterSize(TaillePieddePage);
    TexteAImprimer->setHeaderText(EnTete);

    if (TailleEnTete > 0)
        TexteAImprimer->setHeaderSize(TailleEnTete);
    else
        TexteAImprimer->setHeaderSize(25);
    Pied.replace("{{DUPLI}}","");
    if (!AvecNumPage)
        Pied.replace("&page;","");
    TexteAImprimer->setFooterText(Pied);
    TexteAImprimer->setTopMargin(TailleTopMarge);
    if (!AvecDupli)
        TexteAImprimer->setDuplex(QPrinter::DuplexLongSide);
    bool a = false;
    if (AvecPrevisu)
        a = TexteAImprimer->preview(Etat->document(), NOMFIC_PDF, "");
    else
    {
        if (!AvecChoixImprimante)
            TexteAImprimer->setPrinterName(gnomImprimante);
        a = TexteAImprimer->print(Etat->document(), NOMFIC_PDF, "", AvecChoixImprimante);
    }
    if (a)
        if (AvecDupli)
        {
            QString dupli = "<tr><td >"
                            "<p align=\"center\" ><span style=\"font-family:Arial Black;font-size:30pt;font-style:normal;font-weight:bold;color:#cccccc;\">"
                            "DUPLICATA"
                            "</span></p>"
                            "</td></tr>";
            Pied = PiedDepart.replace("{{DUPLI}}",dupli);
            if (!AvecNumPage)
                Pied.replace("&page;","");
            TexteAImprimer->setFooterText(Pied);
            TexteAImprimer->setFooterSize(TexteAImprimer->footerSize() + 20);
            TexteAImprimer->print(Etat->document(),"","",false);
        }
    gnomImprimante = TexteAImprimer->getPrinterName();
    delete TexteAImprimer;
    return a;
}

void Procedures::Slot_printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    gEtat->print(printer);
#endif
}
//----------------------------Exemple  pour Imprimer un etat  ---------------------------------------------------------------
void Procedures::Imprimer_Etat(QWidget *Formu, QPlainTextEdit *Etat)
{
    #ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, Formu);
    dlg->setWindowTitle(tr("titre"));
    if (dlg->exec() == QDialog::Accepted) {
        Etat->print(&printer);
        }
    delete dlg;
    #endif
}

QString Procedures::RetireCaracteresAccentues(QString nom)
{
    nom.replace(QRegExp("[éêëè]"),"e");
    nom.replace(QRegExp("[ÉÈÊË]"),"E");
    nom.replace(QRegExp("[àâ]"),"a");
    nom.replace(QRegExp("[ÂÀ]"),"A");
    nom.replace(QRegExp("[îï]"),"i");
    nom.replace(QRegExp("[ÏÎ]"),"I");
    nom.replace(QRegExp("[ôö]"),"o");
    nom.replace(QRegExp("[ÔÖ]"),"O");
    nom.replace("ù","u");
    nom.replace("Ù","U");
    nom.replace("ç","c");
    nom.replace("Ç","C");
    nom.replace("Œ","OE");
    nom.replace("œ","oe");
    return nom;
}

QString Procedures::Edit(QString txt, QString titre)
{
    QString         rep("");
    UpDialog        *gAsk           = new UpDialog();
    QVBoxLayout     *globallay      = dynamic_cast<QVBoxLayout*>(gAsk->layout());
    gTxtEdit                        = new UpTextEdit(gAsk);
    int x = qApp->desktop()->availableGeometry().width();
    int y = qApp->desktop()->availableGeometry().height();

    gAsk->setModal(true);
    gTxtEdit->setText(txt);

    gAsk->setMaximumWidth(x);
    gAsk->setMaximumHeight(y);
    gAsk->setWindowTitle(titre);

    gTxtEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(gTxtEdit,   SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(Slot_MenuContextuelUptextEdit()));
    globallay->insertWidget(0,gTxtEdit);

    gAsk->AjouteLayButtons();
    connect(gAsk->OKButton,SIGNAL(clicked(bool)),gAsk,SLOT(accept()));
    gAsk->restoreGeometry(gsettingsIni->value("PositionsFiches/PositionEdit").toByteArray());

    if (gAsk->exec()>0)
        rep = gTxtEdit->toHtml();
    gsettingsIni->setValue("PositionsFiches/PositionEdit",gAsk->saveGeometry());
    delete gAsk;
    return rep;
}

void Procedures::EditHtml(QString txt)
{
    UpDialog        *gAsk           = new UpDialog();
    QVBoxLayout     *globallay      = dynamic_cast<QVBoxLayout*>(gAsk->layout());
    QLabel *lbl                     = new QLabel(gAsk);

    gAsk->setModal(true);
    gAsk->setSizeGripEnabled(false);
    lbl->setStyleSheet("border: 1px solid gray;");
    lbl->setTextFormat(Qt::PlainText);
    lbl->setText(txt);

    globallay->insertWidget(0,lbl);

    gAsk->AjouteLayButtons();
    connect(gAsk->OKButton,SIGNAL(clicked(bool)),gAsk,SLOT(accept()));

    gAsk->exec();
    delete gAsk;
}

void Procedures::Slot_MenuContextuelUptextEdit()
{
    UpTextEdit *TxtEdit = dynamic_cast<UpTextEdit*>(sender());
    if (!TxtEdit) return;
    QMenu *gmenuContextuel          = new QMenu();
    QAction *pAction_ModifPolice    = new QAction(this);
    QAction *pAction_Fontbold       = new QAction(this);
    QAction *pAction_Fontitalic     = new QAction(this);
    QAction *pAction_Fontunderline  = new QAction(this);
    QAction *pAction_Fontnormal     = new QAction(this);
    QAction *pAction_Copier         = new QAction(this);
    QAction *pAction_Cut            = new QAction(this);
    QAction *pAction_Coller         = new QAction(this);
    QAction *pAction_Blockcentr     = new QAction(this);
    QAction *pAction_Blockjust      = new QAction(this);
    QAction *pAction_Blockright     = new QAction(this);
    QAction *pAction_Blockleft      = new QAction(this);

    if (TxtEdit->textCursor().selectedText().size() > 0)   {
        pAction_ModifPolice    = gmenuContextuel->addAction(giconFont,           tr("Modifier la police"));
        pAction_Fontbold       = gmenuContextuel->addAction(giconFontbold,       tr("Gras"));
        pAction_Fontitalic     = gmenuContextuel->addAction(giconFontitalic,     tr("Italique"));
        pAction_Fontunderline  = gmenuContextuel->addAction(giconFontunderline,  tr("Souligné"));
        pAction_Fontnormal     = gmenuContextuel->addAction(giconFontnormal,     tr("Normal"));
        gmenuContextuel->addSeparator();
    }
    pAction_Blockleft           = gmenuContextuel->addAction(giconBlockLeft,      tr("Aligné à gauche"));
    pAction_Blockright          = gmenuContextuel->addAction(giconBlockRight,     tr("Aligné à droite"));
    pAction_Blockcentr          = gmenuContextuel->addAction(giconBlockCenter,    tr("Centré"));
    pAction_Blockjust           = gmenuContextuel->addAction(giconBlockJustify,   tr("Justifié"));
    gmenuContextuel->addSeparator();
    if (TxtEdit->textCursor().selectedText().size() > 0)   {
        pAction_Copier         = gmenuContextuel->addAction(giconCopy,            tr("Copier"));
        pAction_Cut            = gmenuContextuel->addAction(giconCut,             tr("Couper"));
    }
    const QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasText() || mimeData->hasUrls() || mimeData->hasImage() || mimeData->hasHtml())
        pAction_Coller         = gmenuContextuel->addAction(giconPaste,  tr("Coller"));

    connect (pAction_Fontbold,      &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Gras");});
    connect (pAction_Fontitalic,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Italique");});
    connect (pAction_Fontunderline, &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Souligne");});
    connect (pAction_Fontnormal,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Normal");});
    connect (pAction_ModifPolice,   &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Police");});
    connect (pAction_Blockcentr,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Centre");});
    connect (pAction_Blockright,    &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Droite");});
    connect (pAction_Blockleft,     &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Gauche");});
    connect (pAction_Blockjust,     &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Justifie");});
    connect (pAction_Copier,        &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Copier");});
    connect (pAction_Coller,        &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Coller");});
    connect (pAction_Cut,           &QAction::triggered,    [=] {Slot_ChoixMenuContextuelUptextEdit("Couper");});

    // ouvrir le menu
    gmenuContextuel->exec(QCursor::pos());
    delete gmenuContextuel;
}

void Procedures::Slot_ChoixMenuContextuelUptextEdit(QString choix)
{
    if (choix       == "Coller")    gTxtEdit->paste();
    else if (choix  == "Copier")    gTxtEdit->copy();
    else if (choix  == "Couper")    gTxtEdit->cut();
    else if (choix  == "Police")    {
        bool ok = false;
        QFont police = QFontDialog::getFont(&ok, qApp->font(), 0, tr("Choisissez une police"));
        if (ok){
            QTextCharFormat format;
            format.setFont(police);
            gTxtEdit->textCursor().setCharFormat(format);
        }
    }
    else if (choix  == "Gras")    {
        QTextCharFormat format  = gTxtEdit->textCursor().charFormat();
        format.setFontWeight(QFont::Bold);
        format.setFontUnderline(format.fontUnderline());
        format.setFontItalic(format.fontItalic());
        gTxtEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Italique")    {
        QTextCharFormat format  = gTxtEdit->textCursor().charFormat();
        format.setFontItalic(true);
        format.setFontUnderline(format.fontUnderline());
        format.setFontWeight(format.fontWeight());
        gTxtEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Souligne")    {
        QTextCharFormat format  = gTxtEdit->textCursor().charFormat();
        format.setFontUnderline(true);
        format.setFontItalic(format.fontItalic());
        format.setFontWeight(format.fontWeight());
        gTxtEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Normal")    {
        QTextCharFormat format  = gTxtEdit->textCursor().charFormat();
        format.setFont(qApp->font());
        gTxtEdit->textCursor().setCharFormat(format);
    }
    else if (choix  == "Gauche")    {
        QTextCursor curs = gTxtEdit->textCursor();
        gTxtEdit->moveCursor(QTextCursor::StartOfBlock);
        gTxtEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = gTxtEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignLeft);
        gTxtEdit->textCursor().setBlockFormat(blockformat);
        gTxtEdit->setTextCursor(curs);
    }
    else if (choix  == "Justifie")    {
        QTextCursor curs = gTxtEdit->textCursor();
        gTxtEdit->moveCursor(QTextCursor::StartOfBlock);
        gTxtEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = gTxtEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignJustify);
        gTxtEdit->textCursor().setBlockFormat(blockformat);
        gTxtEdit->setTextCursor(curs);
    }
    else if (choix  == "Droite")    {
        QTextCursor curs = gTxtEdit->textCursor();
        gTxtEdit->moveCursor(QTextCursor::StartOfBlock);
        gTxtEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = gTxtEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignRight);
        gTxtEdit->textCursor().setBlockFormat(blockformat);
        gTxtEdit->setTextCursor(curs);
    }
    else if (choix  == "Centre")    {
        QTextCursor curs = gTxtEdit->textCursor();
        gTxtEdit->moveCursor(QTextCursor::StartOfBlock);
        gTxtEdit->moveCursor(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);
        QTextBlockFormat blockformat  = gTxtEdit->textCursor().blockFormat();
        blockformat.setAlignment(Qt::AlignCenter);
        gTxtEdit->textCursor().setBlockFormat(blockformat);
        gTxtEdit->setTextCursor(curs);
    }
}

void Procedures::commit(QSqlDatabase db)
{
    QSqlQuery ("COMMIT;", db);
    QSqlQuery ("UNLOCK TABLES;", db);
    QString commitrequete = "SET AUTOCOMMIT = 1;";
    QSqlQuery commitquery (commitrequete,db);
    TraiteErreurRequete(commitquery, commitrequete, tr("Impossible de valider les mofifications"));
}

void Procedures::rollback(QSqlDatabase db)
{
    QSqlQuery ("ROLLBACK;", db);
    QSqlQuery ("UNLOCK TABLES;", db);
    QString rollbackrequete = "SET AUTOCOMMIT = 1;";
    QSqlQuery rollbackquery (rollbackrequete, db);
    TraiteErreurRequete(rollbackquery,rollbackrequete,"");
}

void Procedures::locktables(QSqlDatabase db, QStringList ListTables, QString ModeBlocage)
{
    QSqlQuery ("SET AUTOCOMMIT = 0;", db);
    QString lockrequete = "LOCK TABLES " + ListTables.at(0) + " " + ModeBlocage;
    for (int i = 1; i < ListTables.size(); i++)
        lockrequete += "," + ListTables.at(i) + " " + ModeBlocage;
    QSqlQuery lockquery (lockrequete, db);
    TraiteErreurRequete(lockquery,lockrequete, tr("Impossible de bloquer les tables en ") + ModeBlocage);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Accesseurs  ----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Procedures::Init()
{
    return initOK;
}

bool Procedures::ApercuAvantImpression()
{
    return (gsettingsIni->value("Param_Imprimante/ApercuAvantImpression").toString() == "YES");
}

QString Procedures::getCodePostalParDefaut()
{
    if (lCPParDefaut != "") return lCPParDefaut;
    lCPParDefaut = gsettingsIni->value("Param_Poste/CodePostalParDefaut").toString();
    return lCPParDefaut;
}

void Procedures::setCodePostalParDefaut(QString CPParDefaut)
{
    lCPParDefaut = CPParDefaut;
}

QSqlDatabase Procedures::getDataBase()
{
    return db;
}

void Procedures::setDataUser(QMap<QString, QVariant> DataUser)
{
    lDataUser = DataUser;
}

QMap<QString,QVariant> Procedures::getDataUser()
{
    return lDataUser;
}

QMap<QString,QVariant> Procedures::setDataOtherUser(int id)
{
    OtherUser = fdatauser(id, gidLieuExercice, db);
    return OtherUser;
}

void Procedures::setDirImagerie()
{
    DirStockageImages = "";
    QString req = "select DirImagerie from " NOM_TABLE_PARAMSYSTEME;
    QSqlQuery quer(req,getDataBase());
    if (quer.size()>0)
    {
        quer.first();
        DirStockageImages = quer.value(0).toString();
    }
}

QString Procedures::DirImagerie()
{
    return DirStockageImages;
}


void Procedures::setidUser(int id)
{
   gidUser = id;
}

void Procedures::setFicheRefractionOuverte(bool a)
{
    dlgrefractionouverte = a;
}

bool Procedures::FicheRefractionOuverte()
{
    return dlgrefractionouverte;
}

void Procedures::setMapDatas()
{
    gmapIcons["OK"]             = giconOK;
    gmapIcons["Annul"]          = giconAnnul;
    gmapIcons["Euro"]           = giconEuro;
    gmapIcons["EuroCount"]      = giconEuroCount;
    gmapIcons["FermeAppuye"]    = giconFermeAppuye;
    gmapIcons["FermeRelache"]   = giconFermeRelache;
    gmapIcons["Help"]           = giconHelp;
    gmapIcons["Null"]           = giconNull;
}

QMap<QString, QIcon> Procedures::MapIcons()
{
    return gmapIcons;
}

int Procedures::getidUser()
{
   return gidUser;
}

void Procedures::setListeComptesUser(int idUser)
{
    ListeComptesUser                = new QStandardItemModel();
    ListeComptesUserAvecDesactive   = new QStandardItemModel();
    QStandardItem *pitem0, *pitem1;
    QStandardItem *oitem0, *oitem1;
    QString req = "SELECT idCompte, NomCompteAbrege, desactive FROM " NOM_TABLE_COMPTES " WHERE idUser = " + QString::number(idUser);
    //qDebug() << req;
    QSqlQuery ChercheNomsComptesQuery (req,getDataBase());
    if (!TraiteErreurRequete(ChercheNomsComptesQuery,req, tr("Impossible de retrouver les comptes de l'utilisateur!")))
    {
        for (int i = 0; i < ChercheNomsComptesQuery.size(); i++)
        {
            ChercheNomsComptesQuery.seek(i);
            pitem0 = new QStandardItem(ChercheNomsComptesQuery.value(0).toString());
            pitem1 = new QStandardItem(ChercheNomsComptesQuery.value(1).toString());
            QList<QStandardItem*> listitems;
            listitems << pitem0 << pitem1;
            ListeComptesUserAvecDesactive    ->appendRow(listitems);
            oitem0 = new QStandardItem(ChercheNomsComptesQuery.value(0).toString());
            oitem1 = new QStandardItem(ChercheNomsComptesQuery.value(1).toString());
            QList<QStandardItem*> olistitems;
            olistitems << oitem0 << oitem1;
            if(ChercheNomsComptesQuery.value(2) != 1)
                ListeComptesUser    ->appendRow(olistitems);
        }
    }
}

QStandardItemModel* Procedures::getListeComptesUser()
{
    return ListeComptesUser;
}

QStandardItemModel* Procedures::getListeComptesUserAvecDesactive()
{
    return ListeComptesUserAvecDesactive;
}

void Procedures::setListeComptesEncaissmtUser(int idUser)
{
    if (OtherUser["idUser"].toInt() != idUser)
        setDataOtherUser(idUser);
    ListeComptesEncaissUser                 = new QStandardItemModel();
    ListeComptesEncaissUserAvecDesactive    = new QStandardItemModel();
    QString usercpt = (OtherUser["Employeur"].toInt() != QVariant(QVariant::Int)? OtherUser["Employeur"].toString(): QString::number(idUser));       //il y a un employeur
    QString req = "select idCompte, nomcompteabrege, desactive, userlogin from " NOM_TABLE_COMPTES " cpt"
                  " left outer join " NOM_TABLE_UTILISATEURS " usr on  usr.iduser = cpt.iduser"
                  " where cpt.idUser = " + usercpt;
    //qDebug() << req;
    QStandardItem *pitem0, *pitem1;
    QStandardItem *oitem0, *oitem1;
    QSqlQuery ChercheNomsComptesQuery (req,getDataBase());
    if (!TraiteErreurRequete(ChercheNomsComptesQuery,req, tr("Impossible de retrouver les comptes de l'utilisateur!")))
    {
        for (int i = 0; i < ChercheNomsComptesQuery.size(); i++)
        {
            ChercheNomsComptesQuery.seek(i);
            pitem0 = new QStandardItem(ChercheNomsComptesQuery.value(3).toString() + "/" + ChercheNomsComptesQuery.value(1).toString());
            pitem1 = new QStandardItem(ChercheNomsComptesQuery.value(0).toString());
            QList<QStandardItem*> listitems;
            listitems << pitem0 << pitem1;
            ListeComptesEncaissUserAvecDesactive    ->appendRow(listitems);
            oitem0 = new QStandardItem(ChercheNomsComptesQuery.value(3).toString() + "/" + ChercheNomsComptesQuery.value(1).toString());
            oitem1 = new QStandardItem(ChercheNomsComptesQuery.value(0).toString());
            QList<QStandardItem*> olistitems;
            olistitems << oitem0 << oitem1;
            if(ChercheNomsComptesQuery.value(2).toInt() != 1)
                ListeComptesEncaissUser    ->appendRow(olistitems);
            //qDebug() << oitem0->text() + " - " + oitem1->text();
        }
    }
    if (ListeComptesEncaissUser->findItems(OtherUser["idCompteEncaissHonoraires"].toString(), Qt::MatchExactly, 1).size()==0)
    {
        QStandardItem *nitem0, *nitem1;
        nitem0 = new QStandardItem(OtherUser["NomUserEncaissHonoraires"].toString() + "/" + OtherUser["NomCompteEncaissHonoraires"].toString());
        nitem1 = new QStandardItem(OtherUser["idCompteEncaissHonoraires"].toString());
        QList<QStandardItem*> nlistitems;
        nlistitems << nitem0 << nitem1;
        ListeComptesEncaissUser->insertRow(0, nlistitems);
    }
    if (ListeComptesEncaissUserAvecDesactive->findItems(OtherUser["idCompteEncaissHonoraires"].toString(), Qt::MatchExactly, 1).size()==0)
    {
        QStandardItem *nitem0, *nitem1;
        nitem0 = new QStandardItem(OtherUser["NomUserEncaissHonoraires"].toString() + "/" + OtherUser["NomCompteEncaissHonoraires"].toString());
        nitem1 = new QStandardItem(OtherUser["idCompteEncaissHonoraires"].toString());
        QList<QStandardItem*> nlistitems;
        nlistitems << nitem0 << nitem1;
        ListeComptesEncaissUserAvecDesactive->insertRow(0, nlistitems);
    }
}

QStandardItemModel* Procedures::getListeComptesEncaissmtUser()
{
    return ListeComptesEncaissUser;
}

QStandardItemModel* Procedures::getListeComptesEncaissmtUserAvecDesactive()
{
    return ListeComptesEncaissUserAvecDesactive;
}

QString Procedures::getLogin(int idUser)
{
    if (gListeUsersModel->rowCount()==0)
        return "";
    if (gListeUsersModel->findItems(QString::number(idUser)).size()==0)
        return "";
    int row = gListeUsersModel->findItems(QString::number(idUser)).at(0)->row();
    if (row > -1)
        return gListeUsersModel->item(row,1)->text();
    else
        return "";
}

QStandardItemModel* Procedures::getListeLiberaux()
{
    return gListeLiberauxModel;
}

QStandardItemModel* Procedures::getListeParents()
{
    return gListeParentsModel;
}

QStandardItemModel* Procedures::getListeSuperviseurs()
{
    return gListeSuperviseursModel;
}

QStandardItemModel* Procedures::getListeUsers()
{
    return gListeUsersModel;
}

void Procedures::setlisteUsers()
{
    gListeUsersModel            = new QStandardItemModel();
    gListeLiberauxModel         = new QStandardItemModel();
    gListeParentsModel          = new QStandardItemModel();
    gListeSuperviseursModel     = new QStandardItemModel();
    QStandardItem       *pitem0, *pitem1, *pitem2, *pitem3, *pitem4;
    QStandardItem       *litem0, *litem1, *litem2, *litem3, *litem4;
    QStandardItem       *oitem0, *oitem1, *oitem2, *oitem3, *oitem4, *oitem5, *oitem6;
    QStandardItem       *mitem0, *mitem1, *mitem2, *mitem3, *mitem4;
    QString req = "select usr.iduser, userlogin, soignant, responsableactes, UserEnregHonoraires, idCompteEncaissHonoraires, nomcompteabrege"
                  " from " NOM_TABLE_UTILISATEURS " as usr"
                  " left outer join " NOM_TABLE_COMPTES " on idcompteencaisshonoraires = idCompte"
                  " where userdesactive is null";
    //qDebug() << req;
    QSqlQuery usrquer(req, db);
    for (int i=0; i<usrquer.size(); i++)
    {
        //1. la liste des utilisateurs
        usrquer.seek(i);
        pitem0 = new QStandardItem(usrquer.value(0).toString());
        pitem1 = new QStandardItem(usrquer.value(1).toString());
        pitem2 = new QStandardItem(usrquer.value(2).toString());
        pitem3 = new QStandardItem(usrquer.value(3).toString());
        pitem4 = new QStandardItem(usrquer.value(4).toString());
        QList<QStandardItem*> listitems;
        listitems << pitem0 << pitem1 << pitem2 << pitem3 << pitem4;
        gListeUsersModel    ->appendRow(listitems);

        bool soignant       = (usrquer.value(2).toInt() == 1 || usrquer.value(2).toInt() == 2 || usrquer.value(2).toInt() == 3);
        bool pasremplacant  = (usrquer.value(4).toInt() != 3);
        bool liberal        = (usrquer.value(4).toInt() == 1);
        bool responsable    = (usrquer.value(3).toInt() == 1 || usrquer.value(3).toInt() == 2);

        if ( soignant && responsable)
        {
            //2. la liste des soignants superviseurs (-> soignants qui peuvent être responsables)
            // on ne peut pas reprendre les mêmes items pour les insérer dans un autre model
            litem0 = new QStandardItem(usrquer.value(0).toString());
            litem1 = new QStandardItem(usrquer.value(1).toString());
            litem2 = new QStandardItem(usrquer.value(2).toString());
            litem3 = new QStandardItem(usrquer.value(3).toString());
            litem4 = new QStandardItem(usrquer.value(4).toString());
            //qDebug() <<usrquer.value(1).toString();
            QList<QStandardItem*> listitems2;
            listitems2 << litem0 << litem1 << litem2 << litem3 << litem4;
            gListeSuperviseursModel    ->appendRow(listitems2);
        }
        if (soignant && liberal)
        {
            //3. la liste des soignants liberaux (-> soignants qui enregistrent une compta)
            // on ne peut pas reprendre les mêmes items pour les insérer dans un autre model
            mitem0 = new QStandardItem(usrquer.value(0).toString());
            mitem1 = new QStandardItem(usrquer.value(1).toString());
            mitem2 = new QStandardItem(usrquer.value(2).toString());
            mitem3 = new QStandardItem(usrquer.value(3).toString());
            mitem4 = new QStandardItem(usrquer.value(4).toString());
            QList<QStandardItem*> listitems2;
            listitems2 << mitem0 << mitem1 << mitem2 << mitem3 << mitem4;
            gListeLiberauxModel   ->appendRow(listitems2);
        }

        if (soignant && pasremplacant)
        {
            //4. la liste des parents possibles et leurs comptes d'encaissement(-> tous les soignants qui ne sont pas remplaçants)
            // on ne peut pas reprendre les mêmes items pour les insérer dans un autre model
            oitem0 = new QStandardItem(usrquer.value(0).toString());
            oitem1 = new QStandardItem(usrquer.value(1).toString());
            oitem2 = new QStandardItem(usrquer.value(2).toString());
            oitem3 = new QStandardItem(usrquer.value(3).toString());
            oitem4 = new QStandardItem(usrquer.value(4).toString());
            oitem5 = new QStandardItem(usrquer.value(5).toString());
            oitem6 = new QStandardItem(usrquer.value(6).toString());
            QList<QStandardItem*> listitems3;
            listitems3 << oitem0 << oitem1 << oitem2 << oitem3 << oitem4 << oitem5 << oitem6;
            gListeParentsModel  ->appendRow(listitems3);
        }
    }
}

QString Procedures::getMDPAdmin()
{
    QSqlQuery mdpquer("select mdpadmin from " NOM_TABLE_PARAMSYSTEME,db);
    mdpquer.first();
    if (mdpquer.value(0).toString() == "")
        QSqlQuery("update " NOM_TABLE_PARAMSYSTEME " set mdpadmin = '" NOM_MDPADMINISTRATEUR "'", db);
    return (mdpquer.value(0).toString() != ""? mdpquer.value(0).toString() : NOM_MDPADMINISTRATEUR);
}

int Procedures::getMAXligneBanque()
{
    int a(0), b(0);
    QString req = "select max(idligne) from " NOM_TABLE_ARCHIVESBANQUE;
    QSqlQuery quer(req,getDataBase());
    if (quer.size()>0){
        quer.first();
        a = quer.value(0).toInt();
    }
    req = "select max(idligne) from " NOM_TABLE_LIGNESCOMPTES;
    QSqlQuery quer2(req,getDataBase());
    if (quer2.size()>0){
        quer2.first();
        if (quer2.value(0).toInt()>a)
            b = quer2.value(0).toInt();
    }
    return (((a<b)?b:a)+1);
}

int Procedures::getModeConnexion()
{
    return gMode;
}

void Procedures::setNomImprimante(QString NomImprimante)
{
    gnomImprimante = NomImprimante;
}

QString Procedures::getNomImprimante()
{
    return gnomImprimante;
}

void Procedures::setrx(QRegExp rxS)
{
   rx = rxS;
}

QRegExp Procedures::getrx()
{
    return rx;
}

void Procedures::setrxAdresse(QRegExp rxA)
{
   rxAdresse = rxA;
}

QRegExp Procedures::getrxAdresse()
{
    return rxAdresse;
}

void Procedures::setrxCot(QRegExp rxC)
{
   rxCot = rxC;
}

QRegExp Procedures::getrxCot()
{
    return rxCot;
}

void Procedures::setrxCP(QRegExp rxC)
{
   rxCP = rxC;
}

QRegExp Procedures::getrxCP()
{
    return rxCP;
}

QRegExp Procedures::getrxIP()
{
    return rxIP;
}

void Procedures::setrxLogin(QRegExp rxL)
{
   rxLogin = rxL;
}

QRegExp Procedures::getrxLogin()
{
    return rxLogin;
}

void Procedures::setrxMail(QRegExp rxM)
{
   rxMail = rxM;
}

QRegExp Procedures::getrxMail()
{
    return rxMail;
}

void Procedures::setrxMdpAdmin(QRegExp rxMDP)
{
   rxMdpAdmin = rxMDP;
}

QRegExp Procedures::getrxMdpAdmin()
{
    return rxMdpAdmin;
}

void Procedures::setrxMdp(QRegExp rxMDP)
{
   rxMdp = rxMDP;
}

QRegExp Procedures::getrxMdp()
{
    return rxMdp;
}

void Procedures::setrxRecherche(QRegExp rxRec)
{
   rxRecherche = rxRec;
}

QRegExp Procedures::getrxRecherche()
{
    return rxRecherche;
}

void Procedures::setrxTel(QRegExp rxT)
{
   rxTel = rxT;
}

QRegExp Procedures::getrxTel()
{
    return rxTel;
}

void Procedures::setrxTabac(QRegExp rxT)
{
   rxTabac = rxT;
}

QRegExp Procedures::getrxTabac()
{
    return rxTabac;
}

void Procedures::setrxVille(QRegExp rxV)
{
   rxVille = rxV;
}

QRegExp Procedures::getrxVille()
{
    return rxVille;
}

int Procedures::TailleEnTete()
{
    return gsettingsIni->value("Param_Imprimante/TailleEnTete").toInt();
}

int Procedures::TailleEnTeteALD()
{
    return gsettingsIni->value("Param_Imprimante/TailleEnTeteALD").toInt();
}

int Procedures::TaillePieddePage()
{
    return gsettingsIni->value("Param_Imprimante/TaillePieddePage").toInt();
}

int Procedures::TaillePieddePageOrdoLunettes()
{
    return gsettingsIni->value("Param_Imprimante/TaillePieddePageOrdoLunettes").toInt();
}

int Procedures::TailleTopMarge()
{
    return gsettingsIni->value("Param_Imprimante/TailleTopMarge").toInt();
}

void Procedures::setVilleParDefaut(QString VilleParDefaut)
{
    lVilleParDefaut = VilleParDefaut;
}

QString Procedures::getVilleParDefaut()
{
    if (lVilleParDefaut != "") return lVilleParDefaut;
    lVilleParDefaut = gsettingsIni->value("Param_Poste/VilleParDefaut").toString();
    return lVilleParDefaut;
}

void Procedures::setPosteImportDocs(bool a)
{
    // Il n'y pas de variables utilisateur globale dans MySQL, on est donc obligé de passer par une procédure stockée pour en simuler une
    // pour créer une procédure avec Qt, séparer le drop du create, ne pas utiliser les délimiteurs et utiliser les retours à la ligne \n\.......
    //if (gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() ==  "YES")

    // si a = true, on se met en poste importateur +/_ prioritaire à la fin suivant le contenu de rufus.ini
    // si a = false, on retire le poste en cours et on met NULL à la place.

    QString IpAdress("NULL");
    QString req = "USE `" NOM_BASE_CONSULTS "`;";
    QSqlQuery quer(req, getDataBase());
    TraiteErreurRequete(quer,req);

    req = "DROP PROCEDURE IF EXISTS " NOM_POSTEIMPORTDOCS ";";
    QSqlQuery quer1(req, getDataBase());
    TraiteErreurRequete(quer1,req);

    if (a)
        IpAdress = QHostInfo::localHostName()  + ((gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() ==  "YES")? " - prioritaire" : "");
    req = "CREATE PROCEDURE " NOM_POSTEIMPORTDOCS "()\n\
          BEGIN\n\
          SELECT '" + IpAdress + "';\n\
          END ;";
    QSqlQuery quer2(req, getDataBase());
    TraiteErreurRequete(quer2,req);
}

QString Procedures::PosteImportDocs()
{
    QString req = "CALL " NOM_BASE_CONSULTS "." NOM_POSTEIMPORTDOCS;
    QSqlQuery quer(req, getDataBase());
    if (quer.size()==-1)
        return "";
    quer.first();
    //qDebug() << "nbre reponses = " + QString::number(quer.size()) << NOM_POSTEIMPORTDOCS " = " + quer.value(0).toString();
    return quer.value(0).toString();
}

bool Procedures::Verif_secure_file_priv()
{
    if (Var_secure_file_priv()==QString())
    {
        UpMessageBox::Watch(0, tr("Configuration du serveur défectueuse"),
                            tr("La variable MySQL 'secure_file_priv' est positionnée à 'NULL'\n"
                               "Vous ne pourrez pas afficher les documents d'imagerie\n"
                               "Veuillez modifier la valeur de cette variable en la faisant pointer sur un sous-dossier du dossier"
                               "'/Users/Votrenomdutilisateur/Rufus' sur le serveur\n"
                               "Reportez-vous au bas de la page\n"
                               "https://www.rufusvision.org/installation-du-serveur-mysql.html\n"
                               "pour savoir comment modifier le fichier de configuration my.cnf\n"
                               "de MySQL sur le serveur puis redémarrez le serveur"));
        return false;
    }
    return true;
}

QString Procedures::Var_secure_file_priv()
{
    QString msg;
    QSqlQuery secquer("SHOW VARIABLES LIKE \"secure_file_priv\";", db);
    secquer.first();
    msg = secquer.value(1).toString();
    if (msg == "NULL")
        msg = QString();
    return msg;
}

QStringList Procedures::DecomposeScriptSQL(QString nomficscript)
{
    QStringList listinstruct;
    QFile file(nomficscript);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        listinstruct << "";
        return QStringList();
    }
    QString queryStr(file.readAll());
    file.close();
    // On retire tous les commentaires, les tabulations, les espaces ou les retours à la ligne multiples
    //        queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/)",   QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("(^;\\n)",                   QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("(--.*\\n)",                 QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
    queryStr = queryStr.replace(QRegularExpression("( +)",                      QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
    queryStr = queryStr.replace(QRegularExpression("((\\t)+)",                  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
    queryStr = queryStr.replace(QRegularExpression("(^ *)",                     QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
    queryStr = queryStr.replace(QRegularExpression("((\\n)+)",                  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
    //Retire les espaces en début et fin de string
    queryStr = queryStr.trimmed();

    QString matched, delimiter, Atraiter;
    QRegularExpression re("^(\\s|\\n)*DELIMITER\\s*(.|\\n)*END\\s*.\\n"); //isole les créations de procédure SQL dans le script

    while (queryStr.size()>0 && queryStr.contains(";"))
    {
        //Edit(queryStr);
        QRegularExpressionMatch match = re.match(queryStr);
        if (match.hasMatch())  // --> c'est une procédure à créer
        {
            matched     = match.capturedTexts().at(0);
            Atraiter    = matched.trimmed();
            //Edit(Atraiter);
            delimiter   = Atraiter.data()[Atraiter.size()-1];
            //Edit(delimiter);
            Atraiter.replace(QRegularExpression("DELIMITER\\s*"),"");
            Atraiter.replace(delimiter,"");
            Atraiter = Atraiter.replace(QRegularExpression("(^ *)",     QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
            Atraiter = Atraiter.replace(QRegularExpression("(^(\\n)+)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
            Atraiter = Atraiter.replace(QRegularExpression("((\\n)+)",  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
            //Edit(Atraiter);
            queryStr.replace(0,matched.size(),"");
        }
        else                    // -- c'est une requête SQL
        {
            matched = queryStr.split(";\n", QString::SkipEmptyParts).at(0);
            Atraiter = matched.trimmed()+ ";";
            queryStr.replace(0,matched.size()+2,"");
            queryStr = queryStr.replace(QRegularExpression("((\\n)+)",  QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "\n");
        }
        queryStr = queryStr.replace(QRegularExpression("(^(\\n)*)",     QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), "");
        listinstruct << Atraiter;
    }
    /*
    QString txt;
    for (int i=0; i<listinstruct.size();i++)
        txt += listinstruct.at(i) + "\n--\n";
    Edit(txt);
    */

    return listinstruct;

    /* POUR CREER DES PROCEDURES AVEC Qt
    1. il faut refaire un Use 'DATABASE'
    2. l'instruction de définition du DELIMITER ne doit pas être utilisée

    Dans les faits
    d'abord
    QString req = "USE `Rufus`;";
    QSQLquery (req, getDataBase());

    puis
    req = "DROP PROCEDURE IF EXISTS MAJ16;";
    QSQLquery (req, getDataBase());

    enfin
    QString req = "DELIMITER |"
        "CREATE PROCEDURE MAJ16()\n"
            "BEGIN\n"
            "DECLARE tot INT DEFAULT 1;\n"
            "SELECT COUNT(*) INTO tot FROM\n"
                "(SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS\n"
                "WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'TextOrigine') as chp;\n"
            "IF tot=0\n"
            "THEN\n"
                "ALTER TABLE `Impressions`\n"
                "ADD COLUMN `TextOrigine` blob NULL DEFAULT NULL AFTER `TextCorps`;\n"
            "END IF;\n"
            "END|;";
     ne marche pas et doit être remplacé par
     QString req =
            "CREATE PROCEDURE MAJ16()\n"
            "BEGIN\n"
            "DECLARE tot INT DEFAULT 1;\n"
            "SELECT COUNT(*) INTO tot FROM\n"
                "(SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS\n"
                "WHERE TABLE_NAME='Impressions' AND COLUMN_NAME = 'TextOrigine') as chp;\n"
            "IF tot=0\n"
            "THEN\n"
                "ALTER TABLE `Impressions`\n"
                "ADD COLUMN `TextOrigine` blob NULL DEFAULT NULL AFTER `TextCorps`;\n"
            "END IF;\n"
            "END;";

    */
}

QString Procedures::getBase()
{
    QString Base = "";
    if (gMode == Poste)
        Base = "BDD_POSTE";
    else if (gMode == ReseauLocal)
        Base = "BDD_LOCAL";
    else if (gMode == Distant)
        Base = "BDD_DISTANT";
    return Base;
}

void Procedures::InfoBase()
{
    UpMessageBox::Watch(0, tr("Connexion à la base de données!"),
                        tr("Vos paramètres de connexion") +
                           "\n" + tr("Serveur     ") + "\n ->\t" + db.hostName() +
                           "\n" + tr("databaseName") + "\n ->\t" + db.databaseName() +
                           "\n" + tr("Login       ") + "\n ->\t" + db.userName() +
                           "\n" + tr("password    ") + "\n ->\t" + db.password() +
                           "\n" + tr("port        ") + "\n ->\t" + QString::number(db.port()));
}

bool Procedures::ReinitBase()
{
    QString req = "select NomPosteConnecte from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte <> '" + QHostInfo::localHostName().left(60) + "'";
    QSqlQuery postesquer(req,db);
    if (postesquer.size() > 0)
    {
        postesquer.first();
        UpMessageBox::Watch(0, tr("Autres postes connectés!"),
                               tr("Vous ne pouvez pas effectuer d'opération de sauvegarde/restauration sur la base de données"
                                  " si vous n'êtes pas le seul poste connecté.\n"
                                  "Le poste ") + postesquer.value(0).toString() + tr(" est aussi connecté"));
        return false;
    }
    UpMessageBox msgbox;
    UpSmallButton *OKBouton = new UpSmallButton;
    UpSmallButton *AnnulBouton = new UpSmallButton;
    msgbox      .setText(tr("Réinitialisation du programme!"));
    msgbox      .setInformativeText(tr("Si vous confirmez la réinitialisation, une sauvegarde de la base patients, du fichier Rufus.ini et des fichiers ressources sera réalisée"
                                      " puis le programme sera arrêté.\n"
                                      "Au redémarrage du programme, une fenêtre de dialogue vous permettra"
                                      " de réinitialiser toutes les données en cliquant sur le bouton \"Premier démarrage de Rufus\".\n"
                                      "Vous pourrez encore annuler la réinitialisation"
                                      " en cliquant sur le bouton \"Restaurer le fichier de paramétrage\""
                                      " et en choisissant le fichier RufusBackup.ini"));
    msgbox      .setIcon(UpMessageBox::Warning);
    OKBouton    ->setText(tr("Réinitialiser"));
    AnnulBouton ->setText(tr("Annuler"));
    msgbox      .addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
    msgbox      .addButton(OKBouton, UpSmallButton::STARTBUTTON);
    msgbox      .exec();
    if (msgbox.clickedButton() == OKBouton)
    {
        if (!ImmediateBackup(true))
            return false;
        QFile FichierIni(gnomFichIni);
        if (FichierIni.exists())
        {
            QFile FichierBup(QDir::homePath() + NOMDIR_RUFUS + "/RufusBackup.ini");
            if (FichierBup.exists())
                FichierBup.remove();
            FichierIni.copy(QDir::homePath() + NOMDIR_RUFUS + "/RufusBackup.ini");
            FichierIni.remove();
        }
        UpMessageBox::Information(0, tr("Arrêt du programme!"));
        exit(0);
    }
    return false;
}

void Procedures::RestoreFontAppliAndGeometry()
{
    // On essaie de retrouver la police écran enregistrée par l'utilisateur, sinon, on prend celle par défaut
    QString fontrequete = "select UserPoliceEcran, UserPoliceAttribut from " NOM_TABLE_UTILISATEURS " where idUser = " + QString::number(gidUser);
    QSqlQuery fontquery (fontrequete,db);
    TraiteErreurRequete(fontquery,fontrequete,"");
    fontquery.first();
    QString fonteFamily = fontquery.value(0).toString().split(",").at(0);
    bool trouvefont = false;
    if (fonteFamily.size() > 0)
    {
        // on en a trouvé une, on vérifie qu'elle est valide
        QFontDatabase database;
        QStringList listfamilies = database.families(QFontDatabase::Latin);
        for (int i=0;i<listfamilies.size();i++)
        {
            gAppFont.setFamily(QFont().defaultFamily());
            if (listfamilies.at(i) == fonteFamily)
            {
                trouvefont = true;
                gAppFont.setFamily(fonteFamily);
                gAppFont.setStyle(QFont::StyleNormal);
                gAppFont.setWeight(QFont::Normal);
                QString             FontAttribut;
                FontAttribut     = fontquery.value(1).toString();
                if (!FontAttribut.contains("Regular",Qt::CaseInsensitive) && FontAttribut != "")
                {
                    if (FontAttribut.contains("Italic",Qt::CaseInsensitive))
                        gAppFont.setStyle(QFont::StyleItalic);
                    if (FontAttribut.contains("Light",Qt::CaseInsensitive))
                        gAppFont.setWeight(QFont::Light);
                    if (FontAttribut.contains("Normal",Qt::CaseInsensitive))
                        gAppFont.setWeight(QFont::Normal);
                    if (FontAttribut.contains("SemiBold",Qt::CaseInsensitive))
                        gAppFont.setWeight(QFont::DemiBold);
                    if (FontAttribut.contains("Bold",Qt::CaseInsensitive))
                        gAppFont.setWeight(QFont::Bold);
                    if (FontAttribut.contains("Black",Qt::CaseInsensitive))
                        gAppFont.setWeight(QFont::Black);
                }
                i = listfamilies.size();
            }
        }
    }

    if (!trouvefont)
     {
        gAppFont = QFont(POLICEPARDEFAUT);
        gAppFont.setPointSize(POINTPARDEFAUT);
    }

    for (int i = 5; i < 30; i++)
    {
        gAppFont.setPointSize(i);
        QFontMetrics fm(gAppFont);
        int Htaille = fm.width("date de naissance");
        if (Htaille > 108 || fm.height()*1.1 > 20)
        {
            gAppFont.setPointSize(i-1);
            i=30;
        }
    }
}

QFont Procedures::AppFont()
{
    return gAppFont;
}


double Procedures::CalcBaseSize()
{
    double basesize = 0;
    QString sizereq = "SELECT SUM(SizeMB) from "
                      "(SELECT table_schema, round(sum(data_length+index_length)/1024/1024,4) AS SizeMB FROM information_schema.tables"
                      " where table_schema = 'ComptaMedicale'"
                      " or table_schema = 'Ophtalmologie'"
                      " or table_schema = 'rufus'"
                      " GROUP BY table_schema)"
                      " as bdd";
    QSqlQuery sizequer (sizereq,db);
    if (sizequer.size()>0)
    {
        sizequer.first();
        basesize = sizequer.value(0).toDouble();
    }
    return basesize;
}

QMap<QString, double> Procedures::dir_size(const QString DirPath)
{
    QMap<QString, double>      DataDir;
    double sizex = 0;
    double nfiles = 0;

    QDir dir(DirPath);
    if(!dir.exists())
    {
        DataDir["Size"]= 0;
        DataDir["Nfiles"]= 0;
        return DataDir;
    }
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();

    for(int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        sizex += (fileInfo.isDir()) ? this->dir_size(fileInfo.absoluteFilePath())["Size"]: fileInfo.size();
        nfiles += (fileInfo.isDir()) ? this->dir_size(fileInfo.absoluteFilePath())["Nfiles"] : i+1;
    }
    DataDir["Size"]= sizex;
    DataDir["Nfiles"]= nfiles;
    return DataDir;
}

QString Procedures::getExpressionSize(double size)
{
    QString com = "Mo";
    if (size>1024)
    {
        com = "Go";
        size /= 1024;
        if (size>1024)
        {
            com = "To";
            size /= 1024;
        }
    }
    return QString::number(size,'f',2) + com;
}

void Procedures::AskBupRestore(bool Restore, QString pathorigin, QString pathdestination, bool OKini, bool OKRssces, bool OKimages, bool OKvideos)
{
    QMap<QString,double>      DataDir;
    // taille de la base de données ----------------------------------------------------------------------------------------------------------------------------------------------
    BaseSize = 0;
    if (Restore)
    {
        QStringList filters, listnomsfilestorestore;
        filters << "*.sql";
        for (int j=0; j<QDir(pathorigin).entryList(filters).size(); j++)
            listnomsfilestorestore << pathorigin + "/" + QDir(pathorigin).entryList(filters).at(j);
        for (int i=0; i<listnomsfilestorestore.size(); i++)
            BaseSize += QFile(listnomsfilestorestore.at(i)).size()/1024/1024;
    }
    else
        BaseSize = CalcBaseSize();
    ImagesSize = 0;
    VideosSize = 0;
    // espace libre sur le disque ------------------------------------------------------------------------------------------------------------------------------------------------
    FreeSpace = QStorageInfo(pathdestination).bytesAvailable();
    FreeSpace = FreeSpace/1024/1024;
    //qDebug() << QStorageInfo(dirbkup).bytesAvailable();
    //qDebug() << QString::number(FreeSpace,'f',0);

    gAskBupRestore = new UpDialog();
    gAskBupRestore->setModal(true);
    gAskBupRestore->setWindowTitle(Restore? tr("Dossiers à restaurer") : tr("Dossiers à sauvegarder"));
    QVBoxLayout *globallay = dynamic_cast<QVBoxLayout*>(gAskBupRestore->layout());
    int labelsize = 15;

    if (Restore)
    {
        QHBoxLayout *layini = new QHBoxLayout;
        UpLabel *labelini = new UpLabel();
        labelini->setVisible(false);
        labelini->setFixedSize(labelsize, labelsize);
        layini->addWidget(labelini);
        UpCheckBox *Inichk  = new UpCheckBox();
        Inichk->setText("fichier de paramètrage Rufus.ini");
        Inichk->setEnabled(OKini);
        Inichk->setChecked(OKini);
        Inichk->setAccessibleDescription("ini");
        layini->addWidget(Inichk);
        layini->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
        globallay->insertLayout(0, layini);

        QHBoxLayout *layRssces = new QHBoxLayout;
        UpLabel *labelrssces = new UpLabel();
        labelrssces->setVisible(false);
        labelrssces->setFixedSize(labelsize, labelsize);
        layRssces->addWidget(labelrssces);
        UpCheckBox *Rssceschk  = new UpCheckBox();
        Rssceschk->setText("fichier ressources d'impression");
        Rssceschk->setEnabled(OKRssces);
        Rssceschk->setChecked(OKRssces);
        Rssceschk->setAccessibleDescription("ressources");
        layRssces->addWidget(Rssceschk);
        layRssces->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
        globallay->insertLayout(0, layRssces);
    }
    if (OKvideos)
    {
        // taille du dossier video ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = dir_size(pathorigin + NOMDIR_VIDEOS);
        VideosSize = DataDir["Size"]/1024/1024;
        QHBoxLayout *layVideos = new QHBoxLayout;
        UpLabel *labeVideos = new UpLabel();
        labeVideos->setVisible(false);
        labeVideos->setFixedSize(labelsize, labelsize);
        layVideos->addWidget(labeVideos);
        UpCheckBox *Videoschk  = new UpCheckBox();
        Videoschk->setText("Videos");
        Videoschk->setEnabled(OKvideos || !Restore);
        Videoschk->setChecked(OKvideos || !Restore);
        Videoschk->setAccessibleDescription("videos");
        layVideos->addWidget(Videoschk);
        layVideos->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
        UpLabel *lblvolvid = new UpLabel();
        lblvolvid->setText(getExpressionSize(VideosSize));
        layVideos->addWidget(lblvolvid);
        globallay->insertLayout(0, layVideos);
        connect(Videoschk, SIGNAL(clicked(bool)), this,    SLOT(Slot_CalcTimeBupRestore()));
    }
    if (OKimages)
    {
        // taille du dossier Images ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = dir_size(pathorigin + NOMDIR_IMAGES);
        ImagesSize = DataDir["Size"]/1024/1024;
        QHBoxLayout *layImges = new QHBoxLayout;
        UpLabel *labelmges = new UpLabel();
        labelmges->setVisible(false);
        labelmges->setFixedSize(labelsize, labelsize);
        layImges->addWidget(labelmges);
        UpCheckBox *Imgeschk  = new UpCheckBox();
        Imgeschk->setText("Images");
        Imgeschk->setEnabled(OKimages || !Restore);
        Imgeschk->setChecked(OKimages || !Restore);
        Imgeschk->setAccessibleDescription("images");
        layImges->addWidget(Imgeschk);
        layImges->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
        UpLabel *lblvolimg = new UpLabel();
        lblvolimg->setText(getExpressionSize(ImagesSize));
        layImges->addWidget(lblvolimg);
        globallay->insertLayout(0, layImges);
        connect(Imgeschk, SIGNAL(clicked(bool)), this,    SLOT(Slot_CalcTimeBupRestore()));
    }

    QHBoxLayout *layBDD = new QHBoxLayout;
    UpLabel *labelBDD = new UpLabel();
    labelBDD->setVisible(false);
    labelBDD->setFixedSize(labelsize, labelsize);
    layBDD->addWidget(labelBDD);
    UpCheckBox *BDDchk  = new UpCheckBox();
    BDDchk->setText("base de données");
    BDDchk->setChecked(true);
    BDDchk->setAccessibleDescription("base");
    layBDD->addWidget(BDDchk);
    layBDD->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    UpLabel *lblvolbase = new UpLabel();
    lblvolbase->setText(getExpressionSize(BaseSize));
    layBDD->addWidget(lblvolbase);
    globallay->insertLayout(0, layBDD);


    QHBoxLayout *layResume = new QHBoxLayout;
    labelResume = new UpLabel();
    layResume->addWidget(labelResume);
    globallay->insertLayout(globallay->count()-1, layResume);

    QHBoxLayout *layVolumeLibre = new QHBoxLayout;
    labelVolumeLibre = new UpLabel();
    layVolumeLibre->addWidget(labelVolumeLibre);
    globallay->insertLayout(globallay->count()-1, layVolumeLibre);

    connect(BDDchk, SIGNAL(clicked(bool)), this,    SLOT(Slot_CalcTimeBupRestore()));

    //globallay->setSizeConstraint(QLayout::SetFixedSize);
    gAskBupRestore->setFixedWidth(400);
    gAskBupRestore->AjouteLayButtons(UpDialog::ButtonOK);
    connect(gAskBupRestore->OKButton,    SIGNAL(clicked(bool)), gAskBupRestore, SLOT(accept()));
    Slot_CalcTimeBupRestore();
}

void Procedures::Slot_CalcTimeBupRestore()
{
    double time(0), volume(0);
    QList<UpCheckBox*> listchk = gAskBupRestore->findChildren<UpCheckBox*>();
    for (int i= 0; i<listchk.size(); i++)
    {
        if (listchk.at(i)->accessibleDescription() == "base")
        {
            if (listchk.at(i)->isChecked())
                volume += BaseSize;
        }
        if (listchk.at(i)->accessibleDescription() == "images")
        {
            if (listchk.at(i)->isChecked())
                volume += ImagesSize;
        }
        if (listchk.at(i)->accessibleDescription() == "videos")
        {
            if (listchk.at(i)->isChecked())
                volume += VideosSize;
        }
    }
    time = (volume/1024 /2)*60000; //duréée approximative de sauvegarde en ms
    QString Volumelitteral = getExpressionSize(volume);
    QString timelitteral;
    if (Volumelitteral.right(2) == "Go")
        timelitteral = QString::number(time/60000,'f',0) + tr(" minutes");
    else if (Volumelitteral.right(2) == "To")
        timelitteral = QString::number(time/60000/60,'f',0) + tr(" heures");
    else
        timelitteral = tr("moins d'une minute");
    QString color = FreeSpace>volume? "green": "red";
    QString msg = tr("Volume à transférer: ") + " <font color=\""+ color + "\"><b>" + Volumelitteral + "</b></font>";
    labelResume->setText(msg + " - " + tr("Temps estimé: ") + timelitteral);
    QString Volumelibre = getExpressionSize(FreeSpace);
    msg = tr("Volume disponible sur le disque: ") + " <font color=\""+ color + "\"><b>" + Volumelibre + "</b></font>";

    labelVolumeLibre->setText(msg);
    gAskBupRestore->OKButton->setEnabled(FreeSpace>volume);
}

bool Procedures::RestaureBase(bool BaseVierge, bool PremierDemarrage, bool VerifUserConnectes)
{
    UpMessageBox    msgbox;
    UpSmallButton   *AnnulBouton = new UpSmallButton();
    UpSmallButton   *OKBouton = new UpSmallButton;
    msgbox.setIcon(UpMessageBox::Warning);
    bool echecfile = true;

    if (BaseVierge)
    {
        QString Hote;
        if (gMode == Poste)
            Hote = tr("ce poste");
        else
            Hote = tr("le serveur ") + gsettingsIni->value(getBase() + "/Serveur").toString();
        msgbox.setInformativeText(tr("Vous avez choisi de créer une base vierge sur ") + Hote + "\n" +
                                  tr("Si une base de données Rufus existe sur ce serveur, "
                                     "elle sera définitivement effacée pour être remplacée par cette base vierge.\n"
                                     "Confirmez-vous la suppression des anciennes données?\n"));
        AnnulBouton->setText(tr("Annuler"));
        OKBouton->setText("J'ai compris\nJe confirme");
        msgbox.addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != OKBouton)
            return false;

        if (!VerifMDP((PremierDemarrage? NOM_MDPADMINISTRATEUR : getMDPAdmin()),tr("Saisissez le mot de passe Administrateur")))
            return false;

        QFile BaseViergeFile(QStringLiteral("://basevierge.sql"));
        BaseViergeFile.copy(QDir::homePath() + "/Documents/Rufus/Ressources/basevierge.sql");
        QFile DumpFile(QDir::homePath() + "/Documents/Rufus/Ressources/basevierge.sql");
        if (!DumpFile.open(QIODevice::ReadOnly))
        {
            UpMessageBox::Watch(0, tr("Echec de la restauration"), tr("Le fichier ") + "basevierge.sql" + tr(" n'a pas été trouvé!"));
            return false;
        }
        else if (DumpFile.size() == 0)
        {
            UpMessageBox::Watch(0, tr("Echec de la restauration"), tr("Le fichier ") + "basevierge.sql" + tr(" est vide!"));
            return false;
        }
        else echecfile = false;
        if (!echecfile)
        {
            emit ConnectTimers(false);
            //Suppression de toutes les tables
            VideDatabases();
            int a = 99;

            QStringList listinstruct = DecomposeScriptSQL(QDir::homePath() + "/Documents/Rufus/Ressources/basevierge.sql");
            QSqlQuery query(getDataBase());
            bool e = true;
            foreach(const QString &s, listinstruct)
            {
                //Edit(s);
                query.exec(s);
                if (TraiteErreurRequete(query, s, ""))
                {
                    e = false;
                    break;
                }
            }
            a = (e? 0:99);
            DumpFile.remove();
            if (a==0)
            {
                UpMessageBox::Information(0, tr("Base vierge créée"),tr("La création de la base vierge a réussi."));
                emit ConnectTimers();
                return true;
            }
        }
        UpMessageBox::Watch(0, tr("Impossible d'éxécuter la restauration!") + "\n" + tr("Le fichier ") + "basevierge.sql" + tr(" ne semble pas conforme!"));
        return false;
    }
    else
    {
        if (VerifUserConnectes)
        {
            QString req = "select NomPosteConnecte from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte <> '" + QHostInfo::localHostName().left(60) + "'";
            QSqlQuery postesquer(req,db);
            if (postesquer.size() > 0)
            {
                postesquer.first();
                UpMessageBox::Information(0, tr("Autres postes connectés!"),
                                             tr("Vous ne pouvez pas effectuer d'opération de sauvegarde/restauration sur la base de données"
                                                " si vous n'êtes pas le seul poste connecté.\n"
                                                "Le poste ") + postesquer.value(0).toString() + tr("est aussi connecté"));
                return false;
            }
        }
        UpMessageBox::Information(0, tr("Choix du dossier de sauvegarde"),
                                  tr("Dans la fiche suivante, choisissez le dossier "
                                  "contenant la sauvegarde de la base.\n\n"
                                  "Une fois le dossier sélectionné, "
                                  "la sauvegarde commencera automatiquement.\n"
                                  "Ce processus est long et peut durer plusieurs minutes.\n"
                                  "(environ 1' pour 2 Go)\n"));
        QString dir = QDir::homePath() + NOMDIR_RUFUS;
        QFileDialog dialog(0,tr("Restaurer à partir du dossier") , dir);
        dialog.setViewMode(QFileDialog::List);
        dialog.setFileMode(QFileDialog::DirectoryOnly);
        bool b = (dialog.exec()>0);
        if (!b)
            return false;
        QDir dirtorestore = dialog.directory();
        if (dirtorestore.dirName()=="")
            return false;
        if (!VerifMDP(getMDPAdmin(),tr("Saisissez le mot de passe Administrateur")))
            return false;


        /* ---------------------------------------------------------------------------------------------------------------------------------------------------------
        * Restauration ---------------------------------------------------------------------------------------------------------------------------------------------
            * de la base de données --------------------------------------------------------------------------------------------------------------------------
            * des fichiers de ressources ---------------------------------------------------------------------------------------------------------------------------
            * de Rufus.ini -----------------------------------------------------------------------------------------------------------------------------------------
            * du dossier d'imagerie --------------------------------------------------------------------------------------------------------------------------------
            * des videos -------------------------------------------------------------------------------------------------------------------------------------------
        * -------------------------------------------------------------------------------------------------------------------------------------------------------*/
        bool OKRessces  = false;
        bool OKini      = false;
        bool OKImages   = false;
        bool OKVideos   = false;

        QString msg;

        if (QDir(dirtorestore.absolutePath() + NOMDIR_RESSOURCES).exists())
            if (QDir(dirtorestore.absolutePath() + NOMDIR_RESSOURCES).entryList(QDir::Files | QDir::NoDotAndDotDot).size()>0)
                OKRessces = true;
        if (QFile(dirtorestore.absolutePath() + "/Rufus.ini").exists())
            OKini = true;
        if (QDir(dirtorestore.absolutePath() + NOMDIR_IMAGES).exists())
            if (QDir(dirtorestore.absolutePath() + NOMDIR_IMAGES).entryList(QDir::Dirs).size()>0)
                OKImages = true;
        if (QDir(dirtorestore.absolutePath() + NOMDIR_VIDEOS).exists())
            if (QDir(dirtorestore.absolutePath() + NOMDIR_VIDEOS).entryList(QDir::Files | QDir::NoDotAndDotDot).size()>0)
                OKVideos = true;

        QString NomDirStockageImagerie("");
        QSqlQuery dirquer("select dirimagerie from " NOM_TABLE_PARAMSYSTEME, db);
        dirquer.first();
        NomDirStockageImagerie = dirquer.value(0).toString();
        if (!QDir(NomDirStockageImagerie).exists())
        {
            UpMessageBox::Watch(0,tr("Pas de dossier de stockage valide"),
                                tr("Le dossier spécifié pour le stockage de l'imagerie n'est pas valide") + "\n"
                                + tr("Indiquez un dossier valide dans la boîte de dialogue qui suit"));
            QFileDialog dialogimg(0,tr("Stocker les images dans le dossier") , QDir::homePath() + NOMDIR_RUFUS);
            dialogimg.setViewMode(QFileDialog::List);
            dialogimg.setFileMode(QFileDialog::DirectoryOnly);
            bool b = (dialogimg.exec()>0);
            if (!b)
                return false;
            QDir dirstock = dialogimg.directory();
            NomDirStockageImagerie = dirstock.dirName();
            if (NomDirStockageImagerie=="")
                return false;
            NomDirStockageImagerie = dirstock.absolutePath();
            gsettingsIni->setValue("BDD_POSTE/DossierImagerie", NomDirStockageImagerie);
            QString reqimg = "update " NOM_TABLE_PARAMSYSTEME " set DirImagerie = '" + NomDirStockageImagerie + "'";
            QSqlQuery (reqimg, db);
        }

        AskBupRestore(true, dirtorestore.absolutePath(), NomDirStockageImagerie, OKini, OKRessces, OKImages, OKVideos);
        if (gAskBupRestore->exec()>0)
        {
            emit ConnectTimers(false);
            QList<UpCheckBox*> listchk = gAskBupRestore->findChildren<UpCheckBox*>();
            for (int i= 0; i<listchk.size(); i++)
            {
                if (listchk.at(i)->accessibleDescription() == "base")
                {
                    if (listchk.at(i)->isChecked())
                    {
                        UpMessageBox msgbox;
                        QStringList listnomsfilestorestore;
                        UpSmallButton *AnnulBouton  = new UpSmallButton();
                        UpSmallButton *OKBouton     = new UpSmallButton;
                        msgbox.setIcon(UpMessageBox::Warning);

                        msgbox.setText(tr("Attention"));
                        msgbox.setInformativeText(tr("Vous avez choisi de restaurer la base à partir du dosssier") + "\n"
                                                  + dirtorestore.absolutePath() + ".\n" +
                                                  tr("Si une base de données Rufus existe sur ce serveur, "
                                                     "elle sera définitivement effacée pour être remplacée par cette sauvegarde.\n"
                                                     "Confirmez-vous la suppression des anciennes données?"));
                        AnnulBouton->setText(tr("Annuler"));
                        OKBouton->setText("J'ai compris\nJe confirme");
                        msgbox.addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
                        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
                        msgbox.exec();
                        if (msgbox.clickedButton() != OKBouton)
                        {
                            msg += tr("Base non restaurée");
                            break;
                        }
                        //        if (!SauvegardeBase())
                        //            return;
                        bool echecfile = true;
                        QString NomDumpFile;
                        QStringList filters;
                        filters << "*.sql";
                        QStringList listfichiers = dirtorestore.entryList(filters);
                        listfichiers.removeAt(listfichiers.indexOf("user.sql"));
                        for (int j=0; j<listfichiers.size(); j++)
                            listnomsfilestorestore << dirtorestore.absolutePath() + "/" + listfichiers.at(j);
                        for (int i=0; i<listnomsfilestorestore.size(); i++)
                        {
                            if (!QFile(listnomsfilestorestore.at(i)).open(QIODevice::ReadOnly))
                                msg = tr("Echec de la restauration") + "\n" + tr("Le fichier ") + listnomsfilestorestore.at(i) + tr(" n'a pas été trouvé!");
                            else if (QFile(listnomsfilestorestore.at(i)).size() == 0)
                                msg = tr("Echec de la restauration") + "\n" + tr("Le fichier ") + listnomsfilestorestore.at(i) + tr(" est vide!");
                            else echecfile = false;
                            if (echecfile)
                            {
                                msg += tr("Base non restaurée") + "\n" + msg;
                                break;
                            }
                        }
                        if (!echecfile)
                        {
                            //Suppression de toutes les tables
                            QString Msg = tr("Suppression de l'ancienne base Rufus en cours");
                            Message(Msg, 3000, false);
                            QSqlQuery ("drop database if exists " NOM_BASE_COMPTA,      db);
                            QSqlQuery ("drop database if exists " NOM_BASE_OPHTA,       db);
                            QSqlQuery ("drop database if exists " NOM_BASE_CONSULTS,    db);
                            QSqlQuery ("drop database if exists " NOM_BASE_IMAGES,      db);
                            int a = 99;
                            //Restauration à partir du dossier sélectionné
                            for (int j=0; j<listnomsfilestorestore.size(); j++)
                            {
                                Msg = (tr("Restauration de la base Rufus à partir de ") + listnomsfilestorestore.at(j) + "\n"
                                               + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
                                Message(Msg, 3000);
                                NomDumpFile = listnomsfilestorestore.at(j);
                                QProcess dumpProcess(parent());
                                QStringList args;
                                args << "-u " + db.userName();
                                args << "-p"  + db.password();
                                args << "-h " + db.hostName();
                                args << "-P " + QString::number(db.port());
                                args << "<";
                                args << listnomsfilestorestore.at(j) + "\"";
                                QDir Dir(QCoreApplication::applicationDirPath());
                                Dir.cdUp();
                                QString DirApp = Dir.absolutePath();
                                QString program   = "bash -c \"" + DirApp + NOMDIR_LIBS2;
                                program += "/mysql";
                                QString argstring = program;
                                for (int i=0; i<args.size();i++)    argstring += " " + args.at(i);
                                //Edit(argstring);
                                dumpProcess.start(argstring);
                                dumpProcess.waitForFinished();
                                if (dumpProcess.exitStatus() == QProcess::NormalExit)
                                    a = dumpProcess.exitCode();
                                QString nombase = listfichiers.at(j);
                                nombase.remove(".sql");
                                if (a>0)
                                {
                                    msg += tr("Erreur de restauration de la base") + " " + nombase;
                                    break;
                                }
                                else msg += tr("Restauration de la base") + " " + nombase + " OK\n";
                            }
                        }
                    }
                }
            }
            for (int i= 0; i<listchk.size(); i++)
            {
                if (listchk.at(i)->accessibleDescription() == "ini")
                {
                    if (listchk.at(i)->isChecked())
                    {
                        QString fileini = dirtorestore.absolutePath() + "/Rufus.ini";
                        QFile FichierIni(gnomFichIni);
                        if (FichierIni.exists())
                            FichierIni.remove();
                        QFile rufusini(fileini);
                        rufusini.copy(gnomFichIni);
                        msg += tr("Fichier de paramétrage Rufus.ini restauré\n");
                        Message(tr("Fichier de paramétrage Rufus.ini restauré"), 3000, false);
                    }
                }
            }
            for (int i= 0; i<listchk.size(); i++)
            {
                if (listchk.at(i)->accessibleDescription() == "ressources")
                {
                    if (listchk.at(i)->isChecked())
                    {
                        QDir DirRssces(QDir(dirtorestore.absolutePath() + NOMDIR_RESSOURCES));
                        QDir sauvRssces;
                        if (!sauvRssces.exists(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES))
                            sauvRssces.mkdir(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES);
                        QStringList listnomfic = DirRssces.entryList();
                        for (int i=0; i<listnomfic.size(); i++)
                        {
                            QFile ficACopier(DirRssces.absolutePath() + "/" + listnomfic.at(i));
                            QString nomficACopier = QFileInfo(listnomfic.at(i)).fileName();
                            ficACopier.copy(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES + "/" + nomficACopier);
                        }
                        msg += tr("Fichiers de ressources d'impression restaurés\n");
                        Message(tr("Fichiers de ressources d'impression restaurés"), 3000, false);
                    }
                }
            }
            for (int i= 0; i<listchk.size(); i++)
            {
                if (listchk.at(i)->accessibleDescription() == "images")
                {
                    if (listchk.at(i)->isChecked())
                    {
                        QDir dirrestaureimagerie = QDir(dirtorestore.absolutePath() + NOMDIR_IMAGES);
                        QString dirdestinationimg       =  NomDirStockageImagerie + NOMDIR_IMAGES;
                        QDir DirDestImg(dirdestinationimg);
                        if (DirDestImg.exists())
                            DirDestImg.removeRecursively();
                        if (!DirDestImg.mkdir(dirdestinationimg))
                        {
                            QString Msg = tr("le dossier de destination de l'imagerie n'existe pas");
                            Message(Msg, 3000, false);
                        }
                        else
                        {
                            QString Msg = (tr("Restauration des fichiers d'imagerie\n")
                                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
                            Message(Msg, 3000);
                            Msg = "cp -R " + dirrestaureimagerie.absolutePath() + " " + NomDirStockageImagerie;
                            //qDebug() << Msg;
                            QProcess::execute(Msg);
                            msg += tr("Fichiers d'imagerie restaurés\n");
                            Message(tr("Fichiers d'imagerie restaurés"), 3000, false);
                        }
                    }
                }
            }
            for (int i= 0; i<listchk.size(); i++)
            {
                if (listchk.at(i)->accessibleDescription() == "videos")
                {
                    if (listchk.at(i)->isChecked())
                    {
                        QDir dirrestaurevideo = QDir(dirtorestore.absolutePath() + NOMDIR_VIDEOS);
                        QString dirdestinationvid       =  NomDirStockageImagerie + NOMDIR_VIDEOS;
                        QDir DirDestVid(dirdestinationvid);
                        if (DirDestVid.exists())
                            DirDestVid.removeRecursively();
                        if (!DirDestVid.mkdir(dirdestinationvid))
                        {
                            QString Msg = tr("le dossier de destination des videos n'existe pas");
                            Message(Msg, 3000, false);
                        }
                        else
                        {
                            QString Msg = (tr("Restauration des fichiers videos\n")
                                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
                            Message(Msg, 3000, false);
                            Msg = "cp -R " + dirrestaurevideo.absolutePath() + " " + NomDirStockageImagerie;
                            QProcess::execute(Msg);
                            msg += tr("Fichiers videos restaurés\n");
                            Message(tr("Fichiers videos restaurés"), 3000);
                        }
                    }
                }
            }
            delete gAskBupRestore;
            //qDebug() << msg;
            UpMessageBox::Watch(0,tr("restauration terminée"),msg);
            return true;
        }
        emit ConnectTimers();
    }
    return false;
}

bool Procedures::VerifBaseEtRessources()
{
    int Versionencours  = 9; //correspond aux premières versions de MAJ de la base
    int Version         = VERSION_BASE;
    QString req         = "select VersionBase from " NOM_TABLE_PARAMSYSTEME;
    QSqlQuery MAJBaseQuery(req,getDataBase());
    bool b              = false;
    if (MAJBaseQuery.lastError().type() != QSqlError::NoError || MAJBaseQuery.size()==0)
        b = true;
    else
    {
        MAJBaseQuery.first();
        Versionencours = MAJBaseQuery.value(0).toInt();
        if (Versionencours < Version)
            b = true;
    }
    bool BupDone = false;
    if (b)
    {
        int nbreMAJ = Version - Versionencours;
        for (int i=1; i< nbreMAJ+1; i++)
        {
            Version = Versionencours + i;
            if (!BupDone)
            {
                UpMessageBox msgbox;
                msgbox.setText(tr("Mise à jour de la base nécessaire"));
                msgbox.setInformativeText(tr("Pour éxécuter cette version de Rufus, la base de données doit être mise à jour vers la version") +
                                          " <b>" + QString::number(Version) + "</b><br />" +
                                          tr("et une sauvegarde de la base actuelle est fortement conseillée"));
                msgbox.setIcon(UpMessageBox::Warning);
                UpSmallButton *OKBouton = new UpSmallButton();
                UpSmallButton *AnnulBouton = new UpSmallButton();
                OKBouton->setText(tr("OK, je vais sauvegarder la base d'abord"));
                AnnulBouton->setText(tr("Pousuivre, la sauvegarde a été faite"));
                msgbox.addButton(OKBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(AnnulBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                if (msgbox.clickedButton() != AnnulBouton)
                    //if (!SauvegardeBase())
                        return false;
                BupDone = true;
            }
            Message(tr("Mise à jour de la base vers la version ") + "<font color=\"red\"><b>" + QString::number(Version) + "</b></font>", 1000, false);
            QString Nomfic = "://majbase" + QString::number(Version) + ".sql";
            QFile DumpFile(Nomfic);
            QString NomDumpFile = QDir::homePath() + "/Documents/Rufus/Ressources/majbase" + QString::number(Version) + ".sql";
            DumpFile.copy(NomDumpFile);
            QFile base(NomDumpFile);
            QStringList listinstruct = DecomposeScriptSQL(NomDumpFile);
            QSqlQuery query(getDataBase());
            bool a = true;
            foreach(const QString &s, listinstruct)
            {
                //Edit(s);
                query.exec(s);
                if (TraiteErreurRequete(query, s, ""))
                    a = false;
            }
            int result=0;
            base.remove();
            if (a)
            {
                result = 1;
                UpMessageBox::Watch(0,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version));
            }
            else
            {
                QSound::play(NOM_ALARME);
                UpMessageBox::Watch(0,tr("Echec de la mise à jour vers la version ") + QString::number(Version) + "\n" + tr("Le programme de mise à jour n'a pas pu effectuer la tâche!"));
            }
            if (result!=1)
                return false;
            if (Version == 30)
            {
                QSqlQuery("LOCK TABLES " NOM_TABLE_UTILISATEURS " WRITE, " NOM_TABLE_LIEUXEXERCICE " WRITE, " NOM_TABLE_JOINTURESLIEUX " WRITE", db);
                QSqlQuery listusrquer("select idUser, UserAdresse1, UserAdresse2, UserAdresse3, UserCodePostal, UserVille, UserTelephone, UserFax from " NOM_TABLE_UTILISATEURS, db);
                for (int i=0; i<listusrquer.size(); i++)
                {
                    listusrquer.seek(i);
                    req = "select idlieu from " NOM_TABLE_LIEUXEXERCICE
                                     " where nomlieu = '" + listusrquer.value(1).toString() + " " + listusrquer.value(5).toString() + "'";
                    //qDebug() << req;
                    QSqlQuery dlquer(req, db);
                    if (dlquer.size()>0)
                    {
                        dlquer.first();
                        req = "insert into " NOM_TABLE_JOINTURESLIEUX " (idUser, idLieu) VALUES(" + listusrquer.value(0).toString() + ", " + dlquer.value(0).toString() + ")";
                        QSqlQuery(req, db);
                    }
                    else
                    {
                        QString req = "insert into " NOM_TABLE_LIEUXEXERCICE " (NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone, LieuFax)"
                                    " VALUES ("
                                    "'" + listusrquer.value(1).toString() + " " + listusrquer.value(5).toString() + "', "
                                    "'" + listusrquer.value(1).toString() + "', "
                                    "'" + listusrquer.value(2).toString() + "', "
                                    "'" + listusrquer.value(3).toString() + "', "
                                    "'" + listusrquer.value(4).toString() + "', "
                                    "'" + listusrquer.value(5).toString() + "', "
                                    "'" + listusrquer.value(6).toString() + "', "
                                    "'" + listusrquer.value(7).toString() + "')";
                        QSqlQuery(req,db);
                        //                    QSqlQuery maxquer("select max(idLieu) from " NOM_TABLE_LIEUXEXERCICE, db);
                        //                    maxquer.first();
                        req = "insert into " NOM_TABLE_JOINTURESLIEUX " (idUser, idLieu) VALUES(" + listusrquer.value(0).toString() + ", (select max(idLieu) from " NOM_TABLE_LIEUXEXERCICE "))";
                        QSqlQuery(req, db);
                    }
                }
                QSqlQuery("UNLOCK TABLES", db);
            }
            if (Version == 31)
            {
                QSqlQuery ("update " NOM_TABLE_UTILISATEURS ",set OPTAM = 1 where UserSecteur = 2", db);
                QSqlQuery ("update " NOM_TABLE_UTILISATEURS ",set UserSecteur = 2 where UserSecteur = 3", db);
                QSqlQuery ("update " NOM_TABLE_UTILISATEURS ",set UserSecteur = 3 where UserSecteur = 4", db);
            }
            if (Version == 34)
            {
                if (gsettingsIni->value("BDD_LOCAL/Serveur").toString() != "")
                    QSqlQuery("update " NOM_TABLE_PARAMSYSTEME " set AdresseServeurLocal = '" + gsettingsIni->value("BDD_LOCAL/Serveur").toString() + "'", db);
                if (gsettingsIni->value("BDD_DISTANT/Serveur").toString() != "")
                    QSqlQuery("update " NOM_TABLE_PARAMSYSTEME " set AdresseServeurDistant = '" + gsettingsIni->value("BDD_DISTANT/Serveur").toString() + "'", db);
            }
            if (Version == 35)
            {
                QString AdressIP, MasqueReseauLocal;
                foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
                    if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
                         AdressIP = address.toString();
                }
                QStringList listIP = AdressIP.split(".");
                for (int i=0;i<listIP.size()-1;i++)
                    MasqueReseauLocal += QString::number(listIP.at(i).toInt()) + ".";
                MasqueReseauLocal += "%";
                QSqlQuery ("create user if not exists '" NOM_ADMINISTRATEURDOCS "'@'localhost' identified by '" NOM_MDPADMINISTRATEUR "'",db);
                QSqlQuery ("create user if not exists '" NOM_ADMINISTRATEURDOCS "'@'" + MasqueReseauLocal + "' identified by '" NOM_MDPADMINISTRATEUR "'",db);
                QSqlQuery ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "'@'localhost' identified by '" NOM_MDPADMINISTRATEUR "' with grant option",db);
                QSqlQuery ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "'@'" + MasqueReseauLocal + "' identified by '" NOM_MDPADMINISTRATEUR "' with grant option",db);
            }
            if (Version == 40)
            {
                QSqlQuery ("create user if not exists '" NOM_ADMINISTRATEURDOCS "SSL'@'%' identified by '" + getMDPAdmin() + "' REQUIRE SSL",db);
                QSqlQuery ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "SSL'@'%' identified by '" + getMDPAdmin() + "' with grant option",db);
            }
        }
    }
    //verification des fichiers ressources
    if (gsettingsIni->value("Param_Poste/VersionRessources").toInt() < VERSION_RESSOURCES)
    {
        PremierParametrageRessources();
        gsettingsIni->setValue("Param_Imprimante/TailleEnTeteALD","63");
        gsettingsIni->setValue("Param_Poste/VersionRessources", VERSION_RESSOURCES);
        Message(tr("Mise à jour des fichiers ressources vers la version ") + "<font color=\"red\"><b>" + QString::number(VERSION_RESSOURCES) + "</b></font>", 1000, false);
    }
    return true;
}


/*--------------------------------------------------------------------------------------------------------------
-- Verification de la version du programme --------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifVersion()
{
    QString dwnldlink = "https://www.dropbox.com/sh/ihxjhevsqldtmtf/AACMKSDCGRJYghkRo-dFyCAda?dl=0";
    QString VersionPrg =  qApp->applicationVersion();
    int SubVersion = 0;
    //QDate DatePrg = QDate::fromString(VersionPrg.split("/").at(0),tr("dd-MM-yyyy"));
    QString Sub = VersionPrg.split("/").at(1);
    if (Sub != "")
        SubVersion = Sub.toInt();
    //qDebug() << "Date publication = " + DatePrg.toString(tr("dd-MM-yyyy")) + " - Version " + QString::number(SubVersion);
    return true;
}

/*--------------------------------------------------------------------------------------------------------------
    -- Choix du mode de connexion ----------------------------------------------------------------------------------
    --------------------------------------------------------------------------------------------------------------*/
bool Procedures::FicheChoixConnexion()
{
    bool lPoste, lDistant, lReseauLocal;
    lPoste                          = (gsettingsIni->value("BDD_POSTE/Active").toString() == "YES"
                                       && (gsettingsIni->value("BDD_POSTE/Port").toInt() == 3306
                                       || gsettingsIni->value("BDD_POSTE/Port").toInt() == 3307)
                                       );
    lReseauLocal                    = (gsettingsIni->value("BDD_LOCAL/Active").toString() == "YES"
                                       && gsettingsIni->value("BDD_LOCAL/Serveur").toString() != ""
                                       && (gsettingsIni->value("BDD_LOCAL/Port").toInt() == 3306
                                       || gsettingsIni->value("BDD_LOCAL/Port").toInt() == 3307)
                                       );
    lDistant                        = (gsettingsIni->value("BDD_DISTANT/Active").toString() == "YES"
                                       && gsettingsIni->value("BDD_DISTANT/Serveur").toString() != ""
                                       && (gsettingsIni->value("BDD_DISTANT/Port").toInt() == 3306
                                       || gsettingsIni->value("BDD_DISTANT/Port").toInt() == 3307)
                                       );
    int a = 0;
    if (lPoste)         a += 1;
    if (lReseauLocal)   a += 1;
    if (lDistant)       a += 1;

    switch (a) {
    case 0: {
        UpMessageBox::Watch(0, tr("Aucune base identifiée dans Rufus.ini !"), tr("Veuillez corriger ce paramètre."));
        exit(0);
        break;
    }
    case 1: {
        if (lPoste)         gMode = Poste;
        if (lReseauLocal)   gMode = ReseauLocal;
        if (lDistant)       gMode = Distant;
        initOK  = true;
        break;
    }
    default: {
        UpMessageBox msgbox;
        msgbox.setText(tr("Quelle base de données souhaitez-vous utiliser?"));
        msgbox.setIcon(UpMessageBox::Quest);
        UpPushButton *OKBouton = new UpPushButton();
        UpPushButton *NoBouton = new UpPushButton();
        UpPushButton *AnnulBouton = new UpPushButton();
        UpPushButton *RejectButton = new UpPushButton();
        RejectButton->setText(tr("Annuler"));
        RejectButton->setIcon(giconAnnul);
        msgbox.addButton(RejectButton);
        if (lReseauLocal)
        {
            NoBouton->setText(tr("Locale, sur ce réseau"));
            msgbox.addButton(NoBouton);
            NoBouton->setIcon(giconNetwork);
        }
        if (lDistant)
        {
            AnnulBouton->setText(tr("Distante, par internet"));
            msgbox.addButton(AnnulBouton);
            AnnulBouton->setIcon(giconInternet);
        }
        if (lPoste)
        {
            OKBouton->setText(tr("Sur cette machine"));
            msgbox.addButton(OKBouton);
            OKBouton->setIcon(giconComputer);
        }
        initOK = false;
        if (msgbox.exec()>0)
        {
            initOK = (msgbox.clickedpushbutton() != RejectButton);
            if (initOK)
            {
                if (msgbox.clickedpushbutton()      == OKBouton)    gMode = Poste;
                else if (msgbox.clickedpushbutton() == NoBouton)    gMode = ReseauLocal;
                else if (msgbox.clickedpushbutton() == AnnulBouton) gMode = Distant;
            }
        }
        delete OKBouton;
        delete AnnulBouton;
        delete RejectButton;
        delete NoBouton;
    }
    }
    return initOK;
}

/*--------------------------------------------------------------------------------------------------------------
-- Connexion à Consults -------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
bool Procedures::Connexion_A_La_Base()
{
    QString Base;
    switch (gMode) {
    case Poste:
        Base = "BDD_POSTE";
        break;
    case ReseauLocal:
        Base = "BDD_LOCAL";
        break;
    case Distant:
        Base = "BDD_DISTANT";
        break;
    default:
        Base = "BDD_POSTE";
        break;
    }

    QString Serveur;
    if (gMode == Poste)
        Serveur = "localhost";
    else
        Serveur = gsettingsIni->value(Base + "/Serveur").toString();
    int Port    = gsettingsIni->value(Base + "/Port").toInt();
    bool SSL    = (gMode == Distant);

    if (!IdentificationUser(Serveur, Port, SSL, Base, false))
        return false;
    gidLieuExercice = DetermineLieuExercice();
    setlisteUsers();
    ChargeDataUser(gidUser);
    if (lDataUser["idLieu"].toInt() == QVariant(QVariant::Int))
            UpMessageBox::Watch(0,tr("Pas d'adresse spécifiée"),
                                  tr("Vous n'avez précisé aucun lieu d'exercice!"));
    gdbOK = true;
    QSqlQuery quer("set global sql_mode = 'NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES';", db);
    TraiteErreurRequete(quer,"set global sql_mode = 'NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES';","");
    QSqlQuery quer2("SET GLOBAL event_scheduler = 1 ;", db);
    TraiteErreurRequete(quer2,"SET GLOBAL event_scheduler = 1 ;","");
    QSqlQuery quer3("SET GLOBAL max_allowed_packet=" MAX_ALLOWED_PACKET "*1024*1024 ;", db);
    TraiteErreurRequete(quer3,"SET GLOBAL max_allowed_packet=" MAX_ALLOWED_PACKET "*1024*1024 ;","");
    return gdbOK;
}

bool Procedures::ChargeDataUser(int iduser)
{
    lDataUser               = fdatauser(iduser, gidLieuExercice, db);
    if (!lDataUser["Success"].toBool())
        return false;
    lDataUser["UserSuperviseur"]        = UserSuperviseur(); // -1 = indéterminé (personnel non soignant) -2 = tout le monde
    lDataUser["idUserComptable"]        = UserComptable();
    lDataUser["idParent"]               = UserParent();
    lDataUser["LoginSuperviseur"]       = getLogin(UserSuperviseur());
    lDataUser["LoginComptable"]         = getLogin(UserComptable());
    lDataUser["LoginParent"]            = getLogin(UserParent());
    if (lDataUser["idUserComptable"].toInt() == -2)
    {
        lDataUser["idCompteParDefaut"]          = -1;
        lDataUser["idCompteEncaissHonoraires"]  = -1;
        lDataUser["NomCompteEncaissHonoraires"] = "pas de compte";
    }
    else if (iduser != UserParent())
    {
        QMap<QString,QVariant>  DataUser        = fdatauser(gidUserParent, gidLieuExercice, db);
        lDataUser["idCompteParDefaut"]          = DataUser["idCompteParDefaut"];
        lDataUser["idCompteEncaissHonoraires"]  = DataUser["idCompteEncaissHonoraires"];
        lDataUser["NomCompteEncaissHonoraires"] = DataUser["NomCompteEncaissHonoraires"];
        lDataUser["Secteur"]                    = DataUser["Secteur"];
        lDataUser["OPTAM"]                      = DataUser["OPTAM"];
    }

    QString Statut =  tr("utilisateur") + "\t= " + lDataUser["UserLogin"].toString()  + "\n";
    Statut +=  tr("Site") + "\t= " + lDataUser["NomLieu"].toString()  + "\n";
    QString d;

    switch (UserSuperviseur()) {
    case -3: d = tr("indéterminé"); break;
    case -2: d = tr("sans objet"); break;
    case -1: d = tr("tout le monde"); break;
    default: d = getLogin(UserSuperviseur()); break;
    }
    //qDebug() << "superviseur    = " + d;
    Statut += tr("superviseur") + "\t= " + d + "\n";

    switch (gidUserParent) {
    case -3: d = tr("indéterminé"); break;
    case -2: d = tr("sans objet"); break;
    default:
        d = getLogin(gidUserParent); break;
    }
    //qDebug() << "parent  = " + d ;
    Statut += tr("parent") + "\t= " + d+ "\n";

    switch (gidUserComptable) {
    case -3: d = tr("indéterminé"); break;
    case -2: d = tr("sans objet"); break;
    case -1: d = tr("sans comptabilité"); break;
    default: d = getLogin(gidUserComptable); break;
    }
    //qDebug() << "comptable      = " + d;
    Statut += tr("comptable") + "\t= " + d + "\n";

    //qDebug() << "compte banc    = " + gNomCompteEncaissHonoraires;
    Statut += tr("cpte banque") + "\t= " + lDataUser["NomCompteEncaissHonoraires"].toString() + "\n";

    //qDebug() << "compta         = " + compta;
    QString compta = "";
    int cpt = AvecCompta();
    switch (cpt) {
    case 0: compta = "avec cotation et comptabilité";       break;
    case 1: compta = "sans cotation ni comptabilité";       break;
    case 2: compta = "avec cotation sans comptabilité";     break;
    case 3: compta = "sans cotation avec comptabilité";     break;
    default: break;
    }
    Statut += tr("comptabilité") + "\t= " + compta;
    lDataUser["Statut"]          = Statut;
    RestoreFontAppliAndGeometry();
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Détermination du lieu exercice pour la session en cours -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
int Procedures::DetermineLieuExercice()
{
    int idLieuExercice = 1;
    if (getModeConnexion() == Procedures::Poste || getModeConnexion() == Procedures::Distant)
    {
        QString lieuxreq = "select joint.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, LieuCodePostal, LieuVille, LieuTelephone, LieuFax from "
                        NOM_TABLE_JOINTURESLIEUX " joint left outer join " NOM_TABLE_LIEUXEXERCICE
                        " lix on joint.idlieu = lix.idLieu"
                        " where iduser = " + QString::number(gidUser);
        //qDebug() << lieuxreq;
        QSqlQuery lxquer(lieuxreq,db);
        TraiteErreurRequete(lxquer, lieuxreq);
        if (lxquer.size()==1)
        {
            lxquer.first();
            idLieuExercice = lxquer.value(0).toInt();
        }
        else if (lxquer.size()>1)
        {
            gAskLieux               = new UpDialog();
            gAskLieux               ->AjouteLayButtons();
            QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(gAskLieux->layout());
            QGroupBox*boxlieux      = new QGroupBox();
            globallay               ->insertWidget(0,boxlieux);
            boxlieux                ->setAccessibleName("Parent");
            boxlieux                ->setTitle(tr("D'où vous connectez-vous?"));

            QFontMetrics fm         = QFontMetrics(qApp->font());
            int hauteurligne        = fm.height()*1.6;
            boxlieux                ->setFixedHeight(((lxquer.size() + 1)*hauteurligne)+5);
            QVBoxLayout *vbox       = new QVBoxLayout;
            for (int i=0; i<lxquer.size(); i++)
            {
                lxquer          .seek(i);
                UpRadioButton   *pradiobutt = new UpRadioButton(boxlieux);
                pradiobutt      ->setText(lxquer.value(1).toString());
                pradiobutt      ->setAccessibleName(lxquer.value(0).toString());
                QString data ("");
                if (lxquer.value(1).toString()!="")
                    data += lxquer.value(1).toString();
                if (data == "" )
                {
                    data += lxquer.value(2).toString();
                    if (lxquer.value(6).toString()!="")
                        data += (data != ""? " " : "") + lxquer.value(6).toString();
                }
                if (lxquer.value(6).toString()!="")
                    data += (data != ""? " - " : "") + lxquer.value(6).toString();
                data = "";
                if (lxquer.value(1).toString()!="")
                    data += lxquer.value(1).toString();
                if (lxquer.value(2).toString()!="")
                    data += (data != ""? "\n" : "") + lxquer.value(2).toString();
                if (lxquer.value(3).toString()!="")
                    data += (data != ""? "\n" : "") + lxquer.value(3).toString();
                if (lxquer.value(4).toString()!="")
                    data += (data != ""? "\n" : "") + lxquer.value(4).toString();
                if (lxquer.value(5).toString()!="")
                    data += (data != ""? "\n" : "") + lxquer.value(5).toString();
                if (lxquer.value(6).toString()!="")
                    data += (data != ""? " " : "") + lxquer.value(6).toString();
                if (lxquer.value(7).toString()!="")
                    data += (data != ""? "\nTel: " : "Tel: ") + lxquer.value(7).toString();
                pradiobutt      ->setImmediateToolTip(data);
                pradiobutt      ->setChecked(i==0);
                vbox            ->addWidget(pradiobutt);
            }
            vbox                ->setContentsMargins(8,0,8,0);
            boxlieux            ->setLayout(vbox);
            gAskLieux           ->setModal(true);
            globallay           ->setSizeConstraint(QLayout::SetFixedSize);
            connect(gAskLieux->OKButton,   SIGNAL(clicked(bool)),  gAskLieux, SLOT(accept()));
            gAskLieux->exec();
            QList<QRadioButton*> listbutt = boxlieux->findChildren<QRadioButton*>();
            for (int j=0; j<listbutt.size(); j++)
                if (listbutt.at(j)->isChecked())
                    idLieuExercice = listbutt.at(j)->accessibleName().toInt();
            delete gAskLieux;
        }
    }
    else
    {
        QString lieuxreq = "select idLieuParDefaut from " NOM_TABLE_PARAMSYSTEME;
        QSqlQuery lxquer(lieuxreq,db);
        TraiteErreurRequete(lxquer,lieuxreq);
        lxquer.first();
        if (lxquer.value(0).toInt()>=1)
            idLieuExercice = lxquer.value(0).toInt();
        else
        {
            QSqlQuery lieuquer("select min(idlieu) from " NOM_TABLE_LIEUXEXERCICE,db);
            lieuquer.first();
            idLieuExercice = lieuquer.value(0).toInt();
            QSqlQuery("update " NOM_TABLE_PARAMSYSTEME " set idLieuParDefaut = " + lieuquer.value(0).toString(), db);
        }
    }
    return idLieuExercice;
}

/*-----------------------------------------------------------------------------------------------------------------
        -- Création d'un utilisateur -------------------------------------------------------------
        -----------------------------------------------------------------------------------------------------------------*/
QStringList Procedures::ChoisirUnLogin()
{
    gLogin                      = "LOG";
    gNouvMDP                    = "NMDP";
    gConfirmMDP                 = "AMDP";
    QStringList list;
    gAskLogin                   = new UpDialog();
    QVBoxLayout *lay            = new QVBoxLayout();
    UpLabel *label              = new UpLabel();
    UpLabel *label2             = new UpLabel();
    UpLabel *label3             = new UpLabel();
    UpLineEdit *Line            = new UpLineEdit();
    UpLineEdit *Line2           = new UpLineEdit();
    UpLineEdit *Line3           = new UpLineEdit();
    QVBoxLayout *globallay      = dynamic_cast<QVBoxLayout*>(gAskLogin->layout());
    QRegExp rx                  = QRegExp("^[A-Za-z0-9]{5,15}$");

    gAskLogin                   ->setModal(true);
    gAskLogin                   ->setFixedSize(300,300);
    gAskLogin                   ->setWindowTitle("");

    Line                        ->setObjectName(gLogin);
    Line2                       ->setObjectName(gNouvMDP);
    Line3                       ->setObjectName(gConfirmMDP);
    Line                        ->setValidator(new QRegExpValidator(rx));
    Line2                       ->setValidator(new QRegExpValidator(rx));
    Line3                       ->setValidator(new QRegExpValidator(rx));
    Line                        ->setAlignment(Qt::AlignCenter);
    Line2                       ->setAlignment(Qt::AlignCenter);
    Line3                       ->setAlignment(Qt::AlignCenter);
    Line                        ->setMaxLength(15);
    Line2                       ->setMaxLength(8);
    Line3                       ->setMaxLength(8);
    Line                        ->setFixedHeight(20);
    Line2                       ->setFixedHeight(20);
    Line3                       ->setFixedHeight(20);
    Line2                       ->setEchoMode(QLineEdit::Password);
    Line3                       ->setEchoMode(QLineEdit::Password);
    label                       ->setMinimumHeight(46);
    label2                      ->setMinimumHeight(46);
    label3                      ->setFixedHeight(16);
    label                       ->setAlignment(Qt::AlignCenter);
    label2                      ->setAlignment(Qt::AlignCenter);
    label3                      ->setAlignment(Qt::AlignCenter);

    lay                         ->setSpacing(2);

    label                       ->setText(tr("Choisissez un login pour le nouvel utilisateur\n- maxi 15 caractères -\n- pas de caractères spéciaux ou accentués -"));
    label2                      ->setText(tr("Choisissez un mot de passe\n- maxi 8 caractères -\n- pas de caractères spéciaux ou accentués -"));
    label3                      ->setText(tr("Confirmez le mot de passe"));

    gAskLogin                        ->AjouteLayButtons(UpDialog::ButtonOK);
    connect(gAskLogin->OKButton,     SIGNAL(clicked(bool)),  this,   SLOT(Slot_VerifLogin()));

    lay                         ->addWidget(label);
    lay                         ->addWidget(Line);
    lay                         ->addWidget(label2);
    lay                         ->addWidget(Line2);
    lay                         ->addWidget(label3);
    lay                         ->addWidget(Line3);
    lay                         ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    lay->setContentsMargins(5,5,5,5);
    lay->setSpacing(5);

    globallay                   ->insertLayout(0,lay);
    globallay                   ->setSizeConstraint(QLayout::SetFixedSize);

    Line                        ->setFocus();

    if (gAskLogin->exec() == 0)
        list << "" << "";
    else
        list << Line->text() << Line2->text();
    return list;
    delete gAskLogin;
}

void Procedures::Slot_VerifLogin()
{
    if (!gAskLogin) return;
    QString msg = "";
    UpLineEdit *Loginline       = gAskLogin->findChild<UpLineEdit*>(gLogin);
    UpLineEdit *MDPline         = gAskLogin->findChild<UpLineEdit*>(gNouvMDP);
    UpLineEdit *ConfirmMDPline  = gAskLogin->findChild<UpLineEdit*>(gConfirmMDP);
    QString login               = Loginline->text();
    QString MDP                 = MDPline->text();
    QRegExp rx                  = QRegExp("^[A-Za-z0-9]{5,15}$");

    if (Loginline->text() == "")
    {
        msg = tr("Vous avez oublié d'indiquer le login");
        Loginline->setFocus();
    }
    if (MDPline->text() == "")
    {
        msg = tr("Vous avez oublié d'indiquer le mot de passe");
        MDPline->setFocus();
    }
    if (!rx.exactMatch(MDPline->text()))
    {
        msg = tr("Le mot de passe n'est pas conforme.") + "\n" +
              tr("Au moins 5 caractères - uniquement des chifres ou des lettres - max. 5 caractères.");
        MDPline->setFocus();
    }
    if (MDPline->text() != ConfirmMDPline->text())
    {
        msg = tr("Les mots de passe ne correspondent pas");
        MDPline->setFocus();
    }
    if (msg != "")
    {
        QSound::play(NOM_ALARME);
        UpMessageBox::Watch(0,msg);
        return;
    }
    gAskLogin->accept();
}


/*-----------------------------------------------------------------------------------------------------------------
    -- Création d'un utilisateur -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::CreerPremierUser(QString Login, QString MDP)
{
    // Bon, on dispose du nouveau login et du nouveau MDP
    if (Login == "")
    {
        UpMessageBox::Watch(0,tr("Impossible de créer l'utilisateur"),tr("Login manquant"));
        return false;
    }
    if (MDP == "")
    {
        UpMessageBox::Watch(0,tr("Impossible de créer l'utilisateur"),tr("Mot de passe manquant"));
        return false;
    }
    gLoginUser  = Login;
    gMDPUser    = MDP;

    //1. On vérifie si ce login existe dans le serveur et si c'est le cas, on détruit toutes les instances de ce login
    QString req = "select user, host from mysql.user where user = '" + gLoginUser + "%'";
    QSqlQuery usrquery(req,db);
    if (usrquery.size()>0)
    {
        usrquery.first();
        for (int i=0; i<usrquery.size(); i++)
        {
            req = "drop user '" + usrquery.value(0).toString() + "'@'" + usrquery.value(1).toString() + "'";
            QSqlQuery (req,db);
            usrquery.next();
        }
    }
     //2. On crée 3 comptes SQL avec ce login et ce MDP: local en localshost, réseau local (p.e. 192.168.1.%) et distant en %-SSL et login avec SSL à la fin
    QString AdressIP, MasqueReseauLocal;
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
             AdressIP = address.toString();
    }
    QStringList listIP = AdressIP.split(".");
    for (int i=0;i<listIP.size()-1;i++)
        MasqueReseauLocal += QString::number(listIP.at(i).toInt()) + ".";
    MasqueReseauLocal += "%";
    QSqlQuery ("create user '" + gLoginUser + "'@'localhost' identified by '" + gMDPUser + "'",db);
    QSqlQuery ("create user '" + gLoginUser + "'@'" + MasqueReseauLocal + "' identified by '" + gMDPUser + "'",db);
    QSqlQuery ("create user '" + gLoginUser + "SSL'@'%' identified by '" + gMDPUser + "' REQUIRE SSL",db);
    QSqlQuery ("grant all on *.* to '" + gLoginUser + "'@'localhost' identified by '" + gMDPUser + "' with grant option",db);
    QSqlQuery ("grant all on *.* to '" + gLoginUser + "SSL'@'%' identified by '" + gMDPUser + "' with grant option",db);
    QSqlQuery ("grant all on *.* to '" + gLoginUser + "'@'" + MasqueReseauLocal + "' identified by '" + gMDPUser + "' with grant option",db);

    // Création de l'administrateur des documents ------------------------------------------------------------------
    QSqlQuery ("create user if not exists '" NOM_ADMINISTRATEURDOCS "'@'localhost' identified by '" NOM_MDPADMINISTRATEUR "'",db);
    QSqlQuery ("create user if not exists '" NOM_ADMINISTRATEURDOCS "'@'" + MasqueReseauLocal + "' identified by '" NOM_MDPADMINISTRATEUR "'",db);
    QSqlQuery ("create user if not exists '" NOM_ADMINISTRATEURDOCS "SSL'@'%' identified by '" NOM_MDPADMINISTRATEUR "' REQUIRE SSL",db);
    QSqlQuery ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "'@'localhost' identified by '" NOM_MDPADMINISTRATEUR "' with grant option",db);
    QSqlQuery ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "'@'" + MasqueReseauLocal + "' identified by '" NOM_MDPADMINISTRATEUR "' with grant option",db);
    QSqlQuery ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "SSL'@'%' identified by '" NOM_MDPADMINISTRATEUR "' with grant option",db);

    // On crée l'utilisateur dans la table utilisateurs
    QSqlQuery ("insert into " NOM_TABLE_UTILISATEURS " (idUser, UserLogin, UserMDP) VALUES (1,'" + gLoginUser + "', '" + gMDPUser + "')",db);
    gidUser                 = 1;
    gidUserSuperViseur      = 1;
    gidUserParent           = 1;
    gidUserComptable        = 1;
    gidCentre               = 1;
    avecLaCompta            = 0;
    gUseCotation            = true;

    if (UpMessageBox::Question(0, tr("Un compte utilisateur a été cré"),
                               tr("Un compte utilisateur factice a été créé\n") + "\n" +
                               CreerUserFactice(gidUser) + "\n\n" +
                               tr("avec le login ") + gLoginUser + " " + tr("et le mot de passe que vous avez fourni") + "\n" +
                               tr("Voulez-vous conserver ces données pour le moment ou les modifier?") + "\n" +
                               tr("Vous pourrez les modifier par la suite\n"),
                               UpDialog::ButtonOK | UpDialog::ButtonEdit, QStringList() << tr("Modifier les données") << tr("Conserver les données"))
        == UpSmallButton::EDITBUTTON)
    {
        Dlg_GestUsr = new dlg_gestionusers(1, gidLieuExercice, db, MapIcons());
        Dlg_GestUsr->setWindowTitle(tr("Enregistrement de l'utilisateur ") + gLoginUser);
        Dlg_GestUsr->setConfig(dlg_gestionusers::PREMIERUSER);
        Dlg_GestUsr->exec();
    }
    gsettingsIni->setValue("Param_Poste/VilleParDefaut","Flayat");
    gsettingsIni->setValue("Param_Poste/CodePostalParDefaut","23260");
    gdbOK = true;
    // On paramètre l'imprimante et les fichiers ressources
    PremierParametrageMateriel();
    PremierParametrageRessources();
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Création d'un utilisateur factice ----------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
QString Procedures::CreerUserFactice(int iduser)
{
    QString id = QString::number(iduser);
    QSqlQuery ("LOCK TABLES " NOM_TABLE_COMPTES " WRITE, " NOM_TABLE_UTILISATEURS " WRITE, " NOM_TABLE_BANQUES " WRITE, " NOM_TABLE_LIEUXEXERCICE " WRITE, " NOM_TABLE_JOINTURESLIEUX " WRITE", db);
    int idbanq = 0;
    QString req = "select idbanque, idbanqueabrege, nombanque from " NOM_TABLE_BANQUES " where idbanqueabrege = 'PaPRS'";
    QSqlQuery quer(req, db);
    if (quer.size()>0)
    {
        quer.first();
        idbanq = quer.value(0).toInt();
    }
    else
    {
        QSqlQuery ("insert into " NOM_TABLE_BANQUES " (idbanqueAbrege, Nombanque) values ('PaPRS','Panama Papers')", db);
        QSqlQuery quer2("select idbanque from " NOM_TABLE_BANQUES " where idbanqueabrege = 'PaPRS'", db);
        quer2.first();
        idbanq = quer2.value(0).toInt();
    }

    int al = 0;
    QString iban = "FR";
    srand(time(NULL));
    al = rand() % 100;
    while (al<10)
        al = rand() % 100;
    iban += QString::number(al) + " ";
    for(int i=0; i<5; i++)
    {
        al = rand() % 10000;
        while (al<1000)
            al = rand() % 10000;
        iban += QString::number(al) + " ";
    }
    al = rand() % 1000;
    while (al<100)
        al = rand() % 1000;
    iban += QString::number(al);

    req  = "insert into " NOM_TABLE_COMPTES
           " (idBanque, idUser, IBAN, IntituleCompte, NomCompteAbrege, SoldeSurDernierReleve)"
           " VALUES (" + QString::number(idbanq) + "," + id + ", '" + iban + "', '" + gLoginUser + "', 'PaPRS" + QString::number(al) + "', 2333.67)";
    QSqlQuery(req,db);
    req = "select max(idcompte) from " NOM_TABLE_COMPTES;
    QSqlQuery idcptquer(req,db);
    idcptquer.first();
    QString idcpt = idcptquer.value(0).toString();


    req = "update " NOM_TABLE_UTILISATEURS
            " set userNom = 'Snow',\n"
            " userPrenom = '" + MajusculePremiereLettre(gLoginUser) +"',\n"
            " UserPoliceEcran = '" POLICEPARDEFAUT "',\n"
            " UserPoliceAttribut = 'Regular',\n"
            " UserTitre = '" + tr("Docteur") + "',\n"
            " UserFonction = '" + tr("Médecin") + "',\n"
            " UserSpecialite = '" + tr("Ophtalmologiste") + "',\n"
            " UserNoSpecialite = 15,\n"
            " UserNumCO = '2 33 2 123456 1 2 3 4',\n "
            " UserNumPS = '123456789',\n "
            " Userdroits = '" OPHTALIBERAL "', \n"
            " UserEnregHonoraires = 1,\n"
            " IdCompteParDefaut = " + idcpt + ",\n"
            " idCompteEncaissHonoraires = " + idcpt + ",\n"
            " Soignant = 1,\n"
            " Medecin = 1,\n"
            " ResponsableActes = 1,\n"
            " UserCCAM = 1,\n"
            " USerAGA = 1,\n"
            " USerSecteur = 1,\n"
            " OPTAM = 1\n"
            " where idUser = " + id;
    //Edit(req);
    QSqlQuery(req,db);
    req = "insert into " NOM_TABLE_LIEUXEXERCICE "(NomLieu, LieuAdresse1, LieuAdresse2, LieuCodePostal, LieuVille, LieuTelephone)  values ("
            "'Centre ophtalmologique de La Mazière', "
            "'place rouge', "
            "'Bâtiment C', "
            "23260, "
            "'La Mazière', "
            "'O4 56 78 90 12')";
    //Edit(req);
    QSqlQuery(req,db);
    req = "select idLieu from " NOM_TABLE_LIEUXEXERCICE;
    QSqlQuery querr(req, db);
    querr.first();
    gidLieuExercice = querr.value(0).toInt();
    req = "insert into " NOM_TABLE_JOINTURESLIEUX " (idUser, idLieu) VALUES(" + id + ", " + QString::number(gidLieuExercice) + ")";
    QSqlQuery(req, db);
    req = "update " NOM_TABLE_PARAMSYSTEME " set idLieuParDefaut = " + QString::number(gidLieuExercice);
    QSqlQuery(req, db);
    QSqlQuery("UNLOCK TABLES", db);
    return (tr ("Docteur") + " "  + MajusculePremiereLettre(gLoginUser) + " Snow, " + tr("Ophtalmologiste libéral"));
}


/*-----------------------------------------------------------------------------------------------------------------
    -- Identification de l'utilisateur -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::IdentificationUser(QString Serveur, int Port, bool SSL, QString Base, bool ChgUsr)
{
    Dlg_IdentUser   = new dlg_identificationuser(NOM_TABLE_UTILISATEURS, Serveur, Port, SSL, Base, ChgUsr);
    Dlg_IdentUser   ->setFont(QFont(POLICEPARDEFAUT,POINTPARDEFAUT));
    bool a = false;

    int result      = Dlg_IdentUser->exec();
    db = Dlg_IdentUser->getdatabase();
    gidUser                 = Dlg_IdentUser->getidUser();
    gLoginUser              = Dlg_IdentUser->ui->LoginlineEdit->text();
    gMDPUser                = Dlg_IdentUser->ui->MDPlineEdit->text();
    //db = QSqlDatabase::addDatabase("QMYSQL","Rufus");
    if (result > 0)
    {
        if (!VerifBaseEtRessources())
        {
            UpMessageBox::Watch(0, tr("Impossible de mettre à jour la base de données\nSortie du programme"));
            exit(0);
        }
        Verif_secure_file_priv();
        if (DefinitRoleUser(Dlg_IdentUser->getidUser()))
        {
            /* definit les iduser pour lequel le user travaille
                . iduser superviseur des actes                      (int gidUserSuperViseurProv)
                    . lui-même s'il est responsable de ses actes
                    . un autre user s'il est assistant
                    . -1 s'il est assistant pour plusieurs utilisateurs en même temps
                    . -2 sans objet
                . idUser parent
                    correspond au membre permanent de la structure de soins pour qui le user travaille
                . iduser qui enregistrera la comptabilité des actes (int gidUserComptableProv)
                    . lui même s'il est responsable et libéral
                    . son employeur s'il est responsable et salarié
                    . s'il est remplaçant (retrocession) on lui demande qui il remplace et le comptable devient
                        . celui qu'il remplace si celui qu'il remplace est libéral
                        . l'employeur de  celui qu'il remplace si  celui qu'il remplace est salarié
                    . -1 s'il n'enregistre pas de compta
                    . -2 sans objet
                . et s'il cote les actes                            (bool gUseCotationProv)
                    0 = AvecCompta
                    1 = SansCompta et sans cotation
                    2 = Sans compta mais avec cotation
                    3 = Avec compta mais sans cotation
           */
            gidUserSuperViseur      = gidUserSuperViseurProv;
            gidUserComptable        = gidUserComptableProv;
            gidUserParent           = gidUserParentProv;
            gUseCotation            = gUseCotationProv;
            avecLaCompta            = (avecLaComptaProv? (gUseCotationProv? 0 : 4) : (gUseCotationProv? 2 : 1));
            //avecLaCompta            = 0;
            QSqlQuery quer("select Numcentre from " NOM_TABLE_PARAMSYSTEME,db);
            quer.first();
            gidCentre = quer.value(0).toInt();
            a = true;
        }
    }
    else if (result == -1) // erreur de couple login-MDP
    {
         // traité en amont
    }
    else if (result == -2) // erreur des droits utilisateur sur le serveur
    {
        // traité en amont
    }
    else if (result == -3) // anomalie sur la base - table utilisateurs manquante ou corrompue
    {
        UpMessageBox     msgbox;
        UpSmallButton    *AnnulBouton   = new UpSmallButton;
        UpSmallButton    *OKBouton      = new UpSmallButton;
        UpSmallButton    *YesBouton     = new UpSmallButton;
        msgbox.setText(tr("Base de données endommagée!"));
        msgbox.setInformativeText(tr("La base de données semble endommagée.\n"
                                  "Voulez-vous la reconstruire à partir"
                                  " d'une sauvegarde ou recréer une base vierge?\n\n"));
        msgbox.setIcon(UpMessageBox::Info);
        AnnulBouton->setText(tr("Annuler"));
        OKBouton->setText(tr("Restaurer la base depuis une sauvegarde"));
        YesBouton->setText(tr("Nouvelle base patients vierge"));
        msgbox.addButton(AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(YesBouton, UpSmallButton::STARTBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::COPYBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() == OKBouton)
            if (RestaureBase(false,false,false))
            {
                UpMessageBox::Watch(0,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
                exit(0);
            }
        if (msgbox.clickedButton() == YesBouton)
        {
            QString NomDirRessrces = QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES;
            QDir DirRessrces(NomDirRessrces);
            if (!DirRessrces.exists())
                DirRessrces.mkdir(NomDirRessrces);
            if (!RestaureBase(true, true))
                exit(0);
            // Création de l'utilisateur
            gdbOK = CreerPremierUser(gLoginUser, gMDPUser);
            setlisteUsers();
            ChargeDataUser(gidUser);
            PremierParametrageMateriel();
            PremierParametrageRessources();
            UpMessageBox::Watch(0,tr("Le programme va se fermer"), tr("Relancez-le pour que certaines données puissent être prises en compte"));
            exit(0);
        }
    }
    else if (result == -4) // table utilisateurs vide
    {
        UpMessageBox::Watch(0, tr("Erreur sur la base patients!"),
                               tr("La connexion au serveur fonctionne mais\n"
                                  "Il n'y a aucun utilisateur enregistré\n"
                                  "avec ce login dans la table des utilisateurs.\n"
                                  "Connectez-vous avec un login enregistré dans la base Rufus"));
        exit(0);
    }
    else if (result == -5) // table utilisateurs non vide mais utilisateur non référencé
    {
       //traité en amont
    }
    else
        a = false;
    delete Dlg_IdentUser;
    /*QString compta = "";
    if (avecLaCompta==0)    compta = "avec cotation et comptabilité";
    else if (avecLaCompta==1)    compta = "sans cotation et sans comptabilité";
    else if (avecLaCompta==2)    compta = "avec cotation et sans comptabilité";
    else if (avecLaCompta==3)    compta = "sans cotation ni comptabilité";

    qDebug() << "user = " + getLogin(gidUser) + " - superviseur = " + (gidUserSuperViseur <0? (gidUserSuperViseur <-1?"tout le monde":"indétreminé") : getLogin(gidUserSuperViseur))
                + " - pour le compte de  = " + (gidUserComptable <0? (gidUserComptable <-1? "tout le monde" : "indéterminé"):getLogin(gidUserComptable))
                + " - compta = " + compta;*/
    return a;
}

void Procedures::DefinitScriptBackup(QString path, bool AvecImages, bool AvecVideos)
{
    QSqlQuery dirquer("select DirBkup from " NOM_TABLE_PARAMSYSTEME, db);
    dirquer.first();
    QString NomDirDestination = dirquer.value(0).toString();
    // élaboration du script de backup
    QString scriptbackup = "#!/bin/bash";
    //# Configuration de base: datestamp e.g. YYYYMMDD
    scriptbackup += "\n";
    scriptbackup += "DATE=$(date +\"%Y%m%d-%H%M\")";
    //# Dossier où sauvegarder les backups (créez le d'abord!)
    scriptbackup += "\n";
    scriptbackup += "BACKUP_DIR=\"" + NomDirDestination + "\"";
    //# Dossier de  ressources
    scriptbackup += "\n";
    scriptbackup += "DIR_RESSOURCES=\"" + QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES + "\"";
    scriptbackup += "\n";
    scriptbackup += "DIR_IMAGES=\"" + path + NOMDIR_IMAGES + "\"";
    scriptbackup += "\n";
    scriptbackup += "DIR_VIDEOS=\"" + path + NOMDIR_VIDEOS + "\"";
    //# Rufus.ini
    scriptbackup += "\n";
    scriptbackup += "RUFUSINI=\"" + QDir::homePath() + NOMFIC_INI + "\"";
    //# Identifiants MySQL
    scriptbackup += "\n";
    scriptbackup += "MYSQL_USER=\"dumprufus\"";
    scriptbackup += "\n";
    scriptbackup += "MYSQL_PASSWORD=\"" + getMDPAdmin() + "\"";
    //# Commandes MySQL
    QDir Dir(QCoreApplication::applicationDirPath());
    Dir.cdUp();
    scriptbackup += "\n";
#ifdef Q_OS_MACX
    scriptbackup += "MYSQL=/usr/local/mysql/bin";           // Depuis HighSierra on ne peut plus utiliser + Dir.absolutePath() + NOMDIR_LIBS2 - le script ne veut pas utiliser le client mysql du package (???)
    scriptbackup += "/mysql";
    scriptbackup += "\n";
    scriptbackup += "MYSQLDUMP=/usr/local/mysql/bin";       // Depuis HighSierra on ne peut plus utiliser + Dir.absolutePath() + NOMDIR_LIBS2 - le script ne veut pas utiliser le client mysql du package (???)
    scriptbackup += "/mysqldump";
    scriptbackup += "\n";
#endif
    //# Bases de données MySQL à ignorer
    scriptbackup += "SKIPDATABASES=\"Database|information_schema|performance_schema|mysql|sys\"";
    //# Nombre de jours à garder les dossiers (seront effacés après X jours)
    scriptbackup += "\n";
    scriptbackup += "RETENTION=14";
    //# Create a new directory into backup directory location for this date
    scriptbackup += "\n";
    scriptbackup += "mkdir -p $BACKUP_DIR/$DATE";
    //# Retrieve a list of all databases
    scriptbackup += "\n";
    scriptbackup += "databases=`$MYSQL -u$MYSQL_USER -p$MYSQL_PASSWORD -e \"SHOW DATABASES;\" | grep -Ev \"($SKIPDATABASES)\"`";
    scriptbackup += "\n";
    scriptbackup += "for db in $databases; do";
    scriptbackup += "\n";
    scriptbackup += "echo $db";
    scriptbackup += "\n";
    scriptbackup += "$MYSQLDUMP --force --opt --user=$MYSQL_USER -p$MYSQL_PASSWORD --skip-lock-tables --events --databases $db > \"$BACKUP_DIR/$DATE/$db.sql\"";
    scriptbackup += "\n";
    scriptbackup += "done";
    // Sauvegarde la table des utilisateurs
    scriptbackup += "\n";
    scriptbackup += "$MYSQLDUMP --force --opt --user=$MYSQL_USER -p$MYSQL_PASSWORD mysql user > \"$BACKUP_DIR/$DATE/user.sql\"";
    // Detruit les anciens fichiers
    scriptbackup += "\n";
    scriptbackup += "find $BACKUP_DIR/* -mtime +$RETENTION -delete";
    // copie les fichiers ressources
    scriptbackup += "\n";
    scriptbackup +=  "cp -R $DIR_RESSOURCES $BACKUP_DIR/$DATE/Ressources";
    // copie les fichiers image
    if (AvecImages)
    {
        scriptbackup += "\n";
        scriptbackup +=  "cp -R $DIR_IMAGES $BACKUP_DIR/$DATE/Images";
    }
    // copie les fichiers video
    if (AvecVideos)
    {
        scriptbackup += "\n";
        scriptbackup +=  "cp -R $DIR_VIDEOS $BACKUP_DIR/$DATE/Videos";
    }
    // copie Rufus.ini
    scriptbackup += "\n";
    scriptbackup +=  "cp $RUFUSINI $BACKUP_DIR/$DATE/Rufus.ini";
    if (QFile::exists(QDir::homePath() + SCRIPTBACKUPFILE))
        QFile::remove(QDir::homePath() + SCRIPTBACKUPFILE);
    QFile fbackup(QDir::homePath() + SCRIPTBACKUPFILE);
    if (fbackup.open(QIODevice::ReadWrite))
    {
        QTextStream out(&fbackup);
        out << scriptbackup ;
        fbackup.close();
    }
}

bool Procedures::DefinitRoleUser(int idUser)
{
    /* definit les iduser pour lequel le user travaille
     *  . iduser
        . iduser superviseur des actes                      (int gidUserSuperViseurProv)
            . lui-même s'il est responsable de ses actes
            . un autre user s'il est assistant
            . -1 s'il est assistant pour plusieurs utilisateurs en même temps
            . -2 = sans objet
            . -3 = indéterminé
        . iduser parent qui correspond à l'id de l'utilisateur remplacé quand le superviseur est un remplaçant
            . = idsuperviseur si l'utilisateur n'est pas remplaçant
            . un autre user superviseur qui correspond au soigant remplacé
            . -2 = sans objet
            . -3 = indéterminé
        . iduser qui enregistrera la comptabilité des actes (int gidUserComptableProv)
            . lui même s'il est responsable et libéral
            . son employeur s'il est responsable et salarié
            . s'il est remplaçant (retrocession) on lui demande qui il remplace et le comptable devient
                . celui qu'il remplace si celui qu'il remplace est libéral
                . l'employeur de  celui qu'il remplace si  celui qu'il remplace est salarié
            . -1 s'il n'enregistre pas de compta
            . -2 = sans objet
            . -3 = indéterminé
        . si le userparent cote les actes                            (bool gUseCotationProv)
        . si le userparent enregistre une comptabilité               (bool AvecLaComptaProv)
    */
    QString errormsg = tr("Impossible de retrouver toutes les données utilisateur");
    QString req = "select soignant, responsableactes, userenreghonoraires, userccam from " NOM_TABLE_UTILISATEURS
                  " where iduser = " + QString::number(idUser);

    gidUserSuperViseurProv      = -3;
    gidUserComptableProv        = -3;
    gidUserParentProv           = -3;

    QSqlQuery rolequery(req, db);
    TraiteErreurRequete(rolequery,req);
    if (rolequery.size()==0)
    {
        UpMessageBox::Watch(0,errormsg);
        return false;
    }
    rolequery.seek(0);

    // determination du metier de l'utilisateur
    /* 1 = ophtalmo
     * 2 = orthoptiste
     * 3 = autre
     * 4 = Non soignant
     * 5 = societe comptable
     */
    if (rolequery.value(0).toInt()==1 || rolequery.value(0).toInt()==2 || rolequery.value(0).toInt()==3)
    {
        gAskUser                    = new UpDialog();
        gAskUser                    ->AjouteLayButtons();
        gAskUser                    ->setAccessibleName(QString::number(idUser));
        QVBoxLayout *globallay      = dynamic_cast<QVBoxLayout*>(gAskUser->layout());
        QString req;
        QVBoxLayout *boxlay         = new QVBoxLayout;
        globallay                   ->insertLayout(0,boxlay);

        QGroupBox* boxrole      = new QGroupBox(gAskUser);
        boxrole                 ->setAccessibleName("Role");
        QString lblRole         = tr("Quel est votre rôle dans cette session?");
        boxrole                 ->setTitle(lblRole);
        boxrole                 ->setVisible(false);
        boxlay                  ->addWidget(boxrole);
        QGroupBox* boxsuperv    = new QGroupBox(gAskUser);
        boxsuperv               ->setAccessibleName("Superv");
        QString lblSuperv       = tr("Qui supervise votre activité pour cette session?");
        boxsuperv               ->setTitle(lblSuperv);
        boxsuperv               ->setVisible(false);
        gBoxSuperviseurVisible  =  false;
        boxlay                  ->addWidget(boxsuperv);
        QGroupBox*boxparent     = new QGroupBox(gAskUser);
        boxparent               ->setAccessibleName("Parent");
        QString lblUsrParent    = tr("Qui enregistre vos honoraires?");
        boxparent               ->setTitle(lblUsrParent);
        boxparent               ->setVisible(false);
        gBoxParentVisible    = false;
        boxlay                  ->addWidget(boxparent);

        // determination du superviseur des actes
        // role du user soignant
        /* 1 = effectue exlusivement des actes sous sa responsabilite
         * 2 = effectue des actes sous sa responsabilite et sous celle des autres users
         * 3 = n effectue aucun acte sous sa responsabilité
        */
        // determination du bénéficiaire comptable des actes
        // mode d'enregistrement comptable des actes
        /* 1 = enregistre toujours des honoraires
         * 2 = Salarie
         * 3 = retrocession honoraires ou remplaçant
         * 4 = pas de comptabilite
         */
        switch (rolequery.value(1).toInt()) {
        case 1:
            // le user est responsable de ses actes - on cherche à savoir qui comptabilise ses actes
            Slot_CalcUserParent();
            break;
        case 2:
            // le user alterne entre responsable des actes et assistant suivant la session - on lui demande son rôle pour cette session
            req   = "select iduser from " NOM_TABLE_UTILISATEURS
                    " where (responsableactes = 1 or responsableactes = 2)"
                    " and iduser <> " + QString::number(idUser);
            if (QSqlQuery(req, db).size() == 0)  // s'il ny a pas de responsable autre que lui dans la bbd,
                                                            // il ne peut se connecter que comme responsable
            {
                UpMessageBox::Watch(0,
                                    tr("Vous ne pourrez pas vous connecter en tant qu'assistant"),
                                    tr("Vous étes enregistré comme pouvant être assistant\n"
                                       "mais il n'y a aucun utilisateur susceptible de superviser\n"
                                       "vos actes enregistré dans la base de données"));
                Slot_CalcUserParent();
            }
            else
                CalcUserResponsable();
            break;
        case 3:{
            // le user est assistant - on lui demande qui supervise ses actes
            gidUserParentProv = - 2;
            Slot_CalcUserSuperviseur();
        }
            break;
        default:
            break;
        }

        gAskUser        ->setModal(true);
        globallay       ->setSizeConstraint(QLayout::SetFixedSize);
        connect(gAskUser->OKButton,   SIGNAL(clicked(bool)),  gAskUser, SLOT(accept()));
        bool affichgAskUser = (gidUserSuperViseurProv==-3 || gidUserParentProv==-3);

        if (affichgAskUser)
        {
            if (gAskUser->exec()==0)
            {
                delete gAskUser;
                return false;
            }
            else
            {
                QList<QGroupBox*> Listgroupbx = gAskUser->findChildren<QGroupBox*>();
                for (int i=0; i<Listgroupbx.size(); i++)
                {
                    if (Listgroupbx.at(i)->accessibleName() == "Superv")
                        if (gBoxSuperviseurVisible)
                        {
                            QList<QRadioButton*> listbutt = Listgroupbx.at(i)->findChildren<QRadioButton*>();
                            for (int j=0; j<listbutt.size(); j++)
                                if (listbutt.at(j)->isChecked())
                                    gidUserSuperViseurProv = listbutt.at(j)->accessibleName().toInt();
                        }
                    if (Listgroupbx.at(i)->accessibleName() == "Parent")
                        if (gBoxParentVisible)
                        {
                            QList<QRadioButton*> listbutt = Listgroupbx.at(i)->findChildren<QRadioButton*>();
                            for (int j=0; j<listbutt.size(); j++)
                                if (listbutt.at(j)->isChecked())
                                    gidUserParentProv = listbutt.at(j)->accessibleName().toInt();
                        }
                }
                delete gAskUser;
            }
        }
        if (gidUserSuperViseurProv ==  -3)
        {
            UpMessageBox::Watch(0,tr("Aucun superviseur valide n'a été défini pour vos actes"), tr("Impossible de continuer"));
            return false;
        }
        else if (gidUserSuperViseurProv == -1)  // le user est assistant et travaille pour tout le monde
        {
            gidUserComptableProv        = -2;
            gidUserParentProv           = -2;
            avecLaComptaProv            = true;
            gUseCotationProv            = true;
        }
        else
        {
            // determination de comptabilité - cotation
            if (gidUserParentProv == -3) // -> le user est assistant et il a un superviseur, on essaie de déterminer qui comptabilise les actes
            {
                QString req2 = "select UserEnregHonoraires, UserEmployeur from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(gidUserSuperViseurProv);
                QSqlQuery cptquery(req2, db);
                TraiteErreurRequete(cptquery,req2);
                cptquery.first();
                if (cptquery.value(0).toInt()==3)
                {
                    // le superviseur est remplaçant, on essaie de savoir s'il a un parent
                    QSqlQuery soignquer("select soignant from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(gidUserSuperViseurProv), db);
                    soignquer.first();
                    QString req   = "select iduser, userlogin from " NOM_TABLE_UTILISATEURS
                            " where (userenreghonoraires = 1 or userenreghonoraires = 2)"
                            " and iduser <> " + QString::number(gidUserSuperViseurProv) +
                            " and iduser <> " + QString::number(idUser) +
                            " and soignant = " + soignquer.value(0).toString() +
                            " and userdesactive is null";
                    //qDebug() << req;
                    QSqlQuery quer(req, db);
                    TraiteErreurRequete(quer,req);
                    if (quer.size() == 1)
                    {
                        quer              .first();
                        gidUserParentProv   = quer.value(0).toInt();
                    }
                    else // on va demander qui est le soignant parent de ce remplaçant....
                    {
                        gAskUser                = new UpDialog();
                        gAskUser                ->AjouteLayButtons();
                        gAskUser                ->setAccessibleName(QString::number(gidUserSuperViseurProv));
                        QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(gAskUser->layout());
                        QVBoxLayout *boxlay     = new QVBoxLayout;
                        globallay               ->insertLayout(0,boxlay);
                        QGroupBox*boxparent     = new QGroupBox(gAskUser);
                        boxparent               ->setAccessibleName("Parent");
                        QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + getLogin(gidUserSuperViseurProv) + "?";
                        boxparent               ->setTitle(lblUsrParent);
                        boxparent               ->setVisible(false);
                        boxlay                  ->addWidget(boxparent);

                        Slot_CalcUserParent();
                        gAskUser                ->setModal(true);
                        globallay               ->setSizeConstraint(QLayout::SetFixedSize);
                        connect(gAskUser->OKButton,   SIGNAL(clicked(bool)),  gAskUser, SLOT(accept()));
                        if (gAskUser->exec()==0)
                        {
                            delete gAskUser;
                            return false;
                        }
                        else
                        {
                            QList<QGroupBox*> Listgroupbx = gAskUser->findChildren<QGroupBox*>();
                            for (int i=0; i<Listgroupbx.size(); i++)
                            {
                                if (Listgroupbx.at(i)->accessibleName() == "Parent")
                                {
                                    QList<QRadioButton*> listbutt = Listgroupbx.at(i)->findChildren<QRadioButton*>();
                                    for (int j=0; j<listbutt.size(); j++)
                                        if (listbutt.at(j)->isChecked())
                                            gidUserParentProv = listbutt.at(j)->accessibleName().toInt();
                                }
                            }
                            delete gAskUser;
                        }
                    }
                }
                else
                    gidUserParentProv = gidUserSuperViseurProv;
            }
            req = "select userenreghonoraires, userccam, userEmployeur from " NOM_TABLE_UTILISATEURS
                    " where iduser = " + QString::number(gidUserParentProv);
            QSqlQuery cptquer(req,db);
            cptquer.first();
            // determination de l'utilisation de la cotation
            gUseCotationProv = (cptquer.value(1).toInt()==1);
            // determination de l'utilisation de la comptabilité
            avecLaComptaProv = (cptquer.value(0).toInt()!=4);
            switch (cptquer.value(0).toInt()) {
            case 1:
                gidUserComptableProv = gidUserParentProv;
                break;
            case 2:
                gidUserComptableProv = cptquer.value(2).toInt();
                break;
            default:
                gidUserComptableProv = -1;
                break;
            }
         }
        return true;
    }
    else  // il s'agit d'un administratif ou d'une société comptable
    {
        gidUserSuperViseurProv      = -2;
        gidUserComptableProv        = -2;
        gidUserParentProv           = -2;
        gUseCotationProv            = true;
        avecLaComptaProv            = true;
        return true;
    }
    return false;
}

void Procedures::CalcUserResponsable()
{
    QList<QGroupBox*> Listgroupbx   = gAskUser->findChildren<QGroupBox*>();
    QGroupBox *ptbox = 0;
    for (int i=0; i<Listgroupbx.size(); i++)
        if (Listgroupbx.at(i)->accessibleName() == "Role")
        {
            ptbox = Listgroupbx.at(i);
            break;
        }
    ptbox                   ->setVisible(true);
    QFontMetrics fm         = QFontMetrics(qApp->font());
    int hauteurligne        = fm.height()*1.6;
    ptbox                   ->setFixedHeight((3*hauteurligne)+5);
    QVBoxLayout *vbox       = new QVBoxLayout;
    QRadioButton *pbuttResp = new QRadioButton(ptbox);
    pbuttResp               ->setText(tr("Responsable de mes actes"));
    pbuttResp               ->setAccessibleName("buttresp");
    connect(pbuttResp,      SIGNAL(clicked(bool)),  this, SLOT(Slot_CalcUserParent()));
    vbox                    ->addWidget(pbuttResp);
    QRadioButton *pbuttAss  = new QRadioButton(ptbox);
    pbuttAss                ->setText(tr("Assistant"));
    pbuttAss                ->setAccessibleName("buttass");
    pbuttAss                ->setChecked(true);      // le user est défini par défaut comme assistant -> on cherche qui supervise les actes
    connect(pbuttAss,       SIGNAL(clicked(bool)),  this, SLOT(Slot_CalcUserSuperviseur()));
    vbox                    ->addWidget(pbuttAss);
    vbox                    ->setContentsMargins(8,0,8,0);
    ptbox                   ->setLayout(vbox);
    dynamic_cast<QVBoxLayout*>(gAskUser->layout())->setSizeConstraint(QLayout::SetFixedSize);
    Slot_CalcUserSuperviseur();
}

void Procedures::Slot_CalcUserSuperviseur()
{
    gidUserSuperViseurProv  = -3;
    gidUserParentProv       = -3;
    int iduser              = gAskUser->accessibleName().toInt();
    QGroupBox *ptbox = 0;
    QList<QGroupBox*> Listgroupbx   = gAskUser->findChildren<QGroupBox*>();
    for (int i=0; i<Listgroupbx.size(); i++)
    {
        if (Listgroupbx.at(i)->accessibleName() == "Superv")
            ptbox = Listgroupbx.at(i);
        if (Listgroupbx.at(i)->accessibleName() == "Parent")
            Listgroupbx.at(i)->setVisible(false);

    }
    QList<QRadioButton*> listbutt = ptbox->findChildren<QRadioButton*>();
    for (int j=0; j<listbutt.size(); j++)
        delete listbutt.at(j);
    delete ptbox->layout();
    QSqlQuery medquer("select Medecin from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(iduser), db);
    medquer.first();
    bool medecin = (medquer.value(0).toInt()==1);
    QString req   = "select iduser, userlogin from " NOM_TABLE_UTILISATEURS
            " where responsableactes < 3"
            " and iduser <> " + QString::number(iduser) +
            " and userdesactive is null";
    if (medecin)
        req += " and medecin = 1";
    //qDebug() << req;
    QSqlQuery quer(req, db);
    TraiteErreurRequete(quer,req);
    ptbox                   ->setVisible(quer.size() > 1);
    gBoxSuperviseurVisible  = (quer.size() > 1);
    gBoxParentVisible    = false;

    if (quer.size() == 1)
    {
        quer              .first();
        gidUserSuperViseurProv   = quer.value(0).toInt();
    }
    else
    {
        QFontMetrics fm         = QFontMetrics(qApp->font());
        int hauteurligne        = fm.height()*1.6;
        ptbox                   ->setFixedHeight(((quer.size() + 2)*hauteurligne)+5);
        QVBoxLayout *vbox       = new QVBoxLayout;
        for (int i=0; i<quer.size(); i++)
        {
            quer            .seek(i);
            QRadioButton *pradiobutt = new QRadioButton(ptbox);
            pradiobutt      ->setText(quer.value(1).toString());
            pradiobutt      ->setAccessibleName(quer.value(0).toString());
            vbox            ->addWidget(pradiobutt);
            pradiobutt      ->setChecked(i==0);
        }
        QRadioButton *pradiobutt = new QRadioButton();
        pradiobutt      ->setText(tr("Tout le monde"));
        pradiobutt      ->setAccessibleName("-1");
        vbox            ->addWidget(pradiobutt);
        vbox            ->setContentsMargins(8,0,8,0);
        ptbox           ->setLayout(vbox);
    }
    dynamic_cast<QVBoxLayout*>(gAskUser->layout())->setSizeConstraint(QLayout::SetFixedSize);
}

void Procedures::Slot_CalcUserParent()
{
    int iduser = gAskUser->accessibleName().toInt();
    gidUserSuperViseurProv  =   iduser;
    QGroupBox *ptbox = 0;
    QList<QGroupBox*> Listgroupbx   = gAskUser->findChildren<QGroupBox*>();
    for (int i=0; i<Listgroupbx.size(); i++)
    {
        if (Listgroupbx.at(i)->accessibleName() == "Superv")
            Listgroupbx.at(i)->setVisible(false);
        if (Listgroupbx.at(i)->accessibleName() == "Parent")
            ptbox = Listgroupbx.at(i);
    }
    ptbox->setVisible(false);
    gBoxSuperviseurVisible  = false;
    gBoxParentVisible    = false;
    // on a déterminé le superviseur, on cherche qui cenregistre les actes
    QString req = "select UserEnregHonoraires, UserEmployeur from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(gidUserSuperViseurProv);
    QSqlQuery quer1(req, db);
    TraiteErreurRequete(quer1,req);
    quer1.first();
    switch (quer1.value(0).toInt()) {
    case 3:                     // *  3. le superviseur est remplaçant                  -> il faut lui demander qui il remplace
    {
        QSqlQuery soignquer("select soignant from " NOM_TABLE_UTILISATEURS " where iduser = " + gAskUser->accessibleName(), db);
        soignquer.first();
        req = "select iduser, userlogin from " NOM_TABLE_UTILISATEURS
                " where (userenreghonoraires = 1 or userenreghonoraires = 2)"
                " and iduser <> " + QString::number(gidUserSuperViseurProv) +
                " and iduser <> " + QString::number(iduser) +
                " and soignant = " + soignquer.value(0).toString() +
                " and userdesactive is null";
        QSqlQuery quer(req, db);
        TraiteErreurRequete(quer,req);
        ptbox                   ->setVisible(quer.size() > 1);
        gBoxParentVisible    = (quer.size() > 1);
        if (quer.size() == 1)
        {
            quer              .first();
            gidUserParentProv   = quer.value(0).toInt();
        }
        else
        {
            QList<QRadioButton*> listbutt = ptbox->findChildren<QRadioButton*>();
            for (int j=0; j<listbutt.size(); j++)
                delete listbutt.at(j);
            delete ptbox->layout();
            QFontMetrics fm         = QFontMetrics(qApp->font());
            int hauteurligne        = fm.height()*1.6;
            ptbox       ->setFixedHeight(((quer.size() + 1)*hauteurligne)+5);
            QVBoxLayout *vbox       = new QVBoxLayout;
            for (int i=0; i<quer.size(); i++)
            {
                quer        .seek(i);
                QRadioButton *pradiobutt = new QRadioButton(ptbox);
                pradiobutt  ->setText(quer.value(1).toString());
                pradiobutt  ->setAccessibleName(quer.value(0).toString());
                pradiobutt  ->setChecked(i==0);
                vbox        ->addWidget(pradiobutt);
            }
            vbox            ->setContentsMargins(8,0,8,0);
            ptbox           ->setLayout(vbox);
        }
    }
        break;
    default:                    // *  4. le superviseur est sans compta               -> pas de compta des actes
                                // *  1. le superviseur est salarié ou libéral     -> c'est lui qui est le parent
        gidUserParentProv           = gidUserSuperViseurProv;
        break;
    }
    dynamic_cast<QVBoxLayout*>(gAskUser->layout())->setSizeConstraint(QLayout::SetFixedSize);
}

int Procedures::idCentre()
{
    return gidCentre;
}

int Procedures::idLieuExercice()
{
    return gidLieuExercice;
}

int Procedures::UserComptable()
{
    return gidUserComptable;
}

int Procedures::UserSuperviseur()
{
    return gidUserSuperViseur;
}

int Procedures::UserParent()
{
    return gidUserParent;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Premier démarrage de Rufus - reconstruction du fichier Rufus.ini et de la base ---------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Procedures::PremierDemarrage()
{
    QMessageBox     msgbox;
    int         protoc;
    enum protoc {BaseExistante, BaseRestauree, BaseVierge};
    UpSmallButton    *AnnulBouton   = new UpSmallButton;
    UpSmallButton    *OKBouton      = new UpSmallButton;
    UpSmallButton    *YesBouton     = new UpSmallButton;
    UpSmallButton    *NoBouton      = new UpSmallButton;

    msgbox.setText(tr("Premier démarrage de Rufus!"));
    msgbox.setInformativeText(tr("Cette étape va vous permettre de configurer le logiciel en quelques secondes.\n\n"
                              "Cette installation ne peut aboutir si vous n'avez pas de serveur MySQL installé.\n"
                              "Dans ce cas, il vous faut annuler et installer un serveur MySQL sur cet ordinateur ou sur un autre poste du réseau.\n\n"
                              "Commencez par choisir la situation qui décrit le mieux votre installation de Rufus.\n\n"
                              "1. J'installe Rufus sur ce poste et ce poste se connecte à une base patients qui existe dèjà sur le serveur\n"
                              "2. J'installe Rufus sur ce poste et ce poste se connectera à une base patients que je vais restaurer sur le serveur"
                              " à partir d'une sauvegarde\n"
                              "3. J'installe Rufus sur ce poste et ce poste se connectera à une base patients vierge que je vais créer sur le serveur\n"));
    msgbox.setIcon(QMessageBox::Information);

    AnnulBouton->setText(tr("Retour\nau menu d'accueil"));
    msgbox.addButton(AnnulBouton, QMessageBox::RejectRole);
    NoBouton->setText(tr("Base patients existante\nsur le serveur"));
    msgbox.addButton(NoBouton, QMessageBox::AcceptRole);
    OKBouton->setText(tr("Base patients restaurée\ndepuis une sauvegarde"));
    msgbox.addButton(OKBouton, QMessageBox::YesRole);
    YesBouton->setText(tr("Nouvelle base\npatients vierge"));
    msgbox.addButton(YesBouton, QMessageBox::ActionRole);
    msgbox.exec();

    protoc = BaseExistante;
    if (msgbox.clickedButton() == AnnulBouton)
        return false;
    else if (msgbox.clickedButton() == NoBouton)
        protoc = BaseExistante;
    else if (msgbox.clickedButton() == OKBouton)
        protoc = BaseRestauree;
    else if (msgbox.clickedButton() == YesBouton)
        protoc = BaseVierge;

    gsettingsIni    = new QSettings(gnomFichIni, QSettings::IniFormat);

    if (protoc == BaseExistante)
    {
        if (VerifParamConnexion())
        {
            int idusr = VerifUserBase(gLoginUser,gMDPUser);
            gdbOK = (idusr > -1);
            if (!gdbOK)
                return false;
            gidUser     = idusr;
            PremierParametrageMateriel();
            PremierParametrageRessources();
            UpMessageBox::Watch(0, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne,\n"
                                       "le login ") + gLoginUser + tr(" est reconnu et le programme va démarrer\n"));
            if (DefinitRoleUser(gidUser))
            {
                gidUserSuperViseur      = gidUserSuperViseurProv;
                gidUserComptable        = gidUserComptableProv;
                gidUserParent           = gidUserParentProv;
                gUseCotation            = gUseCotationProv;
                avecLaCompta            = (avecLaComptaProv? (gUseCotationProv? 0 : 4) : (gUseCotationProv? 2 : 1));
                return true;
            }
        }
    }

    else if (protoc == BaseRestauree)
    {
        bool SansAccesDistant = false;
        if (VerifParamConnexion(SansAccesDistant))
        {
            UpMessageBox::Watch(0,tr("Connexion réussie"),
                                  tr("Bien, la connexion au serveur MySQL fonctionne,\n"));
            // Restauration de la base
            if (!RestaureBase(false, true, false))
                return false;
            PremierParametrageMateriel();
            PremierParametrageRessources();
            int idusr = VerifUserBase(gLoginUser,gMDPUser);
            gdbOK = (idusr > -1);
            if (!gdbOK)
                return false;
            gidUser     = idusr;
            UpMessageBox::Watch(0, tr("Redémarrage nécessaire"),
                                   tr("Le programme va se fermer pour que les modifications de la base Rufus\n"
                                      "puissent être prises en compte\n"));
            exit(0);
        }
    }
    else if (protoc == BaseVierge)
    {
        bool SansAccesDistant = false;
        if (VerifParamConnexion(SansAccesDistant))
        {
            UpMessageBox::Watch(0, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne "
                                       "et le programme va maintenant créer une base de données patients "
                                       "vierge de tout enregistrement."));
            // Création de la base
            QString NomDirRessrces = QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES;
            QDir DirRessrces(NomDirRessrces);
            if (!DirRessrces.exists())
                DirRessrces.mkdir(NomDirRessrces);
            if (!RestaureBase(true, true))
                return false;
            if (gMode == ReseauLocal)
                QSqlQuery("update " NOM_TABLE_PARAMSYSTEME " set AdresseServeurLocal = '" + gsettingsIni->value("BDD_LOCAL/Serveur").toString() + "'", db);

            // Création de l'utilisateur
            gdbOK = CreerPremierUser(gLoginUser, gMDPUser);
            setlisteUsers();
            ChargeDataUser(gidUser);
            return gdbOK;
        }
    }
    return false;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Paramètrage de l'imprimante ---------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Procedures::PremierParametrageMateriel()
{
    gsettingsIni->setValue("Param_Imprimante/TailleEnTete","45");
    gsettingsIni->setValue("Param_Imprimante/TailleEnTeteALD","63");
    gsettingsIni->setValue("Param_Imprimante/TaillePieddePage","20");
    gsettingsIni->setValue("Param_Imprimante/TaillePieddePageOrdoLunettes","40");
    gsettingsIni->setValue("Param_Imprimante/TailleTopMarge","3");
    gsettingsIni->setValue("Param_Imprimante/ApercuAvantImpression","NO");
    gsettingsIni->setValue("PyxInterf/PyxvitalPath", QDir::homePath()+"/Documents/Pyxvital");
    gsettingsIni->setValue("Param_Poste/Autoref","-");
    gsettingsIni->setValue("Param_Poste/Refracteur","-");
    gsettingsIni->setValue("Param_Poste/Fronto","-");
    gsettingsIni->setValue("Param_Poste/Tonometre","-");
    gsettingsIni->setValue("Param_Poste/PortAutoref","-");
    gsettingsIni->setValue("Param_Poste/PortRefracteur","-");
    gsettingsIni->setValue("Param_Poste/PortFronto","-");
    gsettingsIni->setValue("Param_Poste/PortTonometre","-");
    gsettingsIni->setValue("BDD_LOCAL/PrioritaireGestionDocs","NO");
    gsettingsIni->setValue("Param_Poste/VersionRessources", VERSION_RESSOURCES);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Création des fichiers ressources ---------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Procedures::PremierParametrageRessources()
{
    QString NomDirRessrces = QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES;
    QDir DirRessrces(NomDirRessrces);
    if (DirRessrces.exists())
        DirRessrces.rmdir(NomDirRessrces);
    DirRessrces.mkdir(NomDirRessrces);
    QFile COACopier(QStringLiteral("://Corps_Ordonnance.txt"));
    COACopier.copy(QDir::homePath() + NOMFIC_CORPSORDO);
    QFile CO(QDir::homePath() + NOMFIC_CORPSORDO);
    CO.open(QIODevice::ReadWrite);
    CO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile COALDACopier(QStringLiteral("://Corps_OrdoALD.txt"));
    COALDACopier.copy(QDir::homePath() + NOMFIC_CORPSORDOALD);
    QFile COALD(QDir::homePath() + NOMFIC_CORPSORDOALD);
    COALD.open(QIODevice::ReadWrite);
    COALD.setPermissions(QFileDevice::ReadOther     | QFileDevice::WriteOther
                         | QFileDevice::ReadGroup   | QFileDevice::WriteGroup
                         | QFileDevice::ReadOwner   | QFileDevice::WriteOwner
                         | QFileDevice::ReadUser    | QFileDevice::WriteUser);

    QFile EOACopier(QStringLiteral("://Entete_Ordonnance.txt"));
    EOACopier.copy(QDir::homePath() + NOMFIC_ENTETEORDO);
    QFile EO(QDir::homePath() + NOMFIC_ENTETEORDO);
    EO.open(QIODevice::ReadWrite);
    EO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile EOALDACopier(QStringLiteral("://Entete_OrdoALD.txt"));
    EOALDACopier.copy(QDir::homePath() + NOMFIC_ENTETEORDOALD);
    QFile EOALD(QDir::homePath() + NOMFIC_ENTETEORDOALD);
    EOALD.open(QIODevice::ReadWrite);
    EOALD.setPermissions(QFileDevice::ReadOther     | QFileDevice::WriteOther
                         | QFileDevice::ReadGroup   | QFileDevice::WriteGroup
                         | QFileDevice::ReadOwner   | QFileDevice::WriteOwner
                         | QFileDevice::ReadUser    | QFileDevice::WriteUser);

    QFile POLACopier(QStringLiteral("://Pied_Ordonnance_Lunettes.txt"));
    POLACopier.copy(QDir::homePath() + NOMFIC_PIEDORDOLUNETTES);
    QFile POL(QDir::homePath() + NOMFIC_PIEDORDOLUNETTES);
    POL.open(QIODevice::ReadWrite);
    POL.setPermissions(QFileDevice::ReadOther   | QFileDevice::WriteOther
                       | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                       | QFileDevice::ReadOwner | QFileDevice::WriteOwner
                       | QFileDevice::ReadUser  | QFileDevice::WriteUser);

    QFile POACopier(QStringLiteral("://Pied_Ordonnance.txt"));
    POACopier.copy(QDir::homePath() + NOMFIC_PIEDPAGE);
    QFile PO(QDir::homePath() + NOMFIC_PIEDPAGE);
    PO.open(QIODevice::ReadWrite);
    PO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile PDFACopier(QStringLiteral("://pdf.pdf"));
    PDFACopier.copy(QDir::homePath() + NOMFIC_PDF);
    QFile pdf(QDir::homePath() + NOMFIC_PDF);
    pdf.open(QIODevice::ReadWrite);
    pdf.setPermissions(QFileDevice::ReadOther   | QFileDevice::WriteOther
                       | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                       | QFileDevice::ReadOwner | QFileDevice::WriteOwner
                       | QFileDevice::ReadUser  | QFileDevice::WriteUser);
    gsettingsIni->setValue("Param_Poste/VersionRessources",VERSION_RESSOURCES);
    if (gMode == Poste)
    {
        QString NomDirImg = QDir::homePath() + NOMDIR_RUFUS "/Imagerie";
        QDir DirImg(NomDirImg);
        if (DirImg.exists())
            DirImg.rmdir(NomDirImg);
        DirImg.mkdir(NomDirImg);
        gsettingsIni->setValue("BDD_POSTE/DossierImagerie", NomDirImg);
        QString reqimg = "update " NOM_TABLE_PARAMSYSTEME " set DirImagerie = '" + NomDirImg + "'";
        //qDebug() << reqimg;
        QSqlQuery (reqimg, db);
    }
 }

/*------------------------------------------------------------------------------------------------------------------------------------
-- Vérifie la présence et la cohérence du fchier d'initialisation et le reconstruit au besoin ----------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifIni(QString msg, QString msgInfo, bool DetruitIni, bool RecupIni, bool ReconstruitIni, bool ReconstruitBase, bool PremDemarrage, bool BaseVierge)
{
    UpSmallButton *AnnulBouton              = new UpSmallButton();
    UpSmallButton *RecupIniBouton           = new UpSmallButton();
    UpSmallButton *ReconstruitIniBouton     = new UpSmallButton();
    UpSmallButton *ReconstruitBaseBouton    = new UpSmallButton();
    UpSmallButton *PremierDemarrageBouton   = new UpSmallButton();
    UpSmallButton *BaseViergeBouton         = new UpSmallButton();

    QMessageBox msgbox;
    msgbox.setText(msg);
    msgbox.setInformativeText(msgInfo);
    msgbox.setIcon(QMessageBox::Warning);
    if (ReconstruitIni)
    {
        ReconstruitIniBouton->setText(tr("Reconstruire le fichier\nd'initialisation"));
        msgbox.addButton(ReconstruitIniBouton, QMessageBox::AcceptRole);
    }
    if (RecupIni)
    {
        RecupIniBouton->setText(tr("Restaurer le fichier d'initialisation\nà partir d'une sauvegarde"));
        msgbox.addButton(RecupIniBouton, QMessageBox::AcceptRole);
    }
    if (ReconstruitBase)
    {
        ReconstruitBaseBouton->setText(tr("Reconstruire les données\nà partir d'un fichier de sauvegarde"));
        msgbox.addButton(ReconstruitBaseBouton, QMessageBox::AcceptRole);
    }
    if (PremDemarrage)
    {
        PremierDemarrageBouton->setText(tr("Premier démarrage\nde Rufus"));
        msgbox.addButton(PremierDemarrageBouton, QMessageBox::AcceptRole);
    }
    if (BaseVierge)
    {
        BaseViergeBouton->setText(tr("Reconstruire\nune base vierge"));
        msgbox.addButton(BaseViergeBouton, QMessageBox::AcceptRole);
    }
    AnnulBouton->setText(tr("Abandonner et\nquitter Rufus"));
    msgbox.addButton(AnnulBouton, QMessageBox::AcceptRole);
    msgbox.exec();
    bool reponse = false;

    if (msgbox.clickedButton()==AnnulBouton)
    {
        delete AnnulBouton;
        delete RecupIniBouton;
        delete ReconstruitIniBouton;
        delete ReconstruitBaseBouton;
        delete PremierDemarrageBouton;
        delete BaseViergeBouton;
        if (DetruitIni)
            QFile::remove(gnomFichIni);
        exit(0);
    }
    else if (msgbox.clickedButton()==RecupIniBouton)
    {
        QFileDialog dialog(0, tr("Choisir le fichier d'initialisation"), QDir::homePath() + "/Documents/Rufus/","Text files (Rufus*.ini)");
        dialog.setViewMode(QFileDialog::List);
        dialog.setFileMode(QFileDialog::ExistingFile);
        int a = dialog.exec();
        if (a>0)
        {
            QFile FichierIni(gnomFichIni);
            if (FichierIni.exists())
                FichierIni.remove();
            QString fileini = dialog.selectedFiles().at(0);
            QFile rufusini(fileini);
            rufusini.copy(gnomFichIni);
            gsettingsIni    = new QSettings(gnomFichIni, QSettings::IniFormat);
            if (QMessageBox::question(0,"", tr("Restaurer aussi les fichiers modèles d'impression?")) != QMessageBox::NoButton)
                PremierParametrageRessources();
            reponse = true;
        }
    }
    else if (msgbox.clickedButton()==ReconstruitIniBouton)
    {
        //reconstruire le fichier rufus.ini
        //1. on demande les paramètres de connexion au serveur - mode d'accès / user / mdp / port / SSL
        QFile(gnomFichIni).remove();
        gsettingsIni    = new QSettings(gnomFichIni, QSettings::IniFormat);
        reponse = VerifParamConnexion();
        if (reponse)
        {
            int idusr = VerifUserBase(gLoginUser, gMDPUser);
            gdbOK = (idusr > -1);
            if (!gdbOK)
                return false;
            gidUser     = idusr;
            PremierParametrageMateriel();
            UpMessageBox::Watch(0,tr("Le fichier Rufus.ini a été reconstruit"), tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
            exit(0);
        }
    }
    else if (msgbox.clickedButton()==ReconstruitBaseBouton)
    {
        //reconstruire la base de données vierge ou à partir d'un dump
        reponse = RestaureBase(false, false, false);
        if (reponse)
            UpMessageBox::Watch(0,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
        else
            UpMessageBox::Watch(0,tr("Restauration impossible de la base"));
        exit(0);
    }
    else if (msgbox.clickedButton()==PremierDemarrageBouton)
    {
        reponse =  PremierDemarrage();
    }
    else if (msgbox.clickedButton()==BaseViergeBouton)
    {
        reponse = RestaureBase(true, true, false);
        if (reponse)
            UpMessageBox::Watch(0,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
        else
            UpMessageBox::Watch(0,tr("Restauration impossible de la base"));
        exit(0);
    }
    delete AnnulBouton;
    delete RecupIniBouton;
    delete ReconstruitIniBouton;
    delete ReconstruitBaseBouton;
    delete PremierDemarrageBouton;
    delete BaseViergeBouton;
    return reponse;
}

/*---------------------------------------------------------------------------------------------------------------------
    -- VÉRIFICATION DE MDP --------------------------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifMDP(QString MDP, QString Msg)
{
    QInputDialog *quest = new QInputDialog();
    quest->setCancelButtonText("Annuler");
    quest->setLabelText(Msg);
    quest->setInputMode(QInputDialog::TextInput);
    quest->setTextEchoMode(QLineEdit::Password);
    QList<QLineEdit*> list = quest->findChildren<QLineEdit*>();
    for (int i=0;i<list.size();i++)
        list.at(0)->setAlignment(Qt::AlignCenter);
    QList<QLabel*> listlab = quest->findChildren<QLabel*>();
    for (int i=0;i<listlab.size();i++)
        listlab.at(0)->setAlignment(Qt::AlignCenter);
    quest->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    if (quest->exec() > 0)
    {
        if (quest->textValue() == MDP)
            return true;
        else
            UpMessageBox::Watch(0,tr("Mot de passe invalide!"));
    }
    return false;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Vérifie et répare les paramètres de connexion  -----------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifParamConnexion(bool OKAccesDistant, QString)
{
    Dlg_ParamConnex = new dlg_paramconnexion(OKAccesDistant);
    Dlg_ParamConnex ->setWindowTitle(tr("Entrez les paramètres de connexion au serveur"));
    Dlg_ParamConnex ->setFont(QFont(POLICEPARDEFAUT,POINTPARDEFAUT));

    if (Dlg_ParamConnex->exec()>0)
    {
        db = Dlg_ParamConnex->getdatabase();

        QString Base;
        if (Dlg_ParamConnex->ui->PosteradioButton->isChecked())
        {
            Base = "BDD_POSTE";
            gMode = Poste;
        }
        else if (Dlg_ParamConnex->ui->LocalradioButton->isChecked())
        {
            Base = "BDD_LOCAL";
            gsettingsIni->setValue(Base + "/Serveur",   Dlg_ParamConnex->ui->IPlineEdit->text());
            gMode = ReseauLocal;
        }
        else if (Dlg_ParamConnex->ui->DistantradioButton->isChecked())
        {
            Base = "BDD_DISTANT";
            gsettingsIni->setValue(Base + "/Serveur",   Dlg_ParamConnex->ui->IPlineEdit->text());
            gMode = Distant;
        }
        gsettingsIni->setValue(Base + "/Active",    "YES");
        gsettingsIni->setValue(Base + "/Port", Dlg_ParamConnex->ui->PortcomboBox->currentText());

        gLoginUser  =  Dlg_ParamConnex->ui->LoginlineEdit->text();
        gMDPUser    =  Dlg_ParamConnex->ui->MDPlineEdit->text();
        gdbOK = true;
        delete Dlg_ParamConnex;
        return true;
    }
    delete Dlg_ParamConnex;
    return false;
}

/*---------------------------------------------------------------------------------------------------------------------
    -- VÉRIFICATION DES Fichiers ressources ---------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifRessources(QString Nomfile)
{
    QMessageBox msgbox;
    UpSmallButton *OKBouton = new UpSmallButton;
    UpSmallButton *RestaurerBouton = new UpSmallButton;
    UpSmallButton *RemplirBouton = new UpSmallButton;
    msgbox.setText(tr("Il manque un fichier d'impression"));
    msgbox.setInformativeText(tr("Le fichier ressource d'impression ") + Nomfile + tr(" est absent.\n"
                              "Voulez vous restaurer les fichiers ressources d'impression?.\n"));
    msgbox.setIcon(QMessageBox::Warning);
    OKBouton->setText(tr("Annuler"));
    msgbox.addButton(OKBouton, QMessageBox::RejectRole);
    RestaurerBouton->setText(tr("Reconstruire les fichiers à partir d'une sauvegarde"));
    msgbox.addButton(RemplirBouton, QMessageBox::AcceptRole);
    RemplirBouton->setText(tr("Réinitialiser les fichiers"));
    msgbox.addButton(RestaurerBouton, QMessageBox::AcceptRole);
    msgbox.exec();
    if (msgbox.clickedButton()==OKBouton)
        return false;
    else if (msgbox.clickedButton()==RemplirBouton)
        PremierParametrageRessources();
    else if (msgbox.clickedButton()==RestaurerBouton)
    {
        QFileDialog dialog(0, tr("Choisir le dossier de ressources d'impression à restaurer"), QDir::homePath() + "/dumpsRufus","SQL files (dump*.sql)");
        dialog.setViewMode(QFileDialog::List);
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec()>0)
        {
            QDir dockdir = dialog.directory();
            QDir DirRssces;
            if (!DirRssces.exists(QDir::homePath() + "/Documents/Rufus/Ressources"))
                DirRssces.mkdir(QDir::homePath() + "/Documents/Rufus/Ressources");
            QStringList listnomfic = dockdir.entryList();
            for (int i=0; i<listnomfic.size(); i++)
            {
                QFile ficACopier(dockdir.absolutePath() + "/" + listnomfic.at(i));
                QString nomficACopier = QFileInfo(listnomfic.at(i)).fileName();
                ficACopier.copy(QDir::homePath() + "/Documents/Rufus/Ressources/" + nomficACopier);
            }
            return true;
        }
        else return false;
    }
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Vérifie que l'utilisateur existe dans la base  -----------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
int Procedures::VerifUserBase(QString Login, QString MDP)
{
    QMessageBox msgbox;
    UpSmallButton *OKBouton = new UpSmallButton;
    msgbox.setIcon(QMessageBox::Information);
    OKBouton->setText("OK");
    msgbox.addButton(OKBouton, QMessageBox::AcceptRole);
    QString req = "SHOW TABLES FROM " NOM_BASE_CONSULTS " LIKE 'utilisateurs'";
    QSqlQuery VerifBaseQuery(req,db);
    if (VerifBaseQuery.size()==0)
    {
        msgbox.setText(tr("Erreur sur la base patients"));
        msgbox.setInformativeText(tr("La connexion au serveur fonctionne mais "
                                  "votre base de données semble endommagée.\n"
                                  "La table des utilisateurs n'existe pas.\n"
                                  "Impossible de continuer."));
        msgbox.exec();
        return -2;
    }
    req =   "SELECT idUser FROM " NOM_TABLE_UTILISATEURS
            " WHERE UserLogin = '" + CorrigeApostrophe(Login) +
            "' AND UserMDP = '" + CorrigeApostrophe(MDP) + "'" ;
    QSqlQuery idUsrQuery(req,db);
    if (idUsrQuery.size()==0)
    {
        req =   "SELECT UserLogin FROM " NOM_TABLE_UTILISATEURS;
        QSqlQuery listusrquery (req,db);
        if (listusrquery.size() == 0)
        {
            msgbox.setText(tr("Erreur sur la base patients"));
            msgbox.setInformativeText(tr("La connexion au serveur fonctionne mais "
                                      "votre base patients semble endommagée.\n"
                                      "Il n'y a aucun utilisateur enregistré "
                                      "dans la table des utilisateurs.\n"
                                      "Impossible de continuer."));
            msgbox.exec();
            return -2;
        }
        listusrquery.first();
        QString listusr;
        bool ExistLogin = false;
        for (int i=0; i< listusrquery.size(); i++)
        {
            if (listusrquery.value(0).toString() == Login)
                ExistLogin = true;
            else
                listusr += "\n\t" + listusrquery.value(0).toString();
            listusrquery.next();
        }
        if (ExistLogin)
        {
            msgbox.setText(tr("Erreur sur le compte utilisateur"));
            msgbox.setInformativeText(tr("Le Login ") + Login + tr(" existe bien dans la base de données mais pas avec le mot de passe que vous avez utilisé."
                                      "Modifiez le mot de passe MySQL de ") + Login + tr(" pourqu'il soit identique à celui de la base de données Rufus\n"
                                      "ou connectez vous avec un autre des logins référencés dans la base Rufus.\n") + listusr + "\n"
                                      + tr("Impossible de continuer."));
        }
        else
        {
            msgbox.setText(tr("Erreur sur le compte utilisateur"));
            msgbox.setInformativeText(tr("La connexion au serveur fonctionne et "
                                      "la base de données de Rufus semble en bon état mais "
                                      "il est impossible de retrouver l'utilisateur ") + Dlg_ParamConnex->ui->LoginlineEdit->text() + "!\n\n"
                                      + tr("Connectez vous avec un login référencé dans la base Rufus\n") + listusr + "\n");
        }
        msgbox.exec();
        return -3;
    }
    idUsrQuery.first();
    return idUsrQuery.value(0).toInt();
}

void Procedures::VideDatabases()
{
    Message(tr("Suppression de l'ancienne base Rufus en cours"));
    QSqlQuery ("drop database if exists " NOM_BASE_COMPTA,      db);
    QSqlQuery ("drop database if exists " NOM_BASE_OPHTA,       db);
    QSqlQuery ("drop database if exists " NOM_BASE_CONSULTS,    db);
//    QString req = "show tables ";
//    QSqlQuery ("use " NOM_BASE_OPHTA, db);
//    QSqlQuery listophtaquery(req,db);
//    listophtaquery.first();
//    for (int i=0; i<listophtaquery.size();i++)
//    {
//        QSqlQuery ("drop table " + listophtaquery.value(0).toString(), db);
//        listophtaquery.next();
//    }
//    QSqlQuery ("use " NOM_BASE_COMPTA, db);
//    QSqlQuery listcomptaquery(req,db);
//    listcomptaquery.first();
//    for (int i=0; i<listcomptaquery.size();i++)
//    {
//        QSqlQuery ("drop table " + listcomptaquery.value(0).toString(), db);
//        listcomptaquery.next();
//    }
//    QSqlQuery ("use " NOM_BASE_CONSULTS, db);
//    QSqlQuery listtablquery(req,db);
//    listtablquery.first();
//    for (int i=0; i<listtablquery.size();i++)
//    {
//        QSqlQuery ("drop table " + listtablquery.value(0).toString(), db);
//        listtablquery.next();
//    }
}

QString Procedures::PrefixePlus(QString Dioptr)                          // convertit en QString signé + ou - les valeurs de dioptries issues des appareils de mesure
{
    double i = Dioptr.toDouble();
    if (Dioptr != "")
        return (i>0 ?
                    "+" + QLocale().toString(Dioptr.toDouble(),'f',2)
                    :
                    QLocale().toString(Dioptr.toDouble(),'f',2)
                    );
    else
        return "";
}

/* ------------------------------------------------------------------------------------------------------------------------------------------
     AVEC OU SANS COMPTA -------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
void Procedures::setAvecCompta(int compta)
{
    avecLaCompta = compta;
}

void Procedures::getAvecCompta()
{
    //QSqlQuery quer ("select sanscompta from " NOM_TABLE_PARAMSYSTEME, getDataBase());
    //quer.first();
    //avecLaCompta = quer.value(0).toInt();
}
int Procedures::AvecCompta()
{
    return avecLaCompta;
}

/* ------------------------------------------------------------------------------------------------------------------------------------------


GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Ports_Series()
{
    QString NomPort;
    gPortRefracteur             = "";
    gPortFronto                 = "";
    gPortAutoref                = "";
    gPortTono                   = "";
    lPortRefracteur             = 0;
    lPortFronto                 = 0;
    lPortAutoref                = 0;
    lPortTono                   = 0;
    gFrontoParametre            = (gsettingsIni->value("Param_Poste/Fronto").toString() != "-"
                                && gsettingsIni->value("Param_Poste/Fronto").toString() != ""
                                && gsettingsIni->value("Param_Poste/PortFronto").toString() != "Box");
    gAutorefParametre           = (gsettingsIni->value("Param_Poste/Autoref").toString() != "-"
                                && gsettingsIni->value("Param_Poste/Autoref").toString() != ""
                                && gsettingsIni->value("Param_Poste/PortAutoref").toString() != "Box");
    gRefracteurParametre        = (gsettingsIni->value("Param_Poste/Refracteur").toString() != "-"
                                && gsettingsIni->value("Param_Poste/Refracteur").toString() != "");
    gTonoParametre              = (gsettingsIni->value("Param_Poste/Tonometre").toString() != "-"
                                && gsettingsIni->value("Param_Poste/Tonometre").toString() != "");

    if (gAutorefParametre || gRefracteurParametre || gFrontoParametre || gTonoParametre)
    {
        bool portseriedispo = false;
        for (int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
        {
            //qDebug() << QSerialPortInfo::availablePorts().at(i).portName();
            //qDebug() << QSerialPortInfo::availablePorts().at(i).serialNumber();
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
            {
                portseriedispo = true;
                break;
            }
        }
        if (!portseriedispo)
        {
            UpMessageBox::Watch(0, tr("Erreur connexion série"),
                                   tr("Des connexions série sont paramétrées pour certains appareils du poste de réfraction.\n"
                                      "Malheureusement, aucune de ces connexions ne semble fonctionner."));
            return false;
        }
    }
    // PORT FRONTO
    if (gFrontoParametre)
    {
        gPortFronto     = gsettingsIni->value("Param_Poste/PortFronto").toString();
        bool a          = ReglePortFronto();
        a               = (gPortFronto != "");
        if (!a)
            UpMessageBox::Watch(0, tr("Erreur connexion frontofocomètre"));
        if (gPortFronto == "COM1") NomPort = "A";
        else if (gPortFronto == "COM2") NomPort = "B";
        else if (gPortFronto == "COM3") NomPort = "C";
        else if (gPortFronto == "COM4") NomPort = "D";
        else NomPort ="";

        if (NomPort != "")
        {
            lPortFronto     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                //qDebug() << QSerialPortInfo::availablePorts().at(i).serialNumber();
                //UpMessageBox::Watch(this,QSerialPortInfo::availablePorts().at(i).portName());
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial")
                    || QSerialPortInfo::availablePorts().at(i).portName().contains("KeySerial"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort)
                    {
                        lPortFronto->setPort(QSerialPortInfo::availablePorts().at(i));
                        lPortFronto->setBaudRate(ParamPortSerieFronto.baudRate);
                        lPortFronto->setFlowControl(ParamPortSerieFronto.flowControl);
                        lPortFronto->setParity(ParamPortSerieFronto.parity);
                        lPortFronto->setDataBits(ParamPortSerieFronto.dataBits);
                        lPortFronto->setStopBits(ParamPortSerieFronto.stopBits);
                        break;
                    }
                }
            }
            if (lPortFronto->open(QIODevice::ReadWrite))
            {
                ThreadFronto = new SerialThread(lPortFronto);
                ThreadFronto->transaction();
                connect(ThreadFronto,  SIGNAL(reponse(QString)),     this, SLOT(Slot_ReponsePortSerie_Fronto(QString)));
            }
            else
                lPortFronto = 0;
        }
    }

    // PORT REFRACTEUR
    if (gRefracteurParametre)
    {
        gPortRefracteur = gsettingsIni->value("Param_Poste/PortRefracteur").toString();
        bool a          = ReglePortRefracteur();
        a               = (gPortRefracteur != "");
        if (!a)
            UpMessageBox::Watch(0, tr("Erreur connexion refracteur"));
        if (gPortRefracteur == "COM1") NomPort = "A";
        else if (gPortRefracteur == "COM2") NomPort = "B";
        else if (gPortRefracteur == "COM3") NomPort = "C";
        else if (gPortRefracteur == "COM4") NomPort = "D";
        else NomPort ="";
        if (NomPort != "")
        {
            lPortRefracteur     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                //UpMessageBox::Watch(this,QSerialPortInfo::availablePorts().at(i).portName());
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial")
                    || QSerialPortInfo::availablePorts().at(i).portName().contains("KeySerial"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort)
                    {
                        lPortRefracteur->setPort(QSerialPortInfo::availablePorts().at(i));
                        lPortRefracteur->setBaudRate(ParamPortSerieRefracteur.baudRate);
                        lPortRefracteur->setFlowControl(ParamPortSerieRefracteur.flowControl);
                        lPortRefracteur->setParity(ParamPortSerieRefracteur.parity);
                        lPortRefracteur->setDataBits(ParamPortSerieRefracteur.dataBits);
                        lPortRefracteur->setStopBits(ParamPortSerieRefracteur.stopBits);
                        break;
                    }
                }
            }
            if (lPortRefracteur->open(QIODevice::ReadWrite))
            {
                ThreadRefracteur     = new SerialThread(lPortRefracteur);
                ThreadRefracteur->transaction();
                connect(ThreadRefracteur,  SIGNAL(reponse(QString)),     this, SLOT(Slot_ReponsePortSerie_Refracteur(QString)));
            }
            else
                lPortRefracteur = 0;
        }
    }

    //PORT AUTOREF
    if (gAutorefParametre)
    {
        gPortAutoref    = gsettingsIni->value("Param_Poste/PortAutoref").toString();
        bool a          = ReglePortAutoref();
        a               = (gPortAutoref != "");
        if (!a)
            UpMessageBox::Watch(0, tr("Erreur connexion autorefractomètre"));
        if (gPortAutoref == "COM1") NomPort = "A";
        else if (gPortAutoref == "COM2") NomPort = "B";
        else if (gPortAutoref == "COM3") NomPort = "C";
        else if (gPortAutoref == "COM4") NomPort = "D";
        else NomPort ="";
        if (NomPort != "")
        {
            lPortAutoref     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                //UpMessageBox::Watch(this,QSerialPortInfo::availablePorts().at(i).portName());
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial")
                    || QSerialPortInfo::availablePorts().at(i).portName().contains("KeySerial"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort)
                    {
                        lPortAutoref->setPort(QSerialPortInfo::availablePorts().at(i));
                        lPortAutoref->setBaudRate(ParamPortSerieAutoref.baudRate);
                        lPortAutoref->setFlowControl(ParamPortSerieAutoref.flowControl);
                        lPortAutoref->setParity(ParamPortSerieAutoref.parity);
                        lPortAutoref->setDataBits(ParamPortSerieAutoref.dataBits);
                        lPortAutoref->setStopBits(ParamPortSerieAutoref.stopBits);
                        break;
                    }
                }
            }
            if (lPortAutoref->open(QIODevice::ReadWrite))
            {
                ThreadAutoref     = new SerialThread(lPortAutoref);
                ThreadAutoref->transaction();
                connect(ThreadAutoref,  SIGNAL(reponse(QString)),     this, SLOT(Slot_ReponsePortSerie_Autoref(QString)));
            }
            else
                lPortAutoref = 0;
        }
    }
    if (gTonoParametre)
    {
        gPortTono       = gsettingsIni->value("Param_Poste/PortTonometre").toString();
    }
    return false;
}

void Procedures::ClearMesures()
{
    MesureFronto                .clear();
    MesureAutoref               .clear();
    MesureKerato                .clear();
    MesureTono                  .clear();
    MesurePachy                 .clear();
    MesureRefracteurSubjectif   .clear();
    MesureRefracteurFinal       .clear();
}

void Procedures::ClearHtmlMesures()
{
    HtmlMesureFronto                .clear();
    HtmlMesureAutoref               .clear();
    HtmlMesureKerato                .clear();
    HtmlMesureTono                  .clear();
    HtmlMesurePachy                 .clear();
    HtmlMesureRefracteurSubjectif   .clear();
    HtmlMesureRefracteurFinal       .clear();
}

bool Procedures::ReglePortRefracteur()
{
    bool a = true;
    if (gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
        ParamPortSerieRefracteur.baudRate       = QSerialPort::Baud2400;
        ParamPortSerieRefracteur.dataBits       = QSerialPort::Data7;
        ParamPortSerieRefracteur.parity         = QSerialPort::EvenParity;
        ParamPortSerieRefracteur.stopBits       = QSerialPort::TwoStop;
        ParamPortSerieRefracteur.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    return a;
}

QSerialPort* Procedures::PortRefracteur()
{
    return lPortRefracteur;
}

//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série du refracteur
//-----------------------------------------------------------------------------------------
void Procedures::Slot_ReponsePortSerie_Refracteur(const QString &s)
{
    //qDebug() << s;
    gMesureSerie        = s;

    QString OKPourRecevoir ("");
    if (gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
        QByteArray DTRbuff;
        DTRbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
        DTRbuff.append("CRL");                              //CRL
        DTRbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
        DTRbuff.append("SD");                               //SD
        DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
        DTRbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
        OKPourRecevoir = DTRbuff+"\r";
        //OKPourRecevoir = "\001CRL\002SD\027\004\r";
        if (gMesureSerie == OKPourRecevoir)
        {
            RegleRefracteur();
            return;
        }
    }
    setDonneesRefracteur(gMesureSerie);
    if (MesureAutoref.isEmpty()
        && MesureFronto.isEmpty()
        && MesureKerato.isEmpty()
        && MesurePachy.isEmpty()
        && MesureTono.isEmpty()
        && MesureRefracteurSubjectif.isEmpty()
        && MesureRefracteurFinal.isEmpty())
    {
        UpMessageBox::Watch(0,tr("pas de données reçues du refracteur"));
        return;
    }
    setTypeMesureRefraction("Refracteur");
    setHtmlRefracteur();
//    if (MesureRefracteurFinal.isEmpty())
//    {
//        MesureRefracteurFinal = MesureRefracteurSubjectif;
//        MesureRefracteurFinal["Type"] = "Final";
//    }
    emit NouvMesureRefraction();
}

void Procedures::RegleRefracteur()
{
    QMap<QString,QVariant>  Mesure;
    if (gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100"
     || gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
        QString AxeOD, AxeOG;
        QString AddOD, AddOG;
        QString SCAOD, SCAOG;
        QString DataAEnvoyer;
        QByteArray DTRbuff;
        DTRbuff.append(QByteArray::fromHex("O1"));          //SOH -> start of header
        if (!MesureAutoref.isEmpty())
        {
            Mesure = MesureAutoref;
            MesureAutoref.clear();
            if (Mesure["AxeOD"].toInt()<10)
                AxeOD = "  " + QString::number(Mesure["AxeOD"].toInt());
            else if (Mesure["AxeOD"].toInt()<100)
                AxeOD = " " + QString::number(Mesure["AxeOD"].toInt());
            else
                AxeOD = Mesure["AxeOD"].toString();
            if (Mesure["AxeOG"].toInt()<10)
                AxeOG = "  " + QString::number(Mesure["AxeOG"].toInt());
            else if (Mesure["AxeOG"].toInt()<100)
                AxeOG = " " + QString::number(Mesure["AxeOG"].toInt());
            else
                AxeOG = Mesure["AxeOG"].toString();

            SCAOD       = Mesure["SphereOD"].toString() + Mesure["CylOD"].toString() + AxeOD;
            SCAOG       = Mesure["SphereOG"].toString() + Mesure["CylOG"].toString() + AxeOG;
            SCAOD.replace("+0","+ ");
            SCAOD.replace("-0","- ");
            SCAOG.replace("+0","+ ");
            SCAOG.replace("-0","- ");
            DTRbuff.append("DRM");                               //section autoref
            DTRbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
            DTRbuff.append("OR"+ SCAOD);                        //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append("OL"+ SCAOG);                        //SD
            if (Mesure["PD"].toString()!="")
            {
                DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
                DTRbuff.append("PD"+ Mesure["PD"].toString());                        //SD
            }
            //qDebug() << "SCAOD = " << SCAOD << "SCAOG = " << SCAOG;
        }
        if (!MesureFronto.isEmpty())
        {
            Mesure = MesureFronto;
            MesureFronto.clear();
            if (Mesure["AxeOD"].toInt()<10)
                AxeOD = "  " + QString::number(Mesure["AxeOD"].toInt());
            else if (Mesure["AxeOD"].toInt()<100)
                AxeOD = " " + QString::number(Mesure["AxeOD"].toInt());
            else
                AxeOD = Mesure["AxeOD"].toString();
            if (Mesure["AxeOG"].toInt()<10)
                AxeOG = "  " + QString::number(Mesure["AxeOG"].toInt());
            else if (Mesure["AxeOG"].toInt()<100)
                AxeOG = " " + QString::number(Mesure["AxeOG"].toInt());
            else
                AxeOG = Mesure["AxeOG"].toString();

            SCAOD       = Mesure["SphereOD"].toString() + Mesure["CylOD"].toString() + AxeOD;
            SCAOG       = Mesure["SphereOG"].toString() + Mesure["CylOG"].toString() + AxeOG;
            SCAOD.replace("+0","+ ");
            SCAOD.replace("-0","- ");
            SCAOG.replace("+0","+ ");
            SCAOG.replace("-0","- ");
            AddOD       = "+ " + QString::number(Mesure["AddOD"].toDouble(),'f',2);
            AddOG       = "+ " + QString::number(Mesure["AddOG"].toDouble(),'f',2);
            DTRbuff.append("DLM");                               //section fronto
            DTRbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
            DTRbuff.append(" R"+ SCAOD);                        //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append(" L"+ SCAOG);                        //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append("AR" + AddOD);                       //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append("AL" + AddOG);                       //SD
            if (Mesure["PD"].toString()!="")
            {
                DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
                DTRbuff.append("PD"+ Mesure["PD"].toString());                        //SD
            }
        }
        DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
        DTRbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
        DataAEnvoyer = DTRbuff;
        QByteArray Data = DataAEnvoyer.toLocal8Bit();
        PortRefracteur()->clear();
        PortRefracteur()->write(Data);
        PortRefracteur()->waitForBytesWritten(1000);
        //qDebug() << QString(Data);
    }
}

void Procedures::SetDataAEnvoyerAuRefracteur(QMap<QString, QVariant> DataFronto, QMap<QString, QVariant> DataAutoref)
{
//    qDebug() << "Fronto" << DataFronto["SphereOD"].toString() << DataFronto["CylOD"].toString() << DataFronto["AxeOD"].toString() << DataFronto["AddOD"].toString()
//            << DataFronto["SphereOG"].toString() << DataFronto["CylOG"].toString() << DataFronto["AxeOG"].toString() << DataFronto["AddOG"].toString();
//    qDebug() << "Autoref" << DataAutoref["SphereOD"].toString() << DataAutoref["CylOD"].toString() << DataAutoref["AxeOD"].toString() << DataAutoref["AddOD"].toString()
//            << DataAutoref["SphereOG"].toString() << DataAutoref["CylOG"].toString() << DataAutoref["AxeOG"].toString() << DataAutoref["AddOG"].toString();
    ClearMesures();
    MesureAutoref       = DataAutoref;
    MesureFronto        = DataFronto;
    NouvMesureFronto    = false;
    NouvMesureAutoref   = false;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (ThreadRefracteur!=NULL)
    {
        // NIDEK RT-5100
        if (gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            //Dans un premier temps, le PC envoie la séquence SOH puis "C**" puis STX puis "RS" puis ETB puis EOT
            QString ReqPourEnvoyer ("");
            QByteArray DTSbuff;
            DTSbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
            DTSbuff.append("C**");                              //C**
            DTSbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
            DTSbuff.append("RS");                               //RS
            DTSbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
            DTSbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
            ReqPourEnvoyer = DTSbuff;
            QByteArray Data = ReqPourEnvoyer.toLocal8Bit();
            PortRefracteur()->clear();
            PortRefracteur()->write(Data);
            PortRefracteur()->waitForBytesWritten(100);
        }
    }
}

void Procedures::setDonneesRefracteur(QString Mesure)
{
    ClearMesures();
    ClearHtmlMesures();
    //Edit(Mesure);
    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString mAddOD      = "+0.00";
    QString mAddOG      = "+0.00";
    QString AVLOD(""), AVLOG(""), AVPOD(""), AVPOG("");
    QString PD          = "";
    int     idx;
    QString DioptrAstCornOD(""), DioptrAstCornOG("");

    QString mesureOD(""), mesureOG("");
    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    QString mTOOD(""), mTOOG("");

    // TRADUCTION DES DONNEES EN FONCTION DU REFRACTEUR
    // NIDEK RT-5100 - RT-2100 =======================================================================================================================================
    if (gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100"
     || gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
        /*
NIDEK RT-5100 ID             DA2016/12/30
@LM
 R+ 1.50- 1.25 15
 L+ 2.50- 1.00155
@RM
OR+ 1.25- 1.75170
OL+ 1.25- 0.50 20
PD57.530.527.0
@RT
fR- 1.50- 1.00 40
fL+ 2.50- 1.50135
nR+ 1.50- 1.00 40
nL+ 5.50- 1.50135
aR+ 3.00
aL+ 2.75
vR 1.25
vL 0.70
pD57.530.527.0
@NT
TR25.3
TL22.1     */

        // Données du FRONTO ---------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@LM"))                 //=> il y a une mesure pour le fronto
        {
            idx                     = Mesure.indexOf("@LM");
            QString SectionFronto   = Mesure.right(Mesure.length()-idx);
            //Edit(SectionFronto + "\nOK");
            // OEIL DROIT -----------------------------------------------------------------------------
            if (SectionFronto.contains(" R"))
            {
                mesureOD     = SectionFronto.mid(SectionFronto.indexOf(" R")+2,15)   .replace(" ","0");
                mSphereOD    = mesureOD.mid(0,6);
                mCylOD       = mesureOD.mid(6,6);
                mAxeOD       = mesureOD.mid(12,3);
                mAddOD       = SectionFronto.mid(SectionFronto.indexOf("AR")+2,6)    .replace(" ","0");
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionFronto.contains(" L"))
            {
                mesureOG                = SectionFronto.mid(SectionFronto.indexOf(" L")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
                mAddOG       = SectionFronto.mid(SectionFronto.indexOf("AL")+2,6)    .replace(" ","0");
            }
            MesureFronto.clear();
            MesureFronto["SphereOD"]    = mSphereOD;
            MesureFronto["CylOD"]       = mCylOD;
            MesureFronto["AxeOD"]       = mAxeOD;
            MesureFronto["AddOD"]       = mAddOD;
            MesureFronto["SphereOG"]    = mSphereOG;
            MesureFronto["CylOG"]       = mCylOG;
            MesureFronto["AxeOG"]       = mAxeOG;
            MesureFronto["AddOG"]       = mAddOG;
        }
        // Données de l'AUTOREF - REFRACTION et KERATOMETRIE ----------------------------------------------------------------------------------------------
        if (Mesure.contains("@RM"))                 //=> il y a une mesure de refractometrie
        {
            idx                     = Mesure.indexOf("@RM");
            QString SectionAutoref  = Mesure.right(Mesure.length()-idx);
            //Edit(SectionAutoref);
            // OEIL DROIT -----------------------------------------------------------------------------
            if (SectionAutoref.contains("OR"))
            {
                mesureOD     = SectionAutoref.mid(SectionAutoref.indexOf("OR")+2,15)   .replace(" ","0");
                mSphereOD    = mesureOD.mid(0,6);
                mCylOD       = mesureOD.mid(6,6);
                mAxeOD       = mesureOD.mid(12,3);
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionAutoref.contains("OL"))
            {
                mesureOG     = SectionAutoref.mid(SectionAutoref.indexOf("OL")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
            }
            MesureAutoref.clear();
            MesureAutoref["SphereOD"]    = mSphereOD;
            MesureAutoref["CylOD"]       = mCylOD;
            MesureAutoref["AxeOD"]       = mAxeOD;
            MesureAutoref["SphereOG"]    = mSphereOG;
            MesureAutoref["CylOG"]       = mCylOG;
            MesureAutoref["AxeOG"]       = mAxeOG;
        }
        if (Mesure.contains("@KM"))                 //=> il y a une mesure de keratométrie
        {
            MesureKerato.clear();
            idx                     = Mesure.indexOf("@KM");
            QString SectionKerato   = Mesure.right(Mesure.length()-idx);
            //Edit(SectionKerato + "\nOK");
            // OEIL DROIT -----------------------------------------------------------------------------
            if (SectionKerato.contains("CR"))
            {
                mesureOD            = SectionKerato.mid(SectionKerato.indexOf("CR")+2,13)   .replace(" ","0");
                K1OD                = mesureOD.mid(0,5);
                K2OD                = mesureOD.mid(5,5);
                AxeKOD              = mesureOD.mid(10,3).toInt();
                if (SectionKerato.contains("DR"))
                {
                    mesureOD            = SectionKerato.mid(SectionKerato.indexOf("DR")+2,10)   .replace(" ","0");
                    DioptrAstCornOD     = QString::number(mesureOD.mid(0,5).toDouble() - mesureOD.mid(5,5).toDouble(),'f',2);
                    MesureKerato["DioptrK1OD"]  = QString::number(mesureOD.mid(0,5).toDouble(),'f',2);
                    MesureKerato["DioptrK2OD"]  = QString::number(mesureOD.mid(5,5).toDouble(),'f',2);
                    MesureKerato["DioptrKOD"]   = DioptrAstCornOD;
                }
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionKerato.contains("CL"))
            {
                mesureOG            = SectionKerato.mid(SectionKerato.indexOf("CL")+2,13)   .replace(" ","0");
                K1OG                = mesureOG.mid(0,5);
                K2OG                = mesureOG.mid(5,5);
                AxeKOG              = mesureOG.mid(10,3).toInt();
                if (SectionKerato.contains("DL"))
                {
                    mesureOG            = SectionKerato.mid(SectionKerato.indexOf("DL")+2,10)   .replace(" ","0");
                    DioptrAstCornOG     = QString::number(mesureOG.mid(0,5).toDouble() - mesureOG.mid(5,5).toDouble(),'f',2);
                    MesureKerato["DioptrK1OG"]  = QString::number(mesureOG.mid(0,5).toDouble(),'f',2);
                    MesureKerato["DioptrK2OG"]  = QString::number(mesureOG.mid(5,5).toDouble(),'f',2);
                    MesureKerato["DioptrKOG"]   = DioptrAstCornOG;
                }
            }
            MesureKerato["K1OD"]        = K1OD;
            MesureKerato["K2OD"]        = K2OD;
            MesureKerato["AxeKOD"]      = AxeKOD;
            MesureKerato["K1OG"]        = K1OG;
            MesureKerato["K2OG"]        = K2OG;
            MesureKerato["AxeKOG"]      = AxeKOG;
        }
        // Données du REFRACTEUR --------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@RT"))                 //=> il y a une mesure de refraction
        {
            idx                         = Mesure.indexOf("@RT");
            QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);
            PD                          = SectionRefracteur.mid(SectionRefracteur.indexOf("pD")+2,2);
            //Edit(SectionRefracteur + "\nOK");

            // les données subjectives --------------------------------------------------------------------------------------------------------------
            // OEIL DROIT -----------------------------------------------------------------------------
            MesureRefracteurSubjectif.clear();
            MesureRefracteurSubjectif["PD"]          = PD;
            if (SectionRefracteur.contains("fR"))
                {
               mesureOD     = SectionRefracteur.mid(SectionRefracteur.indexOf("fR")+2,15)   .replace(" ","0");
                mSphereOD    = mesureOD.mid(0,6);
                mCylOD       = mesureOD.mid(6,6);
                mAxeOD       = mesureOD.mid(12,3);
                if (SectionRefracteur.indexOf("aR")>-1)
                    mAddOD   = SectionRefracteur.mid(SectionRefracteur.indexOf("aR")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("vR")>-1)
                    AVLOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("vR")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("yR")>-1)
                    AVPOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("yR")+2,5)    .replace(" ","0");
                MesureRefracteurSubjectif["SphereOD"]    = mSphereOD;
                MesureRefracteurSubjectif["CylOD"]       = mCylOD;
                MesureRefracteurSubjectif["AxeOD"]       = mAxeOD;
                MesureRefracteurSubjectif["AddOD"]       = mAddOD;
                MesureRefracteurSubjectif["AVLOD"]       = AVLOD;
                MesureRefracteurSubjectif["AVPOD"]       = AVPOD;
                MesureRefracteurSubjectif["Type"]        = "Subjectif";
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionRefracteur.contains("fL"))
            {
                mesureOG     = SectionRefracteur.mid(SectionRefracteur.indexOf("fL")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
                if (SectionRefracteur.indexOf("aL")>-1)
                    mAddOG   = SectionRefracteur.mid(SectionRefracteur.indexOf("aL")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("vL")>-1)
                    AVLOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("vL")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("yL")>-1)
                    AVPOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("yL")+2,5)    .replace(" ","0");
                MesureRefracteurSubjectif["SphereOG"]    = mSphereOG;
                MesureRefracteurSubjectif["CylOG"]       = mCylOG;
                MesureRefracteurSubjectif["AxeOG"]       = mAxeOG;
                MesureRefracteurSubjectif["AddOG"]       = mAddOG;
                MesureRefracteurSubjectif["AVLOG"]       = AVLOG;
                MesureRefracteurSubjectif["AVPOG"]       = AVPOG;
                MesureRefracteurSubjectif["Type"]        = "Subjectif";
            }

            // les données finales --------------------------------------------------------------------------------------------------------------
            // OEIL DROIT -----------------------------------------------------------------------------
            if (SectionRefracteur.contains("FR") || SectionRefracteur.contains("FL"))
                MesureRefracteurSubjectif.clear();
            if (SectionRefracteur.contains("FR"))
            {
                mesureOD     = SectionRefracteur.mid(SectionRefracteur.indexOf("FR")+2,15)   .replace(" ","0");
                mSphereOD    = mesureOD.mid(0,6);
                mCylOD       = mesureOD.mid(6,6);
                mAxeOD       = mesureOD.mid(12,3);
                if (SectionRefracteur.indexOf("AR")>-1)
                    mAddOD   = SectionRefracteur.mid(SectionRefracteur.indexOf("AR")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("VR")>-1)
                    AVLOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("VR")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("YR")>-1)
                    AVPOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("YR")+2,5)    .replace(" ","0");
                MesureRefracteurFinal["SphereOD"]    = mSphereOD;
                MesureRefracteurFinal["CylOD"]       = mCylOD;
                MesureRefracteurFinal["AxeOD"]       = mAxeOD;
                MesureRefracteurFinal["AddOD"]       = mAddOD;
                MesureRefracteurFinal["AVLOD"]       = AVLOD;
                MesureRefracteurFinal["AVPOD"]       = AVPOD;
                MesureRefracteurFinal["Type"]        = "Final";
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionRefracteur.contains("FR"))
            {
                mesureOG     = SectionRefracteur.mid(SectionRefracteur.indexOf("FL")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
                if (SectionRefracteur.indexOf("AL")>-1)
                    mAddOG   = SectionRefracteur.mid(SectionRefracteur.indexOf("AL")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("VL")>-1)
                    AVLOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("VL")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("YL")>-1)
                    AVPOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("YL")+2,5)    .replace(" ","0");
                MesureRefracteurFinal["SphereOG"]    = mSphereOG;
                MesureRefracteurFinal["CylOG"]       = mCylOG;
                MesureRefracteurFinal["AxeOG"]       = mAxeOG;
                MesureRefracteurFinal["AddOG"]       = mAddOG;
                MesureRefracteurFinal["AVLOG"]       = AVLOG;
                MesureRefracteurFinal["AVPOG"]       = AVPOG;
                MesureRefracteurFinal["Type"]        = "Final";
            }
        }
        // Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@NT"))                 //=> il y a une mesure de tonometrie
        {
            MesureTono.clear();
            idx                     = Mesure.indexOf("@NT");
            QString SectionTono     = Mesure.right(Mesure.length()-idx-5);
            SectionTono             = SectionTono.left(SectionTono.indexOf("@"));
            //Edit(SectionTono+ "\nOK");
            // OEIL DROIT -----------------------------------------------------------------------------
            mTOOD                   = SectionTono.mid(SectionTono.indexOf("TR")+2,4)   .replace(" ","0");
            // OEIL GAUCHE ---------------------------------------------------------------------------
            mTOOG                   = SectionTono.mid(SectionTono.indexOf("TL")+2,4)   .replace(" ","0");
            MesureTono["TOOD"]      = mTOOD;
            MesureTono["TOOG"]      = mTOOG;
        }
    }
    // FIN NIDEK RT-5100 et RT 2100 ==========================================================================================================================
}

QMap<QString,QVariant>  Procedures::DonneesRefracteurSubj()
{
    return MesureRefracteurSubjectif;
}

QMap<QString,QVariant>  Procedures::DonneesRefracteurFin()
{
    return MesureRefracteurFinal;
}

// -------------------------------------------------------------------------------------
// Generation du resumé des données issues du refracteur
//--------------------------------------------------------------------------------------
void Procedures::setHtmlRefracteur()
{
   // CALCUL DE HtmlMesureFronto ====================================================================================================================================
    if (!MesureFronto.isEmpty() && NouvMesureFronto)
        setHtmlFronto();
    // CALCUL DE HtmlMesureAutoref ===================================================================================================================================
    if (!MesureAutoref.isEmpty() && NouvMesureAutoref)
        setHtmlAutoref();
    // CALCUL DE HtmlMesureKerato ====================================================================================================================================
    if (!MesureKerato.isEmpty())
        setHtmlKerato(MesureKerato);
    // CALCUL DE HtmlMesureRefracteurSubjectif =======================================================================================================================
    QString Resultat = "";
    if(!MesureRefracteurSubjectif.isEmpty())
    {
        // - 1 - détermination des verres
        QString mSphereOD   = PrefixePlus(MesureRefracteurSubjectif["SphereOD"].toString());
        QString mCylOD      = PrefixePlus(MesureRefracteurSubjectif["CylOD"].toString());
        QString mAxeOD      = QString::number(MesureRefracteurSubjectif["AxeOD"].toInt());
        QString mAddOD      = PrefixePlus(MesureRefracteurSubjectif["AddOD"].toString());
        QString mAVLOD ("");
        if (MesureRefracteurSubjectif["AVLOD"].toDouble()>0)
            mAVLOD      = QLocale().toString(MesureRefracteurSubjectif["AVLOD"].toDouble()*10) + "/10";
        QString mAVPOD      = MesureRefracteurSubjectif["AVPOD"].toString();
        QString mSphereOG   = PrefixePlus(MesureRefracteurSubjectif["SphereOG"].toString());
        QString mCylOG      = PrefixePlus(MesureRefracteurSubjectif["CylOG"].toString());
        QString mAxeOG      = QString::number(MesureRefracteurSubjectif["AxeOG"].toInt());
        QString mAddOG      = PrefixePlus(MesureRefracteurSubjectif["AddOG"].toString());
        QString mAVLOG ("");
        if (MesureRefracteurSubjectif["AVLOG"].toDouble()>0)
            mAVLOG      = QLocale().toString(MesureRefracteurSubjectif["AVLOG"].toDouble()*10) + "/10";
        QString mAVPOG      = MesureRefracteurSubjectif["AVPOG"].toString();
        QString ResultatVLOD, ResultatVLOG,ResultatVPOD, ResultatVPOG, ResultatOD, ResultatOG;

        // détermination OD
        if (QLocale().toDouble(mCylOD) != 0 && QLocale().toDouble(mSphereOD) != 0)
            ResultatVLOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
        else if (QLocale().toDouble(mCylOD) == 0 && QLocale().toDouble(mSphereOD) != 0)
            ResultatVLOD = mSphereOD;
        else if (QLocale().toDouble(mCylOD) != 0 && QLocale().toDouble(mSphereOD) == 0)
            ResultatVLOD = mCylOD + tr(" à ") + mAxeOD + "°";
        else
            ResultatVLOD = tr("plan");

        if (QLocale().toDouble(mAddOD) > 0)
            ResultatVPOD = mAddOD;
        else
            ResultatVPOD = tr("plan");

        ResultatOD = ResultatVLOD + " add." + ResultatVPOD + " VP";
        if (mAxeOD == "")
            ResultatOD = "Rien";

        // détermination OG
        if (QLocale().toDouble(mCylOG) != 0 && QLocale().toDouble(mSphereOG) != 0)
            ResultatVLOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + ")";
        else if (QLocale().toDouble(mCylOG) == 0 && QLocale().toDouble(mSphereOG) != 0)
            ResultatVLOG = mSphereOG;
        else if (QLocale().toDouble(mCylOG) != 0 && QLocale().toDouble(mSphereOG) == 0)
            ResultatVLOG = mCylOG + tr(" à ") + mAxeOG ;
        else
            ResultatVLOG = tr("plan");

        if (QLocale().toDouble(mAddOG) > 0)
            ResultatVPOG = mAddOG;
        else
            ResultatVPOG = tr("plan");

        ResultatOG = ResultatVLOG + " add." + ResultatVPOG + " VP";
        if (mAxeOG == "")
            ResultatOG = "Rien";

        QString colorVLOD, colorVPOD, colorVLOG, colorVPOG;
        colorVPOG = "\"blue\"";
        colorVPOD = "\"blue\"";
        colorVLOG = "\"blue\"";
        colorVLOD = "\"blue\"";
        int av = 0;
        if (mAVLOD.contains("/"))
            av = mAVLOD.left(mAVLOD.indexOf("/")).toInt();
        if (av < 6)
            colorVLOD =  "\"red\"";
        if (av > 5 && av < 9)
            colorVLOD =  "\"orange\"";
        av = 0;
        if (mAVLOG.contains("/"))
            av = mAVLOG.left(mAVLOG.indexOf("/")).toInt();
        if (av < 6)
            colorVLOG =  "\"red\"";
        if (av >5 && av < 9)
            colorVLOG =  "\"orange\"";
        if (mAVPOD.replace(",",".").toInt() > 4 || mAVPOD.contains("<"))
            colorVPOD =  "\"red\"";
        else if (mAVPOD.replace(",",".").toInt() > 2)
            colorVPOD =  "\"orange\"";
        if (mAVPOG.replace(",",".").toInt() > 4 || mAVPOG.contains("<"))
            colorVPOG =  "\"red\"";
        else if (mAVPOG.replace(",",".").toInt() > 2)
            colorVPOG =  "\"orange\"";

        mAVPOD = (mAVPOD==""? "" : "> P" + mAVPOD.replace("<","&lt;"));
        mAVPOG = (mAVPOG==""? "" : "> P" + mAVPOG.replace("<","&lt;"));

        // Détermination de Resultat
        if (MesureRefracteurSubjectif["AddOD"].toString()!="" || MesureRefracteurSubjectif["AddOG"].toString()!="")  // il y a eu mesure de près et de loin
        {
            if (ResultatOD != "Rien" && QLocale().toDouble(mAddOD) == 0  && ResultatOG == "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " " + tr("OD");

            if (Resultat == "" && ResultatOD != "Rien" && (QLocale().toDouble(mAddOD) == 0 && QLocale().toDouble(mAddOG) == 0) && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " "+ tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG") + "</td>";

            if (Resultat == "" && ResultatOD != "Rien" && (QLocale().toDouble(mAddOD) == 0 && QLocale().toDouble(mAddOG) > 0) && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP "+ tr("OG") + "</td>";

            if (Resultat == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0  && ResultatOG == "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD");

            if (Resultat == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0 && QLocale().toDouble(mAddOG) == 0 && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG") + "</td>";

            if (Resultat == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0 && QLocale().toDouble(mAddOG) > 0 && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP " + tr("OG") + "</td>";

            if (Resultat == "" && (ResultatOD == "Rien" &&  QLocale().toDouble(mAddOG) == 0) && ResultatOG != "Rien")
                Resultat = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG");

            if (Resultat == "" && (ResultatOD == "Rien" &&  QLocale().toDouble(mAddOG) > 0) && ResultatOG != "Rien")
                Resultat = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP " + tr("OG");
        }
        else
        {
            if (Resultat == "" && ResultatOD != "Rien" && ResultatOG == "Rien")
                Resultat = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + mAVLOD + "</b></font> " + tr("OD");

            if (Resultat == "" && ResultatOD != "Rien" && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + mAVLOD + "</b></font> " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<font color = " + colorVLOG + "><b>" + mAVLOG + "</b></font> " + tr("OG") + "</td>";

            if (Resultat == "" && ResultatOD == "Rien" && ResultatOG != "Rien")
                Resultat = ResultatVLOG + "<font color = " + colorVLOG + "><b>" + mAVLOG + "</b></font> " + tr("OG");
        }
        Resultat = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Resultat + "</td><td width=\"70\"><font color = \"red\"></font></td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";
    }
    HtmlMesureRefracteurSubjectif = Resultat;
    // CALCUL DE HtmlMesureTono ======================================================================================================================================
    if (!MesureTono.isEmpty())
        setHtmlTono();
}

QString Procedures::HtmlRefracteur()
{
    if (! MesureRefracteurFinal.isEmpty() )
        return HtmlMesureFronto + HtmlMesureAutoref + HtmlMesureKerato + HtmlMesureRefracteurSubjectif + HtmlMesureTono;
    else
        return HtmlMesureFronto + HtmlMesureAutoref + HtmlMesureKerato + HtmlMesureTono;
}

bool Procedures::ReglePortFronto()
{
    bool a = true;
    if (gsettingsIni->value("Param_Poste/Fronto").toString()=="TOMEY TL-3000C")
    {
        ParamPortSerieFronto.baudRate       = QSerialPort::Baud2400;
        ParamPortSerieFronto.dataBits       = QSerialPort::Data7;
        ParamPortSerieFronto.parity         = QSerialPort::EvenParity;
        ParamPortSerieFronto.stopBits       = QSerialPort::OneStop;
        ParamPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (gsettingsIni->value("Param_Poste/Fronto").toString()=="VISIONIX VL1000"
          || gsettingsIni->value("Param_Poste/Fronto").toString()=="HUVITZ CLM7000")
    {
        ParamPortSerieFronto.baudRate       = QSerialPort::Baud9600;
        ParamPortSerieFronto.dataBits       = QSerialPort::Data7;
        ParamPortSerieFronto.parity         = QSerialPort::EvenParity;
        ParamPortSerieFronto.stopBits       = QSerialPort::OneStop;
        ParamPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800P"
          || gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800PD"
          || gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-500")
    {
        ParamPortSerieFronto.baudRate       = QSerialPort::Baud9600;
        ParamPortSerieFronto.dataBits       = QSerialPort::Data8;
        ParamPortSerieFronto.parity         = QSerialPort::EvenParity;
        ParamPortSerieFronto.stopBits       = QSerialPort::OneStop;
        ParamPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    return a;
}

QSerialPort *Procedures::PortFronto()
{
    return lPortFronto;
}

// lire les ports séries
//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série du Fronto
//-----------------------------------------------------------------------------------------
void Procedures::Slot_ReponsePortSerie_Fronto(const QString &s)
{
    gMesureSerie        = s;
    QString OKPourRecevoir ("");
    if (gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800P"
     || gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800PD"
     || gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-500")
    {
        QByteArray DTRbuff;
        DTRbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
        DTRbuff.append("C**");                              //C**^$
        DTRbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
        DTRbuff.append("RS");                               //RS
        DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
        DTRbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
        OKPourRecevoir = DTRbuff;
        if (gMesureSerie == OKPourRecevoir)
        {
            //Dans un premier temps, le PC envoie la séquence SOH puis "C**" puis STX puis "RS" puis ETB puis EOT
            QString ReqPourEnvoyer ("");
            QByteArray DTSbuff;
            DTSbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
            DTSbuff.append("CLM");                              //CLM^$
            DTSbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
            DTSbuff.append("SD");                               //SD
            DTSbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
            DTSbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
            ReqPourEnvoyer = DTSbuff;
            QByteArray Data = ReqPourEnvoyer.toLocal8Bit();
            PortFronto()->clear();
            PortFronto()->write(Data);
            PortFronto()->waitForBytesWritten(100);
            //QString dat = Data; qDebug() << "OK Fronto " + dat;
            return;
        }
    }
    //qDebug() << gMesureSerie;
    setDonneesFronto(gMesureSerie);
    NouvMesureFronto    = true;
    if (MesureFronto.isEmpty())
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (ThreadRefracteur!=NULL && !FicheRefractionOuverte())
    {
        // NIDEK RT-5100
        if (gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            //Dans un premier temps, le PC envoie la séquence SOH puis "C**" puis STX puis "RS" puis ETB puis EOT
            QString ReqPourEnvoyer ("");
            QByteArray DTSbuff;
            DTSbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
            DTSbuff.append("C**");                              //C**^$
            DTSbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
            DTSbuff.append("RS");                               //RS
            DTSbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
            DTSbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
            ReqPourEnvoyer = DTSbuff;
            QByteArray Data = ReqPourEnvoyer.toLocal8Bit();
            PortRefracteur()->clear();
            PortRefracteur()->write(Data);
            PortRefracteur()->waitForBytesWritten(100);
        }
    }
    setHtmlFronto();
    setTypeMesureRefraction("Fronto");
    emit NouvMesureRefraction();
}

void Procedures::setDonneesFronto(QString Mesure)
{
    ClearMesures();
    ClearHtmlMesures();
    //Edit(Mesure);
    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString mAddOD      = "+0.00";
    QString mAddOG      = "+0.00";
    QString mesureOD, mesureOG;

    //A - AFFICHER LA MESURE --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (gsettingsIni->value("Param_Poste/Fronto").toString()=="TOMEY TL-3000C")
    {
        /* Le fichier de sortie ressemble à ça
        .
        .LM
        .LR- 3.50- 1.25120
        .AR
        .PR
        .DR
        .LL- 4.50- 0.75 77
        .AL
        .PL
        .DL
        .
        */
        mesureOD            = Mesure.mid(Mesure.indexOf("LR")+2,15)   .replace(" ","0");
        mesureOG            = Mesure.mid(Mesure.indexOf("LL")+2,15)   .replace(" ","0");
        // OEIL DROIT -----------------------------------------------------------------------------
        mSphereOD            = mesureOD.mid(0,6);
        mCylOD               = mesureOD.mid(6,6);
        mAxeOD               = mesureOD.mid(12,3);
        if (Mesure.indexOf("AR")>0)
            mAddOD           = PrefixePlus(Mesure.mid(Mesure.indexOf("AR")+2,4));
        // OEIL GAUCHE ---------------------------------------------------------------------------
        mSphereOG            = mesureOG.mid(0,6);
        mCylOG               = mesureOG.mid(6,6);
        mAxeOG               = mesureOG.mid(12,3);
        if (Mesure.indexOf("AL")>0)
            mAddOG           = PrefixePlus(Mesure.mid(Mesure.indexOf("AL")+2,4));
        //les autres champs ne sont pas utilisés pour le moment -------------------------------
    }
    else if (gsettingsIni->value("Param_Poste/Fronto").toString()=="VISIONIX VL1000"
          || gsettingsIni->value("Param_Poste/Fronto").toString()=="HUVITZ CLM7000")
    {
        /* Le fichier de sortie ressemble à ça
            LM2RK   No=00036   R: S=-04.50 C=-00.50 A=103 PX=+00.25 PY=+04.00 PD=00.0 ADD=+2.00 UR=  0   L: S=-05.00 C=-00.50 A=110 PX=+00.00 PY=+05.50 PD=00.0 ADD=+5.00 UL=  0   E$
            */
        mesureOD            = Mesure.mid(Mesure.indexOf("R: ")+3);
        mesureOD            = mesureOD.left(mesureOD.indexOf("L: "));
        mesureOG            = Mesure.mid(Mesure.indexOf("L: ")+3);
        // OEIL DROIT -----------------------------------------------------------------------------
        mSphereOD            = mesureOD.mid(mesureOD.indexOf("S=")+2,6);
        mCylOD               = mesureOD.mid(mesureOD.indexOf("C=")+2,6);
        mAxeOD               = mesureOD.mid(mesureOD.indexOf("A=")+2,3);
        mAddOD               = mesureOD.mid(mesureOD.indexOf("ADD=")+4,5);
        // OEIL GAUCHE ---------------------------------------------------------------------------
        mSphereOG            = mesureOG.mid(mesureOG.indexOf("S=")+2,6);
        mCylOG               = mesureOG.mid(mesureOG.indexOf("C=")+2,6);
        mAxeOG               = mesureOG.mid(mesureOG.indexOf("A=")+2,3);
        mAddOG               = mesureOG.mid(mesureOG.indexOf("ADD=")+4,5);
        //les autres champs ne sont pas utilisés pour le moment -------------------------------
    }
    else if (gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800P"
          || gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800PD"
          || gsettingsIni->value("Param_Poste/Fronto").toString()=="NIDEK LM-500")
    {
        mesureOD            = Mesure.mid(Mesure.indexOf(" R")+2,15)   .replace(" ","0");
        mesureOG            = Mesure.mid(Mesure.indexOf(" L")+2,15)   .replace(" ","0");
        // OEIL DROIT -----------------------------------------------------------------------------
        mSphereOD            = mesureOD.mid(0,6);
        mCylOD               = mesureOD.mid(6,6);
        mAxeOD               = mesureOD.mid(12,3);
        if (Mesure.indexOf("AR")>0)
            mAddOD           = Mesure.mid(Mesure.indexOf("AR")+2,5);
        // OEIL GAUCHE ---------------------------------------------------------------------------
        mSphereOG            = mesureOG.mid(0,6);
        mCylOG               = mesureOG.mid(6,6);
        mAxeOG               = mesureOG.mid(12,3);
        if (Mesure.indexOf("AL")>0)
            mAddOG           = Mesure.mid(Mesure.indexOf("AL")+2,5);
    }
    MesureFronto.clear();
    MesureFronto["SphereOD"]    = mSphereOD;
    MesureFronto["CylOD"]       = mCylOD;
    MesureFronto["AxeOD"]       = mAxeOD;
    MesureFronto["AddOD"]       = mAddOD;
    MesureFronto["SphereOG"]    = mSphereOG;
    MesureFronto["CylOG"]       = mCylOG;
    MesureFronto["AxeOG"]       = mAxeOG;
    MesureFronto["AddOG"]       = mAddOG;
}

QMap<QString,QVariant>  Procedures::DonneesFronto()
{
    return MesureFronto;
}

// -------------------------------------------------------------------------------------
// Generation du resumé des données issues du frontocomètre
//--------------------------------------------------------------------------------------
void Procedures::setHtmlFronto()
{
    QString mSphereOD   = PrefixePlus(MesureFronto["SphereOD"].toString());
    QString mCylOD      = PrefixePlus(MesureFronto["CylOD"].toString());
    QString mAxeOD      = QString::number(MesureFronto["AxeOD"].toInt());
    QString mAddOD      = PrefixePlus(MesureFronto["AddOD"].toString());
    QString mSphereOG   = PrefixePlus(MesureFronto["SphereOG"].toString());
    QString mCylOG      = PrefixePlus(MesureFronto["CylOG"].toString());
    QString mAxeOG      = QString::number(MesureFronto["AxeOG"].toInt());
    QString mAddOG      = PrefixePlus(MesureFronto["AddOG"].toString());

    QString ResultatOD, ResultatVLOD, ResultatVPOD;
    QString ResultatOG, ResultatVLOG, ResultatVPOG;
    QString Resultat = "";

    HtmlMesureFronto = "";
    // QString TagAncre, numIDref; // 07-07-2014 08-08-2014

    // Génération du code html pour TAG Ancre avec ID refraction            // 07-07-2014 // 08-08-2014
    //numIDref = QString::number(gListeRefractionID.at(gListeRefractionID.size()-1));
    //TagAncre = "<a name=\"" + numIDref + "\"></a>" "<span ><a href=\""+ numIDref + "\" style=\"text-decoration:none\" style=\"color:#000000\"> ";
    // - 1 - détermination des verres
    // détermination OD
    ResultatVPOD = "";
    ResultatVLOD = "";
    if (QLocale().toDouble(mCylOD) != 0 && QLocale().toDouble(mSphereOD) != 0)
        ResultatVLOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°" + ")";
    else if (QLocale().toDouble(mCylOD) == 0 && QLocale().toDouble(mSphereOD) != 0)
        ResultatVLOD = mSphereOD;
    else if (QLocale().toDouble(mCylOD) != 0 && QLocale().toDouble(mSphereOD) == 0)
        ResultatVLOD = mCylOD + tr(" à ") + mAxeOD + "°" ;
    else
        ResultatVLOD = tr("plan");

    if (QLocale().toDouble(mAddOD)>0)
        ResultatVPOD = mAddOD;
    ResultatOD = (QLocale().toDouble(mAddOD)>0 ? ResultatVLOD + " add." + ResultatVPOD + " VP" :ResultatVLOD);

    // détermination OG
    ResultatVPOG = "";
    ResultatVLOG = "";
    if (QLocale().toDouble(mCylOG) != 0 && QLocale().toDouble(mSphereOG) != 0)
        ResultatVLOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + "°" + ")";
    else if (QLocale().toDouble(mCylOG) == 0 && QLocale().toDouble(mSphereOG) != 0)
        ResultatVLOG = mSphereOG;
    else if (QLocale().toDouble(mCylOG) != 0 && QLocale().toDouble(mSphereOG) == 0)
        ResultatVLOG = mCylOG + tr(" à ") + mAxeOG + "°" ;
    else
        ResultatVLOG = tr("plan");

    if (QLocale().toDouble(mAddOG)>0)
        ResultatVPOG = mAddOG;
    ResultatOG = (QLocale().toDouble(mAddOG)>0 ? ResultatVLOG + " add." + ResultatVPOG + " VP" :  ResultatVLOG);

    // Détermination de Resultat
    if (ResultatOD == ResultatOG)
    {
        if (QLocale().toDouble(mAddOD)>0)
            Resultat = ResultatOD + " " + tr("ODG");
        else
            Resultat = ResultatVLOD + " VL " + tr("ODG");
    }
    else
        Resultat = ResultatOD + " / " + ResultatOG;

    HtmlMesureFronto =  "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("Porte") + ":</b></font></td><td>" + Resultat + "</p>";
}

QString Procedures::HtmlFronto()
{
    return HtmlMesureFronto;
}

bool Procedures::ReglePortAutoref()
{
    bool a = true;
    if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1S"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-20"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")

    {
        ParamPortSerieAutoref.baudRate       = QSerialPort::Baud9600;
        ParamPortSerieAutoref.dataBits       = QSerialPort::Data8;
        ParamPortSerieAutoref.parity         = QSerialPort::EvenParity;
        ParamPortSerieAutoref.stopBits       = QSerialPort::OneStop;
        ParamPortSerieAutoref.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    return a;
}


QSerialPort* Procedures::PortAutoref()
{
    return lPortAutoref;
}

// lire les ports séries
//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série de l'autoref
//-----------------------------------------------------------------------------------------
void Procedures::Slot_ReponsePortSerie_Autoref(const QString &s)
{
    gMesureSerie        = s;
    //Edit(s);
    //qDebug() << gMesureSerie;

    if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1S"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-20")
    {
        QByteArray DTSbuff;
        DTSbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
        DTSbuff.append("C**");                              //C**
        DTSbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
        DTSbuff.append("RS");                               //RS
        DTSbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
        DTSbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
        if (gMesureSerie == QString(DTSbuff))               // Cas où l'autoref demande la permission d'envoyer
        {
            QString ReqPourEnvoyer ("");
            QByteArray DTSbuff;
            QString cmd;
            if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30")
                cmd ="CRK";
            else
                cmd="CRM";                          //CRK ou CRM suivant les appareils
            DTSbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
            DTSbuff.append("CRK");
            DTSbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
            DTSbuff.append("SD");                               //RS
            DTSbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
            DTSbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
            //DTSbuff.append(QByteArray::fromHex("D"));
            ReqPourEnvoyer = DTSbuff;
            QByteArray Data = ReqPourEnvoyer.toLocal8Bit();
            //QByteArray Data = "\u0001CRK\u0002SD\u0017\u0004";
            //qDebug() << "RS = " + gMesureSerie;
            //qDebug() << "SD = " + DTSbuff << Data;
//            PortAutoref()->setDataTerminalReady(true);
//            if (PortAutoref()->pinoutSignals().testFlag(QSerialPort::DataSetReadySignal))
//            {
                PortAutoref()->write(DTSbuff);                  // On répond à l'autoref OK pour recevoir et sortie de la procedure
                PortAutoref()->waitForBytesWritten(100);
                PortAutoref()->setDataTerminalReady(false);
//                bool e = true;
//                while (e) {
//                    e = (PortAutoref()->pinoutSignals().testFlag(QSerialPort::DataSetReadySignal));
//                }
                //qDebug() << "OK"<< gMesureSerie;
//            }
        }
    }
    //qDebug() << gMesureSerie;
    setDonneesAutoref(gMesureSerie);
    NouvMesureAutoref = true;
    if (MesureAutoref.isEmpty())
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (ThreadRefracteur!=NULL && !FicheRefractionOuverte())
    {
        // NIDEK RT-5100 - NIDEK RT-2100
        if (gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || gsettingsIni->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
             || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")
            {
                setHtmlKerato(MesureKerato);
                setTypeMesureRefraction("Kerato");
                emit NouvMesureRefraction();
            }
            if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")
            {
                setHtmlTono();
                setTypeMesureRefraction("Tono");
                emit NouvMesureRefraction();

                setHtmlPachy();
                setTypeMesureRefraction("Pachy");
                emit NouvMesureRefraction();
            }
            //Dans un premier temps, le PC envoie la séquence SOH puis "C**" puis STX puis "RS" puis ETB puis EOT
            QString ReqPourEnvoyer ("");
            QByteArray DTSbuff;
            DTSbuff.append(QByteArray::fromHex("1"));          //SOH -> start of header
            DTSbuff.append("C**");                              //C**
            DTSbuff.append(QByteArray::fromHex("2"));          //STX -> start of text
            DTSbuff.append("RS");                               //RS
            DTSbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
            DTSbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
            ReqPourEnvoyer = DTSbuff;
            QByteArray Data = ReqPourEnvoyer.toLocal8Bit();
            PortRefracteur()->clear();
            PortRefracteur()->write(Data);
            PortRefracteur()->waitForBytesWritten(100);
        }
    }
    setHtmlAutoref();
    setTypeMesureRefraction("Autoref");
    emit NouvMesureRefraction();
}

void Procedures::setDonneesAutoref(QString Mesure)
{
    ClearMesures();
    ClearHtmlMesures();
    //Edit(Mesure);
    MesureAutoref.clear();
    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString PD          = "";
    QString DioptrAstCornOD, DioptrAstCornOG;

    QString Ref(""), mesureOD(""), mesureOG("");
    QString K(""), KOD(""), KOG("");
    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    int     a(0);

    QString test;
    test =
    "DrmIDNIDEK/ARK-1a\n"
    "NO0009\n"
    "DA18/JAN/2017.08:56\n"
    "VD12.00\n"
    "WD35\n"
    "OL+00.25-00.50025\n"
    "OR+00.00-00.50005\n"
    "DL+00.00-00.00-05\n"
    "DR+00.00-00.25+10\n"
    "DRMIDNIDEK/ARK-1a\n"
    "NO0009\n"
    "DA18/JAN/2017.08:56\n"
    "VD12.00\n"
    "WD35\n"
    "OL+00.25-00.500309\n"
    "OR+00.00-00.251759\n"
    "CL+00.25-00.50030\n"
    "CR+00.00-00.25175\n"
    "PD62????58\n"
    "DKM L07.8707.6618007.77\n"
    "DL43.0044.0018043.50-01.00\n"
    " L07.8707.6618007.77\n"
    "DL43.0044.0018043.50-01.00\n"
    " L07.8707.6618007.77\n"
    "DL43.0044.0018043.50-01.00\n"
    " L07.8707.6618007.77\n"
    "DL43.0044.0018043.50-01.00\n"
    " R07.8107.5600507.69\n"
    "DR43.2544.7500544.00-01.50\n"
    " R07.8107.5600507.69\n"
    "DR43.2544.7500544.00-01.50\n"
    " R07.8107.5600507.69\n"
    "DR43.2544.7500544.00-01.50\n"
    " R07.8107.5500507.68\n"
    "DR43.2544.7500544.00-01.50\n"
    "SL12.1\n"
    "PL05.6N\n"
    "SR12.3\n"
    "PR05.4N\n"
    "8CC7";


    // TRADUCTION DES DONNEES EN FONCTION DE L'AUTOREF

    if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-1S"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30"
     || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK AR-20")
    {
        /*
     NIDEK ARK-1a - exemple de fichier de sortie
     */
     /*
DrmIDNIDEK/ARK-1a
NO0225
DA27/MAR/2017.07:40
VD12.00
WD35
OL-05.25-00.75080
OR-04.75-00.75105
DL-00.25+00.00+05
DR-00.25+00.00-05
DRMIDNIDEK/ARK-1a
NO0225
DA27/MAR/2017.07:40
VD12.00
WD35
OL-05.00-00.75075
OL-05.00-00.750808 
OL-05.00-00.750758 
OL-05.00-00.750758 
OR-04.50-00.75110
OR-04.50-00.751109 
OR-04.50-00.751109 
OR-04.50-00.751109 
CL-04.75-00.50080
CR-04.25-00.75110
PD68????63
DKM L07.7607.7014007.73
DL43.5043.7514043.75-00.25
 L07.7607.7014007.73
DL43.5043.7514043.75-00.25
 L07.7607.7013507.73
DL43.5043.7513543.75-00.25
 L07.7407.6914507.72
DL43.5044.0014543.75-00.50
 R07.8407.7612007.80
DR43.0043.5012043.25-00.50
 R07.8407.7612507.80
DR43.0043.5012543.25-00.50
 R07.8407.7612007.80
DR43.0043.5012043.25-00.50
 R07.8307.7612007.80
DR43.0043.5012043.25-00.50
SL11.8
PL05.7N
SR11.8
PR05.9N
A40F

// Exemple de fichier de sortie pour un TONOREFIII
DRMIDNIDEK/TONOREF3
NO0002
DA31/MAR/2017.01:02
VD12.00
WD40
OL+00.00-00.250559
OL+00.00-00.250559
OR-00.50-00.25090
OR-00.25-00.500909
OR-00.50-00.250909
OR-00.50-00.250909
CL+00.00-00.25055
CR-00.50-00.25090
TL+00.00-00.25055
TR-00.50-00.25090
PD67????63
DKM
 L07.7607.7000007.73
DL43.5043.7500043.75-00.25
 L07.7707.7018007.74
DL43.5043.7518043.50-00.25
 L07.7607.6900007.73
DL43.5044.0000043.75-00.50
 L07.7607.7000007.73
DL43.5043.7500043.75-00.25
 R07.7207.6700507.70
DR43.7544.0000543.75-00.25
 R07.7207.6700507.70
DR43.7544.0000543.75-00.25
 R07.7207.6700507.70
DR43.7544.0000543.75-00.25
 R07.7307.6700507.70
DR43.7544.0000543.75-00.25
Dkm
 L07.7507.6401007.70
DL43.5044.2501043.75-00.75
 L07.7607.6501007.71
DL43.5044.0001043.75-00.50
 L07.7507.6401007.70
DL43.5044.2501043.75-00.75
 L07.7407.6301007.69
DL43.5044.2501044.00-00.75
 R07.7007.6200007.66
DR43.7544.2500044.00-00.50
 R07.6907.6100007.65
DR44.0044.2500044.00-00.25
 R07.7007.6200007.66
DR43.7544.2500044.00-00.50
 R07.7207.6300507.68
DR43.7544.2500544.00-00.50
SL12.2
PL05.0N
SR12.2
PR05.5N
DNT
 L01    17/2.3  AV17.0/2.30
DPM
 L01 0583  AV0583
 R01 0586  AV0586


Exemple de fichier de sortie avec seul l'OG mesuré pour un ARK1A
Drm
0002IDNIDEK/ARK-1a
NO0272
DA10/APR/2017.08:58
VD12.00
WD35
OL+00.00-01.25095
DL+00.25-00.50-05
DRM
NIDEK/ARK-1a
NO0272
DA10/APR/2017.08:58
VD12.00
WD35
OL-00.25-00.75100
OL-00.25-00.751008
OL-00.25-00.751008
OL-00.25-00.751008
CL-00.25-00.75100
CR+00.00+00.00000
DKM
 L07.8907.8214507.86
DL42.7543.2514543.00-00.50
 L07.8807.8314507.86
DL42.7543.0014543.00-00.25
 L07.8907.8214007.86
DL42.7543.2514043.00-00.50
 L07.8907.8214507.86
DL42.7543.2514543.00-00.50
SL11.7
PL04.7N
000461E4
       */
        a               = Mesure.indexOf("VD");
        a               = Mesure.length() - a -1;
        Ref             = Mesure.right(a);
        if (Ref != "")
        {
            a  = Ref.indexOf("OR");
            // OEIL DROIT -----------------------------------------------------------------------------
            if (a>=0)
            {
                mesureOD             = Ref.mid(Ref.indexOf("OR")+2,15)   .replace(" ","0");
                mSphereOD            = mesureOD.mid(0,6);
                mCylOD               = mesureOD.mid(6,6);
                mAxeOD               = mesureOD.mid(12,3);
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            a  = Ref.indexOf("OL");
            if (a>=0)
            {
                mesureOG             = Ref.mid(Ref.indexOf("OL")+2,15)   .replace(" ","0");
                mSphereOG            = mesureOG.mid(0,6);
                mCylOG               = mesureOG.mid(6,6);
                mAxeOG               = mesureOG.mid(12,3);
            }
            a  = Ref.indexOf("PD");
            if (a>=0)
                PD                   = Ref.mid(Ref.indexOf("PD")+2,2);

            K = "";
            if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
                    || gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30")
            {
                // Données de KERATOMETRIE --------------------------------------------------------------------------------------------------------
                if (Mesure.contains("DKM"))                 //=> il y a une mesure de keratometrie
                {
                    a                   = Mesure.indexOf("DKM");
                    a                   = Mesure.length() - a;
                    K                   = Mesure.right(a);
                    // OEIL DROIT -----------------------------------------------------------------------------
                    a  = Ref.indexOf(" R");
                    if (a>=0)
                    {
                        KOD                 = K.mid(K.indexOf(" R")+2,13);
                        K1OD                = KOD.mid(0,5);
                        K2OD                = KOD.mid(5,5);
                        AxeKOD              = KOD.mid(10,3).toInt();
                        //KOD                 = K.mid(K.indexOf("DR")+2,24);
                        //DioptrAstCornOD     = KOD.mid(18,6);
                        MesureKerato["K1OD"]        = K1OD;
                        MesureKerato["K2OD"]        = K2OD;
                        MesureKerato["AxeKOD"]      = AxeKOD;
                        QString mOD                 = K.mid(K.indexOf("DR")+2,10).replace(" ","0");
                        DioptrAstCornOD             = QString::number(mOD.mid(0,5).toDouble() - mOD.mid(5,5).toDouble(),'f',2);
                        MesureKerato["DioptrK1OD"]  = QString::number(mOD.mid(0,5).toDouble(),'f',2);
                        MesureKerato["DioptrK2OD"]  = QString::number(mOD.mid(5,5).toDouble(),'f',2);
                        MesureKerato["DioptrKOD"]   = DioptrAstCornOD;
                    }
                    // OEIL GAUCHE ---------------------------------------------------------------------------
                    a  = Ref.indexOf(" L");
                    if (a>=0)
                    {
                        KOG                         = K.mid(K.indexOf(" L")+2,13);
                        K1OG                        = KOG.mid(0,5);
                        K2OG                        = KOG.mid(5,5);
                        AxeKOG                      = KOG.mid(10,3).toInt();
                        MesureKerato["K1OG"]        = K1OG;
                        MesureKerato["K2OG"]        = K2OG;
                        MesureKerato["AxeKOG"]      = AxeKOG;
                        QString mOG                 = K.mid(K.indexOf("DL")+2,10).replace(" ","0");
                        DioptrAstCornOG             = QString::number(mOG.mid(0,5).toDouble() - mOG.mid(5,5).toDouble(),'f',2);
                        MesureKerato["DioptrK1OG"]  = QString::number(mOG.mid(0,5).toDouble(),'f',2);
                        MesureKerato["DioptrK2OG"]  = QString::number(mOG.mid(5,5).toDouble(),'f',2);
                        MesureKerato["DioptrKOG"]   = DioptrAstCornOG;
                    }
                 }
            }
        }
        if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")
        {
            // Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
            if (Mesure.contains("DNT"))                 //=> il y a une mesure de tonometrie
            {
                QString TonoOD(""), TonoOG("");
                a                       = Mesure.indexOf("DNT");
                a                       = Mesure.length() - a -1;
                QString Tono            = Mesure.right(a);
                int b                   = Tono.indexOf("DPM");
                if (b>=0)
                    Tono                = Tono.left(b-1);
                a= Tono.indexOf(" R");
                if (a>=0)
                {
                    TonoOD          = Tono.right(Tono.length()-a-1);
                    a               = TonoOD.indexOf("AV");
                    TonoOD          = TonoOD.mid(a+2,2).replace(" ","0");
                }
                b= Tono.indexOf(" L");
                if (b>=0)
                {
                    TonoOG          = Tono.right(Tono.length()-b-1);
                    b               = TonoOG.indexOf("AV");
                    TonoOG          = TonoOG.mid(b+2,2).replace(" ","0");
                }
                MesureTono["TOOD"]      = TonoOD;
                MesureTono["TOOG"]      = TonoOG;
            }
            // Données de PACHYMETRIE --------------------------------------------------------------------------------------------------------
            if (Mesure.contains("DPM"))                 //=> il y a une mesure de pachymetrie
            {
                QString PachyOD(""), PachyOG("");
                a                       = Mesure.indexOf("DPM");
                a                       = Mesure.length() - a;
                QString Pachy           = Mesure.right(a);
                if (Pachy.indexOf(" R")>=0)
                    PachyOD         = Pachy.mid(Pachy.indexOf(" R")+6,3);
                if (Pachy.indexOf(" L")>=0)
                    PachyOG         = Pachy.mid(Pachy.indexOf(" L")+6,3);
                MesurePachy["PachyOD"]  = PachyOD;
                MesurePachy["PachyOG"]  = PachyOG;
            }
        }
    }
    MesureAutoref["SphereOD"]    = mSphereOD;
    MesureAutoref["CylOD"]       = mCylOD;
    MesureAutoref["AxeOD"]       = mAxeOD;
    MesureAutoref["SphereOG"]    = mSphereOG;
    MesureAutoref["CylOG"]       = mCylOG;
    MesureAutoref["AxeOG"]       = mAxeOG;
    MesureAutoref["PD"]          = PD;
    //qDebug() << "od" << mSphereOD << mCylOD << mAxeOD << "og" << mSphereOG << mCylOG << mAxeOG << "PD = " + PD;
}


QMap<QString,QVariant>  Procedures::DonneesAutoref()
{
    return MesureAutoref;
}

QMap<QString,QVariant>  Procedures::DonneesKerato()
{
    return MesureKerato;
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'autorefractometre
//--------------------------------------------------------------------------------------
void Procedures::setHtmlAutoref()
{
    QString mSphereOD   = PrefixePlus(MesureAutoref["SphereOD"].toString());
    QString mCylOD      = PrefixePlus(MesureAutoref["CylOD"].toString());
    QString mAxeOD      = QString::number(MesureAutoref["AxeOD"].toInt());
    QString mSphereOG   = PrefixePlus(MesureAutoref["SphereOG"].toString());
    QString mCylOG      = PrefixePlus(MesureAutoref["CylOG"].toString());
    QString mAxeOG      = QString::number(MesureAutoref["AxeOG"].toInt());

    QString ResultatOD("");
    QString ResultatOG("");
    QString Resultat = "";

    // QString TagAncre, numIDref; // 07-07-2014 08-08-2014

    // Génération du code html pour TAG Ancre avec ID refraction            // 07-07-2014 // 08-08-2014
    //numIDref = QString::number(gListeRefractionID.at(gListeRefractionID.size()-1));
    //TagAncre = "<a name=\"" + numIDref + "\"></a>" "<span ><a href=\""+ numIDref + "\" style=\"text-decoration:none\" style=\"color:#000000\"> ";
    // - 1 - détermination des verres
    // détermination OD
    if (MesureAutoref["SphereOD"].toString()!="")
    {
        if (QLocale().toDouble(mCylOD) != 0 && QLocale().toDouble(mSphereOD) != 0)
            ResultatOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
        else if (QLocale().toDouble(mCylOD) == 0 && QLocale().toDouble(mSphereOD) != 0)
            ResultatOD = mSphereOD;
        else if (QLocale().toDouble(mCylOD) != 0 && QLocale().toDouble(mSphereOD) == 0)
            ResultatOD = mCylOD + tr(" à ") + mAxeOD + "°" ;
        else
            ResultatOD = tr("plan");
    }


    // détermination OG
    if (MesureAutoref["SphereOD"].toString()!="")
    {
        if (QLocale().toDouble(mCylOG) != 0 && QLocale().toDouble(mSphereOG) != 0)
            ResultatOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + "°)";
        else if (QLocale().toDouble(mCylOG) == 0 && QLocale().toDouble(mSphereOG) != 0)
            ResultatOG = mSphereOG;
        else if (QLocale().toDouble(mCylOG) != 0 && QLocale().toDouble(mSphereOG) == 0)
            ResultatOG = mCylOG + tr(" à ") + mAxeOG + "°" ;
        else
            ResultatOG = tr("plan");
    }

    // Détermination de Resultat
    if (ResultatOD=="" && ResultatOG=="")
        return;
    else if (ResultatOD == ResultatOG)
            Resultat = ResultatOD + " " + tr("ODG");
    else if (ResultatOD=="" || ResultatOG=="")
    {
        Resultat = ResultatOD + ResultatOG;
        if (ResultatOD=="")
            Resultat += tr("OG");
        else
            Resultat += tr("OD");
    }
    else
        Resultat = ResultatOD + " / " + ResultatOG;
    HtmlMesureAutoref =  "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("Autoref") + ":</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Resultat + "</td></p>";

}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la keratométrie
//--------------------------------------------------------------------------------------
void Procedures::setHtmlKerato(QMap<QString,QVariant>  MKer)
{
    QString mK1OD       = QLocale().toString(MKer["K1OD"].toDouble(),'f',2);
    QString mK2OD       = QLocale().toString(MKer["K2OD"].toDouble(),'f',2);
    QString mAxeKOD     = QString::number(MKer["AxeKOD"].toInt());
    QString mDioptrK1OD = QLocale().toString(MKer["DioptrK1OD"].toDouble(),'f',1);
    QString mDioptrK2OD = QLocale().toString(MKer["DioptrK2OD"].toDouble(),'f',1);
    QString mDioptrKOD  = PrefixePlus(MKer["DioptrKOD"].toString());
    QString mK1OG       = QLocale().toString(MKer["K1OG"].toDouble(),'f',2);
    QString mK2OG       = QLocale().toString(MKer["K2OG"].toDouble(),'f',2);
    QString mAxeKOG     = QString::number(MKer["AxeKOG"].toInt());
    QString mDioptrK1OG = QLocale().toString(MKer["DioptrK1OG"].toDouble(),'f',1);
    QString mDioptrK2OG = QLocale().toString(MKer["DioptrK2OG"].toDouble(),'f',1);
    QString mDioptrKOG  = PrefixePlus(MKer["DioptrKOG"].toString());
    QString kerato = "";

    if (QLocale().toDouble(mK1OD)>0)
    {
        if (QLocale().toDouble(mDioptrKOD)!=0)
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OD + "/" + mK2OD + " Km = " + QString::number((QLocale().toDouble(mK1OD) + QLocale().toDouble(mK2OD))/2,'f',2) + "</td>"
                      "<td width=\"180\">" + mDioptrK1OD + "/" + mDioptrK2OD + " " + mDioptrKOD +  tr(" à ") + mAxeKOD + "°</td></p>";
        else
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OD + tr(" à ") + mAxeKOD + "°/" + mK2OD
                      + " Km = " + QString::number((QLocale().toDouble(mK1OD) + QLocale().toDouble(mK2OD))/2,'f',2) + "</td></p>";
    }
    if (QLocale().toDouble(mK1OG)>0)
    {
        if (QLocale().toDouble(mDioptrKOG)!=0)
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OG + "/" + mK2OG + " Km = " + QString::number((QLocale().toDouble(mK1OG) + QLocale().toDouble(mK2OG))/2,'f',2) + "</td>"
                      "<td width=\"180\">" + mDioptrK1OG + "/" + mDioptrK2OG + " " + mDioptrKOG +  tr(" à ") + mAxeKOG + "°</td></p>";
        else
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"180\">"  + mK1OG +  tr(" à ") + mAxeKOG + "°/" + mK2OG
                      + " Km = " + QString::number((QLocale().toDouble(mK1OG) + QLocale().toDouble(mK2OG))/2,'f',2) + "</td></p>";
    }

    HtmlMesureKerato  = kerato;
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la tonométrie
//--------------------------------------------------------------------------------------
void Procedures::setHtmlTono()
{
    HtmlMesureTono = "";
    QString mTOD        = QLocale().toString(MesureTono["TOOD"].toDouble(),'f',0);
    QString mTOG        = QLocale().toString(MesureTono["TOOG"].toDouble(),'f',0);
    QString Methode     = tr("Air");
    QString Tono        ="";
    QString TODcolor, TOGcolor;

    if (mTOD.toInt() > 0 || mTOG.toInt() > 0)
    {
        if (mTOD.toInt() > 21)
            TODcolor = "<font color = \"red\"><b>" + mTOD + "</b></font>";
        else
            TODcolor = "<font color = \"blue\"><b>" + mTOD + "</b></font>";
        if (mTOG.toInt() > 21)
            TOGcolor = "<font color = \"red\"><b>" + mTOG + "</b></font>";
        else
            TOGcolor = "<font color = \"blue\"><b>" + mTOG + "</b></font>";
        if (mTOD.toInt() == 0 && mTOG.toInt() > 0)
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TOG:") + "</b></font></td><td width=\"80\">" + TOGcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";
        else if (mTOG.toInt() == 0 && mTOD.toInt() > 0)
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TOD:") + "</b></font></td><td width=\"80\">" + TODcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";
        else if (mTOD.toInt() == mTOG.toInt())
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TODG:") + "</b></font></td><td width=\"80\">" + TODcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";
        else
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TO:") + "</b></font></td><td width=\"80\">" + TODcolor + "/" + TOGcolor+ tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";

    }
    HtmlMesureTono = Tono;
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la pachymétrie
//--------------------------------------------------------------------------------------
void Procedures::setHtmlPachy()
{
    HtmlMesureTono = "";
    QString mPachyOD        = QLocale().toString(MesurePachy["PachyOD"].toInt());
    QString mPachyOG        = QLocale().toString(MesurePachy["PachyOG"].toInt());
    QString Pachy        ="";
    int a = mPachyOD.toInt();
    int b = mPachyOG.toInt();
    mPachyOD = "<font color = \"blue\"><b>" + mPachyOD + "</b></font>";
    mPachyOG = "<font color = \"blue\"><b>" + mPachyOG + "</b></font>";

    if (a > 0 || b > 0)
    {
        if (a == 0 && b > 0)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("PachyOG:") + "</b></font></td><td width=\"80\">" + mPachyOG + "</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";
        else if (b == 0 && a > 0)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("PachyOG:") + "</b></font></td><td width=\"80\">" + mPachyOD + "</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";
        else if (a == b)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("PachyODG:") + "</b></font></td><td width=\"80\">" + mPachyOG + "</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";
        else
            Pachy= "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("Pachy:") + "</b></font></td><td width=\"80\">" + mPachyOD + "/" + mPachyOG + "</td><td>" + getDataUser()["UserLogin"].toString() + "</td></p>";

    }
    HtmlMesurePachy = Pachy;
}

QString Procedures::HtmlAutoref()
{
    return HtmlMesureAutoref;
}

QString Procedures::HtmlKerato()
{
    return HtmlMesureKerato;
}

QString Procedures::HtmlTono()
{
    return HtmlMesureTono;
}

QString Procedures::HtmlPachy()
{
    return HtmlMesurePachy;
}

QSerialPort* Procedures::PortTono()
{
    return lPortTono;
}

QString Procedures::NomPortAutoref()
{
    return gPortAutoref;
}

QString Procedures::NomPortFronto()
{
    return gPortFronto;
}

QString Procedures::NomPortRefracteur()
{
    return gPortRefracteur;
}

QString Procedures::NomPortTono()
{
    return gPortTono;
}

QString Procedures::TypeMesureRefraction()
{
    return MesureRef;
}

void Procedures::setTypeMesureRefraction(QString mesure)
{
    MesureRef = mesure;
}

//---------------------------------------------------------------------------------
// Calcul de la formule de refraction
//---------------------------------------------------------------------------------
QString Procedures::CalculeFormule(QMap<QString,QVariant> Mesure,  QString Cote)
{
        QString mSphere;
        QString mCyl;
        QString mAxe;
        QString mAdd;
        if (Cote == "D")
        {
            mSphere   = PrefixePlus(Mesure["SphereOD"].toString());
            mCyl      = PrefixePlus(Mesure["CylOD"].toString());
            mAxe      = QString::number(Mesure["AxeOD"].toInt());
            mAdd      = PrefixePlus(Mesure["AddOD"].toString());
        }
        else if (Cote == "G")
        {
            mSphere   = PrefixePlus(Mesure["SphereOG"].toString());
            mCyl      = PrefixePlus(Mesure["CylOG"].toString());
            mAxe      = QString::number(Mesure["AxeOG"].toInt());
            mAdd      = PrefixePlus(Mesure["AddOG"].toString());
        }
        else return "";
        QString Resultat;
        if (QLocale().toDouble(mCyl) != 0.00 && QLocale().toDouble(mSphere) != 0.00)
            Resultat = mSphere + " (" + mCyl + tr(" à ") + mAxe + "°)" ;
        if (QLocale().toDouble(mCyl) == 0.00 && QLocale().toDouble(mSphere) != 0.00)
            Resultat = mSphere ;
        if (QLocale().toDouble(mCyl) != 0.00 && QLocale().toDouble(mSphere) == 0.00)
            Resultat = mCyl + tr(" à ") + mAxe + "°" ;
        if (QLocale().toDouble(mCyl) == 0.00 && QLocale().toDouble(mSphere) == 0.00)
            Resultat = tr("plan");
        if (QLocale().toDouble(mAdd) > 0.00)
            Resultat += " add." + mAdd + " VP" ;
        return Resultat;
}

void Procedures::InsertRefraction(int idPatient, int idActe, QString Mesure)
{
    QString                 zQuelleMesure;
    QMap<QString,QVariant>  MapMesure;
    if (!MesureFronto.isEmpty() && Mesure == "Fronto" && NouvMesureFronto)
    {
        MapMesure = MesureFronto;
        bool a =
               (MapMesure["SphereOD"].toDouble()== 0
            &&  MapMesure["CylOD"].toDouble()   == 0
            &&  MapMesure["AddVPOD"].toDouble() == 0
            &&  MapMesure["SphereOG"].toDouble()== 0
            &&  MapMesure["CylOG"].toDouble()   == 0
            &&  MapMesure["AddVPOG"].toDouble() == 0
            );
        if (!a)
        {
            QString mSphereOD, mSphereOG;;
            QString mCylOD, mCylOG;
            QString mAxeOD, mAxeOG;
            QString mAddOD, mAddOG;
            mSphereOD       = PrefixePlus(MapMesure["SphereOD"].toString());
            mCylOD          = PrefixePlus(MapMesure["CylOD"].toString());
            mAxeOD          = QString::number(MapMesure["AxeOD"].toInt());
            mAddOD          = PrefixePlus(MapMesure["AddOD"].toString());
            mSphereOG       = PrefixePlus(MapMesure["SphereOG"].toString());
            mCylOG          = PrefixePlus(MapMesure["CylOG"].toString());
            mAxeOG          = QString::number(MapMesure["AxeOG"].toInt());
            mAddOG          = PrefixePlus(MapMesure["AddOG"].toString());
            zQuelleMesure = "P";
            QString requete = "delete from " NOM_TABLE_REFRACTION
                    " where idPat = " + QString::number(idPatient) +
                    " and idacte = " + QString::number(idActe) +
                    " and QuelleMesure = 'P'" +
                    " and FormuleOD = '" + CalculeFormule(MapMesure,"D") + "'" +
                    " and FormuleOG = '" + CalculeFormule(MapMesure,"G") + "'";
            QSqlQuery delquer(requete, db);
            TraiteErreurRequete(delquer, requete, "");

            requete = "INSERT INTO " NOM_TABLE_REFRACTION
                    " (idPat, idActe, DateRefraction, QuelleMesure, QuelleDistance,"
                    " SphereOD, CylindreOD, AxeCylindreOD, AddVPOD, FormuleOD,"
                    " SphereOG, CylindreOG, AxeCylindreOG, AddVPOG, FormuleOG)"
                    " VALUES (" +
                    QString::number(idPatient)  + ", " +
                    QString::number(idActe)     + ", " +
                    "NOW(), '" +
                    zQuelleMesure               + "'," +
                    ((QLocale().toDouble(mAddOD)>0 || QLocale().toDouble(mAddOG)>0)? "'2'" : "null") + "," +
                    QString::number(QLocale().toDouble(mSphereOD))  + "," +
                    QString::number(QLocale().toDouble(mCylOD))     + "," +
                    mAxeOD     + "," +
                    (QLocale().toDouble(mAddOD)>0? QString::number(QLocale().toDouble(mAddOD)) : "null") + ",'" +
                    CalculeFormule(MapMesure,"D") + "'," +
                    QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    QString::number(QLocale().toDouble(mCylOG))     + "," +
                    mAxeOG     + "," +
                    (QLocale().toDouble(mAddOG)>0? QString::number(QLocale().toDouble(mAddOG)) : "null") + ",'" +
                    CalculeFormule(MapMesure,"G") + "')";

            QSqlQuery InsertRefractionQuery (requete, db);
            TraiteErreurRequete(InsertRefractionQuery,requete, tr("Erreur de création de données fronto dans ") + NOM_TABLE_REFRACTION);
        }
    }
    if (!MesureAutoref.isEmpty() && Mesure == "Autoref" && NouvMesureAutoref)
    {
        MapMesure = MesureAutoref;
        bool a =
               (MapMesure["SphereOD"].toDouble()== 0
            &&  MapMesure["CylOD"].toDouble()   == 0
            &&  MapMesure["SphereOG"].toDouble()== 0
            &&  MapMesure["CylOG"].toDouble()   == 0
            );
        if (!a)
        {
            QString mSphereOD, mSphereOG;;
            QString mCylOD, mCylOG;
            QString mAxeOD, mAxeOG;
            QString PD;
            mSphereOD       = PrefixePlus(MapMesure["SphereOD"].toString());
            mCylOD          = PrefixePlus(MapMesure["CylOD"].toString());
            mAxeOD          = QString::number(MapMesure["AxeOD"].toInt());
            mSphereOG       = PrefixePlus(MapMesure["SphereOG"].toString());
            mCylOG          = PrefixePlus(MapMesure["CylOG"].toString());
            mAxeOG          = QString::number(MapMesure["AxeOG"].toInt());
            PD              = MapMesure["PD"].toString();
            if (PD == "")
                PD = "null";
            zQuelleMesure = "A";
            QString requete = "delete from " NOM_TABLE_REFRACTION
                    " where idPat = " + QString::number(idPatient) +
                    " and idacte = " + QString::number(idActe) +
                    " and QuelleMesure = 'A'" ;
            QSqlQuery delquer(requete, db);
            TraiteErreurRequete(delquer, requete, "");

            requete = "INSERT INTO " NOM_TABLE_REFRACTION
                    " (idPat, idActe, DateRefraction, QuelleMesure, QuelleDistance,"
                    " SphereOD, CylindreOD, AxeCylindreOD, FormuleOD,"
                    " SphereOG, CylindreOG, AxeCylindreOG, FormuleOG, PD)"
                    " VALUES (" +
                    QString::number(idPatient)  + ", " +
                    QString::number(idActe)     + ", " +
                    "NOW(), '" +
                    zQuelleMesure               + "'," +
                    "null" + "," +
                    QString::number(QLocale().toDouble(mSphereOD))  + "," +
                    QString::number(QLocale().toDouble(mCylOD))     + "," +
                    mAxeOD     + ",'" +
                    CalculeFormule(MapMesure,"D") + "'," +
                    QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    QString::number(QLocale().toDouble(mCylOG))     + "," +
                    mAxeOG     + ",'" +
                    CalculeFormule(MapMesure,"G") + "', " + PD + ")";

            QSqlQuery InsertRefractionQuery (requete, db);
            TraiteErreurRequete(InsertRefractionQuery,requete, tr("Erreur de création de données autoref dans ") + NOM_TABLE_REFRACTION);
            requete = "select idPat from " NOM_TABLE_DONNEES_OPHTA_PATIENTS " where idPat = " + QString::number(idPatient) + " and QuelleMesure = 'A'";
            QSqlQuery selquer(requete, db);
            TraiteErreurRequete(selquer,requete,"");
            if (selquer.size()== 0)
            {
                requete = "INSERT INTO " NOM_TABLE_DONNEES_OPHTA_PATIENTS
                        " (idPat, DateRefOD, DateRefOG, QuelleMesure, QuelleDistance,"
                        " SphereOD, CylindreOD, AxeCylindreOD,"
                        " SphereOG, CylindreOG, AxeCylindreOG, PD)"
                        " VALUES (" +
                        QString::number(idPatient)  + ", " +
                        "NOW(), NOW(),'" +
                        zQuelleMesure               + "'," +
                        "null" + "," +
                        QString::number(QLocale().toDouble(mSphereOD))  + "," +
                        QString::number(QLocale().toDouble(mCylOD))     + "," +
                        mAxeOD     + "," +
                        QString::number(QLocale().toDouble(mSphereOG))  + "," +
                        QString::number(QLocale().toDouble(mCylOG))     + "," +
                        mAxeOG     + "," +
                        PD + ")";

                QSqlQuery InsertDOPQuery (requete, db);
                TraiteErreurRequete(InsertDOPQuery,requete, tr("Erreur de création de données autoref dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
            else
            {
                requete = "UPDATE " NOM_TABLE_DONNEES_OPHTA_PATIENTS " set"
                        " QuelleMesure = '" + zQuelleMesure + "'," +
                        " QuelleDistance = null," +
                        " DateRefOD = NOW()," +
                        " DateRefOG = NOW()," +
                        " SphereOD = "      + QString::number(QLocale().toDouble(mSphereOD))  + "," +
                        " CylindreOD = "    + QString::number(QLocale().toDouble(mCylOD))     + "," +
                        " AxeCylindreOD = " + mAxeOD + "," +
                        " SphereOG = "      + QString::number(QLocale().toDouble(mSphereOG))  + "," +
                        " CylindreOG = "    + QString::number(QLocale().toDouble(mCylOG))     + "," +
                        " AxeCylindreOG = " + mAxeOG + "," +
                        " PD = "            + PD +
                        " where idpat = "   + QString::number(idPatient);

                QSqlQuery UpdDOPQuery (requete, db);
                TraiteErreurRequete(UpdDOPQuery,requete, tr("Erreur de mise à jour de données autoref dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
        }
    }
    if (!MesureKerato.isEmpty() && Mesure == "Kerato")
    {
        MapMesure = MesureKerato;
        bool a =
               (MapMesure["K1OD"].toDouble()== 0
            &&  MapMesure["K1OG"].toDouble()   == 0
            );
        if (!a)
        {
            QString mK1OD       = QLocale().toString(MapMesure["K1OD"].toDouble(),'f',2);
            QString mK2OD       = QLocale().toString(MapMesure["K2OD"].toDouble(),'f',2);
            QString mAxeKOD     = QString::number(MapMesure["AxeKOD"].toInt());
            QString mK1OG       = QLocale().toString(MapMesure["K1OG"].toDouble(),'f',2);
            QString mK2OG       = QLocale().toString(MapMesure["K2OG"].toDouble(),'f',2);
            QString mAxeKOG     = QString::number(MapMesure["AxeKOG"].toInt());
            //qDebug() << mK1OD << mK2OD << mAxeKOD << mK1OG << mK2OG << mAxeKOG;
            QString requete = "select idPat from " NOM_TABLE_DONNEES_OPHTA_PATIENTS " where idPat = " + QString::number(idPatient) + " and QuelleMesure = 'A'";
            QSqlQuery selquer(requete, db);
            TraiteErreurRequete(selquer,requete,"");
            if (selquer.size()== 0)
            {
                requete = "INSERT INTO " NOM_TABLE_DONNEES_OPHTA_PATIENTS
                        " (idPat, DateK, K1OD, K2OD, AxeKOD, K1OG, K2OG, AxeKOG, OrigineK)"
                        " VALUES (" +
                        QString::number(idPatient)  + ", " +
                        "NOW(), " +
                        QString::number(QLocale().toDouble(mK1OD), 'f', 2)   + "," +
                        QString::number(QLocale().toDouble(mK2OD), 'f', 2)   + "," +
                        mAxeKOD + "," +
                        QString::number(QLocale().toDouble(mK1OG), 'f', 2)   + "," +
                        QString::number(QLocale().toDouble(mK2OG), 'f', 2)   + "," +
                        mAxeKOG + ",'A')";

                QSqlQuery InsertDOPQuery (requete, db);
                TraiteErreurRequete(InsertDOPQuery,requete, tr("Erreur de création de données kératométrie  dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
            else
            {
                requete = "UPDATE " NOM_TABLE_DONNEES_OPHTA_PATIENTS " set"
                        " DateK = NOW(),"
                        " K1OD = " + QString::number(QLocale().toDouble(mK1OD), 'f', 2)  + "," +
                        " K2OD = " + QString::number(QLocale().toDouble(mK2OD), 'f', 2)  + "," +
                        " AxeKOD = " + mAxeKOD  + "," +
                        " K1OG = " + QString::number(QLocale().toDouble(mK1OG), 'f', 2)  + "," +
                        " K2OG = " + QString::number(QLocale().toDouble(mK2OG), 'f', 2)  + "," +
                        " AxeKOG = " + mAxeKOG  + "," +
                        " OrigineK = 'A'" +
                        " where idpat = "+ QString::number(idPatient);

                QSqlQuery UpdDOPQuery (requete, db);
                TraiteErreurRequete(UpdDOPQuery,requete, tr("Erreur de modification de données de kératométrie dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
        }
    }
    if (!MesureRefracteurSubjectif.isEmpty() && Mesure == "Subjectif")
    {
        MapMesure = MesureRefracteurSubjectif;
        bool a =
               (MapMesure["AVLOD"].toDouble() == 0
            &&  MapMesure["AVLOG"].toDouble() == 0
            );
        if (!a)
        {
            QString mSphereOD, mSphereOG;;
            QString mCylOD, mCylOG;
            QString mAxeOD, mAxeOG;
            QString mAddOD, mAddOG;
            QString mAVLOD, mAVLOG;
            QString mAVPOD, mAVPOG;
            QString PD;
            mSphereOD       = PrefixePlus(MapMesure["SphereOD"].toString());
            mCylOD          = PrefixePlus(MapMesure["CylOD"].toString());
            mAxeOD          = QString::number(MapMesure["AxeOD"].toInt());
            mAddOD          = PrefixePlus(MapMesure["AddOD"].toString());
            mAVLOD          = QLocale().toString(MapMesure["AVLOD"].toDouble()*10) + "/10";
            mAVPOD          = MapMesure["AVPOD"].toString();
            mSphereOG       = PrefixePlus(MapMesure["SphereOG"].toString());
            mCylOG          = PrefixePlus(MapMesure["CylOG"].toString());
            mAxeOG          = QString::number(MapMesure["AxeOG"].toInt());
            mAddOG          = PrefixePlus(MapMesure["AddOG"].toString());
            mAVLOD          = QLocale().toString(MapMesure["AVLOG"].toDouble()*10) + "/10";
            mAVPOG          = MapMesure["AVPOG"].toString();
            PD              = MapMesure["PD"].toString();
            if (PD == "")
                PD = "null";
            zQuelleMesure = "R";
            QString requete = "delete from " NOM_TABLE_REFRACTION
                    " where idPat = " + QString::number(idPatient) +
                    " and idacte = " + QString::number(idActe) +
                    " and QuelleMesure = 'R'" ;
            QSqlQuery delquer(requete, db);
            TraiteErreurRequete(delquer, requete, "");
            requete = "INSERT INTO " NOM_TABLE_REFRACTION
                    " (idPat, idActe, DateRefraction, QuelleMesure, QuelleDistance,"
                    " SphereOD, CylindreOD, AxeCylindreOD, AddVPOD, FormuleOD, AVLOD, AVPOD,"
                    " SphereOG, CylindreOG, AxeCylindreOG, AddVPOG, FormuleOG, AVLOG, AVPOG, PD)"
                    " VALUES (" +
                    QString::number(idPatient)  + ", " +
                    QString::number(idActe)     + ", " +
                    "NOW(), '" +
                    zQuelleMesure               + "','" +
                    ((mAVPOD!="" || mAVPOG!="")? "2" : "L") + "'," +
                    QString::number(QLocale().toDouble(mSphereOD))  + "," +
                    QString::number(QLocale().toDouble(mCylOD))     + "," +
                    mAxeOD     + "," +
                    (QLocale().toDouble(mAddOD)>0? QString::number(QLocale().toDouble(mAddOD)) : "null") + ",'" +
                    CalculeFormule(MapMesure,"D") + "','" +
                    mAVLOD + "','" +
                    mAVPOD + "'," +
                    QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    QString::number(QLocale().toDouble(mCylOG))     + "," +
                    mAxeOG     + "," +
                    (QLocale().toDouble(mAddOG)>0? QString::number(QLocale().toDouble(mAddOG)) : "null") + ",'" +
                    CalculeFormule(MapMesure,"G") + "','" +
                    mAVLOG + "','" +
                    mAVPOG + "'," +
                    PD + ")";

            QSqlQuery InsertRefractionQuery (requete, db);
            TraiteErreurRequete(InsertRefractionQuery,requete, tr("Erreur de création  de données de refraction dans ") + NOM_TABLE_REFRACTION);
            requete = "select idPat from " NOM_TABLE_DONNEES_OPHTA_PATIENTS " where idPat = " + QString::number(idPatient) + " and QuelleMesure = 'R'";
            QSqlQuery selquer(requete, db);
            TraiteErreurRequete(selquer,requete,"");
            if (selquer.size()== 0)
            {
                requete = "INSERT INTO " NOM_TABLE_DONNEES_OPHTA_PATIENTS
                        " (idPat, DateRefOD, DateRefOG, QuelleMesure, QuelleDistance,"
                        " SphereOD, CylindreOD, AxeCylindreOD, AddVPOD, AVLOD, AVPOD,"
                        " SphereOG, CylindreOG, AxeCylindreOG, AddVPOG, AVLOG, AVPOG, PD)"
                        " VALUES (" +
                        QString::number(idPatient)  + ", " +
                        "NOW(), NOW(),'" +
                        zQuelleMesure               + "','" +
                        ((mAVPOD!="" || mAVPOG!="")? "2" : "L") + "'," +
                        QString::number(QLocale().toDouble(mSphereOD))  + "," +
                        QString::number(QLocale().toDouble(mCylOD))     + "," +
                        mAxeOD     + "," +
                        (QLocale().toDouble(mAddOD)>0? QString::number(QLocale().toDouble(mAddOD)) : "null") + ",'" +
                        mAVLOD + "','" +
                        mAVPOD + "'," +
                        QString::number(QLocale().toDouble(mSphereOG))  + "," +
                        QString::number(QLocale().toDouble(mCylOG))     + "," +
                        mAxeOG     + "," +
                        (QLocale().toDouble(mAddOG)>0? QString::number(QLocale().toDouble(mAddOG)) : "null") + ",'" +
                        mAVLOG + "','" +
                        mAVPOG + "'," +
                        PD + ")";

                QSqlQuery InsertDOPQuery (requete, db);
                TraiteErreurRequete(InsertDOPQuery,requete, tr("Erreur création de données de refraction dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
            else
            {
                requete = "UPDATE " NOM_TABLE_DONNEES_OPHTA_PATIENTS " set"
                        " QuelleMesure = '" + zQuelleMesure + "'," +
                        " QuelleDistance = null," +
                        " DateRefOD = NOW()," +
                        " DateRefOG = NOW()," +
                        " SphereOD = "      + QString::number(QLocale().toDouble(mSphereOD))  + "," +
                        " CylindreOD = "    + QString::number(QLocale().toDouble(mCylOD))     + "," +
                        " AxeCylindreOD = " + mAxeOD + "," +
                        " AVLOD = '"        + mAVLOD + "'," +
                        " AVPOD = '"        + mAVPOD + "'," +
                        " SphereOG = "      + QString::number(QLocale().toDouble(mSphereOG))  + "," +
                        " CylindreOG = "    + QString::number(QLocale().toDouble(mCylOG))     + "," +
                        " AxeCylindreOG = " + mAxeOG + "," +
                        " AVLOG = '"        + mAVLOG + "'," +
                        " AVPOG = '"        + mAVPOG + "'," +
                        " PD = "            + PD +
                        " where idpat = "   + QString::number(idPatient);

                QSqlQuery UpdDOPQuery (requete, db);
                TraiteErreurRequete(UpdDOPQuery,requete, tr("Erreur de mise à jour de données de refraction dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
        }
    }
}


