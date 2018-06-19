#include "cls_compte.h"

Compte::Compte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Compte::id() const { return m_id; }
QString Compte::nom() const { return m_nom; }
bool Compte::isDesactive() const { return m_desactive; }


void Compte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);
    setDataString(data, "nom", m_nom);
    setDataBool(data, "desactive", m_desactive);
}










Comptes::Comptes()
{
}

QMultiMap<int, Compte *> Comptes::comptes() const
{
    return m_comptes;
}
QMultiMap<int, Compte *> Comptes::comptesAll() const
{
    return m_comptesAll;
}

void Comptes::addCompte(Compte *compte)
{
    if( m_comptesAll.contains(compte->id()) )
        return;

    m_comptesAll.insert(compte->id(), compte);
    if( !compte->isDesactive() )
        m_comptes.insert(compte->id(), compte);
}
void Comptes::addCompte(QList<Compte*> listCompte)
{
    QList<Compte*>::const_iterator it;
    for( it = listCompte.constBegin(); it != listCompte.constEnd(); ++it )
        addCompte( *it );
}
