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

#ifndef CLS_LIGNEPAIEMENT_H
#define CLS_LIGNEPAIEMENT_H

#include "cls_item.h"
#include "macros.h"

/*!
 * \brief LignePaiement class
 * l'ensemble des informations concernant une ligne de paiement
 */

class LignePaiement : public Item
{
    Q_OBJECT
private:
    double m_paye;
    QString m_monnaie;

public:
    explicit LignePaiement(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int idacte() const;
    int idrecette() const;
    double paye() const;
    QString monnaie() const;
};

#endif // CLS_LIGNEPAIEMENT_H
