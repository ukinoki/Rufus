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

#include "cls_sessions.h"

Sessions::Sessions(QObject *parent) : ItemsList(parent)
{
    map_sessions = new QMap<int, Session*>();
}

QMap<int, Session *> *Sessions::sessions() const
{
    return map_sessions;
}

/*!
 * \brief Sessions::getById -> charge un Session à partir de son id
 * \param id
 * \param details si l'Session n'est pas dans liste et si details = LoadDetails => va chercher l'Session dans la BDD sinon, renvoie Q_NULLPTR
 * \return
 * +++++ cette fonction n'ajoute pas l'Session à la map_Sessions quelquesoit son résultat
 */
Session* Sessions::getById(int id, Item::LOADDETAILS details)
{
    Session * sess = Q_NULLPTR;
    QMap<int, Session*>::const_iterator itact = map_sessions->constFind(id);
    if( itact == map_sessions->constEnd() )
    {
        if (details == Item::LoadDetails)
            sess = DataBase::I()->loadSessionById(id);
    }
    else
        sess = const_cast<Session*>(itact.value());
    return sess;
}

void Sessions::SupprimeSession(Session* act)
{
    Supprime(map_sessions, act);
}

Session* Sessions::CreationSession(QHash<QString, QVariant> sets)
{
    Session *sess = Q_NULLPTR;
    int idSession = 0;
    DataBase::I()->locktables(QStringList() << TBL_SESSIONS);
    idSession = DataBase::I()->selectMaxFromTable(CP_ID_SESSIONS, TBL_SESSIONS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idSession;
        sets[CP_ID_SESSIONS] = idSession;
        result = DataBase::I()->InsertSQLByBinds(TBL_SESSIONS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer cette session dans la base!"));
        return sess;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_SESSIONS] = idSession;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDSUPERVISEUR_SESSIONS)         data[champ] = itset.value().toString();
        else if (champ == CP_IDUSER_SESSIONS)           data[champ] = itset.value().toString();
        else if (champ == CP_IDPARENT_SESSIONS)         data[champ] = itset.value().toString();
        else if (champ == CP_IDCOMPTABLE_SESSIONS)      data[champ] = itset.value().toString();
        else if (champ == CP_DATEDEBUT_SESSIONS)        data[champ] = itset.value().toString();
        else if (champ == CP_DATEFIN_SESSIONS)          data[champ] = itset.value().toString();
    }
    m_currentsession = new Session(data);
    if (sess != Q_NULLPTR)
        map_sessions->insert(sess->id(), sess);
    return sess;
}

