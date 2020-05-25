/* (C) 2020 LAINE SERGE
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

    QFile FichierIni(PATH_FILE_INI);
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
            QString msgInfo =   tr("Le fichier d'initialisation\n\"") + PATH_FILE_INI "\"\n" + tr("n'existe pas.\n"
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
            a = VerifIni(msg, msgInfo, true, true, true, true);
        }
    }
    m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
    QSet<int> ports = QSet<int>::fromList(QList<int>() << 3306 << 3307);
    bool k =    (
                  m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Active").toString() == "YES"
                  &&
                  ( ports.find(m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Port").toInt()) != ports.end() )
                )
                ||
                (
                  m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Active").toString() == "YES"
                  && m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Serveur").toString() != ""
                  && ( ports.find(m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Port").toInt()) != ports.end() )
                )
                ||
                (
                  m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Active").toString() == "YES"
                  && m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Serveur").toString() != ""
                  && ( ports.find(m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Port").toInt()) != ports.end() )
                );
    if (!k)
    {
        while (!k)
        {
            QString msg =       tr("Le fichier d'initialisation de l'application est corrompu\n");
            QString msgInfo =   tr("Le fichier d'initialisation\n\"") + PATH_FILE_INI "\"\n" +
                    tr("ne contient pas de renseignement valide\n"
                       "permettant la connexion à la base de données.\n\n"
                       "Ce fichier est indispensable au bon fonctionnement de l'application.\n\n"
                       "1. Si vous disposez d'une sauvegarde du fichier, choisissez \"Restaurer le fichier à partir d'une sauvegarde\"\n"
                       "2. Sinon cliquez sur \"Reconstruire le fichier d'initialisation\" et suivez les étapes de la reconstruction."
                       " Il vous faudra alors compléter de nouveau"
                       " les renseignements concernant les appareils connectés au réseau ou à ce poste d'examen après"
                       " le démarrage complet du logiciel (Menu Edition/Paramètres).\n");
            m_connexionbaseOK = k;
            k = VerifIni(msg, msgInfo, false, true, true, false);
        }
    }

    m_nomImprimante  = "";

    Ouverture_Ports_Series();
    ReconstruitListeModesAcces();
    m_typemesureRefraction               = None;
    m_dlgrefractionouverte    = false;
    int margemm         = TailleTopMarge(); // exprimé en mm
    p_printer             = new QPrinter(QPrinter::HighResolution);
    p_printer             ->setFullPage(true);
    m_rect                = p_printer->paperRect();
    m_rect.adjust(Utils::mmToInches(margemm) * p_printer->logicalDpiX(),
                  Utils::mmToInches(margemm) * p_printer->logicalDpiY(),
                - Utils::mmToInches(margemm) * p_printer->logicalDpiX(),
                - Utils::mmToInches(margemm) * p_printer->logicalDpiY());
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
QMap<Utils::Period, QDate> Procedures::ChoixDate(QWidget *parent)
{
    dlg_choixdate *Dlg_ChxDate = new dlg_choixdate(parent);
    Dlg_ChxDate ->setWindowTitle(tr("Choisir une période"));
    Dlg_ChxDate ->exec();
    QMap<Utils::Period, QDate> DateMap = Dlg_ChxDate->mapdate();
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
    /*! avant la mise à jour 61, on ne peut pas utiliser Datas::I()->users->initListe() parce que le champ DateCreationMDP de la table utilisateurs n'existe pas */
    if (Datas::I()->users->all()->isEmpty())
        Datas::I()->users       ->initShortListe();
    Datas::I()->postesconnectes->initListe();
    int id = 0;
    if (Datas::I()->users->userconnected() != Q_NULLPTR)
        id = Datas::I()->users->userconnected()->id();
    QString stringid = Utils::MACAdress() + " - " + QString::number(id);
    foreach (PosteConnecte *post, Datas::I()->postesconnectes->postesconnectes()->values())
    {
        if (post->stringid() != stringid)
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
        DataDir = Utils::dir_size(pathorigin + NOM_DIR_VIDEOS);
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
        DataDir = Utils::dir_size(pathorigin + NOM_DIR_IMAGES);
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
        DataDir = Utils::dir_size(pathorigin + NOM_DIR_FACTURES);
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
            Fctureschk->setEnabled(OKfactures || op == BackupOp);
            Fctureschk->setChecked(OKfactures || op == BackupOp);
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

bool Procedures::Backup(QString pathdirdestination, bool OKBase, bool OKImages, bool OKVideos, bool OKFactures, bool verifmdp)
{
    auto result = [] (qintptr handle, Procedures *proc)
    {
        ShowMessage::I()->ClosePriorityMessage(handle);
        proc->emit ConnectTimers(true);
    };
    if (QDir(m_parametres->dirimagerieserveur()).exists())
    {
        Utils::cleanfolder(m_parametres->dirimagerieserveur() + NOM_DIR_IMAGES);
        Utils::cleanfolder(m_parametres->dirimagerieserveur() + NOM_DIR_FACTURES);
        Utils::cleanfolder(m_parametres->dirimagerieserveur() + NOM_DIR_VIDEOS);
    }
    else
    {
        OKImages = false;
        OKVideos = false;
        OKFactures = false;
    }

    QString msgEchec = tr("Incident pendant la sauvegarde");
    qintptr handledlg = 0;
    if (verifmdp)
        if (!Utils::VerifMDP(MDPAdmin(),tr("Saisissez le mot de passe Administrateur")))
            return false;
    ShowMessage::I()->PriorityMessage(tr("Sauvegarde en cours"),handledlg);
    emit ConnectTimers(false);

    if (OKBase)
    {
        QFile::remove(PATH_FILE_SCRIPTBACKUP);
        DefinitScriptBackup(pathdirdestination, OKImages, OKVideos, OKFactures);
        QString Msg = (tr("Sauvegarde de la base de données\n")
                       + tr("Ce processus peut durer plusieurs minutes en fonction de la taille de la base de données"));
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
        const QString task = "sh " + PATH_FILE_SCRIPTBACKUP;
        const QString msgOK = tr("Base de données sauvegardée!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller, &Controller::result, this, [=](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            if (OKImages)
                Utils::cleanfolder(pathdirdestination + NOM_DIR_IMAGES);
            if (OKFactures)
                Utils::cleanfolder(pathdirdestination + NOM_DIR_FACTURES);
            if (OKVideos)
                Utils::cleanfolder(pathdirdestination + NOM_DIR_VIDEOS);
            result(handledlg, this);
            QFile::remove(PATH_FILE_SCRIPTBACKUP);
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
        ShowMessage::I()->ClosePriorityMessage(handle);
        proc->emit ConnectTimers(true);
    };
    QString msgEchec = tr("Incident pendant la sauvegarde");
    if (factures) {
        QString Msg = (tr("Sauvegarde des factures\n")
                       + tr("Ce processus peut durer plusieurs minutes en fonction de la taille des fichiers"));
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
        const QString task = "cp -R " + m_parametres->dirimagerieserveur() + NOM_DIR_FACTURES + " " + dirdestination;
        const QString msgOK = tr("Fichiers factures sauvegardés!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller,
                &Controller::result,
                this,
                [=, &factures](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            Utils::cleanfolder(dirdestination + NOM_DIR_FACTURES);
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
        const QString task = "cp -R " + m_parametres->dirimagerieserveur() + NOM_DIR_IMAGES + " " + dirdestination;
        const QString msgOK = tr("Fichiers d'imagerie sauvegardés!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller,
                &Controller::result,
                this,
                [=, &images](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            Utils::cleanfolder(dirdestination + NOM_DIR_IMAGES);
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
        const QString task = "cp -R " + m_parametres->dirimagerieserveur() + NOM_DIR_VIDEOS + " " + dirdestination;
        const QString msgOK = tr("Fichiers videos sauvegardés!");
        m_controller.disconnect(SIGNAL(result(const int &)));
        connect(&m_controller,
                &Controller::result,
                this,
                [=, &videos](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msgOK : msgEchec), Icons::icSunglasses(), 3000);
            Utils::cleanfolder(dirdestination + NOM_DIR_VIDEOS);
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
    scriptbackup += "DIR_RESSOURCES=\"" + PATH_DIR_RESSOURCES + "\"";
    scriptbackup += "\n";
    if (QDir(m_parametres->dirimagerieserveur()).exists())
    {
        if (AvecImages)
        {
            scriptbackup += "DIR_IMAGES=\"" + m_parametres->dirimagerieserveur() + NOM_DIR_IMAGES + "\"";
            scriptbackup += "\n";
        }
        if (AvecFactures)
        {
            scriptbackup += "DIR_FACTURES=\"" + m_parametres->dirimagerieserveur() + NOM_DIR_FACTURES + "\"";
            scriptbackup += "\n";
        }
        if (AvecVideos)
        {
            scriptbackup += "DIR_VIDEOS=\"" + m_parametres->dirimagerieserveur() + NOM_DIR_VIDEOS + "\"";
            scriptbackup += "\n";
        }
    }
    //# Rufus.ini
    scriptbackup += "RUFUSINI=\"" + PATH_FILE_INI "\"";
    //# Identifiants MySQL
    scriptbackup += "\n";
    scriptbackup += "MYSQL_USER=\"" LOGIN_SQL "\"";
    scriptbackup += "\n";
    scriptbackup += "MYSQL_PASSWORD=\"" MDP_SQL "\"";
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
    if (QDir(m_parametres->dirimagerieserveur()).exists())
    {
        //! copie les fichiers image
        if (AvecImages)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR" NOM_DIR_IMAGES;
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_IMAGES $BACKUP_DIR";
            scriptbackup += "\n";
        }
        if (AvecFactures)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR" NOM_DIR_FACTURES;
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_FACTURES $BACKUP_DIR";
            scriptbackup += "\n";
        }
        //! copie les fichiers video
        if (AvecVideos)
        {
            scriptbackup += "mkdir -p $BACKUP_DIR" NOM_DIR_VIDEOS;
            scriptbackup += "\n";
            scriptbackup += "cp -R -f $DIR_VIDEOS $BACKUP_DIR";
            scriptbackup += "\n";
        }
    }
    // copie Rufus.ini
    scriptbackup +=  "cp $RUFUSINI $BACKUP_DIR/$DATE" NOM_FILE_INI;
    if (QFile::exists(PATH_FILE_SCRIPTBACKUP))
        QFile::remove(PATH_FILE_SCRIPTBACKUP);
    QFile fbackup(PATH_FILE_SCRIPTBACKUP);
    if (fbackup.open(QIODevice::ReadWrite))
    {
        QTextStream out(&fbackup);
        out << scriptbackup ;
        fbackup.close();
    }
}

/*!
 * \brief Procedures::DefinitScriptRestore
 * \param ListNomFiles
 */
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
    QString host;
    if( db->ModeAccesDataBase() == Utils::Poste )
        host = "localhost";
    else
        host = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + "/Serveur").toString();
    bool useSSL = (db->ModeAccesDataBase() == Utils::Distant);
    QString login = LOGIN_SQL;
    if (useSSL)
        login += "SSL";
    QString dirkey = "/etc/mysql";
    QString keys = "";
    if (useSSL)
    {
        if (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL").toString() != "")
            dirkey = m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL").toString();
        else
            m_settings->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL",dirkey);
        keys += " --ssl-ca=" + dirkey + "/ca-cert.pem --ssl-cert=" + dirkey + "/client-cert.pem --ssl-key=" + dirkey + "/client-key.pem";
    }
    for (int i=0; i<ListNomFiles.size(); i++)
        if (QFile(ListNomFiles.at(i)).exists())
        {
            scriptrestore += "$MYSQL -u " + login + " -p" MDP_SQL " -h " + host + " -P " + QString::number(db->port()) + keys + " < " + ListNomFiles.at(i);
            scriptrestore += "\n";
        }
    if (QFile::exists(PATH_FILE_SCRIPTRESTORE))
        QFile::remove(PATH_FILE_SCRIPTRESTORE);
    QFile fbackup(PATH_FILE_SCRIPTRESTORE);
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
        OKImages = QDir(m_parametres->dirimagerieserveur()).exists();
        OKVideos = QDir(m_parametres->dirimagerieserveur()).exists();
        OKFactures = QDir(m_parametres->dirimagerieserveur()).exists();
    }
    else
    {
        AskBupRestore(BackupOp, m_parametres->dirimagerieserveur(), dirdestination );
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
    return Backup(dirdestination, OKbase, OKImages, OKVideos, OKFactures, true);
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
    if (QFile::exists(PATH_FILE_SCRIPTBACKUP))
        QFile::remove(PATH_FILE_SCRIPTBACKUP);
    t_timerbackup.disconnect(SIGNAL(timeout()));
    t_timerbackup.stop();
    /*! la suite n'est plus utilisée depuis OsX Catalina parce que OsX Catalina n'accepte plus les launchagents
#ifdef Q_OS_MACX
    QString file = PATH_FILE_SCRIPT_MACOS_PLIST;                          //! file = "/Users/xxxx/Library/LaunchAgents/rufus.bup.plist"
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
                                "\t\t\t<string>" + PATH_FILE_SCRIPTBACUP + "</string>\n"
                            "\t\t</array>\n"
                            "\t\t<key>StartCalendarInterval</key>\n"
                            + jourprg +
                        "\t</dict>\n"
                    "</plist>\n";
    if (QFile::exists(PATH_FILE_SCRIPT_MACOS_PLIST))
        QFile::remove(PATH_FILE_SCRIPT_MACOS_PLIST);
    QFile fplist(PATH_FILE_SCRIPT_MACOS_PLIST);
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

/*---------------------------------------------------------------------------------
    Retourne le corps du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::CalcCorpsImpression(QString text, bool ALD)
{
    QString Corps;
    QString nomModeleCorpsImpression;
    Utils::convertHTML(text);
    if (ALD)
        nomModeleCorpsImpression = PATH_FILE_CORPSORDOALD;
    else
        nomModeleCorpsImpression = PATH_FILE_CORPSORDO;

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
    EnteteMap["Norm"]   = "";
    EnteteMap["ALD"]    = "";
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
        if (user->id() == currentuser()->idsuperviseur())
        {
            // si le user rplct à imprimer est le superviseur du user courant, on récupère le parent du user courant
            idparent = currentuser()->idparent();
        }
        else
        {
            // si le user rplct à imprimer n'est pas le superviseur du user courant, on cherche son parent
            QString reqrp = "select userparent from " TBL_USERSCONNECTES
                            " where usersuperviseur = " + QString::number(user->id());
            QVariantList userdata = db->getFirstRecordFromStandardSelectSQL(reqrp, m_ok);
            if (userdata.size()>0)                // le user est connecté, on cherche qui il remplace - son parent
                idparent = userdata.at(0).toInt();
            else                                // le user n'est pas connecté on demande quel est son parent
            {
                QVariantList soigndata = db->getFirstRecordFromStandardSelectSQL("select " CP_SOIGNANTSTATUS_USR " from " TBL_UTILISATEURS " where " CP_ID_USR " = " + QString::number(user->id()), m_ok);
                QString req   = "select " CP_ID_USR ", " CP_LOGIN_USR " from " TBL_UTILISATEURS
                        " where (" CP_ENREGHONORAIRES_USR " = 1 or " CP_ENREGHONORAIRES_USR " = 2)"
                        " and " CP_ID_USR " <> " + QString::number(user->id()) +
                        " and " CP_SOIGNANTSTATUS_USR " = " + soigndata.at(0).toString() +
                        " and " CP_ISDESACTIVE_USR " is null";
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
            nomModeleEntete = PATH_FILE_ENTETEORDO;
        else
            nomModeleEntete = PATH_FILE_ENTETEORDOALD;
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
            User *userRemp = Datas::I()->users->getById(idparent);
            if(userRemp)
                Entete.replace("{{TITREUSER}}", "<s>" + userRemp->titre() + " " + userRemp->prenom() + " " + userRemp->nom() + "</s> "
                                                "<font color=\"darkblue\">" + tr ("remplacé par") + " "
                                                + (user->titre().size()? user->titre() + " " : "") + user->prenom() + " " + user->nom())
                                                + "</font>";
        }
        else
            Entete.replace("{{TITREUSER}}", (user->titre() != ""? user->titre() + " " : "") + user->prenom() + " " + user->nom());

        if(user->numspecialite() != 0)
            Entete.replace("{{SPECIALITE}}", QString::number(user->numspecialite()) + " " + user->specialite());
        else
            Entete.replace("{{SPECIALITE}}", user->specialite());

        QString adresse ="";
        int nlignesadresse = 0;
        Site *sit = Datas::I()->sites->currentsite();
        if (user != currentuser())
        {
            QList<Site*> listsites = Datas::I()->sites->initListeByUser(user->id());
            if (listsites.size()>0)
                sit = listsites.first(); //TODO ça ne va pas parce qu'on prend arbitrairement la première adreesse
            else {
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'imprimer"), tr("Pas de site de travail référencé pour l'utilisateur ") + user->nom());
                return EnteteMap;
            }
        }
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
        QString nomModelePied = PATH_FILE_PIEDPAGE;
        if (lunettes)
            nomModelePied = PATH_FILE_PIEDPAGEORDOLUNETTES;
        QFile   qFilePied(nomModelePied );
        while (!qFilePied.open( QIODevice::ReadOnly ))
            if (!VerifRessources(nomModelePied))
                return QString();
        long filePied_len = qFilePied.size();
        QByteArray baPied = qFilePied.readAll();
        baPied.resize(filePied_len + 1);
        baPied.data()[filePied_len] = 0;
        qFilePied.close ();
        if (user)
        {
            if( user->isAGA() )
                baPied.replace("{{AGA}}","Membre d'une association de gestion agréée - Le règlement des honoraires par chèque ou carte de crédit est accepté");
        }
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
        a = TexteAImprimer->preview(Etat->document(), PATH_FILE_PDF, "");
    else
    {
        if (!AvecChoixImprimante)
            TexteAImprimer->setPrinterName(m_nomImprimante);
        a = TexteAImprimer->print(Etat->document(), PATH_FILE_PDF, "", AvecChoixImprimante);
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
                    imgs = "select " CP_ID_FACTURES " from " TBL_FACTURES " where " CP_ID_FACTURES " = " + iditem + " and (" CP_PDF_FACTURES " is not null or " CP_JPG_FACTURES " is not null)";
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
                                " LOAD_FILE('" + Utils::correctquoteSQL(m_parametres->dirimagerieserveur() + NOM_DIR_IMAGES + Utils::correctquoteSQL(filename)) + "'), " +
                                QString::number(docmt->compression()) + ")";
                        db->StandardSQL(req);
                        //qDebug() << req;
                    }
                    else
                    {
                        db->StandardSQL("delete from " TBL_ECHANGEIMAGES
                                                             " where idimpression = " + iditem +
                                                             " and facture = 1");
                        QString req = "INSERT INTO " TBL_ECHANGEIMAGES " (idimpression, " + sfx + ", facture) "
                                      "VALUES (" +
                                      iditem + ", " +
                                      " LOAD_FILE('" + Utils::correctquoteSQL(m_parametres->dirimagerieserveur() + NOM_DIR_FACTURES + Utils::correctquoteSQL(filename)) + "'), " +
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
                listimpr = db->StandardSelectSQL("select " CP_PDF_DOCSEXTERNES ", " CP_JPG_DOCSEXTERNES ", " CP_COMPRESSION_DOCSEXTERNES "  from " TBL_DOCSEXTERNES " where " CP_ID_DOCSEXTERNES " = " + iditem
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
                listimpr = db->StandardSelectSQL("select " CP_PDF_FACTURES ", " CP_JPG_FACTURES "  from " TBL_FACTURES " where " CP_ID_FACTURES " = " + iditem
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
        QString ficpdf = PATH_FILE_PDF;
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
    wdg_inflabel->setGeometry(10,htable-40,350,25);

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
        QList<QImage> listimg = Utils::calcImagefromPdf(ba);
        if (listimg.size())
        {
            for (int i=0; i<listimg.size();++i)
            {
                QImage image = listimg.at(i);
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
                            return false;
                        }
                    }
                }
                else
                    Print(p_printer, image);
            }
        }
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

bool Procedures::Imprimer_Document(Patient *pat, User * user, QString titre, QString Entete, QString text, QDate date,
                                                                          bool Prescription, bool ALD, bool AvecPrevisu, bool AvecDupli, bool AvecChoixImprimante, bool Administratif)
{
    if (pat == Q_NULLPTR || user == Q_NULLPTR)
        return false;
    QString     Corps, Pied;
    QTextEdit   *Etat_textEdit = new QTextEdit;
    bool        AvecNumPage = false;
    bool        aa;

    Entete.replace("{{PRENOM PATIENT}}", (Prescription? pat->prenom()        : ""));
    Entete.replace("{{NOM PATIENT}}"   , (Prescription? pat->nom().toUpper() : ""));

    //création du pied
    Pied = CalcPiedImpression(user, false, ALD);
    if (Pied == "") return false;

    // creation du corps
    Corps = CalcCorpsImpression(text, ALD);
    if (Corps == "") return false;
    Etat_textEdit->setHtml(Corps);
    if (Etat_textEdit->toPlainText() == "") return false;

    int tailleEnTete = TailleEnTete();
    if (ALD) tailleEnTete = TailleEnTeteALD();
    aa = Imprime_Etat(Etat_textEdit, Entete, Pied,
                            TaillePieddePage(), tailleEnTete, TailleTopMarge(),
                            AvecDupli, AvecPrevisu, AvecNumPage, AvecChoixImprimante);

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        Utils::nettoieHTML(Corps);

        int idpat = 0;
        idpat = pat->id();

        QHash<QString, QVariant> listbinds;
        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        listbinds[CP_IDUSER_DOCSEXTERNES]        = user->id();
        listbinds[CP_IDPAT_DOCSEXTERNES]         = idpat;
        listbinds[CP_TYPEDOC_DOCSEXTERNES]       = (Prescription? "Prescription" : "Courrier");
        listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES]   = titre;
        listbinds[CP_TITRE_DOCSEXTERNES]         = titre;
        listbinds[CP_TEXTENTETE_DOCSEXTERNES]    = Entete;
        listbinds[CP_TEXTCORPS_DOCSEXTERNES]     = Corps;
        listbinds[CP_TEXTORIGINE_DOCSEXTERNES]   = text;
        listbinds[CP_TEXTPIED_DOCSEXTERNES]      = Pied;
        listbinds[CP_DATE_DOCSEXTERNES]          = date.toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds[CP_IDEMETTEUR_DOCSEXTERNES]    = Datas::I()->users->userconnected()->id();
        listbinds[CP_ALD_DOCSEXTERNES]           = (ALD? "1": QVariant(QVariant::String));
        listbinds[CP_EMISORRECU_DOCSEXTERNES]    = "0";
        listbinds[CP_FORMATDOC_DOCSEXTERNES]     = (Prescription? PRESCRIPTION : (Administratif? COURRIERADMINISTRATIF : COURRIER));
        listbinds[CP_IDLIEU_DOCSEXTERNES]        = Datas::I()->sites->idcurrentsite();
        listbinds[CP_IMPORTANCE_DOCSEXTERNES]    = (Administratif? "0" : "1");
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if(doc != Q_NULLPTR)
            delete doc;
    }
    delete Etat_textEdit;
    return aa;
}

void Procedures::Print(QPrinter *Imprimante, QImage image)
{
    QPainter PrintingPreView(Imprimante);
    QPixmap pix = QPixmap::fromImage(image).scaledToWidth(int(m_rect.width()),Qt::SmoothTransformation);
    PrintingPreView.drawImage(QPoint(0,0),pix.toImage());
}

/*-----------------------------------------------------------------------------------------------------------------
-- Accesseurs  ----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool Procedures::ApercuAvantImpression()
{
    return (m_settings->value("Param_Imprimante/ApercuAvantImpression").toString() == "YES");
}

QString Procedures::CodePostalParDefaut()
{
    QSettings set(PATH_FILE_INI, QSettings::IniFormat);
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

    bool ophtalmo       = currentuser()->isOpthalmo();
    bool orthoptist     = currentuser()->isOrthoptist();
    bool autresoignant  = currentuser()->isAutreSoignant();
    bool soccomptable   = currentuser()->isSocComptable();
    bool medecin        = currentuser()->isMedecin();

    bool assistant      = currentuser()->isAssistant();
    bool responsable    = currentuser()->isResponsable();
    bool responsableles2= currentuser()->isResponsableOuAssistant();

    bool liberal        = currentuser()->isLiberal();
    bool pasliberal     = currentuser()->isSalarie();
    bool retrocession   = currentuser()->isRemplacant();
    bool pasdecompta    = currentuser()->isSansCompta();

    bool cotation       = currentuser()->useCCAM();

    bool soignant           = currentuser()->isSoignant();
    bool soigntnonassistant = soignant && !assistant;
    bool respsalarie        = soigntnonassistant && pasliberal;
    bool respliberal        = soigntnonassistant && liberal;


    QString txtstatut = tr("Vos données enregistrées pour cette session")+ "\n\n" + currentuserstatus();
    txtstatut += "\n" + tr("Site") + "\t\t= " + Datas::I()->sites->currentsite()->nom();
    txtstatut += "\n\n";

    txtstatut += tr("Vos données permanentes") +"\n\n" + tr("Fonction :") + "\t\t\t";
    if (ophtalmo)
        txtstatut += tr("Ophtalmologiste");
    else if (orthoptist)
        txtstatut += tr("Orthoptiste");
    else if (autresoignant)
        txtstatut += currentuser()->specialite();
    else
        txtstatut += currentuser()->fonction();

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
        txtstatut += "\n" + tr("RPPS :\t\t\t") + QString::number(currentuser()->NumPS());
    if (medecin && ! assistant)
        txtstatut += "\nADELI :\t\t\t" + currentuser()->numOrdre();
    User *employeur = Datas::I()->users->getById(currentuser()->idemployeur());
    if (soignant)
    {
        txtstatut += "\n" + tr("Exercice :\t\t\t");
        if (liberal)
            txtstatut += tr("libéral");
        else if (pasliberal)
        {
            QString txtsalarie = tr("salarié");
            txtsalarie += " - " + tr("Employeur : ") + (employeur? employeur->login() : "null");
            txtstatut += txtsalarie;
        }
        else if (retrocession)
            txtstatut += tr("remplaçant");
        else if (pasdecompta)
            txtstatut += tr("sans comptabilité");
    }
    if (respliberal)
    {
        QString txtliberal ("");
        Compte * cptencaissement = Datas::I()->comptes->getById(currentuser()->idcompteencaissementhonoraires());
        if (cptencaissement)
        {
            txtliberal +=  "\n" + tr("Honoraires encaissés sur le compte :\t") + cptencaissement->nomabrege();
            if (Datas::I()->users->getById(currentuser()->idcomptable()) != Q_NULLPTR)
                txtliberal += tr("de") + " " + Datas::I()->users->getById(currentuser()->idcomptable())->login();
        }
        txtstatut += txtliberal;
    }
    else if (respsalarie)
    {
        if (employeur)
        {
            Compte *cptemployeur = Datas::I()->comptes->getById(employeur->idcompteencaissementhonoraires());
            if (cptemployeur)
            {
                txtstatut += "\n" + tr("Honoraires encaissés sur le compte :\t");
                txtstatut += cptemployeur->nomabrege() + " ";
                txtstatut += tr("de") + " " + employeur->login();
            }
        }
    }
    else if (retrocession)
        txtstatut += "\n" + tr("Statut :\t\t\t") + tr("remplaçant");
    if (soigntnonassistant && cotation)
        txtstatut += "\n" + tr("Cotation des actes :\t\t") + (cotation? tr("Oui") : tr("Sans"));
    if (medecin && cotation)
    {
        QString secteur ("");
        switch (currentuser()->secteurconventionnel()) {
        case 1:     secteur = "1";          break;
        case 2:     secteur = "2";          break;
        case 3:     secteur = "3";          break;
        default:
            break;
        }
        txtstatut += "\n" + tr("Secteur conventionnel :\t\t") + secteur;
        txtstatut += "\n" + tr("OPTAM :\t\t\t") + (currentuser()->isOPTAM() ? "Oui": "Non");
    }
    if (respliberal || soccomptable)
    {
        QString cptabledefaut ("");
        Compte *cpt = Datas::I()->comptes->getById(currentuser()->idcomptepardefaut());
        if (cpt)
        {
            User *usrcptble = Datas::I()->users->getById(cpt->idUser());
            cptabledefaut = tr("de") + " " + (usrcptble? usrcptble->login() : "null");
            txtstatut += "\n" + tr("Comptabilité enregistrée sur compte :\t")
                         + cpt->nomabrege() + " "
                         + cptabledefaut;
        }
    }
    if (respliberal)
        txtstatut += "\n" + tr("Membre d'une AGA :\t\t") + (currentuser()->isAGA() ? tr("Oui") : tr("Sans"));
    return txtstatut;
}

/*! --------------------------------------------------------------------------------------------------------------------------------------
    -- détermine le dossier où est stockée l'imagerie -----------------------------------------------------------
    DirStockageImages           = l'emplacement du dossier de l'imagerie sur le poste                   quand on est en mode poste
                                = l'emplacement du dossier de l'imagerie sur le serveur vu par le poste quand on est en mode réseau local
                                = l'emplacement de dossier des copies des images d'origine sur le poste quand on est en mode distant
                                -> utilisé par les postes pour enregistrer une copie de sauvegarde de l'original des fichiers images intégrés dans la base
    DirStockageImagesServeur    = l'emplacement du dossier de l'imagerie sur le serveur - correspond au champ dirimagerie de la table parametressysteme
                                -> utilisé par les requêtes SQL pour réintégrer le contenu de fichiers images dans la base
    ------------------------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- renvoie le chemin du dossier où est stockée l'imagerie -----------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::AbsolutePathDirImagerie()
{
    QString path = "";
    if (db->ModeAccesDataBase() == Utils::Poste)
        path = m_parametres->dirimagerieserveur();
    else
        path = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + "/DossierImagerie").toString();
    return path;
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

QString Procedures::MDPAdmin()
{
    if (m_parametres == Q_NULLPTR)
        m_parametres = db->parametres();
    if (m_parametres == Q_NULLPTR)
        return Utils::calcSHA1(MDP_ADMINISTRATEUR);
    if (m_parametres->mdpadmin() == "")
        db->setmdpadmin(Utils::calcSHA1(MDP_ADMINISTRATEUR));
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
    QSettings set(PATH_FILE_INI, QSettings::IniFormat);
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
        QFile FichierIni(PATH_FILE_INI);
        if (FichierIni.exists())
        {
            QFile FichierBup(PATH_DIR_RUFUS + "/RufusBackup.ini");
            if (FichierBup.exists())
                FichierBup.remove();
            FichierIni.copy(PATH_DIR_RUFUS + "/RufusBackup.ini");
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
        if (db->ModeAccesDataBase() == Utils::Poste)
            Hote = tr("ce poste");
        else
            Hote = tr("le serveur ") + m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + "/Serveur").toString();
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

        if (!Utils::VerifMDP((PremierDemarrage? Utils::calcSHA1(MDP_ADMINISTRATEUR) : MDPAdmin()),tr("Saisissez le mot de passe Administrateur")))
            return false;

        QDir dirtorestore(PATH_DIR_RESSOURCES);
        //qDebug() << dirtorestore.absolutePath();
        QStringList listfichiers = dirtorestore.entryList(QStringList() << "*.sql");
        for (int t=0; t<listfichiers.size(); t++)
        {
            QString nomdocrz  = listfichiers.at(t);
            QString CheminFichierResize = PATH_DIR_RESSOURCES "/" + nomdocrz;
            QFile(CheminFichierResize).remove();
        }
        QFile rufusViergeFile(QStringLiteral("://rufus.sql"));
        rufusViergeFile.copy(PATH_DIR_RESSOURCES "/rufus.sql");
        QFile OphtalmologieViergeFile(QStringLiteral("://Ophtalmologie.sql"));
        OphtalmologieViergeFile.copy(PATH_DIR_RESSOURCES "/Ophtalmologie.sql");
        QFile ImagerieViergeFile(QStringLiteral("://Images.sql"));
        ImagerieViergeFile.copy(PATH_DIR_RESSOURCES "/Images.sql");
        QFile ComptaMedicaleViergeFile(QStringLiteral("://ComptaMedicale.sql"));
        ComptaMedicaleViergeFile.copy(PATH_DIR_RESSOURCES "/ComptaMedicale.sql");

        QStringList listnomsfilestorestore;
        QString msg = "";
        listfichiers = dirtorestore.entryList(QStringList() << "*.sql");
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
        if (msg != "")
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'éxécuter la restauration!"), msg);
            for (int t=0; t<listfichiers.size(); t++)
            {
                QString nomdocrz  = listfichiers.at(t);
                QString CheminFichierResize = PATH_DIR_RESSOURCES "/" + nomdocrz;
                QFile(CheminFichierResize).remove();
            }
            return false;
        }
        if (!echecfile)
        {
            emit ConnectTimers(false);
            //! Suppression de toutes les tables
            QString Msg = tr("Suppression de l'ancienne base Rufus en cours");
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
            db->VideDatabases();
            db->StandardSQL("CREATE USER IF NOT EXISTS '" LOGIN_SQL "'@'%' IDENTIFIED BY '" MDP_SQL "'");
            db->StandardSQL("GRANT ALL ON *.* TO '" LOGIN_SQL "'@'%' IDENTIFIED BY '" MDP_SQL "' WITH GRANT OPTION");
            db->StandardSQL("CREATE USER IF NOT EXISTS '" LOGIN_SQL "SSL'@'%' IDENTIFIED BY '" MDP_SQL "' REQUIRE SSL");
            db->StandardSQL("GRANT ALL ON *.* TO '" LOGIN_SQL "SSL'@'%' IDENTIFIED BY '" MDP_SQL "' WITH GRANT OPTION");

            int a = 99;

            //! Restauration à partir du dossier sélectionné
            DefinitScriptRestore(listnomsfilestorestore);
            QString task = "sh " + PATH_FILE_SCRIPTRESTORE;
            QProcess dumpProcess(parent());
            dumpProcess.start(task);
            dumpProcess.waitForFinished();
             if (dumpProcess.exitStatus() == QProcess::NormalExit)
                a = dumpProcess.exitCode();
            if (a != 0)
            {
                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                for (int t=0; t<listfichiers.size(); t++)
                {
                    QString nomdocrz  = listfichiers.at(t);
                    QString CheminFichierResize = PATH_DIR_RESSOURCES "/" + nomdocrz;
                    QFile(CheminFichierResize).remove();
                }
                QFile::remove(PATH_FILE_SCRIPTRESTORE);
                return false;
            }
            else
            {
                UpMessageBox::Information(Q_NULLPTR, tr("Base vierge créée"),tr("La création de la base vierge a réussi."));
                for (int t=0; t<listfichiers.size(); t++)
                {
                    QString nomdocrz  = listfichiers.at(t);
                    QString CheminFichierResize = PATH_DIR_RESSOURCES "/" + nomdocrz;
                    QFile(CheminFichierResize).remove();
                }
                QFile::remove(PATH_FILE_SCRIPTRESTORE);
                emit ConnectTimers(true);
                return true;
            }
        }
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
        QString dir = PATH_DIR_RUFUS;
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
        if (!Utils::VerifMDP((PremierDemarrage? Utils::calcSHA1(MDP_ADMINISTRATEUR) : MDPAdmin()),tr("Saisissez le mot de passe Administrateur")))
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
        if (QDir(dirtorestore.absolutePath() + NOM_DIR_RESSOURCES).exists())
            if (QDir(dirtorestore.absolutePath() + NOM_DIR_RESSOURCES).entryList(QDir::Files | QDir::NoDotAndDotDot).size()>0)
                OKRessces = true;
        if (QFile(dirtorestore.absolutePath() + NOM_FILE_INI).exists())
            OKini = true;
        QDir rootimgvid = dirtorestore;
        if (rootimgvid.cdUp())
        {
            //qDebug() << rootimgvid.absolutePath() + DIR_IMAGES;
            if (QDir(rootimgvid.absolutePath() + NOM_DIR_IMAGES).exists())
                if (QDir(rootimgvid.absolutePath() + NOM_DIR_IMAGES).entryList(QDir::Dirs).size()>0)
                    OKImages = true;
            if (QDir(rootimgvid.absolutePath() + NOM_DIR_VIDEOS).exists())
                if (QDir(rootimgvid.absolutePath() + NOM_DIR_VIDEOS).entryList(QDir::Files | QDir::NoDotAndDotDot).size()>0)
                    OKVideos = true;
            if (QDir(rootimgvid.absolutePath() + NOM_DIR_FACTURES).exists())
                if (QDir(rootimgvid.absolutePath() + NOM_DIR_FACTURES).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size()>0)
                    OKFactures = true;
        }

        /*! 3 - détermination de l'emplacement de destination des fichiers d'imagerie */
        QString NomDirStockageImagerie = PATH_DIR_IMAGERIE;
        if (OKImages || OKVideos || OKFactures)
        {
            NomDirStockageImagerie = (PremierDemarrage? PATH_DIR_IMAGERIE : m_parametres->dirimagerieserveur());
            if (!QDir(NomDirStockageImagerie).exists())
            {
                bool exist = QDir().exists(PATH_DIR_IMAGERIE);
                QString existdir = (exist? "" : "\n" + tr("Créez-le au besoin"));
                UpMessageBox::Watch(Q_NULLPTR,tr("Pas de dossier de stockage d'imagerie"),
                                    tr("Indiquez un dossier valide dans la boîte de dialogue qui suit") + "\n" +
                                    tr("Utilisez de préférence le dossier ") + PATH_DIR_IMAGERIE +
                                    existdir);
                QFileDialog dialogimg(Q_NULLPTR,tr("Stocker les images dans le dossier") , PATH_DIR_RUFUS + (exist? PATH_DIR_IMAGERIE : ""));
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
                            QString task = "sh " + PATH_FILE_SCRIPTRESTORE;
                            QProcess dumpProcess(parent());
                            dumpProcess.start(task);
                            dumpProcess.waitForFinished(1000000000);
                             if (dumpProcess.exitStatus() == QProcess::NormalExit)
                                a = dumpProcess.exitCode();
                            if (a != 0)
                                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                            QFile::remove(PATH_FILE_SCRIPTRESTORE);
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
                        QString fileini = dirtorestore.absolutePath() + NOM_FILE_INI;
                        QFile FichierIni(PATH_FILE_INI);
                        if (FichierIni.exists())
                            FichierIni.remove();
                        QFile rufusini(fileini);
                        rufusini.copy(PATH_FILE_INI);
                        msg += tr("Fichier de paramétrage Rufus.ini restauré\n");
                        UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Fichier de paramétrage Rufus.ini restauré"), Icons::icSunglasses(), 3000);
                    }
                }
                /*! 4c - restauration des fichiers ressources */
                else if (chk->accessibleDescription() == "ressources")
                {
                    if (chk->isChecked())
                    {
                        QDir DirRssces(QDir(dirtorestore.absolutePath() + PATH_DIR_RESSOURCES));
                        QDir sauvRssces;
                        if (!sauvRssces.exists(PATH_DIR_RESSOURCES))
                            sauvRssces.mkdir(PATH_DIR_RESSOURCES);
                        QStringList listnomfic = DirRssces.entryList();
                        for (int i=0; i<listnomfic.size(); i++)
                        {
                            QFile ficACopier(DirRssces.absolutePath() + "/" + listnomfic.at(i));
                            QString nomficACopier = QFileInfo(listnomfic.at(i)).fileName();
                            ficACopier.copy(PATH_DIR_RESSOURCES + "/" + nomficACopier);
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
                        QString dirdestinationimg   = NomDirStockageImagerie + NOM_DIR_IMAGES;
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
                            QDir dirrestaureimagerie    = QDir(rootimgvid.absolutePath() + NOM_DIR_IMAGES);
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
                        QString dirdestinationfact  = NomDirStockageImagerie + NOM_DIR_FACTURES;
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
                            QDir dirrestaurefactures    = QDir(rootimgvid.absolutePath() + NOM_DIR_FACTURES);
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
                        QString dirdestinationvid   =  NomDirStockageImagerie + NOM_DIR_VIDEOS;
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
                            QDir dirrestaurevideo = QDir(rootimgvid.absolutePath() + NOM_DIR_VIDEOS);
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
    auto erreur = []
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de mettre à jour la base de données\nSortie du programme"));
        exit(0);
    };

    int Version         = VERSION_BASE;
    bool b;
    m_parametres = db->parametres();
    int Versionencours = m_parametres->versionbase();
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
                UpSmallButton *OKBouton = new UpSmallButton();
                UpSmallButton *BackupBouton = new UpSmallButton();
                UpSmallButton *ExitBouton = new UpSmallButton();
                OKBouton->setText(tr("Pousuivre, la sauvegarde a été faite"));
                BackupBouton->setText(tr("OK, je vais sauvegarder la base d'abord"));
                ExitBouton->setText(tr("Annuler et fermer"));
                msgbox.addButton(ExitBouton, UpSmallButton::CLOSEBUTTON);
                msgbox.addButton(BackupBouton, UpSmallButton::CANCELBUTTON);
                msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
                msgbox.exec();
                if (msgbox.clickedButton() == BackupBouton)
                {
                    if (!ImmediateBackup())
                        erreur();
                }
                else if (msgbox.clickedButton() == ExitBouton)
                    erreur();
                BupDone = true;
                Datas::I()->postesconnectes->initListe();
                PosteConnecte* post = Datas::I()->postesconnectes->admin();
                if (post != Q_NULLPTR)
                    UpMessageBox::Watch(Q_NULLPTR,tr("RufusAdmin présent"), tr("Après la mise à jour de la base") + "\n" +
                                                                            tr("Il vous faudra installer une version de RufusAdmin correspondante à la nouvelle version de la base") + "\n" +
                                                                            tr("Il faudra relancer chaque poste du réseau après le redémarrage de RufusAdmin"));
            }
            ShowMessage::I()->SplashMessage(tr("Mise à jour de la base vers la version ") + "<font color=\"red\"><b>" + QString::number(Version) + "</b></font>", 1000);
            QString Nomfic = "://majbase" + QString::number(Version) + ".sql";
            QFile DumpFile(Nomfic);
            int a = 99;
            if (DumpFile.exists())
            {
                QString NomDumpFile = PATH_DIR_RESSOURCES "/majbase" + QString::number(Version) + ".sql";
                QFile::remove(NomDumpFile);
                DumpFile.copy(NomDumpFile);
                emit ConnectTimers(false);
                DefinitScriptRestore(QStringList() << NomDumpFile);
                QString task = "sh " + PATH_FILE_SCRIPTRESTORE;
                QProcess dumpProcess(parent());
                dumpProcess.start(task);
                dumpProcess.waitForFinished();
                if (dumpProcess.exitStatus() == QProcess::NormalExit)
                    a = dumpProcess.exitCode();
                QFile::remove(PATH_FILE_SCRIPTRESTORE);
                QFile::remove(NomDumpFile);
                if (a == 0)
                {
                    UpMessageBox::Watch(Q_NULLPTR,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version));
                    db->initParametresSysteme();
                }
                else
                {
                    QSound::play(NOM_ALARME);
                    UpMessageBox::Watch(Q_NULLPTR,tr("Echec de la mise à jour vers la version ") + QString::number(Version) + "\n" + tr("Le programme de mise à jour n'a pas pu effectuer la tâche!"));
                    erreur();
                }
            }
            if (Version == 66 && a == 0)
            {
                QString req = " select " CP_ID_MANUFACTURER ", CorNom, CorPrenom, CorStatut, CorMail, CorTelephone from " TBL_MANUFACTURERS
                                " where CorNom is not null and CorNom <> ''";
                bool ok;
                QList<QVariantList> listcom = DataBase::I()->StandardSelectSQL(req,ok);
                if (ok && listcom.size()>0)
                    for (int i= 0; i<listcom.size(); ++i)
                    {
                        req = "insert into " TBL_COMMERCIALS "(" CP_NOM_COM ", " CP_PRENOM_COM ", " CP_STATUT_COM ", " CP_MAIL_COM ", " CP_TELEPHONE_COM ", " CP_IDMANUFACTURER_COM ")"
                              " VALUES ( '" + listcom.at(i).at(1).toString() + "', '"  + listcom.at(i).at(2).toString() + "', '"
                                + listcom.at(i).at(3).toString() + "', '"  + listcom.at(i).at(4).toString()
                                + "', '"  + listcom.at(i).at(5).toString() + "', '"  + listcom.at(i).at(0).toString() + "')";
                        DataBase::I()->StandardSQL(req);
                    }
                req = "update " TBL_MANUFACTURERS " set CorNom = null, CorPrenom = null, CorStatut = null, CorMail = null, CorTelephone = null";
                DataBase::I()->StandardSQL(req);
//                DataBase::I()->StandardSQL("ALTER TABLE `rufus`.`Manufacturers` "
//                "DROP COLUMN `CorTelephone`,"
//                "DROP COLUMN `CorMail`,"
//                "DROP COLUMN `CorStatut`,"
//                "DROP COLUMN `CorPrenom`,"
//                "DROP COLUMN `CorNom`;");
            }
        }
    }
    //verification des fichiers ressources
    if (m_settings->value("Param_Poste/VersionRessources").toInt() < VERSION_RESSOURCES)
    {
        PremierParametrageRessources();
        m_settings->setValue("Param_Imprimante/TailleEnTeteALD","63");
        m_settings->setValue("Param_Poste/VersionRessources", VERSION_RESSOURCES);
        ShowMessage::I()->SplashMessage(tr("Mise à jour des fichiers ressources vers la version ") + "<font color=\"red\"><b>" + QString::number(VERSION_RESSOURCES) + "</b></font>", 1000);
    }
    return true;
}


void Procedures::ReconstruitListeModesAcces()
{
    if ( m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Active").toString() == "YES"
       && (m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Port").toInt() == 3306 || m_settings->value(Utils::getBaseFromMode(Utils::Poste) + "/Port").toInt() == 3307) )
        m_listemodesacces << Utils::Poste;
    if (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Active").toString() == "YES"
       && m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Serveur").toString() != ""
       && (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Port").toInt() == 3306 || m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/Port").toInt() == 3307) )
        m_listemodesacces <<  Utils::ReseauLocal;
    if (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Active").toString() == "YES"
       && m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Serveur").toString() != ""
       && (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Port").toInt() == 3306 || m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/Port").toInt() == 3307) )
        m_listemodesacces << Utils::Distant;
}


/*--------------------------------------------------------------------------------------------------------------
    -- Choix du mode de connexion ----------------------------------------------------------------------------------
    --------------------------------------------------------------------------------------------------------------*/
bool Procedures::FicheChoixConnexion()
{
    bool initok;
    UpMessageBox msgbox;
    msgbox.setText(tr("Quelle base de données souhaitez-vous utiliser?"));
    msgbox.setIcon(UpMessageBox::Quest);
    UpPushButton wdg_annulbouton(tr("Annuler"));
    UpPushButton wdg_localbouton(tr("Locale, sur ce réseau"));
    UpPushButton wdg_distantbouton(tr("Distante, par internet"));
    UpPushButton wdg_postebouton(tr("Sur cette machine"));
    wdg_annulbouton.setIcon(Icons::icAnnuler());
    msgbox.addButton(&wdg_annulbouton);
    if (m_listemodesacces.contains(Utils::ReseauLocal))
    {
        wdg_localbouton.setData(Utils::ReseauLocal);
        msgbox.addButton(&wdg_localbouton);
        wdg_localbouton.setIcon(Icons::icNetwork());
    }
    if (m_listemodesacces.contains(Utils::Distant))
    {
        wdg_distantbouton.setData(Utils::Distant);
        msgbox.addButton(&wdg_distantbouton);
        wdg_distantbouton.setIcon(Icons::icInternet());
    }
    if (m_listemodesacces.contains(Utils::Poste))
    {
        wdg_postebouton.setData(Utils::Poste);
        msgbox.addButton(&wdg_postebouton);
        wdg_postebouton.setIcon(Icons::icComputer());
    }
    initok = false;
    if (msgbox.exec()>0)
    {
        initok = (msgbox.clickedpushbutton() != &wdg_annulbouton);
        if (initok)
        {
            db->setModeacces(static_cast<Utils::ModeAcces>(msgbox.clickedpushbutton()->data()));
            Logs::LogSQL("Mode accès - Utils::" + Utils::EnumDescription(QMetaEnum::fromType<Utils::ModeAcces>(), db->ModeAccesDataBase()));
        }
    }
    return initok;
}

/*--------------------------------------------------------------------------------------------------------------
-- Connexion à Consults -------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
bool Procedures::Connexion_A_La_Base()
{
    QString server = "localhost";
    if( db->ModeAccesDataBase() == Utils::Poste )
        server = "localhost";
    else
        server = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + "/Serveur").toString();

    int port = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + "/Port").toInt();

    db->initParametresConnexionSQL(server, port);
    if (!IdentificationUser())
        return false;

    //initListeUsers();
    CalcLieuExercice();
    if (Datas::I()->sites->currentsite() == Q_NULLPTR)
        UpMessageBox::Watch(Q_NULLPTR,tr("Pas d'adresse spécifiée"), tr("Vous n'avez précisé aucun lieu d'exercice!"));
    m_connexionbaseOK = true;

    db->StandardSQL("SET GLOBAL max_allowed_packet=" MAX_ALLOWED_PACKET "*1024*1024 ;");

    return m_connexionbaseOK;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Détermination du lieu exercice pour la session en cours -------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------- */
void Procedures::CalcLieuExercice()
{
    QList<Site*> listEtab = Datas::I()->sites->initListeByUser(currentuser()->id());
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
        pradiobutt->setImmediateToolTip(etab->coordonnees());
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

    // Création du compte administrateur dans la table utilisateurs
    QString req = "insert into " TBL_UTILISATEURS " (" CP_NOM_USR ", " CP_LOGIN_USR ", " CP_MDP_USR ") values ('" NOM_ADMINISTRATEUR "','" NOM_ADMINISTRATEUR "', '" + Utils::calcSHA1(MDP_ADMINISTRATEUR) + "')";
    db->StandardSQL (req);
    // Création du permier utilisateur dans la table utilisateurs
    //qDebug() << req;
    m_idcentre               = 1;
    m_usecotation            = true;
    Datas::I()->banques->initListe();
    bool ok;
    int idusr = db->selectMaxFromTable(CP_ID_USR, TBL_UTILISATEURS, ok);
    ++idusr;
    CreerUserFactice(idusr, Login, MDP);
    db->setidUserConnected(idusr);
    Datas::I()->users->initListe();
    Datas::I()->comptes->initListe();
    MAJComptesBancaires(currentuser());
    currentuser()->setidsuperviseur(idusr);
    currentuser()->setidcomptable(idusr);
    currentuser()->setidparent(idusr);

    if (UpMessageBox::Question(Q_NULLPTR, tr("Un compte utilisateur a été cré"),
                               tr("Un compte utilisateur factice a été créé\n") + "\n" +
                               currentuser()->titre() + " "  + currentuser()->prenom() + " " + currentuser()->nom() + ", " + currentuser()->fonction()
                               + "\n\n" +
                               tr("avec le login ") + Login + " " + tr("et le mot de passe que vous avez fourni") + "\n" +
                               tr("Voulez-vous conserver ces données pour le moment ou les modifier?") + "\n" +
                               tr("Vous pourrez les modifier par la suite")
                               + "\n\n" +
                               tr("Une liste arbitraires de cotations a été créée") + "\n" +
                               tr("Vous pourrez la modifier par la suite dans le menu Edition/Paramètres\n"),
                               UpDialog::ButtonOK | UpDialog::ButtonEdit, QStringList() << tr("Modifier les données") << tr("Conserver les données"))
        == UpSmallButton::EDITBUTTON)
    {
        int gidLieuExercice = -1;
        dlg_gestionusers *Dlg_GestUsr = new dlg_gestionusers(gidLieuExercice, dlg_gestionusers::PREMIERUSER , true);
        Dlg_GestUsr->setWindowTitle(tr("Enregistrement de l'utilisateur ") + Login);
        if (Dlg_GestUsr->exec() > 0)
        {
            m_parametres = db->parametres();
            Datas::I()->comptes         ->initListe();
            Datas::I()->postesconnectes ->initListe();
            Datas::I()->banques         ->initListe();
            Datas::I()->users           ->initListe();
            MAJComptesBancaires(currentuser());
            m_applicationfont = currentuser()->police();
        }
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
    db->StandardSQL ("insert into " TBL_UTILISATEURS " (" CP_ID_USR ", " CP_LOGIN_USR ", " CP_MDP_USR ") VALUES (" + QString::number(idusr) + ", '" + login + "', '" + Utils::calcSHA1(mdp) + "')");

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
            " set " CP_NOM_USR " = 'Snow',\n"
            CP_PRENOM_USR " = '" + Utils::trimcapitilize(login) +"',\n"
            CP_POLICEECRAN_USR " = '" POLICEPARDEFAUT "',\n"
            CP_POLICEATTRIBUT_USR " = '" POLICEATTRIBUTPARDEFAUT "',\n"
            CP_TITRE_USR " = '" + tr("Docteur") + "',\n"
            CP_FONCTION_USR " = '" + tr("Médecin") + "',\n"
            CP_SPECIALITE_USR " = '" + tr("Ophtalmologiste") + "',\n"
            CP_IDSPECIALITE_USR " = 15,\n"
            CP_NUMCO_USR " = '2 33 2 123456 1 2 3 4',\n "
            CP_NUMPS_USR " = '123456789',\n "
            CP_DROITS_USR " = '" OPHTALIBERAL "', \n"
            CP_ENREGHONORAIRES_USR " = 1,\n"
            CP_IDCOMPTEPARDEFAUT_USR " = " + idcpt + ",\n"
            CP_IDCOMPTEENCAISSEMENTHONORAIRES_USR " = " + idcpt + ",\n"
            CP_SOIGNANTSTATUS_USR " = 1,\n"
            CP_ISMEDECIN_USR " = 1,\n"
            CP_RESPONSABLEACTES_USR " = 1,\n"
            CP_CCAM_USR " = 1,\n"
            CP_ISAGA_USR " = 1,\n"
            CP_SECTEUR_USR " = 1,\n"
            CP_ISOPTAM_USR " = 1\n"
            " where " CP_ID_USR " = " + QString::number(idusr);
    //Edit(req);
    db->StandardSQL(req);
    req = "insert into " TBL_LIEUXEXERCICE "(" CP_NOM_SITE ", " CP_ADRESSE1_SITE ", " CP_ADRESSE2_SITE ", "
            CP_CODEPOSTAL_SITE ", " CP_VILLE_SITE ", " CP_TELEPHONE_SITE ", " CP_COULEUR_SITE ")  values ("
            "'Centre ophtalmologique de La Mazière', "
            "'place rouge', "
            "'Bâtiment C', "
            "23260, "
            "'La Mazière', "
            "'O4 56 78 90 12', "
            "'000000')";
    //qDebug() << req;
    db->StandardSQL(req);
    req = "select " CP_ID_SITE " from " TBL_LIEUXEXERCICE;
    int gidLieuExercice = 0;
    QList<QVariantList> lieuxlist = db->StandardSelectSQL(req, m_ok);
    if (m_ok && lieuxlist.size()>0)
        gidLieuExercice = lieuxlist.at(0).at(0).toInt();
    req = "insert into " TBL_JOINTURESLIEUX " (" CP_IDUSER_JOINTSITE ", " CP_IDLIEU_JOINTSITE ") VALUES(" + QString::number(idusr) + ", " + QString::number(gidLieuExercice) + ")";
    db->StandardSQL(req);
    db->setidlieupardefaut(gidLieuExercice);
}


/*-----------------------------------------------------------------------------------------------------------------
    -- Identification de l'utilisateur -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::IdentificationUser()
{
    dlg_identificationuser *dlg_IdentUser   = new dlg_identificationuser();
    dlg_IdentUser   ->setFont(m_applicationfont);
    connect(dlg_IdentUser, &dlg_identificationuser::verifbase, this, &Procedures::VerifBaseEtRessources);
    int result = dlg_IdentUser->exec();
    if( result > 0 )
    {
        m_parametres = db->parametres();
        Datas::I()->villes          ->initListe();
        Datas::I()->sites           ->initListe();
        Datas::I()->comptes         ->initListe();
        Datas::I()->postesconnectes ->initListe();
        Datas::I()->banques         ->initListe();
        Datas::I()->tierspayants    ->initListe();
        Datas::I()->typestiers      ->initListe();
        Datas::I()->motifs          ->initListe();
        Datas::I()->users           ->initListe();
        Datas::I()->manufacturers   ->initListe();
        Datas::I()->motscles        ->initListe();
        MAJComptesBancaires(currentuser());
        m_applicationfont = currentuser()->police();
        qApp->setFont(m_applicationfont);

        Verif_secure_file_priv();
        if (DefinitRoleUser()) //NOTE : User Role
        {
            /*! definit les iduser pour lequel le user travaille
                . iduser superviseur des actes                      (int gidUserSuperViseurProv)
                    . lui-même s'il est responsable de ses actes
                    . un autre user s'il est assistant
                    . -1 s'il est assistant pour plusieurs utilisateurs en même temps
                    . -2 sans objet (personnel non soignant)
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
                    . -2 sans objet (personnel non soignant)
                . et s'il cote les actes                            (bool gUseCotationProv)
                    0 = AvecCompta
                    1 = SansCompta et sans cotation
                    2 = Sans compta mais avec cotation
                    3 = Avec compta mais sans cotation
           */
            currentuser()->setTypeCompta(m_aveccomptaprovisoire ?
                                         (m_usecotation ? User::COMPTA_AVEC_COTATION_AVEC_COMPTABILITE : User::COMPTA_SANS_COTATION_AVEC_COMPTABILITE)
                                           :
                                         (m_usecotation ? User::COMPTA_AVEC_COTATION_SANS_COMPTABILITE : User::COMPTA_SANS_COTATION_SANS_COMPTABILITE));

            //AFFECT USER:
            //qDebug() << "superviseur " << currentuser()->getIdUserActeSuperviseur();
            //qDebug() << "comptable " << currentuser()->getIdUserComptable();
            //qDebug() << "parent " << currentuser()->getIdUserParent();

            m_idcentre = m_parametres->numcentre();
        }
    }
    else if ( result < 0 ) // anomalie sur la base - table utilisateurs manquante ou corrompue
    {
        dlg_identificationuser::LoginResult loginresult = dlg_IdentUser->loginresult();
        QString m_loginSQL      = dlg_IdentUser->ui->LoginlineEdit->text();
        QString m_passwordSQL   = dlg_IdentUser->ui->MDPlineEdit->text();
        UpMessageBox    msgbox;
        UpSmallButton   AnnulBouton(tr("Annuler"));
        UpSmallButton   RestaureBaseBouton(tr("Restaurer la base depuis une sauvegarde"));
        UpSmallButton   BaseViergeBouton(tr("Nouvelle base patients vierge"));
        switch (loginresult) {
        case dlg_identificationuser::CorruptedBase:
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
                Utils::mkpath(PATH_DIR_RESSOURCES);
                if (!RestaureBase(true, true))
                    exit(0);
                CreerPremierUser(m_loginSQL, m_passwordSQL);
                UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va se fermer"), tr("Relancez-le pour que certaines données puissent être prises en compte"));
                Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
                exit(0);
            }
            break;
        default:
            break;
        }
    }
    disconnect(dlg_IdentUser, &dlg_identificationuser::verifbase, this, &Procedures::VerifBaseEtRessources);
    delete dlg_IdentUser;
    return (currentuser() != Q_NULLPTR);
}

QString Procedures::DefinitDossierImagerie()
{
    QString path = "";
    switch (db->ModeAccesDataBase()) {
    case Utils::Poste:
        path = db->parametres()->dirimagerieserveur();
        break;
    case Utils::ReseauLocal:
        path = settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + "/DossierImagerie").toString();
        break;
    case Utils::Distant:
        path = settings()->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierImagerie").toString();
    }
    return path;
}

bool Procedures::DefinitRoleUser() //NOTE : User Role Function
{
    if (currentuser()->isSoignant() )
    {
        QString req;
        dlg_askUser                = new UpDialog();
        dlg_askUser                ->AjouteLayButtons();
        dlg_askUser                ->setAccessibleName(QString::number(currentuser()->id()));
        dlg_askUser->setdata(currentuser());
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
        if( currentuser()->isResponsable() )
            CalcUserParent();

        // le user alterne entre responsable des actes et assistant suivant la session
        // on lui demande son rôle pour cette session
        else if( currentuser()->isResponsableOuAssistant() )
        {
            bool found = false;
            foreach (User *us, Datas::I()->users->actifs()->values())
            {
                if( us->id() == currentuser()->id() )
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
        else if( currentuser()->isAssistant() )
            CalcUserSuperviseur();

        dlg_askUser                ->setModal(true);
        dlg_askUser->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
        connect(dlg_askUser->OKButton, &QPushButton::clicked, dlg_askUser, &UpDialog::accept);

        if( currentuser()->idsuperviseur() == User::ROLE_INDETERMINE || currentuser()->idparent() == User::ROLE_INDETERMINE )
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
                            currentuser()->setidsuperviseur(listbutt.at(j)->accessibleName().toInt());
                            break;
                        }
                }
                else if (groupBox->accessibleName() == "Parent" )
                {
                    QList<QRadioButton*> listbutt = groupBox->findChildren<QRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                        {
                            currentuser()->setidparent(listbutt.at(j)->accessibleName().toInt());
                            break;
                        }
                }
            }
            delete dlg_askUser;
        }
        if( currentuser()->idsuperviseur() == User::ROLE_INDETERMINE )
        {
            UpMessageBox::Watch(Q_NULLPTR,tr("Aucun superviseur valide n'a été défini pour vos actes"), tr("Impossible de continuer"));
            return false;
        }
        // le user est assistant et travaille pour tout le monde
        if( currentuser()->idsuperviseur() == User::ROLE_NON_RENSEIGNE )
        {
            currentuser()->setidparent(User::ROLE_NON_RENSEIGNE);
            currentuser()->setidcomptable(User::ROLE_NON_RENSEIGNE);
            m_aveccomptaprovisoire = true;
            m_usecotation     = true;
        }
        else
        {
            // determination du parent
            if( currentuser()->idparent() == User::ROLE_INDETERMINE )
            {
                if ( currentuser()->idsuperviseur() > 0 )
                {
                    User* superviseurusr = Datas::I()->users->getById( currentuser()->idsuperviseur());
                    if (superviseurusr)
                    {
                        if (superviseurusr->isRemplacant() )
                        {
                            // le superviseur est remplaçant, on essaie de savoir s'il a un parent
                            QList<User*> listUserFound;
                            foreach (User *us, Datas::I()->users->actifs()->values())
                            {
                                if( us->id() == currentuser()->id() )
                                    continue;
                                if( us->id() == currentuser()->idsuperviseur() )
                                    continue;
                                if( !us->isLiberal() && !us->isSalarie() )
                                    continue;
                                listUserFound << us;
                            }
                            if (listUserFound.size() == 1)
                                currentuser()->setidparent( listUserFound.first()->id() );
                            else if( !listUserFound.isEmpty() )
                            {
                                // on va demander qui est le soignant parent de ce remplaçant....
                                dlg_askUser             = new UpDialog();
                                dlg_askUser             ->AjouteLayButtons();
                                dlg_askUser             ->setAccessibleName(QString::number(currentuser()->idsuperviseur()));
                                dlg_askUser             ->setdata(superviseurusr);
                                QVBoxLayout *boxlay     = new QVBoxLayout;
                                dlg_askUser->dlglayout()->insertLayout(0,boxlay);
                                QGroupBox*boxparent     = new QGroupBox(dlg_askUser);
                                boxparent               ->setAccessibleName("Parent");
                                QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + superviseurusr->login() + "?";
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
                                                    currentuser()->setidparent( butt->accessibleName().toInt() );
                                                    break;
                                                }
                                    delete dlg_askUser;
                                }
                            }
                        }
                        else
                            currentuser()->setidparent( currentuser()->idsuperviseur() );
                    }
                    else
                    {
                        UpMessageBox::Watch(Q_NULLPTR,tr("Aucun superviseur valide n'a été défini pour vos actes"), tr("Impossible de continuer"));
                        return false;
                    }
                }
            }
            // determination du comptable et de l'usage de la cotation
            if ( currentuser()->idparent() > 0 && currentuser()->ishisownsupervisor() )
            {
                User *usrparent = Datas::I()->users->getById(currentuser()->idparent());
                if ( usrparent )
                {
                    // determination de l'utilisation de la cotation
                    m_usecotation = usrparent->useCCAM();
                    // determination de l'utilisation de la comptabilité
                     m_aveccomptaprovisoire = !usrparent->isSansCompta();
                    if( usrparent->isLiberal() )
                        currentuser()->setidcomptable(usrparent->id());
                    else if( usrparent->isSalarie() )
                        currentuser()->setidcomptable(usrparent->idemployeur());
                    else
                        currentuser()->setidcomptable(User::ROLE_NON_RENSEIGNE);
                }
                else
                {
                    UpMessageBox::Watch(Q_NULLPTR,tr("Aucun parent valide n'a été défini pour vos actes"), tr("Impossible de continuer"));
                    return false;
                }
            }
            else
                currentuser()->setidcomptable(User::ROLE_VIDE);
        }
        return true;
    }

    // il s'agit d'un administratif ou d'une société comptable
    currentuser()->setidsuperviseur(User::ROLE_VIDE);
    currentuser()->setidcomptable(User::ROLE_VIDE);
    currentuser()->setidparent(User::ROLE_VIDE);
    m_usecotation     = true;
    m_aveccomptaprovisoire = true; //FIXME : avecLaComptaProv
    return true;
}

/*!
 * \brief Procedures::SetUserAllData(User *usr)
 * Charge les données bancaires d'un utilisateur
 * cette fonction fait appel aux deux classes cls_user et cls_compte
 * et ne peut pas figurer dans la classe cls_user
 * en raison de référence croisées
 */
void Procedures::MAJComptesBancaires(User *usr)
{
    if (!usr)
        return;
    usr->setlistecomptesbancaires(Datas::I()->comptes->initListeComptesByIdUser(usr->id()));
    if (usr->isSalarie())
    {
        User *employer = Datas::I()->users->getById(usr->idemployeur());
        usr->setidcompteencaissementhonoraires(employer? employer->idcompteencaissementhonoraires() : 0);
    }
    else if (usr->idcomptable() > 0)
    {
        User *cptble = Datas::I()->users->getById(usr->idcomptable());
        usr->setidcompteencaissementhonoraires(cptble? cptble->idcompteencaissementhonoraires() : 0);
    }
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
    currentuser()->setidsuperviseur(User::ROLE_INDETERMINE);
    currentuser()->setidparent(User::ROLE_INDETERMINE);
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
        if( currentuser()->isMedecin() && !us->isMedecin() )
            continue;
        listUserFound << us;
    }

    if( listUserFound.size() == 1 )
        currentuser()->setidsuperviseur( listUserFound.first()->id() );
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
        foreach (User *us, Datas::I()->users->actifs()->values() )
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
bool Procedures::PremierDemarrage()
{
    QMessageBox     msgbox;
    int         protoc;
    enum protoc {BaseExistante, BaseVierge};
    UpSmallButton    AnnulBouton        (tr("Retour\nau menu d'accueil"));
    UpSmallButton    BaseViergeBouton (tr("Nouvelle base\npatients vierge"));
    UpSmallButton    BaseExistanteBouton(tr("Base patients existante\nsur le serveur"));

    msgbox.setText(tr("Premier démarrage de Rufus!"));
    msgbox.setInformativeText(tr("Cette étape va vous permettre de configurer le logiciel en quelques secondes.\n\n"
                              "Cette installation ne peut aboutir si vous n'avez pas de serveur MySQL installé.\n"
                              "Dans ce cas, il vous faut annuler et installer un serveur MySQL sur cet ordinateur ou sur un autre poste du réseau.\n\n"
                              "Commencez par choisir la situation qui décrit le mieux votre installation de Rufus.\n\n"
                              "1. J'installe Rufus sur ce poste et ce poste se connecte à une base patients qui existe dèjà sur le serveur\n"
                              "2. J'installe Rufus sur ce poste et ce poste se connectera à une base patients vierge que je vais créer sur le serveur\n"));
    msgbox.setIcon(QMessageBox::Information);

    msgbox.addButton(&AnnulBouton,          QMessageBox::AcceptRole);
    msgbox.addButton(&BaseExistanteBouton,  QMessageBox::AcceptRole);
    msgbox.addButton(&BaseViergeBouton,     QMessageBox::AcceptRole);
    msgbox.exec();

    protoc = BaseExistante;
    if (msgbox.clickedButton() == &AnnulBouton)
        return false;
    else if (msgbox.clickedButton() == &BaseExistanteBouton)
        protoc = BaseExistante;
    else if (msgbox.clickedButton() == &BaseViergeBouton)
        protoc = BaseVierge;


    // Création des dossiers
    Utils::mkpath(PATH_DIR_RESSOURCES);
    Utils::mkpath(PATH_DIR_IMAGES);
    Utils::mkpath(PATH_DIR_ECHECSTRANSFERTS);
    Utils::mkpath(PATH_DIR_DOSSIERECHANGE);
    Utils::mkpath(PATH_DIR_VIDEOS);
    Utils::mkpath(PATH_DIR_PROV);
    Utils::mkpath(PATH_DIR_FACTURESSANSLIEN);
    Utils::mkpath(PATH_DIR_FACTURES);
    Utils::mkpath(PATH_DIR_ORIGINAUX NOM_DIR_FACTURES);
    Utils::mkpath(PATH_DIR_ORIGINAUX NOM_DIR_IMAGES);
    m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
    QString login (""), MDP("");
    if (protoc == BaseExistante)
    {
        if (VerifParamConnexion(login, MDP, true))
        {
            PremierParametrageMateriel(false);
            PremierParametrageRessources();
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne,\n"
                                       "le login ") + currentuser()->login() + tr(" est reconnu") + ".\n" +
                                       tr("Le programme va se fermer pour que les modifications") + "\n" +
                                       tr("puissent être prises en compte\n"));
            exit(0);
        }
    }
    else if (protoc == BaseVierge)
    {
        bool SansAccesDistant = false;
        if (VerifParamConnexion(login, MDP, false, SansAccesDistant))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne "
                                       "et le programme va maintenant créer une base de données patients "
                                       "vierge de tout enregistrement."));
            // Création de la base
             if (!RestaureBase(true, true))
                return false;
             if (db->ModeAccesDataBase() == Utils::ReseauLocal)
                 db->setadresseserveurlocal(m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + "/Serveur").toString());
             m_parametres = db->parametres();

             // Création de l'utilisateur
             m_connexionbaseOK = CreerPremierUser(login, MDP);
             Datas::I()->sites->initListe();
             CalcLieuExercice();
             if (Datas::I()->sites->currentsite() == Q_NULLPTR)
                 UpMessageBox::Watch(Q_NULLPTR,tr("Pas d'adresse spécifiée"), tr("Vous n'avez précisé aucun lieu d'exercice!"));
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
void Procedures::PremierParametrageMateriel(bool modifdirimagerie)
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
    Utils::mkpath(PATH_DIR_IMAGERIE);
    QString NomDirImg = (modifdirimagerie? PATH_DIR_IMAGERIE : db->parametres()->dirimagerieserveur());
    m_settings->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierImagerie", NomDirImg);
    if (modifdirimagerie)
        db->setdirimagerie(NomDirImg);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Création des fichiers ressources ---------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Procedures::PremierParametrageRessources()
{
    Utils::mkpath(PATH_DIR_RUFUS);
    QDir DirRessrces(PATH_DIR_RESSOURCES);
    if (DirRessrces.exists())
        DirRessrces.rmdir(PATH_DIR_RESSOURCES);
    DirRessrces.mkpath(PATH_DIR_RESSOURCES);
    QFile COACopier(QStringLiteral(":/" NOM_FILE_CORPSORDO));
    COACopier.copy(PATH_FILE_CORPSORDO);
    QFile CO(PATH_FILE_CORPSORDO);
    CO.open(QIODevice::ReadWrite);
    CO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile COALDACopier(QStringLiteral(":/" NOM_FILE_CORPSORDOALD));
    COALDACopier.copy(PATH_FILE_CORPSORDOALD);
    QFile COALD(PATH_FILE_CORPSORDOALD);
    COALD.open(QIODevice::ReadWrite);
    COALD.setPermissions(QFileDevice::ReadOther     | QFileDevice::WriteOther
                         | QFileDevice::ReadGroup   | QFileDevice::WriteGroup
                         | QFileDevice::ReadOwner   | QFileDevice::WriteOwner
                         | QFileDevice::ReadUser    | QFileDevice::WriteUser);

    QFile EOACopier(QStringLiteral(":/" NOM_FILE_ENTETEORDO));
    EOACopier.copy(PATH_FILE_ENTETEORDO);
    QFile EO(PATH_FILE_ENTETEORDO);
    EO.open(QIODevice::ReadWrite);
    EO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile EOALDACopier(QStringLiteral(":/" NOM_FILE_ENTETEORDOALD));
    EOALDACopier.copy(PATH_FILE_ENTETEORDOALD);
    QFile EOALD(PATH_FILE_ENTETEORDOALD);
    EOALD.open(QIODevice::ReadWrite);
    EOALD.setPermissions(QFileDevice::ReadOther     | QFileDevice::WriteOther
                         | QFileDevice::ReadGroup   | QFileDevice::WriteGroup
                         | QFileDevice::ReadOwner   | QFileDevice::WriteOwner
                         | QFileDevice::ReadUser    | QFileDevice::WriteUser);

    QFile POLACopier(QStringLiteral(":/" NOM_FILE_PIEDPAGEORDOLUNETTES));
    POLACopier.copy(PATH_FILE_PIEDPAGEORDOLUNETTES);
    QFile POL(PATH_FILE_PIEDPAGEORDOLUNETTES);
    POL.open(QIODevice::ReadWrite);
    POL.setPermissions(QFileDevice::ReadOther   | QFileDevice::WriteOther
                       | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                       | QFileDevice::ReadOwner | QFileDevice::WriteOwner
                       | QFileDevice::ReadUser  | QFileDevice::WriteUser);

    QFile POACopier(QStringLiteral(":/" NOM_FILE_PIEDPAGE));
    POACopier.copy(PATH_FILE_PIEDPAGE);
    QFile PO(PATH_FILE_PIEDPAGE);
    PO.open(QIODevice::ReadWrite);
    PO.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                      | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                      | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                      | QFileDevice::ReadUser   | QFileDevice::WriteUser);

    QFile PDFACopier(QStringLiteral(":/" NOM_FILE_PDF));
    PDFACopier.copy(PATH_FILE_PDF);
    QFile pdf(PATH_FILE_PDF);
    pdf.open(QIODevice::ReadWrite);
    pdf.setPermissions(QFileDevice::ReadOther   | QFileDevice::WriteOther
                       | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                       | QFileDevice::ReadOwner | QFileDevice::WriteOwner
                       | QFileDevice::ReadUser  | QFileDevice::WriteUser);
    m_settings->setValue("Param_Poste/VersionRessources",VERSION_RESSOURCES);
 }

/*------------------------------------------------------------------------------------------------------------------------------------
-- Vérifie la présence et la cohérence du fchier d'initialisation et le reconstruit au besoin ----------------------------------------
------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifIni(QString msg, QString msgInfo, bool DetruitIni, bool RecupIni, bool ReconstruitIni, bool PremDemarrage)
{
    UpSmallButton AnnulBouton              (tr("Abandonner et\nquitter Rufus"));
    UpSmallButton RecupIniBouton           (tr("Restaurer le fichier d'initialisation\nà partir d'une sauvegarde"));
    UpSmallButton ReconstruitIniBouton     (tr("Reconstruire le fichier\nd'initialisation"));
    UpSmallButton PremierDemarrageBouton   (tr("Premier démarrage\nde Rufus"));

    QMessageBox *msgbox = new QMessageBox;
    msgbox->setText(msg);
    msgbox->setInformativeText(msgInfo);
    msgbox->setIcon(QMessageBox::Warning);
    if (ReconstruitIni)                     msgbox->addButton(&ReconstruitIniBouton,     QMessageBox::AcceptRole);
    if (RecupIni)                           msgbox->addButton(&RecupIniBouton,           QMessageBox::AcceptRole);
    if (PremDemarrage)                      msgbox->addButton(&PremierDemarrageBouton,   QMessageBox::AcceptRole);
     msgbox->addButton(&AnnulBouton, QMessageBox::AcceptRole);
    msgbox->exec();
    bool reponse = false;

    if (msgbox->clickedButton()==&AnnulBouton)
    {
        if (DetruitIni)
            QFile::remove(PATH_FILE_INI);
        exit(0);
    }
    else if (msgbox->clickedButton()==&RecupIniBouton)
    {
        QFileDialog dialog(Q_NULLPTR, tr("Choisir le fichier d'initialisation"), PATH_DIR_RUFUS,"Text files (Rufus*.ini)");
        dialog.setViewMode(QFileDialog::List);
        dialog.setFileMode(QFileDialog::ExistingFile);
        int a = dialog.exec();
        if (a>0)
        {
            QFile FichierIni(PATH_FILE_INI);
            if (FichierIni.exists())
                FichierIni.remove();
            QString fileini = dialog.selectedFiles().at(0);
            QFile rufusini(fileini);
            rufusini.copy(PATH_FILE_INI);
            m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
            if (QMessageBox::question(Q_NULLPTR,"", tr("Restaurer aussi les fichiers modèles d'impression?")) == QMessageBox::Yes)
                PremierParametrageRessources();
            reponse = true;
        }
    }
    else if (msgbox->clickedButton()==&ReconstruitIniBouton)
    {
        //reconstruire le fichier rufus.ini
        //1. on demande les paramètres de connexion au serveur - mode d'accès / user / mdp / port / SSL
        QFile(PATH_FILE_INI).remove();
        m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
        QString login(""), MDP ("");
        if (VerifParamConnexion(login, MDP, true))
        {
            PremierParametrageMateriel(false);
            UpMessageBox::Watch(Q_NULLPTR,tr("Le fichier Rufus.ini a été reconstruit"), tr("Le programme va se fermer pour que certaines données puissent être prises en compte"));
            exit(0);
        }
    }
    else if (msgbox->clickedButton()==&PremierDemarrageBouton)
        reponse = PremierDemarrage();
    return reponse;
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Vérifie et répare les paramètres de connexion  -----------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifParamConnexion(QString &login, QString &MDP, bool connectavecLoginSQL, bool OKAccesDistant)
{
    dlg_paramconnexion *Dlg_ParamConnex = new dlg_paramconnexion(connectavecLoginSQL,  OKAccesDistant);
    Dlg_ParamConnex ->setWindowTitle(tr("Entrez les paramètres de connexion au serveur"));
    Dlg_ParamConnex ->setFont(m_applicationfont);
    connect(Dlg_ParamConnex, &dlg_paramconnexion::verifbase, this, &Procedures::VerifBaseEtRessources);

    if (Dlg_ParamConnex->exec()>0)
    {
        QString Base;
        if (Dlg_ParamConnex->ui->PosteradioButton->isChecked())
        {
            Base = Utils::getBaseFromMode(Utils::Poste);
            db->setModeacces(Utils::Poste);
        }
        else if (Dlg_ParamConnex->ui->LocalradioButton->isChecked())
        {
            Base = Utils::getBaseFromMode(Utils::ReseauLocal);
            m_settings->setValue(Base + "/Serveur",   Utils::calcIP(Dlg_ParamConnex->ui->IPlineEdit->text(), false));
            db->setModeacces(Utils::ReseauLocal);
        }
        else if (Dlg_ParamConnex->ui->DistantradioButton->isChecked())
        {
            Base = Utils::getBaseFromMode(Utils::Distant);
            m_settings->setValue(Base + "/Serveur",    Utils::calcIP(Dlg_ParamConnex->ui->IPlineEdit->text(), false));
            db->setModeacces(Utils::Distant);
        }
        m_settings->setValue(Base + "/Active",    "YES");
        m_settings->setValue(Base + "/Port", Dlg_ParamConnex->ui->PortcomboBox->currentText());

        m_connexionbaseOK = true;
        MDP = Dlg_ParamConnex->ui->MDPlineEdit->text();
        login = Dlg_ParamConnex->ui->LoginlineEdit->text();
        delete Dlg_ParamConnex;
        return true;
    }
    disconnect(Dlg_ParamConnex, &dlg_paramconnexion::verifbase, this, &Procedures::VerifBaseEtRessources);
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
            if (!DirRssces.exists(PATH_DIR_RESSOURCES))
                DirRssces.mkdir(PATH_DIR_RESSOURCES);
            foreach (const QString &nomfic, dockdir.entryList())
            {
                QFile ficACopier(dockdir.absolutePath() + "/" + nomfic);
                QString nomficACopier = QFileInfo(nomfic).fileName();
                ficACopier.copy(PATH_DIR_RESSOURCES + nomficACopier);
            }
            return true;
        }
        else return false;
    }
    return true;
}


/*! ------------------------------------------------------------------------------------------------------------------------------------------
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
    Datas::I()->mesureacuite->cleandatas();
    Datas::I()->mesurefinal->cleandatas();
    LectureDonneesRefracteur(m_mesureSerie);
    if ( Datas::I()->mesureacuite->isdataclean() && Datas::I()->mesurefinal->isdataclean() )
        return;
    if ( !Datas::I()->mesureacuite->isdataclean() && !Datas::I()->mesurefinal->isdataclean() )
        InsertMesure(Refracteur);
    emit NouvMesure(Refracteur);
}

void Procedures::RegleRefracteur()
{
    /*! Si on lance cette fonction à l'ouverture d'un dossier, on a créé 3 mesures
     * Chacune de ces 3 mesures est envoyée au réfracteur pour le régler
     * Datas::I()->mesurefronto     qui met en fronto et en final la dernière prescription de verres du patient
     * Datas::I()->mesureautoref    qui met en autoref la dernière mesure d'autoref du patient
     * Datas::I()->mesureacuité     qui met en subjectif la dernière mesure d'acuité du patient
     */
    /*! +++ sur les NIDEK, on ne peut que régler l'autoref et le fronto depuis le PC - les refractions subjectives et finales ne peuvent pas être préréglées */
    if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100"
     || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
    {
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

            convertaxe(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
            convertaxe(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
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

/*!
 * \brief Procedures::status
 * génére un résumé des informations de l'utilisateur sur la session courante.
 * \return Chaine de caractères
 */
QString Procedures::currentuserstatus() const
{
    const User *usr = Datas::I()->users->userconnected();
    if (usr == Q_NULLPTR)
        return "";
    QString str = "" +
            tr("utilisateur") + "\t\t= " + usr->login()  + "\n";

    //qDebug() << "superviseur " << usr->idsuperviseur();
    //qDebug() << "parent " << usr->idparent();
    //qDebug() << "comptable " << usr->idcomptable();
    QString strSup = "";
    if ( usr->idsuperviseur() == User::ROLE_NON_RENSEIGNE )           // le user est soignant, assistant et travaille pour plusieurs superviseurs
        strSup = tr("tout le monde");
    else if ( usr->idsuperviseur() == User::ROLE_VIDE )               // le user est un administratif
        strSup = tr("sans objet");
    else if ( usr->idsuperviseur() == User::ROLE_INDETERMINE )        // jamais utilisé
        strSup = tr("indéterminé");
    else if ( Datas::I()->users->getById(usr->idsuperviseur()) != Q_NULLPTR )
        strSup = Datas::I()->users->getById(usr->idsuperviseur())->login();
    str += tr("superviseur") + "\t\t= " + strSup + "\n";

    QString strParent = "";
    if ( usr->idparent() == User::ROLE_NON_RENSEIGNE )                    // le user est soignant, assistant, travaille pour plusieurs superviseurs
        strParent = tr("sans objet");
    else if ( usr->idparent() == User::ROLE_VIDE )                        // le user est un administratif
        strParent = tr("sans objet");
    else if ( usr->idparent() == User::ROLE_INDETERMINE )                 // jamais utilisé
        strParent = tr("indéterminé");
    else if ( Datas::I()->users->getById(usr->idparent()) != Q_NULLPTR )
        strParent = Datas::I()->users->getById(usr->idparent())->login();
    str += tr("parent") + "\t\t= " + strParent + "\n";

    QString strComptable = "";
    User * usrcptble = Datas::I()->users->getById(usr->idcomptable());
    if ( usr->idcomptable() == User::ROLE_NON_RENSEIGNE )
        strComptable = tr("sans objet");
    else if ( usr->idcomptable() == User::ROLE_VIDE )
        strComptable = tr("sans objet");
    else if ( usr->idcomptable() == User::ROLE_INDETERMINE )
        strComptable = tr("indéterminé");
    str += tr("comptable") + "\t\t= " + (usrcptble? strComptable : "null") + "\n";
    if ( usrcptble )
    {
        Compte * cpt = Datas::I()->comptes->getById(usrcptble->idcompteencaissementhonoraires());
        str += tr("cpte banque") + "\t= " + (cpt? cpt->nomabrege() : "null") + "\n";
    }

    QString strCompta = "";
    if ( usr->typecompta() == User::COMPTA_AVEC_COTATION_AVEC_COMPTABILITE )
        strCompta = "avec cotation et comptabilité";
    else if( usr->typecompta() == User::COMPTA_SANS_COTATION_SANS_COMPTABILITE )
        strCompta = "sans cotation ni comptabilité";
    else if( usr->typecompta() == User::COMPTA_AVEC_COTATION_SANS_COMPTABILITE )
        strCompta = "avec cotation sans comptabilité";
    else if( usr->typecompta() == User::COMPTA_SANS_COTATION_AVEC_COMPTABILITE )
        strCompta = "sans cotation avec comptabilité";
    str += tr("comptabilité") + "\t= " + strCompta;

    return str;
}

void Procedures::debugMesure(QObject *mesure, QString titre)
{
    if (titre != "")
        qDebug() << titre;
    Keratometrie *ker = qobject_cast<Keratometrie *>(mesure);
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

void Procedures::LectureDonneesRefracteur(QString Mesure)
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
    QString AVLOD(""), AVLOG(""), AVPOD(""), AVPOG("");
    QString PD          = "";
    int     idx;

    QString mesureOD(""), mesureOG("");
    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    QString mTOOD(""), mTOOG("");

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

        //! Données du FRONTO ---------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@LM") && PortFronto() == Q_NULLPTR)                 //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur
        {
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
                Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
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
                Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
            }
            //debugMesureRefraction(Datas::I()->mesurefronto);
            if (Datas::I()->mesurefronto->isDifferent(oldMesureFronto) && !Datas::I()->mesurefronto->isdataclean())
            {
                InsertMesure(Fronto);
                emit NouvMesure(Fronto);
            }
            delete oldMesureFronto;
        }

        //! Données de l'AUTOREF - REFRACTION et KERATOMETRIE ----------------------------------------------------------------------------------------------
        if (Mesure.contains("@KM") && PortAutoref() == Q_NULLPTR)                 //!=> il y a une mesure de keratométrie et l'autoref est connecté directement à la box du refraacteur
        {
            Keratometrie  *oldMesureKerato = new Keratometrie();
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
                Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
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
                Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                if (SectionKerato.contains("DL"))
                {
                    mesureOG        = SectionKerato.mid(SectionKerato.indexOf("DL")+2,10)   .replace(" ","0");
                    Datas::I()->mesurekerato->setdioptriesK1OG(mesureOG.mid(0,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OG(mesureOG.mid(5,5).toDouble());
                }
            }
            if (Datas::I()->mesurekerato->isDifferent(oldMesureKerato) && !Datas::I()->mesurekerato->isdataclean())
            {
                InsertMesure(Kerato);
                emit NouvMesure(Kerato);
            }
            delete oldMesureKerato;
        }
        if (Mesure.contains("@RM") && PortAutoref() == Q_NULLPTR)                 //!=> il y a une mesure de refractometrie et l'autoref est directement branché sur la box du refracteur
        {
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
                Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
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
                Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            }
            //debugMesureRefraction(Datas::I()->mesureautoref);
            if (Datas::I()->mesureautoref->isDifferent(oldMesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
            {
                InsertMesure(Autoref);
                emit NouvMesure(Autoref);
            }
            delete oldMesureAutoref;
        }

        //! Données du REFRACTEUR --------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@RT"))                 //=> il y a une mesure de refraction
        {
            idx                         = Mesure.indexOf("@RT");
            QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);

            // les données subjectives --------------------------------------------------------------------------------------------------------------
            if (SectionRefracteur.contains("fR") || SectionRefracteur.contains("fL"))
            {
                PD               = SectionRefracteur.mid(SectionRefracteur.indexOf("pD")+2,2);
                Datas::I()->mesureacuite->setecartIP(PD.toInt());
                // OEIL DROIT -----------------------------------------------------------------------------
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
            if (SectionRefracteur.contains("FR") || SectionRefracteur.contains("FL"))
            {
                PD               = SectionRefracteur.mid(SectionRefracteur.indexOf("PD")+2,2);
                Datas::I()->mesurefinal->setecartIP(PD.toInt());
                // OEIL DROIT -----------------------------------------------------------------------------
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

        //! Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@NT") && PortAutoref() == Q_NULLPTR )                 //!=> il y a une mesure de tonometrie et l'autoref est branché sur la box du refracteur
        {
            Datas::I()->mesuretono->cleandatas();
            idx                     = Mesure.indexOf("@NT");
            QString SectionTono     = Mesure.right(Mesure.length()-idx-5);
            SectionTono             = SectionTono.left(SectionTono.indexOf("@"));
            logmesure("LectureDonneesRefracteur() - " + SectionTono);
            //Edit(SectionTono+ "\nOK");
            // OEIL DROIT -----------------------------------------------------------------------------
            mTOOD                   = SectionTono.mid(SectionTono.indexOf("TR")+2,4)   .replace(" ","0");
            // OEIL GAUCHE ---------------------------------------------------------------------------
            mTOOG                   = SectionTono.mid(SectionTono.indexOf("TL")+2,4)   .replace(" ","0");
            logmesure("LectureDonneesRefracteur() - Section Tono - mTOOD = " + mTOOD + " - mTOOG = " + mTOOG);
            Datas::I()->mesuretono->setTOD(int(mTOOD.toDouble()));
            Datas::I()->mesuretono->setTOG(int(mTOOG.toDouble()));
            Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
            logmesure("LectureDonneesRefracteur() - nouvelle mesure tono -> TOD = " + QString::number(Datas::I()->mesuretono->TOD()) + " - TOG = " + QString::number(Datas::I()->mesuretono->TOG()));
            InsertMesure(Tono);                     //! depuis LectureDonneesRefracteur(QString Mesure)
            emit NouvMesure(Tono);
        }
        debugMesure(Datas::I()->mesurekerato, "Procedures::LectureDonneesRefracteur(QString Mesure)");
    }
    // FIN NIDEK RT-5100 et RT 2100 ==========================================================================================================================
}

// -------------------------------------------------------------------------------------
// Generation du resumé Html des données de réfraction sublective issues du refracteur
//--------------------------------------------------------------------------------------
QString Procedures::HtmlRefracteur()
{
    MesureRefraction *acuite = Datas::I()->mesureacuite;
    QString Reponse = "";
    // - 1 - détermination des verres
    QString mSphereOD   = Utils::PrefixePlus(acuite->sphereOD());
    QString mCylOD      = Utils::PrefixePlus(acuite->cylindreOD());
    QString mAxeOD      = QString::number(acuite->axecylindreOD());
    QString mAddOD      = Utils::PrefixePlus(acuite->addVPOD());
    QString mAVLOD ("");
    if (acuite->avlOD().toDouble()>0)
        mAVLOD          = QLocale().toString(acuite->avlOD().toDouble()*10) + "/10";
    QString mAVPOD      = acuite->avpOD();
    QString mSphereOG   = Utils::PrefixePlus(acuite->sphereOG());
    QString mCylOG      = Utils::PrefixePlus(acuite->cylindreOG());
    QString mAxeOG      = QString::number(acuite->axecylindreOG());
    QString mAddOG      = Utils::PrefixePlus(acuite->addVPOG());
    QString mAVLOG ("");
    if (acuite->avlOG().toDouble()>0)
        mAVLOG      = QLocale().toString(acuite->avlOG().toDouble()*10) + "/10";
    QString mAVPOG      = acuite->avpOG();
    QString ResultatVLOD, ResultatVLOG,ResultatVPOD, ResultatVPOG, ResultatOD, ResultatOG;

    // détermination OD
    if (acuite->cylindreOD() != 0.0 && acuite->sphereOD() != 0.0)
        ResultatVLOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
    else if (acuite->cylindreOD() == 0.0 && acuite->sphereOD() != 0.0)
        ResultatVLOD = mSphereOD;
    else if (acuite->cylindreOD() != 0.0 && acuite->sphereOD() == 0.0)
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
    if (acuite->cylindreOG() != 0.0 && acuite->sphereOG() != 0.0)
        ResultatVLOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + ")";
    else if (acuite->cylindreOG() == 0.0 && acuite->sphereOG() != 0.0)
        ResultatVLOG = mSphereOG;
    else if (acuite->cylindreOG() != 0.0 && acuite->sphereOG() == 0.0)
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
    if (acuite->addVPOD() > 0 || acuite->addVPOG() > 0)  // il y a eu mesure de près et de loin
    {
        if (ResultatOD != "Rien" && QLocale().toDouble(mAddOD) == 0.0  && ResultatOG == "Rien")
            Reponse = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " " + tr("OD");

        if (Reponse == "" && ResultatOD != "Rien" && (QLocale().toDouble(mAddOD) == 0.0 && QLocale().toDouble(mAddOG) == 0.0) && ResultatOG != "Rien")
            Reponse = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " "+ tr("OD") + "</td></p>"
                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG") + "</td>";

        if (Reponse == "" && ResultatOD != "Rien" && (QLocale().toDouble(mAddOD) == 0.0 && QLocale().toDouble(mAddOG) > 0) && ResultatOG != "Rien")
            Reponse = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " " + tr("OD") + "</td></p>"
                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP "+ tr("OG") + "</td>";

        if (Reponse == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0  && ResultatOG == "Rien")
            Reponse = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD");

        if (Reponse == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0 && QLocale().toDouble(mAddOG) == 0.0 && ResultatOG != "Rien")
            Reponse = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD") + "</td></p>"
                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG") + "</td>";

        if (Reponse == "" && ResultatOD != "Rien" && QLocale().toDouble(mAddOD) > 0 && QLocale().toDouble(mAddOG) > 0 && ResultatOG != "Rien")
            Reponse = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + mAVLOD + "</font><font color = " + colorVPOD + mAVPOD + "</font></b>" + " add." + mAddOD + "VP " + tr("OD") + "</td></p>"
                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP " + tr("OG") + "</td>";

        if (Reponse == "" && (ResultatOD == "Rien" &&  QLocale().toDouble(mAddOG) == 0.0) && ResultatOG != "Rien")
            Reponse = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " " + tr("OG");

        if (Reponse == "" && (ResultatOD == "Rien" &&  QLocale().toDouble(mAddOG) > 0) && ResultatOG != "Rien")
            Reponse = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + mAVLOG + "</font><font color = " + colorVPOG + mAVPOG + "</font></b>" + " add." + mAddOG + "VP " + tr("OG");
    }
    else
    {
        if (Reponse == "" && ResultatOD != "Rien" && ResultatOG == "Rien")
            Reponse = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + mAVLOD + "</b></font> " + tr("OD");

        if (Reponse == "" && ResultatOD != "Rien" && ResultatOG != "Rien")
            Reponse = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + mAVLOD + "</b></font> " + tr("OD") + "</td></p>"
                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                    + ResultatVLOG + " " + "<font color = " + colorVLOG + "><b>" + mAVLOG + "</b></font> " + tr("OG") + "</td>";

        if (Reponse == "" && ResultatOD == "Rien" && ResultatOG != "Rien")
            Reponse = ResultatVLOG + "<font color = " + colorVLOG + "><b>" + mAVLOG + "</b></font> " + tr("OG");
    }
    Reponse = HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Reponse + "</td><td width=\"70\"><font color = \"red\"></font></td><td>" + currentuser()->login() + "</td></p>";
    acuite = Q_NULLPTR;
    return Reponse;
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
    Datas::I()->mesurefronto->cleandatas();
    LectureDonneesFronto(m_mesureSerie);
    if (Datas::I()->mesurefronto->isdataclean())
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (t_threadRefracteur != Q_NULLPTR && !FicheRefractionOuverte())
    {
        m_flagreglagerefracteur = Fronto;
        // NIDEK RT-5100
        if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            PortRefracteur()->clear();
            PortRefracteur()->write(RequestToSendNIDEK());
            PortRefracteur()->waitForBytesWritten(100);
        }
        InsertMesure(Fronto);
    }
    emit NouvMesure(Fronto);
}

void Procedures::LectureDonneesFronto(QString Mesure)
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
            mesureOD            = Mesure.mid(idxOD+2,15)   .replace(" ","0");
            mSphereOD            = mesureOD.mid(0,6);
            mCylOD               = mesureOD.mid(6,6);
            mAxeOD               = mesureOD.mid(12,3);
            if (Mesure.indexOf("AR")>0)
                mAddOD           = Utils::PrefixePlus(Mesure.mid(Mesure.indexOf("AR")+2,4).toDouble());
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        int idxOG = Mesure.indexOf("LL");
        if (idxOG > 0)
        {
            mesureOG            = Mesure.mid(idxOG+2,15)   .replace(" ","0");
            mSphereOG            = mesureOG.mid(0,6);
            mCylOG               = mesureOG.mid(6,6);
            mAxeOG               = mesureOG.mid(12,3);
            if (Mesure.indexOf("AL")>0)
                mAddOG           = Utils::PrefixePlus(Mesure.mid(Mesure.indexOf("AL")+2,4).toDouble());
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
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
            mesureOD            = Mesure.mid(idxOD+3);
            mSphereOD           = mesureOD.mid(mesureOD.indexOf("S=")+2,6);
            mCylOD              = mesureOD.mid(mesureOD.indexOf("C=")+2,6);
            mAxeOD              = mesureOD.mid(mesureOD.indexOf("A=")+2,3);
            mAddOD              = mesureOD.mid(mesureOD.indexOf("ADD=")+4,5);
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        int idxOG = Mesure.indexOf("L: ");
        if (idxOG > 0)
        {
            mesureOG            = Mesure.mid(idxOG+3);
            mSphereOG           = mesureOG.mid(mesureOG.indexOf("S=")+2,6);
            mCylOG              = mesureOG.mid(mesureOG.indexOf("C=")+2,6);
            mAxeOG              = mesureOG.mid(mesureOG.indexOf("A=")+2,3);
            mAddOG              = mesureOG.mid(mesureOG.indexOf("ADD=")+4,5);
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
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
            mesureOD            = Mesure.mid(idxOD+2,15)   .replace(" ","0");
            mSphereOD            = mesureOD.mid(0,6);
            mCylOD               = mesureOD.mid(6,6);
            mAxeOD               = mesureOD.mid(12,3);
            if (Mesure.indexOf("AR")>0)
                mAddOD           = Mesure.mid(Mesure.indexOf("AR")+2,5);
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        int idxOG = Mesure.indexOf(" L");
        if (idxOG > 0)
        {
            mesureOG            = Mesure.mid(idxOG+2,15)   .replace(" ","0");
            mSphereOG            = mesureOG.mid(0,6);
            mCylOG               = mesureOG.mid(6,6);
            mAxeOG               = mesureOG.mid(12,3);
            if (Mesure.indexOf("AL")>0)
                mAddOG           = Mesure.mid(Mesure.indexOf("AL")+2,5);
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
    }
}

// -------------------------------------------------------------------------------------
// Generation du resumé des données issues du frontocomètre
//--------------------------------------------------------------------------------------
QString Procedures::HtmlFronto()
{
    MesureRefraction *fronto = Datas::I()->mesurefronto;
    QString mSphereOD   = Utils::PrefixePlus(fronto->sphereOD());
    QString mCylOD      = Utils::PrefixePlus(fronto->cylindreOD());
    QString mAxeOD      = QString::number(fronto->axecylindreOD());
    QString mAddOD      = Utils::PrefixePlus(fronto->addVPOD());
    QString mSphereOG   = Utils::PrefixePlus(fronto->sphereOG());
    QString mCylOG      = Utils::PrefixePlus(fronto->cylindreOG());
    QString mAxeOG      = QString::number(fronto->axecylindreOG());
    QString mAddOG      = Utils::PrefixePlus(fronto->addVPOG());

    QString ResultatOD, ResultatVLOD, ResultatVPOD;
    QString ResultatOG, ResultatVLOG, ResultatVPOG;
    QString Reponse = "";

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
            Reponse = ResultatOD + " " + tr("ODG");
        else
            Reponse = ResultatVLOD + " VL " + tr("ODG");
    }
    else
        Reponse = ResultatOD + " / " + ResultatOG;
    fronto = Q_NULLPTR;
    return HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("Porte") + ":</b></font></td><td>" + Reponse + "</p>";
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
    bool autorefhaskerato    = (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
                      || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
                      || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
                      || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
                      || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
                      || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
                      || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
                      || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30");
    bool autorefhastonopachy = (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III");

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
            cmd = (autorefhaskerato? "CRK" : "CRM");     //! CRK ou CRM suivant que les appareils peuvent ou non envoyer la keratométrie
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            PortAutoref()->clear();
            PortAutoref()->write(SendDataNIDEK(cmd));
            PortAutoref()->waitForBytesWritten(100);
            return;
        }
    }

    Datas::I()->mesureautoref   ->cleandatas();
    if (autorefhaskerato)
        Datas::I()->mesurekerato    ->cleandatas();
    if (autorefhastonopachy)
    {
        Datas::I()->mesurepachy     ->cleandatas();
        Datas::I()->mesuretono      ->cleandatas();
    }

    LectureDonneesAutoref(m_mesureSerie);
    if ( !autorefhaskerato && !autorefhastonopachy && Datas::I()->mesureautoref->isdataclean())
        return;
    else if (autorefhaskerato && Datas::I()->mesureautoref->isdataclean() && Datas::I()->mesurekerato->isdataclean())
        return;
    else if (autorefhastonopachy && Datas::I()->mesureautoref->isdataclean()
        &&  Datas::I()->mesurekerato   ->isdataclean()
        &&  Datas::I()->mesuretono      ->isdataclean()
        &&  Datas::I()->mesurepachy     ->isdataclean())
        return;
    //TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (t_threadRefracteur != Q_NULLPTR && !FicheRefractionOuverte())
    {
        m_flagreglagerefracteur = Autoref;
        // NIDEK RT-5100 - NIDEK RT-2100
        if (m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-5100" || m_settings->value("Param_Poste/Refracteur").toString()=="NIDEK RT-2100")
        {
            if (!Datas::I()->mesurekerato->isdataclean())
                InsertMesure(Kerato);
            if (!Datas::I()->mesureautoref->isdataclean())
                InsertMesure(Autoref);
            //Dans un premier temps, le PC envoie la requête d'envoi de données
            PortRefracteur()->clear();
            PortRefracteur()->write(RequestToSendNIDEK());
            PortRefracteur()->waitForBytesWritten(100);
        }
    }
    if (autorefhaskerato && !Datas::I()->mesurekerato->isdataclean())
        emit NouvMesure(Kerato);
    if (!Datas::I()->mesureautoref->isdataclean())
        emit NouvMesure(Autoref);
    if (autorefhastonopachy)
    {
        if (!Datas::I()->mesuretono->isdataclean())
        {
            InsertMesure(Tono);                     //! depuis ReponsePortSerie_Autoref(const QString &s)
            emit NouvMesure(Tono);
        }
        if (!Datas::I()->mesurepachy->isdataclean())
        {
            InsertMesure(Pachy);                    //! depuis ReponsePortSerie_Autoref(const QString &s)
            emit NouvMesure(Pachy);
        }
    }
}

void Procedures::LectureDonneesAutoref(QString Mesure)
{
    Logs::LogToFile("MesuresAutoref.txt", Mesure);
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

/*! NIDEK ARK-1a - exemple de fichier de sortie *//*

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

*//*! Exemple de fichier de sortie pour un TONOREFIII *//*

DRMIDNIDEK/TONOREF3
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

*//*! Exemple de fichier de sortie avec seul l'OG mesuré pour un ARK1A *//*

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
        bool autorefhaskerato    = (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1A"
                          || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1"
                          || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-1S"
                          || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-530A"
                          || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-510A"
                          || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK HandyRef-K"
                          || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III"
                          || m_settings->value("Param_Poste/Autoref").toString()=="NIDEK ARK-30");
        bool autorefhastonopachy = (m_settings->value("Param_Poste/Autoref").toString()=="NIDEK TONOREF III");
        bool autorefhasipmesure = (m_settings->value("Param_Poste/Autoref").toString() != "NIDEK HandyRef-K"
                                || m_settings->value("Param_Poste/Autoref").toString() != "NIDEK ARK-30"
                                || m_settings->value("Param_Poste/Autoref").toString() != "NIDEK AR-20");

        a               = Mesure.indexOf("VD");
        a               = Mesure.length() - a -1;
        QString Ref("");
        Ref             = Mesure.right(a);
        if (Ref != "")
        {
            a  = Ref.indexOf("OR");
            // OEIL DROIT -----------------------------------------------------------------------------
            if (a>=0)
            {
                QString mesureOD("");
                QString mSphereOD   = "+00.00";
                QString mCylOD      = "+00.00";
                QString mAxeOD      = "000";
                mesureOD            = Ref.mid(Ref.indexOf("OR")+2,15)   .replace(" ","0");
                mSphereOD           = mesureOD.mid(0,6);
                mCylOD              = mesureOD.mid(6,6);
                mAxeOD              = mesureOD.mid(12,3);
                Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
                Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
                Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            a  = Ref.indexOf("OL");
            if (a>=0)
            {
                QString mesureOG("");
                QString mSphereOG   = "+00.00";
                QString mCylOG      = "+00.00";
                QString mAxeOG      = "000";
                mesureOG            = Ref.mid(Ref.indexOf("OL")+2,15)   .replace(" ","0");
                mSphereOG           = mesureOG.mid(0,6);
                mCylOG              = mesureOG.mid(6,6);
                mAxeOG              = mesureOG.mid(12,3);
                Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
                Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
                Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            }
            if (autorefhasipmesure)
            {
                a  = Ref.indexOf("PD");
                if (a >= 0) {
                    QString PD      = Ref.mid(Ref.indexOf("PD")+2,2);
                    Datas::I()->mesureautoref->setecartIP(PD.toInt());
                }
            }
            if (autorefhaskerato)
            {
                // Données de KERATOMETRIE --------------------------------------------------------------------------------------------------------
                if (Mesure.contains("DKM"))                 //=> il y a une mesure de keratometrie
                {
                    a                   = Mesure.indexOf("DKM");
                    a                   = Mesure.length() - a;
                    QString K           = Mesure.right(a);
                    // OEIL DROIT -----------------------------------------------------------------------------
                    a  = Ref.indexOf(" R");
                    if (a>=0)
                    {
                        QString KOD("");
                        QString K1OD("null"), K2OD("null");
                        int     AxeKOD(0);
                        KOD                 = K.mid(K.indexOf(" R")+2,13);
                        K1OD                = KOD.mid(0,5);
                        K2OD                = KOD.mid(5,5);
                        AxeKOD              = KOD.mid(10,3).toInt();
                        Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                        Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                        Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                        QString mOD         = K.mid(K.indexOf("DR")+2,10).replace(" ","0");
                        Datas::I()->mesurekerato->setdioptriesK1OD(mOD.mid(0,5).toDouble());
                        Datas::I()->mesurekerato->setdioptriesK2OD(mOD.mid(5,5).toDouble());
                    }
                    // OEIL GAUCHE ---------------------------------------------------------------------------
                    a  = Ref.indexOf(" L");
                    if (a>=0)
                    {
                        QString KOG("");
                        QString K1OG("null"), K2OG("null");
                        int     AxeKOG(0);
                        KOG                 = K.mid(K.indexOf(" L")+2,13);
                        K1OG                = KOG.mid(0,5);
                        K2OG                = KOG.mid(5,5);
                        AxeKOG              = KOG.mid(10,3).toInt();
                        Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                        Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                        Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                        QString mOG         = K.mid(K.indexOf("DL")+2,10).replace(" ","0");
                        Datas::I()->mesurekerato->setdioptriesK1OG(mOG.mid(0,5).toDouble());
                        Datas::I()->mesurekerato->setdioptriesK2OG(mOG.mid(5,5).toDouble());
                    }
                }
            }
        }
        if (autorefhastonopachy)
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
                Datas::I()->mesuretono->setTOD(TonoOD.toInt());
                Datas::I()->mesuretono->setTOG(TonoOG.toInt());
                Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
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
                Datas::I()->mesurepachy->setpachyOD(PachyOD.toInt());
                Datas::I()->mesurepachy->setpachyOG(PachyOG.toInt());
                Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
            }
        }
    }
    //qDebug() << "od" << mSphereOD << mCylOD << mAxeOD << "og" << mSphereOG << mCylOG << mAxeOG << "PD = " + PD;
}


// -------------------------------------------------------------------------------------
// Generation du resumé de l'autorefractometre
//--------------------------------------------------------------------------------------
QString Procedures::HtmlAutoref()
{
    MesureRefraction *autoref = Datas::I()->mesureautoref;
    QString ResultatOD("");
    QString ResultatOG("");
    QString Reponse = "";

    // détermination OD
    if (!autoref->isnullLOD())
    {
        QString mSphereOD   = Utils::PrefixePlus(autoref->sphereOD());
        QString mCylOD      = Utils::PrefixePlus(autoref->cylindreOD());
        QString mAxeOD      = QString::number(autoref->axecylindreOD());
        if (autoref->cylindreOD() != 0.0 && autoref->sphereOD() != 0.0)
            ResultatOD = mSphereOD + " (" + mCylOD + tr(" à ") + mAxeOD + "°)";
        else if (autoref->cylindreOD() == 0.0 && autoref->sphereOD() != 0.0)
            ResultatOD = mSphereOD;
        else if (autoref->cylindreOD() != 0.0 && autoref->sphereOD() == 0.0)
            ResultatOD = mCylOD + tr(" à ") + mAxeOD + "°" ;
        else
            ResultatOD = tr("plan");
    }
    // détermination OG
    if (!autoref->isnullLOG())
    {
        QString mSphereOG   = Utils::PrefixePlus(autoref->sphereOG());
        QString mCylOG      = Utils::PrefixePlus(autoref->cylindreOG());
        QString mAxeOG      = QString::number(autoref->axecylindreOG());
        if (autoref->cylindreOG() != 0.0 && autoref->sphereOG() != 0.0)
            ResultatOG = mSphereOG + " (" + mCylOG + tr(" à ") + mAxeOG + "°)";
        else if (autoref->cylindreOG() == 0.0 && autoref->sphereOG() != 0.0)
            ResultatOG = mSphereOG;
        else if (autoref->cylindreOG() != 0.0 && autoref->sphereOG() == 0.0)
            ResultatOG = mCylOG + tr(" à ") + mAxeOG + "°" ;
        else
            ResultatOG = tr("plan");
    }

    // Détermination de Resultat
    if (ResultatOD == "" && ResultatOG == "")
        return "";
    else if (ResultatOD == ResultatOG)
            Reponse = ResultatOD + " " + tr("ODG");
    else if (ResultatOD == "" || ResultatOG == "")
    {
        Reponse = ResultatOD + ResultatOG;
        if (ResultatOD == "")
            Reponse += tr("OG");
        else
            Reponse += tr("OD");
    }
    else
        Reponse = ResultatOD + " / " + ResultatOG;
    autoref = Q_NULLPTR;
    return  HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>"
                           + tr("Autoref") + ":</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Reponse + "</td></p>";
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la keratométrie
//--------------------------------------------------------------------------------------
QString Procedures::HtmlKerato()
{
    Keratometrie *kerato = Datas::I()->mesurekerato;
    QString Reponse = "";
    if (!kerato->isnullLOD())
    {
        QString mK1OD       = QLocale().toString(kerato->K1OD(),'f',2);
        QString mK2OD       = QLocale().toString(kerato->K2OD(),'f',2);
        QString mKOD        = QLocale().toString(kerato->KMOD(),'f',2);
        QString mAxeKOD     = QString::number(kerato->axeKOD());
        QString mDioptrK1OD = QLocale().toString(kerato->dioptriesK1OD(),'f',1);
        QString mDioptrK2OD = QLocale().toString(kerato->dioptriesK2OD(),'f',1);
        QString mDioptrKOD  = Utils::PrefixePlus(kerato->dioptriesKOD());
        QString mDioptrmOD  = QString::number(kerato->dioptriesKMOD(),'f',2);
        if (QLocale().toDouble(mDioptrK1OD)!=0.0)
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OD + "/" + mK2OD + " Km = " + mKOD + "</td>"
                      "<td width=\"180\">" + mDioptrK1OD + "/" + mDioptrK2OD + " " + mDioptrKOD +  tr(" à ") + mAxeKOD + "°</td></p>";
        else
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OD + tr(" à ") + mAxeKOD + "°/" + mK2OD
                      + " Km = " + mDioptrmOD + "</td></p>";
    }
    if (!kerato->isnullLOG())
    {
        QString mK1OG       = QLocale().toString(kerato->K1OG(),'f',2);
        QString mK2OG       = QLocale().toString(kerato->K2OG(),'f',2);
        QString mKOG        = QLocale().toString(kerato->KMOG(),'f',2);
        QString mAxeKOG     = QString::number(kerato->axeKOG());
        QString mDioptrK1OG = QLocale().toString(kerato->dioptriesK1OG(),'f',1);
        QString mDioptrK2OG = QLocale().toString(kerato->dioptriesK2OG(),'f',1);
        QString mDioptrKOG  = Utils::PrefixePlus(kerato->dioptriesKOG());
        QString mDioptrmOG  = QString::number(kerato->dioptriesKMOG(),'f',2);
        if (QLocale().toDouble(mDioptrK1OG)!=0.0)
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OG + "/" + mK2OG + " Km = " + mKOG + "</td>"
                      "<td width=\"180\">" + mDioptrK1OG + "/" + mDioptrK2OG + " " + mDioptrKOG +  tr(" à ") + mAxeKOG + "°</td></p>";
        else
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"180\">"  + mK1OG +  tr(" à ") + mAxeKOG + "°/" + mK2OG
                      + " Km = " + mDioptrmOG + "</td></p>";
    }
    kerato = Q_NULLPTR;
    return Reponse;
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour la tonométrie
//--------------------------------------------------------------------------------------
QString Procedures::HtmlTono()
{
    Tonometrie *tono = Datas::I()->mesuretono;
    QString Reponse = "";
    if (!tono->isnullLOD() || !tono->isnullLOG())
    {
        QString const dd    = "<a name=\"" HTMLANCHOR_TODEBUT + QString::number(tono->id()) + "\"></a>";
        QString const fd    = "<a name=\"" HTMLANCHOR_TOFIN "\"></a>";
        QString larg =  "190";
        QString title = HTML_RETOURLIGNE "<td width=\"" + larg + "\"><b><font color = \"" COULEUR_TITRES "\">";
        QString Methode = Tonometrie::ConvertMesure(tono->modemesure());
        QString Tono, color;
        color = (tono->TOD() > 21? "red" : "blue");
        QString TODcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOD()) + "</font>";
        color = (tono->TOG() > 21? "red" : "blue");
        QString TOGcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOG()) + "</font>";
        if (tono->TOD() == 0 && tono->TOG() > 0)
            Tono = dd + tr("TOG:") + "</font> "
                    + TOGcolor + "</b>" + tr(" à ")
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        else if (tono->TOG() == 0 && tono->TOD() > 0)
            Tono = dd + tr("TOD:") + "</font> "
                    + TODcolor + "</b>" + tr(" à ")
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        else if (tono->TOD() == tono->TOG())
            Tono = dd + tr("TODG:") + "</font> "
                    + TODcolor + "</b>" + tr(" à ")
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        else
            Tono = dd + tr("TO:") + "</font> " + TODcolor + "/"
                    + TOGcolor + "</b>" + tr(" à ")
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        Reponse = title + Tono + "</td></p>";
        Reponse.insert(Reponse.lastIndexOf("</td>")-1, fd);             //! on met le dernier caractère en ancre
    }
    logmesure("setHtmlTono() -> new m_htmlMesureTono = " + Reponse);
    tono = Q_NULLPTR;
    return Reponse;
}

QString Procedures::HtmlPachy()
{
    Pachymetrie *pachy = Datas::I()->mesurepachy;
    QString Reponse ("");
    if (!pachy->isnullLOD() || !pachy->isnullLOG())
    {
        QString const dd    = "<a name=\"" HTMLANCHOR_PACHYDEBUT + QString::number(pachy->id()) + "\"></a>";
        QString const fd    = "<a name=\"" HTMLANCHOR_PACHYFIN "\"></a>";
        QString pachyOD, pachyOG, Methode;
        pachyOD = QString::number(pachy->pachyOD());
        pachyOG = QString::number(pachy->pachyOG());
        Methode = Pachymetrie::ConvertToReadableMesure(pachy);
        if (pachy->isnullLOD())
            Reponse = "<b><font color = \"" COULEUR_TITRES "\">" + tr("pachy OG") + ":</font> <font color = \"green\">" + pachyOG + "</font></b> (" + Methode;
        else if (pachy->isnullLOG())
            Reponse = "<b><font color = \"" COULEUR_TITRES "\">" + tr("pachy OD") + ":</font> <font color = \"green\">" + pachyOD + "</font></b> (" + Methode;
        else
        {
            if (pachy->pachyOD() == pachy->pachyOG())
                Reponse = "<b><font color = \"" COULEUR_TITRES "\">" + tr("pachy ODG") + ":</font> <font color = \"green\">" + pachyOD + "</font></b> (" + Methode;
            else
                Reponse = "<b><font color = \"" COULEUR_TITRES "\">" + tr("pachy") + ":</font> <font color = \"green\">" + pachyOD +  "/" + pachyOG + "</font></b> (" + Methode;
        }
        Reponse = dd + Reponse + fd + +")";           // on met le dernier caractère en ancre
    }
    pachy = Q_NULLPTR;
    return Reponse;
}

QSerialPort* Procedures::PortTono()
{
    return sp_portTono;
}

Procedures::TypeMesure Procedures::ConvertMesure(QString Mesure)
{
    if (Mesure == "P") return Fronto;
    if (Mesure == "A") return Autoref;
    if (Mesure == "R") return Refracteur;
    return  None;
}

QString Procedures::ConvertMesure(TypeMesure Mesure)
{
    switch (Mesure) {
    case Fronto:        return "P";
    case Autoref:       return "A";
    case Refracteur:    return "R";
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
void Procedures::InsertMesure(TypeMesure typemesure)
{
    if (Datas::I()->patients->currentpatient() == Q_NULLPTR)
        return;
    if (Datas::I()->patients->currentpatient()->isnull())
        return;
    if (Datas::I()->Datas::I()->actes->currentacte() == Q_NULLPTR)
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
    else if (typemesure == Refracteur)
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

        QString requete = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(Refracteur) + "'";
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
                    " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(Refracteur) + "'";

            db->StandardSQL (requete, tr("Erreur de mise à jour de données de refraction dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == Tono)
    {
        Datas::I()->mesuretono->setidpatient(Datas::I()->patients->currentpatient()->id());
        db->locktable(TBL_TONOMETRIE);
        QString req = "INSERT INTO " TBL_TONOMETRIE " (idPat, TOOD, TOOG, TODate, TOType) VALUES  ("
                + QString::number(Datas::I()->mesuretono->idpatient()) + ","
                + QString::number(Datas::I()->mesuretono->TOD()) + ","
                + QString::number(Datas::I()->mesuretono->TOG())
                + ", now(), '" + Tonometrie::ConvertMesure(Datas::I()->mesuretono->modemesure()) + "')";
        db->StandardSQL(req,tr("Impossible de sauvegarder la mesure!"));
        bool ok;
        Datas::I()->mesuretono->setid(db->selectMaxFromTable(CP_ID_TONO, TBL_TONOMETRIE,ok));
        db->unlocktables();
    }
    else if (typemesure == Pachy)
    {
        Datas::I()->mesurepachy->setidpatient(Datas::I()->patients->currentpatient()->id());
        db->locktable(TBL_PACHYMETRIE);
        QString req = "INSERT INTO " TBL_PACHYMETRIE " (idPat, pachyOD, pachyOG, pachyDate, pachyType) VALUES  ("
                + QString::number(Datas::I()->mesurepachy->idpatient()) + ","
                + QString::number(Datas::I()->mesurepachy->pachyOD()) + ","
                + QString::number(Datas::I()->mesurepachy->pachyOG())
                + ", now(), '" + Pachymetrie::ConvertMesure(Datas::I()->mesurepachy->modemesure()) + "')";
        DataBase::I()->StandardSQL(req,tr("Impossible de sauvegarder la mesure!"));
        bool ok;
        Datas::I()->mesurepachy->setid(db->selectMaxFromTable(CP_ID_PACHY, TBL_PACHYMETRIE,ok));
        db->unlocktables();
    }
    if (typemesure != Fronto && typemesure != Tono && typemesure != Pachy)
        Datas::I()->patients->actualiseDonneesOphtaCurrentPatient();
}
