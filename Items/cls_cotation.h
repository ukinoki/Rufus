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

#ifndef CLS_COTATION_H
#define CLS_COTATION_H

#include "cls_item.h"

class Cotation : public Item
{

private:
    int m_iduser, m_frequence, m_idcotation;
    QString m_typeacte, m_descriptif;
    bool m_ccam;
    double m_montantoptam, m_montantnonoptam, m_montantpratique;

public:
    explicit Cotation(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int idcotation() const;
    QString typeacte() const;
    double montantoptam() const;
    double montantnonoptam() const;
    double montantpratique() const;
    bool isCCAM() const;
    int idUser() const;
    int frequence() const;
    QString descriptif() const;
};

#endif // CLS_COTATION_H
