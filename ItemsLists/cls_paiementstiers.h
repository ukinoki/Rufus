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

#ifndef CLS_PAIEMENTSTIERS_H
#define CLS_PAIEMENTSTIERS_H

#include "cls_paiementtiers.h"
#include "cls_user.h"
#include "cls_itemslist.h"

/*!
 * \brief Cette classe gére la liste des paiements par tiers payants
 *  ( = paiements effectués par toute autre personne que le patient )
 */

class PaiementsTiers : public ItemsList
{

private:
    QMap<int, PaiementTiers*> *m_paiementstiers; //!< la liste des paiements par tiers
    void addList(QList<PaiementTiers*> listpaiementstiers);

public:
    explicit PaiementsTiers(QObject *parent = Q_NULLPTR);

    QMap<int, PaiementTiers *> *paiementstiers() const;

    PaiementTiers* getById(int id);
    void initListe(User *usr);
};

#endif // CLS_PAIEMENTSTIERS_H
