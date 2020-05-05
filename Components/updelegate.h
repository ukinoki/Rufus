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

#ifndef UPDELEGATE_H
#define UPDELEGATE_H

#include <QStyledItemDelegate>
#include "uplabel.h"
#include "uplineedit.h"
#include "utils.h"

class UpLabelDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    UpLabelDelegate(QObject* parent = Q_NULLPTR) : QStyledItemDelegate(parent) {}

    QWidget*    createEditor    (QWidget* parent,   const QStyleOptionViewItem& option, const QModelIndex &index) const  Q_DECL_OVERRIDE;
    void        setEditorData   (QWidget* editor,   const QModelIndex& index) const Q_DECL_OVERRIDE;
    bool        editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)  Q_DECL_OVERRIDE;

signals:
    void        focusitem(int a);
};

class UpLineDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    UpLineDelegate(QObject* parent = Q_NULLPTR) : QStyledItemDelegate(parent) {}

    QWidget*    createEditor    (QWidget* parent, const QStyleOptionViewItem&, const QModelIndex &index) const  Q_DECL_OVERRIDE;
    void        setEditorData   (QWidget* editor,   const QModelIndex& index) const Q_DECL_OVERRIDE;
    void        setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const override;
    void        updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool        editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)  Q_DECL_OVERRIDE;
private:
    void        commitEditor();

signals:
    void        focusitem(int a);
    void        editingFinished();
    void        textEdited();
};

class TreeViewDelegate : public QStyledItemDelegate
{
private:
    int m_height;
public:
    TreeViewDelegate(QObject *parent = Q_NULLPTR, int height = -1) : QStyledItemDelegate(parent), m_height(height) {}
    void setHeight(int height) { m_height = height; }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // UPDELEGATE_H
