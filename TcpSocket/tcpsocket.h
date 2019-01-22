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

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include "database.h"
#include "dlg_message.h"
#include "utils.h"

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    static TcpSocket* getInstance();
    bool                TcpConnectToServer(QString ipadrserver = "");   /* Crée la connexion avec le TcpServer sur le réseau */
    void                envoieMessage(QString msg);                     /* envoi d'un message au serveur pour être redispatché vers tous les clients */

private:
    TcpSocket();
    static TcpSocket    *instance;
    DataBase            *db;
    QString             currentmsg, erreurmsg;
    QStringList         gListSockets;
    quint16             PortTCPServer;
    QByteArray          buffer;                                         // le buffer stocke les data jusqu'à ce que tout le bloc soit reçu
    qint32              sizedata;                                       // le stockage de la taille permet de savoir si le bloc a été reçu
    void                erreurSocket();                                 /* traitement des erreurs d'émission de message sur le socket */
    void                TraiteDonneesRecues();                          /* decortiquage des messages reçus */

signals:
    void                tcpmessage(QString msg);
};

#endif // TCPSOCKET_H
