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

#include "uplabel.h"

UpLabel::UpLabel(QWidget *parent, QString txt) : QLabel(txt, parent)  //je ne sais plus pourquoi j'ai mis le parent avant le texte mais il y une raison à un moment donné du code...
{
    id          = -1;
    row         = -1;
    gToolTipMsg = "";
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
}

UpLabel::~UpLabel()
{

}
void UpLabel::AfficheToolTip()
{
    if (gToolTipMsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),gToolTipMsg);
}

bool UpLabel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        emit enter(getId());
        AfficheToolTip();
        return true;
    }
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        emit dblclick(getId());
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease)
    {
        emit clicked(getId());
        return true;
    }
   return QWidget::eventFilter(obj, event);
}

void UpLabel::setData(QMap<QString, QVariant> data)
{
    datas = data;
}

QMap<QString, QVariant> UpLabel::getData() const
{
    return datas;
}

void UpLabel::setId(int idadef)
{
    id = idadef;
}

int UpLabel::getId() const
{
    return id;
}

void UpLabel::setImmediateToolTip(QString Msg)
{
    gToolTipMsg = Msg;
}

void UpLabel::setRow(int rowadef)
{
    row = rowadef;
}

int UpLabel::getRow() const
{
    return row;
}
