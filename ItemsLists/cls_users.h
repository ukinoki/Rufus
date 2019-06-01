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

#ifndef CLS_USERS_H
#define CLS_USERS_H

#include "cls_itemslist.h"
#include "cls_user.h"
#include "database.h"

/*!
 * \brief The Users class
 * Cette classe gére les différents collections de "User"
 */
class Users : public ItemsList
{
private:
    QMap<int, User*> *m_users = Q_NULLPTR;           //!<Collection de tous les User
    QMap<int, User*> *m_superviseurs = Q_NULLPTR;    //!<Collection des superviseurs : User->isResponsable()
    QMap<int, User*> *m_parents = Q_NULLPTR;         //!<Collection des parents : User->isSoignant() && !User->isRemplacant()
    QMap<int, User*> *m_liberaux = Q_NULLPTR;        //!<Collection des liberaux : User->isLiberal()
    QMap<int, User*> *m_comptables = Q_NULLPTR;      //!<Collection des comptables : User->isSocComptable() || User->isLiberal()

public:
    //GETTER
    QMap<int, User *> *all() const;
    QMap<int, User *> *superviseurs() const;
    QMap<int, User *> *parents() const;
    QMap<int, User *> *liberaux() const;
    QMap<int, User *> *comptables() const;

    Users(QObject *parent = Q_NULLPTR);
    bool add(User *usr);
    void addList(QList<User*> listusr);
    User* getById(int id, Item::LOADDETAILS loadDetails = Item::NoLoadDetails, ADDTOLIST addToList = AddToList);
    QString getLoginById(int id);
    void initListe();
    User*   userconnected()             { return DataBase::I()->getUserConnected(); }

};

#endif // CLS_USERS_H
