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

#ifndef CLS_TIERS_H
#define CLS_TIERS_H

#include "cls_item.h"

/*!
 * \brief Tiers class
 * l'ensemble des informations concernant un tiers payant
 *!
 * \brief TypeTiers class
 * non d'un type de tiers payants - utilisée pour les paiements directs
 *  - utilisée pour les paiements directs
 * AME, ACS, CMU, etc..
 * géré par la table ComtaMedicale.tiers
 */


class Tiers : public Item
{

private:
    QString m_nomtiers, m_adressetiers, m_villetiers, m_codepostaltiers, m_telephonetiers, m_faxtiers;
public:
    explicit Tiers(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString nomtiers() const;
    QString codepostaltiers() const;
    QString villetiers() const;
    QString telephonetiers() const;
    QString adressetiers() const;
    QString faxtiers() const;
};

class TypeTiers : public Item
{

private:
    QString m_typetiers;
public:
    explicit TypeTiers(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString typetiers() const;
};


#endif // CLS_TIERS_H
