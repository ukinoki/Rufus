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

#include "cls_sites.h"

/*
 * GETTER
*/
QMap<int, Site *> *Sites::sites() const
{
    return map_all;
}

/*!
 * \brief Sites::Sites
 * Initialise la map m_sites
 */
Sites::Sites(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, Site*>();
}

/*!
 * \brief Sites::getById
 * \param id l'id du site recherché
 * \return Q_NULLPTR si aucun site trouvé
 * \return Site* le site correspondant à l'id
 */
Site* Sites::getById(int id, bool reload)
{
    QMap<int, Site*>::const_iterator itsit = map_all->constFind(id);
    if(  itsit ==  map_all->constEnd() )
    {
        Site* sit = DataBase::I()->loadSiteById(id);
        if (sit)
            add(  map_all, sit, Item::Update );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<Site*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        Site* sit = DataBase::I()->loadSiteById(id);
        if (sit)
        {
            itsit.value()->setData(sit->datas());
            delete sit;
        }
    }
    return  itsit.value();
}


/*!
 * \brief Sites::initListe sites
 * Charge l'ensemble des sites
 * et les ajoute à la classe Sites
 */
void Sites::initListe()
{
    int id = 0;
    if (m_currentsite != Q_NULLPTR)
        id = m_currentsite->id();
    QList<Site*> listsites = DataBase::I()->loadSites();
    epurelist(map_all, &listsites);
    addList(map_all, &listsites);
    m_currentsite = (id>0? getById(id) : Q_NULLPTR);
}

/*!
 * \brief Sites::initListeByUser sites
 * Charge l'ensemble des sites utilisés par un utilisateur
 */
QList<Site*> Sites::initListeByUser(int idusr)
{
    QList<Site*> listsites = QList<Site*>();
    QList<int> listid = DataBase::I()->loadidSitesByUser(idusr);
    foreach (int id, listid)
        if (getById(id) != Q_NULLPTR)
            listsites << getById(id);
    return listsites;
}

void Sites::SupprimeSite(Site* man)
{
    Supprime(map_all, man);
}

Site* Sites::CreationSite(QHash<QString, QVariant> sets)
{
    Site *sit = Q_NULLPTR;
    int idSite = 0;
    DataBase::I()->locktables(QStringList() << TBL_LIEUXEXERCICE);
    idSite = DataBase::I()->selectMaxFromTable(CP_ID_SITE, TBL_LIEUXEXERCICE, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idSite;
        sets[CP_ID_SITE] = idSite;
        result = DataBase::I()->InsertSQLByBinds(TBL_LIEUXEXERCICE, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce fabricant dans la base!"));
        return sit;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_SITE] = idSite;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_NOM_SITE)                           data[champ] = itset.value().toString();
        else if (champ == CP_ADRESSE1_SITE)                 data[champ] = itset.value().toString();
        else if (champ == CP_ADRESSE2_SITE)                 data[champ] = itset.value().toString();
        else if (champ == CP_ADRESSE3_SITE)                 data[champ] = itset.value().toString();
        else if (champ == CP_CODEPOSTAL_SITE)               data[champ] = itset.value().toString();
        else if (champ == CP_VILLE_SITE)                    data[champ] = itset.value().toString();
        else if (champ == CP_TELEPHONE_SITE)                data[champ] = itset.value().toString();
        else if (champ == CP_FAX_SITE)                      data[champ] = itset.value().toString();
        else if (champ == CP_COULEUR_SITE)                  data[champ] = itset.value().toString();
    }
    sit = new Site(data);
    if (sit != Q_NULLPTR)
        map_all->insert(sit->id(), sit);
    return sit;
}

