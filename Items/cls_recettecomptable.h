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

#ifndef CLS_RECETTECOMPTABLE_H
#define CLS_RECETTECOMPTABLE_H


#include "cls_item.h"

/*!
 * \brief RecetteComptable class
 * l'ensemble des informations concernant une recettecomptable
 * traite les enregistrements de la table ComptaMedicale.lignesecettes
 */

class RecetteComptable : public Item
{
public:
    explicit RecetteComptable(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

private:
    //!> m_id = l'id de la recette (pas utilisé dans la bdd)
    int m_iduser;                       //!> l'id du user qui comptabilise la recette
    QDate m_date;                       //!> la date de la recette
    QDate m_dateenregistrement;         //!> la date de l'enregistrement de la recette dans la bdd
    double m_montant;                   //!> le montant de la recette
    QString m_modepaiement;             //!> le mode de paiement: Espèces, chèque Virement, TIP...etc...
    QString m_nomtireurchq;             //!> le nom du tireur du chèque en cas de paiement par chèque
    int m_idcompte;                     //!> l'id du compte bancaire sur lequel la recette a été versée
    QString m_nombanque;                //!> la banque du chèque en cas de paiement par chèque
    bool m_tierspayant;                 //!> true -> la recette vient d'un tiers payant
    QString m_nompayeur;                //!> le nom du payeur
    double m_commission;                //!> le montant de la commission éventuelle sur la recette
    QString m_monnaie;                  //!> monnaie de la recette
    int m_idremise;                     //!> l'id de la remise de chèque si c'est un paiement par chèque
    bool m_chqenattente;                //!> le chèque de cette recette est mis en attente avant encaissement
    int m_iduserenregistreur;           //!> l'id du user qui a enregistré la recette
    int m_typerecette;                  //!> le type de recette - pas utilisé - toujours à 1

public:

    int iduser()                        { return m_iduser; }
    QDate date()                        { return m_date; }
    QDate dateenregistrement()          { return m_dateenregistrement; }
    double montant()                    { return m_montant; }
    QString modepaiement()              { return m_modepaiement; }
    QString tireurcheque()              { return m_nomtireurchq; }
    int compteid()                      { return m_idcompte; }
    QString banquecheaque()             { return m_nombanque; }
    bool istierspayant()                { return m_tierspayant; }
    QString payeur()                    { return m_nompayeur; }
    double commission()                 { return m_commission; }
    QString monnaie()                   { return m_monnaie; }
    int idremisecheque()                { return m_idremise; }
    bool ischeqenattente()              { return m_chqenattente; }
    int iduserenregistreur()            { return m_iduserenregistreur; }
    int typerecette()                   { return m_typerecette; }

    void setiduser(int id)                  { m_iduser = id; }
    void setdate(QDate date)                { m_date = date; }
    void setdateenregistrement(QDate date)  { m_dateenregistrement = date; }
    void setmontant(double mont)            { m_montant = mont; }
    void setmodepaiement(QString txt)       { m_modepaiement = txt; }
    void settireurcheque(QString txt)       { m_nomtireurchq = txt; }
    void setcompteid(int id)                { m_idcompte = id; }
    void setbanquecheaque(QString txt)      { m_nombanque = txt; }
    void setistierspayant(bool logic)       { m_tierspayant = logic; }
    void setpayeur(QString txt)             { m_nompayeur = txt; }
    void setcommission(double com)          { m_commission = com; }
    void setmonnaie(QString mon)            { m_monnaie = mon; }
    void setidremisecheque(int id)          { m_idremise = id; }
    void setischeqenattente(bool logic)     { m_chqenattente = logic; }
    void setiduserenregistreur(int id)      { m_iduserenregistreur = id; }
    void settyperecette(int id)             { m_typerecette = id; }
};

#endif // CLS_RECETTECOMPTABLE_H
