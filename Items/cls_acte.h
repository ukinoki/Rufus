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

#ifndef CLS_ACTE_H
#define CLS_ACTE_H

#include <QDate>
#include "cls_item.h"


/*!
 * \brief The Acte class
 * l'ensemble des informations concernant un acte
 * rappel pour un acte
 * iduser           = le soignant responsable médical de l'acte
 * creepar          = le soignant qui a cree l'acte
 * userparent       = le soignant remplacé si iduser est un remplaçant
 * usercomptable    = le user qui comptabilise l'acte
 */

class Acte : public Item
{

private:
    int m_id;                   //!< Id de l'acte en base

    int m_idPatient;            //!< id du Patient correspondant à l'acte
    int m_idUser;               //!< id du User
    int m_idCreatedBy;          //!< id du User qui a créé l'acte
    int m_idUserParent;         //!< id du User remplacé si le user est remplaçant ( = iduser si pas remplacé)
    int m_idUserComptable;      //!< id du User qui comptabilise l'acte
    int m_numCentre;            //!< id du lieu où se trouve le serveur
    int m_idLieu;               //!< id du lieu où l'acte est effectué

    double m_montant;           //!< montant total

    QString m_motif;            //!< motif
    QString m_texte;            //!< détail
    QString m_conclusion;       //!< conclusion
    QString m_courrierStatus;   //!< status du courrier //TODO : à compléter
    QString m_cotation;         //!< //TODO : à compléter : je ne sais pas
    QString m_monnaie;          //!< en franc "F" ou en euro
    QString m_paiementType;     //!< moyen de paiement
    QString m_paiementTiers;    //!< //TODO : à compléter : je ne sais pas

    QDateTime m_date;           //!< date de la création de l'acte
    QDateTime m_agePatient;     //!< date de naissance du Patient //TODO : MOVE to Patient



public:
    Acte(QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    bool courrierAFaire();
    bool isPayeEnFranc() const;

    /* GETTER / SETTER*/
    int id() const;
    QDate date() const;
    QString motif() const;
    QString texte() const;
    QString conclusion() const;
    QString courrierStatus() const;
    int idCreatedBy() const;
    int idPatient() const;
    QDate agePatient() const;
    QString cotation() const;
    double montant() const;
    QString paiementType() const;
    QString paiementTiers() const;
    int idUser() const;

    int numcentre() const;
    int idlieu() const;

    int idParent() const;
    int idComptable() const;

    void setcotation(QString cot)           { m_cotation = cot; }
    void setcourrierafaire(bool caf)        { m_courrierStatus = (caf? "T" : ""); }
    void setdate(QDate date)                { m_date = QDateTime(date); }
    void setmontant(double montant)         { m_montant = montant; }
    void setmotif(QString motif)            { m_motif = motif; }
    void settexte(QString texte)            { m_texte = texte; }
    void setconclusion(QString conclusion)  { m_conclusion = conclusion; }
};

#endif // CLS_ACTE_H
