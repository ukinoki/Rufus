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

#ifndef CLS_ITEM_H
#define CLS_ITEM_H

#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <QDate>
#include "macros.h"
#include <utils.h>
#include "QDebug"

class Item : public QObject
{
    Q_OBJECT

public:
    enum LOADDETAILS    {LoadDetails, NoLoadDetails};       Q_ENUM(LOADDETAILS)
    enum UPDATE         {NoUpdate, Update};                 Q_ENUM(UPDATE)
    explicit Item(QObject *parent = Q_NULLPTR) : QObject(parent)
    {
        m_id = 0;
        m_stringid = "";
    }
    int id() const                      { return m_id; }
    QString stringid() const            { return m_stringid; }
    QJsonObject datas() const           { return m_data; }

protected:
    int m_id = 0;
    QString  m_stringid = "";
    QJsonObject m_data = QJsonObject{};         //!> les datas d'un item
};

#endif // CLS_ITEM_H
