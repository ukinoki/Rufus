#ifndef DLG_IMAGEVIEWER_H
#define DLG_IMAGEVIEWER_H

#include <QObject>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoWidget>
#include "updialog.h"
#include "cls_docsexternes.h"
#include "upcheckbox.h"
#include "updelegate.h"
#include "uptableview.h"
#include "uptreeview.h"
#include "upstandarditemmodel.h"
#include "upstandarditem.h"
#include "procedures.h"

class dlg_imageviewer : public UpDialog
{
    Q_OBJECT
public:
    dlg_imageviewer(DocsExternes* Docs, QWidget *parent = Q_NULLPTR);
    ~dlg_imageviewer();
    void                    FillXTable(DocsExternes *docs);

private:
    DocsExternes            *m_docs             = Q_NULLPTR;
    UpTableView             *wdg_table;
    QStringList             m_listtypedocs      = QStringList();
    QList<QDate>            m_listdates         = QList<QDate>();
    UpStandardItemModel     *m_Xmodel         = Q_NULLPTR;
    UpStandardItemModel     *m_treemodel        = Q_NULLPTR;
    UpCheckBox              *ODchkBox           = new UpCheckBox(tr("ŒIL DROIT"));
    UpCheckBox              *OGchkBox           = new UpCheckBox(tr("ŒIL GAUCHE"));
    UpCheckBox              *BothchkBox         = new UpCheckBox(tr("LES 2"));
    UpTreeView              *wdg_treeview       = new UpTreeView();
    QList<DocExterne*>      m_listdocsToDisplay;
    QString                 m_formatdate;
    QList<UpStandardItem*>  m_listitems         = QList<UpStandardItem*>();
    QGridLayout             *m_gridlay          = new QGridLayout;
    QMargins                m_marg              = QMargins(5,5,5,5);
    int                     m_spacing           = 5;
    QList<UpStandardItem*>  m_listcheckedItems  = QList<UpStandardItem*>();

    QWidget*                DocWidget(QList<DocExterne *> listdocs);
    bool                    eventFilter(QObject *, QEvent *);


    void                    addItemsToTreeWidget(QList<UpStandardItem *> listupitems);
    void                    removeItemsFromtTreeWidget(QList<UpStandardItem *> listupitems);
    QList<DocExterne*>      getListDocsfromIndex(QModelIndex idx);
    void                    gettoolTipFromCursorPositionInTable();
    UpStandardItem*         itemfromIndex(QModelIndex idx);
    QList<UpStandardItem*>  listcheckabledItems();
    QList<UpStandardItem*>  listcheckedItems();
    void                    checkHorizontalHeader(int idx);
    void                    checkVerticalHeader(int idx);
    void                    checkEyes();
    void                    controlChecks();
    QList<DocExterne *>     listdocsToDisplay();
    QSize                   sizeforunit() {
        int w = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent); //width of scrollbar in qApp
        int width = geometry().width() - wdg_table->width()
                    - m_gridlay->spacing()
                    - m_gridlay->contentsMargins().left()
                    - m_gridlay->contentsMargins().right()
                    - dlglayout()->contentsMargins().left()
                    - dlglayout()->contentsMargins().right()
                    - dlglayout()->spacing()
                    - wdg_treeview->indentation()
                    - w
                    - m_spacing
                    - m_marg.left()
                    - m_marg.right();      //!estimated width for one pic in a row

        return QSize(width/2,width/2);
    }
    void                    ZoomDoc(QWidget *widg);
};
#endif // DLG_IMAGEVIEWER_H
