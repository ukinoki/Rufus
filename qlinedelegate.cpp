/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "qlinedelegate.h"


QLineDelegate::QLineDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QLineDelegate::~QLineDelegate()
{

}
QWidget* QLineDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex &index) const
{
    if (index.data().canConvert(QMetaType::QString))
    {
        UpLineEdit *editor = new UpLineEdit(parent);
        editor->setRowTable(index.row());
        editor->setColumnTable(index.column());
        connect(editor, SIGNAL(upTextEdited(QString,int,int)), this, SLOT (SLOT_VideOuPas(QString,int,int)));
        return editor;
    }
    else
        return QItemDelegate::createEditor(parent, option, index);
}

void QLineDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QRegExp rx("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9 -]+");
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *Line = dynamic_cast<QLineEdit*>(editor);
    if (Line)
    {
        Line->setText(value);
        Line->setValidator(new QRegExpValidator(rx));
    }
}

void QLineDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QLineEdit *Line = static_cast<QLineEdit*>(editor);
    QString value = Line->text();
    model->setData(index, value, Qt::EditRole);
}

//bool QLineDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
//{
//    return false;
//}

void QLineDelegate::SLOT_VideOuPas(QString texte, int row, int col)
{
    emit edititem(texte,row,col);
}
