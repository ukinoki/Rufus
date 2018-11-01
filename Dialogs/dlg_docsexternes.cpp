/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_docsexternes.h"
#include "gbl_datas.h"
#include "icons.h"

static inline double mmToInches(double mm) { return mm * 0.039370147; }

dlg_docsexternes::dlg_docsexternes(Procedures *ProcAPasser, int idpat, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionDocsExternes", parent)
{
    proc                = ProcAPasser;
    gidPatient          = idpat;
    db                  = DataBase::getInstance();

    ScrollTable         = new UpTableWidget(this);
    Scene               = new QGraphicsScene(this);
    GraphicView         = new QGraphicsView(Scene, this);
    ListDocsTreeView    = new QTreeView(this);
    inflabel            = new QLabel();
    QFont font          = qApp->font();
    font                .setPointSize(font.pointSize()+2);
    inflabel            ->setFont(font);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    QHBoxLayout *lay   = new QHBoxLayout();
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());
    ScrollTable->horizontalHeader() ->setVisible(false);
    ScrollTable->verticalHeader()   ->setVisible(false);

    gFont = QApplication::font();
    int d=0;
#ifdef QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE
    d=2;
#endif
    gFont.setPointSize(gFont.pointSize()-d);
    ScrollTable         ->installEventFilter(this);
    GraphicView         ->installEventFilter(this);
    GraphicView         ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GraphicView         ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ScrollTable         ->setFocusPolicy(Qt::NoFocus);
    ScrollTable         ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    ScrollTable         ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setMaximumHeight(qApp->desktop()->availableGeometry().height());
    ListDocsTreeView    ->setFixedWidth(185);
    ListDocsTreeView    ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    player              = new QMediaPlayer;

    lay                 ->addWidget(ListDocsTreeView,2);
    lay                 ->addWidget(ScrollTable,8);
    lay                 ->addWidget(GraphicView,8);
    lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    lay                 ->setSpacing(10);
    globallay           ->insertLayout(0,lay);

    AllDocsupCheckBox           = new UpCheckBox(tr("Tous"));
    OnlyImportantDocsupCheckBox = new UpCheckBox(tr("Importants"));
    AllDocsupCheckBox           ->setImmediateToolTip(tr("Afficher tous les documents\ny compris les documents d'importance minime"));
    OnlyImportantDocsupCheckBox ->setImmediateToolTip(tr("N'afficher que les documents marqués importants"));
    connect (AllDocsupCheckBox,             &QCheckBox::toggled, this, [=] {FiltrerListe(AllDocsupCheckBox);});
    connect (OnlyImportantDocsupCheckBox,   &QCheckBox::toggled, this, [=] {FiltrerListe(OnlyImportantDocsupCheckBox);});
    AjouteWidgetLayButtons(AllDocsupCheckBox, false);
    AjouteWidgetLayButtons(OnlyImportantDocsupCheckBox, false);

    playctrl            = new PlayerControls(player, this);
    connect (playctrl,  &PlayerControls::ctrl,      this,   [=] {PlayerCtrl(playctrl->State());});
    connect (playctrl,  &PlayerControls::recfile,   this,   [=] {EnregistreVideo();});
    AjouteWidgetLayButtons(playctrl, false);
    sw                  = new UpSwitch(this);
    AjouteWidgetLayButtons(sw, false);
    connect(sw,         &UpSwitch::Bascule,         this,   [=] {BasculeTriListe(sw->PosSwitch());});
    AjouteLayButtons(UpDialog::ButtonSuppr | UpDialog::ButtonPrint);

    installEventFilter(this);

    gModeTri            = parDate;
    initOK = true;
    RemplirTreeView();
    if(!initOK)
        return;
    ListDocsTreeView    ->setFont(gFont);
    ListDocsTreeView    ->setEditTriggers(QAbstractItemView::DoubleClicked);
    ListDocsTreeView    ->setSelectionMode(QAbstractItemView::SingleSelection);
    ListDocsTreeView    ->setContextMenuPolicy(Qt::CustomContextMenu);
    ListDocsTreeView    ->setAnimated(true);
    ListDocsTreeView    ->setIndentation(3);
    ListDocsTreeView    ->header()->setVisible(false);

    AvecPrevisu = proc  ->ApercuAvantImpression();
    /*Gestion des XML - exemple
    QString adressexml = QDir::homePath() + NOMDIR_RUFUS + "/XML/" + QString::number(idpat) + "/Exam_Data.xml";
    QFile xmldoc(adressexml);
    if (xmldoc.open(QIODevice::ReadOnly))
    {
        QDomDocument *docxml = new QDomDocument();
        docxml->setContent(&xmldoc);
        QDomElement xml = docxml->documentElement();
        QString info;
        for (int i=0; i<xml.childNodes().size(); i++)
            info += "Nom de la balise\t" + xml.childNodes().at(i).toElement().tagName() + "\n";
        proc->Edit(info);
    }*/

    MAJTreeViewTimer    = new QTimer(this);
    MAJTreeViewTimer    ->start(10000);
    connect(MAJTreeViewTimer,   &QTimer::timeout, this,   [=] {Slot_CompteNbreDocs();});
    int margemm         = proc->TailleTopMarge(); // exprimé en mm
    printer             = new QPrinter(QPrinter::HighResolution);
    printer             ->setFullPage(true);
    rect                = printer->paperRect();

    rect.adjust(mmToInches(margemm) * printer->logicalDpiX(),
                mmToInches(margemm) * printer->logicalDpiY(),
                -mmToInches(margemm) * printer->logicalDpiX(),
                -mmToInches(margemm) * printer->logicalDpiY());
    gMode               = Normal;
    connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
    connect(SupprButton,        &QPushButton::clicked, this,   [=] {SupprimeDoc();});

    ScrollTable     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
    GraphicView     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
    hdelta          = 0;
    wdelta          = 0;
    hdeltaframe     = 0;
    wdeltaframe     = 0;
}

dlg_docsexternes::~dlg_docsexternes()
{
    proc = Q_NULLPTR;
    delete proc;
}

bool dlg_docsexternes::InitOK()
{
    return initOK;
}

void dlg_docsexternes::AfficheCustomMenu()
{
    QModelIndex idx = ListDocsTreeView->indexAt(ListDocsTreeView->mapFromGlobal(cursor().pos()));
    DocExterne *docmt = getDocumentFromIndex(idx);
    if (docmt == Q_NULLPTR)
         return;

    QMenu *menu = new QMenu(this);
    QAction *paction_ImportantMin   = new QAction;
    QAction *paction_ImportantNorm  = new QAction;
    QAction *paction_ImportantMax   = new QAction;
    QAction *paction_Modifier       = new QAction;
    paction_ImportantMin    = menu->addAction(tr("Importance faible"));
    paction_ImportantNorm   = menu->addAction(tr("Importance normale"));
    paction_ImportantMax    = menu->addAction(tr("Importance forte"));
    paction_Modifier        = menu->addAction(tr("Modifier"));
    int imptce = docmt->importance();
    QIcon icon = Icons::icBlackCheck();
    if (imptce == 0)
        paction_ImportantMin->setIcon(icon);
    else if (imptce == 1)
        paction_ImportantNorm->setIcon(icon);
    else if (imptce == 2)
        paction_ImportantMax->setIcon(icon);
    connect (paction_ImportantMin,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Min);});
    connect (paction_ImportantNorm, &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Norm);});
    connect (paction_ImportantMax,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Max);});
    connect (paction_Modifier,      &QAction::triggered,    this,  [=] {ModifierItem(idx);});
    menu->exec(cursor().pos());
}

QString dlg_docsexternes::CalcTitre(DocExterne* docmt)
{
    QString a = docmt->typedoc();                                     // 1 = Type de document
    if (docmt->soustypedoc() != "")
    {
        if (a!="") a += " - ";
        a += docmt->soustypedoc();                                    // 4 - Sous-type du document
    }
    else if (docmt->titre() != "")                                    // 2 - Titre du document
    {
        if (a!="") a += " - ";
        QTextEdit text;
        text.setHtml(docmt->titre());
        a += text.toPlainText();
    }
    return a;
}

void dlg_docsexternes::CorrigeImportance(DocExterne *docmt, enum Importance imptce)
{
    QStandardItem *item = getItemFromDocument(docmt);
    if (item == Q_NULLPTR)
        return;
    switch (imptce) {
    case Min:{
        db->StandardSQL("update " NOM_TABLE_IMPRESSIONS " set Importance = 0 where idImpression = " + QString::number(docmt->id()));
        QFont fontitem  = gFont;
        fontitem.setItalic(true);
        item->setFont(fontitem);
        docmt->setimportance(0);
        break;
    }
    case Norm:{
        db->StandardSQL("update " NOM_TABLE_IMPRESSIONS " set Importance = 1 where idImpression = " + QString::number(docmt->id()));
        item->setFont(gFont);
        docmt->setimportance(1);
        break;
    }
    case Max:{
        QFont fontitem = gFont;
        db->StandardSQL("update " NOM_TABLE_IMPRESSIONS " set Importance = 2 where idImpression = " + QString::number(docmt->id()));
        fontitem.setBold(true);
        item->setFont(fontitem);
        docmt->setimportance(2);
        break;
    }
    }
}

void dlg_docsexternes::AfficheDoc(QModelIndex idxproxy)
{
    DocExterne *docmt = getDocumentFromIndex(idxproxy);

    if (docmt == Q_NULLPTR)
    {
        PrintButton->setEnabled(false);
        SupprButton->setEnabled(false);
        return;
    }
    playctrl                    ->setVisible(false);
    PrintButton                 ->setVisible(true);
    PrintButton                 ->setEnabled(true);
    SupprButton                 ->setEnabled(true);
    QPixmap pix;
    glistPix    .clear();
    ScrollTable ->clear();
    ScrollTable ->setVisible(false);
    GraphicView ->setVisible(false);
    Scene       ->clear();
    double x;
    double y;

    if (docmt->format() == VIDEO)  // le document est une video -> n'est pas stocké dans la base mais est un fichier sur le disque
    {
        if (DataBase::getInstance()->getMode() == DataBase::Distant)
        {
            UpMessageBox::Watch(this, tr("Video non accessible en accès distant"));
            return;
        }
        QString NomOnglet, NomDirStockageImagerie;
        if (DataBase::getInstance()->getMode() == DataBase::Poste)
        {
            NomOnglet = tr("Monoposte");
            NomDirStockageImagerie = proc->DirImagerie();
        }
        if (DataBase::getInstance()->getMode() == DataBase::ReseauLocal)
        {
            NomOnglet = tr("Réseau local");
            NomDirStockageImagerie  = proc->gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();
        }
        if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
        {
            QString msg = tr("Le dossier de sauvegarde d'imagerie ") + "<font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
            QString msg2 = tr("Renseignez un dossier valide dans Editions/Paramètres/Onglet \"ce poste\"/Onglet \"") + NomOnglet + "\"";
            UpMessageBox::Watch(this,msg, msg2);
            return;
        }
        QString filename = NomDirStockageImagerie + NOMDIR_VIDEOS "/" + docmt->lienversfichier();
        QFile   qFile(filename);
        if (!qFile.open(QIODevice::ReadOnly))
        {
            UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
            return;
        }
        inflabel->setParent(GraphicView);
        QString sstitre = "<font color='magenta'>" + docmt->date().toString(tr("d-M-yyyy")) + " - " + docmt->soustypedoc() + "</font>";
        inflabel    ->setText(sstitre);

        gTypeDoc                = VIDEO;
        GraphicView             ->setVisible(true);
        videoItem               = new QGraphicsVideoItem;
        Scene                   ->addItem(videoItem);
        player                  ->setMedia(QUrl::fromLocalFile(filename));
        videoItem               ->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        player                  ->setVideoOutput(videoItem);
        playctrl                ->setPlayer(player);
        playctrl                ->setVisible(true);
        PrintButton             ->setVisible(false);
        x = videoItem->size().width();
        y = videoItem->size().height();
        idealproportion = x/y;
        videoItem               ->setSize(QSize(GraphicView->width(),GraphicView->height()));
        x = videoItem->size().width();
        y = videoItem->size().height();
        Scene->setSceneRect(1,1,x-1,y-1);
        player                  ->play();
    }
    else                                    // le document est une image ou un document écrit (ordonnance, certificat...)
    {
        bool pict = (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU);
        QMap<QString,QVariant> doc = CalcImage(docmt->id(), pict, true);
        QByteArray bapdf = doc.value("ba").toByteArray();
        if (doc.value("Type").toString() == JPG)     // le document est un JPG
        {
            inflabel->setParent(GraphicView);
            gTypeDoc                = JPG;
            GraphicView             ->setVisible(true);
            QImage image;
            if (!image.loadFromData(bapdf))
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            pix = QPixmap::fromImage(image).scaled(QSize(qApp->desktop()->availableGeometry().width(),
                                                         qApp->desktop()->availableGeometry().height()),
                                                   Qt::KeepAspectRatioByExpanding,
                                                   Qt::SmoothTransformation);
            x = pix.size().width();
            y = pix.size().height();
            idealproportion = x/y;
            glistPix << pix;
            pix = QPixmap::fromImage(image).scaled(QSize(GraphicView->width()-2,GraphicView->height()-2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            Scene   ->addPixmap(pix);
            x = pix.size().width();
            y = pix.size().height();
            Scene   ->setSceneRect(1,1,x-1,y-1);
        }
        else if (doc.value("Type").toString() == PDF)     // le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage
        {
            Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
            if (!document || document->isLocked()) {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                delete document;
                return;
            }
            if (document == Q_NULLPTR) {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                delete document;
                return;
            }
            inflabel->setParent(ScrollTable);
            gTypeDoc    = PDF;
            ScrollTable ->setVisible(true);
            ScrollTable ->setColumnCount(1);
            ScrollTable ->setColumnWidth(0,ScrollTable->width()-2);
            document->setRenderHint(Poppler::Document::TextAntialiasing);
            int numpages = document->numPages();
            for (int i=0; i<numpages ;i++)
            {
                ScrollTable->setRowCount(numpages);
                Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
                if (pdfPage == Q_NULLPTR) {
                    UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                    delete document;
                    return;
                }
                QImage image = pdfPage->renderToImage(300,300);
                if (image.isNull()) {
                    UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                    delete document;
                    return;
                }
                // ... use image ...
                pix = QPixmap::fromImage(image).scaled(QSize(qApp->desktop()->availableGeometry().width(),
                                                             qApp->desktop()->availableGeometry().height()),
                                                       Qt::KeepAspectRatioByExpanding,
                                                       Qt::SmoothTransformation);
                if (i==0)
                {
                    x = pix.size().width();
                    y = pix.size().height();
                    idealproportion = x/y;
                }
                glistPix << pix;
                pix = QPixmap::fromImage(image).scaled(QSize(ScrollTable->width()-2,ScrollTable->height()-2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
                ScrollTable->setRowHeight(i,pix.height());
                UpLabel *lab = new UpLabel(ScrollTable);
                lab->resize(pix.width(),pix.height());
                lab->setPixmap(pix);
                connect(lab,    &UpLabel::clicked, this,   [=] {ZoomDoc();});
                delete pdfPage;
                ScrollTable->setCellWidget(i,0,lab);
            }
            delete document;
        }
    }
    if (gMode == Zoom)
    {
        // les dimensions maxi de la zone de visu
        const double maxwscroll  = qApp->desktop()->availableGeometry().width()*2/3    - wdelta - wdeltaframe;
        const double maxhscroll  = qApp->desktop()->availableGeometry().height()       - hdelta - hdeltaframe;
        // les dimensions calculées de la zone de visu
        int wfinal(0), hfinal(0);

        double proportion = maxwscroll/maxhscroll;
        if (idealproportion > proportion)
        {   wfinal  = int(maxwscroll);   hfinal  = int(wfinal / idealproportion); }
        else
        {   hfinal  = int(maxhscroll);   wfinal  = int(hfinal * idealproportion); }

        int w = wfinal + wdelta;
        int h = hfinal + hdelta;
        resize(w, h);
        if (gTypeDoc == PDF)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(glistPix.at(i).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    ScrollTable->setRowHeight(i,hfinal-2);
                    ScrollTable->setColumnWidth(0,wfinal-2);
                    if (i==0) ScrollTable->resize(wfinal, hfinal);
                }
            }
        }
        else if (gTypeDoc == VIDEO)
        {
            GraphicView ->resize(wfinal, hfinal);
            videoItem   ->setSize(QSize(wfinal-2, hfinal-2));
            Scene->setSceneRect(1,1,wfinal-1,hfinal-1);
        }
        else if (gTypeDoc == JPG)
        {
            GraphicView ->resize(wfinal, hfinal);
            Scene->clear();
            QPixmap pix = glistPix.at(0).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            Scene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            Scene->setSceneRect(1,1,x-1,y-1);
        }

        if ((w + wdeltaframe) > (qApp->desktop()->availableGeometry().width() - this->x()))
            move(qApp->desktop()->availableGeometry().width() - (w + wdeltaframe), 0);
    }
    if (gTypeDoc == PDF)
        inflabel    ->setGeometry(10,ScrollTable->viewport()->height()-40, 500, 25);
    else
        inflabel    ->setGeometry(10,GraphicView->height() -40, 500, 25);
}

void dlg_docsexternes::BasculeTriListe(int a)
{
    QString             idimpraretrouver = "";
    ListDocsTreeView    ->disconnect();
    if (ListDocsTreeView->selectionModel()->selectedIndexes().size()>0)
    {
        QModelIndex actifidx = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
        if (!gmodele->itemFromIndex(actifidx)->hasChildren())
            idimpraretrouver = gmodele->itemFromIndex(actifidx)->data().toMap().value("id").toString();
    }
    gmodele             = new QStandardItemModel(this);
     if (a == 0)
    {
        gModeTri = parDate;
        gmodele  = gmodeleTriParDate;
    }
    else if (a == 1)
    {
        gModeTri = parType;
        gmodele = gmodeleTriParType;
    }

    QItemSelectionModel *m = ListDocsTreeView->selectionModel();
    ListDocsTreeView->setModel(gmodele);
    delete m;

    int nrows = gmodele->item(gmodele->rowCount()-1)->rowCount()-1;                 // le nombre de child du dernier item date
    QStandardItem *item =  gmodele->item(gmodele->rowCount()-1)->child(nrows,0);    // le tout dernier item
    QModelIndex idx = item->index();                                                // l'index de ce dernier item
    if (idimpraretrouver != "")
    {
        QModelIndex indx = getIndexFromId(idimpraretrouver.toInt());
        if (indx.isValid())
            idx = indx;
    }
    ListDocsTreeView->setSelectionModel(new QItemSelectionModel(gmodele));
    ListDocsTreeView->expandAll();
    ListDocsTreeView->scrollTo(idx, QAbstractItemView::EnsureVisible);
    ListDocsTreeView->setCurrentIndex(idx);
    connect(ListDocsTreeView->selectionModel()  , &QItemSelectionModel::currentChanged,     this,   [=] {AfficheDoc(ListDocsTreeView->selectionModel()->currentIndex());});
}

void dlg_docsexternes::Slot_CompteNbreDocs()
{
    int ndocs = CompteNbreDocs();
    if (ndocs == 0)
        reject();
    if (nbredocs != ndocs)
    {
        nbredocs = ndocs;
        RemplirTreeView();
    }
}

void dlg_docsexternes::EnregistreVideo()
{
    QString filename = player->media().canonicalUrl().path();
    QFileDialog dialog(this, tr("Enregistrer un fichier"), QDir::homePath());
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        QFile(filename).copy(dockdir.path() + "/" + player->media().canonicalUrl().fileName());
    }
}

void dlg_docsexternes::FiltrerListe(UpCheckBox *chk)
{
    if (chk == OnlyImportantDocsupCheckBox)
    {
        if (chk->isChecked())
            AllDocsupCheckBox->setChecked(false);
    }
    if (chk == AllDocsupCheckBox)
    {
        if (chk->isChecked())
            OnlyImportantDocsupCheckBox->setChecked(false);
    }
}

bool dlg_docsexternes::EcritDansUnFichier(QString NomFichier, QByteArray TexteFichier)
{
    //syntaxe = EcritDansUnFichier(QDir::homePath()+ "/Documents/test.txt", texte);
    QDir DirRssces;
    if (!DirRssces.exists(QDir::homePath() + "/Documents/Rufus/Ressources"))
        DirRssces.mkdir(QDir::homePath() + "/Documents/Rufus/Ressources");
    QFile testfile(NomFichier);
    if (!testfile.open(QIODevice::ReadWrite))
    {
        UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'ouvrir le fichier\n") + NomFichier);
        return false;
    }
    QTextStream out(&testfile);
    out << TexteFichier;
    return true;
}

QMap<QString,QVariant> dlg_docsexternes::CalcImage(int idimpression, bool imagerie, bool afficher)
{
    /* la fonction est appelée par Slot_AfficheDoc(), on utilise la table impressions
     *      pour afficher un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour afficher un document d'imagerie stocké directement dans la base, dans la table impressions - on va extraire le ByteArray directement de la base, de la table impressions
     * la fonction est applée par ImprimeDoc() - on utilise la table echangeimages
     *      pour imprimer un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour imprimer un document d'imagerie stocké dans la table echangeimages - on va extraire le ByteArray directement de la base, de la table echangeimages
    */
    DocExterne *docmt = m_ListDocs.getDocumentById(idimpression);
    QMap<QString,QVariant> result;
    QString idimpr = QString::number(idimpression);
    QString filename = "";
    QByteArray bapdf;
    result["Type"]    = "";
    result["ba"]      = QByteArray("");
    if (imagerie)
    {
        if (afficher)
        {
            QString sstitre = "<font color='magenta'>" + docmt->date().toString(tr("d-M-yyyy")) + " - " + docmt->typedoc() + " - " + docmt->soustypedoc() + "</font>";
            inflabel    ->setText(sstitre);
            filename = docmt->lienversfichier();
            if (filename != "")
            {
                QString filesufx;
                if (filename.contains("."))
                {
                    QStringList lst = filename.split(".");
                    filesufx        = lst.at(lst.size()-1);
                }
                QString sfx = (filesufx == PDF? PDF : JPG);
                QString imgs = "select idimpression from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr + " and (pdf is not null or jpg is not null)";
                //qDebug() << imgs;
                bool ok = false;
                QList<QList<QVariant>> listimpr = db->StandardSelectSQL(imgs, ok);
                if (!ok)
                    UpMessageBox::Watch(this, tr("Impossible d'accéder à la table ") + NOM_TABLE_ECHANGEIMAGES);
                if (listimpr.size()==0)
                {
                    db->StandardSQL("delete from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr);
                    QString req = "INSERT INTO " NOM_TABLE_ECHANGEIMAGES " (idimpression, " + sfx + ", compression) "
                                "VALUES (" +
                                idimpr + ", " +
                                " LOAD_FILE('" + proc->DirImagerie() + NOMDIR_IMAGES + filename + "'), " +
                                QString::number(docmt->compression()) + ")";
                    //qDebug() << req;
                    db->StandardSQL(req);
                }
            }
        }
        bool ok = false;
        QList<QList<QVariant>> listimpr = db->StandardSelectSQL("select pdf, jpg, compression  from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr
                                                                , ok
                                                                , tr("Impossible d'accéder à la table ") + NOM_TABLE_ECHANGEIMAGES);
        if (!ok)
            return result;
        if (listimpr.size()==0)                             // le document n'est pas dans echangeimages, on va le chercher dans impressions
        {
            listimpr = db->StandardSelectSQL("select pdf, jpg, compression  from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr
                                             , ok
                                             , tr("Impossible d'accéder à la table ") + NOM_TABLE_IMPRESSIONS);
        }
        if (listimpr.size()==0)
            return result;
        QList<QVariant> impr = listimpr.at(0);
        if (impr.at(0).toByteArray().size()>0)                                                 // c'est un pdf
        {
            if (impr.at(2).toString()=="1")
                bapdf.append(qUncompress(impr.at(0).toByteArray()));
            else
                bapdf.append(impr.at(0).toByteArray());
            result["Type"]    = PDF;
            result["ba"]      = bapdf;
        }
        else if (impr.at(1).toByteArray().size()>0)                                            // c'est un jpg
        {
            bapdf.append(impr.at(1).toByteArray());
            result["Type"]    = JPG;
            result["ba"]      = bapdf;
        }
    }
    else                                                                                                    // il s'agit d'un document écrit, on le traduit en pdf et on l'affiche
    {
        inflabel    ->setText("");
        QByteArray bapdf;
        QString Entete  = docmt->textentete();
        QString Corps   = docmt->textcorps();
        QString Pied    = docmt->textpied();
        QTextEdit   *Etat_textEdit = new QTextEdit;
        Etat_textEdit->setHtml(Corps);
        TextPrinter *TexteAImprimer = new TextPrinter();
        if (docmt->format() == PRESCRIPTIONLUNETTES)
            TexteAImprimer->setFooterSize(proc->TaillePieddePageOrdoLunettes());
        else
            TexteAImprimer->setFooterSize(proc->TaillePieddePage());
        TexteAImprimer->setHeaderText(Entete);
        int TailleEnTete = proc->TailleEnTete();
        if (docmt->isALD()) TailleEnTete = proc->TailleEnTeteALD();
        TexteAImprimer->setHeaderSize(TailleEnTete);
        TexteAImprimer->setFooterText(Pied);
        TexteAImprimer->setTopMargin(proc->TailleTopMarge());
        TexteAImprimer->print(Etat_textEdit->document(), NOMFIC_PDF, "", false, true);
        // le paramètre true de la fonction print() génère la création du fichier pdf NOMFIC_PDF et pas son impression
        QString ficpdf = QDir::homePath() + NOMFIC_PDF;
        QFile filepdf(ficpdf);
        if (!filepdf.open( QIODevice::ReadOnly ))
            UpMessageBox::Watch(Q_NULLPTR,  tr("Erreur d'accès au fichier:\n") + ficpdf, tr("Impossible d'enregistrer l'impression dans la base"));
        bapdf = filepdf.readAll();
        filepdf.close ();
        result["Type"]    = PDF;
        result["ba"]      = bapdf;
    }
    return result;
}

int dlg_docsexternes::CompteNbreDocs()
{
    bool ok = true;
    QList<QList<QVariant>> list = db->StandardSelectSQL("Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient), ok);
    if (!ok) return 0;
    return list.size();
}

DocExterne* dlg_docsexternes::getDocumentFromIndex(QModelIndex idx)
{
    QStandardItem *it = gmodele->itemFromIndex(idx);
    if (it == Q_NULLPTR || it->hasChildren())
        return Q_NULLPTR;
    int idimpr = it->data().toMap().value("id").toInt();
    return m_ListDocs.getDocumentById(idimpr);
}

QModelIndex dlg_docsexternes::getIndexFromId(int id)
{
    QModelIndex idx;
    for (int m = 0; m<gmodele->rowCount(); m++)
        for (int n=0; n<gmodele->item(m)->rowCount(); n++)
            if (gmodele->item(m)->child(n)->data().toMap().value("id").toInt() == id)
            {
                idx = gmodele->item(m)->child(n)->index();
                break;
            }
    return idx;
}

QStandardItem* dlg_docsexternes::getItemFromDocument(DocExterne* docmt)
{
    QModelIndex idx = getIndexFromId(docmt->id());
    return gmodele->itemFromIndex(idx);
}

void dlg_docsexternes::ImprimeDoc()
{
#ifndef QT_NO_PRINTER
    PrintButton         ->disconnect();
    AvecPrevisu         = proc->ApercuAvantImpression();
    DocExterne * docmt  = getDocumentFromIndex(ListDocsTreeView->selectionModel()->selectedIndexes().at(0));
    docmt               = m_ListDocs.getDocumentById(docmt->id());
    if (docmt != Q_NULLPTR)
    {
        bool detruirealafin = false;


        UpMessageBox msgbox;
        UpSmallButton OKBouton      (tr("Réimprimer\nle document"));
        UpSmallButton NoBouton      (tr("Modifier\net imprimer"));
        UpSmallButton ImpBouton     (tr("Réimprimer à\nla date d'aujourd'hui"));
        UpSmallButton AnnulBouton   (tr("Annuler"));
        msgbox.setText(tr("Réimprimer un document"));
        msgbox.setIcon(UpMessageBox::Print);

        msgbox.addButton(&AnnulBouton,UpSmallButton::CANCELBUTTON);
        if (proc->getUserConnected()->isMedecin()
            && (docmt->format() != IMAGERIE && docmt->format() != DOCUMENTRECU))
        {
            if (QDate::currentDate() == docmt->date().date())
            {
                msgbox.addButton(&NoBouton,UpSmallButton::EDITBUTTON);
                detruirealafin = true;
            }
            else
            {
                if (docmt->textorigine() != "")    // si on a un texte d'origine, on peut modifier le document
                {
                    NoBouton.setText(tr("Modifier et imprimer\nà la date d'aujourd'hui"));
                    msgbox.addButton(&NoBouton,UpSmallButton::EDITBUTTON);
                }
                msgbox.addButton(&ImpBouton,UpSmallButton::COPYBUTTON);
            }
        }
        msgbox.addButton(&OKBouton, UpSmallButton::PRINTBUTTON);
        msgbox.exec();

        //Reimpression simple du document, sans réédition => pas d'action sur la BDD
        if (msgbox.clickedButton() == &OKBouton)
        {
            bool pict = (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU);
            QMap<QString,QVariant> doc = CalcImage(docmt->id(), pict, false);
            QByteArray bapdf = doc.value("ba").toByteArray();
            if (doc.value("Type").toString() == PDF)     // le document est un pdf ou un document texte
            {
                Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
                if (!document || document->isLocked()) {
                    UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                    delete document;
                    connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
                    return;
                }
                if (document == Q_NULLPTR) {
                    UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                    delete document;
                    connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
                    return;
                }

                document->setRenderHint(Poppler::Document::TextAntialiasing);
                int numpages = document->numPages();
                for (int i=0; i<numpages ;i++)
                {
                    Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
                    if (pdfPage == Q_NULLPTR) {
                        UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                        delete document;
                        connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
                        return;
                    }
                    image = pdfPage->renderToImage(600,600);
                    if (image.isNull()) {
                        UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                        delete document;
                        connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
                        return;
                    }
                    // ... use image ...
                    if (i == 0)
                    {
                        if (AvecPrevisu)
                        {
                            QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
                            connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(printer);});
                            dialog->exec();
                            delete dialog;
                        }
                        else
                        {
                            QPrintDialog *dialog = new QPrintDialog(printer, this);
                            if (dialog->exec() != QDialog::Rejected)
                                Print(printer);
                            delete dialog;
                            AvecPrevisu = proc->ApercuAvantImpression();
                        }
                    }
                    else
                        Print(printer);
                    delete pdfPage;
                }
                delete document;
            }
            if (doc.value("Type").toString() == JPG)     // le document est un jpg
            {
                QPixmap pix;
                pix.loadFromData(bapdf);
                image= pix.toImage();
                if (AvecPrevisu)
                {
                    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
                    connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(printer);});
                    dialog->exec();
                    delete dialog;
                }
                else
                {
                    QPrintDialog *dialog = new QPrintDialog(printer, this);
                    if (dialog->exec() != QDialog::Rejected)
                        Print(printer);
                    delete dialog;
                    AvecPrevisu = proc->ApercuAvantImpression();
                }
            }
        }

        //Réédition d'un document - on va réimprimer le document à la datedu jour en le modifiant - ne concerne que les courriers et ordonnances émis => on enregistre le nouveau document dans la BDD
        else if (msgbox.clickedButton() == &NoBouton || msgbox.clickedButton() == &ImpBouton)
        {
            // reconstruire le document en refaisant l'entête et en récupérant le corps et le pied enregistrés dans la base

            QString     Corps, Entete, Pied, txt;
            QTextEdit   *Etat_textEdit  = new QTextEdit;
            bool        AvecNumPage     = false;
            bool        aa;
            bool        ALD             = (docmt->isALD());
            bool        Prescription    = (docmt->format() == PRESCRIPTION || docmt->format() == PRESCRIPTIONLUNETTES);
            Patient*    pat = DataBase::getInstance()->loadPatientById(gidPatient);

            User *userEntete = proc->setDataOtherUser(docmt->iduser());
            if (userEntete == nullptr)
            {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
                return;
            }

            //création de l'entête
            QMap<QString,QString> EnteteMap = proc->ImpressionEntete(QDate::currentDate(), userEntete);
            Entete = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
            if (Entete == "") return;
            Entete.replace("{{TITRE1}}"        , "");
            Entete.replace("{{TITRE}}"         , "");
            Entete.replace("{{DDN}}"           , "");
            Entete.replace("{{PRENOM PATIENT}}", (Prescription? pat->prenom()        : ""));
            Entete.replace("{{NOM PATIENT}}"   , (Prescription? pat->nom().toUpper() : ""));

            //création du pied
            Pied = proc->ImpressionPied(docmt->format() == PRESCRIPTIONLUNETTES, ALD);

            // creation du corps de l'ordonnance
            QString txtautiliser    = (docmt->textorigine() != ""?              docmt->textcorps()              : docmt->textorigine());
            txt                     = (msgbox.clickedButton() == &NoBouton?     proc->Edit(txtautiliser)        : txtautiliser);
            Corps                   = (docmt->typedoc() == tr("Prescription")?  proc->ImpressionCorps(txt,ALD)  : proc->ImpressionCorps(txt));
            Etat_textEdit           ->setHtml(Corps);
            if (Etat_textEdit->toPlainText() == "")
            {   UpMessageBox::Watch(this,tr("Rien à imprimer"));    return; }
            int TailleEnTete        = (ALD?                                     proc->TailleEnTeteALD()         : proc->TailleEnTete());
            bool AvecDupli          = (proc->gsettingsIni->value("Param_Imprimante/OrdoAvecDupli").toString() == "YES" && docmt->typedoc() == tr("Prescription"));
            bool AvecChoixImprimante = true;

            aa = proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                                    proc->TaillePieddePage(), TailleEnTete, proc->TailleTopMarge(),
                                    AvecDupli, AvecPrevisu, AvecNumPage, AvecChoixImprimante);

            // stockage du document dans la base de donnees - table impressions
            if (aa)
            {
                bool a = true;
                while (a) {
                    // il faut retirer la dernière ligne du html qui contient le retour à la ligne
                    int debut = Corps.lastIndexOf("<p");
                    int fin   = Corps.lastIndexOf("</p>");
                    int longARetirer = fin - debut + 4;
                    if (Corps.mid(debut,longARetirer).contains("-qt-paragraph-type:empty;"))
                        Corps.remove(debut,longARetirer);
                    else a = false;
                }
                Corps.replace("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
                Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");

                QSqlQuery query = QSqlQuery(DataBase::getInstance()->getDataBase());
                // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
                query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, SousTypeDoc, Titre, TextEntete, TextCorps, TextOrigine, TextPied, Dateimpression, FormatDoc, idLieu, ALD, UserEmetteur, Importance)"
                                                                   " values(:iduser, :idpat, :typeDoc, :soustypedoc, :titre, :textEntete, :textCorps, :textOrigine, :textPied, :dateimpression, :formatdoc, :idlieu, :ald, :useremetteur, :importance)");
                query.bindValue(":iduser", QString::number(docmt->iduser()));
                query.bindValue(":idpat", QString::number(docmt->idpatient()));
                query.bindValue(":typeDoc", docmt->typedoc());
                query.bindValue(":soustypedoc", docmt->soustypedoc());
                query.bindValue(":titre", docmt->titre());
                query.bindValue(":textEntete", Entete);
                query.bindValue(":textCorps", Corps);
                query.bindValue(":textOrigine", txt);
                query.bindValue(":textPied", Pied);
                query.bindValue(":dateimpression", QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
                query.bindValue(":formatdoc", docmt->format());
                query.bindValue(":idlieu", QString::number(DataBase::getInstance()->getUserConnected()->getSite()->id()));
                query.bindValue(":ald", ALD? "1" : QVariant(QVariant::String));
                query.bindValue(":useremetteur", QString::number(DataBase::getInstance()->getUserConnected()->id()));
                query.bindValue(":importance", docmt->importance());

                /*
                query.bindValue(":iduser", quer.value(0).toString());
                query.bindValue(":idpat", QString::number(gidPatient));
                query.bindValue(":typeDoc", quer.value(2).toString());
                query.bindValue(":soustypedoc", quer.value(9).toString());
                query.bindValue(":titre", quer.value(1).toString());
                query.bindValue(":textEntete", Entete);
                query.bindValue(":textCorps", Corps);
                query.bindValue(":textOrigine", txt);
                query.bindValue(":textPied", Pied);
                query.bindValue(":dateimpression", QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
                query.bindValue(":formatdoc", quer.value(8).toString());
                query.bindValue(":idlieu", QString::number(DataBase::getInstance()->getUserConnected()->getSite()->id()));
                query.bindValue(":ald", ALD? "1" : QVariant(QVariant::String));
                query.bindValue(":useremetteur", QString::number(DataBase::getInstance()->getUserConnected()->id()));
                query.bindValue(":importance", quer.value(10).toInt());
                */


                if(!query.exec())
                {
                    UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
                    connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
                }
                RemplirTreeView();
                if (detruirealafin)
                    db->SupprRecordFromTable(docmt->id(),"idimpression",NOM_TABLE_IMPRESSIONS);
            }
            delete Etat_textEdit;
        }
        msgbox.close();
    }
    connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
#endif
}

int dlg_docsexternes::initListDocs()
{
    m_ListDocs.VideLesListes();
    QList<DocExterne*> listdocsexternesbydate = db->loadDoscExternesByDateByPatientAll(gidPatient);
    m_ListDocs.addListDocsExternesByDat(listdocsexternesbydate);                            // liste des documents classés par date

    QList<DocExterne*> listdocsexternesbytype = db->loadDoscExternesByTypeByPatientAll(gidPatient);
    m_ListDocs.addListDocsExternesByTyp(listdocsexternesbytype);                            // liste des documetns classés par date

    return m_ListDocs.docsexternespardate().size();
}

void dlg_docsexternes::ModifierItem(QModelIndex idx)
{
    DocExterne *docmt = getDocumentFromIndex(idx);
    UpDialog * dlg              = new UpDialog();
    dlg                         ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
    QVBoxLayout *globallay      = dynamic_cast<QVBoxLayout*>(dlg->layout());
    UpLineEdit  *Line           = new UpLineEdit(dlg);
    UpLabel     *label          = new UpLabel(dlg);
    globallay->setSpacing(4);
    globallay->insertWidget(0,Line);
    globallay->insertWidget(0,label);
    dlg->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);

    dlg->setModal(true);
    dlg->setSizeGripEnabled(false);
    dlg->setFixedSize(270,100);
    dlg->move(QPoint(x()+width()/2,y()+height()/2));
    dlg->setWindowTitle(tr("Modifier le titre"));
    Line->setText(docmt->soustypedoc());
    QTimer::singleShot(0, Line, &QLineEdit::selectAll);

    connect(dlg->OKButton,   &QPushButton::clicked,   [=]
    {
        if (Line->text()!="")
        {
            db->StandardSQL("update " NOM_TABLE_IMPRESSIONS " set soustypedoc = '" + Line->text() + "' where idimpression = " + QString::number(docmt->id()));
            gmodele->itemFromIndex(idx)->setText(CalcTitre(m_ListDocs.reloadDocument(docmt)));
            dlg->accept();
        }
        else
        {
            QSound::play(NOM_ALARME);
            QToolTip::showText(cursor().pos(),tr("Vous devez entrer du texte"));
        }
    });

    label->setText(tr("Entrez le titre du document"));
    Line->setValidator(new QRegExpValidator(Utils::rgx_adresse,this));
    Line->setMaxLength(60);
    dlg->exec();
    delete dlg;
}

void dlg_docsexternes::PlayerCtrl(int ctrl)
{
    switch (ctrl){
    case 0: player->stop();     break;
    case 1: player->pause();    break;
    case 2:
        player     ->setVideoOutput(videoItem);
        player->play();
        break;
    case 3:
        player  ->stop();
        player  ->setVideoOutput(videoItem);
        player  ->play();
        break;
    }
}

void dlg_docsexternes::Print(QPrinter *Imprimante)
{
    QPainter PrintingPreView(Imprimante);
    QPixmap pix         = QPixmap::fromImage(image).scaledToWidth(int(rect.width()),Qt::SmoothTransformation);
    PrintingPreView.drawImage(QPoint(0,0),pix.toImage());
}

void dlg_docsexternes::SupprimeDoc()
{
    int ndocs = CompteNbreDocs();
    QModelIndex idx = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
    DocExterne *docmt = getDocumentFromIndex(idx);
    QString idimpr = QString::number(docmt->id());
    if (!proc->getUserConnected()->isSoignant())         //le user n'est pas un soignant
    {
        if (docmt->useremetteur() != DataBase::getInstance()->getUserConnected()->id())
        {
            UpMessageBox::Watch(this,tr("Suppression refusée"), tr("Vous ne pouvez pas supprimer un document dont vous n'êtes pas l'auteur"));
            return;
        }
    }

    if (idimpr != "")
    {
        UpMessageBox msgbox;
        UpSmallButton OKBouton(tr("Supprimer"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.setText("Euuhh... " + proc->getUserConnected()->getLogin());
        msgbox.setInformativeText(tr("Etes vous certain de vouloir supprimer ce document?"));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(&NoBouton,UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            return;
        if (docmt->lienversfichier() != "")
        {
            QString filename = (docmt->format() == VIDEO? "/" : "") + docmt->lienversfichier();
            QString cheminFichier = (docmt->format()== VIDEO? NOMDIR_VIDEOS : NOMDIR_IMAGES);
            filename = cheminFichier + filename;
            db->StandardSQL("insert into " NOM_TABLE_DOCSASUPPRIMER " (FilePath) VALUES ('" + filename + "')");
        }
        QString idaafficher = "";
        if (ndocs>1)    // on recherche le document sur qui va être mis en surbrillance après la suppression
        {
            QHash<int, DocExterne*> listaexplorer = (gModeTri == parDate? m_ListDocs.docsexternespardate() : m_ListDocs.docsexternespartype());
            QHash<int, DocExterne*>::const_iterator itdoc = listaexplorer.find(docmt->idpatient());
            if (itdoc == listaexplorer.constBegin())
                ++itdoc;
            else
                --itdoc;
            idaafficher = QString::number(itdoc.value()->id());
        }
        db->StandardSQL("delete from " NOM_TABLE_REFRACTION " where idrefraction = (select idrefraction from " NOM_TABLE_IMPRESSIONS
                        " where idimpression = " + idimpr + ")");
        db->StandardSQL("delete from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr);
        gmodele->removeRow(idx.row(),idx);
        QModelIndex idx2;
        if (idaafficher != "")
        {
            QModelIndex indx = getIndexFromId(idaafficher.toInt());
            if (indx.isValid())
                idx2 = indx;
        }
        if (ndocs==1)
            reject();
        ListDocsTreeView->reset();
        ListDocsTreeView->expandAll();
        ListDocsTreeView->scrollTo(idx2, QAbstractItemView::EnsureVisible);
        ListDocsTreeView->setCurrentIndex(idx2);
        RemplirTreeView();
     }
}

void dlg_docsexternes::ZoomDoc()
{
    if (gMode == Normal)
    {
        PosOrigine      = pos();
        SizeOrigine     = size();
        gMode           = Zoom;
        ScrollTable     ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //WARNING : icon scaled : pxZoomOut 30,30
        GraphicView     ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //WARNING : icon scaled : pxZoomOut 30,30

        if (hdeltaframe == 0)   hdeltaframe = frameGeometry().height() - height();
        if (wdeltaframe == 0)   wdeltaframe = frameGeometry().width()  - width();
        if (hdelta == 0)
        {
            if (gTypeDoc == PDF)    hdelta  = height() - ScrollTable->height();
            else                    hdelta  = height() - GraphicView->height();
        }
        if (wdelta == 0)
        {
            if (gTypeDoc == PDF)    wdelta  = width() - ScrollTable->width();
            else                    wdelta  = width() - GraphicView->width();
        }

        // les dimensions maxi de la zone de visu
        const double maxwscroll  = qApp->desktop()->availableGeometry().width()*2/3    - wdelta - wdeltaframe;
        const double maxhscroll  = qApp->desktop()->availableGeometry().height()       - hdelta - hdeltaframe;
        // les dimensions calculées de la zone de visu
        int wfinal(0), hfinal(0);

        double proportion = maxwscroll/maxhscroll;
        if (idealproportion > proportion)
        {   wfinal  = int(maxwscroll);   hfinal  = int(wfinal / idealproportion); }
        else
        {   hfinal  = int(maxhscroll);   wfinal  = int(hfinal * idealproportion); }
        int w = wfinal + wdelta;
        int h = hfinal + hdelta;
        resize(w, h);

        if (gTypeDoc == PDF)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(glistPix.at(i).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    ScrollTable->setRowHeight(i,hfinal-2);
                    ScrollTable->setColumnWidth(0,wfinal-2);
                    if (i==0) ScrollTable->resize(wfinal, hfinal);
                }
            }
        }
        else if (gTypeDoc == JPG)
        {
            Scene->clear();
            QPixmap pix = glistPix.at(0).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            Scene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            Scene->setSceneRect(1,1,x-1,y-1);
        }
        else if (gTypeDoc == VIDEO)
        {
            if (Scene->items().size()>0)
            {
                if (dynamic_cast<QGraphicsVideoItem*>(Scene->items().at(0)) != Q_NULLPTR)
                    videoItem   ->setSize(QSize(wfinal-2,hfinal-2));
                int x = int(videoItem->size().width());
                int y = int(videoItem->size().height());
                Scene->setSceneRect(1,1,x-1,y-1);
            }
        }
        move (qApp->desktop()->availableGeometry().width() - w, 0);
    }
    else if (gMode == Zoom)
    {
        ScrollTable     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
        GraphicView     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
        move(PosOrigine);
        resize(SizeOrigine);
        if (gTypeDoc == PDF)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    QPixmap pix = glistPix.at(i).scaled(SizeOrigine.width() - wdelta - 2, SizeOrigine.height() - hdelta - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    lbl->setPixmap(pix);
                    ScrollTable->setRowHeight(i,pix.height());
                    ScrollTable->setColumnWidth(0,pix.width());
                    ScrollTable->resize(SizeOrigine.width()-wdelta, SizeOrigine.height()-hdelta);
                }
            }
        }
        else if (gTypeDoc == JPG)
        {
            Scene->clear();
            QPixmap pix = glistPix.at(0).scaled(SizeOrigine.width() - wdelta - 2, SizeOrigine.height() - hdelta - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            Scene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            Scene->setSceneRect(1,1,x-1,y-1);
        }
        else if (gTypeDoc == VIDEO)
        {
            if (Scene->items().size()>0)
            {
                if (dynamic_cast<QGraphicsVideoItem*>(Scene->items().at(0)) != Q_NULLPTR)
                    videoItem   ->setSize(QSize(SizeOrigine.width() - wdelta - 2, SizeOrigine.height() - hdelta - 2));
                int x = int(videoItem->size().width());
                int y = int(videoItem->size().height());
                Scene->setSceneRect(1,1,x-1,y-1);
            }
        }
        gMode           = Normal;
    }
    setEnregPosition(gMode == Normal);
    inflabel    ->move(10, (gTypeDoc == PDF? ScrollTable->height() : GraphicView->height())-40);
}

bool dlg_docsexternes::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==ScrollTable)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            for (int i=0; i < ScrollTable->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(ScrollTable->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(glistPix.at(i).scaled(ScrollTable->width(), ScrollTable->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    ScrollTable->setRowHeight(i,lbl->pixmap()->height());
                    ScrollTable->setColumnWidth(i,lbl->pixmap()->width());
                }
            }
            inflabel    ->move(10,ScrollTable->viewport()->height()-40);
        }
    }
    if (obj == GraphicView)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            if (gTypeDoc == JPG)
            {
                if (Scene->items().size()>0)
                {
                    Scene->clear();
                    QPixmap pix = glistPix.at(0).scaled(GraphicView->width(), GraphicView->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    Scene->addPixmap(pix);
                    int x = pix.size().width();
                    int y = pix.size().height();
                    Scene->setSceneRect(1,1,x-1,y-1);
                 }
            }
            else if (gTypeDoc == VIDEO)
            {
                if (Scene->items().size()>0)
                {
                    if (dynamic_cast<QGraphicsVideoItem*>(Scene->items().at(0)) != Q_NULLPTR)
                        videoItem   ->setSize(QSize(GraphicView->width(),GraphicView->height()));
                    int x = int(videoItem->size().width());
                    int y = int(videoItem->size().height());
                    Scene->setSceneRect(1,1,x-1,y-1);
                }
            }
            inflabel    ->move(10,GraphicView->height()-40);
        }
        if (event->type() == QEvent::MouseButtonPress)
        {
            //static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
            //qDebug() << QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(event->type());
            ZoomDoc();;
        }

     }
    return QWidget::eventFilter(obj, event);
}

void dlg_docsexternes::RemplirTreeView()
{
    /*
     * 0-idImpression
     * 1-idUser
     * 2-idpat
     * 3-TypeDoc
     * 4-Titre
     * 5-TextEntete
     * 6-TextCorps
     * 7-TextPied
     * 8-Dateimpression
     * 9-pdf
     * 10-formatautre
     * 11-lienversfichier
    */
    int                 ndocs = 0;
    if (ListDocsTreeView != Q_NULLPTR)
        ListDocsTreeView->disconnect();
    if (ListDocsTreeView->selectionModel() != Q_NULLPTR)
        ListDocsTreeView->selectionModel()->disconnect();
    QStandardItem       *dateitem, *typitem, *pitem;
    QString             idimpraretrouver = "";
    gmodele = dynamic_cast<QStandardItemModel*>(ListDocsTreeView->model());
    if (gmodele)
    {
        if (ListDocsTreeView->selectionModel()->selectedIndexes().size()>0)
        {
            QModelIndex actifidx = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
            if (!gmodele->itemFromIndex(actifidx)->hasChildren())
                idimpraretrouver = gmodele->itemFromIndex(actifidx)->data().toMap().value("id").toString();
        }
        gmodele->clear();
        gmodeleTriParDate->clear();
        gmodeleTriParType->clear();
    }
    else
    {
        gmodele             = new QStandardItemModel(this);
        gmodeleTriParDate   = new QStandardItemModel(this);
        gmodeleTriParType   = new QStandardItemModel(this);
    }

    ndocs = initListDocs();
    if (ndocs == 0)
    {
        initOK = false;
        return;
    }
    /*
     *          |---------------------------|           TRI PAR DATE
                |        dateitem           |
                |    text = date examen     |
                |___________________________|---|
                                                |
                                                |––––––––––––––––––––––––––––––––––––––-|
                                                |               pitem                   |
                                                |   text = type + sous type document    |
                                                |   data["id"] = id                     |
                                                |_______________________________________|



                |---------------------------|           TRI PAR TYPE EXAMEN
                |        typitem            |
                |    text = type examen     |
                |___________________________|---|
                                                |
                                                |––––––––––––––––––––––––––––––––––––––-|
                                                |               pitem                   |
                                                |   text = date + sous type document    |
                                                |   data["id"] = id                     |
                                                |_______________________________________|

    */

    //1 Liste des documents (ordonnances, certificats, courriers, imagerie...etc...) imprimés par le poste ou reçus
    QList<QDate> listdates;

    // Tri par date
    for(QHash<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternespardate().constBegin(); itdoc != m_ListDocs.docsexternespardate().constEnd(); ++itdoc )
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());

        // individualisation des dates en item root

        if (!listdates.contains(doc->date().date()))
            listdates << doc->date().date();
    }
    qSort(listdates);
    for (int i=0; i<listdates.size(); ++i)
    {
        QString datestring = listdates.at(i).toString("dd-MM-yyyy");
        dateitem    = new QStandardItem(datestring);
        dateitem    ->setForeground(QBrush(QColor(Qt::red)));
        dateitem    ->setEditable(false);
        gmodeleTriParDate->appendRow(dateitem);
    }

    for(QHash<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternespardate().constBegin(); itdoc != m_ListDocs.docsexternespardate().constEnd(); ++itdoc )
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());      // rajout des items de chaque examen en child des dates

        QString date = doc->date().toString(tr("dd-MM-yyyy"));
        QString a = doc->typedoc();                                     // 1 = Type de document
        pitem           = new QStandardItem(CalcTitre(doc));
        QMap<QString, QVariant> data;
        data["id"]  = QString::number(doc->id());                       // 0 - idimpression
        QFont fontitem  = gFont;
        fontitem        .setBold(doc->importance()==2);                     // 5 - importance
        fontitem        .setItalic(doc->importance()==0);
        pitem           ->setFont(fontitem);
        pitem           ->setData(data);
        pitem           ->setEditable(false);
        QList<QStandardItem *> listitems = gmodeleTriParDate->findItems(date);
        if (listitems.size()>0)
        {
            listitems.at(0)->appendRow(pitem);
            listitems.at(0)->sortChildren(1);
        }
    }

    // Tri par type
    QStringList listtypedocs;
    for(QHash<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternespartype().constBegin(); itdoc != m_ListDocs.docsexternespartype().constEnd(); ++itdoc )
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());
        if (!listtypedocs.contains(doc->typedoc()))                     // individualisation des type document en item root
            listtypedocs << doc->typedoc();
    }
    qSort(listtypedocs);
    for (int i=0; i<listtypedocs.size(); ++i)
    {
        typitem     = new QStandardItem(listtypedocs.at(i));
        typitem     ->setForeground(QBrush(QColor(Qt::red)));
        typitem     ->setEditable(false);
        gmodeleTriParType     ->appendRow(typitem);
    }
    for(QHash<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternespartype().constBegin(); itdoc != m_ListDocs.docsexternespartype().constEnd(); ++itdoc )
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());
        QString date = doc->date().toString(tr("dd-MM-yyyy"));
        QString a = date + " - ";
        pitem           = new QStandardItem(CalcTitre(doc));
        QMap<QString, QVariant> data;
        data["id"]  = QString::number(doc->id());                       // 0 - idimpression
        QFont fontitem  = gFont;
        fontitem        .setBold(doc->importance() == 2);
        fontitem        .setItalic(doc->importance() == 0);
        pitem           ->setFont(fontitem);
        pitem           ->setData(data);
        pitem           ->setEditable(false);
        QList<QStandardItem *> listitems = gmodeleTriParType->findItems(doc->typedoc());
        if (listitems.size()>0)
        {
            listitems.at(0)->appendRow(pitem);
            listitems.at(0)->sortChildren(1);
        }
    }

    if (gModeTri == parDate)
        gmodele = gmodeleTriParDate;
    else
        gmodele = gmodeleTriParType;
    nbredocs = ndocs;
    QItemSelectionModel *m = ListDocsTreeView->selectionModel();
    ListDocsTreeView->setModel(gmodele);
    delete m;

    int nrows = gmodele->item(gmodele->rowCount()-1)->rowCount()-1;                 // le nombre de child du dernier item
    QStandardItem *item =  gmodele->item(gmodele->rowCount()-1)->child(nrows,0);    // le tout dernier item
    QModelIndex idx = item->index();                                                // l'index de ce dernier item
    if (idimpraretrouver != "")
    {
        // la suite ne marche pas et provoque des plantages
        //        QHash<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternespardate().find(idimpraretrouver.toInt());
        //        if (itdoc != m_ListDocs.docsexternespardate().constEnd())
        //        {
        //            qDebug() << itdoc.key();
        //            DocExterne *doc = itdoc.value();
        //            if (getItemFromDocument(doc) != Q_NULLPTR)
        //                idx = getItemFromDocument(doc)->index();
        //        }
        QModelIndex indx = getIndexFromId(idimpraretrouver.toInt());
        if (indx.isValid())
            idx = indx;
    }
    ListDocsTreeView->setSelectionModel(new QItemSelectionModel(gmodele));
    ListDocsTreeView->expandAll();
    ListDocsTreeView->scrollTo(idx, QAbstractItemView::EnsureVisible);
    ListDocsTreeView->setCurrentIndex(idx);
    connect(ListDocsTreeView->selectionModel(), &QItemSelectionModel::currentChanged,   this,   [=] {AfficheDoc(ListDocsTreeView->selectionModel()->currentIndex());});
    connect(ListDocsTreeView,                   &QTreeView::customContextMenuRequested, this,   [=] {AfficheCustomMenu();});
}

