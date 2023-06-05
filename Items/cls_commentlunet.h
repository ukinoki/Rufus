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

#ifndef COMMENTLUNET_H
#define COMMENTLUNET_H

#include "cls_item.h"

class CommentLunet : public Item
{
    Q_OBJECT
public:
    explicit                CommentLunet(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void                    setData(QJsonObject data = QJsonObject{});

private: //Données du commentaire
    //!<m_id = Id du commentaire en base
    int m_iduser                = 0;                //! id du user qui a créé le commentaire
    QString m_text              = "";               //! le texte du commentaire
    QString m_resume            = "";               //! le resumé du commentaire
    bool m_defaut               = false;            //! le commentaire est imprimé par défaut
    bool m_public               = false;            //! le commentaire est public

public:
    int iduser() const                      { return m_iduser; }
    QString texte() const                   { return m_text; }
    QString resume() const                  { return m_resume; }
    bool ispublic() const                   { return m_public; }
    bool isdefaut() const                   { return m_defaut; }

    void settexte(QString txt)              { m_text = txt;
                                              m_data[CP_TEXT_COMLUN] = txt; }
    void setresume(QString txt)             { m_resume = txt;
                                              m_data[CP_RESUME_COMLUN] = txt; }
    void setiduser(int id)                  { m_iduser = id;
                                              m_data[CP_IDUSER_COMLUN] = id; }
    void setpublic(bool logic)              { m_public = logic;
                                              m_data[CP_PUBLIC_COMLUN] = logic; }
    void setdefaut(bool logic)              { m_defaut = logic;
                                              m_data[CP_PARDEFAUT_COMLUN] = logic; }

    void resetdatas();
    bool isnull() const                     { return m_id == 0; }
    QString tooltip() const;
 };


#endif // COMMENTLUNET_H
