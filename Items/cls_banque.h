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

#ifndef CLS_BANQUE_H
#define CLS_BANQUE_H

#include <QObject>
#include "cls_item.h"

/*!
 * \brief Banque class
 * l'ensemble des informations concernant une banque
 */


class Banque : public Item
{
private:
    int m_id, m_codebanque;
    QString m_idbanqueabrege, m_nombanque;

public:
    explicit Banque(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);

    int id() const;
    int CodeBanque() const;
    QString NomBanqueAbrege() const;
    QString NomBanque() const;
};


#endif // CLS_BANQUE_H
