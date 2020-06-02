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

#ifndef MESSAGES_H
#define MESSAGES_H

#include "cls_message.h"
#include "cls_itemslist.h"

class Messages : public ItemsList
{
private:
    QMap<int, Message*> *map_all = Q_NULLPTR;       //!< la liste des messages
    QList<Message*> m_listrecus;                    //!< la liste des messages reçus
    QList<Message*> m_listenvoyes;                  //!< la liste des messages envoyes
    QList<Message*> m_listallmsgs;                  //!< la liste de tous les messages reçus ou envoyes

public:
    explicit Messages(QObject *parent = Q_NULLPTR);

    QMap<int, Message*> *messages() const;
    QList<Message *> messagesrecus() const;
    QList<Message *> messagesenvoyes() const;
    QList<Message *> allmessages() const;

    Message* getById(int id);
    void initListeMsgsrecusByIdUser(int iduser);
    void initListeMsgsenvoyesByIdUser(int iduser);
    void initListeAllMsgsByIdUser(int iduser);

    //!> actions sur les enregistrements
    void           SupprimeMessage(Message *comment);
    bool           CreationMessage(QHash<QString, QVariant> sets, QList<int> listdestinataires);
    bool           UpdateMessage(int idmsg, QHash<QString, QVariant> sets, QList<int> listdestinataires);
    bool           EnregistreDestinataires(int idmsg, QList<int> listdestinataires);
};

#endif // MESSAGES_H
