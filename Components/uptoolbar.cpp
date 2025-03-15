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

#include "uptoolbar.h"
#include "icons.h"

UpToolBar::UpToolBar(QWidget *parent) : QToolBar(parent)
{
    beginning   = new QAction(Icons::icPageAvant(),tr("Début"),this);
    end         = new QAction(Icons::icPageApres(),tr("Fin"),this);
    prec        = new QAction(Icons::icAvant(),tr("Précédent"),this);
    next        = new QAction(Icons::icApres(),tr("Suivant"),this);

    connect(beginning,  &QAction::triggered,  this, [=] {TBChoice(_first);});
    connect(end,        &QAction::triggered,  this, [=] {TBChoice(_last);});
    connect(prec,       &QAction::triggered,  this, [=] {TBChoice(_prec);});
    connect(next,       &QAction::triggered,  this, [=] {TBChoice(_next);});

    addAction(beginning);
    addAction(prec);
    addAction(next);
    addAction(end);

    setFixedHeight(46);
    setIconSize(QSize(35,35));
}

UpToolBar::~UpToolBar()
{
}

UpToolBar::Choice UpToolBar::choice() const
{
    return m_choice;
}

void UpToolBar::TBChoice(Choice choice)
{
    m_choice = choice;
    emit TBSignal();
}

QAction* UpToolBar::First() const
{
    return beginning;
}

QAction* UpToolBar::Last() const
{
    return end;
}

QAction* UpToolBar::Next() const
{
    return next;
}

QAction* UpToolBar::Prec() const
{
    return prec;
}
