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
    cursorlbl           ->setPixmap(Icons::pxSwitchLeft().scaled(55,40)); //WARNING : icon scaled : pxSwitchLeft 55,40
    cursorlbl           ->setFixedWidth(60);
    cursorlbl           ->move(datelbl->width() + (marge*2)+10, 0);
    typelbl             = new QLabel(this);
    typelbl             ->setText("Type");
    typelbl             ->setFixedWidth(50);
    typelbl             ->move(datelbl->width() + cursorlbl->width() + (marge*3)+10, 9);
    typelbl             ->setAlignment(Qt::AlignCenter);

    Activelbl           = datelbl;
    Activelbl           ->setStyleSheet(STYLE_UPSWITCH);

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

int UpSwitch::PosSwitch() const
{
    return pos;
}

void UpSwitch::BasculeSwitch()
{
    if (Activelbl==datelbl)
    {
        Activelbl   = typelbl;
        cursorlbl   ->setPixmap(Icons::pxSwitchRight().scaled(55,40)); //WARNING : icon scaled : pxSwitchRight 55,40
        datelbl     ->setStyleSheet("");
        pos = 1;
        emit Bascule();
    }
    else if (Activelbl==typelbl)
    {
        Activelbl   = datelbl;
        cursorlbl   ->setPixmap(Icons::pxSwitchLeft().scaled(55,40)); //WARNING : icon scaled : pxSwitchLeft 55,40
        typelbl     ->setStyleSheet("");
        pos = 0;
        emit Bascule();
    }
    Activelbl           ->setStyleSheet(STYLE_UPSWITCH);
}
