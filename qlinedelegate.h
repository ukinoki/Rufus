/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QLINEDELEGATE_H
#define QLINEDELEGATE_H

#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QEvent>
#include <uplineedit.h>
#include "upmessagebox.h"

class QLineDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    QLineDelegate(QObject* parent = Q_NULLPTR);
    ~QLineDelegate();

    QWidget*    createEditor    (QWidget* parent,   const QStyleOptionViewItem& option, const QModelIndex &index) const  Q_DECL_OVERRIDE;
    void        setEditorData   (QWidget* editor,   const QModelIndex& index) const Q_DECL_OVERRIDE;
    void        setModelData    (QWidget* editor,   QAbstractItemModel* model,          const QModelIndex& index) const Q_DECL_OVERRIDE;

    //bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;
    int         row, col;

private slots:
    void        SLOT_VideOuPas(QString texte, int row, int col);

signals:
    void        vide(bool check);
    void        edititem(QString texte, int row, int col);

};

#endif // QLINEDELEGATE_H
