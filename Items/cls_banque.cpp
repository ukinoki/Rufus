/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

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
    m_banques = new QMap<int, Banque*>();
}

QMap<int, Banque *> *Banques::banques() const
{
    return m_banques;
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

void Banques::removeBanque(Banque* banque)
{
    m_banques       ->remove(banque->id());
}

Banque* Banques::getBanqueById(int id)
{
    QMap<int, Banque*>::const_iterator itcpt = m_banques->find(id);
    if( itcpt == m_banques->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}
