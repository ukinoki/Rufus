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

#ifndef DLG_PROGRAMMATIONINTERVENTIONS_H
#define DLG_PROGRAMMATIONINTERVENTIONS_H

#include <updialog.h>
#include <procedures.h>
#include <uptableview.h>

class dlg_programmationinterventions : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_programmationinterventions(QWidget *parent = Q_NULLPTR);
    ~dlg_programmationinterventions();

private:
    Procedures *proc = Procedures::I();
};

#endif // DLG_PROGRAMMATIONINTERVENTIONS_H
