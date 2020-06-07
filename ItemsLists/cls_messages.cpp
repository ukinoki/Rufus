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

#include "cls_messages.h"

Messages::Messages(QObject *parent) : ItemsList(parent)
{
    map_all     = new QMap<int, Message*>();
}

QMap<int, Message*>* Messages::messages() const
{
    return map_all;
}

QList<Message*> Messages::messagesrecus() const
{
    return m_listrecus;
}

QList<Message*> Messages::messagesenvoyes() const
{
    return m_listenvoyes;
}

QList<Message*> Messages::allmessages() const
{
    return m_listallmsgs;
}

Message* Messages::getById(int id)
{
    QMap<int, Message*>::const_iterator itref = map_all->constFind(id);
    Message * itm = DataBase::I()->loadMessageById(id);
    if( itref == map_all->constEnd() )
    {
        if (itm != Q_NULLPTR)
            add( map_all, itm );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<Message*>(it.value()) : Q_NULLPTR);
    }
    else
    {
        if (itm)
        {
            itref.value()->setData(itm->datas());
            delete itm;
        }
    }
    return itref.value();
}

/*!
 * \brief Messages::initListeMsgsrecusByIdUser
 * Charge l'ensemble des messages recus par un utilisateur
 * et les ajoute à la liste m_listrecus
 */
void Messages::initListeMsgsrecusByIdUser(int iduser)
{
    QList<Message*> listmessages = DataBase::I()->loadMessagesRecusByIdUser(iduser);
    for (int i=0; i<m_listrecus.size(); ++i)
        delete m_listrecus.at(i);
    m_listrecus = listmessages;
}

/*!
 * \brief Messages::initListeMsgsenvoyesByIdUser
 * Charge l'ensemble des messages envoyes par un utilisateur
 * et les ajoute à la liste m_listenvoyes
 */
void Messages::initListeMsgsenvoyesByIdUser(int iduser)
{
    QList<Message*> listmessages = DataBase::I()->loadMessagesEnvoyesByIdUser(iduser);
    for (int i=0; i<m_listenvoyes.size(); ++i)
        delete m_listenvoyes.at(i);
    m_listenvoyes = listmessages;
}

/*!
 * \brief Messages::initListeAllMsgsByIdUser
 * Charge l'ensemble des messages reçus ou envoyes par un utilisateur
 * et les ajoute à la liste m_listenvoyes
 */
void Messages::initListeAllMsgsByIdUser(int iduser)
{
    QList<Message*> listmessages = DataBase::I()->loadAllMessagesByIdUser(iduser);
    for (int i=0; i<m_listallmsgs.size(); ++i)
        delete m_listallmsgs.at(i);
    m_listallmsgs = listmessages;
}

void Messages::SupprimeMessage(Message* msg)
{
    Supprime(map_all, msg);
}

bool Messages::CreationMessage(QHash<QString, QVariant> sets,  QList<int> listdestinataires)
{
    int idmsg = 0;
    QStringList locklist;
    locklist <<  TBL_MESSAGES << TBL_MESSAGESJOINTURES << TBL_FLAGS;
    DataBase::I()->createtransaction(locklist);
    idmsg = DataBase::I()->selectMaxFromTable(CP_ID_MSG, TBL_MESSAGES, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idmsg;
        sets[CP_ID_MSG] = idmsg;
        result = DataBase::I()->InsertSQLByBinds(TBL_MESSAGES, sets);
        if (!result)
        {
            DataBase::I()->rollback();
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce message dans la base!"));
            return false;
        }
        if (!EnregistreDestinataires(idmsg, listdestinataires))
        {
            DataBase::I()->rollback();
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce message dans la base!"));
            return false;
        }
        DataBase::I()->commit();
    }
    QJsonObject  data = QJsonObject{};
    return true;
}

bool Messages::UpdateMessage(int idmsg, QHash<QString, QVariant> sets,  QList<int> listdestinataires)
{
    QStringList locklist;
    locklist <<  TBL_MESSAGES << TBL_MESSAGESJOINTURES;
    DataBase::I()->createtransaction(locklist);
    DataBase::I()->UpdateTable(TBL_MESSAGES, sets, "where " CP_ID_MSG " = " + QString::number(idmsg));
    DataBase::I()->StandardSQL("delete from " TBL_MESSAGESJOINTURES " where " CP_IDMSG_JOINTURESMSG " = " + QString::number(idmsg));
    if (!EnregistreDestinataires(idmsg, listdestinataires))
    {
        DataBase::I()->rollback();
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce message dans la base!"));
        return false;
    }
    DataBase::I()->commit();
    return true;
}

bool Messages::EnregistreDestinataires(int idmsg, QList<int> listdestinataires)
{
    QString req = "insert into " TBL_MESSAGESJOINTURES " (" CP_IDMSG_JOINTURESMSG ", " CP_IDDESTINATAIRE_JOINTURESMSG ") Values ";
    for (int i=0; i<listdestinataires.size(); i++)
    {
        req += "(" + QString::number(idmsg) + "," + QString::number(listdestinataires.at(i)) + ")";
        if (i<listdestinataires.size()-1)
            req += ",";
    }
    if (!DataBase::I()->StandardSQL(req, tr("Impossible d'enregistrer le message")))
    {
        DataBase::I()->rollback();
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce message dans la base!"));
        return false;
    }
    return true;
}
