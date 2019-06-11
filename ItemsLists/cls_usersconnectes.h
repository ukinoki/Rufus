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

#ifndef CLS_USERSCONNECTES_H
#define CLS_USERSCONNECTES_H

#include "cls_itemslist.h"
#include "cls_userconnecte.h"
#include "database.h"

class UsersConnectes : public ItemsList
{
    /*! la classe userconnecté gère les connexions des utilisateurs
     * un utilisateur peut -être connecté sur plusieurs posts en même temps dans la même session
     * la clé de la table correspond au couple iduser-macadress qui ne peut donc qu'être unique
     * la clé n'du QMap des userconnectés est de fait un QString concaténant l'id du user et l'adresse mac du poste sur lequel le user est connecté,
     * ces 2 éléments séparés par le séparateur TCPMSG_Separator
     * la recherche d'un couple user-macadress ne se fait donc pas par getById(int) mais par getByKey(QString)
     */
public:
    QMap<QString, UserConnecte*> *m_usersconnectes = Q_NULLPTR;  //!< la liste de tous les usersconnectes
    void addList(QList<UserConnecte*> listusr);

public:
    explicit UsersConnectes(QObject *parent = Q_NULLPTR);
    ~UsersConnectes();

    QMap<QString, UserConnecte*>* usersconnectes() const;

    UserConnecte* getByKey(QString key);
    void initListe();

    //!> actions sur les enregistrements
    void             SupprimeUserConnecte(UserConnecte *cpt);
    UserConnecte*    CreationUserConnecte();
};

#endif // CLS_USERSCONNECTES_H
