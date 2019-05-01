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

#include "upradiobutton.h"

UpRadioButton::UpRadioButton(QWidget *parent) : QRadioButton(parent)
{
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    gid         = -1;
    Toggable    = true;
    m_item    = Q_NULLPTR;
}

UpRadioButton::UpRadioButton(const QString Title, QWidget *parent) : QRadioButton(Title, parent)
{
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    gid          = -1;
    Toggable  = true;
}

void UpRadioButton::AfficheToolTip()
{
    if (gToolTipMsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),gToolTipMsg);
}

bool UpRadioButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type()==QEvent::Enter)
    {
        emit enter();
        AfficheToolTip();
        return true;
    }
    if(event->type()==QEvent::MouseButtonPress ||  event->type()==QEvent::MouseButtonDblClick)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            if (!Toggleable())
                return true;
    return QWidget::eventFilter(obj, event);
}

void UpRadioButton::setImmediateToolTip(QString Msg)
{
    gToolTipMsg = Msg;
}
void UpRadioButton::setToggleable(bool val)
{
    Toggable = val;
}
bool UpRadioButton::Toggleable() const
{
    return Toggable;
}
void UpRadioButton::setiD(int val)
{
    gid          = val;
}
int UpRadioButton::iD() const
{
    return gid;
}

void UpRadioButton::setItem(Item* item)
{
    m_item = item;
}
Item* UpRadioButton::getItem()
{
    return m_item;
}
