/* (C) 2026 LAINE SERGE
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

#ifndef RECHERCHEDOSSIER_H
#define RECHERCHEDOSSIER_H

#include <updialog.h>
#include <QDate>
#include <QModelIndex>
#include <QPoint>

class QLineEdit;
class QDateEdit;
class QStandardItemModel;
class UpCheckBox;
class UpTableView;
class UpLabel;
class DataBase;

//-----------------------------------------------------------------------------------------------------
// Recherche manuelle d'un dossier, calquée sur la recherche de la fiche principale : nom / prénom /
// date de naissance en haut à gauche, choix des critères (par nom, par ddn) dans un groupe à droite,
// table des patients (50 premiers) en dessous, total en bas, OK / Annuler.
// À la validation, idChoisi() donne l'idPat sélectionné (0 si aucun).
//-----------------------------------------------------------------------------------------------------
class RechercheDossier : public UpDialog
{
    Q_OBJECT

public:
    explicit RechercheDossier(const QString &nom, const QString &prenom, const QDate &ddn, QWidget *parent = nullptr);
    int  idChoisi() const { return m_idChoisi; }
    bool ouvrir()   const { return m_ouvrir; }   //!< true = ouvrir le dossier ; false = inscrire en salle d'attente

private slots:
    void rechercher();                          //!< relance la recherche à chaque changement de critère
    void activer(const QModelIndex &index);     //!< double-clic : on retient le dossier et on ferme
    void menuContextuel(const QPoint &pos);     //!< clic droit : une entrée, libellée selon le rôle
    void afficheBulle(const QModelIndex &index);//!< info-bulle du dossier survolé

private:
    int  idDeLigne(const QModelIndex &index) const;   //!< idPat porté par une ligne (0 si aucun)

    QLineEdit          *m_nom      = nullptr;
    QLineEdit          *m_prenom   = nullptr;
    QDateEdit          *m_ddn      = nullptr;
    UpCheckBox         *m_parNom   = nullptr;
    UpCheckBox         *m_parDdn   = nullptr;
    UpTableView        *m_table    = nullptr;
    QStandardItemModel *m_model    = nullptr;
    UpLabel            *m_total    = nullptr;
    DataBase           *m_db       = nullptr;
    int                 m_idChoisi = 0;
    bool                m_ouvrir   = true;
};

#endif // RECHERCHEDOSSIER_H
