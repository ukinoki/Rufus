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
#include "mysqlinstaller.h"
#include <QBuffer>
#include <QPdfWriter>
#include <QElapsedTimer>
#include <QEventLoop>

Procedures* Procedures::instance =  Q_NULLPTR;
Procedures* Procedures::I()
{
    if( !instance )
        instance = new Procedures();
    return instance;
}

//! Un rufus.ini est-il VALIDE = contient-il au moins un mode de connexion paramétré (Poste,
//! Réseau local ou Distant) avec un port 3306/3307 (et un serveur renseigné pour les modes réseau) ?
static bool iniContientModeValide(QSettings& s)
{
    static const QSet<int> ports = { 3306, 3307 };
    return (s.value(Utils::getBaseFromMode(Utils::Poste) + Param_Active).toString() == "YES"
            && ports.contains(s.value(Utils::getBaseFromMode(Utils::Poste) + Param_Port).toInt()))
        || (s.value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Active).toString() == "YES"
            && s.value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Serveur).toString() != ""
            && ports.contains(s.value(Utils::getBaseFromMode(Utils::ReseauLocal) + Param_Port).toInt()))
        || (s.value(Utils::getBaseFromMode(Utils::Distant) + Param_Active).toString() == "YES"
            && s.value(Utils::getBaseFromMode(Utils::Distant) + Param_Serveur).toString() != ""
            && ports.contains(s.value(Utils::getBaseFromMode(Utils::Distant) + Param_Port).toInt()));
}

//! Une sauvegarde de Rufus.ini (~/.rufus/.rufus.ini) existe-t-elle ET est-elle elle-même exploitable ?
//! Inutile de proposer de restaurer un fichier cassé.
static bool sauvegardeIniValide()
{
    if (!QFile::exists(PATH_FILE_INI_BACKUP))
        return false;
    QSettings sauvegarde(PATH_FILE_INI_BACKUP, QSettings::IniFormat);
    return iniContientModeValide(sauvegarde);
}

Procedures::Procedures(QObject *parent) :
    QObject(parent)
{
     /*!
    QStringList driverslist = QSqlDatabase::drivers();
    for (int i = 0; i<driverslist.size(); ++i)
        qDebug() << driverslist.at(i);
    //*/
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true); //! to display debug messages in console
    m_CPpardefaut    = "";
    m_Villepardefaut = "";
    db               = DataBase::I();

    QFile FichierIni(PATH_FILE_INI);
    m_applicationfont = QFont(POLICEPARDEFAUT);
    Utils::CalcFontSize(m_applicationfont);
    qApp->setFont(m_applicationfont);
    //qDebug() << "qApp->font().pointSize()" << qApp->font().pointSize();

    m_connexionbaseOK           = false;

    // ── Langue de l'interface ───────────────────────────────────────────────────
    // PRIORITÉ ABSOLUE : rufus.ini fait foi. La détection automatique (QLocale) ne sert
    // QUE s'il n'y a PAS de rufus.ini, OU s'il en existe un mais SANS localisation.
    //   français → FR ; anglais (UK, USA…) → EN ; espagnol (Espagne + toute l'Amérique
    //   latine) → ES ; portugais : Brésil → BR, sinon (Portugal…) → PT.
    // Hors de ces langues → sélecteur, AVEC le français par défaut.
    QString iniVersion;
    if (FichierIni.exists())
        iniVersion = QSettings(PATH_FILE_INI, QSettings::IniFormat).value(Param_Poste_Version).toString();
    if (!iniVersion.isEmpty())
    {
        m_version = iniVersion;                       //! rufus.ini fait foi : aucune détection
    }
    else
    {
        QString detected;
        switch (QLocale::system().language()) {
        case QLocale::French:     detected = "FR"; break;
        case QLocale::English:    detected = "EN"; break;
        case QLocale::Spanish:    detected = "ES"; break;
        case QLocale::Portuguese: detected = (QLocale::system().territory() == QLocale::Brazil) ? "BR" : "PT"; break;
        case QLocale::Italian:    detected = "IT"; break;
        default:                  break;
        }
        if (!detected.isEmpty())
            m_version = detected;
        else
        {
            UpDialog *versiondlg        = new UpDialog;
            QGroupBox *gbox             = new QGroupBox;
            QRadioButton *frbutt        = new QRadioButton("Version française");
            QRadioButton *enbutt        = new QRadioButton("English version");
            QRadioButton *esbutt        = new QRadioButton("Versión española");
            QRadioButton *brbutt        = new QRadioButton("Versão brasileira");
            QRadioButton *ptbutt        = new QRadioButton("Versão portuguesa");
            QRadioButton *itbutt        = new QRadioButton("Versione italiana");
            QVBoxLayout *version_Lay    = new QVBoxLayout();
            version_Lay                 ->addWidget(frbutt);
            version_Lay                 ->addWidget(enbutt);
            version_Lay                 ->addWidget(esbutt);
            version_Lay                 ->addWidget(brbutt);
            version_Lay                 ->addWidget(ptbutt);
            version_Lay                 ->addWidget(itbutt);
            gbox                        ->setLayout(version_Lay);
            versiondlg                  ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
            versiondlg                  ->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
            versiondlg->dlglayout()     ->insertWidget(0,gbox);
            versiondlg->dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
            frbutt                      ->setChecked(true);   //! français par défaut (OK actif d'emblée)
            connect(frbutt, &QRadioButton::clicked, versiondlg, [=] {versiondlg->OKButton->setEnabled(true);});
            connect(enbutt, &QRadioButton::clicked, versiondlg, [=] {versiondlg->OKButton->setEnabled(true);});
            connect(esbutt, &QRadioButton::clicked, versiondlg, [=] {versiondlg->OKButton->setEnabled(true);});
            connect(brbutt, &QRadioButton::clicked, versiondlg, [=] {versiondlg->OKButton->setEnabled(true);});
            connect(ptbutt, &QRadioButton::clicked, versiondlg, [=] {versiondlg->OKButton->setEnabled(true);});
            connect(itbutt, &QRadioButton::clicked, versiondlg, [=] {versiondlg->OKButton->setEnabled(true);});
            connect (versiondlg->CancelButton,   &QPushButton::clicked,   versiondlg, [=] {exit(0);});
            connect (versiondlg->OKButton,   &QPushButton::clicked,   versiondlg, [=, this] {
                if (frbutt->isChecked())
                        m_version = "FR";
                else if (enbutt->isChecked())
                        m_version = "EN";
                else if (esbutt->isChecked())
                        m_version = "ES";
                else if (brbutt->isChecked())
                        m_version = "BR";
                else if (ptbutt->isChecked())
                        m_version = "PT";
                else if (itbutt->isChecked())
                        m_version = "IT";
                else
                    return;
                versiondlg->accept();
            });
            if (versiondlg->exec() != QDialog::Accepted)
                exit(0);
        }
    }
    // Traducteur des boîtes de démarrage (portée constructeur → couvre toutes les boîtes ci-dessous).
    QDir dirlocLang = QDir(QCoreApplication::applicationDirPath());
    dirlocLang.cdUp();
    QTranslator startupTranslator;
    if (startupTranslator.load(dirlocLang.absolutePath() + "/Locale/rufus_" + m_version.toLower() + ".qm"))
        QCoreApplication::installTranslator(&startupTranslator);

    //! Ne rend la main qu'avec un Rufus.ini exploitable et m_settings prêt à l'emploi (§ II.1).
    ReparerIni();

    m_nomImprimante  = "";

    Ouverture_Appareils_Refraction();
    ReconstruitListeModesAcces();
    m_typemesureRefraction               = GenericProtocol::MesureNone;
    m_dlgrefractionouverte    = false;
    /*! on ne crée AUCUN QPrinter ici : sous Windows, construire un QPrinter résout l'imprimante par
        défaut et, si c'est une imprimante réseau éteinte, gèle le démarrage plusieurs dizaines de
        secondes (« tentative de connexion à l'imprimante »). Le QPrinter n'est utile qu'à
        l'impression d'images (Print()) : on l'y crée à la demande. */
    m_printer             = Q_NULLPTR;
}

void Procedures::CleanIniFile()
{
    settings()->remove(Param_Poste "/VersionRessources");
    QStringList ListDevices = Datas::I()->refractiondevices->listRefractionDevices();
    QString device = settings()->value(Param_Poste_Fronto).toString();
    if (!ListDevices.contains(device))
    {
        settings()->remove(Param_Poste_Fronto);
        settings()->remove(Param_Poste_PortFronto_DossierEchange);
        settings()->remove(Param_Poste_PortFronto_COM_baudrate);
        settings()->remove(Param_Poste_PortFronto_COM_databits);
        settings()->remove(Param_Poste_PortFronto_COM_parity);
        settings()->remove(Param_Poste_PortFronto_COM_stopBits);
        settings()->remove(Param_Poste_PortFronto_COM_flowControl);
    }
    device = settings()->value(Param_Poste_Autoref).toString();
    if (!ListDevices.contains(device))
    {
        settings()->remove(Param_Poste_Autoref);
        settings()->remove(Param_Poste_PortAutoref_DossierEchange);
        settings()->remove(Param_Poste_PortAutoref_COM_baudrate);
        settings()->remove(Param_Poste_PortAutoref_COM_databits);
        settings()->remove(Param_Poste_PortAutoref_COM_parity);
        settings()->remove(Param_Poste_PortAutoref_COM_stopBits);
        settings()->remove(Param_Poste_PortAutoref_COM_flowControl);
    }
    device = settings()->value(Param_Poste_Refracteur).toString();
    if (!ListDevices.contains(device))
    {
        settings()->remove(Param_Poste_Refracteur);
        settings()->remove(Param_Poste_PortRefracteur_DossierEchange);
        settings()->remove(Param_Poste_PortRefracteur_COM_baudrate);
        settings()->remove(Param_Poste_PortRefracteur_COM_databits);
        settings()->remove(Param_Poste_PortRefracteur_COM_parity);
        settings()->remove(Param_Poste_PortRefracteur_COM_stopBits);
        settings()->remove(Param_Poste_PortRefracteur_COM_flowControl);
    }
    device = settings()->value(Param_Poste_Tono).toString();
    if (!ListDevices.contains(device))
    {
        settings()->remove(Param_Poste_Tono);
        settings()->remove(Param_Poste_PortTono_DossierEchange);
        settings()->remove(Param_Poste_PortTono_COM_baudrate);
        settings()->remove(Param_Poste_PortTono_COM_databits);
        settings()->remove(Param_Poste_PortTono_COM_parity);
        settings()->remove(Param_Poste_PortTono_COM_stopBits);
        settings()->remove(Param_Poste_PortTono_COM_flowControl);
    }
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
    case Utils::Distant:
        cle = Utils::getBaseFromMode( mode );
        break;
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
                ShowMessage::I()->SplashMessage("<font color=\"red\"><b>"+ tr("Autres postes connectés!") + "</b></font><br/>" +
                                         tr("Vous ne pouvez pas effectuer d'opération de sauvegarde/restauration sur la base de données"
                                         " si vous n'êtes pas le seul poste connecté.") + "<br/>" +
                                         tr("Le poste ") + post->nomposte() + tr(" est aussi connecté"), 6000);
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
            m_basesize += QFile(listnomsfilestorestore.at(i)).size();
    }
    else
        m_basesize = db->DatabaseSize();
    m_imagessize = 0;
    m_videossize = 0;
    // espace libre sur le disque ------------------------------------------------------------------------------------------------------------------------------------------------

    m_freespace = QStorageInfo(pathdestination).bytesAvailable();

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
        Inichk->setText(tr("Fichier de paramètrage Rufus.ini"));
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
        // En RESTAURATION, mesurer depuis le BACKUP (pathorigin + /Videos), comme Images et
        // Factures — et NON depuis le dossier vidéo LIVE (souvent vide après une réinstall), sinon
        // la case « Videos » n'apparaît pas. En SAUVEGARDE, la source reste le dossier vidéo LIVE.
        DataDir = Utils::dir_size(op == RestoreOp
            ? pathorigin + NOM_DIR_VIDEOS
            : m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Videos).toString());
        m_videossize = DataDir["Size"];
        if (m_videossize> 0)
        {
            QHBoxLayout *layVideos = new QHBoxLayout;
            UpLabel *labeVideos = new UpLabel();
            labeVideos->setVisible(false);
            labeVideos->setFixedSize(labelsize, labelsize);
            layVideos->addWidget(labeVideos);
            UpCheckBox *Videoschk  = new UpCheckBox();
            Videoschk->setText(tr("Videos"));
            Videoschk->setEnabled(OKvideos || op == BackupOp);
            Videoschk->setChecked(OKvideos || op == BackupOp);
            Videoschk->setObjectName("videos");
            layVideos->addWidget(Videoschk);
            layVideos->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            UpLabel *lblvolvid = new UpLabel();
            lblvolvid->setText(Utils::getExpressionSize(m_videossize));
            layVideos->addWidget(lblvolvid);
            dlg_buprestore->dlglayout()->insertLayout(0, layVideos);
            connect(Videoschk, &UpCheckBox::clicked, this, [=, this] {CalcTimeBupRestore();});
        }
    }
    if (OKimages)
    {
        // taille du dossier Images ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + NOM_DIR_IMAGES);
        m_imagessize = DataDir["Size"];
        if (m_imagessize > 0)
        {
            QHBoxLayout *layImges = new QHBoxLayout;
            UpLabel *labelmges = new UpLabel();
            labelmges->setVisible(false);
            labelmges->setFixedSize(labelsize, labelsize);
            layImges->addWidget(labelmges);
            UpCheckBox *Imgeschk  = new UpCheckBox();
            Imgeschk->setText(tr("Images"));
            Imgeschk->setEnabled(OKimages || op == BackupOp);
            Imgeschk->setChecked(OKimages || op == BackupOp);
            Imgeschk->setObjectName("images");
            layImges->addWidget(Imgeschk);
            layImges->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            UpLabel *lblvolimg = new UpLabel();
            lblvolimg->setText(Utils::getExpressionSize(m_imagessize));
            layImges->addWidget(lblvolimg);
            dlg_buprestore->dlglayout()->insertLayout(0, layImges);
            connect(Imgeschk, &UpCheckBox::clicked, this, [=, this] {CalcTimeBupRestore();});
        }
    }
    if (OKfactures)
    {
        // taille du dossier Factures ---------------------------------------------------------------------------------------------------------------------------------------
        DataDir = Utils::dir_size(pathorigin + NOM_DIR_FACTURES);
        m_facturessize = DataDir["Size"];
        if (m_facturessize > 0)
        {
            QHBoxLayout *layFctures = new QHBoxLayout;
            UpLabel *labelmges = new UpLabel();
            labelmges->setVisible(false);
            labelmges->setFixedSize(labelsize, labelsize);
            layFctures->addWidget(labelmges);
            UpCheckBox *Fctureschk  = new UpCheckBox();
            Fctureschk->setText(tr("Factures"));
            Fctureschk->setEnabled(OKfactures || op == BackupOp);
            Fctureschk->setChecked(OKfactures || op == BackupOp);
            Fctureschk->setObjectName("factures");
            layFctures->addWidget(Fctureschk);
            layFctures->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
            UpLabel *lblvolfct = new UpLabel();
            lblvolfct->setText(Utils::getExpressionSize(m_facturessize));
            layFctures->addWidget(lblvolfct);
            dlg_buprestore->dlglayout()->insertLayout(0, layFctures);
            connect(Fctureschk, &UpCheckBox::clicked, this, [=, this] {CalcTimeBupRestore();});
        }
    }

    QHBoxLayout *layBDD = new QHBoxLayout;
    UpLabel *labelBDD = new UpLabel();
    labelBDD->setVisible(false);
    labelBDD->setFixedSize(labelsize, labelsize);
    layBDD->addWidget(labelBDD);
    UpCheckBox *BDDchk  = new UpCheckBox();
    BDDchk->setText(tr("Base de données"));
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

    connect(BDDchk, &UpCheckBox::clicked, this, [=, this] {CalcTimeBupRestore();});

    dlg_buprestore->setFixedWidth(400);
    dlg_buprestore->AjouteLayButtons(UpDialog::ButtonOK);
    connect(dlg_buprestore->OKButton,  &UpCheckBox::clicked, dlg_buprestore, &UpDialog::accept);
    CalcTimeBupRestore();
}

//! État partagé du suivi « par table » de la sauvegarde (lu/écrit par le timer de progression) :
//! offset déjà lu de chaque .sql, nombre de tables déjà écrites, dernière table vue.
namespace { struct EtatBkp { QMap<QString, qint64> offsets; int done = 0; QString table; }; }

bool Procedures::Backup(QString pathdirdestination, bool OKBase, bool OKImages, bool OKVideos, bool OKFactures, bool verifmdp, QWidget *parent,
                        QString loginSQL, QString mdpSQL)
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
    emit ConnectTimers(false);

    //On vide les champs blob de la table factures et la table EchangeImages
    db->StandardSQL("UPDATE " TBL_FACTURES " SET " CP_JPG_FACTURES " = null, " CP_PDF_FACTURES " = null");
    db->StandardSQL("DELETE FROM " TBL_ECHANGEIMAGES);

    //! Si le dossier de sauvegarde ne peut pas être créé (droits, support absent, chemin
    //! invalide…), on demande à l'utilisateur d'en choisir un autre plutôt que d'échouer
    //! plus tard, en plein milieu d'une migration de base.
    if (!Utils::mkpath(pathdirdestination))
    {
        QUrl url = Utils::getExistingDirectoryUrl(parent, tr("Choisissez un dossier de sauvegarde"),
                                                  QUrl::fromLocalFile(PATH_DIR_RUFUS));
        if (url == QUrl())
            return false;                                   // annulé par l'utilisateur
        pathdirdestination = url.path();
        if (!Utils::mkpath(pathdirdestination))
        {
            UpMessageBox::Watch(parent, tr("Sauvegarde impossible"),
                                tr("Impossible de créer le dossier de sauvegarde."));
            return false;
        }
    }

    if (OKBase)
    {
        QString pathbackupbase = pathdirdestination + "/" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmm");
        Utils::mkpath(pathbackupbase);

        /*! sauvegarde de la base */
        QFile::remove(PATH_FILE_SCRIPTBACKUP);
        DefinitScriptBackup(pathbackupbase, loginSQL, mdpSQL);
#ifdef Q_OS_WIN
        const QString task = "\"" + QDir::toNativeSeparators(PATH_FILE_SCRIPTBACKUP) + "\"";
#else
        const QString task = "sh \"" + PATH_FILE_SCRIPTBACKUP + "\"";
#endif
        msg += tr("Base de données sauvegardée!\n");

        //! Total de tables à sauvegarder — RECALCULÉ à chaque fois (la base peut avoir changé) : sert
        //! à chiffrer l'avancement « xxx/N ». +1 pour la table `user` (fichier user.sql).
        bool okc = false;
        QVariantList rc = DataBase::I()->getFirstRecordFromStandardSelectSQL(
            "SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_TYPE='BASE TABLE' AND TABLE_SCHEMA IN "
            "('" DB_RUFUS "','" DB_COMPTA "','" DB_IMAGES "','" DB_OPHTA "')", okc);
        const int totalTables = qMax(1, (okc && !rc.isEmpty() ? rc.at(0).toInt() : 0) + 1);

        //! Boîte de progression PAR TABLE. mysqldump écrit, avant chaque table, le commentaire
        //! « Table structure for table `X` » dans le .sql : on relit régulièrement les NOUVEAUX
        //! octets de chaque fichier (pas tout le fichier → léger même sur gros dump) pour compter
        //! les tables déjà écrites et retenir la dernière → « xxx/N — table X ». Sans changer le script
        //! de dump. Les petites tables défilent trop vite pour être lues : peu importe, le
        //! but est de montrer que la machine avance. `etat` = état du comptage (offsets déjà lus par
        //! fichier, nombre de tables écrites, dernière table vue).
        EtatBkp *etat = new EtatBkp();       // sur le TAS : partagé par le timer ET le slot de fin (asynchrones)
        UpProgressDialog *bkpdial = new UpProgressDialog(0, totalTables, parent);
        bkpdial->setLabelText(tr("Sauvegarde de la base de données en cours…"));
        bkpdial->setValue(0);
        bkpdial->show();

        //! Rafraîchissement PAR TABLE : un timer relit les NOUVEAUX octets de chaque .sql (léger même
        //! sur gros dump), compte les marqueurs mysqldump « Table structure for table `X` » et affiche
        //! « xxx/N — table X ».
        QTimer *pollbkp = new QTimer(this);
        connect(pollbkp, &QTimer::timeout, this, [=]() {
            const QByteArray marqueur = "Table structure for table `";
            for (const QString& f : QDir(pathbackupbase).entryList(QStringList() << "*.sql", QDir::Files))
            {
                QFile file(pathbackupbase + "/" + f);
                if (!file.open(QIODevice::ReadOnly))
                    continue;
                const qint64 from = etat->offsets.value(f, 0);
                if (file.size() > from)
                {
                    file.seek(from);
                    const QByteArray chunk = file.readAll();
                    etat->offsets.insert(f, from + chunk.size());
                    int idx = 0;
                    while ((idx = chunk.indexOf(marqueur, idx)) != -1)
                    {
                        idx += marqueur.size();
                        const int fin = chunk.indexOf('`', idx);
                        if (fin != -1) etat->table = QString::fromUtf8(chunk.mid(idx, fin - idx));
                        etat->done++;
                        idx = (fin != -1 ? fin : idx + 1);
                    }
                }
                file.close();
            }
            bkpdial->setValue(etat->done);
            bkpdial->setLabelText(tr("Sauvegarde de la base de données en cours…") + "\n\n"
                + QString("%1/%2   —   ").arg(etat->done).arg(totalTables) + tr("table ") + etat->table);
        });
        pollbkp->start(300);

        //! Dump ASYNCHRONE (m_ostask) : Backup rend la main → c'est la BOUCLE PRINCIPALE de Qt qui
        //! affiche et anime la fiche popup (elle ne s'affiche PAS de façon fiable dans un processEvents
        //! manuel — d'où l'échec des versions synchrones). Le slot `result`, à la fin du dump, ferme la
        //! fiche, notifie, et libère timer/fiche/état.
        m_ostask.disconnect(SIGNAL(result(const int &)));
        connect(&m_ostask, &OsTask::result, this, [=, this](int a) {
            pollbkp->stop();
            pollbkp->deleteLater();
            bkpdial->setValue(totalTables);
            bkpdial->close();
            delete bkpdial;
            delete etat;
            UpSystemTrayIcon::I()->showMessage(tr("Messages"), (a == 0 ? msg : msgEchec), Icons::icSunglasses(), 3000);
            result(handledlg, this);
            QFile::remove(PATH_FILE_SCRIPTBACKUP);
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
                        /*! l'effacement d'une vieille sauvegarde est long : on garde la fiche vivante */
                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);
                    }
            }
        }

        /*! sauvegarde de Rufus.ini et des fichiers ressources */
        QFile file = QFile(PATH_FILE_INI);
        Utils::copyWithPermissions(file, pathbackupbase + "/" NOM_FILE_INI);
        msg += tr("Fichier de paramétrage Rufus.ini sauvegardé\n");
    }
    //! Le dump de la base tourne en tâche de fond (m_ostask, asynchrone). On ATTEND sa fin AVANT
    //! d'enchaîner sur les fichiers — en laissant vivre la fiche des tables (le timer l'anime, le slot
    //! `result` la referme quand le dump se termine). Sans cette attente, la 1re fiche de fichiers
    //! s'afficherait PAR-DESSUS celle des tables encore ouverte (le défaut constaté). On retrouve ainsi
    //! l'enchaînement propre de la restauration : une fiche à la fois.
    if (OKBase)
        while (m_ostask.state() != QProcess::NotRunning)
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 50);

    if (OKImages || OKVideos || OKFactures)
    {
        QString dirNomSource;
        QString dirNomDest;
        QString DirImagery = db->dirimagerie();
        if (!QDir(DirImagery).exists())
            return false;

        if (OKFactures) {
            dirNomSource = DirImagery + NOM_DIR_FACTURES;
            dirNomDest = pathdirdestination + NOM_DIR_FACTURES;
            /*! fiche ouverte AVANT le comptage, en barre animée (0,0) : il dure et laissait l'écran figé */
            UpProgressDialog *progdial = new UpProgressDialog(0,0, parent);
            progdial->setLabelText(tr("Sauvegarde des factures"));
            progdial->show();
            int t = 0;
            Utils::countFilesInDirRecursively(dirNomSource, t);
            progdial->setRange(0,t);
            int n = 0;
            Utils::copyfolderrecursively(dirNomSource, dirNomDest, n, tr("Sauvegarde des factures"), progdial);
            delete progdial;
            msg += tr("Factures sauvegardées\n");
        }
        if (OKImages) {
            dirNomSource = DirImagery + NOM_DIR_IMAGES;
            dirNomDest = pathdirdestination + NOM_DIR_IMAGES;
            UpProgressDialog *progdial = new UpProgressDialog(0,0, parent);
            progdial->setLabelText(tr("Sauvegarde des fichiers d'imagerie"));
            progdial->show();
            int t = 0;
            Utils::countFilesInDirRecursively(dirNomSource, t);
            progdial->setRange(0,t);
            int n = 0;
            Utils::copyfolderrecursively(dirNomSource, dirNomDest, n, tr("Sauvegarde des fichiers d'imagerie"), progdial);
            delete progdial;
            msg += tr("Fichiers imagerie sauvegardés\n");
        }
        if (OKVideos) {
            dirNomSource = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Videos).toString();
            dirNomDest = pathdirdestination + NOM_DIR_VIDEOS;
            UpProgressDialog *progdial = new UpProgressDialog(0,0, parent);
            progdial->setLabelText(tr("Sauvegarde des videos"));
            progdial->show();
            int t = 0;
            Utils::countFilesInDirRecursively(dirNomSource, t);
            progdial->setRange(0,t);
            int n = 0;
            Utils::copyfolderrecursively(dirNomSource, dirNomDest, n, tr("Sauvegarde des videos"), progdial);
            delete progdial;
            msg += tr("Fichiers video sauvegardés");
        }
    }
    else if (!OKBase)            // ni base ni fichiers demandés → rien à sauvegarder
    {
        result(handledlg, this);
        return false;
    }
    //! Récap final SEULEMENT hors sauvegarde de base : quand OKBase, le dump tourne encore en tâche de
    //! fond et ce récap s'afficherait PAR-DESSUS la fiche de progression (superposition). Dans ce cas,
    //! c'est le slot `result` (fin du dump) qui notifie, via l'icône de la barre des tâches.
    if (!OKBase)
    {
        qintptr z = 0;
        ShowMessage::I()->PriorityMessage(msg,z, 10000);
    }
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

void Procedures::DefinitScriptBackup(QString pathbackupbase, QString loginSQL, QString mdpSQL)
{
    if (loginSQL.isEmpty())
    {
        loginSQL = LOGIN_SQL;
        mdpSQL   = MySQLInstaller::motDePasseSQL();
    }
    if (!QDir(pathbackupbase).exists())
        if (!Utils::mkpath(pathbackupbase))
            return;
    if (!QDir(pathbackupbase).exists())
        return;
#ifdef Q_OS_WIN
    QString CRLF="\r\n";
    QString executabledump = "\"" + QDir::toNativeSeparators(dirSQLExecutable()+ "/mysqldump.exe") + "\"";
    QString scriptbackup;
#else
    QString CRLF="\n";
    QString executabledump = "\"" + QDir::toNativeSeparators(dirSQLExecutable() + "/mysqldump") + "\"";
    QString scriptbackup= "#!/bin/bash";
    scriptbackup += CRLF;
#endif
    // élaboration du script de backup
    // Sauvegarde des 4 bases de Rufus
    QString host = "";
    if (DataBase::I()->ModeAccesDataBase() == Utils::ReseauLocal)
        host = " -h " + DataBase::I()->dbase().hostName();
    scriptbackup += executabledump + host + " --force --opt --user=\"" + loginSQL + "\" -p\"" + mdpSQL + "\" --skip-lock-tables --events --databases " DB_RUFUS " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_RUFUS ".sql") + "\"";
    scriptbackup += CRLF;
    scriptbackup += executabledump + host + " --force --opt --user=\"" + loginSQL + "\" -p\"" + mdpSQL + "\" --skip-lock-tables --events --databases " DB_COMPTA " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_COMPTA ".sql") + "\"";
    scriptbackup += CRLF;
    scriptbackup += executabledump + host + " --force --opt --user=\"" + loginSQL + "\" -p\"" + mdpSQL + "\" --skip-lock-tables --events --databases " DB_IMAGES " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_IMAGES ".sql") + "\"";
    scriptbackup += CRLF;
    scriptbackup += executabledump + host + " --force --opt --user=\"" + loginSQL + "\" -p\"" + mdpSQL + "\" --skip-lock-tables --events --databases " DB_OPHTA " > \"" + QDir::toNativeSeparators(pathbackupbase + "/" DB_OPHTA ".sql") + "\"";
    scriptbackup += CRLF;
    // Sauvegarde de la table des utilisateurs
    scriptbackup += executabledump + " --force --opt --user=\"" + loginSQL + "\" -p\"" + mdpSQL + "\" mysql user > \"" + QDir::toNativeSeparators(pathbackupbase + "/user.sql") + "\"";
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
                                      tr("le chemin par défaut")
                                      + " \"" + dirkeys + "\" " + tr("n'est pas valide"),
                                      tr("Choisissez un dossier valide dans la boîte de dialogue suivante"));
        while (!a)
        {
            urlkeys = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                       tr("Choisissez le dossier dans lequel se trouvent les clés SSL"),
                                                       (QDir::rootPath()));
            if (urlkeys == QUrl())
            {
                if (UpMessageBox::Question(Q_NULLPTR,
                                           tr("le chemin choisi") + " \"" + urlkeys.path() + "\" " + tr("n'est pas valide"),
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
int Procedures::ExecuteScriptSQL(QStringList ListScripts, std::function<void()> onProgress)
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
        << ("-p" + MySQLInstaller::motDePasseSQL())
        << "-h" << host
        << "-P" << QString::number(db->port());
#ifndef Q_OS_WIN
    sqlCommand = Utils::quoteShell(sqlCommand);
    for (int i = 0; i < args.size() ; ++i)
        sqlCommand += " " + Utils::quoteShell(args.at(i));
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
            QString command = sqlCommand + " < " + Utils::quoteShell(path);
            QString bat = "bash -c \"" + command + "\"";
            dumpProcess.startCommand(bat);
#endif
            //! Attente qui laisse VIVRE l'interface (la fenêtre de progression reste affichée et
            //! animée, pas de « ne répond pas ») : on fait tourner la boucle d'événements pendant le
            //! chargement, qui peut être long sur une machine lente.
            while (dumpProcess.state() != QProcess::NotRunning)
            {
                if (dumpProcess.waitForFinished(50))
                    break;
                if (onProgress) onProgress();   //! suivi de progression (ex. tables déjà restaurées)
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 40);
            }


            if (dumpProcess.error() == QProcess::FailedToStart)
            {
                Logs::ERROR(tr("Impossible de lancer le processus de chargement de la base de données à partir du fichier ") + QString("\"%1\"").arg(path));
                a = 99;
                break;
            }
            if (dumpProcess.exitStatus() == QProcess::NormalExit)
                a = dumpProcess.exitCode();
            else
            {
                Logs::ERROR(tr("Echec du processus de chargement de la base de données à partir du fichier ") + QString("\"%1\"").arg(path));
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
    QString DirImagery = db->dirimagerie();
    if (full)
    {
        OKbase = true;
        OKImages = QDir(DirImagery).exists();
        OKVideos = QDir(DirImagery).exists();
        OKFactures = QDir(DirImagery).exists();
    }
    else
    {
        AskBupRestore(BackupOp, DirImagery, dirdestination );
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
    connect(&t_timerbackup, &TimerController::timeout, this, [=, this] {BackupWakeUp();});

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
    textcorps.replace("{{POLICE}}", qApp->font().family());


    QRegularExpression rx;
    rx.setPattern("font-size( *: *[\\d]{1,2} *)pt");
    text.replace(rx,"font-size:9pt");

    textcorps.replace("{{TEXTE ORDO}}",text);
    textcorps.replace("{{TEXTE ORDO HORS ALD}}", "");
    return textcorps;
}

/*!
 * \brief Procedures::ResolutionRendu
 * Résolution (dpi) du QPdfWriter, seul périphérique par lequel TextPrinter rend le document
 * (print, preview et exportPdf passent tous par getPDFByteArray). C'est elle qui sert à
 * calibrer la résolution d'enregistrement de la signature.
 */
int Procedures::ResolutionRendu()
{
    QBuffer buf;
    QPdfWriter pw(&buf);
    return pw.resolution();
}

/*---------------------------------------------------------------------------------
    Retourne l'entête du document à imprimer
-----------------------------------------------------------------------------------*/
QMap<QString, QString> Procedures::CalcEnteteImpression(QDate date, User *user, bool withBarCodes)
{
    QMap<QString, QString> EnteteMap = QMap<QString, QString>();
    if (!user)
        return EnteteMap;
    EnteteMap[NORMHeader]   = "";
    EnteteMap[ALDHeader]    = "";
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
            QString reqrp = "select " CP_IDUSERPARENT_USRCONNECT " from " TBL_USERSCONNECTES
                            " where " CP_IDUSERSUPERVISEUR_USRCONNECT " = " + QString::number(user->id());
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

        /*! Pour tester à partir d'un fichier */
        /*QString nomModeleEntete = QDir::homePath() + "/test.html";
        QFile qFileEnTete(nomModeleEntete);
        if (!qFileEnTete.open( QIODevice::ReadOnly ))
            return QMap<QString, QString>();

        QByteArray  baEnTete        = qFileEnTete.readAll();
        qFileEnTete.close ();
        textentete = baEnTete;
        //*/


        textentete =  (i==1?
                        (withBarCodes? Ressources::I()->HeaderOrdoWithBarCode() :  Ressources::I()->HeaderOrdo())
                        :
                        Ressources::I()->HeaderOrdoALD()
                       );

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
        QString adresse ="";
        int nlignesadresse = 0;
        Site *sit = Datas::I()->sites->currentsite();
        if (user != currentuser())
        {
            QMap<Site*,qlonglong> mapsites = Datas::I()->sites->initListeByUser(user->id());
            if (mapsites.size()>0)
                sit = mapsites.firstKey(); //TODO ça ne va pas parce qu'on prend arbitrairement la première adresse
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

        textentete.replace("{{DATE}}", sit->ville()  + tr(", le ") + QLocale::system().toString(date,tr("d MMMM yyyy")));
        Utils::epureFontFamily(textentete);

        (i==1? EnteteMap[NORMHeader] = textentete : EnteteMap[ALDHeader] = textentete);
    }

    return EnteteMap;
}

/*---------------------------------------------------------------------------------
    Retourne le pied du document à imprimer
-----------------------------------------------------------------------------------*/
QString Procedures::CalcPiedImpression(User *user)
{
    QString textpied =  Ressources::I()->FooterOrdo();
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

/*!
 * \brief Procedures::Cree_pdffile
 * \abstract Create pdf file from document
 * \param QString textcorps
 * \param QString textentete
 * \param QString textpied
 * \param QString nomfichier = nom du fichier
 * \param QString nomdossier = dossier où est enregistré le fichier
 * \return
 */
bool Procedures::Cree_pdffile(QString textcorps, QString textentete, QString textpied, QString nomfichier, User *usr, bool ALD, QString nomdossier, QImage signature)
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
    int tailleEnTete = (ALD? TailleEnTeteALD(): TailleEnTete());
    TexteAImprimer->setHeaderSize(tailleEnTete);
    TexteAImprimer->setFooterText(textpied);
    TexteAImprimer->setTopMargin(TailleTopMarge());
    TexteAImprimer->setSignature(signature, SIGNATURE_LARGEUR_IMPRESSION_MM);
    if (usr != Q_NULLPTR && db->parametres()->cotationsfrance())    //! codes barres (n° AM, RPPS) : franco-français
        TexteAImprimer->setmapBarcodes(usr->mapBarCodes());


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

/*!
 * \brief Procedures::Cree_pdfByteArray
 * \abstract Create pdf file from document
 * \param QString textcorps
 * \param QString textentete
 * \param QString textpied
 * \return
 */
QByteArray Procedures::Cree_pdfByteArray(QString textcorps, QString textentete, QString textpied, User *usr, bool ALD, QImage signature)
{
    QTextEdit *Etat = new QTextEdit;
    Etat->setHtml(textcorps);

    TextPrinter *TexteAImprimer = new TextPrinter();
    textpied.replace("{{DUPLI}}","");

    TexteAImprimer->setFooterSize(TaillePieddePage());
    TexteAImprimer->setHeaderText(textentete);
    int tailleEnTete = (ALD? TailleEnTeteALD(): TailleEnTete());
    TexteAImprimer->setHeaderSize(tailleEnTete);
    TexteAImprimer->setFooterText(textpied);
    TexteAImprimer->setTopMargin(TailleTopMarge());
    TexteAImprimer->setSignature(signature, SIGNATURE_LARGEUR_IMPRESSION_MM);
    if (usr != Q_NULLPTR && db->parametres()->cotationsfrance())    //! codes barres (n° AM, RPPS) : franco-français
        TexteAImprimer->setmapBarcodes(usr->mapBarCodes());

    QByteArray ba = TexteAImprimer->getPDFByteArray(Etat->document());
    delete TexteAImprimer;
    delete Etat;
    return ba;
}


/*!
 * \brief Procedures::Imprime_Etat
 * \param parent
 * \param textcorps
 * \param textentete
 * \param textpied
 * \param TaillePieddePage
 * \param TailleEnTete
 * \param TailleTopMarge
 * \param m_mapbarcodes                   si un m_mapbarcodes est précisé les barcodes de cet utilisateur seront imprimés
 * \param AvecDupli
 * \param AvecNumPage
 * \param AvecChoixImprimante
 * \return
 */
bool Procedures::Imprime_Etat(QWidget *parent, QString textcorps, QString textentete, QString textpied,
                              int TaillePieddePage, int TailleEnTete, int TailleTopMarge, QMap<QString, QString> mapbarcodes,
                              bool AvecDupli, bool AvecNumPage, bool AvecChoixImprimante, QImage signature)
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
    if (db->parametres()->cotationsfrance())            //! codes barres (n° AM, RPPS) : franco-français
        TexteAImprimer->setmapBarcodes(mapbarcodes);
    TexteAImprimer->setSignature(signature, SIGNATURE_LARGEUR_IMPRESSION_MM);
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

bool Procedures::QuestionPdfOrPrint(QWidget *parent, bool &pdf)
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

    wdg_printbouton                 ->setFocus();

    bool initok = (msgbox->exec() == QDialog::Accepted && msgbox->clickedpushbutton() != wdg_annulbouton);
    if (initok)
        pdf = (msgbox->clickedpushbutton() == wdg_pdfbouton);
    delete msgbox;
    return initok;
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


bool Procedures::Imprimer_Document(QWidget *parent, Patient *pat, User * user, QString titre, QString textorigine, QDate date,
                                   bool Prescription, bool ALD, bool AvecDupli, bool pdf, bool AvecChoixImprimante, bool Administratif,
                                   QImage signature)
{
    if (pat == Q_NULLPTR || user == Q_NULLPTR)
        return false;
    QString     textcorps, textpied, textentete;
    bool        AvecNumPage = false;
    bool        aa;

    //création de l'entête
    QMap<QString,QString> EnteteMap = CalcEnteteImpression(date, user, Prescription);
    if (EnteteMap.value(NORMHeader) == "")
        return false;
    textentete                      = (ALD? EnteteMap.value(ALDHeader) : EnteteMap.value(NORMHeader));
    if (textentete == "") return false;
    textentete.replace("{{TITRE1}}"        , "");
    textentete.replace("{{TITRE}}"         , "");
    textentete.replace("{{DDN}}"           , "");

    //création du pied
    textpied = CalcPiedImpression(user);
    if (textpied == "")
        return false;

    // creation du corps
    textcorps = CalcCorpsImpression(textorigine, ALD);
    if (ALD)
    {
        textcorps.replace("{{PRENOM PATIENT}}", (Prescription? pat->prenom()        : ""));
        textcorps.replace("{{NOM PATIENT}}"   , (Prescription? pat->nom().toUpper() : ""));
        textcorps.replace("{{DATE}}", tr("le ") + QLocale::system().toString(date,tr("d MMMM yyyy")));
    }
    else
    {
        textentete.replace("{{PRENOM PATIENT}}", (Prescription? pat->prenom()        : ""));
        textentete.replace("{{NOM PATIENT}}"   , (Prescription? pat->nom().toUpper() : ""));
    }
    if (textcorps == "")
        return false;
    QTextEdit   Etat_textEdit;
    Etat_textEdit.setHtml(textcorps);
    if (Etat_textEdit.toPlainText() == "")
        return false;
    if (pdf)
    {
        QString dirname     = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0)) + "/" + m_dirnamepdf;
        QString filename    = pat->prenom() + " " + pat->nom()  + " - " + titre + ".pdf";
        QString msgOK       = tr("fichier") +" " + QDir::toNativeSeparators(filename) + "\n" +
                              tr ("sauvegardé sur le bureau dans le dossier ") + "\n" +
                              m_dirnamepdf;
        Cree_pdffile(textcorps, textentete, textpied,filename, (Prescription? user : Q_NULLPTR), ALD, dirname, signature);
        QFile file          = QFile(dirname + "/" + filename);
        aa                  = file.exists();
        UpMessageBox::Watch(parent,
                            aa? tr("Enregistrement pdf") : tr("Echec enregistrement pdf"),
                            aa? msgOK : tr ("Impossible d'enregistrer le fichier ") + QDir::toNativeSeparators(filename));
    }
    else
    {
        int tailleEnTete = TailleEnTete();
        if (ALD) tailleEnTete = TailleEnTeteALD();
        aa = Imprime_Etat(parent, textcorps, textentete, textpied,
                            TaillePieddePage(), tailleEnTete, TailleTopMarge(), (Prescription? user->mapBarCodes() : QMap<QString,QString>()),
                            AvecDupli, AvecNumPage, AvecChoixImprimante, signature);
    }

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        Utils::nettoieHTML(textcorps, 9);
        QByteArray ba = Cree_pdfByteArray(textcorps, textentete, textpied, (Prescription? user : Q_NULLPTR), ALD, signature);

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
        listbinds[CP_PDFORIGIN_DOCSEXTERNES]     = ba;
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if(doc != Q_NULLPTR)
            delete doc;
    }
    return aa;
}

bool Procedures::createPdfFromListImage(QList<QImage> listimage, QMap<QString, QString> map, QWidget *parent)
{
    auto creepdf = [=] (QPrinter &printer)
    {
        QPainter PrintingPreView(&printer);
        for (int i=0; i<listimage.size();++i)
        {
            if( i > 0 ) {
                printer.newPage();
            }
            //QPixmap pix = QPixmap::fromImage(m_imagelist.at(i)).scaledToWidth(int(m_rect.width()),Qt::SmoothTransformation);
            QPageSize pgSize = printer.pageLayout().pageSize();
            //QPageSize pgSize(QPageSize::A4);
            QImage page = listimage.at(i).scaled(pgSize.sizePixels(printer.resolution()), Qt::KeepAspectRatio);
            PrintingPreView.drawImage(QPoint(0,0),page);
        }
    };
    QPrinter printer(QPrinter::HighResolution);
    printer             .setOutputFormat(QPrinter::PdfFormat);
    QString dirname     = map.value("dir");
    QString filename    = map.value("file");
    QString msgOK       = map.value("msg");
    if (Utils::mkpath(dirname))
    {
        printer             .setOutputFileName(dirname + "/" + filename);
        creepdf(printer);
    }
    QFile file          = QFile(dirname + "/" + filename);
    bool aa             = file.exists();
    UpMessageBox::Watch(parent,
                        aa? tr("Enregistrement pdf") : tr("Echec enregistrement pdf"),
                        aa? msgOK : tr ("Impossible d'enregistrer le fichier ") + QDir::toNativeSeparators(filename));
    return true;
}

void Procedures::PdfOrPrint(QWidget *parent, QList<QImage> listimage, QMap<QString, QString> map)
{
    bool pdf = false;
    if (QuestionPdfOrPrint(parent, pdf))
    {
        if (pdf)
            createPdfFromListImage(listimage, map, parent);
        else
            Print(listimage);
    }
}

bool Procedures::Print(QList<QImage> listimage)
{
    if (m_printer == Q_NULLPTR)                      /*! création différée (voir constructeur) */
    {
        m_printer             = new QPrinter(QPrinter::HighResolution);
        m_printer             ->setFullPage(true);
    }
    auto print = [=, this]
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
    //return true;
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
        txtstatut += "\n" + tr("Responsabilité des actes :") + "\t\t";
        if (responsable)
            txtstatut += tr("toujours responsable");
        else if (responsableles2)
            txtstatut += tr("alterne responsabilité et assistant");
        else
            txtstatut += tr("assistant");
    }

    if (soigntnonassistant && currentuser()->NumPS() > 0)
        txtstatut += "\n" + tr("RPPS :") + "\t\t\t" + QString::number(currentuser()->NumPS());
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
            txtliberal +=  "\n" + tr("Honoraires encaissés sur le compte :") + "\t" + cptencaissement->nomabrege();
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
                txtstatut += "\n" + tr("Honoraires encaissés sur le compte :") + "\t";
                txtstatut += cptemployeur->nomabrege() + " ";
                txtstatut += tr("de") + " " + employeur->login();
            }
        }
    }
    else if (remplacant)
        txtstatut += "\n" + tr("Statut :") + "\t\t\t" + tr("remplaçant");
    if (soigntnonassistant && cotation)
        txtstatut += "\n" + tr("Cotation des actes :") + "\t\t" + (cotation? tr("Oui") : tr("Sans"));
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
        txtstatut += "\n" + tr("Secteur conventionnel :") + "\t\t" + secteur;
        if (!m_parametres->cotationsfrance()) txtstatut += "\n" + tr("OPTAM :") + "\t\t\t" + (currentuser()->isOPTAM() ? "Oui": "Non");
    }
    if (respliberal || soccomptable)
    {
        QString cptabledefaut ("");
        Compte *cpt = Datas::I()->comptes->getById(currentuser()->idcomptepardefaut());
        if (cpt)
        {
            User *usrcptble = Datas::I()->users->getById(cpt->idUser());
            cptabledefaut = tr("de") + " " + (usrcptble? usrcptble->login() : "null");
            txtstatut += "\n" + tr("Comptabilité enregistrée sur compte :") + "\t"
                         + cpt->nomabrege() + " "
                         + cptabledefaut;
        }
    }
    if (respliberal && db->parametres()->comptafrance())
        txtstatut += "\n" + tr("Membre d'une AGA :") + "\t\t" + (currentuser()->isAGA() ? tr("Oui") : tr("Sans"));
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

void Procedures::setPosteImportDocs(QString IPAdress)
{
    /*! Il n'y pas de variables utilisateur globale dans MySQL, on est donc obligé de passer par une procédure stockée pour en simuler une
    * pour créer une procédure avec Qt, séparer le drop du create, ne pas utiliser les délimiteurs et utiliser les retours à la ligne \n\.......
    * if (gsettingsIni->value(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs).toString() ==  "YES")

    * si a = true, on se met en poste importateur +/_ prioritaire à la fin suivant le contenu de rufus.ini
    * si a = false, on retire le poste en cours et on met NULL à la place. */

    QString req = "USE " DB_RUFUS ";";
    db->StandardSQL(req);

    req = "DROP PROCEDURE IF EXISTS " MYSQL_PROC_POSTEIMPORTDOCS ";";
    db->StandardSQL(req);

    req = "CREATE PROCEDURE " MYSQL_PROC_POSTEIMPORTDOCS "()\n\
            BEGIN\n\
            SELECT '" + IPAdress + "';\n\
            END ;";
    db->StandardSQL(req);
}

QString Procedures::PosteImportDocs()
{
    QString rep = "";
    QString req = "";
    bool isMysql8 = false;
    if (db->versionMySQL().split(".").size() > 0)
        isMysql8 = (db->versionMySQL().split(".").at(0).toInt() == 8);
    //qDebug() << "Mysql = " << db->version() << " - Mysql version = " << db->version().split(".").at(0).toInt();

    /*! Il n'y pas de variables utilisateur globale dans MySQL, on est donc obligé serge    gaxt78iy
     *  de passer par une procédure stockée pour en simuler une
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
    //! On décrit les boutons de la fenêtre de récupération par leur FONCTION, et non par leur
    //! libellé exact : cette fenêtre n'apparaît qu'au PROCHAIN démarrage (autre exécution), donc
    //! impossible de garantir ici le texte qu'ils porteront. Citer un libellé le condamnerait à
    //! devenir faux au moindre renommage (cf. l'ancien « Premier démarrage de Rufus »).
    msgbox      .setInformativeText(tr("Si vous confirmez la réinitialisation, une sauvegarde de la base patients, du fichier Rufus.ini et des fichiers ressources sera réalisée"
                                      " puis le programme sera arrêté.\n"
                                      "Au redémarrage, une fenêtre de récupération vous permettra de repartir sur une base patients neuve.\n"
                                      "Vous pourrez encore annuler la réinitialisation en restaurant le fichier de paramétrage"
                                      " à partir de la sauvegarde (fichier RufusBackup.ini)."));
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
    time = (volume/1024/1024/1024 /2)*60000; //duréée approximative de sauvegarde en ms
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

//! Libellé « base : table1, table2, … » d'un fichier .sql, affiché dans la fenêtre de progression
//! pendant la création de la base (les fichiers de base vierge sont petits : lecture sans souci).
static QString libelleTablesFichierSQL(const QString& chemin)
{
    const QString base = QFileInfo(chemin).completeBaseName();
    QFile f(chemin);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return base;
    const QString contenu = QString::fromUtf8(f.readAll());
    f.close();
    static const QRegularExpression rx("CREATE TABLE (?:IF NOT EXISTS )?[`\"]?([A-Za-z0-9_]+)",
                                       QRegularExpression::CaseInsensitiveOption);
    QStringList tables;
    QRegularExpressionMatchIterator it = rx.globalMatch(contenu);
    while (it.hasNext())
        tables << it.next().captured(1);
    return tables.isEmpty() ? base : base + " : " + tables.join(", ");
}

//! Nombre de tables d'un fichier .sql, en comptant le marqueur mysqldump « Table structure for table `X` »
//! par LECTURE EN MORCEAUX (64 Ko) : mémoire bornée, indispensable pour les sauvegardes qui peuvent peser
//! plusieurs Go. `reste` reporte la fin d'un morceau au suivant pour ne pas rater un marqueur à cheval.
static int compterTablesFichierSQL(const QString& chemin)
{
    QFile f(chemin);
    if (!f.open(QIODevice::ReadOnly))
        return 0;
    static const QByteArray marqueur = "Table structure for table `";
    int n = 0;
    QByteArray reste;
    while (!f.atEnd())
    {
        const QByteArray morceau = reste + f.read(1 << 16);
        int idx = 0;
        while ((idx = morceau.indexOf(marqueur, idx)) != -1) { ++n; idx += marqueur.size(); }
        reste = morceau.right(marqueur.size() - 1);   // < taille d'un marqueur → jamais de double comptage
    }
    f.close();
    return n;
}

bool Procedures::RestaureBase(bool BaseVierge, bool PremierDemarrage, bool VerifPostesConnectes, QWidget *parent, QString cheminRestauration)
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
        OKBouton.setText(tr("J'ai compris\nJe confirme"));
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
            // ALTER USER après CREATE USER IF NOT EXISTS : impose le mot de passe
            // (aléatoire du cabinet) même si le compte existait déjà — CREATE USER
            // IF NOT EXISTS ne met PAS à jour le mot de passe d'un compte présent.
            db->StandardSQL("CREATE USER IF NOT EXISTS '" LOGIN_SQL "'@'%' IDENTIFIED BY '" + MySQLInstaller::motDePasseSQL() + "'");
            db->StandardSQL("ALTER USER '" LOGIN_SQL "'@'%' IDENTIFIED BY '" + MySQLInstaller::motDePasseSQL() + "'");
            db->StandardSQL("GRANT ALL ON *.* TO '" LOGIN_SQL "'@'%' WITH GRANT OPTION");
            db->StandardSQL("CREATE USER IF NOT EXISTS '" LOGIN_SQL "SSL'@'%' IDENTIFIED BY '" + MySQLInstaller::motDePasseSQL() + "' REQUIRE SSL");
            db->StandardSQL("ALTER USER '" LOGIN_SQL "SSL'@'%' IDENTIFIED BY '" + MySQLInstaller::motDePasseSQL() + "' REQUIRE SSL");
            db->StandardSQL("GRANT ALL ON *.* TO '" LOGIN_SQL "SSL'@'%' WITH GRANT OPTION");

            //! Restauration base par base (un fichier .sql = une base), avec la MÊME fenêtre de
            //! progression que pour les fichiers, mais montrant les tables de chaque base. Sur une
            //! machine lente la création peut être longue : sans fenêtre visible, on croit à un
            //! plantage. ExecuteScriptSQL laisse tourner la boucle d'événements → la fiche reste vivante.
            UpProgressDialog *progdial = new UpProgressDialog(0, listnomsfilestorestore.size(), parent);
            progdial->show();
            int a = 0;
            for (int i = 0; i < listnomsfilestorestore.size(); i++)
            {
                progdial->setValue(i);
                progdial->setLabelText(tr("Création de la base en cours…") + "\n\n"
                                       + libelleTablesFichierSQL(listnomsfilestorestore.at(i)));
                qApp->processEvents();
                a = ExecuteScriptSQL(QStringList() << listnomsfilestorestore.at(i));
                if (a != 0)
                    break;
            }
            progdial->setValue(listnomsfilestorestore.size());
            delete progdial;
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

        QDir dirtorestore;
        if (!cheminRestauration.isEmpty())
        {
            //! Mode AUTOMATIQUE (migration de base) : dossier imposé par l'appelant, aucune
            //! interaction (ni choix de dossier ni saisie de mot de passe).
            dirtorestore = QDir(cheminRestauration);
            if (!dirtorestore.exists())
                return false;
        }
        else
        {
            /*! 1 - choix du dossier où se situe la sauvegarde */
            UpMessageBox::Information(parent, tr("Choix du dossier de sauvegarde"),
                                      tr("Dans la fiche suivante, choisissez le dossier "
                                      "contenant la sauvegarde de la base.") + "<br/><br/>" +
                                      tr("Une fois le dossier sélectionné, "
                                      "la sauvegarde commencera automatiquement.") + "<br/>" +
                                      tr("Ce processus est long et peut durer plusieurs minutes (environ 1' pour 2 Go)"));
            QString dir = PATH_DIR_RUFUS;
            QUrl url = Utils::getExistingDirectoryUrl(parent, tr("Restaurer à partir du dossier"), QUrl::fromLocalFile(dir));
            if (url == QUrl())
                return false;
            dirtorestore = QDir(url.path());
            //! GARDE-FOU : on vérifie que le dossier choisi contient bien TOUS les fichiers d'une
            //! sauvegarde Rufus complète AVANT d'aller plus loin. Sans ce contrôle, choisir un dossier
            //! quelconque (sans les .sql) faisait planter la restauration.
            {
                const QStringList requis = QStringList()
                    << "rufus.sql" << "Ophtalmologie.sql" << "Images.sql"
                    << "ComptaMedicale.sql" << "user.sql" << "Rufus.ini";
                QStringList manquants;
                for (const QString &f : requis)
                    if (!dirtorestore.exists(f))
                        manquants << f;
                if (!manquants.isEmpty())
                {
                    UpMessageBox::Watch(parent, tr("Dossier de sauvegarde invalide"),
                        tr("Le dossier choisi ne contient pas une sauvegarde Rufus complète.") + "\n" +
                        tr("Fichier(s) manquant(s) :") + "\n" + manquants.join(", "));
                    return false;
                }
            }
            QString mdp("");
            if (!Utils::VerifMDP((PremierDemarrage? Utils::calcSHA1(MDP_ADMINISTRATEUR) : MDPAdmin()),tr("Saisissez le mot de passe Administrateur"), mdp, false, parent))
                return false;
        }


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
        QString dirimagerie = db->dirimagerie();
        if (OKImages || OKVideos || OKFactures)
        {
            if (dirimagerie == QString())
            {
                OKImages    = false;
                OKFactures  = false;
                OKVideos    = false;
            }
        }

        /*! 4 - choix des éléments à restaurer */
        AskBupRestore(RestoreOp, dirtorestore.absolutePath(), dirimagerie, OKini, OKImages, OKVideos, OKFactures);
        int result;
        bool erreurRestauration = false;   //! passe à true si la restauration de la BASE échoue (→ pas de redémarrage auto)
        if (!cheminRestauration.isEmpty())
        {
            //! Migration AUTOMATIQUE : on restaure UNIQUEMENT la base, sans afficher la boîte
            //! de choix (dont le bouton OK ne s'active qu'au clic d'une case).
            foreach (UpCheckBox *chk, dlg_buprestore->findChildren<UpCheckBox*>())
                chk->setChecked(chk->objectName() == "base");
            result = 1;
        }
        else
            result = dlg_buprestore->exec();
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
                                erreurRestauration = true;
                                break;
                            }
                        }
                        if (!echecfile)
                        {
                            //! Suppression de toutes les tables
                            QString Msg = tr("Suppression de l'ancienne base Rufus en cours");
                            UpSystemTrayIcon::I()->showMessage(tr("Messages"), Msg, Icons::icSunglasses(), 3000);
                            db->VideDatabases();

                            //! Progression PAR TABLE, comme la sauvegarde. Ici mysql LIT un .sql déjà
                            //! complet : on ne peut pas suivre sa position. Le bon signal est la base qui se
                            //! REMPLIT → on interroge information_schema (tables créées) pendant que mysql
                            //! travaille (callback appelé par ExecuteScriptSQL). Total N = nombre de marqueurs
                            //! « Table structure » dans les .sql (comptés en morceaux → mémoire bornée).
                            int totalTables = 0;
                            for (const QString& f : listnomsfilestorestore)
                                totalTables += compterTablesFichierSQL(f);
                            totalTables = qMax(1, totalTables);
                            const QString schemas = "'" DB_RUFUS "','" DB_COMPTA "','" DB_IMAGES "','" DB_OPHTA "'";

                            UpProgressDialog *progdial = new UpProgressDialog(0, totalTables, parent);
                            progdial->setLabelText(tr("Restauration de la base en cours…"));
                            progdial->show();
                            qApp->processEvents();

                            //! Appelé régulièrement par ExecuteScriptSQL : compte les tables déjà restaurées
                            //! et la dernière créée. Limité à ~1 interrogation / 400 ms (ne pas surcharger le
                            //! serveur qui restaure en même temps).
                            QElapsedTimer chrono; chrono.start();
                            auto majProgres = [&]() {
                                if (chrono.elapsed() < 400) return;
                                chrono.restart();
                                bool ok = false;
                                QVariantList c = DataBase::I()->getFirstRecordFromStandardSelectSQL(
                                    "SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_TYPE='BASE TABLE' AND TABLE_SCHEMA IN (" + schemas + ")", ok);
                                const int done = (ok && !c.isEmpty()) ? c.at(0).toInt() : progdial->value();
                                ok = false;
                                QVariantList t = DataBase::I()->getFirstRecordFromStandardSelectSQL(
                                    "SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_TYPE='BASE TABLE' AND TABLE_SCHEMA IN (" + schemas + ") ORDER BY CREATE_TIME DESC, TABLE_NAME DESC LIMIT 1", ok);
                                const QString table = (ok && !t.isEmpty()) ? t.at(0).toString() : QString();
                                progdial->setValue(done);
                                progdial->setLabelText(tr("Restauration de la base en cours…") + "\n\n"
                                    + QString("%1/%2   —   ").arg(done).arg(totalTables) + tr("table ") + table);
                            };
                            int a = 0;
                            for (int i = 0; i < listnomsfilestorestore.size(); i++)
                            {
                                a = ExecuteScriptSQL(QStringList() << listnomsfilestorestore.at(i), majProgres);
                                if (a != 0)
                                    break;
                            }
                            progdial->setValue(totalTables);
                            delete progdial;
                            if (a != 0)
                            {
                                UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Incident pendant la restauration"), Icons::icSunglasses(), 3000);
                                erreurRestauration = true;
                            }
                            else
                                msg += tr("Base de données Rufus restaurée\n");
                        }
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
                        QString dirdestinationimg   = dirimagerie + NOM_DIR_IMAGES;
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
                            /*! fiche ouverte AVANT le comptage, en barre animée (0,0) : il dure et laissait l'écran figé */
                            UpProgressDialog *progdial = new UpProgressDialog(0,0, parent);
                            progdial->setLabelText(tr("Restauration des fichiers d'imagerie"));
                            progdial->show();
                            int t = 0;
                            Utils::countFilesInDirRecursively(dirrestaureimagerie, t);
                            progdial->setRange(0,t);
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
                        QString dirdestinationfact  = dirimagerie + NOM_DIR_FACTURES;
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
                            UpProgressDialog *progdial = new UpProgressDialog(0,0, parent);
                            progdial->setLabelText(tr("Restauration des factures"));
                            progdial->show();
                            int t = 0;
                            Utils::countFilesInDirRecursively(dirrestaurefactures, t);
                            progdial->setRange(0,t);
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
                        QString dirdestinationvid   =  dirimagerie + NOM_DIR_VIDEOS;
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
                            UpProgressDialog *progdial = new UpProgressDialog(0,0, parent);
                            progdial->setLabelText(tr("Restauration des videos"));
                            progdial->show();
                            int t = 0;
                            Utils::countFilesInDirRecursively(dirrestaurevideo, t);
                            progdial->setRange(0,t);
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
        //! Restauration depuis une sauvegarde réussie (base sans erreur) : on relance Rufus tout seul
        //! pour repartir proprement sur la base restaurée. Contrairement à la base VIERGE, aucun
        //! CreerPremierUser à faire ici — la sauvegarde contient déjà les utilisateurs. En mode migration
        //! AUTOMATIQUE (cheminRestauration fourni), on laisse l'appelant gérer sa propre relance.
        //! Message d'attente NON bloquant (splash 3 s, pas de clic) puis relance automatique : on laisse
        //! le splash s'afficher ~2,5 s (Utils::Pause fait vivre l'UI) avant de quitter.
        if (result > 0 && cheminRestauration.isEmpty() && !erreurRestauration)
        {
            Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
            Utils::Redemarrage();
        }
        return (result > 0);
    }
}

/*!
 * \brief Procedures::DerniereSauvegardeInstallation
 * Dossier de la sauvegarde valide la plus récente laissée par Rufus avant un effacement du serveur.
 */
QString Procedures::DerniereSauvegardeInstallation()
{
    QDir dir(PATH_DIR_MIGRATIONMYSQL);
    for (const QString& sub : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time))
        if (SauvegardeBaseValide(dir.absolutePath() + "/" + sub))
            return dir.absolutePath() + "/" + sub;
    return "";
}

/*!
 * \brief Procedures::SauvegarderBaseAvantInstallation
 * Sauvegarde la base Rufus d'un serveur qui va être effacé, dans le dossier figé.
 * \param loginSQL  compte administrateur MySQL du serveur à sauvegarder
 * \param mdpSQL    son mot de passe
 */
bool Procedures::SauvegarderBaseAvantInstallation(QString loginSQL, QString mdpSQL)
{
    setDirSQLExecutable();
    db->initParametresConnexionSQL("localhost", 3306);
    if (!db->connectToDataBase(DB_RUFUS, loginSQL, mdpSQL).isEmpty())
        return false;

    //! Marge ×2, plancher 50 Mo pour le SQL et ses entêtes
    const qint64 requis = qMax<qint64>(db->DatabaseSize() * 2, 50LL * 1024 * 1024);
    QString dossier = QString(PATH_DIR_MIGRATIONMYSQL);
    forever
    {
        Utils::mkpath(dossier);
        const qint64 dispo = QStorageInfo(dossier).bytesAvailable();
        if (dispo >= requis)
            break;
        UpMessageBox::Watch(Q_NULLPTR, tr("Espace disque insuffisant"),
            tr("Le support de sauvegarde ne dispose pas d'assez d'espace libre.") + "\n\n" +
            tr("Espace nécessaire (estimé) : ") + Utils::getExpressionSize(requis) + "\n" +
            tr("Espace disponible : ") + Utils::getExpressionSize(dispo) + "\n\n" +
            tr("Choisissez un autre support de sauvegarde (clé USB, disque externe…)."));
        QUrl url = Utils::getExistingDirectoryUrl(Q_NULLPTR, tr("Choisissez un dossier de sauvegarde"),
                                                  QUrl::fromLocalFile(QDir::homePath()));
        if (url == QUrl())
            return false;   //! sans sauvegarde, on n'efface rien
        dossier = url.path();
    }

    //! adminrufus peut ne pas exister (ou son mdp être perdu) : le dump passe par le compte admin saisi
    if (!Backup(dossier, true, false, false, false, false, Q_NULLPTR, loginSQL, mdpSQL))
        return false;
    m_ostask.waitForFinished(-1);

    //! sous-dossier horodaté que Backup vient de créer
    const QStringList sub = QDir(dossier).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    m_sauvegardeInstallation = sub.isEmpty() ? "" : dossier + "/" + sub.first();
    if (m_sauvegardeInstallation.isEmpty() || !SauvegardeBaseValide(m_sauvegardeInstallation))
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Sauvegarde incomplète"),
            tr("La sauvegarde de votre base a échoué. Rien n'a été effacé."));
        m_sauvegardeInstallation = "";
        return false;
    }
    UpMessageBox::Watch(Q_NULLPTR, tr("Base sauvegardée"),
        tr("Votre base a été sauvegardée dans :") + "\n" + m_sauvegardeInstallation + "\n\n" +
        tr("Notez cet emplacement : Rufus vous proposera de restaurer cette sauvegarde après "
           "l'installation, et vous demandera où elle se trouve si ce n'est pas sur ce disque."));
    return true;
}

//! Vérifie qu'une sauvegarde de base (mysqldump) dans 'dossier' est complète AVANT toute
//! destruction : le fichier rufus.sql existe, n'est pas vide, et se termine par la marque
//! « Dump completed » que mysqldump écrit en fin de fichier réussi.
bool Procedures::SauvegardeBaseValide(QString dossier)
{
    const QString chemin = dossier + "/" DB_RUFUS ".sql";
    QFileInfo fi(chemin);
    if (!fi.exists() || fi.size() < 100)
        return false;
    QFile f(chemin);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const qint64 tailLen = qMin<qint64>(fi.size(), 4096);   // mysqldump écrit « -- Dump completed » à la fin
    f.seek(fi.size() - tailLen);
    const QString fin = QString::fromUtf8(f.read(tailLen));
    f.close();
    return fin.contains("Dump completed", Qt::CaseInsensitive);
}

/*!
 * \brief Procedures::MettreAJourSocleMySQL
 * Migration du socle MySQL du poste hôte : sauvegarde validée, puis désinstallation/réinstallation et
 * restauration. Ne désinstalle jamais sans sauvegarde valide.
 */
bool Procedures::MettreAJourSocleMySQL()
{
    UpMessageBox msgbox(Q_NULLPTR);
    msgbox.setText(tr("Mise à jour du serveur MySQL nécessaire"));
    msgbox.setInformativeText(
        tr("Cette version de Rufus nécessite une version plus récente du serveur MySQL.") + "\n\n" +
        tr("Rufus va sauvegarder votre base, désinstaller l'ancien MySQL, installer la nouvelle "
           "version, puis vous proposer de restaurer votre base."));
    msgbox.setIcon(UpMessageBox::Warning);
    //! Refuser n'interrompt pas Rufus : le libellé doit le dire, sinon on n'ose pas cliquer.
    UpSmallButton *Annul = new UpSmallButton(); Annul->setText(tr("Plus tard,\npoursuivre le démarrage"));
    UpSmallButton *Maj   = new UpSmallButton(); Maj  ->setText(tr("Mettre à jour le serveur,\nje dispose d'une sauvegarde"));
    msgbox.addButton(Annul, UpSmallButton::CLOSEBUTTON);
    msgbox.addButton(Maj,   UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != Maj)
        return false;

    if (!SauvegarderBaseAvantInstallation(LOGIN_SQL, MySQLInstaller::motDePasseSQL()))
        return false;   //! rien n'a été désinstallé

    if (!MySQLInstaller().reinstallerSocleMySQLpourMigration())
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Réinstallation impossible"),
            tr("La réinstallation de MySQL a échoué.") + "\n" +
            tr("Votre sauvegarde est conservée dans :") + "\n" + m_sauvegardeInstallation);
        return false;
    }
    if (InstallationRufus(true))
        return true;

    //! serveur neuf mais base vide : sans ce rappel, on ne sait plus où est la sauvegarde
    UpMessageBox::Watch(Q_NULLPTR, tr("Base non restaurée"),
        tr("Le serveur MySQL a été mis à jour, mais votre base n'a pas été restaurée.") + "\n" +
        tr("Votre sauvegarde est conservée dans :") + "\n" + m_sauvegardeInstallation);
    return false;
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
                                          " <b>" + QString::number(VERSION_BASE) + "</b><br />" +
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
            if (Version == 83 && a == 0)
            {
                //! les cotations personnalisées (montant pratiqué par utilisateur) migrent vers
                //! les 4 tables de jointures dédiées, créées par majbase83
                db->exporteJointures();
                //! puis on nettoie cotations : reclasse les associations (CCAM=4) et déduplique
                db->nettoieTableCotations();
                //! on complète les tip manquants pour les cotations NGAP et CCAM
                Datas::I()->cotations->completeTipsManquants();
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
        text += "<br/>" + QObject::tr("pour éviter des dysfonctionnements ou une altération de votre base de données Rufus");
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
    //! Raccourci de lancement « -installMySQL » : va droit à l'installation d'un serveur neuf
    if (QApplication::arguments().contains(QStringLiteral("-installMySQL")))
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Installation de MySQL"),
            tr("Rufus va maintenant installer un serveur MySQL neuf sur ce poste, "
               "puis créer une nouvelle base patients."));
        PremierDemarrage(/*forceBaseVierge=*/true);
    }

    QString server = "localhost";
    if( db->ModeAccesDataBase() == Utils::Poste )
        server = "localhost";
    else
        server = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Serveur).toString();

    int port = m_settings->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Port).toInt();

    db->initParametresConnexionSQL(server, port);

    //! ACCÈS DISTANT : contrôle des CLÉS SSL (client-key.pem, client-cert.pem, ca-cert.pem)
    if (db->ModeAccesDataBase() == Utils::Distant)
    {
        if (!ClesSSLPresentes())
        {
             const QString dirActuel = m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString();
            QString corps = tr("Les clés de cryptage SSL permettant la connexion à distance ne sont pas retrouvées.") + "\n\n";
            if (!dirActuel.isEmpty())
                corps += tr("Rufus les a cherchées dans le dossier :") + "\n\n" +
                         dirActuel + "\n\n";
            corps += tr("Indiquez, dans la boîte de dialogue suivante, le dossier contenant "
                        "client-key.pem et client-cert.pem.");
            UpMessageBox::Watch(Q_NULLPTR, tr("Clés SSL introuvables"), corps);
            if (!ChoisirDossierClesSSL())
            {
                UpMessageBox::Watch(Q_NULLPTR, tr("Clés SSL introuvables"),
                    tr("L'accès distant nécessite les clés SSL du cabinet (client-key.pem, "
                       "client-cert.pem), à copier depuis le poste serveur sur une clé USB."));
                if (ListeModesAcces().size() > 1)
                    return false;
                if (VerifParamConnexion())
                    Utils::Redemarrage();
                return false;
            }
        }
    }

    if (db->ModeAccesDataBase() == Utils::Poste && !MySQLInstaller::serveurLocalPresent())
    {
        CreerOuRestaurerBase(tr("Aucun serveur de base de données"),
                           tr("Aucun serveur MySQL n'est installé sur ce poste.") + "\n" +
                           tr("Pour utiliser Rufus en monoposte, créez une nouvelle base patients "
                              "(le serveur sera installé automatiquement), ou quittez."),
                           false /*proposerRestauration*/);
        return false;
    }

    //! CONNEXION + CONTRÔLE DE VERSION + SÉCURISATION — AVANT l'identification
    QString errConnexion = MySQLInstaller::connecterAvecCandidats(DB_RUFUS);
    while (!errConnexion.isEmpty())
    {
        //! Réseau : vérifier l'adresse du serveur
        if (db->ModeAccesDataBase() != Utils::Poste
         && db->causeEchecConnexion() == DataBase::ServeurInjoignable)
        {
            //! 2. l'adresse est bonne, c'est le serveur qui manque : ne pas la mettre en cause
            if (DataBase::etatAdresse(server, port) == DataBase::PosteSansServeur)
            {
                UpMessageBox::Watch(Q_NULLPTR, tr("Le serveur du cabinet ne fonctionne pas"),
                    tr("Un poste répond bien à cette adresse :") + "\n\n" + server + "\n\n" +
                    tr("mais le serveur de la base de données n'y fonctionne pas.") + "\n\n" +
                    tr("Vérifiez, sur ce poste, que Rufus y a bien été installé et que le serveur est démarré."));
                return false;
            }

            UpMessageBox msgbox(Q_NULLPTR);
            msgbox.setIcon(UpMessageBox::Quest);
            msgbox.setText(tr("L'adresse du serveur est inexacte"));
            msgbox.setInformativeText(
                tr("Rufus cherche la base de données du cabinet à cette adresse :") + "\n\n" +
                server + "\n\n" +
                tr("Il n'y a pas de poste connecté à cette adresse"));
            UpSmallButton *bAnnuler = new UpSmallButton(tr("Annuler"));
            UpSmallButton *bCorriger= new UpSmallButton(tr("Corriger l'adresse"));
            msgbox.addButton(bAnnuler,  UpSmallButton::CANCELBUTTON);
            msgbox.addButton(bCorriger, UpSmallButton::EDITBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != bCorriger)
                return false;

            QString nouvelle = server;
            if (!Utils::SaisirAdresseIP(tr("Adresse du serveur du cabinet :"), nouvelle))
                return false;
            server = Utils::calcIP(nouvelle, false);
            if (DataBase::etatAdresse(server, port) == DataBase::Deserte)
                continue;               /*!< personne non plus à celle-ci : on la redemande, Rufus.ini intact */

            m_settings  ->setValue(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Param_Serveur, server);
            m_settings  ->sync();
            db          ->initParametresConnexionSQL(server, port);
            errConnexion = MySQLInstaller::connecterAvecCandidats(DB_RUFUS);
            continue;                   /*!< connectée, on sort ; sinon la cause a changé et la suite s'en charge */
        }
        //! Un serveur est bien là mais il éconduit ce poste : rien ne se répare depuis ici
        if (db->causeEchecConnexion() == DataBase::ServeurMuet)
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Le serveur refuse la connexion"),
                tr("Un serveur répond bien à cette adresse, mais il refuse de dialoguer avec ce poste.") + "\n\n" +
                tr("Vérifiez, sur le poste qui héberge la base, que ce poste est autorisé à s'y connecter."));
            return false;
        }

        //! Le serveur a répondu et refusé (mdp, clés SSL) : son adresse est bonne, ne la mettons pas en doute
        if (db->causeEchecConnexion() == DataBase::ClesSSL
         && db->ModeAccesDataBase() == Utils::Distant)   /*!< Dossier_ClesSSL n'existe que pour le distant */
        {
            UpSmallButton *bAnnuler   = new UpSmallButton(tr("Annuler"));
            UpSmallButton *bDemarche  = new UpSmallButton(tr("Comment me procurer\ndes clés valides ?"));
            UpSmallButton *bChercher  = new UpSmallButton(tr("Rechercher\nles bonnes clés"));
            UpMessageBox msgbox(Q_NULLPTR);
            msgbox.setIcon(UpMessageBox::Quest);
            msgbox.setText(tr("Liaison chiffrée refusée"));
            msgbox.setInformativeText(
                tr("Le serveur du cabinet répond, mais il refuse la liaison chiffrée.") + "\n\n" +
                tr("Les clés SSL de ce poste sont invalides ou périmées."));
            msgbox.addButton(bAnnuler,  UpSmallButton::CANCELBUTTON);
            msgbox.addButton(bDemarche, UpSmallButton::LOUPEBUTTON);
            msgbox.addButton(bChercher, UpSmallButton::EDITBUTTON);
            msgbox.exec();

            if (msgbox.clickedButton() == bDemarche)
            {
                UpMessageBox::Watch(Q_NULLPTR, tr("Se procurer les clés SSL du cabinet"),
                    tr("Sur le poste qui héberge la base : menu Édition / Paramètres, bouton d'export "
                       "des clés SSL vers une clé USB.") + "\n\n" +
                    tr("Sur ce poste : copiez les fichiers de la clé USB (ca-cert.pem, client-cert.pem, "
                       "client-key.pem) dans un dossier, puis désignez-le à Rufus."));
                continue;               /*!< la fiche reparaît pour choisir entre chercher et abandonner */
            }
            if (msgbox.clickedButton() != bChercher)
                return false;
            if (!ChoisirDossierClesSSL())
                continue;               /*!< pas de clés dans ce dossier : on repropose */
            m_settings  ->sync();
            errConnexion = MySQLInstaller::connecterAvecCandidats(DB_RUFUS);
            continue;                   /*!< connectée, on sort ; sinon la cause a changé et la suite s'en charge */
        }


        /*! 2. Toujours pas de connexion
         *  demander un mdp
         *  et en monoposte
            *  proposer de réinitialiser
            *  proposer de restaurer avec mdp de secours
         */
        if (db->ModeAccesDataBase() != Utils::Poste || MySQLInstaller::socleLocalConforme())
        {
            forever
            {
                const MySQLInstaller::IssueMdp issue =
                    MySQLInstaller::RecupererMotDePasseMySQL(Q_NULLPTR, QString(), QString(), db->ModeAccesDataBase() == Utils::Poste);

                if (issue == MySQLInstaller::IssueMdp::Obtenu)
                {
                    errConnexion = MySQLInstaller::connecterAvecCandidats(DB_RUFUS);
                    break;
                }
                if (issue == MySQLInstaller::IssueMdp::Annule)
                    return false;
                if (db->ModeAccesDataBase() == Utils::Distant)
                    break;   //! le secours ne s'atteint pas depuis internet
                //! Pas de mot de passe valide -> restauration par le mot de passe de secours
                if (issue == MySQLInstaller::IssueMdp::Inconnu
                 || issue == MySQLInstaller::IssueMdp::EchecSaisie)
                {
                    if (UpMessageBox::Question(Q_NULLPTR, tr("Aucun mot de passe ne fonctionne"),
                            tr("Rufus peut rétablir l'accès à la base avec le mot de passe de "
                               "SECOURS choisi à l'installation de la base.") + "\n" +
                            tr("Vos données ne seront pas touchées.") + "\n\n" +
                            tr("Voulez-vous utiliser cette procédure ?"),
                            UpDialog::ButtonCancel | UpDialog::ButtonOK,
                            QStringList() << tr("Annuler") << tr("Rétablir l'accès"))
                        != UpSmallButton::STARTBUTTON)
                        continue;
                    if (MySQLInstaller().restaurerAvecMotDePasseDeSecours())
                    {
                        errConnexion = MySQLInstaller::connecterAvecCandidats(DB_RUFUS);
                        break;
                    }
                    continue;
                }

                //! « Réinitialiser le programme
                if (issue == MySQLInstaller::IssueMdp::Reinitialiser)
                {
                    if (UpMessageBox::Question(Q_NULLPTR, tr("Réinitialiser le programme"),
                            tr("Rufus va installer une base patients neuve sur cet ordinateur.") + "\n" +
                            tr("Les données de la base actuelle ne seront plus accessibles.") + "\n\n" +
                            tr("Voulez-vous continuer ?"),
                            UpDialog::ButtonCancel | UpDialog::ButtonOK,
                            QStringList() << tr("Annuler") << tr("Créer une nouvelle\nbase patients"))
                        != UpSmallButton::STARTBUTTON)
                        continue;
                    PremierDemarrage(/*forceBaseVierge=*/true);
                    return false;
                }
            }
        }
        else
        {
            //! monoposte, socle < 8.0.14 : aucun aléatoire n'a pu être posé, réclamer un mdp n'a pas de sens
            if (UpMessageBox::Question(Q_NULLPTR, tr("Version de MySQL trop ancienne"),
                    tr("Impossible de se connecter à votre serveur MySQL, et sa version est trop "
                       "ancienne pour cette version de Rufus.") + "\n\n" +
                    tr("Rufus doit installer un serveur neuf : tout ce que contient l'actuel sera "
                       "perdu.") + "\n" +
                    tr("Rufus vous proposera ensuite de restaurer une sauvegarde de votre base."),
                    UpDialog::ButtonCancel | UpDialog::ButtonOK,
                    QStringList() << tr("Annuler, je vais\nsauvegarder les données")
                                  << tr("Installer un\nserveur neuf"))
                == UpSmallButton::STARTBUTTON)
                PremierDemarrage(/*forceBaseVierge=*/true, /*demanderRestauration=*/true);
            return false;
        }

        //! Toujours en échec
        if (!errConnexion.isEmpty())
        {
            if (db->ModeAccesDataBase() != Utils::Poste)
            {
                //! le secours passe par un compte que seul le réseau local atteint
                QString corps = tr("Aucun mot de passe connu de ce poste n'ouvre la base du cabinet.") + "\n\n" +
                                tr("Vous devez récupérer un mot de passe valide, copié sur une clé USB "
                                   "depuis un poste qui fonctionne (menu Édition / Paramètres).");
                if (db->ModeAccesDataBase() == Utils::Distant)
                    corps += "\n\n" + tr("À défaut, le mot de passe peut être rétabli par la procédure de "
                                          "secours, mais seulement depuis un poste du réseau local du cabinet.");
                UpMessageBox::Watch(Q_NULLPTR, tr("Connexion à la base impossible"), corps);
                return false;
            }
            UpMessageBox::Watch(Q_NULLPTR, tr("Connexion à la base impossible"),
                tr("Le mot de passe vient pourtant d'ouvrir la base.") + "\n" +
                tr("Vérifiez que le serveur MySQL de ce poste fonctionne, puis relancez Rufus."));
            return false;
        }
    }

    //! COHÉRENCE DE LA CONFIGURATION MySQL (MONOPOSTE uniquement).
    MySQLInstaller().verifierEtReparerConfigMonoposte();

    //! CLÉS SSL (MONOPOSTE)
    MySQLInstaller().controlerClesSSLMonoposte();

    //! ACCÈS DISTANT : avertir PROACTIVEMENT si les clés SSL approchent de l'expiration.
    MySQLInstaller().avertirExpirationClesSSLDistant();

    //! Le partage réseau d'imagerie peut ne pas être monté alors que la base répond : les images
    //! seraient introuvables sans explication. On lit le réglage brut, dirimagerie() créerait le dossier.
    if (db->ModeAccesDataBase() == Utils::ReseauLocal)
    {
        const QString dirimg = m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Imagerie).toString();
        if (dirimg.isEmpty() || !QDir(dirimg).exists())
            UpMessageBox::Watch(Q_NULLPTR, tr("Dossier d'imagerie du cabinet inaccessible"),
                (dirimg.isEmpty()
                    ? tr("Aucun dossier d'imagerie n'est indiqué pour ce poste.")
                    : tr("Rufus ne trouve pas le dossier d'imagerie du cabinet :") + "\n\n"
                      + "<p align=\"center\"><b><span style=\"color:#c00000; font-size:12pt;\">" + dirimg + "</span></b></p>") + "\n\n" +
                tr("Ce dossier est partagé par le poste serveur : montez ce partage réseau sur cet "
                   "ordinateur, ou corrigez son emplacement dans Édition / Paramètres / onglet "
                   "« Réseau local ».") + "\n\n" +
                tr("Rufus démarre quand même, mais les images ne seront ni lues ni enregistrées."));
    }

    //! COHÉRENCE DE LA BASE Rufus : la connexion au serveur a réussi
    {
        bool baseOk = false;
        db->StandardSelectSQL("SELECT 1 FROM " TBL_UTILISATEURS " LIMIT 1", baseOk, "");
        if (!baseOk)
        {
            if (db->ModeAccesDataBase() == Utils::Poste)
            {
                //! poste hôte : restaurer la base, ou en créer une neuve s'il n'a pas de sauvegarde
                CreerOuRestaurerBase(tr("Base de données endommagée"),
                                   tr("La connexion au serveur MySQL fonctionne, mais la base de données patients Rufus est altérée.") + "\n" +
                                   tr("Vous pouvez la restaurer depuis une sauvegarde, créer une nouvelle base patients, ou quitter."),
                                   true /*proposerRestauration*/);
                return false;
            }
            //! Poste CLIENT (réseau local / distant) : il ne répare JAMAIS la base partagée des
            //! autres. On l'informe ; la réparation se fait depuis le poste serveur.
            UpMessageBox::Watch(Q_NULLPTR, tr("Base de données endommagée"),
                tr("La connexion au serveur MySQL fonctionne, mais la base de données patients Rufus est altérée.") + "\n" +
                tr("Vous devez restaurer une base patients ; cette restauration ne peut se faire "
                   "qu'à partir du poste serveur."));
            return false;
        }
    }

    //! Contrôle du socle MySQL : si version < 8.0.14 (cf. VERSION_MYSQL_MINI), on NE fait RIEN ici.
    //! On MÉMORISE simplement le besoin de mise à jour
    m_socleMySQLAMettreAJour = !MySQLInstaller::socleMySQLConforme();

    //! Sécurisation : si la base est encore sur gaxt78iy, pose un aléatoire (en CONSERVANT gaxt78iy
    //! en 2e mot de passe) et écrit le .dbkey ; supprime gaxt78iy si la deadline (sécurisation +
    //! 30 j) est passée. No-op si déjà fait.
    MySQLInstaller::entretienApresConnexion();

    if (!IdentificationUser())
        return false;

    //! Bases installées AVANT le compte de secours
    if (currentuser()->id()== 1 && db->ModeAccesDataBase() == Utils::Poste)
        MySQLInstaller::controlerCompteDeSecours();

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
    -- Contrôle DIFFÉRÉ du socle MySQL, APRÈS l'affichage de la fenêtre Rufus -----------------------------------------
    -----------------------------------------------------------------------------------------------------------------
    Appelée depuis main.cpp juste après w.show(). Si Connexion_A_La_Base() a détecté un socle MySQL
    trop ancien (< VERSION_MYSQL_MINI), c'est ICI — et seulement ici — qu'on en informe l'utilisateur,
    une fois que Rufus est VISIBLE à l'écran (il tourne déjà en mode dégradé sur l'ancien MySQL).
        • Poste HÔTE (monoposte) : on propose la mise à jour du socle (MettreAJourSocleMySQL gère son
          propre dialogue de consentement, la sauvegarde validée, la réinstall, la restauration et la
          relance). Annulation → on continue en mode dégradé.
        • Poste CLIENT (réseau local / distant) : il n'héberge pas la base, il ne peut pas migrer. On
          l'informe simplement que la mise à jour doit se faire depuis le poste serveur.
    Pourquoi processEvents() d'abord : exec() bloque la boucle d'événements ; sans cela, la fenêtre
    principale — montrée par w.show() à l'instant — n'aurait pas encore été PEINTE et le dialogue
    surgirait sur un fond vide. On vide donc la file de peinture en attente (même mécanisme que
    ShowMessage) pour que Rufus soit dessiné DERRIÈRE le dialogue modal. */
void Procedures::ControleSocleMySQLApresAffichage()
{
    if (!m_socleMySQLAMettreAJour)
        return;
    m_socleMySQLAMettreAJour = false;                   //! one-shot : on ne reposera pas la question dans la session

    //! Forcer le premier affichage de la fenêtre principale AVANT le dialogue modal.
    qApp->processEvents(QEventLoop::AllEvents, 100);

    if (db->ModeAccesDataBase() == Utils::Poste)
    {
        //! Poste hôte : MettreAJourSocleMySQL() porte son propre dialogue Annuler / Lancer la mise à
        //! jour, fait la sauvegarde validée puis migre et relance Rufus. Annulation → mode dégradé.
        MettreAJourSocleMySQL();
        return;
    }

    //! Poste CLIENT (LAN / distant) : il ne peut PAS migrer (ce n'est pas lui qui héberge la base).
    UpMessageBox::Watch(Q_NULLPTR, tr("Serveur MySQL à mettre à jour"),
        tr("Le serveur MySQL nécessite d'être mis à jour pour pouvoir utiliser") + "\n" +
        tr("les nouvelles fonctions de sécurité incluses dans cette version de Rufus.") + "\n" +
        tr("La mise à jour doit être effectuée depuis le poste serveur "
           "(en y lançant Rufus, qui s'en chargera).") + "\n\n" +
        tr("Même s'il est fortement conseillé de faire cette mise à jour") + "\n" +
        tr("vous pouvez continuer à travailler normalement."));
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Détermination du lieu exercice pour la session en cours -------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------- */
void Procedures::CalcLieuExercice()
{
    /*! Si le user est remplaçant, la liste des sites utilisés est celle du user remplacé */
    QMap<Site*,qlonglong> mapEtab = Datas::I()->sites->initListeByUser(currentuser()->id());
    QMap<int,qlonglong> mapid = QMap<int,qlonglong>();

    for (auto it = mapEtab.cbegin(); it != mapEtab.cend(); ++it)
    {
        Site *sit = qobject_cast<Site*>(it.key());
        if (sit != Q_NULLPTR)
            mapid.insert(sit->id(), it.value());
    }
    currentuser()->setmapUserSites(mapid);

    if (mapEtab.size() == 0)
        return;
    else if (mapEtab.size() == 1)
        Datas::I()->sites->setcurrentsite(mapEtab.firstKey());
    else
    {
        /*! Cas ou le praticien travaille dans plusieur centres
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
        boxlieux                ->setFixedHeight(((mapEtab.size() + 1)*hauteurligne)+5);
        QVBoxLayout *vbox       = new QVBoxLayout;
        bool isFirst = true;
        for (auto it = mapEtab.cbegin(); it != mapEtab.cend(); ++it)
        {
            Site *sit = qobject_cast<Site*>(it.key());
            if (sit != Q_NULLPTR)
            {
                UpRadioButton *pradiobutt = new UpRadioButton(boxlieux);
                pradiobutt->setText(sit->nom());
                pradiobutt->setrufusitem(sit);
                pradiobutt->setImmediateToolTip(sit->coordonnees());
                pradiobutt->setChecked(isFirst);
                vbox      ->addWidget(pradiobutt);
                isFirst = false;
            }
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
                Datas::I()->sites->setcurrentsite(qobject_cast<Site*>(rb->rufusitem()));
                break;
            }
        delete gAskLieux;
    }
    if (currentuser()->isSoignant() && m_parametres->cotationsfrance())
    {
        if (currentuser()->isRemplacant())
        {
            User *usr = Datas::I()->users->getById(currentuser()->idparent());
            if (usr != Q_NULLPTR)
            {
                currentuser()->setusenum(usr->usenum());
                if (usr->usenum())
                    VerifnumAM();
            }
        }
        else if (currentuser()->usenum())
            VerifnumAM();
    }
}

void Procedures::VerifnumAM()
{
    /*! On a déterminé le site de travail, on cherche le n° AM du user pour ce site de travail"
        Si le user est remplaçant, le n° AM sera celui du user remplacé */

    if (!db->parametres()->cotationsfrance())
        return;
    if(!m_currentuser->isSoignant())
        return;
    QMap<int,qlonglong> mapid = currentuser()->mapUserSites();
    if (mapid == QMap<int,qlonglong>() || mapid.size() == 0)
        return;

    int idsite = 0;
    if (Datas::I()->sites->currentsite() != Q_NULLPTR)
    {
        idsite = Datas::I()->sites->currentsite()->id();
        currentuser()->setidSite(idsite);
    }
    else return;
    if (currentuser()->isRemplacant() || !m_currentuser->ishisownsupervisor())
    {
        User* usr = Datas::I()->users->getById(currentuser()->idparent());
        if (usr != Q_NULLPTR)
        {
            currentuser()->setRPPSnumber(usr->NumPS());
            QMap<Site*,qlonglong> mapEtab = Datas::I()->sites->initListeByUser(usr->id());
            QMap<int,qlonglong> mapidparent = QMap<int,qlonglong>();
            for (auto it = mapEtab.cbegin(); it != mapEtab.cend(); ++it)
            {
                Site *sit = qobject_cast<Site*>(it.key());
                if (sit != Q_NULLPTR)
                    mapidparent.insert(sit->id(), it.value());
            }
            QMap<int,qlonglong>::Iterator itr = mapidparent.find(Datas::I()->sites->currentsite()->id());
            if (itr != mapidparent.end())
            {
                mapid.insert(Datas::I()->sites->currentsite()->id(), itr.value());
                currentuser()->setmapUserSites(mapidparent);
            }
        }
    }
    else if (currentuser()->currentAMnumber() == 0)
    {
        if (currentuser()->m_numAM() >0 && mapid.size() == 1)
        {
            mapid.insert(Datas::I()->sites->currentsite()->id(), currentuser()->m_numAM());
            currentuser()->setmapUserSites(mapid);
            //! enregistre la valeur de AMnumber dans jointuresLieux
            db->StandardSQL("update " TBL_JOINTURESLIEUX " set " CP_AMNUMBER_JOINTSITE " = " + (currentuser()->currentAMnumber()== 0? "null": QString::number(m_currentuser->currentAMnumber())) +
                            " where " CP_IDUSER_JOINTSITE  " = " + QString::number(currentuser()->id())  + " and " CP_IDLIEU_JOINTSITE " = " + QString::number(idsite));
        }
        else
        {
            qlonglong AMnumber = 0;
            UpMessageBox::Watch(Q_NULLPTR, tr("Vous n'avez pas de  numéro AM enregistré pour ce site"),
                                tr("Enregistrez le numéro AM correspondant à") +
                                "<br/><font color=\"blue\"><b>" + Datas::I()->sites->currentsite()->nom() + "</b></font><br/>" +
                                tr("dans la boîte de dialogue suivante"));
            UpDialog* dlg_ask               = new UpDialog();
            int w = 240;
            UpLabel         *lbldebut       = new UpLabel;
            lbldebut        ->setText(tr("Enregistrez le numéro AM (9 chiffres) correspondant à") +
                                      "<br/><font color=\"blue\"><b>" + Datas::I()->sites->currentsite()->nom() + "</b></font>");
            lbldebut->setAlignment(Qt::AlignCenter);
            QHBoxLayout *hlay = new QHBoxLayout;
            hlay->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding));

            QStringList listnumAM = QStringList();
            for (auto it = mapid.cbegin(); it != mapid.cend(); ++it)
                if (it.value() >0)
                    if (!listnumAM.contains(QString::number(it.value())))
                        listnumAM << QString::number(it.value());
            if (currentuser()->m_numAM() >0)
                if (!listnumAM.contains(QString::number(currentuser()->m_numAM())))
                    listnumAM << QString::number(currentuser()->m_numAM());
            std::sort(listnumAM.begin(), listnumAM.end());

            UpComboBox *combo   = new UpComboBox;
            combo               ->setFixedWidth(w);
            QLineEdit *line     = new QLineEdit;
            QCompleter *comp    = new QCompleter(listnumAM);
            comp                ->setCompletionMode(QCompleter::InlineCompletion);
            line                ->setCompleter(comp);
            line                ->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{9}"), Q_NULLPTR));
            combo               ->addItems(listnumAM);
            combo               ->setLineEdit(line);
            combo               ->setEditable(true);
            combo               ->setCurrentText("");

            hlay                ->addWidget(combo);
            hlay                ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding));
            dlg_ask->dlglayout()->insertWidget(0,lbldebut);
            dlg_ask->dlglayout()->insertLayout(1,hlay);
            dlg_ask             ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
            dlg_ask             ->setWindowTitle(tr("Enregistrement numéro AM"));
            dlg_ask->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
            connect(dlg_ask->OKButton,  &QPushButton::clicked,  dlg_ask,    [&] {AMnumber = combo->currentText().toLongLong();
                dlg_ask->accept();});
            if (dlg_ask->exec() == QDialog::Accepted)
            {
                if (AMnumber>0)
                {
                    mapid.insert(Datas::I()->sites->currentsite()->id(), AMnumber);
                    currentuser()->setmapUserSites(mapid);
                    db->StandardSQL("update " TBL_JOINTURESLIEUX " set " CP_AMNUMBER_JOINTSITE " = " +  (currentuser()->currentAMnumber()== 0? "null": QString::number(m_currentuser->currentAMnumber())) +
                                    " where " CP_IDUSER_JOINTSITE  " = " + QString::number(currentuser()->id())  + " and " CP_IDLIEU_JOINTSITE " = " + QString::number(idsite));
                }
            }
            delete dlg_ask;
        }
    }
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
                               tr("Voulez-vous utiliser la base de données des villes françaises?"),
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
                            tr("Voulez-vous utiliser le système français de cotation des actes médicaux?"),
                            UpDialog::ButtonCancel | UpDialog::ButtonOK,
                            QStringList() << tr("Non") << tr("Utiliser les cotations françaises"))
        == UpSmallButton::STARTBUTTON);
    db->setcotationsfrance(a);

    m_connexionbaseOK = true;
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
            CP_USENUM_USR " = 1,\n"
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
            "'04 56 78 90 12', "
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
    //! Identification du PRATICIEN, UNIQUEMENT. À ce stade, la connexion au serveur et la
    //! sécurisation ont DÉJÀ été faites par l'appelant Connexion_A_La_Base() (le contrôle de version
    //! MySQL, lui, est seulement MÉMORISÉ : il est traité APRÈS l'affichage, cf.
    //! ControleSocleMySQLApresAffichage()). Ici, on vérifie seulement que cet utilisateur existe
    //! dans la table des utilisateurs de la base.
    bool ok = false;
    dlg_identificationuser *dlg_IdentUser   = new dlg_identificationuser();
    dlg_IdentUser   ->setFont(m_applicationfont);
    QFile sha = QFile(PATH_DIR_RUFUS + "/sha1.txt");
    if (sha.open(QIODevice::ReadOnly))
    {
        QTextStream  ts(&sha);
        QString filecontents;
        filecontents.append(ts.readAll());
        //! (Re)connexion défensive en CASCADE (aléatoire .dbkey PUIS gaxt78iy) avant la vérification
        //! d'identité — la connexion est déjà ouverte par Connexion_A_La_Base, ce rappel ne nuit pas.
        ok = db->calcidUserConnected(filecontents.split("!!!!").at(0),filecontents.split("!!!!").at(1)) == DataBase::OK;
    }
    if (!ok)
        ok = dlg_IdentUser->exec() == QDialog::Accepted;
    if (ok)
    {
        VerifVersionBase();
        m_parametres = db->parametres();
        //! La langue du poste est portée par rufus.ini (Param_Poste_Version), qui FAIT FOI :
        //! on aligne la version EN MÉMOIRE sur l'ini, mais on NE réécrit PAS l'ini avec la
        //! version de la base — sinon le choix de langue du poste était écrasé (par la version
        //! de la base, FR par défaut) juste après l'identification.
        {
            const QString vposte = m_settings->value(Param_Poste_Version).toString();
            if (!vposte.isEmpty())
                m_parametres->setversion(vposte);
        }

        //! Avis « traduction assistée par IA » pour les versions NON françaises. Affiché UNE
        //! SEULE FOIS (marqueur persistant dans Rufus.ini), ici — après l'identification réussie
        //! et la connexion à la base, au moment où l'utilisateur commence vraiment à travailler.
        //! Le français (langue source) ne le voit pas. Invite à signaler les imperfections / à
        //! contribuer via www.rufusvision.org.
        {
            const QString lang    = m_settings->value(Param_Poste_Version).toString();
            const QString cleAvis = QString(Param_Poste) + "/AvisTraductionIA";
            if (lang != "FR" && !m_settings->value(cleAvis, false).toBool())
            {
                QString titre, texte;
                if (lang == "EN")
                {
                    titre = "A note about this translation";
                    texte = "This non-French version of Rufus was translated with the help of artificial "
                            "intelligence. Some wordings may be imperfect or awkward — thank you for your "
                            "understanding.\n\nIf you would like to help improve the translation, you are "
                            "very welcome: please get in touch via "
                            "<a href=\"https://www.rufusvision.org/contact.html\">www.rufusvision.org</a>.";
                }
                else if (lang == "ES")
                {
                    titre = "Una nota sobre esta traducción";
                    texte = "Esta versión no francesa de Rufus se ha traducido con ayuda de inteligencia "
                            "artificial. Algunas expresiones pueden ser imperfectas o poco naturales; "
                            "gracias por su comprensión.\n\nSi desea ayudar a mejorar la traducción, será "
                            "bienvenido: póngase en contacto a través de "
                            "<a href=\"https://www.rufusvision.org/contact.html\">www.rufusvision.org</a>.";
                }
                else if (lang == "BR")
                {
                    titre = "Uma nota sobre esta tradução";
                    texte = "Esta versão não francesa do Rufus foi traduzida com a ajuda de inteligência "
                            "artificial. Algumas expressões podem estar imperfeitas ou pouco naturais; "
                            "obrigado pela compreensão.\n\nSe você quiser ajudar a melhorar a tradução, "
                            "será bem-vindo: entre em contato pelo site "
                            "<a href=\"https://www.rufusvision.org/contact.html\">www.rufusvision.org</a>.";
                }
                else if (lang == "PT")
                {
                    titre = "Uma nota sobre esta tradução";
                    texte = "Esta versão não francesa do Rufus foi traduzida com a ajuda de inteligência "
                            "artificial. Algumas expressões podem estar imperfeitas ou pouco naturais; "
                            "obrigado pela sua compreensão.\n\nSe quiser ajudar a melhorar a tradução, será "
                            "bem-vindo: contacte-nos através de "
                            "<a href=\"https://www.rufusvision.org/contact.html\">www.rufusvision.org</a>.";
                }
                else if (lang == "IT")
                {
                    titre = "Una nota su questa traduzione";
                    texte = "Questa versione non francese di Rufus è stata tradotta con l'aiuto "
                            "dell'intelligenza artificiale. Alcune espressioni potrebbero essere imperfette "
                            "o poco naturali; grazie per la comprensione.\n\nSe desidera contribuire a "
                            "migliorare la traduzione, sarà il benvenuto: ci contatti tramite "
                            "<a href=\"https://www.rufusvision.org/contact.html\">www.rufusvision.org</a>.";
                }
                if (!texte.isEmpty())
                {
                    UpMessageBox::Watch(Q_NULLPTR, titre, texte, UpDialog::ButtonOK,
                                        "https://www.rufusvision.org/contact.html");
                    m_settings->setValue(cleAvis, true);
                    m_settings->sync();
                }
            }
        }
        if (m_settings->value(Utils::getBaseFromMode(Utils::Poste) + Dossier_Videos).toString() == "")
            m_settings->setValue(Utils::getBaseFromMode(Utils::Poste) + Dossier_Videos, db->dirimagerie() + NOM_DIR_VIDEOS);
        if (m_settings->value(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Videos).toString() == "")
            m_settings->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + Dossier_Videos, db->dirimagerie() + NOM_DIR_VIDEOS);
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
        Datas::I()->refractiondevices   ->initListe();
        //! la MAJ des cotations (CCAM / NGAP) est franco-française : hors France (version internationale)
        //! le fichier de cotations ne concerne pas le poste -> on ne vérifie rien.
        if (m_parametres->cotationsfrance())
        {
            //! cotations, une fois au lancement : (1) MAJ de la base depuis le fichier xml ; (2) chargement
            //! de la map à jour ; (3) remplissage des Tip vides UNIQUEMENT si le fichier a été traité
            //! (sinon il l'était déjà à un lancement précédent -> inutile de re-parser le xml)
            bool cotationsMisesAJour = false;                                    //! le fichier a réellement changé quelque chose
            const bool cotationsTraitees = DataBase::I()->verifMajCotations(cotationsMisesAJour);   //! Action 1 : MAJ base cotations (CCAM/NGAP)
            DataBase::I()                   ->corrigeMontantsPratiques();        //! invariant : pratiqué >= conventionnel (indépendant de la MAJ, à chaque démarrage)
            if (cotationsTraitees)
            {
                Datas::I()->cotations       ->completeTipsManquants();   //! Action 2 : Tip vides, dans la foulée de la MAJ et nulle part ailleurs
                //! le fichier est relu tous les jours : on ne prévient que s'il a apporté quelque chose,
                //! sinon le message tomberait au premier lancement de chaque jour
                if (cotationsMisesAJour)
                    ShowMessage::I()        ->SplashMessage(tr("La base de cotations Rufus a été mise à jour."), 4000, true);
            }
        }
        Datas::I()->cotations           ->initListe();
        MAJComptesBancaires(currentuser());
        m_applicationfont = currentuser()->police();
        qApp->setFont(m_applicationfont);

        AbsolutePathDirImagerie();
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
        UpSmallButton   BaseViergeBouton(tr("Nouvelle base patients"));
        switch (loginresult) {
        case dlg_identificationuser::CorruptedBase:
            msgbox.setText(tr("Base de données endommagée!"));
            msgbox.setInformativeText(tr("La base de données est endommagée.\n"
                                      "Voulez-vous la reconstruire à partir\n"
                                      "d'une sauvegarde ou recréer une base patients vierge?\n\n"));
            msgbox.setIcon(UpMessageBox::Info);
            msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&BaseViergeBouton, UpSmallButton::STARTBUTTON);
            msgbox.addButton(&RestaureBaseBouton, UpSmallButton::COPYBUTTON);
            msgbox.exec();
            if( (msgbox.clickedButton() == &RestaureBaseBouton))
            {
                if (RestaureBase(false,true,false))
                {
                    Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
                    UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va redémarrer pour que certaines données puissent être prises en compte"));
                    Utils::Redemarrage();
                }
            }
            else if (msgbox.clickedButton() == &BaseViergeBouton)
            {
                if (!RestaureBase(true, true))
                    exit(0);
                CreerPremierUser(m_loginSQL, m_passwordSQL);
                Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
                UpMessageBox::Watch(Q_NULLPTR,tr("Le programme va redémarrer pour que certaines données puissent être prises en compte"));
                Utils::Redemarrage();
            }
            break;
        default:
            break;
        }
    }
    delete dlg_IdentUser;
    return (currentuser() != Q_NULLPTR);
}

/*! --------------------------------------------------------------------------------------------------------------------------------------
Renvoie le chemin du dossier où est stockée l'imagerie, tel qu'il est vu en accès monoposte ou en accès distant
ce chemin correspond à:
1.  En accès monoposte c'est secure_file_priv + "/ Rufus" + "/Imagerie" , sur le serveur, et n'est donc pas stocké mais simplement calculé par Rufus
2.  En réseau local, c'est le chemin vers ce dossier sur le serveur soit -> adresse du serveur + secure_file_priv + "/ Rufus" + "/Imagerie"
    La variable correspondant à ce chemin est stockée dans rufus.ini "[BDD_LOCAL]/DossierImagerie"
3.  Ce chemin n'a pas de sens en accès distant mais on l'utilise pour stocker le chemin vers l'emplacement des copies des originaux d'imagerie
    La variable correspondant à ce chemin est stockée dans rufus.ini "[BDD_DISTANT]/DossierImagerie"

Returns the path of the folder where the imagery is stored, as seen in single-user or remote access.
this path corresponds to:
1.  In single-user access it is secure_file_priv + “/ Rufus” + “/Imaging” , on the server, and is therefore not stored but simply calculated by Rufus
2.  On a local network, it is the path to this folder on the server, i.e. -> server address + secure_file_priv + “/ Rufus” + “/Imagerie”.
    The variable corresponding to this path is stored in rufus.ini “[BDD_LOCAL]/DossierImagerie”.
3.  This path has no meaning in remote access, but is used to store the path to the location of copies of imaging originals.
    The variable corresponding to this path is stored in rufus.ini “[BDD_DISTANT]/DossierImagerie”.

Translated with www.DeepL.com/Translator (free version)
    ------------------------------------------------------------------------------------------------------------------------------------*/

QString Procedures::AbsolutePathDirImagerie()
{
    QString dirimagerie = QString();
    if (dirimagerie != QString())
        return dirimagerie;
    switch (db->ModeAccesDataBase()) {
    case Utils::Distant:
    {
        bool okcorrectionmsg = false;
        dirimagerie = settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_Imagerie).toString();
        if (dirimagerie == "")
        {
            okcorrectionmsg = true;
            dirimagerie = PATH_DIR_RUFUS NOM_DIR_IMAGERIE;
        }
        if (!QDir(dirimagerie).exists())
        {
            if (!Utils::mkpath(dirimagerie))
            {
                QString msg = tr("Le dossier de sauvegarde d'imagerie") + " <font color=\"red\"><b>" + dirimagerie + "</b></font>" + tr(" n'existe pas");
                msg += "<br />" + tr("Renseignez un dossier valide dans") + " <font color=\"green\"><b>" + tr("Editions/Paramètres/Onglet ") + tr("Réseau local") + "</b></font>";
                ShowMessage::I()->SplashMessage(msg, 6000);
                dirimagerie = "";
                okcorrectionmsg = false;
            }
            else
                okcorrectionmsg = true;
        }
        if (okcorrectionmsg)
        {
            QString msg = tr("Le dossier de sauvegarde d'imagerie enregistré dans les paramètres était invalide");
            msg += "<br />" + tr("Il a été remplacé par ") + "<font color=\"green\"><b>" + dirimagerie + "</b></font>" +
                    tr(" et enregistré dans ") + "<font color=\"red\"><b>" + tr("Editions/Paramètres/Onglet/Réseau local") + "</b></font>";
            ShowMessage::I()->SplashMessage(msg, 6000);
            settings()->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_Imagerie, dirimagerie);
        }
        break;
    }
    default:
        dirimagerie = db->dirimagerie();
    }
    return dirimagerie;
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
bool Procedures::PremierDemarrage(bool forceBaseVierge, bool demanderRestauration)
{
    if (forceBaseVierge)
    {
        //! Raccourci « -installMySQL » : on saute le choix vierge/existante et on impose la base
        //! vierge. Le serveur vient d'être (ré)installé neuf → il n'existe aucune base à proposer.
        protoc = BaseVierge;
    }
    else
    {
        UpMessageBox *msgbox = new UpMessageBox;

        UpSmallButton    AnnulBouton        (tr("Abandonner"));
        UpSmallButton    BaseViergeBouton (tr("Nouvelle base\npatients"));
        UpSmallButton    BaseExistanteBouton(tr("Base patients existante\nsur le serveur"));
        QString text     =  tr("Premier démarrage de Rufus!");
        QString inftxt   =  tr("Cette étape va vous permettre de configurer le logiciel en quelques secondes") + "\n\n" +
                            tr("Commencez par choisir la situation qui décrit le mieux votre installation de Rufus") + "\n\n" +
                            tr("1. J'installe Rufus sur ce poste en créant une nouvelle base patients") + "\n" +
                            tr("2. J'installe Rufus sur ce poste et Rufus se connectera à une base patients qui existe dèjà");
        msgbox->setText(text);
        msgbox->setInformativeText(inftxt);
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
    }



    if (m_settings != Q_NULLPTR)
        delete m_settings;
    m_settings    = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
    QString login (""), MDP("");
    if (protoc == BaseExistante)
    {
        if (VerifParamConnexion())
        {
            PremierParametrageMateriel();
            //! Pas de « connexion réussie » : rien n'a été testé, c'est la relance qui éprouvera ces paramètres.
            UpMessageBox::Watch(Q_NULLPTR, tr("Paramètres de connexion enregistrés"),
                                   tr("Les paramètres de connexion de ce poste sont enregistrés.") + "\n" +
                                   tr("Le programme va redémarrer pour se connecter à la base patients") + ".\n");
            //! Redémarrage automatique : on relance Rufus avant de quitter, pour que
            //! l'utilisateur n'ait rien à faire (la nouvelle instance repart sur la
            //! configuration fraîchement écrite). argsRelance() : sans -installMySQL (anti-boucle).
            Utils::Redemarrage();
        }
    }
    else if (protoc == BaseVierge)
    {
        //! Avant tout : garantir un serveur MySQL conforme aux exigences de Rufus sur
        //! ce poste (détection ; installation + paramétrage au besoin) et création des
        //! comptes adminrufus/adminrufusSSL avec un mot de passe aléatoire propre au
        //! cabinet, stocké dans rufus.ini (clé Param_MDPSQL). Annulation utilisateur ou
        //! échec → retour à l'écran de premier démarrage (Rufus ne continue pas).
        //! cf MySQLInstaller, docs/NOTES_INTEGRATION_MYSQLINSTALLER.md
        MySQLInstaller installeurMySQL;
        if (!installeurMySQL.run())
            //! En mode FORCÉ (raccourci -installMySQL), on NE recurse PAS : la msgbox étant sautée,
            //! une récursion sur échec n'offrirait aucune sortie. On rend la main (return false) → le
            //! flux normal de Connexion_A_La_Base présente le carrefour « Aucun serveur » (avec Quitter).
            return forceBaseVierge ? false : PremierDemarrage(false, demanderRestauration);

        //! nbp = TOUJOURS monoposte / serveur local. On reloge ici les effets de bord
        //! « Poste » qu'assurait dlg_paramconnexion (qu'on n'appelle plus pour nbp :
        //! l'installeur a déjà tout — login/mdp applicatifs + serveur local).
        QString BasePoste = Utils::getBaseFromMode(Utils::Poste);
        db->setModeacces(Utils::Poste);
        m_settings->setValue(BasePoste + Param_Active, "YES");
        m_settings->setValue(BasePoste + Param_Port, "3306");
        if (dirSQLExecutable() == "")
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur de connexion"),
                                tr("Impossible de trouver l'exécutable MySQL") + "\n" +
                                tr("Le programme ne pourra pas s'intialiser"));
            exit(0);
        }

        //! Ouverture de la connexion Qt à MySQL (compte adminrufus + mot de passe
        //! aléatoire du cabinet, via motDePasseSQL()). INDISPENSABLE : RestaureBase et
        //! CreerPremierUser passent par db->StandardSQL() ; sans cette connexion, ils
        //! échouent silencieusement (la base est créée par le binaire mysql, mais
        //! l'utilisateur applicatif n'est jamais inséré dans rufus.utilisateurs).
        db->initParametresConnexionSQL("localhost", 3306);
        //! Connexion en CASCADE (aléatoire .dbkey PUIS gaxt78iy) : l'aléatoire vient d'être
        //! posé par l'installeur ; s'il était refusé à la toute première connexion (cache
        //! d'auth caching_sha2 « froid »), gaxt78iy (2e mot de passe conservé) prend le relais.
        QString erreurConnexion = MySQLInstaller::connecterAvecCandidats(DB_RUFUS);
        if (!erreurConnexion.isEmpty())
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Erreur de connexion au serveur MySQL"),
                                tr("La connexion à MySQL a échoué après l'installation.") + "\n" + erreurConnexion);
            return forceBaseVierge ? false : PremierDemarrage(false, demanderRestauration);   //! cf. note ci-dessus (pas de récursion en mode forcé)
        }
        m_connexionbaseOK = true;

        //! login/mdp de l'utilisateur APPLICATIF Rufus, saisis dans l'installeur
        //! (les comptes MySQL adminrufus/adminrufusSSL, eux, ont été créés en tâche
        //! de fond avec un mot de passe aléatoire stocké dans ~/.rufus/.dbkey).
        login = installeurMySQL.loginRufus();
        MDP   = installeurMySQL.mdpRufus();

        //! Création de la base : sauvegarde retrouvée -> restauration, sinon toute la structure vierge.
        if (!InstallationRufus(demanderRestauration || installeurMySQL.baseRufusTrouvee()))
            return false;
        m_parametres = db->parametres();

        //! Inscription de l'utilisateur applicatif dans rufus.utilisateurs (mdp SHA1).
        m_connexionbaseOK = CreerPremierUser(login, MDP);

        //! Mot de passe de secours connu du seul praticien, puis suppression du root sans mot de passe
        //! hérité de l'initialisation de MySQL.
        MySQLInstaller::creerCompteDeSecours();
        PremierParametrageMateriel();                      //! élaboration de rufus.ini et des dossiers Rufus
        Datas::I()->sites->initListe();
        CalcLieuExercice();
        if (Datas::I()->sites->currentsite() == Q_NULLPTR)
            UpMessageBox::Watch(nullptr,tr("Pas d'adresse spécifiée"), tr("Vous n'avez précisé aucun lieu d'exercice!"));
        Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
        db->setVersion(m_version);
        UpMessageBox::Watch(nullptr, tr("Redémarrage nécessaire"),
                              tr("Le programme va redémarrer pour que les modifications de la base Rufus puissent être prises en compte.") + "\n\n" +
                              tr("IMPORTANT — un mot de passe de connexion à votre base de données a été créé") + ".\n" +
                              tr("Notez-le et conservez-le en lieu sûr (sur papier ou sur une clé USB)") + "\n" +
                              tr("il est nécessaire pour connecter un autre poste au cabinet, ou pour dépanner cet ordinateur.") + "\n\n" +
                              tr("Mot de passe :") + "\n\n" +
                              "<p align=\"center\"><b><span style=\"color:#c00000; font-size:14pt;\">" + MySQLInstaller::motDePasseSQL() + "</span></b></p>" + "\n\n" +
                              tr("Vous pourrez aussi l'enregistrer sur une clé USB à tout moment depuis Edition/Paramètres/Onglet « Ce poste »."));
        //! Redémarrage automatique APRÈS la dernière boîte (celle qui affiche le mot de
        //! passe) : on relance Rufus puis on quitte, l'utilisateur n'a rien à relancer
        //! lui-même. La nouvelle instance repart sur la base tout juste créée.
        //! argsRelance() : sans -installMySQL, sinon la nouvelle instance réinstallerait en boucle.
        Utils::Redemarrage();
    }
    return false;
}

/*!
 * \brief Procedures::InstallationRufus
 * Crée la base du poste : restauration d'une sauvegarde si on en retrouve une, sinon base vierge. Une base
 * restaurée étant déjà complète, ce cas relance Rufus au lieu de rendre la main.
 * \param demanderRestauration  à défaut, proposer d'en désigner une sur clé USB ou disque externe
 */
bool Procedures::InstallationRufus(bool demanderRestauration)
{
    const QString trouvee = DerniereSauvegardeInstallation();
    bool restaurer = false;

    if (!trouvee.isEmpty())
        restaurer = (UpMessageBox::Question(Q_NULLPTR, tr("Sauvegarde de votre base retrouvée"),
                        tr("Rufus a retrouvé la sauvegarde faite avant l'installation :") + "\n" + trouvee + "\n\n" +
                        tr("Voulez-vous restaurer cette base plutôt que d'en créer une vierge ?"))
                     == UpSmallButton::STARTBUTTON);
    else if (demanderRestauration)
        restaurer = (UpMessageBox::Question(Q_NULLPTR, tr("Restaurer une base existante ?"),
                        tr("Disposez-vous d'une sauvegarde de votre base patients, sur une clé USB "
                           "ou un disque externe, à restaurer sur ce poste ?"))
                     == UpSmallButton::STARTBUTTON);

    //! chemin vide : RestaureBase demande alors où se trouve la sauvegarde
    if (restaurer && RestaureBase(false, true, false, Q_NULLPTR, trouvee))
    {
        if (!trouvee.isEmpty())
            QDir(trouvee).removeRecursively();
        if (!QFile::exists(PATH_FILE_INI))
            PremierParametrageMateriel();
        Datas::I()->postesconnectes->SupprimeAllPostesConnectes();
        UpMessageBox::Watch(Q_NULLPTR, tr("Base restaurée"),
            tr("Votre base patients a été restaurée. Rufus va redémarrer."));
        Utils::Redemarrage();
    }
    return RestaureBase(true, true);
}

/*-----------------------------------------------------------------------------------------------------------------
-- Paramètrage de l'imprimante ---------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void Procedures::PremierParametrageMateriel()
{
    m_settings->setValue(Imprimante_TailleEnTete,"45");
    m_settings->setValue(Imprimante_TailleEnTeteALD,"63");
    m_settings->setValue(Imprimante_TaillePieddePage,"20");
    m_settings->setValue(Imprimante_TailleTopMarge,"3");
    m_settings->setValue(Imprimante_ApercuAvantImpression,"NO");
    m_settings->setValue(Param_Poste_Autoref,"-");
    m_settings->setValue(Param_Poste_Refracteur,"-");
    m_settings->setValue(Param_Poste_Fronto,"-");
    m_settings->setValue(Param_Poste_Tono,"-");
    m_settings->setValue(Param_Poste_PortAutoref,"-");
    m_settings->setValue(Param_Poste_PortRefracteur,"-");
    m_settings->setValue(Param_Poste_PortFronto,"-");
    m_settings->setValue(Param_Poste_PortTono,"-");
    m_settings->setValue(Utils::getBaseFromMode(Utils::ReseauLocal) + PrioritaireGestionDocs,NOimport);
    m_settings->setValue(Param_Poste_Version, m_version);

    // Création des dossiers
    //!    on server
    //!    -- dir defined by variable MySQL secure-file-priv (/Users/Shared on macOS/Linux, Users/Public on W10/11)
    //!                              /Imagerie
    //!                                     /Images                         <- dir where Rufus writes every pict renamed by Rufus with a dir for each date
    //!                                     /DossierEchangeImages           <- dir where each machine writes its picts and where Rufus can read them (with a subdir for each machine)
    //!                                         /one dir for each machine
    //!                                                  (p.e.
    //!                                                  /OCT NIDEK
    //!                                                  /RNM EIDON
    //!                                                  /CV ZEISS
    //!                                                  /...etc...)
    //!                                     /EchecsTransferts
    //!                                     /Factures
    //!                                     /FacturesSansLien
    //!                                     /Originaux
    //!                                         /Factures
    //!                                         /Images
    //!                                     /Video

    //!     on local post
    //!     -- QDir::homePath()/Documents/Rufus
    //!                                         /Imagerie
    //!                                             /Originaux
    //!                                                 /Factures
    //!                                                 /Images
    //!                                             /DossierEchangeRefraction
    //!                                                 /Refracteur
    //!                                                     /In
    //!                                                         /Autoref    <- Rufus     writes here the param file of AR section of phoropter where the phoropter reads it
    //!                                                         /Fronto     <- Rufus     writes here the param file of LM section of phoropter where the phoropter reads it
    //!                                                         /Tono       <- Rufus     writes here the param file of TO section of phoropter where the phoropter reads it
    //!                                                     /Out            <- Phoropter writes here its results file                          where Rufus reads it
    //!                                                 /Fronto             <- Lensmeter writes here its results file                          where Rufus reads it
    //!                                                 /Tono               <- Tonometer writes here its results file                          where Rufus reads it
    //!                                                 /Autoref            <- ARK       writes here its results file                          where Rufus reads it
    //!                                             /EchecTransferts
    //!                                         /Logs
    //!                                         Rufus.ini                   <- param file of Rufus on this post


    if (protoc == BaseVierge)
    {
        /*! Dossiers du serveur : ouverts à tous dès la création, sinon les autres postes n'y écrivent pas
         *  et mysql ne peut pas les traverser (cf. Utils::rendDossierAccessibleAuServeurSQL). */
        const QString       dirimagerie = db->dirimagerie();
        const QStringList   sousdossiers = { NOM_DIR_IMAGES,     NOM_DIR_DOSSIERECHANGEIMAGERIE,
                                             NOM_DIR_ECHECSTRANSFERTS, NOM_DIR_FACTURES,
                                             NOM_DIR_FACTURESSANSLIEN, NOM_DIR_ORIGINAUX,
                                             NOM_DIR_ORIGINAUX NOM_DIR_FACTURES,
                                             NOM_DIR_ORIGINAUX NOM_DIR_IMAGES, NOM_DIR_VIDEOS };
        for (const QString &sousdossier : sousdossiers)
        {
            Utils::mkpath(dirimagerie + sousdossier);
            Utils::rendDossierAccessibleAuServeurSQL(dirimagerie + sousdossier);
        }
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
//! Sauvegarde silencieuse de Rufus.ini dans le dossier caché ~/.rufus (PATH_FILE_INI_BACKUP),
//! appelée à chaque OUVERTURE RÉUSSIE de Rufus (après Connexion_A_La_Base) — et NON à la
//! fermeture : la fermeture ne vérifie pas la cohérence du fichier, alors qu'une ouverture
//! réussie la garantit (la connexion à la base vient d'aboutir). On ne risque donc jamais
//! d'écraser une sauvegarde saine par un Rufus.ini corrompu. Chaque poste sauvegarde le SIEN,
//! ce qui permet à un poste client de restaurer son propre Rufus.ini (pas celui du serveur).
//! Comme personne ne pense à sauvegarder ce fichier, Rufus s'en charge : ReparerIni() pourra le
//! restaurer si l'original est perdu. No-op si Rufus.ini n'existe pas (rien à sauvegarder).
void Procedures::SauvegardeIni()
{
    if (!QFile::exists(PATH_FILE_INI))
        return;
    QDir().mkpath(PATH_DIR_RUFUSKEY);             // ~/.rufus (créé au besoin)
    QFile::remove(PATH_FILE_INI_BACKUP);          // QFile::copy n'écrase pas une cible existante
    QFile::copy(PATH_FILE_INI, PATH_FILE_INI_BACKUP);
}

/*!
 * \brief Procedures::ReparerIni
 * Rufus.ini absent ou sans mode de connexion valide (§ II.1) : une seule fiche, qui ne fait que
 * construire le fichier — quitter, restaurer la sauvegarde, ou revoir les paramètres. Boucle jusqu'à
 * obtenir un fichier exploitable ; c'est Connexion_A_La_Base qui dira ensuite s'il ouvre la base.
 */
/*!
 * \brief Procedures::EprouverConnexionApresSaisie
 * Éprouve les paramètres tout juste saisis : mot de passe du cabinet, à défaut le générique.
 */
bool Procedures::EprouverConnexionApresSaisie()
{
    const QString base = Utils::getBaseFromMode(db->ModeAccesDataBase());
    db->initParametresConnexionSQL(m_settings->value(base + Param_Serveur).toString(),
                                   m_settings->value(base + Param_Port).toInt());

    //! Obtenu = mot de passe éprouvé ET enregistré dans .dbkey
    if (MySQLInstaller::RecupererMotDePasseMySQL(Q_NULLPTR, tr("Mot de passe de la base du cabinet"),
            tr("Indiquez le mot de passe de connexion à la base du cabinet.") + "\n" +
            tr("Il se récupère sur une clé USB depuis le poste qui héberge la base "
               "(menu Édition / Paramètres)."))
        == MySQLInstaller::IssueMdp::Obtenu)
        return true;

    if (db->connectToDataBase(DB_RUFUS, LOGIN_SQL, MDP_SQL).isEmpty())
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Connexion établie sans mot de passe personnel"),
            tr("La base du cabinet ne s'ouvre qu'avec le mot de passe générique de Rufus.") + "\n" +
            tr("Faites-la sécuriser depuis le poste qui l'héberge."));
        return true;
    }

    QString corps;
    switch (db->causeEchecConnexion())
    {
    case DataBase::ServeurInjoignable:
        corps = tr("Aucun serveur ne répond à cette adresse.") + "\n\n" +
                tr("Vérifiez l'adresse du serveur, qu'il est bien allumé et, en accès distant, "
                   "la redirection de ports de votre box.");
        break;
    case DataBase::ClesSSL:
        corps = tr("Le serveur du cabinet répond, mais il refuse la liaison chiffrée.") + "\n\n" +
                tr("Les clés SSL de ce poste sont invalides ou périmées : faites-vous en transmettre "
                   "de nouvelles depuis le poste serveur.");
        break;
    case DataBase::Identifiants:
        corps = tr("Le serveur du cabinet répond, mais il refuse ce mot de passe.") + "\n\n" +
                tr("L'adresse du serveur est donc correcte : c'est le mot de passe de connexion "
                   "qu'il faut récupérer sur le poste qui héberge la base.");
        break;
    default:
        corps = tr("Aucun mot de passe n'ouvre la base avec ces paramètres.") + "\n\n" +
                tr("Vérifiez le mot de passe de connexion, l'adresse du serveur et, en accès distant, "
                   "le dossier des clés SSL.");
    }
    UpMessageBox::Watch(Q_NULLPTR, tr("Connexion à la base impossible"), corps);
    return false;
}

void Procedures::ReparerIni()
{
    forever
    {
        //! Relu à chaque tour ; le delete du QSettings précédent écrit ses valeurs en attente.
        if (m_settings != Q_NULLPTR)
            delete m_settings;
        m_settings = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
        if (QFile::exists(PATH_FILE_INI) && iniContientModeValide(*m_settings))
            return;

        //! Libellés créés avant le message : celui du bouton y est injecté à la place de « %1 ».
        const bool sauvegardeOK   = sauvegardeIniValide();
        UpSmallButton *bAnnuler   = new UpSmallButton(QObject::tr("Abandonner et\nquitter Rufus"));
        UpSmallButton *bSaisir    = new UpSmallButton(QObject::tr("Restaurer le fichier Rufus.ini en saisissant\nde nouveau les paramètres de connexion"));
        UpSmallButton *bReseau    = new UpSmallButton(QObject::tr("Installation d'un poste Rufus\nsur un réseau"));
        UpSmallButton *bPremiere  = new UpSmallButton(QObject::tr("Première installation\nde Rufus"));
        UpSmallButton *bRestaurer = sauvegardeOK
                                  ? new UpSmallButton(QObject::tr("Restaurer la copie\nde sauvegarde"))
                                  : nullptr;
        bReseau  ->setImmediateToolTip(QObject::tr("Se connecter à une base patients Rufus existante sur ce réseau"));
        bPremiere->setImmediateToolTip(QObject::tr("Créer une base patients"));

        QString msgInfo = QObject::tr("Le fichier d'initialisation") + "\n" + PATH_FILE_INI "\n"
                        + QObject::tr("est absent, ou ne contient pas de renseignement valide "
                                      "permettant la connexion à la base de données.") + "\n\n"
                        + QObject::tr("Ce fichier est indispensable au bon fonctionnement de l'application.") + "\n\n"
                        + QObject::tr("Cette absence est normale si vous démarrez l'application pour la première fois sur ce poste.") + "\n"
                        + QObject::tr("Si c'est le cas, choisissez l'option \"%1\"") + "\n";
        msgInfo.replace("%1", bPremiere->text().replace('\n', ' '));
        if (sauvegardeOK)
            msgInfo += "\n" + QObject::tr("Une copie de sauvegarde valide de ce fichier existe sur ce "
                                          "poste : voulez-vous la restaurer ?") + "\n";

        UpMessageBox msgbox(Q_NULLPTR);
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.setText(QObject::tr("Fichier de configuration Rufus.ini absent ou corrompu"));
        msgbox.setInformativeText(msgInfo);
        msgbox.addButton(bAnnuler,  UpSmallButton::CANCELBUTTON);
        msgbox.addButton(bSaisir,   UpSmallButton::EDITBUTTON);
        msgbox.addButton(bReseau,   UpSmallButton::NOBUTTON);
        msgbox.addButton(bPremiere, UpSmallButton::NOBUTTON);
        if (bRestaurer)
            msgbox.addButton(bRestaurer, UpSmallButton::STARTBUTTON);
        msgbox.exec();

        if (bRestaurer && msgbox.clickedButton() == bRestaurer)
        {
            QFile::remove(PATH_FILE_INI);               //! QFile::copy n'écrase pas une cible existante
            QFile::copy(PATH_FILE_INI_BACKUP, PATH_FILE_INI);
            UpMessageBox::Watch(Q_NULLPTR, tr("Rufus.ini restauré"),
                                tr("La configuration de ce poste a été restaurée à partir de la sauvegarde.") + "\n"
                              + tr("Le lancement de Rufus se poursuit."));
            continue;
        }
        if (msgbox.clickedButton() == bPremiere)
        {
            PremierDemarrage(/*forceBaseVierge=*/true);
            continue;
        }
        //! Mêmes gestes pour les deux : resaisir ses paramètres, ou brancher ce poste sur un réseau
        if (msgbox.clickedButton() == bSaisir || msgbox.clickedButton() == bReseau)
        {
            //! La fiche ne teste rien : elle recueille la saisie, VerifParamConnexion écrit Rufus.ini.
            if (VerifParamConnexion())
            {
                //! Sans la langue, le sélecteur reviendrait au prochain lancement.
                m_settings->setValue(Param_Poste_Version, m_version);
                m_settings->sync();   //! connectToDataBase relit le fichier pour le dossier des clés SSL
                UpMessageBox::Watch(Q_NULLPTR, tr("Rufus.ini reconstruit"),
                                    tr("Les paramètres de connexion de ce poste sont enregistrés.") + "\n"
                                  + tr("Le lancement de Rufus se poursuit."));
                if (!EprouverConnexionApresSaisie())
                {
                    //! le delete écrirait les valeurs en attente et recréerait le fichier qu'on efface
                    delete m_settings;
                    m_settings = Q_NULLPTR;
                    QFile::remove(PATH_FILE_INI);
                }
            }
            continue;
        }
        exit(0);
    }
}

bool Procedures::CreerOuRestaurerBase(QString msg, QString msgInfo, bool proposerRestauration)
{
    UpSmallButton AnnulBouton           (tr("Abandonner et\nquitter Rufus"));
    UpSmallButton RestaureBaseBouton    (tr("Restaurer la base de données\nà partir d'une sauvegarde"));
    UpSmallButton CreerBaseBouton       (tr("Créer une base patients"));

    UpMessageBox *msgbox = new UpMessageBox;
    msgbox->setText(msg);
    msgbox->setInformativeText(msgInfo);
    msgbox->setIcon(UpMessageBox::Warning);
    msgbox->addButton(&CreerBaseBouton, UpSmallButton::NOBUTTON);
    if (proposerRestauration)
        msgbox->addButton(&RestaureBaseBouton, UpSmallButton::NOBUTTON);
    msgbox->addButton(&AnnulBouton,            UpSmallButton::CANCELBUTTON);
    msgbox->exec();

    if (msgbox->clickedButton() == &AnnulBouton)
        exit(0);
    if (msgbox->clickedButton() == &RestaureBaseBouton)
    {
        if (RestaureBase(false, false, true, Q_NULLPTR))
        {
            UpMessageBox::Watch(Q_NULLPTR, tr("Base restaurée"),
                                tr("La base de données a été restaurée. Rufus va redémarrer."));
            Utils::Redemarrage();
        }
        return false;
    }
    //! s'y connecter se joue dans la saisie des paramètres (ReparerIni), ici on la crée
    return PremierDemarrage(/*forceBaseVierge=*/true, proposerRestauration);
}

bool Procedures::ClesSSLPresentes() const
{
    const QString dir = m_settings->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL).toString();
    if (dir.isEmpty() || !QDir(dir).exists())
        return false;
    QDir d(dir);
    return d.exists("client-key.pem") && d.exists("client-cert.pem");
}

bool Procedures::ChoisirDossierClesSSL()
{
    const QString dir = QFileDialog::getExistingDirectory(Q_NULLPTR,
        tr("Indiquez le dossier des clés SSL (client-key.pem et client-cert.pem)"));
    if (!dir.isEmpty())
        m_settings  ->setValue(Utils::getBaseFromMode(Utils::Distant) + Dossier_ClesSSL, dir);
    return ClesSSLPresentes();
}

/*-----------------------------------------------------------------------------------------------------------------
    -- Vérifie et répare les paramètres de connexion  -----------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool Procedures::VerifParamConnexion()
{
    dlg_paramconnexion *Dlg_ParamConnex = new dlg_paramconnexion();
    Dlg_ParamConnex ->setWindowTitle(tr("Entrez votre identifiant et votre mot de passe d'utilisateur Rufus"));
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
            //! Le dossier des clés s'enregistre ici, avec le reste : la fiche n'écrit rien dans Rufus.ini.
            m_settings->setValue(Base + Dossier_ClesSSL,  Dlg_ParamConnex->ui->ClesSSLLineEdit->text());
            db->setModeacces(Utils::Distant);
        }
        m_settings->setValue(Base + Param_Active,    "YES");
        m_settings->setValue(Base + Param_Port, Dlg_ParamConnex->ui->PortcomboBox->currentText());

        //! Pas de m_connexionbaseOK ici : aucune connexion tentée. SortieAppli fermerait une session inexistante.
        if (dirSQLExecutable() == "")
        {
            UpMessageBox::Watch(nullptr, tr("Erreur de connexion"), tr("Impossible de trouver l'exécutable MySQL") + "\n" + tr("Le programme ne pourra pas s'intialiser"));
                exit(0);
        }
        delete Dlg_ParamConnex;
        return true;
    }
    delete Dlg_ParamConnex;   // delete déconnecte déjà tout : pas de disconnect() global (joker)
    return false;
}

void Procedures::Ouverture_Appareils_Refraction()
{

    QString nameARK   = m_settings->value(Param_Poste_Autoref).toString();
    QString nameLM    = m_settings->value(Param_Poste_Fronto).toString();
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();
    QString nameTO    = m_settings->value(Param_Poste_Tono).toString();
    Datas::I()->mesureautoref   ->settypemesure(Refraction::Autoref);
    Datas::I()->mesurefronto    ->settypemesure(Refraction::Fronto);
    Datas::I()->mesureacuite    ->settypemesure(Refraction::Acuite);
    Datas::I()->mesurefinal     ->settypemesure(Refraction::Prescription);

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
        m_isReseauFronto?
                    m_devicesLAN.setFlag(Fronto)
                  : m_devicesCOM.setFlag(Fronto);
    }
    if (m_isAutorefParametre)
    {
        bool m_isReseauAutoref      = (m_settings->value(Param_Poste_PortAutoref).toString() == DOSSIER_ECHANGE);
        m_isReseauAutoref?
                    m_devicesLAN.setFlag(Autoref)
                  : m_devicesCOM.setFlag(Autoref);
    }
    if (m_isRefracteurParametre)
    {
        bool m_isReseauRefracteur   = (m_settings->value(Param_Poste_PortRefracteur).toString() == DOSSIER_ECHANGE);
        m_isReseauRefracteur?
                    m_devicesLAN.setFlag(Refracteur)
                  : m_devicesCOM.setFlag(Refracteur);
    }
    if (m_isTonoParametre)
    {
        bool m_isReseauTono         = (m_settings->value(Param_Poste_PortTono).toString() == DOSSIER_ECHANGE);
        m_isReseauTono?
                    m_devicesLAN.setFlag(Tonometre)
                  : m_devicesCOM.setFlag(Tonometre);
    }
    if (mapPortsCOM() == QMap<QString,QString>() && m_devicesCOM > 0)
        UpMessageBox::Watch(Q_NULLPTR, tr("Erreur connexion série"),
                            tr("Des connexions série sont paramétrées pour certains appareils du poste de réfraction.\n"
                               "Malheureusement, aucune de ces connexions ne semble fonctionner."));
    Ouverture_Ports_Series();
    Ouverture_Fichiers_Echange();
    m_hasappareilrefractionconnecte = m_devicesCOM >0 || m_devicesLAN >0;
    if (m_hasappareilrefractionconnecte)
        connect(Nidek::I(), &GenericProtocol::newmesure, this, [=, this](GenericProtocol::TypeMesure typ) {InsertMesure(typ);});
}


/*! ------------------------------------------------------------------------------------------------------------------------------------------
GESTION DES FICHIERS ECHANGE XML DES APPAREILS DE REFRACTION ---------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Fichiers_Echange(QWidget *parent)
{
    bool usetimer = true;  /*! Il semble que la classe QSystemFileWatcher pose quelques problèmes.
                             * au démarrage du système le signal directorychanged ne marche pas bien sur Mac quand le fichier d'échange est sur une machine Linux ou Windows
                             * il faut redémarrer une session Rufus pour que ça se décide à marcher
                             * On peut utiliser un timer à la place. C'est nettement moins élégant mais ça marche très bien.
                             * Il suffit de mettre ce bool à true pour utiliser le timer
                             * Le code pour le QFileSystemWatcher a été conservé au cas où le problème serait résolu */
    QString msg = "";
    QString pathdirfronto ("");
    QString pathdirautoref ("");
    QString pathdirrefracteur ("");
    QString pathdirtono ("");

    auto lecturefichier = [] (QWidget *parent, TypeAppareil appareil, QString pathdirappareil, QStringList listfich)
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
            || (app != tr("le tonomètre")  // on peut faire plusieurs mesures de tonometrie sur la même consultation donc on ne vérifie pas dans ce cas si une mesure a déjà été faite pour cet acte
                &&
                UpMessageBox::Question(parent,
                                       tr("Une mesure vient d'être émise par ") + app + tr(" mais la date de l'acte actuellement affiché n'est pas celle d'aujourd'hui."),
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

    if (m_devicesLAN.testFlag(Autoref))
    {
        m_LANAutoref = true;
        Datas::I()->mesureautoref   ->settypemesure(Refraction::Autoref);
        pathdirautoref = settings()->value(Param_Poste_PortAutoref_DossierEchange).toString();
        if (QDir(pathdirautoref).exists())
            msg =  (msg != ""? "\n":"") + tr("Connexion") + " <font color=\"green\"><b>" + m_settings->value(Param_Poste_Autoref).toString() +"</b></font>" + " "
                  "<font color=\"red\"><b>OK</b></font>" + " "  + tr("sur") + "<br/>"
                  "<font color=\"blue\"><b>" + pathdirautoref + "</b></font>";
        else
            msg =   (msg != ""? "\n":"") + tr("Impossible de connecter") + " <font color=\"red\"><b>" + m_settings->value(Param_Poste_Autoref).toString() + "</b></font><br/>" + tr("sur") + "<br/>" +
                  "<font color=\"blue\"><b>" + pathdirautoref + "</b></font>";
        if (!usetimer && QDir(pathdirautoref).exists())
            m_filewatcherautoref.addPath(pathdirautoref);
    }
    else
    {
        m_LANAutoref = false;
        settings()->remove(Param_Poste_PortAutoref_DossierEchange);
        if (!usetimer)
            m_filewatcherautoref.removePath(pathdirautoref);

    }
    if (m_devicesLAN.testFlag(Fronto))
    {
        m_LANFronto = true;
        Datas::I()->mesurefronto   ->settypemesure(Refraction::Fronto);
        pathdirfronto = settings()->value(Param_Poste_PortFronto_DossierEchange).toString();
        if (QDir(pathdirfronto).exists())
            msg = (msg != ""? "\n":"") + tr("Connexion") + " <font color=\"green\"><b>" + m_settings->value(Param_Poste_Fronto).toString() +"</b></font>" + " "
                  "<font color=\"red\"><b>OK</b></font>" + " " + tr("sur") + "<br/>"
                  "<font color=\"blue\"><b>" + pathdirfronto + "</b></font>";
        else
            msg = (msg != ""? "\n":"") + tr("Impossible de connecter") + "  <font color=\"red\"><b>" + m_settings->value(Param_Poste_Fronto).toString() + "</b></font><br/>" + tr("sur") + "<br/>" +
                  "<font color=\"blue\"><b>" + pathdirfronto + "</b></font>";
        if (!usetimer && QDir(pathdirfronto).exists())
            m_filewatcherfronto.addPath(pathdirfronto);
    }
    else
    {
        m_LANFronto = false;
        settings()->remove(Param_Poste_PortFronto_DossierEchange);
        if (!usetimer)
            m_filewatcherfronto.removePath(pathdirfronto);

    }
    if (m_devicesLAN.testFlag(Refracteur))
    {
        m_LANRefracteur = true;
        Datas::I()->mesurefinal     ->settypemesure(Refraction::Prescription);
        Datas::I()->mesureacuite    ->settypemesure(Refraction::Acuite);
        pathdirrefracteur = settings()->value(Param_Poste_PortRefracteur_DossierEchange).toString();
        if (QDir(pathdirrefracteur).exists())
            msg = (msg != ""? "\n":"") + tr("Connexion") + " <font color=\"green\"><b>" + m_settings->value(Param_Poste_Refracteur).toString() +"</b></font>" + " "
                  "<font color=\"red\"><b>OK</b></font>" + " " + tr("sur") + "<br/>"
                  "<font color=\"blue\"><b>" + pathdirrefracteur + "</b></font>";
        else
            msg =  (msg != ""? "\n":"") + tr("Impossible de connecter") +  " <font color=\"red\"><b>" + m_settings->value(Param_Poste_Refracteur).toString() + "</b></font><br/>" + tr("sur") + "<br/>" +
                  "<font color=\"blue\"><b>" + pathdirrefracteur + "</b></font>";
        if (!usetimer && QDir(pathdirrefracteur).exists())
            m_filewatcherrefracteur.addPath(pathdirrefracteur);
    }
    else
    {
        m_LANRefracteur = false;
        settings()->remove(Param_Poste_PortRefracteur_DossierEchange);
        settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Autoref);
        settings()->remove(Param_Poste_PortRefracteur_DossierEchange_Fronto);
        if (!usetimer)
            m_filewatcherrefracteur.removePath(pathdirrefracteur);

    }
    if (m_devicesLAN.testFlag(Tonometre))
    {
        m_LANTono = true;
        pathdirtono = settings()->value(Param_Poste_PortTono_DossierEchange).toString();
        if (QDir(pathdirtono).exists())
            msg = (msg != ""? "\n":"") + tr("Connexion") + " <font color=\"green\"><b>" + m_settings->value(Param_Poste_Tono).toString() +"</b></font>" + " "
                  "<font color=\"red\"><b>OK</b></font>" + " " + tr("sur") + "<br/>"
                  "<font color=\"blue\"><b>" + pathdirtono + "</b></font>";
        else
            msg =  (msg != ""? "\n":"") + tr("Impossible de connecter") + " <font color=\"red\"><b>" + m_settings->value(Param_Poste_Tono).toString() + "</b></font><br/>" + tr("sur") + "<br/>" +
                  "<font color=\"blue\"><b>" + pathdirtono + "</b></font>";
        if (!usetimer && QDir(pathdirtono).exists())
            m_filewatcherrefracteur.addPath(pathdirtono);
    }
    else
    {
        m_LANTono = false;
        settings()->remove(Param_Poste_PortTono_DossierEchange);
        if (!usetimer)
            m_filewatchertono.removePath(pathdirtono);
    }
    if (msg != "")
        ShowMessage::I()->SplashMessage(msg, 6000);
    if (usetimer)
    {
        t_filewatchtimer.stop();
        disconnect(&t_filewatchtimer,  &QTimer::timeout, nullptr, nullptr);
        if (m_devicesLAN >0)
        {
            t_filewatchtimer.start(1000);
            connect(&t_filewatchtimer,  &QTimer::timeout,     this,
                    [=, this]
            {
                if (m_devicesLAN.testFlag(Autoref) && pathdirautoref != "")
                {
                    QStringList listfichxml = QDir(pathdirautoref).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                    if (listfichxml.size())
                        lecturefichier(parent, Autoref, pathdirautoref, listfichxml);
                }
                if (m_devicesLAN.testFlag(Fronto) && pathdirfronto != "")
                {
                    QStringList formats;
                    formats <<"*.xml";
                    formats <<"*.csv";
                    QStringList listfichxml = QDir(pathdirfronto).entryList(formats, QDir::Files | QDir::NoDotAndDotDot);
                    if (listfichxml.size())
                        lecturefichier(parent, Fronto, pathdirfronto, listfichxml);
                }
                if (m_devicesLAN.testFlag(Refracteur) && pathdirrefracteur != "")
                {
                    QStringList listfichxml = QDir(pathdirrefracteur).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                    if (listfichxml.size())
                        lecturefichier(parent, Refracteur, pathdirrefracteur, listfichxml);
                }
                if (m_devicesLAN.testFlag(Tonometre) && pathdirtono != "")
                {
                    QStringList listfich = QDir(pathdirtono).entryList(QStringList() <<"*.xml", QDir::Files | QDir::NoDotAndDotDot);
                    if (listfich.size())
                        lecturefichier(parent, Tonometre, pathdirtono, listfich);
                }
            });
        }
    }
    else
    {
        disconnect(&m_filewatcherautoref,       &QFileSystemWatcher::directoryChanged, nullptr, nullptr);
        disconnect(&m_filewatcherfronto,        &QFileSystemWatcher::directoryChanged, nullptr, nullptr);
        disconnect(&m_filewatcherrefracteur,    &QFileSystemWatcher::directoryChanged, nullptr, nullptr);
        disconnect(&m_filewatchertono,          &QFileSystemWatcher::directoryChanged, nullptr, nullptr);
        if (m_devicesLAN.testFlag(Autoref) && pathdirautoref != "")
            connect(&m_filewatcherautoref,      &QFileSystemWatcher::directoryChanged,  this,   [=, this]
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
                        lecturefichier(parent, Autoref, pathdirautoref, listfichxml);
                    }
                }
            });

        if (m_devicesLAN.testFlag(Fronto) && pathdirfronto != "")
            connect(&m_filewatcherfronto,       &QFileSystemWatcher::directoryChanged,  this,   [=, this]
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
                        lecturefichier(parent, Fronto, pathdirfronto, listfichxml);
                    }
                }
            });

        if (m_devicesLAN.testFlag(Refracteur) && pathdirrefracteur != "")
            connect(&m_filewatcherrefracteur,   &QFileSystemWatcher::directoryChanged,  this,   [=, this]
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
                        lecturefichier(parent, Refracteur, pathdirrefracteur, listfichxml);
                    }
                }
            });

        if (m_devicesLAN.testFlag(Tonometre) && pathdirtono != "")
            connect(&m_filewatcherfronto,       &QFileSystemWatcher::directoryChanged,  this,   [=, this]
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
                        lecturefichier(parent, Tonometre, pathdirfronto, listfichxml);
                    }
                }
            });
    }
    return true;
}

/*! ------------------------------------------------------------------------------------------------------------------------------------------
GESTION DES PORTS SERIES -------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------------------*/
bool Procedures::Ouverture_Ports_Series(QWidget *parent)
{
    m_portRefracteur            = "";
    m_portFronto                = "";
    m_portAutoref               = "";
    m_portTono                  = "";
    QString                     msg = "";

    /*!
        Ouvre le port dont le portName() est NomPort en rapport avec l'appareil passé en paramètre
    */
    auto loadserialsettings = [&] (TypeAppareil appareil)
    {
        QString PortCom = "";
        QString DeviceName = "";
        Utils::SerialSettings sparamportserie = s_paramPortSerieFronto;
        upSerialPort *serialport = Q_NULLPTR;
        switch (appareil) {
        case Fronto:        sparamportserie = s_paramPortSerieFronto;       PortCom = m_portFronto;     DeviceName = tr("Fronto");      break;
        case Autoref:       sparamportserie = s_paramPortSerieAutoref;      PortCom = m_portAutoref;    DeviceName = tr("Autoref");     break;
        case Refracteur:    sparamportserie = s_paramPortSerieRefracteur;   PortCom = m_portRefracteur; DeviceName = tr("Refracteur");  break;
        case Tonometre:     sparamportserie = s_paramPortSerieTono;         PortCom = m_portTono;       DeviceName = tr("Tono");        break;
        default: break;
        }
        auto it = m_mapports.find(PortCom);
        if (it != m_mapports.end())
        {
            serialport= new upSerialPort(it.value(), DeviceName);
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
    if (m_devicesCOM.testFlag(Fronto))
    {
        ReglePortFronto();
        m_portFronto = m_settings->value(Param_Poste_PortFronto).toString();
        if (m_portFronto == "")
            UpMessageBox::Watch(parent, tr("Erreur paramètrage connexion frontofocomètre"), tr("le port de comunication n'est pas configuré"));
        else
        {
            if (sp_portFronto != Q_NULLPTR)
            {
                sp_portFronto->close();
                delete sp_portFronto;
            }
            sp_portFronto = loadserialsettings(Fronto);
            if (sp_portFronto != Q_NULLPTR)
            {
                if (sp_portFronto->open(QIODevice::ReadWrite))
                {
                    connect(sp_portFronto,  &upSerialPort::newdatacom,     this, &Procedures::ReponsePortSerie_Fronto);
                    msg += (msg != ""? "<br/>" : "") + tr("Connexion frontocomètre") + " " + "<font color=\"red\"><b>OK</b></font>" +" " + tr("sur") + " " + m_portFronto;
                }
                else
                {
                    msg += (msg != ""? "<br/>" : "") + tr("Impossible de connecter le frontocomètre sur ") + m_portFronto;
                    UpMessageBox::Watch(parent, tr("Impossible de connecter le frontocomètre sur ") + m_portFronto, listeports);
                    delete sp_portFronto;
                    sp_portFronto = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(parent, tr("Impossible de connecter le frontocomètre sur ") + m_portFronto, listeports);
        }
    }
    else if (sp_portFronto != Q_NULLPTR)
    {
        delete sp_portFronto;
        sp_portFronto = Q_NULLPTR;
        m_settings->remove(Param_Poste_PortFronto_COM_baudrate);
        m_settings->remove(Param_Poste_PortFronto_COM_databits);
        m_settings->remove(Param_Poste_PortFronto_COM_parity);
        m_settings->remove(Param_Poste_PortFronto_COM_stopBits);
        m_settings->remove(Param_Poste_PortFronto_COM_flowControl);
    }

    // PORT REFRACTEUR
    if (m_devicesCOM.testFlag(Refracteur))
    {
        ReglePortRefracteur();
        m_portRefracteur = m_settings->value(Param_Poste_PortRefracteur).toString();
        if (m_portRefracteur == "")
            UpMessageBox::Watch(parent, tr("Erreur paramètrage connexion refracteur"), tr("le port de comunication n'est pas configuré"));
        else
        {
            if (sp_portRefracteur != Q_NULLPTR)
            {
                sp_portRefracteur->close();
                delete sp_portRefracteur;
            }
            sp_portRefracteur = loadserialsettings(Refracteur);
            if (sp_portRefracteur != Q_NULLPTR)
            {
                if (sp_portRefracteur->open(QIODevice::ReadWrite))
                {
                    connect(sp_portRefracteur,  &upSerialPort::newdatacom,     this, &Procedures::ReponsePortSerie_Refracteur);
                    msg += (msg != ""? "<br/>" : "") + tr("Connexion refracteur") + " " + "<font color=\"red\"><b>OK</b></font>" +" " + tr("sur") + " " + m_portRefracteur;
                }
                else
                {
                    msg += (msg != ""? "<br/>" : "") + tr("Impossible de connecter le refracteur sur ") + m_portRefracteur;
                    UpMessageBox::Watch(parent, tr("Impossible de connecter le refracteur sur ") + m_portRefracteur, listeports);
                    delete sp_portRefracteur;
                    sp_portRefracteur = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(parent, tr("Impossible de connecter le refracteur sur ") + m_portRefracteur, listeports);
        }
    }
    else if (sp_portRefracteur != Q_NULLPTR)
    {
        delete sp_portRefracteur;
        sp_portRefracteur = Q_NULLPTR;
        m_settings->remove(Param_Poste_PortRefracteur_COM_baudrate);
        m_settings->remove(Param_Poste_PortRefracteur_COM_databits);
        m_settings->remove(Param_Poste_PortRefracteur_COM_parity);
        m_settings->remove(Param_Poste_PortRefracteur_COM_stopBits);
        m_settings->remove(Param_Poste_PortRefracteur_COM_flowControl);
    }

    //PORT AUTOREF
    if (m_devicesCOM.testFlag(Autoref))
    {
        ReglePortAutoref();
        m_portAutoref = m_settings->value(Param_Poste_PortAutoref).toString();
        if (m_portAutoref == "")
            UpMessageBox::Watch(parent, tr("Erreur paramètrage connexion autorefractomètre"), tr("le port de comunication n'est pas configuré"));
        else
        {
            if (sp_portAutoref != Q_NULLPTR)
            {
                sp_portAutoref->close();
                delete sp_portAutoref;
            }
            sp_portAutoref = loadserialsettings(Autoref);
            if (sp_portAutoref != Q_NULLPTR)
            {
                if (sp_portAutoref->open(QIODevice::ReadWrite))
                {
                    connect(sp_portAutoref,  &upSerialPort::newdatacom,     this, &Procedures::ReponsePortSerie_Autoref);
                    msg += (msg != ""? "<br/>" : "") + tr("Connexion autorefractomètre") + " " + "<font color=\"red\"><b>OK</b></font>" +" " + tr("sur") + " " + m_portAutoref;
                }
                else
                {
                    msg += (msg != ""? "<br/>" : "") + tr("Impossible de connecter l'autorefractomètre sur ") + m_portAutoref;
                    UpMessageBox::Watch(parent, tr("Impossible de connecter l'autorefractomètre sur ") + m_portAutoref, listeports);
                    delete sp_portAutoref;
                    sp_portAutoref = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(parent, tr("Impossible de connecter l'autorefractomètre sur ") + m_portAutoref, listeports);
        }
    }
    else if (sp_portAutoref != Q_NULLPTR)
    {
        delete sp_portAutoref;
        sp_portAutoref = Q_NULLPTR;
        m_settings->remove(Param_Poste_PortAutoref_COM_baudrate);
        m_settings->remove(Param_Poste_PortAutoref_COM_databits);
        m_settings->remove(Param_Poste_PortAutoref_COM_parity);
        m_settings->remove(Param_Poste_PortAutoref_COM_stopBits);
        m_settings->remove(Param_Poste_PortAutoref_COM_flowControl);
    }

    if (m_devicesCOM.testFlag(Tonometre))
    {
        ReglePortTonometre();
        m_portTono = m_settings->value(Param_Poste_PortTono).toString();
        if (m_portTono == "")
            UpMessageBox::Watch(parent, tr("Erreur paramètrage connexion tonomomètre"), tr("le port de comunication n'est pas configuré"));
        else
        {
            if (sp_portTono != Q_NULLPTR)
            {
                sp_portTono->close();
                delete sp_portTono;
            }
            sp_portTono = loadserialsettings(Tonometre);
            if (sp_portTono != Q_NULLPTR)
            {
                if (sp_portTono->open(QIODevice::ReadWrite))
                {
                    connect(sp_portTono,  &upSerialPort::newdatacom,     this, &Procedures::ReponsePortSerie_Tono);
                    msg += (msg != ""? "<br/>" : "") + tr("Connexion tonomètre") + " " + "<font color=\"red\"><b>OK</b></font>" +" " + tr("sur") + " " +  m_portTono;
                }
                else
                {
                    msg += (msg != ""? "<br/>" : "") + tr("Impossible de connecter le tonomètre sur ") + m_portTono;
                    UpMessageBox::Watch(parent, tr("Impossible de connecter le tonomètre sur ") + m_portTono, listeports);
                    delete sp_portTono;
                    sp_portTono = Q_NULLPTR;
                }
            }
            else
                UpMessageBox::Watch(parent, tr("Impossible de connecter le tonomètre sur ") + m_portTono, listeports);
        }
    }
    else if (sp_portTono != Q_NULLPTR)
    {
        delete sp_portTono;
        sp_portTono = Q_NULLPTR;
        m_settings->remove(Param_Poste_PortTono_COM_baudrate);
        m_settings->remove(Param_Poste_PortTono_COM_databits);
        m_settings->remove(Param_Poste_PortTono_COM_parity);
        m_settings->remove(Param_Poste_PortTono_COM_stopBits);
        m_settings->remove(Param_Poste_PortTono_COM_flowControl);
    }

    if (msg != "")
        ShowMessage::I()->SplashMessage(msg, 6000);
    return false;
}

QMap<QString, QString>  Procedures::DefaultSerialSettings(QString nameDevice)
{
    QMap<QString, QString> map = QMap<QString, QString>();
    QString baud        = QString();
    QString databits    = QString();
    QString parity      = QString();
    QString stopbits    = QString();
    QString flowcontrol = QString();

    if (nameDevice =="NIDEK ARK-1A"
            || nameDevice =="NIDEK ARK-1"
            || nameDevice =="NIDEK ARK-1S"
            || nameDevice =="NIDEK AR-1A"
            || nameDevice =="NIDEK AR-1"
            || nameDevice =="NIDEK AR-1S"
            || nameDevice =="NIDEK ARK-530A"
            || nameDevice =="NIDEK ARK-510A"
            || nameDevice =="NIDEK ARK-30"
            || nameDevice =="NIDEK AR-20"
            || nameDevice =="NIDEK TONOREF III")
    {
        baud            = "Baud9600";
        databits        = "Data8";
        parity          = "EvenParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }
    else if (nameDevice =="NIDEK HandyRef-K")
    {
        baud            = "Baud9600";
        databits        = "Data8";
        parity          = "OddParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }
    else if (nameDevice =="TOMEY RC-5000"
             || nameDevice =="RODENSTOCK CX 2000")
    {
        baud            = "Baud38400";
        databits        = "Data8";
        parity          = "NoParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }
    else if (nameDevice =="TOMEY TL-3000C")
    {
        baud            = "Baud2400";
        databits        = "Data7";
        parity          = "EvenParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }
    else if (nameDevice =="VISIONIX VL1000"
          || nameDevice =="HUVITZ CLM7000")
    {
        baud            = "Baud9600";
        databits        = "Data7";
        parity          = "EvenParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }
    else if (nameDevice =="NIDEK LM-1800P"
          || nameDevice =="NIDEK LM-1800PD"
          || nameDevice =="NIDEK LM-500")
    {
        baud            = "Baud9600";
        databits        = "Data8";
        parity          = "EvenParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }
    else if (nameDevice =="NIDEK RT-5100" || nameDevice =="NIDEK RT-2100")
    {
        baud            = "Baud2400";
        databits        = "Data7";
        parity          = "EvenParity";
        stopbits        = "TwoStop";
        flowcontrol     = "NoFlowControl";
    }
    else if (nameDevice =="TOMEY TAP-2000" || nameDevice =="RODENSTOCK Phoromat 2000")
    {
        baud            = "Baud9600";
        databits        = "Data8";
        parity          = "NoParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }
    else
    {
        qintptr z = 0;
        ShowMessage::I()->PriorityMessage("<font color=\"red\"><b>" + QObject::tr("Réglages par défaut inconnus pour l'appareil") + " " + nameDevice + "</b></font><br/>" +
                                          QObject::tr("Utilisation de réglages génériques") + "<br/>" +
                                          QObject::tr("Il faudra probablement revoir les réglages") + "<br/>" +
                                          QObject::tr("en fonction des caractéristiques du") + " " + nameDevice, z, 6000);
        baud            = "Baud9600";
        databits        = "Data8";
        parity          = "EvenParity";
        stopbits        = "OneStop";
        flowcontrol     = "NoFlowControl";
    }

    map[BAUDRATE]   = baud;
    map[DATABITS]   = databits;
    map[PARITY]     = parity;
    map[STOPBITS]   = stopbits;
    map[FLOWCONTROL]= flowcontrol;
    return map;
}


/*!
 * \brief Procedures::RegleSerialSettings
 * \param appareil
 * \param map les datas à régler
 * char * = le name() du QMetaEnum de data à régler
 * int = l'index de la valeur ddans le QMetaEnum
 */
void Procedures::RegleSerialSettings(TypeAppareil appareil, QMap<QString, QString> map)
{
    QString port(""), baudrate(""),databits(""),parity(""),stopbits(""),flowcontrol("");
    Utils::SerialSettings serialset;
    ResetSerialSettings(serialset);
    switch (appareil) {
    case Fronto :
        port        = Param_Poste_PortFronto;
        baudrate    = Param_Poste_PortFronto_COM_baudrate;
        databits    = Param_Poste_PortFronto_COM_databits;
        parity      = Param_Poste_PortFronto_COM_parity;
        stopbits    = Param_Poste_PortFronto_COM_stopBits;
        flowcontrol = Param_Poste_PortFronto_COM_flowControl;
        m_devicesCOM.setFlag(Fronto, true);
        m_devicesLAN.setFlag(Fronto, false);
        break;
    case Autoref :
        port        = Param_Poste_PortAutoref;
        baudrate    = Param_Poste_PortAutoref_COM_baudrate;
        databits    = Param_Poste_PortAutoref_COM_databits;
        parity      = Param_Poste_PortAutoref_COM_parity;
        stopbits    = Param_Poste_PortAutoref_COM_stopBits;
        flowcontrol = Param_Poste_PortAutoref_COM_flowControl;
        m_devicesCOM.setFlag(Autoref, true);
        m_devicesLAN.setFlag(Autoref, false);
        break;
    case Refracteur :
        port        = Param_Poste_PortRefracteur;
        baudrate    = Param_Poste_PortRefracteur_COM_baudrate;
        databits    = Param_Poste_PortRefracteur_COM_databits;
        parity      = Param_Poste_PortRefracteur_COM_parity;
        stopbits    = Param_Poste_PortRefracteur_COM_stopBits;
        flowcontrol = Param_Poste_PortRefracteur_COM_flowControl;
        m_devicesCOM.setFlag(Refracteur, true);
        m_devicesLAN.setFlag(Refracteur, false);
        break;
    case Tonometre :
        port        = Param_Poste_PortTono;
        baudrate    = Param_Poste_PortTono_COM_baudrate;
        databits    = Param_Poste_PortTono_COM_databits;
        parity      = Param_Poste_PortTono_COM_parity;
        stopbits    = Param_Poste_PortTono_COM_stopBits;
        flowcontrol = Param_Poste_PortTono_COM_flowControl;
        m_devicesCOM.setFlag(Tonometre, true);
        m_devicesLAN.setFlag(Tonometre, false);
        break;
    default:
        return;
    }

    m_settings  ->setValue(port         , map[PORT]);
    m_settings  ->setValue(baudrate     , map[BAUDRATE]);
    m_settings  ->setValue(databits     , map[DATABITS]);
    m_settings  ->setValue(parity       , map[PARITY]);
    m_settings  ->setValue(stopbits     , map[STOPBITS]);
    m_settings  ->setValue(flowcontrol  , map[FLOWCONTROL]);
}

void Procedures::setSerialPortValueFromQSettings(Utils::SerialSettings &comset, QString prop, QString idxstring, TypeAppareil typ)  /*! set SerailPort value from index in QserialPort metaobject metaenum */
{
    QMetaEnum metaEnum = Utils::enumeratorSP(prop);
    bool isinvalidkey = true;
    switch (typ) {
    case Fronto:
        isinvalidkey = m_issettingFrontoFromInvalidKey;
        break;
    case Autoref:
        isinvalidkey = m_issettingAutorefFromInvalidKey;
        break;
    case Refracteur:
        isinvalidkey = m_issettingRefracteurFromInvalidKey;
        break;
    case Tonometre:
        isinvalidkey = m_issettingTonoFromInvalidKey;
        break;
    default:
        break;
    }
    if (prop == BAUDRATE)
    {
        if (isinvalidkey)       //! les infos du QSerialPort étaient auparavant stockées par leur index dans l'enum, d'où cette manipulation pour les remettre en clair
        {
            comset.baudRate = (QSerialPort::BaudRate)metaEnum.value(idxstring.toInt());
            setSerialPortSettingsValueFromIndex(prop, idxstring, typ);
        }
        else
            comset.baudRate = (QSerialPort::BaudRate)metaEnum.keyToValue(idxstring.toLocal8Bit().data());
    }
    else if (prop == DATABITS)
    {
        if (isinvalidkey)      //! les infos du QSerialPort étaient auparavant stockées par leur index dans l'enum, d'où cette manipulation pour les remettre en clair
        {
            comset.dataBits = (QSerialPort::DataBits)metaEnum.value(idxstring.toInt());
            setSerialPortSettingsValueFromIndex(prop, idxstring, typ);
        }
        else
            comset.dataBits = (QSerialPort::DataBits)metaEnum.keyToValue(idxstring.toLocal8Bit().data());
    }
    else if (prop == PARITY)
    {
        if (isinvalidkey)      //! les infos du QSerialPort étaient auparavant stockées par leur index dans l'enum, d'où cette manipulation pour les remettre en clair
        {
            comset.parity = (QSerialPort::Parity)metaEnum.value(idxstring.toInt());
            setSerialPortSettingsValueFromIndex(prop, idxstring, typ);
        }
        else
            comset.parity = (QSerialPort::Parity)metaEnum.keyToValue(idxstring.toLocal8Bit().data());
    }
    else if (prop == STOPBITS)
    {
        if (isinvalidkey)      //! les infos du QSerialPort étaient auparavant stockées par leur index dans l'enum, d'où cette manipulation pour les remettre en clair
        {
            comset.stopBits = (QSerialPort::StopBits)metaEnum.value(idxstring.toInt());
            setSerialPortSettingsValueFromIndex(prop, idxstring, typ);
        }
        else
            comset.stopBits = (QSerialPort::StopBits)metaEnum.keyToValue(idxstring.toLocal8Bit().data());
    }
    else if (prop == FLOWCONTROL)

    {
        if (isinvalidkey)      //! les infos du QSerialPort étaient auparavant stockées par leur index dans l'enum, d'où cette manipulation pour les remettre en clair
        {
            comset.flowControl = (QSerialPort::FlowControl)metaEnum.value(idxstring.toInt());
            setSerialPortSettingsValueFromIndex(prop, idxstring, typ);
        }
        else
            comset.flowControl = (QSerialPort::FlowControl)metaEnum.keyToValue(idxstring.toLocal8Bit().data());
    }
}

bool Procedures::setSerialPortSettingsValueFromIndex(QString prop, QString &idxstring, TypeAppareil typ)                   /*! set SerialPort value in QSettings from index in QserialPort metaobject metaenum */
{
    QString value = Utils::stringKeyFromEnumIndexSP(prop, idxstring.toInt());
    auto setValue = [=, this](QString setkey, QString val){
        if (val == QString())
            m_settings->remove(setkey);
        else
            m_settings->setValue(setkey, val);
    };
    if (prop == BAUDRATE)
    {
        if (typ == Fronto)
            setValue(Param_Poste_PortFronto_COM_baudrate, value);
        else if (typ == Tonometre)
            setValue(Param_Poste_PortTono_COM_baudrate, value);
        else if (typ == Autoref)
            setValue(Param_Poste_PortAutoref_COM_baudrate, value);
        else if (typ == Refracteur)
            setValue(Param_Poste_PortRefracteur_COM_baudrate, value);
    }
    else if (prop == DATABITS)
    {
        if (typ == Fronto)
            setValue(Param_Poste_PortFronto_COM_databits, value);
        else if (typ == Tonometre)
            setValue(Param_Poste_PortTono_COM_databits, value);
        else if (typ == Autoref)
            setValue(Param_Poste_PortAutoref_COM_databits, value);
        else if (typ == Refracteur)
            setValue(Param_Poste_PortRefracteur_COM_databits, value);
    }
    else if (prop == PARITY)
    {
        if (typ == Fronto)
            setValue(Param_Poste_PortFronto_COM_parity, value);
        else if (typ == Tonometre)
            setValue(Param_Poste_PortTono_COM_parity, value);
        else if (typ == Autoref)
            setValue(Param_Poste_PortAutoref_COM_parity, value);
        else if (typ == Refracteur)
            setValue(Param_Poste_PortRefracteur_COM_parity, value);
    }
    else if (prop == STOPBITS)
    {
        if (typ == Fronto)
            setValue(Param_Poste_PortFronto_COM_stopBits, value);
        else if (typ == Tonometre)
            setValue(Param_Poste_PortTono_COM_stopBits, value);
        else if (typ == Autoref)
            setValue(Param_Poste_PortAutoref_COM_stopBits, value);
        else if (typ == Refracteur)
            setValue(Param_Poste_PortRefracteur_COM_stopBits, value);
   }
    else if (prop == FLOWCONTROL)
    {
        if (typ == Fronto)
            setValue(Param_Poste_PortFronto_COM_flowControl, value);
        else if (typ == Tonometre)
            setValue(Param_Poste_PortTono_COM_flowControl, value);
        else if (typ == Autoref)
            setValue(Param_Poste_PortAutoref_COM_flowControl, value);
        else if (typ == Refracteur)
            setValue(Param_Poste_PortRefracteur_COM_flowControl, value);
    }
    return true;
}

void Procedures::ReglePortRefracteur()
{
    QString val("-1");
    QString nameRF    = m_settings->value(Param_Poste_Refracteur).toString();

    ResetSerialSettings(s_paramPortSerieRefracteur);
    if (m_settings->value(Param_Poste_PortRefracteur_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_baudrate).toString();
        val.toInt(&m_issettingRefracteurFromInvalidKey);
        setSerialPortValueFromQSettings(s_paramPortSerieRefracteur, BAUDRATE, val, Refracteur);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_databits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieRefracteur, DATABITS, val, Refracteur);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_parity).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieRefracteur, PARITY, val, Refracteur);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_stopBits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieRefracteur, STOPBITS, val, Refracteur);
    }
    if (m_settings->value(Param_Poste_PortRefracteur_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortRefracteur_COM_flowControl).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieRefracteur, FLOWCONTROL, val, Refracteur);
    }
    if (!isSerialSettingsValid(s_paramPortSerieRefracteur))
    {
        QMap<QString, QString> map = DefaultSerialSettings(nameRF);
        QMetaEnum metaEnum = Utils::enumeratorSP(BAUDRATE);
        s_paramPortSerieRefracteur.baudRate       = (QSerialPort::BaudRate)metaEnum.keyToValue(map[BAUDRATE].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(DATABITS);
        s_paramPortSerieRefracteur.dataBits       = (QSerialPort::DataBits)metaEnum.keyToValue(map[DATABITS].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(STOPBITS);
        s_paramPortSerieRefracteur.stopBits       = (QSerialPort::StopBits)metaEnum.keyToValue(map[STOPBITS].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(PARITY);
        s_paramPortSerieRefracteur.parity         = (QSerialPort::Parity)metaEnum.keyToValue(map[PARITY].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(FLOWCONTROL);
        s_paramPortSerieRefracteur.flowControl    = (QSerialPort::FlowControl)metaEnum.keyToValue(map[FLOWCONTROL].toLocal8Bit().data());

        m_settings                      ->setValue(Param_Poste_PortRefracteur_COM_baudrate,    map[BAUDRATE]);
        m_settings                      ->setValue(Param_Poste_PortRefracteur_COM_databits,    map[DATABITS]);
        m_settings                      ->setValue(Param_Poste_PortRefracteur_COM_stopBits,    map[STOPBITS]);
        m_settings                      ->setValue(Param_Poste_PortRefracteur_COM_parity,      map[PARITY]);
        m_settings                      ->setValue(Param_Poste_PortRefracteur_COM_flowControl, map[FLOWCONTROL]);
    }
}

upSerialPort* Procedures::PortRefracteur()
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
        if (Nidek::I()->isOKtoReceive(m_mesureSerie, "CRL"))
        {
            //Logs::LogToFile("PortSerieRefracteur", "SDN = " + m_mesureSerie + " - " + QDateTime().toString("dd-MM-yyyy HH:mm:ss"));
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
    if (sp_portRefracteur!=Q_NULLPTR) /*! par le port COM */
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
            PortRefracteur()->writeDatas(Nidek::I()->RequestToSend(), " RequestToSendNIDEK() - Refracteur = ");
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
            PortRefracteur()->writeDatas(DTRbuff, " DTRbuff - Refracteur = ");
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
        PortRefracteur()->writeDatas(DTRbuff, " DTRbuff - Refracteur = ");    }
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
        Topcon::I()->RegleRefracteurXML(flag);
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

void Procedures::saveDocumentToFile(DocExterne *doc, QWidget *parent)
{
    if (doc == Q_NULLPTR)
        return;
    Patient *pat = Datas::I()->patients->getById(doc->idpatient());
    if (pat == Q_NULLPTR)
    {
        UpMessageBox::Watch(parent,  tr("Echec"), tr("Impossible de retrouver les données du patient pour ce document"));
        return;
    }
    bool a = false;
    QString name = "";
    if (doc->isVideo())
    {
        Utils::ModeAcces modeacces = DataBase::I()->ModeAccesDataBase();
        QString filename = settings()->value(Utils::getBaseFromMode(modeacces) + Dossier_Videos).toString() + "/" + doc->lienversfichier();
        QFile   qFile(filename);
        if (modeacces == Utils::Distant)
            if (settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_Videos).toString() == "" || !qFile.exists())
            {
                UpMessageBox::Watch(parent, tr("Echec"), tr("Video non accessible en accès distant"));
                return;
            }
        if (!qFile.open(QIODevice::ReadOnly))
        {
            QString msg = tr("Erreur d'accès au fichier:") + " " + filename;
            UpMessageBox::Watch(parent, msg);
            return;
        }
        QFileDialog dialog(parent, tr("Enregistrer un fichier"), QDir::homePath());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setViewMode(QFileDialog::List);
        if (dialog.exec() == QDialog::Accepted)
        {
            name = pat->nomcomplet() + " - " + doc->date().toString("d-MMM-yyyy") + " - " + doc->titrelong() +
                   "." + QFileInfo(filename).suffix().toLower();
            name = dialog.directory().path() + "/" + name;
            a = QFile(filename).copy(name);
        }
    }
    else if (doc->isImage())
    {
        if (doc->imageblob() == QByteArray())
            CalcImageDocument(doc);
        if (doc->imageblob() == QByteArray())
        {
            UpMessageBox::Watch(parent, tr("Echec"), tr("Impossible de charger le document"));
            return;
        }
        QFileDialog dialog(parent, tr("Enregistrer un fichier"), QDir::homePath());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setViewMode(QFileDialog::List);
        if (dialog.exec() == QDialog::Accepted)
        {
            name = pat->nomcomplet() + " - " + doc->date().toString("d-MMM-yyyy") + " - " + doc->titrelong() +
                    "." + doc->imageformat();
            name = dialog.directory().path() + "/" + name;
            a = Utils::writeBinaryFile(doc->imageblob(), name);
        }
    }
    if (a)
        UpMessageBox::Show(parent, tr("Document enregistré dans"), name);
}

QString Procedures::dirnamepdf() const
{
    return m_dirnamepdf;
}

void Procedures::setDirnamepdf(const QString &newDirnamepdf)
{
    m_dirnamepdf = newDirnamepdf;
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
        if (Mesure.contains("*PD"))                 //!=> il y a une mesure d'écart
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
    else if (nameRF =="SHIN-NIPPON DR-900")
    {
        GenericProtocol::TypesMesures flag = GenericProtocol::MesureNone;
        if (Mesure.contains("LM") && PortFronto() == Q_NULLPTR && !m_LANFronto)             //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur
            flag.setFlag(GenericProtocol::MesureFronto);
        if (Mesure.contains("KM") && PortAutoref() == Q_NULLPTR && !m_LANAutoref)           //!=> il y a une mesure de keratométrie et l'autoref est connecté directement à la box du refraacteur
            flag.setFlag(GenericProtocol::MesureKerato);
        if (Mesure.contains("RM") && PortAutoref() == Q_NULLPTR && !m_LANAutoref)           //!=> il y a une mesure de refractometrie et l'autoref est directement branché sur la box du refracteur
            flag.setFlag(GenericProtocol::MesureAutoref);
        if (Mesure.contains("RT"))                                                          //!=> il y a une mesure de refraction
            flag.setFlag(GenericProtocol::MesureRefracteur);
        ShinNippon::I()->LectureDonneesCOMRefracteur(Mesure, flag);
    }
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


void Procedures::ReglePortTonometre()
{
    QString val("-1");

    ResetSerialSettings(s_paramPortSerieTono);
    if (m_settings->value(Param_Poste_PortTono_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_baudrate).toString();
        val.toInt(&m_issettingTonoFromInvalidKey);
        setSerialPortValueFromQSettings(s_paramPortSerieTono, BAUDRATE, val, Tonometre);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_databits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieTono, DATABITS, val, Tonometre);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_parity).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieTono, PARITY, val, Tonometre);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_stopBits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieTono, STOPBITS, val, Tonometre);
    }
    if (m_settings->value(Param_Poste_PortTono_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortTono_COM_flowControl).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieTono, FLOWCONTROL, val, Tonometre);
    }
    if (!isSerialSettingsValid(s_paramPortSerieTono))
    {
        QMetaEnum baudmetaEnum          = Utils::enumeratorSP(BAUDRATE);
        QMetaEnum databitsmetaEnum      = Utils::enumeratorSP(DATABITS);
        QMetaEnum stopbitsmetaEnum      = Utils::enumeratorSP(STOPBITS);
        QMetaEnum paritymetaEnum        = Utils::enumeratorSP(PARITY);
        QMetaEnum flowcontrolmetaEnum   = Utils::enumeratorSP(FLOWCONTROL);
        QString baudval                 = QString(baudmetaEnum.valueToKey(s_paramPortSerieTono.baudRate)).toLocal8Bit();
        QString databitsval             = QString(databitsmetaEnum.valueToKey(s_paramPortSerieTono.dataBits)).toLocal8Bit();
        QString stopbitsval             = QString(stopbitsmetaEnum.valueToKey(s_paramPortSerieTono.stopBits)).toLocal8Bit();
        QString parityval               = QString(paritymetaEnum.valueToKey(s_paramPortSerieTono.parity)).toLocal8Bit();
        QString flowcontrolval          = QString(flowcontrolmetaEnum.valueToKey(s_paramPortSerieTono.flowControl)).toLocal8Bit();
        m_settings  ->setValue(Param_Poste_PortTono_COM_baudrate,    baudval);
        m_settings  ->setValue(Param_Poste_PortTono_COM_databits,    databitsval);
        m_settings  ->setValue(Param_Poste_PortTono_COM_stopBits,    stopbitsval);
        m_settings  ->setValue(Param_Poste_PortTono_COM_parity,      parityval);
        m_settings  ->setValue(Param_Poste_PortTono_COM_flowControl, flowcontrolval);
   }
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
        mAVLOG          = QLocale().toString(acuite->avlOG().toDouble()*10) + "/10";
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

void Procedures::ReglePortFronto()
{
    QString val("-1");
    QString nameLM = m_settings->value(Param_Poste_Fronto).toString();

    ResetSerialSettings(s_paramPortSerieFronto);
    if (m_settings->value(Param_Poste_PortFronto_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_baudrate).toString();
        val.toInt(&m_issettingFrontoFromInvalidKey);
        setSerialPortValueFromQSettings(s_paramPortSerieFronto, BAUDRATE, val, Fronto);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_databits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieFronto, DATABITS, val, Fronto);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_parity).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieFronto, PARITY, val, Fronto);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_stopBits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieFronto, STOPBITS, val, Fronto);
    }
    if (m_settings->value(Param_Poste_PortFronto_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortFronto_COM_flowControl).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieFronto, FLOWCONTROL, val, Fronto);
    }
    if (!isSerialSettingsValid(s_paramPortSerieFronto))
    {
        QMap<QString, QString> map = DefaultSerialSettings(nameLM);
        QMetaEnum metaEnum = Utils::enumeratorSP(BAUDRATE);
        s_paramPortSerieFronto.baudRate       = (QSerialPort::BaudRate)metaEnum.keyToValue(map[BAUDRATE].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(DATABITS);
        s_paramPortSerieFronto.dataBits       = (QSerialPort::DataBits)metaEnum.keyToValue(map[DATABITS].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(STOPBITS);
        s_paramPortSerieFronto.stopBits       = (QSerialPort::StopBits)metaEnum.keyToValue(map[STOPBITS].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(PARITY);
        s_paramPortSerieFronto.parity         = (QSerialPort::Parity)metaEnum.keyToValue(map[PARITY].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(FLOWCONTROL);
        s_paramPortSerieFronto.flowControl    = (QSerialPort::FlowControl)metaEnum.keyToValue(map[FLOWCONTROL].toLocal8Bit().data());

        m_settings                      ->setValue(Param_Poste_PortFronto_COM_baudrate,    map[BAUDRATE]);
        m_settings                      ->setValue(Param_Poste_PortFronto_COM_databits,    map[DATABITS]);
        m_settings                      ->setValue(Param_Poste_PortFronto_COM_stopBits,    map[STOPBITS]);
        m_settings                      ->setValue(Param_Poste_PortFronto_COM_parity,      map[PARITY]);
        m_settings                      ->setValue(Param_Poste_PortFronto_COM_flowControl, map[FLOWCONTROL]);
    }
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

upSerialPort *Procedures::PortFronto()
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
        if (Nidek::I()->isRequestToSend(m_mesureSerie))          //! le fronto demande la permission d'envoyer des données
        {
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            PortFronto()->writeDatas(Nidek::I()->OKtoReceive("CLM"), " SendDataNIDEK(CLM) - Fronto = ");
            return;
        }
    }
    Datas::I()->mesurefronto->cleandatas();
    LectureDonneesCOMFronto(m_mesureSerie);
    if (Datas::I()->mesurefronto->isdataclean())
        return;

    //! Enregistre la mesure dans la base et met à jour les fiches
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

void Procedures::ReglePortAutoref()
{
    QString val("-1");
    QString nameARK = m_settings->value(Param_Poste_Autoref).toString();
    ResetSerialSettings(s_paramPortSerieAutoref);
    if (m_settings->value(Param_Poste_PortAutoref_COM_baudrate) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_baudrate).toString();
        val.toInt(&m_issettingAutorefFromInvalidKey);
        setSerialPortValueFromQSettings(s_paramPortSerieAutoref, BAUDRATE, val, Autoref);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_databits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_databits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieAutoref, DATABITS, val, Autoref);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_parity) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_parity).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieAutoref, PARITY, val, Autoref);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_stopBits) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_stopBits).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieAutoref, STOPBITS, val, Autoref);
    }
    if (m_settings->value(Param_Poste_PortAutoref_COM_flowControl) != QVariant())
    {
        val = m_settings->value(Param_Poste_PortAutoref_COM_flowControl).toString();
        setSerialPortValueFromQSettings(s_paramPortSerieAutoref, FLOWCONTROL, val, Autoref);
    }

    if (!isSerialSettingsValid(s_paramPortSerieAutoref))
    {
        QMap<QString, QString> map = DefaultSerialSettings(nameARK);
        QMetaEnum metaEnum = Utils::enumeratorSP(BAUDRATE);
        s_paramPortSerieAutoref.baudRate       = (QSerialPort::BaudRate)metaEnum.keyToValue(map[BAUDRATE].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(DATABITS);
        s_paramPortSerieAutoref.dataBits       = (QSerialPort::DataBits)metaEnum.keyToValue(map[DATABITS].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(STOPBITS);
        s_paramPortSerieAutoref.stopBits       = (QSerialPort::StopBits)metaEnum.keyToValue(map[STOPBITS].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(PARITY);
        s_paramPortSerieAutoref.parity         = (QSerialPort::Parity)metaEnum.keyToValue(map[PARITY].toLocal8Bit().data());
        metaEnum = Utils::enumeratorSP(FLOWCONTROL);
        s_paramPortSerieAutoref.flowControl    = (QSerialPort::FlowControl)metaEnum.keyToValue(map[FLOWCONTROL].toLocal8Bit().data());

        m_settings                      ->setValue(Param_Poste_PortAutoref_COM_baudrate,    map[BAUDRATE]);
        m_settings                      ->setValue(Param_Poste_PortAutoref_COM_databits,    map[DATABITS]);
        m_settings                      ->setValue(Param_Poste_PortAutoref_COM_stopBits,    map[STOPBITS]);
        m_settings                      ->setValue(Param_Poste_PortAutoref_COM_parity,      map[PARITY]);
        m_settings                      ->setValue(Param_Poste_PortAutoref_COM_flowControl, map[FLOWCONTROL]);
     }
}


upSerialPort* Procedures::PortAutoref()
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
    //qDebug() << "mesureautoref" << m_mesureSerie;
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
        if (Nidek::I()->isRequestToSend(m_mesureSerie))       //! l'autoref demande la permission d'envoyer des données
        {
            QString cmd;
            cmd = (autorefhaskerato? "CRK" : "CRM");     //! CRK ou CRM suivant que les appareils peuvent ou non envoyer la keratométrie
            //!> le PC simule la réponse du refracteur et répond par SendDataNIDEK() pour recevoir les data
            //qDebug() << "cmd" << Nidek::I()->OKtoReceive(cmd);
            PortAutoref()->writeDatas(Nidek::I()->OKtoReceive(cmd), " SendDataNIDEK(cmd) - Autoref = ");
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
    if (nameLM=="NIDEK LM-1800PD" || nameLM=="NIDEK LM-1800P" || nameLM=="NIDEK LM-500" || nameLM=="NIDEK LM-7" || nameLM=="TOMEY TL-6000" || nameLM=="TOMEY TL-7000" || nameLM=="RODENSTOCK AL 6600")
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
    QString quick = "";

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
    if (autoref->isquickOD() && autoref->isquickOG())
        quick = "quick ODG";
    else if (autoref->isquickOD() && !autoref->isquickOG())
        quick = "quick OD";
    else if (!autoref->isquickOD() && autoref->isquickOG())
        quick = "quick OG";
    if (quick != "")
        Reponse += " <font color=\"red\"><b>" + quick + "</b></font>";
    Reponse = HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>"
                           + tr("Autoref") + ":</b></font></td><td width=\"" BIG_LARGEUR_FORMULE "\">" + Reponse + "</td>";
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

upSerialPort* Procedures::PortTono()
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
        listbinds[CP_QUICKOD_REFRACTIONS]               = Datas::I()->mesureautoref->isquickOD();
        listbinds[CP_QUICKOG_REFRACTIONS]               = Datas::I()->mesureautoref->isquickOG();
        listbinds[CP_OGMESURE_REFRACTIONS]              = 1;
        Datas::I()->refractions->CreationRefraction(listbinds);

        QString requete = "select " CP_IDPATIENT_DATAOPHTA " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idPatient) + " and " CP_MESURE_DATAOPHTA " = '" + ConvertMesure(typemesure) + "'";
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

/*! * \brief dlg_docsexternes::CalcImageDocument
      Cette fonction sert à calculer les propriétés m_blob et m_formatimage
        * des documents d'imagerie
        * des courriers émis par le logiciel
      pour les afficher ou les imprimer
    * \param docmt
    * \param typedoc = Text  -> Le document est un document texte (ordo, certificat...etc).
                                Il est déjà dans la table impressions sous la forme de 3 champs html (entete, corps et pied)
                                Ces champs vont être utilisés pour l'impression vers un QByteArray via textprinter::getPDFByteArray
                                Le bytearray sera constitué par le contenu de ce fichier et affiché à l'écran.
             typedoc = Image -> le document est un document d'imagerie stocké sur un fichier. On va le transformer en bytearray
*/
void Procedures::CalcImageDocument(DocExterne *docmt)
{
    if (docmt == Q_NULLPTR )
        return;
    if (docmt->isVideo())
        return;
    QByteArray ba = QByteArray();
    QString filename = "";
    if (docmt->isImage())
    {
        filename = docmt->lienversfichier();
        if (filename != "")
        {
            QString fileformat;
            if (filename.contains("."))
            {
                QStringList lst = filename.split(".");
                fileformat      = lst.at(lst.size()-1);
            }
            if (fileformat == PDF || fileformat == JPG)
                docmt->setimageformat(fileformat);
            else return;
            if (db->ModeAccesDataBase() != Utils::Distant)
            {
                QFile fileimg(db->dirimagerie() + NOM_DIR_IMAGES + filename);
                if (fileimg.open(QIODevice::ReadOnly))
                {
                    ba = fileimg.readAll();
                    docmt->setimageblob(ba);
                    return;
                }
            }
            else
            {
                QString fullFilename = Utils::correctquoteSQL(db->dirimagerie()) + NOM_DIR_IMAGES + Utils::correctquoteSQL(filename);
                ba = getFileFromServer(fullFilename);
            }
        }
        if (ba.size()==0)    //! le document n'est pas enregistré sur le disque, on va le chercher dans la table impressions
            ba = getFileFromSQL(docmt);
        if (ba.size())
            docmt->setimageblob(ba);
    }
    else if (docmt->isText())
    {
        //!> il s'agit d'un document écrit, on le traduit en pdf et on l'affiche
        if (docmt->pdforigin() != QByteArray())
        {
            docmt           ->setimageformat(PDF);
            docmt           ->setimageblob(docmt->pdforigin());
        }
        else
        {
            QString textentete  = docmt->textentete();

            //! Toute la suite sert à nettoyer le code html des entête, pied de page et corps des premières versions de Rufus
            if (Utils::epureFontFamily(textentete) || Utils::corrigeErreurHtmlEntete(textentete, docmt->isALD()))
                ItemsList::update(docmt, CP_TEXTENTETE_DOCSEXTERNES, textentete);
            QString textcorps   = docmt->textcorps();
            if (Utils::epureFontFamily(textcorps))
                ItemsList::update(docmt, CP_TEXTCORPS_DOCSEXTERNES, textcorps);
            QString textpied    = docmt->textpied();
            if (Utils::epureFontFamily(textpied))
                ItemsList::update(docmt, CP_TEXTPIED_DOCSEXTERNES, textpied);

            //! émission du pdf
            QTextEdit   *Etat_textEdit = new UpTextEdit;
            Etat_textEdit   ->setText(textcorps);
            TextPrinter *TexteAImprimer = new TextPrinter();
            TexteAImprimer  ->setHeaderSize(docmt->isALD()? TailleEnTeteALD() : TailleEnTete());
            TexteAImprimer  ->setHeaderText(textentete);
            TexteAImprimer  ->setFooterSize(TaillePieddePage());
            TexteAImprimer  ->setFooterText(textpied);
            TexteAImprimer  ->setTopMargin(TailleTopMarge());
            ba              = TexteAImprimer->getPDFByteArray(Etat_textEdit->document());
            docmt           ->setimageformat(PDF);
            docmt           ->setimageblob(ba);
            delete Etat_textEdit;
            delete TexteAImprimer;
        }
    }
}
