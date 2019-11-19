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
    m_CPpardefaut    = "";
    m_Villepardefaut = "";
    db               = DataBase::I();

    m_nomFichierIni     = QDir::homePath() + FILE_INI;
    QFile FichierIni(m_nomFichierIni);
    m_applicationfont = QFont(POLICEPARDEFAUT);
    Utils::CalcFontSize(m_applicationfont);
    qApp->setFont(m_applicationfont);

    m_connexionbaseOK           = false;
    if (!FichierIni.exists())
    {
        bool a = false;
        while (!a)
        {
            QString msg =       tr("Le fichier d'initialisation de l'application est absent");
            QString msgInfo =   tr("Le fichier d'initialisation\n\"") + m_nomFichierIni + "\"\n" + tr("n'existe pas.\n"
                                "Ce fichier est indispensable au bon fonctionnement de l'application.\n\n"
                                "Cette absence est normale si vous démarrez l'application pour la première fois.\n"
                                "Si c'est le cas, choisissez l'option \"Premier démarrage de Rufus\"\n\n"
                                "Si le logiciel fonctionnait déjà sur ce poste et que le fichier a été effacé par erreur:\n"
                                "1. Si vous disposez d'une sauvegarde du fichier, choisissez \"Restaurer le fichier à partir d'une sauvegarde\"\n"
                                "2. Sinon cliquez sur \"Reconstruire le fichier d'initialisation\" et suivez les étapes de la reconstruction."
                                " Il vous faudra alors compléter de nouveau"
                                " les renseignements concernant les appareils connectés au réseau ou à ce poste d'examen après"
                                " le démarrage complet du logiciel (Menu Edition/Paramètres).\n");
            m_connexionbaseOK = a;
            a = VerifIni(msg, msgInfo, true, true, true, false, true, false);
        }
    }
    m_settings    = new QSettings(m_nomFichierIni, QSettings::IniFormat);

    bool k                          = (m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Active").toString() == "YES"
                                       && (m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Port").toInt() == 3306
                                       || m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Port").toInt() == 3307)
                                       )
                                    || (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Active").toString() == "YES"
                                       && m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Serveur").toString() != ""
                                       && (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Port").toInt() == 3306
                                       || m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Port").toInt() == 3307)
                                       )
                                    || (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Active").toString() == "YES"
                                       && m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Serveur").toString() != ""
                                       && (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Port").toInt() == 3306
                                       || m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Port").toInt() == 3307)
                                       );
   if (!k)
    {
        while (!k)
        {
            QString msg =       tr("Le fichier d'initialisation de l'application est corrompu\n");
            QString msgInfo =   tr("Le fichier d'initialisation\n\"") + m_nomFichierIni + "\"\n" +
                                tr("ne contient pas de renseignement valide\n"
                                "permettant la connexion à la base de données.\n\n"
                                "Ce fichier est indispensable au bon fonctionnement de l'application.\n\n"
                                "1. Si vous disposez d'une sauvegarde du fichier, choisissez \"Restaurer le fichier à partir d'une sauvegarde\"\n"
                                "2. Sinon cliquez sur \"Reconstruire le fichier d'initialisation\" et suivez les étapes de la reconstruction."
                                " Il vous faudra alors compléter de nouveau"
                                " les renseignements concernant les appareils connectés au réseau ou à ce poste d'examen après"
                                " le démarrage complet du logiciel (Menu Edition/Paramètres).\n");
            m_connexionbaseOK = k;
            k = VerifIni(msg, msgInfo, false, true, true, false, false, false);
        }
    }
   else if (!FicheChoixConnexion())
   {
       m_initok = false;
       return;
   }

   m_nomImprimante  = "";

   Ouverture_Ports_Series();
   m_typemesureRefraction               = None;
   m_dlgrefractionouverte    = false;
   m_initok                  = true;
   int margemm         = TailleTopMarge(); // exprimé en mm
   p_printer             = new QPrinter(QPrinter::HighResolution);
   p_printer             ->setFullPage(true);
   m_rect                = p_printer->paperRect();
   m_rect.adjust(Utils::mmToInches(margemm) * p_printer->logicalDpiX(),
                 Utils::mmToInches(margemm) * p_printer->logicalDpiY(),
                -Utils::mmToInches(margemm) * p_printer->logicalDpiX(),
                -Utils::mmToInches(margemm) * p_printer->logicalDpiY());
   connect (this, &Procedures::backupDossiers, this, &Procedures::BackupDossiers);
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
    Dlg_ChxDate = new dlg_choixdate(parent);
    Dlg_ChxDate ->setWindowTitle(tr("Choisir une période"));
    Dlg_ChxDate ->exec();
    QMap<QString, QDate> DateMap = Dlg_ChxDate->map();
    delete Dlg_ChxDate;
    return DateMap;
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- renvoie la valeur du dossier documents pour le type d'appareil concerné -----------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::pathDossierDocuments(QString Appareil, Utils::ModeAcces mode)
{
    QString cle = Utils::getBaseFromMode( mode ) + "/DossiersDocuments/" + Appareil;
    QString dossier = m_settings->value(cle).toString();
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

//!> supprime les fichiers de logs antérieurs à J - anciennete jours
void Procedures::EpureLogs(int anciennete)
{
    QDir dirlogs = QDir(QDir::homePath() + DIR_RUFUS DIR_LOGS);
    QStringList listfiles = dirlogs.entryList();
    for (int i=0; i<listfiles.size(); ++i)
    {
        QFile file(listfiles.at(i));
        QDate datefile = QDate::fromString(file.fileName().left(10), "yyyy-MM-dd");
        if (datefile < QDate::currentDate().addDays(-anciennete))
            QFile::remove(QDir::homePath() + DIR_RUFUS DIR_LOGS + "/" + file.fileName());
    }
}

// ----------------------------------------------------------------------------------
// Modidife la taille de la police utilisée pour les widget d'une liste
// ----------------------------------------------------------------------------------
void Procedures::ModifTailleFont(QWidget *widg, int siz, QFont font)
{
    font.setPointSize(font.pointSize() + siz);
    widg->setFont(font);
    for (int i=0; i<widg->findChildren<QWidget*>().size(); i++)
    {
        //qDebug() << obj->findChildren<QWidget*>().at(i)->objectName();
        widg->findChildren<QWidget*>().at(i)->setFont(font);
    }
}

bool Procedures::AutresPostesConnectes(bool msg)
{
    int id = 0;
    if (Datas::I()->users->userconnected() != Q_NULLPTR)
        id = Datas::I()->users->userconnected()->id();
    PosteConnecte *m_currentposteconnecte = Datas::I()->postesconnectes->getByStringId(Utils::getMACAdress() + " - " + QString::number(id));
    if (m_currentposteconnecte == Q_NULLPTR)
    {
        UpMessageBox::Information(Q_NULLPTR, tr("Problème avec ce poste!"),
                                  tr("Le poste n'est pas connecté"));
        return true;
    }
    foreach (PosteConnecte *post, Datas::I()->postesconnectes->postesconnectes()->values())
    {
        if (post->stringid() != m_currentposteconnecte->stringid())
        {
            if (msg)
                UpMessageBox::Information(Q_NULLPTR, tr("Autres postes connectés!"),
                                         tr("Vous ne pouvez pas effectuer d'opération de sauvegarde/restauration sur la base de données"
                                         " si vous n'êtes pas le seul poste connecté.\n"
                                         "Le poste ") + post->nomposte() + tr(" est aussi connecté"));
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------------
// les sauvegardes
//--------------------------------------------------------------------------------------------------------
/*!
 *  \brief AskBupRestore
 *  la fiche qui permet de paramètrer une opération de sauvegarde ou de restauration
 *  \param restore :            true = restauration - false = backup
 *  \param pathorigin :         le dossier de stockage de l'imagerie sur le serveur
 *  \param pathdestination :    le dossier où se trouve le backup
 *  \param OKini :              le rufus.ini est sauvegardé
 *  \param OKRssces :           les fichiers ressources sont sauvegardés
 *  \param OKimages :           les fichiers images sont sauvegardés
 *  \param OKvideos :           les fichiers videos sont sauvegardés
 *  \param OKfactures :         les factures sont sauvegardées
 *
 */
void Procedures::AskBupRestore(BkupRestore op, QString pathorigin, QString pathdestination, bool OKini, bool OKRssces, bool OKimages, bool OKvideos, bool OKfactures)
{
    if (!QDir(pathdestination).exists())
        Utils::mkpath(pathdestination);
    QMap<QString, qint64>      DataDir;
    // taille de la base de données ----------------------------------------------------------------------------------------------------------------------------------------------
    m_basesize = 0;
    if (op == RestoreOp)
    {
        QStringList filters, listnomsfilestorestore;
        filters << "*.sql";
        for (int j=0; j<QDir(pathorigin).entryList(filters).size(); j++)
            listnomsfilestorestore << pathorigin + "/" + QDir(pathorigin).entryList(filters).at(j);
        for (int i=0; i<listnomsfilestorestore.size(); i++)
            m_basesize += QFile(listnomsfilestorestore.at(i)).size()/1024/1024;
    }
    else
        m_basesize = CalcBaseSize();
    m_imagessize = 0;
    m_videossize = 0;
    // espace libre sur le disque ------------------------------------------------------------------------------------------------------------------------------------------------

    m_freespace = QStorageInfo(pathdestination).bytesAvailable();
    m_freespace = m_freespace/1024/1024;
    //qDebug() << QStorageInfo(dirbkup).bytesAvailable();
    //qDebug() << QString::number(FreeSpace,'f',0);

    dlg_buprestore = new UpDialog();
    dlg_buprestore->setModal(true);
    dlg_buprestore->setWindowTitle(op == RestoreOp? tr("Dossiers à restaurer") : tr("Dossiers à sauvegarder"));
    int labelsize = 15;

    if (op == RestoreOp)
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
        dlg_buprestore->dlglayout()->insertLayout(0, layini);

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
        dlg_buprestore->dlglayout()->insertLayout(0, layRssces);
        QDir rootimgvid = QDir(pathorigin);
        if (rootimgvid.cdUp())
            pathorigin = rootimgvid.absolutePath();
    }
    if (OKvideos)
    {
        // taille du dossier video ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + DIR_VIDEOS);
        m_videossize = DataDir["Size"]/1024/1024;
        if (m_videossize> 0)
        {
            QHBoxLayout *layVideos = new QHBoxLayout;
            UpLabel *labeVideos = new UpLabel();
            labeVideos->setVisible(false);
            labeVideos->setFixedSize(labelsize, labelsize);
            layVideos->addWidget(labeVideos);
            UpCheckBox *Videoschk  = new UpCheckBox();
            Videoschk->setText("Videos");
            Videoschk->setEnabled(OKvideos || op == BackupOp);
            Videoschk->setChecked(OKvideos || op == BackupOp);
            Videoschk->setAccessibleDescription("videos");
            layVideos->addWidget(Videoschk);
            layVideos->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            UpLabel *lblvolvid = new UpLabel();
            lblvolvid->setText(Utils::getExpressionSize(m_videossize));
            layVideos->addWidget(lblvolvid);
            dlg_buprestore->dlglayout()->insertLayout(0, layVideos);
            connect(Videoschk, &UpCheckBox::clicked, this, [=] {CalcTimeBupRestore();});
        }
    }
    if (OKimages)
    {
        // taille du dossier Images ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + DIR_IMAGES);
        m_imagessize = DataDir["Size"]/1024/1024;
        if (m_imagessize > 0)
        {
            QHBoxLayout *layImges = new QHBoxLayout;
            UpLabel *labelmges = new UpLabel();
            labelmges->setVisible(false);
            labelmges->setFixedSize(labelsize, labelsize);
            layImges->addWidget(labelmges);
            UpCheckBox *Imgeschk  = new UpCheckBox();
            Imgeschk->setText("Images");
            Imgeschk->setEnabled(OKimages || op == BackupOp);
            Imgeschk->setChecked(OKimages || op == BackupOp);
            Imgeschk->setAccessibleDescription("images");
            layImges->addWidget(Imgeschk);
            layImges->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            UpLabel *lblvolimg = new UpLabel();
            lblvolimg->setText(Utils::getExpressionSize(m_imagessize));
            layImges->addWidget(lblvolimg);
            dlg_buprestore->dlglayout()->insertLayout(0, layImges);
            connect(Imgeschk, &UpCheckBox::clicked, this, [=] {CalcTimeBupRestore();});
        }
    }
    if (OKfactures)
    {
        // taille du dossier Factures ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + DIR_FACTURES);
        m_facturessize = DataDir["Size"]/1024/1024;
        if (m_facturessize > 0)
        {
            QHBoxLayout *layFctures = new QHBoxLayout;
            UpLabel *labelmges = new UpLabel();
            labelmges->setVisible(false);
            labelmges->setFixedSize(labelsize, labelsize);
            layFctures->addWidget(labelmges);
            UpCheckBox *Fctureschk  = new UpCheckBox();
            Fctureschk->setText("Factures");
            Fctureschk->setEnabled(OKimages || op == BackupOp);
            Fctureschk->setChecked(OKimages || op == BackupOp);
            Fctureschk->setAccessibleDescription("factures");
            layFctures->addWidget(Fctureschk);
            layFctures->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            UpLabel *lblvolfct = new UpLabel();
            lblvolfct->setText(Utils::getExpressionSize(m_facturessize));
            layFctures->addWidget(lblvolfct);
            dlg_buprestore->dlglayout()->insertLayout(0, layFctures);
            connect(Fctureschk, &UpCheckBox::clicked, this, [=] {CalcTimeBupRestore();});
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
    lblvolbase->setText(Utils::getExpressionSize(m_basesize));
    layBDD->addWidget(lblvolbase);
    dlg_buprestore->dlglayout()->insertLayout(0, layBDD);


    QHBoxLayout *layResume = new QHBoxLayout;
    wdg_resumelbl = new UpLabel();
    layResume->addWidget(wdg_resumelbl);
    dlg_buprestore->dlglayout()->insertLayout(dlg_buprestore->dlglayout()->count()-1, layResume);

    QHBoxLayout *layVolumeLibre = new QHBoxLayout;
    wdg_volumelibrelbl = new UpLabel();
    layVolumeLibre->addWidget(wdg_volumelibrelbl);
    dlg_buprestore->dlglayout()->insertLayout(dlg_buprestore->dlglayout()->count()-1, layVolumeLibre);

    connect(BDDchk, &UpCheckBox::clicked, this, [=] {CalcTimeBupRestore();});

    dlg_buprestore->setFixedWidth(400);
    dlg_buprestore->AjouteLayButtons(UpDialog::ButtonOK);
    connect(dlg_buprestore->OKButton,  &UpCheckBox::clicked, dlg_buprestore, &UpDialog::accept);
    CalcTimeBupRestore();
}

bool Procedures::Backup(QString pathdirdestination, bool OKBase, bool OKImages, bool OKVideos, bool OKFactures)
{
    auto result = [] (qintptr handle, Procedures *proc)
    {
        Message::I()->ClosePriorityMessage(handle);
        proc->emit ConnectTimers(true);
    };
    if (QDir(m_parametres->dirimagerie()).exists())
    {
        Utils::cleanfolder(m_parametres->dirimagerie() + DIR_IMAGES);
        Utils::cleanfolder(m_parametres->dirimagerie() + DIR_FACTURES);
        Utils::cleanfolder(m_parametres->dirimagerie() + DIR_VIDEOS);
    }
    else
    {
        OKImages = false;
        OKVideos = false;
        OKFactures = false;
    }

    QString msgEchec = tr("Incident pendant la sauvegarde");
    qintptr handledlg = 0;
    Message::I()->PriorityMessage(tr("Sauvegarde en cours"),handledlg);
    emit ConnectTimers(false);

    if (OKBase)
    {
        QFile::remove(QDir::homePath() + SCRIPTBACKUPFILE);
        DefinitScriptBackup(pathdirdestination, OKImages, OKVideos, OKFactures);
        QString Msg = (tr("Sauvegarde de la base de données\n")
                       + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
        const QString task = "sh " + QDir::homePath() + SCRIPTBACKUPFILE;
        const QString msgOK = tr("Base de données sauvegardée!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller, &Controller::result, this, [=](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            if (OKImages)
                Utils::cleanfolder(pathdirdestination + DIR_IMAGES);
            if (OKFactures)
                Utils::cleanfolder(pathdirdestination + DIR_FACTURES);
            if (OKVideos)
                Utils::cleanfolder(pathdirdestination + DIR_VIDEOS);
            result(handledlg, this);
            return true;
        });
        m_controller.execute(task);
    }
    else if (OKImages || OKVideos || OKFactures)
    {
        QDir dirdest;
        dirdest.mkdir(pathdirdestination);
        pathdirdestination += "/" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmm");
        emit backupDossiers(pathdirdestination, handledlg, OKFactures, OKImages, OKVideos);
    }
    else
    {
        result(handledlg, this);
        return false;
    }
    return true;
}

void Procedures::BackupDossiers(QString dirdestination, qintptr handledlg, bool factures, bool images, bool videos)
{
    auto result = [] (qintptr handle, Procedures *proc)
    {
        Message::I()->ClosePriorityMessage(handle);
        proc->emit ConnectTimers(true);
    };
    QString msgEchec = tr("Incident pendant la sauvegarde");
    if (factures) {
        QString Msg = (tr("Sauvegarde des factures\n")
                       + tr("Ce processus peut durer plusieurs minutes en fonction de la taille des fichiers"));
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
        const QString task = "cp -R " + m_parametres->dirimagerie() + DIR_FACTURES + " " + dirdestination;
        const QString msgOK = tr("Fichiers factures sauvegardés!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller,
                &Controller::result,
                this,
                [=, &factures](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            Utils::cleanfolder(dirdestination + DIR_FACTURES);
            factures = false;
            //qDebug() << "factures" << factures << images << videos;
            if (!images && !videos)
            {
                result(handledlg, this);
                return;
            }
            else
                emit backupDossiers(dirdestination, handledlg, false, images, videos);
        });
        m_controller.execute(task);
        return;
    }
    else if (images) {
        QString Msg = (tr("Sauvegarde des fichiers d'imagerie\n")
                       + tr("Ce processus peut durer plusieurs minutes en fonction de la taille des fichiers"));
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
        const QString task = "cp -R " + m_parametres->dirimagerie() + DIR_IMAGES + " " + dirdestination;
        const QString msgOK = tr("Fichiers d'imagerie sauvegardés!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller,
                &Controller::result,
                this,
                [=, &images](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            Utils::cleanfolder(dirdestination + DIR_IMAGES);
            images = false;
            //qDebug() << "images" << factures << images << videos;
            if (!factures && !videos)
            {
                result(handledlg, this);
                return;
            }
            else
                emit backupDossiers(dirdestination, handledlg, factures, false, videos);
        });
        m_controller.execute(task);
        return;
    }
    else if (videos) {
        QString Msg = (tr("Sauvegarde des videos\n")
                       + tr("Ce processus peut durer plusieurs minutes en fonction de la taille des fichiers"));
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
        const QString task = "cp -R " + m_parametres->dirimagerie() + DIR_VIDEOS + " " + dirdestination;
        const QString msgOK = tr("Fichiers videos sauvegardés!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller,
                &Controller::result,
                this,
                [=, &videos](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            Utils::cleanfolder(dirdestination + DIR_VIDEOS);
            //qDebug() << "videos" << factures << images << videos;
            videos = false;
            if (!images && !factures)
            {
                result(handledlg, this);
                return;
            }
            else
                emit backupDossiers(dirdestination, handledlg, factures, images, false);
        });
        m_controller.execute(task);
       return;
    }
    result(handledlg, this);
}


void Procedures::BackupWakeUp()
{
    //Logs::trace("BKUP", "currentTime() = " + QTime::currentTime().toString("HH:mm:ss") + " - m_parametres->heurebkup() = " + m_parametres->heurebkup().toString("HH:mm:ss"));
    if (QTime::currentTime().toString("HH:mm") == m_parametres->heurebkup().toString("HH:mm") && QDate::currentDate() > m_lastbackupdate)
    {
        Utils::Day daybkup = Utils::Lundi;
        switch (QDate::currentDate().dayOfWeek()) {
        case 1: daybkup = Utils::Lundi; break;
        case 2: daybkup = Utils::Mardi; break;
        case 3: daybkup = Utils::Mercredi; break;
        case 4: daybkup = Utils::Jeudi; break;
        case 5: daybkup = Utils::Vendredi; break;
        case 6: daybkup = Utils::Samedi; break;
        case 7: daybkup = Utils::Dimanche;
        }
        if (m_parametres->daysbkup().testFlag(daybkup))
            if (!AutresPostesConnectes())
            {
                Logs::trace("LANCEMENT DU BKUP", "currentTime() = " + QTime::currentTime().toString("HH:mm:ss") + " - m_parametres->heurebkup() = " + m_parametres->heurebkup().toString("HH:mm:ss"));
                m_lastbackupdate = QDate::currentDate();
                Backup(m_parametres->dirbkup());
            }
    }
}

void Procedures::DefinitScriptBackup(QString pathdirdestination, bool AvecImages, bool AvecVideos, bool AvecFactures)
{
    if (!Utils::mkpath(pathdirdestination))
        return;
    if (!QDir(pathdirdestination).exists())
        return;
    // élaboration du script de backup
    QString scriptbackup = "#!/bin/bash";
    //# Configuration de base: datestamp e.g. YYYYMMDD
    scriptbackup += "\n";
    scriptbackup += "DATE=$(date +\"%Y%m%d-%H%M\")";
    //# Dossier où sauvegarder les backups (créez le d'abord!)
    scriptbackup += "\n";
    scriptbackup += "BACKUP_DIR=\"" + pathdirdestination + "\"";
    //# Dossier de  ressources
    scriptbackup += "\n";
    scriptbackup += "DIR_RESSOURCES=\"" + QDir::homePath() + DIR_RUFUS DIR_RESSOURCES + "\"";
    scriptbackup += "\n";
    if (QDir(m_parametres->dirimagerie()).exists())
    {
        if (AvecImages)
        {
            scriptbackup += "DIR_IMAGES=\"" + m_parametres->dirimagerie() + DIR_IMAGES + "\"";
            scriptbackup += "\n";
        }
        if (AvecFactures)
        {
            scriptbackup += "DIR_FACTURES=\"" + m_parametres->dirimagerie() + DIR_FACTURES + "\"";
            scriptbackup += "\n";
        }
        if (AvecVideos)
        {
            scriptbackup += "DIR_VIDEOS=\"" + m_parametres->dirimagerie() + DIR_VIDEOS + "\"";
            scriptbackup += "\n";
        }
    }
    //# Rufus.ini
    scriptbackup += "RUFUSINI=\"" + QDir::homePath() + FILE_INI + "\"";
    //# Identifiants MySQL
    scriptbackup += "\n";
    scriptbackup += "MYSQL_USER=\"dumprufus\"";
    scriptbackup += "\n";
    scriptbackup += "MYSQL_PASSWORD=\"" + MDPAdmin() + "\"";
    //# Commandes MySQL
    QDir Dir(QCoreApplication::applicationDirPath());
    Dir.cdUp();
    scriptbackup += "\n";
    QString cheminmysql;
#ifdef Q_OS_MACX
    cheminmysql = "/usr/local/mysql/bin";           // Depuis HighSierra on ne peut plus utiliser + Dir.absolutePath() + DIR_LIBS2 - le script ne veut pas utiliser le client mysql du package (???)
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
    if (QDir(m_parametres->dirimagerie()).exists())
    {
        //! copie les fichiers image
        if (AvecImages)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR" DIR_IMAGES;
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_IMAGES $BACKUP_DIR";
            scriptbackup += "\n";
        }
        if (AvecFactures)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR" DIR_FACTURES;
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_FACTURES $BACKUP_DIR";
            scriptbackup += "\n";
        }
        //! copie les fichiers video
        if (AvecVideos)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR" DIR_VIDEOS;
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
    cheminmysql = "/usr/local/mysql/bin";           // Depuis HighSierra on ne peut plus utiliser + Dir.absolutePath() + DIR_LIBS2 - le script ne veut pas utiliser le client mysql du package (???)
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
            if (m_currentuser == Q_NULLPTR)
                scriptrestore += "$MYSQL -u " + m_login +  " -p" +  m_password + " -h localhost -P " + QString::number(db->getDataBase().port()) + " < " + ListNomFiles.at(i);
            else
                scriptrestore += "$MYSQL -u " + m_currentuser->login() +  " -p" +  m_currentuser->password() + " -h localhost -P " + QString::number(db->getDataBase().port()) + " < " + ListNomFiles.at(i);
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
bool Procedures::ImmediateBackup(QString dirdestination, bool verifposteconnecte, bool full)
{
    if (!verifposteconnecte)
        if (AutresPostesConnectes())
            return false;

    if (dirdestination == "")
    {
        QString dirSauv = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                            tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base") + "\n" + tr("Le nom de dossier ne doit pas contenir d'espace"),
                                                            (QDir(m_parametres->dirbkup()).exists()? m_parametres->dirbkup() : QDir::homePath()));
        if (dirSauv.contains(" "))
            UpMessageBox::Watch(Q_NULLPTR, tr("Nom de dossier non conforme"),tr("Vous ne pouvez pas choisir un dossier dont le nom contient des espaces"));
        if (dirSauv == "" || dirSauv.contains(" "))
            return false;
        dirdestination = dirSauv;
    }

    if (!QDir(dirdestination).exists())
        return false;

    bool OKbase     = false;
    bool OKImages   = false;
    bool OKVideos   = false;
    bool OKFactures = false;
    if (full)
    {
        OKbase = true;
        OKImages = QDir(m_parametres->dirimagerie()).exists();
        OKVideos = QDir(m_parametres->dirimagerie()).exists();
        OKFactures = QDir(m_parametres->dirimagerie()).exists();
    }
    else
    {
        AskBupRestore(BackupOp, m_parametres->dirimagerie(), dirdestination );
        if (dlg_buprestore->exec()==0)
            return false;
        QList<UpCheckBox*> listchk = dlg_buprestore->findChildren<UpCheckBox*>();
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
    return Backup(dirdestination, OKbase, OKImages, OKVideos, OKFactures);
}

void Procedures::EffaceBDDDataBackup()
{
    db->setdaysbkup(nullptr);
    db->setheurebkup();
    db->setdirbkup();
    EffaceProgrammationBackup();
}

void Procedures::EffaceProgrammationBackup()
{
    if (QFile::exists(QDir::homePath() + SCRIPTBACKUPFILE))
        QFile::remove(QDir::homePath() + SCRIPTBACKUPFILE);
    t_timerbackup.disconnect(SIGNAL(timeout()));
    t_timerbackup.stop();
    /*! la suite n'est plus utilisée depuis OsX Catalina parce que OsX Catalina n'accepte plus les launchagents
#ifdef Q_OS_MACX
    QString file = QDir::homePath() + SCRIPT_MACOS_PLIST_FILE;                          //! file = "/Users/xxxx/Library/LaunchAgents/rufus.bup.plist"
    if (!QFile::exists(file))
        return;
    QString unload  = "bash -c \"/bin/launchctl unload \"" + file + "\"\"";             //! unload = "bash -c "/bin/launchctl unload "/Users/xxxx/Library/LaunchAgents/rufus.bup.plist""
    QProcess dumpProcess(parent());
    dumpProcess.start(unload);
    dumpProcess.waitForFinished();
    QFile::remove(file);
#endif
    */
}

void Procedures::ParamAutoBackup()
{
    if (m_parametres->dirbkup() == "" || !QDir(m_parametres->dirbkup()).exists() || !m_parametres->heurebkup().isValid() || !m_parametres->daysbkup())
    {
        EffaceProgrammationBackup();
        return;
    }
    t_timerbackup.disconnect(SIGNAL(timeout()));
    t_timerbackup.stop();
    t_timerbackup.start(30000); /*! le timer de déclenchement de la sauvegrade est lancé plus d'une fois par mintue à cause de la grande imprécision des QTimer
                                  * si on le lance toutes les 60", il est possible que le timer ne soit pas lancé dans la minute définie pour la sauvegarde.
                                  * En le lançant toutes les 30", ça marche.
                                  * C'est de la bidouille, je sais */
    connect(&t_timerbackup, &TimerController::timeout, this, [=] {BackupWakeUp();});

    /*! la suite n'est plus utilisée depuis OsX Catalina parce que OsX Catalina n'accepte plus les launchagents
#ifdef Q_OS_MACX
    DefinitScriptBackup(m_parametres->dirbkup());
    // elaboration de rufus.bup.plist
    QString heure   = m_parametres->heurebkup().toString("H");
    QString minute  = m_parametres->heurebkup().toString("m");
    QString jourprg = "\t\t<array>\n";
    QString debutjour =
        "\t\t\t<dict>\n"
        "\t\t\t\t<key>Weekday</key>\n"
        "\t\t\t\t<integer>";
    QString finjour =
        "</integer>\n"
        "\t\t\t\t<key>Hour</key>\n"
        "\t\t\t\t<integer>"+ heure + "</integer>\n"
        "\t\t\t\t<key>Minute</key>\n"
        "\t\t\t\t<integer>" + minute + "</integer>\n"
        "\t\t\t</dict>\n";
    if (m_parametres->daysbkup().testFlag(Utils::Lundi))
        jourprg += debutjour + "1" + finjour;
    if (m_parametres->daysbkup().testFlag(Utils::Mardi))
        jourprg += debutjour + "2" + finjour;
    if (m_parametres->daysbkup().testFlag(Utils::Mercredi))
        jourprg += debutjour + "3" + finjour;
    if (m_parametres->daysbkup().testFlag(Utils::Jeudi))
        jourprg += debutjour + "4" + finjour;
    if (m_parametres->daysbkup().testFlag(Utils::Vendredi))
        jourprg += debutjour + "5" + finjour;
    if (m_parametres->daysbkup().testFlag(Utils::Samedi))
        jourprg += debutjour + "6" + finjour;
    if (m_parametres->daysbkup().testFlag(Utils::Dimanche))
        jourprg += debutjour + "7" + finjour;
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
    if (QFile::exists(QDir::homePath() + SCRIPT_MACOS_PLIST_FILE))
        QFile::remove(QDir::homePath() + SCRIPT_MACOS_PLIST_FILE);
    QFile fplist(QDir::homePath() + SCRIPT_MACOS_PLIST_FILE);
    if (fplist.open(QIODevice::ReadWrite))
    {
        QTextStream out(&fplist);
        out << plist;
        fplist.close();
    }

    // relance du launchd
    QString unload  = "bash -c \"/bin/launchctl unload \"" + QDir::homePath();
    unload += SCRIPT_MACOS_PLIST_FILE "\"\"";
    QString load    = "bash -c \"/bin/launchctl load \""   + QDir::homePath();
    load += SCRIPT_MACOS_PLIST_FILE "\"\"";
    QProcess dumpProcess(parent());
    dumpProcess.start(unload);
    dumpProcess.waitForFinished();
    dumpProcess.start(load);
    dumpProcess.waitForFinished();
#endif
*/
}

void Procedures::ProgrammeSQLVideImagesTemp(QTime timebackup)
{
    //programmation de l'effacement du contenu de la table ImagesEchange
    db->StandardSQL("Use " DB_IMAGES);
    db->StandardSQL("DROP EVENT IF EXISTS VideImagesEchange");
    QString req =   "CREATE EVENT VideImagesEchange "
            "ON SCHEDULE EVERY 1 DAY STARTS '2018-03-23 " + timebackup.addSecs(-60).toString("HH:mm:ss") + "' "
            "DO DELETE FROM " TBL_ECHANGEIMAGES;
    db->StandardSQL(req);
    //programmation de l'effacement des pdf et jpg contenus dans Factures
    db->StandardSQL("Use " DB_COMPTA);
    db->StandardSQL("DROP EVENT IF EXISTS VideFactures");
    req =   "CREATE EVENT VideFactures "
            "ON SCHEDULE EVERY 1 DAY STARTS '2018-03-23 " + timebackup.addSecs(-60).toString("HH:mm:ss") + "' "
            "DO UPDATE " TBL_FACTURES " SET " CP_JPG_FACTURES " = null, " CP_PDF_FACTURES " = null";
    db->StandardSQL(req);
}

//--------------------------------------------------------------------------------------------------------
// fin sauvegardes
//--------------------------------------------------------------------------------------------------------

//TODO : à déplacer
/*---------------------------------------------------------------------------------
    Retourne le corps du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::CalcCorpsImpression(QString text, bool ALD)
{
    QString Corps;
    QString nomModeleCorpsImpression;
    Utils::convertHTML(text);
    if (ALD)
        nomModeleCorpsImpression = QDir::homePath() + FILE_CORPSORDOALD;
    else
        nomModeleCorpsImpression = QDir::homePath() + FILE_CORPSORDO;

    QFile qFile(nomModeleCorpsImpression);
    while (!qFile.open( QIODevice::ReadOnly ))
        if (!VerifRessources(nomModeleCorpsImpression))
            return QString();

    qint64 file_len = qFile.size();
    QByteArray ba = qFile.readAll();
    ba.resize(file_len+1);
    ba.data()[file_len]=0;
    qFile.close ();
    Corps = ba;
    text.replace(QRegExp("font-size( *: *[\\d]{1,2} *)pt"),"font-size:9pt");
    Corps.replace("{{TEXTE ORDO}}",text);
    Corps.replace("{{TEXTE ORDO HORS ALD}}", "");
    return Corps;
}

/*---------------------------------------------------------------------------------
    Retourne l'entête du document à imprimer
-----------------------------------------------------------------------------------*/
QMap<QString, QString> Procedures::CalcEnteteImpression(QDate date, User *user)
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
    if (user && user->modeenregistrementhonoraires() == User::Retrocession)
    {
        rplct = true;
        if (user->id() == m_currentuser->idsuperviseur())
        {
            // si le user rplct à imprimer est le superviseur du user courant, on récupère le parent du user courant
            idparent = m_currentuser->idparent();
        }
        else
        {
            // si le user rplct à imprimer n'est pas le superviseur du user courant, on cherche son parent
            QString reqrp = "select userparent "
                            "from " TBL_USERSCONNECTES
                            " where usersuperviseur = " + QString::number(user->id());
            QVariantList userdata = db->getFirstRecordFromStandardSelectSQL(reqrp, m_ok);
            if (userdata.size()>0)                // le user est connecté, on cherche qui il remplace - son parent
                idparent = userdata.at(0).toInt();
            else                                // le user n'est pas connecté on demande quel est son parent
            {
                QVariantList soigndata = db->getFirstRecordFromStandardSelectSQL("select soignant from " TBL_UTILISATEURS " where iduser = " + QString::number(user->id()), m_ok);
                QString req   = "select iduser, userlogin from " TBL_UTILISATEURS
                        " where (userenreghonoraires = 1 or userenreghonoraires = 2)"
                        " and iduser <> " + QString::number(user->id()) +
                        " and soignant = " + soigndata.at(0).toString() +
                        " and userdesactive is null";
                //qDebug() << req;
                QList<QVariantList> soignlist = db->StandardSelectSQL(req,m_ok);
                if (soignlist.size() == 1)               // une seule réponse, on la récupère
                    idparent   = soignlist.at(0).at(0).toInt();
                else                                // plusieurs réponses possibles, on va demander qui est le parent de ce remplaçant....
                {
                    dlg_askUser                = new UpDialog();
                    dlg_askUser                ->AjouteLayButtons();
                    QGroupBox*boxparent     = new QGroupBox();
                    dlg_askUser->dlglayout()   ->insertWidget(0,boxparent);
                    boxparent               ->setAccessibleName("Parent");
                    QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + user->login() + "?";
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
                    dlg_askUser ->setModal(true);
                    dlg_askUser->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

                    connect(dlg_askUser->OKButton, &QPushButton::clicked, dlg_askUser, &UpDialog::accept);

                    dlg_askUser->exec();

                    QList<QRadioButton*> listbutt = boxparent->findChildren<QRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                            idparent = listbutt.at(j)->accessibleName().toInt();
                    delete dlg_askUser;
                }
            }
        }
    }
    for (int i = 1; i<3; i++)//TODO : ??? pourquoi 3 - reponse: comme ça, pour pas mettre i==2....
    {
        if (i==1)
            nomModeleEntete = QDir::homePath() + FILE_ENTETEORDO;
        else
            nomModeleEntete = QDir::homePath() + FILE_ENTETEORDOALD;
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
            User *userRemp = Datas::I()->users->getById(idparent, Item::LoadDetails);
            if(userRemp && userRemp->titre().size())
                Entete.replace("{{TITREUSER}}", "<s>" + userRemp->titre() + " " + userRemp->prenom() + " " + userRemp->nom() + "</s> "
                                                "<font color=\"darkblue\">" + tr ("remplacé par") + " "
                                                + user->titre() + " " + user->prenom() + " " + user->nom())
                               + "</font>";
            else
                Entete.replace("{{TITREUSER}}", "<s>" + userRemp->prenom() + " " + userRemp->nom() + " </s> "
                                                "<font color=\"red\">" + tr ("remplacé par") + " "
                                                + user->prenom() + " " + user->nom())
                               + "</font>";
        }
        else
        {
            if (user->titre() != "")
                Entete.replace("{{TITREUSER}}", user->titre() + " " + user->prenom() + " " + user->nom());
            else
                Entete.replace("{{TITREUSER}}", user->prenom() + " " + user->nom());
        }
        if(user->numspecialite() != 0)
            Entete.replace("{{SPECIALITE}}", QString::number(user->numspecialite()) + " " + user->specialite());
        else
            Entete.replace("{{SPECIALITE}}", user->specialite());

        QString adresse ="";
        int nlignesadresse = 0;
        Site *sit = Datas::I()->sites->currentsite();
        if (user != m_currentuser)
            sit = Datas::I()->sites->getById(db->loadSitesByUser(user->id()).at(0)->id()); //TODO ça ne va pas parce qu'on prend arbitrairement la première adreesse
        if( sit->nom().size() )
        {
            nlignesadresse  ++;
            adresse         += sit->nom();
        }
        if (sit->adresse1() != "" || sit->adresse2() != "")
        {
            nlignesadresse  ++;
            if (nlignesadresse >0)
                adresse += "<br />";
            if (sit->adresse1() != "" && sit->adresse2() != "")
                adresse += sit->adresse1() + " - " + sit->adresse2();
            else
                adresse += sit->adresse1() + sit->adresse2();
        }
        Entete.replace("{{ADRESSE}}", adresse);
        Entete.replace("{{CPVILLE}}", QString::number(sit->codePostal()) + " " + sit->ville().toUpper());
        Entete.replace("{{TEL}}", "Tél. " + sit->telephone());
        if (nlignesadresse==2)
            Entete.replace("{{LIGNESARAJOUTER}}", "<span style=\"font-size:5pt;\"> <br /></span>");
        else
            Entete.replace("{{LIGNESARAJOUTER}}", "");

        QString NumSS = "";
        if( user->numOrdre().size() )
        {
            NumSS = user->numOrdre();
            if( user->NumPS() > 0 ) NumSS += " - ";
        }
        if (user->NumPS() > 0) NumSS += "RPPS " + QString::number(user->NumPS());
        Entete.replace("{{NUMSS}}", NumSS);
        Entete.replace("{{DATE}}", sit->ville()  + tr(", le ") + date.toString(tr("d MMMM yyyy")));

        (i==1? EnteteMap["Norm"] = Entete : EnteteMap["ALD"] = Entete);
    }
    return EnteteMap;
}

//TODO : à déplacer
/*---------------------------------------------------------------------------------
    Retourne le pied du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::CalcPiedImpression(User *user, bool lunettes, bool ALD)
{
    QString Pied;
    if (ALD)
        Pied = "<html><div align =\"center\"><table>{{DUPLI}}</table></div></html>";
    else
    {
        QString nomModelePied = QDir::homePath() + FILE_PIEDPAGE;
        if (lunettes)
            nomModelePied = QDir::homePath() + FILE_PIEDORDOLUNETTES;
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
        a = TexteAImprimer->preview(Etat->document(), QDir::homePath() + FILE_PDF, "");
    else
    {
        if (!AvecChoixImprimante)
            TexteAImprimer->setPrinterName(m_nomImprimante);
        a = TexteAImprimer->print(Etat->document(), QDir::homePath() + FILE_PDF, "", AvecChoixImprimante);
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
    m_nomImprimante = TexteAImprimer->getPrinterName();
    delete TexteAImprimer;
    return a;
}

bool Procedures::Cree_pdf(QTextEdit *Etat, QString EnTete, QString Pied, QString nomfichier, QString nomdossier)
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

void Procedures::CalcImage(Item *item, bool imagerie, bool afficher)
{
    /*! Cette fonction sert à calculer les propriétés m_blob et m_formatimage des documents d'imagerie ou des courriers émis par le logiciel
     *  pour pouvoir les afficher ou les imprimer

   * \param afficher = true -> la fonction est appelée par Slot_AfficheDoc(), on utilise la table impressions
     *      imagerie = false -> Le document est un document texte (ordo, certificat...etc).
     *                          Il est déjà dans la table impressions sous la forme de 3 champs html (entete, corps et pied)
     *                          Ces champs vont être utilisés pour l'impression vers un fichier pdf.
     *                          Le bytearray sera constitué par le contenu de ce fichier et affiché à l'écran.
     *      imagerie = true ->  le document est un document d'imagerie stocké sur un fichier. On va le transférer dans la table echangeimages et le transformer en bytearray

   * \param afficher = false -> la fonction est applée par dlg_docsexternes::ReImprimeDoc(DocExterne *docmt) - on utilise la table echangeimages
     *      pour imprimer un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour imprimer un document d'imagerie stocké dans la table echangeimages - on va extraire le ByteArray directement de la base de la table echangeimages
     * La fonction est aussi appelée par la table dépenses pour afficher les factures
    */
    DocExterne *docmt = dynamic_cast<DocExterne*>(item);
    Depense *dep = Q_NULLPTR;
    if (docmt == Q_NULLPTR)
        dep = dynamic_cast<Depense*>(item);
    if (docmt == Q_NULLPTR && dep == Q_NULLPTR)
        return;
    QString iditem;
    QString date ("");
    QString sstitre;
    QString imgs;
    QString typedocmt ("");
    QString soustypedocmt ("");
    QString objet ("");
    QString filename = "";

    if (docmt != Q_NULLPTR)
    {
        iditem = QString::number(docmt->id());
        date = docmt->date().toString(tr("d-M-yyyy"));
        typedocmt = docmt->typedoc();
        soustypedocmt = docmt->soustypedoc();
        filename = docmt->lienversfichier();
    }
    else if (dep != Q_NULLPTR)
    {
        iditem = QString::number(dep->idfacture());
        date = dep->date().toString(tr("d-M-yyyy"));
        objet = dep->objet();
        filename = dep->lienfacture();
    }
    QByteArray ba;
    QLabel inflabel;
    if (imagerie)                                           //!> il s'agit d'un fichier image
    {
        if (afficher)                                       //! si on veut afficher une image, on la charge dans une table SQL
                                                            //! pour pouvoir véhiculer son contenu dans le tunnel SQL et profiter du cryptage en cas d'accès distant
        {
            if (docmt != Q_NULLPTR)
                sstitre = "<font color='magenta'>" + date + " - " + typedocmt + " - " + soustypedocmt + "</font>";
            else if (dep != Q_NULLPTR)
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
                if (docmt != Q_NULLPTR)
                    imgs = "select idimpression from " TBL_ECHANGEIMAGES " where idimpression = " + iditem + " and (pdf is not null or jpg is not null)";
                else
                    imgs = "select " CP_IDFACTURE_FACTURES " from " TBL_FACTURES " where " CP_IDFACTURE_FACTURES " = " + iditem + " and (" CP_PDF_FACTURES " is not null or " CP_JPG_FACTURES " is not null)";
                //qDebug() << imgs;
                QList<QVariantList> listid = db->StandardSelectSQL(imgs, m_ok);
                if (!m_ok)
                    UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'accéder à la table ") + (docmt != Q_NULLPTR? TBL_ECHANGEIMAGES : TBL_FACTURES));
                if (listid.size()==0)
                {
                    if (docmt != Q_NULLPTR)
                    {
                        db->StandardSQL("delete from " TBL_ECHANGEIMAGES
                                        " where idimpression = " + iditem +
                                        " and facture is null");
                        QString req = "INSERT INTO " TBL_ECHANGEIMAGES " (idimpression, " + sfx + ", compression)"
                                                                                                  " VALUES (" +
                                iditem + ", " +
                                " LOAD_FILE('" + Utils::correctquoteSQL(DirImagerieServeur() + DIR_IMAGES + Utils::correctquoteSQL(filename)) + "'), " +
                                QString::number(docmt->compression()) + ")";
                        db->StandardSQL(req);
                    }
                    else
                    {
                        db->StandardSQL("delete from " TBL_ECHANGEIMAGES
                                                             " where idimpression = " + iditem +
                                                             " and facture = 1");
                        QString req = "INSERT INTO " TBL_ECHANGEIMAGES " (idimpression, " + sfx + ", facture) "
                                      "VALUES (" +
                                      iditem + ", " +
                                      " LOAD_FILE('" + Utils::correctquoteSQL(DirImagerieServeur() + DIR_FACTURES + Utils::correctquoteSQL(filename)) + "'), " +
                                      "1)";
                        db->StandardSQL(req);
                    }
                }
            }
        }
        //! On charge ensuite le contenu des champs longblob des tables concernées en mémoire pour les afficher
        QList<QVariantList> listimpr;
        if (docmt != Q_NULLPTR)
        {
            listimpr = db->StandardSelectSQL("select pdf, jpg, compression  from " TBL_ECHANGEIMAGES " where idimpression = " + iditem + " and facture is null"
                                                                  , m_ok
                                                                  , tr("Impossible d'accéder à la table ") + TBL_ECHANGEIMAGES);
            if (!m_ok)
                return;
            if (listimpr.size()==0)                             // le document n'est pas dans echangeimages, on va le chercher dans impressions
                listimpr = db->StandardSelectSQL("select pdf, jpg, compression  from " TBL_DOCSEXTERNES " where idimpression = " + iditem
                                                                      , m_ok
                                                                      , tr("Impossible d'accéder à la table ") + TBL_DOCSEXTERNES);
        }
        else
        {
            listimpr = db->StandardSelectSQL("select pdf, jpg  from " TBL_ECHANGEIMAGES " where idimpression = " + iditem + " and facture = 1"
                                                                  , m_ok
                                                                  , tr("Impossible d'accéder à la table ") + TBL_ECHANGEIMAGES);
            if (!m_ok)
                return;
            if (listimpr.size()==0)                             // le document n'est pas dans echangeimages, on va le chercher dans factures
                listimpr = db->StandardSelectSQL("select " CP_PDF_FACTURES ", " CP_JPG_FACTURES "  from " TBL_FACTURES " where " CP_IDFACTURE_FACTURES " = " + iditem
                                                                      , m_ok
                                                                      , tr("Impossible d'accéder à la table ") + TBL_FACTURES);
        }

        if (listimpr.size()==0)
            return;
        QVariantList impr = listimpr.at(0);
        if (impr.at(0).toByteArray().size()>0)            //! le champ SQL pdf de la requête qui est exploré et s'il n'est pas vide, c'est un pdf
        {
            if (docmt != Q_NULLPTR)
            {
                if (impr.at(2).toString()=="1")
                    ba.append(qUncompress(impr.at(0).toByteArray()));
                else
                    ba.append(impr.at(0).toByteArray());
                docmt->setimageformat(PDF);
            }
            else
            {
                ba.append(impr.at(0).toByteArray());
                dep->setfactureformat(PDF);
            }
         }
        else if (impr.at(1).toByteArray().size()>0)       //! le champ SQL jpg de la requête qui est exploré et s'il n'est pas vide, c'est un jpg
        {
            ba.append(impr.at(1).toByteArray());
            if (docmt != Q_NULLPTR)
                docmt->setimageformat(JPG);
            else
                dep->setfactureformat(JPG);
        }
        if (docmt != Q_NULLPTR)
            docmt->setimageblob(ba);
        else
            dep->setfactureblob(ba);
    }
    else                                                    //!> il s'agit d'un document écrit, on le traduit en pdf et on l'affiche
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
        QString ficpdf = QDir::homePath() + FILE_PDF;
        TexteAImprimer->print(Etat_textEdit->document(), ficpdf, "", false, true);
        // le paramètre true de la fonction print() génère la création du fichier pdf FILE_PDF et pas son impression
        QFile filepdf(ficpdf);
        if (!filepdf.open( QIODevice::ReadOnly ))
            UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier:\n") + ficpdf, tr("Impossible d'enregistrer l'impression dans la base"));
        ba = filepdf.readAll();
        filepdf.close ();
        docmt->setimageformat(PDF);
        docmt->setimageblob(ba);
    }
}

QString Procedures::Edit(QString txt, QString titre, bool editable, bool ConnectAuSignal)
{
    QString         rep("");
    QString         geometry("PositionsFiches/PositionEdit");
    UpDialog        *gAsk           = new UpDialog();
    UpTextEdit      *TxtEdit        = new UpTextEdit(gAsk);
    int x = QGuiApplication::screens().first()->geometry().width();
    int y = QGuiApplication::screens().first()->geometry().height();

    gAsk->setModal(true);
    gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    TxtEdit->setText(txt);
    TxtEdit->setTextInteractionFlags(editable? Qt::TextEditorInteraction : (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard));

    gAsk->setMaximumWidth(x);
    gAsk->setMaximumHeight(y);
    gAsk->setWindowTitle(titre);

    gAsk->dlglayout()->insertWidget(0,TxtEdit);

    gAsk->AjouteLayButtons();
    connect(gAsk->OKButton, &QPushButton::clicked,  gAsk,       &QDialog::accept);
    if (ConnectAuSignal)
        connect(this,       &Procedures::ModifEdit, TxtEdit,    [=](QString txt) {TxtEdit->setText(txt);});
    gAsk->restoreGeometry(m_settings->value(geometry).toByteArray());

    if (gAsk->exec()>0)
        rep = TxtEdit->toHtml();
    m_settings->setValue(geometry,gAsk->saveGeometry());
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
    connect(gAsk->OKButton, &QPushButton::clicked,  gAsk, &QDialog::accept);

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
    UpDialog    *gAsk           = new UpDialog();
    wdg_tablewidget             = new UpTableWidget(gAsk);
    wdg_inflabel                = new UpLabel(wdg_tablewidget);
    m_listeimages               = wdg_tablewidget->AfficheDoc(doc);
    wdg_tablewidget ->installEventFilter(this);
    gAsk->setModal(true);
    gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    gAsk->setWindowTitle(titre);
    gAsk->dlglayout()->insertWidget(0,wdg_tablewidget);
    wdg_tablewidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    wdg_tablewidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    gAsk->AjouteLayButtons(Button);
    connect(gAsk->OKButton, &QPushButton::clicked, this, [=] {gAsk->accept();});
    connect(this, &Procedures::CloseEditDocument, gAsk, [=] {gAsk->accept();});
    if (Button.testFlag(UpDialog::ButtonPrint))
    {
        gAsk->PrintButton->setdata(doc);
        connect(gAsk->PrintButton, QOverload<QVariant>::of(&UpSmallButton::clicked), [=](QVariant) {PrintDocument(doc);});
    }
    if (Button.testFlag(UpDialog::ButtonSuppr))
        connect(gAsk->SupprButton, &QPushButton::clicked, this, [=] {emit DelImage();});

    int x = QGuiApplication::screens().first()->geometry().width();
    int y = QGuiApplication::screens().first()->geometry().height();
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
    QPixmap pix = QPixmap::fromImage(m_listeimages.at(0).scaled(QSize(x,y),
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
    wdg_tablewidget->resize(wtable, htable);
    int delta = 0;
    for (int i=0; i < wdg_tablewidget->rowCount(); i++)
    {
        UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_tablewidget->cellWidget(i,0));
        if (lbl != Q_NULLPTR)
        {
            pix = pix.scaled(wtable- delta, htable - delta, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            lbl->setPixmap(pix);
            wdg_tablewidget->setRowHeight(i,htable-delta);
            wdg_tablewidget->setColumnWidth(0,wtable-delta);
        }
    }

    if (w > (x - gAsk->x()))
        gAsk->move(x - w, 0);

    wdg_inflabel    ->setText("<font color='magenta'>" + label + "</font>");
    QFont font = qApp->font();
    font.setPointSize(12);
    wdg_inflabel->setFont(font);
    wdg_inflabel    ->setGeometry(10,htable-40,350,25);

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
                    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(p_printer);
                    connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(p_printer, image);});
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
                    QPrintDialog *dialog = new QPrintDialog(p_printer);
                    if (dialog->exec() != QDialog::Rejected)
                    {
                        p_printer = dialog->printer();
                        Print(p_printer, image);
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
                Print(p_printer, image);
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
            QPrintPreviewDialog *dialog = new QPrintPreviewDialog(p_printer);
            connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(p_printer, image);});
            dialog->exec();
            delete dialog;
        }
        else
        {
            QPrintDialog *dialog = new QPrintDialog(p_printer);
            if (dialog->exec() != QDialog::Rejected)
                Print(p_printer, image);
            delete dialog;
        }
    }
    return true;
}

void Procedures::Print(QPrinter *Imprimante, QImage image)
{
    QPainter PrintingPreView(Imprimante);
    QPixmap pix = QPixmap::fromImage(image).scaledToWidth(int(m_rect.width()),Qt::SmoothTransformation);
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
    return m_initok;
}

bool Procedures::ApercuAvantImpression()
{
    return (m_settings->value("Param_Imprimante/ApercuAvantImpression").toString() == "YES");
}

QString Procedures::CodePostalParDefaut()
{
    QSettings set(QDir::homePath() + FILE_INI, QSettings::IniFormat);
    return set.value("Param_Poste/CodePostalParDefaut").toString();
}

QString Procedures::SessionStatus()
{
    // statut de l'utilisateur pour cette session

    /* Valeurs de soignant
     * 1 = ophtalmo
     * 2 = Orthoptiste
     * 3 = autre soignant
     * 4 = non soignant
     * 5 = societe comptable
     */

    bool ophtalmo       = m_currentuser->isOpthalmo();
    bool orthoptist     = m_currentuser->isOrthoptist();
    bool autresoignant  = m_currentuser->isAutreSoignant();
    bool soccomptable   = m_currentuser->isSocComptable();
    bool medecin        = m_currentuser->isMedecin();

    bool assistant      = m_currentuser->isAssistant();
    bool responsable    = m_currentuser->isResponsable();
    bool responsableles2= m_currentuser->isResponsableOuAssistant();

    bool liberal        = m_currentuser->isLiberal();
    bool pasliberal     = m_currentuser->isSalarie();
    bool retrocession   = m_currentuser->isRemplacant();
    bool pasdecompta    = m_currentuser->isSansCompta();

    bool cotation       = m_currentuser->useCCAM();

    bool soignant           = m_currentuser->isSoignant();
    bool soigntnonassistant = soignant && !assistant;
    bool respsalarie        = soigntnonassistant && pasliberal;
    bool respliberal        = soigntnonassistant && liberal;


    QString txtstatut = tr("Vos données enregistrées pour cette session")+ "\n\n" + m_currentuser->status();
    txtstatut += "\n" + tr("Site") + "\t\t= " + Datas::I()->sites->currentsite()->nom();
    txtstatut += "\n\n";

    txtstatut += tr("Vos données permanentes") +"\n\n" + tr("Fonction :") + "\t\t\t";
    if (ophtalmo)
        txtstatut += tr("Ophtalmologiste");
    else if (orthoptist)
        txtstatut += tr("Orthoptiste");
    else if (autresoignant)
        txtstatut += m_currentuser->specialite();
    else
        txtstatut += m_currentuser->fonction();

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
        txtstatut += "\n" + tr("RPPS :\t\t\t") + QString::number(m_currentuser->NumPS());
    if (medecin && ! assistant)
        txtstatut += "\nADELI :\t\t\t" + m_currentuser->numOrdre();
    if (soignant)
    {
        txtstatut += "\n" + tr("Exercice :\t\t\t");
        if (liberal)
            txtstatut += tr("libéral");
        else if (pasliberal)
            txtstatut += tr("salarié") + " - " + tr("Employeur : ") + Datas::I()->users->getById(m_currentuser->idemployeur())->login();
        else if (retrocession)
            txtstatut += tr("remplaçant");
        else if (pasdecompta)
            txtstatut += tr("sans comptabilité");
    }
    if (respliberal)
        txtstatut += "\n" + tr("Honoraires encaissés sur le compte :\t") + Datas::I()->comptes->getById(m_currentuser->idcompteencaissementhonoraires())->nomabrege() + " " + tr("de") + " " + m_currentuser->comptable()->login();
    else if (respsalarie)
    {
        User *employeur = Datas::I()->users->getById(m_currentuser->idemployeur(), Item::LoadDetails);
        Compte *cptt = Datas::I()->comptes->getById(employeur->idcompteencaissementhonoraires());
        if (cptt != Q_NULLPTR)
        {
            txtstatut += "\n" + tr("Honoraires encaissés sur le compte :\t");
            txtstatut += cptt->nomabrege() + " ";
            txtstatut += tr("de") + " " + Datas::I()->users->getById(m_currentuser->idemployeur())->login();
        }
    }
    else if (retrocession)
        txtstatut += "\n" + tr("Statut :\t\t\t") + tr("remplaçant");
    if (soigntnonassistant && cotation)
        txtstatut += "\n" + tr("Cotation des actes :\t\t") + (cotation? tr("Oui") : tr("Sans"));
    if (medecin && cotation)
    {
        QString secteur ("");
        switch (m_currentuser->secteurconventionnel()) {
        case 1:     secteur = "1";          break;
        case 2:     secteur = "2";          break;
        case 3:     secteur = "3";          break;
        default:
            break;
        }
        txtstatut += "\n" + tr("Secteur conventionnel :\t\t") + secteur;
        txtstatut += "\n" + tr("OPTAM :\t\t\t") + (m_currentuser->isOPTAM() ? "Oui": "Non");
    }
    if (respliberal || soccomptable)
    {
        QString cptabledefaut ("");
        if (m_currentuser->idcomptepardefaut() > 0)
            cptabledefaut = tr("de") + " " + Datas::I()->users->getById(Datas::I()->comptes->getById(m_currentuser->idcomptepardefaut())->idUser())->login();
        txtstatut += "\n" + tr("Comptabilité enregistrée sur compte :\t") + Datas::I()->comptes->getById(m_currentuser->idcomptepardefaut())->nomabrege() + " "
                          + cptabledefaut;
    }
    if (respliberal)
        txtstatut += "\n" + tr("Membre d'une AGA :\t\t") + (m_currentuser->isAGA() ? tr("Oui") : tr("Sans"));
    return txtstatut;
}

/*! --------------------------------------------------------------------------------------------------------------------------------------
    -- détermine le dossier où est stockée l'imagerie -----------------------------------------------------------
    DirStockageImages           = l'emplacement du dossier de l'imagerie sur le poste                   quand on est en mode posetr
                                = l'emplacement du dossier de l'imagerie sur le serveur vu par le poste quand on est en mode réseau local
                                = l'emplacement de dossier des copies des images d'origine sur le poste quand on est en mode distant
                                -> utilisé par les postes pour enregistrer une copie de sauvegarde de l'original des fichiers images intégrés dans la base
    DirStockageImagesServeur    = l'emplacement du dossier de l'imagerie sur le serveur - correspond au champ dirimagerie de la table parametressysteme
                                -> utilisé par les requêtes SQL pour réintégrer le contenu de fichiers images dans la base
    ------------------------------------------------------------------------------------------------------------------------------------*/
void Procedures::setAbsolutePathDirImagerie()
{
    m_absolutepathDirStockageImage = "";
    m_pathDirStockageImagesServeur = m_parametres->dirimagerie();
    if (db->getMode() == Utils::Poste)
        m_absolutepathDirStockageImage = m_pathDirStockageImagesServeur;
    else
        m_absolutepathDirStockageImage = m_settings->value(db->getBase() + "/DossierImagerie").toString();
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- renvoie la valeur du dossier où est stockée l'imagerie -----------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::AbsolutePathDirImagerie()
{
    return m_absolutepathDirStockageImage;
}

QString Procedures::DirImagerieServeur()
{
    return m_pathDirStockageImagesServeur;
}

void Procedures::setFicheRefractionOuverte(bool a)
{
    m_dlgrefractionouverte = a;
}

bool Procedures::FicheRefractionOuverte()
{
    return m_dlgrefractionouverte;
}

bool Procedures::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==wdg_tablewidget)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            for (int i=0; i < wdg_tablewidget->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_tablewidget->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    QPixmap pix = QPixmap::fromImage(m_listeimages.at(i).scaled(wdg_tablewidget->width(), wdg_tablewidget->height(),
                                                           Qt::KeepAspectRatioByExpanding,
                                                           Qt::SmoothTransformation));
                    lbl->setPixmap(pix);
                    wdg_tablewidget->setRowHeight(i,lbl->pixmap()->height());
                    wdg_tablewidget->setColumnWidth(i,lbl->pixmap()->width());
                }
            }
            wdg_inflabel    ->move(10,wdg_tablewidget->height()-40);
        }
    }
    return true;
}

void Procedures::ReconstruitComboCorrespondants(QComboBox* box, Correspondants::TYPECORRESPONDANT typ)
{
    box->clear();
    QStandardItemModel *model = new QStandardItemModel();
    // toute la manip qui suit sert à remettre les correspondants par ordre aplhabétique (dans le QMap, ils sont triés par id croissant) - si  vous trouvez plus simple, ne vous génez pas
    foreach ( Correspondant *cor, Datas::I()->correspondants->correspondants()->values() )
    {
        QList<QStandardItem *> items;
        items << new QStandardItem(cor->nom() + " "  + cor->prenom()) << new QStandardItem(QString::number(cor->id()));
        if (typ == Correspondants::TousLesCorrespondants)
            model->appendRow(items);
        else if (cor->ismedecin())
            model->appendRow(items);
    }
    model->sort(0);
    for(int i=0; i<model->rowCount(); i++)
        box->addItem(model->item(i)->text(), model->item(i,1)->text());
}

//Pas normal, les mots de passes doivent etre chiffrés
QString Procedures::MDPAdmin()
{
    if (m_parametres == Q_NULLPTR)
        m_parametres = db->parametres();
    if (m_parametres == Q_NULLPTR)
        return NOM_MDPADMINISTRATEUR;
    if (m_parametres->mdpadmin() == "")
        db->setmdpadmin(NOM_MDPADMINISTRATEUR);
    return m_parametres->mdpadmin();
}

void Procedures::setNomImprimante(QString NomImprimante)
{
    m_nomImprimante = NomImprimante;
}

QString Procedures::nomImprimante()
{
    return m_nomImprimante;
}

int Procedures::TailleEnTete()
{
    return m_settings->value("Param_Imprimante/TailleEnTete").toInt();
}

int Procedures::TailleEnTeteALD()
{
    return m_settings->value("Param_Imprimante/TailleEnTeteALD").toInt();
}

int Procedures::TaillePieddePage()
{
    return m_settings->value("Param_Imprimante/TaillePieddePage").toInt();
}

int Procedures::TaillePieddePageOrdoLunettes()
{
    return m_settings->value("Param_Imprimante/TaillePieddePageOrdoLunettes").toInt();
}

int Procedures::TailleTopMarge()
{
    return m_settings->value("Param_Imprimante/TailleTopMarge").toInt();
}

QString Procedures::VilleParDefaut()
{
    QSettings set(QDir::homePath() + FILE_INI, QSettings::IniFormat);
    return set.value("Param_Poste/VilleParDefaut").toString();
}

void Procedures::setPosteImportDocs(bool a)
{
    /*! Il n'y pas de variables utilisateur globale dans MySQL, on est donc obligé de passer par une procédure stockée pour en simuler une
    * pour créer une procédure avec Qt, séparer le drop du create, ne pas utiliser les délimiteurs et utiliser les retours à la ligne \n\.......
    * if (gsettingsIni->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs").toString() ==  "YES")

    * si a = true, on se met en poste importateur +/_ prioritaire à la fin suivant le contenu de rufus.ini
    * si a = false, on retire le poste en cours et on met NULL à la place. */

    QString req = "USE `" DB_CONSULTS "`;";
    db->StandardSQL(req);

    req = "DROP PROCEDURE IF EXISTS " NOM_POSTEIMPORTDOCS ";";
    db->StandardSQL(req);

    QString IpAdress("NULL");
    if (a)
    {
        if (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs").toString() ==  "YES")
            IpAdress = QHostInfo::localHostName() + " - prioritaire" ;
        else
            IpAdress = QHostInfo::localHostName();
    }
    req = "CREATE PROCEDURE " NOM_POSTEIMPORTDOCS "()\n\
          BEGIN\n\
          SELECT '" + IpAdress + "';\n\
          END ;";
    db->StandardSQL(req);
}

QString Procedures::PosteImportDocs()
{   QString rep = "";
    QString req = "SELECT name FROM mysql.proc p WHERE db = '" DB_CONSULTS "' AND name = '" NOM_POSTEIMPORTDOCS "'";
    QVariantList imptdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
    if (m_ok && imptdata.size()>0)
    {
        req = "CALL " DB_CONSULTS "." NOM_POSTEIMPORTDOCS;
        QVariantList calldata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
        //qDebug() << "nbre reponses = " + QString::number(calldata.size()) << NOM_POSTEIMPORTDOCS " = " + calldata.at(0).toString();
        if (m_ok && calldata.size()>0)
            rep = calldata.at(0).toString();
    }
    //qDebug() << "posteimportdocs = " + rep;
    return rep;
}

bool Procedures::Verif_secure_file_priv()
{
    QString msg = QString();
    QVariantList vardata = db->getFirstRecordFromStandardSelectSQL("SHOW VARIABLES LIKE \"secure_file_priv\";", m_ok);
    if (m_ok && vardata.size()>0)
        msg = vardata.at(1).toString();
    if (msg == "NULL")
        msg = QString();
    if (msg==QString())
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

bool Procedures::ReinitBase()
{
    if (AutresPostesConnectes())
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
        if (!ImmediateBackup("", true, true))
            return false;
        QFile FichierIni(m_nomFichierIni);
        if (FichierIni.exists())
        {
            QFile FichierBup(QDir::homePath() + DIR_RUFUS + "/RufusBackup.ini");
            if (FichierBup.exists())
                FichierBup.remove();
            FichierIni.copy(QDir::homePath() + DIR_RUFUS + "/RufusBackup.ini");
            FichierIni.remove();
        }
        UpMessageBox::Information(Q_NULLPTR, tr("Arrêt du programme!"));
        exit(0);
    }
    return false;
}

qint64 Procedures::CalcBaseSize()
{
    qint64 basesize = 0;
    QString req = "SELECT SUM(SizeMB) from "
                      "(SELECT table_schema, round(sum(data_length+index_length)/1024/1024,4) AS SizeMB FROM information_schema.tables"
                      " where table_schema = '" DB_COMPTA "'"
                      " or table_schema = '" DB_CONSULTS "'"
                      " or table_schema = '" DB_OPHTA "'"
                      " GROUP BY table_schema)"
                      " as bdd";
    QVariantList basedata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
    if (m_ok && basedata.size()>0)
        basesize = basedata.at(0).toLongLong();
    return basesize;
}

void Procedures::CalcTimeBupRestore()
{
    double time(0);
    qint64 volume(0);
    QList<UpCheckBox*> listchk = dlg_buprestore->findChildren<UpCheckBox*>();
    for (int i= 0; i<listchk.size(); i++)
    {
        if (listchk.at(i)->accessibleDescription() == "base")
        {
            if (listchk.at(i)->isChecked())
                volume += m_basesize;
        }
        if (listchk.at(i)->accessibleDescription() == "images")
        {
            if (listchk.at(i)->isChecked())
                volume += m_imagessize;
        }
        if (listchk.at(i)->accessibleDescription() == "videos")
        {
            if (listchk.at(i)->isChecked())
                volume += m_videossize;
        }
        if (listchk.at(i)->accessibleDescription() == "factures")
        {
            if (listchk.at(i)->isChecked())
                volume += m_facturessize;
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
    QString color = m_freespace>volume? "green": "red";
    QString msg = tr("Volume à transférer: ") + " <font color=\""+ color + "\"><b>" + Volumelitteral + "</b></font>";
    wdg_resumelbl->setText(msg + " - " + tr("Temps estimé: ") + timelitteral);
    QString Volumelibre = Utils::getExpressionSize(m_freespace);
    msg = tr("Volume disponible sur le disque: ") + " <font color=\""+ color + "\"><b>" + Volumelibre + "</b></font>";

    wdg_volumelibrelbl->setText(msg);
    dlg_buprestore->OKButton->setEnabled(m_freespace>volume);
}

bool Procedures::RestaureBase(bool BaseVierge, bool PremierDemarrage, bool VerifPostesConnectes)
{
    UpMessageBox    msgbox;
    UpSmallButton   AnnulBouton;
    UpSmallButton   OKBouton;
    msgbox.setIcon(UpMessageBox::Warning);
    bool echecfile = true;

    if (BaseVierge)
    {
        QString Hote;
        if (db->getMode() == Utils::Poste)
            Hote = tr("ce poste");
        else
            Hote = tr("le serveur ") + m_settings->value(db->getBase() + "/Serveur").toString();
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

        if (!Utils::VerifMDP((PremierDemarrage? NOM_MDPADMINISTRATEUR : MDPAdmin()),tr("Saisissez le mot de passe Administrateur")))
            return false;

        QFile BaseViergeFile(QStringLiteral("://basevierge.sql"));
        BaseViergeFile.copy(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES "/basevierge.sql");
        QFile DumpFile(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES "/basevierge.sql");
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
            emit ConnectTimers(false);
            int a = 99;
            // +++ la fonction DefinitScriptRestore() qu'on pourrait vouloir utiliser dans ce cas là avec le fichier basevierge.sql ne fonctionne pas avec ce fichier
            // et je ne sais pas pourquoi
            // et j'en ai marre de chercher pourquoi
            QStringList listinstruct = Utils::DecomposeScriptSQL(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES "/basevierge.sql");
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
                emit ConnectTimers(true);
                return true;
            }
        }
        UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'éxécuter la restauration!") + "\n" + tr("Le fichier ") + "basevierge.sql" + tr(" ne semble pas conforme!"));
        DumpFile.remove();
        return false;
    }
    else
    {
        if (VerifPostesConnectes)
            if (AutresPostesConnectes())
                 return false;

        /*! 1 - choix du dossier où se situe la sauvegarde */
        UpMessageBox::Information(Q_NULLPTR, tr("Choix du dossier de sauvegarde"),
                                  tr("Dans la fiche suivante, choisissez le dossier "
                                  "contenant la sauvegarde de la base.\n\n"
                                  "Une fois le dossier sélectionné, "
                                  "la sauvegarde commencera automatiquement.\n"
                                  "Ce processus est long et peut durer plusieurs minutes.\n"
                                  "(environ 1' pour 2 Go)\n"));
        QString dir = QDir::homePath() + DIR_RUFUS;
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
        if (!Utils::VerifMDP((PremierDemarrage? NOM_MDPADMINISTRATEUR : MDPAdmin()),tr("Saisissez le mot de passe Administrateur")))
            return false;


        /*! ---------------------------------------------------------------------------------------------------------------------------------------------------------
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

        /*! 2 - détermination des éléments pouvant être restaurés */
        if (QDir(dirtorestore.absolutePath() + DIR_RESSOURCES).exists())
            if (QDir(dirtorestore.absolutePath() + DIR_RESSOURCES).entryList(QDir::Files | QDir::NoDotAndDotDot).size()>0)
                OKRessces = true;
        if (QFile(dirtorestore.absolutePath() + "/Rufus.ini").exists())
            OKini = true;
        QDir rootimgvid = dirtorestore;
        if (rootimgvid.cdUp())
        {
            //qDebug() << rootimgvid.absolutePath() + DIR_IMAGES;
            if (QDir(rootimgvid.absolutePath() + DIR_IMAGES).exists())
                if (QDir(rootimgvid.absolutePath() + DIR_IMAGES).entryList(QDir::Dirs).size()>0)
                    OKImages = true;
            if (QDir(rootimgvid.absolutePath() + DIR_VIDEOS).exists())
                if (QDir(rootimgvid.absolutePath() + DIR_VIDEOS).entryList(QDir::Files | QDir::NoDotAndDotDot).size()>0)
                    OKVideos = true;
            if (QDir(rootimgvid.absolutePath() + DIR_FACTURES).exists())
                if (QDir(rootimgvid.absolutePath() + DIR_FACTURES).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size()>0)
                    OKFactures = true;
        }

        /*! 3 - détermination de l'emplacement de destination des fichiers d'imagerie */
        QString NomDirStockageImagerie = QDir::homePath() + DIR_RUFUS DIR_IMAGERIE;
        if (OKImages || OKVideos || OKFactures)
        {
            NomDirStockageImagerie = (PremierDemarrage? QDir::homePath() + DIR_RUFUS DIR_IMAGERIE : m_parametres->dirimagerie());
            if (!QDir(NomDirStockageImagerie).exists())
            {
                bool exist = QDir().exists(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE);
                QString existdir = (exist? "" : "\n" + tr("Créez-le au besoin"));
                UpMessageBox::Watch(Q_NULLPTR,tr("Pas de dossier de stockage d'imagerie"),
                                    tr("Indiquez un dossier valide dans la boîte de dialogue qui suit") + "\n" +
                                    tr("Utilisez de préférence le dossier ") + QDir::homePath() + DIR_RUFUS DIR_IMAGERIE +
                                    existdir);
                QFileDialog dialogimg(Q_NULLPTR,tr("Stocker les images dans le dossier") , QDir::homePath() + DIR_RUFUS + (exist? DIR_IMAGERIE : ""));
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
                m_settings->setValue(Utils::getBaseFromMode(Utils::Poste) + "/DossierImagerie", NomDirStockageImagerie);
                if (!PremierDemarrage)
                    db->setdirimagerie(NomDirStockageImagerie);
            }
        }

        /*! 4 - choix des éléments à restaurer */
        AskBupRestore(RestoreOp, dirtorestore.absolutePath(), NomDirStockageImagerie, OKini, OKRessces, OKImages, OKVideos, OKFactures);
        int result = dlg_buprestore->exec();
        if (result > 0)
        {
            emit ConnectTimers(false);
            foreach (UpCheckBox *chk, dlg_buprestore->findChildren<UpCheckBox*>())
            {
                /*! 4a - restauration de la base de données */
                if (chk->accessibleDescription() == "base")
                {
                    if (chk->isChecked())
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
                            //! Suppression de toutes les tables
                            QString Msg = tr("Suppression de l'ancienne base Rufus en cours");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                            db->VideDatabases();
                            int a = 99;

                            //! Restauration à partir du dossier sélectionné
                            DefinitScriptRestore(listnomsfilestorestore);
                            QString task = "sh " + QDir::homePath() + SCRIPTRESTOREFILE;
                            QProcess dumpProcess(parent());
                            dumpProcess.start(task);
                            dumpProcess.waitForFinished(1000000000);
                             if (dumpProcess.exitStatus() == QProcess::NormalExit)
                                a = dumpProcess.exitCode();
                            if (a != 0)
                                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                            QFile::remove(QDir::homePath() + SCRIPTRESTOREFILE);
                       }
                    }
                }
            }
            foreach (UpCheckBox *chk, dlg_buprestore->findChildren<UpCheckBox*>())
            {
                /*! 4b - restauration du fichier ini */
                if (chk->accessibleDescription() == "ini")
                {
                    if (chk->isChecked())
                    {
                        QString fileini = dirtorestore.absolutePath() + "/Rufus.ini";
                        QFile FichierIni(m_nomFichierIni);
                        if (FichierIni.exists())
                            FichierIni.remove();
                        QFile rufusini(fileini);
                        rufusini.copy(m_nomFichierIni);
                        msg += tr("Fichier de paramétrage Rufus.ini restauré\n");
                        UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Fichier de paramétrage Rufus.ini restauré"), Icons::icSunglasses(), 3000);
                    }
                }
                /*! 4c - restauration des fichiers ressources */
                else if (chk->accessibleDescription() == "ressources")
                {
                    if (chk->isChecked())
                    {
                        QDir DirRssces(QDir(dirtorestore.absolutePath() + DIR_RESSOURCES));
                        QDir sauvRssces;
                        if (!sauvRssces.exists(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES))
                            sauvRssces.mkdir(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES);
                        QStringList listnomfic = DirRssces.entryList();
                        for (int i=0; i<listnomfic.size(); i++)
                        {
                            QFile ficACopier(DirRssces.absolutePath() + "/" + listnomfic.at(i));
                            QString nomficACopier = QFileInfo(listnomfic.at(i)).fileName();
                            ficACopier.copy(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES + "/" + nomficACopier);
                        }
                        msg += tr("Fichiers de ressources d'impression restaurés\n");
                        UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Fichiers de ressources d'impression restaurés"), Icons::icSunglasses(), 3000);
                    }
                }
                /*! 4d - restauration des images */
                else if (chk->accessibleDescription() == "images")
                {
                    if (chk->isChecked())
                    {
                        QString dirdestinationimg   = NomDirStockageImagerie + DIR_IMAGES;
                        QDir DirDestImg(dirdestinationimg);
                        if (DirDestImg.exists())
                            DirDestImg.removeRecursively();
                        DirDestImg.mkdir(dirdestinationimg);
                        if (!DirDestImg.exists())
                        {
                            QString Msg = tr("le dossier de destination de l'imagerie n'existe pas");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                        }
                        else
                        {
                            QString Msg = (tr("Restauration des fichiers d'imagerie\n")
                                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base d'images"));
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                            QDir dirrestaureimagerie    = QDir(rootimgvid.absolutePath() + DIR_IMAGES);
                            QString task  = "cp -R " + dirrestaureimagerie.absolutePath() + " " + NomDirStockageImagerie;
                            QProcess::execute(task);
                            msg += tr("Fichiers d'imagerie restaurés\n");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Fichiers d'imagerie restaurés"), Icons::icSunglasses(), 3000);
                        }
                    }
                }
                /*! 4e - restauration des factures */
                else if (chk->accessibleDescription() == "factures")
                {
                    if (chk->isChecked())
                    {
                        QString dirdestinationfact  = NomDirStockageImagerie + DIR_FACTURES;
                        QDir DirDestFact(dirdestinationfact);
                        if (DirDestFact.exists())
                            DirDestFact.removeRecursively();
                        DirDestFact.mkdir(dirdestinationfact);
                        if (!DirDestFact.exists())
                        {
                            QString Msg = tr("le dossier de destination des factures n'existe pas");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                        }
                        else
                        {
                            QString Msg = (tr("Restauration des factures\n")
                                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de factures"));
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                            QDir dirrestaurefactures    = QDir(rootimgvid.absolutePath() + DIR_FACTURES);
                            QString task = "cp -R " + dirrestaurefactures.absolutePath() + " " + NomDirStockageImagerie;
                            QProcess::execute(task);
                            msg += tr("Fichiers factures restaurés\n");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Fichiers factures restaurés"), Icons::icSunglasses(), 3000);
                        }
                    }
                }
                /*! 4e - restauration des videos */
                else if (chk->accessibleDescription() == "videos")
                {
                    if (chk->isChecked())
                    {
                        QString dirdestinationvid   =  NomDirStockageImagerie + DIR_VIDEOS;
                        QDir DirDestVid(dirdestinationvid);
                        if (DirDestVid.exists())
                            DirDestVid.removeRecursively();
                        DirDestVid.mkdir(dirdestinationvid);
                        if (!DirDestVid.exists())
                        {
                            QString Msg = tr("le dossier de destination des videos n'existe pas");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                        }
                        else
                        {
                            QString Msg = (tr("Restauration des fichiers videos\n")
                                           + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                            QDir dirrestaurevideo = QDir(rootimgvid.absolutePath() + DIR_VIDEOS);
                            QString task = "cp -R " + dirrestaurevideo.absolutePath() + " " + NomDirStockageImagerie;
                            QProcess::execute(task);
                            msg += tr("Fichiers videos restaurés\n");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Fichiers videos restaurés"), Icons::icSunglasses(), 3000);
                        }
                    }
                }
            }
        }
        delete dlg_buprestore;
        //qDebug() << msg;
        UpMessageBox::Watch(Q_NULLPTR,tr("restauration terminée"),msg);
        emit ConnectTimers(true);
        return (result > 0);
    }
}

bool Procedures::VerifBaseEtRessources()
{
    int Versionencours  = 9; //correspond aux premières versions de MAJ de la base
    int Version         = VERSION_BASE;
    bool b;
    Versionencours = m_parametres->versionbase();
    b = (Versionencours < Version);

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
                    if (!ImmediateBackup("", true, false))
                        return false;
                BupDone = true;
            }
            Message::I()->SplashMessage(tr("Mise à jour de la base vers la version ") + "<font color=\"red\"><b>" + QString::number(Version) + "</b></font>", 1000);
            QString Nomfic = "://majbase" + QString::number(Version) + ".sql";
            QFile DumpFile(Nomfic);
            if (DumpFile.exists())
            {
                QString NomDumpFile = QDir::homePath() + "/Documents/Rufus/Ressources/majbase" + QString::number(Version) + ".sql";
                QFile::remove(NomDumpFile);
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
                    db->initParametres();
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
                 * il n'y a donc plus à faire cette vérification
                 * cette MAJ crée une ligne pour tous les dossiers n'ayant pas la correspondance dans la table renseignementsmedicauxpatients
                 */
                QList<QVariantList> listid =
                        db->StandardSelectSQL("SELECT idpat FROM " TBL_PATIENTS " pat"
                                              " where  pat.idpat not in (select rmp.idpat from " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " rmp)", m_ok);
                if (listid.size()>0)
                {
                    for (int i=0; i<listid.size(); i++)
                    {
                        QString req =   "INSERT INTO " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS
                                " (idPat) VALUES (" + listid.at(i).at(0).toString() + ")";
                        db->StandardSQL(req);
                    }
                    UpMessageBox::Watch(Q_NULLPTR,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version), QString::number(listid.size()) + tr(" enregistrements modifiés"));
                }
                else
                    UpMessageBox::Watch(Q_NULLPTR,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version));
                db->setversionbase(53);            }
        }
    }
    //verification des fichiers ressources
    if (m_settings->value("Param_Poste/VersionRessources").toInt() < VERSION_RESSOURCES)
    {
        PremierParametrageRessources();
        m_settings->setValue("Param_Imprimante/TailleEnTeteALD","63");
        m_settings->setValue("Param_Poste/VersionRessources", VERSION_RESSOURCES);
        Message::I()->SplashMessage(tr("Mise à jour des fichiers ressources vers la version ") + "<font color=\"red\"><b>" + QString::number(VERSION_RESSOURCES) + "</b></font>", 1000);
    }
    return true;
}


/*--------------------------------------------------------------------------------------------------------------
    -- Choix du mode de connexion ----------------------------------------------------------------------------------
    --------------------------------------------------------------------------------------------------------------*/
bool Procedures::FicheChoixConnexion()
{
    bool lPoste, lDistant, lReseauLocal;
    lPoste                          = (m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Active").toString() == "YES"
                                       && (m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Port").toInt() == 3306
                                       || m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Port").toInt() == 3307)
                                       );
    lReseauLocal                    = (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Active").toString() == "YES"
                                       && m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Serveur").toString() != ""
                                       && (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Port").toInt() == 3306
                                       || m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Port").toInt() == 3307)
                                       );
    lDistant                        = (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Active").toString() == "YES"
                                       && m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Serveur").toString() != ""
                                       && (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Port").toInt() == 3306
                                       || m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Port").toInt() == 3307)
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
        if (lPoste)         m_modeacces = Utils::Poste;
        if (lReseauLocal)   m_modeacces = Utils::ReseauLocal;
        if (lDistant)       m_modeacces = Utils::Distant;
        m_initok  = true;
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
        m_initok = false;
        if (msgbox.exec()>0)
        {
            m_initok = (msgbox.clickedpushbutton() != &RejectButton);
            if (m_initok)
            {
                if (msgbox.clickedpushbutton()      == &OKBouton)    m_modeacces = Utils::Poste;
                else if (msgbox.clickedpushbutton() == &NoBouton)    m_modeacces = Utils::ReseauLocal;
                else if (msgbox.clickedpushbutton() == &AnnulBouton) m_modeacces = Utils::Distant;
            }
        }
    }
    }
    return m_initok;
}

/*--------------------------------------------------------------------------------------------------------------
-- Connexion à Consults -------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
bool Procedures::Connexion_A_La_Base()
{
    db->init(*m_settings, m_modeacces);
    if (!IdentificationUser())
        return false;

    //initListeUsers();
    CalcLieuExercice();
    if (Datas::I()->sites->currentsite() == Q_NULLPTR)
        UpMessageBox::Watch(Q_NULLPTR,tr("Pas d'adresse spécifiée"), tr("Vous n'avez précisé aucun lieu d'exercice!"));
    m_connexionbaseOK = true;

    //Etrange le parametrage ici
    db->StandardSQL("set global sql_mode = 'NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES';");
    db->StandardSQL("SET GLOBAL event_scheduler = 1 ;");
    db->StandardSQL("SET GLOBAL max_allowed_packet=" MAX_ALLOWED_PACKET "*1024*1024 ;");

    // on recherche si rufusadmin est en fonction auquel cas on utilise les TCPsocket
    QString req = "select iduser from " TBL_USERSCONNECTES " where iduser = (select iduser from " TBL_UTILISATEURS " where userlogin = '" NOM_ADMINISTRATEURDOCS "')";
    return m_connexionbaseOK;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Détermination du lieu exercice pour la session en cours -------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------- */
void Procedures::CalcLieuExercice()
{
    QList<Site*> listEtab = Datas::I()->sites->initListeByUser(m_currentuser->id());
    if (listEtab.size() == 0)
        return;
    else if (listEtab.size() == 1)
    {
        Datas::I()->sites->setcurrentsite(Datas::I()->sites->sites()->first());
        return;
    }

    /* Cas ou le praticien travaille dans plusieur centres
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
    bool isFirst = true;
    foreach (Site *etab, listEtab)
    {
        UpRadioButton *pradiobutt = new UpRadioButton(boxlieux);
        pradiobutt->setText(etab->nom());
        pradiobutt->setitem(etab);
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
    foreach (UpRadioButton * rb, boxlieux->findChildren<UpRadioButton*>())
        if( rb->isChecked() )
        {
            Datas::I()->sites->setcurrentsite(qobject_cast<Site*>(rb->item()));
            break;
        }
    delete gAskLieux;
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
    QList<QVariantList> usrlist = db->StandardSelectSQL(req, m_ok);
    if (m_ok && usrlist.size()>0)
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
    db->StandardSQL ("insert into " TBL_UTILISATEURS " (idUser, UserNom, UserLogin) values (100, '" NOM_ADMINISTRATEURDOCS "','" NOM_ADMINISTRATEURDOCS "')");

    // On crée l'utilisateur dans la table utilisateurs
    m_idcentre               = 1;
    m_usecotation            = true;
    Datas::I()->banques->initListe();
    CreerUserFactice(1, Login, MDP);
    Datas::I()->users->initListe();
    Datas::I()->comptes->initListe();

    m_currentuser = new User(db->loadUserData(1));
    db->setUserConnected(m_currentuser);
    SetUserAllData(m_currentuser);
    m_currentuser->setidsuperviseur(1);
    m_currentuser->setidusercomptable(1);
    m_currentuser->setidparent(1);

    if (UpMessageBox::Question(Q_NULLPTR, tr("Un compte utilisateur a été cré"),
                               tr("Un compte utilisateur factice a été créé\n") + "\n" +
                               m_currentuser->titre() + " "  + m_currentuser->prenom() + " " + m_currentuser->nom() + ", " + m_currentuser->fonction()
                               + "\n\n" +
                               tr("avec le login ") + Login + " " + tr("et le mot de passe que vous avez fourni") + "\n" +
                               tr("Voulez-vous conserver ces données pour le moment ou les modifier?") + "\n" +
                               tr("Vous pourrez les modifier par la suite\n"),
                               UpDialog::ButtonOK | UpDialog::ButtonEdit, QStringList() << tr("Modifier les données") << tr("Conserver les données"))
        == UpSmallButton::EDITBUTTON)
    {
        int gidLieuExercice = -1;
        Dlg_GestUsr = new dlg_gestionusers(gidLieuExercice, dlg_gestionusers::PREMIERUSER , true);
        Dlg_GestUsr->setWindowTitle(tr("Enregistrement de l'utilisateur ") + Login);
        if (Dlg_GestUsr->exec() > 0)
            SetUserAllData(m_currentuser, Item::Update);
        delete Dlg_GestUsr;
    }
    m_settings->setValue("Param_Poste/VilleParDefaut","Flayat");
    m_settings->setValue("Param_Poste/CodePostalParDefaut","23260");
    m_connexionbaseOK = true;
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
    db->StandardSQL ("insert into " TBL_UTILISATEURS " (idUser, UserLogin, UserMDP) VALUES (" + QString::number(idusr) + ",'" + login + "', '" + mdp + "')");

    int idbanq = 0;
    foreach (Banque* bq, Datas::I()->banques->banques()->values())
        if (bq->nomabrege() == "PaPRS")
        {
            idbanq = bq->id();
            break;
        }
    if (idbanq == 0)
        idbanq = Datas::I()->banques->CreationBanque("PaPRS", "Panama Papers")->id();

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

    QString req  = "insert into " TBL_COMPTES
           " (idBanque, idUser, IBAN, IntituleCompte, NomCompteAbrege, SoldeSurDernierReleve)"
           " VALUES (" + QString::number(idbanq) + "," + QString::number(idusr) + ", '" + iban + "', '" + login + "', 'PaPRS" + QString::number(al) + "', 2333.67)";
    //qDebug() << req;
    db->StandardSQL(req);
    QString idcpt ("");
    req = "select max(idcompte) from " TBL_COMPTES;
    QVariantList cptdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
    if (m_ok && cptdata.size()>0)
        idcpt = cptdata.at(0).toString();

    req = "update " TBL_UTILISATEURS
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
    req = "insert into " TBL_LIEUXEXERCICE "(NomLieu, LieuAdresse1, LieuAdresse2, LieuCodePostal, LieuVille, LieuTelephone)  values ("
            "'Centre ophtalmologique de La Mazière', "
            "'place rouge', "
            "'Bâtiment C', "
            "23260, "
            "'La Mazière', "
            "'O4 56 78 90 12')";
    //Edit(req);
    db->StandardSQL(req);
    req = "select idLieu from " TBL_LIEUXEXERCICE;
    int gidLieuExercice = 0;
    QList<QVariantList> lieuxlist = db->StandardSelectSQL(req, m_ok);
    if (m_ok && lieuxlist.size()>0)
        gidLieuExercice = lieuxlist.at(0).at(0).toInt(); //TODO : ICI
    req = "insert into " TBL_JOINTURESLIEUX " (idUser, idLieu) VALUES(" + QString::number(idusr) + ", " + QString::number(gidLieuExercice) + ")";
    db->StandardSQL(req);
    db->setidlieupardefaut(gidLieuExercice);
}


/*-----------------------------------------------------------------------------------------------------------------
    -- Identification de l'utilisateur -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::IdentificationUser(bool ChgUsr)
{
    dlg_identificationuser *dlg_IdentUser   = new dlg_identificationuser(ChgUsr);
    dlg_IdentUser   ->setFont(m_applicationfont);
    if (m_currentuser != Q_NULLPTR)
        delete m_currentuser;
    m_currentuser = Q_NULLPTR;
    int result = dlg_IdentUser->exec();
    if( result > 0 )
    {
        m_parametres = db->parametres();
        Datas::I()->villes          ->initListe();
        Datas::I()->sites           ->initListe();
        Datas::I()->comptes         ->initListe();
        Datas::I()->users           ->initListe();
        Datas::I()->postesconnectes ->initListe();
        Datas::I()->banques         ->initListe();
        Datas::I()->tierspayants    ->initListe();
        Datas::I()->typestiers      ->initListe();
        Datas::I()->motifs          ->initListe();
        m_currentuser = Datas::I()->users->userconnected();
        SetUserAllData(m_currentuser);
        m_applicationfont = m_currentuser->police();
        qApp->setFont(m_applicationfont);

        if (!VerifBaseEtRessources())
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de mettre à jour la base de données\nSortie du programme"));
            exit(0);
        }
        Verif_secure_file_priv();
        if (DefinitRoleUser()) //NOTE : User Role
        {
            /*! definit les iduser pour lequel le user travaille
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
            m_currentuser->setTypeCompta(m_aveccomptaprovisoire ?
                                         (m_usecotation ? User::COMPTA_AVEC_COTATION_AVEC_COMPTABILITE : User::COMPTA_SANS_COTATION_AVEC_COMPTABILITE)
                                           :
                                         (m_usecotation ? User::COMPTA_AVEC_COTATION_SANS_COMPTABILITE : User::COMPTA_SANS_COTATION_SANS_COMPTABILITE));

            //AFFECT USER:
            //qDebug() << "superviseur " << m_currentuser->getIdUserActeSuperviseur();
            //qDebug() << "comptable " << m_currentuser->getIdUserComptable();
            //qDebug() << "parent " << m_currentuser->getIdUserParent();
            if( m_currentuser->idsuperviseur() > 0 )
                m_currentuser->setsuperviseur(Datas::I()->users->getById(m_currentuser->idsuperviseur()));
            if( m_currentuser->idcomptable() > 0 )
                m_currentuser->setcomptable(Datas::I()->users->getById(m_currentuser->idcomptable()));
            if( m_currentuser->idparent() > 0 )
                m_currentuser->setparent(Datas::I()->users->getById(m_currentuser->idparent()));

            m_idcentre = m_parametres->numcentre();
        }
    }
    else if ( result < 0 ) // anomalie sur la base - table utilisateurs manquante ou corrompue
    {
        m_login         = dlg_IdentUser->ui->LoginlineEdit->text();
        m_password      = dlg_IdentUser->ui->MDPlineEdit->text();
        UpMessageBox    msgbox;
        UpSmallButton   AnnulBouton(tr("Annuler"));
        UpSmallButton   RestaureBaseBouton(tr("Restaurer la base depuis une sauvegarde"));
        UpSmallButton   BaseViergeBouton(tr("Nouvelle base patients vierge"));
        msgbox.setText(tr("Base de données endommagée!"));
        msgbox.setInformativeText(tr("La base de données semble endommagée.\n"
                                  "Voulez-vous la reconstruire à partir"
                                  " d'une sauvegarde ou recréer une base vierge?\n\n"));
        msgbox.setIcon(UpMessageBox::Info);
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&BaseViergeBouton, UpSmallButton::STARTBUTTON);
        msgbox.addButton(&RestaureBaseBouton, UpSmallButton::COPYBUTTON);
        msgbox.exec();
        if( (msgbox.clickedButton() == &RestaureBaseBouton))
        {
            if (RestaureBase(false,true,false))
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
                Datas::I()->postesconnectes->SupprimeAllPostesConnectes();

                exit(0);
            }
        }
        else if (msgbox.clickedButton() == &BaseViergeBouton)
        {
            Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES);
            if (!RestaureBase(true, true))
                exit(0);
            // Création de l'utilisateur
            //TODO : ICI
            m_connexionbaseOK = CreerPremierUser(m_login, m_password);
            Datas::I()->users->initListe();
            UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer"), tr("Relancez-le pour que certaines données puissent être prises en compte"));
            Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
            exit(0);
        }
    }
    delete dlg_IdentUser;
    return (m_currentuser != Q_NULLPTR);
}

bool Procedures::DefinitRoleUser() //NOTE : User Role Function
{
    if (m_currentuser->isSoignant() )
    {
        QString req;
        dlg_askUser                = new UpDialog();
        dlg_askUser                ->AjouteLayButtons();
        dlg_askUser                ->setAccessibleName(QString::number(m_currentuser->id()));
        dlg_askUser->setdata(m_currentuser);
        QVBoxLayout *boxlay     = new QVBoxLayout;
        dlg_askUser->dlglayout()   ->insertLayout(0,boxlay);

        QGroupBox *boxrole      = new QGroupBox(dlg_askUser);
        boxrole                 ->setAccessibleName("Role");
        QString lblRole         = tr("Quel est votre rôle dans cette session?");
        boxrole                 ->setTitle(lblRole);
        boxrole                 ->setVisible(false);
        boxlay                  ->addWidget(boxrole);

        QGroupBox *boxsuperv    = new QGroupBox(dlg_askUser);
        boxsuperv               ->setAccessibleName("Superv");
        QString lblSuperv       = tr("Qui supervise votre activité pour cette session?");
        boxsuperv               ->setTitle(lblSuperv);
        boxsuperv               ->setVisible(false);
        boxlay                  ->addWidget(boxsuperv);

        QGroupBox *boxparent     = new QGroupBox(dlg_askUser);
        boxparent               ->setAccessibleName("Parent");
        QString lblUsrParent    = tr("Qui enregistre les honoraires de vos actes?");
        boxparent               ->setTitle(lblUsrParent);
        boxparent               ->setVisible(false);
        boxlay                  ->addWidget(boxparent);

        // le user est responsable de ses actes - on cherche à savoir qui comptabilise ses actes
        if( m_currentuser->isResponsable() )
            CalcUserParent();

        // le user alterne entre responsable des actes et assistant suivant la session
        // on lui demande son rôle pour cette session
        else if( m_currentuser->isResponsableOuAssistant() )
        {
            bool found = false;
            foreach (User *us, Datas::I()->users->all()->values())
            {
                if( us->id() == m_currentuser->id() )
                    continue;
                if( !us->isResponsable() && !us->isResponsableOuAssistant() )
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
                connect(pbuttResp, &QRadioButton::clicked, this, &Procedures::CalcUserParent);

                QRadioButton *pbuttAss  = new QRadioButton(boxrole);
                pbuttAss                ->setText(tr("Assistant"));
                pbuttAss                ->setAccessibleName("buttass");
                pbuttAss                ->setChecked(true);      // le user est défini par défaut comme assistant -> on cherche qui supervise les actes
                vbox                    ->addWidget(pbuttAss);
                connect(pbuttAss, &QRadioButton::clicked, this, &Procedures::CalcUserSuperviseur);

                vbox                    ->setContentsMargins(8,0,8,0);
                boxrole                 ->setLayout(vbox);
                dynamic_cast<QVBoxLayout*>(dlg_askUser->layout())->setSizeConstraint(QLayout::SetFixedSize);
                CalcUserSuperviseur();
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
                CalcUserParent();
            }
        }

        // le user est assistant - on lui demande qui supervise ses actes
        else if( m_currentuser->isAssistant() )
            CalcUserSuperviseur();

        dlg_askUser                ->setModal(true);
        dlg_askUser->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
        connect(dlg_askUser->OKButton, &QPushButton::clicked, dlg_askUser, &UpDialog::accept);

        if( m_currentuser->idsuperviseur() == User::ROLE_INDETERMINE
                || m_currentuser->idparent() == User::ROLE_INDETERMINE )
        {
            if( dlg_askUser->exec() == 0 )
            {
                delete dlg_askUser;
                return false;
            }
            foreach (QGroupBox *groupBox, dlg_askUser->findChildren<QGroupBox*>())
            {
                if( !groupBox->isVisibleTo(dlg_askUser) )
                    continue;
                if (groupBox->accessibleName() == "Superv" )
                {
                    QList<QRadioButton*> listbutt = groupBox->findChildren<QRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                        {
                            m_currentuser->setidsuperviseur(listbutt.at(j)->accessibleName().toInt());
                            break;
                        }
                }
                else if (groupBox->accessibleName() == "Parent" )
                {
                    QList<QRadioButton*> listbutt = groupBox->findChildren<QRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                        {
                            m_currentuser->setidparent(listbutt.at(j)->accessibleName().toInt());
                            break;
                        }
                }
            }
            delete dlg_askUser;
        }
        if( m_currentuser->idsuperviseur() == User::ROLE_INDETERMINE )
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Aucun superviseur valide n'a été défini pour vos actes"), tr("Impossible de continuer"));
            return false;
        }

        if( m_currentuser->idsuperviseur() == User::ROLE_NON_RENSEIGNE )
        {
            // le user est assistant et travaille pour tout le monde
            m_currentuser->setidparent(User::ROLE_NON_RENSEIGNE);
            m_currentuser->setidusercomptable(User::ROLE_NON_RENSEIGNE);

            m_aveccomptaprovisoire = true;
            m_usecotation     = true;
        }
        else
        {
            // determination de comptabilité - cotation
            if( m_currentuser->idparent() == User::ROLE_INDETERMINE )
            {
                if( Datas::I()->users->getById( m_currentuser->idsuperviseur()) != Q_NULLPTR
                 && Datas::I()->users->getById( m_currentuser->idsuperviseur())->isRemplacant() )
                {
                    // le superviseur est remplaçant, on essaie de savoir s'il a un parent
                    QList<User*> listUserFound;
                    foreach (User *us, Datas::I()->users->all()->values())
                    {
                        if( us->id() == m_currentuser->id() )
                            continue;
                        if( us->id() == m_currentuser->idsuperviseur() )
                            continue;
                        if( !us->isLiberal() && !us->isSalarie() )
                            continue;

                        listUserFound << us;
                    }
                    if (listUserFound.size() == 1)
                        m_currentuser->setidparent( listUserFound.first()->id() );
                    else if( !listUserFound.isEmpty() )
                    {
                        // on va demander qui est le soignant parent de ce remplaçant....
                        dlg_askUser                = new UpDialog();
                        dlg_askUser                ->AjouteLayButtons();
                        dlg_askUser                ->setAccessibleName(QString::number(m_currentuser->idsuperviseur()));
                        dlg_askUser->setdata(Datas::I()->users->getById( m_currentuser->idsuperviseur()));
                        QVBoxLayout *boxlay     = new QVBoxLayout;
                        dlg_askUser->dlglayout()   ->insertLayout(0,boxlay);
                        QGroupBox*boxparent     = new QGroupBox(dlg_askUser);
                        boxparent               ->setAccessibleName("Parent");
                        QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + Datas::I()->users->getById(m_currentuser->idsuperviseur())->login() + "?";
                        boxparent               ->setTitle(lblUsrParent);
                        boxparent               ->setVisible(false);
                        boxlay                  ->addWidget(boxparent);

                        CalcUserParent();
                        dlg_askUser                ->setModal(true);
                        dlg_askUser->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
                        connect(dlg_askUser->OKButton,   SIGNAL(clicked(bool)),  dlg_askUser, SLOT(accept()));
                        if (dlg_askUser->exec()==0)
                        {
                            delete dlg_askUser;
                            return false;
                        }
                        else
                        {
                            foreach (QGroupBox *box, dlg_askUser->findChildren<QGroupBox*>())
                                if (box->accessibleName() == "Parent")
                                    foreach (QRadioButton *butt, box->findChildren<QRadioButton*>())
                                        if (butt->isChecked())
                                        {
                                            //gidUserParentProv = butt->accessibleName().toInt();
                                            m_currentuser->setidparent( butt->accessibleName().toInt() );
                                            break;
                                        }
                            delete dlg_askUser;
                        }
                    }
                }
                else
                    m_currentuser->setidparent( m_currentuser->idsuperviseur() );
            }
            if( Datas::I()->users->getById(m_currentuser->idparent()) != Q_NULLPTR )
            {
                // determination de l'utilisation de la cotation
                m_usecotation = Datas::I()->users->getById(m_currentuser->idparent())->useCCAM();
                // determination de l'utilisation de la comptabilité
                m_aveccomptaprovisoire = !Datas::I()->users->getById(m_currentuser->idparent())->isSansCompta();
                if( Datas::I()->users->getById(m_currentuser->idparent())->isLiberal() )
                    m_currentuser->setidusercomptable(Datas::I()->users->getById(m_currentuser->idparent())->id());
                else if( Datas::I()->users->getById(m_currentuser->idparent())->isSalarie() )
                    m_currentuser->setidusercomptable(Datas::I()->users->getById(m_currentuser->idparent())->idemployeur());
                else
                    m_currentuser->setidusercomptable(User::ROLE_NON_RENSEIGNE);
            }
        }
        return true;
    }

    // il s'agit d'un administratif ou d'une société comptable
    m_currentuser->setidsuperviseur(User::ROLE_VIDE);
    m_currentuser->setidusercomptable(User::ROLE_VIDE);
    m_currentuser->setidparent(User::ROLE_VIDE);
    m_usecotation     = true;
    m_aveccomptaprovisoire = true; //FIXME : avecLaComptaProv
    return true;
}

/*!
 * \brief Procedures::SetUserAllData(User *usr)
 * Charge les données d'un utilisateur, y compris ses données bancaires
 * cette fonction fait appel aux deux classes cls_user et cls_compte
 * et ne peut pas figurer dans la classe cls_user
 * en raison de référence croisées
 */
bool Procedures::SetUserAllData(User *usr, Item::UPDATE upd)
{
    if (!usr->isAllLoaded() || upd == Item::Update)
    {
        QJsonObject data = db->loadUserData(usr->id());
        if(data.isEmpty())
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Les paramètres de ")
                                + usr->login() + tr(" ne sont pas retrouvés"));
            return false;
        }
        usr->setData( data ); //on charge le reste des données
    }
    usr->setlistecomptesbancaires(Datas::I()->comptes->initListeComptesByIdUser(usr->id()));
    if (usr->isSalarie())
        usr->setidcompteencaissementhonoraires(Datas::I()->users->getById(usr->idemployeur())->idcompteencaissementhonoraires());
    return true;
}

/*!
 * \brief Procedures::CalcUserSuperviseur
 *
 * Prépare le UpDialog en ajoutant si besoin une liste d'User
 * susceptible d'être le Parent
 *
 * Résultat :
 * gidUserParentProv = -3
 * gidUserSuperViseurProv = -3 || id
 */
void Procedures::CalcUserSuperviseur()
{
    User *user = qobject_cast<User *>(dlg_askUser->data());
    m_currentuser->setidsuperviseur(User::ROLE_INDETERMINE);
    m_currentuser->setidparent(User::ROLE_INDETERMINE);
    QGroupBox *ptbox = Q_NULLPTR;
    QList<QGroupBox*> Listgroupbx   = dlg_askUser->findChildren<QGroupBox*>();
    for (int i=0; i<Listgroupbx.size(); i++)
    {
        if (Listgroupbx.at(i)->accessibleName() == "Superv")
            ptbox = Listgroupbx.at(i);
        else if (Listgroupbx.at(i)->accessibleName() == "Parent")
            Listgroupbx.at(i)->setVisible(false);
    }
    ptbox->setVisible(false);

    QList<QRadioButton*> listbutt = ptbox->findChildren<QRadioButton*>();
    foreach (QRadioButton * rb, listbutt)
        delete rb;
    delete ptbox->layout();

    QList<User*> listUserFound;
    foreach (User *us, Datas::I()->users->superviseurs()->values())
    {
        if( us->id() == user->id() )
            continue;
        if( m_currentuser->isMedecin() && !us->isMedecin() )
            continue;
        listUserFound << us;
    }

    if( listUserFound.size() == 1 )
    {
        m_currentuser->setidsuperviseur( listUserFound.first()->id() );
    }
    else if( !listUserFound.isEmpty() )
    {
        ptbox->setVisible( true );

        QFontMetrics fm         = QFontMetrics(qApp->font());
        int hauteurligne        = int(fm.height()*1.6);
        ptbox                   ->setFixedHeight(((listUserFound.size() + 2)*hauteurligne)+5);
        QVBoxLayout *vbox       = new QVBoxLayout;
        bool isFirst = true;
        foreach (User *us, listUserFound)
        {
            QRadioButton *pradiobutt = new QRadioButton(ptbox);
            pradiobutt->setText(us->login());
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
    dlg_askUser->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
}

/*!
 * \brief Procedures::CalcUserParent
 * Prépare le UpDialog en ajoutant si besoin une liste d'User
 * susceptibles d'être le Parent
 *
 * Résultat :
 * gidUserSuperViseurProv = user->id()
 * gidUserParentProv = id || -3 || user->id()
 */
void Procedures::CalcUserParent()
{
    User *user = qobject_cast<User *>(dlg_askUser->data());
    //gidUserSuperViseurProv = user->id();
    user->setidsuperviseur( user->id() );
    QGroupBox *ptbox = Q_NULLPTR;
    foreach (QGroupBox * box, dlg_askUser->findChildren<QGroupBox*>())
    {
        if (box->accessibleName() == "Superv")
            box->setVisible(false);
        else if (box->accessibleName() == "Parent")
            ptbox = box;
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
        foreach (User *us, Datas::I()->users->all()->values() )
        {
            if( us->id() == user->id() )
                continue;
            if( !us->isLiberal() && !us->isSalarie() )
                continue;
            if( us->metier() != user->metier() )
                continue;
            listUserFound << us;
        }

        if( listUserFound.size() == 1 )
        {
            //gidUserParentProv = listUserFound.first()->id();
            user->setidparent( listUserFound.first()->id() );
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
            foreach (User *us, listUserFound)
            {
                QRadioButton *pradiobutt = new QRadioButton(ptbox);
                pradiobutt->setText(us->login());
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
        user->setidparent( user->id() );
    }
    dlg_askUser->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
}

int Procedures::idCentre()
{
    return m_idcentre;
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


    // Création des dossiers
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_RESSOURCES);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_IMAGES);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_ECHECSTRANSFERTS);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_DOSSIERECHANGE);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_VIDEOS);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_PROV);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_FACTURESSANSLIEN);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_FACTURES);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_ORIGINAUX DIR_FACTURES);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE DIR_ORIGINAUX DIR_IMAGES);
    m_settings    = new QSettings(m_nomFichierIni, QSettings::IniFormat);
    QString login (""), MDP("");
    if (protoc == BaseExistante)
    {
        if (VerifParamConnexion(login, MDP))
        {
            Datas::I()->banques->initListe();
            Datas::I()->users->initListe();
            Datas::I()->comptes->initListe();
            m_parametres = db->parametres();
            PremierParametrageMateriel();
            PremierParametrageRessources();
            Datas::I()->sites->initListe();
            db->login(login, MDP);
            m_currentuser = db->userConnected();
            SetUserAllData(m_currentuser);
            CalcLieuExercice();
            if (Datas::I()->sites->currentsite() == Q_NULLPTR)
                UpMessageBox::Watch(Q_NULLPTR,tr("Pas d'adresse spécifiée"), tr("Vous n'avez précisé aucun lieu d'exercice!"));
            Datas::I()->users->initListe();
            m_connexionbaseOK = (m_currentuser != Q_NULLPTR);
            if (!m_connexionbaseOK)
                return false;
            //gidUser     = idusr; //TODO : ICI
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne,\n"
                                       "le login ") + m_currentuser->login() + tr(" est reconnu") + ".\n" +
                                       tr("Le programme va se fermer pour que les modifications") + "\n" +
                                       tr("puissent être prises en compte\n"));
            exit(0);
        }
    }

    else if (protoc == BaseRestauree)
    {
        bool SansAccesDistant = false;
        if (VerifParamConnexion(login, MDP, SansAccesDistant))
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Connexion réussie"),
                                  tr("Bien, la connexion au serveur MySQL fonctionne,\n"));
            // Restauration de la base
            m_login = login;
            m_password = MDP;
            if (!RestaureBase(false, true, false))
                return false;
            if (QMessageBox::question(Q_NULLPTR,"", tr("Reinitialiser les fichiers de paramétrage du matériel et d'impression?")) == QMessageBox::Yes)
            {
                PremierParametrageMateriel();
                PremierParametrageRessources();
            }
            db->login(login, MDP);
            m_currentuser = db->userConnected();
            m_connexionbaseOK = (m_currentuser != Q_NULLPTR);
            if (!m_connexionbaseOK)
                return false;
            //gidUser     = idusr; //TODO : ICI
            UpMessageBox::Watch(Q_NULLPTR, tr("Redémarrage nécessaire"),
                                   tr("Le programme va se fermer pour que les modifications de la base Rufus\n"
                                      "puissent être prises en compte\n"));
            Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
            exit(0);
        }
    }
    else if (protoc == BaseVierge)
    {
        bool SansAccesDistant = false;
        if (VerifParamConnexion(login, MDP, SansAccesDistant))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne "
                                       "et le programme va maintenant créer une base de données patients "
                                       "vierge de tout enregistrement."));
            // Création de la base
             if (!RestaureBase(true, true))
                return false;
            if (m_modeacces == Utils::ReseauLocal)
                db->setadresseserveurlocal(m_settings->value(Utils::getBaseFromMode(m_modeacces) + "/Serveur").toString());
            m_parametres = db->parametres();

            // Création de l'utilisateur
            m_connexionbaseOK = CreerPremierUser(login, MDP);
            db->login(m_currentuser->login(), m_currentuser->password());
            SetUserAllData(m_currentuser);
            Datas::I()->sites->initListe();
            CalcLieuExercice();
            if (Datas::I()->sites->currentsite() == Q_NULLPTR)
                UpMessageBox::Watch(Q_NULLPTR,tr("Pas d'adresse spécifiée"), tr("Vous n'avez précisé aucun lieu d'exercice!"));
            Datas::I()->users->initListe();
            UpMessageBox::Watch(Q_NULLPTR, tr("Redémarrage nécessaire"),
                                   tr("Le programme va se fermer pour que les modifications de la base Rufus\n"
                                      "puissent être prises en compte\n"));
            Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
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
    m_settings->setValue("Param_Imprimante/TailleEnTete","45");
    m_settings->setValue("Param_Imprimante/TailleEnTeteALD","63");
    m_settings->setValue("Param_Imprimante/TaillePieddePage","20");
    m_settings->setValue("Param_Imprimante/TaillePieddePageOrdoLunettes","40");
    m_settings->setValue("Param_Imprimante/TailleTopMarge","3");
    m_settings->setValue("Param_Imprimante/ApercuAvantImpression","NO");
    m_settings->setValue("PyxInterf/PyxvitalPath", QDir::homePath() + "/Documents/Pyxvital");
    m_settings->setValue("Param_Poste/Autoref","-");
    m_settings->setValue("Param_Poste/Refracteur","-");
    m_settings->setValue("Param_Poste/Fronto","-");
    m_settings->setValue("Param_Poste/Tonometre","-");
    m_settings->setValue("Param_Poste/PortAutoref","-");
    m_settings->setValue("Param_Poste/PortRefracteur","-");
    m_settings->setValue("Param_Poste/PortFronto","-");
    m_settings->setValue("Param_Poste/PortTonometre","-");
    m_settings->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + "/PrioritaireGestionDocs","NO");
    m_settings->setValue("Param_Poste/VersionRessources", VERSION_RESSOURCES);
    Utils::mkpath(QDir::homePath() + DIR_RUFUS DIR_IMAGERIE);
    QString NomDirImg = QDir::homePath() + DIR_RUFUS DIR_IMAGERIE;
    db->setdirimagerie(NomDirImg);
    m_settings->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierImagerie", NomDirImg);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Création des fichiers ressources ---------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Procedures::PremierParametrageRessources()
{
    Utils::mkpath(QDir::homePath() + DIR_RUFUS);
    QString NomDirRessrces = QDir::homePath() + DIR_RUFUS DIR_RESSOURCES;
    QDir DirRessrces(NomDirRessrces);
    if (DirRessrces.exists())
        DirRessrces.rmdir(NomDirRessrces);
    DirRessrces.mkpath(NomDirRessrces);
    QFile COACopier(QStringLiteral("://Corps_Ordonnance.txt"));
    COACopier.copy(QDir::homePath() + FILE_CORPSORDO);
    QFile CO(QDir::homePath() + FILE_CORPSORDO);
    CO.open(QIODevice::ReadWrite);
    CO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile COALDACopier(QStringLiteral("://Corps_OrdoALD.txt"));
    COALDACopier.copy(QDir::homePath() + FILE_CORPSORDOALD);
    QFile COALD(QDir::homePath() + FILE_CORPSORDOALD);
    COALD.open(QIODevice::ReadWrite);
    COALD.setPermissions(QFileDevice::ReadOther     | QFileDevice::WriteOther
                         | QFileDevice::ReadGroup   | QFileDevice::WriteGroup
                         | QFileDevice::ReadOwner   | QFileDevice::WriteOwner
                         | QFileDevice::ReadUser    | QFileDevice::WriteUser);

    QFile EOACopier(QStringLiteral("://Entete_Ordonnance.txt"));
    EOACopier.copy(QDir::homePath() + FILE_ENTETEORDO);
    QFile EO(QDir::homePath() + FILE_ENTETEORDO);
    EO.open(QIODevice::ReadWrite);
    EO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile EOALDACopier(QStringLiteral("://Entete_OrdoALD.txt"));
    EOALDACopier.copy(QDir::homePath() + FILE_ENTETEORDOALD);
    QFile EOALD(QDir::homePath() + FILE_ENTETEORDOALD);
    EOALD.open(QIODevice::ReadWrite);
    EOALD.setPermissions(QFileDevice::ReadOther     | QFileDevice::WriteOther
                         | QFileDevice::ReadGroup   | QFileDevice::WriteGroup
                         | QFileDevice::ReadOwner   | QFileDevice::WriteOwner
                         | QFileDevice::ReadUser    | QFileDevice::WriteUser);

    QFile POLACopier(QStringLiteral("://Pied_Ordonnance_Lunettes.txt"));
    POLACopier.copy(QDir::homePath() + FILE_PIEDORDOLUNETTES);
    QFile POL(QDir::homePath() + FILE_PIEDORDOLUNETTES);
    POL.open(QIODevice::ReadWrite);
    POL.setPermissions(QFileDevice::ReadOther   | QFileDevice::WriteOther
                       | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                       | QFileDevice::ReadOwner | QFileDevice::WriteOwner
                       | QFileDevice::ReadUser  | QFileDevice::WriteUser);

    QFile POACopier(QStringLiteral("://Pied_Ordonnance.txt"));
    POACopier.copy(QDir::homePath() + FILE_PIEDPAGE);
    QFile PO(QDir::homePath() + FILE_PIEDPAGE);
    PO.open(QIODevice::ReadWrite);
    PO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile PDFACopier(QStringLiteral("://pdf.pdf"));
    PDFACopier.copy(QDir::homePath() + FILE_PDF);
    QFile pdf(QDir::homePath() + FILE_PDF);
    pdf.open(QIODevice::ReadWrite);
    pdf.setPermissions(QFileDevice::ReadOther   | QFileDevice::WriteOther
                       | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                       | QFileDevice::ReadOwner | QFileDevice::WriteOwner
                       | QFileDevice::ReadUser  | QFileDevice::WriteUser);
    m_settings->setValue("Param_Poste/VersionRessources",VERSION_RESSOURCES);
    if (m_modeacces == Utils::Poste)
    {
        QString NomDirImg = QDir::homePath() + DIR_RUFUS DIR_IMAGERIE;
        if (db->getDataBase().isOpen())
            db->setdirimagerie(NomDirImg);
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
            QFile::remove(m_nomFichierIni);
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
            QFile FichierIni(m_nomFichierIni);
            if (FichierIni.exists())
                FichierIni.remove();
            QString fileini = dialog.selectedFiles().at(0);
            QFile rufusini(fileini);
            rufusini.copy(m_nomFichierIni);
            m_settings    = new QSettings(m_nomFichierIni, QSettings::IniFormat);
            if (QMessageBox::question(Q_NULLPTR,"", tr("Restaurer aussi les fichiers modèles d'impression?")) == QMessageBox::Yes)
                PremierParametrageRessources();
            reponse = true;
        }
    }
    else if (msgbox->clickedButton()==&ReconstruitIniBouton)
    {
        //reconstruire le fichier rufus.ini
        //1. on demande les paramètres de connexion au serveur - mode d'accès / user / mdp / port / SSL
        QFile(m_nomFichierIni).remove();
        m_settings    = new QSettings(m_nomFichierIni, QSettings::IniFormat);
        QString login(""), MDP ("");
        reponse = VerifParamConnexion(login, MDP);
        if (reponse)
        {
            db->login(login, MDP);
            m_currentuser = db->userConnected();
            int idusr = VerifUserBase(m_currentuser->login(), m_currentuser->password());
            m_connexionbaseOK = (idusr > -1);
            if (!m_connexionbaseOK)
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
        //reconstruire la base de données à partir d'un dump
        reponse = RestaureBase(false, false, false);
        if (reponse)
            UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
        else
            UpMessageBox::Watch(Q_NULLPTR,tr("Restauration impossible de la base"));
        Datas::I()->postesconnectes->SupprimeAllPostesConnectes();

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
        Datas::I()->postesconnectes->SupprimeAllPostesConnectes();

        exit(0);
    }
    return reponse;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Vérifie et répare les paramètres de connexion  -----------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
//???
bool Procedures::VerifParamConnexion(QString &login, QString &MDP, bool OKAccesDistant, QString)
{
    Dlg_ParamConnex = new dlg_paramconnexion(OKAccesDistant);
    Dlg_ParamConnex ->setWindowTitle(tr("Entrez les paramètres de connexion au serveur"));
    Dlg_ParamConnex ->setFont(m_applicationfont);

    if (Dlg_ParamConnex->exec()>0)
    {
        QString Base;
        if (Dlg_ParamConnex->ui->PosteradioButton->isChecked())
        {
            Base = Utils::getBaseFromMode(Utils::Poste);
            m_modeacces = Utils::Poste;
        }
        else if (Dlg_ParamConnex->ui->LocalradioButton->isChecked())
        {
            Base = Utils::getBaseFromMode(Utils::ReseauLocal);
            m_settings->setValue(Base + "/Serveur",   Dlg_ParamConnex->ui->IPlineEdit->text());
            m_modeacces = Utils::ReseauLocal;
        }
        else if (Dlg_ParamConnex->ui->DistantradioButton->isChecked())
        {
            Base = Utils::getBaseFromMode(Utils::Distant);
            m_settings->setValue(Base + "/Serveur",   Dlg_ParamConnex->ui->IPlineEdit->text());
            m_modeacces = Utils::Distant;
        }
        m_settings->setValue(Base + "/Active",    "YES");
        m_settings->setValue(Base + "/Port", Dlg_ParamConnex->ui->PortcomboBox->currentText());

        m_connexionbaseOK = true;
        MDP = Dlg_ParamConnex->ui->MDPlineEdit->text();
        login = Dlg_ParamConnex->ui->LoginlineEdit->text();
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
            foreach (const QString &nomfic, dockdir.entryList())
            {
                QFile ficACopier(dockdir.absolutePath() + "/" + nomfic);
                QString nomficACopier = QFileInfo(nomfic).fileName();
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
    QString req = "SHOW TABLES FROM " DB_CONSULTS " LIKE 'utilisateurs'";
    QVariantList verifbasedata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
    if (!m_ok || verifbasedata.size()==0)
    {
        msgbox.setText(tr("Erreur sur la base patients"));
        msgbox.setInformativeText(tr("La connexion au serveur fonctionne mais "
                                  "votre base de données semble endommagée.\n"
                                  "La table des utilisateurs n'existe pas.\n"
                                  "Impossible de continuer."));
        msgbox.exec();
        return -2;
    }
    req =   "SELECT idUser FROM " TBL_UTILISATEURS
            " WHERE UserLogin = '" + Utils::correctquoteSQL(Login) +
            "' AND UserMDP = '" + Utils::correctquoteSQL(MDP) + "'" ;
    QVariantList idusrdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
    if (!m_ok || idusrdata.size()==0)
    {
        req =   "SELECT UserLogin FROM " TBL_UTILISATEURS;
        QList<QVariantList> usrlist = db->StandardSelectSQL(req, m_ok);
        if (!m_ok || usrlist.size()==0)
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

/* ------------------------------------------------------------------------------------------------------------------------------------------


GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------


------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Ports_Series()
{
    QString NomPort             = "";
    m_portRefracteur            = "";
    m_portFronto                = "";
    m_portAutoref               = "";
    m_portTono                  = "";
    sp_portRefracteur           = Q_NULLPTR;
    sp_portFronto               = Q_NULLPTR;
    sp_portAutoref              = Q_NULLPTR;
    sp_portTono                 = Q_NULLPTR;
    bool m_isFrontoParametre    = (m_settings->value("Param_Poste/Fronto").toString() != "-"
                                && m_settings->value("Param_Poste/Fronto").toString() != ""
                                && m_settings->value("Param_Poste/PortFronto").toString() != "Box");
    bool m_isAutorefParametre   = (m_settings->value("Param_Poste/Autoref").toString() != "-"
                                && m_settings->value("Param_Poste/Autoref").toString() != ""
                                && m_settings->value("Param_Poste/PortAutoref").toString() != "Box");
    bool m_isRefracteurParametre= (m_settings->value("Param_Poste/Refracteur").toString() != "-"
                                && m_settings->value("Param_Poste/Refracteur").toString() != "");
    bool m_isTonoParametre      = (m_settings->value("Param_Poste/Tonometre").toString() != "-"
                                && m_settings->value("Param_Poste/Tonometre").toString() != "");
    Datas::I()->mesureautoref   ->settypemesure(Refraction::Autoref);
    Datas::I()->mesurefronto    ->settypemesure(Refraction::Fronto);
    Datas::I()->mesurefinal     ->settypemesure(Refraction::Prescription);
    Datas::I()->mesureacuite    ->settypemesure(Refraction::Acuite);


    if (m_isAutorefParametre || m_isRefracteurParametre || m_isFrontoParametre || m_isTonoParametre)
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
    if (m_isFrontoParametre)
    {
        m_portFronto     = m_settings->value("Param_Poste/PortFronto").toString();
        bool a          = ReglePortFronto();
        a               = (m_portFronto != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion frontofocomètre"));
        for (int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
        {
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
            {
                if (m_portFronto == "COM1") NomPort = "A";
                else if (m_portFronto == "COM2") NomPort = "B";
                else if (m_portFronto == "COM3") NomPort = "C";
                else if (m_portFronto == "COM4") NomPort = "D";
                if (NomPort != "") break;
            }
            else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
            {
                if (m_portFronto == "COM1") NomPort = "ttyUSB0";
                else if (m_portFronto == "COM2") NomPort = "ttyUSB1";
                else if (m_portFronto == "COM3") NomPort = "ttyUSB2";
                else if (m_portFronto == "COM4") NomPort = "ttyUSB3";
                if (NomPort != "") break;
            }
        }

        if (NomPort != "")
        {
            sp_portFronto     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                //Debug() << QSerialPortInfo::availablePorts().at(i).portName();
                //UpMessageBox::Watch(this,QSerialPortInfo::availablePorts().at(i).portName());
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
                {
                    QString letter = QSerialPortInfo::availablePorts().at(i).portName().split("-").at(1);
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort || letter.left(1) == NomPort)
                    {
                        sp_portFronto->setPort(QSerialPortInfo::availablePorts().at(i));
                        sp_portFronto->setBaudRate(s_paramPortSerieFronto.baudRate);
                        sp_portFronto->setFlowControl(s_paramPortSerieFronto.flowControl);
                        sp_portFronto->setParity(s_paramPortSerieFronto.parity);
                        sp_portFronto->setDataBits(s_paramPortSerieFronto.dataBits);
                        sp_portFronto->setStopBits(s_paramPortSerieFronto.stopBits);
                        break;
                    }
                }
                else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName() == NomPort)
                    {
                        sp_portFronto->setPort(QSerialPortInfo::availablePorts().at(i));
                        sp_portFronto->setBaudRate(s_paramPortSerieFronto.baudRate);
                        sp_portFronto->setFlowControl(s_paramPortSerieFronto.flowControl);
                        sp_portFronto->setParity(s_paramPortSerieFronto.parity);
                        sp_portFronto->setDataBits(s_paramPortSerieFronto.dataBits);
                        sp_portFronto->setStopBits(s_paramPortSerieFronto.stopBits);
                        break;
                    }
                }
            }
            //qDebug() << "FRONTO -> " + m_portFronto + " - " + NomPort;
            if (sp_portFronto->open(QIODevice::ReadWrite))
            {
                t_threadFronto = new SerialThread(sp_portFronto);
                t_threadFronto->transaction();
                connect(t_threadFronto,  &SerialThread::reponse,     this, &Procedures::ReponsePortSerie_Fronto);
            }
            else
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Connexion impossible"),tr("Impossible de connecter le frontofocomètre") + "\n" + sp_portFronto->errorString());
                delete sp_portFronto;
                sp_portFronto = Q_NULLPTR;
            }
        }
    }

    // PORT REFRACTEUR
    if (m_isRefracteurParametre)
    {
        m_portRefracteur = m_settings->value("Param_Poste/PortRefracteur").toString();
        bool a          = ReglePortRefracteur();
        a               = (m_portRefracteur != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion refracteur"));
        for (int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
        {
            //qDebug() << QSerialPortInfo::availablePorts().at(i).portName();
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
            {
                if (m_portRefracteur == "COM1") NomPort = "A";
                else if (m_portRefracteur == "COM2") NomPort = "B";
                else if (m_portRefracteur == "COM3") NomPort = "C";
                else if (m_portRefracteur == "COM4") NomPort = "D";
                if (NomPort != "") break;
            }
            else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
            {
                if (m_portRefracteur == "COM1") NomPort = "ttyUSB0";
                else if (m_portRefracteur == "COM2") NomPort = "ttyUSB1";
                else if (m_portRefracteur == "COM3") NomPort = "ttyUSB2";
                else if (m_portRefracteur == "COM4") NomPort = "ttyUSB3";
                if (NomPort != "") break;
            }
        }
        if (NomPort != "")
        {
            sp_portRefracteur     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
                {
                    QString letter = QSerialPortInfo::availablePorts().at(i).portName().split("-").at(1);
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort || letter.left(1) == NomPort)
                    {
                        sp_portRefracteur->setPort(QSerialPortInfo::availablePorts().at(i));
                        sp_portRefracteur->setBaudRate(s_paramPortSerieRefracteur.baudRate);
                        sp_portRefracteur->setFlowControl(s_paramPortSerieRefracteur.flowControl);
                        sp_portRefracteur->setParity(s_paramPortSerieRefracteur.parity);
                        sp_portRefracteur->setDataBits(s_paramPortSerieRefracteur.dataBits);
                        sp_portRefracteur->setStopBits(s_paramPortSerieRefracteur.stopBits);
                        break;
                    }
                }
                else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName() == NomPort)
                    {
                        sp_portRefracteur->setPort(QSerialPortInfo::availablePorts().at(i));
                        sp_portRefracteur->setBaudRate(s_paramPortSerieRefracteur.baudRate);
                        sp_portRefracteur->setFlowControl(s_paramPortSerieRefracteur.flowControl);
                        sp_portRefracteur->setParity(s_paramPortSerieRefracteur.parity);
                        sp_portRefracteur->setDataBits(s_paramPortSerieRefracteur.dataBits);
                        sp_portRefracteur->setStopBits(s_paramPortSerieRefracteur.stopBits);
                        break;
                    }
                }
            }
            //qDebug() << "REFRACTEUR -> " + m_portRefracteur + " - " + NomPort;
            if (sp_portRefracteur->open(QIODevice::ReadWrite))
            {
                t_threadRefracteur     = new SerialThread(sp_portRefracteur);
                t_threadRefracteur    ->transaction();
                connect(t_threadRefracteur,  &SerialThread::reponse,     this, &Procedures::ReponsePortSerie_Refracteur);
            }
            else
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Connexion impossible"),tr("Impossible de connecter le refracteur") + "\n" + sp_portRefracteur->errorString());
                delete sp_portRefracteur;
                sp_portRefracteur = Q_NULLPTR;
            }
        }
    }

    //PORT AUTOREF
    if (m_isAutorefParametre)
    {
        m_portAutoref    = m_settings->value("Param_Poste/PortAutoref").toString();
        bool a          = ReglePortAutoref();
        a               = (m_portAutoref != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion autorefractomètre"));
        for (int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
        {
            if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
            {
                if (m_portAutoref == "COM1") NomPort = "A";
                else if (m_portAutoref == "COM2") NomPort = "B";
                else if (m_portAutoref == "COM3") NomPort = "C";
                else if (m_portAutoref == "COM4") NomPort = "D";
                if (NomPort != "") break;
            }
            else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
            {
                if (m_portAutoref == "COM1") NomPort = "ttyUSB0";
                else if (m_portAutoref == "COM2") NomPort = "ttyUSB1";
                else if (m_portAutoref == "COM3") NomPort = "ttyUSB2";
                else if (m_portAutoref == "COM4") NomPort = "ttyUSB3";
                if (NomPort != "") break;
            }
        }
        if (NomPort != "")
        {
            sp_portAutoref     = new QSerialPort();
            for(int i=0; i<QSerialPortInfo::availablePorts().size(); i++)
            {
                if (QSerialPortInfo::availablePorts().at(i).portName().contains("usbserial"))
                {
                    QString letter = QSerialPortInfo::availablePorts().at(i).portName().split("-").at(1);
                    if (QSerialPortInfo::availablePorts().at(i).portName().right(1) == NomPort || letter.left(1) == NomPort)
                    {
                        sp_portAutoref->setPort(QSerialPortInfo::availablePorts().at(i));
                        sp_portAutoref->setBaudRate(s_paramPortSerieAutoref.baudRate);
                        sp_portAutoref->setFlowControl(s_paramPortSerieAutoref.flowControl);
                        sp_portAutoref->setParity(s_paramPortSerieAutoref.parity);
                        sp_portAutoref->setDataBits(s_paramPortSerieAutoref.dataBits);
                        sp_portAutoref->setStopBits(s_paramPortSerieAutoref.stopBits);
                        break;
                    }
                }
                else if (QSerialPortInfo::availablePorts().at(i).portName().contains("ttyUSB"))
                {
                    if (QSerialPortInfo::availablePorts().at(i).portName() == NomPort)
                    {
                        sp_portAutoref->setPort(QSerialPortInfo::availablePorts().at(i));
                        sp_portAutoref->setBaudRate(s_paramPortSerieAutoref.baudRate);
                        sp_portAutoref->setFlowControl(s_paramPortSerieAutoref.flowControl);
                        sp_portAutoref->setParity(s_paramPortSerieAutoref.parity);
                        sp_portAutoref->setDataBits(s_paramPortSerieAutoref.dataBits);
                        sp_portAutoref->setStopBits(s_paramPortSerieAutoref.stopBits);
                        break;
                    }
                }
            }
            //qDebug() << "AUTOREF -> " + m_portAutoref + " - " + NomPort;
            if (sp_portAutoref->open(QIODevice::ReadWrite))
            {
                t_threadAutoref     = new SerialThread(sp_portAutoref);
                t_threadAutoref   ->transaction();
                connect(t_threadAutoref,  &SerialThread::reponse,     this, &Procedures::ReponsePortSerie_Autoref);
            }
            else
            {
                UpMessageBox::Watch(Q_NULLPTR,tr("Connexion impossible"),tr("Impossible de connecter l'autorefractomètre") + "\n" + sp_portAutoref->errorString());
                delete sp_portAutoref;
                sp_portAutoref = Q_NULLPTR;
            }
        }
    }
    if (m_isTonoParametre)
    {
        m_portTono       = m_settings->value("Param_Poste/PortTonometre").toString();
    }
    return false;
}

void Procedures::ClearMesures()
{
    Datas::I()->mesurefronto    ->cleandatas();
    Datas::I()->mesureautoref   ->cleandatas();
    Datas::I()->mesureacuite    ->cleandatas();
    Datas::I()->mesurefinal     ->cleandatas();
    Datas::I()->mesurekerato    ->cleandatas();
    Datas::I()->tono            ->cleandatas();
    Datas::I()->pachy           ->cleandatas();
}

void Procedures::ClearHtmlMesures()
{
    m_htmlMesureFronto                .clear();
    m_htmlMesureAutoref               .clear();
    m_htmlMesureKerato                .clear();
    m_htmlMesureTono                  .clear();
    m_htmlMesurePachy                 .clear();
    m_htmlMesureRefracteurSubjectif   .clear();
    m_htmlMesureRefracteurFinal       .clear();
}

bool Procedures::ReglePortRefracteur()
{
    bool a = true;
    if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
        s_paramPortSerieRefracteur.baudRate       = QSerialPort::Baud2400;
        s_paramPortSerieRefracteur.dataBits       = QSerialPort::Data7;
        s_paramPortSerieRefracteur.parity         = QSerialPort::EvenParity;
        s_paramPortSerieRefracteur.stopBits       = QSerialPort::TwoStop;
        s_paramPortSerieRefracteur.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    return a;
}

QSerialPort* Procedures::PortRefracteur()
{
    return sp_portRefracteur;
}

//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série du refracteur
//-----------------------------------------------------------------------------------------
void Procedures::ReponsePortSerie_Refracteur(const QString &s)
{
    //qDebug() << s;
    m_mesureSerie        = s;
    if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
        if (m_mesureSerie == SendDataNIDEK("CRL"))
        {
            RegleRefracteur();
            return;
        }
    }
    if (!LectureDonneesRefracteur(m_mesureSerie))
        return;
    setHtmlRefracteur();
    if (!Datas::I()->mesureacuite->isdataclean())
    {
        InsertMesure(Subjectif);
        emit NouvMesure(Subjectif);
    }
}

void Procedures::RegleRefracteur()
{
    /*! On a créé 4 mesures
     * une de fronto élaborée à partir de la dernière prescription de verres du patient
     * une d'autoref qui reprend la dernière mesure d'autoref du patient
     * une de subjectif qui reprend la dernière mesure d'acuité du patient
     * et une de final qui reprend la dernière prescription de verres
     * Chacune de ces 4 mesures est envoyée au réfracteur pour le régler
     */
    if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100"
     || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
        /*! sur les NIDEK, on ne peut que régler l'autoref et le fronto depuis le PC - les refractions subjectives et finales ne peuvent pas être préréglées */
        QString AxeOD, AxeOG;
        QString AddOD, AddOG;
        QString SphereOD, SphereOG;
        QString CylindreOD, CylindreOG;
        QString SCAOD, SCAOG;
        QString DataAEnvoyer;

        auto convertaxe = [&] (QString &finalvalue, int originvalue)
        {
            if (originvalue < 10)       finalvalue = "  " + QString::number(originvalue);
            else if (originvalue < 100) finalvalue = " "  + QString::number(originvalue);
            else                        finalvalue = QString::number(originvalue);
        };
        auto convertdioptries = [&] (QString &finalvalue, double originvalue)
        {
            if (originvalue > 0)
                finalvalue = (originvalue < 10? "+0" : "+") + QString::number(originvalue,'f',2);
            else if (originvalue < 0)
                finalvalue = (originvalue > -10? QString::number(originvalue,'f',2).replace("-", "-0") : QString::number(originvalue,'f',2));
        };
        auto initvariables = [&] ()
        {
            AxeOD  = "180";
            AxeOG  = "180";
            AddOD  = "+00.00";
            AddOG  = "+00.00";
            SphereOD  = "+00.00";
            SphereOG  = "+00.00";
            CylindreOD  = "+00.00";
            CylindreOG  = "+00.00";
        };

        QByteArray DTRbuff;
        DTRbuff.append(QByteArray::fromHex("O1"));          //SOH -> start of header

        /*! réglage de l'autoref */
        if (m_flagreglagerefracteur.testFlag(Procedures::Autoref) && !Datas::I()->mesureautoref->isdataclean())
        {
            initvariables();
            convertaxe(AxeOD, Datas::I()->mesureautoref->axecylindreOD());
            convertaxe(AxeOG, Datas::I()->mesureautoref->axecylindreOG());
            convertdioptries(SphereOD, Datas::I()->mesureautoref->sphereOD());
            convertdioptries(SphereOG, Datas::I()->mesureautoref->sphereOG());
            convertdioptries(CylindreOD, Datas::I()->mesureautoref->cylindreOD());
            convertdioptries(CylindreOG, Datas::I()->mesureautoref->cylindreOG());

            SCAOD       = SphereOD + CylindreOD + AxeOD;
            SCAOG       = SphereOG + CylindreOG + AxeOG;
            SCAOD.replace("+0","+ ");
            SCAOD.replace("-0","- ");
            SCAOG.replace("+0","+ ");
            SCAOG.replace("-0","- ");
            DTRbuff.append("DRM");                              //section fronto
            DTRbuff.append(QByteArray::fromHex("2"));           //STX -> start of text
            DTRbuff.append("OR"+ SCAOD);                        //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append("OL"+ SCAOG);                        //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            if (Datas::I()->mesureautoref->ecartIP() > 0)
            {
                DTRbuff.append("PD"+ QString::number(Datas::I()->mesureautoref->ecartIP()));
                                                                //SD
                DTRbuff.append(QByteArray::fromHex("17"));      //ETB -> end of text block
            }
       }

        /*! réglage du fronto */
        if (m_flagreglagerefracteur.testFlag(Procedures::Fronto) && !Datas::I()->mesurefronto->isdataclean())
        {
            initvariables();

            convertaxe(AxeOD, Datas::I()->mesurefronto->axecylindreOD());
            convertaxe(AxeOG, Datas::I()->mesurefronto->axecylindreOG());
            convertdioptries(SphereOD, Datas::I()->mesurefronto->sphereOD());
            convertdioptries(SphereOG, Datas::I()->mesurefronto->sphereOG());
            convertdioptries(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
            convertdioptries(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

            SCAOD       = SphereOD + CylindreOD + AxeOD;
            SCAOG       = SphereOG + CylindreOG + AxeOG;
            SCAOD.replace("+0","+ ");
            SCAOD.replace("-0","- ");
            SCAOG.replace("+0","+ ");
            SCAOG.replace("-0","- ");
            AddOD       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2);
            AddOG       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2);
            DTRbuff.append("DLM");                              //section fronto
            DTRbuff.append(QByteArray::fromHex("2"));           //STX -> start of text
            DTRbuff.append(" R"+ SCAOD);                        //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append(" L"+ SCAOG);                        //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append("AR" + AddOD);                       //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            DTRbuff.append("AL" + AddOG);                       //SD
            DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block
            if (Datas::I()->mesurefronto->ecartIP() > 0)
            {
                DTRbuff.append("PD"+ QString::number(Datas::I()->mesurefronto->ecartIP()));
                                                                //SD
                DTRbuff.append(QByteArray::fromHex("17"));      //ETB -> end of text block
            }
        }

        DTRbuff.append(QByteArray::fromHex("4"));               //EOT -> end of transmission
        //qDebug() << "RegleRefracteur() - DTRBuff = " << DTRbuff;
        DataAEnvoyer = DTRbuff;
        QByteArray Data = DataAEnvoyer.toLocal8Bit();
        PortRefracteur()->clear();
        PortRefracteur()->write(Data);
        PortRefracteur()->waitForBytesWritten(1000);
    }
}

void Procedures::debugMesure(QObject *mesure, QString titre)
{
    if (titre != "")
        qDebug() << titre;
    MesureKerato *ker = qobject_cast<MesureKerato *>(mesure);
    if (ker != Q_NULLPTR)
    {
        QString Formule = "OD : " + QString::number(ker->K1OD()) + "/" + QString::number(ker->K2OD()) + " "  + QString::number(ker->axeKOD());
        qDebug() << Formule;
        Formule = "OG : " + QString::number(ker->K1OG()) + "/" + QString::number(ker->K2OG()) + " "  + QString::number(ker->axeKOG());
        qDebug() << Formule;
        return;
    }
    MesureRefraction *ref = qobject_cast<MesureRefraction *>(mesure);
    if (ref != Q_NULLPTR)
    {
        QString Formule = "OD : " + QString::number(ref->sphereOD());
        if (ref->cylindreOD() > 0)
            Formule += "(" + QString::number(ref->cylindreOD()) + " à " + QString::number(ref->axecylindreOD()) + "°)";
        Formule +=  " add." + QString::number(ref->addVPOD()) +  " VP";
        qDebug() << Utils::EnumDescription(QMetaEnum::fromType<Refraction::Mesure>(), ref->typemesure());
        qDebug() << Formule;
        Formule = "OG : " + QString::number(ref->sphereOG());
        if (ref->cylindreOG() > 0)
            Formule += "(" + QString::number(ref->cylindreOG()) + " à " + QString::number(ref->axecylindreOG()) + "°)";
        Formule +=  " add." + QString::number(ref->addVPOG()) +  " VP";
        qDebug() << Formule;
    }
}

void Procedures::EnvoiDataPatientAuRefracteur()
{
    m_isnewMesureAutoref = false;
    m_isnewMesureFronto  = false;
    m_isnewMesureKerato  = false;
    m_isnewMesureTono    = false;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (t_threadRefracteur!=Q_NULLPTR)
        // NIDEK RT-5100
        if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            PortRefracteur()->clear();
            PortRefracteur()->write(RequestToSendNIDEK());
            PortRefracteur()->waitForBytesWritten(100);
        }
}

QByteArray Procedures::RequestToSendNIDEK()
{
    /*! la séquence SOH "C**" STX "RS" ETB EOT -> Data a envoyer ="\001C**\002RS\017\004" RequestToSendNIDEK() est utilisée dans le système NIDEK pour signifier qu'on veut envoyer des données
     * Si l'appareil cible est OK, il émet la réponse SendDataNIDEK() = "\001CRL\002SD\027\004\r"
     * pour signifier qu'il est prêt à recevoir les données
     * Dans Rufus, cette demande d'envoi est créée à l'ouverture d'un dossier patient et permet de régler le refracteur sur les données de ce patient */
    QByteArray DTSbuff;
    DTSbuff.append(QByteArray::fromHex("1"));           //SOH -> start of header
    DTSbuff.append("C**");                              //C**
    DTSbuff.append(QByteArray::fromHex("2"));           //STX -> start of text
    DTSbuff.append("RS");                               //RS
    DTSbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
    DTSbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
    return  QString(DTSbuff).toLocal8Bit();
}

QByteArray Procedures::SendDataNIDEK(QString mesure)
{
    /*! la séquence SendData = "\001CRL\002SD\017\004" SendDataNIDEK() est utilisée dans le système NIDEK en réponse à une demande d'envoi de données RequestToSendNIDEK() */
    QByteArray DTRbuff;
    DTRbuff.append(QByteArray::fromHex("1"));           //SOH -> start of header
    DTRbuff.append(mesure);                             //CRL pour le refracteur, CLM pour le fronto, CRK ou CRM pour l'autoref
    DTRbuff.append(QByteArray::fromHex("2"));           //STX -> start of text
    DTRbuff.append("SD");                               //SD
    DTRbuff.append(QByteArray::fromHex("17"));          //ETB -> end of text block  -> fin RTS
    DTRbuff.append(QByteArray::fromHex("4"));           //EOT -> end of transmission
    //return  QString(DTRbuff).toLocal8Bit();
    QByteArray reponse = QString(DTRbuff).toLocal8Bit();
    reponse += "\r";                                    /*! +++ il faut rajouter \r à la séquence SendDataNIDEK("CRL") sinon ça ne marche pas .... */
    return reponse;
}

bool Procedures::LectureDonneesRefracteur(QString Mesure)
{
    m_htmlMesureFronto.clear();
    m_htmlMesureAutoref.clear();
    m_htmlMesureKerato.clear();
    m_htmlMesureRefracteurSubjectif.clear();
    m_htmlMesureRefracteurFinal.clear();
    m_htmlMesureTono.clear();
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

    QString mesureOD(""), mesureOG("");
    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    QString mTOOD(""), mTOOG("");

    bool dataok = false;

    // TRADUCTION DES DONNEES EN FONCTION DU REFRACTEUR
    // NIDEK RT-5100 - RT-2100 =======================================================================================================================================
    if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100"
     || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
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
            dataok = true;
            MesureRefraction        *oldMesureFronto = new MesureRefraction();
            oldMesureFronto         ->setdatas(Datas::I()->mesurefronto);
            Datas::I()->mesurefronto->cleandatas();
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
                Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
                Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
                Datas::I()->mesurefronto->setaxecylindreOD(mAxeOD.toInt());
                Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionFronto.contains(" L"))
            {
                mesureOG     = SectionFronto.mid(SectionFronto.indexOf(" L")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
                mAddOG       = SectionFronto.mid(SectionFronto.indexOf("AL")+2,6)    .replace(" ","0");
                Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
                Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
                Datas::I()->mesurefronto->setaxecylindreOG(mAxeOG.toInt());
                Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
            }
            //debugMesureRefraction(Datas::I()->mesurefronto);
            if (PortFronto() == Q_NULLPTR)                                      //! au cas où le fronto est directement branché sur la box du refracteur
                if (Datas::I()->mesurefronto->isDifferent(oldMesureFronto) && !Datas::I()->mesurefronto->isdataclean())
                {
                    m_isnewMesureFronto = true;
                    InsertMesure(Fronto);
                }
            delete oldMesureFronto;
        }

        // Données de l'AUTOREF - REFRACTION et KERATOMETRIE ----------------------------------------------------------------------------------------------
        if (Mesure.contains("@RM"))                 //=> il y a une mesure de refractometrie
        {
            dataok = true;
            MesureRefraction        *oldMesureAutoref = new MesureRefraction();
            oldMesureAutoref        ->setdatas(Datas::I()->mesureautoref);
            Datas::I()->mesureautoref->cleandatas();
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
                Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
                Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
                Datas::I()->mesureautoref->setaxecylindreOD(mAxeOD.toInt());
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionAutoref.contains("OL"))
            {
                mesureOG     = SectionAutoref.mid(SectionAutoref.indexOf("OL")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
                Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
                Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
                Datas::I()->mesureautoref->setaxecylindreOG(mAxeOG.toInt());
            }
            //debugMesureRefraction(Datas::I()->mesureautoref);
            if (PortAutoref() == Q_NULLPTR)                                      //! au cas où l'autoref est directement branché sur la box du refracteur
                if (Datas::I()->mesureautoref->isDifferent(oldMesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
                {
                    m_isnewMesureAutoref = true;
                    InsertMesure(Autoref);
                }
            delete oldMesureAutoref;
        }

        if (Mesure.contains("@KM"))                 //=> il y a une mesure de keratométrie - cette mesure ne peut qu'avoir été effectuée par un autoref connecté directement à la box du refraacteur
        {
            dataok = true;
            MesureKerato  *oldMesureKerato = new MesureKerato();
            oldMesureKerato->setdatas(Datas::I()->mesurekerato);
            Datas::I()->mesurekerato->cleandatas();
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
                Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                Datas::I()->mesurekerato->setaxeKOD(AxeKOD);
                if (SectionKerato.contains("DR"))
                {
                    mesureOD        = SectionKerato.mid(SectionKerato.indexOf("DR")+2,10)   .replace(" ","0");
                    Datas::I()->mesurekerato->setdioptriesK1OD(mesureOD.mid(0,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OD(mesureOD.mid(5,5).toDouble());
                }
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionKerato.contains("CL"))
            {
                mesureOG            = SectionKerato.mid(SectionKerato.indexOf("CL")+2,13)   .replace(" ","0");
                K1OG                = mesureOG.mid(0,5);
                K2OG                = mesureOG.mid(5,5);
                AxeKOG              = mesureOG.mid(10,3).toInt();
                Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                Datas::I()->mesurekerato->setaxeKOG(AxeKOG);
                if (SectionKerato.contains("DL"))
                {
                    mesureOG            = SectionKerato.mid(SectionKerato.indexOf("DL")+2,10)   .replace(" ","0");
                    Datas::I()->mesurekerato->setdioptriesK1OG(mesureOG.mid(0,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OG(mesureOG.mid(5,5).toDouble());
                }
            }
            if (PortAutoref() == Q_NULLPTR)                                      //! au cas où l'autoref est directement branché sur la box du refracteur
                if (Datas::I()->mesurekerato->isDifferent(oldMesureKerato) && !Datas::I()->mesurekerato->isdataclean())
                {
                    m_isnewMesureKerato = true;
                    InsertMesure(Kerato);
                }
            delete oldMesureKerato;
        }

        // Données du REFRACTEUR --------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@RT"))                 //=> il y a une mesure de refraction
        {
            dataok = true;
            idx                         = Mesure.indexOf("@RT");
            QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);

            // les données subjectives --------------------------------------------------------------------------------------------------------------
            // OEIL DROIT -----------------------------------------------------------------------------
            Datas::I()->mesureacuite->cleandatas();
            if (SectionRefracteur.contains("fR") || SectionRefracteur.contains("fL"))
            {
                PD               = SectionRefracteur.mid(SectionRefracteur.indexOf("pD")+2,2);
                Datas::I()->mesureacuite->setecartIP(PD.toInt());
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
                    Datas::I()->mesureacuite->setsphereOD(mSphereOD.toDouble());
                    Datas::I()->mesureacuite->setcylindreOD(mCylOD.toDouble());
                    Datas::I()->mesureacuite->setaxecylindreOD(mAxeOD.toInt());
                    Datas::I()->mesureacuite->setaddVPOD(mAddOD.toDouble());
                    Datas::I()->mesureacuite->setavlOD(AVLOD);
                    Datas::I()->mesureacuite->setavpOD(AVPOD);
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
                    Datas::I()->mesureacuite->setsphereOG(mSphereOG.toDouble());
                    Datas::I()->mesureacuite->setcylindreOG(mCylOG.toDouble());
                    Datas::I()->mesureacuite->setaxecylindreOG(mAxeOG.toInt());
                    Datas::I()->mesureacuite->setaddVPOG(mAddOG.toDouble());
                    Datas::I()->mesureacuite->setavlOG(AVLOG);
                    Datas::I()->mesureacuite->setavpOG(AVPOG);
                }
            }

            // les données finales --------------------------------------------------------------------------------------------------------------
            // OEIL DROIT -----------------------------------------------------------------------------
            Datas::I()->mesurefinal->cleandatas();
            if (SectionRefracteur.contains("FR") || SectionRefracteur.contains("FL"))
            {
                PD               = SectionRefracteur.mid(SectionRefracteur.indexOf("PD")+2,2);
                Datas::I()->mesurefinal->setecartIP(PD.toInt());
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

                    Datas::I()->mesurefinal->setsphereOD(mSphereOD.toDouble());
                    Datas::I()->mesurefinal->setcylindreOD(mCylOD.toDouble());
                    Datas::I()->mesurefinal->setaxecylindreOD(mAxeOD.toInt());
                    Datas::I()->mesurefinal->setaddVPOD(mAddOD.toDouble());
                    Datas::I()->mesurefinal->setavlOD(AVLOD);
                    Datas::I()->mesurefinal->setavpOD(AVPOD);
                }
                // OEIL GAUCHE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("FL"))
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

                    Datas::I()->mesurefinal->setsphereOG(mSphereOG.toDouble());
                    Datas::I()->mesurefinal->setcylindreOG(mCylOG.toDouble());
                    Datas::I()->mesurefinal->setaxecylindreOG(mAxeOG.toInt());
                    Datas::I()->mesurefinal->setaddVPOG(mAddOG.toDouble());
                    Datas::I()->mesurefinal->setavlOG(AVLOG);
                    Datas::I()->mesurefinal->setavpOG(AVPOG);
                }
            }
        }
        // Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@NT"))                 //=> il y a une mesure de tonometrie
        {
            dataok = true;
            Tonometrie  *oldMesureTono = new Tonometrie();
            oldMesureTono->setdatas(Datas::I()->tono);
            logmesure("LectureDonneesRefracteur() - ancienne mesure tono -> TOD = " + QString::number(Datas::I()->tono->TOD()) + " - TOG = " + QString::number(Datas::I()->tono->TOG()));
            Datas::I()->tono->cleandatas();
            idx                     = Mesure.indexOf("@NT");
            QString SectionTono     = Mesure.right(Mesure.length()-idx-5);
            SectionTono             = SectionTono.left(SectionTono.indexOf("@"));
            logmesure("LectureDonneesRefracteur() - " + SectionTono);
            //Edit(SectionTono+ "\nOK");
            // OEIL DROIT -----------------------------------------------------------------------------
            mTOOD                   = SectionTono.mid(SectionTono.indexOf("TR")+2,4)   .replace(" ","0");
            // OEIL GAUCHE ---------------------------------------------------------------------------
            mTOOG                   = SectionTono.mid(SectionTono.indexOf("TL")+2,4)   .replace(" ","0");
            Datas::I()->tono->setTOD(mTOOD.toInt());
            Datas::I()->tono->setTOG(mTOOG.toInt());
            Datas::I()->tono->setmodemesure(Tonometrie::Air);
            logmesure("LectureDonneesRefracteur() - nouvelle mesure tono -> TOD = " + QString::number(Datas::I()->tono->TOD()) + " - TOG = " + QString::number(Datas::I()->tono->TOG()));
            QString portautoref = (PortAutoref() == Q_NULLPTR? "Q_NULLPTR" : "OK");
            logmesure("LectureDonneesRefracteur() - PortAutoref() = " + portautoref);
            if (PortAutoref() == Q_NULLPTR)                                      //! au cas où l'autoref est directement branché sur la box du refracteur
            {
                QString isdifferentmesure = (Datas::I()->tono->isDifferent(oldMesureTono)? "true" : "false");
                logmesure("LectureDonneesRefracteur() - Datas::I()->tono->isDifferent(oldMesureTono) = " + isdifferentmesure);
                QString iscleandatas = (Datas::I()->tono->isdataclean()? "true" : "false");
                logmesure("LectureDonneesRefracteur() - Datas::I()->tono->isdataclean()) = " + iscleandatas);
                if (Datas::I()->tono->isDifferent(oldMesureTono) && !Datas::I()->tono->isdataclean())
                {
                    logmesure("LectureDonneesRefracteur() - OK nouvelle mesure tono");
                    m_isnewMesureTono = true;
                    InsertMesure(Tono);
                }
            }
            delete oldMesureTono;
        }
        debugMesure(Datas::I()->mesurekerato, "Procedures::LectureDonneesRefracteur(QString Mesure)");
    }
    // FIN NIDEK RT-5100 et RT 2100 ==========================================================================================================================
    return dataok;
}

// -------------------------------------------------------------------------------------
// Generation du resumé des données issues du refracteur
//--------------------------------------------------------------------------------------
void Procedures::setHtmlRefracteur()
{
   // CALCUL DE HtmlMesureFronto ====================================================================================================================================
    if (Datas::I()->mesurefronto != Q_NULLPTR && m_isnewMesureFronto)
    {
        setHtmlFronto();
        //debugMesureRefraction(Datas::I()->mesurefronto);
    }
    // CALCUL DE HtmlMesureAutoref ===================================================================================================================================
    if (!Datas::I()->mesureautoref->isdataclean() && m_isnewMesureAutoref)
    {
        setHtmlAutoref();
        //debugMesureRefraction(Datas::I()->mesureautoref);
    }
    // CALCUL DE HtmlMesureKerato ====================================================================================================================================
    if (!Datas::I()->mesurekerato->isdataclean() && m_isnewMesureKerato)
        setHtmlKerato();
    // CALCUL DE HtmlMesureRefracteurSubjectif =======================================================================================================================
    QString Resultat = "";
    if(!Datas::I()->mesureacuite->isdataclean())
    {
        // - 1 - détermination des verres
        QString mSphereOD   = Utils::PrefixePlus(Datas::I()->mesureacuite->sphereOD());
        QString mCylOD      = Utils::PrefixePlus(Datas::I()->mesureacuite->cylindreOD());
        QString mAxeOD      = QString::number(Datas::I()->mesureacuite->axecylindreOD());
        QString mAddOD      = Utils::PrefixePlus(Datas::I()->mesureacuite->addVPOD());
        QString mAVLOD ("");
        if (Datas::I()->mesureacuite->avlOD().toDouble()>0)
            mAVLOD      = QLocale().toString(Datas::I()->mesureacuite->avlOD().toDouble()*10) + "/10";
        QString mAVPOD      = Datas::I()->mesureacuite->avpOD();
        QString mSphereOG   = Utils::PrefixePlus(Datas::I()->mesureacuite->sphereOG());
        QString mCylOG      = Utils::PrefixePlus(Datas::I()->mesureacuite->cylindreOG());
        QString mAxeOG      = QString::number(Datas::I()->mesureacuite->axecylindreOG());
        QString mAddOG      = Utils::PrefixePlus(Datas::I()->mesureacuite->addVPOG());
        QString mAVLOG ("");
        if (Datas::I()->mesureacuite->avlOG().toDouble()>0)
            mAVLOG      = QLocale().toString(Datas::I()->mesureacuite->avlOG().toDouble()*10) + "/10";
        QString mAVPOG      = Datas::I()->mesureacuite->avpOG();
        QString ResultatVLOD, ResultatVLOG,ResultatVPOD, ResultatVPOG, ResultatOD, ResultatOG;

        // détermination OD
        if (Datas::I()->mesureacuite->cylindreOD() != 0.0 && Datas::I()->mesureacuite->sphereOD() != 0.0)
            ResultatVLOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
        else if (Datas::I()->mesureacuite->cylindreOD() == 0.0 && Datas::I()->mesureacuite->sphereOD() != 0.0)
            ResultatVLOD = mSphereOD;
        else if (Datas::I()->mesureacuite->cylindreOD() != 0.0 && Datas::I()->mesureacuite->sphereOD() == 0.0)
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
        if (Datas::I()->mesureacuite->cylindreOG() != 0.0 && Datas::I()->mesureacuite->sphereOG() != 0.0)
            ResultatVLOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + ")";
        else if (Datas::I()->mesureacuite->cylindreOG() == 0.0 && Datas::I()->mesureacuite->sphereOG() != 0.0)
            ResultatVLOG = mSphereOG;
        else if (Datas::I()->mesureacuite->cylindreOG() != 0.0 && Datas::I()->mesureacuite->sphereOG() == 0.0)
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
        if (Datas::I()->mesureacuite->addVPOD() > 0 || Datas::I()->mesureacuite->addVPOG() > 0)  // il y a eu mesure de près et de loin
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
        Resultat = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Resultat + "</td><td width=\"70\"><font color = \"red\"></font></td><td>" + m_currentuser->login() + "</td></p>";
    }
    m_htmlMesureRefracteurSubjectif = Resultat;
    // CALCUL DE HtmlMesureTono ======================================================================================================================================
    if (!Datas::I()->tono->isdataclean() && m_isnewMesureTono)
    {
        setHtmlTono();
        logmesure("setHtmlRefracteur() -> m_htmlMesureTono = " + m_htmlMesureTono);
    }
}

QString Procedures::HtmlRefracteur()
{
    if (!Datas::I()->mesurefinal->isdataclean())
        return m_htmlMesureKerato + m_htmlMesureFronto + m_htmlMesureAutoref + m_htmlMesureRefracteurSubjectif + m_htmlMesureTono;
    else
        return m_htmlMesureKerato + m_htmlMesureFronto + m_htmlMesureAutoref + m_htmlMesureTono;
}

bool Procedures::ReglePortFronto()
{
    bool a = true;
    if (m_settings->value("Param_Poste/Fronto").toString()=="TOMEY TL-3000C")
    {
        s_paramPortSerieFronto.baudRate       = QSerialPort::Baud2400;
        s_paramPortSerieFronto.dataBits       = QSerialPort::Data7;
        s_paramPortSerieFronto.parity         = QSerialPort::EvenParity;
        s_paramPortSerieFronto.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (m_settings->value("Param_Poste/Fronto").toString()=="VISIONIX VL1000"
          || m_settings->value("Param_Poste/Fronto").toString()=="HUVITZ CLM7000")
    {
        s_paramPortSerieFronto.baudRate       = QSerialPort::Baud9600;
        s_paramPortSerieFronto.dataBits       = QSerialPort::Data7;
        s_paramPortSerieFronto.parity         = QSerialPort::EvenParity;
        s_paramPortSerieFronto.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800P"
          || m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800PD"
          || m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-500")
    {
        s_paramPortSerieFronto.baudRate       = QSerialPort::Baud9600;
        s_paramPortSerieFronto.dataBits       = QSerialPort::Data8;
        s_paramPortSerieFronto.parity         = QSerialPort::EvenParity;
        s_paramPortSerieFronto.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    return a;
}

QSerialPort *Procedures::PortFronto()
{
    return sp_portFronto;
}

// lire les ports séries
//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série du Fronto
//-----------------------------------------------------------------------------------------
void Procedures::ReponsePortSerie_Fronto(const QString &s)
{
    m_mesureSerie        = s;
    //qDebug() << gMesureSerie;

    if (m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800P"
     || m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800PD"
     || m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-500")
    {
        if (m_mesureSerie == RequestToSendNIDEK())          //! le fronto demande la permission d'envoyer des données
        {
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            PortFronto()->clear();
            PortFronto()->write(SendDataNIDEK("CLM"));
            PortFronto()->waitForBytesWritten(100);
            return;
        }
    }
    if (!LectureDonneesFronto(m_mesureSerie))
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (t_threadRefracteur!=Q_NULLPTR && !FicheRefractionOuverte())
    {
        m_flagreglagerefracteur = Fronto;
        // NIDEK RT-5100
        if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            PortRefracteur()->clear();
            PortRefracteur()->write(RequestToSendNIDEK());
            PortRefracteur()->waitForBytesWritten(100);
        }
    }
    setHtmlFronto();
    InsertMesure(Fronto);
    emit NouvMesure(Fronto);
}

bool Procedures::LectureDonneesFronto(QString Mesure)
{
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
    Datas::I()->mesurefronto->cleandatas();
    m_htmlMesureFronto.clear();
    bool dataok = false;

    //A - AFFICHER LA MESURE --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (m_settings->value("Param_Poste/Fronto").toString()=="TOMEY TL-3000C")
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
        // OEIL DROIT -----------------------------------------------------------------------------
        int idxOD = Mesure.indexOf("LR");
        if (idxOD > 0)
        {
            dataok = true;
            mesureOD            = Mesure.mid(idxOD+2,15)   .replace(" ","0");
            mSphereOD            = mesureOD.mid(0,6);
            mCylOD               = mesureOD.mid(6,6);
            mAxeOD               = mesureOD.mid(12,3);
            if (Mesure.indexOf("AR")>0)
                mAddOD           = Utils::PrefixePlus(Mesure.mid(Mesure.indexOf("AR")+2,4).toDouble());
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(mAxeOD.toInt());
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        int idxOG = Mesure.indexOf("LL");
        if (idxOG > 0)
        {
            dataok = true;
            mesureOG            = Mesure.mid(idxOG+2,15)   .replace(" ","0");
            mSphereOG            = mesureOG.mid(0,6);
            mCylOG               = mesureOG.mid(6,6);
            mAxeOG               = mesureOG.mid(12,3);
            if (Mesure.indexOf("AL")>0)
                mAddOG           = Utils::PrefixePlus(Mesure.mid(Mesure.indexOf("AL")+2,4).toDouble());
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(mAxeOG.toInt());
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
        //les autres champs ne sont pas utilisés pour le moment -------------------------------
    }
    else if (m_settings->value("Param_Poste/Fronto").toString()=="VISIONIX VL1000"
          || m_settings->value("Param_Poste/Fronto").toString()=="HUVITZ CLM7000")
    {
        /* Le fichier de sortie ressemble à ça
            LM2RK   No=00036   R: S=-04.50 C=-00.50 A=103 PX=+00.25 PY=+04.00 PD=00.0 ADD=+2.00 UR=  0   L: S=-05.00 C=-00.50 A=110 PX=+00.00 PY=+05.50 PD=00.0 ADD=+5.00 UL=  0   E$
            */
        // qDebug() << Mesure;
        // OEIL DROIT -----------------------------------------------------------------------------
        int idxOD = Mesure.indexOf("R: ");
        if (idxOD > 0)
        {
            dataok = true;
            mesureOD            = Mesure.mid(idxOD+3);
            mSphereOD           = mesureOD.mid(mesureOD.indexOf("S=")+2,6);
            mCylOD              = mesureOD.mid(mesureOD.indexOf("C=")+2,6);
            mAxeOD              = mesureOD.mid(mesureOD.indexOf("A=")+2,3);
            mAddOD              = mesureOD.mid(mesureOD.indexOf("ADD=")+4,5);
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(mAxeOD.toInt());
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        int idxOG = Mesure.indexOf("L: ");
        if (idxOG > 0)
        {
            dataok = true;
            mesureOG            = Mesure.mid(idxOG+3);
            mSphereOG           = mesureOG.mid(mesureOG.indexOf("S=")+2,6);
            mCylOG              = mesureOG.mid(mesureOG.indexOf("C=")+2,6);
            mAxeOG              = mesureOG.mid(mesureOG.indexOf("A=")+2,3);
            mAddOG              = mesureOG.mid(mesureOG.indexOf("ADD=")+4,5);
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(mAxeOG.toInt());
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
        //les autres champs ne sont pas utilisés pour le moment -------------------------------
    }
    else if (m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800P"
          || m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-1800PD"
          || m_settings->value("Param_Poste/Fronto").toString()=="NIDEK LM-500")
    {
        // OEIL DROIT -----------------------------------------------------------------------------
        int idxOD = Mesure.indexOf(" R");
        if (idxOD > 0)
        {
            dataok = true;
            mesureOD            = Mesure.mid(idxOD+2,15)   .replace(" ","0");
            mSphereOD            = mesureOD.mid(0,6);
            mCylOD               = mesureOD.mid(6,6);
            mAxeOD               = mesureOD.mid(12,3);
            if (Mesure.indexOf("AR")>0)
                mAddOD           = Mesure.mid(Mesure.indexOf("AR")+2,5);
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(mAxeOD.toInt());
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        int idxOG = Mesure.indexOf(" L");
        if (idxOG > 0)
        {
            dataok = true;
            mesureOG            = Mesure.mid(idxOG+2,15)   .replace(" ","0");
            mSphereOG            = mesureOG.mid(0,6);
            mCylOG               = mesureOG.mid(6,6);
            mAxeOG               = mesureOG.mid(12,3);
            if (Mesure.indexOf("AL")>0)
                mAddOG           = Mesure.mid(Mesure.indexOf("AL")+2,5);
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(mAxeOG.toInt());
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
    }
    return dataok;
}

// -------------------------------------------------------------------------------------
// Generation du resumé des données issues du frontocomètre
//--------------------------------------------------------------------------------------
void Procedures::setHtmlFronto()
{
    QString mSphereOD   = Utils::PrefixePlus(Datas::I()->mesurefronto->sphereOD());
    QString mCylOD      = Utils::PrefixePlus(Datas::I()->mesurefronto->cylindreOD());
    QString mAxeOD      = QString::number(Datas::I()->mesurefronto->axecylindreOD());
    QString mAddOD      = Utils::PrefixePlus(Datas::I()->mesurefronto->addVPOD());
    QString mSphereOG   = Utils::PrefixePlus(Datas::I()->mesurefronto->sphereOG());
    QString mCylOG      = Utils::PrefixePlus(Datas::I()->mesurefronto->cylindreOG());
    QString mAxeOG      = QString::number(Datas::I()->mesurefronto->axecylindreOG());
    QString mAddOG      = Utils::PrefixePlus(Datas::I()->mesurefronto->addVPOG());

    QString ResultatOD, ResultatVLOD, ResultatVPOD;
    QString ResultatOG, ResultatVLOG, ResultatVPOG;
    QString Resultat = "";

    m_htmlMesureFronto = "";

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

    m_htmlMesureFronto =  "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("Porte") + ":</b></font></td><td>" + Resultat + "</p>";
}

QString Procedures::HtmlFronto()
{
    return m_htmlMesureFronto;
}

bool Procedures::ReglePortAutoref()
{
    bool a = true;
    if (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1S"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-20"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")

    {
        s_paramPortSerieAutoref.baudRate       = QSerialPort::Baud9600;
        s_paramPortSerieAutoref.dataBits       = QSerialPort::Data8;
        s_paramPortSerieAutoref.parity         = QSerialPort::EvenParity;
        s_paramPortSerieAutoref.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieAutoref.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    return a;
}


QSerialPort* Procedures::PortAutoref()
{
    return sp_portAutoref;
}

// lire les ports séries
//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série de l'autoref
//-----------------------------------------------------------------------------------------
void Procedures::ReponsePortSerie_Autoref(const QString &s)
{
    m_mesureSerie        = s;
    //qDebug() << gMesureSerie;

    if (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1S"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-20")
    {
        if (m_mesureSerie == RequestToSendNIDEK())       //! l'autoref demande la permission d'envoyer des données
        {
            QString cmd;
            cmd = ((m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30")?
                    "CRK" : "CRM");     //! CRK ou CRM suivant que les appareils peuvent ou non envoyer la keratométrie
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            PortAutoref()->clear();
            PortAutoref()->write(SendDataNIDEK(cmd));
            PortAutoref()->waitForBytesWritten(100);
        }
    }
    if (!LectureDonneesAutoref(m_mesureSerie))
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (t_threadRefracteur!=Q_NULLPTR && !FicheRefractionOuverte())
    {
        m_flagreglagerefracteur = Autoref;
        // NIDEK RT-5100 - NIDEK RT-2100
        if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            if (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
             || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")
            {
                if (!Datas::I()->mesurekerato->isdataclean())
                {
                    setHtmlKerato();
                    InsertMesure(Kerato);
                    emit NouvMesure(Kerato);
                }
                if (!Datas::I()->mesureautoref->isdataclean())
                {
                    setHtmlAutoref();
                    InsertMesure(Autoref);
                    emit NouvMesure(Autoref);
                }
            }
            if (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")
            {
                if (!Datas::I()->tono->isdataclean())
                {
                    setHtmlTono();
                    InsertMesure(Tono);
                    emit NouvMesure(Tono);
                }
                if (!Datas::I()->pachy->isdataclean())
                {
                    setHtmlPachy();
                    InsertMesure(Pachy);
                    emit NouvMesure(Pachy);
                }
            }
            //Dans un premier temps, le PC envoie la requête d'envoi de données
            PortRefracteur()->clear();
            PortRefracteur()->write(RequestToSendNIDEK());
            PortRefracteur()->waitForBytesWritten(100);
        }
    }
}

bool Procedures::LectureDonneesAutoref(QString Mesure)
{
    //Edit(Mesure);
    m_htmlMesureAutoref.clear();
    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString PD          = "";

    QString Ref(""), mesureOD(""), mesureOG("");
    QString K(""), KOD(""), KOG("");
    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    int     a(0);

    bool dataok = false;

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

    if (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-1S"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30"
     || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK AR-20")
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
            Datas::I()->mesureautoref->cleandatas();
            a  = Ref.indexOf("OR");
            // OEIL DROIT -----------------------------------------------------------------------------
            if (a>=0)
            {
                dataok = true;
                mesureOD             = Ref.mid(Ref.indexOf("OR")+2,15)   .replace(" ","0");
                mSphereOD            = mesureOD.mid(0,6);
                mCylOD               = mesureOD.mid(6,6);
                mAxeOD               = mesureOD.mid(12,3);
                Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
                Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
                Datas::I()->mesureautoref->setaxecylindreOD(mAxeOD.toInt());
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            a  = Ref.indexOf("OL");
            if (a>=0)
            {
                dataok = true;
                mesureOG             = Ref.mid(Ref.indexOf("OL")+2,15)   .replace(" ","0");
                mSphereOG            = mesureOG.mid(0,6);
                mCylOG               = mesureOG.mid(6,6);
                mAxeOG               = mesureOG.mid(12,3);
                Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
                Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
                Datas::I()->mesureautoref->setaxecylindreOG(mAxeOG.toInt());
            }
            if (m_settings->value("Param_Poste/Autoref").toString() != "NIDEK HandyRef-K" || m_settings->value("Param_Poste/Autoref").toString()!= "NIDEK ARK-30")
            {
                a  = Ref.indexOf("PD");
                if (a >= 0)
                {
                    PD               = Ref.mid(Ref.indexOf("PD")+2,2);
                    Datas::I()->mesureautoref->setecartIP(PD.toInt());
                }
            }
            if (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
                    || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30")
            {
                // Données de KERATOMETRIE --------------------------------------------------------------------------------------------------------
                m_htmlMesureKerato.clear();
                if (Mesure.contains("DKM"))                 //=> il y a une mesure de keratometrie
                {
                    Datas::I()->mesurekerato->cleandatas();
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
                        Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                        Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                        Datas::I()->mesurekerato->setaxeKOD(AxeKOD);
                        QString mOD         = K.mid(K.indexOf("DR")+2,10).replace(" ","0");
                        Datas::I()->mesurekerato->setdioptriesK1OD(mOD.mid(0,5).toDouble());
                        Datas::I()->mesurekerato->setdioptriesK2OD(mOD.mid(5,5).toDouble());
                    }
                    // OEIL GAUCHE ---------------------------------------------------------------------------
                    a  = Ref.indexOf(" L");
                    if (a>=0)
                    {
                        KOG                 = K.mid(K.indexOf(" L")+2,13);
                        K1OG                = KOG.mid(0,5);
                        K2OG                = KOG.mid(5,5);
                        AxeKOG              = KOG.mid(10,3).toInt();
                        Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                        Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                        Datas::I()->mesurekerato->setaxeKOG(AxeKOG);
                        QString mOG         = K.mid(K.indexOf("DL")+2,10).replace(" ","0");
                        Datas::I()->mesurekerato->setdioptriesK1OG(mOG.mid(0,5).toDouble());
                        Datas::I()->mesurekerato->setdioptriesK2OG(mOG.mid(5,5).toDouble());
                    }
                    if (K1OD !="null" || K1OG != "null")
                            dataok = true;
                }
            }
        }
        if (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III")
        {
            m_htmlMesureTono.clear();
            m_htmlMesurePachy.clear();
            // Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
            Datas::I()->tono->cleandatas();
            if (Mesure.contains("DNT"))                 //=> il y a une mesure de tonometrie
            {
                QString TonoOD(""), TonoOG("");
                a                       = Mesure.indexOf("DNT");
                a                       = Mesure.length() - a -1;
                QString Tono            = Mesure.right(a);
                int b                   = Tono.indexOf("DPM");
                if (b>=0)
                    Tono                = Tono.left(b-1);
                a = Tono.indexOf(" R");
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
                if (TonoOD != "" || TonoOG != "")
                    dataok = true;
                Datas::I()->tono->setTOD(TonoOD.toInt());
                Datas::I()->tono->setTOG(TonoOG.toInt());
                Datas::I()->tono->setmodemesure(Tonometrie::Air);
            }
            // Données de PACHYMETRIE --------------------------------------------------------------------------------------------------------
            Datas::I()->pachy->cleandatas();
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
                if (PachyOD != "" || PachyOG != "")
                    dataok = true;
                Datas::I()->pachy->setpachyOD(PachyOD.toInt());
                Datas::I()->pachy->setpachyOG(PachyOG.toInt());
                Datas::I()->pachy->setmodemesure(Pachymetrie::Optique);
            }
        }
    }
    //qDebug() << "od" << mSphereOD << mCylOD << mAxeOD << "og" << mSphereOG << mCylOG << mAxeOG << "PD = " + PD;
    return dataok;
}


// -------------------------------------------------------------------------------------
// Generation du resumé de l'autorefractometre
//--------------------------------------------------------------------------------------
void Procedures::setHtmlAutoref()
{
    QString ResultatOD("");
    QString ResultatOG("");
    QString Resultat = "";

    // détermination OD
    if (!Datas::I()->mesureautoref->isnullLOD())
    {
        QString mSphereOD   = Utils::PrefixePlus(Datas::I()->mesureautoref->sphereOD());
        QString mCylOD      = Utils::PrefixePlus(Datas::I()->mesureautoref->cylindreOD());
        QString mAxeOD      = QString::number(Datas::I()->mesureautoref->axecylindreOD());
        if (Datas::I()->mesureautoref->cylindreOD() != 0.0 && Datas::I()->mesureautoref->sphereOD() != 0.0)
            ResultatOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
        else if (Datas::I()->mesureautoref->cylindreOD() == 0.0 && Datas::I()->mesureautoref->sphereOD() != 0.0)
            ResultatOD = mSphereOD;
        else if (Datas::I()->mesureautoref->cylindreOD() != 0.0 && Datas::I()->mesureautoref->sphereOD() == 0.0)
            ResultatOD = mCylOD + tr(" à ") + mAxeOD + "°" ;
        else
            ResultatOD = tr("plan");
    }
    // détermination OG
    if (!Datas::I()->mesureautoref->isnullLOG())
    {
        QString mSphereOG   = Utils::PrefixePlus(Datas::I()->mesureautoref->sphereOG());
        QString mCylOG      = Utils::PrefixePlus(Datas::I()->mesureautoref->cylindreOG());
        QString mAxeOG      = QString::number(Datas::I()->mesureautoref->axecylindreOG());
        if (Datas::I()->mesureautoref->cylindreOG() != 0.0 && Datas::I()->mesureautoref->sphereOG() != 0.0)
            ResultatOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + "°)";
        else if (Datas::I()->mesureautoref->cylindreOG() == 0.0 && Datas::I()->mesureautoref->sphereOG() != 0.0)
            ResultatOG = mSphereOG;
        else if (Datas::I()->mesureautoref->cylindreOG() != 0.0 && Datas::I()->mesureautoref->sphereOG() == 0.0)
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
    m_htmlMesureAutoref =  "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>"
                           + tr("Autoref") + ":</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Resultat + "</td></p>";
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la keratométrie
//--------------------------------------------------------------------------------------
void Procedures::setHtmlKerato()
{
    QString kerato = "";
    if (!Datas::I()->mesurekerato->isnullLOD())
    {
        QString mK1OD       = QLocale().toString(Datas::I()->mesurekerato->K1OD(),'f',2);
        QString mK2OD       = QLocale().toString(Datas::I()->mesurekerato->K2OD(),'f',2);
        QString mKOD        = QLocale().toString(Datas::I()->mesurekerato->KMOD(),'f',2);
        QString mAxeKOD     = QString::number(Datas::I()->mesurekerato->axeKOD());
        QString mDioptrK1OD = QLocale().toString(Datas::I()->mesurekerato->dioptriesK1OD(),'f',1);
        QString mDioptrK2OD = QLocale().toString(Datas::I()->mesurekerato->dioptriesK2OD(),'f',1);
        QString mDioptrKOD  = Utils::PrefixePlus(Datas::I()->mesurekerato->dioptriesKOD());
        QString mDioptrmOD  = QString::number(Datas::I()->mesurekerato->dioptriesKMOD(),'f',2);
        if (QLocale().toDouble(mDioptrK1OD)!=0.0)
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OD + "/" + mK2OD + " Km = " + mKOD + "</td>"
                      "<td width=\"180\">" + mDioptrK1OD + "/" + mDioptrK2OD + " " + mDioptrKOD +  tr(" à ") + mAxeKOD + "°</td></p>";
        else
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OD + tr(" à ") + mAxeKOD + "°/" + mK2OD
                      + " Km = " + mDioptrmOD + "</td></p>";
    }
    if (!Datas::I()->mesurekerato->isnullLOG())
    {
        QString mK1OG       = QLocale().toString(Datas::I()->mesurekerato->K1OG(),'f',2);
        QString mK2OG       = QLocale().toString(Datas::I()->mesurekerato->K2OG(),'f',2);
        QString mKOG        = QLocale().toString(Datas::I()->mesurekerato->KMOG(),'f',2);
        QString mAxeKOG     = QString::number(Datas::I()->mesurekerato->axeKOG());
        QString mDioptrK1OG = QLocale().toString(Datas::I()->mesurekerato->dioptriesK1OG(),'f',1);
        QString mDioptrK2OG = QLocale().toString(Datas::I()->mesurekerato->dioptriesK2OG(),'f',1);
        QString mDioptrKOG  = Utils::PrefixePlus(Datas::I()->mesurekerato->dioptriesKOG());
        QString mDioptrmOG  = QString::number(Datas::I()->mesurekerato->dioptriesKMOG(),'f',2);
        if (QLocale().toDouble(mDioptrK1OG)!=0.0)
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OG + "/" + mK2OG + " Km = " + mKOG + "</td>"
                      "<td width=\"180\">" + mDioptrK1OG + "/" + mDioptrK2OG + " " + mDioptrKOG +  tr(" à ") + mAxeKOG + "°</td></p>";
        else
            kerato += "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"180\">"  + mK1OG +  tr(" à ") + mAxeKOG + "°/" + mK2OG
                      + " Km = " + mDioptrmOG + "</td></p>";
    }
    m_htmlMesureKerato = kerato;
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la tonométrie
//--------------------------------------------------------------------------------------
void Procedures::setHtmlTono()
{
    logmesure("setHtmlTono() -> anc m_htmlMesureTono = " + m_htmlMesureTono);
    m_htmlMesureTono = "";
    QString mTOD        = QLocale().toString(Datas::I()->tono->TOD());
    QString mTOG        = QLocale().toString(Datas::I()->tono->TOG());
    QString Methode     = Tonometrie::ConvertMesure(Datas::I()->tono->modemesure());
    QString Tono        ="";
    QString TODcolor, TOGcolor;

    if (mTOD.toInt() > 0 || mTOG.toInt() > 0)
    {
        if (Datas::I()->tono->TOD() > 21)
            TODcolor = "<font color = \"red\"><b>" + mTOD + "</b></font>";
        else
            TODcolor = "<font color = \"blue\"><b>" + mTOD + "</b></font>";
        if (Datas::I()->tono->TOG() > 21)
            TOGcolor = "<font color = \"red\"><b>" + mTOG + "</b></font>";
        else
            TOGcolor = "<font color = \"blue\"><b>" + mTOG + "</b></font>";
        if (Datas::I()->tono->TOD() == 0 && Datas::I()->tono->TOG() > 0)
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("TOG:") + "</b></font></td><td width=\"80\">" + TOGcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td></p>";
        else if (Datas::I()->tono->TOG() == 0 && Datas::I()->tono->TOD() > 0)
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("TOD:") + "</b></font></td><td width=\"80\">" + TODcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td></p>";
        else if (Datas::I()->tono->TOD() == Datas::I()->tono->TOG())
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("TODG:") + "</b></font></td><td width=\"80\">" + TODcolor + tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td></p>";
        else
            Tono = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("TO:") + "</b></font></td><td width=\"80\">" + TODcolor + "/" + TOGcolor+ tr(" à ") + QTime::currentTime().toString("H") + "H</td><td width=\"80\">(" + Methode + ")</td><td>" + m_currentuser->login() + "</td></p>";

    }
    m_htmlMesureTono = Tono;
    logmesure("setHtmlTono() -> new m_htmlMesureTono = " + m_htmlMesureTono);
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la pachymétrie
//--------------------------------------------------------------------------------------
void Procedures::setHtmlPachy()
{
    int D = Datas::I()->pachy->pachyOD();
    int G = Datas::I()->pachy->pachyOG();
    QString mPachyOD        = QLocale().toString(D);
    QString mPachyOG        = QLocale().toString(G);
    QString Pachy        ="";
    mPachyOD = "<font color = \"blue\"><b>" + mPachyOD + "</b></font>";
    mPachyOG = "<font color = \"blue\"><b>" + mPachyOG + "</b></font>";

    if (D > 0 || G > 0)
    {
        if (D == 0 && G > 0)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("PachyOG:") + "</b></font></td><td width=\"80\">" + mPachyOG + "</td><td>" + m_currentuser->login() + "</td></p>";
        else if (D == 0 && G > 0)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("PachyOG:") + "</b></font></td><td width=\"80\">" + mPachyOD + "</td><td>" + m_currentuser->login() + "</td></p>";
        else if (D == G)
            Pachy = "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("PachyODG:") + "</b></font></td><td width=\"80\">" + mPachyOG + "</td><td>" + m_currentuser->login() + "</td></p>";
        else
            Pachy= "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("Pachy:") + "</b></font></td><td width=\"80\">" + mPachyOD + "/" + mPachyOG + "</td><td>" + m_currentuser->login() + "</td></p>";

    }
    m_htmlMesurePachy = Pachy;
}

QString Procedures::HtmlAutoref()
{
    return m_htmlMesureAutoref;
}

QString Procedures::HtmlKerato()
{
    return m_htmlMesureKerato;
}

QString Procedures::HtmlTono()
{
    return m_htmlMesureTono;
}

QString Procedures::HtmlPachy()
{
    return m_htmlMesurePachy;
}

QSerialPort* Procedures::PortTono()
{
    return sp_portTono;
}

Procedures::TypeMesure Procedures::ConvertMesure(QString Mesure)
{
    if (Mesure == "P") return Fronto;
    if (Mesure == "A") return Autoref;
    if (Mesure == "R") return Subjectif;
    if (Mesure == "O") return Final;
    return  None;
}

QString Procedures::ConvertMesure(TypeMesure Mesure)
{
    switch (Mesure) {
    case Fronto:        return "P";
    case Autoref:       return "A";
    case Subjectif:     return "R";
    case Final:         return "O";
    default: return "";
    }
}

//---------------------------------------------------------------------------------
// Calcul de la formule de refraction
//---------------------------------------------------------------------------------
QString Procedures::CalculeFormule(MesureRefraction *ref,  QString Cote)
{
        QString mSphere;
        QString mCyl;
        QString mAxe;
        QString mAdd;
        if (Cote == "D")
        {
            mSphere   = Utils::PrefixePlus(ref->sphereOD());
            mCyl      = Utils::PrefixePlus(ref->cylindreOD());
            mAxe      = QString::number(ref->axecylindreOD());
            mAdd      = Utils::PrefixePlus(ref->addVPOD());
        }
        else if (Cote == "G")
        {
            mSphere   = Utils::PrefixePlus(ref->sphereOG());
            mCyl      = Utils::PrefixePlus(ref->cylindreOG());
            mAxe      = QString::number(ref->axecylindreOG());
            mAdd      = Utils::PrefixePlus(ref->addVPOG());
        }
        else return "";
        QString Resultat;
        if (QLocale().toDouble(mCyl) != 0.00 && QLocale().toDouble(mSphere) != 0.00)
            Resultat = mSphere + " (" + mCyl + QObject::tr(" à ") + mAxe + "°)" ;
        if (QLocale().toDouble(mCyl) == 0.00 && QLocale().toDouble(mSphere) != 0.00)
            Resultat = mSphere ;
        if (QLocale().toDouble(mCyl) != 0.00 && QLocale().toDouble(mSphere) == 0.00)
            Resultat = mCyl + QObject::tr(" à ") + mAxe + "°" ;
        if (QLocale().toDouble(mCyl) == 0.00 && QLocale().toDouble(mSphere) == 0.00)
            Resultat = QObject::tr("plan");
        if (QLocale().toDouble(mAdd) > 0.00)
            Resultat += " add." + mAdd + " VP" ;
        return Resultat;
}

//---------------------------------------------------------------------------------
// Calcul de la formule de refraction
//---------------------------------------------------------------------------------
void Procedures::InsertMesure(TypeMesure typemesure, Tonometrie::Mode  modetono, Pachymetrie::Mode modepachy)
{
    if (Datas::I()->patients->currentpatient() == Q_NULLPTR)
        return;
    if (Datas::I()->patients->currentpatient()->isnull())
        return;
    int idPatient   = Datas::I()->patients->currentpatient()->id();
    int idActe      = Datas::I()->actes->currentacte()->id();
    if (typemesure == Fronto)
    {
        QString mSphereOD, mSphereOG;
        QString mCylOD, mCylOG;
        QString mAxeOD, mAxeOG;
        QString mAddOD, mAddOG;
        mSphereOD       = Utils::PrefixePlus(Datas::I()->mesurefronto->sphereOD());
        mCylOD          = Utils::PrefixePlus(Datas::I()->mesurefronto->cylindreOD());
        mAxeOD          = QString::number(Datas::I()->mesurefronto->axecylindreOD());
        mAddOD          = Utils::PrefixePlus(Datas::I()->mesurefronto->addVPOD());
        mSphereOG       = Utils::PrefixePlus(Datas::I()->mesurefronto->sphereOG());
        mCylOG          = Utils::PrefixePlus(Datas::I()->mesurefronto->cylindreOG());
        mAxeOG          = QString::number(Datas::I()->mesurefronto->axecylindreOG());
        mAddOG          = Utils::PrefixePlus(Datas::I()->mesurefronto->addVPOG());
        foreach (Refraction* ref, Datas::I()->refractions->refractions()->values())
            if (ref->idacte() == idActe
                    && ref->typemesure() == Refraction::Fronto
                    && ref->formuleOD() == CalculeFormule(Datas::I()->mesurefronto,"D")
                    && ref->formuleOG() == CalculeFormule(Datas::I()->mesurefronto,"G"))
                Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(ref->id()));

        QHash<QString, QVariant> listbinds;
        listbinds[CP_IDPAT_REFRACTIONS]                 = idPatient;
        listbinds[CP_IDACTE_REFRACTIONS]                = idActe;
        listbinds[CP_DATE_REFRACTIONS]                  = db->ServerDateTime().date();
        listbinds[CP_TYPEMESURE_REFRACTIONS]            = ConvertMesure(typemesure);
        if (Datas::I()->mesurefronto->addVPOD() > 0 || Datas::I()->mesurefronto->addVPOG() > 0)
            listbinds[CP_DISTANCEMESURE_REFRACTIONS]    = "2";
        if (!Datas::I()->mesurefronto->isnullLOD())
        {
            listbinds[CP_SPHEREOD_REFRACTIONS]          = Datas::I()->mesurefronto->sphereOD();
            listbinds[CP_CYLINDREOD_REFRACTIONS]        = Datas::I()->mesurefronto->cylindreOD();
            listbinds[CP_AXECYLOD_REFRACTIONS]          = Datas::I()->mesurefronto->axecylindreOD();
            if (Datas::I()->mesurefronto->addVPOD() > 0)
                listbinds[CP_ADDVPOD_REFRACTIONS]       = Datas::I()->mesurefronto->addVPOD();
            listbinds[CP_FORMULEOD_REFRACTIONS]         = CalculeFormule(Datas::I()->mesurefronto,"D");
            listbinds[CP_ODMESURE_REFRACTIONS]          = 1;
        }
        if (!Datas::I()->mesurefronto->isnullLOG())
        {
            listbinds[CP_SPHEREOG_REFRACTIONS]          = Datas::I()->mesurefronto->sphereOG();
            listbinds[CP_CYLINDREOG_REFRACTIONS]        = Datas::I()->mesurefronto->cylindreOG();
            listbinds[CP_AXECYLOG_REFRACTIONS]          = Datas::I()->mesurefronto->axecylindreOG();
            if (Datas::I()->mesurefronto->addVPOG() > 0)
                listbinds[CP_ADDVPOG_REFRACTIONS]       = Datas::I()->mesurefronto->addVPOG();
            listbinds[CP_FORMULEOG_REFRACTIONS]         = CalculeFormule(Datas::I()->mesurefronto,"G");
            listbinds[CP_OGMESURE_REFRACTIONS]          = 1;
        }
        if (!Datas::I()->mesurefronto->isnullLOD() && !Datas::I()->mesurefronto->isnullLOG())
            listbinds[CP_PD_REFRACTIONS]                = Datas::I()->mesurefronto->ecartIP();
        Datas::I()->refractions->CreationRefraction(listbinds);
    }
    else if (typemesure == Autoref)
    {
        QString mSphereOD, mSphereOG;
        QString mCylOD, mCylOG;
        QString mAxeOD, mAxeOG;
        QString PD;
        mSphereOD       = Utils::PrefixePlus(Datas::I()->mesureautoref->sphereOD());
        mCylOD          = Utils::PrefixePlus(Datas::I()->mesureautoref->cylindreOD());
        mAxeOD          = QString::number(Datas::I()->mesureautoref->axecylindreOD());
        mSphereOG       = Utils::PrefixePlus(Datas::I()->mesureautoref->sphereOG());
        mCylOG          = Utils::PrefixePlus(Datas::I()->mesureautoref->cylindreOG());
        mAxeOG          = QString::number(Datas::I()->mesureautoref->axecylindreOG());
        PD              = (Datas::I()->mesureautoref->ecartIP() > 0?
                               QString::number(Datas::I()->mesureautoref->ecartIP()) : "null");
        foreach (Refraction* ref, Datas::I()->refractions->refractions()->values())
            if (ref->idacte() == idActe && ref->typemesure() == Refraction::Autoref)
                Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(ref->id()));

        QHash<QString, QVariant> listbinds;
        listbinds[CP_IDPAT_REFRACTIONS]                 = idPatient;
        listbinds[CP_IDACTE_REFRACTIONS]                = idActe;
        listbinds[CP_DATE_REFRACTIONS]                  = db->ServerDateTime().date();
        listbinds[CP_TYPEMESURE_REFRACTIONS]            = ConvertMesure(typemesure);

        listbinds[CP_SPHEREOD_REFRACTIONS]              = Datas::I()->mesureautoref->sphereOD();
        listbinds[CP_CYLINDREOD_REFRACTIONS]            = Datas::I()->mesureautoref->cylindreOD();
        listbinds[CP_AXECYLOD_REFRACTIONS]              = Datas::I()->mesureautoref->axecylindreOD();
        listbinds[CP_FORMULEOD_REFRACTIONS]             = CalculeFormule(Datas::I()->mesureautoref,"D");
        listbinds[CP_ODMESURE_REFRACTIONS]              = 1;

        listbinds[CP_SPHEREOG_REFRACTIONS]              = Datas::I()->mesureautoref->sphereOG();
        listbinds[CP_CYLINDREOG_REFRACTIONS]            = Datas::I()->mesureautoref->cylindreOG();
        listbinds[CP_AXECYLOG_REFRACTIONS]              = Datas::I()->mesureautoref->axecylindreOG();
        listbinds[CP_FORMULEOG_REFRACTIONS]             = CalculeFormule(Datas::I()->mesureautoref,"G");
        listbinds[CP_PD_REFRACTIONS]                    = Datas::I()->mesureautoref->ecartIP();
        listbinds[CP_OGMESURE_REFRACTIONS]              = 1;
        Datas::I()->refractions->CreationRefraction(listbinds);

        QString requete = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and QuelleMesure = '" + ConvertMesure(typemesure) + "'";
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
        if (!m_ok)
            return;
        if (patdata.size()==0)
        {
            requete = "INSERT INTO " TBL_DONNEES_OPHTA_PATIENTS
                    " (" CP_IDPATIENT_DATAOPHTA ", " CP_DATEREFRACTIONOD_DATAOPHTA ", " CP_DATEREFRACTIONOG_DATAOPHTA ", " CP_MESURE_DATAOPHTA ", "
                    CP_SPHEREOD_DATAOPHTA ", " CP_CYLINDREOD_DATAOPHTA ", " CP_AXECYLINDREOD_DATAOPHTA ","
                    CP_SPHEREOG_DATAOPHTA ", " CP_CYLINDREOG_DATAOPHTA ", " CP_AXECYLINDREOG_DATAOPHTA ", " CP_ECARTIP_DATAOPHTA ")"
                    " VALUES (" +
                    QString::number(idPatient)  + ", " +
                    "CURDATE(), CURDATE(), '" +
                    ConvertMesure(typemesure) + "'," +
                    QString::number(QLocale().toDouble(mSphereOD))  + "," +
                    QString::number(QLocale().toDouble(mCylOD))     + "," +
                    mAxeOD     + "," +
                    QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    QString::number(QLocale().toDouble(mCylOG))     + "," +
                    mAxeOG     + "," +
                    PD + ")";

            db->StandardSQL (requete, tr("Erreur de création de données autoref dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
        else
        {
            requete = "UPDATE " TBL_DONNEES_OPHTA_PATIENTS " set "
                    CP_MESURE_DATAOPHTA " = '" + ConvertMesure(typemesure) + "'," +
                    CP_DATEREFRACTIONOD_DATAOPHTA " = CURDATE(), " +
                    CP_DATEREFRACTIONOG_DATAOPHTA " = CURDATE(), " +
                    CP_SPHEREOD_DATAOPHTA " = "      + QString::number(QLocale().toDouble(mSphereOD))  + ", " +
                    CP_CYLINDREOD_DATAOPHTA " = "    + QString::number(QLocale().toDouble(mCylOD))     + ", " +
                    CP_AXECYLINDREOD_DATAOPHTA " = " + mAxeOD + ", " +
                    CP_SPHEREOG_DATAOPHTA " = "      + QString::number(QLocale().toDouble(mSphereOG))  + ", " +
                    CP_CYLINDREOG_DATAOPHTA " = "    + QString::number(QLocale().toDouble(mCylOG))     + ", " +
                    CP_AXECYLINDREOG_DATAOPHTA " = " + mAxeOG + ", " +
                    CP_ECARTIP_DATAOPHTA " = "       + PD +
                    " where " CP_IDPATIENT_DATAOPHTA " = "   + QString::number(idPatient) + " and QuelleMesure = '" + ConvertMesure(typemesure) + "'";

            db->StandardSQL (requete, tr("Erreur de mise à jour de données autoref dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == Kerato)
    {
        QString req = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(Autoref) + "'";
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
        if (!m_ok)
            return;
        if (patdata.size()==0)
        {
            req = "INSERT INTO " TBL_DONNEES_OPHTA_PATIENTS
                    " (" CP_IDPATIENT_DATAOPHTA ", " CP_MESURE_DATAOPHTA ", " CP_DATEKERATO_DATAOPHTA ", " CP_K1OD_DATAOPHTA ", " CP_K2OD_DATAOPHTA ", "
                    CP_AXEKOD_DATAOPHTA ", " CP_K1OG_DATAOPHTA ", " CP_K2OG_DATAOPHTA ", " CP_AXEKOG_DATAOPHTA ", " CP_MODEMESUREKERATO_DATAOPHTA ", "
                    CP_DIOTRIESK1OD_DATAOPHTA ", " CP_DIOTRIESK2OD_DATAOPHTA ", " CP_DIOTRIESK1OG_DATAOPHTA ", " CP_DIOTRIESK2OG_DATAOPHTA ")"
                    " VALUES (" +
                    QString::number(idPatient)  + ", '" +
                    ConvertMesure(Autoref) + "', "
                    "CURDATE(), " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->K1OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->K2OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->axeKOD()))       + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->K1OG(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->K2OG(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->axeKOG()))       + ", " +
                    "'" + ConvertMesure(Autoref) + "', " +
                    (Datas::I()->mesurekerato->isnullLOD() || Datas::I()->mesurekerato->dioptriesK1OD() == 0.0? "null" : QString::number(Datas::I()->mesurekerato->dioptriesK1OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOD() || Datas::I()->mesurekerato->dioptriesK2OD() == 0.0? "null" : QString::number(Datas::I()->mesurekerato->dioptriesK2OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG() || Datas::I()->mesurekerato->dioptriesK1OG() == 0.0? "null" : QString::number(Datas::I()->mesurekerato->dioptriesK1OG(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG() || Datas::I()->mesurekerato->dioptriesK2OG() == 0.0? "null" : QString::number(Datas::I()->mesurekerato->dioptriesK2OG(), 'f', 2)) + ")";

            db->StandardSQL (req, tr("Erreur de création de données de kératométrie  dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
        else
        {
            req = "UPDATE " TBL_DONNEES_OPHTA_PATIENTS " set "
                    CP_DATEKERATO_DATAOPHTA " = CURDATE(), "
                    CP_MODEMESUREKERATO_DATAOPHTA " = '" + ConvertMesure(Autoref) + "'";
            if (!Datas::I()->mesurekerato->isnullLOD())
            {
                req +=
                        ", " CP_K1OD_DATAOPHTA   " = " + QString::number(Datas::I()->mesurekerato->K1OD(), 'f', 2) +
                        ", " CP_K2OD_DATAOPHTA   " = " + QString::number(Datas::I()->mesurekerato->K2OD(), 'f', 2) +
                        ", " CP_AXEKOD_DATAOPHTA " = " + QString::number(Datas::I()->mesurekerato->axeKOD());
                if (Datas::I()->mesurekerato->dioptriesK1OD() > 0)
                    req +=
                            ", " CP_DIOTRIESK1OD_DATAOPHTA " = " + QString::number(Datas::I()->mesurekerato->dioptriesK1OD(), 'f', 2)  +
                            ", " CP_DIOTRIESK2OD_DATAOPHTA " = " + QString::number(Datas::I()->mesurekerato->dioptriesK2OD(), 'f', 2);

            }
            if (!Datas::I()->mesurekerato->isnullLOG())
            {
                req +=
                        ", " CP_K1OG_DATAOPHTA   " = " + QString::number(Datas::I()->mesurekerato->K1OG(), 'f', 2) +
                        ", " CP_K2OG_DATAOPHTA   " = " + QString::number(Datas::I()->mesurekerato->K2OG(), 'f', 2) +
                        ", " CP_AXEKOG_DATAOPHTA " = " + QString::number(Datas::I()->mesurekerato->axeKOG()) ;
                if (Datas::I()->mesurekerato->dioptriesK1OG() > 0)
                    req +=
                            ", " CP_DIOTRIESK1OG_DATAOPHTA " = " + QString::number(Datas::I()->mesurekerato->dioptriesK1OG(), 'f', 2)  +
                            ", " CP_DIOTRIESK2OG_DATAOPHTA " = " + QString::number(Datas::I()->mesurekerato->dioptriesK2OG(), 'f', 2);

            }
            req += " where " CP_IDPATIENT_DATAOPHTA " = "+ QString::number(idPatient) + " and QuelleMesure = '" + ConvertMesure(Autoref) + "'";
            db->StandardSQL (req, tr("Erreur de modification de données de kératométrie dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == Subjectif)
    {
        QString mSphereOD, mSphereOG;
        QString mCylOD, mCylOG;
        QString mAxeOD, mAxeOG;
        QString mAddOD, mAddOG;
        QString mAVLOD, mAVLOG;
        QString mAVPOD, mAVPOG;
        QString PD;
        mSphereOD       = Utils::PrefixePlus(Datas::I()->mesureacuite->sphereOD());
        mCylOD          = Utils::PrefixePlus(Datas::I()->mesureacuite->cylindreOD());
        mAxeOD          = QString::number(Datas::I()->mesureacuite->axecylindreOD());
        mAddOD          = Utils::PrefixePlus(Datas::I()->mesureacuite->addVPOD());
        mAVLOD          = QLocale().toString(Datas::I()->mesureacuite->avlOD().toDouble()*10) + "/10";
        mAVPOD          = Datas::I()->mesureacuite->avpOD();
        mSphereOG       = Utils::PrefixePlus(Datas::I()->mesureacuite->sphereOG());
        mCylOG          = Utils::PrefixePlus(Datas::I()->mesureacuite->cylindreOG());
        mAxeOG          = QString::number(Datas::I()->mesureacuite->axecylindreOG());
        mAddOG          = Utils::PrefixePlus(Datas::I()->mesureacuite->addVPOG());
        mAVLOG          = QLocale().toString(Datas::I()->mesureacuite->avlOG().toDouble()*10) + "/10";
        mAVPOG          = Datas::I()->mesureacuite->avpOG();
        PD              = QString::number(Datas::I()->mesureacuite->ecartIP());
        if (PD == "")
            PD = "null";
        foreach (Refraction* ref, Datas::I()->refractions->refractions()->values())
            if (ref->idacte() == idActe && ref->typemesure() == Refraction::Acuite)
                Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(ref->id()));

        QHash<QString, QVariant> listbinds;
        listbinds[CP_IDPAT_REFRACTIONS]                 = idPatient;
        listbinds[CP_IDACTE_REFRACTIONS]                = idActe;
        listbinds[CP_DATE_REFRACTIONS]                  = db->ServerDateTime().date();
        listbinds[CP_TYPEMESURE_REFRACTIONS]            = ConvertMesure(typemesure);
        listbinds[CP_DISTANCEMESURE_REFRACTIONS]        = ((mAVPOD!="" || mAVPOG!="")? "2" : "L");

        listbinds[CP_SPHEREOD_REFRACTIONS]              = Datas::I()->mesureacuite->sphereOD();
        listbinds[CP_CYLINDREOD_REFRACTIONS]            = Datas::I()->mesureacuite->cylindreOD();
        listbinds[CP_AXECYLOD_REFRACTIONS]              = Datas::I()->mesureacuite->axecylindreOD();
        if (Datas::I()->mesureacuite->addVPOD() > 0)
            listbinds[CP_ADDVPOD_REFRACTIONS]           = Datas::I()->mesureacuite->addVPOD();
        listbinds[CP_FORMULEOD_REFRACTIONS]             = CalculeFormule(Datas::I()->mesureacuite,"D");
        listbinds[CP_AVLOD_REFRACTIONS]                 = QLocale().toString(Datas::I()->mesureacuite->avlOD().toDouble()*10) + "/10";
        listbinds[CP_AVPOD_REFRACTIONS]                 = Datas::I()->mesureacuite->avpOG();
        listbinds[CP_ODMESURE_REFRACTIONS]              = 1;

        listbinds[CP_SPHEREOG_REFRACTIONS]              = Datas::I()->mesureacuite->sphereOG();
        listbinds[CP_CYLINDREOG_REFRACTIONS]            = Datas::I()->mesureacuite->cylindreOG();
        listbinds[CP_AXECYLOG_REFRACTIONS]              = Datas::I()->mesureacuite->axecylindreOG();
        if (Datas::I()->mesureacuite->addVPOG() > 0)
            listbinds[CP_ADDVPOG_REFRACTIONS]           = Datas::I()->mesureacuite->addVPOG();
        listbinds[CP_FORMULEOG_REFRACTIONS]             = CalculeFormule(Datas::I()->mesureacuite,"G");
        listbinds[CP_AVLOG_REFRACTIONS]                 = QLocale().toString(Datas::I()->mesureacuite->avlOG().toDouble()*10) + "/10";
        listbinds[CP_AVPOG_REFRACTIONS]                 = Datas::I()->mesureacuite->avpOG();
        listbinds[CP_OGMESURE_REFRACTIONS]              = 1;

        listbinds[CP_PD_REFRACTIONS]                    = QString::number(Datas::I()->mesureacuite->ecartIP());

        Datas::I()->refractions->CreationRefraction(listbinds);

        QString requete = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(Subjectif) + "'";
        QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
        if (!m_ok)
            return;
        if (patdata.size()==0)
        {
            requete = "INSERT INTO " TBL_DONNEES_OPHTA_PATIENTS
                    " (" CP_IDPATIENT_DATAOPHTA ", " CP_DATEREFRACTIONOD_DATAOPHTA ", " CP_DATEREFRACTIONOG_DATAOPHTA ", " CP_MESURE_DATAOPHTA ", " CP_DISTANCE_DATAOPHTA ", "
                    CP_SPHEREOD_DATAOPHTA ", " CP_CYLINDREOD_DATAOPHTA ", " CP_AXECYLINDREOD_DATAOPHTA ", " CP_ADDVPOD_DATAOPHTA ", " CP_AVLOD_DATAOPHTA ", " CP_AVPOD_DATAOPHTA ", "
                    CP_SPHEREOG_DATAOPHTA ", " CP_CYLINDREOG_DATAOPHTA ", " CP_AXECYLINDREOG_DATAOPHTA ", " CP_ADDVPOG_DATAOPHTA ", " CP_AVLOG_DATAOPHTA ", " CP_AVPOG_DATAOPHTA ", "
                    CP_ECARTIP_DATAOPHTA ")"
                                         " VALUES (" +
                    QString::number(idPatient)  + ", " +
                    "CURDATE(), CURDATE(), '" +
                    ConvertMesure(typemesure)               + "','" +
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

            db->StandardSQL(requete, tr("Erreur création de données de refraction dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
        else
        {
            requete = "UPDATE " TBL_DONNEES_OPHTA_PATIENTS " set "
                    CP_MESURE_DATAOPHTA " = '"      + ConvertMesure(typemesure) + "', "
                    CP_DISTANCE_DATAOPHTA " = '"    + ((mAVPOD!="" || mAVPOG!="")? "2" : "L") + "', "
                    CP_DATEREFRACTIONOD_DATAOPHTA " = CURDATE(),"
                    CP_DATEREFRACTIONOG_DATAOPHTA " = CURDATE()," +
                    CP_SPHEREOD_DATAOPHTA " = "     + QString::number(QLocale().toDouble(mSphereOD))  + ","
                    CP_CYLINDREOD_DATAOPHTA " = "   + QString::number(QLocale().toDouble(mCylOD))     + "," +
                    CP_AXECYLINDREOD_DATAOPHTA " = " + mAxeOD + "," +
                    CP_ADDVPOD_DATAOPHTA " = "      + (QLocale().toDouble(mAddOD)>0? QString::number(QLocale().toDouble(mAddOD)) : "null") + "," +
                    CP_AVLOD_DATAOPHTA " = '"       + mAVLOD + "'," +
                    CP_AVPOD_DATAOPHTA " = '"       + mAVPOD + "'," +
                    CP_SPHEREOG_DATAOPHTA " = "     + QString::number(QLocale().toDouble(mSphereOG))  + "," +
                    CP_CYLINDREOG_DATAOPHTA " = "   + QString::number(QLocale().toDouble(mCylOG))     + "," +
                    CP_AXECYLINDREOG_DATAOPHTA " = " + mAxeOG + "," +
                    CP_ADDVPOG_DATAOPHTA " = "      + (QLocale().toDouble(mAddOG)>0? QString::number(QLocale().toDouble(mAddOG)) : "null") + "," +
                    CP_AVLOG_DATAOPHTA " = '"       + mAVLOG + "'," +
                    CP_AVPOG_DATAOPHTA " = '"       + mAVPOG + "'," +
                    CP_ECARTIP_DATAOPHTA " = "      + PD +
                    " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(Subjectif) + "'";

            db->StandardSQL (requete, tr("Erreur de mise à jour de données de refraction dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == Tono)
    {
        QString req = "INSERT INTO " TBL_TONOMETRIE " (idPat, TOOD, TOOG, TODate, TOType) VALUES  ("
                + QString::number(Datas::I()->patients->currentpatient()->id()) + ","
                + QString::number(Datas::I()->tono->TOD()) + ","
                + QString::number(Datas::I()->tono->TOG())
                + ", now(), '" + Tonometrie::ConvertMesure(modetono) + "')";
        DataBase::I()->StandardSQL(req,tr("Impossible de sauvegarder la mesure!"));
    }
    else if (typemesure == Pachy)
    {
        QString req = "INSERT INTO " TBL_PACHYMETRIE " (idPat, pachyOD, pachyOG, pachyDate, pachyType) VALUES  ("
                + QString::number(Datas::I()->patients->currentpatient()->id()) + ","
                + QString::number(Datas::I()->pachy->pachyOD()) + ","
                + QString::number(Datas::I()->pachy->pachyOG())
                + ", now(), '" + Pachymetrie::ConvertMesure(modepachy) + "')";
        DataBase::I()->StandardSQL(req,tr("Impossible de sauvegarder la mesure!"));
    }
    if (typemesure != Fronto && typemesure != Tono && typemesure != Pachy)
        Datas::I()->patients->actualiseDonneesOphtaCurrentPatient();
}
