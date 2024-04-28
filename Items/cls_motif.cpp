/* (C) 2020 LAINE SERGE
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


#include "cls_motif.h"

Motif::Motif(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

QString Motif::motif() const        { return m_motif; }
QString Motif::raccourci() const    { return m_raccourci; }
QString Motif::couleur() const      { return m_couleur; }
int Motif::duree() const            { return m_duree; }
bool Motif::pardefaut() const       { return m_pardefaut; }
bool Motif::utiliser() const        { return m_utiliser; }
int Motif::noordre() const          { return m_noordre; }

void Motif::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "id", m_id);
    setDataString(data, "motif", m_motif);
    setDataString(data, "raccourci", m_raccourci);
    setDataString(data, "couleur", m_couleur);
    setDataInt(data, "duree", m_duree);
    setDataBool(data, "pardefaut", m_pardefaut);
    setDataBool(data, "utiliser", m_utiliser);
    setDataInt(data, "noordre", m_noordre);
    m_data = data;
}


