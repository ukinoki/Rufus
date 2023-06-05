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
    bool m_sanscompta = true;               //!> utilise ou non la compta
    QString m_adresseserveurlocal = "";     //!> l'adresse IP du serveur dans le réseau local
    QString m_adresseserveurdistant = "";   //!> l'adresse IP ou DNS du lieu où se trouve le serveur
    QString m_dirimagerieserveur = "";      //!> l'adresse du dossier d'imagerie vue depuis le serveur
    Utils::Days m_daysbkup;                 //!> flag énumérant les jours de la sauvegarde
    QTime m_heurebkup = QTime(0,0);         //!> l'heure de la sauvegarde
    QString m_dirbkup = "";                 //!> l'adresse du dossier de sauvegarde vue depuis le serveur
    bool m_villesfrance = true;             //!> utilise la base de données des villes françaises
    bool m_cotationsfrance = true;          //!> utilise les cotations d'actes françaises (CMU, ALD, CCAM, secteur conventionnel, OPTAM...etc...)
    bool m_comptafrance = true;             //!> utilise la comptabilité française

public:
    explicit ParametresSysteme(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString mdpadmin() const;
    int numcentre() const;
    int idlieupardefaut() const;
    bool docscomprimes() const;
    int versionbase() const;
    bool sanscompta() const;
    QString adresseserveurlocal() const;
    QString adresseserveurdistant() const;
    QString dirimagerieserveur() const;
    Utils::Days daysbkup() const;
    QTime heurebkup() const;
    QString dirbkup() const;
    bool villesfrance() const;
    bool cotationsfrance() const;
    bool comptafrance() const;

    void setmdpadmin(QString mdp);
    void setnumcentre(int id);
    void setidlieupardefaut(int id);
    void setdocscomprimes(bool one);
    void setversionbase(int version);
    void setsanscompta(bool one);
    void setadresseserveurlocal(QString  adress);
    void setadresseserveurdistant(QString adress);
    void setdirimagerieserveur(QString adress);
    void setdirbkup(QString adress);
    void setdaysbkup(Utils::Days days);
    void setheurebkup(QTime time);
    void setvillesfrance(bool one);
    void setcotationsfrance(bool one);
    void setcomptafrance(bool one);
};

#endif // CLS_PARAMETRESSYSTEME_H
