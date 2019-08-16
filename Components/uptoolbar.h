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

#ifndef UPTOOLBAR_H
#define UPTOOLBAR_H

#include <QToolBar>
#include <QEvent>

class UpToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit    UpToolBar(bool AvecFinDebut = true, bool AvecReload = false, QWidget *parent = Q_NULLPTR);
    ~UpToolBar();
    QAction*            First() const;
    QAction*            Last() const;
    QAction*            Next() const;
    QAction*            Prec() const;
    QAction*            Reload() const;
    QString             choix() const;
private:
    QString             m_action;
    QAction             *debut, *prec, *suiv, *fin, *reload;
    void                TBChoix(QAction *choix);
signals:
    void                TBSignal();
};

#endif // UPTOOLBAR_H
