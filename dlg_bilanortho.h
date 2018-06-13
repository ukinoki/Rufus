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

#ifndef DLG_BILANORTHO_H
#define DLG_BILANORTHO_H

#include "procedures.h"

namespace Ui {
class dlg_bilanortho;
}

class dlg_bilanortho : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_bilanortho(Procedures *procAPasser, int idActeAPAsser, int idBilanOrtho = 0, QWidget *parent = Q_NULLPTR, int idpat = -1);
    ~dlg_bilanortho();
    Ui::dlg_bilanortho      *ui;
    QString                 calcReponsehTml();
    void                    setDateBO(QDate dateBO);
    QDate                   getDateBO();

private:
    QStringList             TNOlist, XELlist, XEPlist, VergencesCLlist, VergencesDLlist, VergencesCPlist, VergencesDPlist, HLlist, HPlist;
    QStringList             Dioptrieslist, DioptriesNeutraliselist, HDioptrieslist, HDioptriesNeutraliselist, Wirtlist, AnimauxWirtlist, Langlist, PPClist, OcclAlternlist;
    int                     idBilan, idActe, gidpat;
    void                    closeEvent(QCloseEvent *event);
    void                    AfficheBilan(int idBilan);
    Procedures              *proc;
    QString                 CouleurTitres;
    QDate                   DateBO;
    User                    *gDataUser;

    // anciens slots
    void                    AfficheCombobox(int i, QWidget *widg);
    void                    ImprimeBOClicked();
    void                    EnableAnimauxWirtcomboBox(QString PionWirt);

};

#endif // DLG_BILANORTHO_H
