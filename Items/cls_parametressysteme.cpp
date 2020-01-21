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
    Utils::setDataString(data, CP_MDPADMIN_PARAMSYSTEME, m_mdpdmin);
    Utils::setDataInt(data, CP_NUMCENTRE_PARAMSYSTEME, m_numcentre);
    Utils::setDataInt(data, CP_IDLIEUPARDEFAUT_PARAMSYSTEME, m_idlieupardefaut);
    Utils::setDataBool(data, CP_DOCSCOMPRIMES_PARAMSYSTEME, m_docscomprimes);
    Utils::setDataInt(data, CP_VERSIONBASE_PARAMSYSTEME, m_versionbase);
    Utils::setDataBool(data, CP_SANSCOMPTA_PARAMSYSTEME, m_sanscompta);
    Utils::setDataString(data, CP_ADRESSELOCALSERVEUR_PARAMSYSTEME, m_adresseserveurlocal);
    Utils::setDataString(data, CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME, m_adresseserveurdistant);
    Utils::setDataString(data, CP_DIRIMAGERIE_PARAMSYSTEME, m_dirimagerieserveur);
    Utils::setDataTime(data, CP_HEUREBKUP_PARAMSYSTEME, m_heurebkup);
    Utils::setDataString(data, CP_DIRBKUP_PARAMSYSTEME, m_dirbkup);
    m_daysbkup.setFlag(Utils::Lundi,    data[CP_LUNDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Mardi,    data[CP_MARDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Mercredi, data[CP_MERCREDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Jeudi,    data[CP_JEUDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Vendredi, data[CP_VENDREDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Samedi,   data[CP_SAMEDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Dimanche, data[CP_DIMANCHEBKUP_PARAMSYSTEME].toBool());
    m_data = data;
}

QString ParametresSysteme::mdpadmin() const                     { return m_mdpdmin; }
int ParametresSysteme::numcentre() const                        { return m_numcentre; }
int ParametresSysteme::idlieupardefaut() const                  { return m_idlieupardefaut; }
bool ParametresSysteme::docscomprimes() const                   { return m_docscomprimes; }
int ParametresSysteme::versionbase() const                      { return m_versionbase; }
bool ParametresSysteme::sanscompta() const                      { return m_sanscompta; }
QString ParametresSysteme::adresseserveurlocal() const          { return m_adresseserveurlocal; }
QString ParametresSysteme::adresseserveurdistant() const        { return m_adresseserveurdistant; }
QString ParametresSysteme::dirimagerieserveur() const           { return m_dirimagerieserveur; }
Utils::Days ParametresSysteme::daysbkup() const                 { return m_daysbkup; }
QTime ParametresSysteme::heurebkup() const                      { return m_heurebkup; }
QString ParametresSysteme::dirbkup() const                      { return m_dirbkup; }

void ParametresSysteme::setmdpadmin(QString mdp)                { m_mdpdmin = mdp;
                                                                  m_data[CP_MDPADMIN_PARAMSYSTEME] = mdp; }
void ParametresSysteme::setnumcentre(int id)                    { m_numcentre = id;
                                                                  m_data[CP_NUMCENTRE_PARAMSYSTEME] = id; }
void ParametresSysteme::setidlieupardefaut(int id)              { m_idlieupardefaut = id;
                                                                  m_data[CP_IDLIEUPARDEFAUT_PARAMSYSTEME] = id; }
void ParametresSysteme::setdocscomprimes(bool one)              { m_docscomprimes = one;
                                                                  m_data[CP_DOCSCOMPRIMES_PARAMSYSTEME] = one; }
void ParametresSysteme::setversionbase(int version)             { m_versionbase = version;
                                                                  m_data[CP_VERSIONBASE_PARAMSYSTEME] = version; }
void ParametresSysteme::setsanscompta(bool one)                 { m_sanscompta = one;
                                                                  m_data[CP_SANSCOMPTA_PARAMSYSTEME] = one; }
void ParametresSysteme::setadresseserveurlocal(QString  adress) { m_adresseserveurlocal = adress;
                                                                  m_data[CP_ADRESSELOCALSERVEUR_PARAMSYSTEME] = adress; }
void ParametresSysteme::setadresseserveurdistant(QString adress){ m_adresseserveurdistant = adress;
                                                                  m_data[CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME] = adress; }
void ParametresSysteme::setdirimagerieserveur(QString adress)   { m_dirimagerieserveur = adress;
                                                                  m_data[CP_DIRIMAGERIE_PARAMSYSTEME] = adress; }
void ParametresSysteme::setheurebkup(QTime time)                { m_heurebkup = time;
                                                                  m_data[CP_HEUREBKUP_PARAMSYSTEME] = time.toString("HH:mm:ss"); }
void ParametresSysteme::setdirbkup(QString adress)              { m_dirbkup = adress;
                                                                  m_data[CP_DIRBKUP_PARAMSYSTEME] = adress; }
void ParametresSysteme::setdaysbkup(Utils::Days days)           { m_daysbkup = days;
                                                                  m_data[CP_LUNDIBKUP_PARAMSYSTEME] = days.testFlag(Utils::Lundi);
                                                                  m_data[CP_MARDIBKUP_PARAMSYSTEME] = days.testFlag(Utils::Mardi);
                                                                  m_data[CP_MERCREDIBKUP_PARAMSYSTEME] = days.testFlag(Utils::Mercredi);
                                                                  m_data[CP_JEUDIBKUP_PARAMSYSTEME] = days.testFlag(Utils::Jeudi);
                                                                  m_data[CP_VENDREDIBKUP_PARAMSYSTEME] = days.testFlag(Utils::Vendredi);
                                                                  m_data[CP_SAMEDIBKUP_PARAMSYSTEME] = days.testFlag(Utils::Samedi);
                                                                  m_data[CP_DIMANCHEBKUP_PARAMSYSTEME] = days.testFlag(Utils::Dimanche); }
