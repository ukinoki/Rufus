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

#include "upradiobutton.h"

UpRadioButton::UpRadioButton(QWidget *parent) : QRadioButton(parent)
{
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    m_id         = -1;
    m_toggleable    = true;
    m_item    = Q_NULLPTR;
}

UpRadioButton::UpRadioButton(const QString Title, QWidget *parent) : QRadioButton(Title, parent)
{
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    m_id          = -1;
    m_toggleable  = true;
}

void UpRadioButton::AfficheToolTip()
{
    if (m_tooltipmsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),m_tooltipmsg);
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
    m_tooltipmsg = Msg;
}
void UpRadioButton::setToggleable(bool val)
{
    m_toggleable = val;
}
bool UpRadioButton::Toggleable() const
{
    return m_toggleable;
}
void UpRadioButton::setiD(int val)
{
    m_id          = val;
}
int UpRadioButton::iD() const
{
    return m_id;
}

void UpRadioButton::setitem(Item* item)
{
    m_item = item;
}
Item* UpRadioButton::item() const
{
    return m_item;
}
