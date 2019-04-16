#include "cls_typestiers.h"


TypesTiers::TypesTiers()
{
    m_typestiers = new QList<TypeTiers *>();
}

QList<TypeTiers *> *TypesTiers::typestiers() const
{
    return m_typestiers;
}

void TypesTiers::addTypeTiers(TypeTiers *typetiers)
{
    if( m_typestiers->contains(typetiers) )
        return;
    *m_typestiers << typetiers;
}

void TypesTiers::addTypeTiers(QList<TypeTiers*> listTypesTiers)
{
    QList<TypeTiers*>::const_iterator it;
    for( it = listTypesTiers.constBegin(); it != listTypesTiers.constEnd(); ++it )
        addTypeTiers( *it );
}

void TypesTiers::removeTypeTiers(TypeTiers* typetiers)
{
    while( m_typestiers->contains(typetiers) )
        m_typestiers->removeOne(typetiers);
    delete  typetiers;
}
void TypesTiers::clearAll()
{
    while (m_typestiers->size() >0)
        removeTypeTiers(m_typestiers->at(0));
    m_typestiers->clear();
}

/*!
 * \brief TypesTiers::initTypesTiers
 * Charge l'ensemble des types de tiers payants
 * et les ajoute à la classe TypesTiers
 */
void TypesTiers::initListe()
{
    clearAll();
    QList<TypeTiers*> listtypes = DataBase::getInstance()->loadTypesTiers();
    QList<TypeTiers*>::const_iterator ittyp;
    for( ittyp = listtypes.constBegin(); ittyp != listtypes.constEnd(); ++ittyp )
    {
        TypeTiers *typ = const_cast<TypeTiers*>(*ittyp);
        addTypeTiers( typ );
    }
}

