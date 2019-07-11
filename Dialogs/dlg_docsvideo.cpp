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

#include "dlg_docsvideo.h"

dlg_docsvideo::dlg_docsvideo(Patient *pat, QWidget *parent) :
    UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionDocsVideo", parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    proc            = Procedures::I();
    db              = DataBase::I();
    m_currentpatient = pat;
    QString Base;
    switch (db->getMode()) {
    case DataBase::Poste:
        Base = "BDD_POSTE";
        break;
    case DataBase::Distant:
        Base = "BDD_DISTANT";
        break;
    case DataBase::ReseauLocal:
        Base = "BDD_LOCAL";
        break;
    default:
        break;
    }

    docpath = proc->gsettingsIni->value(Base + "/DossiersVideos").toString();
    if (!QDir(docpath).exists())
        docpath = QDir::homePath();
    upvisu          = new QVideoWidget(this);
    inflabel        = new UpLabel(this);
    linetitre       = new UpLineEdit(this);
    editdate        = new QDateEdit(this);
    typeDocCombo    = new UpComboBox(this);
    ListTypeExams   << tr("Video Chirurgie")
                    << tr("Video LAF")
                    << tr("Video Autre");
    toolbar         = new UpToolBar();
    dirsearchbutton = new UpPushButton();

    UpLabel         *lbltitre       = new UpLabel(this);
    UpLabel         *lbldate        = new UpLabel(this);
    UpLabel         *lbltype        = new UpLabel(this);
    QVBoxLayout     *rsgnmtVlay     = new QVBoxLayout();
    QVBoxLayout     *dirVlay        = new QVBoxLayout();
    QHBoxLayout     *dateLay        = new QHBoxLayout();
    QHBoxLayout     *titreLay       = new QHBoxLayout();
    QHBoxLayout     *typeLay        = new QHBoxLayout();

    connect(toolbar,    &UpToolBar::TBSignal,   this, [=] {NavigueVers(toolbar->choix());});

    toolbar     ->setMinimumHeight(30);
    dirsearchbutton->setFixedHeight(30);
    dirsearchbutton->setText(tr("Chercher un fichier"));

    typeDocCombo->insertItems(0,ListTypeExams);
    typeDocCombo->setEditable(false);
    linetitre->setValidator(new QRegExpValidator(Utils::rgx_intitulecompta));

    lbltype     ->setText(tr("Type de document"));
    lbltitre    ->setText(tr("Titre du document"));
    lbldate     ->setText(tr("Date du document"));
    editdate    ->setDate(QDate::currentDate());

    typeLay     ->addWidget(lbltype);
    typeLay     ->addWidget(typeDocCombo);
    titreLay    ->addWidget(lbltitre);
    titreLay    ->addWidget(linetitre);
    dateLay     ->addWidget(lbldate);
    dateLay     ->addWidget(editdate);
    rsgnmtVlay  ->addLayout(typeLay);
    rsgnmtVlay  ->addLayout(titreLay);
    rsgnmtVlay  ->addLayout(dateLay);
    rsgnmtVlay  ->setSpacing(2);
    dirVlay     ->addWidget(inflabel);
    dirVlay     ->addWidget(toolbar);
    dirVlay     ->addSpacerItem(new QSpacerItem(5,5, QSizePolicy::Minimum, QSizePolicy::Expanding));
    dirVlay     ->addWidget(dirsearchbutton);
    dirVlay     ->setSpacing(2);
    titreLay    ->setSpacing(5);
    typeLay     ->setSpacing(5);
    dateLay     ->setSpacing(5);
    rsgnmtVlay  ->setContentsMargins(0,0,0,0);
    typeLay     ->setContentsMargins(0,0,0,0);
    titreLay    ->setContentsMargins(0,0,0,0);
    dateLay     ->setContentsMargins(0,0,0,0);

    dlglayout()   ->insertWidget(0,upvisu);

    QFont font = qApp->font();
    font.setPointSize(12);
    inflabel->setFont(font);

    AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);
    setStageCount(2);
    connect(OKButton,           &QPushButton::clicked, this,   &dlg_docsvideo::ValideFiche);
    connect(dirsearchbutton,    &QPushButton::clicked, this,   &dlg_docsvideo::ChangeFile);

    buttonslayout()->insertLayout(0,rsgnmtVlay);

    buttonslayout()->insertSpacerItem(0,new QSpacerItem(10,10,QSizePolicy::Expanding));

    buttonslayout()->insertLayout(0, dirVlay);
    upvisu->resize(upvisu->sizeHint());
    setModal(true);
    setMinimumWidth(650);
    setStageCount(2);
    int w = width() - dlglayout()->contentsMargins().left() - dlglayout()->contentsMargins().right();
    int y = height() - dlglayout()->contentsMargins().top() - dlglayout()->contentsMargins().bottom() - dlglayout()->spacing()  - widgetbuttons()->height();
    upvisu->resize(w, y);
    NavigueVers("Fin");
}

void dlg_docsvideo::NavigueVers(QString but)
{
    QString fichencours = upvisu->accessibleDescription();
    QStringList filters;
    filters << "*.mp4" << "*.mpg" << "*.m4v";
    QStringList listfich = QDir(docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
    if (listfich.size() == 0)  {
        UpMessageBox::Watch(this,tr("Il n'y a aucun document dans le dossier ") + docpath);
        return;
    }
    int idx = listfich.indexOf(fichencours);
    if (but == "Fin")
        idx = listfich.size()-1;
    else if (but == "Début")
        idx = 0;
    else if (but == "Suivant")
        idx += 1;
    else if (but == "Précédent")
        idx -= 1;
    toolbar->First()    ->setEnabled(idx>0);
    toolbar->Prec()     ->setEnabled(idx>0);
    toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
    toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), docpath,  tr("Video (*.mp4 *.mpg *.m4v)"));
    if (fileName != "")
    {
        docpath = QFileInfo(fileName).dir().absolutePath();
        QString fichierencours = QFileInfo(fileName).fileName();
        QStringList filters;
        filters << "*.mp4" << "*.mpg" << "*.m4v";
        QStringList listfich = QDir(docpath).entryList(filters,QDir::Files,QDir::Time | QDir::Reversed);
        int idx = listfich.indexOf(fichierencours);
        toolbar->First()    ->setEnabled(idx>0);
        toolbar->Prec()     ->setEnabled(idx>0);
        toolbar->Next()     ->setEnabled(idx < listfich.size()-1);
        toolbar->Last()     ->setEnabled(idx < listfich.size()-1);
        AfficheVideo(fichierencours);
        QString Base;
        if (db->getMode() == DataBase::Poste)
            Base = "BDD_POSTE";
        else if (db->getMode() == DataBase::ReseauLocal)
            Base = "BDD_LOCAL";
        else if (db->getMode() == DataBase::Distant)
            Base = "BDD_DISTANT";
        proc->gsettingsIni->setValue(Base + "/DossiersVideos", docpath);
    }
}

void dlg_docsvideo::AfficheVideo(QString filebut)
{
    upvisu     ->setAccessibleDescription(filebut);
    QDir        dirpict = QDir(docpath);
    inflabel    ->setText("<font color='magenta'>" + filebut + "</font>");

    QMediaPlayer *player = new QMediaPlayer;
    player->setVideoOutput(upvisu);
    player->setMedia(QUrl::fromLocalFile(dirpict.filePath(filebut)));
    player->play();

    //QMediaRecorder *rec = new QMediaRecorder(player);
}

void dlg_docsvideo::ValideFiche()
{
    if (typeDocCombo->currentText() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier le type de document"));
        typeDocCombo->setFocus();
        return;
    }
    if (linetitre->text() == "")
    {
        UpMessageBox::Watch(this,tr("Vous avez oublié de spécifier un nom pour le document"));
        linetitre->setFocus();
        return;
    }
    if (editdate->date() == QDate::currentDate())
    {
        editdate->setFocus();
        UpMessageBox msgbox;
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
    txtedit.setHtml(inflabel->text());
    QString filename = docpath + "/" + txtedit.toPlainText();
    QFile   qFile(filename);
    if (!qFile.open( QIODevice::ReadOnly ))
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    // on vérifie qu'un dossier par défaut a été enregistré pour l'imagerie
    QString Base, NomOnglet;
    if (db->getMode() == DataBase::Poste)          {Base = "BDD_POSTE";     NomOnglet = tr("Monoposte");}
    if (db->getMode() == DataBase::ReseauLocal)    {Base = "BDD_LOCAL";     NomOnglet = tr("Réseau local");}
    QString NomDirStockageImagerie  = proc->gsettingsIni->value(Base + "/DossierImagerie").toString();
    if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
    {
        QString msg = tr("Le dossier de sauvegarde d'imagerie ") + "<font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
        QString msg2 = tr("Renseignez un dossier valide dans Editions/Paramètres/Onglet \"ce poste\"/Onglet \"") + NomOnglet + "\"";
        UpMessageBox::Watch(this,msg, msg2);
        return;
    }
    // on vérifie que le dossier de stockage des videos existe sinon on le crée
    QString CheminVideoDir      = NomDirStockageImagerie + DIR_VIDEOS;
    QDir VideoDir;
    if (!QDir(CheminVideoDir).exists())
        if (!VideoDir.mkdir(CheminVideoDir))
        {
            QString msg = tr("Dossier de sauvegarde des videos ") + "<font color=\"red\"><b>" + CheminVideoDir + "</b></font>" + tr(" invalide");
            UpMessageBox::Watch(this,msg);
            return;
        }
    QString sstypedoc = linetitre->text();
    QString NomFileVideoDoc = QString::number(m_currentpatient->id()) + "_"
                             + sstypedoc.replace("/",".") + "_"
                             + editdate->date().toString("yyyyMMdd") + "-" + QFileInfo(qFile).created().toString("HHmmss");

    QHash<QString, QVariant> listbinds;
    bool ok;
    int idimpr =  db->selectMaxFromTable(CP_IDIMPRESSION_IMPRESSIONS, TBL_IMPRESSIONS, ok) + 1;
    NomFileVideoDoc = NomFileVideoDoc + "-" + QString::number(idimpr) + "." + QFileInfo(qFile).suffix();

    listbinds[CP_IDPAT_IMPRESSIONS] =            m_currentpatient->id();
    listbinds[CP_TYPEDOC_IMPRESSIONS] =          typeDocCombo->currentText();
    listbinds[CP_SOUSTYPEDOC_IMPRESSIONS] =      sstypedoc;
    listbinds[CP_TITRE_IMPRESSIONS] =            sstypedoc;
    listbinds[CP_DATE_IMPRESSIONS] =             editdate->date().toString("yyyy-MM-dd") + " 00:00:00";
    listbinds[CP_IDEMETTEUR_IMPRESSIONS] =       db->getUserConnected()->id();
    listbinds[CP_EMISORRECU_IMPRESSIONS] =       "0";
    listbinds[CP_FORMATAUTRE_IMPRESSIONS] =      VIDEO;
    listbinds[CP_LIENFICHIER_IMPRESSIONS] =      NomFileVideoDoc;
    listbinds[CP_FORMATDOC_IMPRESSIONS] =        VIDEO;

    DocExterne * doc = DocsExternes::CreationDocument(listbinds);
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
        proc->Message(tr("Video ") + sstypedoc +  tr(" enregistrée"), 1000, false);
        accept();
    }
}


