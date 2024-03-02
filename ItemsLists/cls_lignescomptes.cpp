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

#include "cls_lignescomptes.h"

LignesComptes::LignesComptes(QObject * parent) : ItemsList(parent)
{
    map_lignes = new QMap<int, LigneCompte*>();
}

LignesComptes::~LignesComptes()
{
    clearAll(map_lignes);
    delete map_lignes;
}

QMap<int, LigneCompte*>* LignesComptes::lignescomptes() const
{
    return map_lignes;
}

LigneCompte* LignesComptes::getById(int id)
{
    QMap<int, LigneCompte*>::const_iterator itlign = map_lignes->constFind(id);
    if( itlign == map_lignes->constEnd() )
    {
        QJsonObject data = DataBase::I()->loadLigneCompteDataById(id);
        if (data != QJsonObject{})
        {
            LigneCompte * lign = new LigneCompte(data);
            add( map_lignes, lign );
            return lign;
        }
        return Q_NULLPTR;
    }
    return itlign.value();
}

void LignesComptes::reloadLigne(LigneCompte *lign)
{
    if (lign == Q_NULLPTR)
        return;
    lign->setData(DataBase::I()->loadLigneCompteDataById(lign->id()));
}

void LignesComptes::initListe(int idcompte)
{
    QList<LigneCompte*> listlignes = DataBase::I()->loadLignesComptesByCompte(idcompte);
    epurelist(map_lignes, &listlignes);
    addList(map_lignes, &listlignes);
}

void LignesComptes::SupprimeLigne(LigneCompte *lign)
{
    Supprime(map_lignes, lign);
}

QMap<int, bool> LignesComptes::initListeLignesByIdCompte(int id)
{
    QMap<int, bool> maplignes;
    foreach (LigneCompte *lign, *map_lignes)
        if (lign->idcompte() == id)
            maplignes.insert(lign->id(), true);
    return maplignes;
}


