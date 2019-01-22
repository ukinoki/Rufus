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

#ifndef DLG_SALLEDATTENTE_H
#define DLG_SALLEDATTENTE_H

#include "procedures.h"

namespace Ui {
class dlg_salledattente;
}

class dlg_salledattente : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_salledattente(int idPatAPasser, int idActeAPasser, QString Titre, QWidget *parent = Q_NULLPTR);
    ~dlg_salledattente();
    Ui::dlg_salledattente *ui;

private slots:
    void                Slot_OKButtonClicked();
    void                Slot_EnableOKButton();

private:
    Procedures          *proc;
    int                 gidPatient;
    int                 gidUser, gidActe, gidUserSuperviseur;
    QString             gTitre;
    DataBase            *db;
};

#endif // DLG_SALLEDATTENTE_H
