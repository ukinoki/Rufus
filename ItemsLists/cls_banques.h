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

#ifndef CLS_BANQUES_H
#define CLS_BANQUES_H

#include "cls_banque.h"
#include "cls_itemslist.h"

class Banques : public ItemsList
{
private:
    QMap<int, Banque*> *map_banques = Q_NULLPTR; //!< la liste des Banques

public:
    explicit Banques(QObject *parent = Q_NULLPTR);

    QMap<int, Banque *> *banques() const;

    Banque* getById(int id);
    void    initListe();

    //!> actions sur les enregistrements
    void       SupprimeBanque(Banque *bq);
    Banque*    CreationBanque(QString idBanqueAbrege, QString NomBanque, int CodeBanque = 0);
};

#endif // CLS_BANQUES_H
