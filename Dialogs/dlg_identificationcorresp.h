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

#ifndef DLG_IDENTIFICATIONCORRESP_H
#define DLG_IDENTIFICATIONCORRESP_H

#include "database.h"
#include "uplabel.h"
#include "uptablewidget.h"
#include "procedures.h"
#include "villecpwidget.h"
#include "ui_villecpwidget.h"

namespace Ui {
class dlg_identificationcorresp;
}

class dlg_identificationcorresp : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_identificationcorresp(QString CreationModification, bool quelesmedecins, int idCorresp, QWidget *parent = Q_NULLPTR);
    Ui::dlg_identificationcorresp *ui;
    ~dlg_identificationcorresp();
    int                 gidCor;
    QLineEdit           *CPlineEdit, *VillelineEdit;
    bool                IdentModified();

private:
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    QString             lCreatModif;
    bool                OnlyDoctors;
    QString             gNomCor, gPrenomCor, Sexe;
    Procedures          *proc;
    QSqlDatabase        db;
    void                AfficheDossierAlOuverture();
    VilleCPWidget       *VilleCPwidg;
    bool                modif;
    void                ReconstruitListeSpecialites();

private slots:
    void                Slot_EnableOKpushButton();
    void                Slot_OKpushButtonClicked();
    void                Slot_Majuscule();
    void                Slot_RegleAffichage();
};

#endif // DLG_IDENTIFICATIONCORRESP_H
