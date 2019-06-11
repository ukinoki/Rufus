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

class Item : public QObject
{
    Q_OBJECT
public: //STATIC
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QDate datedujour);
    static QMap<QString,QVariant> CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour = QDate::currentDate());


public:
    explicit Item(QObject *parent = Q_NULLPTR);


protected:

    void setDataString(QJsonObject data, QString key, QString &prop, bool useTrim=false);
    void setDataInt(QJsonObject data, QString key, int &prop);
    void setDataLongLongInt(QJsonObject data, QString key, qlonglong &prop);
    void setDataDouble(QJsonObject data, QString key, double &prop);
    void setDataBool(QJsonObject data, QString key, bool &prop);
    void setDataDateTime(QJsonObject data, QString key, QDateTime &prop);
    void setDataDate(QJsonObject data, QString key, QDate &prop);
    void setDataByteArray(QJsonObject data, QString key, QByteArray &prop);
    void setDataVariant(QJsonObject data, QString key, QVariant &prop);

private:

signals:

public slots:
};

#endif // CLS_ITEM_H
