#include "dlg_imageviewer.h"
#include <QtWidgets/qheaderview.h>

dlg_imageviewer::dlg_imageviewer(DocsExternes* Docs, QWidget *parent) : UpDialog(Nom_fiche_Viewer, parent)
{
    m_docs                      = Docs;
    m_formatdate                = tr("dd-MM-yyyy");
    AjouteLayButtons(UpDialog::ButtonOK);
    wdg_table                   = new UpTableView();
    wdg_table                   ->setPalette(QPalette(Qt::white));
    wdg_table                   ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_table                   ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_table                   ->setFocusPolicy(Qt::StrongFocus);
    wdg_table                   ->setMouseTracking(true);
    wdg_table->verticalHeader() ->setVisible(true);

    QHBoxLayout *boxODG_Lay     = new QHBoxLayout();
    int width                   = 120;
    QString stylechk            = "font: bold;";
    ODchkBox                    ->setFixedWidth(width);
    OGchkBox                    ->setFixedWidth(width);
    BothchkBox                  ->setFixedWidth(width);
    ODchkBox                    ->setStyleSheet(stylechk);
    OGchkBox                    ->setStyleSheet(stylechk);
    BothchkBox                  ->setStyleSheet(stylechk);
    boxODG_Lay                  ->addLayout(new QHBoxLayout);
    boxODG_Lay                  ->addWidget(ODchkBox);
    boxODG_Lay                  ->addLayout(new QHBoxLayout);
    boxODG_Lay                  ->addWidget(BothchkBox);
    boxODG_Lay                  ->addLayout(new QHBoxLayout);
    boxODG_Lay                  ->addWidget(OGchkBox);
    boxODG_Lay                  ->addLayout(new QHBoxLayout);

    QVBoxLayout *tablelay       = new QVBoxLayout;
    tablelay                    ->addWidget(wdg_table);
    tablelay                    ->setSizeConstraint(QLayout::SetFixedSize);
    m_gridlay                   ->addWidget(new QWidget(),0,0);
    m_gridlay                   ->addLayout(tablelay, 1,0);
    m_gridlay                   ->addLayout(boxODG_Lay, 0,1);
    m_gridlay                   ->addWidget(wdg_treeview,1,1);
    m_gridlay                   ->setColumnStretch(0,2);
    m_gridlay                   ->setColumnStretch(1,10);
    m_gridlay                   ->setRowStretch(0,2);
    m_gridlay                   ->setRowStretch(1,15);

    wdg_table                   ->setSelectionMode(QAbstractItemView::NoSelection);
    wdg_table                   ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    wdg_treeview                ->setSelectionMode(QAbstractItemView::NoSelection);
    wdg_treeview                ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    wdg_treeview                ->setHeaderHidden(true);

    dlglayout()->insertLayout(0,m_gridlay);
    connect(OKButton,           &QPushButton::clicked,  this,   &QDialog::close);

    FillXTable(Docs);

    //! check XTable first row
    UpStandardItem *item = Q_NULLPTR;
    for (int i= 0; i < m_Xmodel->columnCount(); i++)
    {
        item = dynamic_cast<UpStandardItem*>(m_Xmodel->item(0,i));
        if (item != Q_NULLPTR)
            if (item->hascheckBox())
            {
                QModelIndex idx = item->index();                                        // l'index de ce dernier item
                wdg_table->scrollTo(idx, QAbstractItemView::PositionAtTop);
                wdg_table->setCurrentIndex(idx);
                checkVerticalHeader(item->row());
                break;
            }
    }

    controlChecks();
    m_listcheckedItems = listcheckedItems();
    addItemsToTreeWidget(m_listcheckedItems);

    installEventFilter(this);

    connect (wdg_table,                     &QTableView::entered,               this,   &dlg_imageviewer::gettoolTipFromCursorPositionInTable);
    connect (wdg_table,                     &QTableView::clicked,               this,   [=] (QModelIndex idx) {
        controlChecks();
        UpStandardItem *upitem      = dynamic_cast<UpStandardItem *>(m_Xmodel->itemFromIndex(idx));
        if (upitem == Q_NULLPTR)
            return;
        if (upitem->checkState() == Qt::Checked  || upitem->checkState() == Qt::PartiallyChecked)
            addItemsToTreeWidget(QList<UpStandardItem *>() << upitem);
        else
            removeItemsFromtTreeWidget(QList<UpStandardItem *>() << upitem);
    });
    connect (wdg_table->horizontalHeader(), &QHeaderView::sectionClicked,       this,   [=] (int idx) {checkHorizontalHeader(idx);} );
    connect (wdg_table->verticalHeader(),   &QHeaderView::sectionClicked,       this,   [=] (int idx) {checkVerticalHeader(idx);} );
}

void dlg_imageviewer::FillXTable(DocsExternes* docs)
{
    for (auto it = m_docs->docsexternes()->begin(); it  != m_docs->docsexternes()->end(); ++it)
    {
        DocExterne *doc = qobject_cast<DocExterne*>(it.value());
        if (doc != Q_NULLPTR)
        {
            if (!m_listdates.contains(doc->date()))
                m_listdates     << doc->date();
            if (!m_listtypedocs.contains(doc->typedoc()))
                m_listtypedocs  << doc->typedoc();
        }
    }
    std::reverse(m_listdates.begin(), m_listdates.end());

    //! sort mtypedocs with OCT, RNM, Biometry first
    QStringList listtypes = DocExterne::listtypedocs();
    for (int i= 0; i < DocExterne::listtypedocs().size() ; ++i)
    {
        if (!m_listtypedocs.contains(DocExterne::listtypedocs().at(i)))
            listtypes.removeAll(DocExterne::listtypedocs().at(i));
    }
    m_listtypedocs = listtypes;


    if (m_Xmodel != Q_NULLPTR)
        delete m_Xmodel;
    m_Xmodel             = new UpStandardItemModel();
    m_Xmodel             ->setRowCount(m_listdates.size());
    m_Xmodel             ->setColumnCount(m_listtypedocs.size());
    m_Xmodel             ->setHorizontalHeaderItem(0, new UpStandardItem(""));
    m_Xmodel             ->setVerticalHeaderItem(0, new UpStandardItem(""));
    for (int i=0;  i < m_Xmodel->columnCount(); i++)
    {
        m_Xmodel->setHeaderData(i, Qt::Horizontal,
                               m_listtypedocs.at(i),
                               Qt::DisplayRole);
        m_Xmodel->setHeaderData(i, Qt::Horizontal,
                               Icons::pxunCheck(),
                               Qt::DecorationRole);
        m_Xmodel->setHeaderData(i, Qt::Vertical, Qt::AlignCenter, Qt::TextAlignmentRole);
    }

    for (int i=0;  i < m_listdates.size(); i++)
    {
        m_Xmodel->setHeaderData(i, Qt::Vertical,
                               m_listdates.at(i).toString(m_formatdate),
                               Qt::DisplayRole);
        m_Xmodel->setHeaderData(i, Qt::Vertical,
                               Icons::pxunCheck(),
                               Qt::DecorationRole);
        m_Xmodel->setHeaderData(i, Qt::Vertical, Qt::AlignCenter, Qt::TextAlignmentRole);
        //! ----------------------------------------------------------------- create UpStandardItem for each cell of the row
        for (int j=0; j < m_Xmodel->columnCount(); j++)
        {
            //! ------------------------------------------------------------- search all docs for this date
            QMap<int, DocExterne*> mapdocs = QMap<int, DocExterne*>();
            for (auto it = docs->docsexternes()->begin(); it  != docs->docsexternes()->end(); ++it)
            {
                DocExterne *doc = qobject_cast<DocExterne*>(it.value());
                if (doc != Q_NULLPTR)
                    if (doc->date() == m_listdates.at(i))
                        mapdocs.insert(doc->id(), doc);
            }
            //! search all docs for this date & this type
            QList<int> listiddocs = QList<int>();
            for (auto it = mapdocs.begin(); it  != mapdocs.end(); ++it)
            {
                DocExterne *doc = qobject_cast<DocExterne*>(it.value());
                if (doc != Q_NULLPTR)
                    if (doc->typedoc() == m_listtypedocs.at(j))
                    {
                        listiddocs << doc->id();
                    }
            }
            //! -------------------------------------------------------------- if there is 1 or more docs for this date/type, create a checkbox with list of docs->di()
            UpStandardItem * item = new UpStandardItem();
            item->setData(m_listdates.at(i), Qt::UserRole);
            if (listiddocs.size() >0)
            {
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setTextAlignment(Qt::AlignCenter);
                item->setListids(listiddocs);
                item->setCheckState(Qt::Unchecked);
                item->setAutoTristate(listiddocs.size() >1);
                item->setCheckable(true);
                item->setdataType(m_listtypedocs.at(j));
            }
            m_Xmodel->setItem(i,j, item);
        }
    }
    m_Xmodel                    ->setSortRole(Qt::UserRole);
    m_Xmodel                    ->sort(0, Qt::DescendingOrder);
    wdg_table                   ->setModel(m_Xmodel);
    wdg_table                   ->setItemDelegate(new UpchkDelegate);
    int colwidth                = 60;
    wdg_table                   ->setFixedWidth(wdg_table->verticalHeader()->width() + colwidth * m_Xmodel->columnCount());
    for (int i=0;  i != m_Xmodel->columnCount(); i++)
        wdg_table               ->setColumnWidth(i,colwidth);
}

dlg_imageviewer::~dlg_imageviewer()
{
}

void dlg_imageviewer::gettoolTipFromCursorPositionInTable()
{
    QModelIndex pindx   = wdg_table->indexAt(wdg_table->viewport()->mapFromGlobal(cursor().pos()));
    QString msg;
    QList<DocExterne*> listdocs = getListDocsfromIndex(pindx);
    for (int i = 0; i < listdocs.size(); ++i)
    {
        if (listdocs.at(i) != Q_NULLPTR)
        {
            if (msg != "")
                msg += "\n";
            msg += listdocs.at(i)->soustypedoc();
        }
    }
    QToolTip::showText(cursor().pos(),msg);
}

QList<DocExterne*> dlg_imageviewer::getListDocsfromIndex(QModelIndex idx)
{
    QList<DocExterne*>listdocs  = QList<DocExterne*>();
    UpStandardItem *upitem      = dynamic_cast<UpStandardItem *>(m_Xmodel->itemFromIndex(idx));
    if (upitem == Q_NULLPTR)
        return listdocs;
    if (upitem->listids() == QList<int>())
        return listdocs;
    else
    {
        QString msg;
        for (int i = 0; i < upitem->listids().size(); ++i)
        {
            DocExterne *doc = m_docs->getById(upitem->listids().at(i));
            listdocs << doc;
        }
    }
    return listdocs;
}

UpStandardItem* dlg_imageviewer::itemfromIndex(QModelIndex idx)
{
    return dynamic_cast<UpStandardItem *>(m_Xmodel->itemFromIndex(idx));
}

/*!
 * \brief dlg_imageviewer::listcheckabledItems
 * \return list des items pouvant être cochés
*/
QList<UpStandardItem*> dlg_imageviewer::listcheckabledItems()
{
    QList<UpStandardItem*> listitems = QList<UpStandardItem*>();
    for (int i = 0; i<m_Xmodel->rowCount(); i++)
    {
        for (int j = 0; j<m_Xmodel->columnCount(); j++)
        {
            UpStandardItem * item = dynamic_cast<UpStandardItem*>(m_Xmodel->item(i,j));
            if (item != Q_NULLPTR)
            {
                QVariant value = item->data(Qt::CheckStateRole);
                if (value.isValid())
                        listitems << item;
            }
        }
    }
    return listitems;
}

/*!
 * \brief dlg_imageviewer::listcheckabledItems
 * \return list des items cochés
*/
QList<UpStandardItem*> dlg_imageviewer::listcheckedItems()
{
    QList<UpStandardItem*> listitemstocheck = listcheckabledItems();
    QList<UpStandardItem*> listitems = QList<UpStandardItem*>();
    for  (int i =0; i<listitemstocheck.size(); ++i )
        if (listitemstocheck.at(i)->checkState() == Qt::Checked || listitemstocheck.at(i)->checkState() == Qt::PartiallyChecked)
            listitems << listitemstocheck.at(i);
    return listitems;
}

void dlg_imageviewer::checkHorizontalHeader(int idx)
{
    QVariant variant = m_Xmodel->headerData(idx, Qt::Horizontal, Qt::DecorationRole);
    QImage image = variant.value<QImage>();
    bool uncheck = image == Icons::pxunCheck().toImage();
    m_Xmodel->setHeaderData(idx, Qt::Horizontal,
                        uncheck? Icons::pxCheck() : Icons::pxunCheck(),
                        Qt::DecorationRole);
    QList<UpStandardItem*> litsitems =QList<UpStandardItem*>();
    QList<UpStandardItem*> litems = listcheckabledItems();
    for  (int i =0; i<litems.size(); ++i )
        if (litems.at(i)->column() == idx)
        {
            if (litems.at(i)->checkState() == Qt::Checked && uncheck)
                litsitems << litems.at(i);
            else if (litems.at(i)->checkState() != Qt::Checked && !uncheck)
                litsitems << litems.at(i);
            litems.at(i)->setCheckState(uncheck? Qt::Checked : Qt::Unchecked);
        }
    controlChecks();
    if (!uncheck)
        addItemsToTreeWidget(litsitems);
    else
        removeItemsFromtTreeWidget(litsitems);
}

void dlg_imageviewer::checkVerticalHeader(int idx)
{
    QVariant variant = m_Xmodel->headerData(idx, Qt::Vertical, Qt::DecorationRole);
    QImage image = variant.value<QImage>();
    bool uncheck = image == Icons::pxunCheck().toImage();
    m_Xmodel->setHeaderData(idx, Qt::Vertical,
                               uncheck? Icons::pxCheck() :Icons::pxunCheck(),
                               Qt::DecorationRole);
    QList<UpStandardItem*> litsitems =QList<UpStandardItem*>();
    QList<UpStandardItem*> litems = listcheckabledItems();
    for  (int i =0; i<litems.size(); ++i )
        if (litems.at(i)->row() == idx)
        {
            if (litems.at(i)->checkState() == Qt::Checked && uncheck)
                litsitems << litems.at(i);
            else if (litems.at(i)->checkState() != Qt::Checked && !uncheck)
                litsitems << litems.at(i);
            litems.at(i)->setCheckState(uncheck? Qt::Checked : Qt::Unchecked);
        }
    controlChecks();
    if (!uncheck)
        addItemsToTreeWidget(litsitems);
    else
        removeItemsFromtTreeWidget(litsitems);
}

/*!
 \brief dlg_imageviewer::controlChecks
 After a check on un uptstandarditem, correct headers icons
 */
void dlg_imageviewer::controlChecks() //! After a check on an upstandarditem, correct headers icons
{
    // ctrl for rows
    for (int i=0; i < m_Xmodel->rowCount(); i++)
    {
        bool isoneunchecked = false, isonechecked = false;
        for (int j=0; j < m_Xmodel->columnCount(); j++)
        {
            UpStandardItem *item= dynamic_cast<UpStandardItem*>(m_Xmodel->item(i,j));
            if (item != Q_NULLPTR)
                if (item->hascheckBox())
                {
                    if (item->checkState() == Qt::Checked)
                        isonechecked = true;
                    if (item->checkState() == Qt::Unchecked)
                        isoneunchecked = true;
                    if (isoneunchecked && isonechecked)
                        break;
                }
        }
        bool allchecked = !isoneunchecked;
        bool partiallychecked = isoneunchecked && isonechecked;
        bool nonechecked = !isonechecked;
        if (allchecked)
            m_Xmodel->setHeaderData(i, Qt::Vertical,
                                   Icons::pxCheck(),
                                   Qt::DecorationRole);
         else if (partiallychecked)
            m_Xmodel->setHeaderData(i, Qt::Vertical,
                                   Icons::pxblockCheck(),
                                   Qt::DecorationRole);
        else if (nonechecked)
             m_Xmodel->setHeaderData(i, Qt::Vertical,
                                    Icons::pxunCheck(),
                                    Qt::DecorationRole);
     }
    // ctrl for columns
    for (int i=0; i < m_Xmodel->columnCount(); i++)
    {
        bool isoneunchecked = false, isonechecked = false;
        for (int j=0; j < m_Xmodel->rowCount(); j++)
        {
            UpStandardItem *item= dynamic_cast<UpStandardItem*>(m_Xmodel->item(j,i));
            if (item != Q_NULLPTR)
                if (item->hascheckBox())
                {
                    if (item->checkState() == Qt::Checked)
                        isonechecked = true;
                    else if (item->checkState() == Qt::Unchecked)
                        isoneunchecked = true;
                    if (isoneunchecked && isonechecked)
                        break;
                }
        }
        bool allchecked = !isoneunchecked;
        bool partiallychecked = isoneunchecked && isonechecked;
        bool nonechecked = !isonechecked;
        if (allchecked)
            m_Xmodel->setHeaderData(i, Qt::Horizontal,
                                   Icons::pxCheck(),
                                   Qt::DecorationRole);
        else if (partiallychecked)
            m_Xmodel->setHeaderData(i, Qt::Horizontal,
                                   Icons::pxblockCheck(),
                                   Qt::DecorationRole);
        else if (nonechecked)
            m_Xmodel->setHeaderData(i, Qt::Horizontal,
                                   Icons::pxunCheck(),
                                   Qt::DecorationRole);
    }
    m_listdocsToDisplay = listdocsToDisplay();
    checkEyes();
}

void dlg_imageviewer::removeItemsFromtTreeWidget(QList<UpStandardItem *> listupitems)
{
    foreach (UpStandardItem* item, listupitems) {
        for (int i=0; i < m_treemodel->rowCount();)
        {
            QStandardItem* dateitem = m_treemodel->item(i);
            if(dateitem->data(Qt::UserRole).toDate() == item->data(Qt::UserRole).toDate() && m_treemodel->item(i)->hasChildren())
            {

                for (int j=0; j < dateitem->rowCount();)
                {
                    QStandardItem* wdgitem = dateitem->child(j,0);
                    if (wdgitem)
                    {
                        QWidget * widg = wdg_treeview->indexWidget(wdgitem->index());
                        if (widg->property(M_DATE).toDate() == item->data(Qt::UserRole).toDate() && widg->property(M_TYPE).toString() == item->dataType())
                        {
                            QList<QStandardItem*> lisdelit = dateitem->takeRow(j);
                            foreach (QStandardItem *itm, lisdelit)
                                if (itm != Q_NULLPTR) delete itm;
                            j--;
                        }
                    }
                    j++;
                }
                if (!dateitem->hasChildren())
                {
                    QList<QStandardItem*> lisdelit = m_treemodel->takeRow(i);
                    foreach (QStandardItem *itm, lisdelit)
                        if (itm != Q_NULLPTR) delete itm;
                    i--;
                }
            }
            i++;
        }
    }
    /*
    if (m_treemodel != Q_NULLPTR)
        delete m_treemodel;
    m_treemodel             = new UpStandardItemModel();
    QItemSelectionModel *m  = wdg_treeview->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    wdg_treeview            ->setModel(m_treemodel);
    delete m;
    m_listcheckedItems = listcheckedItems();
    addItemsToTreeWidget(m_listcheckedItems);*/
}

QList<DocExterne*> dlg_imageviewer::listdocsToDisplay()
{
    QList<DocExterne*> listdocs = QList<DocExterne*>();
    QList<UpStandardItem*> litems = listcheckabledItems();
    for  (int i =0; i<litems.size(); ++i )
    {
        if (litems.at(i)->checkState() == Qt::Checked)
        {
            QList<int> ids = litems.at(i)->listids();
            for (int j=0; j <ids.size(); j++)
            {
                DocExterne *doc = m_docs->getById(ids.at(j));
                if (doc != Q_NULLPTR)
                    if (!listdocs.contains(doc))
                        listdocs << doc;
            }
        }
    }
    return listdocs;
}

void dlg_imageviewer::checkEyes()
{
    bool checkOD = false;
    bool checkOG = false;
    bool checkBoth = false;
    for (int i=0; i < m_listdocsToDisplay.size(); i++)
    {
        if (m_listdocsToDisplay.at(i)->stringCote() == "R") checkOD = true;
        else if (m_listdocsToDisplay.at(i)->stringCote() == "L") checkOG = true;
        else if (m_listdocsToDisplay.at(i)->stringCote() == "") checkBoth = true;
    }
    if (checkOD)
        ODchkBox->setCheckState(checkOD? Qt::Checked : Qt::Unchecked);
    ODchkBox->setEnabled(checkOD);
    if (checkOG)
        OGchkBox->setCheckState(checkOG? Qt::Checked : Qt::Unchecked);
    OGchkBox->setEnabled(checkOG);
    if (checkBoth)
        BothchkBox->setCheckState(checkBoth? Qt::Checked : Qt::Unchecked);
    BothchkBox->setEnabled(checkBoth);
}

QWidget* dlg_imageviewer::DocWidget(QList<DocExterne *> listdocs)
{
    /*!
    * if sided pics
        * the function has received two elements: one document and one nullptr, or two documents. The docwidget will be created with two elements
            * it the function has received two documents, the docwidget is created with two pics
            * if the function has received one document and a nullptr, a QSpacerItem for the nullptr and one pict for the documents
    * if not sided pics
        * the function has received one or two documents and the dock widget will be created with two or three elements
            * it the function has received two documents, the docwidget is created with two picts
            * if the function has received one document, the docwidget is created with one pict surrounded by two QSpacerItems
    */

    QWidget * widg = Q_NULLPTR;
    if (listdocs.size() == 0 || listdocs.size() >2)
        return widg;
    QGridLayout *glay   = new QGridLayout();
    glay                ->setContentsMargins(m_marg);
    glay                ->setSpacing(m_spacing);
    QDate date = QDate();
    QString type = QString();
    for (int it = 0; it < listdocs.size() ; ++it)
    {
        DocExterne * doc = listdocs.at(it);
        if (doc == Q_NULLPTR)
        {
            glay->addItem(new QSpacerItem(5,5,QSizePolicy::Expanding),0, it);
        }
        else
        {
            if (date == QDate())
                date = doc->date();
            if (type == QString())
                type = doc->typedoc();
            Procedures::I()->CalcImageDocument(doc);
            QList<QPixmap> listpixmp;

            if (doc->isVideo())  // le document est une video -> n'est pas stocké dans la base mais dans un fichier sur le disque
            {
                if (DataBase::I()->ModeAccesDataBase() == Utils::Distant)
                {
                    UpMessageBox::Watch(this, tr("Video non accessible en accès distant"));
                    return Q_NULLPTR;
                }
                QString filename = DataBase::I()->dirimagerie() + NOM_DIR_VIDEOS "/" + doc->lienversfichier();
                QFile   qFile(filename);
                if (!qFile.open(QIODevice::ReadOnly))
                {
                    QString msg = tr("Erreur d'accès au fichier:") + " " + filename;
                    UpMessageBox::Watch(this,msg);
                    return Q_NULLPTR;
                }
                QVideoWidget *wdg_video             = new QVideoWidget();
                QMediaPlayer *medplay_player        = new QMediaPlayer;
                medplay_player                      ->setSource( QUrl::fromLocalFile(filename));
                medplay_player                      ->setVideoOutput(wdg_video);
                medplay_player                      ->play();
                QSize size                          = wdg_video->videoSink()->videoSize();
                int w                               = sizeforunit().width();
                double nx                           = size.width();
                double ny                           = size.height();
                int h                               = int(w*ny/nx);
                wdg_video                           ->setFixedSize(w,h);
                //wdg_video                           ->installEventFilter(this);
                glay->addWidget(wdg_video,0,it);
            }
            else if (doc->imageformat() == JPG)     // le document est un JPG
            {
                QImage image;
                if (!image.loadFromData(doc->imageblob()))
                {
                    UpMessageBox::Watch(this,tr("Impossible de charger le document"));
                    return Q_NULLPTR;
                }
                UpLabel * lab   = new UpLabel;
                lab             ->setImage(image);
                QPixmap pix     = QPixmap::fromImage(image).scaled(sizeforunit(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
                lab             ->setPixmap(pix);
                glay->addWidget(lab,0,it);
            }
            else if (doc->imageformat() == PDF)     // doc is pdf
            {
                QList<QImage> listimg;
                listimg = doc->pagelist();

                if (listimg.size())
                {
                    UpTableWidget *tblwdg = new UpTableWidget();                                  // utilisé pour afficher les pdf qui ont parfois plusieurs pages
                    tblwdg      ->setListimg(listimg);
                    tblwdg->horizontalHeader() ->setVisible(false);
                    tblwdg->verticalHeader()   ->setVisible(false);
                    tblwdg      ->setFocusPolicy(Qt::NoFocus);
                    tblwdg      ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
                    tblwdg      ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    tblwdg      ->setColumnCount(1);
                    tblwdg      ->setRowCount(listimg.size());
                    int maxw(0), maxh(0);
                    for (int i=0; i<listimg.size();++i)
                    {
                        QImage image = listimg.at(i);
                        QPixmap pix     = QPixmap();
                        listpixmp << pix;
                        pix     = QPixmap::fromImage(image).scaled(sizeforunit(),
                                                               Qt::KeepAspectRatioByExpanding,
                                                               Qt::SmoothTransformation);
                        UpLabel *lab            = new UpLabel();
                        lab                     ->resize(pix.width(),pix.height());
                        lab                     ->setPixmap(pix);
                        int x = pix.width();
                        int y = pix.height();
                        tblwdg   ->setColumnWidth(i,x);
                        tblwdg   ->setRowHeight(i,y);
                        tblwdg   ->setCellWidget(i,0,lab);
                        if (x>maxw) maxw = x;
                        if (y>maxh) maxh = y;
                    }
                    tblwdg  ->setFixedSize(maxw,maxh);
                    glay    ->addWidget(tblwdg,0,it);
                }
            }
        }
    }
    widg = new QWidget();
    if (listdocs.size() == 1)
    {
        glay->addItem(new QSpacerItem(5,5, QSizePolicy::Expanding),0,0);
        glay->addItem(new QSpacerItem(5,5, QSizePolicy::Expanding),0,2);
        widg->setLayout(glay);
    }
    else
        widg->setLayout(glay);
    widg->setProperty(M_DATE, date);
    widg->setProperty(M_TYPE, type);
    return widg;
}

void dlg_imageviewer::addItemsToTreeWidget(QList<UpStandardItem *> listupitems)
{
    /*!
     * There are two types of documents: sided or not sided
        * sided means than the document is explicitly assigned to one eye, right or left
        * not sided, the document assignment is unknown or for the two eyes
    * The goal is to create row of two elements that will be inserted in treeview, sorted by date decreasing
    * each row contains one or two elements, built with DocWidget() function
        * for sided docs
            * two pics, one for right eye and one for left eye
                                    * -> DocWidget is called with the two documents
            * if there is only one pic rright or left
                                    * -> DocWidget is called with one document and one null pointer for the missing eye, in order to set the picts on right or left size of the docwidget.
        * for not sided docs
            * two pics, side by side ih the number of docs is 2
                                    * -> DocWidget is called with the two documents
            * one pic centered if ther is only one document
                                    * ->  DocWidget is called with only one document
    */

    auto sstypdoc = [=] (DocExterne* doc) {
        return doc->soustypedoc().replace("OD","").replace("OG","");
    };

    QList<QDate> listdates =  QList<QDate>();
    for (int i=0;  i < m_Xmodel->rowCount(); i++)
    {
        QVariant variant = m_Xmodel->headerData(i, Qt::Vertical, Qt::DecorationRole);
        QImage image = variant.value<QImage>();
        if (image == Icons::pxCheck().toImage() || image == Icons::pxblockCheck().toImage())
            listdates << QDate::fromString(m_Xmodel->headerData(i, Qt::Vertical, Qt::DisplayRole).value<QString>(), m_formatdate);
    }
    if (m_treemodel == Q_NULLPTR)
    {
        m_treemodel                 = new UpStandardItemModel();
        wdg_treeview                ->setModel(m_treemodel);
    }

    for (int i=0; i<listdates.size(); ++i)
    {
        QList<QStandardItem *> listitemsdate    = m_treemodel->findItems(listdates.at(i).toString(m_formatdate));
        if (listitemsdate.size() == 0)
        {
            QString datestring      = listdates.at(i).toString(m_formatdate);
            QStandardItem *dateitem = new QStandardItem(datestring);
            dateitem                ->setForeground(QBrush(QColor(Qt::red)));
            dateitem                ->setEditable(false);
            dateitem                ->setIcon(Icons::icDate());
            dateitem                ->setData(listdates.at(i), Qt::UserRole);
            m_treemodel             ->appendRow(QList<QStandardItem *>() << dateitem);
        }
    }
    //QList<QStandardItem *> listitemsdate = QList<QStandardItem *>();
    //for (int i=0; i<m_treemodel->rowCount(); ++i)
        //listitemsdate << m_treemodel->item(i);
    m_treemodel             ->setSortRole(Qt::UserRole);
    m_treemodel             ->sort(0, Qt::DescendingOrder);
    foreach (UpStandardItem *item, listupitems)
    {
        QList<int> ids                      = QList<int>();
        QList<DocExterne*> listdocssided    = QList<DocExterne*>();
        QList<DocExterne*> listdocsboth     = QList<DocExterne*>();
        int row                             = item->row();
        QString datestring                  = m_Xmodel->headerData(row, Qt::Vertical, Qt::DisplayRole).toString();
        QDate date                          = QDate::fromString(datestring).fromString(m_formatdate);

        if (item->listids().size() >0)
        {
            foreach (int id, item->listids()) {
                if (!ids.contains(id))
                    ids << id;
            }
        }

        //! group by sided or not
        for (int j=0; j <ids.size(); j++)
        {
            DocExterne *doc = m_docs->getById(ids.at(j));
            if (doc != Q_NULLPTR)
            {
                if (doc->cote()>0)
                {
                    if (!listdocssided.contains(doc))
                        listdocssided << doc;
                }
                else if (!listdocsboth.contains(doc))
                    listdocsboth << doc;
            }
        }

//!-----sided docs -> group by ss-type : one row by sstype with image for right eye on right side and for those for left eye on left side
        QStringList listsstype = QStringList();
        foreach (DocExterne* doc, listdocssided)
        {
            QString sstype = sstypdoc(doc);
            if (!listsstype.contains(sstypdoc(doc)))
                listsstype << sstype;
        }

        //! sometimes it's possible to get 3 or more pics for 1 sstype
        //! if number is odd all right pics stay on right side and all left on left side
        //! create rows for each sstype of docs sided
        foreach (QString sstype, listsstype)
        {
            QList<DocExterne*> listdocsR = QList<DocExterne*>();
            QList<DocExterne*> listdocsL = QList<DocExterne*>();
            int nr(0), nl(0), nrows(0);
            foreach (DocExterne* doc, listdocssided)
            {
                if (sstypdoc(doc) == sstype)
                {
                    doc->cote() == 1? ++nr : ++nl;
                    doc->cote() == 1? listdocsR << doc : listdocsL << doc;
                }
            }

            nrows = std::max(nr,nl);
            for (int i = 0; i< nrows; ++i)
            {
                QList<DocExterne*> listrow = QList<DocExterne*>();
                if (listdocsR.size() >0)
                    listrow  << listdocsR.takeFirst();
                else
                    listrow  << Q_NULLPTR;
                if (listdocsL.size() >0)
                    listrow  << listdocsL.takeFirst();
                else
                    listrow  << Q_NULLPTR;
                QStandardItem *item                     = new QStandardItem;
                QStandardItem *pitemsortdate            = new QStandardItem(date.toString("yyyyMMddHHmmss"));

                //listitemsdate << m_treemodel->item(i);
                QList<QStandardItem *> listitemsdate    = m_treemodel->findItems(datestring);
                if (listitemsdate.size()>0 && listitemsdate.at(0)!=Q_NULLPTR)
                {
                    listitemsdate.at(0)->appendRow(QList<QStandardItem*>() << item << pitemsortdate);
                    listitemsdate.at(0)->sortChildren(1);
                }
                wdg_treeview->setIndexWidget(item->index(), DocWidget(listrow));
            }
        }

//!-----not sided docs -> 2 images by row and the last center if number of pics is odd
        while (listdocsboth.size() > 0)
        {
            DocExterne *firstdoc    = Q_NULLPTR;
            DocExterne *scnddoc     = Q_NULLPTR;
            firstdoc = listdocsboth.takeFirst();
            if (listdocsboth.size() >0)
                scnddoc = listdocsboth.takeFirst();
            QStandardItem *item                     = new QStandardItem;
            QStandardItem *pitemsortdate            = new QStandardItem(date.toString("yyyyMMddHHmmss"));
            QList<QStandardItem *> listitemsdate    = m_treemodel->findItems(datestring);
            if (listitemsdate.size()>0)
            {
                listitemsdate.at(0)->appendRow(QList<QStandardItem*>() << item << pitemsortdate);
                listitemsdate.at(0)->sortChildren(1);
            }
            QList<DocExterne*> listdcs = QList<DocExterne*>();
            listdcs << firstdoc;
            if (scnddoc != Q_NULLPTR)
                listdcs << scnddoc;
            wdg_treeview->setIndexWidget(item->index(), DocWidget(listdcs));
        }
    }
    wdg_treeview->setSelectionModel(new QItemSelectionModel(m_treemodel));
    wdg_treeview->expandAll();
    wdg_treeview->setItemsExpandable(false);
}

bool dlg_imageviewer::eventFilter(QObject *obj, QEvent *event)
{
    QResizeEvent *rszevent = dynamic_cast<QResizeEvent*>(event);
    if (rszevent != Q_NULLPTR)
    {
        for (int i = 0; i < m_treemodel->rowCount(); ++i)
        {
            if ( m_treemodel->item(i)->hasChildren())
                for (int j = 0; j< m_treemodel->item(i)->rowCount() ; ++j)
                {
                    QStandardItem* item = m_treemodel->item(i)->child(j,0);
                    if (item != Q_NULLPTR)
                    {
                        QWidget * rwidg = wdg_treeview->indexWidget(item->index());
                        if (rwidg != Q_NULLPTR)
                        {
                            QGridLayout *glay = qobject_cast<QGridLayout*>(rwidg->layout());
                            if (glay != Q_NULLPTR)
                            {
                                for (int k= 0; k < glay->columnCount(); k++)
                                {
                                    QWidget* widg = glay->itemAtPosition(0,k)->widget();
                                    if (widg != Q_NULLPTR)
                                    {
                                        QSize size = sizeforunit();
                                        //! item is jpg pict -> searching an UpLabel and resize the pixmap
                                        UpLabel* lab = dynamic_cast<UpLabel*>(widg);
                                        if (lab)
                                        {
                                            QImage img = lab->image();
                                            QPixmap pix = QPixmap::fromImage(lab->image()).scaled(size, Qt::KeepAspectRatio);
                                            lab->setPixmap(pix);
                                        }
                                        else
                                        {
                                            QVideoWidget *gvdeo = dynamic_cast<QVideoWidget*>(widg);
                                            if (gvdeo)
                                            {
                                                //! item is video? -> searching a QGraphicsView and resize the QGraphicsVideoItem
                                                QSize size  = gvdeo->videoSink()->videoSize();
                                                int w       = sizeforunit().width();
                                                double nx   = size.width();
                                                double ny   = size.height();
                                                int h       = int(w*ny/nx);
                                                gvdeo       ->setFixedSize(w,h);
                                            }
                                            else
                                            {
                                                //! item is pdf? -> searching an UpTableWidget and resize the UpLabel in each cell
                                                UpTableWidget* tbl = dynamic_cast<UpTableWidget*>(widg);
                                                if (tbl)
                                                {
                                                    for (int l=0; l < tbl->rowCount(); l++)
                                                    {
                                                        int maxw(0), maxh(0);
                                                        UpLabel *lbl = qobject_cast<UpLabel*>(tbl->cellWidget(l,0));
                                                        if (lbl != Q_NULLPTR)
                                                        {
                                                            QImage image = tbl->listimg().at(l);
                                                            QPixmap pix = QPixmap::fromImage(image).scaled(size,
                                                                                                           Qt::KeepAspectRatioByExpanding,
                                                                                                           Qt::SmoothTransformation);
                                                            lbl->resize(pix.width(),pix.height());
                                                            lbl->setPixmap(pix);
                                                            int x = pix.width();
                                                            int y = pix.height();
                                                            tbl   ->setColumnWidth(l,x);
                                                            tbl   ->setRowHeight(l,y);
                                                            if (x>maxw) maxw = x;
                                                            if (y>maxh) maxh = y;
                                                        }
                                                        tbl   ->setFixedSize(maxw,maxh);
                                                    }
                                                }
                                                else
                                                //! item is nor jpg, nor pdf, nor video -> removing item and replace by QSPacerItem
                                                {
                                                    QLayoutItem* layit = glay->itemAtPosition(0, k);
                                                    if (layit)
                                                        delete layit;
                                                    glay->addItem(new QSpacerItem(5,5,QSizePolicy::Expanding),0, k);
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QVideoWidget *vwidg = dynamic_cast<QVideoWidget*>(obj);
        if (vwidg)
            Utils::EnChantier(this);
     }
    return QWidget::eventFilter(obj, event);
}

void dlg_imageviewer::ZoomDoc(QWidget *widg)
{
    /*! screen resolution */
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

    QVideoWidget *vwidg = dynamic_cast<QVideoWidget*>(widg);
    if (vwidg)
    {
        int finalh (0), finalw(0);
        double videoratio = vwidg->videoSink()->videoSize().width() / vwidg->videoSink()->videoSize().height();
        /*! if screenration > videoratio  => screenheight is used for max height else screenwidthfor max width */
        if (screenratio > videoratio)
        {
            finalh = int(hscroll * 0.9);
            finalw = int(finalh * videoratio);
        }
        else
        {
            finalw = int(wscroll * 0.9);
            finalh = int(finalw / videoratio);
        }
    }
}
