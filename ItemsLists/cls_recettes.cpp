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

#include "cls_recettes.h"

Recettes::Recettes(QObject *parent) : ItemsList(parent)
{
    map_recettes = new QMap<int, Recette*>();
}

QMap<int, Recette *> *Recettes::recettes() const
{
    return map_recettes;
}

/*!
 * \brief Recettes::initListeRecettes
 * Charge l'ensemble des Recettes
 * et les ajoute à la classe Recettes
 */
void Recettes::initListe(QMap<QString, QDate> DateMap)
{
    QList<Recette*> listrecettes = DataBase::I()->loadRecettesByDate(DateMap["DateDebut"], DateMap["DateFin"]);
    epurelist(map_recettes, &listrecettes);
    addList(map_recettes, &listrecettes, Item::Update);
}

Recette* Recettes::getById(int id)
{
    QMap<int, Recette*>::const_iterator itcpt = map_recettes->find(id);
    if( itcpt == map_recettes->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}
