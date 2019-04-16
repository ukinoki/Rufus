#include "cls_tierspayants.h"

TiersPayants::TiersPayants()
{
    m_tierspayants = new QMap<int, Tiers *>();
}

QMap<int, Tiers *> *TiersPayants::tierspayants() const
{
    return m_tierspayants;
}

void TiersPayants::addTiers(Tiers *Tiers)
{
    if( m_tierspayants->contains(Tiers->id()) )
        return;
    m_tierspayants->insert(Tiers->id(), Tiers);
}

void TiersPayants::addTiers(QList<Tiers*> listTiersPayants)
{
    QList<Tiers*>::const_iterator it;
    for( it = listTiersPayants.constBegin(); it != listTiersPayants.constEnd(); ++it )
        addTiers( *it );
}

Tiers* TiersPayants::getTiersById(int id)
{
    QMultiMap<int, Tiers*>::const_iterator itcpt = m_tierspayants->find(id);
    if( itcpt == m_tierspayants->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void TiersPayants::clearAll()
{
    QList<Tiers*> listtierss;
    for( QMap<int, Tiers*>::const_iterator ittrs = m_tierspayants->constBegin(); ittrs != m_tierspayants->constEnd(); ++ittrs)
        delete ittrs.value();
    m_tierspayants->clear();
}

void TiersPayants::removeTiers(Tiers *tiers)
{
    if (tiers == Q_NULLPTR)
        return;
    while( m_tierspayants->contains(tiers->id()) )
        m_tierspayants->remove(tiers->id());
    delete tiers;
}

/*!
 * \brief TiersPayants::initListeTiers
 * Charge l'ensemble des tiers payants
 * et les ajoute à la classe Tiers
 */
void TiersPayants::initListe()
{
    clearAll();
    QList<Tiers*> listtiers = DataBase::getInstance()->loadTiersPayants();
    QList<Tiers*>::const_iterator ittrs;
    for( ittrs = listtiers.constBegin(); ittrs != listtiers.constEnd(); ++ittrs )
    {
        Tiers *trs = const_cast<Tiers*>(*ittrs);
        addTiers( trs );
    }
}
