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
    m_bufferarray.clear();
    m_datasize = 0;
}

bool TcpSocket::TcpConnectToServer(QString ipadrserver)
{
    if (ipadrserver == "")
        ipadrserver    = DataBase::I()->parametres()->adresseserveurlocal();
    if (ipadrserver == "")
        return false;
    QString port        = NOM_PORT_TCPSERVEUR;
    m_portTCPserver       = port.toUShort();
    /*
     * The main difference between close() and disconnectFromHost() is that the first actually closes the OS socket, while the second does not.
     * The problem is, after a socket was closed, you cannot use it to create a new connection.
     * Thus, if you want to reuse the socket, use disconnectFromHost() otherwise close()
    */
    disconnect();
    if (state() == QAbstractSocket::ConnectedState || state() == QAbstractSocket::ConnectingState)
        disconnectFromHost();
    connect(this,     &QTcpSocket::hostFound, this,   [=] { Logs::LogSktMessage("Connexion OK"); });
    connectToHost(ipadrserver,m_portTCPserver);     // On se connecte au serveur
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
        m_bufferarray.append(readAll());
        while ((m_datasize == 0 && m_bufferarray.size() >= 4) || (m_datasize > 0 && m_bufferarray.size() >= m_datasize)) // on n'a toujours pas la teille du message ou on n'a pas le message complet
        {
            if (m_datasize == 0 && m_bufferarray.size() >= 4)                // on a les 4 premiers caractères => on a la taille du message
            {
                m_datasize = Utils::ArrayToInt(m_bufferarray.mid(0, 4));
                m_bufferarray.remove(0, 4);
            }
            if (m_datasize > 0 && m_bufferarray.size() >= m_datasize)          // le message est complet
            {
                QByteArray data = m_bufferarray.mid(0, m_datasize);
                m_bufferarray.clear();                                     // on remet à 0 buffer et sizedata
                m_datasize = 0;
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
    //qDebug() << erreur;
    switch(erreur)
    {
        case QAbstractSocket::RemoteHostClosedError:
            m_erreurmsg = tr("Le serveur TCP s'est déconnecté");
            break;
        case QAbstractSocket::HostNotFoundError:
            m_erreurmsg = tr("Le serveur TCP est introuvable");
            break;
        case QAbstractSocket::SocketTimeoutError:
            m_erreurmsg = tr("Le serveur TCP ne répond pas");
            break;
        default:
            m_erreurmsg = tr("ERREUR : ") + errorString();
    }

    if (erreur == QAbstractSocket::RemoteHostClosedError       /************** LE SERVER S'EST DÉCONNECTÉ ACCIDENTELLEMENT OU VOLONTAIREMENT *************************** */
     || erreur == QAbstractSocket::HostNotFoundError           /************** LE SERVER NE RÉPOND PAS ************************** */
     || erreur == QAbstractSocket::SocketTimeoutError)         /************** LE SERVER N'A PAS RÉPONDU À UN ENVOI ************************** */
    {
        //qDebug() << m_erreurmsg + " - " + m_currentmsg;
        TcpConnectToServer();
    }
    else
    {
        //qDebug() << m_erreurmsg + " - " + m_currentmsg + " - void Rufus::erreurSocket(QAbstractSocket::SocketError erreur)";
        return;
    }
}

void TcpSocket::envoieMessage(QString msg)
{
    m_currentmsg = msg;
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


