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

#include "cls_recette.h"

Recette::Recette(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

int Recette::idacte() const                         { return m_idacte; }
QDate Recette::date() const                         { return m_date; }
QString Recette::payeur() const                     { return m_nompayeur; }
QString Recette::cotationacte() const               { return m_actecotation; }
double Recette::montant() const                     { return m_actemontant; }
QString Recette::monnaie() const                    { return m_actemonnaie; }
QString Recette::modepaiement() const               { return m_modepaiement; }
QString Recette::typetiers() const                  { return m_typetiers; }
double Recette::encaissement() const                { return m_paye; }
int Recette::iduser() const                         { return m_iduser; }
int Recette::idparent() const                       { return m_iduserparent; }
int Recette::idcomptable() const                    { return m_idusercomptable; }
double Recette::encaissementautrerecette() const    { return m_montantautrerecettes; }
bool Recette::isautrerecette()                      { return m_isautrerecette; }
bool Recette::isapportpraticien()                   { return m_isapportparticien; }

void Recette::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, "id", m_id);
    Utils::setDataInt(data, "idacte", m_idacte);
    Utils::setDataDate(data, "date", m_date);
    Utils::setDataString(data, "payeur", m_nompayeur);
    Utils::setDataString(data, "cotationacte", m_actecotation);
    Utils::setDataDouble(data, "montant", m_actemontant);
    Utils::setDataString(data, "monnaie", m_actemonnaie);
    Utils::setDataString(data, "modepaiement" , m_modepaiement);
    Utils::setDataString(data, "typetiers", m_typetiers);
    Utils::setDataDouble(data, "encaissement", m_paye);
    Utils::setDataInt(data, "iduser", m_iduser);
    Utils::setDataInt(data, "idparent", m_iduserparent);
    Utils::setDataInt(data, "idcomptable", m_idusercomptable);
    Utils::setDataDouble(data, "encaissementautrerecette", m_montantautrerecettes);
    Utils::setDataBool(data, "apportpraticien", m_isapportparticien);
    Utils::setDataBool(data, "autrerecette", m_isautrerecette);
    m_data = data;
}
