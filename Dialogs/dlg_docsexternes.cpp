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

dlg_docsexternes::dlg_docsexternes(int idpat, bool UtiliseTCP, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionDocsExternes", parent)
{
    proc                = Procedures::I();
    gidPatient          = idpat;
    patient             = DataBase::getInstance()->loadPatientById(gidPatient);
    db                  = DataBase::getInstance();
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    installEventFilter(this);
    setMaximumHeight(qApp->desktop()->availableGeometry().height());
    setWindowTitle(tr("Documents de ") + patient->prenom() + " " + patient->nom());

    QFont font          = qApp->font();
    font                .setPointSize(font.pointSize()+2);
    gFont = QApplication::font();
    int d=0;
#ifdef QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE
    d=2;
#endif
    gFont.setPointSize(gFont.pointSize()-d);

    int margemm         = proc->TailleTopMarge(); // exprimé en mm
    printer             = new QPrinter(QPrinter::HighResolution);
    printer             ->setFullPage(true);
    rect                = printer->paperRect();

    rect.adjust(mmToInches(margemm) * printer->logicalDpiX(),
                mmToInches(margemm) * printer->logicalDpiY(),
                -mmToInches(margemm) * printer->logicalDpiX(),
                -mmToInches(margemm) * printer->logicalDpiY());

    Scene               = new QGraphicsScene(this);
    ListDocsTreeView    = new QTreeView(this);
    inflabel            = new QLabel();
    inflabel            ->setFont(font);


    ScrollTable         = new UpTableWidget(this);                      // utilisé pour afficher les pdf qui ont parfois plusieurs pages
    ScrollTable->horizontalHeader() ->setVisible(false);
    ScrollTable->verticalHeader()   ->setVisible(false);
    ScrollTable         ->installEventFilter(this);
    ScrollTable         ->setFocusPolicy(Qt::NoFocus);
    ScrollTable         ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    ScrollTable         ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ScrollTable         ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30

    GraphicView         = new QGraphicsView(Scene, this);               // utilisé pour afficher les jpg et les video
    GraphicView         ->installEventFilter(this);
    GraphicView         ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GraphicView         ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    GraphicView         ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30

    ListDocsTreeView    ->setFixedWidth(185);
    ListDocsTreeView    ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ListDocsTreeView    ->setFont(gFont);
    ListDocsTreeView    ->setEditTriggers(QAbstractItemView::DoubleClicked);
    ListDocsTreeView    ->setSelectionMode(QAbstractItemView::SingleSelection);
    ListDocsTreeView    ->setContextMenuPolicy(Qt::CustomContextMenu);
    ListDocsTreeView    ->setAnimated(true);
    ListDocsTreeView    ->setIndentation(6);
    ListDocsTreeView    ->header()->setVisible(false);

    player              = new QMediaPlayer;

    QHBoxLayout *lay   = new QHBoxLayout();
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());
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
    AjouteWidgetLayButtons(AllDocsupCheckBox, false);
    AjouteWidgetLayButtons(OnlyImportantDocsupCheckBox, false);
    AllDocsupCheckBox->setChecked(true);

    playctrl            = new PlayerControls(player, this);
    AjouteWidgetLayButtons(playctrl, false);
    sw                  = new UpSwitch(this);
    AjouteWidgetLayButtons(sw, false);
    AjouteLayButtons(UpDialog::ButtonSuppr | UpDialog::ButtonPrint);
    setStageCount(1);


    hdelta          = 0;
    wdelta          = 0;
    hdeltaframe     = 0;
    wdeltaframe     = 0;
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


    connect(sw,                             &UpSwitch::Bascule,             this,   [=] {BasculeTriListe(sw->PosSwitch());});
    connect (SupprButton,                   &QPushButton::clicked,          this,   [=] {SupprimeDoc();});
    connect (AllDocsupCheckBox,             &QCheckBox::toggled,            this,   [=] {FiltrerListe(AllDocsupCheckBox);});
    connect (OnlyImportantDocsupCheckBox,   &QCheckBox::toggled,            this,   [=] {FiltrerListe(OnlyImportantDocsupCheckBox);});
    connect (playctrl,                      &PlayerControls::ctrl,          this,   [=] {PlayerCtrl(playctrl->State());});
    connect (playctrl,                      &PlayerControls::recfile,       this,   [=] {EnregistreVideo();});
    connect (proc,                          &Procedures::UpdDocsExternes,   this,   [=] {ActualiseDocsExternes();});
    connect (PrintButton,                   &QPushButton::clicked,          this,   [=] {ImprimeDoc();});

    if (!UtiliseTCP)
    {
        QTimer *TimerActualiseDocsExternes    = new QTimer(this);
        TimerActualiseDocsExternes    ->start(10000);
        connect (TimerActualiseDocsExternes,    &QTimer::timeout,               this,   [=] {ActualiseDocsExternes();});
    }

    gMode               = Normal;
    gModeTri            = parDate;
    initOK = (ActualiseDocsExternes() > 0);
    if(!initOK)
        return;
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

void dlg_docsexternes::AfficheCustomMenu(DocExterne *docmt)
{
    QModelIndex idx = getIndexFromId(gmodele, docmt->id());
    QMenu *menu = new QMenu(this);
    QAction *paction_ImportantMin   = new QAction(tr("Importance faible"));
    QAction *paction_ImportantNorm  = new QAction(tr("Importance normale"));
    QAction *paction_ImportantMax   = new QAction(tr("Importance forte"));
    QAction *paction_Modifier       = new QAction(Icons::icEditer(), tr("Modifier le titre"));
    int imptce = docmt->importance();
    QIcon icon = Icons::icBlackCheck();
    if (imptce == 0)
        paction_ImportantMin->setIcon(icon);
    else if (imptce == 1)
        paction_ImportantNorm->setIcon(icon);
    else if (imptce == 2)
        paction_ImportantMax->setIcon(icon);
    if (proc->getUserConnected()->isMedecin())
    {
        menu->addAction(paction_ImportantMin);
        menu->addAction(paction_ImportantNorm);
        menu->addAction(paction_ImportantMax);
    }
    menu->addAction(paction_Modifier);
    connect (paction_ImportantMin,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Min);});
    connect (paction_ImportantNorm, &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Norm);});
    connect (paction_ImportantMax,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Max);});
    connect (paction_Modifier,      &QAction::triggered,    this,  [=] {ModifierItem(idx);});

#ifndef QT_NO_PRINTER
    if (docmt != Q_NULLPTR && docmt->format()!=VIDEO)
    {
        QMenu *menuImprime  = menu->addMenu(tr("Imprimer"));
        menuImprime         ->setIcon(Icons::icImprimer());

        QAction *paction_Reimprimer = new QAction(tr("Réimprimer"));
        QAction *paction_ModifierReimprimer = new QAction(tr("Modifier et réimprimer"));
        QAction *paction_ModifierReimprimerCeJour = new QAction(tr("Modifier et réimprimer à la date d'aujourd'hui"));
        QAction *paction_ReimprimerCeJour = new QAction(tr("Réimprimer à la date d'aujourd'hui"));
        connect (paction_Reimprimer,                &QAction::triggered,    this,  [=] {ReImprimeDoc(docmt);});
        connect (paction_ModifierReimprimer,        &QAction::triggered,    this,  [=] {ModifieEtReImprimeDoc(docmt, true,  true);});
        connect (paction_ModifierReimprimerCeJour,  &QAction::triggered,    this,  [=] {ModifieEtReImprimeDoc(docmt, true,  false);});
        connect (paction_ReimprimerCeJour,          &QAction::triggered,    this,  [=] {ModifieEtReImprimeDoc(docmt, false, false);});

        // si le document n'est ni une imagerie ni un document reçu, on propose de le modifer
        if (proc->getUserConnected()->isMedecin()
            && (docmt->format() != IMAGERIE && docmt->format() != DOCUMENTRECU))
        {   // si le document a été émis aujourd'hui, on propose de le modifier - dans ce cas, on va créer une copie qu'on va modifier et on détruira le document d'origine à la fin
            if (QDate::currentDate() == docmt->date().date())
                menuImprime->addAction(paction_ModifierReimprimer);
            else
            {   // si on a un texte d'origine, on peut modifier le document - (pour les anciennes versions de Rufus, il n'y avait pas de texte d'origine)
                if (docmt->textorigine() != "")
                    menuImprime->addAction(paction_ModifierReimprimerCeJour);
                menuImprime->addAction(paction_ReimprimerCeJour);
            }
        }
        menuImprime->addAction(paction_Reimprimer);
     }
#endif
    QAction *paction_Poubelle   = new QAction(Icons::icPoubelle(), tr("Supprimer"));
    connect (paction_Poubelle,  &QAction::triggered,    this,  [=] {SupprimeDoc(docmt);});
    if (proc->getUserConnected()->isMedecin())
        menu->addAction(paction_Poubelle);

    menu->exec(cursor().pos());
}

QString dlg_docsexternes::CalcTitre(DocExterne* docmt)
{
    QString a;
    if (docmt->format() == IMAGERIE || docmt->format() == VIDEO)
        a = docmt->typedoc() + " ";
    if (docmt->soustypedoc() != "")
        a += docmt->soustypedoc();
    else if (docmt->titre() != "")
    {
        QTextEdit text;
        text.setHtml(docmt->titre());
        a += text.toPlainText();
    }
    return a;
}

void dlg_docsexternes::CorrigeImportance(DocExterne *docmt, enum Importance imptce)
{
    auto modifieitem = [] (QStandardItem *item, DocExterne *docmt, int imp, QFont fontitem)
    {
        switch (imp) {
        case 0:{
            fontitem.setItalic(true);
            item->setFont(fontitem);
            if (docmt->typedoc() == PRESCRIPTION)
                item->setIcon(Icons::icStetho());
            else if (docmt->format() == PRESCRIPTIONLUNETTES)
                item->setIcon(Icons::icSunglasses());
            else if (docmt->format() == VIDEO)
                item->setIcon(Icons::icCinema());
            else if (docmt->format() == IMAGERIE)
                item->setIcon(Icons::icPhoto());
            else if (docmt->format() == COURRIER)
                item->setIcon(Icons::icImprimer());
            else if (docmt->format() == COURRIERADMINISTRATIF)
                item->setIcon(Icons::icTampon());
            else
                item->setIcon(QIcon());
            docmt->setimportance(0);
            break;
        }
        case 1:{
            item->setFont(fontitem);
            if (docmt->typedoc() == PRESCRIPTION)
                item->setIcon(Icons::icStetho());
            else if (docmt->format() == PRESCRIPTIONLUNETTES)
                item->setIcon(Icons::icSunglasses());
            else if (docmt->format() == VIDEO)
                item->setIcon(Icons::icCinema());
            else if (docmt->format() == IMAGERIE)
                item->setIcon(Icons::icPhoto());
            else if (docmt->format() == COURRIER)
                item->setIcon(Icons::icImprimer());
            else if (docmt->format() == COURRIERADMINISTRATIF)
                item->setIcon(Icons::icTampon());
            else
                item->setIcon(QIcon());
            docmt->setimportance(1);
            break;
        }
        case 2:{
            fontitem.setBold(true);
            item->setFont(fontitem);
            item->setIcon(Icons::icImportant());
            docmt->setimportance(2);
            break;
        }
        }
    };

    int imp = 1;
    if (imptce == Min) imp = 0;
    else if (imptce == Max) imp = 2;
    QStandardItem *item = getItemFromDocument(gmodele, docmt);
    if (item == Q_NULLPTR)
        return;
    int id = docmt->id();
    modifieitem(item, docmt, imp, gFont);
    item = gmodeleTriParDate->itemFromIndex(getIndexFromId(gmodeleTriParDate,id));
    if (item != Q_NULLPTR)
        modifieitem(item, docmt, imp, gFont);
    item = gmodeleTriParType->itemFromIndex(getIndexFromId(gmodeleTriParType,id));
    if (item != Q_NULLPTR)
        modifieitem(item, docmt, imp, gFont);
    db->StandardSQL("update " NOM_TABLE_IMPRESSIONS " set Importance = " + QString::number(imp) + " where idImpression = " + QString::number(id));
    int nimportants = 0;
    for(QMap<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternes().constBegin(); itdoc != m_ListDocs.docsexternes().constEnd(); ++itdoc )
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());
        if (doc->importance() == 2)
        {
            nimportants ++;
            break;
        }
    }
    OnlyImportantDocsupCheckBox->setEnabled( nimportants>0 || OnlyImportantDocsupCheckBox->isChecked());
}

void dlg_docsexternes::AfficheDoc(QModelIndex idx)
{
    DocExterne *docmt = getDocumentFromIndex(idx);

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
            NomOnglet = tr("Monoposte");
        if (DataBase::getInstance()->getMode() == DataBase::ReseauLocal)
            NomOnglet = tr("Réseau local");
        NomDirStockageImagerie  = proc->DirImagerieServeur();
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
            {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                return;
            }
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
        else if (doc.value("Type").toString() == PDF)     // le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage)
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
                lab->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(lab,    &UpLabel::clicked,                      this, [=] {ZoomDoc();});
                connect(lab,    &UpLabel::customContextMenuRequested,   this, [=] {AfficheCustomMenu(docmt);});
                delete pdfPage;
                ScrollTable->setCellWidget(i,0,lab);
            }
            delete document;
        }
        else return;
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
        QModelIndex indx = getIndexFromId(gmodele, idimpraretrouver.toInt());
        if (indx.isValid())
            idx = indx;
    }
    ListDocsTreeView->setSelectionModel(new QItemSelectionModel(gmodele));
    ListDocsTreeView->expandAll();
    ListDocsTreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
    ListDocsTreeView->setCurrentIndex(idx);
    connect(ListDocsTreeView->selectionModel()  ,   &QItemSelectionModel::currentChanged,   this,   [=] {AfficheDoc(ListDocsTreeView->selectionModel()->currentIndex());});
    connect(ListDocsTreeView,                       &QTreeView::customContextMenuRequested, this,   [=] {
        QModelIndex idx = ListDocsTreeView->indexAt(ListDocsTreeView->mapFromGlobal(cursor().pos()));
        DocExterne *docmt = getDocumentFromIndex(idx);
        if (docmt != Q_NULLPTR)
            AfficheCustomMenu(docmt);
    });
}

int dlg_docsexternes::ActualiseDocsExternes()
{
    m_ListDocs.addListDocsExternes(db->loadDoscExternesByPatientAll(gidPatient));
    if (m_ListDocs.NouveauDocument())
    {
        m_ListDocs.setNouveauDocumentFalse();
        RemplirTreeView();
    }
    return m_ListDocs.docsexternes().size();
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
    else if (chk == AllDocsupCheckBox)
    {
        if (chk->isChecked())
            OnlyImportantDocsupCheckBox->setChecked(false);
    }
    RemplirTreeView();                              // après FiltrerListe()
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
    /* Cette fonction sert à stocker dans un QByteArray le contenu des documents d'imagerie ou des courriers émis par le logiciel pour pouvoir les afficher
     * la fonction est appelée par Slot_AfficheDoc(), on utilise la table impressions
     *      imagerie = false -> Le document est un document texte (ordo, certificat...etc).
     *                          Il est déjà dans la table impressions sous la forme de 3 champs html (entete, corps et pied)
     *                          Ces champs vont être utilisés pour l'impression vers un fichier pdf.
     *                          Le bytearray sera constitué par le contenu de ce fichier et affiché à l'écran.
     *      imagerie = true ->  le document est un document d'imagerie stocké sur un fichier. On va le transférer dans la table echangeimages et le transformer en bytearray
   * la fonction est applée par ImprimeDoc() - on utilise la table echangeimages
     *      pour imprimer un document texte. Le document texte est recalculé en pdf et le pdf est incorporé dans un bytearray.
     *      pour imprimer un document d'imagerie stocké dans la table echangeimages - on va extraire le ByteArray directement de la base de la table echangeimages
     * la fonction renvoie un QMap<QString,QVariant> result
     * result["Type"] est un QString qui donne le type de document, jpg ou pdf
     * result["ba"] est un QByteArray qui stocke le contenu du fichier
     * result["lien"] est le lien vers le fichier sur le disque dur du serveur
    */
    DocExterne *docmt = m_ListDocs.getDocumentById(idimpression);
    QMap<QString,QVariant> result;
    QString idimpr = QString::number(idimpression);
    QString filename = "";
    QByteArray bapdf;
    result["Type"]    = "";
    result["ba"]      = QByteArray("");
    result["lien"]    = "";
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
                QString req = "delete from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr + " and Facture is null";
                db->StandardSQL(req);
                req = "INSERT INTO " NOM_TABLE_ECHANGEIMAGES " (idimpression, " + sfx + ", compression) "
                               "VALUES (" +
                               idimpr + ", " +
                               " LOAD_FILE('" + Utils::correctquoteSQL(proc->DirImagerieServeur() + NOMDIR_IMAGES + filename) + "'), " +
                               QString::number(docmt->compression()) + ")";
                 db->StandardSQL(req);
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
            result["lien"]    = filename;
        }
        else if (impr.at(1).toByteArray().size()>0)                                            // c'est un jpg
        {
            bapdf.append(impr.at(1).toByteArray());
            result["Type"]    = JPG;
            result["ba"]      = bapdf;
            result["lien"]    = filename;
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
        result["lien"]    = "";
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

QModelIndex dlg_docsexternes::getIndexFromId(QStandardItemModel *modele, int id)
{
    QModelIndex idx;
    for (int m = 0; m<modele->rowCount(); m++)
        for (int n=0; n<modele->item(m)->rowCount(); n++)
            if (modele->item(m)->child(n)->data().toMap().value("id").toInt() == id)
            {
                idx = modele->item(m)->child(n)->index();
                break;
            }
    return idx;
}

QStandardItem* dlg_docsexternes::getItemFromDocument(QStandardItemModel *model, DocExterne* docmt)
{
    QModelIndex idx = getIndexFromId(model,docmt->id());
    return gmodele->itemFromIndex(idx);
}

void dlg_docsexternes::ImprimeDoc()
{
#ifndef QT_NO_PRINTER
    PrintButton         ->disconnect();  // pour éviter le doubles impressions en cas de double clic lent
    DocExterne * docmt  = getDocumentFromIndex(ListDocsTreeView->selectionModel()->selectedIndexes().at(0));
    docmt               = m_ListDocs.getDocumentById(docmt->id());
    if (docmt != Q_NULLPTR)
    {
        bool detruirealafin = false;

        UpMessageBox msgbox;
        UpSmallButton ReimprBouton          (tr("Réimprimer\nle document"));
        UpSmallButton ModifEtReimprBouton   (tr("Modifier\net imprimer"));
        UpSmallButton ImpAujourdhuiBouton   (tr("Réimprimer à\nla date d'aujourd'hui"));
        UpSmallButton AnnulBouton           (tr("Annuler"));
        msgbox.setText(tr("Réimprimer un document"));
        msgbox.setIcon(UpMessageBox::Print);

        msgbox.addButton(&AnnulBouton,UpSmallButton::CANCELBUTTON);
        if (proc->getUserConnected()->isMedecin()
            && (docmt->format() != IMAGERIE && docmt->format() != DOCUMENTRECU))   // si le document n'est ni une imagerie ni un document reçu, on propose de le modifer
        {
            if (QDate::currentDate() == docmt->date().date())           // si le document a été émis aujourd'hui, on propose de le modifier
                                                                        // dans ce cas, on va créer une copie qu'on va modifier
                                                                        // et on détruira le document d'origine à la fin
            {
                msgbox.addButton(&ModifEtReimprBouton,UpSmallButton::EDITBUTTON);
                detruirealafin = true;
            }
            else
            {
                if (docmt->textorigine() != "")     // si on a un texte d'origine, on peut modifier le document
                                                    // (pour les anciennes versions de Rufus, il n'y avait pas de texte d'origine)
                {
                    ModifEtReimprBouton.setText(tr("Modifier et imprimer\nà la date d'aujourd'hui"));
                    msgbox.addButton(&ModifEtReimprBouton,UpSmallButton::EDITBUTTON);
                }
                msgbox.addButton(&ImpAujourdhuiBouton,UpSmallButton::COPYBUTTON);
            }
        }
        msgbox.addButton(&ReimprBouton, UpSmallButton::PRINTBUTTON);
        msgbox.exec();

        //Reimpression simple du document, sans réédition => pas d'action sur la BDD
        if (msgbox.clickedButton() == &ReimprBouton)
            ReImprimeDoc(docmt);

        //Réédition d'un document - on va réimprimer le document à la date du jour en le modifiant - ne concerne que les courriers et ordonnances émis => on enregistre le nouveau document dans la BDD
        else if (msgbox.clickedButton() == &ModifEtReimprBouton || msgbox.clickedButton() == &ImpAujourdhuiBouton)
            ModifieEtReImprimeDoc(docmt, msgbox.clickedButton() == &ModifEtReimprBouton, detruirealafin);
        msgbox.close();
    }
    connect(PrintButton, &QPushButton::clicked, this, &dlg_docsexternes::ImprimeDoc);
#endif
}

bool dlg_docsexternes::ModifieEtReImprimeDoc(DocExterne *docmt, bool modifiable, bool detruirealafin)
{
    // reconstruire le document en refaisant l'entête et en récupérant le corps et le pied enregistrés dans la base
    QString     Corps, Entete, Pied, txt;
    QTextEdit   *Etat_textEdit  = new QTextEdit;
    bool        AvecNumPage     = false;
    bool        aa;
    bool        ALD             = (docmt->isALD());
    bool        Prescription    = (docmt->format() == PRESCRIPTION || docmt->format() == PRESCRIPTIONLUNETTES);

    User *userEntete = proc->setDataOtherUser(docmt->iduser());
    if (userEntete == Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
        return false;
    }

    //création de l'entête
    QMap<QString,QString> EnteteMap = proc->ImpressionEntete(QDate::currentDate(), userEntete);
    Entete = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
    if (Entete == "")
        return false;
    Entete.replace("{{TITRE1}}"        , "");
    Entete.replace("{{TITRE}}"         , "");
    Entete.replace("{{DDN}}"           , "");
    Entete.replace("{{PRENOM PATIENT}}", (Prescription? patient->prenom()        : ""));
    Entete.replace("{{NOM PATIENT}}"   , (Prescription? patient->nom().toUpper() : ""));

    //création du pied
    Pied = proc->ImpressionPied(docmt->format() == PRESCRIPTIONLUNETTES, ALD);

    // creation du corps de l'ordonnance
    QString txtautiliser    = (docmt->textorigine() == ""?              docmt->textcorps()              : docmt->textorigine());
    txt                     = (modifiable?                              proc->Edit(txtautiliser)        : txtautiliser);
    Corps                   = (docmt->typedoc() == PRESCRIPTION?        proc->ImpressionCorps(txt,ALD)  : proc->ImpressionCorps(txt));
    Etat_textEdit           ->setHtml(Corps);
    if (Etat_textEdit->toPlainText() == "" || txt == "")
    {
        if (Etat_textEdit->toPlainText() == "")
            UpMessageBox::Watch(this,tr("Rien à imprimer"));
        return false;
    }
    int TailleEnTete        = (ALD?                                     proc->TailleEnTeteALD()         : proc->TailleEnTete());
    bool AvecDupli          = (proc->gsettingsIni->value("Param_Imprimante/OrdoAvecDupli").toString() == "YES"
                               && docmt->typedoc() == PRESCRIPTION);
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

        QHash<QString,QVariant> listbinds;
        listbinds["iduser"] = docmt->iduser();
        listbinds["idpat"] = docmt->idpatient();
        listbinds["typeDoc"] = docmt->typedoc();
        listbinds["soustypedoc"] = docmt->soustypedoc();
        listbinds["titre"] = docmt->titre();
        listbinds["textEntete"] = Entete;
        listbinds["textCorps"] = Corps;
        listbinds["textOrigine"] = txt;
        listbinds["textPied"] = Pied;
        listbinds["dateimpression"] = QDate::currentDate().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds["formatdoc"] = docmt->format();
        listbinds["idlieu"] = db->getUserConnected()->getSite()->id();
        listbinds["ald"] = (ALD? "1" : QVariant(QVariant::String));
        listbinds["useremetteur"] = db->getUserConnected()->id();
        listbinds["importance"] = docmt->importance();
        if (!db->InsertSQLByBinds(NOM_TABLE_IMPRESSIONS, listbinds))
        {
            UpMessageBox::Watch(this,tr("Impossible d'enregistrer ce document dans la base!"));
            connect(PrintButton,        &QPushButton::clicked, this,   [=] {ImprimeDoc();});
        }
        else
        {
            if (detruirealafin)
            {
                db->SupprRecordFromTable(docmt->id(),"idimpression",NOM_TABLE_IMPRESSIONS);
                m_ListDocs.RemoveKey(docmt->id());
            }
            ActualiseDocsExternes();
            int idimpr = db->selectMaxFromTable("idimpression", NOM_TABLE_IMPRESSIONS);
            QModelIndex idx = getIndexFromId(gmodele, idimpr);
            ListDocsTreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            ListDocsTreeView->setCurrentIndex(idx);
            AfficheDoc(idx);
        }
    }
    delete Etat_textEdit;
    return true;
}

bool dlg_docsexternes::ReImprimeDoc(DocExterne *docmt)
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
            return false;
        }
        if (document == Q_NULLPTR) {
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            delete document;
            return false;
        }

        document->setRenderHint(Poppler::Document::TextAntialiasing);
        int numpages = document->numPages();
        for (int i=0; i<numpages ;i++)
        {
            Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
            if (pdfPage == Q_NULLPTR) {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                delete document;
                return false;
            }
            image = pdfPage->renderToImage(600,600);
            if (image.isNull()) {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                delete document;
                return false;
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
        }
    }
    return true;
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
    QTimer::singleShot(0, Line, &QLineEdit::selectAll);     // de la bidouille mais pas trouvé autre chose

    connect(dlg->OKButton,   &QPushButton::clicked,   [=]
    {
        if (Line->text()!="")
        {
            db->StandardSQL("update " NOM_TABLE_IMPRESSIONS " set soustypedoc = '" + Utils::correctquoteSQL(Line->text()) + "' where idimpression = " + QString::number(docmt->id()));
            gmodele->itemFromIndex(idx)->setText(CalcTitre(m_ListDocs.reloadDocument(docmt)));
            int id = docmt->id();
            QString titre = CalcTitre(docmt);
            gmodeleTriParDate->itemFromIndex(getIndexFromId(gmodeleTriParDate,id))->setText(titre);
            gmodeleTriParType->itemFromIndex(getIndexFromId(gmodeleTriParType,id))->setText(titre);
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

void dlg_docsexternes::SupprimeDoc(DocExterne *docmt)
{
    if (docmt == Q_NULLPTR)
    {
        QModelIndex idx = ListDocsTreeView->selectionModel()->selectedIndexes().at(0);
        docmt = getDocumentFromIndex(idx);
    }
    if (docmt == Q_NULLPTR)
        return;
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
            db->StandardSQL("insert into " NOM_TABLE_DOCSASUPPRIMER " (FilePath) VALUES ('" + Utils::correctquoteSQL(filename) + "')");
        }
        QString idaafficher = "";
        if (m_ListDocs.docsexternes().size() > 1)    // on recherche le document sur qui va être mis en surbrillance après la suppression
        {
            QMap<int, DocExterne*> listaexplorer = m_ListDocs.docsexternes();
            QMap<int, DocExterne*>::const_iterator itdoc = listaexplorer.find(docmt->id());
            if (itdoc == listaexplorer.constBegin())
                ++itdoc;
            else
                --itdoc;
            idaafficher = QString::number(itdoc.key());
        }
        db->StandardSQL("delete from " NOM_TABLE_REFRACTION " where idrefraction = (select idrefraction from " NOM_TABLE_IMPRESSIONS
                        " where idimpression = " + idimpr + ")");
        db->StandardSQL("delete from " NOM_TABLE_IMPRESSIONS " where idimpression = " + idimpr);
        db->StandardSQL("delete from " NOM_TABLE_ECHANGEIMAGES " where idimpression = " + idimpr);
        m_ListDocs.RemoveKey(docmt->id());
        RemplirTreeView();
        ListDocsTreeView->expandAll();
        if (idaafficher != "")
        {
            QModelIndex idx = getIndexFromId(gmodele, idaafficher.toInt());
            if (idx.isValid())
            {
                ListDocsTreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                ListDocsTreeView->setCurrentIndex(idx);
            }
        }
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
    if (m_ListDocs.docsexternes().size() == 0){
        reject();
        return;  // si on ne met pas ça, le reject n'est pas effectué...
    }
    if (AllDocsupCheckBox->isChecked())
        gModeFiltre = FiltreSans;
    else if (OnlyImportantDocsupCheckBox->isChecked())
        gModeFiltre = ImportantFiltre;
    else
        gModeFiltre = NormalFiltre;

    if (ListDocsTreeView != Q_NULLPTR)
        ListDocsTreeView->disconnect();
    if (ListDocsTreeView->selectionModel() != Q_NULLPTR)
        ListDocsTreeView->selectionModel()->disconnect();
    QString             idimpraretrouver = "";
    gmodele = dynamic_cast<QStandardItemModel*>(ListDocsTreeView->model());
    if (gmodele)
    {
        if (gmodele->rowCount()>0)
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

    /*
     *          |---------------------------|           TRI PAR DATE
                |        dateitem           |
                |    text = date examen     |
                |___________________________|---|
                                                |
                                                |––––––––––––––––––––––––––––––––––––––-|---|––––––––––––––––––––––––––––––––––––––-|
                                                |               pitemdate               |   |            pitemtridated              |
                                                |   text = type + sous type document    |   |   text = datetime - pour le tri       |
                                                |   data["id"] = id                     |   |_______________________________________|
                                                |_______________________________________|



                |---------------------------|           TRI PAR TYPE EXAMEN
                |        typitem            |
                |    text = type examen     |
                |___________________________|---|
                                                |
                                                |––––––––––––––––––––––––––––––––––––––-|---|––––––––––––––––––––––––––––––––––––––-|
                                                |               pitemtype               |   |            pitemtidatep               |
                                                |   text = date + sous type document    |   |   text = datetime - pour le tri       |
                                                |   data["id"] = id                     |   |_______________________________________|
                                                |_______________________________________|

    */
    QStandardItem * rootNodeDate = gmodeleTriParDate->invisibleRootItem();
    QStandardItem * rootNodeType = gmodeleTriParType->invisibleRootItem();

    //1 Liste des documents (ordonnances, certificats, courriers, imagerie...etc...) imprimés par le poste ou reçus
    QList<QDate> listdatesnorm, listdatessansfilte, listdatesimportants;
    QStringList listtypesnorm, listtypessansfiltre, listtypesimportants;

    auto completelistes = [&] (QList<QDate> &dates, QStringList &typedocs, DocExterne* doc)
    {
        if (!dates.contains(doc->date().date()))
            dates << doc->date().date();
        if (!typedocs.contains(doc->typedoc()))
            typedocs << doc->typedoc();
    };

    for(QMap<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternes().constBegin(); itdoc != m_ListDocs.docsexternes().constEnd(); ++itdoc )
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());
        // créations des entêtes par date et par type d'examen
        {
            if (doc->importance()>0)
                completelistes(listdatesnorm, listtypesnorm, doc);
            completelistes(listdatessansfilte, listtypessansfiltre, doc);
            if (doc->importance()==2)
                completelistes(listdatesimportants, listtypesimportants, doc);
        }
    }
    if (listdatessansfilte.size() == 0)
        accept();
    OnlyImportantDocsupCheckBox->setEnabled(listdatesimportants.size() > 0);
    if (gModeFiltre == ImportantFiltre && listdatesimportants.size() == 0)
    {
        GraphicView->setVisible(false);
        ScrollTable->setVisible(false);
    }

    QList<QDate> listdates;
    QStringList listtypes;
    switch (gModeFiltre) {
    case FiltreSans:        listdates = listdatessansfilte;     listtypes = listtypessansfiltre;    break;
    case NormalFiltre:      listdates = listdatesnorm;          listtypes = listtypesnorm;          break;
    case ImportantFiltre:   listdates = listdatesimportants;    listtypes = listtypesimportants;    break;
    }

    QStandardItem       *dateitem, *typitem, *pitemdate, *pitemtype, *pitemtridated, *pitemtridatet;
    // Tri par date
    qSort(listdates);
    for (int i=0; i<listdates.size(); ++i)
    {
        QString datestring = listdates.at(i).toString("dd-MM-yyyy");
        dateitem    = new QStandardItem(datestring);
        dateitem    ->setForeground(QBrush(QColor(Qt::red)));
        dateitem    ->setEditable(false);
        dateitem    ->setIcon(Icons::icDate());
        rootNodeDate->appendRow(dateitem);
    }
    // Tri par type
    qSort(listtypes);
    for (int i=0; i<listtypes.size(); ++i)
    {
        typitem     = new QStandardItem(listtypes.at(i));
        typitem     ->setForeground(QBrush(QColor(Qt::red)));
        typitem     ->setEditable(false);
        typitem     ->setIcon(Icons::icSortirDossier());
        rootNodeType->appendRow(typitem);
    }

    for(QMap<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternes().constBegin(); itdoc != m_ListDocs.docsexternes().constEnd(); ++itdoc )
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());      // rajout des items de chaque examen en child des dates et des types
        QString date = doc->date().toString(tr("dd-MM-yyyy"));
        QString a = doc->typedoc();
        pitemdate           = new QStandardItem(CalcTitre(doc));
        pitemtype           = new QStandardItem(CalcTitre(doc));
        pitemtridated        = new QStandardItem(doc->date().toString("yyyyMMddHHmmss"));
        pitemtridatet        = new QStandardItem(doc->date().toString("yyyyMMddHHmmss"));
        QMap<QString, QVariant> data;
        data["id"]          = QString::number(doc->id());
        QFont fontitem      = gFont;
        fontitem            .setBold(doc->importance()==2);
        fontitem            .setItalic(doc->importance()==0);
        pitemdate           ->setFont(fontitem);
        pitemdate           ->setData(data);
        pitemdate           ->setEditable(false);
        pitemtype           ->setFont(fontitem);
        pitemtype           ->setData(data);
        pitemtype           ->setEditable(false);
        if (doc->format() == PRESCRIPTION)
        {
            pitemdate->setIcon(Icons::icStetho());
            pitemtype->setIcon(Icons::icStetho());
        }
        else if (doc->format() == PRESCRIPTIONLUNETTES)
        {
            pitemdate->setIcon(Icons::icSunglasses());
            pitemtype->setIcon(Icons::icSunglasses());
        }
        else if (doc->format() == VIDEO)
        {
            pitemdate->setIcon(Icons::icCinema());
            pitemtype->setIcon(Icons::icCinema());
        }
        else if (doc->format() == IMAGERIE)
        {
            pitemdate->setIcon(Icons::icPhoto());
            pitemtype->setIcon(Icons::icPhoto());
        }
        else if (doc->format() == COURRIER)
        {
            pitemdate->setIcon(Icons::icImprimer());
            pitemtype->setIcon(Icons::icImprimer());
        }
        else if (doc->format() == COURRIERADMINISTRATIF)
        {
            pitemdate->setIcon(Icons::icTampon());
            pitemtype->setIcon(Icons::icTampon());
        }
        else if (doc->format() == DOCUMENTRECU)
        {
            if (doc->typedoc()== COURRIER)
            {
                pitemdate->setIcon(Icons::icImprimer());
                pitemtype->setIcon(Icons::icImprimer());
            }
                else
            {
                pitemdate->setIcon(Icons::icPhoto());
                pitemtype->setIcon(Icons::icPhoto());
            }
        }
        if (doc->importance()==2)
        {
            pitemdate->setIcon(Icons::icImportant());
            pitemtype->setIcon(Icons::icImportant());
        }
        QList<QStandardItem *> listitemsdate = gmodeleTriParDate->findItems(date);
        if (listitemsdate.size()>0)
        {
            switch (gModeFiltre) {
            case FiltreSans:
                listitemsdate.at(0)->appendRow(QList<QStandardItem*>() << pitemdate << pitemtridated);
                break;
            case NormalFiltre:
                if (doc->importance()>0)
                    listitemsdate.at(0)->appendRow(QList<QStandardItem*>() << pitemdate << pitemtridated);
                break;
            case ImportantFiltre:
                if (doc->importance()==2)
                    listitemsdate.at(0)->appendRow(QList<QStandardItem*>() << pitemdate << pitemtridated);
            }
            listitemsdate.at(0)->sortChildren(1);
        }
        QList<QStandardItem *> listitemstype = gmodeleTriParType->findItems(doc->typedoc());
        if (listitemstype.size()>0)
        {
            switch (gModeFiltre) {
            case FiltreSans:
                listitemstype.at(0)->appendRow(QList<QStandardItem*>() << pitemtype << pitemtridatet);
                break;
            case NormalFiltre:
                if (doc->importance()>0)
                    listitemstype.at(0)->appendRow(QList<QStandardItem*>() << pitemtype << pitemtridatet);
                break;
            case ImportantFiltre:
                if (doc->importance()==2)
                    listitemstype.at(0)->appendRow(QList<QStandardItem*>() << pitemtype << pitemtridatet);
            }
            listitemstype.at(0)->sortChildren(1);
        }
    }
//    qDebug() << "rowCount() = " << gmodeleTriParDate->rowCount();
//    qDebug() << "dernier child = " << gmodeleTriParDate->item(gmodeleTriParDate->rowCount()-1)->text();
//    qDebug() << "rowCount() du dernier child = " << gmodeleTriParDate->item(gmodeleTriParDate->rowCount()-1)->rowCount()-1;

    if (gModeTri == parDate)
        gmodele = gmodeleTriParDate;
    else
        gmodele = gmodeleTriParType;
    QItemSelectionModel *m = ListDocsTreeView->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    ListDocsTreeView->setModel(gmodele);
    delete m;

    int nrows = gmodele->item(gmodele->rowCount()-1)->rowCount()-1;                 // le nombre de child du dernier item
    QStandardItem *item =  gmodele->item(gmodele->rowCount()-1)->child(nrows,0);    // le tout dernier item
    QModelIndex idx = item->index();                                                // l'index de ce dernier item
    if (idimpraretrouver != "")
    {
        // la suite ne marche pas et provoque des plantages ????
        //        QMap<int, DocExterne*>::const_iterator itdoc = m_ListDocs.docsexternes().find(idimpraretrouver.toInt());
        //        if (itdoc != m_ListDocs.docsexternes().constEnd())
        //        {
        //            qDebug() << itdoc.key();
        //            DocExterne *doc = itdoc.value();
        //            if (getItemFromDocument(doc) != Q_NULLPTR)
        //                idx = getItemFromDocument(doc)->index();
        //        }
        QModelIndex indx = getIndexFromId(gmodele, idimpraretrouver.toInt());
        if (indx.isValid())
            idx = indx;
    }
    ListDocsTreeView->setSelectionModel(new QItemSelectionModel(gmodele));
    ListDocsTreeView->expandAll();
    ListDocsTreeView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
    ListDocsTreeView->setCurrentIndex(idx);
    AfficheDoc(idx);
    connect(ListDocsTreeView->selectionModel(), &QItemSelectionModel::currentChanged,   this,   [=] {AfficheDoc(ListDocsTreeView->selectionModel()->currentIndex());});
    connect(ListDocsTreeView,                   &QTreeView::customContextMenuRequested, this,   [=] {
        QModelIndex idx = ListDocsTreeView->indexAt(ListDocsTreeView->mapFromGlobal(cursor().pos()));
        DocExterne *docmt = getDocumentFromIndex(idx);
        if (docmt != Q_NULLPTR)
            AfficheCustomMenu(docmt);
    });
}

