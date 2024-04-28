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

#include "cls_commercial.h"

Commercial::Commercial(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}
void Commercial::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, CP_ID_COM, m_id);
    setDataString(data, CP_NOM_COM, m_nom);
    setDataString(data, CP_PRENOM_COM, m_prenom);
    setDataString(data, CP_STATUT_COM, m_statut);
    setDataString(data, CP_MAIL_COM, m_mail);
    setDataString(data, CP_TELEPHONE_COM, m_telephone);
    setDataInt(data, CP_IDMANUFACTURER_COM, m_idmanufacturer);

    m_data = data;
}

/*! comme son nom l'indique */
void Commercial::resetdatas()
{
    QJsonObject data;
    data[CP_ID_COM] = 0;
    data[CP_NOM_COM] = "";
    data[CP_PRENOM_COM] = "";
    data[CP_STATUT_COM] = "";
    data[CP_MAIL_COM] = "";
    data[CP_TELEPHONE_COM] = "";
    data[CP_IDMANUFACTURER_COM] = 0;
    setData(data);
}

QString Commercial::nom() const             { return m_nom; }
QString Commercial::telephone() const       { return m_telephone; }
QString Commercial::mail() const            { return m_mail; }
QString Commercial::prenom() const          { return m_prenom; }
QString Commercial::statut() const          { return m_statut; }
int Commercial::idmanufacturer() const      { return m_idmanufacturer; }

QString Commercial::tooltip() const
{
    QString ttip = "";
    if (m_telephone != "")
        ttip += tr("Telephone:") + " " + m_telephone;
    if (nom() != "")
        ttip += "\n" + m_nom.toUpper() + " " + m_prenom;
    return ttip;
}


void Commercial::setnom(const QString &nom)                   { m_nom = nom; m_data[CP_NOM_COM] = nom; }
void Commercial::settelephone(const QString &telephone)       { m_telephone = telephone; m_data[CP_TELEPHONE_COM] = telephone; }
void Commercial::setmail(const QString &mail)                 { m_mail = mail; m_data[CP_MAIL_COM] = mail; }
void Commercial::setprenom(const QString &corprenom)          { m_prenom = corprenom; m_data[CP_PRENOM_COM] = corprenom; }
void Commercial::setstatut(const QString &corstatut)          { m_statut = corstatut; m_data[CP_STATUT_COM] = corstatut; }
void Commercial::setidmanufactureur(int id)                   { m_idmanufacturer = id; m_data[CP_IDMANUFACTURER_COM] = id; }

