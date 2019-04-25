#include "cls_actes.h"

Actes::Actes()
{
    m_actes = new QMap<int, Acte*>();
}

QMap<int, Acte *> *Actes::actes() const
{
    return m_actes;
}

/*!
 * \brief Actes::initListe
 * Charge l'ensemble des actes
 * et les ajoute à la classe Actes
 */
void Actes::initListeByPatient(Patient *pat)
{
    clearAll();
    QMap<int, Acte*> listActes = DataBase::getInstance()->loadActesByPat(pat);
    QMap<int, Acte*>::const_iterator itact;
    for( itact = listActes.constBegin(); itact != listActes.constEnd(); ++itact )
    {
        Acte *act = const_cast<Acte*>(*itact);
        add( act );
    }
}

void Actes::add(Acte *acte)
{
    if( m_actes->contains(acte->id()) )
        return;
    m_actes->insert(acte->id(), acte);
}

void Actes::addList(QList<Acte*> listActes)
{
    QList<Acte*>::const_iterator it;
    for( it = listActes.constBegin(); it != listActes.constEnd(); ++it )
        add( *it );
}

void Actes::clearAll()
{
    for( QMap<int, Acte*>::const_iterator itact = m_actes->constBegin(); itact != m_actes->constEnd(); ++itact)
        delete itact.value();
    m_actes->clear();
}

void Actes::remove(Acte *acte)
{
    if (acte == Q_NULLPTR)
        return;
    QMap<int, Acte*>::const_iterator itact = m_actes->find(acte->id());
    if( itact == m_actes->constEnd() )
        return;
    m_actes->remove(acte->id());
    delete acte;
}

Acte* Actes::getById(int id)
{
    QMap<int, Acte*>::const_iterator itact = m_actes->find(id);
    if( itact == m_actes->constEnd() )
        return Q_NULLPTR;
    return itact.value();
}
