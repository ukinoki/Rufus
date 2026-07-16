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

#ifndef FICHEVITALE_H
#define FICHEVITALE_H

#include <updialog.h>
#include <QModelIndex>
#include <QPoint>
#include "lecteurvitale.h"

class UpTableView;
class DataBase;
class QEvent;
class Patient;

//-----------------------------------------------------------------------------------------------------
// Fiche présentant les données d'une carte Vitale lue (réelle ou simulée) :
//   - en-tête : figurine (âge/sexe) à gauche ; NNI + table du PORTEUR à droite ;
//   - en dessous, à droite, la table des AYANTS DROIT ;
//   - en bas, pleine largeur, la table des CORRESPONDANCES de dossiers trouvées en base.
//
// Les tables porteur et ayants droit partagent UNE SEULE surbrillance (sélectionner un ayant droit
// désélectionne le porteur, et inversement). À chaque changement de surbrillance, on recalcule les
// correspondances pour la personne surbrillée. La navigation clavier (flèches haut/bas) franchit la
// frontière entre les deux tables.
//-----------------------------------------------------------------------------------------------------
class FicheVitale : public UpDialog
{
    Q_OBJECT

public:
    explicit FicheVitale(const QList<LecteurVitale::Porteur> &porteurs, QWidget *parent = nullptr);

    //! Action que Rufus doit exécuter après fermeture de la fiche.
    enum Action { Annuler, Ouvrir, SalleAttente, Creer };
    Action action() const { return m_action; }
    //! Dossier choisi (pour Ouvrir / SalleAttente) ; 0 si aucun.
    int idDossierActive() const { return m_idDossierActive; }
    //! Personne lue sur la carte (pour Creer : nom / prénom / date de naissance).
    LecteurVitale::Porteur porteurCourant() const { return m_porteurCourant; }

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;   //!< flèches haut/bas d'une table à l'autre

private slots:
    void selectionPorteurChangee();
    void selectionAyantChangee();
    void afficheBulleCorresp(const QModelIndex &index);   //!< info-bulle du dossier survolé (comme la liste patients)
    void activerDepuisCorresp(const QModelIndex &index);  //!< double-clic sur une correspondance → on renvoie le dossier
    void menuCorresp(const QPoint &pos);                  //!< clic droit : ouvrir le dossier / inscrire en salle d'attente
    void rechercheManuelle();                             //!< recherche manuelle d'un dossier (bouton archive)

private:
    QList<LecteurVitale::Porteur> m_porteurs;   //!< [0] = assuré ; [1..] = ayants droit
    LecteurVitale::Porteur        m_porteurCourant;   //!< dernier porteur surbrillé (pré-remplit la recherche manuelle)
    UpTableView *m_tblPorteur = nullptr;        //!< 1 ligne : l'assuré
    UpTableView *m_tblAyants  = nullptr;        //!< les ayants droit
    UpTableView *m_tblCorresp = nullptr;        //!< dossiers correspondants (résultat de recherche)
    DataBase    *m_db         = nullptr;
    bool         m_majSelection = false;        //!< garde anti-récursion pendant qu'on désélectionne l'autre table
    int          m_idDossierActive = 0;         //!< dossier choisi (Ouvrir / SalleAttente)
    Action       m_action = Annuler;            //!< action demandée en sortie

    void surbrillanceChangee(const LecteurVitale::Porteur &porteur);  //!< recalcule les correspondances
    void selectionnePorteur();
    void selectionneAyant(int row);
    Patient *patientCorresp(const QModelIndex &index) const;          //!< Patient accroché à une ligne de correspondance
    Patient *correspChoisie() const;                                  //!< correspondance sélectionnée (ou la 1re) pour les boutons
};

#endif // FICHEVITALE_H
