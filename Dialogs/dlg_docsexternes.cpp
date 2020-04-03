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

#include "dlg_docsexternes.h"
#include "gbl_datas.h"
#include "icons.h"

dlg_docsexternes::dlg_docsexternes(DocsExternes *Docs, bool UtiliseTCP, QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionDocsExternes", parent)
{
    m_docsexternes  = Docs;

    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    installEventFilter(this);
    setMaximumHeight(QGuiApplication::screens().first()->geometry().height());
    setWindowTitle(tr("Documents de ") + m_docsexternes->patient()->prenom() + " " + m_docsexternes->patient()->nom());

    QFont font  = qApp->font();
    font        .setPointSize(font.pointSize()+2);
    int d=0;
#ifdef QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE
    d=2;
#endif
    m_font      .setPointSize(m_font.pointSize()-d);
    int margemm         = proc->TailleTopMarge(); // exprimé en mm
    m_printer           = new QPrinter(QPrinter::HighResolution);
    m_printer           ->setFullPage(true);
    m_rect              = m_printer->paperRect();

    m_rect.adjust(Utils::mmToInches(margemm) * m_printer->logicalDpiX(),
                  Utils::mmToInches(margemm) * m_printer->logicalDpiY(),
                  - Utils::mmToInches(margemm) * m_printer->logicalDpiX(),
                  - Utils::mmToInches(margemm) * m_printer->logicalDpiY());

    obj_graphicscene        = new QGraphicsScene(this);
    wdg_listdocstreewiew    = new QTreeView(this);
    wdg_inflabel            = new QLabel();
    wdg_inflabel            ->setFont(font);


    wdg_scrolltablewidget   = new UpTableWidget(this);                      // utilisé pour afficher les pdf qui ont parfois plusieurs pages
    wdg_scrolltablewidget->horizontalHeader() ->setVisible(false);
    wdg_scrolltablewidget->verticalHeader()   ->setVisible(false);
    wdg_scrolltablewidget   ->installEventFilter(this);
    wdg_scrolltablewidget   ->setFocusPolicy(Qt::NoFocus);
    wdg_scrolltablewidget   ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    wdg_scrolltablewidget   ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_scrolltablewidget   ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30

    graphview_view          = new QGraphicsView(obj_graphicscene, this);               // utilisé pour afficher les jpg et les video
    graphview_view          ->installEventFilter(this);
    graphview_view          ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphview_view          ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphview_view          ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30

    wdg_listdocstreewiew    ->setFixedWidth(185);
    wdg_listdocstreewiew    ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_listdocstreewiew    ->setFont(m_font);
    wdg_listdocstreewiew    ->setEditTriggers(QAbstractItemView::DoubleClicked);
    wdg_listdocstreewiew    ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_listdocstreewiew    ->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_listdocstreewiew    ->setAnimated(true);
    wdg_listdocstreewiew    ->setIndentation(6);
    wdg_listdocstreewiew    ->header()->setVisible(false);


    QHBoxLayout *lay    = new QHBoxLayout();
    lay                 ->addWidget(wdg_listdocstreewiew,2);
    lay                 ->addWidget(wdg_scrolltablewidget,8);
    lay                 ->addWidget(graphview_view,8);
    lay                 ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    lay                 ->setSpacing(10);
    dlglayout()         ->insertLayout(0,lay);

    wdg_playctrl            = new PlayerControls(this);
    AjouteWidgetLayButtons(wdg_playctrl, false);
    buttonslayout()->insertSpacerItem(0, new QSpacerItem(20,20,QSizePolicy::Fixed, QSizePolicy::Fixed));
    wdg_alldocsupcheckbox               = new UpCheckBox(tr("Tous"));
    wdg_onlyimportantsdocsupcheckbox    = new UpCheckBox(tr("Importants"));
    wdg_alldocsupcheckbox               ->setImmediateToolTip(tr("Afficher tous les documents\ny compris les documents d'importance minime"));
    wdg_onlyimportantsdocsupcheckbox    ->setImmediateToolTip(tr("N'afficher que les documents marqués importants"));
    AjouteWidgetLayButtons(wdg_alldocsupcheckbox, false);
    AjouteWidgetLayButtons(wdg_onlyimportantsdocsupcheckbox, false);
    wdg_alldocsupcheckbox               ->setChecked(true);

    wdg_upswitch        = new UpSwitch(this);
    AjouteWidgetLayButtons(wdg_upswitch, false);
    AjouteLayButtons(UpDialog::ButtonRecord | UpDialog::ButtonSuppr | UpDialog::ButtonPrint);
    //setStageCount(1);

    m_hdelta            = 0;
    m_wdelta            = 0;
    m_hdeltaframe       = 0;
    m_wdeltaframe       = 0;
    m_avecprevisu = proc  ->ApercuAvantImpression();

    /*Gestion des XML - exemple
    QString adressexml = PATHTODIR_RUFUS + "/XML/" + QString::number(idpat) + "/Exam_Data.xml";
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


    connect (wdg_upswitch,                      &UpSwitch::Bascule,             this,   [=] {BasculeTriListe(wdg_upswitch->PosSwitch());});
    connect (SupprButton,                       &QPushButton::clicked,          this,   [=] {SupprimeDoc();});
    connect (wdg_alldocsupcheckbox,             &QCheckBox::toggled,            this,   [=] {FiltrerListe(wdg_alldocsupcheckbox);});
    connect (wdg_onlyimportantsdocsupcheckbox,  &QCheckBox::toggled,            this,   [=] {FiltrerListe(wdg_onlyimportantsdocsupcheckbox);});
    connect (wdg_playctrl,                      &PlayerControls::ctrl,          this,   [=] (PlayerControls::State  state) {    switch (state){
                                                                                                                                case PlayerControls::Stop:  medplay_player->stop();     break;
                                                                                                                                case PlayerControls::Pause: medplay_player->pause();    break;
                                                                                                                                case PlayerControls::Play:  medplay_player->play();
                                                                                                                                }
                                                                                                                        });
    connect (proc,                              &Procedures::UpdDocsExternes,   this,   &dlg_docsexternes::ActualiseDocsExternes);
    connect (PrintButton,                       &QPushButton::clicked,          this,   &dlg_docsexternes::ImprimeDoc);

    if (!UtiliseTCP)
    {
        QTimer *TimerActualiseDocsExternes      = new QTimer(this);
        TimerActualiseDocsExternes              ->start(10000);
        connect (TimerActualiseDocsExternes,    &QTimer::timeout,               this,   &dlg_docsexternes::ActualiseDocsExternes);
    }

    m_mode          = Normal;
    m_modetri       = parDate;
    m_docsexternes  ->setNouveauDocumentExterneFalse();
    RemplirTreeView();
}

dlg_docsexternes::~dlg_docsexternes()
{
    delete m_printer;
}

void dlg_docsexternes::AfficheCustomMenu(DocExterne *docmt)
{
    QModelIndex idx = getIndexFromId(m_model, docmt->id());
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
    if (currentuser()->isMedecin())
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
        if (currentuser()->isMedecin()
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
    if (currentuser()->isMedecin())
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

    int imp;
    switch (imptce) {
    case Min:   imp = 0; break;
    case Norm:  imp = 1; break;
    case Max:   imp = 2; break;
    }
    QStandardItem *item = getItemFromDocument(m_model, docmt);
    if (item == Q_NULLPTR)
        return;
    int id = docmt->id();
    modifieitem(item, docmt, imp, m_font);
    item = m_tripardatemodel->itemFromIndex(getIndexFromId(m_tripardatemodel,id));
    if (item != Q_NULLPTR)
        modifieitem(item, docmt, imp, m_font);
    item = m_tripartypemodel->itemFromIndex(getIndexFromId(m_tripartypemodel,id));
    if (item != Q_NULLPTR)
        modifieitem(item, docmt, imp, m_font);
    ItemsList::update(docmt, CP_IMPORTANCE_DOCSEXTERNES, imp);

    bool hasimportants = false;
    foreach (DocExterne *doc, *m_docsexternes->docsexternes())
    {
        hasimportants = (doc->importance() == 2);
        if (hasimportants)
            break;
    }
    wdg_onlyimportantsdocsupcheckbox->setEnabled( hasimportants || wdg_onlyimportantsdocsupcheckbox->isChecked());
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
    wdg_playctrl                ->setVisible(false);
    PrintButton                 ->setVisible(true);
    PrintButton                 ->setEnabled(true);
    SupprButton                 ->setEnabled(true);
    RecordButton                ->setVisible((docmt->format() == VIDEO || docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU) && DataBase::I()->ModeAccesDataBase() != Utils::Distant);
    RecordButton                ->disconnect();
    QPixmap pix;
    m_listpixmp    .clear();
    wdg_scrolltablewidget ->clear();
    wdg_scrolltablewidget ->setVisible(false);
    graphview_view ->setVisible(false);
    obj_graphicscene       ->clear();
    double x;
    double y;

    if (docmt->format() == VIDEO)  // le document est une video -> n'est pas stocké dans la base mais dans un fichier sur le disque
    {
        if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
        {
            UpMessageBox::Watch(this, tr("Video non accessible en accès distant"));
            return;
        }
        QString NomOnglet, NomDirStockageImagerie;
        if (DataBase::I()->ModeAccesDataBase() == Utils::Poste)
            NomOnglet = tr("Monoposte");
        if (DataBase::I()->ModeAccesDataBase() == Utils::ReseauLocal)
            NomOnglet = tr("Réseau local");
        NomDirStockageImagerie  = proc->AbsolutePathDirImagerie();
        if (!QDir(NomDirStockageImagerie).exists() || NomDirStockageImagerie == "")
        {
            QString msg = tr("Le dossier de sauvegarde d'imagerie ") + "<font color=\"red\"><b>" + NomDirStockageImagerie + "</b></font>" + tr(" n'existe pas");
            QString msg2 = tr("Renseignez un dossier valide dans Editions/Paramètres/Onglet \"ce poste\"/Onglet \"") + NomOnglet + "\"";
            UpMessageBox::Watch(this,msg, msg2);
            return;
        }
        QString filename = NomDirStockageImagerie + NOM_DIR_VIDEOS "/" + docmt->lienversfichier();
        QFile   qFile(filename);
        if (!qFile.open(QIODevice::ReadOnly))
        {
            UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
            return;
        }
        wdg_inflabel->setParent(graphview_view);
        QString sstitre = "<font color='magenta'>" + docmt->date().toString(tr("d-M-yyyy")) + " - " + docmt->soustypedoc() + "</font>";
        wdg_inflabel    ->setText(sstitre);

        m_typedoc           = VIDEO;
        graphview_view      ->setVisible(true);
        medobj_videoitem    = new QGraphicsVideoItem;
        obj_graphicscene    ->addItem(medobj_videoitem);
        medplay_player      = new QMediaPlayer;
        medplay_player      ->setMedia(QUrl::fromLocalFile(filename));
        wdg_playctrl        ->setPlayer(medplay_player);
        medobj_videoitem    ->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        medplay_player      ->setVideoOutput(medobj_videoitem);
        wdg_playctrl        ->setVisible(true);
        PrintButton         ->setVisible(false);
        connect (RecordButton,  &QPushButton::clicked,   this,   &dlg_docsexternes::EnregistreVideo);
        x = medobj_videoitem->size().width();
        y = medobj_videoitem->size().height();
        m_idealproportion   = x/y;
        medobj_videoitem    ->setSize(QSize(graphview_view->width(),graphview_view->height()));
        x = medobj_videoitem->size().width();
        y = medobj_videoitem->size().height();
        obj_graphicscene    ->setSceneRect(1,1,x-1,y-1);
        wdg_playctrl        ->startplay();
    }
    else                                    // le document est une image ou un document écrit (ordonnance, certificat...)
    {
        bool pict = (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU);
        if (docmt->imageformat() == QByteArray())
            proc->CalcImage(docmt, pict, true);
        connect (RecordButton,  &QPushButton::clicked,   this,  [=] {EnregistreImage(docmt);});
        if (docmt->imageformat() == JPG)     // le document est un JPG
        {
            wdg_inflabel->setParent(graphview_view);
            m_typedoc                = JPG;
            graphview_view             ->setVisible(true);
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
            m_idealproportion = x/y;
            m_listpixmp << pix;
            pix = QPixmap::fromImage(image).scaled(QSize(graphview_view->width()-2,graphview_view->height()-2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            obj_graphicscene   ->addPixmap(pix);
            x = pix.size().width();
            y = pix.size().height();
            obj_graphicscene   ->setSceneRect(1,1,x-1,y-1);
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
            wdg_inflabel->setParent(wdg_scrolltablewidget);
            m_typedoc    = PDF;
            wdg_scrolltablewidget ->setVisible(true);
            wdg_scrolltablewidget ->setColumnCount(1);
            wdg_scrolltablewidget ->setColumnWidth(0,wdg_scrolltablewidget->width()-2);
            document->setRenderHint(Poppler::Document::TextAntialiasing);
            int numpages = document->numPages();
            for (int i=0; i<numpages ;i++)
            {
                wdg_scrolltablewidget->setRowCount(numpages);
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
                    m_idealproportion = x/y;
                }
                m_listpixmp << pix;
                pix = QPixmap::fromImage(image).scaled(QSize(wdg_scrolltablewidget->width()-2,wdg_scrolltablewidget->height()-2),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
                wdg_scrolltablewidget->setRowHeight(i,pix.height());
                UpLabel *lab = new UpLabel(wdg_scrolltablewidget);
                lab->resize(pix.width(),pix.height());
                lab->setPixmap(pix);
                lab->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(lab,    &UpLabel::clicked,                      this, &dlg_docsexternes::ZoomDoc);
                connect(lab,    &UpLabel::customContextMenuRequested,   this, [=] {AfficheCustomMenu(docmt);});
                delete pdfPage;
                wdg_scrolltablewidget->setCellWidget(i,0,lab);
            }
            delete document;
        }
        else return;
    }
    if (m_mode == Zoom)
    {
        // les dimensions maxi de la zone de visu
        const double maxwscroll  = QGuiApplication::screens().first()->geometry().width()*2/3    - m_wdelta - m_wdeltaframe;
        const double maxhscroll  = QGuiApplication::screens().first()->geometry().height()       - m_hdelta - m_hdeltaframe;
        // les dimensions calculées de la zone de visu
        int wfinal(0), hfinal(0);

        double proportion = maxwscroll/maxhscroll;
        if (m_idealproportion > proportion)
        {   wfinal  = int(maxwscroll);   hfinal  = int(wfinal / m_idealproportion); }
        else
        {   hfinal  = int(maxhscroll);   wfinal  = int(hfinal * m_idealproportion); }

        int w = wfinal + m_wdelta;
        int h = hfinal + m_hdelta;
        resize(w, h);
        if (m_typedoc == PDF)
        {
            for (int i=0; i < wdg_scrolltablewidget->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_scrolltablewidget->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(m_listpixmp.at(i).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    wdg_scrolltablewidget->setRowHeight(i,hfinal-2);
                    wdg_scrolltablewidget->setColumnWidth(0,wfinal-2);
                    if (i==0) wdg_scrolltablewidget->resize(wfinal, hfinal);
                }
            }
        }
        else if (m_typedoc == VIDEO)
        {
            graphview_view ->resize(wfinal, hfinal);
            medobj_videoitem   ->setSize(QSize(wfinal-2, hfinal-2));
            obj_graphicscene->setSceneRect(1,1,wfinal-1,hfinal-1);
        }
        else if (m_typedoc == JPG)
        {
            graphview_view ->resize(wfinal, hfinal);
            obj_graphicscene->clear();
            QPixmap pix = m_listpixmp.at(0).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            obj_graphicscene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            obj_graphicscene->setSceneRect(1,1,x-1,y-1);
        }

        if ((w + m_wdeltaframe) > (QGuiApplication::screens().first()->geometry().width() - this->x()))
            move(QGuiApplication::screens().first()->geometry().width() - (w + m_wdeltaframe), 0);
    }

    if (m_typedoc == PDF)
        wdg_inflabel    ->setGeometry(10,wdg_scrolltablewidget->viewport()->height()-40, 500, 25);
    else
        wdg_inflabel    ->setGeometry(10,graphview_view->height() -40, 500, 25);
}

void dlg_docsexternes::BasculeTriListe(int a)
{
    QString             idimpraretrouver = "";
    wdg_listdocstreewiew    ->disconnect();
    if (wdg_listdocstreewiew->selectionModel()->selectedIndexes().size()>0)
    {
        QModelIndex actifidx = wdg_listdocstreewiew->selectionModel()->selectedIndexes().at(0);
        if (!m_model->itemFromIndex(actifidx)->hasChildren())
            idimpraretrouver = m_model->itemFromIndex(actifidx)->data().toMap().value("id").toString();
    }
    if (m_model == Q_NULLPTR)
        delete m_model;
    if (a == 0)
    {
        m_modetri = parDate;
        m_model  = m_tripardatemodel;
    }
    else if (a == 1)
    {
        m_modetri = parType;
        m_model = m_tripartypemodel;
    }

    QItemSelectionModel *m = wdg_listdocstreewiew->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    wdg_listdocstreewiew->setModel(m_model);
    delete m;

    int nrows = m_model->item(m_model->rowCount()-1)->rowCount()-1;                 // le nombre de child du dernier item date
    QStandardItem *item =  m_model->item(m_model->rowCount()-1)->child(nrows,0);    // le tout dernier item
    QModelIndex idx = item->index();                                                // l'index de ce dernier item
    if (idimpraretrouver != "")
    {
        QModelIndex indx = getIndexFromId(m_model, idimpraretrouver.toInt());
        if (indx.isValid())
            idx = indx;
    }
    wdg_listdocstreewiew->setSelectionModel(new QItemSelectionModel(m_model));
    wdg_listdocstreewiew->expandAll();
    wdg_listdocstreewiew->scrollTo(idx, QAbstractItemView::PositionAtCenter);
    wdg_listdocstreewiew->setCurrentIndex(idx);
    connect(wdg_listdocstreewiew->selectionModel()  ,   &QItemSelectionModel::currentChanged,   this,   [=] {AfficheDoc(wdg_listdocstreewiew->selectionModel()->currentIndex());});
    connect(wdg_listdocstreewiew,                       &QTreeView::customContextMenuRequested, this,   [=] {
        QModelIndex idx = wdg_listdocstreewiew->indexAt(wdg_listdocstreewiew->mapFromGlobal(cursor().pos()));
        DocExterne *docmt = getDocumentFromIndex(idx);
        if (docmt != Q_NULLPTR)
            AfficheCustomMenu(docmt);
    });
}

void dlg_docsexternes::ActualiseDocsExternes()
{
    m_docsexternes->actualise();
    if (m_docsexternes->NouveauDocumentExterne())
    {
        m_docsexternes->setNouveauDocumentExterneFalse();
        RemplirTreeView();
    }
}

void dlg_docsexternes::EnregistreImage(DocExterne *docmt)
{
    QString filename = proc->AbsolutePathDirImagerie() + NOM_DIR_IMAGES + docmt->lienversfichier();
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
        img.copy(dockdir.path() + "/" + m_docsexternes->patient()->prenom() + " " + m_docsexternes->patient()->nom() + " "+ docmt->soustypedoc() + "." + QFileInfo(img).suffix());
    }
}

void dlg_docsexternes::EnregistreVideo()
{
    QString filename = medplay_player->media().canonicalUrl().path();
    QFileDialog dialog(this, tr("Enregistrer un fichier"), QDir::homePath());
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec()>0)
    {
        QDir dockdir = dialog.directory();
        QFile(filename).copy(dockdir.path() + "/" + medplay_player->media().canonicalUrl().fileName());
    }
}

void dlg_docsexternes::FiltrerListe(UpCheckBox *chk)
{
    if (chk == wdg_onlyimportantsdocsupcheckbox)
    {
        if (chk->isChecked())
            wdg_alldocsupcheckbox->setChecked(false);
    }
    else if (chk == wdg_alldocsupcheckbox)
    {
        if (chk->isChecked())
            wdg_onlyimportantsdocsupcheckbox->setChecked(false);
    }
    RemplirTreeView();                              // après FiltrerListe()
}

DocExterne* dlg_docsexternes::getDocumentFromIndex(QModelIndex idx)
{
    QStandardItem *it = m_model->itemFromIndex(idx);
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
    return m_model->itemFromIndex(idx);
}

void dlg_docsexternes::ImprimeDoc()
{
#ifndef QT_NO_PRINTER
    PrintButton         ->disconnect();  // pour éviter le doubles impressions en cas de double clic lent
    DocExterne * docmt  = getDocumentFromIndex(wdg_listdocstreewiew->selectionModel()->selectedIndexes().at(0));
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
        if (currentuser()->isMedecin()
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

    User *userEntete = Datas::I()->users->getById(docmt->iduser());
    if (userEntete == Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
        return false;
    }

    //création de l'entête
    QMap<QString,QString> EnteteMap = proc->CalcEnteteImpression(QDate::currentDate(), userEntete);
    Entete = (ALD? EnteteMap.value("ALD") : EnteteMap.value("Norm"));
    if (Entete == "")
        return false;
    Entete.replace("{{TITRE1}}"        , "");
    Entete.replace("{{TITRE}}"         , "");
    Entete.replace("{{DDN}}"           , "");
    Entete.replace("{{PRENOM PATIENT}}", (Prescription? m_docsexternes->patient()->prenom()        : ""));
    Entete.replace("{{NOM PATIENT}}"   , (Prescription? m_docsexternes->patient()->nom().toUpper() : ""));

    //création du pied
    Pied = proc->CalcPiedImpression(userEntete, docmt->format() == PRESCRIPTIONLUNETTES, ALD);

    // creation du corps de l'ordonnance
    QString txtautiliser    = (docmt->textorigine() == ""?              docmt->textcorps()              : docmt->textorigine());
    txt                     = (modifiable?                              proc->Edit(txtautiliser)        : txtautiliser);
    Corps                   = (docmt->typedoc() == PRESCRIPTION?        proc->CalcCorpsImpression(txt,ALD)  : proc->CalcCorpsImpression(txt));
    Etat_textEdit           ->setHtml(Corps);
    if (Etat_textEdit->toPlainText() == "" || txt == "")
    {
        if (Etat_textEdit->toPlainText() == "")
            UpMessageBox::Watch(this,tr("Rien à imprimer"));
        return false;
    }
    int TailleEnTete        = (ALD?                                     proc->TailleEnTeteALD()         : proc->TailleEnTete());
    bool AvecDupli          = (proc->settings()->value("Param_Imprimante/OrdoAvecDupli").toString() == "YES"
                               && docmt->typedoc() == PRESCRIPTION);
    bool AvecChoixImprimante = true;

    aa = proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                            proc->TaillePieddePage(), TailleEnTete, proc->TailleTopMarge(),
                            AvecDupli, m_avecprevisu, AvecNumPage, AvecChoixImprimante);

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        Utils::nettoieHTML(Corps);

        QHash<QString, QVariant> listbinds;
        listbinds[CP_IDUSER_DOCSEXTERNES]        = docmt->iduser();
        listbinds[CP_IDPAT_DOCSEXTERNES]         = docmt->idpatient();
        listbinds[CP_TYPEDOC_DOCSEXTERNES]       = docmt->typedoc();
        listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES]   = docmt->soustypedoc();
        listbinds[CP_TITRE_DOCSEXTERNES]         = docmt->titre();
        listbinds[CP_TEXTENTETE_DOCSEXTERNES]    = Entete;
        listbinds[CP_TEXTCORPS_DOCSEXTERNES]     = Corps;
        listbinds[CP_TEXTORIGINE_DOCSEXTERNES]   = txt;
        listbinds[CP_TEXTPIED_DOCSEXTERNES]      = Pied;
        listbinds[CP_DATE_DOCSEXTERNES]          = db->ServerDateTime().toString("yyyy-MM-dd HH:mm:ss");
        listbinds[CP_FORMATDOC_DOCSEXTERNES]     = docmt->format();
        listbinds[CP_IDLIEU_DOCSEXTERNES]        = Datas::I()->sites->idcurrentsite();
        listbinds[CP_ALD_DOCSEXTERNES]           = (ALD? "1" : QVariant(QVariant::String));
        listbinds[CP_IDEMETTEUR_DOCSEXTERNES]    = currentuser()->id();
        listbinds[CP_IMPORTANCE_DOCSEXTERNES]    = docmt->importance();
        DocExterne * doc = m_docsexternes->CreationDocumentExterne(listbinds);
        if (doc != Q_NULLPTR)
        {
            int idimpr = doc->id();
            delete doc;
            if (detruirealafin)
            {
                if (docmt->idrefraction() > 0)
                    Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(docmt->idrefraction()));
                Datas::I()->docsexternes->SupprimeDocumentExterne(docmt);
            }
            ActualiseDocsExternes();
            QModelIndex idx = getIndexFromId(m_model, idimpr);
            wdg_listdocstreewiew->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            wdg_listdocstreewiew->setCurrentIndex(idx);
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
            img_image = pdfPage->renderToImage(600,600);
            if (img_image.isNull()) {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                delete document;
                return false;
            }
            // ... use image ...
            if (i == 0)
            {
                if (m_avecprevisu)
                {
                    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(m_printer, this);
                    connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(m_printer);});
                    dialog->exec();
                    delete dialog;
                }
                else
                {
                    QPrintDialog *dialog = new QPrintDialog(m_printer, this);
                    if (dialog->exec() != QDialog::Rejected)
                        Print(m_printer);
                    delete dialog;
                }
            }
            else
                Print(m_printer);
            delete pdfPage;
        }
        delete document;
    }
    else if (docmt->imageformat() == JPG)     // le document est un jpg
    {
        QPixmap pix;
        pix.loadFromData(docmt->imageblob());
        img_image= pix.toImage();
        if (m_avecprevisu)
        {
            QPrintPreviewDialog *dialog = new QPrintPreviewDialog(m_printer, this);
            connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(m_printer);});
            dialog->exec();
            delete dialog;
        }
        else
        {
            QPrintDialog *dialog = new QPrintDialog(m_printer, this);
            if (dialog->exec() != QDialog::Rejected)
                Print(m_printer);
            delete dialog;
        }
    }
    return true;
}

void dlg_docsexternes::ModifierDate(QModelIndex idx)
{
    DocExterne *docmt = getDocumentFromIndex(idx);
    UpDialog * dlg              = new UpDialog();
    dlg                         ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |Qt::WindowCloseButtonHint);
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
            ItemsList::update(docmt, CP_DATE_DOCSEXTERNES, QDateTime(dateedit->date()));
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
    Line->selectAll();

    connect(dlg->OKButton,   &QPushButton::clicked,   [=]
    {
        if (Line->text()!="")
        {
            ItemsList::update(docmt, CP_SOUSTYPEDOC_DOCSEXTERNES, Line->text());
            QString titre = CalcTitre(docmt);
            m_model->itemFromIndex(idx)->setText(titre);
            int id = docmt->id();
            m_tripardatemodel->itemFromIndex(getIndexFromId(m_tripardatemodel,id))->setText(titre);
            m_tripartypemodel->itemFromIndex(getIndexFromId(m_tripartypemodel,id))->setText(titre);
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
    QPixmap pix         = QPixmap::fromImage(img_image).scaledToWidth(int(m_rect.width()),Qt::SmoothTransformation);
    PrintingPreView.drawImage(QPoint(0,0),pix.toImage());
}

void dlg_docsexternes::SupprimeDoc(DocExterne *docmt)
{
    if (docmt == Q_NULLPTR)
    {
        QModelIndex idx = wdg_listdocstreewiew->selectionModel()->selectedIndexes().at(0);
        docmt = getDocumentFromIndex(idx);
    }
    if (docmt == Q_NULLPTR)
        return;
    if (!currentuser()->isSoignant())         //le user n'est pas un soignant
    {
        if (docmt->useremetteur() != Datas::I()->users->userconnected()->id())
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
        msgbox.setText("Euuhh... " + currentuser()->login());
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
            QString cheminFichier = (docmt->format()== VIDEO? NOM_DIR_VIDEOS : NOM_DIR_IMAGES);
            filename = cheminFichier + filename;
            db->StandardSQL("insert into " TBL_DOCSASUPPRIMER " (FilePath) VALUES ('" + Utils::correctquoteSQL(filename) + "')");
        }
        QString idaafficher = "";
        if (m_docsexternes->docsexternes()->size() > 1)    // on recherche le document sur qui va être mis la surbrillance après la suppression
        {
            auto itdoc =  m_docsexternes->docsexternes()->find(docmt->id());
            if (itdoc != m_docsexternes->docsexternes()->cbegin())
                itdoc --;
            else
                itdoc ++;
            idaafficher = QString::number(itdoc.key());
        }
        if (docmt->idrefraction() > 0)
            Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(docmt->idrefraction()));
        Datas::I()->docsexternes->SupprimeDocumentExterne(docmt);
        RemplirTreeView();
        wdg_listdocstreewiew->expandAll();
        if (idaafficher != "")
        {
            QModelIndex idx = getIndexFromId(m_model, idaafficher.toInt());
            if (idx.isValid())
            {
                wdg_listdocstreewiew->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                wdg_listdocstreewiew->setCurrentIndex(idx);
            }
        }
    }
}

void dlg_docsexternes::ZoomDoc()
{
    if (m_mode == Normal)
    {
        m_positionorigin      = pos();
        m_sizeorigin     = size();
        m_mode           = Zoom;
        wdg_scrolltablewidget     ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //WARNING : icon scaled : pxZoomOut 30,30
        graphview_view     ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //WARNING : icon scaled : pxZoomOut 30,30

        if (m_hdeltaframe == 0)   m_hdeltaframe = frameGeometry().height() - height();
        if (m_wdeltaframe == 0)   m_wdeltaframe = frameGeometry().width()  - width();
        if (m_hdelta == 0)
        {
            if (m_typedoc == PDF)    m_hdelta  = height() - wdg_scrolltablewidget->height();
            else                    m_hdelta  = height() - graphview_view->height();
        }
        if (m_wdelta == 0)
        {
            if (m_typedoc == PDF)    m_wdelta  = width() - wdg_scrolltablewidget->width();
            else                    m_wdelta  = width() - graphview_view->width();
        }

        // les dimensions maxi de la zone de visu
        const double maxwscroll  = QGuiApplication::screens().first()->geometry().width()*2/3    - m_wdelta - m_wdeltaframe;
        const double maxhscroll  = QGuiApplication::screens().first()->geometry().height()       - m_hdelta - m_hdeltaframe;
        // les dimensions calculées de la zone de visu
        int wfinal(0), hfinal(0);

        double proportion = maxwscroll/maxhscroll;
        if (m_idealproportion > proportion)
        {   wfinal  = int(maxwscroll);   hfinal  = int(wfinal / m_idealproportion); }
        else
        {   hfinal  = int(maxhscroll);   wfinal  = int(hfinal * m_idealproportion); }
        int w = wfinal + m_wdelta;
        int h = hfinal + m_hdelta;
        resize(w, h);

        if (m_typedoc == PDF)
        {
            for (int i=0; i < wdg_scrolltablewidget->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_scrolltablewidget->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(m_listpixmp.at(i).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    wdg_scrolltablewidget->setRowHeight(i,hfinal-2);
                    wdg_scrolltablewidget->setColumnWidth(0,wfinal-2);
                    if (i==0) wdg_scrolltablewidget->resize(wfinal, hfinal);
                }
            }
        }
        else if (m_typedoc == JPG)
        {
            obj_graphicscene->clear();
            QPixmap pix = m_listpixmp.at(0).scaled(wfinal-2, hfinal-2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            obj_graphicscene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            obj_graphicscene->setSceneRect(1,1,x-1,y-1);
        }
        else if (m_typedoc == VIDEO)
        {
            if (obj_graphicscene->items().size()>0)
            {
                if (dynamic_cast<QGraphicsVideoItem*>(obj_graphicscene->items().at(0)) != Q_NULLPTR)
                    medobj_videoitem   ->setSize(QSize(wfinal-2,hfinal-2));
                int x = int(medobj_videoitem->size().width());
                int y = int(medobj_videoitem->size().height());
                obj_graphicscene->setSceneRect(1,1,x-1,y-1);
            }
        }
        move (QGuiApplication::screens().first()->geometry().width() - w, 0);
    }
    else if (m_mode == Zoom)
    {
        wdg_scrolltablewidget     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
        graphview_view     ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
        move(m_positionorigin);
        resize(m_sizeorigin);
        if (m_typedoc == PDF)
        {
            for (int i=0; i < wdg_scrolltablewidget->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_scrolltablewidget->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    QPixmap pix = m_listpixmp.at(i).scaled(m_sizeorigin.width() - m_wdelta - 2, m_sizeorigin.height() - m_hdelta - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    lbl->setPixmap(pix);
                    wdg_scrolltablewidget->setRowHeight(i,pix.height());
                    wdg_scrolltablewidget->setColumnWidth(0,pix.width());
                    wdg_scrolltablewidget->resize(m_sizeorigin.width()-m_wdelta, m_sizeorigin.height()-m_hdelta);
                }
            }
        }
        else if (m_typedoc == JPG)
        {
            obj_graphicscene->clear();
            QPixmap pix = m_listpixmp.at(0).scaled(m_sizeorigin.width() - m_wdelta - 2, m_sizeorigin.height() - m_hdelta - 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            obj_graphicscene->addPixmap(pix);
            int x = pix.size().width();
            int y = pix.size().height();
            obj_graphicscene->setSceneRect(1,1,x-1,y-1);
        }
        else if (m_typedoc == VIDEO)
        {
            if (obj_graphicscene->items().size()>0)
            {
                if (dynamic_cast<QGraphicsVideoItem*>(obj_graphicscene->items().at(0)) != Q_NULLPTR)
                    medobj_videoitem   ->setSize(QSize(m_sizeorigin.width() - m_wdelta - 2, m_sizeorigin.height() - m_hdelta - 2));
                int x = int(medobj_videoitem->size().width());
                int y = int(medobj_videoitem->size().height());
                obj_graphicscene->setSceneRect(1,1,x-1,y-1);
            }
        }
        m_mode           = Normal;
    }
    setEnregPosition(m_mode == Normal);
    wdg_inflabel    ->move(10, (m_typedoc == PDF? wdg_scrolltablewidget->height() : graphview_view->height())-40);
}

bool dlg_docsexternes::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==wdg_scrolltablewidget)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            for (int i=0; i < wdg_scrolltablewidget->rowCount(); i++)
            {
                UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_scrolltablewidget->cellWidget(i,0));
                if (lbl != Q_NULLPTR)
                {
                    lbl->setPixmap(m_listpixmp.at(i).scaled(wdg_scrolltablewidget->width(), wdg_scrolltablewidget->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    wdg_scrolltablewidget->setRowHeight(i,lbl->pixmap()->height());
                    wdg_scrolltablewidget->setColumnWidth(i,lbl->pixmap()->width());
                }
            }
            wdg_inflabel    ->move(10,wdg_scrolltablewidget->viewport()->height()-40);
        }
    }
    if (obj == graphview_view)
    {
        QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
        if (rszevent != Q_NULLPTR)
        {
            if (m_typedoc == JPG)
            {
                if (obj_graphicscene->items().size()>0)
                {
                    obj_graphicscene->clear();
                    QPixmap pix = m_listpixmp.at(0).scaled(graphview_view->width(), graphview_view->height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    obj_graphicscene->addPixmap(pix);
                    int x = pix.size().width();
                    int y = pix.size().height();
                    obj_graphicscene->setSceneRect(1,1,x-1,y-1);
                 }
            }
            else if (m_typedoc == VIDEO)
            {
                if (obj_graphicscene->items().size()>0)
                {
                    if (dynamic_cast<QGraphicsVideoItem*>(obj_graphicscene->items().at(0)) != Q_NULLPTR)
                        medobj_videoitem   ->setSize(QSize(graphview_view->width(),graphview_view->height()));
                    int x = int(medobj_videoitem->size().width());
                    int y = int(medobj_videoitem->size().height());
                    obj_graphicscene->setSceneRect(1,1,x-1,y-1);
                }
            }
            wdg_inflabel    ->move(10,graphview_view->height()-40);
        }
        if (event->type() == QEvent::MouseButtonPress)
        {
            //static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
            //qDebug() << QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(event->type());
            ZoomDoc();
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
    if (wdg_alldocsupcheckbox->isChecked())
        m_modefiltre = FiltreSans;
    else if (wdg_onlyimportantsdocsupcheckbox->isChecked())
        m_modefiltre = ImportantFiltre;
    else
        m_modefiltre = NormalFiltre;

    if (wdg_listdocstreewiew != Q_NULLPTR)
        wdg_listdocstreewiew->disconnect();
    if (wdg_listdocstreewiew->selectionModel() != Q_NULLPTR)
        wdg_listdocstreewiew->selectionModel()->disconnect();
    QString             idimpraretrouver = "";
    m_model = dynamic_cast<QStandardItemModel*>(wdg_listdocstreewiew->model());
    if (m_model)
    {
        if (m_model->rowCount()>0)
            if (wdg_listdocstreewiew->selectionModel()->selectedIndexes().size()>0)
            {
                QModelIndex actifidx = wdg_listdocstreewiew->selectionModel()->selectedIndexes().at(0);
                if (!m_model->itemFromIndex(actifidx)->hasChildren())
                    idimpraretrouver = m_model->itemFromIndex(actifidx)->data().toMap().value("id").toString();
            }
    }

    if (m_model == Q_NULLPTR)
        delete m_model;
    m_model = new QStandardItemModel(this);
    if (m_tripardatemodel == Q_NULLPTR)
        delete m_tripardatemodel;
    m_tripardatemodel = new QStandardItemModel(this);
    if (m_tripartypemodel == Q_NULLPTR)
        delete m_tripartypemodel;
    m_tripartypemodel = new QStandardItemModel(this);

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
    QStandardItem * rootNodeDate = m_tripardatemodel->invisibleRootItem();
    QStandardItem * rootNodeType = m_tripartypemodel->invisibleRootItem();

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

    foreach (DocExterne *doc, *m_docsexternes->docsexternes())
    {
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
    wdg_onlyimportantsdocsupcheckbox->setEnabled(listdatesimportants.size() > 0);
    if (m_modefiltre == ImportantFiltre && listdatesimportants.size() == 0)
    {
        graphview_view->setVisible(false);
        wdg_scrolltablewidget->setVisible(false);
    }

    QList<QDate> listdates;
    QStringList listtypes;
    switch (m_modefiltre) {
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

    foreach (DocExterne *doc, *m_docsexternes->docsexternes())
    {
        QString date = doc->date().toString(tr("dd-MM-yyyy"));
        QString a = doc->typedoc();
        pitemdate           = new QStandardItem(CalcTitre(doc));
        pitemtype           = new QStandardItem(CalcTitre(doc));
        pitemtridated       = new QStandardItem(doc->date().toString("yyyyMMddHHmmss"));
        pitemtridatet       = new QStandardItem(doc->date().toString("yyyyMMddHHmmss"));
        QMap<QString, QVariant> data;
        data                .insert("id", QString::number(doc->id()));
        QFont fontitem      = m_font;
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
        QList<QStandardItem *> listitemsdate = m_tripardatemodel->findItems(date);
        if (listitemsdate.size()>0)
        {
            switch (m_modefiltre) {
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
        QList<QStandardItem *> listitemstype = m_tripartypemodel->findItems(doc->typedoc());
        if (listitemstype.size()>0)
        {
            switch (m_modefiltre) {
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

    if (m_modetri == parDate)
        m_model = m_tripardatemodel;
    else
        m_model = m_tripartypemodel;
    QItemSelectionModel *m = wdg_listdocstreewiew->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    wdg_listdocstreewiew->setModel(m_model);
    delete m;

    int nrows = m_model->item(m_model->rowCount()-1)->rowCount()-1;                 // le nombre de child du dernier item
    QStandardItem *item =  m_model->item(m_model->rowCount()-1)->child(nrows,0);    // le tout dernier item
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
        QModelIndex indx = getIndexFromId(m_model, idimpraretrouver.toInt());
        if (indx.isValid())
            idx = indx;
    }
    wdg_listdocstreewiew->setSelectionModel(new QItemSelectionModel(m_model));
    wdg_listdocstreewiew->expandAll();
    wdg_listdocstreewiew->scrollTo(idx, QAbstractItemView::PositionAtCenter);
    wdg_listdocstreewiew->setCurrentIndex(idx);
    AfficheDoc(idx);
    connect(wdg_listdocstreewiew->selectionModel(), &QItemSelectionModel::currentChanged,   this,   [=] {AfficheDoc(wdg_listdocstreewiew->selectionModel()->currentIndex());});
    connect(wdg_listdocstreewiew,                   &QTreeView::customContextMenuRequested, this,   [=] {
        QModelIndex idx = wdg_listdocstreewiew->indexAt(wdg_listdocstreewiew->mapFromGlobal(cursor().pos()));
        DocExterne *docmt = getDocumentFromIndex(idx);
        if (docmt != Q_NULLPTR)
            AfficheCustomMenu(docmt);
    });
}

