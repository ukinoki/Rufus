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

#ifndef CLS_USERCONNECTE_H
#define CLS_USERCONNECTE_H

#include <QMap>
#include <QVariant>
#include <QDate>
#include "cls_item.h"

class UserConnecte : public Item
{

    /*! la classe userconnecté gère les connexions des utilisateurs
     * un utilisateur peut -être connecté sur plusieurs posts en même temps dans la même session
     * la clé de la table correspond au couple iduser-macadress qui ne peut donc qu'être unique
     */
public:
    explicit UserConnecte(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

private:
    //!< m_id = Id de l'utilisateur en base
    //!< m_stringid = l'adresses mac du poste connecté

    QString m_nomposte;                 //!< nom du poste connexté
    QString m_macadress_login;          //!< macadress+login du poste connecté
    bool m_accesdistant;                //!> le poste connecte est en accès distant
    int m_idsuperviseur;                //!> l'id du superviseur
    int m_idparent;                     //!> l'id du parent
    int m_idcomptable;                  //!> l'id du comptable
    int m_idlieu;                       //!> l'id du site de connexion
    int m_idpatencours;                 //!> l'id du patient en train d'être examiné par le user

    /*! Quand le tcp n'est pas utilisé, les déconnexions sont surveillées par un système de timer
     * le couple user-macadress actualise l'heure de dernière connexion toutes les 10 secondes
     * un poste désigné par la fonction Procedures::setPosteImportDocs(bool a) surveille les users connectes toutes les mintutes
     * si un couple user-macadress n'a pas remis à jour sa connection depuis plus d'une minute, il est supprimé de la liste des users connectés */

    QDateTime   m_heurederniereconnexion;   //!> la date et l'heure de la connexion

    /*! Quand le tcp n'est pas utilisé, les changements de la salle d'attente sont surveillées par un système de timer
     * Quand un poste modifie la salle d'attente, il incrémente \param m_idnewmodifsaldat sur tous les enregistrements de la table
     * chaque poste surveille la valeur du  \param m_idnewmodifsaldat et la compare à la valeur de \param m_idlastmodifsaldat toutes les 10 secondes
     * si elles sont differentes, il recharge la salle d'attente */

    int m_idnewmodifsaldat;             //!> le dernier id de modif de la salle d'attente enregistré par UN user
    int m_idlastmodifsaldat;            //!> le dernier id de modif de la salle d'attente enregistré par LE user

public:

    QString nomposte() const                    { return m_nomposte; }
    QString macadresslogin() const              { return m_macadress_login; }
    bool isdistant() const                      { return m_accesdistant; }
    int idsuperviseur() const                   { return m_idsuperviseur; }
    int idparent() const                        { return m_idparent; }
    int idcomptable() const                     { return m_idcomptable; }
    int idlieu() const                          { return m_idlieu; }
    int idpatencours() const                    { return m_idpatencours; }
    QDateTime heurederniereconnexion() const    { return m_heurederniereconnexion; }
    int idnewmodifsaldat() const                { return m_idnewmodifsaldat; }
    int idlastmodifsaldat() const               { return m_idlastmodifsaldat; }

    void setnomposte(QString txt)                       { m_nomposte = txt; }
    void setmacadresslogin(QString txt)                 { m_macadress_login = txt; }
    void setisdistant(bool logic)                       { m_accesdistant = logic; }
    void setidsuperviseur(int id)                       { m_idsuperviseur = id; }
    void setidparent(int id)                            { m_idparent = id; }
    void setidcomptable(int id)                         { m_idcomptable = id; }
    void setidlieu(int id)                              { m_idlieu = id; }
    void setidpatencours(int id)                        { m_idpatencours = id; }
    void setheurederniereconnexion(QDateTime datetime)  { m_heurederniereconnexion = datetime; }
    void setidnewmodifsaldat(int id)                    { m_idnewmodifsaldat = id; }
    void setidlastmodifsaldat(int id)                   { m_idlastmodifsaldat = id; }
};

#endif // CLS_USERCONNECTE_H
