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

#include "cls_correspondants.h"

/*
 * GETTER
*/
QMap<int, Correspondant *> *Correspondants::correspondants() const
{
    return map_all;
}

/*!
 * \brief Correspondants::Correspondants
 * Initialise la map Correspondants
 */
Correspondants::Correspondants(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, Correspondant*>();
}

/*!
 * \brief Correspondants::getById
 * \param id l'id du correspondant recherché
 * \param loadDetails   -> charge les détails si ce n'est pas déjà fait
 * \param addToList     -> ajoute à la liste des correspondants s'il ne s'y trouve pas encore
 * \return Q_NULLPTR si aucun correspondant trouvé
 * \return Correspondant* le correspondant correspondant à l'id
 */
Correspondant* Correspondants::getById(int id, Item::LOADDETAILS loaddetails, ADDTOLIST addToList)
{
    QMap<int, Correspondant*>::const_iterator itcor = map_all->constFind(id);
    Correspondant *result;
    if( itcor == map_all->constEnd() )
        result = new Correspondant();
    else
    {
        result = itcor.value();
        if(loaddetails == Item::NoLoadDetails)
            return result;
        addToList = ItemsList::NoAddToList;
    }

    if( !result->isallloaded())
    {
        QJsonObject jsonCorrespondant = DataBase::I()->loadCorrespondantData(id);
        if( jsonCorrespondant.isEmpty() )
        {
            delete result;
            return Q_NULLPTR;
        }
        else
            result->setData(jsonCorrespondant);
    }
    if( addToList == ItemsList::AddToList)
        add(map_all, result );
    return result;
}

/*!
 * \brief Correspondants::loadAll
 * Charge l'ensemble des data d'un correspondant et
 * update les datas du crorrespondant de même id s'il est présent dans la liste et si on utilise le paramètre Item::ForceUpdate
 * insère le correspondant dans la liste s'il n'est pas présent
 * et les ajoute à la classe Correspondants
 */
void Correspondants::loadAllData(Correspondant *cor, Item::UPDATE upd)
{
    if (cor == Q_NULLPTR)
        return;
    if (!cor->isallloaded() || upd == Item::Update)
    {
        QJsonObject jsoncor = DataBase::I()->loadCorrespondantData(cor->id());
        if( !jsoncor.isEmpty() )
            cor->setData(jsoncor);
    }
    add (map_all, cor, Item::Update);
}


/*!
 * \brief Correspondants::initListe correspondants
 * Charge l'ensemble des correspondants
 * et les ajoute à la classe Correspondants
 */
void Correspondants::initListe(bool all)
{
    QList<Correspondant*> listcorrespondants;
    if (all)
        listcorrespondants = DataBase::I()->loadCorrespondantsALL();
    else
        listcorrespondants = DataBase::I()->loadCorrespondants();
    epurelist(map_all, &listcorrespondants);
    addList(map_all, &listcorrespondants, Item::Update);
}

QStringList Correspondants::autresprofessions()
{
    QStringList listprof = QStringList();
    for (auto it = map_all->constBegin(); it != map_all->constEnd(); ++it)
    {
        Correspondant *cor = const_cast<Correspondant*>(it.value());
        if (!cor->ismedecin())
            listprof << cor->metier();
    }
    return listprof;
}

void Correspondants::SupprimeCorrespondant(Correspondant *cor)
{
    if (cor == Q_NULLPTR)
        return;
    QString id = QString::number(cor->id());
    Supprime(map_all, cor);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg  = null where idcormedmg  = " + id);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe1 = null where idcormedspe1 = " + id);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe2 = null where idcormedspe2 = " + id);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe3 = null where idcormedspe3 = " + id);
}

