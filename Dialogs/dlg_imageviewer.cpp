#include "dlg_imageviewer.h"
#include <QtWidgets/qheaderview.h>

dlg_imageviewer::dlg_imageviewer(QList<int> listiddocs, int idcurrentdoc, QWidget *parent) : UpDialog(Nom_fiche_Viewer, parent)
{
    m_idcurrentdoc              = idcurrentdoc;
    m_listiddocs                = listiddocs;
    m_formatdate                = tr("dd-MM-yyyy");
    AjouteLayButtons(UpDialog::ButtonOK);
    wdg_table                   = new UpTableView();
    wdg_table                   ->setPalette(QPalette(Qt::white));
    wdg_table                   ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_table                   ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_table                   ->setFocusPolicy(Qt::StrongFocus);
    wdg_table                   ->setMouseTracking(true);
    wdg_table->verticalHeader() ->setVisible(true);

    m_hlay                      ->addWidget(wdg_table);
    m_hlay                      ->addWidget(wdg_treeview);
    m_hlay                      ->setStretch(2,10);

    wdg_table                   ->setSelectionMode(QAbstractItemView::NoSelection);
    wdg_table                   ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    wdg_treeview                ->setSelectionMode(QAbstractItemView::NoSelection);
    wdg_treeview                ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    wdg_treeview                ->setHeaderHidden(true);
    wdg_treeview                ->setFocusPolicy(Qt::NoFocus); //! si on ne met pas ça, le UpVideoWidget disparait quand on clique dessus

    dlglayout()->insertLayout(0,m_hlay);
    connect(OKButton,           &QPushButton::clicked,  this,   &QDialog::close);

    FillXTable();

    UpStandardItem *item = Q_NULLPTR;
    if (m_idcurrentdoc == 0)
    {
        //! check XTable first row
        for (int i= 0; i < m_Xmodel->columnCount(); i++)
        {
            item = dynamic_cast<UpStandardItem*>(m_Xmodel->item(0,i));
            if (item != Q_NULLPTR)
                if (item->hascheckBox())
                {
                    QModelIndex idx = item->index();                                        //! index of first item
                    wdg_table->scrollTo(idx, QAbstractItemView::PositionAtTop);
                    wdg_table->setCurrentIndex(idx);
                    checkVerticalHeader(item->row());
                    break;
                }
        }
        m_listcheckedItems = listcheckedItems();
        if (item)
            if (item->listids().size())
                scrollTreeViewToDocument(Datas::I()->docsexternes->getById(item->listids().at(0)));
    }
    else
    {
        DocExterne *doc = Datas::I()->docsexternes->getById(m_idcurrentdoc);
        int row(0), col(0);
        if (doc)
        {
            QDate datedoc = doc->date();
            QString typedoc = doc->typedoc();
            for (int i= 0; i < m_Xmodel->rowCount(); i++)
            {
                 if (m_Xmodel->headerData(i, Qt::Vertical,Qt::DisplayRole).toString() == datedoc.toString(m_formatdate))
                {
                    row = i;
                    i = m_Xmodel->rowCount();
                    for (int j= 0; j < m_Xmodel->columnCount(); j++)
                    {
                        if (m_Xmodel->headerData(j, Qt::Horizontal,Qt::DisplayRole).toString() == typedoc)
                        {
                            col = j;
                            break;
                        }
                    }
                }
            }
        }
        UpStandardItem *item = dynamic_cast<UpStandardItem*>(m_Xmodel->item(row, col));
        item            ->setCheckState(Qt::Checked);
        item            ->setBeforeCheckState(true);
        QModelIndex idx = item->index();
        wdg_table       ->scrollTo(idx);
        wdg_table       ->setCurrentIndex(idx);
        addItemsToTreeWidget(QList<UpStandardItem*>() << item);
        scrollTreeViewToDocument(doc);
    }

    installEventFilter(this);

    connect (wdg_table,                     &QTableView::entered,               this,   &dlg_imageviewer::gettoolTipFromCursorPositionInTable);
    connect (wdg_table,                     &QTableView::clicked,               this,   [=] (QModelIndex idx) {
        UpStandardItem *upitem      = dynamic_cast<UpStandardItem *>(m_Xmodel->itemFromIndex(idx));
        if (upitem == Q_NULLPTR)
            return;
        if ((upitem->checkState() == Qt::Checked  || upitem->checkState() == Qt::PartiallyChecked) && !upitem->beforeCheckState())
        {
            upitem->setBeforeCheckState(true);
            addItemsToTreeWidget(QList<UpStandardItem *>() << upitem);
            if(upitem->listids().size())
            {
                DocExterne *doc = Datas::I()->docsexternes->getById(upitem->listids().at(0));
                if (doc)
                    scrollTreeViewToDocument(doc);
            }
        }
        else if (upitem->checkState() == Qt::Unchecked && upitem->beforeCheckState())
        {
            upitem->setBeforeCheckState(false);
            removeItemsFromtTreeWidget(QList<UpStandardItem *>() << upitem);
        }
    });
    connect (wdg_table->horizontalHeader(), &QHeaderView::sectionClicked,       this,   [=] (int idx) {checkHorizontalHeader(idx);} );
    connect (wdg_table->verticalHeader(),   &QHeaderView::sectionClicked,       this,   [=] (int idx) {checkVerticalHeader(idx);} );
    //connect (qApp, &QApplication::focusChanged, this, [&](QWidget *old, QWidget *now){        qDebug() << old->metaObject()->className() << now->metaObject()->className(); });
}

void dlg_imageviewer::FillXTable()
{
    for (int it = 0; it < m_listiddocs.size(); ++it)
    {
        DocExterne *doc = Datas::I()->docsexternes->getById(m_listiddocs.at(it));
        if (doc != Q_NULLPTR)
        {
            if (!m_listdates.contains(doc->date()))
                m_listdates     << doc->date();
            if (!m_listtypedocs.contains(doc->typedoc()))
                m_listtypedocs  << doc->typedoc();
        }
    }

    std::sort(m_listdates.begin(), m_listdates.end(), std::greater<>());
    /*! +++++ std::reverse gives errors */

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
            for (int it = 0; it < m_listiddocs.size(); ++it)
            {
                DocExterne *doc = Datas::I()->docsexternes->getById(m_listiddocs.at(it));
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
                item->setBeforeCheckState(false);
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
            DocExterne *doc = Datas::I()->docsexternes->getById(upitem->listids().at(i));
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
 * \brief dlg_imageviewer::listcheckedItems
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
    QVariant variant                = m_Xmodel->headerData(idx, Qt::Horizontal, Qt::DecorationRole);
    QImage image                    = variant.value<QImage>();
    QList<UpStandardItem*> litems   = listcheckabledItems();
    bool check                      = image == Icons::pxunCheck().toImage();
    if (!check)
    {
        QList<UpStandardItem*> litemstouncheck = QList<UpStandardItem*>();
        for  (int i =0; i<litems.size(); ++i )
            if (litems.at(i)->column() == idx)
                if (litems.at(i)->checkState() == Qt::Checked)
                {
                    litemstouncheck << litems.at(i);
                    litems.at(i)    ->setCheckState(Qt::Unchecked);
                    litems.at(i)    ->setBeforeCheckState(false);
                }
        m_Xmodel->setHeaderData(idx, Qt::Horizontal, Icons::pxunCheck(), Qt::DecorationRole);
        removeItemsFromtTreeWidget(litemstouncheck);
    }
    else
    {
        QList<UpStandardItem*> litemstocheck = QList<UpStandardItem*>();
        for  (int i =0; i<litems.size(); ++i )
            if (litems.at(i)->column() == idx)
                if (litems.at(i)->checkState() != Qt::Checked)
                {
                    litemstocheck   << litems.at(i);
                    litems.at(i)    ->setCheckState(Qt::Checked);
                    litems.at(i)    ->setBeforeCheckState(true);
               }
        m_Xmodel->setHeaderData(idx, Qt::Horizontal, Icons::pxCheck(), Qt::DecorationRole);
        addItemsToTreeWidget(litemstocheck);
    }
}

void dlg_imageviewer::checkVerticalHeader(int idx)
{
    QVariant variant                    = m_Xmodel->headerData(idx, Qt::Vertical, Qt::DecorationRole);
    QImage image                        = variant.value<QImage>();
    bool check                          = image == Icons::pxunCheck().toImage();
    QList<UpStandardItem*> litems       = listcheckabledItems();
    if (!check)
    {
        QList<UpStandardItem*> litemstouncheck = QList<UpStandardItem*>();
        for  (int i =0; i<litems.size(); ++i )
            if (litems.at(i)->row() == idx)
                if (litems.at(i)->checkState() == Qt::Checked)
                {
                    litemstouncheck << litems.at(i);
                    litems.at(i)    ->setCheckState(Qt::Unchecked);
                    litems.at(i)    ->setBeforeCheckState(false);
                }
        m_Xmodel->setHeaderData(idx, Qt::Vertical, Icons::pxunCheck(), Qt::DecorationRole);
        removeItemsFromtTreeWidget(litemstouncheck);
    }
    else
    {
        QList<UpStandardItem*> litemstocheck = QList<UpStandardItem*>();
        for  (int i =0; i<litems.size(); ++i )
            if (litems.at(i)->row() == idx)
                if (litems.at(i)->checkState() != Qt::Checked)
                {
                    litemstocheck   << litems.at(i);
                    litems.at(i)    ->setCheckState(Qt::Checked);
                    litems.at(i)    ->setBeforeCheckState(true);
                }
        m_Xmodel->setHeaderData(idx, Qt::Vertical, Icons::pxCheck(), Qt::DecorationRole);
        addItemsToTreeWidget(litemstocheck);
    }
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
}

void dlg_imageviewer::removeItemsFromtTreeWidget(QList<UpStandardItem *> listupitems)
{
    controlChecks();
    foreach (UpStandardItem* item, listupitems)
    {
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
                        if (widg)
                        {
                            if (widg->property(M_DATE).toDate() == item->data(Qt::UserRole).toDate() && widg->property(M_TYPE).toString() == item->dataType())
                            {
                                QList<QStandardItem*> lisdelit = dateitem->takeRow(j);
                                foreach (QStandardItem *itm, lisdelit)
                                    if (itm != Q_NULLPTR) delete itm;
                                j--;
                            }
                        }
                        else
                        {
                            dateitem->takeRow(j);
                            j--;
                        }
                    }
                    else
                    {
                        dateitem->takeRow(j);
                        j--;
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
                DocExterne *doc = Datas::I()->docsexternes->getById(ids.at(j));
                if (doc != Q_NULLPTR)
                    if (!listdocs.contains(doc))
                        listdocs << doc;
            }
        }
    }
    return listdocs;
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
    QHBoxLayout *glay   = new QHBoxLayout();
    glay                ->setContentsMargins(m_marg);
    glay                ->setSpacing(m_spacing);
    QDate date = QDate();
    QString type = QString();
    if (listdocs.size() == 1)
        glay->addItem(new QSpacerItem(5,5, QSizePolicy::Expanding));
    for (int it = 0; it < listdocs.size() ; ++it)
    {
        DocExterne * doc = listdocs.at(it);
        if (doc == Q_NULLPTR)
            glay->addItem(new QSpacerItem(5,5,QSizePolicy::Expanding));
        else
        {
            if (date == QDate())
                date = doc->date();
            if (type == QString())
                type = doc->typedoc();
            Procedures::I()->CalcImageDocument(doc);
            QList<QPixmap> listpixmp;
            QWidget *glaywidg = Q_NULLPTR;
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
                UpVideoWidget *wdg_video            = new UpVideoWidget(filename);
                QMediaPlayer *player                = new QMediaPlayer;
                wdg_video                           ->setPlayer(player);
                player                              ->setSource( QUrl::fromLocalFile(filename));
                player                              ->setVideoOutput(wdg_video);
                player                              ->play();
                QSize size                          = wdg_video->videoSink()->videoSize();
                int w                               = sizeforunit().width();
                double nx                           = size.width();
                double ny                           = size.height();
                int h                               = int(w*ny/nx);
                wdg_video                           ->setFixedSize(w,h);
                wdg_video                           ->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(wdg_video, &QWidget::customContextMenuRequested, this, [=] { Utils::EnChantier(this);});
                wdg_video                           ->installEventFilter(this);
                glaywidg = wdg_video;
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
                lab             ->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(lab, &QWidget::customContextMenuRequested, this, [=] { Utils::EnChantier(this);});
                lab             ->installEventFilter(this);
                glaywidg = lab;
            }
            else if (doc->imageformat() == PDF)     // doc is pdf
            {
                QList<QImage> listimg;
                listimg = doc->pagelist();

                if (listimg.size())
                {
                    UpTableWidget *tblwdg = new UpTableWidget();                                  // utilisé pour afficher les pdf qui ont parfois plusieurs pages
                    tblwdg          ->setListimg(listimg);
                    tblwdg->horizontalHeader() ->setVisible(false);
                    tblwdg->verticalHeader()   ->setVisible(false);
                    tblwdg          ->setFocusPolicy(Qt::NoFocus);
                    tblwdg          ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
                    tblwdg          ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    tblwdg          ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    tblwdg          ->setColumnCount(1);
                    tblwdg          ->setSelectionMode(QAbstractItemView::NoSelection);
                    QSize szfinal   = tblwdg->calcSizeForDisplay(sizeforunit());
                    QList<UpLabel*> pdflabels = tblwdg->labels();
                    foreach (UpLabel* lab, pdflabels)
                    {
                        lab             ->setContextMenuPolicy(Qt::CustomContextMenu);
                        connect(lab, &QWidget::customContextMenuRequested, this, [=] { Utils::EnChantier(this);});
                        lab             ->installEventFilter(this);
                    }

                    double w        = szfinal.width();
                    double h        = szfinal.height();
                    tblwdg          ->setFixedSize(szfinal);
                    glaywidg        = tblwdg;
                }
            }
            glay    ->addWidget(glaywidg);
        }
    }
    widg = new QWidget();
    if (listdocs.size() == 1)
        glay->addItem(new QSpacerItem(5,5, QSizePolicy::Expanding));
    widg->setLayout(glay);

    //! I know, the following is ugly
    QString listid = "";
    if (listdocs.at(0) != Q_NULLPTR)
        listid += QString::number(listdocs.at(0)->id());
    if (listdocs.size() > 1)
        if (listdocs.at(1) != Q_NULLPTR)
        {
            if (listid != "")
                listid += M_DELIMITER;
            listid += QString::number(listdocs.at(1)->id());
        }
    widg->setProperty(M_DATE, date);
    widg->setProperty(M_TYPE, type);
    widg->setProperty(M_LISTID, listid);
    return widg;
}

void dlg_imageviewer::scrollTreeViewToDocument(DocExterne* doc)
{
    if (doc == Q_NULLPTR)
        return;
    QDate datedoc = doc->date();
    QString typedoc = doc->typedoc();
    QList<QStandardItem*> litems = m_treemodel->findItems(datedoc.toString(m_formatdate));
    if (litems.size() == 0)
        return;
    QStandardItem *citem = litems.at(0);
    if (citem)
        for (int r=0; r < citem->rowCount(); r++ )
        {
            QStandardItem* it = citem->child(r);
            if (it)
            {
                QWidget * widg = dynamic_cast<QWidget*>(wdg_treeview->indexWidget(it->index()));
                if (widg)
                {
                    QString listid = widg->property(M_LISTID).toString();
                    if (listid != "")
                    {
                        int fid = listid.split(M_DELIMITER).at(0).toInt();
                        int sid(0);
                        if (listid.split(M_DELIMITER).size()>1)
                            sid = listid.split(M_DELIMITER).at(1).toInt();
                        if (doc->id() == fid || doc->id() == sid)
                        {
                            wdg_treeview->scrollTo(it->index());
                            break;
                        }
                    }
                }
            }
        }
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
    controlChecks();
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
            DocExterne *doc = Datas::I()->docsexternes->getById(ids.at(j));
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
                    QStandardItem* item = m_treemodel->item(i)->child(j,0);                                                     //! -> search for childitem
                    if (item != Q_NULLPTR)
                    {
                        QWidget * rwidg = wdg_treeview->indexWidget(item->index());                                             //! -> search if childitem is DocWidget
                        if (rwidg != Q_NULLPTR)
                        {
                            QHBoxLayout *glay = qobject_cast<QHBoxLayout*>(rwidg->layout());                                    //! -> search for layout of DocWidget
                            if (glay != Q_NULLPTR)
                            {
                                for (int k= 0; k < glay->count(); k++)
                                {
                                    QLayoutItem* layit = glay->itemAt(k);                                             //! -> search for QLayoutitem of DocWidget
                                    if (layit)
                                    {
                                        QWidget* widg = layit->widget();                                                        //! -> search for QWidget of QLayoutItem of DocWidget
                                        if (widg != Q_NULLPTR)
                                        {
                                            QSize size = sizeforunit();
                                            UpLabel* lab = dynamic_cast<UpLabel*>(widg);                                        //! widget is UpLabel => jpg pict -> resize the pixmap
                                            if (lab)
                                            {
                                                QImage img = lab->image();
                                                QPixmap pix = QPixmap::fromImage(img).scaled(size, Qt::KeepAspectRatio);
                                                lab->setPixmap(pix);
                                            }
                                            else
                                            {
                                                UpVideoWidget *gvdeo = dynamic_cast<UpVideoWidget*>(widg);                        //! widget is UpVideoWidget => video -> resize the UpVideoWidget
                                                if (gvdeo)
                                                {
                                                    QSize size  = gvdeo->videoSink()->videoSize();
                                                    int w       = sizeforunit().width();
                                                    double nx   = size.width();
                                                    double ny   = size.height();
                                                    int h       = int(w*ny/nx);
                                                    gvdeo       ->setFixedSize(w,h);
                                                }
                                                else
                                                {
                                                    UpTableWidget* tbl = dynamic_cast<UpTableWidget*>(widg);                    //! widget is UpTableWidget=> is pdf -> resize each cell in UptableWidget
                                                    if (tbl)
                                                        tbl->resizetofit(sizeforunit());
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
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        if (dynamic_cast<UpLabel*>(obj) != Q_NULLPTR)
            ZoomDoc(dynamic_cast<QWidget*>(obj));
        else if (dynamic_cast<UpVideoWidget*>(obj) != Q_NULLPTR)
            ZoomDoc(dynamic_cast<QWidget*>(obj));
    }
    return QWidget::eventFilter(obj, event);
}

void dlg_imageviewer::ZoomDoc(QWidget *widg)
{
    ImageZoom *imgzoom = new ImageZoom(widg, this);
    imgzoom->exec();
    delete imgzoom;
}
