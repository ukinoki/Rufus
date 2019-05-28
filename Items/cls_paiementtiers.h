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

#ifndef CLS_PAIEMENTTIERS_H
#define CLS_PAIEMENTTIERS_H

#include "cls_item.h"

/*!
 * \brief PaiementTiers class
 * un paiement effectué par un tiers
 */

class PaiementTiers : public Item
{
public:
    explicit PaiementTiers(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

private:
    /*! les paiements tiers correspondent à tous les paiements de la table comptamedicale.lignesrecettes effectués par des tiers payants : organisemes sociaux, assurances privées, tutelles ..etc...
     *  les paiements par carte de crédit sont aussi considérés comme des paiements par tiers puisque leur paiement se fait par un tiers, la banque, avec une commission
     *  d'une manièere générale tout paiement décalé dans le temps par une autre personne que le patient est considéré comme paiement par tiers
     *  Ces paiements son enregistrés par un 'O' dans le champ TiersPayant de la table comptamedicale.lignesrecettes
     */

    int m_idpaiement;               //!> l'idrecette du paiement en attente
    QDate m_date;                   //!> la date du paiement
    QDate m_dateenregistrement;     //!> la date à laquelle le paiement est enregistré
    double m_montant;               //!> le montant du paiement
    QString m_modepaiement;         //!> le mode de paiement : Espèces, chèque ou virement
    QString m_tireurcheque;         //!> le tireur du chèque en cas de paiement par chèque
    int m_comptevirement;           //!> l'id du compte sur lequel a été effectué le virement en cas de paiement par virement
    QString m_banquecheque;         //!> le nom de la banque émettrice du chèque en cas de paiement par chèque
    QString m_nomtiers;             //!> le nom du tiers payant
    double m_commission;            //!> la commission prise sur le virement en cas de paiement de carte de crédit
    QString m_monnaie;              //!> la monnaie du paiement
    int m_idremisecheque;           //!> l'id de la remise dans la tabke comptamedicale.remisecheques en cas de paiement par chèque
    bool m_chequeenattente;         //!> la mise en attente du chèque si l'émetteur le demande
    int m_iduserenregistreur;       //!> l'id du user qui a enregistré le paiement
    QDate m_dateremisecheques;      //!> la date à laquelle le chèquz est déposé
    double m_encaissement;          //!> ???

public:
    int id() const;                       //!> l'idrecette du paiement en attente
    QDate date() const;                   //!> la date du paiement
    QDate dateenregistrement() const;     //!> la date à laquelle le paiement est enregistré
    double montant() const;               //!> le montant du paiement
    QString modepaiement() const;         //!> le mode de paiement : Espèces, chèque ou virement
    QString tireurcheque() const;         //!> le tireur du chèque en cas de paiement par chèque
    int comptevirement() const;           //!> l'id du compte sur lequel a été effectué le virement en cas de paiement par virement
    QString banquecheque() const;         //!> le nom de la banque émettrice du chèque en cas de paiement par chèque
    QString nomtiers() const;             //!> le nom du tiers payant
    double commission() const;            //!> la commission prise sur le virement en cas de paiement de carte de crédit
    QString monnaie() const;              //!> la monnaie du paiement
    int idremisecheque() const;           //!> l'id de la remise dans la tabke comptamedicale.remisecheques en cas de paiement par chèque
    bool ischequeenattente() const;       //!> la mise en attente du chèque si l'émetteur le demande
    int iduserenregistreur() const;       //!> l'id du user qui a enregistré le paiement
    QDate dateremisecheques() const;      //!> la date à laquelle le chèquz est déposé
    double encaissement() const;          //!> ???

};

#endif // CLS_PAIEMENTTIERS_H
