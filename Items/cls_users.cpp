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

#include "cls_users.h"
#include "database.h"
/*
 * GETTER
*/
QMap<int, User *> *Users::all() const
{
    return m_users;
}
QMap<int, User *> *Users::superviseurs() const
{
    return m_superviseurs;
}
QMap<int, User *> *Users::parents() const
{
    return m_parents;
}
QMap<int, User *> *Users::liberaux() const
{
    return m_liberaux;
}
QMap<int, User *> *Users::comptables() const
{
    return m_comptables;
}

/*!
 * \brief Users::Users
 * Initialise les Maps
 */
Users::Users()
{
    m_users = new QMultiMap<int, User*>();
    m_superviseurs = new QMultiMap<int, User*>();
    m_liberaux = new QMultiMap<int, User*>();
    m_parents = new QMultiMap<int, User*>();
    m_comptables = new QMultiMap<int, User*>();
}

/*!
 * \brief Users::addUser
 * Cette fonction va ajouter l'utilisateur passé en paramètre
 * aux map en fonction de différents critères.
 *
 * \param usr l'utilisateur que l'on veut ajouter
 * \return true si l'utilisateur est ajouté
 * \return false si le paramètre usr est un nullptr
 * \return false si l'utilisateur est déjà présent
 */
bool Users::addUser(User *usr)
{
    if( usr == nullptr)
        return false;

    if( m_users->contains(usr->id()) )
        return false;

    m_users->insert(usr->id(), usr);

    if( usr->isResponsable() || usr->isResponsableEtAssistant())
        m_superviseurs->insert(usr->id(), usr);

    if( usr->isLiberal() )
        m_liberaux->insert(usr->id(), usr);

    if( usr->isSoignant() && !usr->isRemplacant() )
        m_parents->insert(usr->id(), usr);

    if( usr->isComptable() )
        m_comptables->insert(usr->id(), usr);

    return true;
}

/*!
 * \brief Users::getUserById
 * \param id l'id de l'utilisateur recherché
 * \param loadDetails
 * \return nullptr si aucun utilisateur trouvé
 * \return User* l'utilisateur correspondant à l'id
 */
User* Users::getUserById(int id, bool loadDetails, bool addToList)
{
    QMap<int, User*>::const_iterator user = m_users->find(id);
    User *result;
    if( user == m_users->constEnd() )
        result = new User();
    else
    {
        result = user.value();
        if(!loadDetails)
            return result;
        addToList = false;
    }

    if( !result->isAllLoaded() )
    {
        QJsonObject jsonUser = DataBase::getInstance()->loadUserData(id);
        if( jsonUser.isEmpty() )
            return Q_NULLPTR;
        else
            result->setData(jsonUser);
    }
    if( addToList )
        addUser( result );
    return result;
}

/*!
 * \brief Users::getLoginById
 * \param id l'id de l'utilisateur recherché
 * \return "" si aucun utilisateur trouvé
 * \return QString le login de l'utilisateur correspondant à l'id
 */
QString Users::getLoginById(int id)
{
    User* user = getUserById(id);
    if( user != Q_NULLPTR)
        return user->getLogin();

    return "";
}
