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

#ifndef CLS_POSTESCONNECTES_H
#define CLS_POSTESCONNECTES_H

#include "cls_itemslist.h"
#include "cls_posteconnecte.h"
#include "database.h"

class PostesConnectes : public ItemsList
{
    /*! la classe userconnecté gère les connexions des utilisateurs
     * un utilisateur peut-être connecté sur plusieurs postes en même temps dans la même session
     * la clé n'du QMap des userconnectés est l'adresse mac du poste sur lequel le user est connecté,
     */
public:
    QMap<QString, PosteConnecte*> *map_postesconnectes = Q_NULLPTR;  //!< la liste de tous les postesconnectes

private:
    PosteConnecte   *m_admin = Q_NULLPTR;
    bool            adminset = false;

public:
    explicit PostesConnectes(QObject *parent = Q_NULLPTR);
    ~PostesConnectes();

    QMap<QString, PosteConnecte*>* postesconnectes() const;

    PosteConnecte*  getByStringId(QString stringid);
    void            initListe();

    PosteConnecte*   admin(Item::UPDATE upd = Item::Update);                            //!> le poste sur lequel est connecté RufusAdmmin
    PosteConnecte*   currentpost();                                                     //!> le poste en cours d'utilisation

    //!> actions sur les enregistrements
    void             SupprimeAllPostesConnectes();
    void             SupprimePosteConnecte(PosteConnecte *post);
    PosteConnecte*   CreationPosteConnecte(int idsite);
};

#endif // CLS_USERSCONNECTES_H
