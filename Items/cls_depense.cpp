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

int         Depense::id() const                 { return  m_iddepense;}
int         Depense::iduser() const             { return  m_iduser;}
QDate       Depense::date() const               { return  m_datedepepense;}
QString     Depense::rubriquefiscale() const    { return  m_rubriquefiscale;}
int         Depense::idrubriquefiscale() const  { return m_idrubriquefiscale;}
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
QString     Depense::pdfoujpgfacture() const    { return  m_pdfoujpgfacture;}
QByteArray  Depense::imgfacture() const         { return  m_imgfacture;}

void    Depense::setArchivee(bool arch)             { m_auxarchives = (arch? Depense::Oui : Depense::Non);}
void    Depense::setidfacture(int idfact)           { m_idfacture = idfact;}
void    Depense::setlienfacture(QString lien)       { m_lienfacture = lien;}
void    Depense::setecheancier(bool ech)            { m_echeancier = ech;}
void    Depense::setobjetecheancier(QString obj)    { m_objetecheancier = obj;}
void    Depense::setpdfoujpgfacture(QString typeimg){ m_pdfoujpgfacture = typeimg;}
void    Depense::setimgfacture(QByteArray ba)       { m_imgfacture = ba;}

Depense::Depense(QJsonObject data, QObject *parent) : Item(parent)
{
    m_auxarchives = Depense::NoLoSo;
    m_pdfoujpgfacture = "";
    m_imgfacture  = QByteArray();
    setData(data);
}

void Depense::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "iddepense", m_iddepense);
    setDataInt(data, "iduser", m_iduser);
    setDataDate(data, "date", m_datedepepense);
    setDataString(data, "reffiscale", m_rubriquefiscale);
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
    setDataString(data, "objetecheancier", m_objetecheancier);
    setDataInt(data, "idrubrique", m_idrubriquefiscale);
}


