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

#ifndef LIGNECOMPTE_H
#define LIGNECOMPTE_H

#include "cls_item.h"


class LigneCompte : public Item
{

private:
    int m_idcompte = 0;
    int m_iddep = 0;
    int m_idrecette = 0;
    int m_idrecettespeciale = 0;
    int m_idremisecheque = 0;
    QDate m_date = QDate();
    QString m_libelle = "";
    QString m_typeoperation = "";
    bool m_iscredit = true;
    bool m_isconsolide = false;
    double m_montant = 0.00;

public:
    explicit LigneCompte(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int idcompte() const                { return m_idcompte; }              //! le compte bancaire de la ligne
    int iddepense() const               { return m_iddep; }                 //! la dépense concernée si c'est une dépense
    int idrecette() const               { return m_idrecette; }             //! la recette concernée si c'est une recette
    int idrecettespeciale() const       { return m_idrecettespeciale; }     //! la recette spéciale concernée si c'est une recette spéciale
    int idremisecheque() const          { return m_idremisecheque; }        //! l'id de la remise de chèque si c'est une remise de chèque
    QDate date() const                  { return m_date; }                  //! la date
    QString libelle() const             { return m_libelle; }               //! le libellé de l'opération
    QString typeoperation() const       { return m_typeoperation; }         //! le type d'opération
    bool iscredit() const               { return m_iscredit; }              //! true = opération créditrice false = opération débitrice
    bool isconsolide() const            { return m_isconsolide; }           //! opération consolidée ou pas
    double montant() const              { return m_montant; }               //! montant

    void setidcompte(int id)              { m_idcompte = id; }              //! le compte bancaire de la ligne
    void setiddepense(int id)             { m_iddep = id; }                 //! la dépense concernée si c'est une dépense
    void setidrecette(int id)             { m_idrecette = id; }             //! la recette concernée si c'est une recette
    void setidrecettespeciale(int id)     { m_idrecettespeciale = id; }     //! la recette spéciale concernée si c'est une recette spéciale
    void setidremisecheque(int id)        { m_idremisecheque = id; }        //! l'id de la remise de chèque si c'est une remise de chèque
    void setdate(QDate date)              { m_date = date; }                //! la date
    void setlibelle(QString text)         { m_libelle = text; }             //! le libellé de l'opération
    void settypeoperation(QString text)   { m_typeoperation = text; }       //! le type d'opération
    void setiscredit(bool logic)          { m_iscredit = logic; }           //! true = opération créditrice false = opération débitrice
    void setisconsolide(bool logic)       { m_isconsolide = logic; }        //! opération consolidée ou pas
    void setmontant(double montant)       { m_montant = montant; }          //! montant

};

#endif // LIGNECOMPTE_H
