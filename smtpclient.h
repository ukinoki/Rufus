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

#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QSslSocket>

/*! SmtpClient envoie un mail avec une pièce jointe par un serveur d'envoi classique
    * l'envoi est bloquant : la fonction ne rend la main qu'une fois le mail remis au serveur ou l'échec constaté
    * le dialogue SMTP est une suite de commandes en texte, chacune attendant une réponse numérique du serveur
        * EHLO      -> présentation
        * STARTTLS  -> chiffrement de la liaison, sauf sur le port 465 où elle l'est dès la connexion
        * AUTH LOGIN-> identifiant puis mot de passe, encodés en base64
        * MAIL FROM / RCPT TO / DATA -> l'enveloppe puis le message
    * en cas d'échec, erreur() donne la dernière réponse du serveur, à montrer telle quelle à l'utilisateur
*/

class SmtpClient : public QObject
{
    Q_OBJECT
public:
    explicit                SmtpClient(QObject *parent = nullptr);

    /*!
     * \brief SmtpClient::envoie
     * Envoie un mail avec ses pièces jointes et renvoie true si le serveur l'a accepté.
     * \param serveur     adresse du serveur d'envoi
     * \param port        465 = liaison chiffrée dès la connexion, sinon chiffrement après connexion
     * \param login       identifiant de connexion au serveur
     * \param motdepasse  mot de passe de connexion au serveur
     * \param expediteur  adresse de l'expéditeur, doit correspondre au compte utilisé
     * \param destinataire adresse du destinataire
     * \param copiecachee adresse en copie cachée, vide si aucune
     * \param sujet       sujet du mail
     * \param corps       corps du mail, en texte simple
     * \param pieces      les pièces jointes, nom de fichier -> contenu
     */
    bool                    envoie(const QString &serveur, int port,
                                   const QString &login, const QString &motdepasse,
                                   const QString &expediteur, const QString &destinataire, const QString &copiecachee,
                                   const QString &sujet, const QString &corps,
                                   const QMap<QString, QByteArray> &pieces);
    QString                 erreur() const              { return m_erreur; }
    bool                    refusIdentifiants() const   { return m_refusidentifiants; }   /*!< le serveur a rejeté le login ou le mot de passe */

private:
    QSslSocket              m_socket;
    QString                 m_erreur = "";
    bool                    m_refusidentifiants = false;

    bool                    commande(const QString &texte, const QString &codeattendu);
    QString                 lisReponse();
    QByteArray              calcMessage(const QString &expediteur, const QString &destinataire,
                                        const QString &sujet, const QString &corps,
                                        const QMap<QString, QByteArray> &pieces);
};

#endif // SMTPCLIENT_H
