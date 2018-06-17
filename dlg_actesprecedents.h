/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_ACTESPRECEDENTS_H
#define DLG_ACTESPRECEDENTS_H

#include <QMessageBox>
#include "procedures.h"
#include <QWheelEvent>

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
    dlg_actesprecedents(int idPatient, int idActeAPasser, Procedures *procAPAsser, bool AvantDernier = true, QWidget *parent = Q_NULLPTR);
    ~dlg_actesprecedents();
    int                     getidPatient();
    void                    Actualise();
    void                    ActesPrecsAfficheActe(int idActeAAfficher); // Affiche l'acte défini par idActeAAfficher
    int                     getActeAffiche();

private:
    void                    wheelEvent(QWheelEvent *event);
    void                    keyPressEvent(QKeyEvent *keyEvent);
    Ui::dlg_actesprecedents *ui;
    Procedures              *proc;
    int                     position;
        enum                position{Debut, Prec, Suiv, Fin};
    void                    closeEvent(QCloseEvent *event);
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    int                     nbActes, noActe;
    int                     gidActe, gidPatient;
    bool                    NavigationConsult(int i);
    bool                    gAvantDernier;
    QSqlDatabase            db;
    QList<int>              listactes;
    int                     ChercheActeAAfficher();                     // détermine l'acte à afficher suivant l'idPatient et la valeur de AVantDernier
};

#endif // DLG_ACTESPRECEDENTS_H
