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
    // Logs::LogSktMessage("PosteConnecte* PostesConnectes::getByStringId(QString stringid) ----------------------> " + stringid);
    // qDebug() << "PosteConnecte* PostesConnectes::getByStringId(QString stringid)" << stringid;
    PosteConnecte *post = Q_NULLPTR;
    if (stringid.split(" - ").size() < 2)
    {
        // qDebug() << "stringid invalide";
        return post;
    }
    QMap<QString, PosteConnecte*>::const_iterator itcpt = map_postesconnectes->constFind(stringid) ;
    if( itcpt == map_postesconnectes->constEnd() )
    {
        QJsonObject posteData{};
        int iduser = stringid.split(" - ").at(1).toInt();
        QString macadress = stringid.split(" - ").at(0);
        posteData = DataBase::I()->loadPosteConnecteData(iduser, macadress);
        if (posteData.size() != 0)
        {
            post = new PosteConnecte(posteData);
            add(map_postesconnectes, post);
        }
    }
    else
        post = itcpt.value();
    return post;
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
    //!* renvoie le poste utilisé par RufusAdmin après avoir vérifié que RufusAdmin est toujours actif - renvoie qnullptr dans le cas contraire
    if (adminset && upd == Item::NoUpdate)
        return m_admin;
    initListe();
    m_admin = Q_NULLPTR;
    if (DataBase::I()->ModeAccesDataBase() != Utils::Distant)
        foreach (PosteConnecte *post, *map_postesconnectes)
            if(post->isadmin() && post->dateheurederniereconnexion().secsTo(DataBase::I()->ServerDateTime()) < 120)
            {
                m_admin = post;
                break;
            }
    adminset = true;
    return m_admin;
}

PosteConnecte* PostesConnectes::currentpost()
{
    return getByStringId(Utils::MACAdress() + " - " + QString::number(DataBase::I()->idUserConnected()));
}

void PostesConnectes::MAJlistePostesConnectes()
{
    QList<QString> listpostes = DataBase::I()->loadStringIdPostesConnectes();
    for (auto it = map_postesconnectes->constBegin(); it != map_postesconnectes->constEnd();)
    {
        if (!listpostes.contains(it.key()))
            delete map_postesconnectes->take(it.key());
        else
            ++it;
    }
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
        if (m_admin != Q_NULLPTR)
            m_admin = Q_NULLPTR;
    }
    remove(map_postesconnectes, post);
}

PosteConnecte* PostesConnectes::CreationPosteConnecte(User* usr, int idsite)
{
    if (Utils::IPAdress() == "" || Utils::MACAdress() == "")
        return Q_NULLPTR;
    QString macadressid =  Utils::MACAdress() + " - " + QString::number(usr->id());
    QString macadress = Utils::MACAdress() +  (usr->login() == NOM_ADMINISTRATEUR? " - " + usr->login() : "");
    QString MAJConnexionRequete = "insert into " TBL_USERSCONNECTES " (" CP_HEUREDERNIERECONNECTION_USRCONNECT ", "
                                                                        CP_IDUSER_USRCONNECT ", "
                                                                        CP_IDUSERSUPERVISEUR_USRCONNECT ", "
                                                                        CP_IDUSERCOMPTABLE_USRCONNECT ", "
                                                                        CP_IDUSERPARENT_USRCONNECT ", "
                                                                        CP_NOMPOSTE_USRCONNECT ", "
                                                                        CP_MACADRESS_USRCONNECT ", "
                                                                        CP_IPADRESS_USRCONNECT ", "
                                                                        CP_IDLIEU_USRCONNECT ", "
                                                                        CP_DISTANT_USRCONNECT ")"
                               " VALUES (NOW()," +
                               QString::number(usr->id()) + "," +
                               QString::number(usr->idsuperviseur()) + "," +
                               QString::number(usr->idcomptable()) + "," +
                               QString::number(usr->idparent()) +",'" +
                               QHostInfo::localHostName().left(60) + "', '" +
                               macadress + "', '" +
                               Utils::IPAdress() + "', " +
                               QString::number(idsite) + ", " +
                               (DataBase::I()->ModeAccesDataBase() == Utils::Distant? "1" : "null") + ")";
    //qDebug() << MAJConnexionRequete;
    DataBase::I()->locktable(TBL_USERSCONNECTES);
    DataBase::I()->StandardSQL(MAJConnexionRequete, "Rufus::MetAJourUserConnectes()");
    DataBase::I()->unlocktables();
    PosteConnecte *post = new PosteConnecte();
    post->setstringid(macadressid);
    post->setid(usr->id());
    post->setidsuperviseur(usr->idsuperviseur());
    post->setidcomptable(usr->idcomptable());
    post->setidparent(usr->idparent());
    post->setdateheurederniereconnexion(DataBase::I()->ServerDateTime());
    post->setnomposte(QHostInfo::localHostName().left(60));
    post->setmacadress(macadress);
    post->setipadress(Utils::IPAdress());
    post->setisdistant(DataBase::I()->ModeAccesDataBase() == Utils::Distant);
    post->setidlieu(idsite);
    add( map_postesconnectes, post, Item::Update );
    return post;
}
