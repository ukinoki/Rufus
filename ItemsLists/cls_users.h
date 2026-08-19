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

#ifndef CLS_USERS_H
#define CLS_USERS_H

#include "cls_itemslist.h"
#include "cls_user.h"

/*!
 * \brief The Users class
 * Cette classe gére les différents maps de "User"
 */
class Users : public ItemsList
{
    Q_OBJECT
private:
    QMap<int, User*> *map_all = nullptr;             //!< map de tous les users
    QMap<int, User*> *map_actifs = nullptr;          //!< map de tous les users actifs
    QMap<int, User*> *map_inactifs = nullptr;        //!< map de tous les users inactifs
    QMap<int, User*> *map_superviseurs = nullptr;    //!< map des superviseurs : User->isResponsable()
    QMap<int, User*> *map_parents = nullptr;         //!< map des parents : User->isSoignant() && !User->isRemplacant()
    QMap<int, User*> *map_liberaux = nullptr;        //!< map des liberaux : User->isLiberal()
    QMap<int, User*> *map_comptablesactes = nullptr; //!< map des comptables qui enregistrent des actes médicaux: User->isSocComptable() || User->isLiberal()
    QMap<int, User*> *map_comptablessaufactes = nullptr;//!< map des comptables qui enregistrent des recettes dépenses mais pas d'actes médicaux: User->isSocComptable() || User->isLiberal()
    QMap<int, User*> *map_medecins = nullptr;        //!< map des medecins : User->isMedecin()
    bool addUser(User *usr);
    void addList(QList<User*> listusr);
    void mapsclean(User *usr = nullptr);
    User* m_useradmin = nullptr;

public:
    //GETTER
    QMap<int, User *> *all() const;
    QMap<int, User *> *actifs() const;
    QMap<int, User *> *inactifs() const;
    QMap<int, User *> *superviseurs() const;
    QMap<int, User *> *parents() const;
    QMap<int, User *> *liberaux() const;
    QMap<int, User *> *comptablesActes() const;
    QMap<int, User *> *comptablesSaufActes() const;
    QMap<int, User *> *medecins() const;

    Users(QObject *parent = nullptr);
    User*       Admin()  const  { return m_useradmin; }
    User*       getById(int id, Item::UPDATE upd = Item::NoUpdate);
    void        reload(User* usr);
    bool        recalcStatut(User* usr);
    void        initListe();
    void        CalcCompteEncaissementActes (User *usr);
    void        SupprimeUser(User *usr);
    User*       userconnected()     { return getById(DataBase::I()->idUserConnected()); }
};

#endif // CLS_USERS_H
