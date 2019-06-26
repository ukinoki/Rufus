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

#include "cls_parametressysteme.h"

ParametresSysteme::ParametresSysteme(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void ParametresSysteme::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataString(data, "mdpadmin", m_mdpdmin);
    setDataInt(data, "numcentre", m_numcentre);
    setDataInt(data, "idlieupardefaut", m_idlieupardefaut);
    setDataBool(data, "docscomprimes", m_docscomprimes);
    setDataInt(data, "versionbase", m_versionbase);
    setDataBool(data, "aveccompta", m_aveccompta);
    setDataString(data, "adresseserveurlocal", m_adresseserveurlocal);
    setDataString(data, "adresseserveurdistant", m_adresseserveurdistant);
    setDataString(data, "adresseserveurtcp", m_adresseserveurtcp);
    setDataString(data, "dirimagerie", m_dirimagerie);
    setDataBool(data, "lundibkup", m_lundibkup);
    setDataBool(data, "mardibkup", m_mardibkup);
    setDataBool(data, "mercredibkup", m_credibkup);
    setDataBool(data, "jeudibkup", m_jeudibkup);
    setDataBool(data, "vendredibkup", m_dredibkup);
    setDataBool(data, "samedibkup", m_medibkup);
    setDataBool(data, "dimanchebkup", m_dimanchebkup);
    setDataTime(data, "heurebkup", m_heurebkup);
    setDataString(data, "dirbkup", m_dirbkup);
}

QString ParametresSysteme::mdpadmin() const                 { return m_mdpdmin; }
int ParametresSysteme::numcentre() const                    { return m_numcentre; }
int ParametresSysteme::idlieupardefaut() const              { return m_idlieupardefaut; }
bool ParametresSysteme::docscomprimes() const               { return m_docscomprimes; }
int ParametresSysteme::versionbase() const                  { return m_versionbase; }
bool ParametresSysteme::aveccompta() const                  { return m_aveccompta; }
QString ParametresSysteme::adresseserveurlocal() const      { return m_adresseserveurlocal; }
QString ParametresSysteme::adresseserveurdistant() const    { return m_adresseserveurdistant; }
QString ParametresSysteme::adresseserveurtcp() const        { return m_adresseserveurtcp; }
QString ParametresSysteme::dirimagerie() const              { return m_dirimagerie; }
bool ParametresSysteme::lundibkup() const                   { return m_lundibkup; }
bool ParametresSysteme::mardibkup() const                   { return m_mardibkup; }
bool ParametresSysteme::mercredibkup() const                { return m_credibkup; }
bool ParametresSysteme::jeudibkup() const                   { return m_jeudibkup; }
bool ParametresSysteme::vendredibkup() const                { return m_dredibkup; }
bool ParametresSysteme::samedibkup() const                  { return m_medibkup; }
bool ParametresSysteme::dimanchebkup() const                { return m_dimanchebkup; }
QTime ParametresSysteme::heurebkup() const                  { return m_heurebkup; }
QString ParametresSysteme::dirbkup() const                  { return m_dirbkup; }

void ParametresSysteme::setmdpadmin(QString mdp)                { m_mdpdmin = mdp; }
void ParametresSysteme::setnumcentre(int id)                    { m_numcentre = id; }
void ParametresSysteme::setidlieupardefaut(int id)              { m_idlieupardefaut = id; }
void ParametresSysteme::setdocscomprimes(bool one)              { m_docscomprimes = one; }
void ParametresSysteme::setversionbase(int version)             { m_versionbase = version; }
void ParametresSysteme::setaveccompta(bool one)                 { m_aveccompta = one; }
void ParametresSysteme::setadresseserveurlocal(QString  adress) { m_adresseserveurlocal = adress; }
void ParametresSysteme::setadresseserveurdistant(QString adress){ m_adresseserveurdistant = adress; }
void ParametresSysteme::setadresseserveurtcp(QString adress)    { m_adresseserveurtcp = adress; }
void ParametresSysteme::setdirimagerie(QString adress)          { m_dirimagerie = adress; }
void ParametresSysteme::setlundibkup(bool one)                  { m_lundibkup = one; }
void ParametresSysteme::setmardibkup(bool one)                  { m_mardibkup = one; }
void ParametresSysteme::setmercredibkup(bool one)               { m_credibkup = one; }
void ParametresSysteme::setjeudibkup(bool one)                  { m_jeudibkup = one; }
void ParametresSysteme::setvendredibkup(bool one)               { m_dredibkup = one; }
void ParametresSysteme::setsamedibkup(bool one)                 { m_medibkup = one; }
void ParametresSysteme::setdimanchebkup(bool one)               { m_dimanchebkup = one; }
void ParametresSysteme::setheurebkup(QTime time)                { m_heurebkup = time; }
void ParametresSysteme::setdirbkup(QString adress)              { m_dirbkup = adress; }
