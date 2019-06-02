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
    setDataInt(data, "id", m_id);
    setDataInt(data, "idacte", m_idacte);
    setDataDate(data, "date", m_date);
    setDataString(data, "payeur", m_nompayeur);
    setDataString(data, "cotationacte", m_actecotation);
    setDataDouble(data, "montant", m_actemontant);
    setDataString(data, "monnaie", m_actemonnaie);
    setDataString(data, "modepaiement" , m_modepaiement);
    setDataString(data, "typetiers", m_typetiers);
    setDataDouble(data, "encaissement", m_paye);
    setDataInt(data, "iduser", m_iduser);
    setDataInt(data, "idparent", m_iduserparent);
    setDataInt(data, "idcomptable", m_idusercomptable);
    setDataDouble(data, "encaissementautrerecette", m_montantautrerecettes);
    setDataBool(data, "apportpraticien", m_isapportparticien);
    setDataBool(data, "autrerecette", m_isautrerecette);
}
