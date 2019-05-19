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
    explicit dlg_remisecheques(QWidget *parent = Q_NULLPTR);
    ~dlg_remisecheques();
    Ui::dlg_remisecheques   *ui;
    bool                    getInitOK();

private slots:
    void            Slot_ImprimepushButton();
    void            Slot_AnnulupPushButton();
    void            Slot_CorrigeRemise();
    void            Slot_ItemChequeARemettreClicked(int A, int B);              //utilisé pour mettre en attente un chèque
    void            Slot_ItemChequeEnAttenteClicked(int A, int B);              //utilisé pour mettre en encaissement un chèque auparavant en attente
    void            Slot_MiseEnFormeMontant(int A, int B, int C, int D);
    void            Slot_RecalculeMontant();
    void            Slot_ChangeUser();
    void            Slot_RemplirRemisesPrecs(int);
    void            Slot_ToolTip(int A, int B);
    void            Slot_TrierListeCheques(int, int B);
    void            Slot_TrierChequesEnAttente(int, int B);

private:
    DataBase                *db;
    Procedures              *proc;
    bool                    InitOK;
    int                     gMode;
    enum gMode              {NouvelleRemise, RevoirRemisesPrecs, ReprendreRemise};
    bool                    gBloqueCellChanged;  // ce flag sert à empêcher le déclenchement du slot cellchanged sur les tables quand on utilise certianes fonctions
    void                    closeEvent(QCloseEvent *event);
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    void                    keyPressEvent ( QKeyEvent * event );
    void                    reject();
    bool                    ImprimerRemise(int);
    QMap<int, User *>       *m_comptables, *m_comptablesavecchequesenattente;
    QMultiMap<int, Compte*> *m_comptes = Q_NULLPTR;
    User                    *m_userencours;
    QString                 ValeurAvantChangement;
    void                    ChangeCompte();
    void                    ReconstruitListeUsers();
    void                    ReconstruitListeComptes(bool avecinactif = false);
    bool                    VoirNouvelleRemise();
    bool                    VoirRemisesPrecs();
};

#endif // DLG_REMISECHEQUES_H
