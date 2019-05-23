/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "icons.h"
#include "procedures.h"
#include "database.h"
#include "gbl_datas.h"


Procedures* Procedures::instance =  Q_NULLPTR;
Procedures* Procedures::I()
{
    if( !instance )
        instance = new Procedures();

    return instance;
}

Procedures::Procedures(QObject *parent) :
    QObject(parent)
{
    lCPParDefaut    = "";
    lVilleParDefaut = "";
    db              = DataBase::I();

    gnomFichIni     = QDir::homePath() + NOMFIC_INI;
    QFile FichierIni(gnomFichIni);
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
    MesureRef               = None;
    dlgrefractionouverte    = false;
    initOK                  = true;
    int margemm         = TailleTopMarge(); // exprimé en mm
    printer             = new QPrinter(QPrinter::HighResolution);
    printer             ->setFullPage(true);
    rect                = printer->paperRect();
    rect.adjust(Utils::mmToInches(margemm) * printer->logicalDpiX(),
                Utils::mmToInches(margemm) * printer->logicalDpiY(),
                -Utils::mmToInches(margemm) * printer->logicalDpiX(),
                -Utils::mmToInches(margemm) * printer->logicalDpiY());
}

void Procedures::ab(int i)
{
    QString mess;
    if (i == 1) mess = "jusque là, ça va";
    else mess = "là, ça ne va pas";
    UpMessageBox::Watch(Q_NULLPTR, mess);
}

/*--------------------------------------------------------------------------------------------------------------
-- Choix d'une date ou d'une période ---------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
QMap<QString, QDate> Procedures::ChoixDate(QWidget *parent)
{
    QMap<QString, QDate> DateMap;
    Dlg_ChxDate            = new dlg_choixdate(parent);
    Dlg_ChxDate->setWindowTitle(tr("Choisir une période"));
    if (Dlg_ChxDate->exec()> 0)
    {
        DateMap["DateDebut"] = Dlg_ChxDate->ui->DepuisdateEdit->date();
        if (!Dlg_ChxDate->ui->JourradioButton->isChecked())
            DateMap["DateFin"] = Dlg_ChxDate->ui->JusquAdateEdit->date();
        else
            DateMap["DateFin"] = DateMap["DateDebut"];
    }
    delete Dlg_ChxDate;
    return DateMap;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- renvoie la valeur du dossier documents pour le type d'appareil concerné -----------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::getDossierDocuments(QString Appareil, int mode)
{
    QString cle = db->getBaseFromInt( mode ) + "/DossiersDocuments/" + Appareil;
    QString dossier = gsettingsIni->value(cle).toString();
    return dossier;
}

void Procedures::EnChantier(bool avecMsg)
{
    UpMessageBox msgbox;
    msgbox.setIconPixmap(Icons::pxWorkInProgress());
    UpSmallButton OKBouton;
    if (avecMsg)
    msgbox.setInformativeText(tr("Le code qui suit n'est pas achevé et entraînera\nassez rapidement un plantage du programme\navec un risque élevé de corruption des données"));
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
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

bool Procedures::VerifAutresPostesConnectes(bool msg)
{
    QString req = "select NomPosteConnecte from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte <> '" + QHostInfo::localHostName().left(60) + "'";
    QVariantList ttipdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && ttipdata.size() > 0)
    {
        if (msg)
            UpMessageBox::Information(Q_NULLPTR, tr("Autres postes connectés!"),
                                     tr("Vous ne pouvez pas effectuer d'opération de sauvegarde/restauration sur la base de données"
                                     " si vous n'êtes pas le seul poste connecté.\n"
                                     "Le poste ") + ttipdata.at(0).toString() + tr(" est aussi connecté"));
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------------
// les sauvegardes
//--------------------------------------------------------------------------------------------------------
/*!
 *  \brief AskBupRestore
 *  la fiche qui permet de paramètrer une opération de sauvegarde ou de restauration
 *  \param restore :            true = restauration - false = backup
 *  \param pathorigin :         le dossier de stockage de l'imagerie sur le serveur
 *  \param pathdestination :    le dossier où se trouve la backup
 *  \param OKini :              le rufus.ini est sauvegardé
 *  \param OKRssces :           les fichiers ressources sont sauvegardé
 *  \param OKimages :           les fichiers images sont sauvegardés
 *  \param OKvideos :           les fichiers videos sont sauvegardés
 *
 */
void Procedures::AskBupRestore(bool Restore, QString pathorigin, QString pathdestination, bool OKini, bool OKRssces, bool OKimages, bool OKvideos, bool OKfactures)
{
    QMap<QString, qint64>      DataDir;
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
        gAskBupRestore->dlglayout()->insertLayout(0, layini);

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
        gAskBupRestore->dlglayout()->insertLayout(0, layRssces);
    }
    if (OKvideos)
    {
        // taille du dossier video ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + NOMDIR_VIDEOS);
        VideosSize = DataDir["Size"]/1024/1024;
        if (VideosSize> 0)
        {
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
            lblvolvid->setText(Utils::getExpressionSize(VideosSize));
            layVideos->addWidget(lblvolvid);
            gAskBupRestore->dlglayout()->insertLayout(0, layVideos);
            connect(Videoschk, SIGNAL(clicked(bool)), this,    SLOT(Slot_CalcTimeBupRestore()));
        }
    }
    if (OKimages)
    {
        // taille du dossier Images ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + NOMDIR_IMAGES);
        ImagesSize = DataDir["Size"]/1024/1024;
        if (ImagesSize > 0)
        {
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
            lblvolimg->setText(Utils::getExpressionSize(ImagesSize));
            layImges->addWidget(lblvolimg);
            gAskBupRestore->dlglayout()->insertLayout(0, layImges);
            connect(Imgeschk, SIGNAL(clicked(bool)), this,    SLOT(Slot_CalcTimeBupRestore()));
        }
    }
    if (OKfactures)
    {
        // taille du dossier Factures ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + NOMDIR_FACTURES);
        FacturesSize = DataDir["Size"]/1024/1024;
        if (FacturesSize > 0)
        {
            QHBoxLayout *layFctures = new QHBoxLayout;
            UpLabel *labelmges = new UpLabel();
            labelmges->setVisible(false);
            labelmges->setFixedSize(labelsize, labelsize);
            layFctures->addWidget(labelmges);
            UpCheckBox *Fctureschk  = new UpCheckBox();
            Fctureschk->setText("Factures");
            Fctureschk->setEnabled(OKimages || !Restore);
            Fctureschk->setChecked(OKimages || !Restore);
            Fctureschk->setAccessibleDescription("factures");
            layFctures->addWidget(Fctureschk);
            layFctures->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            UpLabel *lblvolfct = new UpLabel();
            lblvolfct->setText(Utils::getExpressionSize(FacturesSize));
            layFctures->addWidget(lblvolfct);
            gAskBupRestore->dlglayout()->insertLayout(0, layFctures);
            connect(Fctureschk, SIGNAL(clicked(bool)), this,    SLOT(Slot_CalcTimeBupRestore()));
        }
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
    lblvolbase->setText(Utils::getExpressionSize(BaseSize));
    layBDD->addWidget(lblvolbase);
    gAskBupRestore->dlglayout()->insertLayout(0, layBDD);


    QHBoxLayout *layResume = new QHBoxLayout;
    labelResume = new UpLabel();
    layResume->addWidget(labelResume);
    gAskBupRestore->dlglayout()->insertLayout(gAskBupRestore->dlglayout()->count()-1, layResume);

    QHBoxLayout *layVolumeLibre = new QHBoxLayout;
    labelVolumeLibre = new UpLabel();
    layVolumeLibre->addWidget(labelVolumeLibre);
    gAskBupRestore->dlglayout()->insertLayout(gAskBupRestore->dlglayout()->count()-1, layVolumeLibre);

    connect(BDDchk, SIGNAL(clicked(bool)), this,    SLOT(Slot_CalcTimeBupRestore()));

    gAskBupRestore->setFixedWidth(400);
    gAskBupRestore->AjouteLayButtons(UpDialog::ButtonOK);
    connect(gAskBupRestore->OKButton,    SIGNAL(clicked(bool)), gAskBupRestore, SLOT(accept()));
    Slot_CalcTimeBupRestore();
}

bool Procedures::Backup(QString dirSauv, bool OKBase, QString NomDirStockageImagerie, bool OKImages, bool OKVideos, bool OKFactures)
{
    if (QDir(NomDirStockageImagerie).exists())
    {
        Utils::cleanfolder(NomDirStockageImagerie + NOMDIR_IMAGES);
        Utils::cleanfolder(NomDirStockageImagerie + NOMDIR_FACTURES);
        Utils::cleanfolder(NomDirStockageImagerie + NOMDIR_VIDEOS);
    }
    else
    {
        OKImages = false;
        OKVideos = false;
        OKFactures = false;
    }

    Message(tr("Sauvegarde en cours"),3000,false);
    connexion = false;
    emit ConnectTimers();

    bool result = true;
    if (OKBase)
    {
        QFile precBup(QDir::homePath() + SCRIPTBACKUPFILE);
        bool b = precBup.exists();
        DefinitScriptBackup(dirSauv, NomDirStockageImagerie, OKImages, OKVideos);
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
        {
            QString NomDirDestination;
            QVariantList dirdata = db->getFirstRecordFromStandardSelectSQL("select DirBkup from " NOM_TABLE_PARAMSYSTEME, ok);
            if (ok && dirdata.size()>0)
                NomDirDestination = dirdata.at(0).toString();
            if (!QDir(NomDirDestination).exists())
            {
                QFile::remove(QDir::homePath() + SCRIPTBACKUPFILE);
                return false;
            }
            DefinitScriptBackup(NomDirDestination, NomDirStockageImagerie);
        }
        else
            QFile::remove(QDir::homePath() + SCRIPTBACKUPFILE);
        result = (a==0);
    }
    else
    {
        QString dest = dirSauv + "/" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmm");
        QDir dirdest;
        if (OKImages || OKVideos)
            dirdest.mkdir(dest);
        if (OKImages)
        {
            QString Msg = (tr("Sauvegarde des fichiers d'imagerie\n")
                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
            Message(Msg, 3000);
            QProcess::execute("cp -R " + NomDirStockageImagerie + NOMDIR_IMAGES + " " + dest);
            Message(tr("Fichiers d'imagerie sauvegardés!"), 3000, false);
        }
        if (OKFactures)
        {
            QString Msg = (tr("Sauvegarde des factures\n")
                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
            Message(Msg, 3000);
            QProcess::execute("cp -R " + NomDirStockageImagerie + NOMDIR_FACTURES + " " + dest);
            Message(tr("Fichiers factures sauvegardés!"), 3000, false);
        }
        if (OKVideos)
        {
            QString Msg = (tr("Sauvegarde des fichiers videos\n")
                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
            Message(Msg, 3000);
            QProcess::execute("cp -R " + NomDirStockageImagerie + NOMDIR_VIDEOS + " " + dest);
            Message(tr("Fichiers video sauvegardés!"), 3000, false);
        }
    }
    connexion = true;
    if (OKImages)
        Utils::cleanfolder(dirSauv + NOMDIR_IMAGES);
    if (OKFactures)
        Utils::cleanfolder(dirSauv + NOMDIR_FACTURES);
    if (OKVideos)
        Utils::cleanfolder(dirSauv + NOMDIR_VIDEOS);
    emit ConnectTimers();
    UpMessageBox::Watch(Q_NULLPTR, tr("Sauvegarde terminée"));
    return result;
}


void Procedures::BackupWakeUp(QString NomDirDestination, QTime timebkup, Days days)
{

    if (QTime::currentTime().toString("HH:mm:ss") == timebkup.toString("HH:mm")+ ":00")
    {
        int day = QDate::currentDate().dayOfWeek();
        Day daybkup = Lundi;
        if (day==2)      daybkup = Mardi;
        else if (day==3) daybkup = Mercredi;
        else if (day==4) daybkup = Jeudi;
        else if (day==5) daybkup = Vendredi;
        else if (day==6) daybkup = Samedi;
        else if (day==7) daybkup = Dimanche;
        if (!days.testFlag(daybkup))
            return;
        bool verifposteconnectes = VerifAutresPostesConnectes(false);
        if (verifposteconnectes)
        {
            bool full = true;
            ImmediateBackup(NomDirDestination, !verifposteconnectes, full);
        }
    }
}

void Procedures::DefinitScriptBackup(QString NomDirDestination,QString NomDirStockageImagerie, bool AvecImages, bool AvecVideos)
{
    if (!QDir(NomDirDestination).exists())
        return;
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
    if (QDir(NomDirStockageImagerie).exists())
    {
        scriptbackup += "DIR_IMAGES=\"" + NomDirStockageImagerie + NOMDIR_IMAGES + "\"";
        scriptbackup += "\n";
        scriptbackup += "DIR_FACTURES=\"" + NomDirStockageImagerie + NOMDIR_FACTURES + "\"";
        scriptbackup += "\n";
        scriptbackup += "DIR_VIDEOS=\"" + NomDirStockageImagerie + NOMDIR_VIDEOS + "\"";
        scriptbackup += "\n";
    }
    //# Rufus.ini
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
    QString cheminmysql;
#ifdef Q_OS_MACX
    cheminmysql = "/usr/local/mysql/bin";           // Depuis HighSierra on ne peut plus utiliser + Dir.absolutePath() + NOMDIR_LIBS2 - le script ne veut pas utiliser le client mysql du package (???)
#endif
#ifdef Q_OS_LINUX
    cheminmysql = "/usr/bin";
#endif
    scriptbackup += "MYSQL=" + cheminmysql;
    scriptbackup += "/mysql";
    scriptbackup += "\n";
    scriptbackup += "MYSQLDUMP=" + cheminmysql;
    scriptbackup += "/mysqldump";
    scriptbackup += "\n";

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
    scriptbackup += "cp -R $DIR_RESSOURCES $BACKUP_DIR/$DATE/Ressources";
    scriptbackup += "\n";
    if (QDir(NomDirStockageImagerie).exists())
    {
        // copie les fichiers image
        if (AvecImages)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR/Images";
            scriptbackup += "\n";
            scriptbackup += "mkdir -p $BACKUP_DIR/Factures";
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_IMAGES $BACKUP_DIR";
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_FACTURES $BACKUP_DIR";
            scriptbackup += "\n";
        }
        // copie les fichiers video
        if (AvecVideos)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR/Videos";
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_VIDEOS $BACKUP_DIR";
            scriptbackup += "\n";
        }
    }
    // copie Rufus.ini
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

void Procedures::DefinitScriptRestore(QStringList ListNomFiles)
{
    /*
#!/bin/bash
MYSQL_USER="Admin"
MYSQL_PASSWORD="bob"
MYSQL_PORT="3306"
MYSQL=/usr/local/mysql/bin/mysql
$MYSQL -u $MYSQL_USER -p$MYSQL_PASSWORD -h localhost -P $MYSQL_PORT < File1"
$MYSQL -u $MYSQL_USER -p$MYSQL_PASSWORD -h localhost -P $MYSQL_PORT < File2"
$MYSQL -u $MYSQL_USER -p$MYSQL_PASSWORD -h localhost -P $MYSQL_PORT < File3"
...etc...
    */
    // élaboration du script de restore
    QString scriptrestore = "#!/bin/bash";
    scriptrestore += "\n";
    QString cheminmysql;
#ifdef Q_OS_MACX
    cheminmysql = "/usr/local/mysql/bin";           // Depuis HighSierra on ne peut plus utiliser + Dir.absolutePath() + NOMDIR_LIBS2 - le script ne veut pas utiliser le client mysql du package (???)
#endif
#ifdef Q_OS_LINUX
    cheminmysql = "/usr/bin";
#endif
    scriptrestore += "MYSQL=" + cheminmysql;
    scriptrestore += "/mysql";
    scriptrestore += "\n";
    for (int i=0; i<ListNomFiles.size(); i++)
    if (QFile(ListNomFiles.at(i)).exists())
    {
        scriptrestore += "$MYSQL -u " + m_userConnected->getLogin() +  " -p" +  m_userConnected->getPassword() + " -h localhost -P " + QString::number(db->I()->getDataBase().port()) + " < " + ListNomFiles.at(i);
        scriptrestore += "\n";
    }
    if (QFile::exists(QDir::homePath() + SCRIPTRESTOREFILE))
        QFile::remove(QDir::homePath() + SCRIPTRESTOREFILE);
    QFile fbackup(QDir::homePath() + SCRIPTRESTOREFILE);
    if (fbackup.open(QIODevice::ReadWrite))
    {
        QTextStream out(&fbackup);
        out << scriptrestore ;
        fbackup.close();
    }
}

/*!
 *  \brief ImmediateBackup()
 *  lance une sauvegarde immédiate de la base
 */
bool Procedures::ImmediateBackup(QString dirSauv, bool verifposteconnecte, bool full)
{
    if (verifposteconnecte)
        if (!VerifAutresPostesConnectes())
            return false;

    QString NomDirStockageImagerie ("");
    QString NomDirDestination ("");
    QVariantList dirdata = db->getFirstRecordFromStandardSelectSQL("select dirimagerie, DirBkup from " NOM_TABLE_PARAMSYSTEME, ok);
    if (ok && dirdata.size()>0)
    {
        NomDirStockageImagerie = dirdata.at(0).toString();
        NomDirDestination = dirdata.at(1).toString();
    }
    if (dirSauv == "")
    {
        QString dirSauv = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                            tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base") + "\n" + tr("Le nom de dossier ne doit pas contenir d'espace"),
                                                            (QDir(NomDirDestination).exists()? NomDirDestination : QDir::homePath()));
        if (dirSauv.contains(" "))
            UpMessageBox::Watch(Q_NULLPTR, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
        if (dirSauv == "" || dirSauv.contains(" "))
            return false;
        NomDirDestination = dirSauv;
    }
    else
        NomDirDestination = dirSauv;
    if (!QDir(NomDirDestination).exists())
        return false;
    bool OKbase     = false;
    bool OKImages   = false;
    bool OKVideos   = false;
    bool OKFactures = false;
    if (full)
    {
        OKbase = true;
        OKImages = true && QDir(NomDirStockageImagerie).exists();
        OKVideos = true && QDir(NomDirStockageImagerie).exists();
        OKFactures = true && QDir(NomDirStockageImagerie).exists();
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
            else if (listchk.at(i)->accessibleDescription() == "factures")
                OKFactures = listchk.at(i)->isChecked();
        }
    }
    if (!OKbase && !OKImages && !OKVideos && !OKFactures)
        return false;
    return Backup(NomDirDestination, OKbase, NomDirStockageImagerie, OKImages, OKVideos, OKFactures);
}

/*!
 *  \brief InitBackupAuto()
 *  lance une sauvegarde immédiate de la base
 */
void Procedures::InitBackupAuto()
{
    if (db->getMode() != DataBase::Poste)
        return;
    Days days;
    QString dirdestination ("");
    QString dirimagerie("");
    QTime timebackup = QTime(0,0,0);

    QString reqBkup = "select LundiBkup, MardiBkup, MercrediBkup, JeudiBkup, VendrediBkup, SamediBkup, DimancheBkup, HeureBkup, DirBkup, DirImagerie from " NOM_TABLE_PARAMSYSTEME;
    QVariantList Bkupdata = db->getFirstRecordFromStandardSelectSQL(reqBkup, ok);
    if (ok && Bkupdata.size()>0)
    {
        dirdestination  = Bkupdata.at(8).toString();
        timebackup      = Bkupdata.at(7).toTime();
        if (Bkupdata.at(0).toInt()==1) days.setFlag(Procedures::Lundi);
        if (Bkupdata.at(1).toInt()==1) days.setFlag(Procedures::Mardi);
        if (Bkupdata.at(2).toInt()==1) days.setFlag(Procedures::Mercredi);
        if (Bkupdata.at(3).toInt()==1) days.setFlag(Procedures::Jeudi);
        if (Bkupdata.at(4).toInt()==1) days.setFlag(Procedures::Vendredi);
        if (Bkupdata.at(5).toInt()==1) days.setFlag(Procedures::Samedi);
        if (Bkupdata.at(6).toInt()==1) days.setFlag(Procedures::Dimanche);
    }
        ParamAutoBackup(dirdestination, dirimagerie, timebackup, days);
}

void Procedures::EffaceAutoBackup()
{
    QString Base = db->getBase();
    if (Base == "")
        return;
    db->StandardSQL("update " NOM_TABLE_PARAMSYSTEME " set HeureBkup = '',"
                                                     " DirBkup = '',"
                                                     " LundiBkup = NULL,"
                                                     " MardiBkup = NULL,"
                                                     " MercrediBkup = NULL,"
                                                     " JeudiBkup = NULL,"
                                                     " VendrediBkup = NULL,"
                                                     " SamediBkup = NULL,"
                                                     " DimancheBkup = NULL");
    EffaceScriptsBackup();
}

void Procedures::EffaceScriptsBackup()
{
#ifdef Q_OS_LINUX
    gTimerBackup.stop();
#endif
#ifdef Q_OS_MACX
    QString unload  = "bash -c \"/bin/launchctl unload \"" + QDir::homePath();
    unload += SCRIPTPLISTFILE "\"\"";
    QProcess dumpProcess(parent());
    dumpProcess.start(unload);
    dumpProcess.waitForFinished();
    if (QFile::exists(QDir::homePath() + SCRIPTPLISTFILE))
        QFile::remove(QDir::homePath() + SCRIPTPLISTFILE);
#endif
    if (QFile::exists(QDir::homePath() + SCRIPTBACKUPFILE))
        QFile::remove(QDir::homePath() + SCRIPTBACKUPFILE);
}

void Procedures::ParamAutoBackup(QString dirdestination, QString dirimagerie, QTime timebackup, Days days)
{
    if (!VerifParamBackup(dirdestination, timebackup, days))
    {
        EffaceScriptsBackup();
        return;
    }
    DefinitScriptBackup(dirdestination, dirimagerie);
//#ifdef Q_OS_LINUX
    gTimerBackup.stop();
    gTimerBackup.start(1000);
    connect(&gTimerBackup, &QTimer::timeout, this, [=] {BackupWakeUp(dirdestination, timebackup, days);});
//#endif
#ifdef Q_OS_MACX
    // elaboration de rufus.bup.plist
    QString heure   = timebackup.toString("H");
    QString minute  = timebackup.toString("m");
    QString jourprg;
    QString a = (days>1? "\t": "");
    if (days>1)
        jourprg += "\t\t<array>\n";

    QString debutjour =
        a + "\t\t<dict>\n" +
        a + "\t\t\t<key>Weekday</key>\n" +
        a + "\t\t\t<integer>";
    QString finjour =
        "</integer>\n" +
        a + "\t\t\t<key>Hour</key>\n" +
        a + "\t\t\t<integer>"+ heure + "</integer>\n" +
        a + "\t\t\t<key>Minute</key>\n" +
        a + "\t\t\t<integer>" + minute + "</integer>\n" +
        a + "\t\t</dict>\n";
    if (days.testFlag(Procedures::Lundi))
        jourprg += debutjour + "1" + finjour;
    if (days.testFlag(Procedures::Mardi))
        jourprg += debutjour + "2" + finjour;
    if (days.testFlag(Procedures::Mercredi))
        jourprg += debutjour + "3" + finjour;
    if (days.testFlag(Procedures::Jeudi))
        jourprg += debutjour + "4" + finjour;
    if (days.testFlag(Procedures::Vendredi))
        jourprg += debutjour + "5" + finjour;
    if (days.testFlag(Procedures::Samedi))
        jourprg += debutjour + "6" + finjour;
    if (days.testFlag(Procedures::Dimanche))
        jourprg += debutjour + "7" + finjour;
    if (days>1)
        jourprg += "\t\t</array>\n";

    QString plist = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                    "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
                    "<plist version=\"1.0\">\n"
                        "\t<dict>\n"
                            "\t\t<key>Label</key>\n"
                            "\t\t<string>rufus.backup</string>\n"
                            "\t\t<key>disabled</key>\n"
                            "\t\t<false/>\n"
                            "\t\t<key>ProgramArguments</key>\n"
                            "\t\t<array>\n"
                                "\t\t\t<string>bash</string>\n"
                                "\t\t\t<string>" + QDir::homePath() + SCRIPTBACKUPFILE + "</string>\n"
                            "\t\t</array>\n"
                            "\t\t<key>StartCalendarInterval</key>\n"
                            + jourprg +
                        "\t</dict>\n"
                    "</plist>\n";
    if (QFile::exists(QDir::homePath() + SCRIPTPLISTFILE))
        QFile::remove(QDir::homePath() + SCRIPTPLISTFILE);
    QFile fplist(QDir::homePath() + SCRIPTPLISTFILE);
    if (fplist.open(QIODevice::ReadWrite))
    {
        QTextStream out(&fplist);
        out << plist;
        fplist.close();
    }

    // relance du launchd
    QString unload  = "bash -c \"/bin/launchctl unload \"" + QDir::homePath();
    unload += SCRIPTPLISTFILE "\"\"";
    QString load    = "bash -c \"/bin/launchctl load \""   + QDir::homePath();
    load += SCRIPTPLISTFILE "\"\"";
    QProcess dumpProcess(parent());
    dumpProcess.start(unload);
    dumpProcess.waitForFinished();
    dumpProcess.start(load);
    dumpProcess.waitForFinished();
#endif
    //programmation de l'effacement du contenu de la table ImagesEchange
    db->StandardSQL("Use " NOM_BASE_IMAGES);
    db->StandardSQL("DROP EVENT IF EXISTS VideImagesEchange");
    QString req =   "CREATE EVENT VideImagesEchange "
            "ON SCHEDULE EVERY 1 DAY STARTS '2018-03-23 " + timebackup.addSecs(-60).toString("HH:mm:ss") + "' "
            "DO DELETE FROM " NOM_TABLE_ECHANGEIMAGES;
    db->StandardSQL(req);
    //programmation de l'effacement des pdf et jpg contenus dans Factures
    db->StandardSQL("Use " NOM_BASE_COMPTA);
    db->StandardSQL("DROP EVENT IF EXISTS VideFactures");
    req =   "CREATE EVENT VideFactures "
            "ON SCHEDULE EVERY 1 DAY STARTS '2018-03-23 " + timebackup.addSecs(-60).toString("HH:mm:ss") + "' "
            "DO UPDATE " NOM_TABLE_FACTURES " SET jpg = null, pdf = null";
    db->StandardSQL(req);
}

bool Procedures::VerifParamBackup(QString dirdestination, QTime time, Days days)
{
    if (!QDir(dirdestination).exists())
        return false;
    if (!time.isValid())
        return false;
    if (days<1)
        return false;
    return true;
}
//--------------------------------------------------------------------------------------------------------
// fin sauvegardes
//--------------------------------------------------------------------------------------------------------

//TODO : à déplacer
/*---------------------------------------------------------------------------------
    Retourne le corps du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::ImpressionCorps(QString text, bool ALD)
{
    QString Corps;
    QString nomModeleCorpsImpression;
    Utils::convertHTML(text);
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

//TODO : à déplacer
/*---------------------------------------------------------------------------------
    Retourne l'entête du document à imprimer
-----------------------------------------------------------------------------------*/
QMap<QString, QString> Procedures::ImpressionEntete(QDate date, User *user)
{
    QMap<QString, QString> EnteteMap;
    QString Entete;
    QString nomModeleEntete;
    int idparent = -1;
    bool rplct = false;
    /*
     * Pour rédiger l'entête, on utilise
     * si l'utilisateur n'est pas remplaçant, les coordonnées de l'utilisateur
     * et si l'utisateur est remplaçant, celles de son parent avec le nom rayé, suivi du nom de l'utilisateur
     */

    // si le user est un remplaçant on essaie de savoir qui il remplace, son parent
    /*
        1 = liberal
        2 = salarie
        3 = retrocession (remplaçant)
        4 = pas de comptabilite
    */
    if (user && user->getEnregHonoraires() == 3)
    {
        rplct = true;
        if (user->id() == m_userConnected->getIdUserActeSuperviseur())
        {
            // si le user rplct à imprimer est le superviseur du user courant, on récupère le parent du user courant
            idparent = m_userConnected->getIdUserParent();
        }
        else
        {
            // si le user rplct à imprimer n'est pas le superviseur du user courant, on cherche son parent
            QString reqrp = "select userparent "
                            "from " NOM_TABLE_USERSCONNECTES
                            " where usersuperviseur = " + QString::number(user->id());
            QVariantList userdata = db->getFirstRecordFromStandardSelectSQL(reqrp, ok);
            if (userdata.size()>0)                // le user est connecté, on cherche qui il remplace - son parent
                idparent = userdata.at(0).toInt();
            else                                // le user n'est pas connecté on demande quel est son parent
            {
                QVariantList soigndata = db->getFirstRecordFromStandardSelectSQL("select soignant from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(user->id()), ok);
                QString req   = "select iduser, userlogin from " NOM_TABLE_UTILISATEURS
                        " where (userenreghonoraires = 1 or userenreghonoraires = 2)"
                        " and iduser <> " + QString::number(user->id()) +
                        " and soignant = " + soigndata.at(0).toString() +
                        " and userdesactive is null";
                //qDebug() << req;
                QList<QVariantList> soignlist = db->StandardSelectSQL(req,ok);
                if (soignlist.size() == 1)               // une seule réponse, on la récupère
                    idparent   = soignlist.at(0).at(0).toInt();
                else                                // plusieurs réponses possibles, on va demander qui est le parent de ce remplaçant....
                {
                    gAskUser                = new UpDialog();
                    gAskUser                ->AjouteLayButtons();
                    QGroupBox*boxparent     = new QGroupBox();
                    gAskUser->dlglayout()   ->insertWidget(0,boxparent);
                    boxparent               ->setAccessibleName("Parent");
                    QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + user->getLogin() + "?";
                    boxparent               ->setTitle(lblUsrParent);

                    QFontMetrics fm         = QFontMetrics(qApp->font());
                    int hauteurligne        = int(fm.height()*1.6);
                    boxparent               ->setFixedHeight(((soignlist.size() + 1)*hauteurligne)+5);
                    QVBoxLayout *vbox       = new QVBoxLayout;
                    for (int i=0; i<soignlist.size(); i++)
                    {
                        QRadioButton *pradiobutt = new QRadioButton(boxparent);
                        pradiobutt  ->setText(soignlist.at(i).at(1).toString());
                        pradiobutt  ->setAccessibleName(soignlist.at(i).at(0).toString());
                        pradiobutt  ->setChecked(i==0);
                        vbox        ->addWidget(pradiobutt);
                    }
                    vbox     ->setContentsMargins(8,0,8,0);
                    boxparent->setLayout(vbox);
                    gAskUser ->setModal(true);
                    gAskUser->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

                    connect(gAskUser->OKButton, &QPushButton::clicked, gAskUser, &UpDialog::accept);

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
    for (int i = 1; i<3; i++)//TODO : ??? pourquoi 3 - reponse: comme ça, pour pas mettre i==2....
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
        Entete.replace("{{POLICE}}", qApp->font().family());
        if (rplct)
        {
            User *userRemp = Datas::I()->users->getById(idparent, ItemsList::LoadDetails);
            if(userRemp && userRemp->getTitre().size())
                Entete.replace("{{TITREUSER}}", "<s>" + userRemp->getTitre() + " " + userRemp->getPrenom() + " " + userRemp->getNom() + "</s> "
                                                "<font color=\"darkblue\">" + tr ("remplacé par") + " "
                                                + user->getTitre() + " " + user->getPrenom() + " " + user->getNom())
                               + "</font>";
            else
                Entete.replace("{{TITREUSER}}", "<s>" + userRemp->getPrenom() + " " + userRemp->getNom() + " </s> "
                                                "<font color=\"red\">" + tr ("remplacé par") + " "
                                                + user->getPrenom() + " " + user->getNom())
                               + "</font>";
        }
        else
        {
            if (user->getTitre() != "")
                Entete.replace("{{TITREUSER}}", user->getTitre() + " " + user->getPrenom() + " " + user->getNom());
            else
                Entete.replace("{{TITREUSER}}", user->getPrenom() + " " + user->getNom());
        }
        if(user->getNoSpecialite() != 0)
            Entete.replace("{{SPECIALITE}}", QString::number(user->getNoSpecialite()) + " " + user->getSpecialite());
        else
            Entete.replace("{{SPECIALITE}}", user->getSpecialite());

        QString adresse ="";
        int nlignesadresse = 0;
        if (user != m_userConnected)
            user->setSite(db->loadSitesByUser(user->id()).at(0)); //TODO ça ne va pas parce qu'on prend arbitrairement la première adreesse
        if( user->getSite()->nom().size() )
        {
            nlignesadresse  ++;
            adresse         += user->getSite()->nom();
        }
        if (user->getSite()->adresse1() != "" || user->getSite()->adresse2() != "")
        {
            nlignesadresse  ++;
            if (nlignesadresse >0)
                adresse += "<br />";
            if (user->getSite()->adresse1() != "" && user->getSite()->adresse2() != "")
                adresse += user->getSite()->adresse1() + " - " + user->getSite()->adresse2();
            else
                adresse += user->getSite()->adresse1() + user->getSite()->adresse2();
        }
        Entete.replace("{{ADRESSE}}", adresse);
        Entete.replace("{{CPVILLE}}", QString::number(user->getSite()->codePostal()) + " " + user->getSite()->ville().toUpper());
        Entete.replace("{{TEL}}", "Tél. " + user->getSite()->telephone());
        if (nlignesadresse==2)
            Entete.replace("{{LIGNESARAJOUTER}}", "<span style=\"font-size:5pt;\"> <br /></span>");
        else
            Entete.replace("{{LIGNESARAJOUTER}}", "");

        QString NumSS = "";
        if( user->getNumCO().size() )
        {
            NumSS = user->getNumCO();
            if( user->getNumPS() > 0 ) NumSS += " - ";
        }
        if (user->getNumPS() > 0) NumSS += "RPPS " + QString::number(user->getNumPS());
        Entete.replace("{{NUMSS}}", NumSS);
        Entete.replace("{{DATE}}", user->getSite()->ville()  + tr(", le ") + date.toString(tr("d MMMM yyyy")));

        (i==1? EnteteMap["Norm"] = Entete : EnteteMap["ALD"] = Entete);
    }
    return EnteteMap;
}

//TODO : à déplacer
/*---------------------------------------------------------------------------------
    Retourne le pied du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::ImpressionPied(User *user, bool lunettes, bool ALD)
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
        if( user->isAGA() )
            baPied.replace("{{AGA}}","Membre d'une association de gestion agréée - Le règlement des honoraires par chèque ou carte de crédit est accepté");
        else
            baPied.replace("{{AGA}}","");
        Pied = baPied;
    }
    return Pied;
}

//TODO : à déplacer
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
        a = TexteAImprimer->preview(Etat->document(), QDir::homePath() + NOMFIC_PDF, "");
    else
    {
        if (!AvecChoixImprimante)
            TexteAImprimer->setPrinterName(gnomImprimante);
        a = TexteAImprimer->print(Etat->document(), QDir::homePath() + NOMFIC_PDF, "", AvecChoixImprimante);
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

bool Procedures::Imprime_pdf(QTextEdit *Etat, QString EnTete, QString Pied, QString nomfichier, QString nomdossier)
{
    bool a = false;
    if (nomdossier == "")
        nomdossier = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
    QDir DirDest(nomdossier);
    if (DirDest.exists())
        DirDest.rmdir(nomdossier);
    Utils::mkpath(nomdossier);
    QString nomficpdf = nomdossier + "/" + nomfichier;

    TextPrinter *TexteAImprimer = new TextPrinter();
    Pied.replace("{{DUPLI}}","");

    TexteAImprimer->setFooterSize(TaillePieddePage());
    TexteAImprimer->setHeaderText(EnTete);
    TexteAImprimer->setHeaderSize(TailleEnTete());
    TexteAImprimer->setFooterText(Pied);
    TexteAImprimer->setTopMargin(TailleTopMarge());


    TexteAImprimer->print(Etat->document(), nomficpdf, "", false, true);
    // le paramètre true de la fonction print() génère la création du fichier pdf nomficpdf et pas son impression
    QFile filepdf(nomficpdf);
    if (!filepdf.open( QIODevice::ReadOnly ))
        UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier:\n") + nomficpdf, tr("Impossible d'enregistrer l'impression dans la base"));
    else
        a = true;
    filepdf.close ();
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

QMap<QString,QVariant> Procedures::CalcImage(int idimpression, QString typedoc, bool imagerie, bool afficher)
{
    /* Cette fonction sert à stocker dans un QByteArray le contenu des documents d'imagerie ou des courriers émis par le logiciel pour pouvoir les afficher
     * la fonction est appelée par Slot_AfficheDoc(), on utilise la table impressions
     *      pour afficher un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour afficher un document d'imagerie stocké directement dans la base, dans la table impressions - on va extraire le ByteArray directement de la base, de la table impressions
     * la fonction est applée par ImprimeDoc() - on utilise la table echangeimages
     *      pour imprimer un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour imprimer un document d'imagerie stocké dans la table echangeimages - on va extraire le ByteArray directement de la base de la table echangeimages
     * la fonction renvoie un QMap<QString,QVariant> result
     * result["type"] est un QString qui donne le type de document, jpg ou pdf
     * result["ba"] est un QByteArray qui stocke le contenu du fichier
    */
    DocExterne *docmt = Q_NULLPTR;
    Depense *dep = Q_NULLPTR;
    QString iditem;
    QString date ("");
    QString sstitre;
    QString imgs;
    QString typedocmt ("");
    QString soustypedocmt ("");
    QString objet ("");
    QString filename = "";

    if (typedoc != FACTURE)
    {
        docmt = Datas::I()->docsexternes->getById(idimpression);
        iditem = QString::number(idimpression);
        date = docmt->date().toString(tr("d-M-yyyy"));
        typedocmt = docmt->typedoc();
        soustypedocmt = docmt->soustypedoc();
        filename = docmt->lienversfichier();
    }
    else
    {
        dep = Datas::I()->depenses->getById(idimpression);
        iditem = QString::number(dep->idfacture());
        date = dep->date().toString(tr("d-M-yyyy"));
        objet = dep->objet();
        filename = dep->lienfacture();
    }
    QMap<QString,QVariant> result;
    QByteArray ba;
    QLabel inflabel;
    result["type"]    = "";
    result["ba"]      = QByteArray("");
    result["lien"]    = "";
    if (imagerie)                                                  // il s'agit d'un fichier image
    {
        if (afficher)                               // si on veut afficher une image, on la charge dans une table SQL
                                                    // pour pouvoir véhiculer son contenu dans le tunnel SQL et profiter du crypatge en cas d'accès distant
        {
            if (typedoc != FACTURE)
                sstitre = "<font color='magenta'>" + date + " - " + typedocmt + " - " + soustypedocmt + "</font>";
            else
                sstitre = "<font color='magenta'>" + date + " - " + objet + "</font>";
            inflabel   .setText(sstitre);
            if (filename != "")
            {
                QString filesufx;
                if (filename.contains("."))
                {
                    QStringList lst = filename.split(".");
                    filesufx        = lst.at(lst.size()-1);
                }
                QString sfx = (filesufx == PDF? PDF : JPG);
                if (typedoc != FACTURE)
                    imgs = "select idimpression from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + iditem + " and (pdf is not null or jpg is not null)";
                else
                    imgs = "select idfacture from " NOM_TABLE_FACTURES " where idfacture = " + iditem + " and (pdf is not null or jpg is not null)";
                //qDebug() << imgs;
                QList<QVariantList> listid = db->StandardSelectSQL(imgs, ok);
                if (!ok)
                    UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'accéder à la table ") + (typedoc != FACTURE? NOM_TABLE_ECHANGEIMAGES : NOM_TABLE_FACTURES));
                if (listid.size()==0)
                {
                    if (typedoc != FACTURE)
                    {
                        if (docmt != Q_NULLPTR)
                        {
                            db->StandardSQL("delete from " NOM_TABLE_ECHANGEIMAGES
                                            " where idimpression = " + iditem +
                                            " and facture is null");
                            QString req = "INSERT INTO " NOM_TABLE_ECHANGEIMAGES " (idimpression, " + sfx + ", compression)"
                                          " VALUES (" +
                                          iditem + ", " +
                                          " LOAD_FILE('" + Utils::correctquoteSQL(DirImagerieServeur() + NOMDIR_IMAGES + Utils::correctquoteSQL(filename)) + "'), " +
                                          QString::number(docmt->compression()) + ")";
                            db->StandardSQL(req);
                        }
                    }
                    else
                    {
                        db->StandardSQL("delete from " NOM_TABLE_ECHANGEIMAGES
                                                             " where idimpression = " + iditem +
                                                             " and facture = 1");
                        QString req = "INSERT INTO " NOM_TABLE_ECHANGEIMAGES " (idimpression, " + sfx + ", facture) "
                                      "VALUES (" +
                                      iditem + ", " +
                                      " LOAD_FILE('" + Utils::correctquoteSQL(DirImagerieServeur() + NOMDIR_FACTURES + Utils::correctquoteSQL(filename)) + "'), " +
                                      "1)";
                        db->StandardSQL(req);
                    }
                }
            }
        }
        // On charge ensuite le contenu des champs longblob des tables concernées en mémoire pour les afficher
        QList<QVariantList> listimpr;
        if (typedoc != FACTURE)
        {
            listimpr = db->StandardSelectSQL("select pdf, jpg, compression  from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + iditem + " and facture is null"
                                                                  , ok
                                                                  , tr("Impossible d'accéder à la table ") + NOM_TABLE_ECHANGEIMAGES);
            if (!ok)
            {
                return result;
            }
            if (listimpr.size()==0)                             // le document n'est pas dans echangeimages, on va le chercher dans impressions
            {
                listimpr = db->StandardSelectSQL("select pdf, jpg, compression  from " NOM_TABLE_IMPRESSIONS " where idimpression = " + iditem
                                                                      , ok
                                                                      , tr("Impossible d'accéder à la table ") + NOM_TABLE_IMPRESSIONS);
            }
        }
        else
        {
            listimpr = db->StandardSelectSQL("select pdf, jpg  from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + iditem + " and facture = 1"
                                                                  , ok
                                                                  , tr("Impossible d'accéder à la table ") + NOM_TABLE_ECHANGEIMAGES);
            if (!ok)
            {
                return result;
            }
            if (listimpr.size()==0)                             // le document n'est pas dans echangeimages, on va le chercher dans factures
            {
                listimpr = db->StandardSelectSQL("select pdf, jpg  from " NOM_TABLE_FACTURES " where idfacture = " + iditem
                                                                      , ok
                                                                      , tr("Impossible d'accéder à la table ") + NOM_TABLE_FACTURES);
            }
        }

        if (listimpr.size()==0)
        {
            return result;
        }
        QVariantList impr = listimpr.at(0);
        if (impr.at(0).toByteArray().size()>0)            // c'est le champ SQL pdf de la requête qui est exploré et s'il n'est pas vide, c'est un pdf
        {
            if (typedoc != FACTURE)
            {
                if (impr.at(2).toString()=="1")
                    ba.append(qUncompress(impr.at(0).toByteArray()));
                else
                    ba.append(impr.at(0).toByteArray());
            }
            else
                ba.append(impr.at(0).toByteArray());
            result["type"]    = PDF;
        }
        else if (impr.at(1).toByteArray().size()>0)       // c'est le champ SQL jpg de la requête qui est exploré et s'il n'est pas vide, c'est un jpg
        {
            ba.append(impr.at(1).toByteArray());
            result["type"]    = JPG;
        }
        result["ba"]      = ba;
        result["lien"]    = filename;
    }
    else                                                  // il s'agit d'un document écrit, on le traduit en pdf et on l'affiche
    {
        inflabel    .setText("");
        QByteArray ba;
        QString Entete  = docmt->textentete();
        QString Corps   = docmt->textcorps();
        QString Pied    = docmt->textpied();
        QTextEdit   *Etat_textEdit = new QTextEdit;
        Etat_textEdit->setHtml(Corps);
        TextPrinter *TexteAImprimer = new TextPrinter();
        if (docmt->format() == PRESCRIPTIONLUNETTES)
            TexteAImprimer->setFooterSize(TaillePieddePageOrdoLunettes());
        else
            TexteAImprimer->setFooterSize(TaillePieddePage());
        TexteAImprimer->setHeaderText(Entete);
        TexteAImprimer->setHeaderSize(docmt->isALD()? TailleEnTeteALD() : TailleEnTete());
        TexteAImprimer->setFooterText(Pied);
        TexteAImprimer->setTopMargin(TailleTopMarge());
        QString ficpdf = QDir::homePath() + NOMFIC_PDF;
        TexteAImprimer->print(Etat_textEdit->document(), ficpdf, "", false, true);
        // le paramètre true de la fonction print() génère la création du fichier pdf NOMFIC_PDF et pas son impression
        QFile filepdf(ficpdf);
        if (!filepdf.open( QIODevice::ReadOnly ))
            UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier:\n") + ficpdf, tr("Impossible d'enregistrer l'impression dans la base"));
        ba = filepdf.readAll();
        filepdf.close ();
        result["type"]    = PDF;
        result["ba"]      = ba;
    }
    return result;
}

/* abandonné parce QWebEngine pèse beaucoup trop lourd */
void Procedures::DisplayWebPage(QUrl)
{
    /*
    QString         rep("");
    QString         geometry("PositionsFiches/PositionDisplayWebPage");
    UpDialog        *gAsk           = new UpDialog();
    QWebEngineView  *WebView        = new QWebEngineView(gAsk);
    UpSmallButton   *QwButt         = new UpSmallButton(gAsk);
    UpSmallButton   *HomeButt       = new UpSmallButton(gAsk);
    UpToolBar       *toolbar        = new UpToolBar(false, true);

    int x = qApp->desktop()->availableGeometry().width();
    int y = qApp->desktop()->availableGeometry().height();

    gAsk->setModal(true);
    gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    gAsk->setMaximumWidth(x);
    gAsk->setMaximumHeight(y);

    QwButt  ->setUpButtonStyle(UpSmallButton::QWANTBUTTON);
    HomeButt->setUpButtonStyle(UpSmallButton::HOMEBUTTON);
    gAsk->dlglayout()->insertWidget(0,WebView);
    gAsk->AjouteLayButtons();
    gAsk->AjouteWidgetLayButtons(toolbar,false);
    gAsk->AjouteWidgetLayButtons(HomeButt,false);
    gAsk->AjouteWidgetLayButtons(QwButt,false);
    connect(WebView,        &QWebEngineView::loadFinished,  this,   [=] { gAsk->setWindowTitle(WebView->page()->title());
                                                                          gAsk->setWindowIcon(WebView->page()->icon()); });
    connect(HomeButt,       &QPushButton::clicked,          this,   [=] { WebView->setUrl(QUrl(LIEN_CCAM));});
    connect(QwButt,         &QPushButton::clicked,          this,   [=] { WebView->setUrl(QUrl("https://www.qwant.com"));});
    connect(toolbar,        &UpToolBar::TBSignal,           this,   [=] { if (toolbar->action == "Précédent")   WebView->back();
                                                                     else if (toolbar->action == "Suivant")     WebView->forward();
                                                                     else if (toolbar->action == "Recharger")   WebView->reload();
                                                                     });

    connect(gAsk->OKButton, &UpPushButton::clicked,         gAsk,   &UpDialog::accept);
    gAsk->restoreGeometry(gsettingsIni->value(geometry).toByteArray());
    WebView->setUrl(webpage);
    gAsk->exec();

    gsettingsIni->setValue(geometry,gAsk->saveGeometry());
    delete gAsk;
    */
}

QString Procedures::Edit(QString txt, QString titre, bool editable, bool ConnectAuSignal)
{
    QString         rep("");
    QString         geometry("PositionsFiches/PositionEdit");
    UpDialog        *gAsk           = new UpDialog();
    UpTextEdit      *TxtEdit        = new UpTextEdit(gAsk);
    int x = qApp->desktop()->availableGeometry().width();
    int y = qApp->desktop()->availableGeometry().height();

    gAsk->setModal(true);
    gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    TxtEdit->setText(txt);
    TxtEdit->setTextInteractionFlags(editable? Qt::TextEditorInteraction : (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard));

    gAsk->setMaximumWidth(x);
    gAsk->setMaximumHeight(y);
    gAsk->setWindowTitle(titre);

    gAsk->dlglayout()->insertWidget(0,TxtEdit);

    gAsk->AjouteLayButtons();
    connect(gAsk->OKButton, SIGNAL(clicked(bool)),  gAsk,       SLOT(accept()));
    if (ConnectAuSignal)
        connect(this,       &Procedures::ModifEdit, TxtEdit,    [=](QString txt) {TxtEdit->setText(txt);});
    gAsk->restoreGeometry(gsettingsIni->value(geometry).toByteArray());

    if (gAsk->exec()>0)
        rep = TxtEdit->toHtml();
    gsettingsIni->setValue(geometry,gAsk->saveGeometry());
    delete gAsk;
    return rep;
}

void Procedures::EditHtml(QString txt)
{
    UpDialog        *gAsk           = new UpDialog();
    QLabel *lbl                     = new QLabel(gAsk);

    gAsk->setModal(true);
    gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    gAsk->setSizeGripEnabled(false);
    lbl->setStyleSheet("border: 1px solid gray;");
    lbl->setTextFormat(Qt::PlainText);
    lbl->setText(txt);

    gAsk->dlglayout()->insertWidget(0,lbl);

    gAsk->AjouteLayButtons();
    connect(gAsk->OKButton,SIGNAL(clicked(bool)),gAsk,SLOT(accept()));

    gAsk->exec();
    delete gAsk;
}

/*!
 * \brief Procedures::EditDocument
 * affiche le contenu d'un fichier image pdf ou jpg dans une fenêtre à la taille maximale pouvant être contenue dans l'écran, sans dépasser les 2/3 en largeur
 * argument QMap<QString,QVariant> doc contient 2 éléments
    . doc["ba"] = le QByteArray correspondant au contenu du fichier   = QFile(emplacementdufichier)->readAll())
    . doc["type"] = "jpg" ou "pdf" correspondant au type du fichier   = QFileInfo(emplacementdufichier)->suffix();
 * argument label le label de l'image affiché dans un QLabel magenta en bas à gauche de l'image
 * argument titre le titre de la fiche
 * argument Buttons, les boutons affichés en dessous de l'image, OKButton par défaut
 * si le bouton PrintButton est utilisé il permet d'imprimer l'image en appelant la fonction PrintDocument(QMap<QString,QVariant> doc)
 */
void Procedures::EditDocument(QMap<QString,QVariant> doc, QString label, QString titre, UpDialog::Buttons Button)
{
    UpDialog    *gAsk       = new UpDialog();
    uptable                 = new UpTableWidget(gAsk);
    inflabel                = new UpLabel(uptable);
    listimage               = uptable->AfficheDoc(doc);
    uptable ->installEventFilter(this);
    gAsk->setModal(true);
    gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    gAsk->setWindowTitle(titre);
    gAsk->dlglayout()->insertWidget(0,uptable);
    uptable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    uptable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    gAsk->AjouteLayButtons(Button);
    connect(gAsk->OKButton, &QPushButton::clicked, this, [=] {gAsk->accept();});
    connect(this, &Procedures::CloseEditDocument, gAsk, [=] {gAsk->accept();});
    if (Button.testFlag(UpDialog::ButtonPrint))
    {
        gAsk->PrintButton->setData(doc);
        connect(gAsk->PrintButton, QOverload<QVariant>::of(&UpSmallButton::clicked), [=](QVariant) {PrintDocument(doc);});
    }
    if (Button.testFlag(UpDialog::ButtonSuppr))
        connect(gAsk->SupprButton, &QPushButton::clicked, this, [=] {emit DelImage();});

    int x = qApp->desktop()->availableGeometry().width();
    int y = qApp->desktop()->availableGeometry().height();
    gAsk->setMaximumWidth(x);
    gAsk->setMaximumHeight(y);
    int topmarge    = gAsk->dlglayout()->contentsMargins().top();
    int bottommarge = gAsk->dlglayout()->contentsMargins().bottom();
    int leftmarge   = gAsk->dlglayout()->contentsMargins().left();
    int rightmarge  = gAsk->dlglayout()->contentsMargins().right();
    int spacing     = gAsk->dlglayout()->spacing();
    int hdeltaframe = 60;//gAsk->frameGeometry().height() - gAsk->height();
                            // on a un problème avec ce calcul sous mac puisque qt ne connait pas la hauteur du dock
    int wdeltaframe = gAsk->frameGeometry().width() - gAsk->width();

    int hdelta = topmarge + bottommarge + spacing + gAsk->widgetbuttons()->height();
        // la différence totale entre le hauteur de la fiche et la hauteur de la table
    int wdelta = leftmarge + rightmarge + spacing;
        // la différence totale entre la largeur de la fiche et la largeur de la table

    // les dimensions maxi de la zone de visu
    const double maxwscroll  = x*2/3 - wdelta - wdeltaframe;
    const double maxhscroll  = y - hdelta - hdeltaframe;
    // les dimensions calculées de la zone de visu
    int wtable(0), htable(0);

    const double proportion = maxwscroll/maxhscroll;
    QPixmap pix = QPixmap::fromImage(listimage.at(0).scaled(QSize(x,y),
                                           Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation));
    const double pw = pix.size().width();
    const double ph = pix.size().height();
    const double idealproportion = pw/ph;

    if (idealproportion > proportion)
    {   wtable  = int(maxwscroll);   htable  = int(wtable / idealproportion); }
    else
    {   htable  = int(maxhscroll);   wtable  = int(htable * idealproportion); }
    int w = wtable + wdelta;
    int h = htable + hdelta;
    gAsk->resize(w, h);
    uptable->resize(wtable, htable);
    int delta = 0;
    for (int i=0; i < uptable->rowCount(); i++)
    {
        UpLabel *lbl = dynamic_cast<UpLabel*>(uptable->cellWidget(i,0));
        if (lbl != Q_NULLPTR)
        {
            pix = pix.scaled(wtable- delta, htable - delta, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            lbl->setPixmap(pix);
            uptable->setRowHeight(i,htable-delta);
            uptable->setColumnWidth(0,wtable-delta);
        }
    }

    if (w > (x - gAsk->x()))
        gAsk->move(x - w, 0);

    inflabel    ->setText("<font color='magenta'>" + label + "</font>");
    QFont font = qApp->font();
    font.setPointSize(12);
    inflabel->setFont(font);
    inflabel    ->setGeometry(10,htable-40,350,25);

    gAsk->exec();
    delete gAsk;
}

/*!
 * \brief Procedures::PrintDocument
 * imprime le contenu d'un fichier image de type pdf ou jpg
 * argument QMap<QString,QVariant> doc contient 2 éléments
    . ba = le QByteArray contenant les données
    . type = jpg ou pdf
 */
bool Procedures::PrintDocument(QMap<QString,QVariant> doc)
{
    bool AvecPrevisu = false;
    QByteArray ba = doc.value("ba").toByteArray();
    if (doc.value("type").toString() == PDF)     // le document est un pdf
    {
        Poppler::Document* document = Poppler::Document::loadFromData(ba);
        if (!document || document->isLocked() || document == Q_NULLPTR)
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de charger le document"));
            delete document;
            return false;
        }
        document->setRenderHint(Poppler::Document::TextAntialiasing);
//        if (AvecPrevisu)
//        {
//            QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer);
//            connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {PrintPdf(printer, document, ok);});
//            dialog->exec();
//            delete dialog;
//            delete document;
//            return ok;
//        }
//        else
        for (int i=0; i<document->numPages() ;i++)
        {
            Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
            if (pdfPage == Q_NULLPTR) {
                UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
                delete document;
                return false;
            }
            QImage image = pdfPage->renderToImage(600,600);
            if (image.isNull()) {
                UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
                delete document;
                return false;
            }
            // ... use image ...
            if (i==0)
            {
                if (AvecPrevisu)
                {
                    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer);
                    connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(printer, image);});
                    if (dialog->exec() != QDialog::Rejected)
                        delete dialog;
                    else {
                        delete dialog;
                        delete pdfPage;
                        delete document;
                        return false;
                    }
                }
                else
                {
                    QPrintDialog *dialog = new QPrintDialog(printer);
                    if (dialog->exec() != QDialog::Rejected)
                    {
                        printer = dialog->printer();
                        Print(printer, image);
                        delete dialog;
                    }
                    else {
                        delete dialog;
                        delete pdfPage;
                        delete document;
                        return false;
                    }
                }
            }
            else
                Print(printer, image);
            delete pdfPage;
        }
        delete document;
    }
    else if (doc.value("type").toString() == JPG)     // le document est un jpg
    {
        QPixmap pix;
        pix.loadFromData(ba);
        QImage image= pix.toImage();
        if (AvecPrevisu)
        {
            QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer);
            connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(printer, image);});
            dialog->exec();
            delete dialog;
        }
        else
        {
            QPrintDialog *dialog = new QPrintDialog(printer);
            if (dialog->exec() != QDialog::Rejected)
                Print(printer, image);
            delete dialog;
        }
    }
    return true;
}

void Procedures::Print(QPrinter *Imprimante, QImage image)
{
    QPainter PrintingPreView(Imprimante);
    QPixmap pix = QPixmap::fromImage(image).scaledToWidth(int(rect.width()),Qt::SmoothTransformation);
    PrintingPreView.drawImage(QPoint(0,0),pix.toImage());
}
void Procedures::PrintPdf(QPrinter *Imprimante, Poppler::Document* document, bool &printok)
{
    for (int i=0; i<document->numPages() ;i++)
    {
        Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
        if (pdfPage == Q_NULLPTR) {
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
            printok = false;
            break;
        }
        QImage image = pdfPage->renderToImage(600,600);
        if (image.isNull()) {
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
            printok = false;
            break;
        }
        // ... use image ...
        Print(Imprimante, image);
        delete pdfPage;
    }
    printok = true;
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

QString Procedures::getSessionStatus()
{
    // statut de l'utilisateur pour cette session

    /* Valeurs de soignant
     * 1 = ophtalmo
     * 2 = Orthoptiste
     * 3 = autre soignant
     * 4 = non soignant
     * 5 = societe comptable
     */

    bool ophtalmo       = m_userConnected->isOpthalmo();
    bool orthoptist     = m_userConnected->isOrthoptist();
    bool autresoignant  = m_userConnected->isAutreSoignant();
    bool soccomptable   = m_userConnected->isSocComptable();
    bool medecin        = m_userConnected->isMedecin();

    bool assistant      = m_userConnected->isAssistant();
    bool responsable    = m_userConnected->isResponsable();
    bool responsableles2= m_userConnected->isResponsableEtAssistant();

    bool liberal        = m_userConnected->isLiberal();
    bool pasliberal     = m_userConnected->isSalarie();
    bool retrocession   = m_userConnected->isRemplacant();
    bool pasdecompta    = m_userConnected->isSansCompta();

    bool cotation       = m_userConnected->isCotation();

    bool soignant           = m_userConnected->isSoignant();
    bool soigntnonassistant = soignant && !assistant;
    bool respsalarie        = soigntnonassistant && pasliberal;
    bool respliberal        = soigntnonassistant && liberal;


    QString txtstatut = tr("Vos données enregistrées pour cette session")+ "\n\n" + m_userConnected->getStatus() + "\n\n";

    txtstatut += tr("Vos données permanentes") +"\n\n" + tr("Fonction :") + "\t\t\t";
    if (ophtalmo)
        txtstatut += tr("Ophtalmologiste");
    else if (orthoptist)
        txtstatut += tr("Orthoptiste");
    else if (autresoignant)
        txtstatut += m_userConnected->getSpecialite();
    else
        txtstatut += m_userConnected->getFonction();

    if (soignant)
    {
        txtstatut += "\n" + tr("Responsabilité des actes :\t\t");
        if (responsable)
            txtstatut += tr("toujours responsable");
        else if (responsableles2)
            txtstatut += tr("alterne responsabilité et assistant");
        else
            txtstatut += tr("assistant");
    }

    if (soigntnonassistant)
        txtstatut += "\n" + tr("RPPS :\t\t\t") + QString::number(m_userConnected->getNumPS());
    if (medecin && ! assistant)
        txtstatut += "\nADELI :\t\t\t" + m_userConnected->getNumCO();
    if (soignant)
    {
        txtstatut += "\n" + tr("Exercice :\t\t\t");
        if (liberal)
            txtstatut += tr("libéral");
        else if (pasliberal)
            txtstatut += tr("salarié") + " - " + tr("Employeur : ") + Datas::I()->users->getById(m_userConnected->getEmployeur())->getLogin();
        else if (retrocession)
            txtstatut += tr("remplaçant");
        else if (pasdecompta)
            txtstatut += tr("sans comptabilité");
    }
    if (respliberal||respsalarie)
        txtstatut += "\n" + tr("Honoraires encaissés sur le compte :\t") + Datas::I()->users->getById(m_userConnected->getCompteEncaissement()->id())->getLogin() + " " + tr("de") + " " + m_userConnected->getUserComptable()->getLogin();
    else if (retrocession)
        txtstatut += "\n" + tr("Statut :\t\t\t") + tr("remplaçant");
    if (soigntnonassistant && cotation)
        txtstatut += "\n" + tr("Cotation des actes :\t\t") + (cotation? tr("Oui") : tr("Sans"));
    if (medecin && cotation)
    {
        QString secteur ("");
        switch (m_userConnected->getSecteur()) {
        case 1:     secteur = "1";          break;
        case 2:     secteur = "2";          break;
        case 3:     secteur = "3";          break;
        default:
            break;
        }
        txtstatut += "\n" + tr("Secteur conventionnel :\t\t") + secteur;
        txtstatut += "\n" + tr("OPTAM :\t\t\t") + (m_userConnected->isOPTAM() ? "Oui": "Non");
    }
    if (respliberal || soccomptable)
    {
        QString cptabledefaut ("");
        if (m_userConnected->getCompteParDefaut() != Q_NULLPTR)
            cptabledefaut = tr("de") + " " + Datas::I()->users->getById(m_userConnected->getCompteParDefaut()->idUser())->getLogin();
        txtstatut += "\n" + tr("Comptabilité enregistrée sur compte :\t") + m_userConnected->getCompteParDefaut()->nom() + " "
                          + cptabledefaut;
    }
    if (respliberal)
        txtstatut += "\n" + tr("Membre d'une AGA :\t\t") + (m_userConnected->isAGA() ? tr("Oui") : tr("Sans"));
    return txtstatut;
}

/*! --------------------------------------------------------------------------------------------------------------------------------------
    -- détermine le dossier où est stockée l'imagerie -----------------------------------------------------------
    DirStockageImages           = l'emplacement du dossier de l'imagerie sur le poste quand il est serveur
                                = l'emplacement du dossier de l'imagerie sur le serveur vu par le poste sur le réseau local
                                = l'emplacement de dossier des copies des images d'origine sur les postes distants
                                -> utilisé par les postes pour enregistrer une copie de sauvegarde de l'original des ficiers images intégrés dans la base
    DirStockageImagesServeur    = l'emplacement du dossier de l'imagerie sur le serveur - correspond au champ dirimagerie de la table parametressysteme
                                -> utilisé par les requêtes SQL pour réintégrer le contenu de ficiers images dans la base
    ------------------------------------------------------------------------------------------------------------------------------------*/
void Procedures::setDirImagerie()
{
    DirStockageImages = "";
    QString req = "select dirimagerie from " NOM_TABLE_PARAMSYSTEME;
    QVariantList dirdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && dirdata.size()>0)
        DirStockageImagesServeur = dirdata.at(0).toString();
    switch (db->getMode()) {
    case DataBase::Poste:
    {
        DirStockageImages = DirStockageImagesServeur;
        break;
    }
    case DataBase::Distant:
    {
        DirStockageImages  = gsettingsIni->value("BDD_DISTANT/DossierImagerie").toString();
        break;
    }
    case DataBase::ReseauLocal:
    {
        DirStockageImages  = gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();
        break;
    }
    default:
        break;
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- renvoie la valeur du dossier où est stockée l'imagerie -----------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::DirImagerie()
{
    return DirStockageImages;
}

QString Procedures::DirImagerieServeur()
{
    return DirStockageImagesServeur;;
}

void Procedures::setFicheRefractionOuverte(bool a)
{
    dlgrefractionouverte = a;
}

bool Procedures::FicheRefractionOuverte()
{
    return dlgrefractionouverte;
}

bool Procedures::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==uptable)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            for (int i=0; i < uptable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(uptable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    QPixmap pix = QPixmap::fromImage(listimage.at(i).scaled(uptable->width(), uptable->height(),
                                                           Qt::KeepAspectRatioByExpanding,
                                                           Qt::SmoothTransformation));
                    lbl->setPixmap(pix);
                    uptable->setRowHeight(i,lbl->pixmap()->height());
                    uptable->setColumnWidth(i,lbl->pixmap()->width());
                }
            }
            inflabel    ->move(10,uptable->height()-40);
        }
    }
    return true;
}

void Procedures::ReconstruitComboCorrespondants(QComboBox* box, bool all)
{
    box->clear();
    QMap<int, Correspondant *> *listcor = Datas::I()->correspondants->correspondants();
    QMap<int, Correspondant *>::const_iterator itcor;
    QStandardItemModel *model = new QStandardItemModel();
    // toute la manip qui suit sert à remettre les correspondants par ordre aplhabétique (dans le QMap, ils sont triés par id croissant) - si  vous trouvez plus simple, ne vous génez pas
    for( itcor = listcor->constBegin(); itcor != listcor->constEnd(); ++itcor )
    {
        Correspondant *cor = const_cast<Correspondant*>(*itcor);
        QList<QStandardItem *> items;
        items << new QStandardItem(cor->nom() + " "  + cor->prenom()) << new QStandardItem(QString::number(cor->id()));
        if (all)
            model->appendRow(items);
        else if (cor->ismedecin())
            model->appendRow(items);
    }
    model->sort(0);
    for(int i=0; i<model->rowCount(); i++)
        box->addItem(model->item(i)->text(), model->item(i,1)->text());
}

void Procedures::setmg(Patient *pat, int idmg)
{
    QString val = (idmg == 0? "null" : QString::number(idmg));
    QString req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg = " + val + " where idpat = " + QString::number(pat->id());
    db->StandardSQL(req);
    pat->setmg(idmg);
}

void Procedures::setspe1(Patient *pat, int idspe1)
{
    QString val = (idspe1 == 0? "null" : QString::number(idspe1));
    QString req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe1 = " + val + " where idpat = " + QString::number(pat->id());
    db->StandardSQL(req);
    pat->setspe1(idspe1);
}

void Procedures::setspe2(Patient *pat, int idspe2)
{
    QString val = (idspe2 == 0? "null" : QString::number(idspe2));
    QString req = "update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe2 = " + val + " where idpat = " + QString::number(pat->id());
    db->StandardSQL(req);
    pat->setspe2(idspe2);
}


//Pas normal, les mots de passes doivent etre chiffrés
QString Procedures::getMDPAdmin()
{
    QString req = "select mdpadmin from " NOM_TABLE_PARAMSYSTEME;
    QVariantList mdpdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && mdpdata.size()>0)
        if (mdpdata.at(0).toString() == "")
            db->StandardSQL("update " NOM_TABLE_PARAMSYSTEME " set mdpadmin = '" NOM_MDPADMINISTRATEUR "'");
    return (mdpdata.at(0).toString() != ""? mdpdata.at(0).toString() : NOM_MDPADMINISTRATEUR);
}

void Procedures::setNomImprimante(QString NomImprimante)
{
    gnomImprimante = NomImprimante;
}

QString Procedures::getNomImprimante()
{
    return gnomImprimante;
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
    db->StandardSQL(req);

    req = "DROP PROCEDURE IF EXISTS " NOM_POSTEIMPORTDOCS ";";
    db->StandardSQL(req);

    if (a)
        IpAdress = QHostInfo::localHostName()  + ((gsettingsIni->value("BDD_LOCAL/PrioritaireGestionDocs").toString() ==  "YES")? " - prioritaire" : "");
    req = "CREATE PROCEDURE " NOM_POSTEIMPORTDOCS "()\n\
          BEGIN\n\
          SELECT '" + IpAdress + "';\n\
          END ;";
    gisPosteImpotDocs = db->StandardSQL(req);
}

bool Procedures::isPosteImportDocs()
{
    return gisPosteImpotDocs;
}

QString Procedures::PosteImportDocs()
{   QString rep = "";
    QString req = "SELECT name FROM mysql.proc p WHERE db = '" NOM_BASE_CONSULTS "' AND name = '" NOM_POSTEIMPORTDOCS "'";
    QVariantList imptdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && imptdata.size()>0)
    {
        req = "CALL " NOM_BASE_CONSULTS "." NOM_POSTEIMPORTDOCS;
        QVariantList calldata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        //qDebug() << "nbre reponses = " + QString::number(calldata.size()) << NOM_POSTEIMPORTDOCS " = " + calldata.at(0).toString();
        if (ok && calldata.size()>0)
            rep = calldata.at(0).toString();
    }
    //qDebug() << "posteimportdocs = " + rep;
    return rep;
}

bool Procedures::Verif_secure_file_priv()
{
    if (Var_secure_file_priv()==QString())
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Configuration du serveur défectueuse"),
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

void Procedures::TestAdminPresent()             // Vérifie si RufusAdmin est utilisé
{
    QString req = "select iduser from " NOM_TABLE_USERSCONNECTES " where iduser = (select iduser from " NOM_TABLE_UTILISATEURS " where userlogin = '" NOM_ADMINISTRATEURDOCS "')";
    QVariantList tcpdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    OKAdmin = (ok && tcpdata.size()>0  && db->getMode() != DataBase::Distant);
}

bool Procedures::isadminpresent()
{
    return OKAdmin;
}

void Procedures::setoktcp(bool oktcp)
{
    OKTCP = oktcp;
    if (!oktcp && !OKAdmin)
        db->StandardSQL("update " NOM_TABLE_PARAMSYSTEME " set AdresseTCPServeur = null");
}

QString Procedures::Var_secure_file_priv()
{
    QString msg = QString();
    QVariantList vardata = db->getFirstRecordFromStandardSelectSQL("SHOW VARIABLES LIKE \"secure_file_priv\";", ok);
    if (ok && vardata.size()>0)
        msg = vardata.at(1).toString();
    if (msg == "NULL")
        msg = QString();
    //qDebug() << msg;
    return msg;
}

bool Procedures::ReinitBase()
{
    if (!VerifAutresPostesConnectes())
        return false;
    UpMessageBox msgbox;
    UpSmallButton OKBouton(tr("Réinitialiser"));
    UpSmallButton AnnulBouton(tr("Annuler"));
    msgbox      .setText(tr("Réinitialisation du programme!"));
    msgbox      .setInformativeText(tr("Si vous confirmez la réinitialisation, une sauvegarde de la base patients, du fichier Rufus.ini et des fichiers ressources sera réalisée"
                                      " puis le programme sera arrêté.\n"
                                      "Au redémarrage du programme, une fenêtre de dialogue vous permettra"
                                      " de réinitialiser toutes les données en cliquant sur le bouton \"Premier démarrage de Rufus\".\n"
                                      "Vous pourrez encore annuler la réinitialisation"
                                      " en cliquant sur le bouton \"Restaurer le fichier de paramétrage\""
                                      " et en choisissant le fichier RufusBackup.ini"));
    msgbox      .setIcon(UpMessageBox::Warning);
    msgbox      .addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
    msgbox      .addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox      .exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        if (!ImmediateBackup("", false, true))
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
        UpMessageBox::Information(Q_NULLPTR, tr("Arrêt du programme!"));
        exit(0);
    }
    return false;
}

void Procedures::RestoreFontAppliAndGeometry()
{
    // On essaie de retrouver la police écran enregistrée par l'utilisateur, sinon, on prend celle par défaut
    QString fonteFamily("");
    QString req = "select UserPoliceEcran, UserPoliceAttribut from " NOM_TABLE_UTILISATEURS " where idUser = " + QString::number(db->getUserConnected()->id());
    QVariantList fontdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && fontdata.size()>0)
        fonteFamily = fontdata.at(0).toString().split(",").at(0);
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
                FontAttribut     = fontdata.at(1).toString();
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
#ifdef Q_OS_LINUX
    gAppFont.setPointSize(gAppFont.pointSize()+1);
#endif
    qApp->setFont(gAppFont);

}

qint64 Procedures::CalcBaseSize()
{
    qint64 basesize = 0;
    QString req = "SELECT SUM(SizeMB) from "
                      "(SELECT table_schema, round(sum(data_length+index_length)/1024/1024,4) AS SizeMB FROM information_schema.tables"
                      " where table_schema = 'ComptaMedicale'"
                      " or table_schema = 'Ophtalmologie'"
                      " or table_schema = 'rufus'"
                      " GROUP BY table_schema)"
                      " as bdd";
    QVariantList basedata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && basedata.size()>0)
        basesize = basedata.at(0).toLongLong();
    return basesize;
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
        if (listchk.at(i)->accessibleDescription() == "factures")
        {
            if (listchk.at(i)->isChecked())
                volume += FacturesSize;
        }
    }
    time = (volume/1024 /2)*60000; //duréée approximative de sauvegarde en ms
    QString Volumelitteral = Utils::getExpressionSize(volume);
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
    QString Volumelibre = Utils::getExpressionSize(FreeSpace);
    msg = tr("Volume disponible sur le disque: ") + " <font color=\""+ color + "\"><b>" + Volumelibre + "</b></font>";

    labelVolumeLibre->setText(msg);
    gAskBupRestore->OKButton->setEnabled(FreeSpace>volume);
}

bool Procedures::RestaureBase(bool BaseVierge, bool PremierDemarrage, bool VerifUserConnectes)
{
    UpMessageBox    msgbox;
    UpSmallButton   AnnulBouton;
    UpSmallButton   OKBouton;
    msgbox.setIcon(UpMessageBox::Warning);
    bool echecfile = true;

    if (BaseVierge)
    {
        QString Hote;
        if (db->getMode() == DataBase::Poste)
            Hote = tr("ce poste");
        else
            Hote = tr("le serveur ") + gsettingsIni->value(db->getBase() + "/Serveur").toString();
        msgbox.setInformativeText(tr("Vous avez choisi de créer une base vierge sur ") + Hote + "\n" +
                                  tr("Si une base de données Rufus existe sur ce serveur, "
                                     "elle sera définitivement effacée pour être remplacée par cette base vierge.\n"
                                     "Confirmez-vous la suppression des anciennes données?\n"));
        AnnulBouton.setText(tr("Annuler"));
        OKBouton.setText("J'ai compris\nJe confirme");
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            return false;

        if (!Utils::VerifMDP((PremierDemarrage? NOM_MDPADMINISTRATEUR : getMDPAdmin()),tr("Saisissez le mot de passe Administrateur")))
            return false;

        QFile BaseViergeFile(QStringLiteral("://basevierge.sql"));
        BaseViergeFile.copy(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES "/basevierge.sql");
        QFile DumpFile(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES "/basevierge.sql");
        if (!DumpFile.open(QIODevice::ReadOnly))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Echec de la restauration"), tr("Le fichier ") + "basevierge.sql" + tr(" n'a pas été trouvé!"));
            return false;
        }
        else if (DumpFile.size() == 0)
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Echec de la restauration"), tr("Le fichier ") + "basevierge.sql" + tr(" est vide!"));
            return false;
        }
        else echecfile = false;
        if (!echecfile)
        {
            connexion = false;
            emit ConnectTimers();
            int a = 99;
            // +++ la fonction DefinitScriptRestore() qu'on pourrait vouloir utiliser dans ce cas là avec le fichier basevierge.sql ne fonctionne pas avec ce fichier
            // et je ne sais pas pourquoi
            // et j'en ai marre de chercher pourquoi
            QStringList listinstruct = Utils::DecomposeScriptSQL(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES "/basevierge.sql");
            bool e = true;
            foreach(const QString &s, listinstruct)
                if (!db->StandardSQL(s))
                {
                    e = false;
                    break;
                }
            a = (e? 0:99);
            DumpFile.remove();
            if (a==0)
            {
                UpMessageBox::Information(Q_NULLPTR, tr("Base vierge créée"),tr("La création de la base vierge a réussi."));
                connexion = true;
                emit ConnectTimers();
                return true;
            }
        }
        UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'éxécuter la restauration!") + "\n" + tr("Le fichier ") + "basevierge.sql" + tr(" ne semble pas conforme!"));
        DumpFile.remove();
        return false;
    }
    else
    {
        if (VerifUserConnectes)
        {
            QString req = "select NomPosteConnecte from " NOM_TABLE_USERSCONNECTES " where NomPosteConnecte <> '" + QHostInfo::localHostName().left(60) + "'";
            QVariantList nompostedata = db->getFirstRecordFromStandardSelectSQL(req, ok);
            if (!ok)
                return false;
            if (nompostedata.size()>0)
            {
                UpMessageBox::Information(Q_NULLPTR, tr("Autres postes connectés!"),
                                             tr("Vous ne pouvez pas effectuer d'opération de sauvegarde/restauration sur la base de données"
                                                " si vous n'êtes pas le seul poste connecté.\n"
                                                "Le poste ") + nompostedata.at(0).toString() + " " + tr("est aussi connecté"));
                return false;
            }
        }
        UpMessageBox::Information(Q_NULLPTR, tr("Choix du dossier de sauvegarde"),
                                  tr("Dans la fiche suivante, choisissez le dossier "
                                  "contenant la sauvegarde de la base.\n\n"
                                  "Une fois le dossier sélectionné, "
                                  "la sauvegarde commencera automatiquement.\n"
                                  "Ce processus est long et peut durer plusieurs minutes.\n"
                                  "(environ 1' pour 2 Go)\n"));
        QString dir = QDir::homePath() + NOMDIR_RUFUS;
        QFileDialog dialog(Q_NULLPTR,tr("Restaurer à partir du dossier") , dir);
        dialog.setViewMode(QFileDialog::List);
        dialog.setFileMode(QFileDialog::DirectoryOnly);
        bool b = (dialog.exec()>0);
        if (!b)
            return false;
        QDir dirtorestore = dialog.directory();
        if (dirtorestore.dirName()=="")
            return false;
        if (dirtorestore.absolutePath().contains(" "))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Echec de la restauration"), tr("Le chemin vers le dossier ") + dirtorestore.absolutePath() + tr(" contient des espaces!"));
            return false;
        }
        if (!Utils::VerifMDP(getMDPAdmin(),tr("Saisissez le mot de passe Administrateur")))
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
        bool OKFactures = false;
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
        if (QDir(dirtorestore.absolutePath() + NOMDIR_FACTURES).exists())
            if (QDir(dirtorestore.absolutePath() + NOMDIR_FACTURES).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size()>0)
                OKFactures = true;

        QString NomDirStockageImagerie = QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGES;
        QString req = "select dirimagerie from " NOM_TABLE_PARAMSYSTEME;
        QVariantList dirdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (ok && dirdata.size()>0)
        {
            NomDirStockageImagerie = dirdata.at(0).toString();
            if (!QDir(NomDirStockageImagerie).exists())
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Pas de dossier de stockage valide"),
                                    tr("Le dossier spécifié pour le stockage de l'imagerie n'est pas valide") + "\n"
                                    + tr("Indiquez un dossier valide dans la boîte de dialogue qui suit"));
                QFileDialog dialogimg(Q_NULLPTR,tr("Stocker les images dans le dossier") , QDir::homePath() + NOMDIR_RUFUS);
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
                if (NomDirStockageImagerie.contains(" "))
                {
                    UpMessageBox::Watch(Q_NULLPTR, tr("Echec de la restauration"), tr("Le chemin vers le dossier ") + NomDirStockageImagerie + tr(" contient des espaces!"));
                    return false;
                }
            }
        }
        if (!QDir(NomDirStockageImagerie).exists())
        {
            QDir dirstock;
            dirstock.mkdir(NomDirStockageImagerie);
        }

        req = "update " NOM_TABLE_PARAMSYSTEME " set dirImagerie = '" + NomDirStockageImagerie + "'";
        db->StandardSQL(req, "bool Procedures::RestaureBase(bool BaseVierge, bool PremierDemarrage, bool VerifUserConnectes) - 1st");
        AskBupRestore(true, dirtorestore.absolutePath(), NomDirStockageImagerie, OKini, OKRessces, OKImages, OKVideos, OKFactures);
        if (gAskBupRestore->exec()>0)
        {
            connexion = false;
            emit ConnectTimers();
            QList<UpCheckBox*> listchk = gAskBupRestore->findChildren<UpCheckBox*>();
            for (int i= 0; i<listchk.size(); i++)
            {
                if (listchk.at(i)->accessibleDescription() == "base")
                {
                    if (listchk.at(i)->isChecked())
                    {
                        UpMessageBox msgbox;
                        QStringList listnomsfilestorestore;
                        UpSmallButton AnnulBouton(tr("Annuler"));
                        UpSmallButton OKBouton(tr("J'ai compris\nJe confirme"));
                        msgbox.setIcon(UpMessageBox::Warning);

                        msgbox.setText(tr("Attention"));
                        msgbox.setInformativeText(tr("Vous avez choisi de restaurer la base à partir du dosssier") + "\n"
                                                  + dirtorestore.absolutePath() + ".\n" +
                                                  tr("Si une base de données Rufus existe sur ce serveur, "
                                                     "elle sera définitivement effacée pour être remplacée par cette sauvegarde.\n"
                                                     "Confirmez-vous la suppression des anciennes données?"));
                        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
                        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
                        msgbox.exec();
                        if (msgbox.clickedButton() != &OKBouton)
                        {
                            msg += tr("Base non restaurée");
                            break;
                        }

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
                            VideDatabases();
                            int a = 99;
                            //Restauration à partir du dossier sélectionné
                            DefinitScriptRestore(listnomsfilestorestore);
                            QString msg = "sh " + QDir::homePath() + SCRIPTRESTOREFILE;
                            QProcess dumpProcess(parent());
                            dumpProcess.start(msg);
                            dumpProcess.waitForFinished(1000000000);
                             if (dumpProcess.exitStatus() == QProcess::NormalExit)
                                a = dumpProcess.exitCode();
                            if (a != 0)
                                Message(tr("Incident pendant la sauvegarde"),3000,false);
                            QFile::remove(QDir::homePath() + SCRIPTRESTOREFILE);
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
                                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille du dosseir"));
                            Message(Msg, 3000, false);
                            Msg = "cp -R " + dirrestaurevideo.absolutePath() + " " + NomDirStockageImagerie;
                            QProcess::execute(Msg);
                            msg += tr("Fichiers videos restaurés\n");
                            Message(tr("Fichiers videos restaurés"), 3000);
                        }
                    }
                }
            }
            for (int i= 0; i<listchk.size(); i++)
            {
                if (listchk.at(i)->accessibleDescription() == "factures")
                {
                    if (listchk.at(i)->isChecked())
                    {
                        QDir dirrestaurefactures = QDir(dirtorestore.absolutePath() + NOMDIR_FACTURES);
                        QString dirdestinationfactures       =  NomDirStockageImagerie + NOMDIR_FACTURES;
                        QDir DirDestFac(dirdestinationfactures);
                        if (DirDestFac.exists())
                            DirDestFac.removeRecursively();
                        if (!DirDestFac.mkdir(dirdestinationfactures))
                        {
                            QString Msg = tr("le dossier de destination des factures n'existe pas");
                            Message(Msg, 3000, false);
                        }
                        else
                        {
                            QString Msg = (tr("Restauration des fichiers facturess\n")
                                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille du dossier"));
                            Message(Msg, 3000, false);
                            Msg = "cp -R " + dirrestaurefactures.absolutePath() + " " + NomDirStockageImagerie;
                            QProcess::execute(Msg);
                            msg += tr("Factures restaurées\n");
                            Message(tr("Factures restaurées"), 3000);
                        }
                    }
                }
            }
            delete gAskBupRestore;
            //qDebug() << msg;
            UpMessageBox::Watch(Q_NULLPTR,tr("restauration terminée"),msg);
            return true;
        }
        connexion = true;
        emit ConnectTimers();
    }
    return false;
}

//TODO : !!! checkBaseVersion doit disparaitre, c'est le serveur qui gère tout seul la version de la base sans rien demander
bool Procedures::VerifBaseEtRessources()
{
    int Versionencours  = 9; //correspond aux premières versions de MAJ de la base
    int Version         = VERSION_BASE;
    QString req         = "select VersionBase from " NOM_TABLE_PARAMSYSTEME;
    QVariantList versiondata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    bool b              = (!ok || versiondata.size() == 0);
    if (!b)
    {
        Versionencours = versiondata.at(0).toInt();
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
                UpSmallButton OKBouton(tr("OK, je vais sauvegarder la base d'abord"));
                UpSmallButton AnnulBouton(tr("Pousuivre, la sauvegarde a été faite"));
                msgbox.addButton(&OKBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(&AnnulBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                if (msgbox.clickedButton() != &AnnulBouton)
                    if (!ImmediateBackup("", false, false))
                        return false;
                BupDone = true;
            }
            Message(tr("Mise à jour de la base vers la version ") + "<font color=\"red\"><b>" + QString::number(Version) + "</b></font>", 1000, false);
            QString Nomfic = "://majbase" + QString::number(Version) + ".sql";
            QFile DumpFile(Nomfic);
            if (DumpFile.exists())
            {
                QString NomDumpFile = QDir::homePath() + "/Documents/Rufus/Ressources/majbase" + QString::number(Version) + ".sql";
                DumpFile.copy(NomDumpFile);
                QFile base(NomDumpFile);
                QStringList listinstruct = Utils::DecomposeScriptSQL(NomDumpFile);
                bool a = true;
                foreach(const QString &s, listinstruct)
                {
                    if (!db->StandardSQL(s))
                        a = false;
                }
                int result=0;
                base.remove();
                if (a)
                {
                    result = 1;
                    UpMessageBox::Watch(Q_NULLPTR,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version));
                }
                else
                {
                    QSound::play(NOM_ALARME);
                    UpMessageBox::Watch(Q_NULLPTR,tr("Echec de la mise à jour vers la version ") + QString::number(Version) + "\n" + tr("Le programme de mise à jour n'a pas pu effectuer la tâche!"));
                }
                if (result!=1)
                    return false;
            }
            if (Version == 53)
            {
                /*! dans les anciennes versions du programme antérieures à la 53, pour des raisons d'économie d'espace disque,
                 * la création d'un dossier n'entraînait pas systématiquement
                 * la création d'une ligne corresondante dans la table renseignementsmedicauxpatients
                 * à partir de la version 53, cette ligen est créée systématiquement pour ne pas avoir à on vérifier sa présence
                 *  à chaque fois qu'on veut enregistrer un renseignement
                 * A partir de la version 53, cette ligne est systématiquement créée lors de la création d'un dossier
                 * il n'y a donc plus à faire cette vérification
                 * cette MAJ crée une ligne pour tous les dossiers n'ayant pas la correspondance dans la table renseignementsmedicauxpatients
                 */
                QList<QVariantList> listid =
                        db->StandardSelectSQL("SELECT idpat FROM " NOM_TABLE_PATIENTS " pat"
                                              " where  pat.idpat not in (select rmp.idpat from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " rmp)", ok);
                if (listid.size()>0)
                {
                    for (int i=0; i<listid.size(); i++)
                    {
                        req =   "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                                " (idPat) VALUES (" + listid.at(i).at(0).toString() + ")";
                        db->StandardSQL(req);
                    }
                    UpMessageBox::Watch(Q_NULLPTR,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version), QString::number(listid.size()) + tr(" enregistrements modifiés"));
                }
                else
                    UpMessageBox::Watch(Q_NULLPTR,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version));
                db->StandardSQL("UPDATE " NOM_TABLE_PARAMSYSTEME " SET VersionBase = 53");            }
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
        UpMessageBox::Watch(Q_NULLPTR, tr("Aucune base identifiée dans Rufus.ini !"), tr("Veuillez corriger ce paramètre."));
        exit(0);
    }
    case 1: {
        if (lPoste)         gMode2 = DataBase::Poste;
        if (lReseauLocal)   gMode2 = DataBase::ReseauLocal;
        if (lDistant)       gMode2 = DataBase::Distant;
        initOK  = true;
        break;
    }
    default: {
        UpMessageBox msgbox;
        msgbox.setText(tr("Quelle base de données souhaitez-vous utiliser?"));
        msgbox.setIcon(UpMessageBox::Quest);
        UpPushButton OKBouton(tr("Sur cette machine"));
        UpPushButton NoBouton(tr("Locale, sur ce réseau"));
        UpPushButton AnnulBouton(tr("Distante, par internet"));
        UpPushButton RejectButton(tr("Annuler"));
        RejectButton.setIcon(Icons::icAnnuler());
        msgbox.addButton(&RejectButton);
        if (lReseauLocal)
        {
            msgbox.addButton(&NoBouton);
            NoBouton.setIcon(Icons::icNetwork());
        }
        if (lDistant)
        {
            msgbox.addButton(&AnnulBouton);
            AnnulBouton.setIcon(Icons::icInternet());
        }
        if (lPoste)
        {
            msgbox.addButton(&OKBouton);
            OKBouton.setIcon(Icons::icComputer());
        }
        initOK = false;
        if (msgbox.exec()>0)
        {
            initOK = (msgbox.clickedpushbutton() != &RejectButton);
            if (initOK)
            {
                if (msgbox.clickedpushbutton()      == &OKBouton)    gMode2 = DataBase::Poste;
                else if (msgbox.clickedpushbutton() == &NoBouton)    gMode2 = DataBase::ReseauLocal;
                else if (msgbox.clickedpushbutton() == &AnnulBouton) gMode2 = DataBase::Distant;
            }
        }
    }
    }
    return initOK;
}

/*--------------------------------------------------------------------------------------------------------------
-- Connexion des timers -------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
bool Procedures::Connexion()
{
    return connexion;
}
/*--------------------------------------------------------------------------------------------------------------
-- Connexion à Consults -------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
bool Procedures::Connexion_A_La_Base()
{
    db->init(*gsettingsIni, gMode2);
    if (!IdentificationUser())
        return false;

    //initListeUsers();

    m_userConnected->setSite( DetermineLieuExercice() );
    if (m_userConnected->getSite() == Q_NULLPTR )
        UpMessageBox::Watch(Q_NULLPTR,tr("Pas d'adresse spécifiée"), tr("Vous n'avez précisé aucun lieu d'exercice!"));
    gdbOK = true;

    //Etrange le parametrage ici
    db->StandardSQL("set global sql_mode = 'NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES';");
    db->StandardSQL("SET GLOBAL event_scheduler = 1 ;");
    db->StandardSQL("SET GLOBAL max_allowed_packet=" MAX_ALLOWED_PACKET "*1024*1024 ;");

    // on recherche si rufusadmin est en fonction auquel cas on utilise les TCPsocket
    QString req = "select iduser from " NOM_TABLE_USERSCONNECTES " where iduser = (select iduser from " NOM_TABLE_UTILISATEURS " where userlogin = '" NOM_ADMINISTRATEURDOCS "')";
    TestAdminPresent();
    return gdbOK;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Détermination du lieu exercice pour la session en cours -------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------- */
Site* Procedures::DetermineLieuExercice()
{
    QList<Site*> listEtab = db->loadSitesByUser(m_userConnected->id());
    if( listEtab.size() == 1 )
        return listEtab[0];

    /* Cas ou le praticien est dans plusieur centres
     * on lui demande de sélectionner le centre où il se trouve au moment de la connexion
    */
    UpDialog *gAskLieux     = new UpDialog();
    gAskLieux               ->AjouteLayButtons();
    QGroupBox*boxlieux      = new QGroupBox();
    gAskLieux->dlglayout()  ->insertWidget(0,boxlieux);
    boxlieux                ->setAccessibleName("Parent");
    boxlieux                ->setTitle(tr("D'où vous connectez-vous?"));
    QFontMetrics fm         = QFontMetrics(qApp->font());
    int hauteurligne        = int(fm.height()*1.6);
    boxlieux                ->setFixedHeight(((listEtab.size() + 1)*hauteurligne)+5);
    QVBoxLayout *vbox       = new QVBoxLayout;
    QList<Site*>::const_iterator itEtab;
    bool isFirst = true;
    for(itEtab = listEtab.constBegin(); itEtab != listEtab.constEnd(); ++itEtab)
    {
        Site *etab = const_cast<Site*>(*itEtab);
        UpRadioButton *pradiobutt = new UpRadioButton(boxlieux);
        pradiobutt->setText(etab->nom());
        pradiobutt->setAccessibleName(QString::number(etab->id()));
        pradiobutt->setItem(etab);
        QString data("");
        if( etab->nom().size() )
            data += etab->nom();
        if( etab->adresse1().size() )
            data += (data.size() ? "\n" : "") + etab->adresse1();
        if( etab->adresse2().size() )
            data += (data.size() ? "\n" : "") + etab->adresse2();
        if( etab->adresse3().size() )
            data += (data.size() ? "\n" : "") + etab->adresse3();
        if( etab->codePostal() )
            data += (data.size() ? "\n" : "") + QString::number(etab->codePostal());
        if( etab->ville().size() )
            data += (data.size() ? "\n" : "") + etab->ville();
        if( etab->telephone().size() )
            data += (data != ""? "\nTel: " : "Tel: ") + etab->telephone();
        pradiobutt->setImmediateToolTip(data);
        pradiobutt->setChecked(isFirst);
        vbox      ->addWidget(pradiobutt);
        isFirst = false;
    }
    vbox                    ->setContentsMargins(8,0,8,0);
    boxlieux                ->setLayout(vbox);
    gAskLieux               ->setModal(true);
    gAskLieux->dlglayout()  ->setSizeConstraint(QLayout::SetFixedSize);
    connect(gAskLieux->OKButton,   &QPushButton::clicked,  gAskLieux, &UpDialog::accept);
    gAskLieux->exec();
    QList<UpRadioButton*> listbutt = boxlieux->findChildren<UpRadioButton*>();
    QList<UpRadioButton*>::const_iterator itRB;
    for(itRB = listbutt.constBegin(); itRB != listbutt.constEnd(); ++itRB)
    {
        UpRadioButton *rb = const_cast<UpRadioButton*>(*itRB);
        if( rb->isChecked() )
            return qobject_cast<Site*>(rb->getItem());
    }
    return Q_NULLPTR;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Création d'un utilisateur -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::CreerPremierUser(QString Login, QString MDP)
{
    // Bon, on dispose du nouveau login et du nouveau MDP
    if (Login.isEmpty())
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de créer l'utilisateur"),tr("Login manquant"));
        return false;
    }
    if (MDP.isEmpty())
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de créer l'utilisateur"),tr("Mot de passe manquant"));
        return false;
    }

    //1. On vérifie si ce login existe dans le serveur et si c'est le cas, on détruit toutes les instances de ce login
    //TODO : !!! un peu brutal
    QString req = "select user, host from mysql.user where user like '" + Login + "%'";
    QList<QVariantList> usrlist = db->StandardSelectSQL(req, ok);
    if (ok && usrlist.size()>0)
        for (int i=0; i<usrlist.size(); i++)
            db->StandardSQL("drop user '" + usrlist.at(i).at(0).toString() + "'@'" + usrlist.at(i).at(1).toString() + "'");

    //2. On crée 3 comptes SQL avec ce login et ce MDP: local en localshost, réseau local (p.e. 192.168.1.%) et distant en %-SSL et login avec SSL à la fin
    //TODO : pas de compte SQL, uniquement interne au système Rufus pour des questions de sécurités, sinon, n'importe qui peux attaquer la base directement.
    // Serge Oui c'est une grosse erreur de conception mais tant que le logiciel ne sera pas modifié, il est impossible d'installler le programme sans en passer par là
    QString AdressIP (Utils::getIpAdress()), MasqueReseauLocal;
    QStringList listIP = AdressIP.split(".");
    for (int i=0;i<listIP.size()-1;i++)
        MasqueReseauLocal += QString::number(listIP.at(i).toInt()) + ".";
    MasqueReseauLocal += "%";
    db->StandardSQL ("create user '" + Login + "'@'localhost' identified by '" + MDP + "'");
    db->StandardSQL ("create user '" + Login + "'@'" + MasqueReseauLocal + "' identified by '" + MDP + "'");
    db->StandardSQL ("create user '" + Login + "SSL'@'%' identified by '" + MDP + "' REQUIRE SSL");
    db->StandardSQL ("grant all on *.* to '" + Login + "'@'localhost' identified by '" + MDP + "' with grant option");
    db->StandardSQL ("grant all on *.* to '" + Login + "SSL'@'%' identified by '" + MDP + "' with grant option");
    db->StandardSQL ("grant all on *.* to '" + Login + "'@'" + MasqueReseauLocal + "' identified by '" + MDP + "' with grant option");

    // Création de l'administrateur des documents ------------------------------------------------------------------
    db->StandardSQL ("create user if not exists '" NOM_ADMINISTRATEURDOCS "'@'localhost' identified by '" NOM_MDPADMINISTRATEUR "'");
    db->StandardSQL ("create user if not exists '" NOM_ADMINISTRATEURDOCS "'@'" + MasqueReseauLocal + "' identified by '" NOM_MDPADMINISTRATEUR "'");
    db->StandardSQL ("create user if not exists '" NOM_ADMINISTRATEURDOCS "SSL'@'%' identified by '" NOM_MDPADMINISTRATEUR "' REQUIRE SSL");
    db->StandardSQL ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "'@'localhost' identified by '" NOM_MDPADMINISTRATEUR "' with grant option");
    db->StandardSQL ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "'@'" + MasqueReseauLocal + "' identified by '" NOM_MDPADMINISTRATEUR "' with grant option");
    db->StandardSQL ("grant all on *.* to '" NOM_ADMINISTRATEURDOCS "SSL'@'%' identified by '" NOM_MDPADMINISTRATEUR "' with grant option");
    db->StandardSQL ("insert into " NOM_TABLE_UTILISATEURS " (idUser, UserNom, UserLogin) values (100, '" NOM_ADMINISTRATEURDOCS "','" NOM_ADMINISTRATEURDOCS "')");

    // On crée l'utilisateur dans la table utilisateurs
    gidCentre               = 1;
    gUseCotation            = true;
    CreerUserFactice(1, Login, MDP);
    db->setUserConnected( new User(Login, MDP, db->loadUserData(1)) );
    m_userConnected = db->getUserConnected();
    m_userConnected->setIdUserActeSuperviseur(1);
    m_userConnected->setIdUserComptable(1);
    m_userConnected->setIdUserParent(1);

    if (UpMessageBox::Question(Q_NULLPTR, tr("Un compte utilisateur a été cré"),
                               tr("Un compte utilisateur factice a été créé\n") + "\n" +
                               m_userConnected->getTitre() + " "  + m_userConnected->getPrenom() + " " + m_userConnected->getNom() + ", " + m_userConnected->getFonction()
                               + "\n\n" +
                               tr("avec le login ") + Login + " " + tr("et le mot de passe que vous avez fourni") + "\n" +
                               tr("Voulez-vous conserver ces données pour le moment ou les modifier?") + "\n" +
                               tr("Vous pourrez les modifier par la suite\n"),
                               UpDialog::ButtonOK | UpDialog::ButtonEdit, QStringList() << tr("Modifier les données") << tr("Conserver les données"))
        == UpSmallButton::EDITBUTTON)
    {
        int gidLieuExercice = -1;
        Dlg_GestUsr = new dlg_gestionusers(1, gidLieuExercice, true);
        Dlg_GestUsr->setWindowTitle(tr("Enregistrement de l'utilisateur ") + Login);
        Dlg_GestUsr->setConfig(dlg_gestionusers::PREMIERUSER);
        Dlg_GestUsr->exec();
        delete Dlg_GestUsr;
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
void Procedures::CreerUserFactice(int idusr, QString login, QString mdp)
{
    //TODO : Revoir
    db->StandardSQL ("insert into " NOM_TABLE_UTILISATEURS " (idUser, UserLogin, UserMDP) VALUES (" + QString::number(idusr) + ",'" + login + "', '" + mdp + "')");

    int idbanq = 0;
    QString req = "select idbanque, idbanqueabrege, nombanque from " NOM_TABLE_BANQUES " where idbanqueabrege = 'PaPRS'";
    QVariantList bqdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && bqdata.size()>0)
        idbanq = bqdata.at(0).toInt();
    else
    {
        db->StandardSQL("insert into " NOM_TABLE_BANQUES " (idbanqueAbrege, Nombanque) values ('PaPRS','Panama Papers')");
        QVariantList bqdata = db->getFirstRecordFromStandardSelectSQL("select idbanque from " NOM_TABLE_BANQUES " where idbanqueabrege = 'PaPRS'", ok);
        if (ok && bqdata.size()>0)
            idbanq = bqdata.at(0).toInt();
    }

    int al = 0;
    QString iban = "FR";
    srand(static_cast<uint>(time(Q_NULLPTR)));
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
           " VALUES (" + QString::number(idbanq) + "," + QString::number(idusr) + ", '" + iban + "', '" + login + "', 'PaPRS" + QString::number(al) + "', 2333.67)";
    //qDebug() << req;
    db->StandardSQL(req);
    QString idcpt ("");
    req = "select max(idcompte) from " NOM_TABLE_COMPTES;
    QVariantList cptdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && cptdata.size()>0)
        idcpt = cptdata.at(0).toString();

    req = "update " NOM_TABLE_UTILISATEURS
            " set userNom = 'Snow',\n"
            " userPrenom = '" + Utils::trimcapitilize(login) +"',\n"
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
            " where idUser = " + QString::number(idusr);
    //Edit(req);
    db->StandardSQL(req);
    req = "insert into " NOM_TABLE_LIEUXEXERCICE "(NomLieu, LieuAdresse1, LieuAdresse2, LieuCodePostal, LieuVille, LieuTelephone)  values ("
            "'Centre ophtalmologique de La Mazière', "
            "'place rouge', "
            "'Bâtiment C', "
            "23260, "
            "'La Mazière', "
            "'O4 56 78 90 12')";
    //Edit(req);
    db->StandardSQL(req);
    req = "select idLieu from " NOM_TABLE_LIEUXEXERCICE;
    int gidLieuExercice = 0;
    QList<QVariantList> lieuxlist = db->StandardSelectSQL(req, ok);
    if (ok && lieuxlist.size()>0)
        gidLieuExercice = lieuxlist.at(0).at(0).toInt(); //TODO : ICI
    req = "insert into " NOM_TABLE_JOINTURESLIEUX " (idUser, idLieu) VALUES(" + QString::number(idusr) + ", " + QString::number(gidLieuExercice) + ")";
    db->StandardSQL(req);
    req = "update " NOM_TABLE_PARAMSYSTEME " set idLieuParDefaut = " + QString::number(gidLieuExercice);
    db->StandardSQL(req, "void Procedures::CreerUserFactice(User &user)");
}


/*-----------------------------------------------------------------------------------------------------------------
    -- Identification de l'utilisateur -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::IdentificationUser(bool ChgUsr)
{
    dlg_identificationuser *dlg_IdentUser   = new dlg_identificationuser(ChgUsr);
    dlg_IdentUser   ->setFont(QFont(POLICEPARDEFAUT,POINTPARDEFAUT));
    bool a = false;

    int result = dlg_IdentUser->exec();

    if( result > 0 )
    {
        m_userConnected = db->getUserConnected();
        Datas::I()->villes->initListe();
        Datas::I()->sites->initListe();
        Datas::I()->comptes->initListe();
        SetUserAllData(m_userConnected);
        if (!VerifBaseEtRessources())
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de mettre à jour la base de données\nSortie du programme"));
            exit(0);
        }
        Verif_secure_file_priv();
        Datas::I()->users->initListe();
        if (DefinitRoleUser()) //NOTE : User Role
        {
            /* definit les iduser pour lequel le user travaille
                . iduser superviseur des actes                      (int gidUserSuperViseurProv)
                    . lui-même s'il est responsable de ses actes
                    . un autre user s'il est assistant
                    . -1 s'il est assistant pour plusieurs utilisateurs en même temps
                    . -2 sans objet
                . idUser parent
                    correspond au membre permanent de la structure de soins pour qui le user travaille
                    si le User est remplaçant -> le user remplacé, sinon, le usersuperviseur
                . iduser qui enregistrera la comptabilité des actes (int gidUserComptableProv)
                    . lui même s'il est responsable et libéral
                    . son employeur s'il est responsable et salarié
                    . s'il est remplaçant (retrocession) on lui demande qui il remplace et le comptable devient
                        . celui qu'il remplace si celui qu'il remplace est libéral
                        . l'employeur de  celui qu'il remplace si celui qu'il remplace est salarié
                    . -1 s'il n'enregistre pas de compta
                    . -2 sans objet
                . et s'il cote les actes                            (bool gUseCotationProv)
                    0 = AvecCompta
                    1 = SansCompta et sans cotation
                    2 = Sans compta mais avec cotation
                    3 = Avec compta mais sans cotation
           */
            m_userConnected->setTypeCompta(avecLaComptaProv ? (gUseCotation ? 0 : 4) : (gUseCotation ? 2 : 1));

            //AFFECT USER:
            //qDebug() << "superviseur " << m_userConnected->getIdUserActeSuperviseur();
            //qDebug() << "comptable " << m_userConnected->getIdUserComptable();
            //qDebug() << "parent " << m_userConnected->getIdUserParent();
            if( m_userConnected->getIdUserActeSuperviseur() > 0 )
                m_userConnected->setUserSuperviseur(Datas::I()->users->getById(m_userConnected->getIdUserActeSuperviseur()));
            if( m_userConnected->getIdUserComptable() > 0 )
                m_userConnected->setUserComptable(Datas::I()->users->getById(m_userConnected->getIdUserComptable()));
            if( m_userConnected->getIdUserParent() > 0 )
                m_userConnected->setUserParent(Datas::I()->users->getById(m_userConnected->getIdUserParent()));

            QList<QVariantList> lieuxlist = db->StandardSelectSQL("select Numcentre from " NOM_TABLE_PARAMSYSTEME, ok);
            if (ok && lieuxlist.size()>0)
                gidCentre = lieuxlist.at(0).at(0).toInt();
            a = true;
        }
    }
    else if (result == -1 || result == -2 || result == -5 ) // erreur de couple login-MDP
    {
        // traité en amont
        // -1 : erreur de couple login-MDP
        // -2 : erreur des droits utilisateur sur le serveur
        // -5 : table utilisateurs non vide mais utilisateur non référencé
    }
    //BUG : A SUPP, géré par le serveur
    else if (result == -3) // anomalie sur la base - table utilisateurs manquante ou corrompue
    {
        UpMessageBox     msgbox;
        UpSmallButton    AnnulBouton(tr("Annuler"));
        UpSmallButton    RestaureBaseBouton(tr("Restaurer la base depuis une sauvegarde"));
        UpSmallButton    BaseViergeBouton(tr("Nouvelle base patients vierge"));
        msgbox.setText(tr("Base de données endommagée!"));
        msgbox.setInformativeText(tr("La base de données semble endommagée.\n"
                                  "Voulez-vous la reconstruire à partir"
                                  " d'une sauvegarde ou recréer une base vierge?\n\n"));
        msgbox.setIcon(UpMessageBox::Info);
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&BaseViergeBouton, UpSmallButton::STARTBUTTON);
        msgbox.addButton(&RestaureBaseBouton, UpSmallButton::COPYBUTTON);
        msgbox.exec();
        if( (msgbox.clickedButton() == &RestaureBaseBouton) && RestaureBase(false,false,false))
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
            db->StandardSQL("delete from " NOM_TABLE_USERSCONNECTES);
            exit(0);
        }
        if (msgbox.clickedButton() == &BaseViergeBouton)
        {
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES);
            if (!RestaureBase(true, true))
                exit(0);
            // Création de l'utilisateur
            //TODO : ICI
            gdbOK = CreerPremierUser(m_userConnected->getLogin(), m_userConnected->getPassword());
            Datas::I()->users->initListe();
            UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer"), tr("Relancez-le pour que certaines données puissent être prises en compte"));
            db->StandardSQL("delete from " NOM_TABLE_USERSCONNECTES);
            exit(0);
        }
    }
    //BUG : A SUPP, géré par le serveur
    else if (result == -4) // table utilisateurs vide
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Erreur sur la base patients!"),
                               tr("La connexion au serveur fonctionne mais\n"
                                  "Il n'y a aucun utilisateur enregistré\n"
                                  "avec ce login dans la table des utilisateurs.\n"
                                  "Connectez-vous avec un login enregistré dans la base Rufus"));
        exit(0);
    }
    delete dlg_IdentUser;

    return a;
}

bool Procedures::DefinitRoleUser() //NOTE : User Role Function
{
    if (m_userConnected->isSoignant() )
    {
        QString req;
        gAskUser                = new UpDialog();
        gAskUser                ->AjouteLayButtons();
        gAskUser                ->setAccessibleName(QString::number(m_userConnected->id()));
        gAskUser                ->mData = m_userConnected;
        QVBoxLayout *boxlay     = new QVBoxLayout;
        gAskUser->dlglayout()   ->insertLayout(0,boxlay);

        QGroupBox *boxrole      = new QGroupBox(gAskUser);
        boxrole                 ->setAccessibleName("Role");
        QString lblRole         = tr("Quel est votre rôle dans cette session?");
        boxrole                 ->setTitle(lblRole);
        boxrole                 ->setVisible(false);
        boxlay                  ->addWidget(boxrole);

        QGroupBox *boxsuperv    = new QGroupBox(gAskUser);
        boxsuperv               ->setAccessibleName("Superv");
        QString lblSuperv       = tr("Qui supervise votre activité pour cette session?");
        boxsuperv               ->setTitle(lblSuperv);
        boxsuperv               ->setVisible(false);
        boxlay                  ->addWidget(boxsuperv);

        QGroupBox *boxparent     = new QGroupBox(gAskUser);
        boxparent               ->setAccessibleName("Parent");
        QString lblUsrParent    = tr("Qui enregistre vos honoraires?");
        boxparent               ->setTitle(lblUsrParent);
        boxparent               ->setVisible(false);
        boxlay                  ->addWidget(boxparent);

        // le user est responsable de ses actes - on cherche à savoir qui comptabilise ses actes
        if( m_userConnected->isResponsable() )
            Slot_CalcUserParent();

        // le user alterne entre responsable des actes et assistant suivant la session
        // on lui demande son rôle pour cette session
        else if( m_userConnected->isResponsableEtAssistant() )
        {
            bool found = false;
            for( QMap<int, User *>::const_iterator itUser = Datas::I()->users->all()->constBegin();
                 itUser != Datas::I()->users->all()->constEnd(); ++itUser )
            {
                User *us = const_cast<User*>(itUser.value());
                if( us->id() == m_userConnected->id() )
                    continue;
                if( !us->isResponsable() && !us->isResponsableEtAssistant() )
                    continue;

                found = true;
                break;
            }

            if (found)
            {
                boxrole                 ->setVisible(true);
                QFontMetrics fm         = QFontMetrics(qApp->font());
                int hauteurligne        = int(fm.height()*1.6);
                boxrole                 ->setFixedHeight((3*hauteurligne)+5);

                QVBoxLayout *vbox       = new QVBoxLayout;
                QRadioButton *pbuttResp = new QRadioButton(boxrole);
                pbuttResp               ->setText(tr("Responsable de mes actes"));
                pbuttResp               ->setAccessibleName("buttresp");
                vbox                    ->addWidget(pbuttResp);
                connect(pbuttResp, &QRadioButton::clicked, this, &Procedures::Slot_CalcUserParent);

                QRadioButton *pbuttAss  = new QRadioButton(boxrole);
                pbuttAss                ->setText(tr("Assistant"));
                pbuttAss                ->setAccessibleName("buttass");
                pbuttAss                ->setChecked(true);      // le user est défini par défaut comme assistant -> on cherche qui supervise les actes
                vbox                    ->addWidget(pbuttAss);
                connect(pbuttAss, &QRadioButton::clicked, this, &Procedures::Slot_CalcUserSuperviseur);

                vbox                    ->setContentsMargins(8,0,8,0);
                boxrole                 ->setLayout(vbox);
                dynamic_cast<QVBoxLayout*>(gAskUser->layout())->setSizeConstraint(QLayout::SetFixedSize);
                Slot_CalcUserSuperviseur();
            }
            else
            {
                // s'il ny a pas de responsable autre que lui dans la bbd,
                // il ne peut se connecter que comme responsable
                UpMessageBox::Watch(Q_NULLPTR,
                                    tr("Vous ne pourrez pas vous connecter en tant qu'assistant"),
                                    tr("Vous étes enregistré comme pouvant être assistant\n"
                                       "mais il n'y a aucun utilisateur susceptible de superviser\n"
                                       "vos actes enregistré dans la base de données"));
                Slot_CalcUserParent();
            }
        }

        // le user est assistant - on lui demande qui supervise ses actes
        else if( m_userConnected->isAssistant() )
            Slot_CalcUserSuperviseur();

        gAskUser                ->setModal(true);
        gAskUser->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
        connect(gAskUser->OKButton, &QPushButton::clicked, gAskUser, &UpDialog::accept);

        if( m_userConnected->getIdUserActeSuperviseur() == User::ROLE_INDETERMINE
                || m_userConnected->getIdUserParent() == User::ROLE_INDETERMINE )
        {
            if( gAskUser->exec() == 0 )
            {
                delete gAskUser;
                return false;
            }

            QList<QGroupBox*> Listgroupbx = gAskUser->findChildren<QGroupBox*>();
            QList<QGroupBox*>::const_iterator itGroup;
            for( itGroup=Listgroupbx.constBegin(); itGroup < Listgroupbx.constEnd(); ++itGroup )
            {
                QGroupBox *groupBox = const_cast<QGroupBox*>(*itGroup);
                if( !groupBox->isVisibleTo(gAskUser) )
                    continue;

                if (groupBox->accessibleName() == "Superv" )
                {
                    QList<QRadioButton*> listbutt = groupBox->findChildren<QRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                        {
                            m_userConnected->setIdUserActeSuperviseur(listbutt.at(j)->accessibleName().toInt());
                            break;
                        }
                }
                else if (groupBox->accessibleName() == "Parent" )
                {
                    QList<QRadioButton*> listbutt = groupBox->findChildren<QRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                        {
                            m_userConnected->setIdUserParent(listbutt.at(j)->accessibleName().toInt());
                            break;
                        }
                }
            }
            delete gAskUser;
        }
        if( m_userConnected->getIdUserActeSuperviseur() == User::ROLE_INDETERMINE )
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Aucun superviseur valide n'a été défini pour vos actes"), tr("Impossible de continuer"));
            return false;
        }

        if( m_userConnected->getIdUserActeSuperviseur() == User::ROLE_NON_RENSEIGNE )
        {
            // le user est assistant et travaille pour tout le monde
            m_userConnected->setIdUserParent(User::ROLE_NON_RENSEIGNE);
            m_userConnected->setIdUserComptable(User::ROLE_NON_RENSEIGNE);

            avecLaComptaProv = true;
            gUseCotation     = true;
        }
        else
        {
            // determination de comptabilité - cotation
            if( m_userConnected->getIdUserParent() == User::ROLE_INDETERMINE )
            {
                if( Datas::I()->users->getById( m_userConnected->getIdUserActeSuperviseur()) != Q_NULLPTR
                 && Datas::I()->users->getById( m_userConnected->getIdUserActeSuperviseur())->isRemplacant() )
                {
                    // le superviseur est remplaçant, on essaie de savoir s'il a un parent
                    QList<User*> listUserFound;
                    for( QMap<int, User *>::const_iterator itUser = Datas::I()->users->all()->constBegin();
                         itUser != Datas::I()->users->all()->constEnd(); ++itUser )
                    {
                        User *us = const_cast<User*>(itUser.value());
                        if( us->id() == m_userConnected->id() )
                            continue;
                        if( us->id() == m_userConnected->getIdUserActeSuperviseur() )
                            continue;
                        if( !us->isLiberal() && !us->isSalarie() )
                            continue;

                        listUserFound << us;
                    }
                    if (listUserFound.size() == 1)
                        m_userConnected->setIdUserParent( listUserFound.first()->id() );
                    else if( !listUserFound.isEmpty() )
                    {
                        // on va demander qui est le soignant parent de ce remplaçant....
                        gAskUser                = new UpDialog();
                        gAskUser                ->AjouteLayButtons();
                        gAskUser                ->setAccessibleName(QString::number(m_userConnected->getIdUserActeSuperviseur()));
                        gAskUser->mData         = Datas::I()->users->getById( m_userConnected->getIdUserActeSuperviseur());
                        QVBoxLayout *boxlay     = new QVBoxLayout;
                        gAskUser->dlglayout()   ->insertLayout(0,boxlay);
                        QGroupBox*boxparent     = new QGroupBox(gAskUser);
                        boxparent               ->setAccessibleName("Parent");
                        QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + Datas::I()->users->getById(m_userConnected->getIdUserActeSuperviseur())->getLogin() + "?";
                        boxparent               ->setTitle(lblUsrParent);
                        boxparent               ->setVisible(false);
                        boxlay                  ->addWidget(boxparent);

                        Slot_CalcUserParent();
                        gAskUser                ->setModal(true);
                        gAskUser->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
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
                                        {
                                            //gidUserParentProv = listbutt.at(j)->accessibleName().toInt();
                                            m_userConnected->setIdUserParent( listbutt.at(j)->accessibleName().toInt() );
                                            break;
                                        }
                                }
                            }
                            delete gAskUser;
                        }
                    }
                }
                else
                    m_userConnected->setIdUserParent( m_userConnected->getIdUserActeSuperviseur() );
            }
            if( Datas::I()->users->getById(m_userConnected->getIdUserParent()) != Q_NULLPTR )
            {
                // determination de l'utilisation de la cotation
                gUseCotation = Datas::I()->users->getById(m_userConnected->getIdUserParent())->isCotation();
                // determination de l'utilisation de la comptabilité
                avecLaComptaProv = !Datas::I()->users->getById(m_userConnected->getIdUserParent())->isSansCompta();
                if( Datas::I()->users->getById(m_userConnected->getIdUserParent())->isLiberal() )
                    m_userConnected->setIdUserComptable(Datas::I()->users->getById(m_userConnected->getIdUserParent())->id());
                else if( Datas::I()->users->getById(m_userConnected->getIdUserParent())->isSalarie() )
                    m_userConnected->setIdUserComptable(Datas::I()->users->getById(m_userConnected->getIdUserParent())->getEmployeur());
                else
                    m_userConnected->setIdUserComptable(User::ROLE_NON_RENSEIGNE);
            }
        }
        return true;
    }

    // il s'agit d'un administratif ou d'une société comptable
    m_userConnected->setIdUserActeSuperviseur(User::ROLE_VIDE);
    m_userConnected->setIdUserComptable(User::ROLE_VIDE);
    m_userConnected->setIdUserParent(User::ROLE_VIDE);
    gUseCotation     = true;
    avecLaComptaProv = true; //FIXME : avecLaComptaProv
    return true;
}

/*!
 * \brief Procedures::SetUserAllData(User *usr)
 * Charge les données d'un utilisateur, y compris ses données bancaires
 * cette fonction fait appel aux deux classes cls_user et cls_compte
 * et ne peut pas figurer dans la classe cls_user
 * en raison de référence croisées
 */
bool Procedures::SetUserAllData(User *usr)
{
    if (!usr->isAllLoaded())
    {
        QJsonObject data = db->loadUserData(usr->id());
        if(data.isEmpty())
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Les paramètres de ")
                                + usr->getLogin() + tr("ne sont pas retrouvés"));
            return false;
        }
        usr->setData( data ); //on charge le reste des données
    }
    dlg_gestioncomptes::ReconstruitListeComptes(usr);
    usr->setCompteParDefaut(Datas::I()->comptes->getById(usr->getIdCompteParDefaut()));
    usr->setCompteEncaissement(Datas::I()->comptes->getById(usr->getIdCompteEncaissHonoraires()));
    return true;
}

/*!
 * \brief Procedures::Slot_CalcUserSuperviseur
 *
 * Prépare le UpDialog en ajoutant si besoin une liste d'User
 * susceptible d'être le Parent
 *
 * Résultat :
 * gidUserParentProv = -3
 * gidUserSuperViseurProv = -3 || id
 */
void Procedures::Slot_CalcUserSuperviseur()
{
    User *user = qobject_cast<User *>(gAskUser->mData);
    m_userConnected->setIdUserActeSuperviseur(User::ROLE_INDETERMINE);
    m_userConnected->setIdUserParent(User::ROLE_INDETERMINE);
    QGroupBox *ptbox = Q_NULLPTR;
    QList<QGroupBox*> Listgroupbx   = gAskUser->findChildren<QGroupBox*>();
    for (int i=0; i<Listgroupbx.size(); i++)
    {
        if (Listgroupbx.at(i)->accessibleName() == "Superv")
            ptbox = Listgroupbx.at(i);
        else if (Listgroupbx.at(i)->accessibleName() == "Parent")
            Listgroupbx.at(i)->setVisible(false);
    }
    ptbox->setVisible(false);

    QList<QRadioButton*> listbutt = ptbox->findChildren<QRadioButton*>();
    foreach(QRadioButton * rb, listbutt)
        delete rb;
    delete ptbox->layout();

    QList<User*> listUserFound;
    for( QMap<int, User *>::const_iterator itUser = Datas::I()->users->all()->constBegin();
         itUser != Datas::I()->users->all()->constEnd(); ++itUser )
    {
        User *us = const_cast<User*>(itUser.value());
        if( us->id() == user->id() )
            continue;
        if( !us->isResponsable() && !us->isResponsableEtAssistant() )
            continue;
        if( m_userConnected->isMedecin() && !us->isMedecin() )
            continue;

        listUserFound << us;
    }

    if( listUserFound.size() == 1 )
    {
        m_userConnected->setIdUserActeSuperviseur( listUserFound.first()->id() );
    }
    else if( !listUserFound.isEmpty() )
    {
        ptbox->setVisible( true );

        QFontMetrics fm         = QFontMetrics(qApp->font());
        int hauteurligne        = int(fm.height()*1.6);
        ptbox                   ->setFixedHeight(((listUserFound.size() + 2)*hauteurligne)+5);
        QVBoxLayout *vbox       = new QVBoxLayout;
        bool isFirst = true;
        QList<User*>::const_iterator it;
        for( it=listUserFound.constBegin(); it!=listUserFound.constEnd(); ++it )
        {
            User *us = const_cast<User*>(*it);
            QRadioButton *pradiobutt = new QRadioButton(ptbox);
            pradiobutt->setText(us->getLogin());
            pradiobutt->setAccessibleName(QString::number(us->id()));
            if( isFirst )
            {
                isFirst = false;
                pradiobutt->setChecked(true);
            }
            vbox->addWidget(pradiobutt);
        }
        QRadioButton *pradiobutt = new QRadioButton();
        pradiobutt   ->setText(tr("Tout le monde"));
        pradiobutt   ->setAccessibleName("-1");
        vbox         ->addWidget(pradiobutt);
        vbox         ->setContentsMargins(8,0,8,0);
        ptbox        ->setLayout(vbox);
    }
    else
    {
        // ??? Cas ou list est vide ?
        // SL -> il n'y a aucun superviseur valide => il faut refuser la connexion
    }
    gAskUser->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
}

/*!
 * \brief Procedures::Slot_CalcUserParent
 * Prépare le UpDialog en ajoutant si besoin une liste d'User
 * susceptibles d'être le Parent
 *
 * Résultat :
 * gidUserSuperViseurProv = user->id()
 * gidUserParentProv = id || -3 || user->id()
 */
void Procedures::Slot_CalcUserParent()
{
    User *user = qobject_cast<User *>(gAskUser->mData);
    //gidUserSuperViseurProv = user->id();
    user->setIdUserActeSuperviseur( user->id() );
    QGroupBox *ptbox = Q_NULLPTR;
    QList<QGroupBox*> Listgroupbx = gAskUser->findChildren<QGroupBox*>();
    for (int i=0; i<Listgroupbx.size(); i++)
    {
        if (Listgroupbx.at(i)->accessibleName() == "Superv")
            Listgroupbx.at(i)->setVisible(false);
        else if (Listgroupbx.at(i)->accessibleName() == "Parent")
            ptbox = Listgroupbx.at(i);
    }
    ptbox->setVisible(false);

    // on a déterminé le superviseur, on cherche qui enregistre les actes
    if( user->isRemplacant() )
    {
        // *  3. le superviseur est remplaçant -> il faut lui demander qui il remplace
        /* on fait la liste des user susceptibles d'être remplacés
         * . les user de même profession
         * . qui sont enregistrés comme liberaux ou salaries : champ UserEnregHonoraires de Utilisateurs = 1 ou 2
         */

        QList<User*> listUserFound;
        for( QMap<int, User *>::const_iterator itUser = Datas::I()->users->all()->constBegin();
             itUser != Datas::I()->users->all()->constEnd(); ++itUser )
        {
            User *us = const_cast<User*>(itUser.value());
            if( us->id() == user->id() )
                continue;
            if( !us->isLiberal() && !us->isSalarie() )
                continue;
            if( us->getSoignant() != user->getSoignant() )
                continue;
            listUserFound << us;
        }

        if( listUserFound.size() == 1 )
        {
            //gidUserParentProv = listUserFound.first()->id();
            user->setIdUserParent( listUserFound.first()->id() );
        }
        else if( !listUserFound.isEmpty() )
        {
            ptbox->setVisible( true );
            QList<QRadioButton*> listbutt = ptbox->findChildren<QRadioButton*>();
            foreach(QRadioButton * rb, listbutt)
                delete rb;
            delete ptbox->layout();

            QFontMetrics fm  = QFontMetrics(qApp->font());
            int hauteurligne = int(fm.height()*1.6);
            ptbox->setFixedHeight(((listUserFound.size() + 1)*hauteurligne)+5);
            QVBoxLayout *vbox = new QVBoxLayout;
            bool isFirst = true;
            QList<User*>::const_iterator it;
            for( it=listUserFound.constBegin(); it!=listUserFound.constEnd(); ++it )
            {
                User *us = const_cast<User*>(*it);
                QRadioButton *pradiobutt = new QRadioButton(ptbox);
                pradiobutt->setText(us->getLogin());
                pradiobutt->setAccessibleName(QString::number(us->id()));
                if( isFirst )
                {
                    isFirst = false;
                    pradiobutt->setChecked(true);
                }
                vbox->addWidget(pradiobutt);
            }
            vbox->setContentsMargins(8,0,8,0);
            ptbox->setLayout(vbox);
        }
        else
        {
            // ??? Cas ou list est vide ?
            // SL -> le remplaçant n'a personne à remplacer => il faut refuser la connexion
        }
    }
    else
    {
        // *  1|2. le superviseur est salarié ou libéral  -> c'est lui qui est le parent
        // *  4.   le superviseur est sans compta         -> pas de compta des actes
        //gidUserParentProv = gidUserSuperViseurProv;
        user->setIdUserParent( user->id() );
    }
    gAskUser->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
}

int Procedures::idCentre()
{
    return gidCentre;
}

int Procedures::idLieuExercice()
{
    if( m_userConnected )
        return m_userConnected->getSite()->id();
    return -1;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Premier démarrage de Rufus - reconstruction du fichier Rufus.ini et de la base ---------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Procedures::PremierDemarrage() //TODO : CONFIG
{
    QMessageBox     msgbox;
    int         protoc;
    enum protoc {BaseExistante, BaseRestauree, BaseVierge};
    UpSmallButton    AnnulBouton        (tr("Retour\nau menu d'accueil"));
    UpSmallButton    BaseRestaureeBouton(tr("Base patients restaurée\ndepuis une sauvegarde"));
    UpSmallButton    BaseViergeBouton (tr("Nouvelle base\npatients vierge"));
    UpSmallButton    BaseExistanteBouton(tr("Base patients existante\nsur le serveur"));

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

    msgbox.addButton(&AnnulBouton,          QMessageBox::RejectRole);
    msgbox.addButton(&BaseExistanteBouton,  QMessageBox::AcceptRole);
    msgbox.addButton(&BaseRestaureeBouton,  QMessageBox::YesRole);
    msgbox.addButton(&BaseViergeBouton,     QMessageBox::ActionRole);
    msgbox.exec();

    protoc = BaseExistante;
    if (msgbox.clickedButton() == &AnnulBouton)
        return false;
    else if (msgbox.clickedButton() == &BaseExistanteBouton)
        protoc = BaseExistante;
    else if (msgbox.clickedButton() == &BaseRestaureeBouton)
        protoc = BaseRestauree;
    else if (msgbox.clickedButton() == &BaseViergeBouton)
        protoc = BaseVierge;

    gsettingsIni    = new QSettings(gnomFichIni, QSettings::IniFormat);

    if (protoc == BaseExistante)
    {
        if (VerifParamConnexion())
        {
            int idusr = VerifUserBase(m_userConnected->getLogin(),m_userConnected->getPassword());
            gdbOK = (idusr > -1);
            if (!gdbOK)
                return false;
            //gidUser     = idusr; //TODO : ICI
            PremierParametrageMateriel();
            PremierParametrageRessources();
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne,\n"
                                       "le login ") + m_userConnected->getLogin() + tr(" est reconnu et le programme va démarrer\n"));
            if( DefinitRoleUser() ) //NOTE : User Role : 1er demarrage
            {
                m_userConnected->setTypeCompta(avecLaComptaProv ? (gUseCotation ? 0 : 4) : (gUseCotation ? 2 : 1));
                return true;
            }
        }
    }

    else if (protoc == BaseRestauree)
    {
        bool SansAccesDistant = false;
        if (VerifParamConnexion(SansAccesDistant))
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Connexion réussie"),
                                  tr("Bien, la connexion au serveur MySQL fonctionne,\n"));
            // Restauration de la base
            if (!RestaureBase(false, true, false))
                return false;
            PremierParametrageMateriel();
            PremierParametrageRessources();
            int idusr = VerifUserBase(m_userConnected->getLogin(),m_userConnected->getPassword());
            gdbOK = (idusr > -1);
            if (!gdbOK)
                return false;
            //gidUser     = idusr; //TODO : ICI
            UpMessageBox::Watch(Q_NULLPTR, tr("Redémarrage nécessaire"),
                                   tr("Le programme va se fermer pour que les modifications de la base Rufus\n"
                                      "puissent être prises en compte\n"));
            db->StandardSQL("delete from " NOM_TABLE_USERSCONNECTES);
            exit(0);
        }
    }
    else if (protoc == BaseVierge)
    {
        bool SansAccesDistant = false;
        if (VerifParamConnexion(SansAccesDistant))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne "
                                       "et le programme va maintenant créer une base de données patients "
                                       "vierge de tout enregistrement."));
            // Création des dossiers
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_RESSOURCES);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_IMAGES);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_ECHECSTRANSFERTS);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_DOSSIERECHANGE);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_VIDEOS);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_PROV);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_FACTURESSANSLIEN);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_FACTURES);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_ORIGINAUX NOMDIR_FACTURES);
            Utils::mkpath(QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE NOMDIR_ORIGINAUX NOMDIR_IMAGES);
            // Création de la base
            if (!RestaureBase(true, true))
                return false;
            if (gMode2 == DataBase::ReseauLocal)
                db->StandardSQL("update " NOM_TABLE_PARAMSYSTEME " set AdresseServeurLocal = '" + gsettingsIni->value("BDD_LOCAL/Serveur").toString() + "'");

            // Création de l'utilisateur
            gdbOK = CreerPremierUser(m_userConnected->getLogin(), m_userConnected->getPassword());
            db->login(m_userConnected->getLogin(), m_userConnected->getPassword());
            SetUserAllData(m_userConnected);
            Datas::I()->users->initListe();
            UpMessageBox::Watch(Q_NULLPTR, tr("Redémarrage nécessaire"),
                                   tr("Le programme va se fermer pour que les modifications de la base Rufus\n"
                                      "puissent être prises en compte\n"));
            db->StandardSQL("delete from " NOM_TABLE_USERSCONNECTES);
            exit(0);
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
    QString NomDirImg = QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE;
    db->StandardSQL("update " NOM_TABLE_PARAMSYSTEME " set DirImagerie = '" + NomDirImg + "'", "void Procedures::PremierParametrageMateriel()");
    gsettingsIni->setValue("BDD_DISTANT/DossierImagerie", NomDirImg);
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
    if (gMode2 == DataBase::Poste)
    {
        QString NomDirImg = QDir::homePath() + NOMDIR_RUFUS NOMDIR_IMAGERIE;
        QString reqimg = "update " NOM_TABLE_PARAMSYSTEME " set DirImagerie = '" + NomDirImg + "'";
        db->StandardSQL( reqimg, "void Procedures::PremierParametrageRessources()");
    }
 }

/*------------------------------------------------------------------------------------------------------------------------------------
-- Vérifie la présence et la cohérence du fchier d'initialisation et le reconstruit au besoin ----------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifIni(QString msg, QString msgInfo, bool DetruitIni, bool RecupIni, bool ReconstruitIni, bool ReconstruitBase, bool PremDemarrage, bool BaseVierge)
{
    UpSmallButton AnnulBouton              (tr("Abandonner et\nquitter Rufus"));
    UpSmallButton RecupIniBouton           (tr("Restaurer le fichier d'initialisation\nà partir d'une sauvegarde"));
    UpSmallButton ReconstruitIniBouton     (tr("Reconstruire le fichier\nd'initialisation"));
    UpSmallButton ReconstruitBaseBouton    (tr("Reconstruire les données\nà partir d'un fichier de sauvegarde"));
    UpSmallButton PremierDemarrageBouton   (tr("Premier démarrage\nde Rufus"));
    UpSmallButton BaseViergeBouton         (tr("Reconstruire\nune base vierge"));

    QMessageBox *msgbox = new QMessageBox;
    msgbox->setText(msg);
    msgbox->setInformativeText(msgInfo);
    msgbox->setIcon(QMessageBox::Warning);
    if (ReconstruitIni)                     msgbox->addButton(&ReconstruitIniBouton,     QMessageBox::AcceptRole);
    if (RecupIni)                           msgbox->addButton(&RecupIniBouton,           QMessageBox::AcceptRole);
    if (ReconstruitBase)                    msgbox->addButton(&ReconstruitBaseBouton,    QMessageBox::AcceptRole);
    if (PremDemarrage)                      msgbox->addButton(&PremierDemarrageBouton,   QMessageBox::AcceptRole);
    if (BaseVierge)                         msgbox->addButton(&BaseViergeBouton,         QMessageBox::AcceptRole);
    msgbox->addButton(&AnnulBouton, QMessageBox::AcceptRole);
    msgbox->exec();
    bool reponse = false;

    if (msgbox->clickedButton()==&AnnulBouton)
    {
        if (DetruitIni)
            QFile::remove(gnomFichIni);
        exit(0);
    }
    else if (msgbox->clickedButton()==&RecupIniBouton)
    {
        QFileDialog dialog(Q_NULLPTR, tr("Choisir le fichier d'initialisation"), QDir::homePath() + "/Documents/Rufus/","Text files (Rufus*.ini)");
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
            if (QMessageBox::question(Q_NULLPTR,"", tr("Restaurer aussi les fichiers modèles d'impression?")) != QMessageBox::NoButton)
                PremierParametrageRessources();
            reponse = true;
        }
    }
    else if (msgbox->clickedButton()==&ReconstruitIniBouton)
    {
        //reconstruire le fichier rufus.ini
        //1. on demande les paramètres de connexion au serveur - mode d'accès / user / mdp / port / SSL
        QFile(gnomFichIni).remove();
        gsettingsIni    = new QSettings(gnomFichIni, QSettings::IniFormat);
        reponse = VerifParamConnexion();
        if (reponse)
        {
            int idusr = VerifUserBase(m_userConnected->getLogin(), m_userConnected->getPassword());
            gdbOK = (idusr > -1);
            if (!gdbOK)
            {
                delete msgbox;
                return false;
            }
            //gidUser     = idusr; //TODO : ICI
            PremierParametrageMateriel();
            UpMessageBox::Watch(Q_NULLPTR,tr("Le fichier Rufus.ini a été reconstruit"), tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
            exit(0);
        }
    }
    else if (msgbox->clickedButton()==&ReconstruitBaseBouton)
    {
        //reconstruire la base de données vierge ou à partir d'un dump
        reponse = RestaureBase(false, false, false);
        if (reponse)
            UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
        else
            UpMessageBox::Watch(Q_NULLPTR,tr("Restauration impossible de la base"));
        db->StandardSQL("delete from " NOM_TABLE_USERSCONNECTES);
        exit(0);
    }
    else if (msgbox->clickedButton()==&PremierDemarrageBouton)
    {
        reponse =  PremierDemarrage();
    }
    else if (msgbox->clickedButton()==&BaseViergeBouton)
    {
        reponse = RestaureBase(true, true, false);
        if (reponse)
            UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
        else
            UpMessageBox::Watch(Q_NULLPTR,tr("Restauration impossible de la base"));
        db->StandardSQL("delete from " NOM_TABLE_USERSCONNECTES);
        exit(0);
    }
    return reponse;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Vérifie et répare les paramètres de connexion  -----------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
//???
bool Procedures::VerifParamConnexion(bool OKAccesDistant, QString)
{
    Dlg_ParamConnex = new dlg_paramconnexion(OKAccesDistant);
    Dlg_ParamConnex ->setWindowTitle(tr("Entrez les paramètres de connexion au serveur"));
    Dlg_ParamConnex ->setFont(QFont(POLICEPARDEFAUT,POINTPARDEFAUT));

    if (Dlg_ParamConnex->exec()>0)
    {
        QString Base;
        if (Dlg_ParamConnex->ui->PosteradioButton->isChecked())
        {
            Base = "BDD_POSTE";
            gMode2 = DataBase::Poste;
        }
        else if (Dlg_ParamConnex->ui->LocalradioButton->isChecked())
        {
            Base = "BDD_LOCAL";
            gsettingsIni->setValue(Base + "/Serveur",   Dlg_ParamConnex->ui->IPlineEdit->text());
            gMode2 = DataBase::ReseauLocal;
        }
        else if (Dlg_ParamConnex->ui->DistantradioButton->isChecked())
        {
            Base = "BDD_DISTANT";
            gsettingsIni->setValue(Base + "/Serveur",   Dlg_ParamConnex->ui->IPlineEdit->text());
            gMode2 = DataBase::Distant;
        }
        gsettingsIni->setValue(Base + "/Active",    "YES");
        gsettingsIni->setValue(Base + "/Port", Dlg_ParamConnex->ui->PortcomboBox->currentText());

        m_userConnected = new User(Dlg_ParamConnex->ui->LoginlineEdit->text(),Dlg_ParamConnex->ui->MDPlineEdit->text());

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
    UpSmallButton OKBouton(tr("Annuler"));
    UpSmallButton RestaurerBouton(tr("Reconstruire les fichiers à partir d'une sauvegarde"));
    UpSmallButton RemplirBouton(tr("Réinitialiser les fichiers"));
    msgbox.setText(tr("Il manque un fichier d'impression"));
    msgbox.setInformativeText(tr("Le fichier ressource d'impression ") + Nomfile + tr(" est absent.\n"
                              "Voulez vous restaurer les fichiers ressources d'impression?.\n"));
    msgbox.setIcon(QMessageBox::Warning);
    msgbox.addButton(&OKBouton, QMessageBox::RejectRole);
    msgbox.addButton(&RemplirBouton, QMessageBox::AcceptRole);
    msgbox.addButton(&RestaurerBouton, QMessageBox::AcceptRole);
    msgbox.exec();
    if (msgbox.clickedButton()==&OKBouton)
        return false;
    else if (msgbox.clickedButton()==&RemplirBouton)
        PremierParametrageRessources();
    else if (msgbox.clickedButton()==&RestaurerBouton)
    {
        QFileDialog dialog(Q_NULLPTR, tr("Choisir le dossier de ressources d'impression à restaurer"), QDir::homePath() + "/dumpsRufus","SQL files (dump*.sql)");
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
    UpSmallButton OKBouton("OK");
    msgbox.setIcon(QMessageBox::Information);
    msgbox.addButton(&OKBouton, QMessageBox::AcceptRole);
    QString req = "SHOW TABLES FROM " NOM_BASE_CONSULTS " LIKE 'utilisateurs'";
    QVariantList verifbasedata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (!ok || verifbasedata.size()==0)
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
            " WHERE UserLogin = '" + Utils::correctquoteSQL(Login) +
            "' AND UserMDP = '" + Utils::correctquoteSQL(MDP) + "'" ;
    QVariantList idusrdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (!ok || idusrdata.size()==0)
    {
        req =   "SELECT UserLogin FROM " NOM_TABLE_UTILISATEURS;
        QList<QVariantList> usrlist = db->StandardSelectSQL(req, ok);
        if (!ok || usrlist.size()==0)
        {
            msgbox.setText(tr("Erreur sur la base patients"));
            msgbox.setInformativeText(tr("La connexion au serveur fonctionne mais "
                                      "votre base semble endommagée.\n"
                                      "Il n'y a aucun utilisateur enregistré "
                                      "dans la table des utilisateurs.\n"
                                      "Impossible de continuer."));
            msgbox.exec();
            return -2;
        }
        QString listusr;
        bool ExistLogin = false;
        for (int i=0; i< usrlist.size(); i++)
        {
            if (usrlist.at(i).at(0).toString() == Login)
                ExistLogin = true;
            else
                listusr += "\n\t" + usrlist.at(i).at(0).toString();
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
    return idusrdata.at(0).toInt();
}

void Procedures::VideDatabases()
{
    Message(tr("Suppression de l'ancienne base Rufus en cours"));
    db->StandardSQL ("drop database if exists " NOM_BASE_COMPTA );
    db->StandardSQL ("drop database if exists " NOM_BASE_OPHTA );
    db->StandardSQL ("drop database if exists " NOM_BASE_CONSULTS );
    db->StandardSQL ("drop database if exists " NOM_BASE_IMAGES );
}

/* ------------------------------------------------------------------------------------------------------------------------------------------


GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Ports_Series()
{
    QString NomPort             = "";
    gPortRefracteur             = "";
    gPortFronto                 = "";
    gPortAutoref                = "";
    gPortTono                   = "";
    lPortRefracteur             = Q_NULLPTR;
    lPortFronto                 = Q_NULLPTR;
    lPortAutoref                = Q_NULLPTR;
    lPortTono                   = Q_NULLPTR;
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
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial") | QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
            {
                portseriedispo = true;
                break;
            }
        }
        if (!portseriedispo)
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion série"),
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
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion frontofocomètre"));
        for (int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
        {
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
            {
                if (gPortFronto == "COM1") NomPort = "A";
                else if (gPortFronto == "COM2") NomPort = "B";
                else if (gPortFronto == "COM3") NomPort = "C";
                else if (gPortFronto == "COM4") NomPort = "D";
                if (NomPort != "") break;
            }
            else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
            {
                if (gPortFronto == "COM1") NomPort = "ttyUSB0";
                else if (gPortFronto == "COM2") NomPort = "ttyUSB1";
                else if (gPortFronto == "COM3") NomPort = "ttyUSB2";
                else if (gPortFronto == "COM4") NomPort = "ttyUSB3";
                if (NomPort != "") break;
            }
        }

        if (NomPort != "")
        {
            lPortFronto     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                //Debug() << QSerialPortInfo::availablePorts().at(i).portName();
                //UpMessageBox::Watch(this,QSerialPortInfo::availablePorts().at(i).portName());
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
                {
                    QString letter = QSerialPortInfo::availablePorts().at(i).portName().split("-").at(1);
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort || letter.left(1) == NomPort)
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
                else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName() == NomPort)
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
            qDebug() << "FRONTO -> " + gPortFronto + " - " + NomPort;
            if (lPortFronto->open(QIODevice::ReadWrite))
            {
                ThreadFronto = new SerialThread(lPortFronto);
                ThreadFronto->transaction();
                connect(ThreadFronto,  SIGNAL(reponse(QString)),     this, SLOT(Slot_ReponsePortSerie_Fronto(QString)));
            }
            else
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Connexion impossible"),tr("Impossible de connecter le frontofocomètre") + "\n" + lPortFronto->errorString());
                lPortFronto = Q_NULLPTR;
            }
        }
    }

    // PORT REFRACTEUR
    if (gRefracteurParametre)
    {
        gPortRefracteur = gsettingsIni->value("Param_Poste/PortRefracteur").toString();
        bool a          = ReglePortRefracteur();
        a               = (gPortRefracteur != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion refracteur"));
        for (int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
        {
            //qDebug() << QSerialPortInfo::availablePorts().at(i).portName();
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
            {
                if (gPortRefracteur == "COM1") NomPort = "A";
                else if (gPortRefracteur == "COM2") NomPort = "B";
                else if (gPortRefracteur == "COM3") NomPort = "C";
                else if (gPortRefracteur == "COM4") NomPort = "D";
                if (NomPort != "") break;
            }
            else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
            {
                if (gPortRefracteur == "COM1") NomPort = "ttyUSB0";
                else if (gPortRefracteur == "COM2") NomPort = "ttyUSB1";
                else if (gPortRefracteur == "COM3") NomPort = "ttyUSB2";
                else if (gPortRefracteur == "COM4") NomPort = "ttyUSB3";
                if (NomPort != "") break;
            }
        }
        if (NomPort != "")
        {
            lPortRefracteur     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
                {
                    QString letter = QSerialPortInfo::availablePorts().at(i).portName().split("-").at(1);
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort || letter.left(1) == NomPort)
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
                else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName() == NomPort)
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
            qDebug() << "REFRACTEUR -> " + gPortRefracteur + " - " + NomPort;
            if (lPortRefracteur->open(QIODevice::ReadWrite))
            {
                ThreadRefracteur     = new SerialThread(lPortRefracteur);
                ThreadRefracteur    ->transaction();
                connect(ThreadRefracteur,  SIGNAL(reponse(QString)),     this, SLOT(Slot_ReponsePortSerie_Refracteur(QString)));
            }
            else
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Connexion impossible"),tr("Impossible de connecter le refracteur") + "\n" + lPortRefracteur->errorString());
                lPortRefracteur = Q_NULLPTR;
            }
        }
    }

    //PORT AUTOREF
    if (gAutorefParametre)
    {
        gPortAutoref    = gsettingsIni->value("Param_Poste/PortAutoref").toString();
        bool a          = ReglePortAutoref();
        a               = (gPortAutoref != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion autorefractomètre"));
        for (int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
        {
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
            {
                if (gPortAutoref == "COM1") NomPort = "A";
                else if (gPortAutoref == "COM2") NomPort = "B";
                else if (gPortAutoref == "COM3") NomPort = "C";
                else if (gPortAutoref == "COM4") NomPort = "D";
                if (NomPort != "") break;
            }
            else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
            {
                if (gPortAutoref == "COM1") NomPort = "ttyUSB0";
                else if (gPortAutoref == "COM2") NomPort = "ttyUSB1";
                else if (gPortAutoref == "COM3") NomPort = "ttyUSB2";
                else if (gPortAutoref == "COM4") NomPort = "ttyUSB3";
                if (NomPort != "") break;
            }
        }
        if (NomPort != "")
        {
            lPortAutoref     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
                {
                    QString letter = QSerialPortInfo::availablePorts().at(i).portName().split("-").at(1);
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort || letter.left(1) == NomPort)
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
                else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName() == NomPort)
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
            qDebug() << "AUTOREF -> " + gPortAutoref + " - " + NomPort;
            if (lPortAutoref->open(QIODevice::ReadWrite))
            {
                ThreadAutoref     = new SerialThread(lPortAutoref);
                ThreadAutoref   ->transaction();
                connect(ThreadAutoref,  SIGNAL(reponse(QString)),     this, SLOT(Slot_ReponsePortSerie_Autoref(QString)));
            }
            else
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Connexion impossible"),tr("Impossible de connecter l'autorefractomètre") + "\n" + lPortAutoref->errorString());
                lPortAutoref = Q_NULLPTR;
            }
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
        UpMessageBox::Watch(Q_NULLPTR,tr("pas de données reçues du refracteur"));
        return;
    }
    setTypeMesureRefraction(Subjectif);
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
            //debugformule(MesureAutoref, "Autoref");
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
            DTRbuff.append("DRM");                              //section autoref
            DTRbuff.append(QByteArray::fromHex("2"));           //STX -> start of text
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
            //debugformule(MesureFronto, "Fronto");
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
            DTRbuff.append("DLM");                              //section fronto
            DTRbuff.append(QByteArray::fromHex("2"));           //STX -> start of text
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
        qDebug() << QString(Data);
    }
}
void Procedures::debugformule(QMap<QString, QVariant> Data, QString type)
{
    auto calcvaleur = [] (QString Mesure)
    {
        if (Mesure.size()>2)
            if (Mesure.at(1) == "0" and Mesure.at(2).isNumber())
                Mesure.replace(1,1,"");
        return Mesure;
    };
    auto calcdouble = [] (QString Mesure)
    {
        if (Mesure.size()>2)
            if (Mesure.at(1) == "0" and Mesure.at(2).isNumber())
                Mesure.replace(1,1,"");
        if (Mesure.size()>1)
            if (Mesure.at(0) == "+")
                Mesure.replace(0,1,"");
        return Mesure.toDouble();
    };
    QString Formule = "OD : " + calcvaleur(Data["SphereOD"].toString());
    if (calcdouble(Data["CylOD"].toString()) != 0.0)
        Formule += "(" + calcvaleur(Data["CylOD"].toString()) + " à " + Data["AxeOD"].toString() + "°)";
    Formule +=  " add." + calcvaleur(Data["AddOD"].toString()) +  " VP";
    qDebug() << type;
    qDebug() << Formule;
    Formule = "OG : " + calcvaleur(Data["SphereOG"].toString());
    if (calcdouble(Data["CylOG"].toString()) != 0.0)
        Formule += "(" + calcvaleur(Data["CylOG"].toString()) + " à " + Data["AxeOG"].toString() + "°)";
    Formule +=  " add." + calcvaleur(Data["AddOG"].toString()) +  " VP";
    qDebug() << Formule;
}

void Procedures::SetDataAEnvoyerAuRefracteur(QMap<QString, QVariant> DataFronto, QMap<QString, QVariant> DataAutoref)
{
    debugformule(DataFronto, "Fronto");
    debugformule(DataAutoref, "Autoref");
    ClearMesures();
    MesureAutoref       = DataAutoref;
    MesureFronto        = DataFronto;
    NouvMesureFronto    = false;
    NouvMesureAutoref   = false;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (ThreadRefracteur!=Q_NULLPTR)
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
        QString mSphereOD   = Utils::PrefixePlus(MesureRefracteurSubjectif["SphereOD"].toString());
        QString mCylOD      = Utils::PrefixePlus(MesureRefracteurSubjectif["CylOD"].toString());
        QString mAxeOD      = QString::number(MesureRefracteurSubjectif["AxeOD"].toInt());
        QString mAddOD      = Utils::PrefixePlus(MesureRefracteurSubjectif["AddOD"].toString());
        QString mAVLOD ("");
        if (MesureRefracteurSubjectif["AVLOD"].toDouble()>0)
            mAVLOD      = QLocale().toString(MesureRefracteurSubjectif["AVLOD"].toDouble()*10) + "/10";
        QString mAVPOD      = MesureRefracteurSubjectif["AVPOD"].toString();
        QString mSphereOG   = Utils::PrefixePlus(MesureRefracteurSubjectif["SphereOG"].toString());
        QString mCylOG      = Utils::PrefixePlus(MesureRefracteurSubjectif["CylOG"].toString());
        QString mAxeOG      = QString::number(MesureRefracteurSubjectif["AxeOG"].toInt());
        QString mAddOG      = Utils::PrefixePlus(MesureRefracteurSubjectif["AddOG"].toString());
        QString mAVLOG ("");
        if (MesureRefracteurSubjectif["AVLOG"].toDouble()>0)
            mAVLOG      = QLocale().toString(MesureRefracteurSubjectif["AVLOG"].toDouble()*10) + "/10";
        QString mAVPOG      = MesureRefracteurSubjectif["AVPOG"].toString();
        QString ResultatVLOD, ResultatVLOG,ResultatVPOD, ResultatVPOG, ResultatOD, ResultatOG;

        // détermination OD
        if (QLocale().toDouble(mCylOD) != 0.0 && QLocale().toDouble(mSphereOD) != 0.0)
            ResultatVLOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
        else if (QLocale().toDouble(mCylOD) == 0.0 && QLocale().toDouble(mSphereOD) != 0.0)
            ResultatVLOD = mSphereOD;
        else if (QLocale().toDouble(mCylOD) != 0.0 && QLocale().toDouble(mSphereOD) == 0.0)
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
        if (QLocale().toDouble(mCylOG) != 0.0 && QLocale().toDouble(mSphereOG) != 0.0)
            ResultatVLOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + ")";
        else if (QLocale().toDouble(mCylOG) == 0.0 && QLocale().toDouble(mSphereOG) != 0.0)
            ResultatVLOG = mSphereOG;
        else if (QLocale().toDouble(mCylOG) != 0.0 && QLocale().toDouble(mSphereOG) == 0.0)
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
            if (ResultatOD != "Rien" && QLocale().toDouble(mAddOD) == 0.0  && ResultatOG == "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " " + tr("OD");

            if (Resultat == "" && ResultatOD != "Rien" && (QLocale().toDouble(mAddOD) == 0.0 && QLocale().toDouble(mAddOG) == 0.0) && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " "+ tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG") + "</td>";

            if (Resultat == "" && ResultatOD != "Rien" && (QLocale().toDouble(mAddOD) == 0.0 && QLocale().toDouble(mAddOG) > 0) && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP "+ tr("OG") + "</td>";

            if (Resultat == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0  && ResultatOG == "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD");

            if (Resultat == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0 && QLocale().toDouble(mAddOG) == 0.0 && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG") + "</td>";

            if (Resultat == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0 && QLocale().toDouble(mAddOG) > 0 && ResultatOG != "Rien")
                Resultat = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP " + tr("OG") + "</td>";

            if (Resultat == "" && (ResultatOD == "Rien" &&  QLocale().toDouble(mAddOG) == 0.0) && ResultatOG != "Rien")
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
        Resultat = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Resultat + "</td><td width=\"70\"><font color = \"red\"></font></td><td>" + m_userConnected->getLogin() + "</td></p>";
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
    //qDebug() << gMesureSerie;

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
    setDonneesFronto(gMesureSerie);
    NouvMesureFronto    = true;
    if (MesureFronto.isEmpty())
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (ThreadRefracteur!=Q_NULLPTR && !FicheRefractionOuverte())
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
    setTypeMesureRefraction(Fronto);
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
            mAddOD           = Utils::PrefixePlus(Mesure.mid(Mesure.indexOf("AR")+2,4));
        // OEIL GAUCHE ---------------------------------------------------------------------------
        mSphereOG            = mesureOG.mid(0,6);
        mCylOG               = mesureOG.mid(6,6);
        mAxeOG               = mesureOG.mid(12,3);
        if (Mesure.indexOf("AL")>0)
            mAddOG           = Utils::PrefixePlus(Mesure.mid(Mesure.indexOf("AL")+2,4));
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
    QString mSphereOD   = Utils::PrefixePlus(MesureFronto["SphereOD"].toString());
    QString mCylOD      = Utils::PrefixePlus(MesureFronto["CylOD"].toString());
    QString mAxeOD      = QString::number(MesureFronto["AxeOD"].toInt());
    QString mAddOD      = Utils::PrefixePlus(MesureFronto["AddOD"].toString());
    QString mSphereOG   = Utils::PrefixePlus(MesureFronto["SphereOG"].toString());
    QString mCylOG      = Utils::PrefixePlus(MesureFronto["CylOG"].toString());
    QString mAxeOG      = QString::number(MesureFronto["AxeOG"].toInt());
    QString mAddOG      = Utils::PrefixePlus(MesureFronto["AddOG"].toString());

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
    if (QLocale().toDouble(mCylOD) != 0.0 && QLocale().toDouble(mSphereOD) != 0.0)
        ResultatVLOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°" + ")";
    else if (QLocale().toDouble(mCylOD) == 0.0 && QLocale().toDouble(mSphereOD) != 0.0)
        ResultatVLOD = mSphereOD;
    else if (QLocale().toDouble(mCylOD) != 0.0 && QLocale().toDouble(mSphereOD) == 0.0)
        ResultatVLOD = mCylOD + tr(" à ") + mAxeOD + "°" ;
    else
        ResultatVLOD = tr("plan");

    if (QLocale().toDouble(mAddOD)>0)
        ResultatVPOD = mAddOD;
    ResultatOD = (QLocale().toDouble(mAddOD)>0 ? ResultatVLOD + " add." + ResultatVPOD + " VP" :ResultatVLOD);

    // détermination OG
    ResultatVPOG = "";
    ResultatVLOG = "";
    if (QLocale().toDouble(mCylOG) != 0.0 && QLocale().toDouble(mSphereOG) != 0.0)
        ResultatVLOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + "°" + ")";
    else if (QLocale().toDouble(mCylOG) == 0.0 && QLocale().toDouble(mSphereOG) != 0.0)
        ResultatVLOG = mSphereOG;
    else if (QLocale().toDouble(mCylOG) != 0.0 && QLocale().toDouble(mSphereOG) == 0.0)
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
    setDonneesAutoref(gMesureSerie);
    NouvMesureAutoref = true;
    if (MesureAutoref.isEmpty())
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (ThreadRefracteur!=Q_NULLPTR && !FicheRefractionOuverte())
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
                setTypeMesureRefraction(Kerato);
                emit NouvMesureRefraction();
            }
            if (gsettingsIni->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")
            {
                setHtmlTono();
                setTypeMesureRefraction(Tono);
                emit NouvMesureRefraction();

                setHtmlPachy();
                setTypeMesureRefraction(Pachy);
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
    setTypeMesureRefraction(Autoref);
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
    QString mSphereOD   = Utils::PrefixePlus(MesureAutoref["SphereOD"].toString());
    QString mCylOD      = Utils::PrefixePlus(MesureAutoref["CylOD"].toString());
    QString mAxeOD      = QString::number(MesureAutoref["AxeOD"].toInt());
    QString mSphereOG   = Utils::PrefixePlus(MesureAutoref["SphereOG"].toString());
    QString mCylOG      = Utils::PrefixePlus(MesureAutoref["CylOG"].toString());
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
        if (QLocale().toDouble(mCylOD) != 0.0 && QLocale().toDouble(mSphereOD) != 0.0)
            ResultatOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
        else if (QLocale().toDouble(mCylOD) == 0.0 && QLocale().toDouble(mSphereOD) != 0.0)
            ResultatOD = mSphereOD;
        else if (QLocale().toDouble(mCylOD) != 0.0 && QLocale().toDouble(mSphereOD) == 0.0)
            ResultatOD = mCylOD + tr(" à ") + mAxeOD + "°" ;
        else
            ResultatOD = tr("plan");
    }


    // détermination OG
    if (MesureAutoref["SphereOD"].toString()!="")
    {
        if (QLocale().toDouble(mCylOG) != 0.0 && QLocale().toDouble(mSphereOG) != 0.0)
            ResultatOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + "°)";
        else if (QLocale().toDouble(mCylOG) == 0.0 && QLocale().toDouble(mSphereOG) != 0.0)
            ResultatOG = mSphereOG;
        else if (QLocale().toDouble(mCylOG) != 0.0 && QLocale().toDouble(mSphereOG) == 0.0)
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
    QString mDioptrKOD  = Utils::PrefixePlus(MKer["DioptrKOD"].toString());
    QString mK1OG       = QLocale().toString(MKer["K1OG"].toDouble(),'f',2);
    QString mK2OG       = QLocale().toString(MKer["K2OG"].toDouble(),'f',2);
    QString mAxeKOG     = QString::number(MKer["AxeKOG"].toInt());
    QString mDioptrK1OG = QLocale().toString(MKer["DioptrK1OG"].toDouble(),'f',1);
    QString mDioptrK2OG = QLocale().toString(MKer["DioptrK2OG"].toDouble(),'f',1);
    QString mDioptrKOG  = Utils::PrefixePlus(MKer["DioptrKOG"].toString());
    QString kerato = "";

    if (QLocale().toDouble(mK1OD)>0)
    {
        if (QLocale().toDouble(mDioptrKOD)!=0.0)
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
        if (QLocale().toDouble(mDioptrKOG)!=0.0)
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
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TOG:") + "</b></font></td><td width=\"80\">" + TOGcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_userConnected->getLogin() + "</td></p>";
        else if (mTOG.toInt() == 0 && mTOD.toInt() > 0)
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TOD:") + "</b></font></td><td width=\"80\">" + TODcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_userConnected->getLogin() + "</td></p>";
        else if (mTOD.toInt() == mTOG.toInt())
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TODG:") + "</b></font></td><td width=\"80\">" + TODcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_userConnected->getLogin() + "</td></p>";
        else
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("TO:") + "</b></font></td><td width=\"80\">" + TODcolor + "/" + TOGcolor+ tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_userConnected->getLogin() + "</td></p>";

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
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("PachyOG:") + "</b></font></td><td width=\"80\">" + mPachyOG + "</td><td>" + m_userConnected->getLogin() + "</td></p>";
        else if (b == 0 && a > 0)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("PachyOG:") + "</b></font></td><td width=\"80\">" + mPachyOD + "</td><td>" + m_userConnected->getLogin() + "</td></p>";
        else if (a == b)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("PachyODG:") + "</b></font></td><td width=\"80\">" + mPachyOG + "</td><td>" + m_userConnected->getLogin() + "</td></p>";
        else
            Pachy= "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + CouleurTitres + "><b>" + tr("Pachy:") + "</b></font></td><td width=\"80\">" + mPachyOD + "/" + mPachyOG + "</td><td>" + m_userConnected->getLogin() + "</td></p>";

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

Procedures::TypeMesure Procedures::TypeMesureRefraction()
{
    return MesureRef;
}

void Procedures::setTypeMesureRefraction(TypeMesure mesure)
{
    MesureRef = mesure;
}

//---------------------------------------------------------------------------------
// Calcul de la formule de refraction
//---------------------------------------------------------------------------------
void Procedures::InsertRefraction(int idPatient, int idActe, TypeMesure Mesure)
{
    QString                 zQuelleMesure;
    QMap<QString,QVariant>  MapMesure;
    if (!MesureFronto.isEmpty() && Mesure == Fronto && NouvMesureFronto)
    {
        MapMesure = MesureFronto;
        bool a =
               (MapMesure["SphereOD"].toDouble()== 0.0
            &&  MapMesure["CylOD"].toDouble()   == 0.0
            &&  MapMesure["AddVPOD"].toDouble() == 0.0
            &&  MapMesure["SphereOG"].toDouble()== 0.0
            &&  MapMesure["CylOG"].toDouble()   == 0.0
            &&  MapMesure["AddVPOG"].toDouble() == 0.0
            );
        if (!a)
        {
            QString mSphereOD, mSphereOG;;
            QString mCylOD, mCylOG;
            QString mAxeOD, mAxeOG;
            QString mAddOD, mAddOG;
            mSphereOD       = Utils::PrefixePlus(MapMesure["SphereOD"].toString());
            mCylOD          = Utils::PrefixePlus(MapMesure["CylOD"].toString());
            mAxeOD          = QString::number(MapMesure["AxeOD"].toInt());
            mAddOD          = Utils::PrefixePlus(MapMesure["AddOD"].toString());
            mSphereOG       = Utils::PrefixePlus(MapMesure["SphereOG"].toString());
            mCylOG          = Utils::PrefixePlus(MapMesure["CylOG"].toString());
            mAxeOG          = QString::number(MapMesure["AxeOG"].toInt());
            mAddOG          = Utils::PrefixePlus(MapMesure["AddOG"].toString());
            zQuelleMesure = "P";
            QString requete = "delete from " NOM_TABLE_REFRACTION
                    " where idPat = " + QString::number(idPatient) +
                    " and idacte = " + QString::number(idActe) +
                    " and QuelleMesure = 'P'" +
                    " and FormuleOD = '" + Utils::CalculeFormule(MapMesure,"D") + "'" +
                    " and FormuleOG = '" + Utils::CalculeFormule(MapMesure,"G") + "'";
            db->StandardSQL(requete);

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
                    Utils::CalculeFormule(MapMesure,"D") + "'," +
                    QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    QString::number(QLocale().toDouble(mCylOG))     + "," +
                    mAxeOG     + "," +
                    (QLocale().toDouble(mAddOG)>0? QString::number(QLocale().toDouble(mAddOG)) : "null") + ",'" +
                    Utils::CalculeFormule(MapMesure,"G") + "')";

            db->StandardSQL (requete, tr("Erreur de création de données fronto dans ") + NOM_TABLE_REFRACTION);
        }
    }
    if (!MesureAutoref.isEmpty() && Mesure == Autoref && NouvMesureAutoref)
    {
        MapMesure = MesureAutoref;
        bool a =
               (MapMesure["SphereOD"].toDouble()== 0.0
            &&  MapMesure["CylOD"].toDouble()   == 0.0
            &&  MapMesure["SphereOG"].toDouble()== 0.0
            &&  MapMesure["CylOG"].toDouble()   == 0.0
            );
        if (!a)
        {
            QString mSphereOD, mSphereOG;;
            QString mCylOD, mCylOG;
            QString mAxeOD, mAxeOG;
            QString PD;
            mSphereOD       = Utils::PrefixePlus(MapMesure["SphereOD"].toString());
            mCylOD          = Utils::PrefixePlus(MapMesure["CylOD"].toString());
            mAxeOD          = QString::number(MapMesure["AxeOD"].toInt());
            mSphereOG       = Utils::PrefixePlus(MapMesure["SphereOG"].toString());
            mCylOG          = Utils::PrefixePlus(MapMesure["CylOG"].toString());
            mAxeOG          = QString::number(MapMesure["AxeOG"].toInt());
            PD              = MapMesure["PD"].toString();
            if (PD == "")
                PD = "null";
            zQuelleMesure = "A";
            QString requete = "delete from " NOM_TABLE_REFRACTION
                    " where idPat = " + QString::number(idPatient) +
                    " and idacte = " + QString::number(idActe) +
                    " and QuelleMesure = 'A'" ;
            db->StandardSQL(requete);

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
                    Utils::CalculeFormule(MapMesure,"D") + "'," +
                    QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    QString::number(QLocale().toDouble(mCylOG))     + "," +
                    mAxeOG     + ",'" +
                    Utils::CalculeFormule(MapMesure,"G") + "', " + PD + ")";

            db->StandardSQL (requete, tr("Erreur de création de données autoref dans ") + NOM_TABLE_REFRACTION);
            requete = "select idPat from " NOM_TABLE_DONNEES_OPHTA_PATIENTS " where idPat = " + QString::number(idPatient) + " and QuelleMesure = 'A'";
            QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
            if (!ok)
                return;
            if (patdata.size()==0)
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

                db->StandardSQL (requete, tr("Erreur de création de données autoref dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
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

                db->StandardSQL (requete, tr("Erreur de mise à jour de données autoref dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
        }
    }
    if (!MesureKerato.isEmpty() && Mesure == Kerato)
    {
        MapMesure = MesureKerato;
        bool a =
               (MapMesure["K1OD"].toDouble()== 0.0
            &&  MapMesure["K1OG"].toDouble()== 0.0
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
            QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
            if (!ok)
                return;
            if (patdata.size()==0)
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

                db->StandardSQL (requete, tr("Erreur de création de données kératométrie  dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
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

                db->StandardSQL (requete, tr("Erreur de modification de données de kératométrie dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
        }
    }
    if (!MesureRefracteurSubjectif.isEmpty() && Mesure == Subjectif)
    {
        MapMesure = MesureRefracteurSubjectif;
        bool a =
               (MapMesure["AVLOD"].toDouble() == 0.0
            &&  MapMesure["AVLOG"].toDouble() == 0.0
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
            mSphereOD       = Utils::PrefixePlus(MapMesure["SphereOD"].toString());
            mCylOD          = Utils::PrefixePlus(MapMesure["CylOD"].toString());
            mAxeOD          = QString::number(MapMesure["AxeOD"].toInt());
            mAddOD          = Utils::PrefixePlus(MapMesure["AddOD"].toString());
            mAVLOD          = QLocale().toString(MapMesure["AVLOD"].toDouble()*10) + "/10";
            mAVPOD          = MapMesure["AVPOD"].toString();
            mSphereOG       = Utils::PrefixePlus(MapMesure["SphereOG"].toString());
            mCylOG          = Utils::PrefixePlus(MapMesure["CylOG"].toString());
            mAxeOG          = QString::number(MapMesure["AxeOG"].toInt());
            mAddOG          = Utils::PrefixePlus(MapMesure["AddOG"].toString());
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
            db->StandardSQL(requete);
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
                    Utils::CalculeFormule(MapMesure,"D") + "','" +
                    mAVLOD + "','" +
                    mAVPOD + "'," +
                    QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    QString::number(QLocale().toDouble(mCylOG))     + "," +
                    mAxeOG     + "," +
                    (QLocale().toDouble(mAddOG)>0? QString::number(QLocale().toDouble(mAddOG)) : "null") + ",'" +
                    Utils::CalculeFormule(MapMesure,"G") + "','" +
                    mAVLOG + "','" +
                    mAVPOG + "'," +
                    PD + ")";

            db->StandardSQL(requete, tr("Erreur de création  de données de refraction dans ") + NOM_TABLE_REFRACTION);
            requete = "select idPat from " NOM_TABLE_DONNEES_OPHTA_PATIENTS " where idPat = " + QString::number(idPatient) + " and QuelleMesure = 'R'";
            QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
            if (!ok)
                return;
            if (patdata.size()==0)
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

                db->StandardSQL(requete, tr("Erreur création de données de refraction dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
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

                db->StandardSQL (requete, tr("Erreur de mise à jour de données de refraction dans ") + NOM_TABLE_DONNEES_OPHTA_PATIENTS);
            }
        }
    }
}
