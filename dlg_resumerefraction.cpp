/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_resumerefraction.h"

dlg_resumerefraction::dlg_resumerefraction(Procedures *procAPasser, QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionResumeRef", parent)
{
    proc = procAPasser;
    txtedit = new UpTextEdit;
    txtedit->setFixedWidth(820);
    dynamic_cast<QVBoxLayout*>(layout())->insertWidget(0,txtedit);
    AjouteLayButtons(UpDialog::ButtonClose);
    connect (CloseButton,   SIGNAL(clicked()),  this,   SLOT (close()));
    setAttribute(Qt::WA_DeleteOnClose);
 }

dlg_resumerefraction::~dlg_resumerefraction()
{
}

