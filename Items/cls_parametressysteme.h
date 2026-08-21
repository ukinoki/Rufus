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

#ifndef CLS_PARAMETRESSYSTEME_H
#define CLS_PARAMETRESSYSTEME_H


#include "cls_item.h"
#include "utils.h"

/*!
 * \brief ParametresSysteme class
 * l'ensemble des parametres systeme
 */

class ParametresSysteme : public Item
{
    Q_OBJECT
private:
    QString m_mdpdmin = "";                 //!> le mdp administrateur
    int m_numcentre = 0;                    //!> l'id de l'installation de la base de données - spécifique d'une base Rufus - pas utilisé pour le moment - fixé à 1
    int m_idlieupardefaut = 0;              //!> l'id du lieu principal de l'installation, où se trouve le serveur
    bool m_docscomprimes = false;           //!> utilise la compression des pdf - pas utilisé
    int m_versionbase = 0;                  //!> la version de la base de données
    double m_versionbaseiol = 1;            //!> la version de la base de données des IOL
    double m_versionCCAM = 77;              //!> la version de la nomenclature CCAM chargée
    QDate m_versionNGAP = QDate(2026,7,1);  //!> la date de la version de la NGAP chargée
    double m_valeurAMYmetropole = 2.60;     //!> la valeur de la lettre-clé AMY en métropole
    double m_valeurAMYDOM = 2.72;           //!> la valeur de la lettre-clé AMY dans les DOM
    int m_compta = 1;                       //!> gestion de la comptabilité (1 = normale, 2 = réduite, 3 = aucune)
    QString m_adresseserveurlocal = "";     //!> l'adresse IP du serveur dans le réseau local
    QString m_adresseserveurdistant = "";   //!> l'adresse IP ou DNS du lieu où se trouve le serveur
    Utils::Days m_daysbkup;                 //!> flag énumérant les jours de la sauvegarde
    QTime m_heurebkup = QTime(0,0);         //!> l'heure de la sauvegarde
    QString m_dirbkup = "";                 //!> l'adresse du dossier de sauvegarde vue depuis le serveur
    bool m_villesfrance = true;             //!> utilise la base de données des villes françaises
    bool m_cotationsfrance = true;          //!> utilise les cotations d'actes françaises (CMU, ALD, CCAM, secteur conventionnel, OPTAM...etc...)
    QString m_version = "FR";               //!> la version du logiciel
    QString m_country = QLocale::territoryToCode(QLocale::system().territory());               //!> le pays où s'éxécute le programme - code pays ISO 3166-1 alpha-2 (FR, ES, MX, CL...)

public:
    explicit ParametresSysteme(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data) override;

    QString mdpadmin() const;
    int numcentre() const;
    int idlieupardefaut() const;
    bool docscomprimes() const;
    int versionbase() const;
    double versionbaseiol() const;
    double versionCCAM() const;
    QDate versionNGAP() const;
    double valeurAMYmetropole() const;
    double valeurAMYDOM() const;
    int compta() const;
    QString adresseserveurlocal() const;
    QString adresseserveurdistant() const;
    QString dirimagerieserveur() const;
    Utils::Days daysbkup() const;
    QTime heurebkup() const;
    QString dirbkup() const;
    bool villesfrance() const;
    bool cotationsfrance() const;
    bool comptanormale() const;
    bool comptareduite() const;
    bool sanscompta() const;
    bool comptafrance() const;
    QString version()                       { return m_version; }
    QString country()                       { return m_country; }

    void setmdpadmin(QString mdp);
    void setnumcentre(int id);
    void setidlieupardefaut(int id);
    void setdocscomprimes(bool one);
    void setversionbase(int version);
    void setversionbaseiol(double version);
    void setversionCCAM(double version);
    void setversionNGAP(QDate date);
    void setvaleurAMYmetropole(double valeur);
    void setvaleurAMYDOM(double valeur);
    void setcompta(int one);
    void setcomptanormale();
    void setcomptareduite();
    void setsanscompta();
    void setcomptafrance();
    void setadresseserveurlocal(QString  adress);
    void setadresseserveurdistant(QString adress);
    void setdirbkup(QString adress);
    void setdaysbkup(Utils::Days days);
    void setheurebkup(QTime time);
    void setvillesfrance(bool one);
    void setcotationsfrance(bool one);
    void setversion(QString version)        { m_version = version; }
    void setpays(QString country)           { m_country = country; }
};

#endif // CLS_PARAMETRESSYSTEME_H
