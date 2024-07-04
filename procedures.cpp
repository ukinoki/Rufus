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

#include "procedures.h"

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
     /*!
    QStringList driverslist = QSqlDatabase::drivers();
    for (int i = 0; i<driverslist.size(); ++i)
        qDebug() << driverslist.at(i);
    //*/
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
    m_CPpardefaut    = "";
    m_Villepardefaut = "";
    db               = DataBase::I();


    QFile FichierIni(PATH_FILE_INI);
    m_applicationfont = QFont(POLICEPARDEFAUT);
    Utils::CalcFontSize(m_applicationfont);
    qApp->setFont(m_applicationfont);
    //qDebug() << "qApp->font().pointSize()" << qApp->font().pointSize();

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
                                "2. Sinon cliquez sur \"Reconstruire le fichier d'initialisation\" et suivez les étapes de la reconstruction.\n"
                                " Il vous faudra alors compléter de nouveau\n"
                                " les renseignements concernant les appareils connectés au réseau ou à ce poste d'examen après\n"
                                " le démarrage complet du logiciel (Menu Edition/Paramètres).\n");
            m_connexionbaseOK = a;
            a = VerifIni(msg, msgInfo, true, true, true, true);
        }
    }
    m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);

    QList<int> lports ={ 3306 , 3307};
    QSet<int> ports =QSet<int>(lports.constBegin(), lports.constEnd());
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
                       "2. Sinon cliquez sur \"Reconstruire le fichier d'initialisation\" et suivez les étapes de la reconstruction.\n"
                       " Il vous faudra alors compléter de nouveau\n"
                       " les renseignements concernant les appareils connectés au réseau ou à ce poste d'examen après\n"
                       " le démarrage complet du logiciel (Menu Edition/Paramètres).\n");
            m_connexionbaseOK = k;
            k = VerifIni(msg, msgInfo, false, true, true, false);
        }
    }

    m_nomImprimante  = "";

    mapPortsCOM();
    Ouverture_Appareils_Refraction();
    ReconstruitListeModesAcces();
    m_typemesureRefraction               = GenericProtocol::MesureNone;
    m_dlgrefractionouverte    = false;
    int margemm         = TailleTopMarge(); // exprimé en mm
    m_printer             = new QPrinter(QPrinter::HighResolution);
    m_printer             ->setFullPage(true);
    m_rect                = m_printer->paperRect(QPrinter::Inch);
    m_rect.adjust(Utils::mmToInches(margemm) * m_printer->logicalDpiX(),
                  Utils::mmToInches(margemm) * m_printer->logicalDpiY(),
                - Utils::mmToInches(margemm) * m_printer->logicalDpiX(),
                - Utils::mmToInches(margemm) * m_printer->logicalDpiY());
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
    QString cle ("");
    switch (mode) {
    case Utils::ReseauLocal:
        cle = Utils::getBaseFromMode( mode );
        break;
    default:
        cle = Utils::getBaseFromMode( Utils::Poste );
        break;
    }
    cle += Dossier_Documents + Appareil;
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
        Datas::I()->users       ->initListe();
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
void Procedures::AskBupRestore(BkupRestore op, QString pathorigin, QString pathdestination, bool OKini, bool OKimages, bool OKvideos, bool OKfactures)
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
        Inichk->setObjectName("ini");
        layini->addWidget(Inichk);
        layini->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
        dlg_buprestore->dlglayout()->insertLayout(0, layini);

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
            Videoschk->setObjectName("videos");
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
            Imgeschk->setObjectName("images");
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
            Fctureschk->setObjectName("factures");
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
    BDDchk->setObjectName("base");
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

bool Procedures::Backup(QString pathdirdestination, bool OKBase, bool OKImages, bool OKVideos, bool OKFactures, bool verifmdp, QWidget *parent)
{
    auto result = [] (qintptr handle, Procedures *proc)
    {
        ShowMessage::I()->ClosePriorityMessage(handle);
        emit proc->ConnectTimers(true);
    };

    QString msgEchec = tr("Incident pendant la sauvegarde");
    QString msg ("");
    qintptr handledlg = 0;
    if (verifmdp)
    {
        QString mdp("");
        if (!Utils::VerifMDP(MDPAdmin(),tr("Saisissez le mot de passe Administrateur"), mdp, false, parent))
            return false;
    }
    ShowMessage::I()->PriorityMessage(tr("Sauvegarde de la base Rufus"),handledlg);
    emit ConnectTimers(false);

    //On vide les champs blob de la table factures et la table EchangeImages
    db->StandardSQL("UPDATE " TBL_FACTURES " SET " CP_JPG_FACTURES " = null, " CP_PDF_FACTURES " = null");
    db->StandardSQL("DELETE FROM " TBL_ECHANGEIMAGES);
    Utils::mkpath(pathdirdestination);

    if (OKBase)
    {
        QString pathbackupbase = pathdirdestination + "/" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmm");
        Utils::mkpath(pathbackupbase);

        /*! sauvegarde de la base */
        QFile::remove(PATH_FILE_SCRIPTBACKUP);
        DefinitScriptBackup(pathbackupbase);
#ifdef Q_OS_WIN
        const QString task = QDir::toNativeSeparators(PATH_FILE_SCRIPTBACKUP);
#else
        const QString task = "sh " + PATH_FILE_SCRIPTBACKUP;
#endif
        msg += tr("Base de données sauvegardée!\n");
        m_ostask.disconnect(SIGNAL(result(const int &)));
        connect(&m_ostask, &OsTask::result, this, [=](int a) {
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0? msg : msgEchec), Icons::icSunglasses(), 3000);
            result(handledlg, this);
            QFile::remove(PATH_FILE_SCRIPTBACKUP);
            return true;
        });
        m_ostask.execute(task);

        /*! élimination des anciennes sauvegardes */
        QDir dir(pathdirdestination);
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList list = dir.entryInfoList();
        for(int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.fileName().split("-").size()>0)
            {
                QString date    = fileInfo.fileName().split("-").at(0);
                int year        = date.left(4).toInt();
                int month       = date.mid(4,2).toInt();
                int day         = date.right(2).toInt();
                QDate birthtime = QDate(year,month,day);
                if (birthtime.isValid())
                    if (birthtime.daysTo(QDateTime::currentDateTime().date()) > 14)
                    {
                        if (fileInfo.isDir())
                            QDir(fileInfo.absoluteFilePath()).removeRecursively();
                        else if (fileInfo.isFile())
                            QFile(fileInfo.absoluteFilePath()).remove();
                    }
            }
        }

        /*! sauvegarde de Rufus.ini et des fichiers ressources */
        QFile file = QFile(PATH_FILE_INI);
        Utils::copyWithPermissions(file, pathbackupbase + "/" NOM_FILE_INI);
        msg += tr("Fichier de paramétrage Rufus.ini sauvegardé\n");
    }
    if (OKImages || OKVideos || OKFactures)
    {
        QString dirNomSource;
        QString dirNomDest;

        if (OKFactures) {
            dirNomSource = db->dirimagerie() + NOM_DIR_FACTURES;
            dirNomDest = pathdirdestination + NOM_DIR_FACTURES;
            int t = 0;
            Utils::countFilesInDirRecursively(dirNomSource, t);
            UpProgressDialog *progdial = new UpProgressDialog(0,t, parent);
            progdial->show();
            int n = 0;
            Utils::copyfolderrecursively(dirNomSource, dirNomDest, n, tr("Sauvegarde des factures"), progdial);
            delete progdial;
            msg += tr("Factures sauvegardées\n");
        }
        if (OKImages) {
            dirNomSource = db->dirimagerie() + NOM_DIR_IMAGES;
            dirNomDest = pathdirdestination + NOM_DIR_IMAGES;
            int t = 0;
            Utils::countFilesInDirRecursively(dirNomSource, t);
            UpProgressDialog *progdial = new UpProgressDialog(0,t, parent);
            progdial->show();
            int n = 0;
            Utils::copyfolderrecursively(dirNomSource, dirNomDest, n, tr("Sauvegarde des fichiers d'imagerie"), progdial);
            delete progdial;
            msg += tr("Fichiers imagerie sauvegardés\n");
        }
        if (OKVideos) {
            dirNomSource = db->dirimagerie() + NOM_DIR_VIDEOS;
            dirNomDest = pathdirdestination + NOM_DIR_VIDEOS;
            int t = 0;
            Utils::countFilesInDirRecursively(dirNomSource, t);
            UpProgressDialog *progdial = new UpProgressDialog(0,t, parent);
            progdial->show();
            int n = 0;
            Utils::copyfolderrecursively(dirNomSource, dirNomDest, n, tr("Sauvegarde des videos"), progdial);
            delete progdial;
            msg += tr("Fichiers video sauvegardés");
        }
    }
    else
    {
        result(handledlg, this);
        return false;
    }
    qintptr z = 0;
    ShowMessage::I()->PriorityMessage(msg,z, 10000);
    return true;
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

void Procedures::DefinitScriptBackup(QString pathbackupbase)
{
    if (!QDir(pathbackupbase).exists())
        if (!Utils::mkpath(pathbackupbase))
            return;
    if (!QDir(pathbackupbase).exists())
        return;
#ifdef Q_OS_WIN
    QString CRLF="\r\n";
    QString executabledump = QDir::toNativeSeparators(dirSQLExecutable()+ "/mysqldump.exe");
    QString scriptbackup;
#else
    QString CRLF="\n";
    QString executabledump = QDir::toNativeSeparators(dirSQLExecutable() + "/mysqldump");
    QString scriptbackup= "#!/bin/bash";
    scriptbackup += CRLF;
#endif
    // élaboration du script de backup
    // Sauvegarde des 4 bases de Rufus
    scriptbackup += executabledump + " --force --opt --user=\"" LOGIN_SQL "\" -p\"" MDP_SQL "\" --skip-lock-tables --events --databases " DB_RUFUS " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_RUFUS ".sql") + "\"";
    scriptbackup += CRLF;
    scriptbackup += executabledump + " --force --opt --user=\"" LOGIN_SQL "\" -p\"" MDP_SQL "\" --skip-lock-tables --events --databases " DB_COMPTA " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_COMPTA ".sql") + "\"";
    scriptbackup += CRLF;
    scriptbackup += executabledump + " --force --opt --user=\"" LOGIN_SQL "\" -p\"" MDP_SQL "\" --skip-lock-tables --events --databases " DB_IMAGES " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_IMAGES ".sql") + "\"";
    scriptbackup += CRLF;
    scriptbackup += executabledump + " --force --opt --user=\"" LOGIN_SQL "\" -p\"" MDP_SQL "\" --skip-lock-tables --events --databases " DB_OPHTA " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_OPHTA ".sql") + "\"";
    scriptbackup += CRLF;
    // Sauvegarde de la table des utilisateurs
    scriptbackup += executabledump + " --force --opt --user=\"" LOGIN_SQL "\" -p\"" MDP_SQL "\" mysql user > \"" + QDir::toNativeSeparators(pathbackupbase + "/user.sql") + "\"";
    scriptbackup += CRLF;

    if (QFile::exists(PATH_FILE_SCRIPTBACKUP))
        QFile::remove(PATH_FILE_SCRIPTBACKUP);
    QFile fbackup(PATH_FILE_SCRIPTBACKUP);
    if (fbackup.exists())
        Utils::removeWithoutPermissions(fbackup);
    if (fbackup.open(QIODevice::ReadWrite))
    {
        QTextStream out(&fbackup);
        out << scriptbackup ;
        fbackup.close();
    }
}

/*!
 * \brief Procedures::sqlExecutable
 * \return le chemin vers les éxécutable mysql et mysqldump
 * Le chemin est stocké dans Rufus.ini au format Unix avec des "/"
 * Pour le retrouver au format natif, on lui applique la fonction QDir::toNativeSeparators()
 */
QString Procedures::dirSQLExecutable()
{
    if (m_dirSQLExecutable == "")
        setDirSQLExecutable();
    return m_dirSQLExecutable;
}

/*!
 * \brief Procedures::setDirSQLExecutable
 * La fonction recherche les éxécutables SQL: mysql et mysqldump
 * Elle les recherche d'abord dans le package logiciel
 * puis dans Rufus.ini
 * puis dans les standardpaths du système
 * Si elle ne les trouve dans aucun de ces 3 endroits, elle interroge l'utilisateur
 * et si l'utilisateur l'informe qu'il ne peut pas trouver les executables
 * le programme est quand même lancé en informant l'utilisateur qu'il ne pourra faire aucune opération de restauration, sauvegarde ou mise à jour de la base
*/
void Procedures::setDirSQLExecutable()
{
    QString dirdefaultsqlexecutable ("");
    QString dirsqlexecutable ("");
    m_executable        = "/mysql";
    m_dumpexecutable    = "/mysqldump";
    bool a = false;

/*! 1. On recherche dans le package logiciel */
/*! ne marche pas sous Mac Silicon pour le moment */
#ifdef Q_OS_MACOS
    QDir mysqldir           = QDir(QCoreApplication::applicationDirPath());
    mysqldir.cdUp();
    dirdefaultsqlexecutable = mysqldir.absolutePath() + "/Applications";
    a                       = QFile(dirdefaultsqlexecutable + m_executable).exists();
#endif

#ifdef Q_OS_WIN
    m_executable            = "/mysql.exe";
    m_dumpexecutable        = "/mysqldump.exe";
    QDir mysqldir           = QDir(QCoreApplication::applicationDirPath());
    dirdefaultsqlexecutable = mysqldir.absolutePath() + "/Applications";
    a                       = QFile(dirdefaultsqlexecutable + m_executable).exists();
#endif
    if (a)
    {
        if (dirdefaultsqlexecutable != "")
            settings()->setValue(Param_SQLExecutable, dirdefaultsqlexecutable);
        m_dirSQLExecutable = dirdefaultsqlexecutable;
        return;
    }

/*! 2. on recherche dans les chemins habituels du système */
#ifdef Q_OS_MACOS
    dirsqlexecutable = "/usr/local/mysql/bin";
    if (!QFile(dirsqlexecutable + "/mysql").exists())
    {
        dirsqlexecutable = "/usr/local/bin";
        if (!QFile(dirsqlexecutable + "/mysql").exists())
            dirsqlexecutable = "/opt/homebrew/opt/mariadb/bin";
    }
    a = (QFile(dirsqlexecutable + "/mysql").exists());
#endif
#ifdef Q_OS_LINUX
    dirsqlexecutable = "/usr/bin";
    a = (QFile(dirsqlexecutable + m_executable).exists());
#endif
#ifdef Q_OS_WIN
    QDir programs = QDir("C:/Program Files");
    if (programs.exists())
    {
        QStringList listmariadbdirs = programs.entryList(QStringList() << "MariaDB *", QDir::Dirs);
        if (listmariadbdirs.size() > 0)
        {
            dirsqlexecutable = programs.absolutePath() + "/"  + listmariadbdirs.at(0) + "/bin";
            a = (QFile(dirsqlexecutable + m_executable).exists());
        }
        if (!a)
        {
            programs = QDir("C:/Program Files/MySQL");
            if (programs.exists())
            {
                QStringList listmysqldirs = programs.entryList(QStringList() << "MySQL Server *", QDir::Dirs);
                if (listmysqldirs.size() > 0)
                {
                    dirsqlexecutable = programs.absolutePath() + "/"  + listmysqldirs.at(0) + "/bin";
                    a = (QFile(dirsqlexecutable + m_executable).exists());
                }
            }
        }
    }
#endif

    if (a)
    {
        settings()->setValue(Param_SQLExecutable, dirsqlexecutable);
        m_dirSQLExecutable = dirsqlexecutable;
        return;
    }

/*! 3. On n'a rien trouvé - on teste la valeur enregistrée dans rufus.ini */

    dirsqlexecutable = settings()->value(Param_SQLExecutable).toString();
    if (QFile(dirsqlexecutable + m_executable).exists())
    {
        m_dirSQLExecutable = dirsqlexecutable;
        return;
    }

/*! 4. On n'a rien trouvé - on interroge l'utilisateur */

    UpMessageBox::Information(Q_NULLPTR,
                              tr("le chemin des programmes mysql et mysqldump (") + dirsqlexecutable + ") n'est pas valide"),
                              tr("Choisissez un dossier valide dans la boîte de dialogue suivante");
        while (!a)
    {

        QString urlexecutabledir = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                          tr("Choisissez le dossier dans lequel se trouvent les executables mysql et mysqldump"),
                                                          (QDir::rootPath()));
        QString path = urlexecutabledir + m_executable;
        if (!QFile(path).exists())
        {
            if (UpMessageBox::Question(Q_NULLPTR,
                                       tr("le chemin choisi (") + urlexecutabledir + tr(") n'est pas valide"),
                                       tr("Voulez vous annuler?") + "\n" +tr("Si vous annulez, la fonction demandée ne pourra pas s'éxécuter!"),
                                       UpDialog::ButtonCancel | UpDialog::ButtonOK,
                                       QStringList() << tr("Annuler") << tr("Reprendre"))
                != UpSmallButton::STARTBUTTON)
            {
                settings()->remove(Param_SQLExecutable);
                return;
            }
        }
        else
        {
            dirsqlexecutable = urlexecutabledir;
            a = true;
        }
    }
    if (dirsqlexecutable != settings()->value(Param_SQLExecutable).toString())
        settings()->setValue(Param_SQLExecutable, dirsqlexecutable);
    m_dirSQLExecutable = dirsqlexecutable;
}

/*!
 * \brief Procedures::dirSSLKey
 * \return le chemin vers le dossier des clés SSL
 */
QString Procedures::dirSSLKeys()
{
    if (m_dirSSLkeys == "")
        setDirSSLKeys();
    return m_dirSSLkeys;
}

void Procedures::setDirSSLKeys()
{
    QUrl urlkeys = QUrl();
    QString dirkeys = settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString();
    if (dirkeys == "")
    {
        dirkeys = "/etc/mysql";
        urlkeys.setPath(QDir::toNativeSeparators(dirkeys));
        bool a = urlkeys.isValid();
        if (!a)
            UpMessageBox::Information(Q_NULLPTR,
                                      tr("le chemin par défaut (") + dirkeys + ") n'est pas valide"),
                                      tr("Choisissez un dossier valide dans la boîte de dialogue suivante");
        while (!a)
        {
            urlkeys = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                       tr("Choisissez le dossier dans lequel se trouvent les clés SSL"),
                                                       (QDir::rootPath()));
            if (urlkeys == QUrl())
            {
                if (UpMessageBox::Question(Q_NULLPTR,
                                           tr("le chemin choisi (") + urlkeys.path() + tr(") n'est pas valide"),
                                           tr("Voulez vous annuler?") + "\n" +tr("Si vous annulez, la fonction demandée ne pourra pas s'éxécuter!"),
                                           UpDialog::ButtonCancel | UpDialog::ButtonOK,
                                           QStringList() << tr("Annuler") << tr("Reprendre"))
                        != UpSmallButton::STARTBUTTON)
                    return;
                else
                {
                    dirkeys = urlkeys.path();
                    a = true;
                }
            }
            dirkeys = urlkeys.path();
        }
        if (dirkeys !=settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString())
            settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL, dirkeys);
    }
    m_dirSSLkeys = dirkeys;
}

/*!
 * \brief Procedures::ExecuteScriptSQL
 * \param ListScripts
 * Execute une liste de scripts SQL
 */
int Procedures::ExecuteScriptSQL(QStringList ListScripts)
{
    int a = 99;

    QString sqlCommand = QDir::toNativeSeparators(dirSQLExecutable()+ m_executable);
    QString host;
    if( db->ModeAccesDataBase() == Utils::Poste )
        host = "localhost";
    else
        host = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Serveur).toString();
    bool useSSL = (db->ModeAccesDataBase() == Utils::Distant);
    QString login = LOGIN_SQL;
    if (useSSL)
         login += "SSL";
     QString keys = "";
     if (useSSL)
     {
         QString dirkey = QDir::toNativeSeparators(dirSSLKeys());
         keys += " --ssl-ca=" + dirkey + "/ca-cert.pem --ssl-cert=" + dirkey + "/client-cert.pem --ssl-key=" + dirkey + "/client-key.pem";
     }
     QStringList args = QStringList()
        << "-u" << login
        << "-p" MDP_SQL
        << "-h" << host
        << "-P" << QString::number(db->port());
#ifndef Q_OS_WIN
    for (int i = 0; i < args.size() ; ++i)
        sqlCommand += " " + args.at(i);
#endif

    for (int i=0; i<ListScripts.size(); i++)
        if (QFile(ListScripts.at(i)).exists())
        {
            QString path = ListScripts.at(i);
            QProcess dumpProcess(parent());
#ifdef Q_OS_WIN
            dumpProcess.setStandardInputFile(path);
            dumpProcess.start(sqlCommand, args);
#else
            /*! les commandes start ou startDetached ne fonctionnent pas sous MacOS ou Ubuntu */
            /*! https://www.qtcentre.org/threads/23460-QProcess-and-mysql-lt-backup-sql
             *  dumpProcess.setStandardInputFile(path);
             *  dumpProcess.start(sqlCommand, args);                    NE MARCHE PLUS DEPUIS Qt6 sous MacOS ou Linux */
            QString command = sqlCommand + " < " + path;
            QString bat = "bash -c \"" + command + "\"";
            dumpProcess.startCommand(bat);
#endif
            dumpProcess.waitForFinished(1000000);


            if (dumpProcess.error() == QProcess::FailedToStart)
            {
                Logs::ERROR(tr("Impossible de lancer le processus de chargement de la base de données à partir du fichier \"%1\"").arg(path));
                a = 99;
                break;
            }
            if (dumpProcess.exitStatus() == QProcess::NormalExit)
                a = dumpProcess.exitCode();
            else
            {
                Logs::ERROR(tr("Le processus de chargement de la base de données à partir du fichier \"%1\" a échoué").arg(path));
                break;
            }

            if (a != 0)
                break;
        }
    return a;
}

/*!
 *  \brief ImmediateBackup()
 *  lance une sauvegarde immédiate de la base
 */
bool Procedures::ImmediateBackup(QString dirdestination, bool verifposteconnecte, bool full, QWidget *parent)
{
    if (!verifposteconnecte)
        if (AutresPostesConnectes())
            return false;

    if (dirdestination == "")
    {
        QString dirSauv = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                            tr("Choisissez le dossier dans lequel vous voulez sauvegarder la base"),
                                                            (QDir(m_parametres->dirbkup()).exists()? m_parametres->dirbkup() : QDir::homePath()));
        if (dirSauv == "")
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
        OKImages = QDir(db->dirimagerie()).exists();
        OKVideos = QDir(db->dirimagerie()).exists();
        OKFactures = QDir(db->dirimagerie()).exists();
    }
    else
    {
        AskBupRestore(BackupOp, db->dirimagerie(), dirdestination );
        if (dlg_buprestore->exec() != QDialog::Accepted)
            return false;
        QList<UpCheckBox*> listchk = dlg_buprestore->findChildren<UpCheckBox*>();
        for (int i= 0; i<listchk.size(); i++)
        {
            if (listchk.at(i)->objectName() == "base")
                OKbase = listchk.at(i)->isChecked();
            else if (listchk.at(i)->objectName() == "images")
                OKImages = listchk.at(i)->isChecked();
            else if (listchk.at(i)->objectName() == "videos")
                OKVideos = listchk.at(i)->isChecked();
            else if (listchk.at(i)->objectName() == "factures")
                OKFactures = listchk.at(i)->isChecked();
        }
    }
    if (!OKbase && !OKImages && !OKVideos && !OKFactures)
        return false;
    return Backup(dirdestination, OKbase, OKImages, OKVideos, OKFactures, true, parent);
}

void Procedures::EffaceBDDDataBackup()
{
    db->setdaysbkup(Utils::Day::Aucun);
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
#ifdef Q_OS_MACOS
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
#ifdef Q_OS_MACOS
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
//! fin sauvegardes
//--------------------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------
    Retourne le corps du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::CalcCorpsImpression(QString text, bool ALD)
{
    QString textcorps = (ALD? Ressources::I()->BodyOrdoALD() : Ressources::I()->BodyOrdo());

    QRegularExpression rx;
    rx.setPattern("font-size( *: *[\\d]{1,2} *)pt");
    text.replace(rx,"font-size:9pt");

    textcorps.replace("{{TEXTE ORDO}}",text);
    textcorps.replace("{{TEXTE ORDO HORS ALD}}", "");
    return textcorps;
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
    QString textentete;

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
        3 = Remplacant (remplaçant)
        4 = pas de comptabilite
    */
    if (user && user->isRemplacant())
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
                        " where (" CP_ENREGHONORAIRES_USR " = 1 or " CP_ENREGHONORAIRES_USR " = 2 or " CP_ENREGHONORAIRES_USR " = 5)"
                        " and " CP_ID_USR " <> " + QString::number(user->id()) +
                        " and " CP_SOIGNANTSTATUS_USR " = " + soigndata.at(0).toString() +
                        " and " CP_ISDESACTIVE_USR " is null";
                //qDebug() << req;
                QList<QVariantList> soignlist = db->StandardSelectSQL(req,m_ok);
                if (soignlist.size() == 1)               // une seule réponse, on la récupère
                    idparent   = soignlist.at(0).at(0).toInt();
                else                                // plusieurs réponses possibles, on va demander qui est le parent de ce remplaçant....
                {
                    dlg_askUser             = new UpDialog();
                    dlg_askUser             ->AjouteLayButtons();
                    QGroupBox*boxparent     = new QGroupBox();
                    dlg_askUser->dlglayout()->insertWidget(0,boxparent);
                    boxparent               ->setObjectName("Parent");
                    QString lblUsrParent    = tr("Qui enregistre les honoraires pour ") + user->login() + "?";
                    boxparent               ->setTitle(lblUsrParent);

                    QFontMetrics fm         = QFontMetrics(qApp->font());
                    int hauteurligne        = int(fm.height()*1.6);
                    boxparent               ->setFixedHeight(((soignlist.size() + 1)*hauteurligne)+5);
                    QVBoxLayout *vbox       = new QVBoxLayout;
                    for (int i=0; i<soignlist.size(); i++)
                    {
                        UpRadioButton *pradiobutt = new UpRadioButton(boxparent);
                        pradiobutt  ->setText(soignlist.at(i).at(1).toString());
                        pradiobutt  ->setiD(soignlist.at(i).at(0).toInt());
                        pradiobutt  ->setChecked(i==0);
                        vbox        ->addWidget(pradiobutt);
                    }
                    vbox     ->setContentsMargins(8,0,8,0);
                    boxparent->setLayout(vbox);
                    dlg_askUser ->setModal(true);
                    dlg_askUser->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

                    connect(dlg_askUser->OKButton, &QPushButton::clicked, dlg_askUser, &UpDialog::accept);

                    dlg_askUser->exec();

                    QList<UpRadioButton*> listbutt = boxparent->findChildren<UpRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                            idparent = listbutt.at(j)->iD();
                    delete dlg_askUser;
                    dlg_askUser = Q_NULLPTR;
                }
            }
        }
    }
    for (int i = 1; i<3; i++)
    {
        /*!
        nomModeleEntete = (i==1? PATH_FILE_ENTETEORDO : PATH_FILE_ENTETEORDOALD);
        QFile qFileEnTete(nomModeleEntete);
        if (!qFileEnTete.open( QIODevice::ReadOnly ))
            return QMap<QString, QString>();

        QByteArray  baEnTete        = qFileEnTete.readAll();
        qFileEnTete.close ();
        textentete = baEnTete;
        */

        textentete =  (i==1? Ressources::I()->HeaderOrdo() : Ressources::I()->HeaderOrdoALD());

        textentete.replace("{{POLICE}}", qApp->font().family());

        if (rplct)
        {
            User *userRemp = Datas::I()->users->getById(idparent);
            if(userRemp)
                textentete.replace("{{TITREUSER}}", "<s>" + userRemp->titre() + " " + userRemp->prenom() + " " + userRemp->nom() + "</s> "
                                                "<font color=\"darkblue\">" + tr ("remplacé par") + " "
                                                + (user->titre().size()? user->titre() + " " : "") + user->prenom() + " " + user->nom())
                                                + "</font>";
        }
        else
            textentete.replace("{{TITREUSER}}", (user->titre() != ""? user->titre() + " " : "") + user->prenom() + " " + user->nom());

        if(user->numspecialite() != 0)
            textentete.replace("{{SPECIALITE}}", QString::number(user->numspecialite()) + " " + user->specialite());
        else
            textentete.replace("{{SPECIALITE}}", user->specialite());
        if (i==1)
        {
            textentete.replace("{{LARGEURG}}", HTML_LARGEUR_ENTETE_GAUCHE);
            textentete.replace("{{LARGEURD}}", HTML_LARGEUR_ENTETE_DROITE);
        }
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
        textentete.replace("{{ADRESSE}}", adresse);
        textentete.replace("{{CPVILLE}}", sit->codepostal() + " " + sit->ville().toUpper());
        textentete.replace("{{TEL}}", "Tél. " + sit->telephone());
        if (nlignesadresse==2)
            textentete.replace("{{LIGNESARAJOUTER}}", "<span style=\"font-size:5pt;\"> <br /></span>");
        else
            textentete.replace("{{LIGNESARAJOUTER}}", "");

        QString NumSS = "";
        if( user->numOrdre().size() )
        {
            NumSS = user->numOrdre();
            if( user->NumPS() > 0 ) NumSS += " - ";
        }
        if (user->NumPS() > 0) NumSS += "RPPS " + QString::number(user->NumPS());
        textentete.replace("{{NUMSS}}",  db->parametres()->cotationsfrance()? NumSS : "");
        textentete.replace("{{DATE}}", sit->ville()  + tr(", le ") + QLocale::system().toString(date,tr("d MMMM yyyy")));
        Utils::epureFontFamily(textentete);

        (i==1? EnteteMap["Norm"] = textentete : EnteteMap["ALD"] = textentete);
    }
    return EnteteMap;
}

/*---------------------------------------------------------------------------------
    Retourne le pied du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::CalcPiedImpression(User *user, bool lunettes, bool ALD)
{
    QString textpied;
    if (ALD)
        textpied =  Ressources::I()->FooterOrdo();
    else
   {
        textpied = (lunettes? Ressources::I()->FooterOrdoLunettes() : Ressources::I()->FooterOrdo());
        if (lunettes)
            textpied.replace("{{METIER}}", (Datas::I()->users->userconnected()->metier()==User::Ophtalmo? tr("ophtalmologiste"):
                                            (Datas::I()->users->userconnected()->metier()==User::Orthoptiste)? tr("orthoptiste") : tr("optométriste")));
    }
    bool isaga = false;
    if (user)
    {
        User *parent = Datas::I()->users->getById(user->idparent());
        if (parent)
            isaga = parent->isAGA();
    }
    textpied.replace("{{AGA}}",(isaga?"Membre d'une association de gestion agréée - Le règlement des honoraires par chèque ou carte de crédit est accepté":""));
    return textpied;
}

bool Procedures::Imprime_Etat(QWidget *parent, QString textcorps, QString textentete, QString textpied,
                              int TaillePieddePage, int TailleEnTete, int TailleTopMarge,
                              bool AvecDupli, bool AvecNumPage, bool AvecChoixImprimante)
{
    TextPrinter *TexteAImprimer = new TextPrinter(parent);
    QTextEdit *Etat = new QTextEdit;
    Etat->setHtml(textcorps);
    QString PiedDepart = textpied;
    TexteAImprimer->setFooterSize(TaillePieddePage);
    TexteAImprimer->setHeaderText(textentete);

    if (TailleEnTete > 0)
        TexteAImprimer->setHeaderSize(TailleEnTete);
    else
        TexteAImprimer->setHeaderSize(25);
    textpied.replace("{{DUPLI}}","");
    if (!AvecNumPage)
        textpied.replace("&page;","");
    TexteAImprimer->setFooterText(textpied);
    TexteAImprimer->setTopMargin(TailleTopMarge);
    //QByteArray ba = TexteAImprimer->getPDFByteArray(Etat_textEdit->document());
    if (!AvecDupli)
        TexteAImprimer->setDuplex(QPrinter::DuplexLongSide);
    bool a = false;
    if (ApercuAvantImpression())
        a = TexteAImprimer->preview(Etat->document());
    else
    {
        if (!AvecChoixImprimante)
            TexteAImprimer->setPrinterName(m_nomImprimante);
        a = TexteAImprimer->print(Etat->document(), "", "", AvecChoixImprimante);
    }
    if (a)
        if (AvecDupli)
        {
            QString dupli = "<p align=\"center\"><span style=\"font-family:Arial Black;font-size:24pt;font-style:normal;font-weight:bold;color:#cccccc;\">DUPLICATA</span></p>";
            textpied = PiedDepart.replace("{{DUPLI}}",dupli);
            if (!AvecNumPage)
                textpied.replace("&page;","");
            TexteAImprimer->setFooterText(textpied);
            TexteAImprimer->setFooterSize(TexteAImprimer->footerSize() + 20);
            TexteAImprimer->print(Etat->document(),"","",false);
        }
    m_nomImprimante = TexteAImprimer->getPrinterName();
    delete TexteAImprimer;
    delete Etat;
    return a;
}

//----------------------------------------------------------------------------------------------------------------
//!    -- Choice printing : pdf or print ------------------------------------------------------------------------
//  --------------------------------------------------------------------------------------------------------------

bool Procedures::QuestionPdfOrPrint(QWidget *parent, bool &ok)
{
    UpMessageBox *msgbox            = new UpMessageBox(parent);
    msgbox                          ->setText(tr("Imprimer ou créer un pdf?"));
    msgbox                          ->setIcon(UpMessageBox::Quest);
    UpPushButton *wdg_annulbouton   = new UpPushButton(tr("Annuler"));
    UpPushButton *wdg_printbouton   = new UpPushButton(tr("Imprimer"));
    UpPushButton *wdg_pdfbouton     = new UpPushButton(tr("Créer un pdf"));
    wdg_annulbouton                 ->setIcon(Icons::icAnnuler());
    wdg_printbouton                 ->setIcon(Icons::icImprimer());
    wdg_pdfbouton                   ->setIcon(Icons::icPdf());

    wdg_annulbouton                 ->setData(UpPushButton::ANNULBUTTON);

    msgbox                          ->addButton(wdg_pdfbouton);
    msgbox                          ->addButton(wdg_printbouton);
    msgbox                          ->addButton(wdg_annulbouton);

    bool initok = (msgbox->exec() == QDialog::Accepted && msgbox->clickedpushbutton() != wdg_annulbouton);
    if (initok)
        ok = (msgbox->clickedpushbutton() == wdg_pdfbouton);
    delete msgbox;
    return initok;
}


/*!
 * \brief Procedures::Cree_pdf
 * \abstract Create pdf file from document
 * \param QString textcorps
 * \param QString textentete
 * \param QString textpied
 * \param QString nomfichier = nom du fichier
 * \param QString nomdossier = dossier où est enregistré le fichier
 * \return
 */
bool Procedures::Cree_pdf(QString textcorps, QString textentete, QString textpied, QString nomfichier, QString nomdossier)
{
    bool a = false;
    QTextEdit *Etat = new QTextEdit;
    Etat->setHtml(textcorps);
    if (nomdossier == "")
        nomdossier = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0));
    Utils::mkpath(nomdossier);
    QString nomficpdf = nomdossier + "/" + nomfichier;

    TextPrinter *TexteAImprimer = new TextPrinter();
    textpied.replace("{{DUPLI}}","");

    TexteAImprimer->setFooterSize(TaillePieddePage());
    TexteAImprimer->setHeaderText(textentete);
    TexteAImprimer->setHeaderSize(TailleEnTete());
    TexteAImprimer->setFooterText(textpied);
    TexteAImprimer->setTopMargin(TailleTopMarge());


    TexteAImprimer->print(Etat->document(), nomficpdf);
    // le paramètre true de la fonction print() génère la création du fichier pdf nomficpdf et pas son impression
    QFile filepdf(nomficpdf);
    if (!filepdf.open( QIODevice::ReadOnly ))
        UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier:\n") + nomficpdf, tr("Impossible d'enregistrer l'impression dans la base"));
    else
        a = true;
    filepdf.close ();
    delete TexteAImprimer;
    delete Etat;
    return a;
}

// Get file content from SQL table
QByteArray Procedures::getFileFromSQL(Item *item)
{
    QByteArray ba       = QByteArray();
    DocExterne *docmt   = qobject_cast<DocExterne*>(item);
    Depense *dep        = qobject_cast<Depense*>(item);
    bool isdocument     = (docmt != Q_NULLPTR);
    bool isfacture      = (dep != Q_NULLPTR);
    if (!isdocument && ! isfacture)
        return ba;
    QString sQuery;
    if (isdocument)
        sQuery = "select " CP_PDF_DOCSEXTERNES ", " CP_JPG_DOCSEXTERNES ", " CP_COMPRESSION_DOCSEXTERNES "  from " TBL_DOCSEXTERNES " where " CP_ID_DOCSEXTERNES " = " + QString::number(docmt->id());
    else if (isfacture)
        sQuery = "select " CP_PDF_FACTURES ", " CP_JPG_FACTURES "  from " TBL_FACTURES " where " CP_ID_FACTURES " = " + QString::number(dep->idfacture());

    QList<QVariantList> listimpr;
    listimpr = db->StandardSelectSQL(sQuery, m_ok,  tr("Impossible d'accéder à la table ") + (isdocument? TBL_DOCSEXTERNES : TBL_FACTURES));

    if( m_ok && listimpr.size() >0)
    {
        if (isdocument)
        {
            if (listimpr.at(0).at(1).toByteArray().size() >0) //c'est un jpg
            {
                if (listimpr.at(0).at(2).toInt() == 1)
                    ba.append(qUncompress(listimpr.at(0).at(1).toByteArray()));
                else
                    ba.append(listimpr.at(0).at(1).toByteArray());
                docmt->setimageformat(JPG);
            }
            else if (listimpr.at(0).at(0).toByteArray().size() >0) //c'est un pdf
            {
                ba.append(listimpr.at(0).at(0).toByteArray());
                docmt->setimageformat(PDF);
            }
        }
        else if (isfacture)
        {
            if (listimpr.at(0).at(1).toByteArray().size() >0) //c'est un jpg
            {
                ba.append(listimpr.at(0).at(1).toByteArray());
                dep->setfactureformat(JPG);
            }
            else if (listimpr.at(0).at(0).toByteArray().size() >0) //c'est un pdf
            {
                ba.append(listimpr.at(0).at(0).toByteArray());
                dep->setfactureformat(PDF);
            }
        }
    }
    return ba;
}

QByteArray Procedures::getFileFromServer(QString filename)
{
    QByteArray ba = QByteArray();
    QList<QVariantList> listimpr;
    QString req = "SELECT LOAD_FILE('" + filename + "') AS content";
    //qDebug() << req;
    listimpr = db->StandardSelectSQL(req,m_ok, tr("Impossible d'accéder au fichier ") + filename);
    if(m_ok && listimpr.size() >0)
        ba.append(listimpr.at(0).at(0).toByteArray());
    return ba;
}

QString Procedures::Edit(QString txt, QString titre, bool editable, bool ConnectAuSignal, QWidget *parent)
{
    QString         rep("");
    QString         geometry(Position_Fiche Nom_fiche_Edit);
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

    gAsk->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
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
    gAsk->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
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
 * \abstract affiche le contenu d'un fichier image pdf ou jpg dans une fenêtre à la taille maximale pouvant être contenue dans l'écran, sans dépasser les 2/3 en largeur
 * \param doc QMap<QString,QVariant>contient 2 éléments
    . \arg doc["ba"] = le QByteArray correspondant au contenu du fichier   = QFile(emplacementdufichier)->readAll())
    . \arg doc["type"] = "jpg" ou "pdf" correspondant au type du fichier   = QFileInfo(emplacementdufichier)->suffix();
 * \param label le label de l'image affiché dans un QLabel magenta en bas à gauche de l'image
 * \param titre le titre de la fiche
 * \param Buttons, les boutons affichés en dessous de l'image, OKButton par défaut
 * \param parent
 * si le bouton PrintButton est utilisé il permet d'imprimer l'image en appelant la fonction PrintDocument(QMap<QString,QVariant> doc)
 */
void Procedures::EditDocument(QMap<QString,QVariant> doc, QString label, QString titre, UpDialog::Buttons Button, QWidget *parent)
{
    gEditDocumentDialog         = new UpDialog(parent);
    wdg_tablewidget             = new UpTableWidget(gEditDocumentDialog);
    wdg_inflabel                = new UpLabel(wdg_tablewidget);
    m_listeimages               = wdg_tablewidget->AfficheDoc(doc);
    wdg_tablewidget ->installEventFilter(this);
    gEditDocumentDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    gEditDocumentDialog->setWindowTitle(titre);
    gEditDocumentDialog->setWindowModality(Qt::WindowModal);
    gEditDocumentDialog->dlglayout()->insertWidget(0,wdg_tablewidget);
    wdg_tablewidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    wdg_tablewidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    gEditDocumentDialog->AjouteLayButtons(Button);
    connect(gEditDocumentDialog->OKButton, &QPushButton::clicked, this, [=] {gEditDocumentDialog->accept();});
    if (Button.testFlag(UpDialog::ButtonPrint))
    {
        gEditDocumentDialog->PrintButton->setdata(doc);
        connect(gEditDocumentDialog->PrintButton, &UpSmallButton::clickedvar, this, [=](QVariant) {PrintDocument(doc);});
    }
    if (Button.testFlag(UpDialog::ButtonSuppr))
        connect(gEditDocumentDialog->SupprButton, &QPushButton::clicked, this, [=] {emit DelImage();});

    QList<QScreen*> listscreens = QGuiApplication::screens();
    int x = 0;
    int y = 0;
    if (listscreens.size())
    {
        x = listscreens.first()->geometry().width();
        y = listscreens.first()->geometry().height();
    }
    gEditDocumentDialog->setMaximumWidth(x);
    gEditDocumentDialog->setMaximumHeight(y);
    int topmarge    = gEditDocumentDialog->dlglayout()->contentsMargins().top();
    int bottommarge = gEditDocumentDialog->dlglayout()->contentsMargins().bottom();
    int leftmarge   = gEditDocumentDialog->dlglayout()->contentsMargins().left();
    int rightmarge  = gEditDocumentDialog->dlglayout()->contentsMargins().right();
    int spacing     = gEditDocumentDialog->dlglayout()->spacing();
    int hdeltaframe = 60;//gAsk->frameGeometry().height() - gAsk->height();
                            // on a un problème avec ce calcul sous mac puisque qt ne connait pas la hauteur du dock
    int wdeltaframe = gEditDocumentDialog->frameGeometry().width() - gEditDocumentDialog->width();

    int hdelta = topmarge + bottommarge + spacing + gEditDocumentDialog->widgetbuttons()->height();
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
    gEditDocumentDialog->resize(w, h);
    wdg_tablewidget->resize(wtable, htable);
    int delta = 0;
    for (int i=0; i < wdg_tablewidget->rowCount(); i++)
    {
        UpLabel *lbl = qobject_cast<UpLabel*>(wdg_tablewidget->cellWidget(i,0));
        if (lbl != Q_NULLPTR)
        {
            pix = pix.scaled(wtable- delta, htable - delta, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            lbl->setPixmap(pix);
            wdg_tablewidget->setRowHeight(i,htable-delta);
            wdg_tablewidget->setColumnWidth(0,wtable-delta);
        }
    }

    if (w > (x - gEditDocumentDialog->x()))
        gEditDocumentDialog->move(x - w, 0);

    wdg_inflabel    ->setText("<font color='magenta'>" + label + "</font>");
    QFont font = qApp->font();
    font.setPointSize(12);
    wdg_inflabel->setFont(font);
    wdg_inflabel->setGeometry(10,htable-40,350,25);

    gEditDocumentDialog->exec();
    delete gEditDocumentDialog;
    gEditDocumentDialog = Q_NULLPTR;
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
    QByteArray ba = doc.value("ba").toByteArray();
    QList<QImage> listimg;
    if (doc.value("type").toString() == PDF)     // le document est un pdf
        listimg = Utils::calcImagefromPdf(ba);
    else if (doc.value("type").toString() == JPG)     // le document est un jpg
    {
        QPixmap pix;
        pix.loadFromData(ba);
        QImage image= pix.toImage();
        listimg << image;
    }
    return Print(listimg);
}

bool Procedures::Imprimer_Document(QWidget *parent, Patient *pat, User * user, QString titre, QString textorigine, QDate date,
                                   bool Prescription, bool ALD, bool AvecDupli, bool AvecChoixImprimante, bool Administratif)
{
    if (pat == Q_NULLPTR || user == Q_NULLPTR)
        return false;
    QString     textcorps, textpied, textentete;
    bool        AvecNumPage = false;
    bool        aa;

    //création de l'entête
    QMap<QString,QString> EnteteMap = CalcEnteteImpression(date, user);
    if (EnteteMap.value("Norm") == "")
        return false;
    textentete                      = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
    if (textentete == "") return false;
    textentete.replace("{{TITRE1}}"        , "");
    textentete.replace("{{TITRE}}"         , "");
    textentete.replace("{{DDN}}"           , "");
    textentete.replace("{{PRENOM PATIENT}}", (Prescription? pat->prenom()        : ""));
    textentete.replace("{{NOM PATIENT}}"   , (Prescription? pat->nom().toUpper() : ""));

    //création du pied
    textpied = CalcPiedImpression(user, false, ALD);
    if (textpied == "")
        return false;

    // creation du corps
    textcorps = CalcCorpsImpression(textorigine, ALD);
    if (textcorps == "")
        return false;
    QTextEdit   Etat_textEdit;
    Etat_textEdit.setHtml(textcorps);
    if (Etat_textEdit.toPlainText() == "")
        return false;
    int tailleEnTete = TailleEnTete();
    if (ALD) tailleEnTete = TailleEnTeteALD();

    aa = Imprime_Etat(parent, textcorps, textentete, textpied,
                            TaillePieddePage(), tailleEnTete, TailleTopMarge(),
                            AvecDupli, AvecNumPage, AvecChoixImprimante);

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        Utils::nettoieHTML(textcorps, 9);

        int idpat = 0;
        idpat = pat->id();

        QHash<QString, QVariant> listbinds;
        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        listbinds[CP_IDUSER_DOCSEXTERNES]        = user->id();
        listbinds[CP_IDPAT_DOCSEXTERNES]         = idpat;
        listbinds[CP_TYPEDOC_DOCSEXTERNES]       = (Prescription? "Prescription" : "Courrier");
        listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES]   = titre;
        listbinds[CP_TITRE_DOCSEXTERNES]         = titre;
        listbinds[CP_TEXTENTETE_DOCSEXTERNES]    = textentete;
        listbinds[CP_TEXTCORPS_DOCSEXTERNES]     = textcorps;
        listbinds[CP_TEXTORIGINE_DOCSEXTERNES]   = textorigine;
        listbinds[CP_TEXTPIED_DOCSEXTERNES]      = textpied.replace("{{DUPLI}}","");
        listbinds[CP_DATE_DOCSEXTERNES]          = date.toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds[CP_IDEMETTEUR_DOCSEXTERNES]    = Datas::I()->users->userconnected()->id();
        listbinds[CP_ALD_DOCSEXTERNES]           = (ALD? "1": QVariant(QString()));
        listbinds[CP_EMISORRECU_DOCSEXTERNES]    = "0";
        listbinds[CP_FORMATDOC_DOCSEXTERNES]     = (Prescription? PRESCRIPTION : (Administratif? COURRIERADMINISTRATIF : COURRIER));
        listbinds[CP_IDLIEU_DOCSEXTERNES]        = Datas::I()->sites->idcurrentsite();
        listbinds[CP_IMPORTANCE_DOCSEXTERNES]    = (Administratif? "0" : "1");
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if(doc != Q_NULLPTR)
            delete doc;
    }
    return aa;
}

bool Procedures::Print(QList<QImage> listimage)
{
    auto print = [=]
    {
        QPainter PrintingPreView(m_printer);
        for (int i=0; i<listimage.size();++i)
        {
            if( i > 0 ) {
                m_printer->newPage();
            }
            //QPixmap pix = QPixmap::fromImage(m_imagelist.at(i)).scaledToWidth(int(m_rect.width()),Qt::SmoothTransformation);
            QPageSize pgSize = m_printer->pageLayout().pageSize();
            QImage page = listimage.at(i).scaled(pgSize.sizePixels(m_printer->resolution()), Qt::KeepAspectRatio);
            PrintingPreView.drawImage(QPoint(0,0),page);
        }
    };

    //bool Apercu = true;
    if (ApercuAvantImpression())
    {
        QPrintPreviewDialog *dialog = new QPrintPreviewDialog(m_printer);
        dialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {print();});
        if (dialog->exec() == QDialog::Accepted)
            delete dialog;
        else {
            delete dialog;
            return false;
        }
    }
    else
    {
        QPrintDialog *dialog = new QPrintDialog(m_printer);
        if (dialog->exec() == QDialog::Accepted)
        {
            m_printer = dialog->printer();
            print();
            delete dialog;
        }
        else {
            delete dialog;
            return false;
        }
    }
    return true;
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
    bool responsableles2= currentuser()->isAlterneResponsableEtAssistant();

    bool liberal        = currentuser()->isLiberal();
    bool liberalSEL     = currentuser()->isLiberalSEL();
    bool salarie        = currentuser()->isSoignantSalarie();
    bool remplacant   = currentuser()->isRemplacant();

    bool cotation       = currentuser()->useCotationsActes();

    bool soignant           = currentuser()->isSoignant();
    bool soigntnonassistant = soignant && !assistant;
    bool respsalarie        = soigntnonassistant && (salarie||liberalSEL);
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

    if (soigntnonassistant && currentuser()->NumPS() > 0)
        txtstatut += "\n" + tr("RPPS :\t\t\t") + QString::number(currentuser()->NumPS());
    if (medecin && ! assistant &&  currentuser()->numOrdre() !="")
        txtstatut += "\nADELI :\t\t\t" + currentuser()->numOrdre();
    User *employeur = Datas::I()->users->getById(currentuser()->idemployeur());
    if (soignant)
    {
        txtstatut += "\n" + tr("Exercice :\t\t\t");
        if (liberal)
            txtstatut += tr("libéral");
        else if (liberalSEL)
        {
            QString txtsalarie = tr("libéral en SEL");
            txtsalarie += " - " + (employeur? employeur->login() : "null");
            txtstatut += txtsalarie;
        }
        else if (salarie)
        {
            QString txtsalarie = tr("salarié");
            txtsalarie += " - " + tr("Employeur : ") + (employeur? employeur->login() : "null");
            txtstatut += txtsalarie;
        }
        else if (remplacant)
            txtstatut += tr("remplaçant");
    }
    if (respliberal)
    {
        QString txtliberal ("");
        Compte * cptencaissement = Datas::I()->comptes->getById(currentuser()->idcompteencaissementhonoraires());
        if (cptencaissement)
        {
            txtliberal +=  "\n" + tr("Honoraires encaissés sur le compte :\t") + cptencaissement->nomabrege();
            if (Datas::I()->users->getById(currentuser()->idcomptableactes()) != Q_NULLPTR)
                txtliberal += tr("de") + " " + Datas::I()->users->getById(currentuser()->idcomptableactes())->login();
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
    else if (remplacant)
        txtstatut += "\n" + tr("Statut :\t\t\t") + tr("remplaçant");
    if (soigntnonassistant && cotation)
        txtstatut += "\n" + tr("Cotation des actes :\t\t") + (cotation? tr("Oui") : tr("Sans"));
    if (medecin && cotation && db->parametres()->cotationsfrance())
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
    if (respliberal && db->parametres()->comptafrance())
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
                UpLabel *lbl = qobject_cast<UpLabel*>(wdg_tablewidget->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    QPixmap pix = QPixmap::fromImage(m_listeimages.at(i).scaled(wdg_tablewidget->width(), wdg_tablewidget->height(),
                                                           Qt::KeepAspectRatioByExpanding,
                                                           Qt::SmoothTransformation));
                    lbl->setPixmap(pix);
                    wdg_tablewidget->setRowHeight(i,lbl->pixmap().height());
                    wdg_tablewidget->setColumnWidth(i,lbl->pixmap().width());
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
    if (m_settings->value(Imprimante_TailleTopMarge).toInt() < 10)
        m_settings->setValue(Imprimante_TailleTopMarge, 5);
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

    QString req = "USE `" DB_RUFUS "`;";
    db->StandardSQL(req);

    req = "DROP PROCEDURE IF EXISTS " MYSQL_PROC_POSTEIMPORTDOCS ";";
    db->StandardSQL(req);

    QString IpAdress("NULL");
    if (a)
    {
        if (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs).toString() ==  "YES")
            IpAdress = QHostInfo::localHostName() + " - prioritaire" ;
        else
            IpAdress = QHostInfo::localHostName();
    }
    req = "CREATE PROCEDURE " MYSQL_PROC_POSTEIMPORTDOCS "()\n\
            BEGIN\n\
            SELECT '" + IpAdress + "';\n\
            END ;";
    db->StandardSQL(req);
}

QString Procedures::PosteImportDocs()
{
    QString rep = "";
    QString req = "";
    bool isMysql8 = false;
    if (db->version().split(".").size() > 0)
        isMysql8 = (db->version().split(".").at(0).toInt() == 8);
    //qDebug() << "Mysql = " << db->version() << " - Mysql version = " << db->version().split(".").at(0).toInt();

    /*! Il n'y pas de variables utilisateur globale dans MySQL, on est donc obligé de passer par une procédure stockée pour en simuler une
    * pour créer une procédure avec Qt, séparer le drop du create, ne pas utiliser les délimiteurs et utiliser les retours à la ligne \n\.......
    * if (gsettingsIni->value(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs).toString() ==  "YES")

    * si a = true, on se met en poste importateur +/_ prioritaire à la fin suivant le contenu de rufus.ini
    * si a = false, on retire le poste en cours et on met NULL à la place. */

    if (isMysql8)
        req = "SELECT ROUTINE_SCHEMA, ROUTINE_NAME FROM information_schema.routines WHERE ROUTINE_SCHEMA = '" DB_RUFUS "' AND ROUTINE_NAME = '" MYSQL_PROC_POSTEIMPORTDOCS "'";
    else
        req = "SELECT name FROM mysql.proc p WHERE db = '" DB_RUFUS "' AND name = '" MYSQL_PROC_POSTEIMPORTDOCS "'";

    QVariantList imptdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
    if (m_ok && imptdata.size()>0)
    {
        req = "CALL " DB_RUFUS "." MYSQL_PROC_POSTEIMPORTDOCS;
        QVariantList calldata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
        //qDebug() << "nbre reponses = " + QString::number(calldata.size()) << NOM_POSTEIMPORTDOCS " = " + calldata.at(0).toString();
        if (m_ok && calldata.size()>0)
            rep = calldata.at(0).toString();
    }
    //qDebug() << "posteimportdocs = " + rep;
    return rep;
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
                Utils::removeWithoutPermissions(FichierBup);
            Utils::copyWithPermissions(FichierIni,PATH_DIR_RUFUS + "/RufusBackup.ini");
            Utils::removeWithoutPermissions(FichierIni);
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
                      " or table_schema = '" DB_RUFUS "'"
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
        if (listchk.at(i)->objectName() == "base")
        {
            if (listchk.at(i)->isChecked())
                volume += m_basesize;
        }
        if (listchk.at(i)->objectName() == "images")
        {
            if (listchk.at(i)->isChecked())
                volume += m_imagessize;
        }
        if (listchk.at(i)->objectName() == "videos")
        {
            if (listchk.at(i)->isChecked())
                volume += m_videossize;
        }
        if (listchk.at(i)->objectName() == "factures")
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

        QDir dir(PATH_DIR_RESSOURCES);
        if (!dir.exists())
            Utils::mkpath(PATH_DIR_RESSOURCES);
        Utils::setDirPermissions(PATH_DIR_RESSOURCES);
        QStringList listfichiers = dir.entryList(QStringList() << "*.sql");
        for (int t=0; t<listfichiers.size(); t++)
        {
            QString filename  = listfichiers.at(t);
            QString filepath = PATH_DIR_RESSOURCES "/" + filename;
            QFile file(filepath);
            Utils::removeWithoutPermissions(file);
        }
        QFile rufusViergeFile(QStringLiteral("://rufus.sql"));
        Utils::copyWithPermissions(rufusViergeFile, PATH_DIR_RESSOURCES "/rufus.sql");
        QFile OphtalmologieViergeFile(QStringLiteral("://Ophtalmologie.sql"));
        Utils::copyWithPermissions(OphtalmologieViergeFile, PATH_DIR_RESSOURCES "/Ophtalmologie.sql");
        QFile ImagerieViergeFile(QStringLiteral("://Images.sql"));
        Utils::copyWithPermissions( ImagerieViergeFile, PATH_DIR_RESSOURCES "/Images.sql");
        QFile ComptaMedicaleViergeFile(QStringLiteral("://ComptaMedicale.sql"));
        Utils::copyWithPermissions(ComptaMedicaleViergeFile, PATH_DIR_RESSOURCES "/ComptaMedicale.sql");

        QStringList listnomsfilestorestore;
        QString msg = "";
        listfichiers = dir.entryList(QStringList() << "*.sql");
        for (int j=0; j<listfichiers.size(); j++)
            listnomsfilestorestore << dir.absolutePath() + "/" + listfichiers.at(j);
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
            dir.removeRecursively();
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
            db->StandardSQL("GRANT ALL ON *.* TO '" LOGIN_SQL "'@'%' WITH GRANT OPTION");
            db->StandardSQL("CREATE USER IF NOT EXISTS '" LOGIN_SQL "SSL'@'%' IDENTIFIED BY '" MDP_SQL "' REQUIRE SSL");
            db->StandardSQL("GRANT ALL ON *.* TO '" LOGIN_SQL "SSL'@'%' WITH GRANT OPTION");

            //! Restauration à partir du dossier sélectionné
            int a = ExecuteScriptSQL(listnomsfilestorestore);
            if (a != 0)
            {
                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                dir.removeRecursively();
                return false;
            }
            else
            {
                UpMessageBox::Information(Q_NULLPTR, tr("Base vierge créée"),tr("La création de la base vierge a réussi."));
                dir.removeRecursively();
                emit ConnectTimers(true);
                return true;
            }
        }
        dir.removeRecursively();
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
        QUrl url = Utils::getExistingDirectoryUrl(parent, tr("Restaurer à partir du dossier"), QUrl::fromLocalFile(dir), QStringList(), false);
        if (url == QUrl())
            return false;
        QDir dirtorestore = QDir(url.path());
        QString mdp("");
        if (!Utils::VerifMDP((PremierDemarrage? Utils::calcSHA1(MDP_ADMINISTRATEUR) : MDPAdmin()),tr("Saisissez le mot de passe Administrateur"), mdp))
            return false;


        /*! ---------------------------------------------------------------------------------------------------------------------------------------------------------
        * Restauration ---------------------------------------------------------------------------------------------------------------------------------------------
            * de la base de données --------------------------------------------------------------------------------------------------------------------------
            * des fichiers de ressources ---------------------------------------------------------------------------------------------------------------------------
            * de Rufus.ini -----------------------------------------------------------------------------------------------------------------------------------------
            * du dossier d'imagerie --------------------------------------------------------------------------------------------------------------------------------
            * des videos -------------------------------------------------------------------------------------------------------------------------------------------
        * -------------------------------------------------------------------------------------------------------------------------------------------------------*/
        bool OKini      = false;
        bool OKImages   = false;
        bool OKFactures = false;
        bool OKVideos   = false;

        QString msg;

        /*! 2 - détermination des éléments pouvant être restaurés */
        //qDebug() << dirtorestore.absolutePath() + NOM_FILE_INI;
        if (QFile(dirtorestore.absolutePath() + NOM_FILE_INI).exists())
            OKini = true;
        QDir rootimg = dirtorestore;
        if (rootimg.cdUp())
        {
            //qDebug() << rootimg.absolutePath() + NOM_DIR_IMAGES;
            if (QDir(rootimg.absolutePath() + NOM_DIR_IMAGES).exists())
                if (QDir(rootimg.absolutePath() + NOM_DIR_IMAGES).entryList(QDir::Dirs).size()>0)
                    OKImages = true;
            if (QDir(rootimg.absolutePath() + NOM_DIR_VIDEOS).exists())
                if (QDir(rootimg.absolutePath() + NOM_DIR_VIDEOS).entryList(QDir::Files | QDir::NoDotAndDotDot).size()>0)
                    OKVideos = true;
            if (QDir(rootimg.absolutePath() + NOM_DIR_FACTURES).exists())
                if (QDir(rootimg.absolutePath() + NOM_DIR_FACTURES).entryList(QDir::Dirs | QDir::NoDotAndDotDot).size()>0)
                    OKFactures = true;
        }

        /*! 3 - détermination de l'emplacement de destination des fichiers d'imagerie */
        if (OKImages || OKVideos || OKFactures)
        {
            m_dirimagerie = db->dirimagerie();
            if (m_dirimagerie == QString())
            {
                OKImages    = false;
                OKFactures  = false;
                OKVideos    = false;
            }
            else if (!QDir(m_dirimagerie).exists())
            {
                OKImages    = false;
                OKFactures  = false;
                OKVideos    = false;
            }
        }

        /*! 4 - choix des éléments à restaurer */
        AskBupRestore(RestoreOp, dirtorestore.absolutePath(), m_dirimagerie, OKini, OKImages, OKVideos, OKFactures);
        int result = dlg_buprestore->exec();
        if (result > 0)
        {
            QFileDevice::Permissions permissions = QFileDevice::ReadOther  | QFileDevice::WriteOther
                                                 | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                                                 | QFileDevice::ReadOwner  | QFileDevice::WriteOwner | QFileDevice::ExeOwner;
            emit ConnectTimers(false);
            foreach (UpCheckBox *chk, dlg_buprestore->findChildren<UpCheckBox*>())
            {
                /*! 4a - restauration de la base de données */
                if (chk->objectName() == "base")
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

                        qintptr handledlg = 0;
                        ShowMessage::I()->PriorityMessage(tr("Restauration de la base en cours"),handledlg);

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
                            int a = ExecuteScriptSQL(listnomsfilestorestore);
                            if (a != 0)
                                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                            else
                                msg += tr("Base de données Rufus restaurée\n");
                        }
                        ShowMessage::I()->ClosePriorityMessage(handledlg);
                    }
                }
            }
            foreach (UpCheckBox *chk, dlg_buprestore->findChildren<UpCheckBox*>())
            {
                /*! 4b - restauration du fichier ini */
                if (chk->objectName() == "ini")
                {
                    if (chk->isChecked())
                    {
                        QString fileini = dirtorestore.absolutePath() + NOM_FILE_INI;
                        QFile FichierIni(PATH_FILE_INI);
                        if (FichierIni.exists())
                            Utils::removeWithoutPermissions(FichierIni);
                        QFile rufusini(fileini);
                        Utils::copyWithPermissions(rufusini, PATH_FILE_INI);
                        msg += tr("Fichier de paramétrage Rufus.ini restauré\n");
                        UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Fichier de paramétrage Rufus.ini restauré"), Icons::icSunglasses(), 3000);
                    }
                }
                /*! 4c - restauration des images */
                else if (chk->objectName() == "images")
                {
                    if (chk->isChecked())
                    {
                        QString dirdestinationimg   = m_dirimagerie + NOM_DIR_IMAGES;
                        QDir DirDestImg(dirdestinationimg);
                        if (DirDestImg.exists())
                            DirDestImg.removeRecursively();
                        DirDestImg.mkdir(dirdestinationimg, permissions);
                        if (!DirDestImg.exists())
                        {
                            QString Msg = tr("le dossier de destination de l'imagerie n'existe pas");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                        }
                        else
                        {
                            QString dirrestaureimagerie    = rootimg.absolutePath() + NOM_DIR_IMAGES;
                            int t = 0;
                            Utils::countFilesInDirRecursively(dirrestaureimagerie, t);
                            UpProgressDialog *progdial = new UpProgressDialog(0,t, parent);
                            progdial->show();
                            int n = 0;
                            Utils::copyfolderrecursively(dirrestaureimagerie, dirdestinationimg, n, tr("Restauration des fichiers d'imagerie"), progdial);
                            delete progdial;
                            msg += tr("Fichiers d'imagerie restaurés\n");
                        }
                    }
                }
                /*! 4d - restauration des factures */
                else if (chk->objectName() == "factures")
                {
                    if (chk->isChecked())
                    {
                        QString dirdestinationfact  = m_dirimagerie + NOM_DIR_FACTURES;
                        QDir DirDestFact(dirdestinationfact);
                        if (DirDestFact.exists())
                            DirDestFact.removeRecursively();
                        DirDestFact.mkdir(dirdestinationfact,permissions);
                        if (!DirDestFact.exists())
                        {
                            QString Msg = tr("le dossier de destination des factures n'existe pas");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                        }
                        else
                        {
                            QString dirrestaurefactures    = rootimg.absolutePath() + NOM_DIR_FACTURES;
                            int t = 0;
                            Utils::countFilesInDirRecursively(dirrestaurefactures, t);
                            UpProgressDialog *progdial = new UpProgressDialog(0,t, parent);
                            progdial->show();
                            int n = 0;
                            Utils::copyfolderrecursively(dirrestaurefactures, dirdestinationfact, n, tr("Restauration des factures"), progdial);
                            delete progdial;
                            msg += tr("Fichiers factures restaurés\n");
                        }
                    }
                }
                /*! 4e - restauration des videos */
                else if (chk->objectName() == "videos")
                {
                    if (chk->isChecked())
                    {
                        QString dirdestinationvid   =  m_dirimagerie + NOM_DIR_VIDEOS;
                        QDir DirDestVid(dirdestinationvid);
                        if (DirDestVid.exists())
                            DirDestVid.removeRecursively();
                        DirDestVid.mkdir(dirdestinationvid, permissions);
                        if (!DirDestVid.exists())
                        {
                            QString Msg = tr("le dossier de destination des videos n'existe pas");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                        }
                        else
                        {
                            QString dirrestaurevideo = rootimg.absolutePath() + NOM_DIR_VIDEOS;
                            int t = 0;
                            Utils::countFilesInDirRecursively(dirrestaurevideo, t);
                            UpProgressDialog *progdial = new UpProgressDialog(0,t, parent);
                            progdial->show();
                            int n = 0;
                            Utils::copyfolderrecursively(dirrestaurevideo, dirdestinationvid, n, tr("Restauration des videos"), progdial);
                            delete progdial;
                            msg += tr("Fichiers video restaurés\n");
                        }
                    }
                }
            }
        }
        delete dlg_buprestore;
        dlg_buprestore = Q_NULLPTR;
        //qDebug() << msg;
        UpMessageBox::Watch(parent,tr("Restauration terminée"),msg);
        emit ConnectTimers(true);
        return (result > 0);
    }
}

bool Procedures::VerifVersionBase(QWidget* parent)
{
    auto erreur = [] (QWidget *parent)
    {
        UpMessageBox::Watch(parent, tr("Impossible de mettre à jour la base de données\nSortie du programme"));
        exit(0);
    };

    int Version         = VERSION_BASE;
    m_parametres = db->parametres();
    int Versionencours = m_parametres->versionbase();

    bool BupDone = false;
    if (Versionencours < Version)
    {
        int nbreMAJ = Version - Versionencours;
        for (int i=1; i< nbreMAJ+1; i++)
        {
            Version = Versionencours + i;
            if (!BupDone)
            {
                UpMessageBox msgbox(parent);
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
                        erreur(parent);
                }
                else if (msgbox.clickedButton() == ExitBouton)
                    erreur(parent);
                BupDone = true;
                Datas::I()->postesconnectes->initListe();
                PosteConnecte* post = Datas::I()->postesconnectes->admin();
                if (post != Q_NULLPTR)
                    UpMessageBox::Watch(parent,tr("RufusAdmin présent"), tr("Après la mise à jour de la base") + "\n" +
                                                                            tr("Il vous faudra installer une version de RufusAdmin correspondante à la nouvelle version de la base") + "\n" +
                                                                            tr("Il faudra relancer chaque poste du réseau après le redémarrage de RufusAdmin"));
            }
            ShowMessage::I()->SplashMessage(tr("Mise à jour de la base vers la version ") + "<font color=\"red\"><b>" + QString::number(Version) + "</b></font>", 1000);
            QString Nomfic = "://majbase" + QString::number(Version) + ".sql";
            QFile DumpFile(Nomfic);
            int a = 99;
            QDir dir(PATH_DIR_RESSOURCES);
            if (!dir.exists())
                Utils::mkpath(PATH_DIR_RESSOURCES);
            Utils::setDirPermissions(PATH_DIR_RESSOURCES);
            QStringList listfichiers = dir.entryList(QStringList() << "*.sql");
            for (int t=0; t<listfichiers.size(); t++)
            {
                QString filename  = listfichiers.at(t);
                QString filepath = PATH_DIR_RESSOURCES "/" + filename;
                QFile file(filepath);
                Utils::removeWithoutPermissions(file);
            }
            if (DumpFile.exists())
            {
                QString NomDumpFile = PATH_DIR_RESSOURCES "/majbase" + QString::number(Version) + ".sql";
                QFile::remove(NomDumpFile);
                Utils::copyWithPermissions(DumpFile, NomDumpFile);
                emit ConnectTimers(false);
                a = ExecuteScriptSQL(QStringList() << NomDumpFile);
                QFile::remove(NomDumpFile);
                if (a == 0)
                {
                    UpMessageBox::Watch(parent,tr("Mise à jour effectuée de la base vers la version ") + QString::number(Version));
                    db->initParametresSysteme();
                }
                else
                {
                    Utils::playAlarm();
                    UpMessageBox::Watch(parent,tr("Echec de la mise à jour vers la version ") + QString::number(Version) + "\n" + tr("Le programme de mise à jour n'a pas pu effectuer la tâche!"));
                    erreur(parent);
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
            if (Version == 74)
            {
                QSettings settings(PATH_FILE_INI, QSettings::IniFormat);
                if (settings.contains("Param_Poste/Utilise_BasedeDonnees_Villes"))
                {
                    if (settings.value("Param_Poste/Utilise_BasedeDonnees_Villes").toBool() == false)
                        db->setvillesfrance(false);
                    settings.remove("Param_Poste/Utilise_BasedeDonnees_Villes");
                }
            }
        }
        QDir dir(PATH_DIR_RESSOURCES);
        if (dir.exists())
            dir.removeRecursively();
    }
    if (Versionencours > Version)
    {
        QString text = QObject::tr("Vous utilisez sur ce poste une version de Rufus prévue pour la version") + " " + QString::number(Version) + " " + QObject::tr("de la base de données");
        text += "<br/>" + QObject::tr("Cette version est peut-être incompatible avec la version") + " " + QString::number(Versionencours) + " " + tr("actuellement installée sur ce poste");
        text += "<br/>" + QObject::tr("Il est fortement conseillé de faire une mise à jour de Rufus");
        text += "<br/>" + QObject::tr("pour éviter des dysfonctionnements ou une altération votre base de données Rufus");
        text += "<br/>" + QObject::tr("Vous pouvez télécharger la dernière version sur la page Téléchargements du site") + " <a href=\"https://www.rufusvision.org\">www.rufusvision.org</a>";
        UpMessageBox::Watch(parent, tr("Version de Rufus trop ancienne"), text, UpDialog::ButtonOK, "https://www.rufusvision.org");
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

    if (dirSQLExecutable() == "")
    {
        Logs::ERROR(tr("Impossible de trouver l'exécutable MySQL"));
        UpMessageBox::Watch(nullptr, tr("Erreur de connexion"), tr("Impossible de trouver l'exécutable MySQL") + "\n" + tr("Le programme ne pourra effectuer aucune opération de sauvegarde, restauration ou mise à jour de la base"));
    }

    //initListeUsers();
    CalcLieuExercice();

    /*! Création de la session */
    QHash<QString, QVariant>                m_listbinds;
    m_listbinds[CP_IDUSER_SESSIONS]         = currentuser()->id();
    m_listbinds[CP_IDSUPERVISEUR_SESSIONS]  = currentuser()->idsuperviseur();
    m_listbinds[CP_IDPARENT_SESSIONS]       = currentuser()->idparent();
    m_listbinds[CP_IDCOMPTABLE_SESSIONS]    = currentuser()->idcomptableactes();
    if (Datas::I()->sites->currentsite())
        m_listbinds[CP_IDLIEU_SESSIONS]     = Datas::I()->sites->currentsite()->id();
    m_listbinds[CP_DATEDEBUT_SESSIONS]      = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    Datas::I()->sessions->CreationSession(m_listbinds);

    if (Datas::I()->sites->currentsite() == Q_NULLPTR && currentuser()->isSoignant())
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
        Datas::I()->sites->setcurrentsite(listEtab.first());
        return;
    }

    /* Cas ou le praticien travaille dans plusieur centres
     * on lui demande de sélectionner le centre où il se trouve au moment de la connexion
    */
    UpDialog *gAskLieux     = new UpDialog();
    gAskLieux               ->AjouteLayButtons();
    QGroupBox*boxlieux      = new QGroupBox();
    gAskLieux->dlglayout()  ->insertWidget(0,boxlieux);
    boxlieux                ->setObjectName("Parent");
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
    User *user = currentuser();
    if (user == Q_NULLPTR)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de créer l'utilisateur"),tr("Erreur de création de l'utilisateur"));
        return false;
    }
    user->setidsuperviseur(idusr);
    user->setidcomptableactes(idusr);
    user->setidparent(idusr);
    MAJComptesBancaires(user);
    // la suite sert à corriger les tables documents remises en exemple qui peuvent avoir été créées à partir d'autres bases Rufus par un iduser différent auquel cas ces documents ne seraient pas modifiables
    req = "update " TBL_IMPRESSIONS " set " CP_IDUSER_IMPRESSIONS " = " + QString::number(idusr) + ", " CP_DOCPUBLIC_IMPRESSIONS " = 1";
    db->StandardSQL (req);
    req = "update " TBL_DOCSEXTERNES " set " CP_IDUSER_DOCSEXTERNES " = " + QString::number(idusr) + ", " CP_IDLIEU_DOCSEXTERNES " = "  + QString::number(idusr) + ", " CP_IDEMETTEUR_DOCSEXTERNES " = "  + QString::number(idusr);
    db->StandardSQL (req);
    req = "update " TBL_DOSSIERSIMPRESSIONS " set " CP_IDUSER_DOSSIERIMPRESSIONS " = " + QString::number(idusr) + ", " CP_PUBLIC_DOSSIERIMPRESSIONS " = 1";
    db->StandardSQL (req);
    req = "update " TBL_COMMENTAIRESLUNETTES " set " CP_IDUSER_COMLUN " = " + QString::number(idusr) + ", " CP_PUBLIC_COMLUN " = 1";
    db->StandardSQL (req);
    req = "update " TBL_COTATIONS " set " CP_IDUSER_COTATIONS " = " + QString::number(idusr);
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
    QString CP(""),ville("");
    enum Villes::TownsFrom from;
    if (UpMessageBox::Question(Q_NULLPTR,
                               tr("Base de données des villes et codes postaux"),
                               tr("Voulez-vous utiliser la base de données des villes françaises?)"),
                               UpDialog::ButtonCancel | UpDialog::ButtonOK,
                               QStringList() << tr("Non") << tr("Utiliser les codes postaux français"))
            == UpSmallButton::STARTBUTTON)
        from = Villes::DATABASE;
    else
        from = Villes::CUSTOM;
    Datas::I()->villes          ->initListe(from);

    if (Datas::I()->villes->ListeCodesPostaux().size()>0)
    {
        Ville *town = Datas::I()->villes->villes()->first();
        CP = town->codepostal();
        ville = town->nom();
    }
    db->setvillesfrance(from == Villes::DATABASE);

    bool a = (UpMessageBox::Question(Q_NULLPTR,
                            tr("Cotations françaises"),
                            tr("Voulez-vous utiliser le système français de cotation des actes médicaux?)"),
                            UpDialog::ButtonCancel | UpDialog::ButtonOK,
                            QStringList() << tr("Non") << tr("Utiliser les cotations françaises"))
        == UpSmallButton::STARTBUTTON);
    db->setcotationsfrance(a);

    m_connexionbaseOK = true;
    // On paramètre les dossiers, l'imprimante et les fichiers ressources
    PremierParametrageMateriel();
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
            CP_SOIGNANTSTATUS_USR " = 1,\n"
            CP_ISMEDECIN_USR " = 1,\n"
            CP_RESPONSABLEACTES_USR " = 1,\n"
            CP_COTATION_USR " = 1,\n"
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
    if (dlg_IdentUser->exec() == QDialog::Accepted)
    {
        VerifVersionBase();
        m_parametres = db->parametres();        
        enum Villes::TownsFrom from;
        if (m_parametres->villesfrance())
            from = Villes::DATABASE;
        else
            from = Villes::CUSTOM;
        Datas::I()->villes              ->initListe(from);
        Datas::I()->sites               ->initListe();
        Datas::I()->comptes             ->initListe();
        Datas::I()->postesconnectes     ->initListe();
        Datas::I()->banques             ->initListe();
        Datas::I()->tierspayants        ->initListe();
        Datas::I()->typestiers          ->initListe();
        Datas::I()->motifs              ->initListe();
        Datas::I()->users               ->initListe();
        Datas::I()->manufacturers       ->initListe();
        Datas::I()->motscles            ->initListe();
        Datas::I()->typesinterventions  ->initListe();
        MAJComptesBancaires(currentuser());
        m_applicationfont = currentuser()->police();
        qApp->setFont(m_applicationfont);

        m_dirimagerie = db->dirimagerie();
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
                    . lui même s'il est libéral
                    . son employeur s'il est salarié ou libéral en  SEL
                    . s'il est remplaçant (Remplacant) on lui demande qui il remplace et le comptable devient
                        . celui qu'il remplace si celui qu'il remplace est libéral
                        . l'employeur de  celui qu'il remplace si celui qu'il remplace est salarié
                    . -1 s'il n'enregistre pas de compta
                    . -2 sans objet (personnel non soignant)

           */

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
    delete dlg_IdentUser;
    return (currentuser() != Q_NULLPTR);
}

/*--------------------------------------------------------------------------------------------------------------------------------------
    -- renvoie le chemin du dossier où est stockée l'imagerie -----------------------------------------------------------
    ------------------------------------------------------------------------------------------------------------------------------------*/
QString Procedures::AbsolutePathDirImagerie()
{
    QString path ("");
    switch (db->ModeAccesDataBase()) {
    case Utils::Poste:
        path = db->dirimagerie();
        break;
    case Utils::ReseauLocal:
        path = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Imagerie).toString();
        break;
    case Utils::Distant:
        path = PATH_DIR_RUFUS NOM_DIR_IMAGERIE;
        if (!QDir(path).exists())
            Utils::mkpath(path);
        break;
    }
    return path;
}

bool Procedures::DefinitRoleUser() //NOTE : User Role Function
{
    if (currentuser()->isSoignant() )
    {
        dlg_askUser                 = new UpDialog();
        dlg_askUser                 ->AjouteLayButtons();
        dlg_askUser                 ->setdata(currentuser());
        QVBoxLayout *boxlay         = new QVBoxLayout;
        dlg_askUser->dlglayout()    ->insertLayout(0,boxlay);

        QGroupBox *boxrole      = new QGroupBox(dlg_askUser);
        boxrole                 ->setObjectName("Role");
        QString lblRole         = tr("Quel est votre rôle dans cette session?");
        boxrole                 ->setTitle(lblRole);
        boxrole                 ->setVisible(false);
        boxlay                  ->addWidget(boxrole);

        QGroupBox *boxsuperv    = new QGroupBox(dlg_askUser);
        boxsuperv               ->setObjectName("Superv");
        QString lblSuperv       = tr("Qui supervise votre activité pour cette session?");
        boxsuperv               ->setTitle(lblSuperv);
        boxsuperv               ->setVisible(false);
        boxlay                  ->addWidget(boxsuperv);

        QGroupBox *boxparent     = new QGroupBox(dlg_askUser);
        boxparent               ->setObjectName("Parent");
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
        else if( currentuser()->isAlterneResponsableEtAssistant() )
        {
            bool found = false;
            for (auto it = Datas::I()->users->actifs()->constBegin(); it != Datas::I()->users->actifs()->constEnd(); ++it)
            {
                User *usr = const_cast<User*>(it.value());
                if( usr->id() == currentuser()->id() )
                    continue;
                if( !usr->isResponsable() && !usr->isAlterneResponsableEtAssistant() )
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
                pbuttResp               ->setObjectName("buttresp");
                vbox                    ->addWidget(pbuttResp);
                connect(pbuttResp, &QRadioButton::clicked, this, &Procedures::CalcUserParent);

                QRadioButton *pbuttAss  = new QRadioButton(boxrole);
                pbuttAss                ->setText(tr("Assistant"));
                pbuttAss                ->setObjectName("buttass");
                pbuttAss                ->setChecked(true);      // le user est défini par défaut comme assistant -> on cherche qui supervise les actes
                vbox                    ->addWidget(pbuttAss);
                connect(pbuttAss, &QRadioButton::clicked, this, &Procedures::CalcUserSuperviseur);

                vbox                    ->setContentsMargins(8,0,8,0);
                boxrole                 ->setLayout(vbox);
                qobject_cast<QVBoxLayout*>(dlg_askUser->layout())->setSizeConstraint(QLayout::SetFixedSize);
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
            if( dlg_askUser->exec() != QDialog::Accepted)
            {
                delete dlg_askUser;
                dlg_askUser = Q_NULLPTR;
                return false;
            }
            foreach (QGroupBox *groupBox, dlg_askUser->findChildren<QGroupBox*>())
            {
                if( !groupBox->isVisibleTo(dlg_askUser) )
                    continue;
                if (groupBox->objectName() == "Superv" )
                {
                    QList<UpRadioButton*> listbutt = groupBox->findChildren<UpRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                        {
                            currentuser()->setidsuperviseur(listbutt.at(j)->iD());
                            break;
                        }
                }
                else if (groupBox->objectName() == "Parent" )
                {
                    QList<UpRadioButton*> listbutt = groupBox->findChildren<UpRadioButton*>();
                    for (int j=0; j<listbutt.size(); j++)
                        if (listbutt.at(j)->isChecked())
                        {
                            currentuser()->setidparent(listbutt.at(j)->iD());
                            break;
                        }
                }
            }
            delete dlg_askUser;
            dlg_askUser = Q_NULLPTR;
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
            currentuser()->setidcomptableactes(User::ROLE_NON_RENSEIGNE);
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
                                if( !usr->isLiberal() && !usr->isSoignantSalarie() && !usr->isLiberalSEL())
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
                                dlg_askUser             ->setdata(superviseurusr);
                                QVBoxLayout *boxlay     = new QVBoxLayout;
                                dlg_askUser->dlglayout()->insertLayout(0,boxlay);
                                QGroupBox*boxparent     = new QGroupBox(dlg_askUser);
                                boxparent               ->setObjectName("Parent");
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
                                    dlg_askUser = Q_NULLPTR;
                                    return false;
                                }
                                else
                                {
                                    foreach (QGroupBox *box, dlg_askUser->findChildren<QGroupBox*>())
                                        if (box->objectName() == "Parent")
                                        {
                                            foreach (UpRadioButton *butt, box->findChildren<UpRadioButton*>())
                                                if (butt->isChecked())
                                                {
                                                    currentuser()->setidparent( butt->iD() );
                                                    break;
                                                }
                                        }
                                    delete dlg_askUser;
                                    dlg_askUser = Q_NULLPTR;
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
                    m_usecotation = usrparent->useCotationsActes();
                    // determination de l'utilisation de la comptabilité
                    if( usrparent->isLiberal() )
                        currentuser()->setidcomptableactes(usrparent->id());
                    else if( usrparent->isLiberalSEL() || usrparent->isSoignantSalarie() )
                        currentuser()->setidcomptableactes(usrparent->idemployeur());
                    else
                        currentuser()->setidcomptableactes(User::ROLE_NON_RENSEIGNE);
                    currentuser()->setAGA(usrparent->isAGA());
                    currentuser()->setsecteurconventionnel(usrparent->secteurconventionnel());
                    currentuser()->setOPTAM(usrparent->isOPTAM());
                    MAJComptesBancaires(usrparent);
                    //qDebug() << "secteur = " << currentuser()->secteurconventionnel() << " - OPTAM = " << currentuser()->isOPTAM();
                }
                else
                {
                    UpMessageBox::Watch(Q_NULLPTR,tr("Aucun parent valide n'a été défini pour vos actes"), tr("Impossible de continuer"));
                    return false;
                }
            }
            else
                currentuser()->setidcomptableactes(User::ROLE_VIDE);
        }
        return true;
    }

    // il s'agit d'un administratif ou d'une société comptable
    currentuser()->setidsuperviseur(User::ROLE_VIDE);
    if (currentuser()->isSocComptable())
        currentuser()->setidcomptableactes(currentuser()->id());
    else
        currentuser()->setidcomptableactes(User::ROLE_VIDE);
    currentuser()->setidparent(User::ROLE_VIDE);
    m_usecotation     = true;
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
    if (!usr->isSoignant() && !usr->isSocComptable())
        return;
    usr->setlistecomptesbancaires(Datas::I()->comptes->initListeComptesByIdUser(usr->id()));
    if (usr->isSoignant())
    {
        if (usr->idcomptableactes() > 0)
        {
            User *cptble = Datas::I()->users->getById(usr->idcomptableactes());
            usr->setidcompteencaissementhonoraires(cptble? cptble->idcompteencaissementhonoraires() : 0);
        }
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
        if (Listgroupbx.at(i)->objectName() == "Superv")
            ptbox = Listgroupbx.at(i);
        else if (Listgroupbx.at(i)->objectName() == "Parent")
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
            UpRadioButton *pradiobutt = new UpRadioButton(ptbox);
            pradiobutt->setText(us->login());
            pradiobutt->setiD(us->id());
            if( isFirst )
            {
                isFirst = false;
                pradiobutt->setChecked(true);
            }
            vbox->addWidget(pradiobutt);
        }
        UpRadioButton *pradiobutt = new UpRadioButton();
        pradiobutt   ->setText(tr("Tout le monde"));
        pradiobutt   ->setiD(-1);
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

    user->setidsuperviseur( user->id() );
    QGroupBox *ptbox = Q_NULLPTR;
    foreach (QGroupBox * box, dlg_askUser->findChildren<QGroupBox*>())
    {
        if (box->objectName() == "Superv")
            box->setVisible(false);
        else if (box->objectName() == "Parent")
            ptbox = box;
    }
    if (ptbox)
        ptbox->setVisible(false);

    //! on a déterminé le superviseur, on cherche qui enregistre les actes
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
            if( !usr->isLiberal() && !usr->isSoignantSalarie() )
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
                UpRadioButton *pradiobutt = new UpRadioButton(ptbox);
                pradiobutt->setText(us->login());
                pradiobutt->setiD(us->id());
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
    UpMessageBox *msgbox = new UpMessageBox;
    UpSmallButton    AnnulBouton        (tr("Retour\nau menu d'accueil"));
    UpSmallButton    BaseViergeBouton (tr("Nouvelle base\npatients vierge"));
    UpSmallButton    BaseExistanteBouton(tr("Base patients existante\nsur le serveur"));

    msgbox->setText(tr("Premier démarrage de Rufus!"));
    msgbox->setInformativeText(tr("Cette étape va vous permettre de configurer le logiciel en quelques secondes.\n\n"
                              "Cette installation ne peut aboutir si vous n'avez pas de serveur MySQL installé.\n"
                              "Dans ce cas, il vous faut annuler et installer un serveur MySQL sur cet ordinateur ou sur un autre poste du réseau.\n\n"
                              "Commencez par choisir la situation qui décrit le mieux votre installation de Rufus.\n\n"
                              "1. J'installe Rufus sur ce poste et ce poste se connectera à une base patients qui existe dèjà\n"
                              "2. J'installe Rufus sur ce poste et ce poste se connectera à une base patients vierge que je vais créer\n"));
    msgbox->setIcon(UpMessageBox::Info);

    msgbox->addButton(&BaseViergeBouton,    UpSmallButton::NOBUTTON);
    msgbox->addButton(&BaseExistanteBouton, UpSmallButton::NOBUTTON);
    msgbox->addButton(&AnnulBouton,         UpSmallButton::CANCELBUTTON);
    msgbox->exec();

    protoc = BaseExistante;
    if (msgbox->clickedButton() == &AnnulBouton)
        return false;
    else if (msgbox->clickedButton() == &BaseExistanteBouton)
        protoc = BaseExistante;
    else if (msgbox->clickedButton() == &BaseViergeBouton)
        protoc = BaseVierge;



    if (m_settings != Q_NULLPTR)
        delete m_settings;
    m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
    QString login (""), MDP("");
    if (protoc == BaseExistante)
    {
        if (VerifParamConnexion(login, MDP, true))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne,\n"
                                       "le login ") + currentuser()->login() + tr(" est reconnu") + ".\n" +
                                       tr("Le programme va se fermer pour que les modifications") + "\n" +
                                       tr("puissent être prises en compte\n"));
            PremierParametrageMateriel();
            exit(0);
        }
    }
    else if (protoc == BaseVierge)
    {
        bool AccesDistant = false;
        if (VerifParamConnexion(login, MDP, false, AccesDistant))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion réussie"),
                                   tr("Bien, la connexion au serveur MySQL fonctionne "
                                       "et le programme va maintenant créer une base de données patients "
                                       "vierge de tout enregistrement."));
            // Création de la base
             if (!RestaureBase(true, true))
                return false;
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
void Procedures::PremierParametrageMateriel()
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

    // Création des dossiers
    //!    on server
    //!    -- secure-file-priv (/Users/Shared on macOS Users/Public on W10/11)
    //!                              /Imagerie
    //!                                     /Images
    //!                                     /DossierEchangeImages
    //!                                         /one dir for each machine
    //!                                     /EchecsTransferts
    //!                                     /Factures
    //!                                     /FacturesSansLien
    //!                                     /Originaux
    //!                                         /Factures
    //!                                         /Images
    //!                                     /Video

    //!     on local post
    //!     -- QDir::homePath()/Doduments/Rufus
    //!                                         /Imagerie
    //!                                             /Originaux
    //!                                                 /Factures
    //!                                                 /Images
    //!                                             /DossierEchangeRefraction
    //!                                                 /Refracteur
    //!                                                     /In
    //!                                                         /Autoref
    //!                                                         /Fronto
    //!                                                         /Tono
    //!                                                     /Out
    //!                                                 /Fronto
    //!                                                 /Tono
    //!                                                 /Autoref
    //!                                             /EchecTransferts
    //!                                         /Logs
    //!                                         Rufus.ini


    if (protoc == BaseVierge)
    {
        Utils::mkpath(db->dirimagerie() + NOM_DIR_IMAGES);
        Utils::mkpath(db->dirimagerie() + NOM_DIR_DOSSIERECHANGEIMAGERIE);
        Utils::mkpath(db->dirimagerie() + NOM_DIR_ECHECSTRANSFERTS);
        Utils::mkpath(db->dirimagerie() + NOM_DIR_FACTURES);
        Utils::mkpath(db->dirimagerie() + NOM_DIR_FACTURESSANSLIEN);
        Utils::mkpath(db->dirimagerie() + NOM_DIR_ORIGINAUX NOM_DIR_FACTURES);
        Utils::mkpath(db->dirimagerie() + NOM_DIR_ORIGINAUX NOM_DIR_IMAGES);
        Utils::mkpath(db->dirimagerie() + NOM_DIR_VIDEOS);
    }

    Utils::mkpath(PATH_DIR_REFRACTEUR_IN NOM_DIR_AUTOREF);
    Utils::mkpath(PATH_DIR_REFRACTEUR_IN NOM_DIR_FRONTO);
    Utils::mkpath(PATH_DIR_REFRACTEUR_IN NOM_DIR_TONO);
    Utils::mkpath(PATH_DIR_REFRACTEUR_OUT);
    Utils::mkpath(PATH_DIR_AUTOREF);
    Utils::mkpath(PATH_DIR_FRONTO);
    Utils::mkpath(PATH_DIR_TONO);
    Utils::mkpath(PATH_DIR_RUFUS NOM_DIR_LOGS);
    Utils::mkpath(PATH_DIR_ORIGINAUX NOM_DIR_FACTURES);
    Utils::mkpath(PATH_DIR_ORIGINAUX NOM_DIR_IMAGES);
    Utils::mkpath(PATH_DIR_RUFUS NOM_DIR_IMAGERIE NOM_DIR_ECHECSTRANSFERTS);
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

    UpMessageBox *msgbox = new UpMessageBox;
    msgbox->setText(msg);
    msgbox->setInformativeText(msgInfo);
    msgbox->setIcon(UpMessageBox::Warning);
    if (PremDemarrage)                      msgbox->addButton(&PremierDemarrageBouton,   UpSmallButton::NOBUTTON);
    if (ReconstruitIni)                     msgbox->addButton(&ReconstruitIniBouton,     UpSmallButton::NOBUTTON);
    if (RecupIni)                           msgbox->addButton(&RecupIniBouton,           UpSmallButton::NOBUTTON);
    msgbox->addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
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
                Utils::removeWithoutPermissions(FichierIni);
            QString fileini = dialog.selectedFiles().at(0);
            QFile rufusini(fileini);
            Utils::copyWithPermissions(rufusini, PATH_FILE_INI);
            if (m_settings != Q_NULLPTR)
                delete m_settings;
            m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
            reponse = true;
        }
    }
    else if (msgbox->clickedButton()==&ReconstruitIniBouton)
    {
        //reconstruire le fichier rufus.ini
        //1. on demande les paramètres de connexion au serveur - mode d'accès / user / mdp / port / SSL
        QFile file(PATH_FILE_INI);
        Utils::removeWithoutPermissions(file);
        if (m_settings != Q_NULLPTR)
            delete m_settings;
        m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
        QString login(""), MDP ("");
        if (VerifParamConnexion(login, MDP, true))
        {
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
        if (dirSQLExecutable() == "")
        {
            UpMessageBox::Watch(nullptr, tr("Erreur de connexion"), tr("Impossible de trouver l'exécutable MySQL") + "\n" + tr("Le programme ne pourra pas s'intialiser"));
                exit(0);
        }
        delete Dlg_ParamConnex;
        return true;
    }
    Dlg_ParamConnex->disconnect();
    delete Dlg_ParamConnex;
    return false;
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
                                && m_settings->value(Param_Poste_PortFronto).toString() != BOX);
    bool m_isAutorefParametre   = (nameARK != "-"
                                && nameARK != ""
                                && m_settings->value(Param_Poste_PortAutoref).toString() != BOX);
    bool m_isRefracteurParametre= (nameRF != "-"
                                && nameRF != "");
    bool m_isTonoParametre      = (nameTO != "-"
                                && nameTO != "");
    if (m_isFrontoParametre)
    {
        bool m_isReseauFronto       = (m_settings->value(Param_Poste_PortFronto).toString() == DOSSIER_ECHANGE);
        m_isReseauFronto?           appareilsreseau.setFlag(Fronto)      : appareilscom.setFlag(Fronto);
    }
    if (m_isAutorefParametre)
    {
        bool m_isReseauAutoref      = (m_settings->value(Param_Poste_PortAutoref).toString() == DOSSIER_ECHANGE);
        m_isReseauAutoref?          appareilsreseau.setFlag(Autoref)     : appareilscom.setFlag(Autoref);
    }
    if (m_isRefracteurParametre)
    {
        bool m_isReseauRefracteur   = (m_settings->value(Param_Poste_PortRefracteur).toString() == DOSSIER_ECHANGE);
        m_isReseauRefracteur?       appareilsreseau.setFlag(Refracteur) : appareilscom.setFlag(Refracteur);
    }
    if (m_isTonoParametre)
    {
        bool m_isReseauTono         = (m_settings->value(Param_Poste_PortTono).toString() == DOSSIER_ECHANGE);
        m_isReseauTono?             appareilsreseau.setFlag(Tonometre)        : appareilscom.setFlag(Tonometre);
    }
    if (appareilscom > 0)
        Ouverture_Ports_Series(appareilscom);
    if (appareilsreseau > 0)
        Ouverture_Fichiers_Echange(appareilsreseau);
    m_hasappareilrefractionconnecte = appareilscom >0 || appareilsreseau >0;
    if (m_hasappareilrefractionconnecte)
        connect(Nidek::I(), &GenericProtocol::newmesure, this, [=](GenericProtocol::TypeMesure typ) {InsertMesure(typ);});
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
        case Tonometre:     app = tr("le tonomètre");           break;
        default: break;
        }
        const QString nomfichier      = pathdirappareil + "/" + listfich.at(0);
        QFile datafile(nomfichier);

        QDomDocument docxml;
        QString filecontents ="";
        int filetype = 0;

        // XML file
        if(nomfichier.endsWith(".xml",Qt::CaseInsensitive))
        {
            filetype=Xml;
            if (datafile.open(QIODevice::ReadOnly))
            {
                docxml.setContent(&datafile);
                Utils::removeWithoutPermissions(datafile);
            }
        }
        // CSV file
        else if(nomfichier.endsWith(".csv",Qt::CaseInsensitive))
        {
            filetype=Csv;
            if (datafile.open(QIODeviceBase::ReadOnly)) {
                QTextStream  ts(&datafile);
                filecontents.append(ts.readAll());
                Utils::removeWithoutPermissions(datafile);
            }
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
                case Fronto:
                    switch (filetype)
                    {
                    case Xml:
                        Procedures::I()->ReponseXML_Fronto(docxml);
                        break;
                    case Csv:
                        Procedures::I()->ReponseCSV_Fronto(filecontents);
                        break;
                    }
                    break;
                case Autoref:       Procedures::I()->ReponseXML_Autoref(docxml);    break;
                case Refracteur:    Procedures::I()->ReponseXML_Refracteur(docxml); break;
                case Tonometre:     Procedures::I()->ReponseXML_Tono(docxml);       break;
                default: break;
                }
            }
        }
        listfich = QDir(pathdirappareil).entryList(QDir::Files | QDir::NoDotAndDotDot);
        for(int i = 0; i < listfich.size(); ++i)
        {
            QFile file(pathdirappareil + "/" + listfich.at(i));
            Utils::removeWithoutPermissions(file);
        }
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
        pathdirautoref = settings()->value(Param_Poste_PortAutoref_DossierEchange).toString();
        if (!usetimer)
            m_filewatcherautoref.addPath(pathdirautoref);
    }
    if (appareils.testFlag(Fronto))
    {
        m_LANFronto = true;
        Datas::I()->mesurefronto   ->settypemesure(Refraction::Fronto);
        pathdirfronto = settings()->value(Param_Poste_PortFronto_DossierEchange).toString();
        if (!usetimer)
            m_filewatcherfronto.addPath(pathdirfronto);
    }
    if (appareils.testFlag(Refracteur))
    {
        m_LANRefracteur = true;
        Datas::I()->mesurefinal     ->settypemesure(Refraction::Prescription);
        Datas::I()->mesureacuite    ->settypemesure(Refraction::Acuite);
        pathdirrefracteur = settings()->value(Param_Poste_PortRefracteur_DossierEchange).toString();
        if (!usetimer)
            m_filewatcherrefracteur.addPath(pathdirrefracteur);
    }
    if (appareils.testFlag(Tonometre))
    {
        m_LANTono = true;
        pathdirtono = settings()->value(Param_Poste_PortTono_DossierEchange).toString();
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
                QStringList listfichxml = QDir(pathdirautoref).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                    lecturefichier(Autoref, pathdirautoref, listfichxml);
            }
            if (appareils.testFlag(Fronto) && pathdirfronto != "")
            {
                QStringList formats;
                formats <<"*.xml";
                formats <<"*.csv";
                QStringList listfichxml = QDir(pathdirfronto).entryList(formats, QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                    lecturefichier(Fronto, pathdirfronto, listfichxml);
            }
            if (appareils.testFlag(Refracteur) && pathdirrefracteur != "")
            {
                QStringList listfichxml = QDir(pathdirrefracteur).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                    lecturefichier(Refracteur, pathdirrefracteur, listfichxml);
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
                    if (m_filewatcherautoreffile != listfichxml.at(0) || (m_filewatcherautoreffile == listfichxml.at(0) && m_filewatcherautorefcreated != QFileInfo(xmldoc).birthTime()))
                    {
                        m_filewatcherautoreffile = listfichxml.at(0);
                        m_filewatcherautorefcreated = QFileInfo(xmldoc).birthTime();
                        lecturefichier(Autoref, pathdirautoref, listfichxml);
                    }
                }
            });

        if (appareils.testFlag(Fronto) && pathdirfronto != "")
            connect(&m_filewatcherfronto,       &QFileSystemWatcher::directoryChanged,  this,   [=]
            {
                QStringList formats;
                formats <<"*.xml";
                formats <<"*.csv";
                QStringList listfichxml = QDir(pathdirfronto).entryList(formats, QDir::Files | QDir::NoDotAndDotDot);
                if (listfichxml.size())
                {
                    const QString nomfichierxml      = pathdirfronto + "/" + listfichxml.at(0);
                    QFile xmldoc(nomfichierxml);
                    if (m_filewatcherfrontofile != listfichxml.at(0) || (m_filewatcherfrontofile == listfichxml.at(0) && m_filewatcherfrontocreated != QFileInfo(xmldoc).birthTime()))
                    {
                        m_filewatcherfrontofile = listfichxml.at(0);
                        m_filewatcherfrontocreated = QFileInfo(xmldoc).birthTime();
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
                    if (m_filewatcherrefracteurfile != listfichxml.at(0) || (m_filewatcherrefracteurfile == listfichxml.at(0) && m_filewatcherrefracteurcreated != QFileInfo(xmldoc).birthTime()))
                    {
                        m_filewatcherrefracteurfile = listfichxml.at(0);
                        m_filewatcherrefracteurcreated = QFileInfo(xmldoc).birthTime();
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
                    if (m_filewatchertonofile != listfichxml.at(0) || (m_filewatchertonofile == listfichxml.at(0) && m_filewatchertonocreated != QFileInfo(xmldoc).birthTime()))
                    {
                        m_filewatchertonofile = listfichxml.at(0);
                        m_filewatchertonocreated = QFileInfo(xmldoc).birthTime();
                        lecturefichier(Tonometre, pathdirfronto, listfichxml);
                    }
                }
            });
    }
    return true;
}

/*! ------------------------------------------------------------------------------------------------------------------------------------------
GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Ports_Series(TypesAppareils appareils)
{
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
    // We read only once in this function the available ports List
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();

    /*!
        Ouvre le port dont le portName() est NomPort en rapport avec l'appareil passé en paramètre
    */
    if (m_mapports == QMap<QString,QString>())
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion série"),
                            tr("Des connexions série sont paramétrées pour certains appareils du poste de réfraction.\n"
                               "Malheureusement, aucune de ces connexions ne semble fonctionner."));
        return false;
    }
    auto openserialport = [&] (TypeAppareil appareil)
    {
        QSerialPort *serialport = Q_NULLPTR;
        QString PortCom = "";
        SerialSettings sparamportserie;
        InitialiseSerialSettings(sparamportserie);
        switch (appareil) {
        case Fronto:        sparamportserie = s_paramPortSerieFronto;       PortCom = m_portFronto;     break;
        case Autoref:       sparamportserie = s_paramPortSerieAutoref;      PortCom = m_portAutoref;    break;
        case Refracteur:    sparamportserie = s_paramPortSerieRefracteur;   PortCom = m_portRefracteur; break;
        case Tonometre:     sparamportserie = s_paramPortSerieTono;         PortCom = m_portTono;       break;
        default: break;
        }
        auto it = m_mapports.find(PortCom);
        if (it != m_mapports.end())
        {
            serialport= new QSerialPort(it.value());
            serialport->setBaudRate(sparamportserie.baudRate);
            serialport->setFlowControl(sparamportserie.flowControl);
            serialport->setParity(sparamportserie.parity);
            serialport->setDataBits(sparamportserie.dataBits);
            serialport->setStopBits(sparamportserie.stopBits);
        }
        return serialport;
    };

    QString listeports = "";
    for (auto it = m_mapports.begin(); it != m_mapports.end(); it++)
    {
        listeports += "\n";
        listeports += it.key() + " / " + it.value();
    }
    if (listeports != "")
        listeports = tr("Liste des ports disponibles") + " - " + listeports;
    else
        listeports = tr("Aucun port COM disponible sur le système");

    /*!
    for (int i=0; i< availablePorts.size(); i++)
    {
        Logs::LogToFile("PortsSeries.txt", availablePorts.at(i).portName() + " - " + QDateTime().toString("dd-MM-yyyy HH:mm:ss"));
        qDebug() << availablePorts.at(i).portName();
    }
    */
    // PORT FRONTO
    if (appareils.testFlag(Fronto))
    {
        m_portFronto = m_settings->value(Param_Poste_PortFronto).toString();
        if (m_portFronto == "")
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur paramètrage connexion frontofocomètre"));
        else
        {
            ReglePortFronto();
            sp_portFronto = openserialport(Fronto);
            if (sp_portFronto != Q_NULLPTR)
            {
                if (sp_portFronto->open(QIODevice::ReadWrite))
                {
                    //qDebug() << "FRONTO -> " + m_portFronto;
                    sp_portFronto->setDataTerminalReady(true);
                    if (t_threadFronto != Q_NULLPTR)
                        delete t_threadFronto;
                    t_threadFronto = new SerialThread(sp_portFronto, "Fronto");
                    t_threadFronto->transaction();
                    connect(t_threadFronto,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Fronto);
                    msg = tr("Connexion frontocomètre OK sur ") + m_portFronto;
                }
                else
                {
                    msg =  tr("Impossible de connecter le frontocomètre sur ") + m_portFronto;
                    UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le frontocomètre sur ") + m_portFronto, listeports);
                    if (sp_portFronto != Q_NULLPTR)
                        delete sp_portFronto;
                    sp_portFronto = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le frontocomètre sur ") + m_portFronto, listeports);
        }
    }

    // PORT REFRACTEUR
    if (appareils.testFlag(Refracteur))
    {
        m_portRefracteur = m_settings->value(Param_Poste_PortRefracteur).toString();
        if (m_portRefracteur == "")
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur paramètrage connexion refracteur"));
        else
        {
            ReglePortRefracteur();
            sp_portRefracteur = openserialport(Refracteur);
            if (sp_portRefracteur != Q_NULLPTR)
            {
                if (sp_portRefracteur->open(QIODevice::ReadWrite))
                {
                    //qDebug() << "REFRACTEUR -> " + m_portRefracteur;
                    sp_portRefracteur->setDataTerminalReady(true);
                    if (t_threadRefracteur != Q_NULLPTR)
                        delete t_threadRefracteur;
                    t_threadRefracteur = new SerialThread(sp_portRefracteur, "Refracteur");
                    t_threadRefracteur->transaction();
                    connect(t_threadRefracteur,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Refracteur);
                    msg = tr("Connexion refracteur OK sur ") + m_portRefracteur;
                }
                else
                {
                    msg =  tr("Impossible de connecter le refracteur sur ") + m_portRefracteur;
                    UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le refracteur sur ") + m_portRefracteur, listeports);
                    if (sp_portRefracteur != Q_NULLPTR)
                        delete sp_portRefracteur;
                    sp_portRefracteur = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le refracteur sur ") + m_portRefracteur, listeports);
        }
    }

    //PORT AUTOREF
    if (appareils.testFlag(Autoref))
    {
        m_portAutoref = m_settings->value(Param_Poste_PortAutoref).toString();
        if (m_portAutoref == "")
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur paramètrage connexion autorefractomètre"));
        else
        {
            ReglePortAutoref();
            sp_portAutoref = openserialport(Autoref);
            if (sp_portAutoref != Q_NULLPTR)
            {
                if (sp_portAutoref->open(QIODevice::ReadWrite))
                {
                    //qDebug() << "AUTOREF -> " + m_portAutoref;
                    sp_portAutoref->setDataTerminalReady(true);
                    if (t_threadAutoref != Q_NULLPTR)
                        delete t_threadAutoref;
                    t_threadAutoref = new SerialThread(sp_portAutoref, "Autoref");
                    t_threadAutoref->transaction();
                    connect(t_threadAutoref,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Autoref);
                    msg = tr("Connexion autorefractomètre OK sur ") + m_portAutoref;
                }
                else
                {
                    msg =  tr("Impossible de connecter l'autorefractomètre sur ") + m_portAutoref;
                    UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter l'autorefractomètre sur ") + m_portAutoref, listeports);
                    if (sp_portAutoref != Q_NULLPTR)
                        delete sp_portAutoref;
                    sp_portAutoref = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter l'autorefractomètre sur ") + m_portAutoref, listeports);
        }
    }
    if (appareils.testFlag(Tonometre))
    {        
        m_portTono = m_settings->value(Param_Poste_PortTono).toString();
        if (m_portTono == "")
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur paramètrage connexion tonomomètre"));
        else
        {
            ReglePortTonometre();
            sp_portTono = openserialport(Tonometre);
            if (sp_portTono != Q_NULLPTR)
            {
                if (sp_portTono->open(QIODevice::ReadWrite))
                {
                    //qDebug() << "TONOMETRE -> " + m_portTono;
                    sp_portTono->setDataTerminalReady(true);
                    if (t_threadTono != Q_NULLPTR)
                        delete t_threadTono;
                    t_threadTono = new SerialThread(sp_portTono, "Tono");
                    t_threadTono->transaction();
                    connect(t_threadTono,  &SerialThread::newdatacom,     this, &Procedures::ReponsePortSerie_Tono);
                    msg = tr("Connexion tonomètre OK sur ") + m_portTono;
                }
                else
                {
                    msg =  tr("Impossible de connecter le tonomètre sur ") + m_portTono;
                    UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le tonoomètre sur ") + m_portTono, listeports);
                    if (sp_portTono != Q_NULLPTR)
                        delete sp_portTono;
                    sp_portTono = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(Q_NULLPTR, tr("Impossible de connecter le tonomètre sur ") + m_portTono, listeports);
        }
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
    InitialiseSerialSettings(serialset);
    QSerialPort *serialport;
    TypesAppareils appareilscom;
    switch (appareil) {
    case Fronto :
        baudrate    = Param_Poste_PortFronto_COM_baudrate;
        databits    = Param_Poste_PortFronto_COM_databits;
        parity      = Param_Poste_PortFronto_COM_parity;
        stopbits    = Param_Poste_PortFronto_COM_stopBits;
        flowcontrol = Param_Poste_PortFronto_COM_flowControl;
        if (sp_portFronto == Q_NULLPTR)
        {
            sp_portFronto = new QSerialPort();
            appareilscom.setFlag(Fronto);
        }
        serialport  = sp_portFronto;
        break;
    case Autoref :
        baudrate    = Param_Poste_PortAutoref_COM_baudrate;
        databits    = Param_Poste_PortAutoref_COM_databits;
        parity      = Param_Poste_PortAutoref_COM_parity;
        stopbits    = Param_Poste_PortAutoref_COM_stopBits;
        flowcontrol = Param_Poste_PortAutoref_COM_flowControl;
        if (sp_portAutoref == Q_NULLPTR)
        {
            sp_portAutoref = new QSerialPort();
            appareilscom.setFlag(Autoref);
        }
        serialport  = sp_portAutoref;
        break;
    case Refracteur :
        baudrate    = Param_Poste_PortRefracteur_COM_baudrate;
        databits    = Param_Poste_PortRefracteur_COM_databits;
        parity      = Param_Poste_PortRefracteur_COM_parity;
        stopbits    = Param_Poste_PortRefracteur_COM_stopBits;
        flowcontrol = Param_Poste_PortRefracteur_COM_flowControl;
        if (sp_portRefracteur == Q_NULLPTR)
        {
            sp_portRefracteur = new QSerialPort();
            appareilscom.setFlag(Refracteur);
        }
        serialport  = sp_portRefracteur;
        break;
    case Tonometre :
        baudrate    = Param_Poste_PortTono_COM_baudrate;
        databits    = Param_Poste_PortTono_COM_databits;
        parity      = Param_Poste_PortTono_COM_parity;
        stopbits    = Param_Poste_PortTono_COM_stopBits;
        flowcontrol = Param_Poste_PortTono_COM_flowControl;
        if (sp_portTono == Q_NULLPTR)
        {
            sp_portTono = new QSerialPort();
            appareilscom.setFlag(Tonometre);
        }
        serialport  = sp_portTono;
        break;
    default: return;
    }
    int index;
    QMetaEnum metaEnum;
    QString name ("");
    port = "COM"+ QString::number(map[PORT]);
    if (serialport != Q_NULLPTR)
        serialport->setBaudRate(serialset.baudRate);
    name = BAUDRATE;
    index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
    metaEnum = QSerialPort().metaObject()->enumerator(index);
    m_settings->setValue(baudrate, map[name]);
    serialset.baudRate = (QSerialPort::BaudRate)metaEnum.value(map[name]);
    if (serialport != Q_NULLPTR)
        serialport->setBaudRate(serialset.baudRate);
    name = DATABITS;
    index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
    metaEnum = QSerialPort().metaObject()->enumerator(index);
    m_settings->setValue(databits,  map[name]);
    serialset.dataBits = (QSerialPort::DataBits)metaEnum.value(map[name]);
    if (serialport != Q_NULLPTR)
        serialport->setDataBits(serialset.dataBits);
    name = PARITY;
    index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
    metaEnum = QSerialPort().metaObject()->enumerator(index);
    m_settings->setValue(parity, map[name]);
    serialset.parity = (QSerialPort::Parity)metaEnum.value(map[name]);
    if (serialport != Q_NULLPTR)
        serialport->setParity(serialset.parity);
    name = STOPBITS;
    index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
    metaEnum = QSerialPort().metaObject()->enumerator(index);
    m_settings->setValue(stopbits, map[name]);
    serialset.stopBits = (QSerialPort::StopBits)metaEnum.value(map[name]);
    if (serialport != Q_NULLPTR)
        serialport->setStopBits(serialset.stopBits);
    name = FLOWCONTROL;
    index = QSerialPort().metaObject()->indexOfEnumerator(name.toUtf8());
    metaEnum = QSerialPort().metaObject()->enumerator(index);
    m_settings->setValue(flowcontrol, map[name]);
    serialset.flowControl = (QSerialPort::FlowControl)metaEnum.value(map[name]);
    if (serialport != Q_NULLPTR)
        serialport->setFlowControl(serialset.flowControl);
    switch (appareil) {
    case Fronto :
        m_portFronto = port;
        s_paramPortSerieFronto = serialset;
        break;
    case Autoref :
        m_portAutoref = port;
        s_paramPortSerieAutoref = serialset;
        break;
    case Refracteur :
        m_portRefracteur = port;
        s_paramPortSerieRefracteur = serialset;
        break;
    case Tonometre :
        m_portTono = port;
        s_paramPortSerieTono = serialset;
        break;
    default: return;
    }
//    if (appareilscom>0 && m_mapports.size()>0)
//        Ouverture_Ports_Series(appareilscom);
}

bool Procedures::ReglePortRefracteur()
{
    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;

    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    InitialiseSerialSettings(s_paramPortSerieRefracteur);

    if (m_settings->value(Param_Poste_PortRefracteur_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_baudrate).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_databits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_parity).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.parity = (QSerialPort::Parity)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_stopBits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieRefracteur.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_flowControl).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
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
        s_paramPortSerieRefracteur.baudRate       = QSerialPort::Baud9600;
        s_paramPortSerieRefracteur.dataBits       = QSerialPort::Data8;
        s_paramPortSerieRefracteur.parity         = QSerialPort::NoParity;
        s_paramPortSerieRefracteur.stopBits       = QSerialPort::OneStop;
        s_paramPortSerieRefracteur.flowControl    = QSerialPort::NoFlowControl;
    }
    else a = false;

    if (a)
    {
        index       = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortRefracteur_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieRefracteur.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
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
    /*!
    qDebug() << "ReponsePortSerie_Refracteur(const QString &s)" << "s" << s;
    qDebug() << "ReponsePortSerie_Refracteur(const QString &s)" << "SendDataNidek(CRL)" << SendDataNIDEK("CRL");
    qDebug() << "ReponsePortSerie_Refracteur(const QString &s)" << "Utils::cleanByteArray(SendDataNIDEK(CRL)" << Utils::cleanByteArray(SendDataNIDEK("CRL"));
    //*/
    m_mesureSerie        = s;
    QString nameRF = m_settings->value(Param_Poste_Refracteur).toString();
    if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
    {
        if (m_mesureSerie == Utils::cleanByteArray(Nidek::I()->OKtoReceive("CRL")))
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

    //! Enregistre la mesure dans la base et met à jour les fiches
    InsertMesure(GenericProtocol::MesureRefracteur);
}

void Procedures::RegleRefracteur(GenericProtocol::TypesMesures flag)
{
    if (t_threadRefracteur!=Q_NULLPTR) /*! par le port COM */
    {
        QString nompat = "";
        Patient *pat = Datas::I()->patients->currentpatient();
        if (pat)
            nompat = pat->prenom() + " " + pat->nom().toUpper();
        //! NIDEK RT-5100 - NIDEK RT-2100
        QString name = m_settings->value(Param_Poste_Refracteur).toString();
        if (name =="NIDEK RT-5100" || name =="NIDEK RT-2100")
        {
            /*!
            Logs::LogToFile("PortSerieRefracteur.txt", "RTS = " + RequestToSendNIDEK() + " - "
                            + QDateTime().toString("dd-MM-yyyy HH:mm:ss")
                            + (nompat != ""? " - " : "") + nompat);
            qDebug() << "RegleRefracteur(TypesMesures flag) - RTS Nidek = " + RequestToSendNIDEK();
            //*/
            m_flagreglagerefracteurNidek = flag;
            Utils::writeDatasSerialPort(PortRefracteur(), Nidek::I()->RequestToSend(), " RequestToSendNIDEK() - Refracteur = ");
        }
        //! TOMEY TAP-2000 et Rodenstock Phoromat 2000
        else if (name =="TOMEY TAP-6000" || name =="RODENSTOCK Phoromat 2000")
            RegleRefracteurCOM(flag);
    }
    //! NIDEK RT-6100 - NIDEK Glasspop
    else if (m_LANRefracteur) /*! par le réseau */
        RegleRefracteurXML(flag);
}

void Procedures::RegleRefracteurCOM(GenericProtocol::TypesMesures flag)
{
    /*! Si on lance cette fonction à l'ouverture d'un dossier, on a créé 3 mesures
     * Chacune de ces 3 mesures est envoyée au réfracteur pour le régler
     * Datas::I()->mesurefronto     qui met en fronto et en final la dernière prescription de verres du patient
     * Datas::I()->mesureautoref    qui met en autoref la dernière mesure d'autoref du patient
     * Datas::I()->mesureacuité     qui met en subjectif la dernière mesure d'acuité du patient
     */
    /*! +++ sur les NIDEK, on ne peut que régler l'autoref et le fronto depuis le PC - les refractions subjectives et finales ne peuvent pas être préréglées */
    QString AxeOD, AxeOG;
    QString AddOD, AddOG;
    QString SphereOD, SphereOG;
    QString CylindreOD, CylindreOG;
    QString SCAOD, SCAOG;
    QByteArray DTRbuff = QByteArray();
    QString nameRF = m_settings->value(Param_Poste_Refracteur).toString();

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

    if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
    {
        DTRbuff = Nidek::I()->RegleRefracteurCOM(flag);
        if (DTRbuff != QByteArray())
            Utils::writeDatasSerialPort(PortRefracteur(), DTRbuff, " DTRbuff - Refracteur = ", 1000);
    }

    // TOMEY TAP-2000 et Rodenstock Phoromat 2000 =======================================================================================================================================
    else if (nameRF =="TOMEY TAP-6000" || nameRF =="RODENSTOCK Phoromat 2000")
    {
        /*! SORTIE EXEMPLE POUR UN PHOROMAT RODENSTOCK
         * SOH =    SOH            //SOH -> start of header
         * STX =    STX            //STX -> start of text
         * ETB =    ETB           //ETB -> end of text block
         * EOT =    EOT            //EOT -> end of transmission
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
        DTRbuff.append(SOH);                                                //SOH -> start of header
        DTRbuff.append(Utils::StringToArray("*PC_SND_S"));
        DTRbuff.append(EOT);                                                //EOT -> end of transmission
        DTRbuff.append(CR);                                                 //CR LF
        DTRbuff.append(LF);                                                 //CR LF

        DTRbuff.append(STX);                                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray("*Phoromat 2000|000000001|0"));
        DTRbuff.append(ETB);                                                //ETB -> end of text block
        DTRbuff.append(CR);                                                 //CR LF
        DTRbuff.append(LF);                                                 //CR LF

        /*! écart interpupillaire
            *PD|32.0|32.0|                  ->PD | left PD result | right PD result |        */
        DTRbuff.append(STX);                                                //STX -> start of text
        double eip = static_cast<double>(Datas::I()->mesureautoref->ecartIP());
        QString halfeip = QString::number(eip/2,'f',1);                     // quel bricolage nul....
        DTRbuff.append(Utils::StringToArray("*PD|" + halfeip + "|" + halfeip + "|"));
        DTRbuff.append(ETB);                                                //ETB -> end of text block
        DTRbuff.append(CR);                                                 //CR LF
        DTRbuff.append(LF);                                                 //CR LF

        /*! réglage du fronto`
            *LM                             -> Fronto
            *SP| -0.25| -0.25|              -> Sphere | left result | right result|
            *CY| -0.25| -0.25|              -> Cylindre | left result | right result|
            *AX|135|135|                    -> Axe | left result | right result|
            *AD| 1.50| 1.50|                -> Addition | left result | right result|        */
        if (flag.testFlag(GenericProtocol::MesureFronto) && !Datas::I()->mesurefronto->isdataclean())
        {
            initvariables();

            convertaxeTOMEY(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
            convertaxeTOMEY(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
            convertdioptriesTOMEY(SphereOD, Datas::I()->mesurefronto->sphereOD());
            convertdioptriesTOMEY(SphereOG, Datas::I()->mesurefronto->sphereOG());
            convertdioptriesTOMEY(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
            convertdioptriesTOMEY(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

            AddOD       = " " + QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2);
            AddOG       = " " + QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2);

            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*LM"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*SP|"+ SphereOG + "|" + SphereOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*CY|"+ CylindreOG + "|" + CylindreOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*AX|"+ AxeOG + "|" + AxeOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*AD|"+ AddOG + "|" + AddOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
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
        if (flag.testFlag(GenericProtocol::MesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
        {
            initvariables();
            convertaxeTOMEY(AxeOD, Datas::I()->mesureautoref->axecylindreOD());
            convertaxeTOMEY(AxeOG, Datas::I()->mesureautoref->axecylindreOG());
            convertdioptriesTOMEY(SphereOD, Datas::I()->mesureautoref->sphereOD());
            convertdioptriesTOMEY(SphereOG, Datas::I()->mesureautoref->sphereOG());
            convertdioptriesTOMEY(CylindreOD, Datas::I()->mesureautoref->cylindreOD());
            convertdioptriesTOMEY(CylindreOG, Datas::I()->mesureautoref->cylindreOG());

            AddOD       = " " + QString::number(Datas::I()->mesureautoref->addVPOD(),'f',2);
            AddOG       = " " + QString::number(Datas::I()->mesureautoref->addVPOG(),'f',2);

            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*AR"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*SP|"+ SphereOG + "|" + SphereOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*CY|"+ CylindreOG + "|" + CylindreOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*AX|"+ AxeOG + "|" + AxeOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*AD|"+ AddOG + "|" + AddOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            /*!
            if (idpat == 0)
                idpat = Datas::I()->mesurefronto->idpatient();
            */
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
        if (flag.testFlag(GenericProtocol::MesureRefracteur) && !Datas::I()->mesurefronto->isdataclean())
        {
            initvariables();

            convertaxeTOMEY(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
            convertaxeTOMEY(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
            convertdioptriesTOMEY(SphereOD, Datas::I()->mesurefronto->sphereOD());
            convertdioptriesTOMEY(SphereOG, Datas::I()->mesurefronto->sphereOG());
            convertdioptriesTOMEY(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
            convertdioptriesTOMEY(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

            AddOD       = " " + QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2);
            AddOG       = " " + QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2);

            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*SJ"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*SP|"+ SphereOG + "|" + SphereOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*CY|"+ CylindreOG + "|" + CylindreOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*AX|"+ AxeOG + "|" + AxeOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            DTRbuff.append(STX);                                            //STX -> start of text
            DTRbuff.append(Utils::StringToArray("*AD|"+ AddOG + "|" + AddOD + "|"));
            DTRbuff.append(ETB);                                            //ETB -> end of text block
            DTRbuff.append(CR);                                                 //CR LF
            DTRbuff.append(LF);                                                 //CR LF
            /*!
            if (idpat == 0)
                idpat = Datas::I()->mesurefronto->idpatient();
            */
        }
        /*! séquence de fin
            SOH*PC_SND_EEOT
        */
        DTRbuff.append(SOH);                                                //SOH -> start of header
        DTRbuff.append(Utils::StringToArray("*PC_SND_E"));
        DTRbuff.append(EOT);                                                //EOT -> end of transmission
        DTRbuff.append(CR);                                                 //CR LF
        DTRbuff.append(LF);                                                 //CR LF

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
        Utils::writeDatasSerialPort(PortRefracteur(), DTRbuff, " DTRbuff - Refracteur = ", 1000);    }
    // FIN TOMEY TAP-2000 et Rodenstock Phoromat 2000 =======================================================================================================================================
}

void Procedures::RegleRefracteurXML(GenericProtocol::TypesMesures flag)
{
    /*! Si on lance cette fonction à l'ouverture d'un dossier, on a créé 3 mesures
     * Chacune de ces 3 mesures est envoyée au réfracteur pour le régler
     * Datas::I()->mesurefronto     qui met en fronto et en final la dernière prescription de verres du patient
     * Datas::I()->mesureautoref    qui met en autoref la dernière mesure d'autoref du patient
     * Datas::I()->mesureacuité     qui met en subjectif la dernière mesure d'acuité du patient
     */

    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    if (nameRF == "TOPCON CV-5000")
    {
        Topcon::I()->RegleRefracteurXML(flag, nameRF);
    }

    if (nameRF == "NIDEK RT-6100" || nameRF == "NIDEK Glasspop" )
    {
        Nidek::I()->RegleRefracteurXML(flag, nameRF);
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
    User * usrcptble = Datas::I()->users->getById(usr->idcomptableactes());
    if ( usr->idcomptableactes() == User::ROLE_NON_RENSEIGNE )
        strComptable = tr("sans objet");
    else if ( usr->idcomptableactes() == User::ROLE_VIDE )
        strComptable = tr("sans objet");
    else if ( usr->idcomptableactes() == User::ROLE_INDETERMINE )
        strComptable = tr("indéterminé");
    else if (usrcptble)
        strComptable = usrcptble->login();
    str += tr("comptable") + "\t\t= " + (usrcptble? strComptable : "null") + "\n";
    if ( usrcptble )
    {
        Compte * cpt = Datas::I()->comptes->getById(usrcptble->idcompteencaissementhonoraires());
        str += tr("cpte banque") + "\t= " + (cpt? cpt->nomabrege() : "null") + "\n";
    }

    return str;
}


bool Procedures::isUserConnected(User *usr)
{
    if (Datas::I()->postesconnectes->admin(Item::NoUpdate) == Q_NULLPTR)
        Datas::I()->postesconnectes->MAJlistePostesConnectes();
    for (auto it = Datas::I()->postesconnectes->postesconnectes()->constBegin(); it !=  Datas::I()->postesconnectes->postesconnectes()->constEnd(); ++it)
    {
        PosteConnecte *post = const_cast<PosteConnecte*>(it.value());
        if (post->iduser() == usr->id())
            return true;
    }
    return false;
}

void Procedures::debugMesure(QObject *mesure, QString titre)
{
    bool a = true;
    if (a)
        return;
    if (titre != "" && mesure != Q_NULLPTR)
        return;

    /*!
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
    //*/
}

void Procedures::EnvoiDataPatientAuRefracteur()
{
    RegleRefracteur(GenericProtocol::MesureAutoref | GenericProtocol::MesureFronto | GenericProtocol::MesureRefracteur);
}

void Procedures::LectureDonneesCOMRefracteur(QString Mesure)
{
    //qDebug() << "LectureDonneesCOMRefracteur(QString Mesure)" << Mesure;
    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString mAddOD      = "+0.00";
    QString mAddOG      = "+0.00";
    QString AVLOD(""), AVLOG("");
    QString PD          = "";
    int     idx;

    QString nameRF = m_settings->value(Param_Poste_Refracteur).toString();

    // TRADUCTION DES DONNEES EN FONCTION DU REFRACTEUR
    // NIDEK RT-5100 - RT-2100 =======================================================================================================================================
    if (nameRF =="NIDEK RT-5100" || nameRF =="NIDEK RT-2100")
    {
        GenericProtocol::TypesMesures flag = GenericProtocol::MesureNone;
        if (Mesure.contains("@LM") && PortFronto() == Q_NULLPTR && !m_LANFronto)             //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur
            flag.setFlag(GenericProtocol::MesureFronto);
        if (Mesure.contains("@KM") && PortAutoref() == Q_NULLPTR && !m_LANAutoref)           //!=> il y a une mesure de keratométrie et l'autoref est connecté directement à la box du refraacteur
            flag.setFlag(GenericProtocol::MesureKerato);
        if (Mesure.contains("@RM") && PortAutoref() == Q_NULLPTR && !m_LANAutoref)           //!=> il y a une mesure de refractometrie et l'autoref est directement branché sur la box du refracteur
            flag.setFlag(GenericProtocol::MesureAutoref);
        if (Mesure.contains("@RT"))                                                          //!=> il y a une mesure de refraction
            flag.setFlag(GenericProtocol::MesureRefracteur);
        if (Mesure.contains("@NT") && PortAutoref() == Q_NULLPTR &&  !m_LANAutoref )         //!=> il y a une mesure de tonometrie et l'autoref est branché sur la box du refracteur
            flag.setFlag(GenericProtocol::MesureTono);
        Nidek::I()->LectureDonneesCOMRefracteur(Mesure, flag);
        //debugMesure(Datas::I()->mesurekerato, "Procedures::LectureDonneesRefracteur(QString Mesure)");
    }

    // TOMEY TAP-2000 et Rodenstock Phoromat 2000 =======================================================================================================================================
    else if (nameRF =="TOMEY TAP-2000" || nameRF =="RODENSTOCK Phoromat 2000")
    {
        /*! SORTIE EXEMPLE POUR UN PHOROMAT RODENSTOCK
         * SOH =    SOH            //SOH -> start of header
         * STX =    STX            //STX -> start of text
         * ETB =    ETB           //ETB -> end of text block
         * EOT =    EOT            //EOT -> end of transmission
         * La 1ere et la dernière lignes commencent par SOH et se terminent par EOT - représentés ici
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
*VA|0.2|0.4|0.5|                -> left result | both | right result|
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
                mSphereOG   = mSphere.split("|").at(0);
                mSphereOD   = mSphere.split("|").at(1);
                Datas::I()->mesurefronto->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                Datas::I()->mesurefronto->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
            }
            // CYLINDRE ---------------------------------------------------------------------------
            if (SectionFronto.contains("*CY"))
            {
                mCyl     = SectionFronto.mid(SectionFronto.indexOf("CY")+3,13)   .replace(" ","");
                mCylOG   = mCyl.split("|").at(0);
                mCylOD   = mCyl.split("|").at(1);
                Datas::I()->mesurefronto->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                Datas::I()->mesurefronto->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
            }
            // AXE ---------------------------------------------------------------------------
            if (SectionFronto.contains("*AX"))
            {
                mAxe     = SectionFronto.mid(SectionFronto.indexOf("AX")+3,7)   .replace(" ","");
                mAxeOG   = mAxe.split("|").at(0);
                mAxeOD   = mAxe.split("|").at(1);
                Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            }
            // ADD ---------------------------------------------------------------------------
            if (SectionFronto.contains("*AD"))
            {
                mAdd     = SectionFronto.mid(SectionFronto.indexOf("AD")+3,11)   .replace(" ","");
                mAddOG   = mAdd.split("|").at(0);
                mAddOD   = mAdd.split("|").at(1);
                Datas::I()->mesurefronto->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                Datas::I()->mesurefronto->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
            }
            //debugMesureRefraction(Datas::I()->mesurefronto);
            if (Datas::I()->mesurefronto->isDifferent(oldMesureFronto) && !Datas::I()->mesurefronto->isdataclean())
            {
                InsertMesure(GenericProtocol::MesureFronto);
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
                mSphereOG   = mSphere.split("|").at(0);
                mSphereOD   = mSphere.split("|").at(1);
                Datas::I()->mesureautoref->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                Datas::I()->mesureautoref->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
            }
            // CYLINDRE ---------------------------------------------------------------------------
            if (SectionAutoref.contains("*CY"))
            {
                mCyl     = SectionAutoref.mid(SectionAutoref.indexOf("CY")+3,13)   .replace(" ","");
                mCylOG   = mCyl.split("|").at(0);
                mCylOD   = mCyl.split("|").at(1);
                Datas::I()->mesureautoref->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                Datas::I()->mesureautoref->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
            }
            // AXE ---------------------------------------------------------------------------
            if (SectionAutoref.contains("*AX"))
            {
                mAxe     = SectionAutoref.mid(SectionAutoref.indexOf("AX")+3,7)   .replace(" ","");
                mAxeOG   = mAxe.split("|").at(0);
                mAxeOD   = mAxe.split("|").at(1);
                Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            }
            // ADD ---------------------------------------------------------------------------
            if (SectionAutoref.contains("*AD"))
            {
                mAdd     = SectionAutoref.mid(SectionAutoref.indexOf("AD")+3,11)   .replace(" ","");
                mAddOG   = mAdd.split("|").at(0);
                mAddOD   = mAdd.split("|").at(1);
                Datas::I()->mesureautoref->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                Datas::I()->mesureautoref->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
            }
            //debugMesureRefraction(Datas::I()->mesureautoref);
            if (Datas::I()->mesureautoref->isDifferent(oldMesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
            {
                InsertMesure(GenericProtocol::MesureAutoref);
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
                    mSphereOG   = mSphere.split("|").at(0);
                    mSphereOD   = mSphere.split("|").at(1);
                    Datas::I()->mesureacuite->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                    Datas::I()->mesureacuite->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
                }
                // CYLINDRE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*CY"))
                {
                    mCyl     = SectionRefracteur.mid(SectionRefracteur.indexOf("CY")+3,13)   .replace(" ","");
                    mCylOG   = mCyl.split("|").at(0);
                    mCylOD   = mCyl.split("|").at(1);
                    Datas::I()->mesureacuite->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                    Datas::I()->mesureacuite->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
                }
                // AXE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AX"))
                {
                    mAxe     = SectionRefracteur.mid(SectionRefracteur.indexOf("AX")+3,7)   .replace(" ","");
                    mAxeOG   = mAxe.split("|").at(0);
                    mAxeOD   = mAxe.split("|").at(1);
                    Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                    Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                }
                // ADD ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AD"))
                {
                    mAdd     = SectionRefracteur.mid(SectionRefracteur.indexOf("AD")+3,11)   .replace(" ","");
                    mAddOG   = mAdd.split("|").at(0);
                    mAddOD   = mAdd.split("|").at(1);
                    Datas::I()->mesureacuite->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                    Datas::I()->mesureacuite->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
                }
                // ACUITE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*VA"))
                {
                    AVL     = SectionRefracteur.mid(SectionRefracteur.indexOf("VA")+3);
                    AVLOG   = AVL.split("|").at(0);
                    AVLOD   = AVL.split("|").at(2);
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
                    mSphereOG   = mSphere.split("|").at(0);
                    mSphereOD   = mSphere.split("|").at(1);
                    Datas::I()->mesurefinal->setsphereOD(Utils::roundToNearestPointTwentyFive(mSphereOD.toDouble()));
                    Datas::I()->mesurefinal->setsphereOG(Utils::roundToNearestPointTwentyFive(mSphereOG.toDouble()));
                }
                // CYLINDRE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*CY"))
                {
                    mCyl     = SectionRefracteur.mid(SectionRefracteur.indexOf("CY")+3,13)   .replace(" ","");
                    mCylOG   = mCyl.split("|").at(0);
                    mCylOD   = mCyl.split("|").at(1);
                    Datas::I()->mesurefinal->setcylindreOD(Utils::roundToNearestPointTwentyFive(mCylOD.toDouble()));
                    Datas::I()->mesurefinal->setcylindreOG(Utils::roundToNearestPointTwentyFive(mCylOG.toDouble()));
                }
                // AXE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AX"))
                {
                    mAxe     = SectionRefracteur.mid(SectionRefracteur.indexOf("AX")+3,7)   .replace(" ","");
                    mAxeOG   = mAxe.split("|").at(0);
                    mAxeOD   = mAxe.split("|").at(1);
                    Datas::I()->mesurefinal->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                    Datas::I()->mesurefinal->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                }
                // ADD ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*AD"))
                {
                    mAdd     = SectionRefracteur.mid(SectionRefracteur.indexOf("AD")+3,11)   .replace(" ","");
                    mAddOG   = mAdd.split("|").at(0);
                    mAddOD   = mAdd.split("|").at(1);
                    Datas::I()->mesurefinal->setaddVPOD(Utils::roundToNearestPointTwentyFive(mAddOD.toDouble()));
                    Datas::I()->mesurefinal->setaddVPOG(Utils::roundToNearestPointTwentyFive(mAddOG.toDouble()));
                }
                // ACUITE ---------------------------------------------------------------------------
                if (SectionRefracteur.contains("*VA"))
                {
                    AVL     = SectionRefracteur.mid(SectionRefracteur.indexOf("VA")+3);
                    AVLOG   = AVL.split("|").at(0);
                    AVLOD   = AVL.split("|").at(2);
                    Datas::I()->mesurefinal->setavlOD(AVLOD);
                    Datas::I()->mesurefinal->setavlOG(AVLOG);
                }
            }
        }
    }
    // FIN TOMEY TAP-2000 et Rodenstock Phoromat 2000 ==========================================================================================================================}
}

void Procedures::LectureDonneesXMLRefracteur(QDomDocument docxml)
{
    Logs::LogToFile("MesuresRefracteur.txt", docxml.toByteArray());
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    if (nameRF == "TOPCON CV-5000" )
    {
        Topcon::I()->LectureDonneesXMLRefracteur(docxml, nameRF);
    }
    else if (nameRF == "NIDEK RT-6100" || nameRF == "NIDEK Glasspop")
    {
        GenericProtocol::TypesMesures flag = GenericProtocol::MesureNone;
        if (PortFronto() == Q_NULLPTR && !m_LANFronto)             //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur)
            flag.setFlag(GenericProtocol::MesureFronto);
        if (PortAutoref() == Q_NULLPTR && !m_LANAutoref)           //!=> il y a une mesure pour l'autoref et l'autoref est directement branché sur la box du refracteur)
        {
            flag.setFlag(GenericProtocol::MesureAutoref);
            flag.setFlag(GenericProtocol::MesureKerato);
            flag.setFlag(GenericProtocol::MesureTono);
            flag.setFlag(GenericProtocol::MesurePachy);
        }
        Nidek::I()->LectureDonneesXMLRefracteur(docxml, flag);
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

    QString tono = m_settings->value(Param_Poste_Tono).toString();
    if (tono=="TOMEY TOP-1000" || tono == "RODENSTOCK Topascope" )
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


bool Procedures::ReglePortTonometre()
{
    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;

    InitialiseSerialSettings(s_paramPortSerieTono);
    if (m_settings->value(Param_Poste_PortTono_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_baudrate).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_databits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_parity).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.parity = (QSerialPort::Parity)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_stopBits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_flowControl).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieTono.flowControl = (QSerialPort::FlowControl)metaEnum.value(val);
    }
    if (a)
    {
        index       = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortTono_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieTono.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
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
    return Reponse;
}

bool Procedures::ReglePortFronto()
{
    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;
    QString nameLM = m_settings->value(Param_Poste_Fronto).toString();
    InitialiseSerialSettings(s_paramPortSerieFronto);
    if (m_settings->value(Param_Poste_PortFronto_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_baudrate).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_databits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_parity).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.parity = (QSerialPort::Parity)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_stopBits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieFronto.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_flowControl).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
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
        index       = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortFronto_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieFronto.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
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


//-----------------------------------------------------------------------------------------
// Lecture du flux de données sur le port série du tonometre
//-----------------------------------------------------------------------------------------
void Procedures::ReponsePortSerie_Tono(const QString &s)
{
    m_mesureSerie        = s;

    QString tono = m_settings->value(Param_Poste_Tono).toString();
    if (tono=="TOMEY TOP-1000" || tono == "RODENSTOCK Topascope" )
    {
        Datas::I()->mesuretono->cleandatas();
        Tomey::I()->LectureDonneesTOP1000( m_mesureSerie);
        if (Datas::I()->mesuretono->isdataclean())
            return;

        //! Enregistre les mesures dans la base et met à jour les fiches
        if (!Datas::I()->mesuretono->isdataclean())
            InsertMesure(GenericProtocol::MesureTono);
        if (!Datas::I()->mesurepachy->isdataclean())
            InsertMesure(GenericProtocol::MesurePachy);
    }
    else
    {
           Utils::EnChantier();
    }
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
    //qDebug() << "void Procedures::ReponsePortSerie_Fronto(const QString &s)" << m_mesureSerie;
    QString nameLM    = m_settings->value(Param_Poste_Fronto).toString();

    if (nameLM == "NIDEK LM-1800P"
     || nameLM == "NIDEK LM-1800PD"
     || nameLM == "NIDEK LM-500")
    {
        if (m_mesureSerie ==  Nidek::I()->RequestToSend())          //! le fronto demande la permission d'envoyer des données
        {
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            //PortFronto()->waitForReadyRead(100);
            Utils::writeDatasSerialPort(PortFronto(), Nidek::I()->OKtoReceive("CLM"), " SendDataNIDEK(CLM) - Fronto = ");
            return;
        }
    }
    Datas::I()->mesurefronto->cleandatas();
    LectureDonneesCOMFronto(m_mesureSerie);
    if (Datas::I()->mesurefronto->isdataclean())
        return;

    //! Enregistre la mesures dans la base et met à jour les fiches
    InsertMesure(GenericProtocol::MesureFronto);

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    /*! règle le flag de reglage du refracteur sur Fronto seulement */
    RegleRefracteur(GenericProtocol::MesureFronto);
}



void Procedures::LectureDonneesCOMFronto(QString Mesure)
{
    //qDebug() << "LectureDonneesCOMFronto(QString Mesure)" << Mesure;
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
    if (nameLM == "TOMEY TL-3000C")
    {
        //Edit(Mesure);
        /* Le fichier de sortie ressemble à ça
        LM
        LR- 3.75- 0.75 99
        AR1.75
        PR
        DR
        LL- 4.25- 0.50 99
        AL2.25
        PL
        DL
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
                mAddOD           = Mesure.mid(Mesure.indexOf("AR")+2,4);
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
                mAddOG           = Mesure.mid(Mesure.indexOf("AL")+2,4);
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
        //les autres champs ne sont pas utilisés pour le moment -------------------------------
    }
    else if (nameLM == "VISIONIX VL1000"
          || nameLM == "HUVITZ CLM7000")
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
    else if (nameLM == "NIDEK LM-1800P"
          || nameLM == "NIDEK LM-1800PD"
          || nameLM == "NIDEK LM-500")
        Nidek::I()->LectureDonneesCOMFronto(Mesure);
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
    bool a = true;
    int val(-1), index(-1);
    QMetaEnum metaEnum;
    QString nameARK = m_settings->value(Param_Poste_Autoref).toString();
    InitialiseSerialSettings(s_paramPortSerieAutoref);
    if (m_settings->value(Param_Poste_PortAutoref_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_baudrate).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.baudRate = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_databits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.dataBits = (QSerialPort::DataBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_parity).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.parity = (QSerialPort::Parity)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_stopBits).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
        s_paramPortSerieAutoref.stopBits = (QSerialPort::StopBits)metaEnum.value(val);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_flowControl).toInt();
        index = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
        QMetaEnum metaEnum = QSerialPort().metaObject()->enumerator(index);
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
        index       = QSerialPort().metaObject()->indexOfEnumerator("BaudRate");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_baudrate,       Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.baudRate));
        index       = QSerialPort().metaObject()->indexOfEnumerator("DataBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_databits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.dataBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("Parity");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_parity,         Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.parity));
        index       = QSerialPort().metaObject()->indexOfEnumerator("StopBits");
        metaEnum    = QSerialPort().metaObject()->enumerator(index);
        m_settings  ->setValue(Param_Poste_PortAutoref_COM_stopBits,       Utils::getindexFromValue(metaEnum, s_paramPortSerieAutoref.stopBits));
        index       = QSerialPort().metaObject()->indexOfEnumerator("FlowControl");
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

    if (nameARK == "HUVITZ HTR-1A")
        return;
    if (nameARK == "NIKON Speedy-K")
        nameARK = "HUVITZ HTR-1A";

    bool autorefhaskerato    = (nameARK =="NIDEK ARK-1A"
                      || nameARK =="NIDEK ARK-1"
                      || nameARK =="NIDEK ARK-1S"
                      || nameARK =="NIDEK ARK-530A"
                      || nameARK =="NIDEK ARK-510A"
                      || nameARK =="NIDEK HandyRef-K"
                      || nameARK =="NIDEK TONOREF III"
                      || nameARK =="NIDEK ARK-30"
                      || nameARK == "HUVITZ HTR-1A");
    bool autorefhastonopachy = (nameARK =="NIDEK TONOREF III" || nameARK == "HUVITZ HTR-1A");
    Datas::I()->mesureautoref   ->cleandatas();
    if (autorefhaskerato)
        Datas::I()->mesurekerato    ->cleandatas();
    if (autorefhastonopachy)
    {
        Datas::I()->mesurepachy     ->cleandatas();
        Datas::I()->mesuretono      ->cleandatas();
    }

    LectureDonneesXMLAutoref(xmldoc);

    //! Enregistre la mesures dans la base et met à jour les fiches
    if (!Datas::I()->mesurekerato->isdataclean())
        InsertMesure(GenericProtocol::MesureKerato);
    if (!Datas::I()->mesureautoref->isdataclean())
        InsertMesure(GenericProtocol::MesureAutoref);
    if (autorefhastonopachy)
    {
        if (!Datas::I()->mesuretono->isdataclean())
            InsertMesure(GenericProtocol::MesureTono);                     //! depuis ReponsePortSerie_Autoref(const QString &s)
        if (!Datas::I()->mesurepachy->isdataclean())
            InsertMesure(GenericProtocol::MesurePachy);                    //! depuis ReponsePortSerie_Autoref(const QString &s)
    }

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    /*! règle le flag de reglage du refracteur sur Autoref seulement */
    RegleRefracteur(GenericProtocol::MesureAutoref);
}

//! lire les fichiers xml des appareils de refraction
//! -----------------------------------------------------------------------------------------
//! Lecture du fichier CSV du fronto
//! -----------------------------------------------------------------------------------------
void Procedures::ReponseCSV_Fronto(const QString filecontents)
{
    Datas::I()->mesurefronto->cleandatas();
    LectureDonneesCSVFronto(filecontents);
    if (Datas::I()->mesurefronto->isdataclean())
        return;

    //! Enregistre la mesures dans la base et met à jour les fiches
    InsertMesure(GenericProtocol::MesureFronto);

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    /*! règle le flag de reglage du refracteur sur Fronto seulement */
    RegleRefracteur(GenericProtocol::MesureFronto);
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

    //! Enregistre la mesures dans la base et met à jour les fiches
    InsertMesure(GenericProtocol::MesureFronto);

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    /*! règle le flag de reglage du refracteur sur Fronto seulement */
    RegleRefracteur(GenericProtocol::MesureFronto);
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

    //! Enregistre les mesures dans la base et met à jour les fiches
    InsertMesure(GenericProtocol::MesureRefracteur);
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
        InsertMesure(GenericProtocol::MesureTono);
    if (!Datas::I()->mesurepachy->isdataclean())
        InsertMesure(GenericProtocol::MesurePachy);
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
    bool autorefhaskerato    = (nameARK == "NIDEK ARK-1A"
                      || nameARK == "NIDEK ARK-1"
                      || nameARK == "NIDEK ARK-1S"
                      || nameARK == "NIDEK ARK-530A"
                      || nameARK == "NIDEK ARK-510A"
                      || nameARK == "NIDEK HandyRef-K"
                      || nameARK == "NIDEK TONOREF III"
                      || nameARK == "NIDEK ARK-30");
    bool autorefhastonopachy = (nameARK == "NIDEK TONOREF III");

    if (nameARK == "NIDEK ARK-1A"
     || nameARK == "NIDEK ARK-1"
     || nameARK == "NIDEK ARK-1S"
     || nameARK == "NIDEK AR-1A"
     || nameARK == "NIDEK AR-1"
     || nameARK == "NIDEK AR-1S"
     || nameARK == "NIDEK ARK-530A"
     || nameARK == "NIDEK ARK-510A"
     || nameARK == "NIDEK HandyRef-K"
     || nameARK == "NIDEK TONOREF III"
     || nameARK == "NIDEK ARK-30"
     || nameARK == "NIDEK AR-20")
    {
        if (m_mesureSerie ==  Nidek::I()->RequestToSend())       //! l'autoref demande la permission d'envoyer des données
        {
            QString cmd;
            cmd = (autorefhaskerato? "CRK" : "CRM");     //! CRK ou CRM suivant que les appareils peuvent ou non envoyer la keratométrie
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            //PortAutoref()->waitForReadyRead(100);
            Utils::writeDatasSerialPort(PortAutoref(), Nidek::I()->OKtoReceive(cmd), " SendDataNIDEK(cmd) - Autoref = ");
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
        InsertMesure(GenericProtocol::MesureKerato);
    if (!Datas::I()->mesureautoref->isdataclean())
        InsertMesure(GenericProtocol::MesureAutoref);
    if (autorefhastonopachy)
    {
        if (!Datas::I()->mesuretono->isdataclean())
            InsertMesure(GenericProtocol::MesureTono);                     //! depuis ReponsePortSerie_Autoref(const QString &s)
        if (!Datas::I()->mesurepachy->isdataclean())
            InsertMesure(GenericProtocol::MesurePachy);                    //! depuis ReponsePortSerie_Autoref(const QString &s)
    }

    //! TRANSMETTRE LES DONNEES AU REFRACTEUR --------------------------------------------------------------------------------------------------------------------------------------------------------
    /*! règle le flag de reglage du refracteur sur Autoref seulement */
    RegleRefracteur(GenericProtocol::MesureAutoref);
}

void Procedures::LectureDonneesCOMAutoref(QString Mesure)
{
    Logs::LogToFile("MesuresAutoref.txt", Mesure);

    QString nameARK   = m_settings->value(Param_Poste_Autoref).toString();
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

    if (nameARK == "NIDEK ARK-1A"
     || nameARK == "NIDEK ARK-1"
     || nameARK == "NIDEK ARK-1S"
     || nameARK == "NIDEK AR-1A"
     || nameARK == "NIDEK AR-1"
     || nameARK == "NIDEK AR-1S"
     || nameARK == "NIDEK ARK-530A"
     || nameARK == "NIDEK ARK-510A"
     || nameARK == "NIDEK HandyRef-K"
     || nameARK == "NIDEK TONOREF III"
     || nameARK == "NIDEK ARK-30"
     || nameARK == "NIDEK AR-20")
        Nidek::I()->LectureDonneesCOMAutoref(Mesure, nameARK);

    else if (nameARK == "TOMEY RC-5000" || nameARK == "RODENSTOCK CX 2000")
        Tomey::I()->LectureDonneesRC5000Form(Mesure);

    //qDebug() << "od" << mSphereOD << mCylOD << mAxeOD << "og" << mSphereOG << mCylOG << mAxeOG << "PD = " + PD;
}

void Procedures::LectureDonneesXMLAutoref(QDomDocument docxml)
{
    Logs::LogToFile("MesuresAutoref.txt", docxml.toByteArray());
    QString nameARK = m_settings->value(Param_Poste_Autoref).toString();
      if (nameARK == "HUVITZ HTR-1A")
          return;
      if (nameARK == "NIKON Speedy-K")
          nameARK = "HUVITZ HTR-1A";
    if (nameARK == "NIDEK ARK-1A"
     || nameARK == "NIDEK ARK-1"
     || nameARK == "NIDEK ARK-1S"
     || nameARK == "NIDEK AR-1A"
     || nameARK == "NIDEK AR-1"
     || nameARK == "NIDEK AR-1S"
     || nameARK == "NIDEK ARK-530A"
     || nameARK == "NIDEK ARK-510A"
     || nameARK == "NIDEK HandyRef-K"
     || nameARK == "NIDEK TONOREF III"
     || nameARK == "NIDEK ARK-30"
     || nameARK == "NIDEK AR-20")
    {
        Nidek::I()->LectureDonneesXMLAutoref(docxml, nameARK);
    }
/*! exemple de fichier xml pour un HUVITZ HTR-1A
 *
 *
<datalab>
        <patient>00004<\patient>
        <date>2022/04/26<\date>
        <version>1.00.00c<\version>
        <ref>
            <rrs>-0.25<\rrs>
            <rrc>-0.5<\rrc>
            <rra>7<\rra>
            <rrp>68<\rrp>
            <rls>-0.5<\rls>
            <rlc>-0.5<\rlc>
            <rla>165<\rla>
            <rlp>68<\rlp>
        <\ref>
        <ker>
            <krr1>8.22<\krr1>
            <krr2>8.12<\krr2>
            <krax>176<\krax>
            <klr1>8.2<\klr1>
            <klr2>8.07<\klr2>
            <klax>179<\klax>
        <\ker>
        <tono>
            <tr01>16<\tr01>
            <tr>16<\tr>
            <tl01>15<\tl01>
            <tl>15<\tl>
        <\tono>
        <pachy>
            <pr01>541<\pr01>
            <pr>541.8<\pr>
             <pl01>561<\pl01>
             <pl>561.3<\pl>
        <\pachy>
        <misc>
            <C-R01><\C-R01>
            <C-R-A><\C-R-A>
            <C-L01><\C-L01>
            <C-L-A><\C-L-A>
        <\misc>
<\datalab>
*/
    else if (nameARK == "HUVITZ HTR-1A")
    {
        QDomElement datalab = docxml.documentElement();
        QDomElement ref = datalab.firstChildElement("ref");
        if(!ref.isNull())
        {
            // Ref droit
            QDomElement rrs = ref.firstChildElement("rrs"); // Sphere
            QDomElement rrc = ref.firstChildElement("rrc"); // Cylindre
            QDomElement rra = ref.firstChildElement("rra"); // Axe
            QDomElement rrp = ref.firstChildElement("rrp"); // Distance pupillaire (identique pour les deux yeux)
            if(!rrs.isNull())
                Datas::I()->mesureautoref->setsphereOD(Utils::roundToNearestPointTwentyFive(rrs.text().toDouble()));
            if(!rrc.isNull())
                Datas::I()->mesureautoref->setcylindreOD(Utils::roundToNearestPointTwentyFive(rrc.text().toDouble()));
            if(!rra.isNull())
                Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(rra.text().toInt()));
            if(!rrp.isNull())
                Datas::I()->mesureautoref->setecartIP(rrp.text().toDouble());

            // Ref gauche
            QDomElement rls = ref.firstChildElement("rls");
            QDomElement rlc = ref.firstChildElement("rlc");
            QDomElement rla = ref.firstChildElement("rla");
            QDomElement rlp = ref.firstChildElement("rlp");
            if(!rls.isNull())
                Datas::I()->mesureautoref->setsphereOG(Utils::roundToNearestPointTwentyFive(rls.text().toDouble()));
            if(!rlc.isNull())
                Datas::I()->mesureautoref->setcylindreOG(Utils::roundToNearestPointTwentyFive(rlc.text().toDouble()));
            if(!rla.isNull())
                Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(rla.text().toInt()));
            if (!rlp.isNull() && Datas::I()->mesureautoref->ecartIP() == 0)
                Datas::I()->mesureautoref->setecartIP(rlp.text().toDouble());

        }
        QDomElement ker = datalab.firstChildElement("ker");
        if(!ker.isNull())
        {
            // Keratometrie droit
            QDomElement krr1 = ker.firstChildElement("krr1");
            QDomElement krr2 = ker.firstChildElement("krr2");
            QDomElement axeKD = ker.firstChildElement("krax");
            if(!krr1.isNull())
                Datas::I()->mesurekerato->setK1OD(krr1.text().toDouble());
            if(!krr2.isNull())
                Datas::I()->mesurekerato->setK2OD(krr2.text().toDouble());
            if(!axeKD.isNull())
                Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(axeKD.text().toInt()));

            // Keratometrie gauche
            QDomElement klr1 = ker.firstChildElement("klr1");
            QDomElement klr2 = ker.firstChildElement("klr2");
            QDomElement axeKG = ker.firstChildElement("klax");
            if(!klr1.isNull())
                Datas::I()->mesurekerato->setK1OG(klr1.text().toDouble());
            if(!klr2.isNull())
                Datas::I()->mesurekerato->setK2OG(klr2.text().toDouble());
            if(!axeKG.isNull())
                Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(axeKG.text().toInt()));
        }
        QDomElement tono = datalab.firstChildElement("tono");
        if (!tono.isNull())
        {
            // Tono droit
            QDomElement tr = tono.firstChildElement("tr");
            if(!tr.isNull())
            {
                Datas::I()->mesuretono->setTOD(tr.text().toInt());
                Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
            }

            // Tono gauche
            QDomElement tl = tono.firstChildElement("tl");
            if(!tl.isNull())
            {
                Datas::I()->mesuretono->setTOG(tl.text().toInt());
                Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
            }
        }
        QDomElement correctedtono = datalab.firstChildElement("misc");
        if (!correctedtono.isNull())
        {
            // Tono droit
            QDomElement tr = correctedtono.firstChildElement("C-R-A");
            if(!tr.isNull())
                Datas::I()->mesuretono->setTODcorrigee(tr.text().toInt());
            // Tono gauche
            QDomElement tl = correctedtono.firstChildElement("C-L-A");
            if(!tl.isNull())
                Datas::I()->mesuretono->setTOGcorrigee(tl.text().toInt());
        }
        QDomElement pachy = datalab.firstChildElement("pachy");
        if(!pachy.isNull())
        {
            // Pachy droit
            QDomElement pr = pachy.firstChildElement("pr");
            if(!pr.isNull())
            {
                Datas::I()->mesurepachy->setpachyOD(pr.text().toDouble());
                Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
            }
            // Pachy gauche
            QDomElement pl = pachy.firstChildElement("pl");
            if(!pl.isNull())
            {
                Datas::I()->mesurepachy->setpachyOG(pl.text().toDouble());
                Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
            }
        }
    }
    //else qDebug() << "Erreur de lecture du fichier XML, Autoref inconnu : " << nameARK;

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


void Procedures::LectureDonneesCSVFronto(QString filecontents)
{
    QString name =m_settings->value(Param_Poste_Fronto).toString();
    if (name=="TOMEY TL-6100" || name == "RODENSTOCK AL 6600" )
    {
        Tomey::I()->LectureDonneesAL6400( filecontents);
    }
}

void Procedures::LectureDonneesXMLFronto(QDomDocument docxml)
{
    Logs::LogToFile("MesuresFronto.txt", docxml.toByteArray());
    QString nameLM =m_settings->value(Param_Poste_Fronto).toString();
    if (nameLM=="NIDEK LM-1800P" || nameLM=="NIDEK LM-500" || nameLM=="NIDEK LM-7" || nameLM=="TOMEY TL-6000" || nameLM=="TOMEY TL-7000" || nameLM=="RODENSTOCK AL 6600")
    {
        Nidek::I()->LectureDonneesXMLFronto(docxml);
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

GenericProtocol::TypeMesure Procedures::ConvertMesure(QString Mesure)
{
    if (Mesure == "P") return GenericProtocol::MesureFronto;
    if (Mesure == "A") return GenericProtocol::MesureAutoref;
    if (Mesure == "R") return GenericProtocol::MesureRefracteur;
    return  GenericProtocol::MesureNone;
}

QString Procedures::ConvertMesure(GenericProtocol::TypeMesure Mesure)
{
    switch (Mesure) {
    case GenericProtocol::MesureFronto:        return "P";
    case GenericProtocol::MesureAutoref:       return "A";
    case GenericProtocol::MesureRefracteur:    return "R";
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
void Procedures::InsertMesure(GenericProtocol::TypeMesure typemesure)
{
    if (Datas::I()->patients->currentpatient() == Q_NULLPTR)
        return;
    if (Datas::I()->Datas::I()->actes->currentacte() == Q_NULLPTR)
        return;
    int idPatient   = Datas::I()->patients->currentpatient()->id();
    int idActe      = Datas::I()->actes->currentacte()->id();
    if (typemesure == GenericProtocol::MesureFronto)
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
        for (auto it = Datas::I()->refractions->refractions()->cbegin(); it != Datas::I()->refractions->refractions()->cend();)
        {
            Refraction *ref = const_cast<Refraction*>(it.value());
            if (ref)
            {
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
                    ++ it;
            }
            else
                it = Datas::I()->refractions->refractions()->erase(it);
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
    else if (typemesure == GenericProtocol::MesureAutoref)
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
        for (auto it = Datas::I()->refractions->refractions()->cbegin(); it != Datas::I()->refractions->refractions()->cend();)
        {
            Refraction *ref = const_cast<Refraction*>(it.value());
            if (ref)
            {
                if (ref->idacte() == idActe && ref->typemesure() == Refraction::Autoref)
                {
                    DataBase::I()->SupprRecordFromTable(ref->id(), CP_ID_REFRACTIONS, TBL_REFRACTIONS);
                    delete ref;
                    it = Datas::I()->refractions->refractions()->erase(it);
                }
                else
                    ++ it;
            }
            else
                it = Datas::I()->refractions->refractions()->erase(it);
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
    else if (typemesure == GenericProtocol::MesureKerato)
    {
        QString req = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(GenericProtocol::MesureAutoref) + "'";
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
                    ConvertMesure(GenericProtocol::MesureAutoref) + "', "
                    "CURDATE(), " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->K1OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->K2OD(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOD()? "null" : QString::number(Datas::I()->mesurekerato->axeKOD()))       + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->K1OG(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->K2OG(), 'f', 2)) + ", " +
                    (Datas::I()->mesurekerato->isnullLOG()? "null" : QString::number(Datas::I()->mesurekerato->axeKOG()))       + ", " +
                    "'" + ConvertMesure(GenericProtocol::MesureAutoref) + "', " +
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
                    CP_MODEMESUREKERATO_DATAOPHTA " = '" + ConvertMesure(GenericProtocol::MesureAutoref) + "'";
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
            req += " where " CP_IDPATIENT_DATAOPHTA " = "+ QString::number(idPatient) + " and QuelleMesure = '" + ConvertMesure(GenericProtocol::MesureAutoref) + "'";
            db->StandardSQL (req, tr("Erreur de modification de données de kératométrie dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == GenericProtocol::MesureRefracteur)
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
        for (auto it = Datas::I()->refractions->refractions()->cbegin(); it != Datas::I()->refractions->refractions()->cend();)
        {
            Refraction *ref = const_cast<Refraction*>(it.value());
            if (ref)
            {
                if (ref->idacte() == idActe && ref->typemesure() == Refraction::Acuite)
                {
                    DataBase::I()->SupprRecordFromTable(ref->id(), CP_ID_REFRACTIONS, TBL_REFRACTIONS);
                    delete ref;
                    it = Datas::I()->refractions->refractions()->erase(it);
                }
                else
                    ++ it;
            }
            else
                it = Datas::I()->refractions->refractions()->erase(it);
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

        QString requete = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(GenericProtocol::MesureRefracteur) + "'";
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
                    " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(GenericProtocol::MesureRefracteur) + "'";

            db->StandardSQL (requete, tr("Erreur de mise à jour de données de refraction dans ") + TBL_DONNEES_OPHTA_PATIENTS);
        }
    }
    else if (typemesure == GenericProtocol::MesureTono)
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
    else if (typemesure == GenericProtocol::MesurePachy)
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
    if (typemesure != GenericProtocol::MesureFronto && typemesure != GenericProtocol::MesureTono && typemesure != GenericProtocol::MesurePachy)
        Datas::I()->patients->actualiseDonneesOphtaCurrentPatient();
    //! emit NouvMesure() sert à afficher, dans la fiche active (rufus.cpp ou dlg_refraction.cpp), la mesure qui vient d'être effectuées
    emit NouvMesure(typemesure);
}
