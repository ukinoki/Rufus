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


#include "cls_depense.h"

int         Depense::iduser() const             { return  m_iduser;}
QDate       Depense::date() const               { return  m_datedepepense;}
QString     Depense::rubriquefiscale() const    { return  m_rubriquefiscale;}
int         Depense::idrubriquefiscale() const  { return  m_idrubriquefiscale;}
QString     Depense::objet() const              { return  m_objetdepense;}
double      Depense::montant() const            { return  m_montant;}
QString     Depense::famillefiscale() const     { return  m_famillefiscale;}
QString     Depense::monnaie() const            { return  m_monnaie;}
int         Depense::idrecette() const          { return  m_idRec;}
QString     Depense::modepaiement() const       { return  m_modepaiement;}
int         Depense::comptebancaire() const     { return  m_compte;}
int         Depense::nocheque() const           { return  m_nocheque;}
int         Depense::idfacture() const          { return  m_idfacture;}
QString     Depense::lienfacture() const        { return  m_lienfacture;}
bool        Depense::isecheancier() const       { return  m_echeancier;}
QString     Depense::objetecheancier() const    { return  m_objetecheancier;}
int         Depense::annee() const              { return  m_datedepepense.toString("yyyy").toInt();}
int         Depense::isArchivee() const         { return  m_auxarchives;}
QString     Depense::factureformat() const      { return  m_formatfacture;}
QByteArray  Depense::factureblob() const        { return  m_blob;}

Depense::Depense(QJsonObject data, QObject *parent) : Item(parent)
{
    m_auxarchives = Depense::NoLoSo;
    m_formatfacture = "";
    m_blob = QByteArray();
    setData(data);
}

void Depense::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data,    CP_ID_DEPENSES,      m_id);
    Utils::setDataInt(data,    CP_IDUSER_DEPENSES,         m_iduser);
    Utils::setDataDate(data,   CP_DATE_DEPENSES,           m_datedepepense);
    Utils::setDataString(data, CP_REFFISCALE_DEPENSES,     m_rubriquefiscale);
    Utils::setDataString(data, CP_OBJET_DEPENSES,          m_objetdepense);
    Utils::setDataDouble(data, CP_MONTANT_DEPENSES,        m_montant);
    Utils::setDataString(data, CP_FAMILLEFISCALE_DEPENSES, m_famillefiscale);
    Utils::setDataString(data, CP_MONNAIE_DEPENSES,        m_monnaie);
    Utils::setDataInt(data,    CP_IDRECETTE_DEPENSES,      m_idRec);
    Utils::setDataString(data, CP_MODEPAIEMENT_DEPENSES,   m_modepaiement);
    Utils::setDataInt(data,    CP_COMPTE_DEPENSES,         m_compte);
    Utils::setDataInt(data,    CP_NUMCHEQUE_DEPENSES,      m_nocheque);
    Utils::setDataInt(data,    CP_IDFACTURE_DEPENSES,      m_idfacture);
    Utils::setDataString(data, CP_LIENFICHIER_FACTURES,    m_lienfacture);
    Utils::setDataBool(data,   CP_ECHEANCIER_FACTURES,     m_echeancier);
    Utils::setDataString(data, CP_INTITULE_FACTURES,       m_objetecheancier);
    Utils::setDataInt(data,    "idrubrique",               m_idrubriquefiscale);
    m_data = data;
}


