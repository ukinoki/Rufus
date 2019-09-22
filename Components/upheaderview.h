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

#ifndef UPHEADERVIEW_H
#define UPHEADERVIEW_H

#include <QHeaderView>
#include <QAbstractItemModel>
#include <QTableWidget>
#include <QScrollBar>
#include <QEvent>

class UpHeaderView : public QHeaderView
{
    /*
     * Cette classe permet de faire des span sur les entêtes horizontaux de qtableview et qtablewidget
     * exemple dans dlg_gestionusers.cpp fonction ReconstruitListeLieuxExercice()
     */
    Q_OBJECT
public:
    explicit    UpHeaderView(QHeaderView *header);
    void        reDim(int section, int debut, int fin); //permet à la secion section de recouvrir les colonnes depuis debut à fin

private:
    bool        eventFilter(QObject *o, QEvent *e);
    void        updateOffset();
    int         getSectionSizes(int first, int second);
    QHeaderView *mainHeader;
};


#endif // UPHEADERVIEW_H
