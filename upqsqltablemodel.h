/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UPQSQLTABLEMODEL_H
#define UPQSQLTABLEMODEL_H

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QDebug>

class UpQSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit UpQSqlTableModel(QSqlQuery quer, QObject *parent = Q_NULLPTR);
    explicit UpQSqlTableModel(QString Table, QObject *parent = Q_NULLPTR, QSqlDatabase db = QSqlDatabase());
    ~UpQSqlTableModel();

private:
    QSqlQuery       gquer;
};

#endif // UPQSQLTABLEMODEL_H
