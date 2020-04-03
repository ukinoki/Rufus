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

#ifndef CLS_RECETTE_H
#define CLS_RECETTE_H


#include "cls_item.h"

/*!
 * \brief Recette class
 * l'ensemble des informations concernant une recette
 * les recettes sont considéréees de 2 points de vue:
 * l'activité professionnelle: quels actes a pratiqué tel soignant?
    * on va faire la liste des actes pratiqués par un soigant responsable pendant une période donnée.
    * On ne comptabilisera aucun encaissement direct, ni aucun paiement par tiers.
    * On se contente de faire la somme des montants facturés par acte.
    * Ce décompte est utile pour faire par exemple le total des actes facturés par un remplaçant pour juger son activité et calculer les reversements d'honoraires
 * l'activité comptable: qui a encaissé quoi et de qui?
     * on fait la liste des honoraires pour un comptable.
     * Ne figureront ddonc pas dans cette liste les actes payés par tiers payant
     * Par contre, on y trouvera tous les paiements par tiers, les recettes spéciales et les actes payés directement, en chèque ou en espèces.
 */


class Recette : public Item
{

private:
    //!> m_id = l'id de la recette (pas utilisé dans la bdd)
    int m_idacte;                       //!> l'id de l'acte correspondant si la recette correspond à un acte payé directement
    QDate m_date;                       //!> la date de la recette
    QString m_nompayeur;                //!> le nom du payeur
    QString m_actecotation;             //!> la cotation de l'acte si la recette correspond à un acte
    double m_actemontant;               //!> le montant facturé de l'acte
    QString m_actemonnaie;              //!> la monnaie de la recette
    QString m_modepaiement;             //!> le mode de paiement: Espèces, chèque Virement, TIP...etc...
    QString m_typetiers;                //!> le type de tiers payant pour un acte non payé directement : CMU, AT, AMO, AME, CB...etc...
    double m_paye;                      //!> le montant encaissé
    int m_iduser;                       //!> l'id du user responsable de l'acte
    int m_iduserparent;                 //!> l'id du user parent du user responsable de l'acte (le user remplacé si le user responsable est remplaçant, le user responsable lui-même sinon)
    int m_idusercomptable;              //!> l'id du user comptable de la recette
    double m_montantautrerecettes;      //!> le montant encaissé d'une recette spéciale
    bool m_isapportparticien;           //! la  recette est un apport praticien
    bool m_isautrerecette;              //! la  recette est une autre recette

public:
    explicit Recette(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int idacte() const;
    QDate date() const;
    QString payeur() const;
    QString cotationacte() const;
    double montant() const;
    QString monnaie() const;
    QString modepaiement() const;
    QString typetiers() const;
    double encaissement() const;
    int iduser() const;
    int idparent() const;
    int idcomptable() const;
    double encaissementautrerecette() const;
    bool isapportpraticien();
    bool isautrerecette();
};

#endif // CLS_RECETTE_H
