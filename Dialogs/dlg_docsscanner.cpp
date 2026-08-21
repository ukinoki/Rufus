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

#include "dlg_docsscanner.h"

dlg_docsscanner::dlg_docsscanner(Item *item, Mode mode, QString titre, QWidget *parent) :
    QObject(parent)
{
    m_docpath = proc->settings()->value(Param_Poste Dossier_DocsScannes).toString();
    if (QDir(m_docpath).entryList(m_filters,QDir::Files,QDir::Time | QDir::Reversed).size() == 0 || !QDir(m_docpath).exists() || m_docpath == "")
    {
        bool pathchanged;
        if (!searchDir(pathchanged))
            UpMessageBox::Watch(parent, tr("Dossier vide"), tr("il n'y a aucun fichier image dans le dossier par défaut ") + m_docpath);
    }
    if (!QDir(m_docpath).exists())
        m_docpath = QDir::homePath();

    m_mode           = mode;
    if ( m_mode == Document)
        m_iditem = qobject_cast<Patient*>(item)->id();
    else
        m_iditem = qobject_cast<Depense*>(item)->id();
    dlg_imgviewer     = new dlg_singleimageviewer(parent);
    dlg_imgviewer     ->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    dlg_imgviewer     ->setAttribute(Qt::WA_DeleteOnClose, true);

    /* utilisé pour les tests en simulant un accès distant
    AccesDistant = true;
    Base = Utils::getBaseFromMode(Utils::ReseauLocal);*/
    m_accesdistant = (db->ModeAccesDataBase()==Utils::Distant);

    wdg_linetitre       = new UpLineEdit();
    wdg_editdate        = new QDateEdit();
    wdg_editdate        ->setDisplayFormat(tr("dd/MM/yyyy"));
    wdg_typedoccombobx  = new UpComboBox();
    int maxlength       = 200;
    wdg_linetitre       ->setMaximumWidth(maxlength);
    wdg_typedoccombobx  ->setMaximumWidth(maxlength + 10);
    wdg_editdate        ->setMaximumWidth(100);
    switch ( m_mode) {
    case Document:
        m_listtypesexamen   << COURRIER
                        << tr("CV")
                        << tr("Orthoptie")
                        << tr("ANGIO")
                        << tr("OCT")
                        << tr("Biométrie")
                        << tr("RNM")
                        << tr("Speculaire")
                        << tr("Topographie")
                        << tr("Hess-Weiss")
                        << tr("CRO")
                        << tr("Autre Imagerie");
        break;
    case Facture:
        m_listtypesexamen   << FACTURE;
        break;
    case Echeancier:
        m_listtypesexamen   << ECHEANCIER;
        break;
    }
    /*wdg_typedoccombobx->addItem(tr("Courrier")  , COURRIER);
    wdg_typedoccombobx->addItem(tr("CV")        , CV);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);
    wdg_typedoccombobx->addItem(tr("Courrier"), COURRIER);*/
    wdg_toolbar         = new UpToolBar();
    wdg_dirsearchbutton = new UpPushButton();

    UpLabel         *lbltitre       = new UpLabel();
    UpLabel         *lbldate        = new UpLabel();
    UpLabel         *lbltype        = new UpLabel();
    QVBoxLayout     *rsgnmtVlay     = new QVBoxLayout();
    QVBoxLayout     *dirVlay        = new QVBoxLayout();
    QHBoxLayout     *dateLay        = new QHBoxLayout();
    QHBoxLayout     *titreLay       = new QHBoxLayout();
    QHBoxLayout     *typeLay        = new QHBoxLayout();

    connect(wdg_toolbar, &UpToolBar::TBSignal, this, [=, this] {NavigueVers(wdg_toolbar->choice());});

    wdg_toolbar         ->setMinimumHeight(30);
    wdg_dirsearchbutton ->setFixedHeight(30);
    wdg_dirsearchbutton ->setText(tr("Chercher un fichier"));

    wdg_typedoccombobx  ->insertItems(0,m_listtypesexamen);
    wdg_typedoccombobx  ->setEditable(false);
    wdg_linetitre       ->setValidator(new QRegularExpressionValidator(Utils::rgx_intitulecompta));

    lbltype     ->setText(tr("Type de document"));
    lbltitre    ->setText(tr("Titre du document"));
    lbldate     ->setText(tr("Date du document"));
    wdg_linetitre   ->setText(titre);
    wdg_editdate    ->setDate(m_currentdate);

    typeLay     ->addWidget(lbltype);
    typeLay     ->addWidget(wdg_typedoccombobx);
    titreLay    ->addWidget(lbltitre);
    titreLay    ->addWidget(wdg_linetitre);
    dateLay     ->addWidget(lbldate);
    dateLay     ->addWidget(wdg_editdate);
    rsgnmtVlay  ->addLayout(typeLay);
    rsgnmtVlay  ->addLayout(titreLay);
    rsgnmtVlay  ->addLayout(dateLay);
    rsgnmtVlay  ->setSpacing(2);
    dirVlay     ->addWidget(wdg_toolbar);
    dirVlay     ->addSpacerItem(new QSpacerItem(5,5, QSizePolicy::Minimum, QSizePolicy::Expanding));
    dirVlay     ->addWidget(wdg_dirsearchbutton);
    dirVlay     ->setSpacing(2);
    titreLay    ->setSpacing(5);
    typeLay     ->setSpacing(5);
    dateLay     ->setSpacing(5);
    rsgnmtVlay  ->setContentsMargins(0,0,0,0);
    typeLay     ->setContentsMargins(0,0,0,0);
    titreLay    ->setContentsMargins(0,0,0,0);
    dateLay     ->setContentsMargins(0,0,0,0);


    dlg_imgviewer     ->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    connect(dlg_imgviewer->OKButton,  &QPushButton::clicked, this,   &dlg_docsscanner::ValideFiche);
    connect(wdg_dirsearchbutton,    &QPushButton::clicked, this,   &dlg_docsscanner::ChangeFile);

    dlg_imgviewer     ->buttonslayout()->insertLayout(0,rsgnmtVlay);

    dlg_imgviewer     ->buttonslayout()->insertSpacerItem(0,new QSpacerItem(10,10,QSizePolicy::Expanding));

    dlg_imgviewer     ->buttonslayout()->insertLayout(0, dirVlay);
    dlg_imgviewer     ->setStageCount(2);

    dlg_imgviewer     ->setMinimumWidth(650);
    dlg_imgviewer     ->setStageCount(2);
    m_initok        = true;
    NavigueVers(UpToolBar::_last);
    dlg_imgviewer     ->setEnregPosition(true);
    dlg_imgviewer     ->setSaveGeometry(Nom_fiche_DocsScanner);
}

dlg_docsscanner::~dlg_docsscanner()
{
}

void dlg_docsscanner::NavigueVers(UpToolBar::Choice choix)
{
    QStringList listfich = QDir(m_docpath).entryList(m_filters,QDir::Files,QDir::Time | QDir::Reversed);
    if (listfich.size() == 0)  {
        UpMessageBox::Watch(dlg_imgviewer,tr("Il n'y a aucun document dans le dossier ") + m_docpath,
                             tr("Vous devez scanner les documents au format pdf, png ou jpg."));
        return;
    }
    int idx = listfich.indexOf(m_currentImagefile);
    if (choix == UpToolBar::_last)
        idx = listfich.size()-1;
    else if (choix == UpToolBar::_first)
        idx = 0;
    else if (choix == UpToolBar::_next)
        idx += 1;
    else if (choix == UpToolBar::_prec)
        idx -= 1;
    wdg_toolbar->First()    ->setEnabled(idx>0);
    wdg_toolbar->Prec()     ->setEnabled(idx>0);
    wdg_toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
    wdg_toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
    if (idx>-1)
    {
        m_currentImagefile = listfich.at(idx);
        bool initOK = false;
        DocExterne docmt(m_docpath + "/" + m_currentImagefile, initOK, dlg_imgviewer);
        if (initOK)
            dlg_imgviewer->setListDocuments(QList<DocExterne*>() << &docmt);
    }
}

void dlg_docsscanner::ChangeFile()
{
    bool pathchanged = true;
    if (!searchDir(pathchanged))
        if (!pathchanged)
            return;
    QStringList listfich    = QDir(m_docpath).entryList(m_filters, QDir::Files,QDir::Time | QDir::Reversed);
    int idx                 = listfich.indexOf(m_currentImagefile);
    wdg_toolbar->First()    ->setEnabled(idx>0);
    wdg_toolbar->Prec()     ->setEnabled(idx>0);
    wdg_toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
    wdg_toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
    bool initOK             = false;
    DocExterne docmt(m_docpath + "/" + m_currentImagefile, initOK, dlg_imgviewer);
    if (initOK)
        dlg_imgviewer->setListDocuments(QList<DocExterne*>() << &docmt);
}

bool dlg_docsscanner::searchDir(bool &pathchanged)
{
    //! Filtres par TYPE MIME plutôt que par chaîne "*.pdf …". Sur macOS, le panneau natif grisait à
    //! tort des fichiers valides (PDF) quand ce dialogue était ouvert sous la modalité PROFONDE du
    //! chemin depuis dlg_depenses (Rufus → dlg_depenses modal → dlg_imgviewer modal → panneau) : le
    //! délégué d'activation piloté par le filtre d'extension est capricieux sous imbrication modale.
    //! Les types MIME mappent vers les vrais UTI (com.adobe.pdf, public.jpeg, public.png) et sont
    //! robustes à ce contexte. On garde le panneau natif (aucun DontUseNativeDialog).
    QFileDialog fdlg(dlg_imgviewer, tr("Choisir un fichier"), m_docpath);
    fdlg.setFileMode(QFileDialog::ExistingFile);
    fdlg.setAcceptMode(QFileDialog::AcceptOpen);
    fdlg.setMimeTypeFilters(QStringList() << "application/pdf" << "image/jpeg" << "image/png");
    QString fileName;
    if (fdlg.exec() == QDialog::Accepted && !fdlg.selectedFiles().isEmpty())
        fileName = fdlg.selectedFiles().first();
    if (fileName != "")
    {
        pathchanged         = (m_docpath != QFileInfo(fileName).dir().absolutePath());
        m_currentImagefile  = QFileInfo(fileName).fileName();
        if (pathchanged)
        {
            m_docpath       = QFileInfo(fileName).dir().absolutePath();
            proc->settings()->setValue(Param_Poste Dossier_DocsScannes, m_docpath);
        }
    }
    if (QDir(m_docpath).entryList(m_filters,QDir::Files,QDir::Time | QDir::Reversed).size() == 0)
        return false;
    return true;
}

QMap<QString, QVariant> dlg_docsscanner::getdataFacture()
{
    return map_datafacture;
}

dlg_singleimageviewer *dlg_docsscanner::dialog() const
{
    return dlg_imgviewer;
}

bool dlg_docsscanner::initOK() const
{
    return m_initok;
}

void dlg_docsscanner::ValideFiche()
{
    if (wdg_typedoccombobx->currentText() == "")
    {
        UpMessageBox::Watch(dlg_imgviewer,tr("Vous avez oublié de spécifier le type de document"));
        wdg_typedoccombobx->setFocus();
        return;
    }
    if (wdg_linetitre->text() == "")
    {
        UpMessageBox::Watch(dlg_imgviewer,tr("Vous avez oublié de spécifier un nom pour le document"));
        wdg_linetitre->setFocus();
        return;
    }
    if (wdg_editdate->date() == m_currentdate)
    {
        wdg_editdate->setFocus();
        UpMessageBox msgbox(dlg_imgviewer);
        msgbox.setText(tr("Confirmez la date d'aujourd'hui pour ce document"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKDateBouton;
        OKDateBouton.setText(tr("Je confirme"));
        UpSmallButton AnnulBouton;
        AnnulBouton.setText(tr("Annuler"));
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKDateBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKDateBouton)
            return;
    }

    // enregistrement du document ----------------------------------------------------------------------------------------------------------------------------------------------
    QString filename = m_docpath + "/" + m_currentImagefile;
    QFile   file_origin(filename);
    if (!file_origin.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(dlg_imgviewer, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    QByteArray ba = file_origin.readAll();
    file_origin.close();

    QString datetransfer = m_currentdate.toString("yyyy-MM-dd");
    QString user("");
    if ( m_mode != Document )                   //! il s'agit d'une facture ou d'un échéancier
    {
        int iduserdep = 0;
        if (Datas::I()->depenses->getById(m_iditem) != nullptr)
            iduserdep = Datas::I()->depenses->getById(m_iditem)->iduser();
        if (Datas::I()->users->getById(iduserdep) != nullptr)
            user = Datas::I()->users->getById(iduserdep)->login();
    }

    //! copie de l'original sur le poste importateur en accès distant et sur le serveur dans les autres cas
    QString CheminBackup = proc->AbsolutePathDirImagerie() + NOM_DIR_ORIGINAUX
                         + ( m_mode==Document? NOM_DIR_IMAGES : NOM_DIR_FACTURES) + "/" + ( m_mode==Document? datetransfer : user);
    Utils::mkpath(CheminBackup);
    Utils::copyWithPermissions(file_origin, CheminBackup + "/" + m_currentImagefile);

    //! création du dossier d'enregistrement du document renommé et compressé en mode monoposte ou réseau local
    if (!m_accesdistant)
    {
        QString CheminOKTransfrDir  = db->dirimagerie()
                                    + ( m_mode == Document? NOM_DIR_IMAGES "/" + datetransfer : NOM_DIR_FACTURES "/" + user) ;
        if (!Utils::mkpath(CheminOKTransfrDir))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
            ShowMessage::I()->SplashMessage(msg, 3000);
            return;
        }
        //! Dossier créé sur le serveur (via le partage réseau si ce poste est un poste Windows) :
        //! on le rend traversable par le compte du serveur MySQL, sinon LOAD_FILE ne pourra pas
        //! relire les fichiers qu'il contient depuis un poste distant (cf. helper).
        Utils::rendDossierAccessibleAuServeurSQL(CheminOKTransfrDir);
    }

    //! compression du document si c'est un jpg
    QString suffixe = QFileInfo(file_origin).suffix().toLower();
    if (suffixe == JPG ||suffixe == PNG || suffixe == JPEG)
    {
        QString msg = "";
        if (!Utils::CompressFileToJPG(filename, msg))
            return;
        suffixe = JPG;
        file_origin.setFileName(filename);
        if (!file_origin.open( QIODevice::ReadOnly ))
        ba = file_origin.readAll();
    }

    QString sstypedoc = wdg_linetitre->text();
    int idimpr (0);
    QHash<QString,QVariant> listbinds;
    bool b = false;
    bool ok;
    QString lien;

    if ( m_mode == Document)      // c'est un document scanné
    {
        DataBase::I()->locktables(QStringList() << TBL_DOCSEXTERNES);
        idimpr =  db->selectMaxFromTable(CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES, ok) + 1;
        QString NomFileDoc = QString::number(m_iditem) + "_"
                + wdg_typedoccombobx->currentText() + "_"
                + sstypedoc.replace("/",".") + "_"                  // on fait ça pour que le / ne soit pas interprété comme un / de séparation de dossier dans le nom du fichier, ce qui planterait l'enregistrement
                + wdg_editdate->dateTime().toString("yyyy-MM-dd");
        lien = "/" + datetransfer + "/" + NomFileDoc + "-" + QString::number(idimpr) + "." + suffixe;
        if (!m_accesdistant)
        {
            listbinds[CP_ID_DOCSEXTERNES] =               idimpr;
            listbinds[CP_IDPAT_DOCSEXTERNES] =            m_iditem;
            listbinds[CP_TYPEDOC_DOCSEXTERNES] =          wdg_typedoccombobx->currentText();
            listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES] =      sstypedoc;
            listbinds[CP_TITRE_DOCSEXTERNES] =            wdg_typedoccombobx->currentText();
            listbinds[CP_DATE_DOCSEXTERNES] =             wdg_editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
            listbinds[CP_IDEMETTEUR_DOCSEXTERNES] =       Datas::I()->users->userconnected()->id();
            listbinds[CP_LIENFICHIER_DOCSEXTERNES] =      lien;
            listbinds[CP_EMISORRECU_DOCSEXTERNES] =       "1";
            listbinds[CP_FORMATDOC_DOCSEXTERNES] =        DOCUMENTRECU;
            listbinds[CP_IDLIEU_DOCSEXTERNES] =           Datas::I()->sites->idcurrentsite();
        }
        else
        {
            if (suffixe == PDF)
                suffixe = CP_PDF_DOCSEXTERNES;
            else if (suffixe == JPG)
                suffixe = CP_JPG_DOCSEXTERNES;
            listbinds[CP_ID_DOCSEXTERNES] =               idimpr;
            listbinds[CP_IDPAT_DOCSEXTERNES] =            m_iditem;
            listbinds[CP_TYPEDOC_DOCSEXTERNES] =          wdg_typedoccombobx->currentText();
            listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES] =      sstypedoc;
            listbinds[CP_TITRE_DOCSEXTERNES] =            wdg_typedoccombobx->currentText();
            listbinds[CP_DATE_DOCSEXTERNES] =             wdg_editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
            listbinds[CP_IDEMETTEUR_DOCSEXTERNES] =       Datas::I()->users->userconnected()->id();
            listbinds[suffixe] =                          ba;
            listbinds[CP_EMISORRECU_DOCSEXTERNES] =       "1";
            listbinds[CP_FORMATDOC_DOCSEXTERNES] =        DOCUMENTRECU;
            listbinds[CP_IDLIEU_DOCSEXTERNES] =           Datas::I()->sites->idcurrentsite();
        }
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        b = (doc != nullptr);
        delete doc;
    }
    else                        // c'est une facture ou un échéancier
    {
        idimpr =  db->selectMaxFromTable(CP_ID_FACTURES, TBL_FACTURES, ok) + 1;
        QString NomFileDoc = QString::number(idimpr) + "_"
                + wdg_typedoccombobx->currentText() + "_"
                + sstypedoc.replace("/",".") + "_"                  // on fait ça pour que le / ne soit pas interprété comme un / de séparation de dossier dans le nom du fichier, ce qui planterait l'enregistrement
                + wdg_editdate->dateTime().toString("yyyy-MM-dd");
        lien = "/" + user + "/" + NomFileDoc  + ( m_mode== Echeancier? "" : "-" + QString::number(idimpr)) +"." + suffixe;
        if (!m_accesdistant)
        {
            listbinds[CP_ID_FACTURES] =               idimpr;
            listbinds[CP_DATEFACTURE_FACTURES] =      wdg_editdate->date().toString("yyyy-MM-dd");
            listbinds[CP_INTITULE_FACTURES] =         sstypedoc;
            listbinds[CP_LIENFICHIER_FACTURES] =      lien;
            listbinds[CP_ECHEANCIER_FACTURES] =       ( m_mode== Echeancier? "1" : QVariant(QMetaType::fromType<QString>()));
            listbinds[CP_IDDEPENSE_FACTURES] =        ( m_mode== Echeancier? QVariant(QMetaType::fromType<QString>()) : QString::number(m_iditem));
            map_datafacture["lien"] =                 lien;
        }
        else
        {
            listbinds[CP_ID_FACTURES] =               idimpr;
            listbinds[CP_DATEFACTURE_FACTURES] =      wdg_editdate->date().toString("yyyy-MM-dd");
            listbinds[CP_INTITULE_FACTURES] =         sstypedoc;
            listbinds[CP_ECHEANCIER_FACTURES] =       ( m_mode== Echeancier? "1" : QVariant(QMetaType::fromType<QString>()));
            listbinds[CP_IDDEPENSE_FACTURES] =        ( m_mode== Echeancier? QVariant(QMetaType::fromType<QString>()) : QString::number(m_iditem));
            listbinds[suffixe] =                      ba;
            map_datafacture["lien"] =                 "";
        }
        b = db->InsertSQLByBinds(TBL_FACTURES, listbinds);
        if(!b)
            UpMessageBox::Watch(dlg_imgviewer,tr("Impossible d'enregistrer ce document dans la base!"));
        map_datafacture["idfacture"] = idimpr;
        map_datafacture["echeancier"] = ( m_mode == Echeancier);
        map_datafacture["objetecheancier"] = ( m_mode == Echeancier? sstypedoc : "");
    }
    if(!b)
    {
        file_origin.close ();
        dlg_imgviewer->reject();
        return;
    }
    else if (!m_accesdistant)
    {
        QString CheminOKTransfrDoc = db->dirimagerie() + ( m_mode == Document? NOM_DIR_IMAGES : NOM_DIR_FACTURES) + lien;
        if (suffixe == JPG)
        {
            QFile CF(filename);
            Utils::copyWithPermissions(CF,CheminOKTransfrDoc);
        }
        else if (suffixe == PDF)
            Utils::copyWithPermissions(file_origin, CheminOKTransfrDoc);
     }
    Utils::removeWithoutPermissions(file_origin);
    QString msg;
    switch ( m_mode) {
    case Document:      msg = tr("Document ") + sstypedoc +  tr(" enregistré");     break;
    case Facture:       msg = tr("Facture ") + sstypedoc +  tr(" enregistrée");     break;
    case Echeancier:    msg = tr("Echeancier ") + sstypedoc +  tr(" enregistré");   break;
    }
    UpSystemTrayIcon::I()->showMessage(tr("Messages"), msg, Icons::icSunglasses(), 1000);
    dlg_imgviewer->accept();
    return;
}
