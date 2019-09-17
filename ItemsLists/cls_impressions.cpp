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

#include "cls_impressions.h"


Impressions::Impressions(QObject *parent) : ItemsList(parent)
{
    map_impressions = new QMap<int, Impression*>();
}

QMap<int, Impression *> *Impressions::impressions() const
{
    return map_impressions;
}

Impression* Impressions::getById(int id)
{
    QMap<int, Impression*>::const_iterator itdoc = map_impressions->find(id);
    if( itdoc == map_impressions->constEnd() )
        return Q_NULLPTR;
    return itdoc.value();
}

/*!
 * \brief Impressions::initListe
 * Charge l'ensemble des impressions accessibles à l'utilisateur en cours
 * et les ajoute à la classe Impressions
 */
void Impressions::initListe()
{
    QList<Impression *> listimpressions = DataBase::I()->loadImpressions();
    epurelist(map_impressions, &listimpressions);
    addList(map_impressions, &listimpressions, Item::Update);
}


DossiersImpressions::DossiersImpressions()
{
    map_dossiersimpressions = new QMap<int, DossierImpression*>();
}

QMap<int, DossierImpression *> *DossiersImpressions::dossiersimpressions() const
{
    return map_dossiersimpressions;
}

DossierImpression* DossiersImpressions::getById(int id)
{
    QMap<int, DossierImpression*>::const_iterator itdoc = map_dossiersimpressions->find(id);
    if( itdoc == map_dossiersimpressions->constEnd() )
        return Q_NULLPTR;
    return itdoc.value();
}

/*!
 * \brief DossiersImpressions::initListeDocument
 * Charge l'ensemble des dossiers d'impressions accessibles à l'utilisateur en cours
 * et les ajoute à la classe DossiersImpressions
 */
void DossiersImpressions::initListe()
{
    QList<DossierImpression *> listdossiers = DataBase::I()->loadDossiersImpressions();
    epurelist(map_dossiersimpressions, &listdossiers);
    addList(map_dossiersimpressions, &listdossiers);
}

