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

#include "uptoolbar.h"
#include "icons.h"

UpToolBar::UpToolBar(QWidget *parent) : QToolBar(parent)
{
    debut   = new QAction(Icons::icPageAvant(),tr("Début"),this);
    prec    = new QAction(Icons::icAvant(),tr("Précédent"),this);
    suiv    = new QAction(Icons::icApres(),tr("Suivant"),this);
    fin     = new QAction(Icons::icPageApres(),tr("Fin"),this);

    connect(debut,  SIGNAL(triggered(bool)), this, SLOT(Slot_TBChoix()));
    connect(prec,   SIGNAL(triggered(bool)), this, SLOT(Slot_TBChoix()));
    connect(suiv,   SIGNAL(triggered(bool)), this, SLOT(Slot_TBChoix()));
    connect(fin,    SIGNAL(triggered(bool)), this, SLOT(Slot_TBChoix()));

    addAction(debut);
    addAction(prec);
    addAction(suiv);
    addAction(fin);

    setFixedHeight(46);
    setIconSize(QSize(35,35));
}

UpToolBar::~UpToolBar()
{
}

void UpToolBar::Slot_TBChoix()
{
    QAction *Choix = static_cast<QAction*>(sender());
    action = Choix->text();
    emit TBSignal();
}

QAction* UpToolBar::First()
{
    return debut;
}

QAction* UpToolBar::Last()
{
    return fin;
}

QAction* UpToolBar::Next()
{
    return suiv;
}

QAction* UpToolBar::Prec()
{
    return prec;
}



