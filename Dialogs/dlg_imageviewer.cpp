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
    QGridLayout *gridlay        = new QGridLayout;
    gridlay                     ->addWidget(new QWidget(),0,0);
    gridlay                     ->addLayout(tablelay, 1,0);
    gridlay                     ->addLayout(boxODG_Lay, 0,1);
    gridlay                     ->addWidget(wdg_treeview,1,1);
    gridlay                     ->setColumnStretch(0,2);
    gridlay                     ->setColumnStretch(1,10);
    gridlay                     ->setRowStretch(0,2);
    gridlay                     ->setRowStretch(1,15);

    dlglayout()->insertLayout(0,gridlay);
    connect(OKButton,           &QPushButton::clicked,  this,   &QDialog::close);

    UpdateTables(Docs);
}

void dlg_imageviewer::UpdateTables(DocsExternes* docs)
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


    if (m_tblmodel != Q_NULLPTR)
        delete m_tblmodel;
    m_tblmodel             = new UpStandardItemModel();
    m_tblmodel             ->setRowCount(m_listdates.size());
    m_tblmodel             ->setColumnCount(m_listtypedocs.size());
    m_tblmodel             ->setHorizontalHeaderItem(0, new UpStandardItem(""));
    m_tblmodel             ->setVerticalHeaderItem(0, new UpStandardItem(""));
    for (int i=0;  i < m_tblmodel->columnCount(); i++)
    {
        m_tblmodel->setHeaderData(i, Qt::Horizontal,
                               m_listtypedocs.at(i),
                               Qt::DisplayRole);
        m_tblmodel->setHeaderData(i, Qt::Horizontal,
                               Icons::pxunCheck(),
                               Qt::DecorationRole);
        m_tblmodel->setHeaderData(i, Qt::Vertical, Qt::AlignCenter, Qt::TextAlignmentRole);
    }

    for (int i=0;  i < m_listdates.size(); i++)
    {
        m_tblmodel->setHeaderData(i, Qt::Vertical,
                               m_listdates.at(i).toString(m_formatdate),
                               Qt::DisplayRole);
        m_tblmodel->setHeaderData(i, Qt::Vertical,
                               Icons::pxunCheck(),
                               Qt::DecorationRole);
        m_tblmodel->setHeaderData(i, Qt::Vertical, Qt::AlignCenter, Qt::TextAlignmentRole);
        //! ----------------------------------------------------------------- create UpStandardItem for each cell of the row
        for (int j=0; j < m_tblmodel->columnCount(); j++)
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
                        listiddocs << doc->id();
            }
            //! -------------------------------------------------------------- if there is 1 or more docs for this date/type, create a checkbox with list of docs->di()
            UpStandardItem * item = new UpStandardItem();
            if (listiddocs.size() >0)
            {
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setTextAlignment(Qt::AlignCenter);
                item ->setListids(listiddocs);
                item->setCheckState(Qt::Unchecked);
                item->setAutoTristate(listiddocs.size() >1);
                item->setCheckable(true);
            }
            m_tblmodel->setItem(i,j, item);
        }
    }
    wdg_table                    ->setModel(m_tblmodel);
    wdg_table                    ->setItemDelegate(new UpchkDelegate);
    int colwidth = 60;
    wdg_table->setFixedWidth(wdg_table->verticalHeader()->width() + colwidth * m_tblmodel->columnCount());
    for (int i=0;  i != m_tblmodel->columnCount(); i++)
        wdg_table->setColumnWidth(i,colwidth);

    UpStandardItem *item = Q_NULLPTR;
    for (int i= 0; i < m_tblmodel->columnCount(); i++)
    {
        item = dynamic_cast<UpStandardItem*>(m_tblmodel->item(0,i));
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

    connect (wdg_table,                      &QTableView::entered,               this,   &dlg_imageviewer::gettoolTipFromCursorPositionInTable);
    connect (m_tblmodel,                     &QStandardItemModel::itemChanged,   this,   &dlg_imageviewer::controlChecks);
    connect (wdg_table->horizontalHeader(),  &QHeaderView::sectionClicked,       this,   [=] (int idx) {checkHorizontalHeader(idx);} );
    connect (wdg_table->verticalHeader(),    &QHeaderView::sectionClicked,       this,   [=] (int idx) {checkVerticalHeader(idx);} );
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
    UpStandardItem *upitem      = dynamic_cast<UpStandardItem *>(m_tblmodel->itemFromIndex(idx));
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
    return dynamic_cast<UpStandardItem *>(m_tblmodel->itemFromIndex(idx));
}

QList<UpStandardItem*> dlg_imageviewer::listitems()
{
    QList<UpStandardItem*> listitems = QList<UpStandardItem*>();
    for (int i = 0; i<m_tblmodel->rowCount(); i++)
    {
        for (int j = 0; j<m_tblmodel->columnCount(); j++)
        {
            UpStandardItem * item = dynamic_cast<UpStandardItem*>(m_tblmodel->item(i,j));
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

void dlg_imageviewer::checkHorizontalHeader(int idx)
{
    QVariant variant = m_tblmodel->headerData(idx, Qt::Horizontal, Qt::DecorationRole);
    QImage image = variant.value<QImage>();
    bool unchek = image == Icons::pxunCheck().toImage();
    m_tblmodel->setHeaderData(idx, Qt::Horizontal,
                        unchek? Icons::pxCheck() : Icons::pxunCheck(),
                        Qt::DecorationRole);
    QList<UpStandardItem*> litems = listitems();
    for  (int i =0; i<litems.size(); ++i )
        if (litems.at(i)->column() == idx)
                litems.at(i)->setCheckState(unchek? Qt::Checked : Qt::Unchecked);
    controlChecks();
}

void dlg_imageviewer::checkVerticalHeader(int idx)
{
    QVariant variant = m_tblmodel->headerData(idx, Qt::Vertical, Qt::DecorationRole);
    QImage image = variant.value<QImage>();
    bool unchek = image == Icons::pxunCheck().toImage();
    m_tblmodel->setHeaderData(idx, Qt::Vertical,
                               unchek? Icons::pxCheck() :Icons::pxunCheck(),
                               Qt::DecorationRole);
    QList<UpStandardItem*> litems = listitems();
    for  (int i =0; i<litems.size(); ++i )
        if (litems.at(i)->row() == idx)
            litems.at(i)->setCheckState(unchek? Qt::Checked : Qt::Unchecked);
    controlChecks();
}

/*!
 \brief dlg_imageviewer::controlChecks
 After a check on un uptstandarditem, correct headers icons
 */
void dlg_imageviewer::controlChecks() //! After a check on un uptstandarditem, correct headers icons
{
    // ctrl for rows
    for (int i=0; i < m_tblmodel->rowCount(); i++)
    {
        bool isoneunchecked = false, isonechecked = false;
        for (int j=0; j < m_tblmodel->columnCount(); j++)
        {
            UpStandardItem *item= dynamic_cast<UpStandardItem*>(m_tblmodel->item(i,j));
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
            m_tblmodel->setHeaderData(i, Qt::Vertical,
                                   Icons::pxCheck(),
                                   Qt::DecorationRole);
         else if (partiallychecked)
            m_tblmodel->setHeaderData(i, Qt::Vertical,
                                   Icons::pxblockCheck(),
                                   Qt::DecorationRole);
        else if (nonechecked)
             m_tblmodel->setHeaderData(i, Qt::Vertical,
                                    Icons::pxunCheck(),
                                    Qt::DecorationRole);
     }
    // ctrl for columns
    for (int i=0; i < m_tblmodel->columnCount(); i++)
    {
        bool isoneunchecked = false, isonechecked = false;
        for (int j=0; j < m_tblmodel->rowCount(); j++)
        {
            UpStandardItem *item= dynamic_cast<UpStandardItem*>(m_tblmodel->item(j,i));
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
            m_tblmodel->setHeaderData(i, Qt::Horizontal,
                                   Icons::pxCheck(),
                                   Qt::DecorationRole);
        else if (partiallychecked)
            m_tblmodel->setHeaderData(i, Qt::Horizontal,
                                   Icons::pxblockCheck(),
                                   Qt::DecorationRole);
        else if (nonechecked)
            m_tblmodel->setHeaderData(i, Qt::Horizontal,
                                   Icons::pxunCheck(),
                                   Qt::DecorationRole);
    }
    m_listdocsToDisplay = listdocsToDisplay();
    checkEyes();
    fillTreeWidg(m_listdocsToDisplay);
}

QList<DocExterne*> dlg_imageviewer::listdocsToDisplay()
{
    QList<DocExterne*> listdocs = QList<DocExterne*>();
        QList<UpStandardItem*> litems = listitems();
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

void dlg_imageviewer::fillTreeWidg(QList<DocExterne*> listdocs)
{
    QList<QDate> listdates =  QList<QDate>();
    for (int i=0;  i < m_tblmodel->rowCount(); i++)
    {
        QVariant variant = m_tblmodel->headerData(i, Qt::Vertical, Qt::DecorationRole);
        QImage image = variant.value<QImage>();
        if (image == Icons::pxCheck().toImage() || image == Icons::pxblockCheck().toImage())
            listdates << QDate::fromString(m_tblmodel->headerData(i, Qt::Vertical, Qt::DisplayRole).value<QString>(), m_formatdate);
    }
    if (m_treemodel != Q_NULLPTR)
        delete m_treemodel;
    m_treemodel             = new UpStandardItemModel();
    QItemSelectionModel *m = wdg_treeview->selectionModel(); // il faut détruire le selectionModel pour éviter des bugs d'affichage quand on réinitialise le modèle
    wdg_treeview->setModel(m_treemodel);
    delete m;

    QStandardItem * rootNodeDate = m_treemodel->invisibleRootItem();
    for (int i=0; i<listdates.size(); ++i)
    {
        QString datestring = listdates.at(i).toString(m_formatdate);
        QStandardItem *dateitem    = new QStandardItem(datestring);
        dateitem    ->setForeground(QBrush(QColor(Qt::red)));
        dateitem    ->setEditable(false);
        dateitem    ->setIcon(Icons::icDate());
        rootNodeDate->appendRow(dateitem);
    }
    foreach (DocExterne *doc, listdocs)
    {
        QStandardItem *item = new QStandardItem;
        QWidget *widg = DocWidget(doc);
        QStandardItem *pitemtridated       = new QStandardItem(doc->datetimeimpression().toString("yyyyMMddHHmmss"));
        QString date = doc->date().toString(m_formatdate);
        QList<QStandardItem *> listitemsdate = m_treemodel->findItems(date);
        if (listitemsdate.size()>0)
        {
            listitemsdate.at(0)->appendRow(QList<QStandardItem*>() << item << pitemtridated);
            listitemsdate.at(0)->sortChildren(1);
        }
        wdg_treeview->setIndexWidget(item->index(), widg);
    }

    wdg_treeview->setSelectionModel(new QItemSelectionModel(m_treemodel));
    wdg_treeview->expandAll();
}

QWidget* dlg_imageviewer::DocWidget(DocExterne *doc)
{
    Procedures::I()->CalcImageDocument(doc);
    QList<QPixmap> listpixmp;
    QSize size = QSize(360,240);
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
        QGraphicsScene *obj_graphicscene    = new QGraphicsScene();
        QGraphicsVideoItem *obj_videoitem   = new QGraphicsVideoItem;
        obj_graphicscene                    ->addItem(obj_videoitem);
        QMediaPlayer *medplay_player        = new QMediaPlayer;
        medplay_player                      ->setSource( QUrl::fromLocalFile(filename));
        medplay_player                      ->setVideoOutput(obj_videoitem);

        QGraphicsView *wdg_graphview       = new QGraphicsView(obj_graphicscene);                // utilisé pour afficher les jpg et les video
        wdg_graphview       ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        wdg_graphview       ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        wdg_graphview       ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        obj_videoitem       ->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
        obj_videoitem       ->setSize(size);
        int x               = obj_videoitem->size().width();
        int y               = obj_videoitem->size().height();
        obj_graphicscene    ->setSceneRect(0,0,x,y);
        wdg_graphview       ->resize(x,y);
        medplay_player->play();

        QWidget *wdg = new QWidget;
        QHBoxLayout * lay = new QHBoxLayout();
        lay->addWidget(wdg_graphview);
        lay->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
        wdg->setLayout(lay);
        return wdg;
    }
    else if (doc->imageformat() == JPG)     // le document est un JPG
    {
        QImage image;
        if (!image.loadFromData(doc->imageblob()))
        {
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            return Q_NULLPTR;
        }
        QGraphicsScene *obj_graphicscene    = new QGraphicsScene();
        QGraphicsView *wdg_graphview       = new QGraphicsView(obj_graphicscene);                      // utilisé pour afficher les jpg et les video
        wdg_graphview       ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        wdg_graphview       ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        wdg_graphview       ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        QPixmap pix         = QPixmap::fromImage(image).scaled(size,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
        int x = pix.width();
        int y = pix.height();
        obj_graphicscene        ->addPixmap(pix);
        obj_graphicscene        ->setSceneRect(0,0,x,y);
        wdg_graphview           ->resize(x,y);
        QWidget *wdg = new QWidget;
        QHBoxLayout * lay = new QHBoxLayout();
        lay->addWidget(wdg_graphview);
        lay->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
        wdg->setLayout(lay);
        return wdg;
    }
    else if (doc->imageformat() == PDF)     // le document est un pdf (document d'imagerie ou document écrit transformé en pdf par CalcImage)
    {
        QList<QImage> listimg;
        listimg = doc->pagelist();

        if (listimg.size())
        {
            UpTableWidget *tblwdg = new UpTableWidget();                                  // utilisé pour afficher les pdf qui ont parfois plusieurs pages
            tblwdg->horizontalHeader() ->setVisible(false);
            tblwdg->verticalHeader()   ->setVisible(false);
            tblwdg      ->setFocusPolicy(Qt::NoFocus);
            tblwdg      ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
            tblwdg      ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            tblwdg      ->setColumnCount(1);
            tblwdg      ->setRowCount(listimg.size());
            tblwdg      ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            for (int i=0; i<listimg.size();++i)
            {
                QImage image = listimg.at(i);
                QPixmap pix     = QPixmap();
                listpixmp << pix;
                pix     = QPixmap::fromImage(image).scaled(size,
                                                       Qt::KeepAspectRatioByExpanding,
                                                       Qt::SmoothTransformation);
                UpLabel *lab            = new UpLabel();
                lab                     ->resize(pix.width(),pix.height());
                lab                     ->setPixmap(pix);
                int x = pix.width();
                int y = pix.height();
                tblwdg   ->setColumnWidth(i,x);
                tblwdg   ->setRowHeight(i,y);
                tblwdg   ->setFixedSize(x,y);
                tblwdg   ->setCellWidget(i,0,lab);
                return tblwdg;
            }
        }
    }
    return Q_NULLPTR;
}
