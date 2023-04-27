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

#ifndef CLS_SESSION_H
#define CLS_SESSION_H

#include "cls_item.h"

class Session : public Item
{
    Q_OBJECT
public: //static
    static int ROLE_NON_RENSEIGNE; //-1
    static int ROLE_VIDE; //-2
    static int ROLE_INDETERMINE; //-3

private:
    int m_iduser = ROLE_INDETERMINE;
    int m_idlieu = ROLE_INDETERMINE;
    int m_idUserSuperviseur = ROLE_INDETERMINE;     //!< son id si le user est responsable de ses SESSIONS
                                                    //!< l'id du user assisté s'il est assistant
    int m_idUserParent = ROLE_INDETERMINE;          //!< son id si le user n'est pas remplaçant
                                                    //!< l'id du user remplacé s'il est remplacé
    int m_idUserComptable = ROLE_INDETERMINE;       //!< si le  user est soignant
                                                        //! s'il est responsable de ses SESSIONS =>
                                                            //!< son id s'il est libéral
                                                            //!< l'id de son employeur s'il est salarié
                                                            //!< s'il est remplaçant (Remplacant) on lui demande qui il remplace et le user comptable devient
                                                            //!< . celui qu'il remplace si celui qu'il remplace est libéral
                                                            //!< . l'employeur de celui qu'il remplace si celui qu'il remplace est salarié
                                                        //! s'il n'est pas responsable de ses SESSIONS
                                                            //! -2 = ROLE_VIDE - le comptable de l'Session sera enregistré au moment de l'enregistrement de la cotation
                                                            //! ce sera l'id du user qui enregistrera la cotation
    QDateTime m_datedebut = QDateTime();
    QDateTime m_datefin = QDateTime();

public:
    Session(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QDateTime datedebut() const             { return m_datedebut; };
    QDateTime datefin() const               { return m_datefin; };
    int iduser() const                      { return m_iduser; };
    int idsite() const                      { return m_idlieu; };
    int idsuperviseur() const               { return m_idUserSuperviseur; };
    int idParent() const                    { return m_idUserParent; };
    int idComptable() const                 { return m_idUserComptable; };
    
    void setid(int id)                      { m_id = id;
                                              m_data[CP_ID_SESSIONS] = id; }
    void setdatedebut(QDateTime date)       { m_datedebut = date;
                                              m_data[CP_DATEDEBUT_SESSIONS] = date.toString("yyyy-MM-dd hh:mm:ss"); }
    void setdatefin(QDateTime date)         { m_datefin = date;
                                              m_data[CP_DATEFIN_SESSIONS] = date.toString("yyyy-MM-dd hh:mm:ss"); }
    void setidsuperviseur(int id)           { m_iduser = id;
                                              m_data[CP_IDUSER_SESSIONS] = id; }
    void setiduser(int id)                  { m_idUserSuperviseur = id;
                                              m_data[CP_IDSUPERVISEUR_SESSIONS] = id; }
    void setidcomptable(int id)             { m_idUserComptable = id;
                                              m_data[CP_IDCOMPTABLE_SESSIONS] = id; }
    void setidlieu(int id)                  { m_idlieu = id;
                                              m_data[CP_IDLIEU_SESSIONS] = id; }
    void setidparent(int id)                { m_idUserParent = id;
                                              m_data[CP_IDPARENT_SESSIONS] = id; }
};

#endif // CLS_SESSION_H
