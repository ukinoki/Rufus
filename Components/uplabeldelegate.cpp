/* (C) 2018 LAINE SERGE
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

#include <QDebug>
#include "uplabeldelegate.h"

UpLabelDelegate::UpLabelDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget* UpLabelDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex &index) const
{
    if (index.data().canConvert(QMetaType::QString))
    {
        UpLabel *editor = new UpLabel(parent);
        editor->setRow(index.row());
        return editor;
    }
    else
        return QStyledItemDelegate::createEditor(parent, option, index);
}

void UpLabelDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QString value = index.model()->data(index).toString();
    UpLabel *Lbl = dynamic_cast<UpLabel*>(editor);
    if (Lbl)
        Lbl->setText(value);
}

bool UpLabelDelegate::editorEvent(QEvent *event, QAbstractItemModel*model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonRelease)
         emit focusitem(index.row());
    return QAbstractItemDelegate ::editorEvent(event, model, option, index);
}
