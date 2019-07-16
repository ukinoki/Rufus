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

#ifndef CLS_REFRACTIONS_H
#define CLS_REFRACTIONS_H

#include "database.h"
#include "cls_refraction.h"
#include "cls_itemslist.h"

class Refractions : public ItemsList
{
private:
    QMap<int, Refraction*> *m_refractions = Q_NULLPTR;      //!< la liste des refractions

public:
    explicit Refractions(QObject *parent = Q_NULLPTR);

    QMap<int, Refraction *> *refractions() const;

    Refraction* getById(int id);
    void initListebyPatId(int id);

    //!> actions sur les enregistrements
    void    SupprimeRefraction(Refraction *ref);
};

#endif // CLS_REFRACTIONS_H
