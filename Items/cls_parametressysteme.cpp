/* (C) 2020 LAINE SERGE
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
    setDataString(data, CP_MDPADMIN_PARAMSYSTEME, m_mdpdmin);
    setDataInt(data, CP_NUMCENTRE_PARAMSYSTEME, m_numcentre);
    setDataInt(data, CP_IDLIEUPARDEFAUT_PARAMSYSTEME, m_idlieupardefaut);
    setDataBool(data, CP_DOCSCOMPRIMES_PARAMSYSTEME, m_docscomprimes);
    setDataInt(data, CP_VERSIONBASE_PARAMSYSTEME, m_versionbase);
    setDataDouble(data, CP_VERSIONBASEIOL_PARAMSYSTEME, m_versionbaseiol);
    setDataDouble(data, CP_VERSIONCCAM_PARAMSYSTEME, m_versionCCAM);
    setDataDate(data, CP_VERSIONNGAP_PARAMSYSTEME, m_versionNGAP);
    setDataDouble(data, CP_VALEURAMYMETROPOLE_PARAMSYSTEME, m_valeurAMYmetropole);
    setDataDouble(data, CP_VALEURAMYDOM_PARAMSYSTEME, m_valeurAMYDOM);
    setDataString(data, CP_ADRESSELOCALSERVEUR_PARAMSYSTEME, m_adresseserveurlocal);
    setDataString(data, CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME, m_adresseserveurdistant);
    setDataTime(data, CP_HEUREBKUP_PARAMSYSTEME, m_heurebkup);
    setDataString(data, CP_DIRBKUP_PARAMSYSTEME, m_dirbkup);
    m_daysbkup.setFlag(Utils::Lundi,    data[CP_LUNDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Mardi,    data[CP_MARDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Mercredi, data[CP_MERCREDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Jeudi,    data[CP_JEUDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Vendredi, data[CP_VENDREDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Samedi,   data[CP_SAMEDIBKUP_PARAMSYSTEME].toBool());
    m_daysbkup.setFlag(Utils::Dimanche, data[CP_DIMANCHEBKUP_PARAMSYSTEME].toBool());
    setDataBool(data, CP_VILLES_PARAMSYSTEME, m_villesfrance);
    setDataInt(data, CP_COMPTA_PARAMSYSTEME, m_compta);
    setDataBool(data, CP_COTATIONS_PARAMSYSTEME, m_cotationsfrance);
    setDataString(data, CP_VERSION_PARAMSYSTEME, m_version);
    m_data = data;
}

QString ParametresSysteme::mdpadmin() const                     { return m_mdpdmin; }
int ParametresSysteme::numcentre() const                        { return m_numcentre; }
int ParametresSysteme::idlieupardefaut() const                  { return m_idlieupardefaut; }
bool ParametresSysteme::docscomprimes() const                   { return m_docscomprimes; }
int ParametresSysteme::versionbase() const                      { return m_versionbase; }
double ParametresSysteme::versionbaseiol() const                { return m_versionbaseiol; }
double ParametresSysteme::versionCCAM() const                   { return m_versionCCAM; }
QDate ParametresSysteme::versionNGAP() const                    { return m_versionNGAP; }
double ParametresSysteme::valeurAMYmetropole() const            { return m_valeurAMYmetropole; }
double ParametresSysteme::valeurAMYDOM() const                  { return m_valeurAMYDOM; }
QString ParametresSysteme::adresseserveurlocal() const          { return m_adresseserveurlocal; }
QString ParametresSysteme::adresseserveurdistant() const        { return m_adresseserveurdistant; }
Utils::Days ParametresSysteme::daysbkup() const                 { return m_daysbkup; }
QTime ParametresSysteme::heurebkup() const                      { return m_heurebkup; }
QString ParametresSysteme::dirbkup() const                      { return m_dirbkup; }
bool ParametresSysteme::villesfrance() const                    { return m_villesfrance; }
bool ParametresSysteme::cotationsfrance() const                 { return m_cotationsfrance; }
int ParametresSysteme::compta() const                           { return m_compta; }
bool ParametresSysteme::comptanormale() const                   { return m_compta == 1; }
bool ParametresSysteme::comptafrance() const                    { return m_compta == 1  && QLocale::system().country() == QLocale::France; }
bool ParametresSysteme::comptareduite() const                   { return m_compta == 2; }
bool ParametresSysteme::sanscompta() const                      { return m_compta == 3; }
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
void ParametresSysteme::setversionbaseiol(double version)       { m_versionbaseiol = version;
                                                                  m_data[CP_VERSIONBASEIOL_PARAMSYSTEME] = version; }
void ParametresSysteme::setversionCCAM(double version)          { m_versionCCAM = version;
                                                                  m_data[CP_VERSIONCCAM_PARAMSYSTEME] = version; }
void ParametresSysteme::setversionNGAP(QDate date)              { m_versionNGAP = date;
                                                                  m_data[CP_VERSIONNGAP_PARAMSYSTEME] = date.toString("yyyy-MM-dd"); }
void ParametresSysteme::setvaleurAMYmetropole(double valeur)   { m_valeurAMYmetropole = valeur;
                                                                  m_data[CP_VALEURAMYMETROPOLE_PARAMSYSTEME] = valeur; }
void ParametresSysteme::setvaleurAMYDOM(double valeur)         { m_valeurAMYDOM = valeur;
                                                                  m_data[CP_VALEURAMYDOM_PARAMSYSTEME] = valeur; }
void ParametresSysteme::setadresseserveurlocal(QString  adress) { m_adresseserveurlocal = adress;
                                                                  m_data[CP_ADRESSELOCALSERVEUR_PARAMSYSTEME] = adress; }
void ParametresSysteme::setadresseserveurdistant(QString adress){ m_adresseserveurdistant = adress;
                                                                  m_data[CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME] = adress; }
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
void ParametresSysteme::setvillesfrance(bool one)               { m_villesfrance = one;
                                                                  m_data[CP_VILLES_PARAMSYSTEME] = one; }
void ParametresSysteme::setcotationsfrance(bool one)            { m_cotationsfrance = one;
                                                                  m_data[CP_COTATIONS_PARAMSYSTEME] = one; }
void ParametresSysteme::setcompta(int one)                      { m_compta = one;
                                                                  m_data[CP_COMPTA_PARAMSYSTEME] = one; }
void ParametresSysteme::setcomptanormale()                      { m_compta = 1;
                                                                  m_data[CP_COMPTA_PARAMSYSTEME] = 1; }
void ParametresSysteme::setcomptafrance()                       { m_compta = 1;
                                                                  m_data[CP_COMPTA_PARAMSYSTEME] = 1; }
void ParametresSysteme::setcomptareduite()                      { m_compta = 2;
                                                                  m_data[CP_COMPTA_PARAMSYSTEME] = 2; }
void ParametresSysteme::setsanscompta()                         { m_compta = 3;
                                                                  m_data[CP_COMPTA_PARAMSYSTEME] = 2; }

