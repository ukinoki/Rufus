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

#include "upcheckbox.h"

UpCheckBox::UpCheckBox(QWidget *parent) : QCheckBox(parent)
{
    m_rowtable    = -1;
    m_id          = -1;
    m_toggleable  = true;
    setContextMenuPolicy(Qt::NoContextMenu);
    installEventFilter(this);
}

UpCheckBox::UpCheckBox(QString text, QWidget *parent) : UpCheckBox(parent)
{
    setText(text);
}

UpCheckBox::~UpCheckBox()
{

}

void UpCheckBox::AfficheToolTip()
{
    if (m_tooltipmsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),m_tooltipmsg);
}

bool UpCheckBox::eventFilter(QObject *obj, QEvent *event)
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

void UpCheckBox::setImmediateToolTip(QString Msg)
{
    m_tooltipmsg = Msg;
}
void UpCheckBox::setRowTable(int val)
{
    m_rowtable    = val;
}
int UpCheckBox::rowTable() const
{
    return m_rowtable;
}
void UpCheckBox::setiD(int val)
{
    m_id          = val;
}
int UpCheckBox::iD() const
{
    return m_id;
}
void UpCheckBox::setToggleable(bool val)
{
    m_toggleable = val;
}
bool UpCheckBox::Toggleable() const
{
    return m_toggleable;
}
