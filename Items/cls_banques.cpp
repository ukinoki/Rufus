#include "cls_banques.h"

Banques::Banques()
{
    m_banques = new QMap<int, Banque*>();
}

QMap<int, Banque *> *Banques::banques() const
{
    return m_banques;
}

/*!
 * \brief Naques::initListeBanques
 * Charge l'ensemble des banques
 * et les ajoute à la classe Banques
 */
void Banques::initListe()
{
    clearAll();
    QList<Banque*> listbanques = DataBase::getInstance()->loadBanques();
    QList<Banque*>::const_iterator itbq;
    for( itbq = listbanques.constBegin(); itbq != listbanques.constEnd(); ++itbq )
    {
        Banque *bq = const_cast<Banque*>(*itbq);
        addBanque( bq );
    }
}

void Banques::addBanque(Banque *banque)
{
    if( m_banques->contains(banque->id()) )
        return;
    m_banques->insert(banque->id(), banque);
}

void Banques::addBanque(QList<Banque*> listbanques)
{
    QList<Banque*>::const_iterator it;
    for( it = listbanques.constBegin(); it != listbanques.constEnd(); ++it )
        addBanque( *it );
}

void Banques::clearAll()
{
    QList<Banque*> listbanqs;
    for( QMap<int, Banque*>::const_iterator itbq = m_banques->constBegin(); itbq != m_banques->constEnd(); ++itbq)
        delete itbq.value();
    m_banques->clear();
}

void Banques::removeBanque(Banque *banq)
{
    if (banq == Q_NULLPTR)
        return;
    QMap<int, Banque*>::const_iterator itbanq = m_banques->find(banq->id());
    if( itbanq == m_banques->constEnd() )
        return;
    m_banques->remove(banq->id());
    delete banq;
}

Banque* Banques::getBanqueById(int id)
{
    QMap<int, Banque*>::const_iterator itcpt = m_banques->find(id);
    if( itcpt == m_banques->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}
