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

#include "smtpclient.h"
#include <QMimeDatabase>

#define DELAI_SMTP 20000        /*!< au-delà, le serveur ne répondra plus */

SmtpClient::SmtpClient(QObject *parent) : QObject(parent)
{
}

QString SmtpClient::lisReponse()
{
    if (!m_socket.waitForReadyRead(DELAI_SMTP))
        return "";
    QString rep = QString::fromUtf8(m_socket.readAll());
    while (m_socket.bytesAvailable() > 0 || m_socket.waitForReadyRead(200))
        rep += QString::fromUtf8(m_socket.readAll());
    return rep;
}

/*!
 * \brief SmtpClient::commande
 * Envoie une commande et vérifie que la réponse du serveur commence par le code attendu.
 * \param texte        la commande, sans le retour à la ligne
 * \param codeattendu  le code de réponse d'un succès (250, 235, 334...)
 */
bool SmtpClient::commande(const QString &texte, const QString &codeattendu)
{
    if (texte != "")
    {
        m_socket.write((texte + "\r\n").toUtf8());
        if (!m_socket.waitForBytesWritten(DELAI_SMTP))
        {
            m_erreur = tr("le serveur ne répond plus");
            return false;
        }
    }
    const QString rep = lisReponse();
    qDebug() << "SMTP attendu" << codeattendu << "recu" << rep.trimmed();
    if (rep.startsWith(codeattendu))
        return true;
    m_erreur = (rep == ""? tr("le serveur ne répond plus") : rep.trimmed());
    m_refusidentifiants = rep.startsWith("535") || rep.startsWith("534");
    return false;
}

/*!
 * \brief SmtpClient::calcMessage
 * Assemble le message au format MIME : le texte puis les pièces jointes encodées en base64.
 */
QByteArray SmtpClient::calcMessage(const QString &expediteur, const QString &destinataire,
                                   const QString &sujet, const QString &corps,
                                   const QMap<QString, QByteArray> &pieces)
{
    const QString limite = "----RufusMIME000";      /*!< sépare les parties du message */
    QString msg;
    msg += "From: <" + expediteur + ">\r\n";
    msg += "To: <" + destinataire + ">\r\n";
    msg += "Subject: =?UTF-8?B?" + sujet.toUtf8().toBase64() + "?=\r\n";   /*!< le sujet n'accepte pas les accents en clair */
    msg += "MIME-Version: 1.0\r\n";
    msg += "Content-Type: multipart/mixed; boundary=\"" + limite + "\"\r\n\r\n";

    msg += "--" + limite + "\r\n";
    msg += "Content-Type: text/plain; charset=UTF-8\r\n";
    msg += "Content-Transfer-Encoding: base64\r\n\r\n";
    msg += corps.toUtf8().toBase64() + "\r\n\r\n";

    for (auto it = pieces.cbegin(); it != pieces.cend(); ++it)
    {
        msg += "--" + limite + "\r\n";
        const QString typemime = QMimeDatabase().mimeTypeForFile(it.key(), QMimeDatabase::MatchExtension).name();
        msg += "Content-Type: " + typemime + "; name=\"" + it.key() + "\"\r\n";
        msg += "Content-Transfer-Encoding: base64\r\n";
        msg += "Content-Disposition: attachment; filename=\"" + it.key() + "\"\r\n\r\n";
        const QByteArray b64 = it.value().toBase64();
        for (int i = 0; i < b64.size(); i += 76)    /*!< certains serveurs refusent les lignes trop longues */
            msg += QString::fromLatin1(b64.mid(i, 76)) + "\r\n";
        msg += "\r\n";
    }
    msg += "--" + limite + "--\r\n";
    return msg.toUtf8();
}

bool SmtpClient::envoie(const QString &serveur, int port,
                        const QString &login, const QString &motdepasse,
                        const QString &expediteur, const QString &destinataire, const QString &copiecachee,
                        const QString &sujet, const QString &corps,
                        const QMap<QString, QByteArray> &pieces)
{
    m_erreur = "";
    m_refusidentifiants = false;
    qDebug() << "SMTP serveur" << serveur << "port" << port << "login" << login << "expediteur" << expediteur;

    if (port == 465)                                /*!< liaison chiffrée dès la connexion */
    {
        m_socket.connectToHostEncrypted(serveur, quint16(port));
        if (!m_socket.waitForEncrypted(DELAI_SMTP))
        {
            m_erreur = tr("connexion impossible à ") + serveur + " : " + m_socket.errorString();
            return false;
        }
    }
    else
    {
        m_socket.connectToHost(serveur, quint16(port));
        if (!m_socket.waitForConnected(DELAI_SMTP))
        {
            m_erreur = tr("connexion impossible à ") + serveur + " : " + m_socket.errorString();
            return false;
        }
    }

    bool ok = commande("", "220")                   /*!< bannière d'accueil, sans commande */
              && commande("EHLO rufus", "250");
    if (ok && port != 465)                          /*!< chiffrement demandé après la connexion */
    {
        ok = commande("STARTTLS", "220");
        if (ok)
        {
            m_socket.startClientEncryption();
            if (!m_socket.waitForEncrypted(DELAI_SMTP))
            {
                m_erreur = tr("chiffrement refusé par ") + serveur;
                ok = false;
            }
            else
                ok = commande("EHLO rufus", "250"); /*!< à refaire une fois la liaison chiffrée */
        }
    }
    if (ok)
        ok = commande("AUTH LOGIN", "334")
             && commande(login.toUtf8().toBase64(), "334")
             && commande(motdepasse.toUtf8().toBase64(), "235");
    if (ok)
    {
        ok = commande("MAIL FROM:<" + expediteur + ">", "250")
             && commande("RCPT TO:<" + destinataire + ">", "250");
        if (ok && copiecachee != "")
            ok = commande("RCPT TO:<" + copiecachee + ">", "250");
        if (ok && commande("DATA", "354"))
        {
            m_socket.write(calcMessage(expediteur, destinataire, sujet, corps, pieces));
            m_socket.waitForBytesWritten(DELAI_SMTP);
            ok = commande("\r\n.", "250");          /*!< un point seul sur sa ligne termine le message */
        }
        else
            ok = false;
    }
    commande("QUIT", "221");
    m_socket.disconnectFromHost();
    return ok;
}
