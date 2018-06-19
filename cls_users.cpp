#include "cls_users.h"
/*
 * GETTER
*/
QMap<int, User *> *Users::users() const
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

    if( usr->isResponsable() )
        m_superviseurs->insert(usr->id(), usr);

    if( usr->isLiberal() )
        m_liberaux->insert(usr->id(), usr);

    if( usr->isSoignant() && !usr->isRemplacant() )
        m_parents->insert(usr->id(), usr);

    return true;
}

/*!
 * \brief Users::getUser
 * \param id l'id de l'utilisateur recherché
 * \return nullptr si aucun utilisateur trouvé
 * \return User* l'utilisateur correspondant à l'id
 */
User* Users::getUserById(int id)
{
    QMap<int, User*>::const_iterator user = m_users->find(id);
    if( user == m_users->constEnd() )
        return nullptr;
    return user.value();
}
