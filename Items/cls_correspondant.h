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

#ifndef CLS_CORRESPONDANT_H
#define CLS_CORRESPONDANT_H

#include "cls_item.h"

/*!
 * \brief The correspondant class
 * l'ensemble des informations concernant un correspondant
 */

class Correspondant : public Item
{
    Q_OBJECT
private: //Données du correspondant
    QString m_nom;                  //!< Nom du correspondant
    QString m_prenom;               //!< Prénom du correspondant
    QString m_sexe;                 //!< Sexe du correspondant
    QString m_metier;               //!< metier du correspondant
    QString m_adresse1;             //!< 1ere ligne de l'adresse du correspondant
    QString m_adresse2;             //!< 2ème ligne de l'adresse du correspondant
    QString m_adresse3;             //!< 3ème ligne de l'adresse du correspondant
    QString m_codepostal;           //!< code postal du correspondant
    QString m_ville;                //!< ville du correspondant
    QString m_telephone;            //!< telephone du correspondant
    QString m_portable;             //!< no de portable du correspondant
    QString m_fax;                  //!< no de fax du correspondant
    QString m_mail;                 //!< mail du correspondant
    bool    m_medecin;              //!< medecin?
    bool    m_generaliste;          //!< generaliste?
    bool    m_isAllLoaded = false;  //!< tous les renseignements concernant ce correspondant sont chargés
    int     m_idspecialite;         //!< id de la specialite correspondant en base

private:

public:
    //GETTER | SETTER
    enum typecorrespondant {MG, Spe1, Spe2}; Q_ENUM(typecorrespondant)                    //! les 3 types de correspondants MG = médecin traitant, Spe1 ou Spe2 = tout autre type de correpondant
    bool isMG() const;
    bool ismedecin() const;
    QString nom() const;
    QString prenom() const;
    QString nomprenom() const;
    QString sexe() const;
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    QString adresseComplete() const;
    QString codepostal() const;
    QString ville() const;
    QString telephone() const;
    QString metier() const;
    QString portable() const;
    QString fax() const;
    QString mail() const;
    int idspecialite() const;
    bool isallloaded() const;

    explicit Correspondant(QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data);
    void setid(int id)                      { m_id = id;
                                              m_data[CP_ID_CORRESP] = id; }
};

#endif // CLS_CORRESPONDANT_H
