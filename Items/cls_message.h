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

#ifndef MESSAGE_H
#define MESSAGE_H

#include "cls_item.h"

class Message : public Item
{
    Q_OBJECT
public:
    explicit                Message(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void                    setData(QJsonObject data = QJsonObject{});

private: //Données du message
    //!<m_id = Id du message en base
    int m_idemetteur            = 0;                //! id du user qui a créé le message
    QString m_text              = "";               //! le texte du message
    int m_idpatient             = 0;                //! id du patient concerné par le message
    bool m_istache              = false;            //! le message concerne une tâche à effectuer
    QDate m_datelimite          = QDate();          //! le message a une date limite (pour une tâche à effectuer p.e.)
    QDateTime m_datecreation    = QDateTime();      //! la date de création du messafe
    bool m_isurgent             = false;            //! le message est urgent
    int m_idreponsea            = 0;                //! l'e'id de l'utilisateur à qui répond le message
    bool m_isasupprimer         = false;            //! le message est à supprimer
    bool m_islu                 = false;            //! le message a été lu
    bool m_isfait               = false;            //! la tache concernée par le message a été effectuée
    int m_idjointure            = 0;                //! id de la jointure du message
    int m_iddestinataire        = 0;                //! id du destinataire

public:

    void resetdatas();
    bool isnull() const                     { return m_id == 0; }

    int idemetteur() const                  { return m_idemetteur; }
    QString texte() const                   { return m_text; }
    int idpatient() const                   { return m_idpatient; }
    bool istache() const                    { return m_istache;  }
    QDate datelimite() const                { return m_datelimite; }
    QDateTime datecreation() const          { return m_datecreation; }
    bool isurgent() const                   { return m_isurgent; }
    int idreponsea() const                  { return m_idreponsea; }
    bool isasupprimer() const               { return m_isasupprimer; }
    bool islu() const                       { return m_islu; }
    bool isfait() const                     { return m_isfait; }
    int idjointure() const                  { return m_idjointure; }
    int iddestinataire() const              { return m_iddestinataire; }

    void setidemetteur(int id)              { m_idemetteur = id;
                                              m_data[CP_IDEMETTEUR_MSG] = id; }
    void settexte(QString txt)              { m_text = txt;
                                              m_data[CP_TEXT_MSG] = txt; }
    void setidpatient(int idpatient)        { m_idpatient = idpatient;
                                              m_data[CP_IDPATIENT_MSG] = idpatient; }
    void settache(bool istache)             { m_istache = istache;
                                              m_data[CP_TACHE_MSG] = istache; }
    void setdatelimite(const QDate &date)   { m_datelimite = date;
                                              m_data[CP_DATELIMITE_MSG] = date.toString("yyyy-MM-dd"); }
    void setdatecreation(const QDateTime &date) { m_datecreation = date;
                                              m_data[CP_DATECREATION_MSG] = date.toString("yyyy-MM-dd"); }
    void seturgent(bool logic)              { m_isurgent = logic;
                                              m_data[CP_URGENT_MSG] = logic; }
    void setidreponsea(int id)              { m_idreponsea = id;
                                              m_data[CP_ENREPONSEA_MSG] = id; }
    void setasupprimer(bool logic)          { m_isasupprimer = logic;
                                              m_data[CP_ASUPPRIMER_MSG] = logic; }
    void setlu(bool logic)                  { m_islu = logic;
                                              m_data[CP_LU_JOINTURESMSG] = logic; }
    void setfait(bool logic)                { m_isfait = logic;
                                              m_data[CP_FAIT_JOINTURESMSG] = logic; }
    void setidjointure(int id)              { m_idjointure = id;
                                             m_data[CP_ID_JOINTURESMSG] = id; }
    void setiddestinataire(int id)          { m_iddestinataire = id;
                                              m_data[CP_IDDESTINATAIRE_JOINTURESMSG] = id; }
};

#endif // MESSAGE_H
