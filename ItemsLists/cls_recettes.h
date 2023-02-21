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

#ifndef CLS_RECETTES_H
#define CLS_RECETTES_H

#include "cls_recette.h"
#include "cls_itemslist.h"

class Recettes : public ItemsList
{
private:
    QMap<int, Recette*> *map_recettes = Q_NULLPTR; //!< la liste des recettes

public:
    explicit Recettes(QObject *parent = Q_NULLPTR);

    QMap<int, Recette *> *recettes() const;

    Recette* getById(int id);
    void    initListe(QMap<Utils::Period, QDate> DateMap);
};

#endif // CLS_RECETTES_H
