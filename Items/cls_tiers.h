/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CLS_TIERS_H
#define CLS_TIERS_H

#include <QObject>
#include "cls_item.h"

/*!
 * \brief Tiers class
 * l'ensemble des informations concernant un tiers payant
 *!
 * \brief TierPayants class
 * la liste des tiers payants
 *!
 * \brief TypeTiers class
 * non d'un type de tiers payants - utilisée pour les paiements directs
 * \brief TypesTiers class
 * la liste des types de tiers payants - utilisée pour les paiements directs
 */


class Tiers : public Item
{
private:
    int m_id;
    QString m_nomtiers, m_adressetiers, m_villetiers, m_codepostaltiers, m_telephonetiers, m_faxtiers;
public:
    explicit Tiers(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);

    int id() const;
    QString nomtiers() const;
    QString codepostaltiers() const;
    QString villetiers() const;
    QString telephonetiers() const;
    QString adressetiers() const;
    QString faxtiers() const;
};


class TiersPayants
{
private:
    QMap<int, Tiers*> *m_tierspayants; //!< la liste des tiers payants

public:
    explicit TiersPayants();

    QMap<int, Tiers *> *tierspayants() const;

    void addTiers(Tiers *Tiers);
    void addTiers(QList<Tiers*> listTierss);
    void removeTiers(Tiers* Tiers);
    Tiers* getTiersById(int id);
};

class TypeTiers : public Item
{
private:
    QString m_typetiers;
public:
    explicit TypeTiers(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data);

    QString typetiers() const;
};

class TypesTiers
{
private:
    QList<TypeTiers *> *m_typestiers; //!< la liste des types de tiers payants

public:
    explicit TypesTiers();

    QList<TypeTiers *> *typestiers() const;

    void addTypeTiers(TypeTiers *Tiers);
    void addTypeTiers(QList<TypeTiers*> listTierss);
    void removeTypeTiers(TypeTiers* Tiers);
};


#endif // CLS_TIERS_H
