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
    void                envoieMessage(QString msg);                     /* envoi d'un message au serveur pour être redispatché vers tous les clients */
    void                envoieMessageA(QList<int> listidusr);           /* envoi d'un message à une liste d'utilisateurs */
    bool                TcpConnectToServer(QString ipadrserver = "");   /* Crée la connexion avec le TcpServer sur le réseau */

private:
    TcpSocket();
    static TcpSocket    *instance;
    DataBase            *db;
    QString             currentmsg, erreurmsg;
    QStringList         gListSockets;
    quint16             PortTCPServer;
    void                erreurSocket();                                 /* traitement des erreurs d'émission de message sur le socket */
    void                TraiteDonneesRecues();                          /* decortiquage des messages reçus */

signals:
    void                tcpmessage(QString msg);
};

#endif // TCPSOCKET_H
