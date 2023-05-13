/* (C) 2020 LAINE SERGE
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

#include "upprogressdialog.h"

UpProgressDialog::UpProgressDialog(int n, int t, QWidget *parent, Qt::WindowFlags f) : QProgressDialog(parent, f)
{
    QVBoxLayout *globallay = new QVBoxLayout(this);
    globallay   ->setContentsMargins(10,10,10,10);
    label->setAlignment(Qt::AlignLeft);
    globallay->insertWidget(0,bar);
    globallay->insertWidget(0,label);
    setLabel(label);
    setFixedWidth(600);
    setBar(bar);
    setRange(n,t);
    setCancelButton(Q_NULLPTR);
    setWindowModality(Qt::WindowModal);
}

UpProgressDialog::~UpProgressDialog()
{
}


