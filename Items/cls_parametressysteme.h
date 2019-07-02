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

#ifndef CLS_PARAMETRESSYSTEME_H
#define CLS_PARAMETRESSYSTEME_H


#include "cls_item.h"

/*!
 * \brief ParametresSysteme class
 * l'ensemble des parametres systeme
 */

class ParametresSysteme : public Item
{

private:
    QString m_mdpdmin;                  //!> le mdp administrateur
    int m_numcentre;                    //!> l'id de l'installation de la base de données - spécifique d'une base Rufus - pas utilisé pour le moment - fixé à 1
    int m_idlieupardefaut;              //!> l'id du lieu principal de l'installation, où se trouve le serveur
    bool m_docscomprimes;               //!> utilise la compression des pdf - pas utilisé
    int m_versionbase;                  //!> la version de la base de données
    bool m_aveccompta;                  //!> utilise ou non la compta
    QString m_adresseserveurlocal;      //!> l'adresse du serveur dans le réseau local
    QString m_adresseserveurdistant;    //!> l'adresse IP ou DNS du lieu où se trouve le serveur
    QString m_dirimagerie;              //!> adresse du dossier d'imagerie depuis le serveur
    bool m_lundibkup;                   //!> sauvegarde le lundi
    bool m_mardibkup;                   //!> sauvegarde le mardi
    bool m_credibkup;                   //!> sauvegarde le credi
    bool m_jeudibkup;                   //!> sauvegarde le jeudi
    bool m_dredibkup;                   //!> sauvegarde le dredi
    bool m_medibkup;                    //!> sauvegarde le medi
    bool m_dimanchebkup;                //!> sauvegarde le dimanche
    QTime m_heurebkup;                  //!> heure de la sauvegarde
    QString m_dirbkup;                  //!> adresse du dossier de sauvegarde depuis le serveur

public:
    explicit ParametresSysteme(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString mdpadmin() const;
    int numcentre() const;
    int idlieupardefaut() const;
    bool docscomprimes() const;
    int versionbase() const;
    bool aveccompta() const;
    QString adresseserveurlocal() const;
    QString adresseserveurdistant() const;
    QString dirimagerie() const;
    bool lundibkup() const;
    bool mardibkup() const;
    bool mercredibkup() const;
    bool jeudibkup() const;
    bool vendredibkup() const;
    bool samedibkup() const;
    bool dimanchebkup() const;
    QTime heurebkup() const;
    QString dirbkup() const;

    void setmdpadmin(QString mdp);
    void setnumcentre(int id);
    void setidlieupardefaut(int id);
    void setdocscomprimes(bool one);
    void setversionbase(int version);
    void setaveccompta(bool one);
    void setadresseserveurlocal(QString  adress);
    void setadresseserveurdistant(QString adress);
    void setdirimagerie(QString adress);
    void setlundibkup(bool one);
    void setmardibkup(bool one);
    void setmercredibkup(bool one);
    void setjeudibkup(bool one);
    void setvendredibkup(bool one);
    void setsamedibkup(bool one);
    void setdimanchebkup(bool one);
    void setheurebkup(QTime time);
    void setdirbkup(QString adress);
};

#endif // CLS_PARAMETRESSYSTEME_H
