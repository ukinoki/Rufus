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

/*!
 *  \brief Calcul de l'âge
 *
 *  Methode qui permet ????
 *
 *  \param datedenaissance : la date de naissance
 *  \param Sexe : le sexe de la personne [""]
 *  \param datedujour : la date du jour [Date du jour]
 *
 *  \return un object contenant :
 * toString : une chaine de caractères ( ex: 2 ans 3 mois )
 * annee : l'age brut de la personne
 * mois :
 * icone : l'icone à utiliser [man women, girl, boy, kid, baby]
 * formule : une valeur parmi [l'enfant, la jeune, le jeune, madame, monsieur]
 *
 */
QMap<QString,QVariant> Item::CalculAge(QDate datedenaissance)
{
    return Item::CalculAge(datedenaissance, "", QDate::currentDate());
}
QMap<QString,QVariant> Item::CalculAge(QDate datedenaissance, QDate datedujour)
{
    return Item::CalculAge(datedenaissance, "", datedujour);
}
QMap<QString,QVariant> Item::CalculAge(QDate datedenaissance, QString Sexe, QDate datedujour)
{
    QMap<QString,QVariant>  Age;
    int         AnneeNaiss, MoisNaiss, JourNaiss;
    int         AnneeCurrent, MoisCurrent, JourCurrent;
    int         AgeAnnee, AgeMois;
    int         FormuleMoisJourNaissance, FormuleMoisJourAujourdhui;

    AnneeNaiss                  = datedenaissance.toString("yyyy").toInt();
    MoisNaiss                   = datedenaissance.toString("MM").toInt();
    JourNaiss                   = datedenaissance.toString("dd").toInt();
    AnneeCurrent                = datedujour.toString("yyyy").toInt();
    MoisCurrent                 = datedujour.toString("MM").toInt();
    JourCurrent                 = datedujour.toString("dd").toInt();
    FormuleMoisJourNaissance    = (MoisNaiss*100) + JourNaiss;
    FormuleMoisJourAujourdhui   = (MoisCurrent*100) + JourCurrent;
    AgeAnnee                    = AnneeCurrent - AnneeNaiss;
    AgeMois                     = MoisCurrent - MoisNaiss;
    if (FormuleMoisJourAujourdhui < FormuleMoisJourNaissance)   AgeAnnee --;
    if (JourNaiss > JourCurrent)                                AgeMois --;
    if (AgeMois < 0)                                            AgeMois = AgeMois + 12;

    Age["annee"] = AgeAnnee;
    Age["mois"]  = AgeMois;

    // On formate l'âge pour l'affichage
    switch (AgeAnnee) {
    case 0:
        if (datedenaissance.daysTo(datedujour) > 31)
            Age["toString"]               = QString::number(AgeMois) + " mois";
        else
            Age["toString"]               = QString::number(datedenaissance.daysTo(datedujour)) + " jours";
        break;
    case 1: case 2: case 3: case 4:
        Age["toString"]                    = QString::number(AgeAnnee) + " an";
        if (AgeAnnee > 1) Age["toString"]  = Age["toString"].toString() + "s";
        if (AgeMois > 0)  Age["toString"]  = Age["toString"].toString() + " " + QString::number(AgeMois) + " mois";
        break;
    default:
        Age["toString"]                    = QString::number(AgeAnnee) + " ans";
        break;
    }

    // On cherche l'icone correspondant au mieux à la personne
    QString img = "silhouette";
    if (AgeAnnee < 2)                       img = "baby";
    else if (AgeAnnee < 8)                  img = "kid";
    else if (AgeAnnee < 16 && Sexe == "M")  img = "boy";
    else if (AgeAnnee < 16 && Sexe == "F")  img = "girl";
    else if (Sexe =="M")                    img = "man";
    else if (Sexe =="F")                    img = "women";
    Age["icone"] = img;

    // On cherche la formule de polistesse correspondant au mieux à la personne
    QString formule = "";
    if (AgeAnnee < 11)                  formule = "l'enfant";
    else if (AgeAnnee < 18) {
        if (Sexe == "F")                formule = "la jeune";
        if (Sexe == "M")                formule = "le jeune";
    }
    else {
        if (Sexe == "F")                formule = "madame";
        if (Sexe == "M")                formule = "monsieur";
    }
    Age["formule"] = formule;

    return Age;
}


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
