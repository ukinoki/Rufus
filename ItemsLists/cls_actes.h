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

#ifndef CLS_ACTES_H
#define CLS_ACTES_H

#include "database.h"
#include "cls_acte.h"
#include "cls_itemslist.h"

class Actes: public ItemsList

{
private:
    QMap<int, Acte*> *m_actes = Q_NULLPTR; //!< une liste d'actes

public:
    explicit Actes(QObject *parent = Q_NULLPTR);

    QMap<int, Acte *> *actes() const;

    void    add(Acte *acte);
    void    addList(QList<Acte*> listActes);
    void    remove(Acte* acte);
    void    clearAll();
    Acte*   getById(int id, ADDTOLIST add = AddToList);
    void    initListeByPatient(Patient *pat);
    void    reloadActe(Acte* acte);

};


#endif // CLS_ACTES_H
