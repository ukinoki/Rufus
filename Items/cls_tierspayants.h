#ifndef TIERSPAYANTS_H
#define TIERSPAYANTS_H

#include "database.h"
#include "cls_tiers.h"


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
    void clearAll();
    void initListe();
};

#endif // TIERSPAYANTS_H
