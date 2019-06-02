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

#include "cls_acte.h"

QDate Acte::date() const                { return m_date.date(); }
QString Acte::motif() const             { return m_motif; }
QString Acte::texte() const             { return m_texte; }
QString Acte::conclusion() const        { return m_conclusion; }
QString Acte::courrierStatus() const    { return m_courrierStatus; }
int Acte::idCreatedBy() const           { return m_idCreatedBy; }
int Acte::idPatient() const             { return m_idPatient; }
QDate Acte::agePatient() const          { return m_agePatient.date(); }
QString Acte::cotation() const          { return m_cotation; }
double Acte::montant() const            { return (isFactureEnFranc()? m_montant / 6.55957 : m_montant); }
QString Acte::paiementType() const      { return m_paiementType; }
QString Acte::paiementTiers() const     { return m_paiementTiers; }
int Acte::idUser() const                { return m_idUser; }
int Acte::idParent() const              { return m_idUserParent; }
int Acte::idComptable() const           { return m_idUserComptable; }
int Acte::numcentre() const             { return m_numCentre; }
int Acte::idlieu() const                { return m_idLieu; }
QTime Acte::heure() const               { return m_heure; }
bool Acte::effectueparremplacant() const{ return m_remplacant; }
QJsonObject Acte::datas() const         { return m_data; }

Acte::Acte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Acte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);
    setDataInt(data, "idCreatedBy", m_idCreatedBy);
    setDataInt(data, "idPatient", m_idPatient);
    setDataInt(data, "idUser", m_idUser);
    setDataInt(data, "idUserParent", m_idUserParent);
    setDataInt(data, "idUserComptable", m_idUserComptable);
    setDataInt(data, "NumCentre", m_numCentre);
    setDataInt(data, "idLieu", m_idLieu);

    setDataDouble(data, "montant", m_montant);

    setDataString(data, "motif", m_motif, true);
    setDataString(data, "texte", m_texte, true);
    setDataString(data, "conclusion", m_conclusion, true);
    setDataString(data, "courrierStatus", m_courrierStatus);
    setDataString(data, "cotation", m_cotation);
    setDataString(data, "monnaie", m_monnaie);
    setDataString(data, "paiementType", m_paiementType);
    setDataString(data, "paiementTiers", m_paiementTiers);

    setDataDateTime(data, "date", m_date);
    setDataDateTime(data, "agePatient", m_agePatient);
    setDataTime(data, "heure", m_heure);
    setDataBool(data, "remplacant", m_remplacant);
    m_data = data;
}

bool Acte::courrierAFaire() { return m_courrierStatus == "T" || m_courrierStatus == "1"; }
bool Acte::isFactureEnFranc() const { return m_monnaie == "F"; }

