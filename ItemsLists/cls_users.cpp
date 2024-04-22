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
QMap<int, User *> *Users::comptablesActes() const
{
    return map_comptablesactes;
}
QMap<int, User *> *Users::comptablesSaufActes() const
{
    return map_comptablessaufactes;
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
    map_all                 = new QMap<int, User*>();
    map_actifs              = new QMap<int, User*>();
    map_inactifs            = new QMap<int, User*>();
    map_superviseurs        = new QMap<int, User*>();
    map_liberaux            = new QMap<int, User*>();
    map_parents             = new QMap<int, User*>();
    map_comptablesactes     = new QMap<int, User*>();
    map_comptablessaufactes = new QMap<int, User*>();
    map_medecins            = new QMap<int, User*>();
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
bool Users::addUser(User *user)
{
    if( user == Q_NULLPTR)
        return false;
    int iduser = user->id();
    add(map_all, user, Item::Update);
    auto itusr = map_all->constFind(iduser);
    User *usr = itusr.value();
    if(usr)
    {
        mapsclean(usr);
        if (!usr->isDesactive())
        {
            usr->setmodecomptable();
            map_actifs->insert(usr->id(), usr);
            if( usr->isResponsable() || usr->isAlterneResponsableEtAssistant())
                map_superviseurs->insert(usr->id(), usr);
            if( usr->isLiberal() || usr->isLiberalSEL())
                map_liberaux->insert(usr->id(), usr);
            if( usr->isSoignant() && !usr->isRemplacant() )
                map_parents->insert(usr->id(), usr);
            if( usr->modecomptable().testFlag(User::ComptaMedicalActs))
                map_comptablesactes->insert(usr->id(), usr);
            if( usr->modecomptable().testFlag(User::ComptaNoMedical))
                map_comptablessaufactes->insert(usr->id(), usr);
            if( usr->isMedecin() )
                map_medecins->insert(usr->id(), usr);
        }
        else
            map_inactifs->insert(usr->id(), usr);
    }
    return true;
}

void Users::addList(QList<User*> listusr)
{
    foreach (User *usr, listusr)
        if (usr != Q_NULLPTR)
            addUser(usr);
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
    User *result = Q_NULLPTR;;
    if( ituser == map_all->constEnd() )
    {
        QJsonObject jsonUser = DataBase::I()->loadUserData(id);
        if( !jsonUser.isEmpty() )
        {
            result = new User(jsonUser);
            addUser(result);
        }
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
        addUser(usr);
}

bool Users::recalcStatut(User *usr)
{
    return addUser(usr);
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
        idcomptable     = userconnected()->idcomptableactes();
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
            continue;
        }
        CalcCompteEncaissementActes(usr);
    }
    if (userconnected() != Q_NULLPTR)
    {
        userconnected() ->setidparent(idparent);
        userconnected() ->setidcomptableactes(idcomptable);
        userconnected() ->setidsuperviseur(idsuperviseur);
    }
}

/*!
 *  \brief Users::CalcCompteEncaissementActes
 * Détermine le compte bancaire sur lequel seront encissés les paiements pour ce user
*/
void Users::CalcCompteEncaissementActes(User *usr)
{
    usr->setidcompteencaissementhonoraires(0);
    if (usr->isSocComptable())
        usr->setidcompteencaissementhonoraires(usr->idcomptepardefaut());
    else if (usr->isSoignant())
    {
        if (usr->isLiberal())
            usr->setidcompteencaissementhonoraires(usr->idcomptepardefaut());
        else if (usr->isLiberalSEL() || usr->isSoignantSalarie())
        {
            auto it = map_comptablesactes->constFind(usr->idemployeur());
            if (it != map_comptablesactes->cend())
            {
                User *usrcpt = it.value();
                if (usrcpt)
                    usr->setidcompteencaissementhonoraires(usrcpt->idcomptepardefaut());
            }
        }
    }
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
        map_actifs              ->clear();
        map_inactifs            ->clear();
        map_superviseurs        ->clear();
        map_liberaux            ->clear();
        map_parents             ->clear();
        map_comptablesactes     ->clear();
        map_comptablessaufactes ->clear();
        map_medecins            ->clear();
    }
    else
    {
        User *user;
        user = qobject_cast<User*>(map_actifs->take(usr->id()));
        if (user != usr)
            delete user;
        user = qobject_cast<User*>(map_inactifs->take(usr->id()));
        if (user != usr)
            delete user;
        user = qobject_cast<User*>(map_superviseurs->take(usr->id()));
        if (user != usr)
            delete user;
        user = qobject_cast<User*>(map_liberaux->take(usr->id()));
        if (user != usr)
            delete user;
        user = qobject_cast<User*>(map_parents->take(usr->id()));
        if (user != usr)
            delete user;
        user = qobject_cast<User*>(map_comptablesactes->take(usr->id()));
        if (user != usr)
            delete user;
        user = qobject_cast<User*>(map_comptablessaufactes->take(usr->id()));
        if (user != usr)
            delete user;
        user = qobject_cast<User*>(map_medecins->take(usr->id()));
        if (user != usr)
            delete user;
    }
}



