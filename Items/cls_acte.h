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

    int m_idPatient;            //!< id du Patient correspondant à l'acte
    int m_idSuperviseur;        //!< id du User superviseur de l'acte
    int m_idCreatedBy;          //!< id du User qui a créé l'acte
    int m_idParent;             //!< id du User remplacé si le user est remplaçant ( = iduser si pas remplacé)
    int m_idComptable;          //!< id du User qui comptabilise l'acte
    int m_numCentre;            //!< id du lieu où se trouve le serveur
    int m_idsite;               //!< id du lieu où l'acte est effectué
    int m_idintervention = 0;   //!< l'id de l'intervention quand l'acte correspond à une intervention - utilisé pour la gestion des programmes opératoires
    bool m_remplacant;          //!> le superviseur de l'acte est remplaçant au moment de la réalisation de l'acte

    double m_montant = 0.0;     //!< montant total

    QString m_motif;            //!< motif
    QString m_texte;            //!< détail
    QString m_conclusion;       //!< conclusion
    QString m_courrierStatus;   //!< status du courrier //TODO : à compléter
    QString m_cotation;         //!< //TODO : à compléter : je ne sais pas
    QString m_monnaie;          //!< en franc "F" ou en euro
    QString m_paiementType;     //!< moyen de paiement
    QString m_paiementTiers;    //!< //TODO : à compléter : je ne sais pas

    QDate m_date;               //!< date de la création de l'acte
    QTime m_heure;              //!< heure de la création de l'acte

public:
    Acte(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    bool courrierAFaire();
    bool isFactureEnFranc() const;
    bool isIntervention() const;

    /* GETTER / SETTER*/

    QDate date() const;
    QTime heure() const;
    QString motif() const;
    QString texte() const;
    QString conclusion() const;
    QString courrierStatus() const;
    int idCreatedBy() const;
    int idPatient() const;
    QString cotation() const;
    double montant() const;
    QString paiementType() const;
    QString paiementTiers() const;
    int idUserSuperviseur() const;
    bool effectueparremplacant() const;
    int idIntervention() const;

    int numcentre() const;
    int idsite() const;

    int idParent() const;
    int idComptable() const;

    void setid(int id)                      { m_id = id;
                                              m_data[CP_ID_ACTES] = id; }
    void setdate(QDate date)                { m_date = date;
                                              m_data[CP_DATE_ACTES] = date.toString("yyyy-MM-dd"); }
    void setheure(QTime heure)              { m_heure = heure;
                                              m_data[CP_HEURE_ACTES] = heure.toString("HH:mm:ss"); }
    void setcotation(QString cot)           { m_cotation = cot;
                                              m_data[CP_COTATION_ACTES] = cot; }
    void setcourrierafaire(bool caf)        { m_courrierStatus = (caf? "T" : "");
                                              m_data[CP_COURRIERAFAIRE_ACTES] = m_courrierStatus; }
    void setmontant(double montant)         { m_montant = montant;
                                              m_data[CP_MONTANT_ACTES] = montant; }
    void setidsuperviseur(int id)           { m_idSuperviseur = id;
                                              m_data[CP_IDUSER_ACTES] = id; }
    void setidpatient(int id)               { m_idPatient = id;
                                              m_data[CP_IDPAT_ACTES] = id; }
    void setidcomptable(int id)             { m_idComptable = id;
                                              m_data[CP_IDUSERCOMPTABLE_ACTES] = id; }
    void setidcreateur(int id)              { m_idCreatedBy = id;
                                              m_data[CP_IDUSERCREATEUR_ACTES] = id; }
    void setidlieu(int id)                  { m_idsite = id;
                                              m_data[CP_IDLIEU_ACTES] = id; }
    void setidparent(int id)                { m_idParent = id;
                                              m_data[CP_IDUSERPARENT_ACTES] = id; }
    void setnumcentre(int id)               { m_numCentre = id;
                                              m_data[CP_NUMCENTRE_ACTES] = id; }
    void setmotif(QString motif)            { m_motif = motif;
                                              m_data[CP_MOTIF_ACTES] = motif; }
    void settexte(QString texte)            { m_texte = texte;
                                              m_data[CP_TEXTE_ACTES] = texte; }
    void setconclusion(QString conclusion)  { m_conclusion = conclusion;
                                              m_data[CP_CONCLUSION_ACTES] = conclusion; }
    void seteffectueparremplacant(bool logic) { m_remplacant = logic;
                                                m_data[CP_SUPERVISEURREMPLACANT_ACTES] = logic; }
    void setidintervention(int id)          { m_idintervention = id;
                                              m_data[CP_ID_LIGNPRGOPERATOIRE] = id; }
};

#endif // CLS_ACTE_H
