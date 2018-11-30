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
