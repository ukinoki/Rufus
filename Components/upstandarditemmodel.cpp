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

#include "upstandarditemmodel.h"


UpStandardItemModel::UpStandardItemModel(QObject *parent) : QStandardItemModel(parent)
{

}

int UpStandardItemModel::getRowFromItem(Item *itm)
{
    int row = -1;
    if (!itm)
        return row;
    for (int i=0; i<rowCount(); i++)
    {
        UpStandardItem *sitm = dynamic_cast<UpStandardItem*>(item(i));
        if (sitm)
        {
            Item *its = sitm->item();
            if (its == itm)
            {
                row = i;
                i = rowCount();
            }
        }
    }
    return row;
}
