/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UPTOOLBAR_H
#define UPTOOLBAR_H

#include <QToolBar>
#include <QEvent>
#include <QIcon>

class UpToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit    UpToolBar(QWidget *parent = Q_NULLPTR);
    ~UpToolBar();
    QAction*            First();
    QAction*            Last();
    QAction*            Next();
    QAction*            Prec();
private:
    QIcon               giconPageAvant, giconAvant,giconApres, giconPageApres;
    void                EmetReponse();
    QAction             *debut, *prec, *suiv, *fin;
private slots:
    void                Slot_TBChoix();
signals:
    void                TBSignal(QString);
};

#endif // UPTOOLBAR_H
