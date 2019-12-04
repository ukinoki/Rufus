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
    map_postesconnectes = new QMap<QString, PosteConnecte*>();
}

PostesConnectes::~PostesConnectes()
{
    clearAll(map_postesconnectes);
    delete map_postesconnectes;
}

QMap<QString, PosteConnecte*>* PostesConnectes::postesconnectes() const
{
    return map_postesconnectes;
}

PosteConnecte* PostesConnectes::getByStringId(QString stringid)
{
    QMap<QString, PosteConnecte*>::const_iterator itcpt = map_postesconnectes->find(stringid) ;
    if( itcpt == map_postesconnectes->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}

void PostesConnectes::initListe()
{
    QList<PosteConnecte*> listpostes = DataBase::I()->loadPostesConnectes();
    epurelist(map_postesconnectes, &listpostes);
    addList(map_postesconnectes, &listpostes);
}

void PostesConnectes::SupprimeAllPostesConnectes()
{
    clearAll(map_postesconnectes);
    DataBase::I()->StandardSQL("delete from " TBL_USERSCONNECTES);
}

PosteConnecte* PostesConnectes::admin(Item::UPDATE upd)
{
    if (adminset && upd == Item::NoUpdate)
        return m_admin;
    initListe();
    m_admin = Q_NULLPTR;
    if (DataBase::I()->getMode() != Utils::Distant)
    {
        int idAdministrateur = -1;
        QJsonObject jadmin = DataBase::I()->loadAdminData();
        if (jadmin.size() > 0)
            idAdministrateur = jadmin[CP_ID_USR].toInt();
        foreach (PosteConnecte *post, *map_postesconnectes)
            if(post->id() == idAdministrateur && idAdministrateur > -1 && post->dateheurederniereconnexion().secsTo(DataBase::I()->ServerDateTime()) < 120)
            {
                m_admin = post;
                break;
            }
    }
    adminset = true;
    return m_admin;
}

PosteConnecte* PostesConnectes::currentpost()
{
    return getByStringId(Utils::MACAdress() + " - " + QString::number(DataBase::I()->idUserConnected()));
}

void PostesConnectes::SupprimePosteConnecte(PosteConnecte *post)
{
    if (post == Q_NULLPTR)
        return;
    bool canremoveverrouactes = true;
    QString req = "delete from " TBL_USERSCONNECTES " where " CP_IDUSER_USRCONNECT " = " + QString::number(post->id()) + " and " CP_MACADRESS_USRCONNECT " = '" + post->macadress() + "'";
    DataBase::I()->StandardSQL(req);
    foreach (PosteConnecte *postit, *map_postesconnectes)
        if (postit->id() == post->id() && postit->nomposte() != post->nomposte())
        {
            canremoveverrouactes = false;
            break;
        }
    if (canremoveverrouactes)
        DataBase::I()->StandardSQL("delete from " TBL_VERROUCOMPTAACTES " where PosePar = " + QString::number(post->id()));
    if (post->isadmin()) {
        adminset = true;
        if (m_admin != Q_NULLPTR) {
            delete m_admin;
            m_admin = Q_NULLPTR;
        }
    }
    remove(map_postesconnectes, post);
}

PosteConnecte* PostesConnectes::CreationPosteConnecte(int idsite)
{
    if (Utils::IPAdress() == "" || Utils::MACAdress() == "")
        return Q_NULLPTR;
    QString macadressid =  Utils::MACAdress() + " - " + QString::number(DataBase::I()->userConnected()->id());
    QString macadress = Utils::MACAdress() +  (DataBase::I()->userConnected()->login() == NOM_ADMINISTRATEURDOCS? " - " + DataBase::I()->userConnected()->login() : "");
    QString MAJConnexionRequete = "insert into " TBL_USERSCONNECTES "(" CP_HEUREDERNIERECONNECTION_USRCONNECT ", "
                                                                        CP_IDUSER_USRCONNECT ", "
                                                                        CP_IDUSERSUPERVISEUR_USRCONNECT ", "
                                                                        CP_IDUSERCOMPTABLE_USRCONNECT ", "
                                                                        CP_IDUSERPARENT_USRCONNECT ", "
                                                                        CP_NOMPOSTE_USRCONNECT ", "
                                                                        CP_MACADRESS_USRCONNECT ", "
                                                                        CP_IPADRESS_USRCONNECT ", "
                                                                        CP_IDLIEU_USRCONNECT ", "
                                                                        CP_DISTANT_USRCONNECT ")"
                               " VALUES(NOW()," +
                               QString::number(DataBase::I()->userConnected()->id()) + "," +
                               QString::number(DataBase::I()->userConnected()->idsuperviseur()) + "," +
                               QString::number(DataBase::I()->userConnected()->idcomptable()) + "," +
                               QString::number(DataBase::I()->userConnected()->idparent()) +",'" +
                               QHostInfo::localHostName().left(60) + "', '" +
                               macadress + "', '" +
                               Utils::IPAdress() + "', " +
                               QString::number(idsite) + ", " +
                               (DataBase::I()->getMode() == Utils::Distant? "1" : "null") + ")";
    //qDebug() << MAJConnexionRequete;
    DataBase::I()->StandardSQL(MAJConnexionRequete, "Rufus::MetAJourUserConnectes()");
    PosteConnecte *post = new PosteConnecte();
    post->setstringid(macadressid);
    post->setid(DataBase::I()->userConnected()->id());
    post->setidsuperviseur(DataBase::I()->userConnected()->idsuperviseur());
    post->setidcomptable(DataBase::I()->userConnected()->idcomptable());
    post->setidparent(DataBase::I()->userConnected()->idparent());
    post->setdateheurederniereconnexion(DataBase::I()->ServerDateTime());
    post->setnomposte(QHostInfo::localHostName().left(60));
    post->setmacadress(macadress);
    post->setipadress(Utils::IPAdress());
    post->setisdistant(DataBase::I()->getMode() == Utils::Distant);
    post->setidlieu(idsite);
    add( map_postesconnectes, post, Item::Update );
    return post;
}
