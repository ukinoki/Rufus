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


#include "cls_paiementtiers.h"

PaiementTiers::PaiementTiers(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void PaiementTiers::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, "id", m_idpaiement);
    setDataDate(data, "date", m_date);
    setDataDate(data, "dateenregistrement", m_dateenregistrement);
    setDataDouble(data, "montant", m_montant);
    setDataString(data, "modepaiement", m_modepaiement);
    setDataString(data, "tireurcheque", m_tireurcheque);
    setDataInt(data, "comptevirement", m_comptevirement);
    setDataString(data, "banquecheque", m_banquecheque);
    setDataString(data, "nomtiers", m_nomtiers);
    setDataDouble(data, "commission", m_commission);
    setDataString(data, "monnaie", m_monnaie);
    setDataInt(data, "idremisecheque", m_idremisecheque);
    setDataBool(data, "chequeenattente", m_chequeenattente);
    setDataInt(data, "iduserenregistreur", m_iduserenregistreur);
    setDataDate(data, "dateremisecheques", m_dateremisecheques);
    setDataDouble(data, "encaissement", m_encaissement);

}

int PaiementTiers::id() const                       { return m_idpaiement; }
QDate PaiementTiers::date() const                   { return m_date; }
QDate PaiementTiers::dateenregistrement() const     { return m_dateenregistrement; }
double PaiementTiers::montant() const               { return m_montant; }
QString PaiementTiers::modepaiement() const         { return m_modepaiement; }
QString PaiementTiers::tireurcheque() const         { return m_tireurcheque; }
int PaiementTiers::comptevirement() const           { return m_comptevirement; }
QString PaiementTiers::banquecheque() const         { return m_banquecheque; }
QString PaiementTiers::nomtiers() const             { return m_nomtiers; }
double PaiementTiers::commission() const            { return m_commission; }
QString PaiementTiers::monnaie() const              { return m_monnaie; }
int PaiementTiers::idremisecheque() const           { return m_idremisecheque; }
bool PaiementTiers::ischequeenattente() const       { return m_chequeenattente; }
int PaiementTiers::iduserenregistreur() const       { return m_iduserenregistreur; }
QDate PaiementTiers::dateremisecheques() const      { return m_dateremisecheques; }
double PaiementTiers::encaissement() const          { return m_encaissement; }
