#include "tcpsocket.h"

TcpSocket* TcpSocket::instance = Q_NULLPTR;

TcpSocket* TcpSocket::getInstance()
{
    if( !instance )
        instance = new TcpSocket();
    return instance;
}

TcpSocket::TcpSocket()
{
    db = DataBase::getInstance();
    buffer.clear();
    sizedata = 0;
}

bool TcpSocket::TcpConnectToServer(QString ipadrserver)
{
    if (ipadrserver == "")
    {
        bool ok = true;
        QList<QList<QVariant>> listadress = db->StandardSelectSQL("select AdresseTCPServeur from " NOM_TABLE_PARAMSYSTEME, ok);
        ipadrserver    = listadress.at(0).at(0).toString();
    }
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
    connectToHost(ipadrserver,PortTCPServer);//});      // On se connecte au serveur
    if (waitForConnected(5000))
    {
        connect(this,                 &QTcpSocket::readyRead,                                              this,   &TcpSocket::TraiteDonneesRecues);
        connect(this,                 QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,   &TcpSocket::erreurSocket);
        // envoi iduser
        envoieMessage(QString::number(db->getUserConnected()->id()) + TCPMSG_idUser);
        // envoi adresse IP, adresse MAC, nom d'hôte
        envoieMessage(Utils::getIpAdress() + TCPMSG_Separator + Utils::getMACAdress() + TCPMSG_Separator + QHostInfo::localHostName() + TCPMSG_DataSocket);
        return true;
    }
    else
    {
        dlg_message(QStringList() << "<b>" + tr("Le serveur enregistré dans la base ne répond pas.") + "</b><br/>"+ tr("Fonctionnement sans Tcpsocket"), 5000, false);
        return false;
    }
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
    QByteArray paquet   = currentmsg.toUtf8();
    QByteArray size     = Utils::IntToArray(paquet.size());
    if(state() == QAbstractSocket::ConnectedState)
    {
        write(size);                //envoie la taille du message
        write(paquet);              //envoie le message
        waitForBytesWritten(5000);
    }
}

void TcpSocket::envoieMessageA(QList<int> listidusr)
{
    QString listid;
    for (int i=0; i<listidusr.size(); i++)
    {
        listid += QString::number(listidusr.at(i));
        if (listidusr.at(i) < (listidusr.size()-1))
            listid += ",";
    }
    QString msg = listid + TCPMSG_MsgBAL;
    currentmsg = tr("courrier");
    envoieMessage(msg);
}


