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

#ifndef DLG_RESUMEREFRACTION_H
#define DLG_RESUMEREFRACTION_H

#include "procedures.h"
#include "updialog.h"

namespace Ui {
class dlg_resumerefraction;
}

class dlg_resumerefraction : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_resumerefraction(Procedures *procAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_resumerefraction();
    UpTextEdit          *txtedit;

private:
    Procedures          *proc;

};

#endif // DLG_RESUMEREFRACTION_H