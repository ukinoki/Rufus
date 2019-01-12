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

#include "cls_cotation.h"
#include <QDebug>

Cotation::Cotation(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Cotation::id() const                    { return m_id; }
int Cotation::idcotation() const            { return m_idcotation; }
QString Cotation::typeacte() const          { return m_typeacte; }
double Cotation::montantoptam() const       { return m_montantoptam; }
double Cotation::montantnonoptam() const    { return m_montantnonoptam; }
double Cotation::montantpratique() const    { return m_montantpratique; }
bool Cotation::isCCAM() const               { return m_ccam; }
int Cotation::idUser() const                { return m_iduser; }
int Cotation::frequence() const             { return m_frequence; }
QString Cotation::descriptif() const        { return m_descriptif; }

void Cotation::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);
    setDataInt(data, "idcotation", m_idcotation);
    setDataString(data, "typeacte", m_typeacte);
    setDataDouble(data, "montantoptam", m_montantoptam);
    setDataDouble(data, "montantnonoptam", m_montantnonoptam);
    setDataDouble(data, "montantpratique", m_montantpratique);
    setDataBool(data, "ccam", m_ccam);
    setDataInt(data, "iduser", m_iduser);
    setDataInt(data, "frequence", m_frequence);
    setDataString(data, "descriptif", m_descriptif);
}


Cotations::Cotations()
{
    m_cotations = new QMap<int, Cotation*>();
    m_cotationsbyuser = new QMap<int, Cotation*>();
}

QMap<int, Cotation *> *Cotations::cotations() const
{
    return m_cotations;
}

QMap<int, Cotation *> *Cotations::cotationsbyuser() const
{
    return m_cotationsbyuser;
}

void Cotations::addCotation(Cotation *cotation)
{
    if( m_cotations->contains(cotation->id()) )
        return;
    m_cotations->insert(cotation->id(), cotation);
}

void Cotations::addCotationByUser(Cotation *cotation)
{
    if( m_cotationsbyuser->contains(cotation->id()) )
        return;
    m_cotationsbyuser->insert(cotation->id(), cotation);
}

void Cotations::addCotation(QList<Cotation*> listcotations)
{
    QList<Cotation*>::const_iterator it;
    for( it = listcotations.constBegin(); it != listcotations.constEnd(); ++it )
        addCotation( *it );
}

Cotation* Cotations::getCotationById(int id)
{
    QMap<int, Cotation*>::const_iterator itcot = m_cotations->find(id);
    if( itcot == m_cotations->constEnd() )
        return Q_NULLPTR;
    return itcot.value();
}

void Cotations::clearAll()
{
    QList<Cotation*> listcots;
    for( QMap<int, Cotation*>::const_iterator itcot = m_cotations->constBegin(); itcot != m_cotations->constEnd(); ++itcot)
    {
        Cotation *cot = const_cast<Cotation*>(*itcot);
        if (!listcots.contains(cot))
                listcots << cot;
    }
    for (int i=listcots.size()-1; i==0; --i)
        removeCotation(listcots.at(i));
    m_cotations->clear();
}

void Cotations::removeCotation(Cotation *cot)
{
    if (cot == Q_NULLPTR)
        return;
    QMap<int, Cotation*>::const_iterator itcot = m_cotations->find(cot->id());
    if( itcot == m_cotations->constEnd() )
        return;
    m_cotations->remove(cot->id());
    delete cot;
}