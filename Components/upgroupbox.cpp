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

#include "upgroupbox.h"

UpGroupBox::UpGroupBox(QWidget *parent) : QGroupBox(parent)
{
    installEventFilter(this);
    setStyleSheet(STYLE_UPGROUBOXINACTIVE);
}

bool UpGroupBox::eventFilter(QObject *obj, QEvent *event)           //!> ce code ne marche pas et provoque un plantage sous MacOS à partir de Qt5.10 - pas sous Linux (XCode?)
{
    if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved)
    {
        foreach (QWidget* widg, this->findChildren<QWidget*>())
            widg->installEventFilter(this);
    }
    else if (event->type() == QEvent::FocusIn )
    {
        QWidget *widg = dynamic_cast<QWidget *>(obj);
        if (widg!=Q_NULLPTR)
            setStyleSheet(STYLE_UPGROUBOXACTIVE);
    }
    else if (event->type() == QEvent::FocusOut )
    {
        QWidget *widg = dynamic_cast<QWidget *>(obj);
        if (widg!=Q_NULLPTR)
            setStyleSheet(STYLE_UPGROUBOXINACTIVE);
    }
    return QWidget::eventFilter(obj, event);
}
