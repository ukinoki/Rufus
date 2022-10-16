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

#include "uptableview.h"

UpTableView::UpTableView(QWidget *parent) : QTableView(parent)
{

}

void UpTableView::FixLargeurTotale()
{
    int larg = 0;
    for (int i=0; i < model()->columnCount(); i++)
        if (!isColumnHidden(i))
            larg += columnWidth(i);
    setFixedWidth(larg+2);
}

int UpTableView::rowNoHiddenCount() const
{
    int nrow = 0;
    for (int i=0; i<model()->rowCount(); i++)
        if (!isRowHidden(i))
            nrow += 1;
    return nrow;
}
