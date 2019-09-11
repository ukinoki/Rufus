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

#ifndef UPSTANDARDITEM_H
#define UPSTANDARDITEM_H

#include <QStandardItem>
#include "cls_item.h"

class UpStandardItem : public QStandardItem
{
public:
    UpStandardItem();
    UpStandardItem(QString txt);
    void    setitem(Item* item);
    Item*   item() const;

private:
    Item*   m_item = Q_NULLPTR;
};

#endif // UPSTANDARDITEM_H
