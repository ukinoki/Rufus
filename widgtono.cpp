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

#include "widgtono.h"
#include "ui_widgtono.h"

WidgTono::WidgTono(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgTono)
{
    ui->setupUi(this);
    ui->TOODSpinBox->setFocus();
    ui->TOODSpinBox->selectAll();
}

WidgTono::~WidgTono()
{
    delete ui;
}
