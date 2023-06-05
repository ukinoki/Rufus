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

#ifndef CLS_TIERS_H
#define CLS_TIERS_H

#include "cls_item.h"

/*!
 * \brief Tiers class
 * l'ensemble des informations concernant un tiers payant
 *!
 * \brief TypeTiers class
 * non d'un type de tiers payants - utilisée pour les paiements directs
 *  - utilisée pour les paiements directs
 * AME, ACS, CMU, etc..
 * géré par la table ComtaMedicale.tiers
 */


class Tiers : public Item
{
    Q_OBJECT
private:
    QString m_nom = "";
    QString m_adresse1 = "";
    QString m_adresse2 = "";
    QString m_adresse3 = "";
    QString m_ville = "";
    QString m_codepostal = "";
    QString m_telephone = "";
    QString m_fax = "";
    QString m_mail = "";
    QString m_web = "";
    bool m_utilise;
public:
    explicit Tiers(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString nom() const;
    QString codepostal() const;
    QString ville() const;
    QString telephone() const;
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    QString fax() const;
    QString mail() const;
    QString website() const;
    bool isutilise() const { return m_utilise; }
    void setNom(const QString &nom)               { m_nom = nom;                m_data[CP_NOM_TIERS]        = nom;}
    void setAdresse1(const QString &adresse)      { m_adresse1 = adresse;       m_data[CP_ADRESSE1_TIERS]    = adresse;}
    void setAdresse2(const QString &adresse)      { m_adresse2 = adresse;       m_data[CP_ADRESSE2_TIERS]    = adresse;}
    void setAdresse3(const QString &adresse)      { m_adresse3 = adresse;       m_data[CP_ADRESSE3_TIERS]    = adresse;}
    void setVille(const QString &ville)           { m_ville = ville;            m_data[CP_CODEPOSTAL_TIERS] = ville;}
    void setCodepostal(const QString &codepostal) { m_codepostal = codepostal;  m_data[CP_VILLE_TIERS]      = codepostal;}
    void setTelephone(const QString &telephone)   { m_telephone = telephone;    m_data[CP_TELEPHONE_TIERS]  = telephone;}
    void setFax(const QString &fax)               { m_fax = fax;                m_data[CP_FAX_TIERS]  = fax;}
    void setMail(const QString &mail)             { m_mail = mail;              m_data[CP_MAIL_TIERS]  = mail;}
    void setWebsite(const QString &web)           { m_web = web;                m_data[CP_WEB_TIERS]  = web;}
    void setUtilise(const bool isutilise)         { m_utilise = isutilise; }
    void resetdatas();
    bool isnull() const                           { return m_id == 0; }
    QString tooltip() const;
};

class TypeTiers : public Item
{

private:
    QString m_typetiers;
public:
    explicit TypeTiers(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString typetiers() const;
};


#endif // CLS_TIERS_H
