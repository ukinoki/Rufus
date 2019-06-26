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

#include "cls_postesconnectes.h"

PostesConnectes::PostesConnectes(QObject * parent) : ItemsList(parent)
{
    m_postesconnectes = new QMap<QString, PosteConnecte*>();
}

PostesConnectes::~PostesConnectes()
{
    clearAll(m_postesconnectes);
    delete m_postesconnectes;
}

QMap<QString, PosteConnecte*>* PostesConnectes::postesconnectes() const
{
    return m_postesconnectes;
}

void PostesConnectes::addList(QList<PosteConnecte*> listpost)
{
    QList<PosteConnecte*>::const_iterator it;
    for( it = listpost.constBegin(); it != listpost.constEnd(); ++it )
    {
        PosteConnecte* item = const_cast<PosteConnecte*>(*it);
        add( m_postesconnectes, item->stringid(), item );
    }
}

PosteConnecte* PostesConnectes::getById(QString stringid)
{
    QMap<QString, PosteConnecte*>::const_iterator itcpt = m_postesconnectes->find(stringid) ;
    if( itcpt == m_postesconnectes->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void PostesConnectes::initListe()
{
    clearAll(m_postesconnectes);
    addList(DataBase::I()->loadPostesConnectes());
}

void PostesConnectes::SupprimeAllPostesConnectes()
{
    clearAll(m_postesconnectes);
    DataBase::I()->StandardSQL("delete from " TBL_USERSCONNECTES);
}

void PostesConnectes::SupprimePosteConnecte(PosteConnecte *post)
{
    if (post == Q_NULLPTR)
        return;
    bool canremoveverrouactes = true;
    DataBase::I()->StandardSQL("delete from " TBL_USERSCONNECTES " where " CP_IDUSER_USRCONNECT " = " + QString::number(post->id()) + " and " CP_MACADRESS_USRCONNECT " like '" + post->stringid() + "%'");
    for (QMap<QString, PosteConnecte*>::const_iterator itpost = m_postesconnectes->constBegin(); itpost != m_postesconnectes->constEnd(); ++itpost)
    {
        PosteConnecte *postit = const_cast<PosteConnecte*>(itpost.value());
        if (postit != Q_NULLPTR)
            if (postit->id() == post->id() && postit->nomposte() != post->nomposte())
            {
                canremoveverrouactes = false;
                itpost = m_postesconnectes->constEnd();
            }
    }
    if (canremoveverrouactes)
        DataBase::I()->StandardSQL("delete from " TBL_VERROUCOMPTAACTES " where PosePar = " + QString::number(post->id()));
    remove(m_postesconnectes, post);
}

PosteConnecte* PostesConnectes::CreationPosteConnecte()
{
    QString macadress =  Utils::getMACAdress();
    QString MAJConnexionRequete = "insert into " TBL_USERSCONNECTES "(" CP_HEUREDERNIERECONNECTION_USRCONNECT ", "
                                                                        CP_IDUSER_USRCONNECT ", "
                                                                        CP_IDUSERSUPERVISEUR_USRCONNECT ", "
                                                                        CP_IDUSERCOMPTABLE_USRCONNECT ", "
                                                                        CP_IDUSERPARENT_USRCONNECT ", "
                                                                        CP_NOMPOSTE_USRCONNECT ", "
                                                                        CP_MACADRESS_USRCONNECT ", "
                                                                        CP_IPADRESS_USRCONNECT ")"
                               " VALUES(NOW()," +
                               QString::number(DataBase::I()->getUserConnected()->id()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserActeSuperviseur()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserComptable()) + "," +
                               QString::number(DataBase::I()->getUserConnected()->getIdUserParent()) +",'" +
                               QHostInfo::localHostName().left(60) + "', '" +
                               macadress + " - " + DataBase::I()->getUserConnected()->getLogin() + "', '" +
                               Utils::getIpAdress() + "')";
    //qDebug() << MAJConnexionRequete;
    DataBase::I()->StandardSQL(MAJConnexionRequete, "Rufus::MetAJourUserConnectes()");
    PosteConnecte *post = new PosteConnecte();
    post->setstringid(Utils::getMACAdress());
    post->setid(DataBase::I()->getUserConnected()->id());
    post->setidsuperviseur(DataBase::I()->getUserConnected()->getIdUserActeSuperviseur());
    post->setidcomptable(DataBase::I()->getUserConnected()->getIdUserComptable());
    post->setidparent(DataBase::I()->getUserConnected()->getIdUserParent());
    post->setheurederniereconnexion(DataBase::I()->ServerDateTime());
    post->setnomposte(QHostInfo::localHostName().left(60));
    post->setmacadresslogin(macadress + " - " + DataBase::I()->getUserConnected()->getLogin());
    post->setipadress(Utils::getIpAdress());
    add( m_postesconnectes, post->stringid(), post );
    return post;
}
