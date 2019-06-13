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

QDate Acte::date() const                { return m_date; }
QString Acte::motif() const             { return m_motif; }
QString Acte::texte() const             { return m_texte; }
QString Acte::conclusion() const        { return m_conclusion; }
QString Acte::courrierStatus() const    { return m_courrierStatus; }
int Acte::idCreatedBy() const           { return m_idCreatedBy; }
int Acte::idPatient() const             { return m_idPatient; }
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

Acte::Acte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Acte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, CP_IDACTE_ACTES, m_id);
    setDataInt(data, CP_IDUSERCREATEUR_ACTES, m_idCreatedBy);
    setDataInt(data, CP_IDPAT_ACTES, m_idPatient);
    setDataInt(data, CP_IDUSER_ACTES, m_idUser);
    setDataInt(data, CP_IDUSERPARENT_ACTES, m_idUserParent);
    setDataInt(data, CP_IDUSERCOMPTABLE_ACTES, m_idUserComptable);
    setDataInt(data, CP_NUMCENTRE_ACTES, m_numCentre);
    setDataInt(data, CP_IDLIEU_ACTES, m_idLieu);

    setDataDouble(data, CP_MONTANT_ACTES, m_montant);

    setDataString(data, CP_MOTIF_ACTES, m_motif, true);
    setDataString(data, CP_TEXTE_ACTES, m_texte, true);
    setDataString(data, CP_CONCLUSION_ACTES, m_conclusion, true);
    setDataString(data, CP_COURRIERAFAIRE_ACTES, m_courrierStatus);
    setDataString(data, CP_COTATION_ACTES, m_cotation);
    setDataString(data, CP_MONNNAIE_ACTES, m_monnaie);
    setDataString(data, "paiementType", m_paiementType);
    setDataString(data, "paiementTiers", m_paiementTiers);

    setDataDate(data, CP_DATE_ACTES, m_date);
    setDataTime(data, CP_HEURE_ACTES, m_heure);
    setDataBool(data, CP_SUPERVISEURREMPLACANT_ACTES, m_remplacant);
    m_data = data;
}

bool Acte::courrierAFaire() { return m_courrierStatus == "T" || m_courrierStatus == "1"; }
bool Acte::isFactureEnFranc() const { return m_monnaie == "F"; }

