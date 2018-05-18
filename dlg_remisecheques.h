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

#ifndef DLG_REMISECHEQUES_H
#define DLG_REMISECHEQUES_H

#include <QLocale>
#include <QToolTip>

#include "procedures.h"
#include "uptablewidget.h"
#include "upcheckbox.h"
#include "upcombobox.h"
#include "uplineedit.h"

namespace Ui {
class dlg_remisecheques;
}

class dlg_remisecheques : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_remisecheques(Procedures *procAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_remisecheques();
    Ui::dlg_remisecheques   *ui;
    bool                    getInitOK();
    void                    setInitOK(bool init);

private slots:
    void            Slot_ImprimepushButton();
    void            Slot_AnnulupPushButton();
    void            Slot_CorrigeRemise();
    void            Slot_ItemChequeARemettreClicked(int A, int B);
    void            Slot_ItemChequeEnAttenteClicked(int A, int B);
    void            Slot_MiseEnFormeMontant(int A, int B, int C, int D);
    void            Slot_RecalculeMontant();
    void            Slot_ChangeCompte();
    void            Slot_RemplirRemisesPrecs(int);
    void            Slot_ToolTip(int A, int B);
    void            Slot_TrierListeCheques(int, int B);
    void            Slot_TrierChequesEnAttente(int, int B);
    void            Slot_VoirRemisesPrecs();

private:
    Procedures              *proc;
    bool                    InitOK;
    int                     gMode;
    int                     gidUserACrediter, gidCompteACrediter;
    enum gMode              {NouvelleRemise, RevoirRemisesPrecs, ReprendreRemise};
    bool                    gBloqueCellChanged;  // ce flag sert à empêcher le déclenchement du slot cellchanged sur les tables quand on utilise certianes fonctions
    void                    closeEvent(QCloseEvent *event);
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    void                    keyPressEvent ( QKeyEvent * event );
    void                    reject();
    bool                    ConfigMode (int);
    bool                    ImprimerRemise(int);
    QString                 ValeurAvantChangement;
    QList<int>              glistidRem;
    QSqlDatabase            db;
    void                    ReconstruitListeUsers();
};

#endif // DLG_REMISECHEQUES_H
