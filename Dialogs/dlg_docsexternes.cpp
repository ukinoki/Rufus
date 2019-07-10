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

#include "dlg_docsexternes.h"
#include "gbl_datas.h"
#include "icons.h"

dlg_docsexternes::dlg_docsexternes(DocsExternes *Docs, Patient *pat, bool iscurrentpatient, bool UtiliseTCP, QWidget *parent) :
    UpDialog(QDir::homePath() + FILE_INI, "PositionsFiches/PositionDocsExternes", parent)
{
    proc                = Procedures::I();
    db                  = DataBase::I();
    m_currentpatient    = pat;
    m_currentuser       = Datas::I()->users->userconnected();
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    installEventFilter(this);
    setMaximumHeight(QGuiApplication::screens().first()->geometry().height());
    setWindowTitle(tr("Documents de ") + m_currentpatient->prenom() + " " + m_currentpatient->nom());
    setModal(!iscurrentpatient); //quand la fiche ne concerne pas le patient en cours

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

    rect.adjust(Utils::mmToInches(margemm) * printer->logicalDpiX(),
                Utils::mmToInches(margemm) * printer->logicalDpiY(),
                -Utils::mmToInches(margemm) * printer->logicalDpiX(),
                -Utils::mmToInches(margemm) * printer->logicalDpiY());

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


    QHBoxLayout *lay    = new QHBoxLayout();
    lay                 ->addWidget(ListDocsTreeView,2);
    lay                 ->addWidget(ScrollTable,8);
    lay                 ->addWidget(GraphicView,8);
    lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    lay                 ->setSpacing(10);
    dlglayout()         ->insertLayout(0,lay);

    playctrl            = new PlayerControls(this);
    AjouteWidgetLayButtons(playctrl, false);
    buttonslayout()->insertSpacerItem(0, new QSpacerItem(20,20,QSizePolicy::Fixed, QSizePolicy::Fixed));
    AllDocsupCheckBox           = new UpCheckBox(tr("Tous"));
    OnlyImportantDocsupCheckBox = new UpCheckBox(tr("Importants"));
    AllDocsupCheckBox           ->setImmediateToolTip(tr("Afficher tous les documents\ny compris les documents d'importance minime"));
    OnlyImportantDocsupCheckBox ->setImmediateToolTip(tr("N'afficher que les documents marqués importants"));
    AjouteWidgetLayButtons(AllDocsupCheckBox, false);
    AjouteWidgetLayButtons(OnlyImportantDocsupCheckBox, false);
    AllDocsupCheckBox->setChecked(true);

    sw                  = new UpSwitch(this);
    AjouteWidgetLayButtons(sw, false);
    AjouteLayButtons(UpDialog::ButtonRecord | UpDialog::ButtonSuppr | UpDialog::ButtonPrint);
    //setStageCount(1);


    hdelta          = 0;
    wdelta          = 0;
    hdeltaframe     = 0;
    wdeltaframe     = 0;
    AvecPrevisu = proc  ->ApercuAvantImpression();

    /*Gestion des XML - exemple
    QString adressexml = QDir::homePath() + DIR_RUFUS + "/XML/" + QString::number(idpat) + "/Exam_Data.xml";
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


    connect (sw,                            &UpSwitch::Bascule,             this,   [=] {BasculeTriListe(sw->PosSwitch());});
    connect (SupprButton,                   &QPushButton::clicked,          this,   [=] {SupprimeDoc();});
    connect (AllDocsupCheckBox,             &QCheckBox::toggled,            this,   [=] {FiltrerListe(AllDocsupCheckBox);});
    connect (OnlyImportantDocsupCheckBox,   &QCheckBox::toggled,            this,   [=] {FiltrerListe(OnlyImportantDocsupCheckBox);});
    connect (playctrl,                      &PlayerControls::ctrl,          this,   [=] (PlayerControls::State  state) {    switch (state){
                                                                                                                                case PlayerControls::Stop:  player->stop();     break;
                                                                                                                                case PlayerControls::Pause: player->pause();    break;
                                                                                                                                case PlayerControls::Play:  player->play();
                                                                                                                                }
                                                                                                                        });
    connect (proc,                          &Procedures::UpdDocsExternes,   this,   &dlg_docsexternes::ActualiseDocsExternes);
    connect (PrintButton,                   &QPushButton::clicked,          this,   &dlg_docsexternes::ImprimeDoc);

    if (!UtiliseTCP)
    {
        QTimer *TimerActualiseDocsExternes    = new QTimer(this);
        TimerActualiseDocsExternes    ->start(10000);
        connect (TimerActualiseDocsExternes,    &QTimer::timeout,           this,   &dlg_docsexternes::ActualiseDocsExternes);
    }

    gMode               = Normal;
    gModeTri            = parDate;
    m_docsexternes      = Docs;
    conservealafin      = iscurrentpatient;
    m_docsexternes->setNouveauDocumentFalse();
    RemplirTreeView();
}

dlg_docsexternes::~dlg_docsexternes()
{
    delete printer;
    if (!conservealafin)
    {
        m_docsexternes->clearAll(m_docsexternes->docsexternes());
        delete m_docsexternes;
    }
}

void dlg_docsexternes::AfficheCustomMenu(DocExterne *docmt)
{
    QModelIndex idx = getIndexFromId(gmodele, docmt->id());
    QMenu *menu = new QMenu(this);
    QAction *paction_ImportantMin   = new QAction(tr("Importance faible"));
    QAction *paction_ImportantNorm  = new QAction(tr("Importance normale"));
    QAction *paction_ImportantMax   = new QAction(tr("Importance forte"));
    QAction *paction_Modifier       = new QAction(Icons::icEditer(), tr("Le titre"));
    QAction *paction_ModifierDate   = new QAction(Icons::icDate(), tr("La date"));
    int imptce = docmt->importance();
    QIcon icon = Icons::icBlackCheck();
    if (imptce == 0)
        paction_ImportantMin->setIcon(icon);
    else if (imptce == 1)
        paction_ImportantNorm->setIcon(icon);
    else if (imptce == 2)
        paction_ImportantMax->setIcon(icon);
    if (m_currentuser->isMedecin())
    {
        menu->addAction(paction_ImportantMin);
        menu->addAction(paction_ImportantNorm);
        menu->addAction(paction_ImportantMax);
    }
    QMenu *menuModif  = menu->addMenu(tr("Modifier"));
    menuModif         ->setIcon(Icons::icEditer());
    menuModif->addAction(paction_Modifier);
    if (docmt->format() == VIDEO || docmt->format() == DOCUMENTRECU)
        menuModif->addAction(paction_ModifierDate);
    connect (paction_ImportantMin,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Min);});
    connect (paction_ImportantNorm, &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Norm);});
    connect (paction_ImportantMax,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Max);});
    connect (paction_Modifier,      &QAction::triggered,    this,  [=] {ModifierItem(idx);});
    connect (paction_ModifierDate,  &QAction::triggered,    this,  [=] {ModifierDate(idx);});

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
        if (m_currentuser->isMedecin()
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
    if (m_currentuser->isMedecin())
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
            break;
        }
        case 2:{
            fontitem.setBold(true);
            item->setFont(fontitem);
            item->setIcon(Icons::icImportant());
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
    ItemsList::update(docmt, CP_IMPORTANCE_IMPRESSIONS, imp);
    int nimportants = 0;

    QMapIterator<int, DocExterne*> itdoc (*m_docsexternes->docsexternes());
    while (itdoc.hasNext())
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.next().value());
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
    RecordButton                ->setVisible((docmt->format() == VIDEO || docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU) && DataBase::I()->getMode() != DataBase::Distant);
    RecordButton                ->disconnect();
    QPixmap pix;
    glistPix    .clear();
    ScrollTable ->clear();
    ScrollTable ->setVisible(false);
    GraphicView ->setVisible(false);
    Scene       ->clear();
    double x;
    double y;

    if (docmt->format() == VIDEO)  // le document est une video -> n'est pas stocké dans la base mais dans un fichier sur le disque
    {
        if (DataBase::I()->getMode() == DataBase::Distant)
        {
            UpMessageBox::Watch(this, tr("Video non accessible en accès distant"));
            return;
        }
        QString NomOnglet, NomDirStockageImagerie;
        if (DataBase::I()->getMode() == DataBase::Poste)
            NomOnglet = tr("Monoposte");
        if (DataBase::I()->getMode() == DataBase::ReseauLocal)
            NomOnglet = tr("Réseau local");
        NomDirStockageImagerie  = proc->DirImagerie();
        if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
        {
            QString msg = tr("Le dossier de sauvegarde d'imagerie ") + "<font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
            QString msg2 = tr("Renseignez un dossier valide dans Editions/Paramètres/Onglet \"ce poste\"/Onglet \"") + NomOnglet + "\"";
            UpMessageBox::Watch(this,msg, msg2);
            return;
        }
        QString filename = NomDirStockageImagerie + DIR_VIDEOS "/" + docmt->lienversfichier();
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
        player                  = new QMediaPlayer;
        player                  ->setMedia(QUrl::fromLocalFile(filename));
        playctrl                ->setPlayer(player);
        videoItem               ->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        player                  ->setVideoOutput(videoItem);
        playctrl                ->setVisible(true);
        PrintButton             ->setVisible(false);
        connect (RecordButton,  &QPushButton::clicked,   this,   &dlg_docsexternes::EnregistreVideo);
        x = videoItem->size().width();
        y = videoItem->size().height();
        idealproportion = x/y;
        videoItem               ->setSize(QSize(GraphicView->width(),GraphicView->height()));
        x = videoItem->size().width();
        y = videoItem->size().height();
        Scene->setSceneRect(1,1,x-1,y-1);
        playctrl                ->startplay();
    }
    else                                    // le document est une image ou un document écrit (ordonnance, certificat...)
    {
        bool pict = (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU);
        if (docmt->imageformat() == QByteArray())
            proc->CalcImage(docmt, pict, true);
        connect (RecordButton,  &QPushButton::clicked,   this,  [=] {EnregistreImage(docmt);});
        if (docmt->imageformat() == JPG)     // le document est un JPG
        {
            inflabel->setParent(GraphicView);
            gTypeDoc                = JPG;
            GraphicView             ->setVisible(true);
            QImage image;
            if (!image.loadFromData(docmt->imageblob()))
            {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                return;
            }
            pix = QPixmap::fromImage(image).scaled(QSize(QGuiApplication::screens().first()->geometry().width(),
                                                         QGuiApplication::screens().first()->geometry().height()),
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
        else if (docmt->imageformat() == PDF)     // le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage)
        {
            Poppler::Document* document = Poppler::Document::loadFromData(docmt->imageblob());
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
                pix = QPixmap::fromImage(image).scaled(QSize(QGuiApplication::screens().first()->geometry().width(),
                                                             QGuiApplication::screens().first()->geometry().height()),
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
                connect(lab,    &UpLabel::clicked,                      this, &dlg_docsexternes::ZoomDoc);
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
        const double maxwscroll  = QGuiApplication::screens().first()->geometry().width()*2/3    - wdelta - wdeltaframe;
        const double maxhscroll  = QGuiApplication::screens().first()->geometry().height()       - hdelta - hdeltaframe;
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

        if ((w + wdeltaframe) > (QGuiApplication::screens().first()->geometry().width() - this->x()))
            move(QGuiApplication::screens().first()->geometry().width() - (w + wdeltaframe), 0);
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

    QItemSelectionModel *m = ListDocsTreeView->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
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

void dlg_docsexternes::ActualiseDocsExternes()
{
    m_docsexternes->actualise();
    if (m_docsexternes->NouveauDocument())
    {
        m_docsexternes->setNouveauDocumentFalse();
        RemplirTreeView();
    }
}

void dlg_docsexternes::EnregistreImage(DocExterne *docmt)
{
    QString filename = proc->DirImagerie() + DIR_IMAGES + docmt->lienversfichier();
    QFile img(filename);
    if (!img.open(QIODevice::ReadOnly))
    {
        UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    QFileDialog dialog(this, tr("Enregistrer un fichier"), QDir::homePath());
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        img.copy(dockdir.path() + "/" + m_currentpatient->prenom() + " " + m_currentpatient->nom() + " "+ docmt->soustypedoc() + "." + QFileInfo(img).suffix());
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
    else if (chk == AllDocsupCheckBox)
    {
        if (chk->isChecked())
            OnlyImportantDocsupCheckBox->setChecked(false);
    }
    RemplirTreeView();                              // après FiltrerListe()
}

DocExterne* dlg_docsexternes::getDocumentFromIndex(QModelIndex idx)
{
    QStandardItem *it = gmodele->itemFromIndex(idx);
    if (it == Q_NULLPTR || it->hasChildren())
        return Q_NULLPTR;
    int idimpr = it->data().toMap().value("id").toInt();
    return m_docsexternes->getById(idimpr);
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
    docmt               = m_docsexternes->getById(docmt->id());
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
        if (m_currentuser->isMedecin()
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

    User *userEntete = Datas::I()->users->getById(docmt->iduser(), Item::LoadDetails);
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
    Entete.replace("{{PRENOM PATIENT}}", (Prescription? m_currentpatient->prenom()        : ""));
    Entete.replace("{{NOM PATIENT}}"   , (Prescription? m_currentpatient->nom().toUpper() : ""));

    //création du pied
    Pied = proc->ImpressionPied(userEntete, docmt->format() == PRESCRIPTIONLUNETTES, ALD);

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
        Utils::nettoieHTML(Corps);

        QHash<QString,QVariant> listbinds;
        listbinds[CP_IDUSER_IMPRESSIONS]        = docmt->iduser();
        listbinds[CP_IDPAT_IMPRESSIONS]         = docmt->idpatient();
        listbinds[CP_TYPEDOC_IMPRESSIONS]       = docmt->typedoc();
        listbinds[CP_SOUSTYPEDOC_IMPRESSIONS]   = docmt->soustypedoc();
        listbinds[CP_TITRE_IMPRESSIONS]         = docmt->titre();
        listbinds[CP_TEXTENTETE_IMPRESSIONS]    = Entete;
        listbinds[CP_TEXTCORPS_IMPRESSIONS]     = Corps;
        listbinds[CP_TEXTORIGINE_IMPRESSIONS]   = txt;
        listbinds[CP_TEXTPIED_IMPRESSIONS]      = Pied;
        listbinds[CP_DATE_IMPRESSIONS]          = db->ServerDateTime().toString("yyyy-MM-dd HH:mm:ss");
        listbinds[CP_FORMATDOC_IMPRESSIONS]     = docmt->format();
        listbinds[CP_IDLIEU_IMPRESSIONS]        = m_currentuser->getSite()->id();
        listbinds[CP_ALD_IMPRESSIONS]           = (ALD? "1" : QVariant(QVariant::String));
        listbinds[CP_IDEMETTEUR_IMPRESSIONS]    = m_currentuser->id();
        listbinds[CP_IMPORTANCE_IMPRESSIONS]    = docmt->importance();
        DocExterne * doc = m_docsexternes->CreationDocument(listbinds);
        if (doc != Q_NULLPTR)
        {
            int idimpr = doc->id();
            delete doc;
            if (detruirealafin)
            {
                if (docmt->idrefraction() > 0)
                    Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(docmt->idrefraction()));
                m_docsexternes->SupprimeDocument(docmt);
            }
            ActualiseDocsExternes();
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
    if (docmt->imageblob() == QByteArray())
        proc->CalcImage(docmt, pict, false);
    if (docmt->imageformat() == PDF)     // le document est un pdf ou un document texte
    {
        Poppler::Document* document = Poppler::Document::loadFromData(docmt->imageblob());
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
    else if (docmt->imageformat() == JPG)     // le document est un jpg
    {
        QPixmap pix;
        pix.loadFromData(docmt->imageblob());
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

void dlg_docsexternes::ModifierDate(QModelIndex idx)
{
    DocExterne *docmt = getDocumentFromIndex(idx);
    UpDialog * dlg              = new UpDialog();
    dlg                         ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);;
    QDateEdit   *dateedit       = new QDateEdit(dlg);
    UpLabel     *label          = new UpLabel(dlg);
    dlg->dlglayout()->insertWidget(0,dateedit);
    dlg->dlglayout()->insertWidget(0,label);
    dlg->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);

    dlg->setModal(true);
    dlg->setSizeGripEnabled(false);
    dlg->setFixedSize(200,100);
    dlg->move(QPoint(x()+width()/2,y()+height()/2));
    dlg->setWindowTitle(tr("Modifier la date"));
    dateedit->setDate(docmt->date().date());
    dateedit->setSelectedSection(QDateTimeEdit::DaySection);

    connect(dlg->OKButton,   &QPushButton::clicked,   [=]
    {
        if (dateedit->date().isValid())
        {
            ItemsList::update(docmt, CP_DATE_IMPRESSIONS, QDateTime(dateedit->date()));
            RemplirTreeView();
            dlg->accept();
        }
        else
        {
            QSound::play(NOM_ALARME);
            QToolTip::showText(cursor().pos(),tr("Vous devez entrer une date valide"));
        }
    });

    label->setText(tr("Entrez la date du document"));
    dlg->exec();
    delete dlg;
}

void dlg_docsexternes::ModifierItem(QModelIndex idx)
{
    DocExterne *docmt = getDocumentFromIndex(idx);
    UpDialog * dlg              = new UpDialog();
    dlg                         ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
    UpLineEdit  *Line           = new UpLineEdit(dlg);
    UpLabel     *label          = new UpLabel(dlg);
    dlg->dlglayout()->insertWidget(0,Line);
    dlg->dlglayout()->insertWidget(0,label);
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
            ItemsList::update(docmt, CP_SOUSTYPEDOC_IMPRESSIONS, Line->text());
            QString titre = CalcTitre(docmt);
            gmodele->itemFromIndex(idx)->setText(titre);
            int id = docmt->id();
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
    if (!m_currentuser->isSoignant())         //le user n'est pas un soignant
    {
        if (docmt->useremetteur() != DataBase::I()->getUserConnected()->id())
        {
            UpMessageBox::Watch(this,tr("Suppression refusée"), tr("Vous ne pouvez pas supprimer un document dont vous n'êtes pas l'auteur"));
            return;
        }
    }
    if (docmt->id() > 0)
    {
        UpMessageBox msgbox;
        UpSmallButton OKBouton(tr("Supprimer"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.setText("Euuhh... " + m_currentuser->getLogin());
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
            QString cheminFichier = (docmt->format()== VIDEO? DIR_VIDEOS : DIR_IMAGES);
            filename = cheminFichier + filename;
            db->StandardSQL("insert into " TBL_DOCSASUPPRIMER " (FilePath) VALUES ('" + Utils::correctquoteSQL(filename) + "')");
        }
        QString idaafficher = "";
        if (m_docsexternes->docsexternes()->size() > 1)    // on recherche le document sur qui va être mis la surbrillance après la suppression
        {
            QMap<int, DocExterne*>* listaexplorer = m_docsexternes->docsexternes();
            QMapIterator<int, DocExterne*> itdoc (*listaexplorer);
            if (!itdoc.findPrevious(docmt))
                itdoc.next();
            idaafficher = QString::number(itdoc.key());
        }
        if (docmt->idrefraction() > 0)
            Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(docmt->idrefraction()));
        m_docsexternes->SupprimeDocument(docmt);
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
        const double maxwscroll  = QGuiApplication::screens().first()->geometry().width()*2/3    - wdelta - wdeltaframe;
        const double maxhscroll  = QGuiApplication::screens().first()->geometry().height()       - hdelta - hdeltaframe;
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
        move (QGuiApplication::screens().first()->geometry().width() - w, 0);
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
    if (m_docsexternes->docsexternes()->size() == 0){
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

    QMapIterator<int, DocExterne*> itdoc (*m_docsexternes->docsexternes());
    while (itdoc.hasNext())
    {
        DocExterne *doc = const_cast<DocExterne*>(itdoc.next().value());
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
    std::sort(listdates.begin(), listdates.end());
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
    std::sort(listtypes.begin(), listtypes.end());
    for (int i=0; i<listtypes.size(); ++i)
    {
        typitem     = new QStandardItem(listtypes.at(i));
        typitem     ->setForeground(QBrush(QColor(Qt::red)));
        typitem     ->setEditable(false);
        typitem     ->setIcon(Icons::icSortirDossier());
        rootNodeType->appendRow(typitem);
    }

    itdoc.toFront();
    while (itdoc.hasNext())
    {
        itdoc.next();
        DocExterne *doc = const_cast<DocExterne*>(itdoc.value());      // rajout des items de chaque examen en child des dates et des types
        QString date = doc->date().toString(tr("dd-MM-yyyy"));
        QString a = doc->typedoc();
        pitemdate           = new QStandardItem(CalcTitre(doc));
        pitemtype           = new QStandardItem(CalcTitre(doc));
        pitemtridated        = new QStandardItem(doc->date().toString("yyyyMMddHHmmss"));
        pitemtridatet        = new QStandardItem(doc->date().toString("yyyyMMddHHmmss"));
        QMap<QString, QVariant> data;
        data.insert("id", QString::number(doc->id()));
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
        //        QMap<int, DocExterne*>::const_iterator itdoc = m_docsexternes->docsexternes()->find(idimpraretrouver.toInt());
        //        if (itdoc != m_docsexternes->docsexternes()->constEnd())
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

