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

#include "tcpsocket.h"

TcpSocket* TcpSocket::instance = Q_NULLPTR;

TcpSocket* TcpSocket::I()
{
    if (instance == Q_NULLPTR)
        instance = new TcpSocket();
    return instance;
}

TcpSocket::TcpSocket()
{
    buffer.clear();
    sizedata = 0;
}

bool TcpSocket::TcpConnectToServer(QString ipadrserver)
{
    if (ipadrserver == "")
        ipadrserver    = DataBase::I()->parametres()->adresseserveurlocal();
    if (ipadrserver == "")
        return false;
    QString port        = NOM_PORT_TCPSERVEUR;
    PortTCPServer       = port.toUShort();
    /*
     * The main difference between close() and disconnectFromHost() is that the first actually closes the OS socket, while the second does not.
     * The problem is, after a socket was closed, you cannot use it to create a new connection.
     * Thus, if you want to reuse the socket, use disconnectFromHost() otherwise close()
    */
    disconnect();
    if (state() == QAbstractSocket::ConnectedState || state() == QAbstractSocket::ConnectingState)
        disconnectFromHost();
    connect(this,     &QTcpSocket::hostFound, this,   [=] { Logs::MSGSOCKET("Connexion OK"); });
    connectToHost(ipadrserver,PortTCPServer);     // On se connecte au serveur
    bool a = waitForConnected();
    if (a)
    {
        connect(this,                 &QTcpSocket::readyRead,                                              this,   &TcpSocket::TraiteDonneesRecues);
        connect(this,                 QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,   &TcpSocket::erreurSocket);
    }
    else
    {
        disconnect();
        close();
        instance = Q_NULLPTR;
    }
    return a;
}

void TcpSocket::TraiteDonneesRecues()
{
    while (bytesAvailable() > 0)
    {
        buffer.append(readAll());
        while ((sizedata == 0 && buffer.size() >= 4) || (sizedata > 0 && buffer.size() >= sizedata)) // on n'a toujours pas la teille du message ou on n'a pas le message complet
        {
            if (sizedata == 0 && buffer.size() >= 4)                // on a les 4 premiers caractères => on a la taille du message
            {
                sizedata = Utils::ArrayToInt(buffer.mid(0, 4));
                buffer.remove(0, 4);
            }
            if (sizedata > 0 && buffer.size() >= sizedata)          // le message est complet
            {
                QByteArray data = buffer.mid(0, sizedata);
                buffer.clear();                                     // on remet à 0 buffer et sizedata
                sizedata = 0;
                QString msg = QString::fromUtf8(data);
                //qDebug() << msg;
                emit tcpmessage(msg);
            }
        }
    }
}

void TcpSocket::erreurSocket()
{
    QAbstractSocket::SocketError erreur = error();
    qDebug() << erreur;
    switch(erreur)
    {
        case QAbstractSocket::RemoteHostClosedError:
            erreurmsg = tr("Le serveur TCP s'est déconnecté");
            break;
        case QAbstractSocket::HostNotFoundError:
            erreurmsg = tr("Le serveur TCP est introuvable");
            break;
        case QAbstractSocket::SocketTimeoutError:
            erreurmsg = tr("Le serveur TCP ne répond pas");
            break;
        default:
            erreurmsg = tr("ERREUR : ") + errorString();
    }

    if (erreur == QAbstractSocket::RemoteHostClosedError       /************** LE SERVER S'EST DÉCONNECTÉ ACCIDENTELLEMENT OU VOLONTAIREMENT *************************** */
     || erreur == QAbstractSocket::HostNotFoundError           /************** LE SERVER NE RÉPOND PAS ************************** */
     || erreur == QAbstractSocket::SocketTimeoutError)         /************** LE SERVER N'A PAS RÉPONDU À UN ENVOI ************************** */
    {
        qDebug() << erreurmsg + " - " + currentmsg;
        TcpConnectToServer();
    }
    else
    {
        qDebug() << erreurmsg + " - " + currentmsg + " - void Rufus::erreurSocket(QAbstractSocket::SocketError erreur)";
        return;
    }
}

void TcpSocket::envoieMessage(QString msg)
{
    currentmsg = msg;
    //qDebug() << msg << "void TcpSocket::envoieMessage(QString msg)";
    QByteArray paquet   = msg.toUtf8();
    QByteArray size     = Utils::IntToArray(paquet.size());
    if(state() == QAbstractSocket::ConnectedState)
    {
        write(size);                //envoie la taille du message
        write(paquet);              //envoie le message
        waitForBytesWritten(1000);
    }
}


