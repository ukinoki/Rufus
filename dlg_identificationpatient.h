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

#ifndef DLG_IDENTIFICATIONPATIENT_H
#define DLG_IDENTIFICATIONPATIENT_H

#include "dlg_identificationcorresp.h"
#include "ui_dlg_identificationcorresp.h"
#include <QTimer>

namespace Ui {
class dlg_identificationpatient;
}

class dlg_identificationpatient : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_identificationpatient(QString *CreationModification, int *idPatAPasser, Procedures *procAPasser,
                                       QWidget *parent = Q_NULLPTR);
    ~dlg_identificationpatient();
    Ui::dlg_identificationpatient   *ui;
    int                             gidPatient;
    bool                            ReconstruireListMG;
    QLineEdit                       *CPlineEdit, *VillelineEdit;

private:
    dlg_identificationcorresp       *Dlg_IdentCorresp;
    bool                            gAutorDepart, gControleMGCombo;
    QString                         lCreatModifCopie;
    QString                         gNomPatient, gPrenomPatient, Sexe;
    Procedures                      *proc;
    QSqlDatabase                    db;
    QTimer                          *gTimer;
    UpSmallButton                   *VitaleButton;
    VilleCPWidget                   *VilleCPwidg;


    int                             CloseReason;
    enum                            CloseReason {Accept,Reject};
    int                             gflagMG;
    void                            FermeFiche(enum CloseReason);

    bool                            eventFilter(QObject *obj, QEvent *event)  ;
    void                            AfficheDossierAlOuverture();
    int                             EnregistreNouveauCorresp();
    void                            MAJMG();
    void                            ReconstruitListeMG();

    // anciens slots
    void                            ChoixMG();

private slots:
    void                            Slot_EnableOKpushButton();
    void                            Slot_OKpushButtonClicked();
    void                            Slot_AnnulpushButtonClicked();
    void                            Slot_Majuscule();
    void                            Slot_ModifDDN();
    void                            Slot_VerifMGFlag();
};

#endif // DLG_IDENTIFICATIONPATIENT_H
