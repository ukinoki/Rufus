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

dlg_docsvideo::dlg_docsvideo(Patient *pat, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionDocsVideo", parent)
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    m_currentpatient = pat;
    setWindowTitle(tr("Enregistrer une video dans le dossier de ") + pat->nom().toUpper() + " " + pat->prenom());

    m_docpath = proc->settings()->value(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Videos).toString();
    if (!QDir(m_docpath).exists())
        m_docpath = QDir::homePath();
    wdg_visuvideowdg    = new QVideoWidget(this);
    wdg_inflabel        = new UpLabel(this);
    wdg_linetitre       = new UpLineEdit(this);
    wdg_editdate        = new QDateEdit(this);
    wdg_typedoccombobx  = new UpComboBox(this);
    m_listtypesexamen   << tr("Video Chirurgie")
                        << tr("Video LAF")
                        << tr("Video Autre");
    wdg_toolbar         = new UpToolBar();
    wdg_dirsearchbutton = new UpPushButton();

    UpLabel         *lbltitre       = new UpLabel(this);
    UpLabel         *lbldate        = new UpLabel(this);
    UpLabel         *lbltype        = new UpLabel(this);
    QVBoxLayout     *rsgnmtVlay     = new QVBoxLayout();
    QVBoxLayout     *dirVlay        = new QVBoxLayout();
    QHBoxLayout     *dateLay        = new QHBoxLayout();
    QHBoxLayout     *titreLay       = new QHBoxLayout();
    QHBoxLayout     *typeLay        = new QHBoxLayout();

    connect(wdg_toolbar,    &UpToolBar::TBSignal,   this, [=] {NavigueVers(wdg_toolbar->choix());});

    wdg_toolbar     ->setMinimumHeight(30);
    wdg_dirsearchbutton->setFixedHeight(30);
    wdg_dirsearchbutton->setText(tr("Chercher un fichier"));

    wdg_typedoccombobx->insertItems(0,m_listtypesexamen);
    wdg_typedoccombobx->setEditable(false);
    wdg_linetitre->setValidator(new QRegularExpressionValidator(Utils::rgx_intitulecompta));

    lbltype     ->setText(tr("Type de document"));
    lbltitre    ->setText(tr("Titre du document"));
    lbldate     ->setText(tr("Date du document"));
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
    dirVlay     ->addWidget(wdg_inflabel);
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

    dlglayout()   ->insertWidget(0,wdg_visuvideowdg);

    QFont font = qApp->font();
    font.setPointSize(12);
    wdg_inflabel->setFont(font);

    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    setStageCount(2);
    connect(OKButton,           &QPushButton::clicked, this,   &dlg_docsvideo::ValideFiche);
    connect(wdg_dirsearchbutton,    &QPushButton::clicked, this,   &dlg_docsvideo::ChangeFile);

    buttonslayout()->insertLayout(0,rsgnmtVlay);

    buttonslayout()->insertSpacerItem(0,new QSpacerItem(10,10,QSizePolicy::Expanding));

    buttonslayout()->insertLayout(0, dirVlay);
    wdg_visuvideowdg->resize(wdg_visuvideowdg->sizeHint());
    setMinimumWidth(650);
    setStageCount(2);
    int w = width() - dlglayout()->contentsMargins().left() - dlglayout()->contentsMargins().right();
    int y = height() - dlglayout()->contentsMargins().top() - dlglayout()->contentsMargins().bottom() - dlglayout()->spacing()  - widgetbuttons()->height();
    wdg_visuvideowdg->resize(w, y);
    NavigueVers(UpToolBar::_last);
}

void dlg_docsvideo::NavigueVers(UpToolBar::Choix choix)
{
    QString fichencours = wdg_visuvideowdg->accessibleDescription();
    QStringList filters;
    filters << "*.mp4" << "*.mpg" << "*.m4v";
    QStringList listfich = QDir(m_docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
    if (listfich.size() == 0)  {
        UpMessageBox::Watch(this,tr("Il n'y a aucun document dans le dossier ") + m_docpath);
        return;
    }
    int idx = listfich.indexOf(fichencours);
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
        QString filebut = listfich.at(idx);
        //for (int i = 0; i<listfich.size();i++)
        //    qDebug() << listfich.at(i) + " - index = " + QString::number(i) + "/" + QString::number(listfich.size());
        AfficheVideo(filebut);
    }
}

void dlg_docsvideo::ChangeFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), m_docpath,  tr("Video (*.mp4 *.mpg *.m4v)"));
    if (fileName != "")
    {
        m_docpath = QFileInfo(fileName).dir().absolutePath();
        QString fichierencours = QFileInfo(fileName).fileName();
        QStringList filters;
        filters << "*.mp4" << "*.mpg" << "*.m4v";
        QStringList listfich = QDir(m_docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
        int idx = listfich.indexOf(fichierencours);
        wdg_toolbar->First()    ->setEnabled(idx>0);
        wdg_toolbar->Prec()     ->setEnabled(idx>0);
        wdg_toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
        wdg_toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
        AfficheVideo(fichierencours);
        proc->settings()->setValue(Utils::getBaseFromMode(db->ModeAccesDataBase()) + Dossier_Videos, m_docpath);
    }
}

void dlg_docsvideo::AfficheVideo(QString filebut)
{
    wdg_visuvideowdg     ->setAccessibleDescription(filebut);
    QDir        dirpict = QDir(m_docpath);
    wdg_inflabel    ->setText("<font color='magenta'>" + filebut + "</font>");

    QMediaPlayer *player = new QMediaPlayer;
    player->setVideoOutput(wdg_visuvideowdg);
    player->setSource(QUrl::fromLocalFile(dirpict.filePath(filebut)));
    player->play();

    //QMediaRecorder *rec = new QMediaRecorder(player);
}

void dlg_docsvideo::ValideFiche()
{
    if (wdg_typedoccombobx->currentText() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier le type de document"));
        wdg_typedoccombobx->setFocus();
        return;
    }
    if (wdg_linetitre->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un nom pour le document"));
        wdg_linetitre->setFocus();
        return;
    }
    if (wdg_editdate->date() == m_currentdate)
    {
        wdg_editdate->setFocus();
        UpMessageBox msgbox(this);
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
    QTextEdit txtedit;
    txtedit.setHtml(wdg_inflabel->text());
    QString filename = m_docpath + "/" + txtedit.toPlainText();
    QFile   qFile(filename);
    if (!qFile.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    // on vérifie qu'un dossier par défaut a été enregistré pour l'imagerie
    QString NomOnglet;
    if (db->ModeAccesDataBase() == Utils::Poste)                NomOnglet = tr("Monoposte");
    else if (db->ModeAccesDataBase() == Utils::ReseauLocal)     NomOnglet = tr("Réseau local");
    QString NomDirStockageImagerie = Procedures::I()->AbsolutePathDirImagerie();
    if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie ") + "<font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
        QString msg2 = tr("Renseignez un dossier valide dans Editions/Paramètres/Onglet \"ce poste\"/Onglet \"") + NomOnglet + "\"";
        UpMessageBox::Watch(this,msg, msg2);
        return;
    }
    // on vérifie que le dossier de stockage des videos existe sinon on le crée
    QString CheminVideoDir      = NomDirStockageImagerie + NOM_DIR_VIDEOS;
    QDir VideoDir;
    if (!QDir(CheminVideoDir).exists())
        if (!VideoDir.mkdir(CheminVideoDir))
        {
            QString msg = tr("Dossier de sauvegarde des videos ") + "<font color=\"red\"><b>" + CheminVideoDir + "</b></font>" + tr(" invalide");
            UpMessageBox::Watch(this,msg);
            return;
        }
    QString sstypedoc = wdg_linetitre->text();
    QString NomFileVideoDoc = QString::number(m_currentpatient->id()) + "_"
                             + sstypedoc.replace("/",".") + "_"
                             + wdg_editdate->date().toString("yyyyMMdd") + "-" + QFileInfo(qFile).birthTime().toString("HHmmss");

    QHash<QString, QVariant> listbinds;
    bool ok;
    int idimpr =  db->selectMaxFromTable(CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES, ok) + 1;
    NomFileVideoDoc = NomFileVideoDoc + "-" + QString::number(idimpr) + "." + QFileInfo(qFile).suffix();

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
        qFile.copy(CheminVideoDir + "/" + NomFileVideoDoc);
        QFile CO(CheminVideoDir + "/" + NomFileVideoDoc);
        CO.open(QIODevice::ReadWrite);
        CO.setPermissions(QFileDevice::ReadOther
                          | QFileDevice::ReadGroup
                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
        if (QFileInfo(qFile).absoluteFilePath() != CheminVideoDir + "/" + NomFileVideoDoc)
            qFile.remove();
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Video ") + sstypedoc +  tr(" enregistrée"), Icons::icSunglasses(), 1000);
        close();
    }
}


