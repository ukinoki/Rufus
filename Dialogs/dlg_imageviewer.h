#ifndef DLG_IMAGEVIEWER_H
#define DLG_IMAGEVIEWER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QGroupBox>
#include <QMediaPlayer>
#include <QTreeWidget>
#include <updialog.h>
#include <cls_docsexternes.h>
#include "upcheckbox.h"
#include "updelegate.h"
#include "uptableview.h"
#include "upstandarditemmodel.h"
#include "upstandarditem.h"
#include "procedures.h"

class dlg_imageviewer : public UpDialog
{
    Q_OBJECT
public:
    dlg_imageviewer(DocsExternes* Docs, QWidget *parent = Q_NULLPTR);
    ~dlg_imageviewer();
    void                    UpdateTables(DocsExternes *docs);

private:
    DocsExternes            *m_docs             = Q_NULLPTR;
    UpTableView             *wdg_table;
    QStringList             m_listtypedocs      = QStringList();
    QList<QDate>            m_listdates         = QList<QDate>();
    UpStandardItemModel     *m_tblmodel         = Q_NULLPTR;
    UpStandardItemModel     *m_treemodel        = Q_NULLPTR;
    UpCheckBox              *ODchkBox           = new UpCheckBox(tr("ŒIL DROIT"));
    UpCheckBox              *OGchkBox           = new UpCheckBox(tr("ŒIL GAUCHE"));
    UpCheckBox              *BothchkBox         = new UpCheckBox(tr("LES 2"));
    QTreeView               *wdg_treeview       = new QTreeView();
    QList<DocExterne*>      m_listdocsToDisplay;
    QString                 m_formatdate;
    QWidget*                DocWidget(DocExterne *doc);


    QList<DocExterne*>      getListDocsfromIndex(QModelIndex idx);
    void                    gettoolTipFromCursorPositionInTable();
    UpStandardItem*         itemfromIndex(QModelIndex idx);
    QList<UpStandardItem*>  listitems();
    void                    checkHorizontalHeader(int idx);
    void                    checkVerticalHeader(int idx);
    void                    checkEyes();
    void                    controlChecks();
    QList<DocExterne *> listdocsToDisplay();
    void                    fillTreeWidg(QList<DocExterne*> listdocs);
};
#endif // DLG_IMAGEVIEWER_H
