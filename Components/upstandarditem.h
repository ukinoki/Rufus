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

#ifndef UPSTANDARDITEM_H
#define UPSTANDARDITEM_H

#include <QStandardItem>
#include <QApplication>
#include "cls_item.h"

class Item;
class UpStandardItem : public QStandardItem
{
public:
    UpStandardItem();
    UpStandardItem(QString txt, Item* rufusitem = nullptr);
    UpStandardItem(const QIcon &icon, const QString &text, Item* rufusitem = nullptr);
    void    setrufusitem(Item* rufusitem);
    Item*   rufusitem() const;
    bool    hasrufusitem() const    { return m_rufusitem != nullptr; }
    bool    ischecked() const       { return checkState() == Qt::Checked; }

    QList<int> listids() const;
    void setListids(const QList<int> &newListids);
    bool        hascheckBox();

    QDate date() const;
    void setDate(const QDate &newDate);

    QString dataType() const;
    void setdataType(const QString &newType);

    bool beforeCheckState();
    void setBeforeCheckState(bool newBeforeCheckState);

private:
    Item*       m_rufusitem = nullptr;
    QList<int>  m_listids   = QList<int>();
    QDate       m_date      = QDate();
    QString     m_datatype  = QString();
    bool        m_beforeCheckState = false;   /*! when you click an item outside the checkbox, the signal clicked is emitted  but the checkstate doesn't change
                                                  - so, you need to compare the state before the cilcked signal with this afte to know ik checkstate has changed
                                                by this way, you can check if checkstate has changed; */
};

#endif // UPSTANDARDITEM_H
