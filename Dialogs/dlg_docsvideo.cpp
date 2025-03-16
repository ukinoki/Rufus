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

#include "dlg_docsvideo.h"

dlg_docsvideo::dlg_docsvideo(Patient *pat, QWidget *parent) : QObject(parent)
{
    m_docpath = proc->settings()->value(Param_Poste Dossier_Videos).toString();
    if (QDir(m_docpath).entryList(m_filters,QDir::Files,QDir::Time | QDir::Reversed).size() == 0)
    {
        bool pathchanged;
        if (!searchDir(pathchanged))
            UpMessageBox::Watch(parent, tr("Dossier vide"), tr("il n'y a aucun fichier video dans le dossier par défaut ") + m_docpath);
    }
    if (!QDir(m_docpath).exists())
        m_docpath = QDir::homePath();
    m_currentpatient = pat;


    dlg_imgviewer         = new dlg_singleimageviewer();
    dlg_imgviewer         ->setWindowTitle(tr("Enregistrer une video dans le dossier de ") + pat->nom().toUpper() + " " + pat->prenom());
    dlg_imgviewer         ->setAttribute(Qt::WA_DeleteOnClose, true);
    m_docpath           = proc->settings()->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Videos).toString();

    wdg_linetitre       = new UpLineEdit();
    wdg_editdate        = new QDateEdit();
    wdg_typedoccombobx  = new UpComboBox();
    int maxlength       = 200;
    wdg_linetitre       ->setMaximumWidth(maxlength);
    wdg_typedoccombobx  ->setMaximumWidth(maxlength + 10);
    wdg_editdate        ->setMaximumWidth(100);
    m_listtypesexamen   << tr("Video Chirurgie")
                        << tr("Video LAF")
                        << tr("Video Autre");
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

    connect(wdg_toolbar,    &UpToolBar::TBSignal,   this, [=] {NavigueVers(wdg_toolbar->choice());});

    wdg_toolbar         ->setMinimumHeight(30);
    wdg_dirsearchbutton ->setFixedHeight(30);
    wdg_dirsearchbutton ->setText(tr("Chercher un fichier"));

    wdg_typedoccombobx  ->insertItems(0,m_listtypesexamen);
    wdg_typedoccombobx  ->setEditable(false);
    wdg_linetitre       ->setValidator(new QRegularExpressionValidator(Utils::rgx_intitulecompta));

    lbltype     ->setText(tr("Type de document"));
    lbltitre    ->setText(tr("Titre du document"));
    lbldate     ->setText(tr("Date du document"));
    wdg_editdate    ->setDisplayFormat(tr("dd/MM/yyyy"));
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
    QWidget *rsgnmtwidg = new QWidget();
    rsgnmtwidg  ->setLayout(rsgnmtVlay);
    rsgnmtwidg  ->setMaximumWidth(400);
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

    dlg_imgviewer ->buttonslayout()->insertWidget(0,rsgnmtwidg);
    dlg_imgviewer ->buttonslayout()->insertSpacerItem(0,new QSpacerItem(10,10,QSizePolicy::Expanding));
    dlg_imgviewer ->buttonslayout()->insertLayout(0, dirVlay);
    dlg_imgviewer ->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    dlg_imgviewer ->setMinimumWidth(650);
    dlg_imgviewer ->setStageCount(2);

    connect(dlg_imgviewer->OKButton,        &QPushButton::clicked,                  this,   &dlg_docsvideo::ValideFiche);
    connect(wdg_dirsearchbutton,            &QPushButton::clicked,                  this,   &dlg_docsvideo::ChangeFile);
    connect(dlg_imgviewer->imagewidget(),   &QWidget::customContextMenuRequested,   this,   &dlg_docsvideo::addFullScreenMenu);

    NavigueVers(UpToolBar::_last);
    m_initOK = true;
    dlg_imgviewer         ->setEnregPosition(true);
    dlg_imgviewer         ->setSaveGeometry(Nom_fiche_DocsVideo);
}

void dlg_docsvideo::NavigueVers(UpToolBar::Choice choix)
{
    QStringList listfich = QDir(m_docpath).entryList(m_filters,QDir::Files,QDir::Time | QDir::Reversed);
    if (listfich.size() == 0)  {
        UpMessageBox::Watch(dlg_imgviewer,tr("Il n'y a aucun document dans le dossier ") + m_docpath);
        dlg_imgviewer->setStageCount(2);
        return;
    }
    int idx = listfich.indexOf(m_currentvideofile);
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
        m_currentvideofile  = listfich.at(idx);
        dlg_imgviewer       ->setVideofile(m_docpath + "/" + m_currentvideofile);
    }
}

dlg_singleimageviewer *dlg_docsvideo::dialog() const
{
    return dlg_imgviewer;
}

void dlg_docsvideo::ChangeFile()
{
    bool pathchanged = true;
    if (!searchDir(pathchanged))
        if (!pathchanged)
            return;
    QStringList listfich    = QDir(m_docpath).entryList(m_filters, QDir::Files,QDir::Time | QDir::Reversed);
    int idx                 = listfich.indexOf(m_currentvideofile);
    wdg_toolbar->First()    ->setEnabled(idx>0);
    wdg_toolbar->Prec()     ->setEnabled(idx>0);
    wdg_toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
    wdg_toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
    dlg_imgviewer           ->setVideofile(m_docpath + "/" + m_currentvideofile);
}

void dlg_docsvideo::addFullScreenMenu()
{
    if (!QFile(m_docpath + "/" + m_currentvideofile).exists())
        return;
    if (m_Menu != Q_NULLPTR)
        delete m_Menu;
    m_Menu                          = new QMenu(dlg_imgviewer->imagewidget());
    QAction *paction_Fullscreen     = new QAction(Icons::pxFullscreen(), tr("Afficher en plein écran"));
    m_Menu->addAction(paction_Fullscreen);
    connect (paction_Fullscreen,    &QAction::triggered,    dlg_imgviewer->imagewidget(),  [=] {
        QString filepath            = m_docpath + "/" + m_currentvideofile;
        Utils::playVideoFullScreen(filepath, dlg_imgviewer);
    });
    m_Menu->exec(dlg_imgviewer->imagewidget()->cursor().pos());
    m_Menu->close();
}

bool dlg_docsvideo::searchDir(bool &pathchanged)
{
    QString fileName = QFileDialog::getOpenFileName(dlg_imgviewer, tr("Choisir un fichier"), m_docpath,  tr("Video (*.mp4 *.mpg *.m4v)"));
    if (fileName != "")
    {
        pathchanged = (m_docpath != QFileInfo(fileName).dir().absolutePath());
        m_currentvideofile = QFileInfo(fileName).fileName();
        if (pathchanged)
        {
            m_docpath = QFileInfo(fileName).dir().absolutePath();
            proc->settings()->setValue(Param_Poste Dossier_Videos, m_docpath);
        }
    }
    if (QDir(m_docpath).entryList(m_filters,QDir::Files,QDir::Time | QDir::Reversed).size() == 0)
        return false;
    return true;
}

void dlg_docsvideo::ValideFiche()
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
        msgbox.setText(tr("Confirmez la date d'aujourd'hui pour cette video"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKDateBouton;
        OKDateBouton.setText("Je confirme");
        UpSmallButton AnnulBouton;
        AnnulBouton.setText(tr("Annuler"));
        msgbox.addButton(&AnnulBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKDateBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKDateBouton)
            return;
        }

    // enregistrement du document ----------------------------------------------------------------------------------------------------------------------------------------------
    QString filename = m_docpath + "/" + m_currentvideofile;
    QFile   qFile(filename);
    if (!qFile.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(dlg_imgviewer, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    // on vérifie qu'un dossier par défaut a été enregistré pour l'imagerie
    // on vérifie que le dossier de stockage des videos existe sinon on le crée
    QString CheminVideoDir      = proc->settings()->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Videos).toString();
    QDir VideoDir;
    if (!QDir(CheminVideoDir).exists())
        if (!VideoDir.mkdir(CheminVideoDir))
        {
            QString msg = tr("Dossier de sauvegarde des videos ") + "<font color=\"red\"><b>" + CheminVideoDir + "</b></font>" + tr(" invalide");
            UpMessageBox::Watch(dlg_imgviewer,msg);
            return;
        }
    QString sstypedoc = wdg_linetitre->text();
    QString NomFileVideoDoc = QString::number(m_currentpatient->id()) + "_"
                             + sstypedoc.replace("/",".") + "_"
                             + wdg_editdate->date().toString("yyyyMMdd") + "-" + QFileInfo(qFile).birthTime().toString("HHmmss");

    QHash<QString, QVariant> listbinds;
    bool ok;
    int idimpr =  db->selectMaxFromTable(CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES, ok) + 1;
    NomFileVideoDoc = NomFileVideoDoc + "-" + QString::number(idimpr) + "." + QFileInfo(qFile).suffix().toLower();

    listbinds[CP_IDPAT_DOCSEXTERNES] =            m_currentpatient->id();
    listbinds[CP_TYPEDOC_DOCSEXTERNES] =          wdg_typedoccombobx->currentText();
    listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES] =      sstypedoc;
    listbinds[CP_TITRE_DOCSEXTERNES] =            sstypedoc;
    listbinds[CP_DATE_DOCSEXTERNES] =             wdg_editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
    listbinds[CP_IDEMETTEUR_DOCSEXTERNES] =       Datas::I()->users->userconnected()->id();
    listbinds[CP_EMISORRECU_DOCSEXTERNES] =       "0";
    listbinds[CP_FORMATAUTRE_DOCSEXTERNES] =      VIDEO;
    listbinds[CP_LIENFICHIER_DOCSEXTERNES] =      NomFileVideoDoc;
    listbinds[CP_FORMATDOC_DOCSEXTERNES] =        VIDEO;

    DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
    bool b = (doc != Q_NULLPTR);
    delete doc;
    if(!b)
        qFile.close ();
    else
    {
        Utils::copyWithPermissions(qFile, CheminVideoDir + "/" + NomFileVideoDoc);
        if (QFileInfo(qFile).absoluteFilePath() != CheminVideoDir + "/" + NomFileVideoDoc)
            Utils::removeWithoutPermissions(qFile);
        UpSystemTrayIcon::I()   ->showMessage(tr("Messages"), tr("Video ") + sstypedoc +  tr(" enregistrée"), Icons::icSunglasses(), 1000);
        dlg_imgviewer           ->close();
    }
}


