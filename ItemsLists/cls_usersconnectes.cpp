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
    m_usersconnectes = new QMap<int, UserConnecte*>();
}

UsersConnectes::~UsersConnectes()
{
    clearAll(m_usersconnectes);
    delete m_usersconnectes;
}

QMap<int, UserConnecte*>* UsersConnectes::usersconnectes() const
{
    return m_usersconnectes;
}

void UsersConnectes::addList(QList<UserConnecte*> listusr)
{
    QList<UserConnecte*>::const_iterator it;
    for( it = listusr.constBegin(); it != listusr.constEnd(); ++it )
    {
        UserConnecte* item = const_cast<UserConnecte*>(*it);
        add( m_usersconnectes, item->id(), item );
    }
}

UserConnecte* UsersConnectes::getById(int id)
{
    QMap<int, UserConnecte*>::const_iterator itcpt = m_usersconnectes->find(id);
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
    DataBase::I()->SupprRecordFromTable(usr->id(), CP_IDUSER_USRCONNECT, TBL_USERSCONNECTES);
    remove(m_usersconnectes, usr);

}
UserConnecte* UsersConnectes::CreationUserConnecte()
{
    bool ok;
    UserConnecte *usr = Q_NULLPTR;
    QString MAJConnexionRequete;
    bool MAJsaldat = false;
    QString macadress =  Utils::getMACAdress() + " - " + DataBase::I()->getUserConnected()->getLogin();

    QString req = "select iduser from " TBL_USERSCONNECTES
                      " where " CP_MACADRESS_USRCONNECT "= '" + macadress + "'"
                      " and idUser = " + QString::number(DataBase::I()->getUserConnected()->id());
    //qDebug() << req;
    QVariantList usrdata = DataBase::I()->getFirstRecordFromStandardSelectSQL(req, ok, "Rufus::MetAJourUserConnectes()");
    if (!ok)
        return Q_NULLPTR;
    if (usrdata.size()>0)
        MAJConnexionRequete = "UPDATE " TBL_USERSCONNECTES " SET HeureDerniereConnexion = NOW(), "
                              " idUser = " + QString::number(DataBase::I()->getUserConnected()->id()) + ","
                              " NomPosteConnecte = '" + QHostInfo::localHostName().left(60) + "'"
                              " where MACAdressePosteConnecte = '" + macadress + "'"
                              " and idUser = " + QString::number(DataBase::I()->getUserConnected()->id()) ;
    else
    {
        MAJsaldat = true;
        MAJConnexionRequete = "insert into " TBL_USERSCONNECTES "(HeureDerniereConnexion, idUser, UserSuperviseur, UserComptable, UserParent, NomPosteConnecte, MACAdressePosteConnecte)"
                               " VALUES(NOW()," +
                               QString::number(DataBase::I()->getUserConnected()->id()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserActeSuperviseur()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserComptable()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserParent()) +",'" +
                               QHostInfo::localHostName().left(60) + "', '" +
                               macadress + "')";
    }
    //qDebug() << MAJConnexionRequete;
    DataBase::I()->StandardSQL(MAJConnexionRequete, "Rufus::MetAJourUserConnectes()");
    return usr;
}
