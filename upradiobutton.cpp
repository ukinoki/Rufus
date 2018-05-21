/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "upradiobutton.h"

UpRadioButton::UpRadioButton(QWidget *parent) : QRadioButton(parent)
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
    if (event->type() == QEvent::FocusIn )
    {
        QGroupBox *box = dynamic_cast<QGroupBox*>(parent());
        if (box != Q_NULLPTR)
            box->setStyleSheet("QGroupBox {border: 1px solid rgb(164, 205, 255); border-radius: 10px;"
                               "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);}");
    }

    if (event->type() == QEvent::FocusOut )
    {
        QGroupBox *box = dynamic_cast<QGroupBox*>(parent());
        if (box != Q_NULLPTR)
            box->setStyleSheet("");
    }
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
