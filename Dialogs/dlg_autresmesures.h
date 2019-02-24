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

#ifndef DLG_AUTRESMESURES_H
#define DLG_AUTRESMESURES_H

#include "procedures.h"
#include "widgtono.h"
#include "ui_widgtono.h"

namespace Ui {
class dlg_autresmesures;
}

class dlg_autresmesures : public UpDialog
{
    Q_OBJECT

public:
    ~dlg_autresmesures();
    int mode;
    enum mode {TONO, PACHY};
    explicit dlg_autresmesures(int idPatAPasser, enum mode mod = TONO, QWidget *parent = Q_NULLPTR);
    WidgTono            *widgto;

private:
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                EnregistreTono();
    double              gTOD, gTOG;
    Procedures          *proc;
    int                 gidPatient;
    QSqlDatabase        db;
    void                OKButtonClicked();
    QKeyEvent           *keyEvent;
};

#endif // DLG_AUTRESMESURES_H
