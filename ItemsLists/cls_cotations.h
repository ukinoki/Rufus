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
private:
    QMap<int, Cotation*> *map_cotations;    //!< la liste des cotations pratiquées par un utilisateur
    int m_iduser;                           //!< l'id du user concerné par la liste de cotations

public:
    explicit Cotations(QObject *parent = Q_NULLPTR);
    QMap<int, Cotation *> *cotations() const;
    void initListeByUser(User *usr);
    int iduser() const;
    void setiduser(int iduser);
};

#endif // COTATIONS_H
