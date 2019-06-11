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

#include "cls_usersconnectes.h"

UsersConnectes::UsersConnectes(QObject * parent) : ItemsList(parent)
{
    m_usersconnectes = new QMap<QString, UserConnecte*>();
}

UsersConnectes::~UsersConnectes()
{
    clearAll(m_usersconnectes);
    delete m_usersconnectes;
}

QMap<QString, UserConnecte*>* UsersConnectes::usersconnectes() const
{
    return m_usersconnectes;
}

void UsersConnectes::addList(QList<UserConnecte*> listusr)
{
    QList<UserConnecte*>::const_iterator it;
    for( it = listusr.constBegin(); it != listusr.constEnd(); ++it )
    {
        UserConnecte* item = const_cast<UserConnecte*>(*it);
        add( m_usersconnectes, item->stringid(), item );
    }
}

UserConnecte* UsersConnectes::getByKey(QString key)
{
    QMap<QString, UserConnecte*>::const_iterator itcpt = m_usersconnectes->find(key) ;
    if( itcpt == m_usersconnectes->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void UsersConnectes::initListe()
{
    QList<UserConnecte*> listUsersConnectes;
    addList(DataBase::I()->loadUsersConnectes());
}

void UsersConnectes::SupprimeUserConnecte(UserConnecte *usr)
{
    if (usr == Q_NULLPTR)
        return;
    DataBase::I()->StandardSQL("delete from " TBL_USERSCONNECTES " where " CP_IDUSER_USRCONNECT " = " + QString::number(usr->id()) + " and " CP_MACADRESS_USRCONNECT " = '" + usr->macadress() + "'");
    remove(m_usersconnectes, usr);

}
UserConnecte* UsersConnectes::CreationUserConnecte()
{
    bool ok;
    UserConnecte *usr = Q_NULLPTR;
//    QString MAJConnexionRequete;
//    bool MAJsaldat = false;

//    QString req = "select iduser from " TBL_USERSCONNECTES
//                      " where " CP_MACADRESS_USRCONNECT "= '" + macadress + "'"
//                      " and " CP_IDUSER_USRCONNECT " = " + QString::number(DataBase::I()->getUserConnected()->id());
//    //qDebug() << req;
//    QVariantList usrdata = DataBase::I()->getFirstRecordFromStandardSelectSQL(req, ok, "Rufus::MetAJourUserConnectes()");
//    if (!ok)
//        return Q_NULLPTR;
//    if (usrdata.size()>0)
//        MAJConnexionRequete = "UPDATE " TBL_USERSCONNECTES " SET"
//                              CP_HEUREDERNIERECONNECTION_USRCONNECT " = NOW(), "
//                              CP_IDUSER_USRCONNECT " = " + QString::number(DataBase::I()->getUserConnected()->id()) + ","
//                              CP_NOMPOSTE_USRCONNECT " = '" + QHostInfo::localHostName().left(60) + "'"
//                              " where " CP_MACADRESS_USRCONNECT " = '" + macadress + "'"
//                              " and " CP_IDUSER_USRCONNECT " = " + QString::number(DataBase::I()->getUserConnected()->id()) ;
//    else
//    {
//        MAJsaldat = true;
    QString macadress =  Utils::getMACAdress() + " - " + DataBase::I()->getUserConnected()->getLogin();
    QString MAJConnexionRequete = "insert into " TBL_USERSCONNECTES "(" CP_HEUREDERNIERECONNECTION_USRCONNECT ", "
                                                                        CP_IDUSER_USRCONNECT ", "
                                                                        CP_IDUSERSUPERVISEUR_USRCONNECT ", "
                                                                        CP_IDUSERCOMPTABLE_USRCONNECT ", "
                                                                        CP_IDUSERPARENT_USRCONNECT ", "
                                                                        CP_NOMPOSTE_USRCONNECT ", "
                                                                        CP_MACADRESS_USRCONNECT ")"
                               " VALUES(NOW()," +
                               QString::number(DataBase::I()->getUserConnected()->id()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserActeSuperviseur()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserComptable()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserParent()) +",'" +
                               QHostInfo::localHostName().left(60) + "', '" +
                               macadress + "')";
    //qDebug() << MAJConnexionRequete;
    DataBase::I()->StandardSQL(MAJConnexionRequete, "Rufus::MetAJourUserConnectes()");
    usr = new UserConnecte();
    usr->setstringid(QString::number(DataBase::I()->getUserConnected()->id()) + TCPMSG_Separator + macadress);
    usr->setiduser(DataBase::I()->getUserConnected()->id());
    usr->setidsuperviseur(DataBase::I()->getUserConnected()->getIdUserActeSuperviseur());
    usr->setidcomptable(DataBase::I()->getUserConnected()->getIdUserComptable());
    usr->setidparent(DataBase::I()->getUserConnected()->getIdUserParent());
    usr->setheurederniereconnexion(DataBase::I()->ServerDateTime().time());
    usr->setnomposte(QHostInfo::localHostName().left(60));
    usr->setmacadress(macadress);
    add( m_usersconnectes, QString::number(usr->id()) + TCPMSG_Separator + usr->macadress(), usr );

    return usr;
}
