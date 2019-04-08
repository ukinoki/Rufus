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

#ifndef DLG_GESTIONCOTATIONS_H
#define DLG_GESTIONCOTATIONS_H

#include "updialog.h"
#include "procedures.h"

class dlg_gestioncotations : public UpDialog
{
    Q_OBJECT

public:
    enum Mode   {
        Creation,
        Modification
    };
    enum TypeActe   {
        Association,
        HorsNomenclature
    };
    explicit dlg_gestioncotations(enum TypeActe type, enum Mode mode, QString CodeActe = "", QWidget *parent = Q_NULLPTR);
    ~dlg_gestioncotations();

private:
    DataBase    *db;
    Mode        gMode;
    TypeActe    gTypeActe;
    int         gidUser, gSecteurUser;
    QString     gCodeActe;
    QStringList glistActes;
    bool        VerifFiche();
    UpLineEdit  *codeline, *tarifoptamline, *tarifnooptamline, *tarifpratiqueline, *tipline;
    QWidget     *codewidg, *tarifoptamwidg, *tarifnooptamwidg, *tarifpratiquewidg, *tipwidg;
};

#endif // DLG_GESTIONCOTATIONS_H
