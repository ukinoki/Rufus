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
    UpDialog(Nom_fiche_DocsExternes, parent)
{
    m_docsexternes  = Docs;

    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    QList<QScreen*> listscreens = QGuiApplication::screens();
    if (listscreens.size()>0)
        setMaximumHeight(listscreens.first()->geometry().height());

    setWindowTitle(tr("Documents de ") + m_docsexternes->patient()->prenom() + " " + m_docsexternes->patient()->nom());

    QFont font  = qApp->font();
    font        .setPointSize(font.pointSize()+2);
    int d=0;
#ifdef QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE
    d=2;
#endif
    m_font              .setPointSize(m_font.pointSize()-d);

    wdg_listdocstreewiew    ->setFixedWidth(m_treeviewwidth);
    wdg_listdocstreewiew    ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_listdocstreewiew    ->setFont(m_font);
    wdg_listdocstreewiew    ->setEditTriggers(QAbstractItemView::DoubleClicked);
    wdg_listdocstreewiew    ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_listdocstreewiew    ->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_listdocstreewiew    ->setAnimated(true);
    wdg_listdocstreewiew    ->setIndentation(6);
    wdg_listdocstreewiew    ->header()->setVisible(false);

    mainscroll              ->setWidgetResizable(true);
    mainscroll              ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainscroll              ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mainscroll              ->setMouseTracking(true);

    wdg_inflabel        = new QLabel(mainscroll);
    wdg_inflabel        ->setFont(m_font);

    m_lay               ->addWidget(wdg_listdocstreewiew,2);
    m_lay               ->addWidget(mainscroll,10);
    m_lay               ->setSpacing(m_spacing);
    m_lay               ->setContentsMargins(m_margins);
    dlglayout()         ->insertLayout(0,m_lay);

    UpSmallButton *ViewerButton = new UpSmallButton();
    ViewerButton                ->setIcon(Icons::icViewer());
    ViewerButton                ->setImmediateToolTip(tr("Afficher l'imagerie"));
    AjouteWidgetLayButtons(ViewerButton, false);

    AjouteWidgetLayButtons(wdg_playctrl);

    wdg_alldocsupcheckbox               = new UpCheckBox(tr("Tous"));
    wdg_onlyimportantsdocsupcheckbox    = new UpCheckBox(tr("Importants"));
    wdg_alldocsupcheckbox               ->setImmediateToolTip(tr("Afficher tous les documents\ny compris les documents d'importance minime"));
    wdg_onlyimportantsdocsupcheckbox    ->setImmediateToolTip(tr("N'afficher que les documents marqués importants"));
    AjouteWidgetLayButtons(wdg_alldocsupcheckbox, false);
    AjouteWidgetLayButtons(wdg_onlyimportantsdocsupcheckbox, false);
    wdg_alldocsupcheckbox               ->setChecked(true);

    wdg_updatetypebox       = new UpComboBox(this);
    wdg_updatetypebox       ->insertItems(0,QStringList() << tr("Date") << tr("Tyoe"));
    wdg_updatetypebox       ->setFixedWidth(80);
    AjouteWidgetLayButtons(wdg_updatetypebox, false);

    AjouteLayButtons(UpDialog::ButtonRecord | UpDialog::ButtonSuppr | UpDialog::ButtonPrint);

    m_docsimagery = new DocsExternes();
    for (auto it = m_docsexternes->docsexternes()->begin(); it  != m_docsexternes->docsexternes()->end(); ++it)
    {
        DocExterne *doc = qobject_cast<DocExterne*>(it.value());
        if (doc != Q_NULLPTR)
            if (doc->isMedicalImagery() || doc->isVideo())
                m_docsimagery->docsexternes()->insert(it.key(),it.value());
    }
    ViewerButton->setVisible(m_docsimagery->docsexternes()->size()>0);
    //setStageCount(1);

    connect (wdg_updatetypebox,                 &UpComboBox::currentTextChanged,             this,   [=] (QString text) {BasculeTriListe(text == tr("Date")?parDate:parType);});
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
    connect (ViewerButton,                      &QPushButton::clicked,          this,   &dlg_docsexternes::OpenMultiImageViewer);

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
    installEventFilter(this);
}

dlg_docsexternes::~dlg_docsexternes()
{
    delete m_tripardatemodel;
    delete m_tripartypemodel;
    delete m_docsimagery;
}

void dlg_docsexternes::AfficheCustomMenu(DocExterne *docmt)
{
    QModelIndex idx = getIndexFromId(m_model, docmt->id());
    QMenu *menu = new QMenu(this);
    QAction *paction_Modifier       = new QAction(Icons::icEditer(), tr("Le titre"));
    QAction *paction_ModifierDate   = new QAction(Icons::icDate(), tr("La date"));
    QAction *paction_Zoom           = new QAction(m_mode == Zoom? Icons::pxZoomOut() : Icons::pxZoomIn(), m_mode == Zoom?  tr("Réduire") : tr("Agrandir"));
    if (currentuser()->isSoignant())
    {
        if  (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU || docmt->isVideo())
        {
            QAction *paction_Viewer         = new QAction(Icons::icViewer(), tr("Ouvrir dans le visualisateur"));
            menu->addAction(paction_Viewer);
            connect (paction_Viewer,    &QAction::triggered,    this,  [=] {OpenMultiImageViewer(docmt->id());});
        }
        QMenu *menuImportance  = menu->addMenu(tr("Importance"));
        QAction *paction_ImportantMin   = new QAction(tr("Faible"));
        QAction *paction_ImportantNorm  = new QAction(tr("Normale"));
        QAction *paction_ImportantMax   = new QAction(tr("Forte"));
        int imptce      = docmt->importance();
        QIcon icon      = Icons::icBlackCheck();
        if (imptce == 0)
            paction_ImportantMin->setIcon(icon);
        else if (imptce == 1)
            paction_ImportantNorm->setIcon(icon);
        else if (imptce == 2)
            paction_ImportantMax->setIcon(icon);
        menuImportance  ->setIcon(Icons::icMedoc());
        menuImportance  ->addAction(paction_ImportantMin);
        menuImportance  ->addAction(paction_ImportantNorm);
        menuImportance  ->addAction(paction_ImportantMax);
        connect (paction_ImportantMin,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Min);});
        connect (paction_ImportantNorm, &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Norm);});
        connect (paction_ImportantMax,  &QAction::triggered,    this,  [=] {CorrigeImportance(docmt, Max);});
    }
    QMenu *menuModif    = menu->addMenu(tr("Modifier"));
    menuModif           ->setIcon(Icons::icEditer());
    menuModif           ->addAction(paction_Modifier);
    menu                ->addAction(paction_Zoom);
    if (docmt->format() == VIDEO || docmt->format() == DOCUMENTRECU)
        menuModif->addAction(paction_ModifierDate);
    connect (paction_Modifier,      &QAction::triggered,    this,  [=] {ModifierItem(idx);});
    connect (paction_ModifierDate,  &QAction::triggered,    this,  [=] {ModifierDate(idx);});
    connect (paction_Zoom,          &QAction::triggered,    this,  [=] {ZoomDoc();});

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
        if (currentuser()->isSoignant()
            && (docmt->format() != IMAGERIE && docmt->format() != DOCUMENTRECU))
        {   // si le document a été émis aujourd'hui, on propose de le modifier - dans ce cas, on va créer une copie qu'on va modifier et on détruira le document d'origine à la fin
            if (m_currentdate == docmt->datetimeimpression().date())
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
    if (currentuser()->isSoignant())
        menu->addAction(paction_Poubelle);

    menu->exec(cursor().pos());
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

    int imp = 0;
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
    DocExterne *docmt = getDocumentFromIndex(idx);              //! load all details from document
    m_currentdocument = docmt;
    if (docmt == Q_NULLPTR)
    {
        PrintButton->setEnabled(false);
        SupprButton->setEnabled(false);
        return;
    }
    PrintButton         ->setVisible(true);
    PrintButton         ->setEnabled(true);
    SupprButton         ->setEnabled(true);
    bool j              =((docmt->format() == VIDEO || docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU)
                                   && DataBase::I()->ModeAccesDataBase() != Utils::Distant);
    RecordButton        ->setVisible(j);
    RecordButton        ->disconnect();
    double x;
    double y;
    wdg_inflabel        ->setVisible(!docmt->isVideo());
    wdg_playctrl        ->setVisible(docmt->isVideo());

    if (docmt->isVideo())  // le document est une video -> n'est pas stocké dans la base mais dans un fichier sur le disque
    {
        wdg_video = new QVideoWidget;

        mainscroll->setWidget(wdg_video);
        if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
        {
            UpMessageBox::Watch(this, tr("Video non accessible en accès distant"));
            return;
        }
        QString filename = db->dirimagerie() + NOM_DIR_VIDEOS "/" + docmt->lienversfichier();
        QFile   qFile(filename);
        if (!qFile.open(QIODevice::ReadOnly))
        {
            QString msg = tr("Erreur d'accès au fichier:") + " " + filename;
            UpMessageBox::Watch(this,msg);
            return;
        }
        medplay_player  ->setSource(QUrl::fromLocalFile(filename));
        medplay_player  ->setVideoOutput(wdg_video);
        wdg_playctrl    ->setPlayer(medplay_player);
        QSize size      = wdg_video->videoSink()->videoSize();
        int w           = sizeforunit().width();
        double nx       = size.width();
        double ny       = size.height();
        int h           = int(w*ny/nx);
        m_vidorimgratio = nx/ny;
        wdg_video       ->setFixedSize(w,h);
        wdg_video       ->installEventFilter(this);
        int hf          = std::min(h, wdg_listdocstreewiew->height());
        mainscroll      ->resize(w,hf);

        wdg_video       ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
        wdg_video       ->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(wdg_video,      &QWidget::customContextMenuRequested,   this,   [=] {AfficheCustomMenu(docmt);});
        connect (RecordButton,  &QPushButton::clicked,                  this,   &dlg_docsexternes::EnregistreVideo);
        wdg_playctrl    ->move(20, 20);//sizeforunit().height()-40);
        PrintButton     ->setVisible(false);
        wdg_playctrl    ->startplay();
        wdg_inflabel    ->setText("");
    }
    else                                    // le document est une image ou un document écrit (ordonnance, certificat...)
    {
        proc            ->CalcImageDocument(docmt);
        QString sstitre = "";
        if  (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU)
            sstitre = "<font color='magenta'>" + docmt->datetimeimpression().toString(tr("d-M-yyyy")) + " - " + docmt->soustypedoc() + "</font>";
        wdg_inflabel    ->setText(sstitre);
        connect (RecordButton,  &QPushButton::clicked,   this,  [=] {EnregistreImage(docmt);});
        if (docmt->isJPG())     // le document est un JPG
        {
            QImage image;
            if (!image.loadFromData(docmt->imageblob()))
            {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                return;
            }
            wdg_jpglbl          = new UpLabel();
            wdg_jpglbl          ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
            mainscroll          ->setWidget(wdg_jpglbl);
            QPixmap pix         = QPixmap::fromImage(image).scaled(sizeforunit(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
            x                   = pix.width();
            y                   = pix.height();
            m_vidorimgratio     = x/y;
            wdg_jpglbl          ->setPixmap(pix);
            wdg_jpglbl          ->setImage(image);
            wdg_jpglbl          ->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(wdg_jpglbl,    &UpLabel::clicked,                      this, [=] {(docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU) && m_mode== Normal?
                                                                                        OpenMultiImageViewer(docmt->id()) :
                                                                                        ZoomDoc();});
            connect(wdg_jpglbl,    &UpLabel::customContextMenuRequested,   this, [=] {AfficheCustomMenu(docmt);});
            int hf              = std::min(int(y), wdg_listdocstreewiew->height());
            mainscroll          ->resize(x,hf);
        }
        else if (docmt->isPDF())     // le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage)
        {
            QList<QImage> listimg;
            listimg         = docmt->pagelist();
            if (listimg.size() == 0)
            {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                return;
            }

            wdg_pdftbl      = new UpTableWidget();
            wdg_pdftbl->horizontalHeader() ->setVisible(false);
            wdg_pdftbl->verticalHeader()   ->setVisible(false);
            wdg_pdftbl      ->setFocusPolicy(Qt::NoFocus);
            wdg_pdftbl      ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); //! sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
            wdg_pdftbl      ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            wdg_pdftbl      ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            wdg_pdftbl      ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30)));      // WARNING : icon scaled : pxZoomIn 30,30
            wdg_pdftbl      ->setColumnCount(1);
            wdg_pdftbl      ->setSelectionMode(QAbstractItemView::NoSelection);
            wdg_pdftbl      ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30

            mainscroll      ->setWidget(wdg_pdftbl);
            if (listimg.size())
            {
                wdg_pdftbl                  ->setListimg(listimg);
                QSize szfinal               = QSize();
                QList<UpLabel *> pdflabels  = wdg_pdftbl->calcSizeForDisplay(sizeforunit(), szfinal);
                double w                    = szfinal.width();
                double h                    = szfinal.height();
                m_vidorimgratio             = w/h;
                for (int i=0; i<pdflabels.size(); i++)
                {
                    connect(pdflabels.at(i),    &UpLabel::clicked,                      this, [=] {(docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU) && m_mode== Normal?
                                                                                                    OpenMultiImageViewer(docmt->id()) :
                                                                                                    ZoomDoc();});
                    connect(pdflabels.at(i),    &UpLabel::customContextMenuRequested,   this, [=] {AfficheCustomMenu(docmt);});
                }
                wdg_pdftbl  ->setFixedSize(szfinal);
                int hf = std::min(szfinal.height(), wdg_listdocstreewiew->height());
                mainscroll  ->resize(szfinal.width(),hf);
            }
        }
        else return;
    }
    if (m_mode == Zoom)
    {
        ZoomDoc(false);
    }
}

void dlg_docsexternes::BasculeTriListe(ModeTri mode)
{
    QString             idimpraretrouver = "";
    wdg_listdocstreewiew    ->disconnect();
    if (wdg_listdocstreewiew->selectionModel()->selectedIndexes().size()>0)
    {
        QModelIndex actifidx = wdg_listdocstreewiew->selectionModel()->selectedIndexes().at(0);
        if (!m_model->itemFromIndex(actifidx)->hasChildren())
            idimpraretrouver = m_model->itemFromIndex(actifidx)->data().toMap().value("id").toString();
    }
    m_modetri = mode;
    switch (m_modetri) {
    case parDate:
        m_model  = m_tripardatemodel;
        break;
    case parType:
        m_model  = m_tripartypemodel;
        break;
    }

    QItemSelectionModel *m = wdg_listdocstreewiew->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    wdg_listdocstreewiew->setModel(m_model);
    delete m;

    if (m_model)
        if (m_model->rowCount()>0)
        {
            int nrows = 0;
            int n = m_model->rowCount()-1;
            if (m_model->item(n) != Q_NULLPTR)
                nrows = m_model->item(n)->rowCount()-1;                 // le nombre de child du dernier item date
            QStandardItem *item =  m_model->item(n)->child(nrows,0);    // le tout dernier item
            QModelIndex idx = item->index();                            // l'index de ce dernier item
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
        }
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
    QString filename = db->dirimagerie() + NOM_DIR_IMAGES + docmt->lienversfichier();
    QFile img(filename);
    if (!img.open(QIODevice::ReadOnly))
    {
        UpMessageBox::Watch(this, tr("Erreur d'accès au fichier:"), filename);
        return;
    }
    QFileDialog dialog(this, tr("Enregistrer un fichier"), QDir::homePath());
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec() == QDialog::Accepted)
    {
        QDir dockdir = dialog.directory();
        img.copy(dockdir.path() + "/" + m_docsexternes->patient()->prenom() + " " + m_docsexternes->patient()->nom() + " "+ docmt->soustypedoc() + "." + QFileInfo(img).suffix().toLower());
    }
}

void dlg_docsexternes::EnregistreVideo()
{
    QString filename = medplay_player->source().path();
    QFileDialog dialog(this, tr("Enregistrer un fichier"), QDir::homePath());
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::List);
    if (dialog.exec() == QDialog::Accepted)
    {
        QDir dockdir = dialog.directory();
        QFile(filename).copy(dockdir.path() + "/" + medplay_player->source().fileName());
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

        UpMessageBox msgbox(this);
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
            if (m_currentdate == docmt->datetimeimpression().date())           // si le document a été émis aujourd'hui, on propose de le modifier
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
    QString     textcorps, textentete, textpied, txt;
    QTextEdit   *Etat_textEdit  = new QTextEdit;
    bool        aa;
    bool        ALD             = (docmt->isALD());
    bool        Prescription    = docmt->isprescription();

    if (currentuser() == Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver les données de l'en-tête"), tr("Annulation de l'impression"));
        delete Etat_textEdit;
        return false;
    }

    //création de l'entête
    QMap<QString,QString> EnteteMap = proc->CalcEnteteImpression(m_currentdate, currentuser(), Prescription);
    textentete = (ALD? EnteteMap.value(ALDHeader) : EnteteMap.value(NORMHeader));
    if (textentete == "")
    {
        delete Etat_textEdit;
        return false;
    }
    textentete.replace("{{TITRE1}}"        , "");
    textentete.replace("{{TITRE}}"         , "");
    textentete.replace("{{DDN}}"           , "");
    textentete.replace("{{PRENOM PATIENT}}", (Prescription? m_docsexternes->patient()->prenom()        : ""));
    textentete.replace("{{NOM PATIENT}}"   , (Prescription? m_docsexternes->patient()->nom().toUpper() : ""));

    //création du pied
    textpied = proc->CalcPiedImpression(currentuser(), docmt->format() == PRESCRIPTIONLUNETTES, ALD);

    // creation du corps de l'ordonnance
    QString txtautiliser    = (docmt->textorigine() == ""?              docmt->textcorps()              : docmt->textorigine());
    txt                     = (modifiable?                              proc->Edit(txtautiliser)        : txtautiliser);
    textcorps               = (docmt->typedoc() == PRESCRIPTION?        proc->CalcCorpsImpression(txt,ALD)  : proc->CalcCorpsImpression(txt));
    if (ALD)
    {
        textcorps.replace("{{PRENOM PATIENT}}", m_docsexternes->patient()->prenom());
        textcorps.replace("{{NOM PATIENT}}"   , m_docsexternes->patient()->nom().toUpper());
        textcorps.replace("{{DATE}}", tr("le ") + QLocale::system().toString(QDate::currentDate(),tr("d MMMM yyyy")));
    }
    Etat_textEdit           ->setHtml(textcorps);
    if (Etat_textEdit->toPlainText() == "" || txt == "")
    {
        if (Etat_textEdit->toPlainText() == "")
            UpMessageBox::Watch(this,tr("Rien à imprimer"));
        delete Etat_textEdit;
        return false;
    }
    delete Etat_textEdit;
    int TailleEnTete        = (ALD?                                     proc->TailleEnTeteALD()         : proc->TailleEnTete());
    bool AvecDupli          = (proc->settings()->value(Imprimante_OrdoAvecDupli).toString() == "YES"
                               && docmt->typedoc() == PRESCRIPTION);

    QMap<QString,QString> mapbarcodes = QMap<QString,QString>();
    User *usr = Q_NULLPTR;
    if (Prescription)
        usr = Datas::I()->users->getById(docmt->iduser());
    if (usr != Q_NULLPTR)
        mapbarcodes = usr->mapBarCodes();

    aa = proc->Imprime_Etat(this, textcorps, textentete, textpied,
                            proc->TaillePieddePage(), TailleEnTete, proc->TailleTopMarge(), mapbarcodes,
                            AvecDupli);

    // stockage du document dans la base de donnees - table impressions
    if (aa)
    {
        Utils::nettoieHTML(textcorps, 9);
        QByteArray ba = QByteArray();
        if (usr != Q_NULLPTR)
            ba = proc->Cree_pdfByteArray(textcorps, textentete, textpied, usr);

        QHash<QString, QVariant> listbinds;
        listbinds[CP_IDUSER_DOCSEXTERNES]        = currentuser()->id();
        listbinds[CP_IDPAT_DOCSEXTERNES]         = docmt->idpatient();
        listbinds[CP_TYPEDOC_DOCSEXTERNES]       = docmt->typedoc();
        listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES]   = docmt->soustypedoc();
        listbinds[CP_TITRE_DOCSEXTERNES]         = docmt->titre();
        listbinds[CP_TEXTENTETE_DOCSEXTERNES]    = textentete;
        listbinds[CP_TEXTCORPS_DOCSEXTERNES]     = textcorps;
        listbinds[CP_TEXTORIGINE_DOCSEXTERNES]   = txt;
        listbinds[CP_TEXTPIED_DOCSEXTERNES]      = textpied.replace("{{DUPLI}}","");
        listbinds[CP_DATE_DOCSEXTERNES]          = db->ServerDateTime().toString("yyyy-MM-dd HH:mm:ss");
        listbinds[CP_FORMATDOC_DOCSEXTERNES]     = docmt->format();
        listbinds[CP_IDLIEU_DOCSEXTERNES]        = Datas::I()->sites->idcurrentsite();
        listbinds[CP_ALD_DOCSEXTERNES]           = (ALD? "1" : QVariant(QMetaType::fromType<QString>()));
        listbinds[CP_IDEMETTEUR_DOCSEXTERNES]    = currentuser()->id();
        listbinds[CP_IMPORTANCE_DOCSEXTERNES]    = docmt->importance();
        listbinds[CP_PDFORIGIN_DOCSEXTERNES]     = ba;
        DocExterne * doc = m_docsexternes->CreationDocumentExterne(listbinds);
        if (doc != Q_NULLPTR)
        {
            int idimpr = doc->id();
            delete doc;
            if (detruirealafin)
            {
                if (docmt->idrefraction() > 0)
                    Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(docmt->idrefraction()));
                m_docsexternes->SupprimeDocumentExterne(docmt);
            }
            ActualiseDocsExternes();
            QModelIndex idx = getIndexFromId(m_model, idimpr);
            wdg_listdocstreewiew->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            wdg_listdocstreewiew->setCurrentIndex(idx);
            AfficheDoc(idx);
        }
    }
    return true;
}

void dlg_docsexternes::ReImprimeDoc(DocExterne *docmt)
{
    if (docmt->imageblob() == QByteArray())
        proc->CalcImageDocument(docmt);

    //
    // First, we fill img_list with document pages
    //
    QList<QImage> imglist = QList<QImage>();
    if (docmt->imageformat() == PDF)     // le document est un pdf ou un document texte
        imglist = docmt->pagelist();
    else if (docmt->imageformat() == JPG)     // le document est un jpg
    {
        QPixmap pix;
        pix.loadFromData(docmt->imageblob());
        imglist << pix.toImage();
    }

    proc->Print(imglist);

    if (currentpatient() != Datas::I()->patients->currentpatient())
        close();
}

void dlg_docsexternes::ModifierDate(QModelIndex idx)
{
    DocExterne *docmt = getDocumentFromIndex(idx);
    UpDialog * dlg              = new UpDialog();
    dlg                         ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QDateEdit   *dateedit       = new QDateEdit(dlg);
    UpLabel     *label          = new UpLabel(dlg);
    dlg->dlglayout()->insertWidget(0,dateedit);
    dlg->dlglayout()->insertWidget(0,label);
    dlg->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);

    dlg->setWindowModality(Qt::WindowModal);
    dlg->setFixedSize(200,100);
    dlg->move(QPoint(x()+width()/2,y()+height()/2));
    dlg->setWindowTitle(tr("Modifier la date"));
    dateedit->setDate(docmt->datetimeimpression().date());
    dateedit->setSelectedSection(QDateTimeEdit::DaySection);
    dateedit->setDisplayFormat(tr("dd/MM/yyyy"));

    connect(dlg->OKButton,   &QPushButton::clicked, this,   [=]
    {
        if (dateedit->date().isValid())
        {
            ItemsList::update(docmt, CP_DATE_DOCSEXTERNES, QDate(dateedit->date()));
            RemplirTreeView();
            dlg->accept();
        }
        else
        {
            Utils::playAlarm();
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
    dlg                         ->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    UpLineEdit  *Line           = new UpLineEdit(dlg);
    UpLabel     *label          = new UpLabel(dlg);
    dlg->dlglayout()->insertWidget(0,Line);
    dlg->dlglayout()->insertWidget(0,label);
    dlg->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);

    dlg->setWindowModality(Qt::WindowModal);
    dlg->setFixedSize(270,100);
    dlg->move(QPoint(x()+width()/2,y()+height()/2));
    Line->setText(docmt->soustypedoc());
    Line->selectAll();

    connect(dlg->OKButton,   &QPushButton::clicked, this,  [=]
    {
        if (Line->text()!="")
        {
            ItemsList::update(docmt, CP_SOUSTYPEDOC_DOCSEXTERNES, Line->text());
            QString titre = docmt->titrelong();
            m_model->itemFromIndex(idx)->setText(titre);
            int id = docmt->id();
            m_tripardatemodel->itemFromIndex(getIndexFromId(m_tripardatemodel,id))->setText(titre);
            m_tripartypemodel->itemFromIndex(getIndexFromId(m_tripartypemodel,id))->setText(titre);
            dlg->accept();
        }
        else
        {
            Utils::playAlarm();
            QToolTip::showText(cursor().pos(),tr("Vous devez entrer du texte"));
        }
    });

    label->setText(tr("Entrez le titre du document"));
    Line->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse,this));
    Line->setMaxLength(60);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->exec();
    delete dlg;
}

void dlg_docsexternes::OpenMultiImageViewer(int iddoc)
{
    dlg_imageviewer *viewer = new dlg_imageviewer(m_docsimagery, iddoc, this);
    viewer->exec();
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
        UpMessageBox msgbox(this);
        UpSmallButton OKBouton(tr("Supprimer"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.setText("Euuhh... " + currentuser()->login());
        msgbox.setInformativeText(tr("Etes vous certain de vouloir supprimer le document ") + docmt->titrelong() + "?");
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
            db->StandardSQL("insert into " TBL_DOCSASUPPRIMER " (" CP_FILEPATH_DOCSASUPPR ") VALUES ('" + Utils::correctquoteSQL(filename) + "')");
        }
        QString idaafficher = "";
        if (m_docsexternes->docsexternes()->size() > 1)    // on recherche le document sur qui va être mis la surbrillance après la suppression
        {
            auto itdoc =  m_docsexternes->docsexternes()->find(docmt->id());
            if (itdoc != m_docsexternes->docsexternes()->begin())
                itdoc --;
            else
                itdoc ++;
            idaafficher = QString::number(itdoc.key());
        }
        if (docmt->idrefraction() > 0)
            Datas::I()->refractions->SupprimeRefraction(Datas::I()->refractions->getById(docmt->idrefraction()));
        m_docsexternes->SupprimeDocumentExterne(docmt);
        if(m_docsexternes->docsexternes()->size() > 0)
        {
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
        else
            close();
    }
}

void dlg_docsexternes::ZoomDoc(bool changemode)
{
    if (!changemode)
        m_mode = (m_mode == Zoom? Normal : Zoom);
    if (m_mode == Normal)
    {
        if (changemode)
        {
            m_positionorigin = pos();
            m_sizeorigin     = size();
        }
        m_mode       = Zoom;

        if (m_hdelta == 0)
            m_hdelta = height() - wdg_listdocstreewiew->height();
        if (m_wdelta == 0)
        {
            if (m_currentdocument->isPDF())         m_wdelta  = width() - wdg_pdftbl->width();
            else if (m_currentdocument->isJPG())    m_wdelta  = width() - wdg_jpglbl->width();
            else if (m_currentdocument->isVideo())  m_wdelta  = width() - wdg_video->width();
        }

        //! max dimensions zone visu
        double wscroll  = 0;
        double hscroll  = 0;
        double screenratio = 1;
        QList<QScreen*> listscreens = QGuiApplication::screens();
        if (listscreens.size())
        {
            wscroll  = listscreens.first()->geometry().width();
            hscroll  = listscreens.first()->geometry().height();
            screenratio = wscroll/hscroll;
        }

        /*! if screenration > videoratio  => screenheight is used for max height else screenwidthfor max width */
        int finalh (0), finalw(0);
        if (screenratio > m_vidorimgratio)
        {
            finalh = int(hscroll * 0.9);
            double hd = finalh - m_hdelta;    //! height available for scrollarea - double is used to cast (hd * m_vidorimgratio) to double
            finalw = int(hd * m_vidorimgratio) + m_wdelta;
        }
        else
        {
            finalw = int(wscroll * 0.9);
            double wd = finalw - m_wdelta;     //! width available for scrollarea - double is used to cast (wd / m_vidorimgratio) to double
            finalh = int(wd / m_vidorimgratio) + m_wdelta;
        }
        resize(finalw, finalh);
        QSize szavailable = QSize(finalw - m_wdelta, finalh - m_hdelta);

        if (m_currentdocument->isPDF())
        {
            wdg_pdftbl      ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
            QSize szfinal   = QSize();
            wdg_pdftbl      ->resizetofit(szavailable, szfinal);
            int hf          = std::min(szfinal.height(), wdg_listdocstreewiew->height());
            mainscroll      ->resize(szfinal.width(),hf);
        }
        else if (m_currentdocument->isJPG())
        {
            wdg_jpglbl      ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
            QPixmap pix     = QPixmap::fromImage(wdg_jpglbl->image()).scaled(szavailable,
                                                   Qt::KeepAspectRatioByExpanding,
                                                   Qt::SmoothTransformation);
            wdg_jpglbl      ->setPixmap(pix);
            wdg_jpglbl      ->resize(finalw, finalh);
        }
        else if (m_currentdocument->isVideo())
        {
            wdg_video   ->setCursor(QCursor(Icons::pxZoomOut().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
            wdg_video   ->setFixedSize(finalw - m_wdelta, finalh - m_hdelta);
        }
        if (listscreens.size())
            move (listscreens.first()->geometry().width() - width(), 0);
    }
    else if (m_mode == Zoom)
    {
        move(m_positionorigin);
        resize(m_sizeorigin);
        int worigin = m_sizeorigin.width() - m_wdelta;
        int horigin = m_sizeorigin.height() - m_hdelta;
        if (m_currentdocument->isPDF())
        {
            wdg_pdftbl      ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
            QSize szfinal   = QSize();
            wdg_pdftbl      ->resizetofit(QSize(worigin,horigin), szfinal);
            int hf          = std::min(szfinal.height(), wdg_listdocstreewiew->height());
            mainscroll      ->resize(szfinal.width(),hf);
        }
        else if (m_currentdocument->isJPG())
        {
            wdg_jpglbl  ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
            QPixmap pix = QPixmap::fromImage(wdg_jpglbl->image()).scaled(worigin, horigin, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            wdg_jpglbl  ->setPixmap(pix);
            wdg_jpglbl  ->resize(worigin,horigin);
        }
        else if (m_currentdocument->isVideo())
        {
            wdg_video   ->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30
            wdg_video   ->setFixedSize(worigin,horigin);
        }
        m_mode           = Normal;
    }
    setEnregPosition(m_mode == Normal);
}

bool dlg_docsexternes::eventFilter(QObject *obj, QEvent *event)
{
    QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
    if (rszevent != Q_NULLPTR)
    {
        if (m_currentdocument->isPDF())
        {
            QSize szfinal   = QSize();
            wdg_pdftbl      ->resizetofit(sizeforunit(), szfinal);
            int hf          = std::min(szfinal.height(), wdg_listdocstreewiew->height());
            mainscroll      ->resize(szfinal.width(),hf);
        }
        else if (m_currentdocument->isJPG())
        {
            QImage img  = wdg_jpglbl->image();
            QPixmap pix = QPixmap::fromImage(img).scaled(sizeforunit(), Qt::KeepAspectRatio);
            wdg_jpglbl  ->setPixmap(pix);
            int x       = pix.width();
            int y       = pix.height();
            int hf      = std::min(y, wdg_listdocstreewiew->height());
            mainscroll  ->resize(x,hf);
        }
        else if (m_currentdocument->isVideo())
        {
            QSize size  = wdg_video->videoSink()->videoSize();
            int w       = sizeforunit().width();
            double nx   = size.width();
            double ny   = size.height();
            int h       = int(w*ny/nx);
            wdg_video   ->setFixedSize(w,h);
            int hf      = std::min(h, wdg_listdocstreewiew->height());
            mainscroll  ->resize(w,hf);
        }
        wdg_inflabel    ->setGeometry(20, mainscroll->height()-35, 500, 25);
    }
    if (event)
        if (obj == wdg_video)
            if (event->type() == QEvent::MouseButtonPress)
                if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
                    ZoomDoc();

    return QWidget::eventFilter(obj, event);
}

void dlg_docsexternes::RemplirTreeView()
{
    if (m_docsexternes->docsexternes()->size() == 0)
    {
        reject();
        return;  // si on ne met pas ça, le reject n'est pas effectué.
    }
    if (wdg_alldocsupcheckbox->isChecked())
        m_modefiltre = FiltreSans;
    else if (wdg_onlyimportantsdocsupcheckbox->isChecked())
        m_modefiltre = ImportantFiltre;
    else
        m_modefiltre = NormalFiltre;

    if (!wdg_listdocstreewiew)
        return;
    if (wdg_listdocstreewiew)
        wdg_listdocstreewiew->disconnect();
    if (wdg_listdocstreewiew->selectionModel())
        wdg_listdocstreewiew->selectionModel()->disconnect();
    QString             idimpraretrouver = "";
    m_model = qobject_cast<QStandardItemModel*>(wdg_listdocstreewiew->model());
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

    if (m_tripardatemodel != Q_NULLPTR)
        delete m_tripardatemodel;
    m_tripardatemodel = new QStandardItemModel(this);
    if (m_tripartypemodel != Q_NULLPTR)
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
        if (!dates.contains(doc->datetimeimpression().date()))
            dates << doc->datetimeimpression().date();
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
        wdg_jpglbl->setVisible(false);
        wdg_pdftbl->setVisible(false);
        wdg_video->setVisible(false);
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
        QString datestring = listdates.at(i).toString(tr("dd-MM-yyyy"));
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
        QString date = doc->datetimeimpression().toString(tr("dd-MM-yyyy"));
        //qDebug() << date << doc->titre();
        pitemdate           = new QStandardItem(doc->titrelong());
        pitemtype           = new QStandardItem(doc->titrelong());
        pitemtridated       = new QStandardItem(doc->datetimeimpression().toString("yyyyMMddHHmmss"));
        pitemtridatet       = new QStandardItem(doc->datetimeimpression().toString("yyyyMMddHHmmss"));
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
    connect(wdg_listdocstreewiew->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            [=] {
                    AfficheDoc(wdg_listdocstreewiew->selectionModel()->currentIndex());
                });
    connect(wdg_listdocstreewiew,
            &QTreeView::customContextMenuRequested,
            this,
            [=] {
                    QModelIndex idx = wdg_listdocstreewiew->indexAt(wdg_listdocstreewiew->mapFromGlobal(cursor().pos()));
                    DocExterne *docmt = getDocumentFromIndex(idx);
                    if (docmt != Q_NULLPTR)
                        AfficheCustomMenu(docmt);
                });
}
