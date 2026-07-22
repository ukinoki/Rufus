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

#ifndef CLS_COTATION_H
#define CLS_COTATION_H

#include "cls_item.h"

class Cotation : public Item
{
    Q_OBJECT
private:
    int m_frequence;
    QString m_typeacte, m_descriptif;
    int m_typcotation = 1;                  //!< type de cotation : 1=CCAM, 2=NGAP, 3=horsNGAPnorCCAM, 4=association CCAM
    bool m_isCCAM = false;                  //!< les 4 booléens ci-dessous sont déduits de m_typcotation
    bool m_isNGAP = false;
    bool m_isAssocCCAM = false;
    bool m_isnorGAPnorCCAM = false;
    double m_montantoptam, m_montantnonoptam, m_montantpratique, m_montantconventionnel;
    double montantoptam() const;
    double montantnonoptam() const;
public:
    explicit Cotation(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString typeacte() const;
    double montantconventionnel() const;
    double montantpratique() const;
    int  typcotation() const     { return m_typcotation; }
    void settypcotation(int typ) { m_typcotation = typ;
                                   m_isCCAM          = (typ == 1);
                                   m_isNGAP          = (typ == 2);
                                   m_isnorGAPnorCCAM = (typ == 3);
                                   m_isAssocCCAM     = (typ == 4); }
    bool isCCAM() const          { return m_isCCAM; }
    bool isNGAP() const          { return m_isNGAP; }
    bool isAssocCCAM() const     { return m_isAssocCCAM; }
    bool isnorGAPnorCCAM() const { return m_isnorGAPnorCCAM; }
    int frequence() const;
    QString descriptif() const;
};

#endif // CLS_COTATION_H
