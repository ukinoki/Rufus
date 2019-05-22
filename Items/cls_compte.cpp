/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cls_compte.h"

Compte::Compte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Compte::id() const                  { return m_id; }
int Compte::idUser() const              { return m_iduser; }
int Compte::idBanque() const            { return m_idbanque; }
QString Compte::iban() const            { return m_iban; }
QString Compte::intitulecompte() const  { return m_intitulecompte; }
QString Compte::nom() const             { return m_nom; }
double Compte::solde() const            { return m_solde; }
bool Compte::isDesactive() const        { return m_desactive; }
bool Compte::isPartage() const          { return m_partage; }
QString Compte::nombanque() const       { return m_nombanque; }
bool Compte::isPrefere() const          { return m_prefere; }


void Compte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);
    setDataInt(data, "iduser", m_iduser);
    setDataInt(data, "idbanque", m_idbanque);
    setDataString(data, "IBAN", m_iban);
    setDataString(data, "IntituleCompte", m_intitulecompte);
    setDataString(data, "nom", m_nom);
    setDataDouble(data, "solde", m_solde);
    setDataBool(data, "desactive", m_desactive);
    setDataBool(data, "partage", m_partage);
    setDataString(data, "NomBanque", m_nombanque);
    setDataBool(data, "prefere", m_prefere);
}

void Compte::setSolde(double solde)
{
    m_solde = solde;
}


Comptes::Comptes()
{
    m_comptes = new QMultiMap<int, Compte*>();
    m_comptesAll = new QMultiMap<int, Compte*>();
    m_comptesAllusers = new QMultiMap<int, Compte*>();
}

Comptes::~Comptes()
{
    clearAll();
    delete m_comptes;
    delete m_comptesAll;
    delete m_comptesAllusers;
}

QMultiMap<int, Compte *>* Comptes::comptes() const
{
    return m_comptes;
}
QMultiMap<int, Compte *>* Comptes::comptesAll() const
{
    return m_comptesAll;
}

void Comptes::addCompte(Compte *compte)
{
    if( m_comptesAll->contains(compte->id()) )
        return;

    m_comptesAll->insert(compte->id(), compte);
    if( !compte->isDesactive() )
        m_comptes->insert(compte->id(), compte);
}

void Comptes::addCompte(QList<Compte*> listCompte)
{
    QList<Compte*>::const_iterator it;
    for( it = listCompte.constBegin(); it != listCompte.constEnd(); ++it )
        addCompte( *it );
}

void Comptes::clearAll()
{
    QList<Compte*> listcpts;
    QMap<int, Compte*>::const_iterator itcpt;
    for( itcpt = m_comptesAll->constBegin(); itcpt != m_comptesAll->constEnd(); ++itcpt)
        delete itcpt.value();
    m_comptesAll->clear();
    m_comptes->clear();
    m_comptesAllusers->clear();
}

void Comptes::removeCompte(Compte* cpt)
{
    if (cpt == Q_NULLPTR)
        return;
    QMap<int, Compte*>::const_iterator itcpt;
    m_comptes->find(cpt->id());
    if( itcpt != m_comptes->constEnd() )
        m_comptes           ->remove(cpt->id());
    m_comptesAll->find(cpt->id());
    if( itcpt != m_comptesAll->constEnd() )
        m_comptesAll        ->remove(cpt->id());
    m_comptesAllusers->find(cpt->id());
    if( itcpt != m_comptesAllusers->constEnd() )
        m_comptesAllusers   ->remove(cpt->id());
    delete cpt;
}

Compte* Comptes::getCompteById(int id)
{
    QMultiMap<int, Compte*>::const_iterator itcpt = m_comptesAll->find(id);
    if( itcpt == m_comptesAll->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

