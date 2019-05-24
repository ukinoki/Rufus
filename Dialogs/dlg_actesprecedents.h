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

#include <QMessageBox>
#include "cls_acte.h"
#include "procedures.h"
#include <QWheelEvent>
#include "gbl_datas.h"
#include "cls_user.h"
#include "icons.h"

/* La fiche dlg_actesprecednets présente les actes d'un patient dans une fiche dans laquelle aucun champ ne peut être modifié.
 * Elle sert à consulter simplement les actes effectués sur un patient
 * Elle est appelée dans 4 circonstances
 *  . systématiquement, à l'ouverture d'un dossier s'il y a plusieurs actes dans ce dossier
 *  . en cliquant sur le bouton ui->OuvreActesPrespushButton ("Actes Précédents")
 *  . lors de la création d'un acte, si, du coup, on se retrouve avec plusieurs actes pour le même patients
 *      dans ces 3 premiers cas, elle se positionne automatiquement sur la consultation précédent celle qui est affichée dans le dossier et n'est pas modale.
 *      elle est appelée par Rufus::OuvresActesPrecedents(int idActe)
 *  . par un clic droit sur un mon de patient  dans la liste des patients et le choix "Visualiser le dossier"
 *      cette 4ème possibilité permet d'afficher la fiche d'un patient quand on consulte un autre dossier. Dans ce cas, la fiche est modale
 * les variables
 *  . idpatient     = l'idpatient
 *  . idActeAPasser = id de l'acte en cours d'affichage (le dernier en cas d'appel par "Visualiser le dossier"
 *  . procAPasser   = procedures.h
 *  . AvantDernier  = bool qui indique si on se positionne sur idActeAPasser (false) ou sur celui qui précède (true)
 *  . le parent sert à retrouver la fiche depuis rufus.cpp si on veut le fermer
 */
namespace Ui {
class dlg_actesprecedents;
}

class dlg_actesprecedents : public QDialog
{
    Q_OBJECT

public:
    dlg_actesprecedents(QMap<int, Acte *> *listactes, bool AvantDernier = true, QWidget *parent = Q_NULLPTR);
    ~dlg_actesprecedents();
    Patient*                getPatient();
    void                    Actualise();
    void                    ActesPrecsAfficheActe(Acte *acte); // Affiche l'acte défini
    void                    ActesPrecsAfficheActe();
    Acte                    *getActeAffiche();

private:
    Patient                 *m_currentpatient;
    void                    wheelEvent(QWheelEvent *event);
    virtual void            keyPressEvent(QKeyEvent *keyEvent); //PAS UTILISE
    Ui::dlg_actesprecedents *ui;
    Procedures              *proc;
    int                     position;
        enum                position{Debut, Prec, Suiv, Fin};
    void                    closeEvent(QCloseEvent *event);
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    Acte                    *acte;
    QMap<int, Acte*>::const_iterator    itCurrentActe;
    QMap<int, Acte*>        *m_listeActes;
    bool                    NavigationConsult(int i);
    bool                    gAvantDernier;
};

#endif // DLG_ACTESPRECEDENTS_H
