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
    explicit dlg_bilanortho(Acte *acte, bool nouveaubilan, QWidget *parent = Q_NULLPTR);
    ~dlg_bilanortho();
    Ui::dlg_bilanortho      *ui;
    QString                 calcReponsehTml(QString textorigin ="");
    void                    setDateBO(QDate dateBO);

private:
    QStringList             TNOlist, XELlist, XEPlist, VergencesCLlist, VergencesDLlist, VergencesCPlist, VergencesDPlist, HLlist, HPlist;
    QStringList             Dioptrieslist, DioptriesNeutraliselist, HDioptrieslist, HDioptriesNeutraliselist, Wirtlist, AnimauxWirtlist, Langlist, PPClist, OcclAlternlist;
    void                    closeEvent(QCloseEvent *event);
    void                    AfficheBilan(Acte *acte);
    void                    ExtraitRefraction(QString textorigin, QString &ReponseaModifier);
    Procedures              *proc;
    DataBase                *db;
    Acte                    *m_currentact;
    QDate                   DateBO;

    void                    AfficheCombobox(int i, QWidget *widg);
    void                    ImprimeBOClicked();
    void                    EnableAnimauxWirtcomboBox(QString PionWirt);

};

#endif // DLG_BILANORTHO_H
