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

#include "cls_manufacturer.h"

Manufacturer::Manufacturer(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Manufacturer::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    Utils::setDataInt(data, CP_ID_MANUFACTURER, m_id);
    Utils::setDataString(data, CP_NOM_MANUFACTURER, m_nom);
    Utils::setDataString(data, CP_ADRESSE1_MANUFACTURER, m_adresse1);
    Utils::setDataString(data, CP_ADRESSE2_MANUFACTURER, m_adresse2);
    Utils::setDataString(data, CP_ADRESSE3_MANUFACTURER, m_adresse3);
    Utils::setDataString(data, CP_CODEPOSTAL_MANUFACTURER, m_codepostal);
    Utils::setDataString(data, CP_VILLE_MANUFACTURER, m_ville);
    Utils::setDataString(data, CP_TELEPHONE_MANUFACTURER, m_telephone);
    Utils::setDataString(data, CP_FAX_MANUFACTURER, m_fax);
    Utils::setDataString(data, CP_PORTABLE_MANUFACTURER, m_portable);
    Utils::setDataString(data, CP_WEBSITE_MANUFACTURER, m_website);
    Utils::setDataString(data, CP_MAIL_MANUFACTURER, m_mail);
    Utils::setDataBool(data, CP_INACTIF_MANUFACTURER, m_inactif);
    Utils::setDataInt(data, CP_DISTRIBUEPAR_MANUFACTURER, m_distributeur);

    m_data = data;
}

/*! comme son nom l'indique */
void Manufacturer::resetdatas()
{
    QJsonObject data;
    data[CP_ID_MANUFACTURER] = 0;
    data[CP_NOM_MANUFACTURER] = "";
    data[CP_ADRESSE1_MANUFACTURER] = "";
    data[CP_ADRESSE2_MANUFACTURER] = "";
    data[CP_ADRESSE3_MANUFACTURER] = "";
    data[CP_CODEPOSTAL_MANUFACTURER] = 0;
    data[CP_VILLE_MANUFACTURER] = "";
    data[CP_TELEPHONE_MANUFACTURER] = "";
    data[CP_FAX_MANUFACTURER] = "";
    data[CP_PORTABLE_MANUFACTURER] = "";
    data[CP_WEBSITE_MANUFACTURER] = "";
    data[CP_MAIL_MANUFACTURER] = "";
    data[CP_INACTIF_MANUFACTURER] = false;
    data[CP_DISTRIBUEPAR_MANUFACTURER] = 0;
    setData(data);
}

QString Manufacturer::nom() const           { return m_nom; }
QString Manufacturer::adresse1() const      { return m_adresse1; }
QString Manufacturer::adresse2() const      { return m_adresse2; }
QString Manufacturer::adresse3() const      { return m_adresse3; }
QString Manufacturer::codepostal() const    { return m_codepostal; }
QString Manufacturer::ville() const         { return m_ville; }
QString Manufacturer::telephone() const     { return m_telephone; }
QString Manufacturer::fax() const           { return m_fax; }
QString Manufacturer::portable() const      { return m_portable; }
QString Manufacturer::website() const       { return m_website; }
QString Manufacturer::mail() const          { return m_mail; }
bool Manufacturer::isactif() const          { return !m_inactif; }
int Manufacturer::iddistributeur() const    { return m_distributeur; }
int Manufacturer::idrufus() const           { return m_idrufus; }

bool Manufacturer::buildIOls() const        { return m_buildIOLS; }

QString Manufacturer::tooltip() const
{
    QString ttip = "";
    if (m_telephone != "")
        ttip += tr("Telephone:") + " " + m_telephone;
    return ttip;
}

QString Manufacturer::coordonnees() const
{
    QString cord = m_nom.toUpper();
    if (m_adresse1 != "" || m_adresse2 != "" || m_adresse3 != "")
    {
        cord += "\n";
        QString adr = "";
        if (m_adresse1 != "")
        {
            adr += m_adresse1;
            if (m_adresse2 != "" || m_adresse3 != "")
                adr += " - ";
        }
        if (m_adresse2 != "")
        {
            adr += m_adresse2;
            if (m_adresse3 != "")
                adr += " - ";
        }
        if (m_adresse3 != "")
            adr += " - " + m_adresse3;
        cord += adr;
    }
    if (m_codepostal!= "" || m_ville != "")
    {
        cord += "\n";
        QString cpville = "";
        if (m_codepostal != "")
        {
            cpville += m_codepostal;
            if (m_ville != "")
                cpville += " - ";
        }
        if (m_ville != "")
            cpville += m_ville.toUpper();
        cord += cpville;
    }
    if (m_telephone != ""|| m_fax != "")
    {
        cord += "\n";
        QString telfax = "";
        if (m_telephone != "")
        {
            telfax += tr("Tel:") + " " + m_telephone;
            if (m_fax != "")
                telfax += " - ";
        }
        if (m_fax != "")
            telfax += tr("Fax:") + " " + m_fax;
        cord += telfax;
    }
    return cord;
}

void Manufacturer::setnom(const QString &nom)                   { m_nom = nom; m_data[CP_NOM_MANUFACTURER] = nom; }
void Manufacturer::setadresse1(const QString &adresse1)         { m_adresse1 = adresse1; m_data[CP_ADRESSE1_MANUFACTURER] = adresse1; }
void Manufacturer::setadresse2(const QString &adresse2)         { m_adresse2 = adresse2; m_data[CP_ADRESSE2_MANUFACTURER] = adresse2; }
void Manufacturer::setadresse3(const QString &adresse3)         { m_adresse3 = adresse3; m_data[CP_ADRESSE3_MANUFACTURER] = adresse3; }
void Manufacturer::setcodepostal(const QString codepostal)      { m_codepostal = codepostal; m_data[CP_CODEPOSTAL_MANUFACTURER] = codepostal; }
void Manufacturer::setville(const QString &ville)               { m_ville = ville; m_data[CP_VILLE_MANUFACTURER] = ville; }
void Manufacturer::settelephone(const QString &telephone)       { m_telephone = telephone; m_data[CP_TELEPHONE_MANUFACTURER] = telephone; }
void Manufacturer::setfax(const QString &fax)                   { m_fax = fax; m_data[CP_FAX_MANUFACTURER] = fax; }
void Manufacturer::setportable(const QString &portable)         { m_portable = portable; m_data[CP_PORTABLE_MANUFACTURER] = portable; }
void Manufacturer::setwebsite(const QString &website)           { m_website = website; m_data[CP_WEBSITE_MANUFACTURER] = website; }
void Manufacturer::setmail(const QString &mail)                 { m_mail = mail; m_data[CP_MAIL_MANUFACTURER] = mail; }
void Manufacturer::setactif(const bool &actif)                  { m_inactif = !actif; m_data[CP_INACTIF_MANUFACTURER] = !actif; }
void Manufacturer::setiddistributeur(int id)                    { m_distributeur = id; m_data[CP_DISTRIBUEPAR_MANUFACTURER] = id; }
void Manufacturer::setidrufus(int id)                           { m_idrufus = id; m_data[CP_IDRUFUS_MANUFACTURER] = id; }

void Manufacturer::setbuildIOLS(bool build)                     { m_buildIOLS = build; }


QString Manufacturer::adresseComplete() const
{
    QString ttip;
    if (m_adresse1!= "") ttip += m_adresse1;
    if (m_adresse2 != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_adresse2;
    }
    if (m_adresse3 != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_adresse3;
    }
    if (m_codepostal + m_ville != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_codepostal;
        if (m_codepostal != "" && m_ville != "")
            ttip += " ";
        ttip += m_ville;
    }
    if (m_telephone != "")
    {
        if (ttip != "") ttip += "\n";
        ttip += m_telephone;
    }
    return ttip;
}
