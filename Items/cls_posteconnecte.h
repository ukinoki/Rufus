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

#ifndef CLS_POSTECONNECTE_H
#define CLS_POSTECONNECTE_H

#include <QMap>
#include <QVariant>
#include <QDate>
#include "cls_item.h"

class PosteConnecte : public Item
{

    /*! la classe userconnecté gère les connexions des utilisateurs
     * un utilisateur peut -être connecté sur plusieurs posts en même temps dans la même session
     * la clé de la table correspond au couple iduser-macadress qui ne peut donc qu'être unique
     */
public:
    explicit PosteConnecte(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

private:
    //!< m_id = id de l'utilisateur en base
    //!< m_stringid = l'adresses mac du poste connecté suivi de l'id de l'utilisateur séparé par " - "

    QString m_nomposte;                 //!< nom du poste connexté
    QString m_macadress;                //!< macadress+login du poste connectéadresse IP du poste connecté
    QString m_ipadress;                 //!> l'adresse IP du poste connecté
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

public:

    QString nomposte() const                    { return m_nomposte; }
    QString macadress() const                   { return m_macadress; }
    bool isdistant() const                      { return m_accesdistant; }
    int idsuperviseur() const                   { return m_idsuperviseur; }
    int idparent() const                        { return m_idparent; }
    int idcomptable() const                     { return m_idcomptable; }
    int idlieu() const                          { return m_idlieu; }
    int idpatencours() const                    { return m_idpatencours; }
    QDateTime heurederniereconnexion() const    { return m_heurederniereconnexion; }
    QString ipadress() const                    { return m_ipadress; }

    void setnomposte(QString txt)                       { m_nomposte = txt; }
    void setmacadress(QString txt)                      { m_macadress = txt; }
    void setipadress(QString txt)                       { m_ipadress = txt; }
    void setisdistant(bool logic)                       { m_accesdistant = logic; }
    void setidsuperviseur(int id)                       { m_idsuperviseur = id; }
    void setidparent(int id)                            { m_idparent = id; }
    void setidcomptable(int id)                         { m_idcomptable = id; }
    void setidlieu(int id)                              { m_idlieu = id; }
    void setidpatencours(int id)                        { m_idpatencours = id; }
    void setheurederniereconnexion(QDateTime datetime)  { m_heurederniereconnexion = datetime; }
};

#endif // CLS_POSTECONNECTE_H
