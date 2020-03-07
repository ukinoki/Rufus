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

#ifndef MANUFACTURERS_H
#define MANUFACTURERS_H

#include "cls_manufacturer.h"
#include "cls_itemslist.h"

class Manufacturers : public ItemsList
{
private:
    QMap<int, Manufacturer*> *map_actifs = Q_NULLPTR;    //!< la liste des manufacturers actifs
    QMap<int, Manufacturer*> *map_all = Q_NULLPTR;      //!< la liste de tous les manufacturers, y compris les inactifs

public:
    explicit Manufacturers(QObject *parent = Q_NULLPTR);

    QMap<int, Manufacturer*> *actifs() const;
    QMap<int, Manufacturer*> *alls() const;

    Manufacturer* getById(int id);
    void initListe();

    //!> actions sur les enregistrements
    void                SupprimeManufacturer(Manufacturer *Manufacturer);
    Manufacturer*       CreationManufacturer(QHash<QString, QVariant> sets);
};

#endif // MANUFACTURERS_H
