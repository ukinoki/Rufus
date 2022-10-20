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

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include "gbl_datas.h"
#include "database.h"
#include "dlg_message.h"
#include "utils.h"

            /*! L'UTILISATION DU TCP PAR RUFUS
            * Rufus peut fonctionner en réseau de 2 manières, avec ou sans RufusAdmin.
            * Certaines tâches requièrent l'échange d'informations entre postes:
                * modification de la salle d'attente,
                * modification de la liste des correspondants,
                * envoi de messages entre postes
                * enregistrement d'un nouveau document externe pour un patient

            * A SANS RUFUS ADMIN = UTILISATION DE TIMERS
                * Il n'y a pas d'envoi direct d'informations d'un poste à un autre
                * chaque poste scrute à intervalle régulier le contenu de la table rufus.flags pour savoir si
                    * la salle d'attente a été modifiée
                    * la liste de correspondants a été modifiée
                    * de nouveaux messages sont échangés
                * chaque poste met à jour les flags correspondants quand il modifie la salle d'attente, la liste de correspondants ou quand il envoie un message
                * quand un poste affiche le dossier d'un patient, il scrute en plus la table documents pour savoir si de nouveaux documents sont arrivés pour ce patient
                * Bien entendu, ce système consomme des ressources au niveau du serveur et ne peut donc pas être envisagé qund le nombre de postes sur le réseau est élevé.

            * B AVEC RUFUSADMIN = AVEC TCP
                * chaque poste envoie un message TCP à RufusAdmin quand
                    * il a modifié la salle d'attente
                    * il a modifié la liste de correspondants
                    * il a envoyé un message à un ou plusieurs autres postes
                    * RufusAdmin traite et rediffuse le message vers les postes concernés, toujours par TCP
                * quand RufusAdmin enregistre un nouveau document, il en informe le poste qui affiche le dossier correspondant à ce document.

            * C LES POSTES DISTANTS
                * avec ou sans RufusAdmin, les postes distants fonctionnent sans TCP
            * RufusAdmin continue à surveiller et à mettre à jour les flags correspondants pour informer/surveiller les postes distants qui fonctionnent sans TCP
            */

/*! LES MESSAGES
 * TCPMSG_StringidPoste
    * envoyé immédiatement après la connexion, composé du stringid du poste qui vient de se connnecter puis TCPMSG_StringidPoste
 * TCPMSG_DeconnexionPoste
    * envoyé immédiatement après la déconnexion d'un poste, composé du stringid du poste qui vient de se déconnnecter puis TCPMSG_DeconnexionPoste
 * TCPMSG_MAJSalAttente
    * utilisé seul, envoyé au serveurTCP pour inviter à faire une MAJ de la salle d'attente
 * TCPMSG_MAJCorrespondants
    * utilisé seul, envoyé au serveurTCP pour inviter à faire une MAJ de la liste des correspondants
 * TCPMSG_MAJDocsExternes
    * message reçu du serveurTCP, composé de idpatient puis TCPMSG_MAJDocsExternes -> incite à mettre à jour les documents externes du patient idpatient
 * TCPMSG_MsgBAL
    * gère la BAL
        * message reçu du serveur      -> composé de nombre de messages puis TCPMSG_MsgBAL -> indique que le nombre de messages vient d'être reçu
        * message envoyé au serveur    -> composé de la liste des idUser destinataires séparés par des virgules puis separateur puis nombre de messages puis TCPMSG_MsgBAL
 * TCPMSG_ListeSockets
    * message reçu du serveur, composé des dats de chaque poste connecté, séparés par {}
    * chaque data contient adresseIP, adresseMac, LoaclhostName(), idUser puis  TCPMSG_ListeSockets
 * TCPMSG_MAJPatient
    * utilisé seul, envoyé au serveurTCP pour inviter à faire une MAJ d'e la liste des correspondants d'un patient
 * TCPMSG_Separator
    * le séparateur des éléments d'un message
 */

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    static TcpSocket*   I();
    bool                TcpConnectToServer(QString ipadrserver = "");   /* Crée la connexion avec le TcpServer sur le réseau */
    void                envoieMessage(QString msg);                     /* envoi d'un message au serveur pour être redispatché vers tous les clients */

private:
    TcpSocket();
    static TcpSocket    *instance;
    QString             m_currentmsg, m_erreurmsg;
    QStringList         m_listesockets;
    quint16             m_portTCPserver;
    QByteArray          m_bufferarray;                                  // le buffer stocke les data jusqu'à ce que tout le bloc soit reçu
    qint32              m_datasize;                                     // le stockage de la taille permet de savoir si le bloc a été reçu
    void                erreurSocket();                                 /* traitement des erreurs d'émission de message sur le socket */
    void                TraiteDonneesRecues();                          /* decortiquage des messages reçus */

signals:
    void                receiveTCPmsg(QString msg);
};

#endif // TCPSOCKET_H
