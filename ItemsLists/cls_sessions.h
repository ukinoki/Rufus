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

#ifndef CLS_SESSIONS_H
#define CLS_SESSIONS_H

#include "cls_session.h"
#include "cls_itemslist.h"

class Sessions: public ItemsList
{
private:
    QMap<int, Session*>         *map_sessions = Q_NULLPTR;                                      //!< une liste de sessions
    Session                     *m_currentsession = Q_NULLPTR;                                  //!> la session en cours

public:
    explicit    Sessions(QObject *parent = Q_NULLPTR);

    QMap<int, Session *> *sessions() const;

    Session*    getById(int id, Item::LOADDETAILS details = Item::LoadDetails);                 //!> charge une session à partir de son id
    Session* currentsession() { return m_currentsession; };

    //!> actions combinées sur l'item et l'enregistrement correspondant en base de données

    //!> actions sur les champs

    //!> actions sur les enregistrements
    void        SupprimeSession(Session *ses);
    Session*    CreationSession(QHash<QString, QVariant> sets);

};

#endif // CLS_SESSIONS_H
