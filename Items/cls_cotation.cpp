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

#include "cls_cotation.h"
#include <QDebug>

Cotation::Cotation(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

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


