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

#include "uplabelitem.h"

UpLabelItem::UpLabelItem(QWidget *parent, QString txt) : QLabel(txt, parent)  //je ne sais plus pourquoi j'ai mis le parent avant le texte mais il y une raison à un moment donné du code...
{
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
}

UpLabelItem::UpLabelItem(Item *item, QString txt, QWidget *parent) : QLabel(txt, parent)  //je ne sais plus pourquoi j'ai mis le parent avant le texte mais il y une raison à un moment donné du code...
{
    m_item = item;
    m_item->setParent(this);
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
}

UpLabelItem::~UpLabelItem()
{
}

void UpLabelItem::AfficheToolTip()
{
    if (m_tooltipmsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),m_tooltipmsg);
}

bool UpLabelItem::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        emit enter(iD());
        AfficheToolTip();
        return true;
    }
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        emit dblclick(iD());
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease)
    {
        emit clicked(iD());
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void UpLabelItem::setdatas(QMap<QString, QVariant> data)
{
    m_datas = data;
}

QMap<QString, QVariant> UpLabelItem::datas() const
{
    return m_datas;
}

void UpLabelItem::setiD(int idadef)
{
    m_id = idadef;
}

int UpLabelItem::iD() const
{
    return m_id;
}

void UpLabelItem::setImmediateToolTip(QString Msg)
{
    m_tooltipmsg = Msg;
}

void UpLabelItem::setRow(int rowadef)
{
    m_row = rowadef;
}

int UpLabelItem::Row() const
{
    return m_row;
}
