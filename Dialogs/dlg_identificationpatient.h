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

#ifndef DLG_IDENTIFICATIONPATIENT_H
#define DLG_IDENTIFICATIONPATIENT_H

#include "dlg_identificationcorresp.h"
#include "ui_dlg_identificationcorresp.h"
#include <QTimer>

namespace Ui {
    class dlg_identificationpatient;
}

/*!
 * \brief The dlg_identificationpatient class
 * Affiche les informations de profil d'un patient
 */
class dlg_identificationpatient : public UpDialog
{
    Q_OBJECT

public:
    enum Mode   {
        Copie,
        Creation,
        Modification
    };
    explicit dlg_identificationpatient(enum Mode mode, Patient *pat, QWidget *parent = Q_NULLPTR);
    ~dlg_identificationpatient();
    Ui::dlg_identificationpatient   *ui;
    QLineEdit                       *CPlineEdit, *VillelineEdit;
    bool                            listecorrespondantsmodifiee();
    Patient*                        getPatient();

private:
    Patient                         *m_currentpatient;
    dlg_identificationcorresp       *Dlg_IdentCorresp;
    bool                            ListeCorModifiee;
    Mode                            gMode;
    Procedures                      *proc;
    DataBase                        *db;
    bool                            ok;
    QMenu                           *gmenuContextuel;
    QTimer                          *gTimer;
    UpSmallButton                   *VitaleButton;
    VilleCPWidget                   *VilleCPwidg;


    int                             CloseReason;
    enum                            CloseReason {Accept,Reject};
    int                             gflagMG;

    bool                            eventFilter(QObject *obj, QEvent *event)  ;
    void                            AfficheDossierAlOuverture();
    int                             EnregistreNouveauCorresp();
    void                            MAJMG();

    void                            MenuContextuelMedecin();
    void                            ModifCorrespondant();
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
