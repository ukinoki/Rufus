#ifndef DLG_IMAGEVIEWER_H
#define DLG_IMAGEVIEWER_H

#include <QObject>
#include <QGroupBox>
#include <QScrollArea>
#include <updialog.h>
#include <cls_docsexternes.h>
#include "upcheckbox.h"
#include "updelegate.h"
#include "uptableview.h"
#include "upstandarditemmodel.h"
#include "upstandarditem.h"

class dlg_imageviewer : public UpDialog
{
    Q_OBJECT
public:
    dlg_imageviewer(DocsExternes* Docs, QWidget *parent = Q_NULLPTR);
    ~dlg_imageviewer();
    void                    UpdateTables(DocsExternes *docs);

private:
    DocsExternes            *m_docs             = Q_NULLPTR;
    UpTableView             *wdg_bigtable;
    QStringList             m_listtypedocs      = QStringList();
    QList<QDate>            m_listdates         = QList<QDate>();
    UpStandardItemModel     *m_model            = Q_NULLPTR;
    UpCheckBox              *ODchkBox           = new UpCheckBox(tr("ŒIL DROIT"));
    UpCheckBox              *OGchkBox           = new UpCheckBox(tr("ŒIL GAUCHE"));
    UpCheckBox              *BothchkBox         = new UpCheckBox(tr("LES 2"));
    QScrollArea             *wdg_scrollarea     = new QScrollArea();
    QList<DocExterne*>      getListDocsfromIndex(QModelIndex idx);
    void                    gettoolTipFromCursorPositionInTable();
    UpStandardItem*         itemfromIndex(QModelIndex idx);
    QList<UpStandardItem*>  listitems();
    QList<UpStandardItem*>  listnomastersitems();
    void                    checkItems();
    void                    controlChecks();
};
#endif // DLG_IMAGEVIEWER_H
