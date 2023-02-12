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
                  m_settings->value(Utils::getBaseFromMode(Utils::Poste) + Param_Active).toString() == "YES"
                  &&
                  ( ports.find(m_settings->value(Utils::getBaseFromMode(Utils::Poste) + Param_Port).toInt()) != ports.end() )
                )
                ||
                (
                  m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Active).toString() == "YES"
                  && m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Serveur).toString() != ""
                  && ( ports.find(m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Port).toInt()) != ports.end() )
                )
                ||
                (
                  m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Param_Active).toString() == "YES"
                  && m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Param_Serveur).toString() != ""
                  && ( ports.find(m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Param_Port).toInt()) != ports.end() )
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

    Ouverture_Appareils_Refraction();
    ReconstruitListeModesAcces();
    m_typemesureRefraction               = MesureNone;
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
    dlg_choixdate *Dlg_ChxDate = new dlg_choixdate(db->ServerDate(), parent);
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
    QString cle = Utils::getBaseFromMode( mode ) + Dossier_Documents + Appareil;
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
    for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it != Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
    {
        PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
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
        proc->ConnectTimers(true);
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
    {
        QString mdp("");
        if (!Utils::VerifMDP(MDPAdmin(),tr("Saisissez le mot de passe Administrateur"), mdp))
            return false;
    }
    ShowMessage::I()->PriorityMessage(tr("Sauvegarde en cours"),handledlg);
    emit ConnectTimers(false);

    //On vide les champs blob de la table factures et la table EchangeImages
    db->StandardSQL("UPDATE " TBL_FACTURES " SET " CP_JPG_FACTURES " = null, " CP_PDF_FACTURES " = null");
    db->StandardSQL("DELETE FROM " TBL_ECHANGEIMAGES);

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
        proc->ConnectTimers(true);
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
int Procedures::ExecuteSQLScript(QStringList ListScripts)
{
    QStringList listpaths;
    int a = 99;
    QString cheminmysql;
#ifdef Q_OS_MACX
    cheminmysql = "/usr/local/mysql/bin";           // Depuis HighSierra on ne peut plus utiliser + Dir.absolutePath() + DIR_LIBS2 - le script ne veut pas utiliser le client mysql du package (???)
    /*! cheminmysql = QCoreApplication::applicationDirPath()+ "/Contents/Applications";           // Depuis HighSierra on ne peut plus utiliser ce chemin - le script ne veut pas utiliser le client mysql du package (???)*/
#endif
#ifdef Q_OS_LINUX
    cheminmysql = "/usr/bin";
#endif
    QString host;
    if( db->ModeAccesDataBase() == Utils::Poste )
        host = "localhost";
    else
        host = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Serveur).toString();
    bool useSSL = (db->ModeAccesDataBase() == Utils::Distant);
    QString login = LOGIN_SQL;
    if (useSSL)
        login += "SSL";
    QString dirkey = "/etc/mysql";
    QString keys = "";
    if (useSSL)
    {
        if (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString() != "")
            dirkey = m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString();
        else
            m_settings->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL,dirkey);
        keys += " --ssl-ca=" + dirkey + "/ca-cert.pem --ssl-cert=" + dirkey + "/client-cert.pem --ssl-key=" + dirkey + "/client-key.pem";
    }
    QString command = cheminmysql + "/mysql -u " + login + " -p" MDP_SQL " -h " + host + " -P " + QString::number(db->port()) + keys;
    for (int i=0; i<ListScripts.size(); i++)
        if (QFile(ListScripts.at(i)).exists())
        {
            QString path = ListScripts.at(i);
            listpaths << path;
        }

    QProcess dumpProcess(parent());
    for (int i=0; i< listpaths.size(); i++)
    {
        QString path = listpaths.at(i);
        dumpProcess.setStandardInputFile(path);
        dumpProcess.start(command);
        dumpProcess.waitForFinished(1000000000); /*! sur des systèmes lents, la création de la base prend parfois plus que les 30 secondes que sont la valeur par défaut de l'instruction waitForFinished()
                                              * et dans ce cas le processus est interrompu avant que toute la base soit créée */
        //qDebug() << Utils::EnumDescription(QMetaEnum::fromType<QProcess::ExitStatus>(), dumpProcess.exitCode()) << "dumpProcess.exitCode()" << dumpProcess.exitCode() << dumpProcess.errorString();
        if (dumpProcess.exitStatus() == QProcess::NormalExit)
            a = dumpProcess.exitCode();
        if (a != 0)
            i = listpaths.size();
    }
    return a;
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
        if (dlg_buprestore->exec() != QDialog::Accepted)
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
    if (m_parametres->dirbkup() == "")
    {
        EffaceProgrammationBackup();
        return;
    }
    if (!QDir(m_parametres->dirbkup()).exists() || !m_parametres->heurebkup().isValid() || !m_parametres->daysbkup())
    {
        EffaceProgrammationBackup();
        return;
    }
    t_timerbackup.disconnect(SIGNAL(timeout()));
    t_timerbackup.stop();
    t_timerbackup.start(30000); /*! le timer de déclenchement de la sauvegarde est lancé plus d'une fois par minute à cause de la grande imprécision des QTimer
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

void Procedures::ProgrammeSQLVideImagesTemp(QTime timebackup) /*!  - abandonné parce qu'il continue à fonctionner même en cas de plantage du programme */
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
    QMap<QString, QString> EnteteMap = QMap<QString, QString>();
    if (!user)
        return EnteteMap;
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
        Pied =  "<html>"
                "{{DUPLI}}"
                "<p align = \"center\"; style = \"margin-top:0px; margin-bottom:0px;\"><span style=\"font-size:6pt\">{{AGA}}</span></p>"
                "</html>";
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
        Pied = baPied;
    }
    bool isaga = false;
    if (user)
    {
        User *parent = Datas::I()->users->getById(user->idparent());
        if (parent)
            isaga = parent->isAGA();
    }
    Pied.replace("{{AGA}}",(isaga?"Membre d'une association de gestion agréée - Le règlement des honoraires par chèque ou carte de crédit est accepté":""));
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
            QString dupli = "<p align=\"center\"><span style=\"font-family:Arial Black;font-size:24pt;font-style:normal;font-weight:bold;color:#cccccc;\">DUPLICATA</span></p>";
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
        date = docmt->datetimeimpression().toString(tr("d-M-yyyy"));
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
                    imgs = "select " CP_ID_ECHGIMAGES " from " TBL_ECHANGEIMAGES " where " CP_ID_ECHGIMAGES " = " + iditem + " and (" CP_PDF_ECHGIMAGES " is not null or " CP_JPG_ECHGIMAGES " is not null)";
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
                                        " where " CP_ID_ECHGIMAGES " = " + iditem +
                                        " and " CP_FACTURE_ECHGIMAGES " is null");
                        QString req = "INSERT INTO " TBL_ECHANGEIMAGES " (" CP_ID_ECHGIMAGES ", " + sfx + ", " CP_COMPRESSION_ECHGIMAGES ")"
                                                                                                  " VALUES (" +
                                iditem + ", " +
                                " LOAD_FILE('" + Utils::correctquoteSQL(m_parametres->dirimagerieserveur()) + NOM_DIR_IMAGES + Utils::correctquoteSQL(filename) + "'), " +
                                QString::number(docmt->compression()) + ")";
                        db->StandardSQL(req);
                        //qDebug() << req;
                    }
                    else
                    {
                        db->StandardSQL("delete from " TBL_ECHANGEIMAGES
                                                             " where " CP_ID_ECHGIMAGES " = " + iditem +
                                                             " and " CP_FACTURE_ECHGIMAGES " = 1");
                        QString req = "INSERT INTO " TBL_ECHANGEIMAGES " (" CP_ID_ECHGIMAGES ", " + sfx + ", " CP_FACTURE_ECHGIMAGES ") "
                                      "VALUES (" +
                                      iditem + ", " +
                                      " LOAD_FILE('" + Utils::correctquoteSQL(m_parametres->dirimagerieserveur()) + NOM_DIR_FACTURES + Utils::correctquoteSQL(filename) + "'), " +
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
            listimpr = db->StandardSelectSQL("select " CP_PDF_ECHGIMAGES ", " CP_JPG_ECHGIMAGES ", " CP_COMPRESSION_ECHGIMAGES "  from " TBL_ECHANGEIMAGES " where " CP_ID_ECHGIMAGES " = " + iditem + " and " CP_FACTURE_ECHGIMAGES " is null"
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
            listimpr = db->StandardSelectSQL("select " CP_PDF_ECHGIMAGES ", " CP_JPG_ECHGIMAGES "  from " TBL_ECHANGEIMAGES " where " CP_ID_ECHGIMAGES " = " + iditem + " and facture = 1"
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

QString Procedures::Edit(QString txt, QString titre, bool editable, bool ConnectAuSignal, QWidget *parent)
{
    QString         rep("");
    QString         geometry(Position_Fiche "Edit");
    UpDialog        *gAsk           = new UpDialog(parent);
    UpTextEdit      *TxtEdit        = new UpTextEdit(gAsk);
    QList<QScreen*> listscreens = QGuiApplication::screens();
    int x = 0;
    int y = 0;
    if (listscreens.size())
    {
        x = listscreens.first()->geometry().width();
        y = listscreens.first()->geometry().height();
    }

    gAsk->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    if (!editable)
        gAsk->setWindowModality(Qt::WindowModal);

    TxtEdit->setText(txt);
    TxtEdit->setTextInteractionFlags(editable? Qt::TextEditorInteraction : (Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard));

    gAsk->setMaximumWidth(x);
    gAsk->setMaximumHeight(y);
    gAsk->setWindowTitle(titre);

    gAsk->dlglayout()->insertWidget(0,TxtEdit);

    gAsk->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(gAsk->OKButton, &QPushButton::clicked,  gAsk,       &QDialog::accept);
    if (ConnectAuSignal)
        connect(this,       &Procedures::ModifEdit, TxtEdit,    [=](QString txt) {TxtEdit->setText(txt);});
    gAsk->restoreGeometry(m_settings->value(geometry).toByteArray());

    if (gAsk->exec() == QDialog::Accepted)
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
        connect(gAsk->PrintButton, QOverload<QVariant>::of(&UpSmallButton::clicked), this, [=](QVariant) {PrintDocument(doc);});
    }
    if (Button.testFlag(UpDialog::ButtonSuppr))
        connect(gAsk->SupprButton, &QPushButton::clicked, this, [=] {emit DelImage();});

    QList<QScreen*> listscreens = QGuiApplication::screens();
    int x = 0;
    int y = 0;
    if (listscreens.size())
    {
        x = listscreens.first()->geometry().width();
        y = listscreens.first()->geometry().height();
    }
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
                        if (dialog->exec() == QDialog::Accepted)
                            delete dialog;
                        else {
                            delete dialog;
                            return false;
                        }
                    }
                    else
                    {
                        QPrintDialog *dialog = new QPrintDialog(p_printer);
                        if (dialog->exec() == QDialog::Accepted)
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
            if (dialog->exec() == QDialog::Accepted)
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
    if (Pied == "")
    {
        delete Etat_textEdit;
        return false;
    }

    // creation du corps
    Corps = CalcCorpsImpression(text, ALD);
    if (Corps == "")
    {
        delete Etat_textEdit;
        return false;
    }
    Etat_textEdit->setHtml(Corps);
    if (Etat_textEdit->toPlainText() == "")
    {
        delete Etat_textEdit;
        return false;
    }
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
        listbinds[CP_TEXTPIED_DOCSEXTERNES]      = Pied.replace("{{DUPLI}}","");
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
    return (m_settings->value(Imprimante_ApercuAvantImpression).toString() == "YES");
}

QString Procedures::CodePostalParDefaut()
{
    QSettings set(PATH_FILE_INI, QSettings::IniFormat);
    return set.value(CodePostal_Defaut).toString();
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
        path = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Imagerie).toString();
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
    for (auto it = Datas::I()->correspondants->correspondants()->constBegin(); it != Datas::I()->correspondants->correspondants()->constEnd(); ++it)
    {
        Correspondant *cor = const_cast<Correspondant*>(it.value());
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
    return m_settings->value(Imprimante_TailleEnTete).toInt();
}

int Procedures::TailleEnTeteALD()
{
    return m_settings->value(Imprimante_TailleEnTeteALD).toInt();
}

int Procedures::TaillePieddePage()
{
    return m_settings->value(Imprimante_TaillePieddePage).toInt();
}

int Procedures::TaillePieddePageOrdoLunettes()
{
    return m_settings->value(Imprimante_TaillePieddePageOrdoLunettes).toInt();
}

int Procedures::TailleTopMarge()
{
    return m_settings->value(Imprimante_TailleTopMarge).toInt();
}

QString Procedures::VilleParDefaut()
{
    QSettings set(PATH_FILE_INI, QSettings::IniFormat);
    return set.value(Ville_Defaut).toString();
}

void Procedures::setPosteImportDocs(bool a)
{
    /*! Il n'y pas de variables utilisateur globale dans MySQL, on est donc obligé de passer par une procédure stockée pour en simuler une
    * pour créer une procédure avec Qt, séparer le drop du create, ne pas utiliser les délimiteurs et utiliser les retours à la ligne \n\.......
    * if (gsettingsIni->value(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs).toString() ==  "YES")

    * si a = true, on se met en poste importateur +/_ prioritaire à la fin suivant le contenu de rufus.ini
    * si a = false, on retire le poste en cours et on met NULL à la place. */

    QString req = "USE `" DB_CONSULTS "`;";
    db->StandardSQL(req);

    req = "DROP PROCEDURE IF EXISTS " NOM_POSTEIMPORTDOCS ";";
    db->StandardSQL(req);

    QString IpAdress("NULL");
    if (a)
    {
        if (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs).toString() ==  "YES")
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
    {
        QString timeb = QString::number(time/60000,'f',0);
        if (timeb != "0")
            timelitteral = timeb + tr(" minutes");
        else
            timelitteral = tr("moins d'une minute");
    }
    else if (Volumelitteral.right(2) == "To")
    {
        QString timeb = QString::number(time/60000/60,'f',0);
        if (timeb != "0")
            timelitteral = timeb + tr(" heures");
        else
            timelitteral = tr("moins d'une heure");
    }
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

bool Procedures::RestaureBase(bool BaseVierge, bool PremierDemarrage, bool VerifPostesConnectes, QWidget *parent)
{
    UpMessageBox    msgbox(parent);
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
            Hote = tr("le serveur ") + m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Serveur).toString();
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

        QString mdp("");
        if (!Utils::VerifMDP((PremierDemarrage? Utils::calcSHA1(MDP_ADMINISTRATEUR) : MDPAdmin()),tr("Saisissez le mot de passe Administrateur"), mdp))
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
            UpMessageBox::Watch(parent, tr("Impossible d'éxécuter la restauration!"), msg);
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

            //! Restauration à partir du dossier sélectionné
            int a = ExecuteSQLScript(listnomsfilestorestore);
            if (a != 0)
            {
                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                for (int t=0; t<listfichiers.size(); t++)
                {
                    QString nomdocrz  = listfichiers.at(t);
                    QString CheminFichierResize = PATH_DIR_RESSOURCES "/" + nomdocrz;
                    QFile(CheminFichierResize).remove();
                }
                return false;
            }
            else
            {
                UpMessageBox::Information(parent, tr("Base vierge créée"),tr("La création de la base vierge a réussi."));
                for (int t=0; t<listfichiers.size(); t++)
                {
                    QString nomdocrz  = listfichiers.at(t);
                    QString CheminFichierResize = PATH_DIR_RESSOURCES "/" + nomdocrz;
                    QFile(CheminFichierResize).remove();
                }
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
        UpMessageBox::Information(parent, tr("Choix du dossier de sauvegarde"),
                                  tr("Dans la fiche suivante, choisissez le dossier "
                                  "contenant la sauvegarde de la base.\n\n"
                                  "Une fois le dossier sélectionné, "
                                  "la sauvegarde commencera automatiquement.\n"
                                  "Ce processus est long et peut durer plusieurs minutes.\n"
                                  "(environ 1' pour 2 Go)\n"));
        QString dir = PATH_DIR_RUFUS;
        QUrl url = Utils::getExistingDirectoryUrl(Q_NULLPTR, tr("Restaurer à partir du dossier"), QUrl::fromLocalFile(dir));
        if (url == QUrl())
            return false;
        QDir dirtorestore = QDir(url.path());
        QString NomDirtorestore = dirtorestore.absolutePath();
        if (NomDirtorestore.contains(" "))
        {
            UpMessageBox::Watch(parent, tr("Echec de la restauration"), tr("Le chemin vers le dossier ") + NomDirtorestore + tr(" contient des espaces!"));
            return false;
        }
        QString mdp("");
        if (!Utils::VerifMDP((PremierDemarrage? Utils::calcSHA1(MDP_ADMINISTRATEUR) : MDPAdmin()),tr("Saisissez le mot de passe Administrateur"), mdp, false, parent))
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
            if (!QDir(PATH_DIR_IMAGERIE).exists())
            {
                UpMessageBox::Watch(parent,tr("Pas de dossier de stockage d'imagerie"),
                                    tr("Indiquez un dossier valide dans la boîte de dialogue qui suit") + "\n" +
                                    tr("Utilisez de préférence le dossier ") + PATH_DIR_IMAGERIE + " " +tr("Créez-le au besoin"));
                QUrl url = Utils::getExistingDirectoryUrl(Q_NULLPTR, tr("Stocker les images dans le dossier") , PATH_DIR_IMAGERIE);
                if (url == QUrl())
                    return false;
                QDir dirstock = QDir(url.path());
                NomDirStockageImagerie = dirstock.absolutePath();
                if (NomDirStockageImagerie.contains(" "))
                {
                    UpMessageBox::Watch(parent, tr("Echec de la restauration"), tr("Le chemin vers le dossier ") + NomDirStockageImagerie + tr(" contient des espaces!"));
                    return false;
                }
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
                        UpMessageBox msgbox(parent);
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

                            //! Restauration à partir du dossier sélectionné
                            int a = ExecuteSQLScript(listnomsfilestorestore);
                            if (a != 0)
                                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                            db->setdirimagerie(NomDirStockageImagerie);
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
        UpMessageBox::Watch(parent,tr("restauration terminée"),msg);
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
                a = ExecuteSQLScript(QStringList() << NomDumpFile);
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
            }
        }
    }
    //verification des fichiers ressources
    if (m_settings->value(Poste_VersionRessources).toInt() < VERSION_RESSOURCES)
    {
        PremierParametrageRessources();
        m_settings->setValue(Imprimante_TailleEnTeteALD,"63");
        m_settings->setValue(Poste_VersionRessources, VERSION_RESSOURCES);
        ShowMessage::I()->SplashMessage(tr("Mise à jour des fichiers ressources vers la version ") + "<font color=\"red\"><b>" + QString::number(VERSION_RESSOURCES) + "</b></font>", 5000);
    }
    return true;
}


void Procedures::ReconstruitListeModesAcces()
{
    if ( m_settings->value(Utils::getBaseFromMode(Utils::Poste) + Param_Active).toString() == "YES"
       && (m_settings->value(Utils::getBaseFromMode(Utils::Poste) + Param_Port).toInt() == 3306 || m_settings->value(Utils::getBaseFromMode(Utils::Poste) + Param_Port).toInt() == 3307) )
        m_listemodesacces << Utils::Poste;
    if (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Active).toString() == "YES"
       && m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Serveur).toString() != ""
       && (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Port).toInt() == 3306 || m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Port).toInt() == 3307) )
        m_listemodesacces <<  Utils::ReseauLocal;
    if (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Param_Active).toString() == "YES"
       && m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Param_Serveur).toString() != ""
       && (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Param_Port).toInt() == 3306 || m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Param_Port).toInt() == 3307) )
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
    if (msgbox.exec() == QDialog::Accepted)
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
        server = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Serveur).toString();

    int port = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Port).toInt();

    db->initParametresConnexionSQL(server, port);
    if (!IdentificationUser())
        return false;

    //initListeUsers();
    CalcLieuExercice();

    /*! Création de la session */
    QHash<QString, QVariant>                m_listbinds;
    m_listbinds[CP_IDUSER_SESSIONS]         = currentuser()->id();
    m_listbinds[CP_IDSUPERVISEUR_SESSIONS]  = currentuser()->idsuperviseur();
    m_listbinds[CP_IDPARENT_SESSIONS]       = currentuser()->idparent();
    m_listbinds[CP_IDCOMPTABLE_SESSIONS]    = currentuser()->idcomptable();
    if (Datas::I()->sites->currentsite())
        m_listbinds[CP_IDLIEU_SESSIONS]     = Datas::I()->sites->currentsite()->id();
    m_listbinds[CP_DATEDEBUT_SESSIONS]      = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    Datas::I()->sessions->CreationSession(m_listbinds);

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
    // Création du premier utilisateur dans la table utilisateurs
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
    // la suite sert à corriger les tables documents remises en exemple qui peuvent avoir été créées à partir d'autres bases Rufus par un iduser différent auquel cas ces documents ne seraient pas modifiables
    req = "update " TBL_IMPRESSIONS " set " CP_IDUSER_IMPRESSIONS " = " + QString::number(idusr) + ", " CP_DOCPUBLIC_IMPRESSIONS " = 1";
    db->StandardSQL (req);
    req = "update " TBL_DOSSIERSIMPRESSIONS " set " CP_IDUSER_DOSSIERIMPRESSIONS " = " + QString::number(idusr) + ", " CP_PUBLIC_DOSSIERIMPRESSIONS " = 1";
    db->StandardSQL (req);
    req = "update " TBL_COMMENTAIRESLUNETTES " set " CP_IDUSER_COMLUN " = " + QString::number(idusr) + ", " CP_PUBLIC_COMLUN " = 1";
    db->StandardSQL (req);

    if (UpMessageBox::Question(Q_NULLPTR, tr("Un compte utilisateur a été créé"),
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
        if (Dlg_GestUsr->exec() == QDialog::Accepted)
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
    m_settings->setValue(Ville_Defaut,"Flayat");
    m_settings->setValue(CodePostal_Defaut,"23260");
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
    for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
    {
        Banque *bq = const_cast<Banque*>(it.value());
        if (bq->nomabrege() == "PaPRS")
        {
            idbanq = bq->id();
            break;
        }
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
           " (" CP_IDBANQUE_COMPTES ", " CP_IDUSER_COMPTES ", " CP_IBAN_COMPTES ", " CP_INTITULE_COMPTES ", " CP_NOMABREGE_COMPTES ", " CP_SOLDE_COMPTES ")"
           " VALUES (" + QString::number(idbanq) + "," + QString::number(idusr) + ", '" + iban + "', '" + login + "', 'PaPRS" + QString::number(al) + "', 2333.67)";
    //qDebug() << req;
    db->StandardSQL(req);
    QString idcpt ("");
    req = "select max(" CP_ID_COMPTES ") from " TBL_COMPTES;
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
    if (dlg_IdentUser->exec() == QDialog::Accepted)
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
    else // anomalie sur la base - table utilisateurs manquante ou corrompue
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
        path = settings()->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Imagerie).toString();
        break;
    case Utils::Distant:
        path = settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_Imagerie).toString();
    }
    return path;
}

bool Procedures::DefinitRoleUser() //NOTE : User Role Function
{
    if (currentuser()->isSoignant() )
    {
        dlg_askUser                 = new UpDialog();
        dlg_askUser                 ->AjouteLayButtons();
        dlg_askUser                 ->setAccessibleName(QString::number(currentuser()->id()));
        dlg_askUser                 ->setdata(currentuser());
        QVBoxLayout *boxlay         = new QVBoxLayout;
        dlg_askUser->dlglayout()    ->insertLayout(0,boxlay);

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
        {
            CalcUserParent();
            User *usrparent = Datas::I()->users->getById(currentuser()->idparent());
            if ( usrparent )
            {
                currentuser()->setAGA(usrparent->isAGA());
                currentuser()->setsecteurconventionnel(usrparent->secteurconventionnel());
                currentuser()->setOPTAM(usrparent->isOPTAM());
                //qDebug() << "secteur = " << currentuser()->secteurconventionnel() << " - OPTAM = " << currentuser()->isOPTAM();
            }
        }

    // le user alterne entre responsable des actes et assistant suivant la session
        // on lui demande son rôle pour cette session
        else if( currentuser()->isResponsableOuAssistant() )
        {
            bool found = false;
            for (auto it = Datas::I()->users->actifs()->constBegin(); it != Datas::I()->users->actifs()->constEnd(); ++it)
            {
                User *usr = const_cast<User*>(it.value());
                if( usr->id() == currentuser()->id() )
                    continue;
                if( !usr->isResponsable() && !usr->isResponsableOuAssistant() )
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
                User *usrparent = Datas::I()->users->getById(currentuser()->idparent());
                if ( usrparent )
                {
                    currentuser()->setAGA(usrparent->isAGA());
                    currentuser()->setsecteurconventionnel(usrparent->secteurconventionnel());
                    currentuser()->setOPTAM(usrparent->isOPTAM());
                    //qDebug() << "secteur = " << currentuser()->secteurconventionnel() << " - OPTAM = " << currentuser()->isOPTAM();
                }
            }
        }

    // le user est assistant - on lui demande qui supervise ses actes
        else if( currentuser()->isAssistant() )
            CalcUserSuperviseur();

        dlg_askUser->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
        connect(dlg_askUser->OKButton, &QPushButton::clicked, dlg_askUser, &UpDialog::accept);

        if( currentuser()->idsuperviseur() == User::ROLE_INDETERMINE || currentuser()->idparent() == User::ROLE_INDETERMINE )
        {
            if( dlg_askUser->exec() != QDialog::Accepted )
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
                            for (auto it = Datas::I()->users->actifs()->constBegin(); it != Datas::I()->users->actifs()->constEnd(); ++it)
                            {
                                User *usr = const_cast<User*>(it.value());
                                if( usr->id() == currentuser()->id() )
                                    continue;
                                if( usr->id() == currentuser()->idsuperviseur() )
                                    continue;
                                if( !usr->isLiberal() && !usr->isSalarie() )
                                    continue;
                                listUserFound << usr;
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
                                if (dlg_askUser->exec() != QDialog::Accepted)
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
                    currentuser()->setAGA(usrparent->isAGA());
                    currentuser()->setsecteurconventionnel(usrparent->secteurconventionnel());
                    currentuser()->setOPTAM(usrparent->isOPTAM());
                    //qDebug() << "secteur = " << currentuser()->secteurconventionnel() << " - OPTAM = " << currentuser()->isOPTAM();
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

//! la liste des appareils d'imagerie
QList<AppareilImagerie*> Procedures::listeappareils()
{
    return m_listeappareils;
}
void Procedures::setlisteappareils (QList<AppareilImagerie*> listappareils)
{
    for (int i=0; i < m_listeappareils.size(); i++)
        delete m_listeappareils.at(i);
    m_listeappareils = listappareils;
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
    if (ptbox)
    {
        ptbox->setVisible(false);
        QList<QRadioButton*> listbutt = ptbox->findChildren<QRadioButton*>();
        foreach (QRadioButton * rb, listbutt)
            delete rb;
        delete ptbox->layout();
    }

    QList<User*> listUserFound;
    for (auto it = Datas::I()->users->actifs()->constBegin(); it != Datas::I()->users->actifs()->constEnd(); ++it)
    {
        User *usr = const_cast<User*>(it.value());
        if( usr->id() == user->id() )
            continue;
        if( currentuser()->isMedecin() && !usr->isMedecin() )
            continue;
        if( currentuser()->isOrthoptist() && !usr->isSoignant() )
            continue;
        if (usr->isadmin() || usr->isNeutre())
            continue;
        listUserFound << usr;
    }

    if( listUserFound.size() == 1 )
        currentuser()->setidsuperviseur( listUserFound.first()->id() );
    else if( !listUserFound.isEmpty() && ptbox )
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
    if (ptbox)
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
        for (auto it = Datas::I()->users->actifs()->constBegin(); it != Datas::I()->users->actifs()->constEnd(); ++it)
        {
            User *usr = const_cast<User*>(it.value());
            if( usr->id() == user->id() )
                continue;
            if( !usr->isLiberal() && !usr->isSalarie() )
                continue;
            if( usr->metier() != user->metier() )
                continue;
            listUserFound << usr;
        }

        if( listUserFound.size() == 1 )
        {
            //gidUserParentProv = listUserFound.first()->id();
            user->setidparent( listUserFound.first()->id() );
        }
        else if( !listUserFound.isEmpty() && ptbox )
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
                              "1. J'installe Rufus sur ce poste et ce poste se connectera à une base patients qui existe dèjà\n"
                              "2. J'installe Rufus sur ce poste et ce poste se connectera à une base patients vierge que je vais créer\n"));
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
    Utils::mkpath(PATH_DIR_ORIGINAUX NOM_DIR_IMAGES);
    Utils::mkpath(PATH_DIR_REFRACTEUR_IN NOM_DIR_AUTOREF);
    Utils::mkpath(PATH_DIR_REFRACTEUR_IN NOM_DIR_FRONTO);
    Utils::mkpath(PATH_DIR_REFRACTEUR_IN NOM_DIR_TONO);
    Utils::mkpath(PATH_DIR_REFRACTEUR_OUT);
    Utils::mkpath(PATH_DIR_TONO);
    Utils::mkpath(PATH_DIR_AUTOREF);
    Utils::mkpath(PATH_DIR_FRONTO);

    if (m_settings != Q_NULLPTR)
        delete m_settings;
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
                 db->setadresseserveurlocal(m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Serveur).toString());
             else
                 db->setdirimagerie(PATH_DIR_IMAGERIE);
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
    m_settings->setValue(Imprimante_TailleEnTete,"45");
    m_settings->setValue(Imprimante_TailleEnTeteALD,"63");
    m_settings->setValue(Imprimante_TaillePieddePage,"20");
    m_settings->setValue(Imprimante_TaillePieddePageOrdoLunettes,"40");
    m_settings->setValue(Imprimante_TailleTopMarge,"3");
    m_settings->setValue(Imprimante_ApercuAvantImpression,"NO");
    m_settings->setValue("PyxInterf/PyxvitalPath", QDir::homePath() + "/Documents/Pyxvital");
    m_settings->setValue(Param_Poste_Autoref,"-");
    m_settings->setValue(Param_Poste_Refracteur,"-");
    m_settings->setValue(Param_Poste_Fronto,"-");
    m_settings->setValue(Param_Poste_Tono,"-");
    m_settings->setValue(Param_Poste_PortAutoref,"-");
    m_settings->setValue(Param_Poste_PortRefracteur,"-");
    m_settings->setValue(Param_Poste_PortFronto,"-");
    m_settings->setValue(Param_Poste_PortTono,"-");
    m_settings->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs,"NO");
    m_settings->setValue(Poste_VersionRessources, VERSION_RESSOURCES);
    Utils::mkpath(PATH_DIR_IMAGERIE);
    QString NomDirImg = (modifdirimagerie? PATH_DIR_IMAGERIE : db->parametres()->dirimagerieserveur());
    m_settings->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_Imagerie, NomDirImg);
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
        DirRessrces.removeRecursively();
    Utils::mkpath(PATH_DIR_RESSOURCES);
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
    m_settings->setValue(Poste_VersionRessources,VERSION_RESSOURCES);
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
        if (dialog.exec() == QDialog::Accepted)
        {
            QFile FichierIni(PATH_FILE_INI);
            if (FichierIni.exists())
                FichierIni.remove();
            QString fileini = dialog.selectedFiles().at(0);
            QFile rufusini(fileini);
            rufusini.copy(PATH_FILE_INI);
            if (m_settings != Q_NULLPTR)
                delete m_settings;
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
        if (m_settings != Q_NULLPTR)
            delete m_settings;
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

    if (Dlg_ParamConnex->exec() == QDialog::Accepted)
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
            m_settings->setValue(Base + Param_Serveur,   Utils::calcIP(Dlg_ParamConnex->ui->IPlineEdit->text(), false));
            db->setModeacces(Utils::ReseauLocal);
        }
        else if (Dlg_ParamConnex->ui->DistantradioButton->isChecked())
        {
            Base = Utils::getBaseFromMode(Utils::Distant);
            m_settings->setValue(Base + Param_Serveur,    Utils::calcIP(Dlg_ParamConnex->ui->IPlineEdit->text(), false));
            db->setModeacces(Utils::Distant);
        }
        m_settings->setValue(Base + Param_Active,    "YES");
        m_settings->setValue(Base + Param_Port, Dlg_ParamConnex->ui->PortcomboBox->currentText());

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
        if (dialog.exec() == QDialog::Accepted)
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


void Procedures::Ouverture_Appareils_Refraction()
{
    QString nameARK   = m_settings->value(Param_Poste_Autoref).toString();
    QString nameLM    = m_settings->value(Param_Poste_Fronto).toString();
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    QString nameTO    = m_settings->value(Param_Poste_Tono).toString();
    TypesAppareils appareilscom, appareilsreseau;
    bool m_isFrontoParametre    = (nameLM != "-"
                                && nameLM != ""
                                && m_settings->value(Param_Poste_PortFronto).toString() != "Box");
    bool m_isAutorefParametre   = (nameARK != "-"
                                && nameARK != ""
                                && m_settings->value(Param_Poste_PortAutoref).toString() != "Box");
    bool m_isRefracteurParametre= (nameRF != "-"
                                && nameRF != "");
    bool m_isTonoParametre      = (nameTO != "-"
                                && nameTO != "");
    if (m_isFrontoParametre)
    {
        bool m_isReseauFronto       = (m_settings->value(Param_Poste_PortFronto).toString() == RESEAU);
        m_isReseauFronto?           appareilsreseau.setFlag(Fronto)      : appareilscom.setFlag(Fronto);
    }
    if (m_isAutorefParametre)
    {
        bool m_isReseauAutoref      = (m_settings->value(Param_Poste_PortAutoref).toString() == RESEAU);
        m_isReseauAutoref?          appareilsreseau.setFlag(Autoref)     : appareilscom.setFlag(Autoref);
    }
    if (m_isRefracteurParametre)
    {
        bool m_isReseauRefracteur   = (m_settings->value(Param_Poste_PortRefracteur).toString() == RESEAU);
        m_isReseauRefracteur?       appareilsreseau.setFlag(Refracteur) : appareilscom.setFlag(Refracteur);
    }
    if (m_isTonoParametre)
    {
        bool m_isReseauTono         = (m_settings->value(Param_Poste_PortTono).toString() == RESEAU);
        m_isReseauTono?             appareilsreseau.setFlag(Tonometre)        : appareilscom.setFlag(Tonometre);
    }
    if (appareilscom > 0)
        Ouverture_Ports_Series(appareilscom);
    if (appareilsreseau > 0)
        Ouverture_Fichiers_Echange(appareilsreseau);
    m_hasappareilrefractionconnecte = appareilscom + appareilsreseau > 0;
}

/*! ------------------------------------------------------------------------------------------------------------------------------------------
GESTION DES FICHIERS ECHANGE XML DES APPAREILS DE REFRACTION ---------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Fichiers_Echange(TypesAppareils appareils)
{
    auto lecturefichier = [] (TypeAppareil appareil, QString pathdirappareil, QStringList listfich)
    {
        QString app = "";
        switch (appareil)
        {
        case Fronto:        app = tr("le frontofocomètre");     break;
        case Autoref:       app = tr("l'autorefractomètre");    break;
        case Refracteur:    app = tr("le refracteur");          break;
        case Tonometre:     app = tr("le tonomètre");          break;
        default: break;
        }
        const QString nomfichier      = pathdirappareil + "/" + listfich.at(0);
        QFile datafile(nomfichier);
        QDomDocument docxml;
        if (datafile.open(QIODevice::ReadOnly))
        {
            docxml.setContent(&datafile);
            datafile.remove();
        }

        if (Datas::I()->actes->currentacte() != Q_NULLPTR)
        {
            if (Datas::I()->actes->currentacte()->date() == DataBase::I()->ServerDate()
              || (app != tr("le tonomètre")  // on peut faire plusieurs mesures de tonometrie sur la même consultation donc n ne vérifie pas dans ce cas si une mesure a déjà été faite pour cet acte
                  &&
                  UpMessageBox::Question(Q_NULLPTR,
                                          tr("Une mesure vient d'être émise par ") + app + tr(" mais la date de l'acte actuellement affiché n'est pas celle d'aujour'hui."),
                                          "\n" +
                                          tr("Voulez-vous quand même enregistrer cette mesure?"),
                                          UpDialog::ButtonOK | UpDialog::ButtonCancel, QStringList() << tr("Annuler") << tr("Enregistrer la mesure"))
                  == UpSmallButton::STARTBUTTON)
               )
            {
                switch (appareil)
                {
                    case Fronto:        Procedures::I()->ReponseXML_Fronto(docxml);     break;
                    case Autoref:       Procedures::I()->ReponseXML_Autoref(docxml);    break;
                    case Refracteur:    Procedures::I()->ReponseXML_Refracteur(docxml); break;
                    case Tonometre:     Procedures::I()->ReponseXML_Tono(docxml);       break;
                    default: break;
                }
            }
        }
        listfich = QDir(pathdirappareil).entryList(QDir::Files | QDir::NoDotAndDotDot);
            for(int i = 0; i < listfich.size(); ++i)
                QFile(pathdirappareil + "/" + listfich.at(i)).remove();
    };
    bool usetimer = true;  /*! Il semble que la classe QSystemFileWatcher pose quelques problèmes.
                             * au démarrage du système le signal directorychanged ne marche pas bien sur Mac quand le fichier d'échange est sur une machine Linux ou Windows
                             * il faut redémarrer une session Rufus pour que ça se décide à marcher
                             * On peut utiliser un timer à la place. C'est nettement moins élégant mais ça marche très bien.
                             * Il suffit de mettre ce bool à true pour utiliser le timer
                             * Le code pour le QFileSystemWatcher a été conservé au cas où le problème serait résolu */

    QString pathdirfronto ("");
    QString pathdirautoref ("");
    QString pathdirrefracteur ("");
    QString pathdirtono ("");
    if (appareils.testFlag(Autoref))
    {
        m_LANAutoref = true;
        Datas::I()->mesureautoref   ->settypemesure(Refraction::Autoref);
        pathdirautoref = settings()->value(Param_Poste_PortAutoref_Reseau).toString();
        if (!usetimer)
            m_filewatcherautoref.addPath(pathdirautoref);
    }
    if (appareils.testFlag(Fronto))
    {
        m_LANFronto = true;
        Datas::I()->mesurefronto   ->settypemesure(Refraction::Fronto);
        pathdirfronto = settings()->value(Param_Poste_PortFronto_Reseau).toString();
        if (!usetimer)
            m_filewatcherfronto.addPath(pathdirfronto);
    }
    if (appareils.testFlag(Refracteur))
    {
        m_LANRefracteur = true;
        Datas::I()->mesurefinal     ->settypemesure(Refraction::Prescription);
        Datas::I()->mesureacuite    ->settypemesure(Refraction::Acuite);
        pathdirrefracteur = settings()->value(Param_Poste_PortRefracteur_Reseau).toString();
        if (!usetimer)
            m_filewatcherrefracteur.addPath(pathdirrefracteur);
    }
    if (appareils.testFlag(Tonometre))
    {
        m_LANTono = true;
        pathdirtono = settings()->value(Param_Poste_PortTono_Reseau).toString();
        if (!usetimer)
            m_filewatcherrefracteur.addPath(pathdirtono);
    }
    if (usetimer)
    {
        t_filewatchtimer.start(1000);
        connect(&t_filewatchtimer,  &QTimer::timeout,     this,
                [=]
        {
            if (appareils.testFlag(Autoref) && pathdirautoref != "")
            {
                QStringList listfich = QDir(pathdirautoref).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfich.size())
                    lecturefichier(Autoref, pathdirautoref, listfich);
            }
            if (appareils.testFlag(Fronto) && pathdirfronto != "")
            {
                QStringList listfich = QDir(pathdirfronto).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfich.size())
                    lecturefichier(Fronto, pathdirfronto, listfich);
            }
            if (appareils.testFlag(Refracteur) && pathdirrefracteur != "")
            {
                QStringList listfich = QDir(pathdirrefracteur).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfich.size())
                    lecturefichier(Refracteur, pathdirrefracteur, listfich);
            }
            if (appareils.testFlag(Tonometre) && pathdirtono != "")
            {
                QStringList listfich = QDir(pathdirtono).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfich.size())
                    lecturefichier(Tonometre, pathdirtono, listfich);
            }
        });
    }
    else
    {
       if (appareils.testFlag(Autoref) && pathdirautoref != "")
            connect(&m_filewatcherautoref,      &QFileSystemWatcher::directoryChanged,  this,   [=]
            {
                QStringList listfichxml = QDir(pathdirautoref).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                {
                    /*! le patacaisse qui suit est nécessaire pour contourner un bug connu de Qt. Le signal directroychanged est parfois émis 2 fois et déclenche alors un double enregistrement de la mesure. */
                    const QString nomfichierxml      = pathdirautoref + "/" + listfichxml.at(0);
                    QFile xmldoc(nomfichierxml);
                    if (m_filewatcherautoreffile != listfichxml.at(0) || (m_filewatcherautoreffile == listfichxml.at(0) && m_filewatcherautorefcreated != QFileInfo(xmldoc).created()))
                    {
                        m_filewatcherautoreffile = listfichxml.at(0);
                        m_filewatcherautorefcreated = QFileInfo(xmldoc).created();
                        lecturefichier(Autoref, pathdirautoref, listfichxml);
                    }
                }
            });

        if (appareils.testFlag(Fronto) && pathdirfronto != "")
            connect(&m_filewatcherfronto,       &QFileSystemWatcher::directoryChanged,  this,   [=]
            {
                QStringList listfichxml = QDir(pathdirfronto).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                {
                    const QString nomfichierxml      = pathdirfronto + "/" + listfichxml.at(0);
                    QFile xmldoc(nomfichierxml);
                    if (m_filewatcherfrontofile != listfichxml.at(0) || (m_filewatcherfrontofile == listfichxml.at(0) && m_filewatcherfrontocreated != QFileInfo(xmldoc).created()))
                    {
                        m_filewatcherfrontofile = listfichxml.at(0);
                        m_filewatcherfrontocreated = QFileInfo(xmldoc).created();
                        lecturefichier(Fronto, pathdirfronto, listfichxml);
                    }
                }
            });

        if (appareils.testFlag(Refracteur) && pathdirrefracteur != "")
            connect(&m_filewatcherrefracteur,   &QFileSystemWatcher::directoryChanged,  this,   [=]
            {
                QStringList listfichxml = QDir(pathdirrefracteur).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                {
                    const QString nomfichierxml      = pathdirrefracteur + "/" + listfichxml.at(0);
                    QFile xmldoc(nomfichierxml);
                    if (m_filewatcherrefracteurfile != listfichxml.at(0) || (m_filewatcherrefracteurfile == listfichxml.at(0) && m_filewatcherrefracteurcreated != QFileInfo(xmldoc).created()))
                    {
                        m_filewatcherrefracteurfile = listfichxml.at(0);
                        m_filewatcherrefracteurcreated = QFileInfo(xmldoc).created();
                        lecturefichier(Refracteur, pathdirrefracteur, listfichxml);
                    }
                }
            });

        if (appareils.testFlag(Tonometre) && pathdirtono != "")
            connect(&m_filewatcherfronto,       &QFileSystemWatcher::directoryChanged,  this,   [=]
            {
                QStringList listfichxml = QDir(pathdirtono).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                {
                    const QString nomfichierxml      = pathdirtono + "/" + listfichxml.at(0);
                    QFile xmldoc(nomfichierxml);
                    if (m_filewatchertonofile != listfichxml.at(0) || (m_filewatchertonofile == listfichxml.at(0) && m_filewatchertonocreated != QFileInfo(xmldoc).created()))
                    {
                        m_filewatchertonofile = listfichxml.at(0);
                        m_filewatchertonocreated = QFileInfo(xmldoc).created();
                        lecturefichier(Tonometre, pathdirfronto, listfichxml);
                    }
                }
            });

    }
    return true;
}

/*!
  calcule le nom physique du port concerné à partir de la liste des ports disponibles et du nom du port déclaré dans rufus.ini (COM1,COM2,COM3 ou COM4)
*/

/*! ------------------------------------------------------------------------------------------------------------------------------------------
GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Ports_Series(TypesAppareils appareils)
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
    QString                     msg = "";
    Datas::I()->mesureautoref   ->settypemesure(Refraction::Autoref);
    Datas::I()->mesurefronto    ->settypemesure(Refraction::Fronto);
    Datas::I()->mesurefinal     ->settypemesure(Refraction::Prescription);
    Datas::I()->mesureacuite    ->settypemesure(Refraction::Acuite);
    QList<QSerialPortInfo> availableports = QSerialPortInfo::availablePorts();

    /*!
        Ouvre le port dont le portName() est NomPort en rapport avec l'appareil passé en paramètre
    */
    auto openserialport = [&] (TypeAppareil appareil, QString NomPort)
    {
        QSerialPort *serialport;
        SerialSettings sparamportserie;
        switch (appareil) {
        case Fronto:        sparamportserie = s_paramPortSerieFronto;       break;
        case Autoref:       sparamportserie = s_paramPortSerieAutoref;      break;
        case Refracteur:    sparamportserie = s_paramPortSerieRefracteur;   break;
        case Tonometre:     sparamportserie = s_paramPortSerieTono;         break;
        default: break;
        }
        for(int i=0; i<availableports.size(); i++)
        {
            // qDebug() << availableports.at(i).portName() << NomPort;
            if (availableports.at(i).portName()== NomPort)
            {
                serialport= new QSerialPort(availableports.at(i));
                serialport->setBaudRate(sparamportserie.baudRate);
                serialport->setFlowControl(sparamportserie.flowControl);
                serialport->setParity(sparamportserie.parity);
                serialport->setDataBits(sparamportserie.dataBits);
                serialport->setStopBits(sparamportserie.stopBits);
                break;
            }
        }
        switch (appareil) {
        case Fronto:        sp_portFronto = serialport;     break;
        case Autoref:       sp_portAutoref = serialport;    break;
        case Refracteur:    sp_portRefracteur = serialport; break;
        case Tonometre:     sp_portTono = serialport;       break;
        default: break;
        }
    };
    if (appareils == 0)
    {
        bool portseriedispo = false;
        for (int i=0; i<availableports.size(); i++)
        {
            //qDebug() << availableports.at(i).portName();
            if (availableports.at(i).portName().contains("usbserial") || availableports.at(i).portName().contains("ttyUSB") || availableports.at(i).portName().contains("COM"))
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
    QString listeports = "";
    for (int i=0; i<availableports.size(); i++)
    {
        if (availableports.at(i).portName().contains("usbserial") || availableports.at(i).portName().contains("ttyUSB") || availableports.at(i).portName().contains("COM"))
        {
            if (listeports != "")
                listeports += " - ";
            listeports += availableports.at(i).portName();
        }
    }
    if (listeports != "")
        listeports = tr("Liste des ports disponibles") + " - " + listeports;
    else
        listeports = tr("Aucun port COM disponible sur le système");

    /*!
    for (int i=0; i<availableports.size(); i++)
    }
        Logs::LogToFile("PortsSeries.txt", availableports.at(i).portName() + " - " + QDateTime().toString("dd-MM-yyyy HH:mm:ss"));
        qDebug() << availableports.at(i).portName();
    }
    */
    // PORT FRONTO
    if (appareils.testFlag(Fronto))
    {
        m_portFronto     = m_settings->value(Param_Poste_PortFronto).toString();
        ReglePortFronto();
        bool a           = (m_portFronto != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion frontofocomètre"));
        NomPort = Utils::RetrouveNomPort(m_portFronto);

        if (NomPort != "")
        {
            openserialport(Fronto, NomPort);
            if (t_threadFronto != Q_NULLPTR)
                delete t_threadFronto;
            if (sp_portFronto->open(QIODevice::ReadWrite))
            {
                //qDebug() << "FRONTO -> " + m_portFronto + " - " + NomPort;
                t_threadFronto = new SerialThread(sp_portFronto);
                t_threadFronto->transaction();
                connect(t_threadFronto,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Fronto);
                msg = tr("Connexion frontocomètre OK sur ") + m_portFronto;
            }
            else
            {
                msg =  tr("Impossible de connecter le frontocomètre sur ") + m_portFronto;
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le frontocomètre sur ") + m_portFronto, listeports);
                delete sp_portFronto;
                sp_portFronto = Q_NULLPTR;
            }
        }
        else
            UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le frontocomètre sur ") + m_portFronto, listeports);
    }

    // PORT REFRACTEUR
    if (appareils.testFlag(Refracteur))
    {
        m_portRefracteur = m_settings->value(Param_Poste_PortRefracteur).toString();
        //UpMessageBox::Watch(Q_NULLPTR, m_portRefracteur);
        ReglePortRefracteur();
        bool a           = (m_portRefracteur != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion refracteur"));
        NomPort = Utils::RetrouveNomPort(m_portRefracteur);
        if (NomPort != "")
        {
            openserialport(Refracteur, NomPort);
            if (t_threadRefracteur != Q_NULLPTR)
                delete t_threadRefracteur;
            if (sp_portRefracteur->open(QIODevice::ReadWrite))
            {
                //qDebug() << "REFRACTEUR -> " + m_portRefracteur + " - " + NomPort;
                t_threadRefracteur     = new SerialThread(sp_portRefracteur);
                t_threadRefracteur    ->transaction();
                connect(t_threadRefracteur,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Refracteur);
                if (msg != "")
                    msg += "\r";
                msg += tr("Connexion refracteur OK sur ") + m_portRefracteur;
            }
            else
            {
                if (msg != "")
                    msg += "\r";
                msg += tr("Impossible de connecter le refracteur sur ") + m_portRefracteur;
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le refracteur sur ") + m_portRefracteur, listeports);
                delete sp_portRefracteur;
                sp_portRefracteur = Q_NULLPTR;
            }
        }
        else
            UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le refracteur sur ") + m_portRefracteur, listeports);
    }

    //PORT AUTOREF
    if (appareils.testFlag(Autoref))
    {
        m_portAutoref    = m_settings->value(Param_Poste_PortAutoref).toString();
        ReglePortAutoref();
        bool a           = (m_portAutoref != "");
        if (!a)
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion autorefractomètre"));
        NomPort = Utils::RetrouveNomPort(m_portAutoref);
        if (NomPort != "")
        {
            openserialport(Autoref,NomPort);
            sp_portAutoref     = new QSerialPort();
            if (t_threadAutoref != Q_NULLPTR)
                delete t_threadAutoref;
            if (sp_portAutoref->open(QIODevice::ReadWrite))
            {
                //qDebug() << "AUTOREF -> " + m_portAutoref + " - " + NomPort;
                t_threadAutoref     = new SerialThread(sp_portAutoref);
                t_threadAutoref   ->transaction();
                connect(t_threadAutoref,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Autoref);
                if (msg != "")
                    msg += " \r";
                msg += tr("Connexion autoref OK sur ") + m_portAutoref;
            }
            else
            {
                if (msg != "")
                    msg += "\r";
                msg += tr("Impossible de connecter l'autoref sur ") + m_portAutoref;
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter l'autoref sur ") + m_portAutoref, listeports);
                delete sp_portAutoref;
                sp_portAutoref = Q_NULLPTR;
            }
        }
        else
            UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter l'autoref sur ") + m_portAutoref, listeports);
    }

    //PORT TONOMETRE
    if (appareils.testFlag(Tonometre))
    {
    m_portTono = m_settings->value(Param_Poste_PortRefracteur).toString();
    //UpMessageBox::Watch(Q_NULLPTR, m_portRefracteur);
    ReglePortTonometre();
    bool a           = (m_portTono != "");
    if (!a)
        UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion tonomètre"));
    NomPort = Utils::RetrouveNomPort(m_portTono);
    if (NomPort != "")
    {
        openserialport(Tonometre, NomPort);
        if (t_threadTono != Q_NULLPTR)
            delete t_threadTono;
        if (sp_portTono->open(QIODevice::ReadWrite))
        {
            //qDebug() << "TONOMETRE -> " + m_portTono + " - " + NomPort;
            t_threadTono     = new SerialThread(sp_portTono);
            t_threadTono    ->transaction();
            connect(t_threadTono,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Tono);
            if (msg != "")
                msg += "\r";
            msg += tr("Connexion tonmètre OK sur ") + m_portTono;
        }
        else
        {
            if (msg != "")
                msg += "\r";
            msg += tr("Impossible de connecter le tonomètre sur ") + m_portTono;
            UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le tonomètre sur ") + m_portTono, listeports);
            delete sp_portTono;
            sp_portTono = Q_NULLPTR;
        }
    }
    else
        UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le tonomètre sur ") + m_portTono, listeports);
    }

    if (msg != "")
        ShowMessage::I()->SplashMessage(msg, 3000);

    return false;
}

/*!
 * \brief Procedures::RegleSerialSettings
 * \param appareil
 * \param map les datas à régler char * = le name() du QMetaEnum de data à régler, int = l'index de la valeur ddans le QMetaEnum
 */
void Procedures::RegleSerialSettings(TypeAppareil appareil, QMap<QString, int> map)
{
    QString port(""), baudrate(""),databits(""),parity(""),stopbits(""),flowcontrol("");
    SerialSettings serialset;
    QSerialPort *serialport;
    switch (appareil) {
    case Fronto :
        baudrate    = Param_Poste_PortFronto_COM_baudrate;
        databits    = Param_Poste_PortFronto_COM_databits;
        parity      = Param_Poste_PortFronto_COM_parity;
        stopbits    = Param_Poste_PortFronto_COM_stopBits;
        flowcontrol = Param_Poste_PortFronto_COM_flowControl;
        serialport  = sp_portFronto;
        break;
    case Autoref :
        baudrate    = Param_Poste_PortAutoref_COM_baudrate;
        databits    = Param_Poste_PortAutoref_COM_databits;
        parity      = Param_Poste_PortAutoref_COM_parity;
        stopbits    = Param_Poste_PortAutoref_COM_stopBits;
        flowcontrol = Param_Poste_PortAutoref_COM_flowControl;
        serialport  = sp_portAutoref;
        break;
    case Refracteur :
        baudrate    = Param_Poste_PortRefracteur_COM_baudrate;
        databits    = Param_Poste_PortRefracteur_COM_databits;
        parity      = Param_Poste_PortRefracteur_COM_parity;
        stopbits    = Param_Poste_PortRefracteur_COM_stopBits;
        flowcontrol = Param_Poste_PortRefracteur_COM_flowControl;
        serialport  = sp_portRefracteur;
        break;
    case Tonometre :
        baudrate    = Param_Poste_PortTono_COM_baudrate;
        databits    = Param_Poste_PortTono_COM_databits;
        parity      = Param_Poste_PortTono_COM_parity;
        stopbits    = Param_Poste_PortTono_COM_stopBits;
        flowcontrol = Param_Poste_PortTono_COM_flowControl;
        serialport  = sp_portTono;
        break;
    default: return;
    }
    int index;
    QMetaEnum metaEnum;
    QString name ("");
    name = PORT;
    auto it = map.constFind(name);
    if (it != map.constEnd())
    {
        port = "COM"+ QString::number(it.value());
        if (serialport != Q_NULLPTR)
            serialport->setBaudRate(serialset.baudRate);
    }
    name = BAUDRATE;
    it = map.constFind(name);
    if (it != map.constEnd())
    {
        index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
        metaEnum = QSerialPort().metaObject()->enumerator(index);
        m_settings->setValue(baudrate,  it.value());
        serialset.baudRate = (QSerialPort::BaudRate)metaEnum.value(it.value());
        if (serialport != Q_NULLPTR)
            serialport->setBaudRate(serialset.baudRate);
    }
    name = DATABITS;
    it = map.constFind(name);
    if (it != map.constEnd())
    {
        index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
        metaEnum = QSerialPort().metaObject()->enumerator(index);
        m_settings->setValue(databits,  it.value());
        serialset.dataBits = (QSerialPort::DataBits)metaEnum.value(it.value());
        if (serialport != Q_NULLPTR)
            serialport->setDataBits(serialset.dataBits);
    }
    name = PARITY;
    it = map.constFind(name);
    if (it != map.constEnd())
    {
        index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
        metaEnum = QSerialPort().metaObject()->enumerator(index);
        m_settings->setValue(parity,  it.value());
        serialset.parity = (QSerialPort::Parity)metaEnum.value(it.value());
        if (serialport != Q_NULLPTR)
            serialport->setParity(serialset.parity);
    }
    name = STOPBITS;
    it = map.constFind(name);
    if (it != map.constEnd())
    {
        index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
        metaEnum = QSerialPort().metaObject()->enumerator(index);
        m_settings->setValue(stopbits,  it.value());
        serialset.stopBits = (QSerialPort::StopBits)metaEnum.value(it.value());
        if (serialport != Q_NULLPTR)
            serialport->setStopBits(serialset.stopBits);
    }
    name = FLOWCONTROL;
    it = map.constFind(name);
    if (it != map.constEnd())
    {
        index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
        metaEnum = QSerialPort().metaObject()->enumerator(index);
        m_settings->setValue(flowcontrol,  it.value());
        serialset.flowControl = (QSerialPort::FlowControl)metaEnum.value(it.value());
        if (serialport != Q_NULLPTR)
            serialport->setFlowControl(serialset.flowControl);
    }
    switch (appareil) {
    case Fronto :
        m_portFronto = Utils::RetrouveNomPort(port);
        s_paramPortSerieFronto = serialset;
        serialport  = sp_portFronto;
        break;
    case Autoref :
        m_portAutoref = Utils::RetrouveNomPort(port);
        s_paramPortSerieAutoref = serialset;
        serialport  = sp_portAutoref;
        break;
    case Refracteur :
        m_portRefracteur = Utils::RetrouveNomPort(port);
        s_paramPortSerieRefracteur = serialset;
        serialport  = sp_portRefracteur;
        break;
    case Tonometre :
        m_portTono = Utils::RetrouveNomPort(port);
        s_paramPortSerieTono = serialset;
        serialport  = sp_portTono;
        break;
    default: return;
    }
}

bool Procedures::ReglePortRefracteur()
{
    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    ReinitialiseSerialSettings(s_paramPortSerieRefracteur);
    if (m_settings->value(Param_Poste_PortRefracteur_COM_baudrate)      != QVariant()
     && m_settings->value(Param_Poste_PortRefracteur_COM_databits)      != QVariant()
     && m_settings->value(Param_Poste_PortRefracteur_COM_parity)        != QVariant()
     && m_settings->value(Param_Poste_PortRefracteur_COM_stopBits)      != QVariant()
     && m_settings->value(Param_Poste_PortRefracteur_COM_flowControl)   != QVariant())
    {
        val         = m_settings->value(Param_Poste_PortRefracteur_COM_baudrate).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortRefracteur_COM_databits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortRefracteur_COM_parity).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.parity = (QSerialPort::Parity)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortRefracteur_COM_stopBits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortRefracteur_COM_flowControl).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.flowControl = (QSerialPort::FlowControl)metaEnum.value(val);
    }
    if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
    {
        s_paramPortSerieRefracteur.baudRate       = QSerialPort::Baud2400;
        s_paramPortSerieRefracteur.dataBits       = QSerialPort::Data7;
        s_paramPortSerieRefracteur.parity         = QSerialPort::EvenParity;
        s_paramPortSerieRefracteur.stopBits       = QSerialPort::TwoStop;
        s_paramPortSerieRefracteur.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (nameRF =="TOMEY TAP-2000" || nameRF =="RODENSTOCK Phoromat 2000")
    {
        s_paramPortSerieRefracteur.baudRate       = QSerialPort::Baud2400;
        s_paramPortSerieRefracteur.dataBits       = QSerialPort::Data7;
        s_paramPortSerieRefracteur.parity         = QSerialPort::EvenParity;
        s_paramPortSerieRefracteur.stopBits       = QSerialPort::TwoStop;
        s_paramPortSerieRefracteur.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    if (a)
    {
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_flowControl,    Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.flowControl));
    }
    else
    {
        m_settings->setValue(Param_Poste_PortRefracteur_COM_baudrate,    QVariant());
        m_settings->setValue(Param_Poste_PortRefracteur_COM_databits,    QVariant());
        m_settings->setValue(Param_Poste_PortRefracteur_COM_parity,      QVariant());
        m_settings->setValue(Param_Poste_PortRefracteur_COM_stopBits,    QVariant());
        m_settings->setValue(Param_Poste_PortRefracteur_COM_flowControl, QVariant());
    }
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
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
    {
        if (m_mesureSerie == SendDataNIDEK("CRL"))
        {
            //Logs::LogToFile("PortSerieRefracteur", "SDN = " + m_mesureSerie + " - " + QDateTime().toString("dd-MM-yyyy HH:mm:ss"));
            //PortRefracteur()->waitForReadyRead(100);
            RegleRefracteurCOM(m_flagreglagerefracteurNidek);
            return;
        }
    }
    Datas::I()->mesureacuite->cleandatas();
    Datas::I()->mesurefinal->cleandatas();
    LectureDonneesCOMRefracteur(m_mesureSerie);
    if ( Datas::I()->mesureacuite->isdataclean() && Datas::I()->mesurefinal->isdataclean() )
        return;

    //! Enregistre les mesures dans la base
    InsertMesure(MesureRefracteur);

    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), les mesures qui viennent d'être effectuées
    emit NouvMesure(MesureRefracteur);
}

void Procedures::RegleRefracteur(TypesMesures flag)
{
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    if (t_threadRefracteur!=Q_NULLPTR) /*! par le port COM */
    {
        QString nompat = "";
        Patient *pat = Datas::I()->patients->currentpatient();
        if (pat)
            nompat = pat->prenom() + " " + pat->nom().toUpper();
        //! NIDEK RT-5100 - NIDEK RT-2100
        if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
        {
            /*!
            Logs::LogToFile("PortSerieRefracteur.txt", "RTS = " + RequestToSendNIDEK() + " - "
                            + QDateTime().toString("dd-MM-yyyy HH:mm:ss")
                            + (nompat != ""? " - " : "") + nompat);
            qDebug() << "RTS = " + RequestToSendNIDEK();
            */
            m_flagreglagerefracteurNidek = flag;
            Utils::writeDatasSerialPort(PortRefracteur(), RequestToSendNIDEK(), " RequestToSendNIDEK() - Refracteur = ");
        }
        //! TOMEY TAP-2000 et Rodenstock Phoromat 2000
        else if (nameRF =="TOMEY TAP-6000" || nameRF =="RODENSTOCK Phoromat 2000")
            RegleRefracteurCOM(flag);
    }
    //! NIDEK RT-6100 - NIDEK Glasspop
    else if (m_LANRefracteur) /*! par le réseau */
        RegleRefracteurXML(flag);
}

void Procedures::RegleRefracteurCOM(TypesMesures flag)
{
    /*! Si on lance cette fonction à l'ouverture d'un dossier, on a créé 3 mesures
     * Chacune de ces 3 mesures est envoyée au réfracteur pour le régler
     * Datas::I()->mesurefronto     qui met en fronto et en final la dernière prescription de verres du patient
     * Datas::I()->mesureautoref    qui met en autoref la dernière mesure d'autoref du patient
     * Datas::I()->mesureacuité     qui met en subjectif la dernière mesure d'acuité du patient
     */
    /*! +++ sur les NIDEK, on ne peut que régler l'autoref et le fronto depuis le PC - les refractions subjectives et finales ne peuvent pas être préréglées */
    int idpat = 0;
    QString AxeOD, AxeOG;
    QString AddOD, AddOG;
    QString SphereOD, SphereOG;
    QString CylindreOD, CylindreOG;
    QString SCAOD, SCAOG;
    QString DataAEnvoyer;
    QByteArray DTRbuff;
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
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

    // ----------------- CONNECTION SERIE
    // NIDEK RT-5100 - RT-2100 =======================================================================================================================================
    if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
    {
        auto convertdioptriesNIDEK = [&] (QString &finalvalue, double originvalue)
        {
            if (originvalue > 0)
                finalvalue = (originvalue < 10? "+0" : "+") + QString::number(originvalue,'f',2);
            else if (originvalue < 0)
                finalvalue = (originvalue > -10? QString::number(originvalue,'f',2).replace("-", "-0") : QString::number(originvalue,'f',2));
        };
        auto convertaxeNIDEK = [&] (QString &finalvalue, int originvalue)
        {
            if (originvalue < 10)       finalvalue = "  " + QString::number(originvalue);
            else if (originvalue < 100) finalvalue = " "  + QString::number(originvalue);
            else                        finalvalue = QString::number(originvalue);
        };
        DTRbuff.append(QByteArray::fromHex("O1"));          //SOH -> start of header

        /*! réglage de l'autoref */
        if (flag.testFlag(Procedures::MesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
        {
            initvariables();
            convertaxeNIDEK(AxeOD, Datas::I()->mesureautoref->axecylindreOD());
            convertaxeNIDEK(AxeOG, Datas::I()->mesureautoref->axecylindreOG());
            convertdioptriesNIDEK(SphereOD, Datas::I()->mesureautoref->sphereOD());
            convertdioptriesNIDEK(SphereOG, Datas::I()->mesureautoref->sphereOG());
            convertdioptriesNIDEK(CylindreOD, Datas::I()->mesureautoref->cylindreOD());
            convertdioptriesNIDEK(CylindreOG, Datas::I()->mesureautoref->cylindreOG());

            SCAOD       = SphereOD + CylindreOD + AxeOD;
            SCAOG       = SphereOG + CylindreOG + AxeOG;
            SCAOD.replace("+0","+ ");
            SCAOD.replace("-0","- ");
            SCAOG.replace("+0","+ ");
            SCAOG.replace("-0","- ");
            DTRbuff.append("DRM");                              //section fronto
            DTRbuff.append(STX);           //STX -> start of text
            DTRbuff.append("OR"+ SCAOD);                        //SD
            DTRbuff.append(ETB);          //ETB -> end of text block
            DTRbuff.append("OL"+ SCAOG);                        //SD
            DTRbuff.append(ETB);          //ETB -> end of text block
            if (Datas::I()->mesureautoref->ecartIP() > 0)
            {
                DTRbuff.append("PD"+ QString::number(Datas::I()->mesureautoref->ecartIP()));
                //SD
                DTRbuff.append(ETB);      //ETB -> end of text block
            }
            idpat = Datas::I()->mesureautoref->idpatient();
        }

        /*! réglage du fronto */
        if (flag.testFlag(Procedures::MesureFronto) && !Datas::I()->mesurefronto->isdataclean())
        {
            initvariables();

            convertaxeNIDEK(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
            convertaxeNIDEK(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
            convertdioptriesNIDEK(SphereOD, Datas::I()->mesurefronto->sphereOD());
            convertdioptriesNIDEK(SphereOG, Datas::I()->mesurefronto->sphereOG());
            convertdioptriesNIDEK(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
            convertdioptriesNIDEK(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

            SCAOD       = SphereOD + CylindreOD + AxeOD;
            SCAOG       = SphereOG + CylindreOG + AxeOG;
            SCAOD.replace("+0","+ ");
            SCAOD.replace("-0","- ");
            SCAOG.replace("+0","+ ");
            SCAOG.replace("-0","- ");
            AddOD       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2);
            AddOG       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2);
            DTRbuff.append("DLM");                              //section fronto
            DTRbuff.append(STX);           //STX -> start of text
            DTRbuff.append(" R"+ SCAOD);                        //SD
            DTRbuff.append(ETB);          //ETB -> end of text block
            DTRbuff.append(" L"+ SCAOG);                        //SD
            DTRbuff.append(ETB);          //ETB -> end of text block
            DTRbuff.append("AR" + AddOD);                       //SD
            DTRbuff.append(ETB);          //ETB -> end of text block
            DTRbuff.append("AL" + AddOG);                       //SD
            DTRbuff.append(ETB);          //ETB -> end of text block
            if (Datas::I()->mesurefronto->ecartIP() > 0)
            {
                DTRbuff.append("PD"+ QString::number(Datas::I()->mesurefronto->ecartIP()));
                //SD
                DTRbuff.append(ETB);      //ETB -> end of text block
            }
            if (idpat == 0)
                idpat = Datas::I()->mesurefronto->idpatient();
        }
        DTRbuff.append(QByteArray::fromHex("4"));               //EOT -> end of transmission

        /*!
        qDebug() << "RegleRefracteur() - DTRBuff = " << QString(DTRbuff).toLocal8Bit() << "RegleRefracteur() - DTRBuff.size() = " << QString(DTRbuff).toLocal8Bit().size();
        QString nompat = tr("patient inconnu");
        Patient *pat = Datas::I()->patients->getById(idpat);
        if (pat)
            nompat = pat->prenom() + " " + pat->nom().toUpper();
        Logs::LogToFile("PortSerieRefracteur.txt", "Datas = " + QString(DTRbuff).toLocal8Bit() + " - "
                        + QDateTime().toString("dd-MM-yyyy HH:mm:ss") + " - " + nompat);
        //*/
        //        qint32 baud = port->baudRate();
        //        int timetowaitms= int (datas.size()*8*1000 / baud);
        //        timetowaitms += 10;
        //        qDebug() << " DTRbuff - Refracteur = " << QString(DTRbuff).toLocal8Bit();
        //        PortRefracteur()->write(QString(DTRbuff).toLocal8Bit());
        //        PortRefracteur()->flush();
        //        PortRefracteur()->waitForBytesWritten(1000);
        Utils::writeDatasSerialPort(PortRefracteur(), QString(DTRbuff).toLocal8Bit(), " DTRbuff - Refracteur = ", 1000);
    }
    // FIN NIDEK RT-5100 - RT-2100 =======================================================================================================================================
    // TOMEY TAP-2000 et Rodenstock Phoromat 2000 =======================================================================================================================================
    else if (nameRF =="TOMEY TAP-6000" || nameRF =="RODENSTOCK Phoromat 2000")
    {
        /*! SORTIE EXEMPLE POUR UN PHOROMAT RODENSTOCK
         * SOH =    QByteArray::fromHex("1")            //SOH -> start of header
         * STX =    QByteArray::fromHex("2")            //STX -> start of text
         * ETB =    QByteArray::fromHex("17")           //ETB -> end of text block
         * EOT =    QByteArray::fromHex("4")            //EOT -> end of transmission
         * La 1ere et la dernière lignes commencent par SOH et se terminent par EOT - représentés ici
         * Les autres lignes commencent par STX et se terminent par ETP


SOH*PC_SND_SEOT                 -> start block
*Phoromat 2000|000000001|0      -> id material
*PD|32.0|32.0|                  ->PD | left PD result | right PD result |
*WD|40|                         ->???
*UN                             -> AV sans correction
*VA|0.2|0.4|0.5|
*LM                             -> Fronto
*SP| -0.25| -0.25|              -> Sphere | left result | right result|
*CY| -0.25| -0.25|              -> Cylindre | left result | right result|
*AX|135|135|                    -> Axe | left result | right result|
*AD| 1.50| 1.50|                -> Addition | left result | right result|
*VA|0.2|0.4|0.5|                -> ???
*PH|O| 0.50|O| 0.50|            -> Prisme horizontal
*PV|U| 0.50|D| 0.50|            -> Prisme vertical
*AR                             -> Autoref
*SP| 0.25| 0.25|
*CY| -4.25| -4.25|
*AX| 45| 45|
*AD| 1.60| 1.70|
*VA|0.2|0.4|0.5|
*PH|O| 1.00|O| 1.00|
*PV|U| 1.50|D| 1.50|
*SJ                             -> Refraction subjective
*SP| 0.50| 0.50|
*CY| -0.25| -0.25|
*AX|135|135|
*AD| 1.50| 1.40|
*VA|0.2|0.4|0.5|
*PH|O| 0.50|O| 0.50|
*PV|U| 0.50|D| 0.50|
*FN                             -> Refraction finale
*SP| -0.50| -0.50|
*CY| -5.00| -5.00|
*AX| 30| 30|
*AD| 1.70| 1.50|
*VA|0.2|0.4|0.5|
*PH|O| 0.50|O| 0.50|
*PV|D| 1.00|U| 1.00|
*KM|R1|L| 8.04| 42.00| 85|      -> Keratometrie | R1 | Side | mm | Diopter | Axis |
*KM|R2|L| 7.74| 43.50|175|      -> Keratometrie | R2 | Side | mm | Diopter | Axis |
*KM|R1|R| 8.04| 42.00| 85|      -> Keratometrie | R1 | Side | mm | Diopter | Axis |
*KM|R2|R| 7.74| 43.50|175|      -> Keratometrie | R2 | Side | mm | Diopter | Axis |
*AV                             -> Advanced data (??? - absolutely unexplained in user manual...)
*NPC|-8| 9.5| 64.8|
*NPA|B|33| 3.03|
*NPA|L|33| 3.03|
*NPA|R|33| 3.03|
*NRA|B| 2.00| 1.75|
*PRA|B| 3.50| 2.50|
*DIV|f| 9.00|16.00|10.00|
*DIV|n| 3.00| 5.25| 8.00|
*CON|f| 7.00|20.00|15.00|
*CON|n| 2.00| 4.10| 6.00|
*TIME|2012/10/06 04:06:58       -> time date
SOH*PC_SND_EEOT                 -> end block
*/

        auto convertdioptriesTOMEY = [&] (QString &finalvalue, double originvalue)
        {
            originvalue = Utils::roundToNearestPointTwentyFive(originvalue);
            if (originvalue < 0)
                finalvalue = (originvalue > -10? QString::number(originvalue,'f',2).replace("-", " -") : QString::number(originvalue,'f',2));
            else
                finalvalue = (originvalue < 10? " " : "") + QString::number(originvalue,'f',2);
        };
        auto convertaxeTOMEY = [&] (QString &finalvalue, int originvalue)
        {
            originvalue = Utils::roundToNearestFive(originvalue);
            if (originvalue < 10)       finalvalue = "  " + QString::number(originvalue);
            else if (originvalue < 100) finalvalue = " "  + QString::number(originvalue);
            else                        finalvalue = QString::number(originvalue);
        };
        /*! séquence de départ
            SOH*PC_SND_SEOT
            *Phoromat 2000|000000001|0
        */
        DTRbuff.append(SOH);          //SOH -> start of header
        DTRbuff.append("*PC_SND_S");
        DTRbuff.append(QByteArray::fromHex("4"));          //EOT -> end of transmission
        DTRbuff.append(STX);          //STX -> start of text
        DTRbuff.append("*Phoromat 2000|000000001|0");
        DTRbuff.append(ETB);         //ETB -> end of text block

        /*! écart interpupillaire
            *PD|32.0|32.0|                  ->PD | left PD result | right PD result |        */
        DTRbuff.append(STX);          //STX -> start of text
        double eip = static_cast<double>(Datas::I()->mesureautoref->ecartIP());
        QString halfeip = QString::number(eip/2,'f',1);    // quel bricolage nul....
        DTRbuff.append("*PD|" + halfeip + "|" + halfeip + "|");
        DTRbuff.append(ETB);         //ETB -> end of text block

        /*! réglage du fronto`
            *LM                             -> Fronto
            *SP| -0.25| -0.25|              -> Sphere | left result | right result|
            *CY| -0.25| -0.25|              -> Cylindre | left result | right result|
            *AX|135|135|                    -> Axe | left result | right result|
            *AD| 1.50| 1.50|                -> Addition | left result | right result|        */
        if (flag.testFlag(Procedures::MesureFronto) && !Datas::I()->mesurefronto->isdataclean())
        {
            initvariables();

            convertaxeTOMEY(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
            convertaxeTOMEY(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
            convertdioptriesTOMEY(SphereOD, Datas::I()->mesurefronto->sphereOD());
            convertdioptriesTOMEY(SphereOG, Datas::I()->mesurefronto->sphereOG());
            convertdioptriesTOMEY(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
            convertdioptriesTOMEY(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*LM");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*SP|"+ SphereOD + "|" + SphereOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*CY|"+ CylindreOD + "|" + CylindreOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*AX|"+ AxeOD + "|" + AxeOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*AD|"+ AddOD + "|" + AddOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            if (idpat == 0)
                idpat = Datas::I()->mesurefronto->idpatient();
        }

        /*! réglage de l'autoref
            *AR                             -> Autoref
            *SP| 0.25| 0.25|
            *CY| -4.25| -4.25|
            *AX| 45| 45|
            *AD| 1.60| 1.70|
            *VA|0.2|0.4|0.5|
            *PH|O| 1.00|O| 1.00|
            *PV|U| 1.50|D| 1.50|                */
        if (flag.testFlag(Procedures::MesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
        {
            initvariables();
            convertaxeTOMEY(AxeOD, Datas::I()->mesureautoref->axecylindreOD());
            convertaxeTOMEY(AxeOG, Datas::I()->mesureautoref->axecylindreOG());
            convertdioptriesTOMEY(SphereOD, Datas::I()->mesureautoref->sphereOD());
            convertdioptriesTOMEY(SphereOG, Datas::I()->mesureautoref->sphereOG());
            convertdioptriesTOMEY(CylindreOD, Datas::I()->mesureautoref->cylindreOD());
            convertdioptriesTOMEY(CylindreOG, Datas::I()->mesureautoref->cylindreOG());

            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*AR");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*SP|"+ SphereOD + "|" + SphereOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*CY|"+ CylindreOD + "|" + CylindreOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*AX|"+ AxeOD + "|" + AxeOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*AD|"+ AddOD + "|" + AddOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            if (idpat == 0)
                idpat = Datas::I()->mesurefronto->idpatient();
        }

        /*! réglage de refraction subjecctive
            *SJ                             -> Refraction subjective
            *SP| 0.50| 0.50|
            *CY| -0.25| -0.25|
            *AX|135|135|
            *AD| 1.50| 1.40|
            *VA|0.2|0.4|0.5|
            *PH|O| 0.50|O| 0.50|
            *PV|U| 0.50|D| 0.50|    */
        if (flag.testFlag(Procedures::MesureRefracteur) && !Datas::I()->mesurefronto->isdataclean())
        {
            initvariables();

            convertaxeTOMEY(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
            convertaxeTOMEY(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
            convertdioptriesTOMEY(SphereOD, Datas::I()->mesurefronto->sphereOD());
            convertdioptriesTOMEY(SphereOG, Datas::I()->mesurefronto->sphereOG());
            convertdioptriesTOMEY(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
            convertdioptriesTOMEY(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*SJ");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*SP|"+ SphereOD + "|" + SphereOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*CY|"+ CylindreOD + "|" + CylindreOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*AX|"+ AxeOD + "|" + AxeOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            DTRbuff.append(STX);          //STX -> start of text
            DTRbuff.append("*AD|"+ AddOD + "|" + AddOG + "|");
            DTRbuff.append(ETB);         //ETB -> end of text block
            if (idpat == 0)
                idpat = Datas::I()->mesurefronto->idpatient();
        }
        /*! séquence de fin
            SOH*PC_SND_EEOT
        */
        DTRbuff.append(SOH);          //SOH -> start of header
        DTRbuff.append("*PC_SND_E");
        DTRbuff.append(EOT);          //EOT -> end of transmission

        /*!
        qDebug() << "RegleRefracteur() - DTRBuff = " << QString(DTRbuff).toLocal8Bit() << "RegleRefracteur() - DTRBuff.size() = " << QString(DTRbuff).toLocal8Bit().size();
        QString nompat = "";
        Patient *pat = Datas::I()->patients->getById(idpat);
        if (pat)
            nompat = pat->prenom() + " " + pat->nom().toUpper();
        Logs::LogToFile("PortSerieRefracteur.txt", "Datas = " + QString(DTRbuff).toLocal8Bit() + " - "
                        + QDateTime().toString("dd-MM-yyyy HH:mm:ss")
                        + (nompat != ""? " - " : "") + nompat);
        */
        //Edit(QString(DTRbuff).toLocal8Bit());
        //        qint32 baud = port->baudRate();
//        int timetowaitms= int (datas.size()*8*1000 / baud);
//        timetowaitms += 10;
//        qDebug() << " DTRbuff - Refracteur = " << QString(DTRbuff).toLocal8Bit();
//        PortRefracteur()->write(QString(DTRbuff).toLocal8Bit());
//        PortRefracteur()->flush();
//        PortRefracteur()->waitForBytesWritten(1000);
        Utils::writeDatasSerialPort(PortRefracteur(), QString(DTRbuff).toLocal8Bit(), " DTRbuff - Refracteur = ", 1000);
    }
    // FIN TOMEY TAP-2000 et Rodenstock Phoromat 2000 =======================================================================================================================================
}

void Procedures::RegleRefracteurXML(TypesMesures flag)
{
    /*! Si on lance cette fonction à l'ouverture d'un dossier, on a créé 3 mesures
     * Chacune de ces 3 mesures est envoyée au réfracteur pour le régler
     * Datas::I()->mesurefronto     qui met en fronto et en final la dernière prescription de verres du patient
     * Datas::I()->mesureautoref    qui met en autoref la dernière mesure d'autoref du patient
     * Datas::I()->mesureacuité     qui met en subjectif la dernière mesure d'acuité du patient
     */
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    auto EnregistreFileDatasXML = [] (QDomDocument xml, Procedures::TypeMesure typmesure)
    {
        const QByteArray codecname = "UTF16LE";
        QTextCodec *codec = QTextCodec::codecForName(codecname);
        QString Adress ("");
        QString typfile("");
        if (typmesure == Procedures::MesureAutoref)
        {
            Adress = Procedures::I()->settings()->value(Param_Poste_PortRefracteur_Reseau_AdressAutoref).toString();
            typfile = "ARK";
        }
        else if (typmesure == Procedures::MesureFronto)
        {
            Adress = Procedures::I()->settings()->value(Param_Poste_PortRefracteur_Reseau_AdressFronto).toString();
            typfile = "LM";
        }
        else
            return;
        QDir Dir(Adress);
        if (!Dir.exists(Adress))
            Dir.mkdir(Adress);
        QStringList listfiles = Dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for(int i = 0; i < listfiles.size(); ++i)
            QFile(Adress + "/" + listfiles.at(i)).remove();
        QString filename = Adress + "/" + typfile + "_" + codecname + "_" + QString::number(Datas::I()->patients->currentpatient()->id()) + ".xml";
        QFile file(filename);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream( &file );
            stream.setCodec(codec);         /*! Impose le codec UTF16LE que les Nidek exigent pour les fichiers xml */
            QString strxml = xml.toString();
            stream << strxml;
            file.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                                  | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                                  | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                  | QFileDevice::ReadUser   | QFileDevice::WriteUser);
            file.close();
        }
    };


    if (nameRF =="NIDEK RT-6100" || nameRF =="NIDEK Glasspop")
    {
        /*! Il faut régler le réfracteur en créant un fichier xml pour l'autoref et un pour le fronto à partir des données du dossier du patient en cours
         * Il faut déposer ces fichiers dans le dossier réseau correspondant surveillé par le refracteur
        */

/*! LE FRONTO */
        bool ExistMesureFronto = flag.testFlag(Procedures::MesureFronto) && !Datas::I()->mesurefronto->isdataclean();
        if (ExistMesureFronto)
        {
            QDomDocument LMxml("");
            QDomElement ophtalmology = LMxml.createElement("Ophtalmology");
            LMxml.appendChild(ophtalmology);
            {
                QDomElement Common = LMxml.createElement("Common");
                ophtalmology.appendChild(Common);
                {
                    QDomElement company = LMxml.createElement("Company");
                    Common.appendChild(company);
                        company.appendChild(LMxml.createTextNode("NIDEK"));
                    QDomElement model = LMxml.createElement("ModelName");
                    Common.appendChild(model);
                        model.appendChild(LMxml.createTextNode("LM-1800PD"));
                    Common.appendChild(LMxml.createElement("MachineNo"));
                    Common.appendChild(LMxml.createElement("ROMVersion"));
                    QDomElement version = LMxml.createElement("Version");
                    Common.appendChild(version);
                        version.appendChild(LMxml.createTextNode("NIDEK_V1.01"));
                    QDomElement date = LMxml.createElement("Date");
                    Common.appendChild(date);
                        date.appendChild(LMxml.createTextNode(QDate::currentDate().toString(tr("yyyy/MM/dd"))));
                    QDomElement time = LMxml.createElement("Time");
                    Common.appendChild(time);
                        time.appendChild(LMxml.createTextNode(QTime::currentTime().toString(tr("HH:mm:ss"))));
                    QDomElement patient = LMxml.createElement("Patient");
                    Common.appendChild(patient);
                    {
                        QDomElement patientno = LMxml.createElement("No.");
                        patient.appendChild(patientno);
                        {
                            QDomText id = LMxml.createTextNode(Datas::I()->patients->currentpatient()? QString::number(Datas::I()->patients->currentpatient()->id()) : "0");
                            patientno.appendChild(id);
                        }
                        QDomElement patientid = LMxml.createElement("ID");
                        patient.appendChild(patientid);
                        {
                            QDomText id = LMxml.createTextNode(Datas::I()->patients->currentpatient()? QString::number(Datas::I()->patients->currentpatient()->id()) : "0");
                            patientid.appendChild(id);
                        }
                        patient.appendChild(LMxml.createElement("FirstName"));
                        patient.appendChild(LMxml.createElement("MiddleName"));
                        patient.appendChild(LMxml.createElement("LastName"));
                        patient.appendChild(LMxml.createElement("Sex"));
                        patient.appendChild(LMxml.createElement("Age"));
                        patient.appendChild(LMxml.createElement("DOB"));
                        patient.appendChild(LMxml.createElement("NameJ1"));
                        patient.appendChild(LMxml.createElement("NameJ2"));
                    }
                    QDomElement oper = LMxml.createElement("Operator");
                    Common.appendChild(oper);
                    {
                        oper.appendChild(LMxml.createElement("No."));
                        oper.appendChild(LMxml.createElement("ID"));
                    }
                }
                QDomElement Measure = LMxml.createElement("Measure");
                Measure.setAttribute("Type","LM");
                ophtalmology.appendChild(Measure);
                {
                    Measure.appendChild(LMxml.createElement("MeasureMode"));
                    Measure.appendChild(LMxml.createElement("DiopterStep"));
                    Measure.appendChild(LMxml.createElement("AxisStep"));
                    Measure.appendChild(LMxml.createElement("CylinderMode"));
                    Measure.appendChild(LMxml.createElement("PrismDiopterStep"));
                    Measure.appendChild(LMxml.createElement("PrismBaseStep"));
                    Measure.appendChild(LMxml.createElement("PrismMode"));
                    Measure.appendChild(LMxml.createElement("AddMode"));
                    QDomElement lm = LMxml.createElement("LM");
                    Measure.appendChild(lm);
                    {
                        QDomElement s = LMxml.createElement("S");
                        lm.appendChild(s);
                        {
                            s.appendChild(LMxml.createElement("Sphere"));
                            s.appendChild(LMxml.createElement("Cylinder"));
                            s.appendChild(LMxml.createElement("Axis"));
                            s.appendChild(LMxml.createElement("SE"));
                            s.appendChild(LMxml.createElement("ADD"));
                            s.appendChild(LMxml.createElement("ADD2"));
                            s.appendChild(LMxml.createElement("NearSphere"));
                            s.appendChild(LMxml.createElement("NearSPhere2"));
                            s.appendChild(LMxml.createElement("Prism"));
                            s.appendChild(LMxml.createElement("PrismBase"));
                            s.appendChild(LMxml.createElement("PrismX"));
                            s.appendChild(LMxml.createElement("PrismY"));
                            s.appendChild(LMxml.createElement("Prism"));
                            s.appendChild(LMxml.createElement("UVTransmittance"));
                            s.appendChild(LMxml.createElement("ConfidenceIndex"));
                        }
                        QDomElement r = LMxml.createElement("R");
                        lm.appendChild(r);
                        {
                            QDomElement sph = LMxml.createElement("Sphere");
                            sph.setAttribute("unit","D");
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->sphereOD(),'f',2));
                                sph.appendChild(val);
                            }
                            r.appendChild(sph);
                            QDomElement cyl = LMxml.createElement("Cylinder");
                            cyl.setAttribute("unit","D");
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->cylindreOD(),'f',2));
                                cyl.appendChild(val);
                            }
                            r.appendChild(cyl);
                            QDomElement ax = LMxml.createElement("Axis");
                            ax.setAttribute("unit","deg");
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->axecylindreOD()));
                                ax.appendChild(val);
                            }
                            r.appendChild(ax);
                            r.appendChild(LMxml.createElement("SE"));
                            QDomElement add = LMxml.createElement("ADD");
                            r.appendChild(add);
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2));
                                add.appendChild(val);
                            }
                            r.appendChild(add);
                            r.appendChild(LMxml.createElement("ADD2"));
                            r.appendChild(LMxml.createElement("NearSphere"));
                            r.appendChild(LMxml.createElement("NearSPhere2"));
                            r.appendChild(LMxml.createElement("Prism"));
                            r.appendChild(LMxml.createElement("PrismBase"));
                            r.appendChild(LMxml.createElement("PrismX"));
                            r.appendChild(LMxml.createElement("PrismY"));
                            r.appendChild(LMxml.createElement("Prism"));
                            r.appendChild(LMxml.createElement("UVTransmittance"));
                            r.appendChild(LMxml.createElement("ConfidenceIndex"));
                        }
                        QDomElement l = LMxml.createElement("L");
                        lm.appendChild(l);
                        {
                            QDomElement sph = LMxml.createElement("Sphere");
                            sph.setAttribute("unit","D");
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->sphereOG(),'f',2));
                                sph.appendChild(val);
                            }
                            l.appendChild(sph);
                            QDomElement cyl = LMxml.createElement("Cylinder");
                            cyl.setAttribute("unit","D");
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->cylindreOG(),'f',2));
                                cyl.appendChild(val);
                            }
                            l.appendChild(cyl);
                            QDomElement ax = LMxml.createElement("Axis");
                            ax.setAttribute("unit","deg");
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->axecylindreOG()));
                                ax.appendChild(val);
                            }
                            l.appendChild(ax);
                            l.appendChild(LMxml.createElement("SE"));
                            QDomElement add = LMxml.createElement("ADD");
                            l.appendChild(add);
                            {
                                QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2));
                                add.appendChild(val);
                            }
                            l.appendChild(add);
                            l.appendChild(LMxml.createElement("ADD2"));
                            l.appendChild(LMxml.createElement("NearSphere"));
                            l.appendChild(LMxml.createElement("NearSPhere2"));
                            l.appendChild(LMxml.createElement("Prism"));
                            l.appendChild(LMxml.createElement("PrismBase"));
                            l.appendChild(LMxml.createElement("PrismX"));
                            l.appendChild(LMxml.createElement("PrismY"));
                            l.appendChild(LMxml.createElement("Prism"));
                            l.appendChild(LMxml.createElement("UVTransmittance"));
                            l.appendChild(LMxml.createElement("ConfidenceIndex"));
                        }
                    }
                    QDomElement pd = LMxml.createElement("PD");
                    Measure.appendChild(pd);
                    {
                        QDomElement pdb = LMxml.createElement("Distance");
                        pdb.setAttribute("Unit","mm");
                        if (Datas::I()->mesurefronto->ecartIP()>0)
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->ecartIP()));
                            pdb.appendChild(val);
                        }
                        pd.appendChild(pdb);
                        QDomElement pdr = LMxml.createElement("DistanceR");
                        pdr.setAttribute("Unit","mm");
                        pd.appendChild(pdr);
                        QDomElement pdl = LMxml.createElement("DistanceR");
                        pdl.setAttribute("Unit","mm");
                        pd.appendChild(pdl);
                    }
                    QDomElement nidek = LMxml.createElement("NIDEK");
                    Measure.appendChild(nidek);
                    {
                        QDomElement s = LMxml.createElement("S");
                        nidek.appendChild(s);
                        {
                            QDomElement length = LMxml.createElement("Length");
                            length.setAttribute("unit","mm");
                            s.appendChild(length);
                            QDomElement chwidth = LMxml.createElement("Length");
                            chwidth.setAttribute("unit","mm");
                            s.appendChild(chwidth);
                            QDomElement chlength = LMxml.createElement("Length");
                            chlength.setAttribute("unit","mm");
                            s.appendChild(chlength);
                            s.appendChild(LMxml.createElement("Index"));
                            QDomElement gtrs = LMxml.createElement("GreenTransmittance");
                            gtrs.setAttribute("unit","%");
                            s.appendChild(gtrs);
                        }
                        QDomElement r = LMxml.createElement("R");
                        nidek.appendChild(r);
                        {
                            QDomElement length = LMxml.createElement("Length");
                            length.setAttribute("unit","mm");
                            r.appendChild(length);
                            QDomElement chwidth = LMxml.createElement("Length");
                            chwidth.setAttribute("unit","mm");
                            r.appendChild(chwidth);
                            QDomElement chlength = LMxml.createElement("Length");
                            chlength.setAttribute("unit","mm");
                            r.appendChild(chlength);
                            r.appendChild(LMxml.createElement("Index"));
                            QDomElement gtrs = LMxml.createElement("GreenTransmittance");
                            gtrs.setAttribute("unit","%");
                            r.appendChild(gtrs);
                        }
                        QDomElement l = LMxml.createElement("L");
                        nidek.appendChild(l);
                        {
                            QDomElement length = LMxml.createElement("Length");
                            length.setAttribute("unit","mm");
                            l.appendChild(length);
                            QDomElement chwidth = LMxml.createElement("Length");
                            chwidth.setAttribute("unit","mm");
                            l.appendChild(chwidth);
                            QDomElement chlength = LMxml.createElement("Length");
                            chlength.setAttribute("unit","mm");
                            l.appendChild(chlength);
                            l.appendChild(LMxml.createElement("Index"));
                            QDomElement gtrs = LMxml.createElement("GreenTransmittance");
                            gtrs.setAttribute("unit","%");
                            l.appendChild(gtrs);
                        }
                        QDomElement netprism = LMxml.createElement("NetPrism");
                        nidek.appendChild(netprism);
                        {
                            QDomElement neth = LMxml.createElement("NetHPrism");
                            neth.setAttribute("unit","pri");
                            neth.setAttribute("base","in");
                            netprism.appendChild(neth);
                            QDomElement netv = LMxml.createElement("NetVPrism");
                            netv.setAttribute("unit","pri");
                            netv.setAttribute("base","out");
                            netprism.appendChild(netv);
                        }
                        QDomElement inside = LMxml.createElement("Inside");
                        nidek.appendChild(netprism);
                        {
                            QDomElement r = LMxml.createElement("InsideR");
                            r.setAttribute("unit","mm");
                            inside.appendChild(r);
                            QDomElement l = LMxml.createElement("InsideL");
                            l.setAttribute("unit","mm");
                            inside.appendChild(l);
                        }
                    }
                }
            }
            EnregistreFileDatasXML(LMxml, Procedures::MesureFronto);
        }

/*! L'AUTOREF */
        bool ExistMesureAutoref = flag.testFlag(Procedures::MesureAutoref) && !Datas::I()->mesureautoref->isdataclean();
        if (ExistMesureAutoref)
        {
            QDomDocument ARxml("");
            QDomElement Data = ARxml.createElement("Data");
            ARxml.appendChild(Data);
            QDomElement company = ARxml.createElement("Company");
            Data.appendChild(company);
            {
                QDomText t = ARxml.createTextNode("NIDEK");
                company.appendChild(t);
            }
            QDomElement model = ARxml.createElement("ModelName");
            Data.appendChild(model);
            {
                QDomText u = ARxml.createTextNode("ARK-1s");
                model.appendChild(u);
            }
            QDomElement rom = ARxml.createElement("ROMVersion");
            Data.appendChild(rom);
            QDomElement date = ARxml.createElement("Date");
            Data.appendChild(date);
            {
                QDomText w = ARxml.createTextNode(QDate::currentDate().toString(tr("yyyy/MM/dd")));
                date.appendChild(w);
            }
            QDomElement time = ARxml.createElement("Time");
            Data.appendChild(time);
            {
                QDomText x = ARxml.createTextNode(QTime::currentTime().toString(tr("HH:mm:ss")));
                time.appendChild(x);
            }
            QDomElement patient = ARxml.createElement("Patient");
            Data.appendChild(patient);
            {
                QDomElement patientno = ARxml.createElement("No.");
                patient.appendChild(patientno);
                {
                    QDomText id = ARxml.createTextNode(Datas::I()->patients->currentpatient()? QString::number(Datas::I()->patients->currentpatient()->id()) : "0");
                    patientno.appendChild(id);
                }
                QDomElement patientid = ARxml.createElement("ID");
                patient.appendChild(patientid);
            }
            QDomElement comment = ARxml.createElement("Comment");
            Data.appendChild(comment);
            QDomElement vd = ARxml.createElement("VD");
            Data.appendChild(vd);
            QDomElement wd = ARxml.createElement("WorkingDistance");
            Data.appendChild(wd);
            QDomElement ds = ARxml.createElement("DiopterStep");
            Data.appendChild(ds);
            QDomElement as = ARxml.createElement("AxisStep");
            Data.appendChild(as);
            QDomElement cm = ARxml.createElement("CylinderMode");
            Data.appendChild(cm);
            QDomElement ri = ARxml.createElement("RefractiveIndex");
            Data.appendChild(ri);
            QDomElement r = ARxml.createElement("R");
            Data.appendChild(r);
            {
                QDomElement ar = ARxml.createElement("AR");
                r.appendChild(ar);
                {
                    QDomElement arlist = ARxml.createElement("ARList");
                    ar.appendChild(arlist);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        arlist.appendChild(sph);
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        arlist.appendChild(cyl);
                        QDomElement ax = ARxml.createElement("Axis");
                        arlist.appendChild(ax);
                        QDomElement cm = ARxml.createElement("CataractMode");
                        arlist.appendChild(cm);
                        QDomElement ci = ARxml.createElement("ConfidenceIndex");
                        arlist.appendChild(ci);
                        QDomElement se = ARxml.createElement("SE");
                        arlist.appendChild(se);
                    }
                    QDomElement armed = ARxml.createElement("ARMedian");
                    ar.appendChild(armed);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        armed.appendChild(sph);
                        {
                            QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->sphereOD(),'f',2));
                            sph.appendChild(val);
                        }
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        armed.appendChild(cyl);
                        {
                            QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->cylindreOD(),'f',2));
                            cyl.appendChild(val);
                        }
                        QDomElement ax = ARxml.createElement("Axis");
                        armed.appendChild(ax);
                        {
                            QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->axecylindreOD()));
                            ax.appendChild(val);
                        }
                        QDomElement se = ARxml.createElement("SE");
                        armed.appendChild(se);
                    }
                    QDomElement tl = ARxml.createElement("TrialLens");
                    ar.appendChild(tl);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        tl.appendChild(sph);
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        tl.appendChild(cyl);
                        QDomElement ax = ARxml.createElement("Axis");
                        tl.appendChild(ax);
                    }
                    QDomElement cl = ARxml.createElement("ContactLens");
                    ar.appendChild(cl);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        cl.appendChild(sph);
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        cl.appendChild(cyl);
                        QDomElement ax = ARxml.createElement("Axis");
                        cl.appendChild(ax);
                        QDomElement se = ARxml.createElement("SE");
                        cl.appendChild(se);
                    }
                    QDomElement img = ARxml.createElement("RingImage");
                    ar.appendChild(img);

                }
                QDomElement va = ARxml.createElement("VA");
                r.appendChild(va);
                {
                    QDomElement ucva = ARxml.createElement("UCVA");
                    va.appendChild(ucva);
                    QDomElement bcva = ARxml.createElement("BCVA");
                    va.appendChild(bcva);
                    QDomElement lva = ARxml.createElement("LVA");
                    va.appendChild(lva);
                    QDomElement gva = ARxml.createElement("GVA");
                    va.appendChild(gva);
                    QDomElement nva = ARxml.createElement("NVA");
                    va.appendChild(nva);
                    QDomElement wd = ARxml.createElement("WorkingDistance");
                    va.appendChild(wd);
                }
                QDomElement sr = ARxml.createElement("SR");
                r.appendChild(sr);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    sr.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    sr.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    sr.appendChild(ax);
                    QDomElement se = ARxml.createElement("SE");
                    sr.appendChild(se);
                    QDomElement add = ARxml.createElement("ADD");
                    sr.appendChild(add);
                    QDomElement wd = ARxml.createElement("WorkingDistance");
                    sr.appendChild(wd);
                }
                QDomElement lm = ARxml.createElement("LM");
                r.appendChild(lm);
                {
                    lm.appendChild(ARxml.createElement("Sphere"));
                    lm.appendChild(ARxml.createElement("Cylinder"));
                    lm.appendChild(ARxml.createElement("Axis"));
                    lm.appendChild(ARxml.createElement("ADD"));
                    lm.appendChild(ARxml.createElement("ADD2"));
                }
                QDomElement km = ARxml.createElement("KM");
                r.appendChild(km);
                {
                    QDomElement kmlist = ARxml.createElement("KMList");
                    kmlist.setAttribute("No","1");
                    km.appendChild(kmlist);
                    {
                        QDomElement r1 = ARxml.createElement("R1");
                        kmlist.appendChild(r1);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r1.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r1.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r1.appendChild(ax);
                        }
                        QDomElement r2 = ARxml.createElement("R2");
                        kmlist.appendChild(r2);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r2.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r2.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r2.appendChild(ax);
                        }
                        QDomElement avg = ARxml.createElement("Average");
                        kmlist.appendChild(avg);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            avg.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            avg.appendChild(pwr);
                        }
                        QDomElement kmc = ARxml.createElement("KMCylinder");
                        kmlist.appendChild(kmc);
                        {
                            QDomElement pwr = ARxml.createElement("Power");
                            kmc.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            kmc.appendChild(ax);
                        }
                    }
                    QDomElement kmmed = ARxml.createElement("KMMedian");
                    km.appendChild(kmmed);
                    {
                        QDomElement r1 = ARxml.createElement("R1");
                        kmmed.appendChild(r1);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r1.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r1.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r1.appendChild(ax);
                        }
                        QDomElement r2 = ARxml.createElement("R2");
                        kmmed.appendChild(r2);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r2.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r2.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r2.appendChild(ax);
                        }
                        QDomElement avg = ARxml.createElement("Average");
                        kmmed.appendChild(avg);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            avg.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            avg.appendChild(pwr);
                        }
                        QDomElement kmc = ARxml.createElement("KMCylinder");
                        kmmed.appendChild(kmc);
                        {
                            QDomElement pwr = ARxml.createElement("Power");
                            kmc.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            kmc.appendChild(ax);
                        }
                    }
                }
                QDomElement cs = ARxml.createElement("CS");
                r.appendChild(cs);
                {
                    QDomElement cslist = ARxml.createElement("CSList");
                    cslist.setAttribute("No","1");
                    cs.appendChild(cslist);
                    {
                        QDomElement sz = ARxml.createElement("Size");
                        cslist.appendChild(sz);
                    }
                }
                QDomElement ps = ARxml.createElement("PS");
                r.appendChild(ps);
                {
                    QDomElement pslist = ARxml.createElement("PSList");
                    pslist.setAttribute("No","1");
                    ps.appendChild(pslist);
                    {
                        QDomElement sz = ARxml.createElement("Size");
                        pslist.appendChild(sz);
                        QDomElement lp = ARxml.createElement("Lamp");
                        pslist.appendChild(lp);
                    }
                }
                QDomElement ac = ARxml.createElement("AC");
                r.appendChild(ac);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    ac.appendChild(sph);
                    QDomElement maxps = ARxml.createElement("MaxPS");
                    ac.appendChild(maxps);
                    QDomElement minps = ARxml.createElement("MinPS");
                    ac.appendChild(minps);
                    QDomElement img = ARxml.createElement("AccImage");
                    ac.appendChild(img);
                }
                QDomElement ri = ARxml.createElement("RI");
                r.appendChild(ri);
                {
                    QDomElement coih = ARxml.createElement("COIH");
                    ri.appendChild(coih);
                    QDomElement coia = ARxml.createElement("COIA");
                    ri.appendChild(coia);
                    QDomElement poi = ARxml.createElement("POI");
                    ri.appendChild(poi);
                    QDomElement rimg = ARxml.createElement("RetroImage");
                    ri.appendChild(rimg);
                }
            }
            QDomElement l = ARxml.createElement("L");
            Data.appendChild(l);
            {
                QDomElement ar = ARxml.createElement("AR");
                l.appendChild(ar);
                {
                    QDomElement arlist = ARxml.createElement("ARList");
                    ar.appendChild(arlist);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        arlist.appendChild(sph);
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        arlist.appendChild(cyl);
                        QDomElement ax = ARxml.createElement("Axis");
                        arlist.appendChild(ax);
                        QDomElement cm = ARxml.createElement("CataractMode");
                        arlist.appendChild(cm);
                        QDomElement ci = ARxml.createElement("ConfidenceIndex");
                        arlist.appendChild(ci);
                        QDomElement se = ARxml.createElement("SE");
                        arlist.appendChild(se);
                    }
                    QDomElement armed = ARxml.createElement("ARMedian");
                    ar.appendChild(armed);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        armed.appendChild(sph);
                        {
                            QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->sphereOG(),'f',2));
                            sph.appendChild(val);
                        }
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        armed.appendChild(cyl);
                        {
                            QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->cylindreOG(),'f',2));
                            cyl.appendChild(val);
                        }
                        QDomElement ax = ARxml.createElement("Axis");
                        armed.appendChild(ax);
                        {
                            QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->axecylindreOG()));
                            ax.appendChild(val);
                        }
                        QDomElement se = ARxml.createElement("SE");
                        armed.appendChild(se);
                    }
                    QDomElement tl = ARxml.createElement("TrialLens");
                    ar.appendChild(tl);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        tl.appendChild(sph);
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        tl.appendChild(cyl);
                        QDomElement ax = ARxml.createElement("Axis");
                        tl.appendChild(ax);
                    }
                    QDomElement cl = ARxml.createElement("ContactLens");
                    ar.appendChild(cl);
                    {
                        QDomElement sph = ARxml.createElement("Sphere");
                        cl.appendChild(sph);
                        QDomElement cyl = ARxml.createElement("Cylinder");
                        cl.appendChild(cyl);
                        QDomElement ax = ARxml.createElement("Axis");
                        cl.appendChild(ax);
                        QDomElement se = ARxml.createElement("SE");
                        cl.appendChild(se);
                    }
                    QDomElement img = ARxml.createElement("RingImage");
                    ar.appendChild(img);

                }
                QDomElement va = ARxml.createElement("VA");
                l.appendChild(va);
                {
                    QDomElement ucva = ARxml.createElement("UCVA");
                    va.appendChild(ucva);
                    QDomElement bcva = ARxml.createElement("BCVA");
                    va.appendChild(bcva);
                    QDomElement lva = ARxml.createElement("LVA");
                    va.appendChild(lva);
                    QDomElement gva = ARxml.createElement("GVA");
                    va.appendChild(gva);
                    QDomElement nva = ARxml.createElement("NVA");
                    va.appendChild(nva);
                    QDomElement wd = ARxml.createElement("WorkingDistance");
                    va.appendChild(wd);
                }
                QDomElement sr = ARxml.createElement("SR");
                l.appendChild(sr);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    sr.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    sr.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    sr.appendChild(ax);
                    QDomElement se = ARxml.createElement("SE");
                    sr.appendChild(se);
                    QDomElement add = ARxml.createElement("ADD");
                    sr.appendChild(add);
                    QDomElement wd = ARxml.createElement("WorkingDistance");
                    sr.appendChild(wd);
                }
                QDomElement lm = ARxml.createElement("LM");
                l.appendChild(lm);
                {
                    lm.appendChild(ARxml.createElement("Sphere"));
                    lm.appendChild(ARxml.createElement("Cylinder"));
                    lm.appendChild(ARxml.createElement("Axis"));
                    lm.appendChild(ARxml.createElement("ADD"));
                    lm.appendChild(ARxml.createElement("ADD2"));
                }
                QDomElement km = ARxml.createElement("KM");
                l.appendChild(km);
                {
                    QDomElement kmlist = ARxml.createElement("KMList");
                    kmlist.setAttribute("No","1");
                    km.appendChild(kmlist);
                    {
                        QDomElement r1 = ARxml.createElement("R1");
                        kmlist.appendChild(r1);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r1.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r1.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r1.appendChild(ax);
                        }
                        QDomElement r2 = ARxml.createElement("R2");
                        kmlist.appendChild(r2);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r2.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r2.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r2.appendChild(ax);
                        }
                        QDomElement avg = ARxml.createElement("Average");
                        kmlist.appendChild(avg);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            avg.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            avg.appendChild(pwr);
                        }
                        QDomElement kmc = ARxml.createElement("KMCylinder");
                        kmlist.appendChild(kmc);
                        {
                            QDomElement pwr = ARxml.createElement("Power");
                            kmc.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            kmc.appendChild(ax);
                        }
                    }
                    QDomElement kmmed = ARxml.createElement("KMMedian");
                    km.appendChild(kmmed);
                    {
                        QDomElement r1 = ARxml.createElement("R1");
                        kmmed.appendChild(r1);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r1.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r1.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r1.appendChild(ax);
                        }
                        QDomElement r2 = ARxml.createElement("R2");
                        kmmed.appendChild(r2);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            r2.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            r2.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            r2.appendChild(ax);
                        }
                        QDomElement avg = ARxml.createElement("Average");
                        kmmed.appendChild(avg);
                        {
                            QDomElement rd = ARxml.createElement("Radius");
                            avg.appendChild(rd);
                            QDomElement pwr = ARxml.createElement("Power");
                            avg.appendChild(pwr);
                        }
                        QDomElement kmc = ARxml.createElement("KMCylinder");
                        kmmed.appendChild(kmc);
                        {
                            QDomElement pwr = ARxml.createElement("Power");
                            kmc.appendChild(pwr);
                            QDomElement ax = ARxml.createElement("Axis");
                            kmc.appendChild(ax);
                        }
                    }
                }
                QDomElement cs = ARxml.createElement("CS");
                l.appendChild(cs);
                {
                    QDomElement cslist = ARxml.createElement("CSList");
                    cslist.setAttribute("No","1");
                    cs.appendChild(cslist);
                    {
                        QDomElement sz = ARxml.createElement("Size");
                        cslist.appendChild(sz);
                    }
                }
                QDomElement ps = ARxml.createElement("PS");
                l.appendChild(ps);
                {
                    QDomElement pslist = ARxml.createElement("PSList");
                    pslist.setAttribute("No","1");
                    ps.appendChild(pslist);
                    {
                        QDomElement sz = ARxml.createElement("Size");
                        pslist.appendChild(sz);
                        QDomElement lp = ARxml.createElement("Lamp");
                        pslist.appendChild(lp);
                    }
                }
                QDomElement ac = ARxml.createElement("AC");
                l.appendChild(ac);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    ac.appendChild(sph);
                    QDomElement maxps = ARxml.createElement("MaxPS");
                    ac.appendChild(maxps);
                    QDomElement minps = ARxml.createElement("MinPS");
                    ac.appendChild(minps);
                    QDomElement img = ARxml.createElement("AccImage");
                    ac.appendChild(img);
                }
                QDomElement ri = ARxml.createElement("RI");
                l.appendChild(ri);
                {
                    QDomElement coih = ARxml.createElement("COIH");
                    ri.appendChild(coih);
                    QDomElement coia = ARxml.createElement("COIA");
                    ri.appendChild(coia);
                    QDomElement poi = ARxml.createElement("POI");
                    ri.appendChild(poi);
                    QDomElement rimg = ARxml.createElement("RetroImage");
                    ri.appendChild(rimg);
                }
            }
            QDomElement pd = ARxml.createElement("PD");
            Data.appendChild(pd);
            {
                QDomElement pdl = ARxml.createElement("PDList");
                pdl.setAttribute("No","1");
                pd.appendChild(pdl);
                {
                    QDomElement fpd = ARxml.createElement("FarPD");
                    pdl.appendChild(fpd);
                    if (Datas::I()->mesurefronto->ecartIP()>0)
                    {
                        QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->ecartIP()));
                        fpd.appendChild(val);
                    }
                    QDomElement rpd = ARxml.createElement("RPD");
                    pdl.appendChild(rpd);
                    QDomElement lpd = ARxml.createElement("LPD");
                    pdl.appendChild(lpd);
                    QDomElement npd = ARxml.createElement("NearPD");
                    pdl.appendChild(npd);
                }
            }

            /*! Pour le Glasspop, on met un délai de 3 secondes avant l'envoi des datas refracteur sinon il s'emmêle les crayons s'il y a des données données LM */
            if (nameRF =="NIDEK Glasspop" && ExistMesureFronto)
            {
                t_xmlfiletimer.setSingleShot(true);
                t_xmlfiletimer.setInterval(3000);
                t_xmlfiletimer.start();
                connect(&t_xmlfiletimer, &QTimer::timeout, this, [=] { EnregistreFileDatasXML(ARxml, Procedures::MesureAutoref); });
            }
            else
                EnregistreFileDatasXML(ARxml, Procedures::MesureAutoref);
        }
    }
}

bool Procedures::ReglePortTonometre()
{
    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;
    ReinitialiseSerialSettings(s_paramPortSerieTono);
    if (m_settings->value(Param_Poste_PortTono_COM_baudrate)      != QVariant()
     && m_settings->value(Param_Poste_PortTono_COM_databits)      != QVariant()
     && m_settings->value(Param_Poste_PortTono_COM_parity)        != QVariant()
     && m_settings->value(Param_Poste_PortTono_COM_stopBits)      != QVariant()
     && m_settings->value(Param_Poste_PortTono_COM_flowControl)   != QVariant())
    {
        val         = m_settings->value(Param_Poste_PortTono_COM_baudrate).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortTono_COM_databits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortTono_COM_parity).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.parity = (QSerialPort::Parity)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortTono_COM_stopBits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortTono_COM_flowControl).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.flowControl = (QSerialPort::FlowControl)metaEnum.value(val);
    }
    if (a)
    {
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_flowControl,    Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.flowControl));
    }
    else
    {
        m_settings->setValue(Param_Poste_PortTono_COM_baudrate,    QVariant());
        m_settings->setValue(Param_Poste_PortTono_COM_databits,    QVariant());
        m_settings->setValue(Param_Poste_PortTono_COM_parity,      QVariant());
        m_settings->setValue(Param_Poste_PortTono_COM_stopBits,    QVariant());
        m_settings->setValue(Param_Poste_PortTono_COM_flowControl, QVariant());
    }
    return a;
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
    User * usrsuperviseur = Datas::I()->users->getById(usr->idsuperviseur());
    if ( usr->idsuperviseur() == User::ROLE_NON_RENSEIGNE )           // le user est soignant, assistant et travaille pour plusieurs superviseurs
        strSup = tr("tout le monde");
    else if ( usr->idsuperviseur() == User::ROLE_VIDE )               // le user est un administratif
        strSup = tr("sans objet");
    else if ( usr->idsuperviseur() == User::ROLE_INDETERMINE )        // jamais utilisé
        strSup = tr("indéterminé");
    else if ( usrsuperviseur )
        strSup = usrsuperviseur->login();
    str += tr("superviseur") + "\t\t= " + strSup + "\n";

    QString strParent = "";
    User * usrparent = Datas::I()->users->getById(usr->idparent());
    if ( usr->idparent() == User::ROLE_NON_RENSEIGNE )                    // le user est soignant, assistant, travaille pour plusieurs superviseurs
        strParent = tr("sans objet");
    else if ( usr->idparent() == User::ROLE_VIDE )                        // le user est un administratif
        strParent = tr("sans objet");
    else if ( usr->idparent() == User::ROLE_INDETERMINE )                 // jamais utilisé
        strParent = tr("indéterminé");
    else if ( usrparent )
        strParent = usrparent->login();
    str += tr("parent") + "\t\t= " + strParent + "\n";

    QString strComptable = "";
    User * usrcptble = Datas::I()->users->getById(usr->idcomptable());
    if ( usr->idcomptable() == User::ROLE_NON_RENSEIGNE )
        strComptable = tr("sans objet");
    else if ( usr->idcomptable() == User::ROLE_VIDE )
        strComptable = tr("sans objet");
    else if ( usr->idcomptable() == User::ROLE_INDETERMINE )
        strComptable = tr("indéterminé");
    else if (usrcptble)
        strComptable = usrcptble->login();
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

bool Procedures::isUserConnected(User *usr)
{
    if (Datas::I()->postesconnectes->admin(Item::NoUpdate) == Q_NULLPTR)
        Datas::I()->postesconnectes->MAJlistePostesConnectes();
    for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it !=  Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
    {
        PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
        if (post->id() == usr->id())
            return true;
    }
    return false;
}

void Procedures::debugMesure(QObject *mesure, QString titre)
{
    if (titre != "")
        qDebug() << titre;
    Pachymetrie *pachy = qobject_cast<Pachymetrie *>(mesure);
    if (pachy != Q_NULLPTR)
    {
        qDebug() << "Pachymétrie";
        QString Formule = "OD : " + QString::number(pachy->pachyOD()) + "µ";
        qDebug() << Formule;
        Formule = "OG : " + QString::number(pachy->pachyOG()) + "µ";
        qDebug() << Formule;
        qDebug() << Utils::EnumDescription(QMetaEnum::fromType<Pachymetrie::Mode>(), pachy->modemesure());
        return;
    }
    Tonometrie *tono = qobject_cast<Tonometrie *>(mesure);
    if (tono != Q_NULLPTR)
    {
        qDebug() << "Tonométrie";
        QString Formule = "OD : " + QString::number(tono->TOD()) + "mmHg";
        if (tono->TODcorrigee() >0)
            Formule += " - corrigée : " + QString::number(tono->TODcorrigee()) + "mmHg";
        qDebug() << Formule;
        Formule = "OG : " + QString::number(tono->TOG()) + "mmHg";
        if (tono->TOGcorrigee() >0)
            Formule += " - corrigée : " + QString::number(tono->TOGcorrigee()) + "mmHg";
        qDebug() << Formule;
        return;
    }
    Keratometrie *ker = qobject_cast<Keratometrie *>(mesure);
    if (ker != Q_NULLPTR)
    {
        qDebug() << "Keratométrie";
        QString Formule = "OD : " + QString::number(ker->K1OD()) + "/" + QString::number(ker->K2OD()) + " "  + QString::number(ker->axeKOD());
        qDebug() << Formule;
        Formule = "OG : " + QString::number(ker->K1OG()) + "/" + QString::number(ker->K2OG()) + " "  + QString::number(ker->axeKOG());
        qDebug() << Formule;
        return;
    }
    MesureRefraction *ref = qobject_cast<MesureRefraction *>(mesure);
    if (ref != Q_NULLPTR)
    {
        qDebug() << Utils::EnumDescription(QMetaEnum::fromType<Refraction::Mesure>(), ref->typemesure());
        QString Formule = "OD : " + Utils::PrefixePlus(ref->sphereOD());
        if (ref->cylindreOD() != 0)
            Formule += "(" + Utils::PrefixePlus(ref->cylindreOD()) + " à " + QString::number(ref->axecylindreOD()) + "°)";
        if (ref->addVPOD() > 0.0)
            Formule +=  " add." + Utils::PrefixePlus(ref->addVPOD()) +  " VP";
        qDebug() << Formule;
        Formule = "OG : " + Utils::PrefixePlus(ref->sphereOG());
        if (ref->cylindreOG() != 0)
            Formule += "(" + Utils::PrefixePlus(ref->cylindreOG()) + " à " + QString::number(ref->axecylindreOG()) + "°)";
        if (ref->addVPOG() > 0.0)
            Formule +=  " add." + Utils::PrefixePlus(ref->addVPOG()) +  " VP";
        qDebug() << Formule;
        if (ref->ecartIP() > 0)
        {
            Formule = "Ecart interpupillaire : " + QString::number(ref->ecartIP()) +  "mm";
            qDebug() << Formule;
        }
    }
}

void Procedures::EnvoiDataPatientAuRefracteur()
{
    TypesMesures flag = MesureAutoref | MesureFronto | MesureRefracteur;
    RegleRefracteur(flag);
}

QByteArray Procedures::RequestToSendNIDEK()
{
    /*! la séquence SOH "C**" STX "RS" ETB EOT -> Data a envoyer ="\001C**\002RS\017\004" RequestToSendNIDEK() est utilisée dans le système NIDEK pour signifier qu'on veut envoyer des données
     * Si l'appareil cible est OK, il émet la réponse SendDataNIDEK() = "\001CRL\002SD\027\004\r"
     * pour signifier qu'il est prêt à recevoir les données
     * Dans Rufus, cette demande d'envoi est créée à l'ouverture d'un dossier patient et permet de régler le refracteur sur les données de ce patient */
    QByteArray DTSbuff;
    DTSbuff.append(SOH);           //SOH -> start of header
    DTSbuff.append("C**");                              //C**
    DTSbuff.append(STX);           //STX -> start of text
    DTSbuff.append("RS");                               //RS
    DTSbuff.append(ETB);          //ETB -> end of text block  -> fin RTS
    DTSbuff.append(EOT);           //EOT -> end of transmission
    //qDebug() << "RequestToSendNIDEK() = " << QString(DTSbuff).toLocal8Bit();
    return  QString(DTSbuff).toLocal8Bit();
}

QByteArray Procedures::SendDataNIDEK(QString mesure)
{
    /*! la séquence SendData = "\001CRL\002SD\017\004" SendDataNIDEK() est utilisée dans le système NIDEK en réponse à une demande d'envoi de données RequestToSendNIDEK() */
    QByteArray DTRbuff;
    DTRbuff.append(SOH);           //SOH -> start of header
    DTRbuff.append(mesure);                             //CRL pour le refracteur, CLM pour le fronto, CRK ou CRM pour l'autoref
    DTRbuff.append(STX);           //STX -> start of text
    DTRbuff.append("SD");                               //SD
    DTRbuff.append(ETB);          //ETB -> end of text block  -> fin RTS
    DTRbuff.append(EOT);           //EOT -> end of transmission
    QByteArray reponse = QString(DTRbuff).toLocal8Bit();
    reponse += "\r";                                    /*! +++ il faut rajouter \r à la séquence SendDataNIDEK("CRL") sinon ça ne marche pas .... */
    //qDebug() << "SendDataNidek = " << reponse;
    return reponse;
}

void Procedures::LectureDonneesCOMRefracteur(QString Mesure)
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
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();

    // TRADUCTION DES DONNEES EN FONCTION DU REFRACTEUR
    // NIDEK RT-5100 - RT-2100 =======================================================================================================================================
    if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
    {
        /*!
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
        if (Mesure.contains("@LM") && PortFronto() == Q_NULLPTR && !m_LANFronto)             //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur
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
                InsertMesure(MesureFronto);
                emit NouvMesure(MesureFronto);
            }
            delete oldMesureFronto;
        }

        //! Données de l'AUTOREF - REFRACTION et KERATOMETRIE ----------------------------------------------------------------------------------------------
        if (Mesure.contains("@KM") && PortAutoref() == Q_NULLPTR && !m_LANAutoref)                 //!=> il y a une mesure de keratométrie et l'autoref est connecté directement à la box du refraacteur
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
                AxeKOD              = mesureOD.midRef(10,3).toInt();
                Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                if (SectionKerato.contains("DR"))
                {
                    mesureOD        = SectionKerato.mid(SectionKerato.indexOf("DR")+2,10)   .replace(" ","0");
                    Datas::I()->mesurekerato->setdioptriesK1OD(mesureOD.midRef(0,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OD(mesureOD.midRef(5,5).toDouble());
                }
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionKerato.contains("CL"))
            {
                mesureOG            = SectionKerato.mid(SectionKerato.indexOf("CL")+2,13)   .replace(" ","0");
                K1OG                = mesureOG.mid(0,5);
                K2OG                = mesureOG.mid(5,5);
                AxeKOG              = mesureOG.midRef(10,3).toInt();
                Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                if (SectionKerato.contains("DL"))
                {
                    mesureOG        = SectionKerato.mid(SectionKerato.indexOf("DL")+2,10)   .replace(" ","0");
                    Datas::I()->mesurekerato->setdioptriesK1OG(mesureOG.midRef(0,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OG(mesureOG.midRef(5,5).toDouble());
                }
            }
            if (Datas::I()->mesurekerato->isDifferent(oldMesureKerato) && !Datas::I()->mesurekerato->isdataclean())
            {
                InsertMesure(MesureKerato);
                emit NouvMesure(MesureKerato);
            }
            delete oldMesureKerato;
        }
        if (Mesure.contains("@RM") && PortAutoref() == Q_NULLPTR && !m_LANAutoref)                 //!=> il y a une mesure de refractometrie et l'autoref est directement branché sur la box du refracteur
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
                InsertMesure(MesureAutoref);
                emit NouvMesure(MesureAutoref);
            }
            delete oldMesureAutoref;
        }

        //! Données du REFRACTEUR --------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@RT"))                 //=> il y a une mesure de refraction
        {
            idx                         = Mesure.indexOf("@RT");
            QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);
            //qDebug() << "Procedures::LectureDonneesRefracteur(QString Mesure) - SectionRefracteur = " << SectionRefracteur;

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
                    Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
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
                    Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
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
                    Datas::I()->mesurefinal->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
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
                    Datas::I()->mesurefinal->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                    Datas::I()->mesurefinal->setaddVPOG(mAddOG.toDouble());
                    Datas::I()->mesurefinal->setavlOG(AVLOG);
                    Datas::I()->mesurefinal->setavpOG(AVPOG);
                }
            }
        }

        //! Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
        if (Mesure.contains("@NT") && PortAutoref() == Q_NULLPTR &&  !m_LANAutoref )                 //!=> il y a une mesure de tonometrie et l'autoref est branché sur la box du refracteur
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
            InsertMesure(MesureTono);                     //! depuis LectureDonneesRefracteur(QString Mesure)
            emit NouvMesure(MesureTono);
        }
        //debugMesure(Datas::I()->mesurekerato, "Procedures::LectureDonneesRefracteur(QString Mesure)");
    }
    // FIN NIDEK RT-5100 et RT 2100 ==========================================================================================================================

    // TOMEY TAP-2000 et Rodenstock Phoromat 2000 =======================================================================================================================================
    else if (nameRF =="TOMEY TAP-2000"|| nameRF =="RODENSTOCK Phoromat 2000")
    {
        /*! SORTIE EXEMPLE POUR UN PHOROMAT RODENSTOCK
         * SOH =    QByteArray::fromHex("1")            //SOH -> start of header
         * STX =    QByteArray::fromHex("2")            //STX -> start of text
         * ETB =    QByteArray::fromHex("17")           //ETB -> end of text block
         * EOT =    QByteArray::fromHex("4")            //EOT -> end of transmission
         * La 1ere et la dernière lignes commencent par SOH et se termine par EOT - représentés ici
         * Les autres lignes commencent par STX et se terminent par ETB
         * Distinguish the FAR mode as a capital letter(UN, LM, S, C, A..) and NEAR mode as a small letter(un, lm, s, c, a..).


SOH*PC_RCV_SEOT                 -> start block
*Phoromat 2000|000000001|0      -> id material
*PD|32.0|32.0|                  ->PD | left PD result | right PD result |
*WD|40|                         ->???
*UN                             -> AV sans correction
*VA|0.2|0.4|0.5|
*LM                             -> Fronto
*SP| -0.25| -0.25|              -> Sphere | left result | right result|
*CY| -0.25| -0.25|              -> Cylindre | left result | right result|
*AX|135|135|                    -> Axe | left result | right result|
*AD| 1.50| 1.50|                -> Addition | left result | right result|
*VA|0.2|0.4|0.5|                -> ???
*PH|O| 0.50|O| 0.50|            -> Prisme horizontal
*PV|U| 0.50|D| 0.50|            -> Prisme vertical
*AR                             -> Autoref
*SP| 0.25| 0.25|
*CY| -4.25| -4.25|
*AX| 45| 45|
*AD| 1.60| 1.70|
*VA|0.2|0.4|0.5|
*PH|O| 1.00|O| 1.00|
*PV|U| 1.50|D| 1.50|
*SJ                             -> Refraction subjective
*SP| 0.50| 0.50|
*CY| -0.25| -0.25|
*AX|135|135|
*AD| 1.50| 1.40|
*VA|0.2|0.4|0.5|
*PH|O| 0.50|O| 0.50|
*PV|U| 0.50|D| 0.50|
*FN                             -> Refraction finale
*SP| -0.50| -0.50|
*CY| -5.00| -5.00|
*AX| 30| 30|
*AD| 1.70| 1.50|
*VA|0.2|0.4|0.5|
*PH|O| 0.50|O| 0.50|
*PV|D| 1.00|U| 1.00|
*KM|R1|L| 8.04| 42.00| 85|      -> Keratometrie | R1 | Side | mm | Diopter | Axis |
*KM|R2|L| 7.74| 43.50|175|      -> Keratometrie | R2 | Side | mm | Diopter | Axis |
*KM|R1|R| 8.04| 42.00| 85|      -> Keratometrie | R1 | Side | mm | Diopter | Axis |
*KM|R2|R| 7.74| 43.50|175|      -> Keratometrie | R2 | Side | mm | Diopter | Axis |
*AV                             -> Advanced data (totally unexplained in user manual...)
*NPC|-8| 9.5| 64.8|
*NPA|B|33| 3.03|
*NPA|L|33| 3.03|
*NPA|R|33| 3.03|
*NRA|B| 2.00| 1.75|
*PRA|B| 3.50| 2.50|
*DIV|f| 9.00|16.00|10.00|
*DIV|n| 3.00| 5.25| 8.00|
*CON|f| 7.00|20.00|15.00|
*CON|n| 2.00| 4.10| 6.00|
*TIME|2012/10/06 04:06:58       -> time date
SOH*PC_RCV_EEOT                 -> end block
*/

        QString mSphere     = "";
        QString mCyl        = "";
        QString mAxe        = "";
        QString mAdd        = "";
        QString AVL         = "";

        //! Ecart interpupillaire ---------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("*PD"))                 //=> il y a une mesure d'écart
        {
            /*! PD|32.0|32.0|                  ->PD | left PD result | right PD result | */
            idx                         = Mesure.indexOf("@RT");
            QString SectionEIP          = Mesure.right(Mesure.length()-idx);
            PD                          = SectionEIP.mid(SectionEIP.indexOf("PD")+3,9);
            QStringList PDlist          = PD.split("|");
            Datas::I()->mesureacuite->setecartIP(static_cast<int>(PDlist.at(0).toDouble() + PDlist.at(0).toDouble()));
        }

        //! Données du FRONTO ---------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("*LM") && PortFronto() == Q_NULLPTR && !m_LANFronto)        //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur
        {
            /*!
            *LM
            *SP| -0.25| -0.25|
            *CY| -0.25| -0.25|
            *AX|135|135|
            *AD| 1.50| 1.50|
            *VA|0.2|0.4|0.5|
            *PH|O| 0.50|O| 0.50|
            *PV|U| 0.50|D| 0.50|
            */

            MesureRefraction        *oldMesureFronto = new MesureRefraction();
            oldMesureFronto         ->setdatas(Datas::I()->mesurefronto);
            Datas::I()->mesurefronto->cleandatas();
            idx                     = Mesure.indexOf("*LM");
            QString SectionFronto   = Mesure.right(Mesure.length()-idx);
            //Edit(SectionFronto + "\nOK");
            // SPHERE -----------------------------------------------------------------------------
            if (SectionFronto.contains("*SP"))
            {
                mSphere     = SectionFronto.mid(SectionFronto.indexOf("SP")+3,13)   .replace(" ","");
                mSphereOD   = mSphere.split("|").at(0);
                mSphereOG   = mSphere.split("|").at(1);
                Datas::I()->mesurefronto->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                Datas::I()->mesurefronto->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
            }
            // CYLINDRE ---------------------------------------------------------------------------
            if (SectionFronto.contains("*CY"))
            {
                mCyl     = SectionFronto.mid(SectionFronto.indexOf("CY")+3,13)   .replace(" ","");
                mCylOD   = mCyl.split("|").at(0);
                mCylOG   = mCyl.split("|").at(1);
                Datas::I()->mesurefronto->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                Datas::I()->mesurefronto->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
            }
            // AXE ---------------------------------------------------------------------------
            if (SectionFronto.contains("*AX"))
            {
                mAxe     = SectionFronto.mid(SectionFronto.indexOf("AX")+3,7)   .replace(" ","");
                mAxeOD   = mAxe.split("|").at(0);
                mAxeOG   = mAxe.split("|").at(1);
                Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            }
            // ADD ---------------------------------------------------------------------------
            if (SectionFronto.contains("*AD"))
            {
                mAdd     = SectionFronto.mid(SectionFronto.indexOf("AD")+3,11)   .replace(" ","");
                mAddOD   = mAdd.split("|").at(0);
                mAddOG   = mAdd.split("|").at(1);
                Datas::I()->mesurefronto->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                Datas::I()->mesurefronto->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
            }
            //debugMesureRefraction(Datas::I()->mesurefronto);
            if (Datas::I()->mesurefronto->isDifferent(oldMesureFronto) && !Datas::I()->mesurefronto->isdataclean())
            {
                InsertMesure(MesureFronto);
                emit NouvMesure(MesureFronto);
            }
            delete oldMesureFronto;
        }

        //! Données de l'AUTOREF ----------------------------------------------------------------------------------------------
        if (Mesure.contains("*AR") && PortAutoref() == Q_NULLPTR && !m_LANAutoref)      //!=> il y a une mesure de refractometrie et l'autoref est directement branché sur la box du refracteur
        {
            /*!
            *AR
            *SP| 0.25| 0.25|
            *CY| -4.25| -4.25|
            *AX| 45| 45|
            *AD| 1.60| 1.70|
            *VA|0.2|0.4|0.5|
            *PH|O| 1.00|O| 1.00|
            *PV|U| 1.50|D| 1.50|
            */
            MesureRefraction        *oldMesureAutoref = new MesureRefraction();
            oldMesureAutoref        ->setdatas(Datas::I()->mesureautoref);
            Datas::I()->mesureautoref->cleandatas();
            idx                     = Mesure.indexOf("*AR");
            QString SectionAutoref  = Mesure.right(Mesure.length()-idx);
            //Edit(SectionAutoref + "\nOK");
            // SPHERE -----------------------------------------------------------------------------
            if (SectionAutoref.contains("*SP"))
            {
                mSphere     = SectionAutoref.mid(SectionAutoref.indexOf("SP")+3,13)   .replace(" ","");
                mSphereOD   = mSphere.split("|").at(0);
                mSphereOG   = mSphere.split("|").at(1);
                Datas::I()->mesureautoref->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                Datas::I()->mesureautoref->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
            }
            // CYLINDRE ---------------------------------------------------------------------------
            if (SectionAutoref.contains("*CY"))
            {
                mCyl     = SectionAutoref.mid(SectionAutoref.indexOf("CY")+3,13)   .replace(" ","");
                mCylOD   = mCyl.split("|").at(0);
                mCylOG   = mCyl.split("|").at(1);
                Datas::I()->mesureautoref->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                Datas::I()->mesureautoref->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
            }
            // AXE ---------------------------------------------------------------------------
            if (SectionAutoref.contains("*AX"))
            {
                mAxe     = SectionAutoref.mid(SectionAutoref.indexOf("AX")+3,7)   .replace(" ","");
                mAxeOD   = mAxe.split("|").at(0);
                mAxeOG   = mAxe.split("|").at(1);
                Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            }
            // ADD ---------------------------------------------------------------------------
            if (SectionAutoref.contains("*AD"))
            {
                mAdd     = SectionAutoref.mid(SectionAutoref.indexOf("AD")+3,11)   .replace(" ","");
                mAddOD   = mAdd.split("|").at(0);
                mAddOG   = mAdd.split("|").at(1);
                Datas::I()->mesureautoref->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                Datas::I()->mesureautoref->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
            }
            //debugMesureRefraction(Datas::I()->mesureautoref);
            if (Datas::I()->mesureautoref->isDifferent(oldMesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
            {
                InsertMesure(MesureAutoref);
                emit NouvMesure(MesureAutoref);
            }
            delete oldMesureAutoref;
        }

        //! Données du REFRACTEUR --------------------------------------------------------------------------------------------------------------------
        if (Mesure.contains("*SJ") || Mesure.contains("*FN"))                           //!=> il y a une mesure de refraction
        {
            //qDebug() << "Procedures::LectureDonneesRefracteur(QString Mesure) - SectionRefracteur = " << SectionRefracteur;

            // les données subjectives --------------------------------------------------------------------------------------------------------------
            if (Mesure.contains("*SJ"))
            {
                /*!
                *SJ
                *SP| 0.50| 0.50|
                *CY| -0.25| -0.25|
                *AX|135|135|
                *AD| 1.50| 1.40|
                *VA|0.2|0.4|0.5|
                *PH|O| 0.50|O| 0.50|
                *PV|U| 0.50|D| 0.50|
                */
                idx                         = Mesure.indexOf("*SJ");
                QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);
                if (SectionRefracteur.contains("*SP"))
                {
                    mSphere     = SectionRefracteur.mid(SectionRefracteur.indexOf("SP")+3,13)   .replace(" ","");
                    mSphereOD   = mSphere.split("|").at(0);
                    mSphereOG   = mSphere.split("|").at(1);
                    Datas::I()->mesureacuite->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                    Datas::I()->mesureacuite->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
                }
                // CYLINDRE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*CY"))
                {
                    mCyl     = SectionRefracteur.mid(SectionRefracteur.indexOf("CY")+3,13)   .replace(" ","");
                    mCylOD   = mCyl.split("|").at(0);
                    mCylOG   = mCyl.split("|").at(1);
                    Datas::I()->mesureacuite->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                    Datas::I()->mesureacuite->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
                }
                // AXE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AX"))
                {
                    mAxe     = SectionRefracteur.mid(SectionRefracteur.indexOf("AX")+3,7)   .replace(" ","");
                    mAxeOD   = mAxe.split("|").at(0);
                    mAxeOG   = mAxe.split("|").at(1);
                    Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                    Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                }
                // ADD ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AD"))
                {
                    mAdd     = SectionRefracteur.mid(SectionRefracteur.indexOf("AD")+3,11)   .replace(" ","");
                    mAddOD   = mAdd.split("|").at(0);
                    mAddOG   = mAdd.split("|").at(1);
                    Datas::I()->mesureacuite->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                    Datas::I()->mesureacuite->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
                }
                // ACUITE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*VA"))
                {
                    AVL     = SectionRefracteur.mid(SectionRefracteur.indexOf("VA")+3,11);
                    AVLOD   = AVL.split("|").at(0);
                    AVLOG   = AVL.split("|").at(1);
                    Datas::I()->mesureacuite->setavlOD(AVLOD);
                    Datas::I()->mesureacuite->setavlOG(AVLOG);
                }
            }

            // les données finales --------------------------------------------------------------------------------------------------------------
            if (Mesure.contains("*FN"))
            {
                /*!
                *FN
                *SP| -0.50| -0.50|
                *CY| -5.00| -5.00|
                *AX| 30| 30|
                *AD| 1.70| 1.50|
                *VA|0.2|0.4|0.5|
                *PH|O| 0.50|O| 0.50|
                *PV|D| 1.00|U| 1.00|
                */
                idx                         = Mesure.indexOf("*FN");
                QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);
                if (SectionRefracteur.contains("*SP"))
                {
                    mSphere     = SectionRefracteur.mid(SectionRefracteur.indexOf("SP")+3,13)   .replace(" ","");
                    mSphereOD   = mSphere.split("|").at(0);
                    mSphereOG   = mSphere.split("|").at(1);
                    Datas::I()->mesurefinal->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                    Datas::I()->mesurefinal->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
                }
                // CYLINDRE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*CY"))
                {
                    mCyl     = SectionRefracteur.mid(SectionRefracteur.indexOf("CY")+3,13)   .replace(" ","");
                    mCylOD   = mCyl.split("|").at(0);
                    mCylOG   = mCyl.split("|").at(1);
                    Datas::I()->mesurefinal->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                    Datas::I()->mesurefinal->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
                }
                // AXE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AX"))
                {
                    mAxe     = SectionRefracteur.mid(SectionRefracteur.indexOf("AX")+3,7)   .replace(" ","");
                    mAxeOD   = mAxe.split("|").at(0);
                    mAxeOG   = mAxe.split("|").at(1);
                    Datas::I()->mesurefinal->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                    Datas::I()->mesurefinal->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                }
                // ADD ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AD"))
                {
                    mAdd     = SectionRefracteur.mid(SectionRefracteur.indexOf("AD")+3,11)   .replace(" ","");
                    mAddOD   = mAdd.split("|").at(0);
                    mAddOG   = mAdd.split("|").at(1);
                    Datas::I()->mesurefinal->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                    Datas::I()->mesurefinal->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
                }
                // ACUITE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*VA"))
                {
                    AVL     = SectionRefracteur.mid(SectionRefracteur.indexOf("VA")+3,11);
                    AVLOD   = AVL.split("|").at(0);
                    AVLOG   = AVL.split("|").at(1);
                    Datas::I()->mesurefinal->setavlOD(AVLOD);
                    Datas::I()->mesurefinal->setavlOG(AVLOG);
                }
            }
        }
    }
    // FIN TOMEY TAP-2000 et Rodenstock Phoromat 2000 ==========================================================================================================================
}

void Procedures::LectureDonneesXMLRefracteur(QDomDocument docxml)
{
    /*! exemple de fichier xml pour un RT-6100
     *
     *
<?xml version="1.0" encoding="UTF-16"?>
<?xml-stylesheet type="text/xsl" href="NIDEK_RT_Stylesheet.xsl"?>
<Ophthalmology>
  <Common>
    <Company>NIDEK</Company>
    <ModelName>RT-6100</ModelName>
    <MachineNo>360008</MachineNo>
    <ROMVersion>1.1.4</ROMVersion>
    <Version>NIDEK_RT_V1.00</Version>
    <Date>2021-10-26</Date>
    <Time>17-16-48</Time>
    <Patient>
    <No>0138</No>
    <ID></ID>
      <FirstName/>
      <MiddleName/>
      <LastName/>
      <Sex/>
    <Age>40</Age>
      <DOB/>
      <NameJ1/>
      <NameJ2/>
    </Patient>
  </Common>
  <Measure Type="RT">
    <Phoropter>
      <DiopterStep unit="D"/>
      <AxisStep unit="deg"/>
      <CylinderMode>-</CylinderMode>
      <Corrected CorrectionType="LM_Base" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <SatisfactionLevel MaxLevel="5">4</SatisfactionLevel>
        <Satisfaction>Yes</Satisfaction>
        <R>
          <Sphere unit="D">1.00</Sphere>
          <Cylinder unit="D">-0.50</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm"/>
        </R>
        <L>
          <Sphere unit="D">1.50</Sphere>
          <Cylinder unit="D">-0.75</Cylinder>
          <Axis unit="deg">80</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>0.90</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm"/>
        </L>
        <B>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm"/>
        </B>
      </Corrected>
      <Corrected CorrectionType="REF_Base" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <R>
          <Sphere unit="D">0.50</Sphere>
          <Cylinder unit="D">-1.00</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </R>
        <L>
          <Sphere unit="D">1.00</Sphere>
          <Cylinder unit="D">-1.25</Cylinder>
          <Axis unit="deg">80</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </L>
        <B>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">61.00</PD>
        </B>
      </Corrected>
      <Corrected CorrectionType="Full" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <R>
          <Sphere unit="D">1.25</Sphere>
          <Cylinder unit="D">-1.00</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </R>
        <L>
          <Sphere unit="D">1.50</Sphere>
          <Cylinder unit="D">-1.25</Cylinder>
          <Axis unit="deg">85</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </L>
        <B>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">61.00</PD>
        </B>
      </Corrected>
      <Corrected CorrectionType="Best" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <R>
          <Sphere unit="D">0.75</Sphere>
          <Cylinder unit="D">-0.50</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit= "D">0.25</ADD>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </R>
        <L>
          <Sphere unit="D">1.50</Sphere>
          <Cylinder unit="D">-1.25</Cylinder>
          <Axis unit="deg">85</Axis>
          <ADD unit= "D">0.25</ADD>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </L>
        <B>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">61.00</PD>
        </B>
      </Corrected>
      <Other/>
    </Phoropter>
    <The21pointExams>
      <No07>
        <R>
        <Sphere unit="D">0.75</Sphere>
        <Cylinder unit="D">-0.50</Cylinder>
        <Axis unit="deg">90</Axis>
        </R>
        <L>
        <Sphere unit="D">1.50</Sphere>
        <Cylinder unit="D">-1.25</Cylinder>
        <Axis unit="deg">85</Axis>
        </L>
      </No07>
    </The21pointExams>
    <KM>
      <ReferenceInstance></ReferenceInstance>
      <DiopterStep Unit="D">0.25D</DiopterStep>
      <AxisStep Unit="deg">5°</AxisStep>
      <CylinderMode>-</CylinderMode>
      <R>
        <Median>
          <R1>
            <Radius>7.69</Radius>
            <Power>44.00</Power>
            <Axis>86</Axis>
          </R1>
          <R2>
            <Radius>7.58</Radius>
            <Power>44.50</Power>
            <Axis>176</Axis>
          </R2>
        </Median>
      </R>
      <L>
        <Median>
          <R1>
            <Radius>7.69</Radius>
            <Power>44.00</Power>
            <Axis>95</Axis>
          </R1>
          <R2>
            <Radius>7.53</Radius>
            <Power>44.75</Power>
            <Axis>5</Axis>
          </R2>
        </Median>
      </L>
    </KM>
    <TM>
      <ReferenceInstance></ReferenceInstance>
      <R>
        <Average>
          <IOP_mmHg unit="mmHg">12.20</IOP_mmHg>
          <IOP_Pa unit="kPa">1.61</IOP_Pa>
        </Average>
      </R>
      <L>
        <Average>
          <IOP_mmHg unit="mmHg">12.20</IOP_mmHg>
          <IOP_Pa unit="kPa">1.60</IOP_Pa>
        </Average>
      </L>
    </TM>
    <PACHY>
      <ReferenceInstance></ReferenceInstance>
      <R>
        <Average>
          <Thickness>581</Thickness>
        </Average>
      </R>
      <L>
        <Average>
          <Thickness>590</Thickness>
        </Average>
      </L>
    </PACHY>
    <Test_Time unit="sec">5</Test_Time>
    <SystemNo>0</SystemNo>
  </Measure>
</Ophthalmology>

*/
    Logs::LogToFile("MesuresRefracteur.txt", docxml.toByteArray());
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
    QString PDD(""), PDG("");

    QString mesureOD(""), mesureOG("");
    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    QString mTOOD(""), mTOOG("");
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();

    // TRADUCTION DES DONNEES EN FONCTION DU REFRACTEUR
    // NIDEK RT-6100 - Glasspop =======================================================================================================================================
    if (nameRF =="NIDEK RT-6100" || nameRF =="NIDEK Glasspop")
    {
        QDomElement xml = docxml.documentElement();

        for (int i=0; i<xml.childNodes().size(); i++)
        {
            QDomElement childnode = xml.childNodes().at(i).toElement();
            if (childnode.tagName() == "Measure")
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement childnodemeasure = childnode.childNodes().at(j).toElement();
                    // On essaie de récupérer les mesures de fronto, autoref, refractions subjective et fianle
                    if (childnodemeasure.tagName() == "Phoropter")
                    {
                        /*! On a 4 tags Corrected à récupérer avec 3 attributs pou chacun
                         * le premier attribut ("CorrectionType") peut être
                            * LM_Base pour fronto
                            * REF_Base pour autoref
                            * Full pour acuité subjective
                            * Best pour final
                         * Les 2 autres attributs sont
                            * La distance ("Vision"): Distant ou Near (mais tout est dans Distant, on ne prend pas Near ???)
                            * L'ambiance lumineuse de mesure ("Situation"): Standard ou Night - on ne prend que Standard
                         */

                        bool OKMesureFronto = false;
                        bool OKMesureAutoref = false;
                        bool OKMesureAcuite = false;
                        bool OKMesureFinal = false;
                        QDomElement childnodephoropter = childnodemeasure;
                        for (int k=0; k<childnodephoropter.childNodes().size(); k++)
                        {
                            QDomElement childnodephoropterit = childnodemeasure.childNodes().at(k).toElement();
                            int a = childnodephoropterit.attributes().count();
                            if (a > 0)
                            {
                                QStringList listattributesnames, listattributesvalues;
                                for (int b=0;b<a;b++)
                                {
                                    listattributesnames     << childnodephoropterit.attributes().item(b).toAttr().name();
                                    listattributesvalues    << childnodephoropterit.attributes().item(b).toAttr().value();
                                }

            //! On essaie de récupérer une mesure Fronto
                                OKMesureFronto =   listattributesnames.contains("Vision")
                                                        && listattributesnames.contains("CorrectionType")
                                                        && listattributesnames.contains("Situation")
                                                        && listattributesvalues.contains("Distant")
                                                        && listattributesvalues.contains("LM_Base")
                                                        && listattributesvalues.contains("Standard");
                                if (OKMesureFronto && PortFronto() == Q_NULLPTR && !m_LANFronto)             //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur)
                                {
                                    //qDebug() << "OK Fronto" << childnodephoropter.tagName() << " - " << childnodephoropter.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                    MesureRefraction        *oldMesureFronto = new MesureRefraction();
                                    oldMesureFronto         ->setdatas(Datas::I()->mesurefronto);
                                    Datas::I()->mesurefronto->cleandatas();
                                    for (int frit=0; frit<childnodephoropterit.childNodes().size(); frit++)
                                    {
                                        QDomElement childnodefronto = childnodephoropterit.childNodes().at(frit).toElement();
                                        // OEIL DROIT -----------------------------------------------------------------------------
                                        if (childnodefronto.tagName() == "R")
                                        {
                                            for (int frR=0; frR<childnodefronto.childNodes().size(); frR++)
                                            {
                                                QDomElement childnodeitem = childnodefronto.childNodes().at(frR).toElement();
                                                if (childnodeitem.tagName() == "Sphere")
                                                    mSphereOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Cylinder")
                                                    mCylOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Axis")
                                                    mAxeOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "ADD")
                                                    mAddOD = childnodeitem.text();
                                            }
                                            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
                                            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
                                            Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
                                        }
                                        // OEIL GAUCHE -----------------------------------------------------------------------------
                                        if (childnodefronto.tagName() == "L")
                                        {
                                            for (int frG=0; frG<childnodefronto.childNodes().size(); frG++)
                                            {
                                                QDomElement childnodeitem = childnodefronto.childNodes().at(frG).toElement();
                                                if (childnodeitem.tagName() == "Sphere")
                                                    mSphereOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Cylinder")
                                                    mCylOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Axis")
                                                    mAxeOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "ADD")
                                                    mAddOG = childnodeitem.text();
                                            }
                                            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
                                            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
                                            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
                                        }
                                        // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                        if (childnodefronto.tagName() == "B")
                                            for (int frEIP=0; frEIP<childnodefronto.childNodes().size(); frEIP++)
                                            {
                                                QDomElement childnodeitem = childnodefronto.childNodes().at(frEIP).toElement();
                                                if (childnodeitem.tagName() == "PD")
                                                {
                                                    PD = childnodeitem.text();
                                                    Datas::I()->mesurefronto->setecartIP(PD.toInt());
                                                }
                                            }
                                    }
                                    if (Datas::I()->mesurefronto->isDifferent(oldMesureFronto) && !Datas::I()->mesurefronto->isdataclean())
                                    {
                                        InsertMesure(MesureFronto);
                                        emit NouvMesure(MesureFronto);
                                    }
                                    delete oldMesureFronto;
                                    if (!OKMesureFronto)
                                        qDebug() << "pas OK Fronto";
                                }

            //! On essaie de récupérer une mesure Autoref
                                OKMesureAutoref =   listattributesnames.contains("Vision")
                                            && listattributesnames.contains("CorrectionType")
                                            && listattributesnames.contains("Situation")
                                            && listattributesvalues.contains("Distant")
                                            && listattributesvalues.contains("REF_Base")
                                            && listattributesvalues.contains("Standard");
                                if (OKMesureAutoref && PortAutoref() == Q_NULLPTR && !m_LANAutoref)             //!=> il y a une mesure pour l'autoref et l'autoref est directement branché sur la box du refracteur)
                                    {
                                        //qDebug() << "OK Autoref" << childnodephoropterit.tagName() << " - " << childnodephoropterit.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                        MesureRefraction        *oldMesureAutoref = new MesureRefraction();
                                        oldMesureAutoref         ->setdatas(Datas::I()->mesureautoref);
                                        Datas::I()->mesureautoref->cleandatas();
                                        for (int arfit=0; arfit<childnodephoropterit.childNodes().size(); arfit++)
                                        {
                                            QDomElement childnodeautoref = childnodephoropterit.childNodes().at(arfit).toElement();
                                            // OEIL DROIT -----------------------------------------------------------------------------
                                            if (childnodeautoref.tagName() == "R")
                                            {
                                                for (int arfR=0; arfR<childnodeautoref.childNodes().size(); arfR++)
                                                {
                                                    QDomElement childnodeitem = childnodeautoref.childNodes().at(arfR).toElement();
                                                    if (childnodeitem.tagName() == "Sphere")
                                                        mSphereOD = childnodeitem.text();
                                                    if (childnodeitem.tagName() == "Cylinder")
                                                        mCylOD = childnodeitem.text();
                                                    if (childnodeitem.tagName() == "Axis")
                                                        mAxeOD = childnodeitem.text();
                                                    if (childnodeitem.tagName() == "ADD")
                                                        mAddOD = childnodeitem.text();
                                                }
                                                Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
                                                Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
                                                Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                                Datas::I()->mesureautoref->setaddVPOD(mAddOD.toDouble());
                                            }
                                            // OEIL GAUCHE -----------------------------------------------------------------------------
                                            if (childnodeautoref.tagName() == "L")
                                            {
                                                for (int frG=0; frG<childnodeautoref.childNodes().size(); frG++)
                                                {
                                                    QDomElement childnodeitem = childnodeautoref.childNodes().at(frG).toElement();
                                                    if (childnodeitem.tagName() == "Sphere")
                                                        mSphereOG = childnodeitem.text();
                                                    if (childnodeitem.tagName() == "Cylinder")
                                                        mCylOG = childnodeitem.text();
                                                    if (childnodeitem.tagName() == "Axis")
                                                        mAxeOG = childnodeitem.text();
                                                    if (childnodeitem.tagName() == "ADD")
                                                        mAddOG = childnodeitem.text();
                                                }
                                                Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
                                                Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
                                                Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                                Datas::I()->mesureautoref->setaddVPOG(mAddOG.toDouble());
                                            }
                                            // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                            if (childnodeautoref.tagName() == "B")
                                                for (int frEIP=0; frEIP<childnodeautoref.childNodes().size(); frEIP++)
                                                {
                                                    QDomElement childnodeitem = childnodeautoref.childNodes().at(frEIP).toElement();
                                                    if (childnodeitem.tagName() == "PD")
                                                    {
                                                        PD = childnodeitem.text();
                                                        Datas::I()->mesureautoref->setecartIP(PD.toInt());
                                                    }
                                                }
                                        }
                                        if (Datas::I()->mesureautoref->isDifferent(oldMesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
                                        {
                                            InsertMesure(MesureAutoref);
                                            emit NouvMesure(MesureAutoref);
                                        }
                                        delete oldMesureAutoref;
                                        if (!OKMesureAutoref)
                                            qDebug() << "pas OK Autoref";
                                    }

            //! On récupère la refraction subjective
                                OKMesureAcuite =   listattributesnames.contains("Vision")
                                                        && listattributesnames.contains("CorrectionType")
                                                        && listattributesnames.contains("Situation")
                                                        && listattributesvalues.contains("Distant")
                                                        && listattributesvalues.contains("Full")
                                                        && listattributesvalues.contains("Standard");
                                if (OKMesureAcuite)
                                {
                                   //qDebug() << "OK Acuite" << childnodephoropter.tagName() << " - " << childnodephoropter.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                   Datas::I()->mesureacuite->cleandatas();
                                    for (int AVit=0; AVit<childnodephoropterit.childNodes().size(); AVit++)
                                    {
                                        QDomElement childnodeAV = childnodephoropterit.childNodes().at(AVit).toElement();
                                        // OEIL DROIT -----------------------------------------------------------------------------
                                        if (childnodeAV.tagName() == "R")
                                            for (int AVR=0; AVR<childnodeAV.childNodes().size(); AVR++)
                                            {
                                                QDomElement childnodeitem = childnodeAV.childNodes().at(AVR).toElement();
                                                if (childnodeitem.tagName() == "Sphere")
                                                    mSphereOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Cylinder")
                                                    mCylOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Axis")
                                                    mAxeOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "ADD")
                                                    mAddOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "VA")
                                                    AVLOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "PD")
                                                    PDD = childnodeitem.text();
                                           }
                                        // OEIL GAUCHE -----------------------------------------------------------------------------
                                        if (childnodeAV.tagName() == "L")
                                            for (int frG=0; frG<childnodeAV.childNodes().size(); frG++)
                                            {
                                                QDomElement childnodeitem = childnodeAV.childNodes().at(frG).toElement();
                                                if (childnodeitem.tagName() == "Sphere")
                                                    mSphereOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Cylinder")
                                                    mCylOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Axis")
                                                    mAxeOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "ADD")
                                                    mAddOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "VA")
                                                    AVLOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "PD")
                                                    PDG = childnodeitem.text();
                                           }
                                        // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                    }
                                    //qDebug() << PDG << PDD;
                                    Datas::I()->mesureacuite->setsphereOD(mSphereOD.toDouble());
                                    Datas::I()->mesureacuite->setcylindreOD(mCylOD.toDouble());
                                    Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                    Datas::I()->mesureacuite->setaddVPOD(mAddOD.toDouble());
                                    Datas::I()->mesureacuite->setavlOD(AVLOD);
                                    Datas::I()->mesureacuite->setsphereOG(mSphereOG.toDouble());
                                    Datas::I()->mesureacuite->setcylindreOG(mCylOG.toDouble());
                                    Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                    Datas::I()->mesureacuite->setaddVPOG(mAddOG.toDouble());
                                    Datas::I()->mesureacuite->setavlOG(AVLOG);
                                    Datas::I()->mesureacuite->setecartIP(static_cast<int>(std::round(PDD.toDouble() + PDG.toDouble())));
                                }

            //! On récupère la refraction Finale
                                OKMesureFinal =   listattributesnames.contains("Vision")
                                                        && listattributesnames.contains("CorrectionType")
                                                        && listattributesnames.contains("Situation")
                                                        && listattributesvalues.contains("Distant")
                                                        && listattributesvalues.contains("Best")
                                                        && listattributesvalues.contains("Standard");
                                if (OKMesureFinal)
                                {
                                    PDD = ""; PDG = "";
                                    //qDebug() << "OK Final" << childnodephoropter.tagName() << " - " << childnodephoropter.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                    Datas::I()->mesurefinal->cleandatas();
                                    for (int AVit=0; AVit<childnodephoropterit.childNodes().size(); AVit++)
                                    {
                                        QDomElement childnodeAV = childnodephoropterit.childNodes().at(AVit).toElement();
                                        // OEIL DROIT -----------------------------------------------------------------------------
                                        if (childnodeAV.tagName() == "R")
                                            for (int AVR=0; AVR<childnodeAV.childNodes().size(); AVR++)
                                            {
                                                QDomElement childnodeitem = childnodeAV.childNodes().at(AVR).toElement();
                                                if (childnodeitem.tagName() == "Sphere")
                                                    mSphereOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Cylinder")
                                                    mCylOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Axis")
                                                    mAxeOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "ADD")
                                                    mAddOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "VA")
                                                    AVLOD = childnodeitem.text();
                                                if (childnodeitem.tagName() == "PD")
                                                    PDD = childnodeitem.text();
                                           }
                                        // OEIL GAUCHE -----------------------------------------------------------------------------
                                        if (childnodeAV.tagName() == "L")
                                            for (int frG=0; frG<childnodeAV.childNodes().size(); frG++)
                                            {
                                                QDomElement childnodeitem = childnodeAV.childNodes().at(frG).toElement();
                                                if (childnodeitem.tagName() == "Sphere")
                                                    mSphereOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Cylinder")
                                                    mCylOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "Axis")
                                                    mAxeOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "ADD")
                                                    mAddOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "VA")
                                                    AVLOG = childnodeitem.text();
                                                if (childnodeitem.tagName() == "PD")
                                                    PDG = childnodeitem.text();
                                           }
                                        // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                    }
                                    Datas::I()->mesurefinal->setsphereOD(mSphereOD.toDouble());
                                    Datas::I()->mesurefinal->setcylindreOD(mCylOD.toDouble());
                                    Datas::I()->mesurefinal->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                    Datas::I()->mesurefinal->setaddVPOD(mAddOD.toDouble());
                                    Datas::I()->mesurefinal->setavlOD(AVLOD);
                                    Datas::I()->mesurefinal->setsphereOG(mSphereOG.toDouble());
                                    Datas::I()->mesurefinal->setcylindreOG(mCylOG.toDouble());
                                    Datas::I()->mesurefinal->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                    Datas::I()->mesurefinal->setaddVPOG(mAddOG.toDouble());
                                    Datas::I()->mesurefinal->setavlOG(AVLOG);
                                    Datas::I()->mesurefinal->setecartIP(static_cast<int>(std::round(PDD.toDouble() + PDG.toDouble())));
                                }
                            }
                        }
                        //qDebug() << "OK Phoropter";
                    }
            //! On essaie de récupérer une mesure de Keratométrie
                    if (childnodemeasure.tagName() == "KM" && PortAutoref() == Q_NULLPTR && !m_LANAutoref)             //!=> il y a une mesure pour l'autoref et l'autoref est directement branché sur la box du refracteur)
                    {
                        Keratometrie  *oldMesureKerato = new Keratometrie();
                        oldMesureKerato->setdatas(Datas::I()->mesurekerato);
                        Datas::I()->mesurekerato->cleandatas();
                        QDomElement childnodekerato = childnodemeasure;
                        for (int KMit=0; KMit<childnodekerato.childNodes().size(); KMit++)
                        {
                            QDomElement childnodekeratoit = childnodekerato.childNodes().at(KMit).toElement();
                            // OEIL DROIT -----------------------------------------------------------------------------
                            if (childnodekeratoit.tagName() == "R")
                            {
                                for (int KMitR=0; KMitR<childnodekeratoit.childNodes().size(); KMitR++)
                                {
                                    QDomElement childnodemedian = childnodekeratoit.childNodes().at(KMitR).toElement();
                                    if (childnodemedian.tagName() == "Median")
                                        for (int KMitRMed=0; KMitRMed<childnodemedian.childNodes().size(); KMitRMed++)
                                        {
                                            QDomElement childnodeR = childnodemedian.childNodes().at(KMitRMed).toElement();
                                            if (childnodeR.tagName() == "R1")
                                            {
                                                for (int KMitRMedR1=0; KMitRMedR1<childnodeR.childNodes().size(); KMitRMedR1++)
                                                {
                                                    QDomElement childnodeR1 = childnodeR.childNodes().at(KMitRMedR1).toElement();
                                                    if (childnodeR1.tagName() == "Radius" && childnodeR1.text() !="")
                                                    {
                                                        K1OD = childnodeR1.text();
                                                        Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                                                    }
                                                    if (childnodeR1.tagName() == "Axis" && childnodeR1.text() != "")
                                                    {
                                                        AxeKOD = childnodeR1.text().toInt();
                                                        Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                                                    }
                                                }
                                            }
                                            if (childnodeR.tagName() == "R2")
                                            {
                                                for (int frR2=0; frR2<childnodeR.childNodes().size(); frR2++)
                                                {
                                                    QDomElement childnodeR2 = childnodeR.childNodes().at(frR2).toElement();
                                                    if (childnodeR2.tagName() == "Radius" && childnodeR2.text() !="")
                                                    {
                                                        K2OD = childnodeR2.text();
                                                        Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                                                    }
                                                }
                                            }
                                        }
                                }
                            }
                            // OEIL GAUCHE -----------------------------------------------------------------------------
                            if (childnodekeratoit.tagName() == "L")
                            {
                                for (int KMitL=0; KMitL<childnodekeratoit.childNodes().size(); KMitL++)
                                {
                                    QDomElement childnodemedian = childnodekeratoit.childNodes().at(KMitL).toElement();
                                    if (childnodemedian.tagName() == "Median")
                                        for (int KMitLMed=0; KMitLMed<childnodemedian.childNodes().size(); KMitLMed++)
                                        {
                                            QDomElement childnodeL = childnodemedian.childNodes().at(KMitLMed).toElement();
                                            if (childnodeL.tagName() == "R1")
                                            {
                                                for (int KMitLMedR1=0; KMitLMedR1<childnodeL.childNodes().size(); KMitLMedR1++)
                                                {
                                                    QDomElement childnodeR1 = childnodeL.childNodes().at(KMitLMedR1).toElement();
                                                    if (childnodeR1.tagName() == "Radius" && childnodeR1.text() !="")
                                                    {
                                                        K1OG = childnodeR1.text();
                                                        Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                                                    }
                                                    if (childnodeR1.tagName() == "Axis" && childnodeR1.text() != "")
                                                    {
                                                        AxeKOG = childnodeR1.text().toInt();
                                                        Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                                                    }
                                                }
                                            }
                                            if (childnodeL.tagName() == "R2")
                                            {
                                                for (int KMitLMedR2=0; KMitLMedR2<childnodeL.childNodes().size(); KMitLMedR2++)
                                                {
                                                    QDomElement childnodeR2 = childnodeL.childNodes().at(KMitLMedR2).toElement();
                                                    if (childnodeR2.tagName() == "Radius" && childnodeR2.text() !="")
                                                    {
                                                        K2OG = childnodeR2.text();
                                                        Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                                                    }
                                                }
                                            }
                                        }
                                }
                            }
                        }
                        if (Datas::I()->mesurekerato->isDifferent(oldMesureKerato) && !Datas::I()->mesurekerato->isdataclean())
                        {
                            InsertMesure(MesureKerato);
                            emit NouvMesure(MesureKerato);
                            delete oldMesureKerato;
                        }
                    }
            //! On essaie de récupérer une mesure de tonométrie
                    if (childnodemeasure.tagName() == "TM" && PortAutoref() == Q_NULLPTR && !m_LANAutoref)             //!=> il y a une mesure pour la tono et l'autoref est directement branché sur la box du refracteur)
                    {
                        Datas::I()->mesuretono->cleandatas();
                        QDomElement childnodetono = childnodemeasure;
                        for (int TOK=0; TOK<childnodetono.childNodes().size(); TOK++)
                        {
                            QDomElement childnodetonoit = childnodemeasure.childNodes().at(TOK).toElement();
                            // OEIL DROIT -----------------------------------------------------------------------------
                            if (childnodetonoit.tagName() == "R")
                                for (int TOR=0; TOR<childnodetonoit.childNodes().size(); TOR++)
                                {
                                    QDomElement childnodeAverage = childnodetonoit.childNodes().at(TOR).toElement();
                                    if (childnodeAverage.tagName() == "Average")
                                    {
                                        for (int TOAVG=0; TOAVG<childnodeAverage.childNodes().size(); TOAVG++)
                                        {
                                            QDomElement childnodeR = childnodeAverage.childNodes().at(TOAVG).toElement();
                                            if (childnodeR.tagName() == "IOP_mmHg" && childnodeR.text() != "")
                                            {
                                                mTOOD = childnodeR.text();
                                                Datas::I()->mesuretono->setTOD(int(mTOOD.toDouble()));
                                            }
                                        }
                                    }
                                }
                            // OEIL GAUCHE -----------------------------------------------------------------------------
                            if (childnodetonoit.tagName() == "L")
                                for (int TOL=0; TOL<childnodetonoit.childNodes().size(); TOL++)
                                {
                                    QDomElement childnodeAverage = childnodetonoit.childNodes().at(TOL).toElement();
                                    if (childnodeAverage.tagName() == "Average")
                                    {
                                        for (int TOAVG=0; TOAVG<childnodeAverage.childNodes().size(); TOAVG++)
                                        {
                                            QDomElement childnodeL = childnodeAverage.childNodes().at(TOAVG).toElement();
                                            if (childnodeL.tagName() == "IOP_mmHg" && childnodeL.text() != "")
                                            {
                                                mTOOG = childnodeL.text();
                                                Datas::I()->mesuretono->setTOG(int(mTOOG.toDouble()));
                                            }
                                        }
                                    }
                                }
                        }
                        if (!Datas::I()->mesuretono->isdataclean())
                        {
                            Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                            //qDebug() << "TOD = " + QString::number(Datas::I()->mesuretono->TOD()) + "mmHg - TOG = " + QString::number(Datas::I()->mesuretono->TOG()) + "mmHg";
                            InsertMesure(MesureTono);                     //! depuis LectureDonneesRefracteur(QString Mesure)
                            emit NouvMesure(MesureTono);
                        }
                    }
            //! On essaie de récupérer une mesure de pachymétrie
                    if (childnodemeasure.tagName() == "PACHY" && PortAutoref() == Q_NULLPTR && !m_LANAutoref)             //!=> il y a une mesure pour la pachy et l'autoref est directement branché sur la box du refracteur)
                    {
                        Datas::I()->mesurepachy->cleandatas();
                        for (int pachyK=0; pachyK<childnodemeasure.childNodes().size(); pachyK++)
                        {
                            QDomElement childnodepachy = childnodemeasure.childNodes().at(pachyK).toElement();
                            // OEIL DROIT -----------------------------------------------------------------------------
                            if (childnodepachy.tagName() == "R")
                                for (int pachyR=0; pachyR<childnodepachy.childNodes().size(); pachyR++)
                                {
                                    QDomElement childnodeAverage = childnodepachy.childNodes().at(pachyR).toElement();
                                    if (childnodeAverage.tagName() == "Average")
                                        for (int pachyAVG=0; pachyAVG<childnodeAverage.childNodes().size(); pachyAVG++)
                                        {
                                            QDomElement childnodeR = childnodeAverage.childNodes().at(pachyAVG).toElement();
                                            if (childnodeR.tagName() == "Thickness" && childnodeR.text() != "")
                                                Datas::I()->mesurepachy->setpachyOD(int(childnodeR.text().toDouble()));
                                        }
                                }
                            // OEIL GAUCHE -----------------------------------------------------------------------------
                            if (childnodepachy.tagName() == "L")
                                for (int pachyL=0; pachyL<childnodepachy.childNodes().size(); pachyL++)
                                {
                                    QDomElement childnodeAverage = childnodepachy.childNodes().at(pachyL).toElement();
                                    if (childnodeAverage.tagName() == "Average")
                                        for (int pachyAVG=0; pachyAVG<childnodeAverage.childNodes().size(); pachyAVG++)
                                        {
                                            QDomElement childnodeL = childnodeAverage.childNodes().at(pachyAVG).toElement();
                                            if (childnodeL.tagName() == "Thickness" && childnodeL.text() != "")
                                                Datas::I()->mesurepachy->setpachyOG(int(childnodeL.text().toDouble()));
                                        }
                                }
                        }
                        if (!Datas::I()->mesurepachy->isdataclean())
                        {
                            Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
                            //qDebug() << "pachyOD = " + QString::number(Datas::I()->mesurepachy->pachyOD()) + " - pachyOG = " + QString::number(Datas::I()->mesurepachy->pachyOG());
                            InsertMesure(MesurePachy);                     //! depuis LectureDonneesXMLRefracteur(QString Mesure)
                            emit NouvMesure(MesurePachy);
                        }
                   }
                }
            }
        }
    }
    debugMesure(Datas::I()->mesureautoref);
    debugMesure(Datas::I()->mesurefronto);
    debugMesure(Datas::I()->mesuretono);
    debugMesure(Datas::I()->mesurepachy);
    debugMesure(Datas::I()->mesurekerato);
    debugMesure(Datas::I()->mesureacuite);
    debugMesure(Datas::I()->mesurefinal);
}

void Procedures::LectureDonneesXMLTono(QDomDocument docxml)
{
    /*! exemple de fichier xml pour un RODENSTOCK TOPASCOPE / TOMEY TOP-1000
     *

<Measurement iop_unit="mmHg" cct_unit="um">
    <Company>Rodenstock</Company>
    <Model>TopaScope</Model>
    <PatientID>00000033</PatientID>
    <DateTime>2020-12-04_17-24-30</DateTime>
    <Eye type="OD">
        <IOP quality="Normal">16</IOP>
        <IOP quality="Low">24</IOP>
        <IOP quality="Normal">15</IOP>
        <IOPAvg>15.6</IOPAvg>
        <CCT>--.-</CCT>
        <CIOP>--.-</CIOP>
    </Eye>
    <Eye type="OS">
        <IOP quality="Normal">18</IOP>
        <IOP quality="Normal">19</IOP>
        <IOP quality="Normal">18</IOP>
        <IOPAvg>18.3</IOPAvg>
        <CCT>--.-</CCT>
        <CIOP>--.-</CIOP>
    </Eye>
    <Message>
    </Message>
</Measurement>
*/
    Logs::LogToFile("MesuresPachy.txt", docxml.toByteArray());

    QString nameTO = m_settings->value(Param_Poste_Tono).toString();
    if (nameTO =="TOMEY TOP-1000" || nameTO == "TODENSTOCK Topascope" )
    {
        QString avgOD = "";
        QString avgOG = "";
        QString cctOD = "";
        QString cctOG = "";
        QString ciopOD = "";
        QString ciopOG = "";
        QDomElement xml = docxml.documentElement();
        for (int h=0; h<xml.childNodes().size(); h++)
        {
            QDomElement level1 = xml.childNodes().at(h).toElement();
            if (level1.tagName() == "Measurement")
            {
                for (int g=0; g<level1.childNodes().size(); g++)
                {
                    QDomElement level2 = level1.childNodes().at(g).toElement();
                    if (level2.tagName() == "Eye")
                    {
                        QString oeil = level2.attribute("type","");        /*!  Lire Eye type (OD / OS)  --------------------------------------------------*/
                        for (int i=0; i<level2.childNodes().size(); i++)
                        {
                            QDomElement measure = level2.childNodes().at(i).toElement();
                            QString value = measure.text();
                            if( value == "--.-") value = "";
                            if (measure.tagName() == "IOPAvg")
                            {
                                if (oeil == "OD")                           /*! OEIL DROIT  ------------------------------------------------------------------*/
                                    avgOD = value;
                                else                                        /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                                    avgOG = value;
                            }
                            if (measure.tagName() == "CCT")
                            {
                                if (oeil == "OD")                           /*! OEIL DROIT  ------------------------------------------------------------------*/
                                    cctOD = value;
                                else                                        /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                                    cctOG = value;
                            }
                            if (measure.tagName() == "CIOP")
                            {
                                if (oeil == "OD")                           /*! OEIL DROIT  ------------------------------------------------------------------*/
                                    ciopOD = value;
                                else                                        /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                                    ciopOG = value;
                            }
                        }
                    }
                }
            }
        }
        Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
        if (cctOD.toDouble()>0 && cctOG.toDouble()>0)
        {
            Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
            if (cctOD.toDouble()>0)
                Datas::I()->mesurepachy->setpachyOD(int(cctOD.toDouble()));
            if (cctOG.toDouble()>0)
                Datas::I()->mesurepachy->setpachyOG(int(cctOG.toDouble()));
        }
        if (avgOD.toDouble()>0 && avgOG.toDouble()>0)
        {
            Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
            if (avgOD.toDouble()>0)
                Datas::I()->mesuretono->setTOD(int(avgOD.toDouble()));
            if (avgOG.toDouble()>0)
                Datas::I()->mesuretono->setTOG(int(avgOG.toDouble()));
            if (ciopOD.toDouble()>0)
                Datas::I()->mesuretono->setTODcorrigee(int(ciopOD.toDouble()));
            if (ciopOG.toDouble()>0)
                Datas::I()->mesuretono->setTOGcorrigee(int(ciopOG.toDouble()));
        }
    }
    debugMesure(Datas::I()->mesuretono);
    debugMesure(Datas::I()->mesurepachy);
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
        av = mAVLOD.leftRef(mAVLOD.indexOf("/")).toInt();
    if (av < 6)
        colorVLOD =  "\"red\"";
    if (av > 5 && av < 9)
        colorVLOD =  "\"orange\"";
    av = 0;
    if (mAVLOG.contains("/"))
        av = mAVLOG.leftRef(mAVLOG.indexOf("/")).toInt();
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
    return Reponse;
}

bool Procedures::ReglePortFronto()
{
    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;
    QString nameLM    = m_settings->value(Param_Poste_Fronto).toString();
    ReinitialiseSerialSettings(s_paramPortSerieFronto);
    if (m_settings->value(Param_Poste_PortFronto_COM_baudrate)      != QVariant()
     && m_settings->value(Param_Poste_PortFronto_COM_databits)      != QVariant()
     && m_settings->value(Param_Poste_PortFronto_COM_parity)        != QVariant()
     && m_settings->value(Param_Poste_PortFronto_COM_stopBits)      != QVariant()
     && m_settings->value(Param_Poste_PortFronto_COM_flowControl)   != QVariant())
    {
        val         = m_settings->value(Param_Poste_PortFronto_COM_baudrate).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortFronto_COM_databits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortFronto_COM_parity).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.parity = (QSerialPort::Parity)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortFronto_COM_stopBits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortFronto_COM_flowControl).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.flowControl = (QSerialPort::FlowControl)metaEnum.value(val);
    }
    if (nameLM =="TOMEY TL-3000C")
    {
        s_paramPortSerieFronto.baudRate       = QSerialPort::Baud2400;
        s_paramPortSerieFronto.dataBits       = QSerialPort::Data7;
        s_paramPortSerieFronto.parity         = QSerialPort::EvenParity;
        s_paramPortSerieFronto.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (nameLM =="VISIONIX VL1000"
          || nameLM =="HUVITZ CLM7000")
    {
        s_paramPortSerieFronto.baudRate       = QSerialPort::Baud9600;
        s_paramPortSerieFronto.dataBits       = QSerialPort::Data7;
        s_paramPortSerieFronto.parity         = QSerialPort::EvenParity;
        s_paramPortSerieFronto.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (nameLM =="NIDEK LM-1800P"
          || nameLM =="NIDEK LM-1800PD"
          || nameLM =="NIDEK LM-500")
    {
        s_paramPortSerieFronto.baudRate       = QSerialPort::Baud9600;
        s_paramPortSerieFronto.dataBits       = QSerialPort::Data8;
        s_paramPortSerieFronto.parity         = QSerialPort::EvenParity;
        s_paramPortSerieFronto.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieFronto.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    if (a)
    {
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_flowControl,    Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.flowControl));
    }
    else
    {
        m_settings->setValue(Param_Poste_PortFronto_COM_baudrate,    QVariant());
        m_settings->setValue(Param_Poste_PortFronto_COM_databits,    QVariant());
        m_settings->setValue(Param_Poste_PortFronto_COM_parity,      QVariant());
        m_settings->setValue(Param_Poste_PortFronto_COM_stopBits,    QVariant());
        m_settings->setValue(Param_Poste_PortFronto_COM_flowControl, QVariant());
    }
    return a;
}

QSerialPort *Procedures::PortFronto()
{
    return sp_portFronto;
}

//!  lire les ports séries
//! -----------------------------------------------------------------------------------------
//! Lecture du flux de données sur le port série du Fronto
//! -----------------------------------------------------------------------------------------
void Procedures::ReponsePortSerie_Fronto(const QString &s)
{
    m_mesureSerie        = s;
    //qDebug() << m_mesureSerie;
    QString nameLM    = m_settings->value(Param_Poste_Fronto).toString();

    if (nameLM =="NIDEK LM-1800P"
     || nameLM =="NIDEK LM-1800PD"
     || nameLM =="NIDEK LM-500")
    {
        if (m_mesureSerie == RequestToSendNIDEK())          //! le fronto demande la permission d'envoyer des données
        {
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            //PortFronto()->waitForReadyRead(100);
            Utils::writeDatasSerialPort(PortFronto(), SendDataNIDEK("CLM"), " SendDataNIDEK(CLM) - Fronto = ");
            return;
        }
    }
    Datas::I()->mesurefronto->cleandatas();
    LectureDonneesCOMFronto(m_mesureSerie);
    if (Datas::I()->mesurefronto->isdataclean())
        return;
    //! Enregistre la mesures dans la base
    InsertMesure(MesureFronto);

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    TypesMesures flag = MesureFronto;                        /*! règle le flag de reglage du refracteur sur Fronto seulement */
    RegleRefracteur(flag);

    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), la mesure qui vient d'être effectuée
    emit NouvMesure(MesureFronto);
}

void Procedures::LectureDonneesCOMFronto(QString Mesure)
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
    QString nameLM    = m_settings->value(Param_Poste_Fronto).toString();

    //A - AFFICHER LA MESURE --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (nameLM =="TOMEY TL-3000C")
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
                mAddOD           = Utils::PrefixePlus(Mesure.midRef(Mesure.indexOf("AR")+2,4).toDouble());
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
                mAddOG           = Utils::PrefixePlus(Mesure.midRef(Mesure.indexOf("AL")+2,4).toDouble());
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
        //les autres champs ne sont pas utilisés pour le moment -------------------------------
    }
    else if (nameLM =="VISIONIX VL1000"
          || nameLM =="HUVITZ CLM7000")
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
    else if (nameLM =="NIDEK LM-1800P"
          || nameLM =="NIDEK LM-1800PD"
          || nameLM =="NIDEK LM-500")
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

//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série du tonometre
//-----------------------------------------------------------------------------------------
void Procedures::ReponsePortSerie_Tono(const QString &s)
{
    Utils::EnChantier();
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
    return HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("Porte") + ":</b></font></td><td>" + Reponse + "</p>";
}

bool Procedures::ReglePortAutoref()
{
    QString nameARK   = m_settings->value(Param_Poste_Autoref).toString();

    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;
    ReinitialiseSerialSettings(s_paramPortSerieAutoref);
    if (m_settings->value(Param_Poste_PortAutoref_COM_baudrate)      != QVariant()
     && m_settings->value(Param_Poste_PortAutoref_COM_databits)      != QVariant()
     && m_settings->value(Param_Poste_PortAutoref_COM_parity)        != QVariant()
     && m_settings->value(Param_Poste_PortAutoref_COM_stopBits)      != QVariant()
     && m_settings->value(Param_Poste_PortAutoref_COM_flowControl)   != QVariant())
    {
        val         = m_settings->value(Param_Poste_PortAutoref_COM_baudrate).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortAutoref_COM_databits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortAutoref_COM_parity).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.parity = (QSerialPort::Parity)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortAutoref_COM_stopBits).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
        val         = m_settings->value(Param_Poste_PortAutoref_COM_flowControl).toInt();
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.flowControl = (QSerialPort::FlowControl)metaEnum.value(val);
    }
    if (nameARK =="NIDEK ARK-1A"
     || nameARK =="NIDEK ARK-1"
     || nameARK =="NIDEK ARK-1S"
     || nameARK =="NIDEK AR-1A"
     || nameARK =="NIDEK AR-1"
     || nameARK =="NIDEK AR-1S"
     || nameARK =="NIDEK ARK-530A"
     || nameARK =="NIDEK ARK-510A"
     || nameARK =="NIDEK HandyRef-K"
     || nameARK =="NIDEK ARK-30"
     || nameARK =="NIDEK AR-20"
     || nameARK =="NIDEK TONOREF III")
    {
        s_paramPortSerieAutoref.baudRate       = QSerialPort::Baud9600;
        s_paramPortSerieAutoref.dataBits       = QSerialPort::Data8;
        s_paramPortSerieAutoref.parity         = QSerialPort::EvenParity;
        s_paramPortSerieAutoref.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieAutoref.flowControl    = QSerialPort::NoFlowControl;
    }
    else if (nameARK =="TOMEY RC-5000"
          || nameARK =="RODENSTOCK CX 2000")
    {
        s_paramPortSerieAutoref.baudRate       = QSerialPort::Baud38400;
        s_paramPortSerieAutoref.dataBits       = QSerialPort::Data8;
        s_paramPortSerieAutoref.parity         = QSerialPort::NoParity;
        s_paramPortSerieAutoref.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieAutoref.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;
    if (a)
    {
        index       = QSerialPort().metaObject()->indexOfEnumerator(BAUDRATE);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator(DATABITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(PARITY);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator(STOPBITS);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator(FLOWCONTROL);
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_flowControl,    Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.flowControl));
    }
    else
    {
        m_settings->setValue(Param_Poste_PortAutoref_COM_baudrate,    QVariant());
        m_settings->setValue(Param_Poste_PortAutoref_COM_databits,    QVariant());
        m_settings->setValue(Param_Poste_PortAutoref_COM_parity,      QVariant());
        m_settings->setValue(Param_Poste_PortAutoref_COM_stopBits,    QVariant());
        m_settings->setValue(Param_Poste_PortAutoref_COM_flowControl, QVariant());
    }
    return a;
}


QSerialPort* Procedures::PortAutoref()
{
    return sp_portAutoref;
}

//! lire les fichiers xml des appareils de refraction
//! -----------------------------------------------------------------------------------------
//! Lecture du fichier xml de l'autoref
//! -----------------------------------------------------------------------------------------
void Procedures::ReponseXML_Autoref(const QDomDocument &xmldoc)
{
    QString nameARK   = m_settings->value(Param_Poste_Autoref).toString();

    bool autorefhaskerato    = (nameARK =="NIDEK ARK-1A"
                      || nameARK =="NIDEK ARK-1"
                      || nameARK =="NIDEK ARK-1S"
                      || nameARK =="NIDEK ARK-530A"
                      || nameARK =="NIDEK ARK-510A"
                      || nameARK =="NIDEK HandyRef-K"
                      || nameARK =="NIDEK TONOREF III"
                      || nameARK =="NIDEK ARK-30");
    bool autorefhastonopachy = (nameARK =="NIDEK TONOREF III");
    Datas::I()->mesureautoref   ->cleandatas();
    if (autorefhaskerato)
        Datas::I()->mesurekerato    ->cleandatas();
    if (autorefhastonopachy)
    {
        Datas::I()->mesurepachy     ->cleandatas();
        Datas::I()->mesuretono      ->cleandatas();
    }

    LectureDonneesXMLAutoref(xmldoc);
    if ( !autorefhaskerato && !autorefhastonopachy && Datas::I()->mesureautoref->isdataclean())
        return;
    else if (autorefhaskerato && Datas::I()->mesureautoref->isdataclean() && Datas::I()->mesurekerato->isdataclean())
        return;
    else if (autorefhastonopachy && Datas::I()->mesureautoref->isdataclean()
        &&  Datas::I()->mesurekerato   ->isdataclean()
        &&  Datas::I()->mesuretono      ->isdataclean()
        &&  Datas::I()->mesurepachy     ->isdataclean())
        return;
    //! Enregistre les mesures dans la base
    if (!Datas::I()->mesurekerato->isdataclean())
        InsertMesure(MesureKerato);
    if (!Datas::I()->mesureautoref->isdataclean())
        InsertMesure(MesureAutoref);
    if (autorefhastonopachy)
    {
        if (!Datas::I()->mesuretono->isdataclean())
            InsertMesure(MesureTono);                     //! depuis ReponsePortSerie_Autoref(const QString &s)
        if (!Datas::I()->mesurepachy->isdataclean())
            InsertMesure(MesurePachy);                    //! depuis ReponsePortSerie_Autoref(const QString &s)
    }

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    TypesMesures flag = MesureAutoref;                        /*! règle le flag de reglage du refracteur sur Autoref seulement */
    RegleRefracteur(flag);

    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), les mesures qui viennent d'être effectuées
    if (autorefhaskerato && !Datas::I()->mesurekerato->isdataclean())
        emit NouvMesure(MesureKerato);
    if (!Datas::I()->mesureautoref->isdataclean())
        emit NouvMesure(MesureAutoref);
    if (autorefhastonopachy)
    {
        if (!Datas::I()->mesuretono->isdataclean())
            emit NouvMesure(MesureTono);
        if (!Datas::I()->mesurepachy->isdataclean())
            emit NouvMesure(MesurePachy);
    }

}


//! lire les fichiers xml des appareils de refraction
//! -----------------------------------------------------------------------------------------
//! Lecture du fichier xml du fronto
//! -----------------------------------------------------------------------------------------
void Procedures::ReponseXML_Fronto(const QDomDocument &xmldoc)
{
    Datas::I()->mesurefronto   ->cleandatas();
    LectureDonneesXMLFronto(xmldoc);
    if (Datas::I()->mesurefronto->isdataclean())
        return;
    //! Enregistre la mesure dans la base
    InsertMesure(MesureFronto);

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    TypesMesures flag = MesureFronto;                        /*! règle le flag de reglage du refracteur sur Fronto seulement */
    RegleRefracteur(flag);

    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), la mesure qui vient d'être effectuée
    emit NouvMesure(MesureFronto);
}

//! lire les fichiers xml des appareils de refraction
//! -----------------------------------------------------------------------------------------
//! Lecture du fichier xml du refracteur
//! -----------------------------------------------------------------------------------------
void Procedures::ReponseXML_Refracteur(const QDomDocument &xmldoc)
{
    Datas::I()->mesureacuite   ->cleandatas();
    Datas::I()->mesurefinal    ->cleandatas();

    LectureDonneesXMLRefracteur(xmldoc);
    if ( Datas::I()->mesureacuite->isdataclean() && Datas::I()->mesurefinal->isdataclean() )
        return;

    //! Enregistre les mesures dans la base
    InsertMesure(MesureRefracteur);

    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), les mesures qui viennent d'être effectuées
    emit NouvMesure(MesureRefracteur);
}

//! lire les fichiers xml des appareils de refraction
//! -----------------------------------------------------------------------------------------
//! Lecture du fichier xml du tonometre
//! -----------------------------------------------------------------------------------------
void Procedures::ReponseXML_Tono(const QDomDocument &xmldoc)
{
    Datas::I()->mesuretono   ->cleandatas();
    Datas::I()->mesurepachy  ->cleandatas();

    LectureDonneesXMLTono(xmldoc);

    //! Enregistre les mesures dans la base
    if (!Datas::I()->mesuretono->isdataclean())
        InsertMesure(MesureTono);
    if (!Datas::I()->mesurepachy->isdataclean())
        InsertMesure(MesurePachy);

    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), les mesures qui viennent d'être effectuées
    if (!Datas::I()->mesuretono->isdataclean())
        emit NouvMesure(MesureTono);
    if (!Datas::I()->mesurepachy->isdataclean())
        emit NouvMesure(MesurePachy);
}

//! lire les ports séries
//! -----------------------------------------------------------------------------------------
//! Lecture du flux de données sur le port série de l'autoref
//! -----------------------------------------------------------------------------------------
void Procedures::ReponsePortSerie_Autoref(const QString &s)
{
    m_mesureSerie        = s;
    //qDebug() << m_mesureSerie;
    QString nameARK   = m_settings->value(Param_Poste_Autoref).toString();

    bool autorefhaskerato    = (nameARK =="NIDEK ARK-1A"
                      || nameARK =="NIDEK ARK-1"
                      || nameARK =="NIDEK ARK-1S"
                      || nameARK =="NIDEK ARK-530A"
                      || nameARK =="NIDEK ARK-510A"
                      || nameARK =="NIDEK HandyRef-K"
                      || nameARK =="NIDEK TONOREF III"
                      || nameARK =="NIDEK ARK-30"
                      || nameARK =="TOMEY RC-5000"
                      || nameARK =="RODENSTOCK CX 2000");

    bool autorefhastonopachy = (nameARK =="NIDEK TONOREF III");

    if (nameARK =="NIDEK ARK-1A"
     || nameARK =="NIDEK ARK-1"
     || nameARK =="NIDEK ARK-1S"
     || nameARK =="NIDEK AR-1A"
     || nameARK =="NIDEK AR-1"
     || nameARK =="NIDEK AR-1S"
     || nameARK =="NIDEK ARK-530A"
     || nameARK =="NIDEK ARK-510A"
     || nameARK =="NIDEK HandyRef-K"
     || nameARK =="NIDEK TONOREF III"
     || nameARK =="NIDEK ARK-30"
     || nameARK =="NIDEK AR-20")
    {
        if (m_mesureSerie == RequestToSendNIDEK())       //! l'autoref demande la permission d'envoyer des données
        {
            QString cmd;
            cmd = (autorefhaskerato? "CRK" : "CRM");     //! CRK ou CRM suivant que les appareils peuvent ou non envoyer la keratométrie
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            //PortAutoref()->waitForReadyRead(100);
            Utils::writeDatasSerialPort(PortAutoref(), SendDataNIDEK(cmd), " SendDataNIDEK(cmd) - Autoref = ");
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

    LectureDonneesCOMAutoref(m_mesureSerie);
    if ( !autorefhaskerato && !autorefhastonopachy && Datas::I()->mesureautoref->isdataclean())
        return;
    else if (autorefhaskerato && Datas::I()->mesureautoref->isdataclean() && Datas::I()->mesurekerato->isdataclean())
        return;
    else if (autorefhastonopachy && Datas::I()->mesureautoref->isdataclean()
        &&  Datas::I()->mesurekerato   ->isdataclean()
        &&  Datas::I()->mesuretono      ->isdataclean()
        &&  Datas::I()->mesurepachy     ->isdataclean())
        return;

    //! Enregistre les mesures dans la base
    if (!Datas::I()->mesurekerato->isdataclean())
        InsertMesure(MesureKerato);
    if (!Datas::I()->mesureautoref->isdataclean())
        InsertMesure(MesureAutoref);
    if (autorefhastonopachy)
    {
        if (!Datas::I()->mesuretono->isdataclean())
            InsertMesure(MesureTono);                     //! depuis ReponsePortSerie_Autoref(const QString &s)
        if (!Datas::I()->mesurepachy->isdataclean())
            InsertMesure(MesurePachy);                    //! depuis ReponsePortSerie_Autoref(const QString &s)
    }

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    TypesMesures flag = MesureAutoref;                        /*! règle le flag de reglage du refracteur sur Autoref seulement */
    RegleRefracteur(flag);

    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), les mesures qui viennent d'être effectuées
    if (autorefhaskerato && !Datas::I()->mesurekerato->isdataclean())
        emit NouvMesure(MesureKerato);
    if (!Datas::I()->mesureautoref->isdataclean())
        emit NouvMesure(MesureAutoref);
    if (autorefhastonopachy)
    {
        if (!Datas::I()->mesuretono->isdataclean())
            emit NouvMesure(MesureTono);
        if (!Datas::I()->mesurepachy->isdataclean())
            emit NouvMesure(MesurePachy);
    }
}

void Procedures::LectureDonneesCOMAutoref(QString Mesure)
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
    QString nameARK   = m_settings->value(Param_Poste_Autoref).toString();

    if (nameARK =="NIDEK ARK-1A"
     || nameARK =="NIDEK ARK-1"
     || nameARK =="NIDEK ARK-1S"
     || nameARK =="NIDEK AR-1A"
     || nameARK =="NIDEK AR-1"
     || nameARK =="NIDEK AR-1S"
     || nameARK =="NIDEK ARK-530A"
     || nameARK =="NIDEK ARK-510A"
     || nameARK =="NIDEK HandyRef-K"
     || nameARK =="NIDEK TONOREF III"
     || nameARK =="NIDEK ARK-30"
     || nameARK =="NIDEK AR-20")
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
        bool autorefhaskerato    = (nameARK =="NIDEK ARK-1A"
                          || nameARK =="NIDEK ARK-1"
                          || nameARK =="NIDEK ARK-1S"
                          || nameARK =="NIDEK ARK-530A"
                          || nameARK =="NIDEK ARK-510A"
                          || nameARK =="NIDEK HandyRef-K"
                          || nameARK =="NIDEK TONOREF III"
                          || nameARK =="NIDEK ARK-30");
        bool autorefhastonopachy = (nameARK =="NIDEK TONOREF III");
        bool autorefhasipmesure = (nameARK  != "NIDEK HandyRef-K"
                                || nameARK  != "NIDEK ARK-30"
                                || nameARK  != "NIDEK AR-20");

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
                        AxeKOD              = KOD.midRef(10,3).toInt();
                        Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                        Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                        Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                        QString mOD         = K.mid(K.indexOf("DR")+2,10).replace(" ","0");
                        Datas::I()->mesurekerato->setdioptriesK1OD(mOD.midRef(0,5).toDouble());
                        Datas::I()->mesurekerato->setdioptriesK2OD(mOD.midRef(5,5).toDouble());
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
                        AxeKOG              = KOG.midRef(10,3).toInt();
                        Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                        Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                        Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                        QString mOG         = K.mid(K.indexOf("DL")+2,10).replace(" ","0");
                        Datas::I()->mesurekerato->setdioptriesK1OG(mOG.midRef(0,5).toDouble());
                        Datas::I()->mesurekerato->setdioptriesK2OG(mOG.midRef(5,5).toDouble());
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
    else if (nameARK =="TOMEY RC-5000"
     || nameARK =="RODENSTOCK CX 2000")
    {
        /*! SORTIE EXEMPLE POUR UN TOMEY RC-5000
         * SOH =    QByteArray::fromHex("1")            //SOH -> start of header
         * STX =    QByteArray::fromHex("2")            //STX -> start of text
         * CR =     QByteArray::fromHex("13")           //CR -> carriage return
         * EOT =    QByteArray::fromHex("4")            //EOT -> end of transmission
         * La 1ere ligne commence par SOH, la dernière par EOT- représentés ici
         * Les autres lignes commencent par STX
         * Toutes les lignes se terminent par CR


SOH*1234                                    -> id (4 octets)
RK
0R+ 5.25- 0.25179                           -> AR Côté (R/L) Sphere (6o) Cylindre (6o) Axe (3o)
0L+ 3.00- 0.75 89
DB60.5                                      -> EIP (4o)
CR 5.0045.00 89 4.0040.00 75 4.50- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o)
CL 5.0045.00 89 4.0040.00 75 4.50- 0.25
*/
        a               = Mesure.indexOf("RK");
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
            a  = Ref.indexOf("DB");
            if (a >= 0) {
                QString PD      = Ref.mid(Ref.indexOf("DB")+2,2);
                Datas::I()->mesureautoref->setecartIP(PD.toInt());
            }
            // Données de KERATOMETRIE -------------------------------------------------------------------------------------------------------
            // OEIL DROIT -----------------------------------------------------------------------------
            a  = Ref.indexOf("CR");
            if (a>=0)
            {
                /*!->CR 5.0045.00 89 4.0040.00 75 4.50- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
                QString KOD("");
                QString K1OD("null"), K2OD("null");
                int     AxeKOD(0);
                KOD                 = Ref.mid(Ref.indexOf("CR")+2,37);
                /*!              1           2           3
                 *!->01234|56789|012|34567|89012|345|67890|123456 */
                /*   mK1OD|dK1OD|Axe|mK2OD|dK2OD|Axe|mAvgK|Cylndr
                 *!-> 5.00|45.00| 89| 4.00|40.00| 75| 4.50|- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
                KOD.replace(" ", "0");
                /*!->05.00|45.00|089|04.00|40.00|075|04.50|-00.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
                K1OD                = KOD.mid(0,5);
                K2OD                = KOD.mid(13,5);
                AxeKOD              = KOD.midRef(10,3).toInt();
                if (K1OD.toDouble() != 0 && K2OD.toDouble() != 0)
                {
                    Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                    Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                    Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                    Datas::I()->mesurekerato->setdioptriesK1OD(KOD.midRef(5,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OD(KOD.midRef(18,5).toDouble());
                }
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            a  = Ref.indexOf("CL");
            if (a>=0)
            {
                /*!->CL 5.0045.00 89 4.0040.00 75 4.50- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
                QString KOG("");
                QString K1OG("null"), K2OG("null");
                int     AxeKOG(0);
                KOG                 = Ref.mid(Ref.indexOf("CR")+2,37);
                /*!              1           2           3
                 *!->01234|56789|012|34567|89012|345|67890|123456 */
                /*   mK1OG|dK1OG|Axe|mK2OG|dK2OG|Axe|mAvgK|Cylndr
                 *!-> 5.00|45.00| 89| 4.00|40.00| 75| 4.50|- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
                KOG.replace(" ", "0");
                /*!->05.00|45.00|089|04.00|40.00|075|04.50|-00.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
                K1OG                = KOG.mid(0,5);
                K2OG                = KOG.mid(13,5);
                AxeKOG              = KOG.midRef(10,3).toInt();
                if (K1OG.toDouble() != 0 && K2OG.toDouble() != 0)
                {
                    Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                    Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                    Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                    Datas::I()->mesurekerato->setdioptriesK1OG(KOG.midRef(5,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OG(KOG.midRef(18,5).toDouble());
                }
            }
        }
    }
    //qDebug() << "od" << mSphereOD << mCylOD << mAxeOD << "og" << mSphereOG << mCylOG << mAxeOG << "PD = " + PD;
}

void Procedures::LectureDonneesXMLAutoref(QDomDocument docxml)
{
    /*! exemple de fichier xml pour un ARK-1s
     *
     *
<?xml version="1.0" encoding="UTF-16"?>
<?xml-stylesheet type="text/xsl" href="RKT_style.xsl"?>
<Data>
    <Company>NIDEK</Company>
    <ModelName>ARK-1s</ModelName>
    <ROMVersion>1.00.02 /5.05</ROMVersion>
    <Version>1.01</Version>
    <Date>2013/03/11</Date>
    <Time>16:03:07</Time>
    <Patient>
        <No.>0003</No.>
        <ID>4902205625223</ID>
    </Patient>
    <Comment> NIDEK ARK-1s</Comment>
    <VD>12.00 mm</VD>
    <WorkingDistance>40 cm</WorkingDistance>
    <DiopterStep>0.01D</DiopterStep>
    <AxisStep>1°</AxisStep>
    <CylinderMode>-</CylinderMode>
    <RefractiveIndex>1.3375</RefractiveIndex>
    <R>
        <AR>
            <ARList No = "1">
                <Sphere>-6.38</Sphere>
                <Cylinder>-0.63</Cylinder>
                <Axis>179</Axis>
                <CataractMode>ON</CataractMode>
                <ConfidenceIndex>9</ConfidenceIndex>
                <SE>-6.70</SE>
            </ARList>
            <ARList No = "2">
                <Error>COVR </Error>
            </ARList>
            <ARMedian>
                <Sphere>-6.38</Sphere>
                <Cylinder>-0.64</Cylinder>
                <Axis>177</Axis>
                <SE>-6.70</SE>
            </ARMedian>
            <TrialLens>
                <Sphere>-6.25</Sphere>
                <Cylinder>-0.75</Cylinder>
                <Axis>177</Axis>
            </TrialLens>
            <ContactLens>
                <Sphere>-5.93</Sphere>
                <Cylinder>-0.54</Cylinder>
                <Axis>177</Axis>
                <SE>-6.20</SE>
            </ContactLens>
            <RingImage>
                ARK_4902205625223 _20130311160307RA1.jpg
            </RingImage>
        </AR>
        <VA>
            <UCVA>&lt;0.1</UCVA>
            <BCVA>1.0</BCVA>
            <LVA>0.8</LVA>
            <GVA>0.5</GVA>
            <NVA>0.8</NVA>
            <WorkingDistance>35 cm</WorkingDistance>
        </VA>
        <SR>
            <Sphere>-6.25</Sphere>
            <Cylinder>-0.75</Cylinder>
            <Axis>177</Axis>
            <SE>-6.75</SE>
            <ADD>+1.75</ADD>
            <WorkingDistance>35 cm</WorkingDistance>
        </SR>
        <LM>
            <Sphere>-0.50</Sphere>
            <Cylinder>-0.00</Cylinder>
            <Axis>0</Axis>
            <ADD>+3.00</ADD>
            <ADD2>+3.50</ADD2>
        </LM>
        <KM>
            <KMList No = "1">
                <R1>
                    <Radius>7.56</Radius>
                    <Power>44.64</Power>
                    <Axis>179</Axis>
                </R1>
                <R2>
                    <Radius>7.29</Radius>
                    <Power>46.30</Power>
                    <Axis>89</Axis>
                </R2>
                <Average>
                    <Radius>7.43</Radius>
                    <Power>45.42</Power>
                </Average>
                <KMCylinder>
                    <Power>-1.66</Power>
                    <Axis>179</Axis>
                </KMCylinder>
            </KMList>
            <KMMedian>
                <R1>
                    <Radius>7.55</Radius>
                    <Power>44.70</Power>
                    <Axis>178</Axis>
                </R1>
                <R2>
                    <Radius>7.29</Radius>
                    <Power>46.30</Power>
                    <Axis>88</Axis>
                </R2>
                <Average>
                    <Radius>7.42</Radius>
                    <Power>45.49</Power>
                </Average>
                <KMCylinder>
                    <Power>-1.60</Power>
                    <Axis>178</Axis>
                </KMCylinder>
            </KMMedian>
        </KM>
        <CS>
            <CSList No = "1">
                <Size>12.1</Size>
            </CSList>
        </CS>
        <PS>
            <PSList No = "1">
                <Size>4.7</Size>
                <Lamp>ON</Lamp>
            </PSList>
        </PS>
        <AC>
            <Sphere>8.15</Sphere>
            <MaxPS>4.1</MaxPS>
            <MinPS>1.6</MinPS>
            <AccImage>ARK_4902205625223_20130311160307RC1.jpg</AccImage>
        </AC>
        <RI>
            <COIH>0.7</COIH>
            <COIA>1</COIA>
            <POI>0</POI>
            <RetroImage>ARK_4902205625223_20130311160307RI1.jpg</RetroImage>
        </RI>
    </R>
    <L>
    </L>
    <PD>
        <PDList No = "1">
            <FarPD>56</FarPD>
            <RPD>28</RPD>
            <LPD>28</LPD>
            <NearPD>53</NearPD>
         </PDList>
    </PD>
</Data>

*/
    Logs::LogToFile("MesuresAutoref.txt", docxml.toByteArray());
    QString nameARK = m_settings->value(Param_Poste_Autoref).toString();
    bool autorefhastonopachy = false;
    if (nameARK =="NIDEK ARK-1A"
     || nameARK =="NIDEK ARK-1"
     || nameARK =="NIDEK ARK-1S"
     || nameARK =="NIDEK AR-1A"
     || nameARK =="NIDEK AR-1"
     || nameARK =="NIDEK AR-1S"
     || nameARK =="NIDEK ARK-530A"
     || nameARK =="NIDEK ARK-510A"
     || nameARK =="NIDEK HandyRef-K"
     || nameARK =="NIDEK TONOREF III"
     || nameARK =="NIDEK ARK-30"
     || nameARK =="NIDEK AR-20")
    {
        bool autorefhaskerato    = (nameARK =="NIDEK ARK-1A"
                          || nameARK =="NIDEK ARK-1"
                          || nameARK =="NIDEK ARK-1S"
                          || nameARK =="NIDEK ARK-530A"
                          || nameARK =="NIDEK ARK-510A"
                          || nameARK =="NIDEK HandyRef-K"
                          || nameARK =="NIDEK TONOREF III"
                          || nameARK =="NIDEK ARK-30");
        autorefhastonopachy = (nameARK =="NIDEK TONOREF III");
        bool autorefhasipmesure = (nameARK  != "NIDEK HandyRef-K"
                                || nameARK  != "NIDEK ARK-30"
                                || nameARK  != "NIDEK AR-20");
        bool istonorefIII = (nameARK =="NIDEK TONOREF III");
        QDomElement xml = docxml.documentElement();
        for (int i=0; i<xml.childNodes().size(); i++)
        {
            QDomElement childnode = xml.childNodes().at(i).toElement();
            if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement childRnode = childnode.childNodes().at(j).toElement();
                    if (childRnode.tagName() == "AR")
                    {
                        for (int k=0; k<childRnode.childNodes().size(); k++)
                        {
                            QDomElement childARnode = childRnode.childNodes().at(k).toElement();
                            if (childARnode.tagName() == "ARMedian")
                            {
                                for (int l=0; l<childARnode.childNodes().size(); l++)
                                {
                                    QDomElement childARmednode = childARnode.childNodes().at(l).toElement();
                                    if (childARmednode.tagName() == "Sphere")
                                        Datas::I()->mesureautoref->setsphereOD(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                    if (childARmednode.tagName() == "Cylinder")
                                        Datas::I()->mesureautoref->setcylindreOD(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                    if (childARmednode.tagName() == "Axis")
                                        Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(childARmednode.text().toInt()));
                                }
                            }
                        }
                    }
                }
            }
            if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement childRnode = childnode.childNodes().at(j).toElement();
                    if (childRnode.tagName() == "AR")
                    {
                        for (int k=0; k<childRnode.childNodes().size(); k++)
                        {
                            QDomElement childARnode = childRnode.childNodes().at(k).toElement();
                            if (childARnode.tagName() == "ARMedian")
                            {
                                for (int l=0; l<childARnode.childNodes().size(); l++)
                                {
                                    QDomElement childARmednode = childARnode.childNodes().at(l).toElement();
                                    if (childARmednode.tagName() == "Sphere")
                                        Datas::I()->mesureautoref->setsphereOG(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                    if (childARmednode.tagName() == "Cylinder")
                                        Datas::I()->mesureautoref->setcylindreOG(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                    if (childARmednode.tagName() == "Axis")
                                        Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(childARmednode.text().toInt()));
                                }
                            }
                        }
                    }
                }
            }
        }
        if (autorefhasipmesure)
        {
            Datas::I()->mesureautoref->setecartIP(0);
            for (int i=0; i<xml.childNodes().size(); i++)
            {
                QDomElement childnode = xml.childNodes().at(i).toElement();
                if (childnode.tagName() == "PD")                         /*! ECART INTERPUPILLAIRE  --------------------------------------------------------*/
                {
                    for (int j=0; j<childnode.childNodes().size(); j++)
                    {
                        QDomElement childPDnode = childnode.childNodes().at(j).toElement();
                        if (childPDnode.tagName() == "PDList" && childPDnode.attributeNode("No").value().toInt() == 1)
                        {
                            for (int k=0; k<childPDnode.childNodes().size(); k++)
                            {
                                QDomElement childPD1node = childPDnode.childNodes().at(k).toElement();
                                if (childPD1node.tagName() == "FarPD")
                                {
                                    Datas::I()->mesureautoref->setecartIP(childPD1node.text().toInt());
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
        if (autorefhaskerato)
        {
            QString DiametreKeratoAUtiliserTonorefIII = "2.4mm";
            //! Données de KERATOMETRIE --------------------------------------------------------------------------------------------------------
            for (int i=0; i<xml.childNodes().size(); i++)
            {
                QDomElement childnode = xml.childNodes().at(i).toElement();
                if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
                {
                    for (int j=0; j<childnode.childNodes().size(); j++)
                    {
                        QDomElement K = childnode.childNodes().at(j).toElement();
                        if ((K.tagName() == "KM" && !istonorefIII)
                         || (K.tagName() == "KM" && istonorefIII && (K.attributeNode("condition").value() == "ø"+ DiametreKeratoAUtiliserTonorefIII || K.attributeNode("condition").value() == "Φ" + DiametreKeratoAUtiliserTonorefIII)))
                        {
                            for (int k=0; k<K.childNodes().size(); k++)
                            {
                                QDomElement KM = K.childNodes().at(k).toElement();
                                if (KM.tagName() == "KMMedian")
                                {
                                    for (int l=0; l<KM.childNodes().size(); l++)
                                    {
                                        QDomElement KMR = KM.childNodes().at(l).toElement();
                                        if (KMR.tagName() == "R1")
                                        {
                                            for (int m=0; m<KMR.childNodes().size(); m++)
                                            {
                                                QDomElement KMR1 = KMR.childNodes().at(m).toElement();
                                                if (KMR1.tagName() == "Radius" && KMR1.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setK1OD(KMR1.text().toDouble());
                                                if (KMR1.tagName() == "Power" && KMR1.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setdioptriesK1OD(round(KMR1.text().toDouble()*10)/10);
                                           }
                                        }
                                        if (KMR.tagName() == "R2")
                                        {
                                            for (int m=0; m<KMR.childNodes().size(); m++)
                                            {
                                                QDomElement KMR2 = KMR.childNodes().at(m).toElement();
                                                if (KMR2.tagName() == "Radius" && KMR2.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setK2OD(KMR2.text().toDouble());
                                                if (KMR2.tagName() == "Power" && KMR2.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setdioptriesK2OD(round(KMR2.text().toDouble()*10)/10);
                                            }
                                        }
                                        if (KMR.tagName() == "KMCylinder")
                                        {
                                            for (int m=0; m<KMR.childNodes().size(); m++)
                                            {
                                                QDomElement axeK = KMR.childNodes().at(m).toElement();
                                                if (axeK.tagName() == "Axis" && axeK.text().toInt() > 0)
                                                    Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(axeK.text().toInt()));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                {
                    for (int j=0; j<childnode.childNodes().size(); j++)
                    {
                        QDomElement K = childnode.childNodes().at(j).toElement();
                        if ((K.tagName() == "KM" && !istonorefIII)
                         || (K.tagName() == "KM" && istonorefIII && (K.attributeNode("condition").value() == "ø" + DiametreKeratoAUtiliserTonorefIII || K.attributeNode("condition").value() == "Φ" + DiametreKeratoAUtiliserTonorefIII)))
                        {
                            for (int k=0; k<K.childNodes().size(); k++)
                            {
                                QDomElement KM = K.childNodes().at(k).toElement();
                                if (KM.tagName() == "KMMedian")
                                {
                                    for (int l=0; l<KM.childNodes().size(); l++)
                                    {
                                        QDomElement KMR = KM.childNodes().at(l).toElement();
                                        if (KMR.tagName() == "R1")
                                        {
                                            for (int m=0; m<KMR.childNodes().size(); m++)
                                            {
                                                QDomElement KMR1 = KMR.childNodes().at(m).toElement();
                                                if (KMR1.tagName() == "Radius" && KMR1.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setK1OG(KMR1.text().toDouble());
                                                if (KMR1.tagName() == "Power" && KMR1.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setdioptriesK1OG(Utils::roundToNearestPointTwentyFive(KMR1.text().toDouble()));
                                           }
                                        }
                                        if (KMR.tagName() == "R2")
                                        {
                                            for (int m=0; m<KMR.childNodes().size(); m++)
                                            {
                                                QDomElement KMR2 = KMR.childNodes().at(m).toElement();
                                                if (KMR2.tagName() == "Radius" && KMR2.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setK2OG(KMR2.text().toDouble());
                                                if (KMR2.tagName() == "Power" && KMR2.text().toDouble() > 0)
                                                    Datas::I()->mesurekerato->setdioptriesK2OG(Utils::roundToNearestPointTwentyFive(KMR2.text().toDouble()));
                                            }
                                        }
                                        if (KMR.tagName() == "KMCylinder")
                                        {
                                            for (int m=0; m<KMR.childNodes().size(); m++)
                                            {
                                                QDomElement axeK = KMR.childNodes().at(m).toElement();
                                                if (axeK.tagName() == "Axis" && axeK.text().toInt() > 0)
                                                    Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(axeK.text().toInt()));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (autorefhastonopachy)
        {
            //! Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
            for (int i=0; i<xml.childNodes().size(); i++)
            {
                QDomElement childnode = xml.childNodes().at(i).toElement();
                if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
                {
                    for (int j=0; j<childnode.childNodes().size(); j++)
                    {
                        QDomElement TO = childnode.childNodes().at(j).toElement();
                        if (TO.tagName() == "NT")
                        {
                            for (int k=0; k<TO.childNodes().size(); k++)
                            {
                                QDomElement TOD = TO.childNodes().at(k).toElement();
                                if (TOD.tagName() == "NTAverage")
                                {
                                    for (int l=0; l<TOD.childNodes().size(); l++)
                                    {
                                        QDomElement TOunit = TOD.childNodes().at(l).toElement();
                                        if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                        {
                                            Datas::I()->mesuretono->setTOD(std::round(TOunit.text().toDouble()));
                                            Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                        }
                                    }
                                }
                                if (TOD.tagName() == "CorrectedIOP")
                                {
                                    for (int l=0; l<TOD.childNodes().size(); l++)
                                    {
                                        QDomElement TOcor = TOD.childNodes().at(l).toElement();
                                        if (TOcor.tagName() == "Corrected")
                                        {
                                            for (int m=0; m<TOcor.childNodes().size(); m++)
                                            {
                                                QDomElement TOunit = TOcor.childNodes().at(m).toElement();
                                                if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                                {
                                                    Datas::I()->mesuretono->setTODcorrigee(std::round(TOunit.text().toDouble()));
                                                    Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                {
                    for (int j=0; j<childnode.childNodes().size(); j++)
                    {
                        QDomElement TO = childnode.childNodes().at(j).toElement();
                        if (TO.tagName() == "NT")
                        {
                            for (int k=0; k<TO.childNodes().size(); k++)
                            {
                                QDomElement TOG = TO.childNodes().at(k).toElement();
                                if (TOG.tagName() == "NTAverage")
                                {
                                    for (int l=0; l<TOG.childNodes().size(); l++)
                                    {
                                        QDomElement TOunit = TOG.childNodes().at(l).toElement();
                                        if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                        {
                                            Datas::I()->mesuretono->setTOG(std::round(TOunit.text().toDouble()));
                                            Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                        }
                                    }
                                }
                                if (TOG.tagName() == "CorrectedIOP")
                                {
                                    for (int l=0; l<TOG.childNodes().size(); l++)
                                    {
                                        QDomElement TOcor = TOG.childNodes().at(l).toElement();
                                        if (TOcor.tagName() == "Corrected")
                                        {
                                            for (int m=0; m<TOcor.childNodes().size(); m++)
                                            {
                                                QDomElement TOunit = TOcor.childNodes().at(m).toElement();
                                                if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                                {
                                                    Datas::I()->mesuretono->setTOGcorrigee(std::round(TOunit.text().toDouble()));
                                                    Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            //! Données de PACHYMETRIE --------------------------------------------------------------------------------------------------------
            for (int i=0; i<xml.childNodes().size(); i++)
            {
                QDomElement childnode = xml.childNodes().at(i).toElement();
                if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
                {
                    for (int j=0; j<childnode.childNodes().size(); j++)
                    {
                        QDomElement pachy = childnode.childNodes().at(j).toElement();
                        if (pachy.tagName() == "PACHY")
                        {
                            for (int k=0; k<pachy.childNodes().size(); k++)
                            {
                                QDomElement pachyD = pachy.childNodes().at(k).toElement();
                                if (pachyD.tagName() == "PACHYAverage")
                                {
                                    for (int l=0; l<pachyD.childNodes().size(); l++)
                                    {
                                        QDomElement thickness = pachyD.childNodes().at(l).toElement();
                                        if (thickness.tagName() == "Thickness" && thickness.text().toInt() > 0)
                                        {
                                            Datas::I()->mesurepachy->setpachyOD(thickness.text().toInt());
                                            Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                {
                    for (int j=0; j<childnode.childNodes().size(); j++)
                    {
                        QDomElement pachy = childnode.childNodes().at(j).toElement();
                        if (pachy.tagName() == "PACHY")
                        {
                            for (int k=0; k<pachy.childNodes().size(); k++)
                            {
                                QDomElement pachyG = pachy.childNodes().at(k).toElement();
                                if (pachyG.tagName() == "PACHYAverage")
                                {
                                    for (int l=0; l<pachyG.childNodes().size(); l++)
                                    {
                                        QDomElement thickness = pachyG.childNodes().at(l).toElement();
                                        if (thickness.tagName() == "Thickness" && thickness.text().toInt() > 0)
                                        {
                                            Datas::I()->mesurepachy->setpachyOG(thickness.text().toInt());
                                            Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
//    debugMesure(Datas::I()->mesureautoref);
//    debugMesure(Datas::I()->mesurekerato);
//    if (autorefhastonopachy)
//    {
//        if (!Datas::I()->mesuretono->isdataclean())
//            debugMesure(Datas::I()->mesuretono);
//        if (!Datas::I()->mesurepachy->isdataclean())
//            debugMesure(Datas::I()->mesurepachy);
//    }
}

void Procedures::LectureDonneesXMLFronto(QDomDocument docxml)
{
    /*! exemple de fichier xml pour un ARK-1s
     *‌ il est conforme au standard de la Japan Ophthalmic Instrument Association (https://www.joia.or.jp/)
     * donc le code pour parser le xml est le même pour tous les appareils qui suivent ce standard p.e. Tomey Tl-6000 et TL-7000, Rodenstock AL 6600

<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="NIDEK_LM_Stylesheet.xsl" ?
<Ophthalmology>
<Common>
    <Company>NIDEK</Company>
    <ModelName>LM-1800PD</ModelName>
    <MachineNo></MachineNo>
    <ROMVersion>1.10</ROMVersion>
    <Version>NIDEK_V1.01</Version>
    <Date>2011-03-04</Date>
    <Time>08:08:03</Time>
    <Patient>
        <No.>0001</No.>
        <ID>pati</ID>
        <FirstName></FirstName>
        <MiddleName></MiddleName>
        <LastName></LastName>
        <Sex></Sex>
        <Age></Age>
        <DOB></DOB>
        <NameJ1></NameJ1>
        <NameJ2></NameJ2>
    </Patient>
    <Operator>
        <No.></No.>
        <ID>ope</ID>
    </Operator>
</Common>
<Measure Type="LM">
    <MeasureMode>AutoProgressive</MeasureMode>
    <DiopterStep unit="D">0.25</DiopterStep>
    <AxisStep unit="deg">1</AxisStep>
    <CylinderMode>-</CylinderMode>
    <PrismDiopterStep unit="pri">0.25</PrismDiopterStep>
    <PrismBaseStep unit="deg">1</PrismBaseStep>
    <PrismMode>xy</PrismMode>
    <AddMode>add</AddMode>
    <LM>
        <S>
            <Sphere unit="D">-2.00</Sphere>
            <Cylinder unit="D">-1.50</Cylinder>
            <Axis unit="deg">143</Axis>
            <SE unit="D"></SE>
            <ADD unit="D">1.75</ADD>
            <ADD2 unit="D"></ADD2>
            <NearSphere unit="D">0.00</NearSphere>
            <NearSphere2 unit="D"></NearSphere2>
            <Prism unit="pri">2.25</Prism>
            <PrismBase unit="deg">77</PrismBase>
            <PrismX unit="pri" base="in">0.50</PrismX>
            <PrismY unit="pri" base="up">2.00</PrismY>
            <UVTransmittance unit="%">5</UVTransmittance>
            <ConfidenceIndex></ConfidenceIndex>
        </S>
        <S>
            <Error>MEASUREMENT ERROR</Error>
        </S>
        <R>
            <Sphere unit="D">-2.00</Sphere>
            <Cylinder unit="D">-1.50</Cylinder>
            <Axis unit="deg">143</Axis>
            <SE unit="D"></SE>
            <ADD unit="D">1.75</ADD>
            <ADD2 unit="D"></ADD2>
            <NearSphere unit="D">0.00</NearSphere>
            <NearSphere2 unit="D"></NearSphere2>
            <Prism unit="pri">2.25</Prism>
            <PrismBase unit="deg">77</PrismBase>
            <PrismX unit="pri" base="in">0.50</PrismX>
            <PrismY unit="pri" base="up">2.00</PrismY>
            <UVTransmittance unit="%">5</UVTransmittance>
            <ConfidenceIndex></ConfidenceIndex>
        </R>
        <L>
            <Sphere unit="D">-2.00</Sphere>
            <Cylinder unit="D">-1.50</Cylinder>
            <Axis unit="deg">148</Axis>
            <SE unit="D"></SE>
            <ADD unit="D">1.75</ADD>
            <ADD2 unit="D"></ADD2>
            <NearSphere unit="D">-0.25</NearSphere>
            <NearSphere2 unit="D"></NearSphere2>
            <Prism unit="pri">1.50</Prism>
            <PrismBase unit="deg">61</PrismBase>
            <PrismX unit="pri" base="out">0.75</PrismX>
            <PrismY unit="pri" base="up">1.25</PrismY>
            <UVTransmittance unit="%">5</UVTransmittance>
            <ConfidenceIndex></ConfidenceIndex>
        </L>
    </LM>
    <PD>
        <Distance unit="mm">70.5</Distance>
        <DistanceR unit="mm">35.5</DistanceR>
        <DistanceL unit="mm">35.0</DistanceL>
    </PD>
    <NIDEK>
        <S>
            <Length unit="mm">23</Length>
            <ChannelWidth unit="mm">18</ChannelWidth>
            <ChannelLength unit="mm">23</ChannelLength>
            <Index></Index>
            <GreenTransmittance unit="%"></GreenTransmittance unit="%">
        </S>
        <R>
            <Length unit="mm">23</Length>
            <ChannelWidth unit="mm">18</ChannelWidth>
            <ChannelLength unit="mm">23</ChannelLength>
            <Index></Index>
            <GreenTransmittance unit="%"></GreenTransmittance unit="%">
        </R>
        <L>
            <Length unit="mm">22</Length>
            <ChannelWidth unit="mm">15</ChannelWidth>
            <ChannelLength unit="mm">20</ChannelLength>
            <Index></Index>
            <GreenTransmittance unit="%"></GreenTransmittance unit="%">
        </L>
        <NetPrism>
            <NetHPrism unit="pri" base="in">3</NetHPrism>
            <NetVPrism unit="pri" base="out">2</NetVPrism>
        </NetPrism>
        <Inside>
            <InsideR unit="mm">2.0</InsideR>
            <InsideL unit="mm">1.0</InsideL>
        </Inside>
    </NIDEK>
</Measure>
</Ophthalmology>
*/
    Logs::LogToFile("MesuresFronto.txt", docxml.toByteArray());
    QString nameLM = m_settings->value(Param_Poste_Fronto).toString();
    if (nameLM =="NIDEK LM-1800" || nameLM =="NIDEK LM-500" || nameLM =="NIDEK LM-7" || nameLM =="TOMEY TL-6000" || nameLM =="TOMEY TL-7000" || nameLM =="RODENSTOCK AL 6600")
    {
        QDomElement xml = docxml.documentElement();
        for (int h=0; h<xml.childNodes().size(); h++)
        {
            QDomElement childnode = xml.childNodes().at(h).toElement();
            if (childnode.tagName() == "Measure")
            {
                for (int g=0; g<childnode.childNodes().size(); g++)
                {
                    QDomElement measure = childnode.childNodes().at(g).toElement();
                    if (measure.tagName() == "LM")
                    {
                        for (int i=0; i<measure.childNodes().size(); i++)
                        {
                            QDomElement lm = measure.childNodes().at(i).toElement();
                            if (lm.tagName() == "R")                        /*! OEIL DROIT  ------------------------------------------------------------------*/
                            {
                                for (int l=0; l<lm.childNodes().size(); l++)
                                {
                                    QDomElement valnode = lm.childNodes().at(l).toElement();
                                    if (valnode.tagName() == "Sphere")
                                        Datas::I()->mesurefronto->setsphereOD(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                    if (valnode.tagName() == "Cylinder")
                                        Datas::I()->mesurefronto->setcylindreOD(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                    if (valnode.tagName() == "Axis")
                                        Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(valnode.text().toInt()));
                                    if (valnode.tagName() == "ADD")
                                        Datas::I()->mesurefronto->setaddVPOD(Utils::roundToNearestFive(valnode.text().toInt()));
                                }
                            }
                            if (lm.tagName() == "L")                        /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                            {
                                for (int l=0; l<lm.childNodes().size(); l++)
                                {
                                    QDomElement valnode = lm.childNodes().at(l).toElement();
                                    if (valnode.tagName() == "Sphere")
                                        Datas::I()->mesurefronto->setsphereOG(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                    if (valnode.tagName() == "Cylinder")
                                        Datas::I()->mesurefronto->setcylindreOG(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                    if (valnode.tagName() == "Axis")
                                        Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(valnode.text().toInt()));
                                    if (valnode.tagName() == "ADD")
                                        Datas::I()->mesurefronto->setaddVPOG(Utils::roundToNearestFive(valnode.text().toInt()));
                                }
                            }
                        }
                    }
                    if (measure.tagName() == "PD")                          /*! ECART INTERPUPILLAIRE  --------------------------------------------------------*/
                    {
                        for (int i=0; i<measure.childNodes().size(); i++)
                        {
                            QDomElement PD = measure.childNodes().at(i).toElement();
                            if (PD.tagName() == "Distance")
                                Datas::I()->mesurefronto->setecartIP(PD.text().toInt());
                        }
                    }
                }
            }
        }
    }
    debugMesure(Datas::I()->mesurefronto);
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
    Reponse = HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>"
                           + tr("Autoref") + ":</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + Reponse + "</td>";
    if (autoref->ecartIP() >0)
        Reponse += "<td width=\"60\"><font color = " COULEUR_TITRES "><b>"
                + tr("EIP") + ":</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + QString::number(autoref->ecartIP()) + "mm</td>";
    Reponse += "</p>";
    return Reponse;
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
        QString mDioptrKOD  = Utils::PrefixePlus(Utils::roundToNearestPointTwentyFive(kerato->dioptriesKOD()));
        QString mDioptrmOD  = QString::number(kerato->dioptriesKMOD(),'f',1);
        if (QLocale().toDouble(mDioptrK1OD)!=0.0)
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"160\">" + mK1OD + "/" + mK2OD + " - Km = " + mKOD + "</td>"
                      "<td width=\"240\">" + mDioptrK1OD + "/" + mDioptrK2OD + " -> <font color = \"" COULEUR_TITRES "\">" + mDioptrKOD +  tr(" à ") + mAxeKOD + "°</font> - Km = " + mDioptrmOD + "D</td></p>";
        else
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td>"
                      "<td width=\"180\">" + mK1OD + tr(" à ") + mAxeKOD + "°/" + mK2OD
                      + " - Km = " + mKOD + "</td></p>";
    }
    if (!kerato->isnullLOG())
    {
        QString mK1OG       = QLocale().toString(kerato->K1OG(),'f',2);
        QString mK2OG       = QLocale().toString(kerato->K2OG(),'f',2);
        QString mKOG        = QLocale().toString(kerato->KMOG(),'f',2);
        QString mAxeKOG     = QString::number(kerato->axeKOG());
        QString mDioptrK1OG = QLocale().toString(kerato->dioptriesK1OG(),'f',1);
        QString mDioptrK2OG = QLocale().toString(kerato->dioptriesK2OG(),'f',1);
        QString mDioptrKOG  = Utils::PrefixePlus(Utils::roundToNearestPointTwentyFive(kerato->dioptriesKOG()));
        QString mDioptrmOG  = QString::number(kerato->dioptriesKMOG(),'f',1);
        if (QLocale().toDouble(mDioptrK1OG)!=0.0)
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"160\">" + mK1OG + "/" + mK2OG + " - Km = " + mKOG + "</td>"
                      "<td width=\"240\">" + mDioptrK1OG + "/" + mDioptrK2OG + " -> <font color = \"" COULEUR_TITRES "\">" + mDioptrKOG +  tr(" à ") + mAxeKOG + "°</font> - Km = " + mDioptrmOG + "D</td></p>";
        else
            Reponse += HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td>"
                      "<td width=\"180\">"  + mK1OG +  tr(" à ") + mAxeKOG + "°/" + mK2OG
                      + " - Km = " + mKOG + "</td></p>";
    }

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
        if (tono->TODcorrigee() >0 || tono->TOGcorrigee() >0)
            larg = "330";
        QString title = HTML_RETOURLIGNE "<td width=\"" + larg + "\"><b><font color = \"" COULEUR_TITRES "\">";
        QString Methode = Tonometrie::ConvertMesure(tono->modemesure());
        QString Tono, color, Tonocor, colorcor;
        if (tono->TOD() == 0 && tono->TOG() > 0)
        {
            color = (tono->TOG() > 21? "red" : "blue");
            QString TOGcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOG()) + "</font>";
            if (tono->TOGcorrigee() >0)
            {
                color = (tono->TOGcorrigee() > 21? "red" : "blue");
                TOGcolor += " (/<font color = \"" + color + "\">" + QString::number(tono->TOGcorrigee()) + " " + tr("corr.") + "</font>)";
            }
            Tono = dd + tr("TOG:") + "</font> "
                    + TOGcolor + "</b> " + tr("à") + " "
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        }
        else if (tono->TOG() == 0 && tono->TOD() > 0)
        {
            color = (tono->TOD() > 21? "red" : "blue");
            QString TODcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOD()) + "</font>";
            if (tono->TODcorrigee() >0)
            {
                color = (tono->TODcorrigee() > 21? "red" : "blue");
                TODcolor += " (<font color = \"" + color + "\">" + QString::number(tono->TODcorrigee()) + " " + tr("corr.") + "</font>)";
            }
            Tono = dd + tr("TOD:") + "</font> "
                    + TODcolor + "</b> " + tr("à") + " "
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        }
        else if (tono->TOD() == tono->TOG() && tono->TODcorrigee() == tono->TOGcorrigee())
        {
            color = (tono->TOD() > 21? "red" : "blue");
            QString TODcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOD()) + "</font>";
            if (tono->TODcorrigee() >0)
            {
                color = (tono->TODcorrigee() > 21? "red" : "blue");
                TODcolor += " (/<font color = \"" + color + "\">" + QString::number(tono->TODcorrigee()) + " " + tr("corr.") + "</font>)";
            }
            Tono = dd + tr("TODG:") + "</font> "
                    + TODcolor + "</b> " + tr("à") + " "
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        }
        else
        {
            color = (tono->TOD() > 21? "red" : "blue");
            QString TODcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOD()) + "</font>";
            color = (tono->TOG() > 21? "red" : "blue");
            QString TOGcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOG()) + "</font>";
            Tono = dd + tr("TO:") + "</font> " + TODcolor + " / " + TOGcolor;
            if (tono->TODcorrigee() >0 || tono->TOGcorrigee() >0)
            {
                color = (tono->TODcorrigee() > 21? "red" : "blue");
                TODcolor = "<font color = \"" + color + "\">" + QString::number(tono->TODcorrigee()) + "</font>";
                color = (tono->TOGcorrigee() > 21? "red" : "blue");
                TOGcolor = "<font color = \"" + color + "\">" + QString::number(tono->TOGcorrigee()) + "</font>";
                Tono += " (" + TODcolor + " / " + TOGcolor + " " + tr("corr.") + ")";
            }
            Tono += "</b> " + tr("à") + " "
                    + QTime::currentTime().toString("H")
                    + "H (" + Methode + ")</td><td>"
                    + currentuser()->login();
        }
        Reponse = title + Tono + "</td></p>";
        Reponse.insert(Reponse.lastIndexOf("</td>")-1, fd);             //! on met le dernier caractère en ancre
    }
    logmesure("setHtmlTono() -> new m_htmlMesureTono = " + Reponse);
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
    return Reponse;
}

QSerialPort* Procedures::PortTono()
{
    return sp_portTono;
}

bool Procedures::HasAppareilRefractionConnecte()
{
    return m_hasappareilrefractionconnecte;
}

Procedures::TypeMesure Procedures::ConvertMesure(QString Mesure)
{
    if (Mesure == "P") return MesureFronto;
    if (Mesure == "A") return MesureAutoref;
    if (Mesure == "R") return MesureRefracteur;
    return  MesureNone;
}

QString Procedures::ConvertMesure(TypeMesure Mesure)
{
    switch (Mesure) {
    case MesureFronto:        return "P";
    case MesureAutoref:       return "A";
    case MesureRefracteur:    return "R";
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
    if (Datas::I()->Datas::I()->actes->currentacte() == Q_NULLPTR)
        return;
    int idPatient   = Datas::I()->patients->currentpatient()->id();
    int idActe      = Datas::I()->actes->currentacte()->id();
    if (typemesure == MesureFronto)
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
        for (auto it = Datas::I()->refractions->refractions()->begin(); it != Datas::I()->refractions->refractions()->end();)
        {
            Refraction *ref = const_cast<Refraction*>(it.value());
            if (ref->idacte() == idActe
                    && ref->typemesure() == Refraction::Fronto
                    && ref->formuleOD() == CalculeFormule(Datas::I()->mesurefronto,"D")
                    && ref->formuleOG() == CalculeFormule(Datas::I()->mesurefronto,"G"))
            {
                DataBase::I()->SupprRecordFromTable(ref->id(), CP_ID_REFRACTIONS, TBL_REFRACTIONS);
                delete ref;
                it = Datas::I()->refractions->refractions()->erase(it);
            }
            else
                ++it;
        }

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
    else if (typemesure == MesureAutoref)
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
//        for (auto it = Datas::I()->refractions->refractions()->constBegin(); it != Datas::I()->refractions->refractions()->constEnd(); ++it)
//        {
//            Refraction *ref = const_cast<Refraction*>(it.value());
//            if (ref->idacte() == idActe && ref->typemesure() == Refraction::Autoref)
//            {
//                Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(ref->id()));
//                //++it;
//            }
//        }
        for (auto it = Datas::I()->refractions->refractions()->begin(); it != Datas::I()->refractions->refractions()->end();)
        {
            Refraction *ref = const_cast<Refraction*>(it.value());
            if (ref->idacte() == idActe && ref->typemesure() == Refraction::Autoref)
            {
                DataBase::I()->SupprRecordFromTable(ref->id(), CP_ID_REFRACTIONS, TBL_REFRACTIONS);
                delete ref;
                it = Datas::I()->refractions->refractions()->erase(it);
            }
            else
                ++it;
        }

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
    else if (typemesure == MesureKerato)
    {
        QString req = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(MesureAutoref) + "'";
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
                    ConvertMesure(MesureAutoref) + "', "
                    "CURDATE(), " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->K1OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->K2OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->axeKOD()))       + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->K1OG(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->K2OG(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->axeKOG()))       + ", " +
                    "'" + ConvertMesure(MesureAutoref) + "', " +
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
                    CP_MODEMESUREKERATO_DATAOPHTA " = '" + ConvertMesure(MesureAutoref) + "'";
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
            req += " where " CP_IDPATIENT_DATAOPHTA " = "+ QString::number(idPatient) + " and QuelleMesure = '" + ConvertMesure(MesureAutoref) + "'";
            db->StandardSQL (req, tr("Erreur de modification de données de kératométrie dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == MesureRefracteur)
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
        for (auto it = Datas::I()->refractions->refractions()->begin(); it != Datas::I()->refractions->refractions()->end();)
        {
            Refraction *ref = const_cast<Refraction*>(it.value());
            if (ref->idacte() == idActe && ref->typemesure() == Refraction::Acuite)
            {
                DataBase::I()->SupprRecordFromTable(ref->id(), CP_ID_REFRACTIONS, TBL_REFRACTIONS);
                delete ref;
                it = Datas::I()->refractions->refractions()->erase(it);
            }
            else
                ++it;
        }

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

        QString requete = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(MesureRefracteur) + "'";
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
                    " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(MesureRefracteur) + "'";

            db->StandardSQL (requete, tr("Erreur de mise à jour de données de refraction dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == MesureTono)
    {
        Datas::I()->mesuretono->setidpatient(Datas::I()->patients->currentpatient()->id());
        db->locktable(TBL_TONOMETRIE);
        QString req = "INSERT INTO " TBL_TONOMETRIE " (" CP_IDPAT_TONO ", " CP_TOD_TONO ", " CP_TOG_TONO ", " CP_TODATE_TONO ", " CP_TOTYPE_TONO ") VALUES  ("
                + QString::number(Datas::I()->mesuretono->idpatient()) + ","
                + QString::number(Datas::I()->mesuretono->TOD()) + ","
                + QString::number(Datas::I()->mesuretono->TOG())
                + ", now(), '" + Tonometrie::ConvertMesure(Datas::I()->mesuretono->modemesure()) + "')";
        db->StandardSQL(req,tr("Impossible de sauvegarder la mesure!"));
        bool ok;
        Datas::I()->mesuretono->setid(db->selectMaxFromTable(CP_ID_TONO, TBL_TONOMETRIE,ok));
        db->unlocktables();
    }
    else if (typemesure == MesurePachy)
    {
        Datas::I()->mesurepachy->setidpatient(Datas::I()->patients->currentpatient()->id());
        db->locktable(TBL_PACHYMETRIE);
        QString req = "INSERT INTO " TBL_PACHYMETRIE " (" CP_IDPAT_PACHY ", " CP_PACHYOD_PACHY ", " CP_PACHYOG_PACHY ", " CP_PACHYDATE_PACHY ", " CP_PACHYTYPE_PACHY ") VALUES  ("
                + QString::number(Datas::I()->mesurepachy->idpatient()) + ","
                + QString::number(Datas::I()->mesurepachy->pachyOD()) + ","
                + QString::number(Datas::I()->mesurepachy->pachyOG())
                + ", now(), '" + Pachymetrie::ConvertMesure(Datas::I()->mesurepachy->modemesure()) + "')";
        DataBase::I()->StandardSQL(req,tr("Impossible de sauvegarder la mesure!"));
        bool ok;
        Datas::I()->mesurepachy->setid(db->selectMaxFromTable(CP_ID_PACHY, TBL_PACHYMETRIE,ok));
        db->unlocktables();
    }
    if (typemesure != MesureFronto && typemesure != MesureTono && typemesure != MesurePachy)
        Datas::I()->patients->actualiseDonneesOphtaCurrentPatient();
}
