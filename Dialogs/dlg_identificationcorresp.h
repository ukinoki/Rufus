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
    enum Mode   {
        Creation,
        Modification
    };
    explicit dlg_identificationcorresp(enum Mode mode, bool quelesmedecins, Correspondant *cor = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    Ui::dlg_identificationcorresp *ui;
    ~dlg_identificationcorresp();
    QLineEdit           *CPlineEdit, *VillelineEdit;
    bool                identcorrespondantmodifiee();
    Correspondant*      correspondantrenvoye();

private:
    int                 gidCor;
    Mode                gMode;
    Correspondant       *gCorrespondant;
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    QString             lCreatModif;
    bool                OnlyDoctors;
    QString             gNomCor, gPrenomCor, Sexe;
    Procedures          *proc;
    DataBase            *db;
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
