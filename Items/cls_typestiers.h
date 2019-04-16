#ifndef CLS_TYPESTIERS_H
#define CLS_TYPESTIERS_H

#include "cls_tiers.h"
#include "database.h"

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
    void clearAll();
    void initListe();
};


#endif // CLS_TYPESTIERS_H
