#include "cls_banque.h"

Banque::Banque(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Banque::id() const                    { return m_id; }
int Banque::CodeBanque() const            { return m_codebanque; }
QString Banque::NomBanque() const         { return m_nombanque; }
QString Banque::NomBanqueAbrege() const   { return m_idbanqueabrege; }

void Banque::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "id", m_id);
    setDataString(data, "nombanque", m_nombanque);
    setDataString(data, "idbanqueabrege", m_idbanqueabrege);
    setDataInt(data, "codebanque", m_codebanque);
}


Banques::Banques()
{
}

QMultiMap<int, Banque *> Banques::banques() const
{
    return m_banques;
}

void Banques::addBanque(Banque *banque)
{
    if( m_banques.contains(banque->id()) )
        return;
    m_banques.insert(banque->id(), banque);
}

void Banques::addBanque(QList<Banque*> listbanques)
{
    QList<Banque*>::const_iterator it;
    for( it = listbanques.constBegin(); it != listbanques.constEnd(); ++it )
        addBanque( *it );
}

void Banques::removeBanque(Banque* banque)
{
    m_banques       .remove(banque->id());
}

Banque* Banques::getBanqueById(int id)
{
    QMultiMap<int, Banque*>::const_iterator itcpt = m_banques.find(id);
    if( itcpt == m_banques.constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}
