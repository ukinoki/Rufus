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

#include "upswitch.h"
#include "icons.h"

UpSwitch::UpSwitch(QWidget *parent) : QFrame(parent)
{
    setFixedSize(200,40);
    setFrameShape(QFrame::NoFrame);

    int marge           = 2;
    datelbl             = new QLabel(this);
    datelbl             ->setText("Date");
    datelbl             ->setFixedWidth(50);
    datelbl             ->move(marge+10, 9);
    datelbl             ->setAlignment(Qt::AlignCenter);
    cursorlbl           = new QLabel(this);
    cursorlbl           ->setPixmap(Icons::pxSwitchLeft().scaled(55,40)); //TODO : icon scaled : pxSwitchLeft 55,40
    cursorlbl           ->setFixedWidth(60);
    cursorlbl           ->move(datelbl->width() + (marge*2)+10, 0);
    typelbl             = new QLabel(this);
    typelbl             ->setText("Type");
    typelbl             ->setFixedWidth(50);
    typelbl             ->move(datelbl->width() + cursorlbl->width() + (marge*3)+10, 9);
    typelbl             ->setAlignment(Qt::AlignCenter);

    Activelbl           = datelbl;
    Style =             "border-radius: 10px; background-color:rgb(50,200,105,145);";
    Activelbl           ->setStyleSheet(Style);

    datelbl             ->installEventFilter(this);
    typelbl             ->installEventFilter(this);
    cursorlbl           ->installEventFilter(this);
}

bool UpSwitch::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease)
        BasculeSwitch();
    return QWidget::eventFilter(obj, event);
}

void UpSwitch::BasculeSwitch()
{
    if (Activelbl==datelbl)
    {
        Activelbl   = typelbl;
        cursorlbl   ->setPixmap(Icons::pxSwitchRight().scaled(55,40)); //TODO : icon scaled : pxSwitchRight 55,40
        datelbl     ->setStyleSheet("");
        emit Bascule(1);
    }
    else if (Activelbl==typelbl)
    {
        Activelbl   = datelbl;
        cursorlbl   ->setPixmap(Icons::pxSwitchLeft().scaled(55,40)); //TODO : icon scaled : pxSwitchLeft 55,40
        typelbl     ->setStyleSheet("");
        emit Bascule(0);
    }
    Activelbl           ->setStyleSheet(Style);
}
