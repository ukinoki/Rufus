#ifndef DLG_MULTIIMAGEVIEWER_H
#define DLG_MULTIIMAGEVIEWER_H

#include "dlg_singleimageviewer.h"
#include "updelegate.h"
#include "upmediaplayer.h"
#include "uptreeview.h"
#include "procedures.h"

class dlg_multiimageviewer : public UpDialog
{
    Q_OBJECT
public:
    dlg_multiimageviewer(QList<int> listiddocs, int idcurrentdoc = 0, QWidget *parent = Q_NULLPTR);
    ~dlg_multiimageviewer();
    void                    Actualize();

private:
    QList<int>              m_listiddocs;
    int                     m_idcurrentdoc      = 0;
    UpTableView             *wdg_table;
    QStringList             m_listtypedocs      = QStringList();
    QList<QDate>            m_listdates         = QList<QDate>();
    UpStandardItemModel     *m_Xmodel           = Q_NULLPTR;
    UpStandardItemModel     *m_treemodel        = Q_NULLPTR;
    UpCheckBox              *ODchkBox           = new UpCheckBox(tr("ŒIL DROIT"));
    UpCheckBox              *OGchkBox           = new UpCheckBox(tr("ŒIL GAUCHE"));
    UpCheckBox              *BothchkBox         = new UpCheckBox(tr("LES 2"));
    UpTreeView              *wdg_treeview       = new UpTreeView();
    QList<DocExterne*>      m_listdocsToDisplay;
    QString                 m_formatdate;
    QList<UpStandardItem*>  m_listitems         = QList<UpStandardItem*>();
    QHBoxLayout             *m_hlay             = new QHBoxLayout;
    QMargins                m_marg              = QMargins(5,5,5,5);
    int                     m_spacing           = 5;
    QList<UpStandardItem*>  m_listcheckedItems  = QList<UpStandardItem*>();

    QWidget*                DockWidget(QList<DocExterne *> listdocs);
    bool                    eventFilter(QObject *, QEvent *);


    void                    addItemsToTreeWidget(QList<UpStandardItem *> listupitems);
    void                    removeItemsFromtTreeWidget(QList<UpStandardItem *> listupitems);
    void                    FillXTable();
    QList<DocExterne*>      getListDocsfromIndexInTableView(QModelIndex idx);
    void                    gettoolTipFromCursorPositionInTable();
    UpStandardItem*         itemfromIndex(QModelIndex idx);
    QList<UpStandardItem*>  listcheckabledItems();
    QList<UpStandardItem*>  listcheckedItems();
    void                    checkHorizontalHeader(int idx);
    void                    checkVerticalHeader(int idx);
    void                    controlChecks();
    void                    scrollTreeViewToDocument(DocExterne* doc);
    QList<DocExterne *>     listdocsToDisplay();
    QSize                   sizeforunit() {
//        int w = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent); //width of scrollbar in qApp
        int width = sizefordisplay().width()
                    - wdg_table->width()
            // inside m_hlay inside dlglayout()
                    - m_spacing
                    - m_hlay->contentsMargins().left()
                    - m_hlay->contentsMargins().right()
            // wdg_treeview->indentation()
                    - wdg_treeview->indentation()
            // inside glay (DocWidget)
                    - m_spacing
                    - m_marg.left()
                    - m_marg.right();      //! width for one pic in a row

        return QSize(width/2,width/2);
    }
    void                    ZoomDoc(QWidget *widg);
};
#endif // DLG_MULTIIMAGEVIEWER_H
