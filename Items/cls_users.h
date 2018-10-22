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
    QMap<int, User*> *m_users = Q_NULLPTR;           //!<Collection de tous les User
    QMap<int, User*> *m_superviseurs = Q_NULLPTR;    //!<Collection des superviseurs : User->isResponsable()
    QMap<int, User*> *m_parents = Q_NULLPTR;         //!<Collection des parents : User->isLiberal()
    QMap<int, User*> *m_liberaux = Q_NULLPTR;        //!<Collection des liberaux : User->isSoignant() && !User->isRemplacant()
    QMap<int, User*> *m_comptables = Q_NULLPTR;      //!<Collection des comptables : User->isSocComptable() || User->isLiberal()

public:
    //GETTER
    QMap<int, User *> *all() const;
    QMap<int, User *> *superviseurs() const;
    QMap<int, User *> *parents() const;
    QMap<int, User *> *liberaux() const;
    QMap<int, User *> *comptables() const;


    Users();
    bool addUser(User *usr);
    User* getUserById(int id, bool loadDetails=false);
    QString getLoginById(int id);

};

#endif // CLS_USERS_H
