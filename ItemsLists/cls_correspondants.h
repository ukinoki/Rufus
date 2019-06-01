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

#ifndef CLS_CORRESPONDANTS_H
#define CLS_CORRESPONDANTS_H

#include "cls_correspondant.h"
#include "database.h"
#include "cls_itemslist.h"

class Correspondants : public ItemsList
{
private:
    QMap<int, Correspondant*> *m_correspondants;    //!<Collection de tous les correspondants sans exception, généralistes ou pas

public:
    //GETTER
    QMap<int, Correspondant *> *correspondants()     const;

    Correspondants(QObject *parent = Q_NULLPTR);

    bool add(Correspondant *cor);
    void addList(QList<Correspondant*> listcor);
    Correspondant* getById(int id, Item::LOADDETAILS loadDetails = Item::NoLoadDetails, bool addToList = true);
    void remove(Correspondant* cor);
    void clearAll();
    void initListe(bool all = false);
};


#endif // CLS_CORRESPONDANTS_H
