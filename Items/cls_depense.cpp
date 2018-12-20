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

#include "cls_depense.h"

int     Depense::id() const             { return  m_iddepense;}
int     Depense::iduser() const         { return  m_iduser;}
QDate   Depense::date() const           { return  m_datedepepense;}
QString Depense::reffiscale() const     { return  m_referencefiscale;}
QString Depense::objet() const          { return  m_objetdepense;}
double  Depense::montant() const        { return  m_montant;}
QString Depense::famillefiscale() const { return  m_famillefiscale;}
QString Depense::monnaie() const        { return  m_monnaie;}
int     Depense::idrecette() const      { return  m_idRec;}
QString Depense::modepaiement() const   { return  m_modepaiement;}
int     Depense::comptebancaire() const { return  m_compte;}
int     Depense::nocheque() const       { return  m_nocheque;}
int     Depense::idfacture() const      { return  m_idfacture;}
QString Depense::lienfacture() const    { return  m_lienfacture;}
bool    Depense::isecheancier() const   { return  m_echeancier;}
int     Depense::annee()                { return  m_datedepepense.toString("yyyy").toInt();}
int     Depense::isArchivee()           { return  m_auxarchives;}

void    Depense::setArchivee(bool arch)         { m_auxarchives = (arch? Depense::Oui : Depense::Non);}
void    Depense::setidfacture(int idfact)       { m_idfacture = idfact;}
void    Depense::setlienfacture(QString lien)   { m_lienfacture = lien;}
void    Depense::setecheancier(bool ech)        { m_echeancier = ech;}

Depense::Depense(QJsonObject data, QObject *parent) : Item(parent)
{
    m_auxarchives = Depense::NoLoSo;
    setData(data);
}

void Depense::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "iddepense", m_iddepense);
    setDataInt(data, "iduser", m_iduser);
    setDataDate(data, "date", m_datedepepense);
    setDataString(data, "reffiscale", m_referencefiscale);
    setDataString(data, "objet", m_objetdepense);
    setDataDouble(data, "montant", m_montant);
    setDataString(data, "famfiscale", m_famillefiscale);
    setDataString(data, "monnaie", m_monnaie);
    setDataInt(data, "idrecette", m_idRec);
    setDataString(data, "modepaiement", m_modepaiement);
    setDataInt(data, "compte", m_compte);
    setDataInt(data, "nocheque", m_nocheque);
    setDataInt(data, "idfacture", m_idfacture);
    setDataString(data, "lienfacture", m_lienfacture);
    setDataBool(data, "echeancier", m_echeancier);
}


