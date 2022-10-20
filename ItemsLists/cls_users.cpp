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

#include "cls_users.h"
#include "database.h"
/*
 * GETTER
*/
QMap<int, User *> *Users::all() const
{
    return map_all;
}
QMap<int, User *> *Users::actifs() const
{
    return map_actifs;
}
QMap<int, User *> *Users::inactifs() const
{
    return map_inactifs;
}
QMap<int, User *> *Users::superviseurs() const
{
    return map_superviseurs;
}
QMap<int, User *> *Users::parents() const
{
    return map_parents;
}
QMap<int, User *> *Users::liberaux() const
{
    return map_liberaux;
}
QMap<int, User *> *Users::comptables() const
{
    return map_comptables;
}
QMap<int, User *> *Users::medecins() const
{
    return map_medecins;
}

/*!
 * \brief Users::Users
 * Initialise les Maps
 */
Users::Users(QObject *parent) : ItemsList(parent)
{
    map_all             = new QMap<int, User*>();
    map_actifs          = new QMap<int, User*>();
    map_inactifs        = new QMap<int, User*>();
    map_superviseurs    = new QMap<int, User*>();
    map_liberaux        = new QMap<int, User*>();
    map_parents         = new QMap<int, User*>();
    map_comptables      = new QMap<int, User*>();
    map_medecins        = new QMap<int, User*>();
}

/*!
 * \brief Users::addUser
 * Cette fonction va ajouter l'utilisateur passé en paramètre
 * aux map en fonction de différents critères.
 *
 * \param usr l'utilisateur que l'on veut ajouter
 * \return true si l'utilisateur est ajouté
 * \return false si le paramètre usr est un Q_NULLPTR
 * \return false si l'utilisateur est déjà présent
 */
bool Users::add(User *usr)
{
    if( usr == Q_NULLPTR)
        return false;

    auto itusr = map_all->constFind(usr->id());
    if( itusr != map_all->constEnd() )
        itusr.value()->setData(usr->datas());
    else
        map_all->insert(usr->id(), usr);
    mapsclean(usr);
    if (!usr->isDesactive())
    {
        map_actifs->insert(usr->id(), usr);
        if( usr->isResponsable() || usr->isResponsableOuAssistant())
            map_superviseurs->insert(usr->id(), usr);
        if( usr->isLiberal() )
            map_liberaux->insert(usr->id(), usr);
        if( usr->isSoignant() && !usr->isRemplacant() )
            map_parents->insert(usr->id(), usr);
        if( usr->isComptable() )
            map_comptables->insert(usr->id(), usr);
        if( usr->isMedecin() )
            map_medecins->insert(usr->id(), usr);
    }
    else
        map_inactifs->insert(usr->id(), usr);
    return true;
}

void Users::addList(QList<User*> listusr)
{
    foreach (User *usr, listusr)
        add(usr);
}

/*!
 * \brief Users::getById
 * \param id l'id de l'utilisateur recherché
 * \param loadDetails
 * \return Q_NULLPTR si aucun utilisateur trouvé
 * \return User* l'utilisateur correspondant à l'id
 */
User* Users::getById(int id, Item::UPDATE upd)
{
    QMap<int, User*>::const_iterator ituser = map_all->constFind(id);
    User *result;
    if( ituser == map_all->constEnd() )
    {
        QJsonObject jsonUser = DataBase::I()->loadUserData(id);
        if( jsonUser.isEmpty() )
            return Q_NULLPTR;
        result = new User(jsonUser);
        add(result);
    }
    else
    {
        result = ituser.value();
        if (upd == Item::Update)
        {
            QJsonObject jsonUser = DataBase::I()->loadUserData(id);
            if( !jsonUser.isEmpty() )
            {
                result->setData(jsonUser);
                recalcStatut(result);
            }
        }
    }
    return result;
}

/*!
 * \brief Users::reload
 * \param id l'id de l'utilisateur recherché
 * \return Q_NULLPTR si aucun utilisateur trouvé
 * \return User* l'utilisateur correspondant à l'id
 */
void Users::reload(User *usr)
{
    QJsonObject jsonUser = DataBase::I()->loadUserData(usr->id());
    if( jsonUser.isEmpty() )
        return;
    usr->setData(jsonUser);
    if( map_all->constFind(usr->id()) == map_all->constEnd() )
        add(usr);
}

bool Users::recalcStatut(User *usr)
{
    return add(usr);
}

/*!
 * \brief Users::initListe
 * Charge l'ensemble des utilisateurs
 * et les ajoute à la classe Users
 */
void Users::initListe()
{
    DataBase::I()->NettoieTableUsers();
    int idcomptable     = User::ROLE_INDETERMINE;
    int idparent        = User::ROLE_INDETERMINE;
    int idsuperviseur   = User::ROLE_INDETERMINE;
    if (userconnected() != Q_NULLPTR)
    {
        idcomptable     = userconnected()->idcomptable();
        idparent        = userconnected()->idparent();
        idsuperviseur   = userconnected()->idsuperviseur();
    }
    QList<User*> listusers = DataBase::I()->loadUsers();
    epurelist(map_all, &listusers);
    mapsclean();
    addList(listusers);
    for (auto it = map_actifs->constBegin(); it != map_actifs->constEnd(); ++it)
    {
        User *usr = const_cast<User*>(it.value());
         if (usr->login() == NOM_ADMINISTRATEUR)
        {
            m_useradmin = usr;
            break;
        }
    }
    if (userconnected() != Q_NULLPTR)
    {
        userconnected() ->setidparent(idparent);
        userconnected() ->setidcomptable(idcomptable);
        userconnected() ->setidsuperviseur(idsuperviseur);
    }
}

/*!
 * \brief Users::initListe
 * Charge l'ensemble des utilisateurs avec des renseignements succincts
 * et les ajoute à la classe Users
 */
void Users::initShortListe()
{
    QList<User*> listusers = DataBase::I()->loadUsersShortListe();
    epurelist(map_all, &listusers);
    mapsclean();
    foreach (User *usr, listusers)
        if( usr != Q_NULLPTR)
        {
            auto itusr = map_all->constFind(usr->id());
            if( itusr != map_all->constEnd() )
                itusr.value()->setData(usr->datas());
            else
                map_all->insert(usr->id(), usr);
            if (usr->login() == NOM_ADMINISTRATEUR)
                m_useradmin = usr;
        }
}

/*!
 * \brief Users::remplaceUserListes
 * Remplace dans chaque liste un User par un autre user avec le même id
 * utilisé p.e. quand on a un nouveau currentuser pour remplacer l'ancien dans les listes
 */
void Users::remplaceUserListes(User *usr)
{
    auto it = map_all->find(usr->id());
    if (it != map_all->end())
    {
        delete it.value();
        map_all->insert(it.key(), usr);
        auto ita = map_actifs->find(usr->id());
        if (ita != map_actifs->end())
            map_actifs->insert(ita.key(), usr);
        auto iti = map_inactifs->find(usr->id());
        if (iti != map_inactifs->end())
            map_inactifs->insert(iti.key(), usr);
        auto its = map_superviseurs->find(usr->id());
        if (its != map_superviseurs->end())
            map_superviseurs->insert(its.key(), usr);
        auto itl = map_liberaux->find(usr->id());
        if (itl != map_liberaux->end())
            map_liberaux->insert(itl.key(), usr);
        auto itp = map_parents->find(usr->id());
        if (itp != map_parents->end())
            map_parents->insert(itp.key(), usr);
        auto itc = map_comptables->find(usr->id());
        if (itc != map_comptables->end())
            map_comptables->insert(itc.key(), usr);
        auto itm = map_medecins->find(usr->id());
        if (itm != map_medecins->end())
            map_medecins->insert(itm.key(), usr);
    }
    else
        add(usr);
}

void Users::SupprimeUser(User *usr)
{
    if( usr == Q_NULLPTR)
        return;
    map_all ->remove(usr->id());
    mapsclean(usr);
    DataBase::I()->SupprRecordFromTable(usr->id(), CP_ID_USR, TBL_UTILISATEURS);
    delete usr;
}

void Users::mapsclean(User *usr)
{
    if (usr == Q_NULLPTR)
    {
        map_actifs          ->clear();
        map_inactifs        ->clear();
        map_superviseurs    ->clear();
        map_liberaux        ->clear();
        map_parents         ->clear();
        map_comptables      ->clear();
        map_medecins        ->clear();
    }
    else
    {
        map_actifs        ->remove(usr->id());
        map_inactifs      ->remove(usr->id());
        map_superviseurs  ->remove(usr->id());
        map_liberaux      ->remove(usr->id());
        map_parents       ->remove(usr->id());
        map_comptables    ->remove(usr->id());
        map_medecins      ->remove(usr->id());
    }
}



