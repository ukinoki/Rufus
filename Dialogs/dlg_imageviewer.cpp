#include "dlg_imageviewer.h"
#include <QtWidgets/qheaderview.h>

dlg_imageviewer::dlg_imageviewer(DocsExternes* Docs, QWidget *parent) : UpDialog(PATH_FILE_INI, "PositionsFiches/PositionViewer", parent)
{
    AjouteLayButtons(UpDialog::ButtonOK);
    wdg_bigtable        = new UpTableView();
    wdg_bigtable        ->setPalette(QPalette(Qt::white));
    wdg_bigtable        ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_bigtable        ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_bigtable        ->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable        ->setMouseTracking(true);
    wdg_bigtable->verticalHeader()  ->setVisible(true);

    QHBoxLayout *boxODG_Lay         = new QHBoxLayout();
    int width = 120;
    QString stylechk = "font: bold;";
    ODchkBox                        ->setFixedWidth(width);
    OGchkBox                        ->setFixedWidth(width);
    BothchkBox                      ->setFixedWidth(width);
    ODchkBox                        ->setStyleSheet(stylechk);
    OGchkBox                        ->setStyleSheet(stylechk);
    BothchkBox                      ->setStyleSheet(stylechk);
    boxODG_Lay                      ->addLayout(new QHBoxLayout);
    boxODG_Lay                      ->addWidget(ODchkBox);
    boxODG_Lay                      ->addLayout(new QHBoxLayout);
    boxODG_Lay                      ->addWidget(BothchkBox);
    boxODG_Lay                      ->addLayout(new QHBoxLayout);
    boxODG_Lay                      ->addWidget(OGchkBox);
    boxODG_Lay                      ->addLayout(new QHBoxLayout);

    QVBoxLayout *tablelay           = new QVBoxLayout;
    tablelay                        ->addWidget(wdg_bigtable);
    tablelay                        ->setSizeConstraint(QLayout::SetFixedSize);
    QGridLayout *gridlay            = new QGridLayout;
    gridlay                         ->addWidget(new QWidget(),0,0);
    gridlay                         ->addLayout(tablelay, 1,0);
    gridlay                         ->addLayout(boxODG_Lay, 0,1);
    gridlay                         ->addWidget(wdg_scrollarea,1,1);
    gridlay                         ->setColumnStretch(0,2);
    gridlay                         ->setColumnStretch(1,10);
    gridlay                         ->setRowStretch(0,2);
    gridlay                         ->setRowStretch(1,15);

    dlglayout()->insertLayout(0,gridlay);
    connect(OKButton,           &QPushButton::clicked,              this,   &QDialog::close);

    UpdateTables(Docs);
}

void dlg_imageviewer::UpdateTables(DocsExternes* docs)
{
    if (m_docs != Q_NULLPTR)
        delete m_docs;
    m_docs = new DocsExternes();
    for (auto it = docs->docsexternes()->begin(); it  != docs->docsexternes()->end(); ++it)
    {
        DocExterne *doc = qobject_cast<DocExterne*>(it.value());
        if (doc != Q_NULLPTR)
        {
            if (doc->isMedicalImagery())
            {
                m_docs->docsexternes()->insert(it.key(),it.value());
                if (!m_listdates.contains(doc->date()))
                    m_listdates << doc->date();
                if (!m_listtypedocs.contains(doc->typedoc()))
                    m_listtypedocs << doc->typedoc();
            }
        }
    }
    std::sort(m_listdates.begin(), m_listdates.end());

    //! sort mtypedocs with OCT, RNM, Biometry first
    QStringList listtypes = DocExterne::listtypedocs();
    for (int i= 0; i < DocExterne::listtypedocs().size() ; ++i)
    {
        if (!m_listtypedocs.contains(DocExterne::listtypedocs().at(i)))
            listtypes.removeAll(DocExterne::listtypedocs().at(i));
    }
    m_listtypedocs = listtypes;


    if (m_model != Q_NULLPTR)
        delete m_model;
    m_model             = new UpStandardItemModel();
    m_model             ->setRowCount(m_listdates.size()+1);
    m_model             ->setColumnCount(m_listtypedocs.size()+1);
    m_model             ->setHorizontalHeaderItem(0, new UpStandardItem(""));
    m_model             ->setVerticalHeaderItem(0, new UpStandardItem(""));
    for (int i=1;  i < m_model->columnCount(); i++)
    {
        m_model->setHeaderData(i, Qt::Horizontal,
                               m_listtypedocs.at(i-1),
                               Qt::DisplayRole);
        m_model->setHeaderData(i, Qt::Vertical, Qt::AlignCenter, Qt::TextAlignmentRole);
    }

    UpStandardItem * item = new UpStandardItem();
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setTextAlignment(Qt::AlignCenter);
    item->setCheckState(Qt::Unchecked);
    item->setAutoTristate(true);
    item->setCheckable(true);
    m_model->setItem(0,0, item);

    for (int j=1; j < m_model->columnCount(); j++)
    {
        UpStandardItem * item = new UpStandardItem();
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setTextAlignment(Qt::AlignCenter);
        item->setCheckState(Qt::Unchecked);
        item->setAutoTristate(true);
        item->setCheckable(true);
        m_model->setItem(0,j, item);
    }
    for (int j=1; j < m_model->rowCount(); j++)
    {
        UpStandardItem * item = new UpStandardItem();
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setTextAlignment(Qt::AlignCenter);
        item->setCheckState(Qt::Unchecked);
        item->setAutoTristate(true);
        item->setCheckable(true);
        m_model->setItem(j,0, item);
    }
    for (int i=1;  i < m_listdates.size()+1; i++)
    {
        m_model->setHeaderData(i, Qt::Vertical,
                              m_listdates.at(i-1).toString(tr("dd-MM-yyyy")),
                              Qt::DisplayRole);
        m_model->setHeaderData(i, Qt::Vertical, Qt::AlignCenter, Qt::TextAlignmentRole);
        //! ----------------------------------------------------------------- create UpStandardItem for each cell of the row
        for (int j=1; j < m_model->columnCount(); j++)
        {
            //! ------------------------------------------------------------- search all docs for this date
            QMap<int, DocExterne*> mapdocs = QMap<int, DocExterne*>();
            for (auto it = docs->docsexternes()->begin(); it  != docs->docsexternes()->end(); ++it)
            {
                DocExterne *doc = qobject_cast<DocExterne*>(it.value());
                if (doc != Q_NULLPTR)
                    if (doc->date() == m_listdates.at(i-1))
                        mapdocs.insert(doc->id(), doc);
            }
            //! search all docs for this date & this type
            QList<int> listiddocs = QList<int>();
            for (auto it = mapdocs.begin(); it  != mapdocs.end(); ++it)
            {
                DocExterne *doc = qobject_cast<DocExterne*>(it.value());
                if (doc != Q_NULLPTR)
                    if (doc->typedoc() == m_listtypedocs.at(j-1))
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
                item->setAutoTristate(true);
                item->setCheckable(true);
            }
            m_model->setItem(i,j, item);
        }
    }
    wdg_bigtable                    ->setModel(m_model);
    wdg_bigtable                    ->setItemDelegate(new UpchkDelegate);
    int colwidth = 60;
    wdg_bigtable->setFixedWidth(wdg_bigtable->verticalHeader()->width() + colwidth*m_model->columnCount());
    for (int i=0;  i!=m_model->columnCount(); i++)
    {
        wdg_bigtable->setColumnWidth(i,colwidth);
    }

    connect (wdg_bigtable,      &QTableView::entered,               this,   &dlg_imageviewer::gettoolTipFromCursorPositionInTable);
    connect (m_model,           &QStandardItemModel::itemChanged,   this,   &dlg_imageviewer::checkItems);
}

dlg_imageviewer::~dlg_imageviewer()
{
    delete m_docs;
}

void dlg_imageviewer::gettoolTipFromCursorPositionInTable()
{
    QModelIndex pindx   = wdg_bigtable->indexAt(wdg_bigtable->viewport()->mapFromGlobal(cursor().pos()));
    QString msg;
    QList<DocExterne*> listdocs = getListDocsfromIndex(pindx);
    for (int i = 0; i < listdocs.size(); ++i)
    {
        if (listdocs.at(i) != Q_NULLPTR)
        {
            if (msg != "")
                msg += "\n";
            msg += listdocs.at(i)->soustypedoc() + " " + listdocs.at(i)->date().toString("d MMM yyy");
        }
    }
    QToolTip::showText(cursor().pos(),msg);
}

QList<DocExterne*> dlg_imageviewer::getListDocsfromIndex(QModelIndex idx)
{
    QList<DocExterne*>listdocs = QList<DocExterne*>();
    UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(m_model->itemFromIndex(idx));
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
    return dynamic_cast<UpStandardItem *>(m_model->itemFromIndex(idx));
}

QList<UpStandardItem*> dlg_imageviewer::listitems()
{
    QList<UpStandardItem*> listitems = QList<UpStandardItem*>();
    for (int i = 0; i<m_model->rowCount(); i++)
    {
        for (int j = 0; j<m_model->columnCount(); j++)
        {
            UpStandardItem * item = dynamic_cast<UpStandardItem*>(m_model->item(i,j));
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

QList<UpStandardItem*> dlg_imageviewer::listnomastersitems()
{
    QList<UpStandardItem*> listitems = QList<UpStandardItem*>();
    for (int i = 1; i<m_model->rowCount(); i++)
    {
        for (int j = 1; j<m_model->columnCount(); j++)
        {
            UpStandardItem * item = dynamic_cast<UpStandardItem*>(m_model->item(i,j));
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

void dlg_imageviewer::checkItems()
{
    QList<UpStandardItem*> litems = listitems();
    disconnect (m_model,           &QStandardItemModel::itemChanged,   this,   &dlg_imageviewer::checkItems);
    QModelIndex pindx   = wdg_bigtable->indexAt(wdg_bigtable->viewport()->mapFromGlobal(cursor().pos()));
    UpStandardItem *item = itemfromIndex(pindx);
    if (item == Q_NULLPTR)
    {
        connect (m_model,           &QStandardItemModel::itemChanged,   this,   &dlg_imageviewer::checkItems);
        return;
    }
    Qt::CheckState chkstate;
    if (item->hascheckBox())
        chkstate = item->checkState();
    //qDebug() << litems.size() << "row" << item->row() << "col" << item->column() << Utils::EnumDescription(QMetaEnum::fromType<Qt::CheckState>(), item->checkState());;
    if (item->row() == 0 && item->column() == 0)
    {
        for (int i=0; i < litems.size(); i++)
            litems.at(i)->setCheckState(chkstate);
    }
    else if (item->row() == 0)
    {
        for (int i=0; i < litems.size(); i++)
            if (litems.at(i)->column() == item->column())
                litems.at(i)->setCheckState(chkstate);
    }
    else if (item->column() == 0)
    {
        for (int i=0; i < litems.size(); i++)
            if (litems.at(i)->row() == item->row())
                litems.at(i)->setCheckState(chkstate);
    }
    if (item->row() != 0 || item->column() != 0)
        controlChecks();
    connect (m_model,           &QStandardItemModel::itemChanged,   this,   &dlg_imageviewer::checkItems);
}

void dlg_imageviewer::controlChecks() //! After a check on un uptstandarditem, correct "headers"
{
    QList<UpStandardItem*> litems = listitems();
    // ctrl for check(0,0)
    {
        bool isoneunchecked = false, isonechecked = false;
        for (int i=1; i < m_model->rowCount(); i++)
        {
            for (int j=1; j < m_model->columnCount(); j++)
            {
                UpStandardItem *item= dynamic_cast<UpStandardItem*>(m_model->item(i,j));
                if (item != Q_NULLPTR)
                    if (item->hascheckBox())
                    {
                        if (item->checkState() == Qt::Checked)
                            isonechecked = true;
                        if (item->checkState() == Qt::Unchecked)
                            isoneunchecked = true;
                    }
            }
        }
        bool allchecked = !isoneunchecked;
        bool partiallychecked = isoneunchecked && isonechecked;
        bool nonechecked = !isonechecked;
        if (allchecked)
            m_model->item(0,0)->setCheckState(Qt::Checked);
        else if (partiallychecked)
            m_model->item(0,0)->setCheckState(Qt::PartiallyChecked);
        else if (nonechecked)
            m_model->item(0,0)->setCheckState(Qt::Unchecked);
    }
    // ctrl for rows
    for (int i=1; i < m_model->rowCount(); i++)
    {
        bool isoneunchecked = false, isonechecked = false;
        for (int j=1; j < m_model->columnCount(); j++)
        {
            UpStandardItem *item= dynamic_cast<UpStandardItem*>(m_model->item(i,j));
            if (item != Q_NULLPTR)
                if (item->hascheckBox())
                {
                    if (item->checkState() == Qt::Checked)
                        isonechecked = true;
                    if (item->checkState() == Qt::Unchecked)
                        isoneunchecked = true;
                }
        }
        bool allchecked = !isoneunchecked;
        bool partiallychecked = isoneunchecked && isonechecked;
        bool nonechecked = !isonechecked;
        if (allchecked)
            m_model->item(i,0)->setCheckState(Qt::Checked);
        else if (partiallychecked)
            m_model->item(i,0)->setCheckState(Qt::PartiallyChecked);
        else if (nonechecked)
            m_model->item(i,0)->setCheckState(Qt::Unchecked);
    }
    // ctrl for columns
    for (int i=1; i < m_model->columnCount(); i++)
    {
        bool isoneunchecked = false, isonechecked = false;
        for (int j=1; j < m_model->rowCount(); j++)
        {
            UpStandardItem *item= dynamic_cast<UpStandardItem*>(m_model->item(j,i));
            if (item != Q_NULLPTR)
                if (item->hascheckBox())
                {
                    if (item->checkState() == Qt::Checked)
                        isonechecked = true;
                    if (item->checkState() == Qt::Unchecked)
                        isoneunchecked = true;
                }
        }
        bool allchecked = !isoneunchecked;
        bool partiallychecked = isoneunchecked && isonechecked;
        bool nonechecked = !isonechecked;
        if (allchecked)
            m_model->item(0,i)->setCheckState(Qt::Checked);
        else if (partiallychecked)
            m_model->item(0,i)->setCheckState(Qt::PartiallyChecked);
        else if (nonechecked)
            m_model->item(0,i)->setCheckState(Qt::Unchecked);
    }
}
