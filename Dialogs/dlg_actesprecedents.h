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

#ifndef DLG_ACTESPRECEDENTS_H
#define DLG_ACTESPRECEDENTS_H

#include "procedures.h"
#include <QWheelEvent>

/*! La fiche dlg_actesprecedents présente les actes d'un patient dans une fiche dans laquelle aucun champ ne peut être modifié.
 * Elle sert à consulter simplement les actes effectués sur un patient
 * Elle est appelée dans 4 circonstances
 *  . systématiquement, à l'ouverture d'un dossier s'il y a plusieurs actes dans ce dossier
 *  . en cliquant sur le bouton ui->OuvreActesPrespushButton ("Actes Précédents")
 *  . lors de la création d'un acte, si, du coup, on se retrouve avec plusieurs actes pour le même patient
 *      dans ces 3 premiers cas, elle se positionne automatiquement sur la consultation précédent celle qui est affichée dans le dossier et n'est pas modale.
 *      elle est appelée par Rufus::OuvresActesPrecedents(int idActe)
 *  . par un clic droit sur un mon de patient  dans la liste des patients et le choix "Visualiser le dossier"
 *      cette 4ème possibilité permet d'afficher la fiche d'un patient quand on consulte un autre dossier. Dans ce cas, la fiche est modale
 * les variables
 *  . Actes     = la classe Actes à afficher
 *  . AvantDernier  = bool qui indique si on se positionne sur idActeAPasser (false) ou sur celui qui précède (true)
 *  . le parent sert à retrouver la fiche depuis rufus.cpp si on veut le fermer
 */
namespace Ui {
class dlg_actesprecedents;
}

class dlg_actesprecedents : public QDialog
{

public:
    dlg_actesprecedents(Actes *actes, bool AvantDernier = true, QWidget *parent = Q_NULLPTR);
    ~dlg_actesprecedents();
    int                     idcurrentpatient() const;       //!> renvoie à rufus.cpp le patient concerné
    void                    Actualise();
    void                    ActesPrecsAfficheActe(Acte *acte); // Affiche l'acte défini
    void                    ActesPrecsAfficheActe();
    Acte*                   currentacte() const;

private:
    Ui::dlg_actesprecedents             *ui;
    Procedures                          *proc = Procedures::I();
    LignesPaiements                     *m_listepaiements = Datas::I()->lignespaiements;
    Patient                             *m_currentpatient;
    QMap<int, Acte*>::const_iterator    it_currentacte;
    Actes                               *m_listeactes;
    bool                                m_avantdernieracte;
    int                                 m_idpatient;

    void                    wheelEvent(QWheelEvent *event);
    virtual void            keyPressEvent(QKeyEvent *keyEvent); //PAS UTILISE
    void                    closeEvent(QCloseEvent *event);
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    bool                    NavigationConsult(ItemsList::POSITION i);
};

#endif // DLG_ACTESPRECEDENTS_H
