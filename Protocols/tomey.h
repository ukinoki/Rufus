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

#ifndef Tomey_H
#define Tomey_H

/**
* \file Tomey.h
* \brief Cette classe gére les protocols de Tomey/Rodenstock
*
*
*/

#include <QBuffer>
#include <QJsonObject>
#include <QHostInfo>
#include <QObject>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QJsonDocument>

class Tomey : public QObject
{
    Q_OBJECT
private:
    Tomey();
    static Tomey *instance;

public:
    static Tomey*        I();

    void LectureDonneesRC5000Form(QString Mesure);
    void LectureDonneesRC5000(QString Mesure);
    void LectureDonneesTOP1000(QString Mesure);
    void LectureDonneesAL6400(QString Mesure);

};

#endif // DataBase_H
