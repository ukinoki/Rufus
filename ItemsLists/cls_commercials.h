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

#ifndef COMMERCIALS_H
#define COMMERCIALS_H

#include "cls_itemslist.h"
#include "cls_commercial.h"

class Commercials : public ItemsList
{
private:
    QMap<int, Commercial*> *map_all = Q_NULLPTR;      //!< la liste de tous les commerciaux

public:
    explicit Commercials(QObject *parent = Q_NULLPTR);

    QMap<int, Commercial*> *commercials() const;

    Commercial* getById(int id);
    void initListe();
    void initListebyidManufacturer(int idmanufacturer);

    //!> actions sur les enregistrements
    void                SupprimeCommercial(Commercial *Commercial);
    Commercial*         CreationCommercial(QHash<QString, QVariant> sets);
};

#endif // COMMERCIALS_H
