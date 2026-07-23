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

#ifndef CLS_COTATIONS_H
#define CLS_COTATIONS_H

#include "cls_cotation.h"
#include "cls_itemslist.h"
#include "cls_user.h"

class Cotations : public ItemsList
{
    Q_OBJECT
private:
    QMap<int, Cotation*> *map_cotations;        //!< la liste des cotations pratiquées par un utilisateur
    QMap<int, Cotation*> *map_usercotations;    //!< les cotations d'un user réunies depuis les 4 tables de jointures
    User* m_userparent = nullptr;               //!< le user dont on a chargé les cotations

public:
    explicit Cotations(QObject *parent = nullptr);
    QMap<int, Cotation *> *cotations() const;
    QMap<int, Cotation *> *usercotations() const;
    void loadCotations();                   //!< charge toutes les cotations de la table (avec leur type), sans idUser ni montant pratiqué
    void completeTipsManquants();           //!< renseigne le Tip vide (CCAM depuis ccam, NGAP depuis le xml) — À N'APPELER QU'UNE FOIS au lancement, pas à chaque loadCotations
    void loadUserCotations(User *usr);      //!< réunit dans map_usercotations les cotations du user depuis les 4 tables de jointures
    User* userparent()      { return m_userparent; }
};

#endif // COTATIONS_H
