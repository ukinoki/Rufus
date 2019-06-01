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

#include "cls_item.h"
#include "utils.h"

#include <QDateTime>


Item::Item(QObject *parent) : QObject(parent)
{
}

void Item::setDataString(QJsonObject data, QString key, QString &prop, bool useTrim)
{
    if( data.contains(key) )
    {
        QString str = data[key].toString();
        if( useTrim )
            str = Utils::trim(str);
        prop = str;
    }
}
void Item::setDataInt(QJsonObject data, QString key, int &prop)
{
    if( data.contains(key) )
        prop = data[key].toInt();
}
void Item::setDataLongLongInt(QJsonObject data, QString key, qlonglong &prop)
{
    if( data.contains(key) )
        prop = data[key].toVariant().toLongLong();
}
void Item::setDataDouble(QJsonObject data, QString key, double &prop)
{
    if( data.contains(key) )
        prop = data[key].toDouble();
}
void Item::setDataBool(QJsonObject data, QString key, bool &prop)
{
    if( data.contains(key) )
        prop = data[key].toBool();
}
void Item::setDataDateTime(QJsonObject data, QString key, QDateTime &prop)
{
    if( data.contains(key) )
    {
        double time = data[key].toDouble();
        QDateTime dt;
        dt.setMSecsSinceEpoch( time );
        prop = dt;
    }
}
void Item::setDataTime(QJsonObject data, QString key, QTime &prop)
{
    if( data.contains(key) )
    {
        prop = QTime::fromString(data[key].toString(),"HH:mm:ss");
    }
}
void Item::setDataDate(QJsonObject data, QString key, QDate &prop)
{
    if( data.contains(key) )
    {
        prop = QDate::fromString(data[key].toString(),"yyyy-MM-dd");
    }
}
void Item::setDataByteArray(QJsonObject data, QString key, QByteArray &prop)
{
    if( data.contains(key) )
    {
        prop = data[key].toVariant().toByteArray();
    }
}
void Item::setDataVariant(QJsonObject data, QString key, QVariant &prop)
{
    if( data.contains(key) )
    {
        prop = data[key].toVariant();
    }
}
