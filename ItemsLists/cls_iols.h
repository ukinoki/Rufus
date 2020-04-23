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

#ifndef IOLS_H
#define IOLS_H

#include "cls_iol.h"
#include "cls_itemslist.h"


class IOLs : public ItemsList
{
private:
    QMap<int, IOL*> *map_all = Q_NULLPTR;       //!< la liste de tous les IOLs, y compris ceux qui nes ont plus fabriqués

public:
    explicit IOLs(QObject *parent = Q_NULLPTR);

    QMap<int, IOL*> *iols() const;

    IOL*    getById(int id, bool reload = false);
    void    initListe();                        //! la liste de tous les IOLs

    //!> actions sur les enregistrements
    void    SupprimeIOL(IOL *iol);
    IOL*    CreationIOL(QHash<QString, QVariant> sets);
};


#endif // IOLS_H
