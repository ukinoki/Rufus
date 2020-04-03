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

#ifndef CLS_TYPESTIERS_H
#define CLS_TYPESTIERS_H

#include "cls_tiers.h"
#include "cls_itemslist.h"

/*!
 * \brief TypesTiers class
 * la liste des types de tiers payants
 *  - utilisée pour les paiements directs
 * AME, ACS, CMU, etc..
 * géré par la table rufus.listetiers
*/

class TypesTiers : public ItemsList
{
private:
    QList<TypeTiers *> *m_typestiers; //!< la liste des types de tiers payants
    bool add(TypeTiers *Tiers);
    void addList(QList<TypeTiers*> listTierss);
    void remove(TypeTiers* Tiers);

public:
    explicit TypesTiers(QObject *parent = Q_NULLPTR);

    QList<TypeTiers *> *typestiers() const;

    void clearAll();
    void initListe();
};


#endif // CLS_TYPESTIERS_H
