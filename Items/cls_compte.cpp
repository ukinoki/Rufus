#include "cls_compte.h"

Compte::Compte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Compte::id() const { return m_id; }
QString Compte::nom() const { return m_nom; }
bool Compte::isDesactive() const { return m_desactive; }
QString Compte::iban() const { return m_iban; }
QString Compte::intitulecompte() const { return m_intitulecompte; }
QString Compte::nombanque() const { return m_nombanque; }


void Compte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);
    setDataString(data, "nom", m_nom);
    setDataBool(data, "desactive", m_desactive);
    setDataString(data, "IBAN", m_iban);
    setDataString(data, "IntituleCompte", m_intitulecompte);
    setDataString(data, "NomBanque", m_nombanque);
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
