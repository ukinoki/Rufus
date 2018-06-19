#ifndef CLS_USERS_H
#define CLS_USERS_H

#include <QObject>
#include "cls_user.h"
/*!
 * \brief The Users class
 * Cette classe gére les différents collections de "User"
 */
class Users
{
private:
    QMap<int, User*> *m_users = NULL;           //!<Collection de tous les User
    QMap<int, User*> *m_superviseurs = NULL;    //!<Collection des superviseurs : User->isResponsable()
    QMap<int, User*> *m_parents = NULL;         //!<Collection des parents : User->isLiberal()
    QMap<int, User*> *m_liberaux = NULL;        //!<Collection des liberaux : User->isSoignant() && !User->isRemplacant()

public:
    //GETTER
    QMap<int, User *> *users() const;
    QMap<int, User *> *superviseurs() const;
    QMap<int, User *> *parents() const;
    QMap<int, User *> *liberaux() const;


    Users();
    bool addUser(User *usr);
    User* getUserById(int id);


};

#endif // CLS_USERS_H
