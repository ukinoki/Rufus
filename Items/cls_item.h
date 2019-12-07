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

#ifndef CLS_ITEM_H
#define CLS_ITEM_H

#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <QDate>
#include "macros.h"
#include "utils.h"
#include "QDebug"

class Item : public QObject
{
    Q_OBJECT

public:
    enum LOADDETAILS {LoadDetails, NoLoadDetails};  Q_ENUM(LOADDETAILS)
    enum UPDATE {NoUpdate, Update};                 Q_ENUM(UPDATE)
    enum Logic {True, False, Null};                 Q_ENUM(Logic)
        /*! pour certaines données bool, on a 3 valeurs possibles, true, false et null
         * en fait, on pourrait même faire une 4ème valeur correspondaant à "ne sait pas" -> ne sait pas si la valeeur est true, false ou null
         */
    explicit Item(QObject *parent = Q_NULLPTR);
    int id() const                      { return m_id; }
    void setid(int id)                  { m_id = id; }
    QString stringid() const            { return m_stringid; }
    void setstringid(QString stringid)  { m_stringid = stringid; }
    QJsonObject datas() const           { return m_data; }

protected:

    void setDataString(QJsonObject data, QString key, QString &prop, bool useTrim=false);
    void setDataInt(QJsonObject data, QString key, int &prop);
    void setDataLongLongInt(QJsonObject data, QString key, qlonglong &prop);
    void setDataDouble(QJsonObject data, QString key, double &prop);
    void setDataBool(QJsonObject data, QString key, bool &prop);
    void setDataDateTime(QJsonObject data, QString key, QDateTime &prop);
    void setDataDate(QJsonObject data, QString key, QDate &prop);
    void setDataTime(QJsonObject data, QString key, QTime &prop);
    void setDataByteArray(QJsonObject data, QString key, QByteArray &prop);
    void setDataVariant(QJsonObject data, QString key, QVariant &prop);
    void setDataLogic(QJsonObject data, QString key, Item::Logic &prop);

    int m_id = 0;
    QString  m_stringid = "";
    QJsonObject m_data = QJsonObject{};         //!> les datas d'un item
};

#endif // CLS_ITEM_H
