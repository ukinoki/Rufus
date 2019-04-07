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

#ifndef CLS_PATIENT_H
#define CLS_PATIENT_H

#include "cls_item.h"
#include "cls_acte.h"

#include <QObject>

/*!
 * \brief The Patient class
 * l'ensemble des informations concernant une patient
 */

class Patient : public Item
{
    Q_OBJECT

private: //Données du patient
    int m_id;                       //!< Id du patient en base
    QString m_nom;                  //!< Nom du patient
    QString m_prenom;               //!< Prénom du patient
    QString m_sexe;                 //!< Sexe du patient
    QDate m_datecreation;           //!< Date de creation du dossier
    int m_idcreateur;               //!< id du user qui a créé le dossier

    QDate m_dateDeNaissance;        //!< Date de naissance du patient



private:
    bool m_isAllLoaded = false;

    QMap<int, Acte*> *m_actes;      //!< ensemble des actes du patient

public:
    //GETTER | SETTER
    bool isAllLoaded() const;

    int     id() const;
    QString nom() const;
    QString prenom() const;
    QString sexe() const;
    QDate   datecreationdossier() const;
    QDate   datedenaissance() const;
    int     idcreateur() const;

    void setActes(QMap<int, Acte *> *actes);
    QMap<int, Acte *> *actes() const;

    explicit Patient(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);
    void addActe(Acte *acte);

};

#endif // CLS_PATIENT_H
