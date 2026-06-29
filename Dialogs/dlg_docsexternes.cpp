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
#include "dlg_multiimageviewer.h"
#include "gbl_datas.h"
#include "icons.h"
#include <QMainWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QPainter>

dlg_docsexternes::dlg_docsexternes(DocsExternes *Docs, bool UtiliseTCP, QWidget *parent) : dlg_singleimageviewer(parent)
{
    m_docsexternes  = Docs;

    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Documents de ") + m_docsexternes->patient()->prenom() + " " + m_docsexternes->patient()->nom());

    mainlayout()            ->setSpacing(10);

    wdg_listdocstreewiew    ->setFixedWidth(m_treeviewwidth);
    wdg_listdocstreewiew    ->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wdg_listdocstreewiew    ->setFont(m_font);
    wdg_listdocstreewiew    ->setEditTriggers(QAbstractItemView::DoubleClicked);
    wdg_listdocstreewiew    ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_listdocstreewiew    ->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_listdocstreewiew    ->setAnimated(true);
    wdg_listdocstreewiew    ->setIndentation(6);
    wdg_listdocstreewiew    ->header()->setVisible(false);

    mainlayout()            ->insertWidget(0,wdg_listdocstreewiew);
    setCorrectionwidget(wdg_listdocstreewiew);

    int s = 25;
    QSize szicon = QSize(s,s);

    m_ZoomInButton          ->setIcon(Icons::icZoomIn());
    m_ZoomInButton          ->setIconSize(szicon);
    m_ZoomInButton          ->setImmediateToolTip(tr("Agrandir"));
    buttonslayout()         ->insertWidget(buttonslayout()->count() - nbbuttons(), m_ZoomInButton);

    m_ZoomOutButton         ->setIcon(Icons::icZoomOut());
    m_ZoomOutButton         ->setIconSize(szicon);
    m_ZoomOutButton         ->setImmediateToolTip(tr("Réduire"));
    buttonslayout()         ->insertWidget(buttonslayout()->count() - nbbuttons(), m_ZoomOutButton);
    m_ZoomOutButton         ->setVisible(false);

    UpSmallButton *ViewerButton = new UpSmallButton();
    ViewerButton                ->setIcon(Icons::icViewer());
    ViewerButton                ->setIconSize(szicon);
    ViewerButton                ->setImmediateToolTip(tr("Afficher l'imagerie"));
    buttonslayout()             ->insertWidget(buttonslayout()->count() - nbbuttons(), ViewerButton);

    wdg_alldocsupcheckbox               = new UpCheckBox(tr("Tous"));
    wdg_onlyimportantsdocsupcheckbox    = new UpCheckBox(tr("Importants"));
    wdg_alldocsupcheckbox               ->setImmediateToolTip(tr("Afficher tous les documents\ny compris les documents d'importance minime"));
    wdg_onlyimportantsdocsupcheckbox    ->setImmediateToolTip(tr("N'afficher que les documents marqués importants"));
    AjouteWidgetLayButtons(wdg_alldocsupcheckbox, false);
    AjouteWidgetLayButtons(wdg_onlyimportantsdocsupcheckbox, false);
    wdg_alldocsupcheckbox               ->setChecked(true);

    wdg_updatetypebox       = new UpComboBox(this);
    wdg_updatetypebox       ->insertItems(0,QStringList() << tr("Date") << tr("Type"));
    wdg_updatetypebox       ->setFixedWidth(80);
    AjouteWidgetLayButtons(wdg_updatetypebox, false);

    AjouteLayButtons(UpDialog::ButtonRecord | UpDialog::ButtonSuppr | UpDialog::ButtonPrint);

    for (auto it = m_docsexternes->docsexternes()->begin(); it  != m_docsexternes->docsexternes()->end(); ++it)
    {
        DocExterne *doc = qobject_cast<DocExterne*>(it.value());
        if (doc != Q_NULLPTR)
            if (doc->isMedicalImagery() || (doc->isVideo() && DataBase::I()->ModeAccesDataBase() != Utils::Distant))
                m_listiddocsimagery << it.key();
    }
    ViewerButton->setVisible(m_listiddocsimagery.size()>0);
    setStageCount(1);
    connect (wdg_updatetypebox,                 &UpComboBox::currentTextChanged,             this,   [=] (QString text) {BasculeTriListe(text == tr("Date")?parDate:parType);});
    connect (SupprButton,                       &QPushButton::clicked,          this,   [=] {SupprimeDoc();});
    connect (wdg_alldocsupcheckbox,             &QCheckBox::toggled,            this,   [=] {FiltrerListe(wdg_alldocsupcheckbox);});
    connect (wdg_onlyimportantsdocsupcheckbox,  &QCheckBox::toggled,            this,   [=] {FiltrerListe(wdg_onlyimportantsdocsupcheckbox);});
    connect (proc,                              &Procedures::UpdDocsExternes,   this,   &dlg_docsexternes::ActualiseDocsExternes);
    connect (PrintButton,                       &QPushButton::clicked,          this,   &dlg_docsexternes::ImprimeDoc);
    connect (ViewerButton,                      &QPushButton::clicked,          this,   &dlg_docsexternes::OpenMultiImageViewer);
    connect (m_ZoomInButton,                    &QPushButton::clicked,          this,   [=] {ZoomDoc();});
    connect (m_ZoomOutButton,                   &QPushButton::clicked,          this,   [=] {ZoomDoc();});

    if (!UtiliseTCP)
    {
        QTimer *TimerActualiseDocsExternes      = new QTimer(this);
        TimerActualiseDocsExternes              ->start(10000);
        connect (TimerActualiseDocsExternes,    &QTimer::timeout,               this,   &dlg_docsexternes::ActualiseDocsExternes);
    }

    m_modetri       = parDate;
    m_docsexternes  ->setNouveauDocumentExterneFalse();
    RemplirTreeView();
    setEnregPosition(true);
    setSaveGeometry(Nom_fiche_DocsExternes);

    //! Placement par défaut quand AUCUNE géométrie n'est encore enregistrée (1er affichage).
    //! Sans cela, originalgeometry() est nul et Display() positionne la fenêtre en haut-gauche,
    //! DERRIÈRE la fenêtre principale → invisible pour l'utilisateur. On l'amène donc dans
    //! l'espace vacant en BAS À DROITE : accolée au flanc droit de la fenêtre principale, sous
    //! la moitié haute occupée par dlg_actesprecedents, jusqu'aux bords droit/bas de l'écran.
    //! Dès que l'utilisateur la déplace, sa position est mémorisée (setEnregPosition(true)) et
    //! ce calcul ne s'applique plus.
    if (originalgeometry() == QRect())
    {
        QWidget *principale = Q_NULLPTR;
        const auto tops = qApp->topLevelWidgets();
        for (QWidget *w : tops)
            if (qobject_cast<QMainWindow*>(w)) { principale = w; break; }
        QScreen *ecr = (principale && principale->screen()) ? principale->screen()
                                                            : QGuiApplication::primaryScreen();
        if (ecr != Q_NULLPTR)
        {
            const QRect dispo = ecr->availableGeometry();
            int x, y;
            if (principale != Q_NULLPTR)
            {
                const QRect r = principale->frameGeometry();
                x = r.right() + 1;            //! accolée au flanc droit de la fenêtre principale
                y = r.top() + r.height() / 2; //! moitié basse, sous dlg_actesprecedents
            }
            else { x = dispo.left() + dispo.width() / 2; y = dispo.top() + dispo.height() / 2; }
            x = qBound(dispo.left(), x, dispo.right()  - 200);
            y = qBound(dispo.top(),  y, dispo.bottom() - 200);
            const QRect defaut(QPoint(x, y), QPoint(dispo.right(), dispo.bottom()));
            setOriginalgeometry(defaut);  //! utilisée par dlg_singleimageviewer::Display() pour le move()
            setGeometry(defaut);
        }
    }
}

dlg_docsexternes::~dlg_docsexternes()
{
}

void dlg_docsexternes::AfficheCustomMenu(DocExterne *docmt)
{
    QModelIndex idx = getIndexFromId(m_model, docmt->id());
    QMenu *menu = new QMenu(this);
    QAction *paction_Modifier       = new QAction(Icons::icEditer(), tr("Le titre"));
    QAction *paction_ModifierDate   = new QAction(Icons::icDate(), tr("La date"));
    QAction *paction_Zoom           = new QAction(mode() == dlg_singleimageviewer::Zoom? Icons::pxZoomOut() : Icons::pxZoomIn(), mode() == dlg_singleimageviewer::Zoom?  tr("Réduire") : tr("Agrandir"));
    if (currentuser()->isSoignant())
    {
        if  (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU || docmt->isVideo())
        {
            QAction *paction_Viewer         = new QAction(Icons::icViewer(), tr("Ouvrir dans le visualisateur"));
            menu->addAction(paction_Viewer);
            connect (paction_Viewer,    &QAction::triggered,    this,  [=] {OpenMultiImageViewer(docmt->id());});
        }
        if (docmt->isVideo())
        {
            QAction *paction_Fullscreen         = new QAction(Icons::pxFullscreen(), tr("Afficher en plein écran"));
            menu->addAction(paction_Fullscreen);
            connect (paction_Fullscreen,    &QAction::triggered,    this,  [=] {
                QString filepath = Procedures::I()->settings()->value(Utils::getBaseFromMode(DataBase::I()->ModeAccesDataBase()) + Dossier_Videos).toString() + "/" + m_currentdocument->lienversfichier();
                Utils::playVideoFullScreen(filepath, this);
            });
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
        connect (paction_Reimprimer,                &QAction::triggered,    this,  [=] {proc->PdfOrPrint(this, docmt->pagelist(), CalcNomFilePdf());});
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
    menu                ->addAction(paction_Zoom);          //! passage en mode zoom : toujours en DERNIER item
    menu->exec(cursor().pos());
    menu->close();
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
    if (mode() == dlg_singleimageviewer::Normal)
        setOriginalgeometry(geometry());

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
    connect (RecordButton,  &UpSmallButton::clicked,    this,   [=] {proc->saveDocumentToFile(m_currentdocument, this);});
    //PrintButton         ->setVisible(!docmt->isVideo());
    labinfowidget()     ->setVisible(!docmt->isVideo());

    if (docmt->isVideo())                   //! le document est une video -> n'est pas stocké dans la base mais dans un fichier sur le disque
    {
        Utils::ModeAcces modeacces = DataBase::I()->ModeAccesDataBase();
        QString filename = proc->settings()->value(Utils::getBaseFromMode(modeacces) + Dossier_Videos).toString() + "/" + docmt->lienversfichier();
        QFile   qFile(filename);
        if (modeacces == Utils::Distant)
            if (proc->settings()->value(Utils::getBaseFromMode(Utils::Distant) + Dossier_Videos).toString() == "" || !qFile.exists())
            {
                UpMessageBox::Watch(this, tr("Video non accessible en accès distant"));
                return;
            }
        if (!qFile.open(QIODevice::ReadOnly))
        {
            QString msg = tr("Erreur d'accès au fichier:") + " " + filename;
            UpMessageBox::Watch(this,msg);
            return;
        }
        DisplayVideo(filename);

        labinfowidget() ->setText("");
    }
    else                                    //! le document est une image ou un document écrit (ordonnance, certificat...)
    {
        proc            ->CalcImageDocument(docmt);
        QString sstitre = "";
        if  (docmt->format() == IMAGERIE || docmt->format() == DOCUMENTRECU)
            sstitre = "<font color='magenta'>" + docmt->datetimeimpression().toString(tr("d-M-yyyy")) + " - " + docmt->soustypedoc() + "</font>";
        labinfowidget()         ->setText(sstitre);
        if (docmt->isJPG())             //! le document est un JPG
        {
            QImage image;
            if (!image.loadFromData(docmt->imageblob()))
            {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                return;
            }
        }
        else if (docmt->isPDF())        //! le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage)
        {
            QList<QImage> listimg;
            listimg         = docmt->pagelist();
            if (listimg.size() == 0)
            {
                UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                return;
            }
        }
        else return;
        setListDocuments(QList<DocExterne*>() << docmt);
    }
    imagewidget()->disconnect();
    connect(imagewidget(),  &QWidget::customContextMenuRequested,   this, [=] {AfficheCustomMenu(docmt);});
    if (mode() == dlg_singleimageviewer::Zoom)
        ZoomDoc(false);
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
        emit Actualize();
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
            proc->PdfOrPrint(this, docmt->pagelist(), CalcNomFilePdf());

        //Réédition d'un document - on va réimprimer le document à la date du jour en le modifiant - ne concerne que les courriers et ordonnances émis => on enregistre le nouveau document dans la BDD
        else if (msgbox.clickedButton() == &ModifEtReimprBouton || msgbox.clickedButton() == &ImpAujourdhuiBouton)
            ModifieEtReImprimeDoc(docmt, msgbox.clickedButton() == &ModifEtReimprBouton, detruirealafin);
        msgbox.close();
    }
    connect(PrintButton, &QPushButton::clicked, this, &dlg_docsexternes::ImprimeDoc);
#endif
}

QMap<QString, QString> dlg_docsexternes:: CalcNomFilePdf()
{
    QMap<QString, QString>  map = QMap<QString, QString>();
    proc->setDirnamepdf(tr("Documents") + " - " + currentuser()->prenom() + " " + currentuser()->nom() + " - " + QLocale::system().toString(QDate::currentDate(),"dd MMM yyyy"));
    QString dirname     = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at((0)) + "/" + proc->dirnamepdf();
    QString filename    = m_docsexternes->patient()->prenom() + " " + m_docsexternes->patient()->nom()  + " - " + m_currentdocument->titre() + ".pdf";
    QString msgOK       = tr("fichier") +" " + QDir::toNativeSeparators(filename) + "\n" +
                    tr ("sauvegardé sur le bureau dans le dossier ") + "\n" +
                    proc->dirnamepdf();
    map.insert("dir", dirname);
    map.insert("file", filename);
    map.insert("msg", msgOK);
    return map;
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
    textpied = proc->CalcPiedImpression(currentuser());

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

    bool pdf = false;
    if (!proc->QuestionPdfOrPrint(this, pdf))
        return false;
    if (pdf)
    {
        QMap<QString, QString> map = CalcNomFilePdf();
        QString dirname     = map.value("dir");
        QString filename    = map.value("file");
        QString msgOK       = map.value("msg");
        proc->Cree_pdffile(textcorps, textentete, textpied,filename, (Prescription? currentuser() : Q_NULLPTR), ALD, dirname);
        QFile file          = QFile(dirname + "/" + filename);
        aa                  = file.exists();
        UpMessageBox::Watch(this,
                            aa? tr("Enregistrement pdf") : tr("Echec enregistrement pdf"),
                            aa? msgOK : tr ("Impossible d'enregistrer le fichier ") + QDir::toNativeSeparators(filename));
    }
    else
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

void dlg_docsexternes::ModifierDate(QModelIndex idx)
{
    DocExterne *docmt = getDocumentFromIndex(idx);
    UpDialog * dlg              = new UpDialog();
    dlg                         ->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QDateEdit   *dateedit       = new QDateEdit(dlg);
    UpLabel     *label          = new UpLabel(dlg);
    dlg->dlglayout()->insertWidget(0,dateedit);
    dlg->dlglayout()->insertWidget(0,label);
    dlg->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);

    dlg->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    dlg->setStageCount(0.6);
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
    dlg                         ->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    UpLineEdit  *Line           = new UpLineEdit(dlg);
    UpLabel     *label          = new UpLabel(dlg);
    dlg->dlglayout()->insertWidget(0,Line);
    dlg->dlglayout()->insertWidget(0,label);
    dlg->AjouteLayButtons(UpDialog::ButtonCancel|UpDialog::ButtonOK);

    dlg->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
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
    dlg->setStageCount(0.6);
    dlg->exec();
    delete dlg;
}

void dlg_docsexternes::OpenMultiImageViewer(int iddoc)
{
    dlg_multiimageviewer *viewer = new dlg_multiimageviewer(m_listiddocsimagery, iddoc);
    connect(this, &dlg_docsexternes::Actualize, viewer, &dlg_multiimageviewer::Actualize);
    viewer->exec();
    delete viewer;
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
    QModelIndex idx = wdg_listdocstreewiew->currentIndex();
    if (!changemode)
        setMode(mode() == dlg_singleimageviewer::Zoom? dlg_singleimageviewer::Normal : dlg_singleimageviewer::Zoom);
    if (mode() == dlg_singleimageviewer::Normal)
    {
        if (changemode)
        {
            m_positionorigin = pos();
            m_sizeorigin     = size();
        }
        //! set dimensions to max (changeMode() centre déjà la fiche -> plus de move(0,0) à la main)
        changeMode(dlg_singleimageviewer::Zoom);
    }
    else if (mode() == dlg_singleimageviewer::Zoom)
    {
        move(m_positionorigin);
        resize(m_sizeorigin);
        changeMode(dlg_singleimageviewer::Normal);
    }
    bool zoomvisible = mode() == dlg_singleimageviewer::Normal;
    m_ZoomInButton      ->setVisible(zoomvisible);
    m_ZoomOutButton     ->setVisible(!zoomvisible);
    wdg_listdocstreewiew->scrollTo(idx, QAbstractItemView::PositionAtCenter);
    wdg_listdocstreewiew->setCurrentIndex(idx);
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
        if (imagewidget())  imagewidget()->setVisible(false);

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
        //! icône selon le format du document (calculée UNE fois, posée sur les deux modèles)
        QIcon docicon;
        if      (doc->format() == PRESCRIPTION)          docicon = Icons::icStetho();
        else if (doc->format() == PRESCRIPTIONLUNETTES)  docicon = Icons::icSunglasses();
        else if (doc->format() == VIDEO)                 docicon = Icons::icCinema();
        else if (doc->format() == IMAGERIE)              docicon = Icons::icPhoto();
        else if (doc->format() == COURRIER)              docicon = Icons::icImprimer();
        else if (doc->format() == COURRIERADMINISTRATIF) docicon = Icons::icTampon();
        else if (doc->format() == DOCUMENTRECU)
            docicon = (doc->typedoc() == COURRIER)? Icons::icImprimer() : Icons::icPhoto();
        //! document important : on ACCOLE l'étoile à droite de l'icône de format (au lieu de
        //! l'écraser) -> on conserve l'info du type ET de l'importance.
        if (doc->importance() == 2)
        {
            const int s = 24;
            QPixmap combine(2*s + 2, s);
            combine.fill(Qt::transparent);
            QPainter peintre(&combine);
            peintre.drawPixmap(0,     0, docicon.pixmap(s, s));
            peintre.drawPixmap(s + 2, 0, Icons::icImportant().pixmap(s, s));
            peintre.end();
            docicon = QIcon(combine);
        }
        pitemdate->setIcon(docicon);
        pitemtype->setIcon(docicon);
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
