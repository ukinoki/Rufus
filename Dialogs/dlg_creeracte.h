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

#ifndef DLG_CREERACTE_H
#define DLG_CREERACTE_H

#include "updialog.h"
#include "procedures.h"

namespace Ui {
class dlg_creeracte;
}

class dlg_creeracte : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_creeracte(QString Typeacte, QString Mode, QString CodeActe = "", QWidget *parent = Q_NULLPTR);
    ~dlg_creeracte();
    void        Initialise(Procedures *procAPasser);

private:
    Ui::dlg_creeracte *ui;
    Procedures  *proc;
    int         gMode;      enum gMode      {Creation, Modification};
    int         gTypeActe;  enum gTypeActe  {CCAM, AssocCCAM, HorsNomenclature};
    int         gidUser, gSecteurUser;
    QString     gCodeActe;
    QStringList glistActes;
    bool        VerifFiche();
};

#endif // DLG_CREERACTE_H
