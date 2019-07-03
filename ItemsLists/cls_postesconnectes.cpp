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

PosteConnecte* PostesConnectes::admin(Item::UPDATE upd)
{
    if (adminset && upd == Item::NoUpdate)
        return m_admin;
    initListe();
    m_admin = Q_NULLPTR;
    if (DataBase::I()->getMode() != DataBase::Distant)
    {
        int idAdministrateur = -1;
        QJsonObject jadmin = DataBase::I()->loadAdminData();
        if (jadmin.size() > 0)
            idAdministrateur = jadmin.value("id").toInt();
         QMapIterator<QString, PosteConnecte*> itpost(*m_postesconnectes);
        while (itpost.hasNext()) {
            itpost.next();
            PosteConnecte *post = itpost.value();
            if(post->id() == idAdministrateur && idAdministrateur > -1)
            {
                m_admin = post;
                itpost.toBack();
            }
        }
    }
    adminset = true;
    return m_admin;
}

PosteConnecte* PostesConnectes::currentpost()
{
    return getById(Utils::getMACAdress() + " - " + QString::number(DataBase::I()->getUserConnected()->id()));
}

void PostesConnectes::SupprimePosteConnecte(PosteConnecte *post)
{
    if (post == Q_NULLPTR)
        return;
    bool canremoveverrouactes = true;
    QString req = "delete from " TBL_USERSCONNECTES " where " CP_IDUSER_USRCONNECT " = " + QString::number(post->id()) + " and " CP_MACADRESS_USRCONNECT " = '" + post->macadress() + "'";
    DataBase::I()->StandardSQL(req);
    QMapIterator<QString, PosteConnecte*> itpost(*m_postesconnectes);
    while (itpost.hasNext())
    {
        PosteConnecte *postit = const_cast<PosteConnecte*>(itpost.next().value());
        if (postit != Q_NULLPTR)
            if (postit->id() == post->id() && postit->nomposte() != post->nomposte())
            {
                canremoveverrouactes = false;
                itpost.toBack();
            }
    }
    if (canremoveverrouactes)
        DataBase::I()->StandardSQL("delete from " TBL_VERROUCOMPTAACTES " where PosePar = " + QString::number(post->id()));
    remove(m_postesconnectes, post);
}

PosteConnecte* PostesConnectes::CreationPosteConnecte()
{
    QString macadressid =  Utils::getMACAdress() + " - " + QString::number(DataBase::I()->getUserConnected()->id());
    QString macadress = Utils::getMACAdress() +  (DataBase::I()->getUserConnected()->getLogin() == NOM_ADMINISTRATEURDOCS? DataBase::I()->getUserConnected()->getLogin() : "");
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
                               macadress + "', '" +
                               Utils::getIpAdress() + "')";
    //qDebug() << MAJConnexionRequete;
    DataBase::I()->StandardSQL(MAJConnexionRequete, "Rufus::MetAJourUserConnectes()");
    PosteConnecte *post = new PosteConnecte();
    post->setstringid(macadressid);
    post->setid(DataBase::I()->getUserConnected()->id());
    post->setidsuperviseur(DataBase::I()->getUserConnected()->getIdUserActeSuperviseur());
    post->setidcomptable(DataBase::I()->getUserConnected()->getIdUserComptable());
    post->setidparent(DataBase::I()->getUserConnected()->getIdUserParent());
    post->setheurederniereconnexion(DataBase::I()->ServerDateTime());
    post->setnomposte(QHostInfo::localHostName().left(60));
    post->setmacadress(macadress);
    post->setipadress(Utils::getIpAdress());
    add( m_postesconnectes, post->stringid(), post );
    return post;
}
