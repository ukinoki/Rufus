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

#include "cls_interventions.h"

Interventions::Interventions(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, Intervention*>();
}

QMap<int, Intervention*>* Interventions::interventions() const
{
    return map_all;
}

Intervention* Interventions::getById(int id, bool reload)
{
    auto itref = map_all->constFind(id);
    if( itref == map_all->constEnd() )
    {
        Intervention * itm = DataBase::I()->loadInterventionById(id);
        if (itm)
            add( map_all, itm );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<Intervention*>(it.value()) : Q_NULLPTR);
    }
    else
    {
        if (reload)
        {
            Intervention* itm = DataBase::I()->loadInterventionById(id);
            if (itm)
            {
                itref.value()->setData(itm->datas());
                delete itm;
            }
        }
        return const_cast<Intervention*>(itref.value());
    }
}

Intervention* Interventions::getInterventionByDateIdPatient(QDate date, int idpatient)
{
    Intervention * ref = DataBase::I()->loadInterventionByDateIdPatient(date, idpatient);
    return ref;
}

/*!
 * \brief Interventions::initListeBySessionId
 * Charge l'ensemble des interventions relatives à la session
 * et les ajoute à la classe Interventions
 */
void Interventions::initListebySessionId(int id)
{
    m_idsession = id;
    QList<Intervention*> listInterventions = DataBase::I()->loadInterventionsBySessionId(m_idsession);
    epurelist(map_all, &listInterventions);
    addList(map_all, &listInterventions, Item::Update);
}

void Interventions::setcurrentintervention(Intervention *interv)
{
    m_currentintervention = interv;
}

void Interventions::SupprimeIntervention(Intervention* intervention)
{
    Supprime(map_all, intervention);
}

Intervention* Interventions::CreationIntervention(QHash<QString, QVariant> sets)
{
    Intervention *intervention = Q_NULLPTR;
    int idintervention = 0;
    DataBase::I()->locktables(QStringList() << TBL_LIGNESPRGOPERATOIRES);
    idintervention = DataBase::I()->selectMaxFromTable(CP_ID_LIGNPRGOPERATOIRE, TBL_LIGNESPRGOPERATOIRES, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idintervention;
        sets[CP_ID_LIGNPRGOPERATOIRE] = idintervention;
        result = DataBase::I()->InsertSQLByBinds(TBL_LIGNESPRGOPERATOIRES, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer cette intervention dans la base!"));
        return intervention;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_LIGNPRGOPERATOIRE] = idintervention;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDPATIENT_LIGNPRGOPERATOIRE)                data[champ] = itset.value().toInt();
        else if (champ == CP_HEURE_LIGNPRGOPERATOIRE)               data[champ] = itset.value().toString();
        else if (champ == CP_IDSESSION_LIGNPRGOPERATOIRE)           data[champ] = itset.value().toInt();
        else if (champ == CP_IDACTE_LIGNPRGOPERATOIRE)              data[champ] = itset.value().toInt();
        else if (champ == CP_TYPEANESTH_LIGNPRGOPERATOIRE)          data[champ] = itset.value().toString();
        else if (champ == CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE)  data[champ] = itset.value().toInt();
        else if (champ == CP_COTE_LIGNPRGOPERATOIRE)                data[champ] = itset.value().toString();
        else if (champ == CP_IDIOL_LIGNPRGOPERATOIRE)               data[champ] = itset.value().toInt();
        else if (champ == CP_PWRIOL_LIGNPRGOPERATOIRE)              data[champ] = itset.value().toDouble();
        else if (champ == CP_CYLIOL_LIGNPRGOPERATOIRE)              data[champ] = itset.value().toDouble();
        else if (champ == CP_OBSERV_LIGNPRGOPERATOIRE)              data[champ] = itset.value().toString();
    }
    intervention = new Intervention(data);
    if (intervention != Q_NULLPTR)
        map_all->insert(intervention->id(), intervention);
    return intervention;
}

SessionsOperatoires::SessionsOperatoires(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, SessionOperatoire*>();
}

QMap<int, SessionOperatoire*>* SessionsOperatoires::sessions() const
{
    return map_all;
}

SessionOperatoire* SessionsOperatoires::getById(int id, bool reload)
{
    QMap<int, SessionOperatoire*>::const_iterator itref = map_all->constFind(id);
    if( itref == map_all->constEnd() )
    {
        SessionOperatoire * itm = DataBase::I()->loadSessionOpById(id);
        if (itm != Q_NULLPTR)
            add( map_all, itm );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<SessionOperatoire*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        SessionOperatoire* itm = DataBase::I()->loadSessionOpById(id);
        if (itm)
        {
            itref.value()->setData(itm->datas());
            delete itm;
        }
    }
    return itref.value();
}

/*!
 * \brief SessionsOperatoires::initListeByUserId
 * Charge l'ensemble des documments accessibles à l'utilisateur en cours
 * et les ajoute à la classe SessionsOperatoires
 */
void SessionsOperatoires::initListebyUserId(int id)
{
    m_iduser = id;
    QList<SessionOperatoire*> listsessions = DataBase::I()->loadSessionsOpByUserId(m_iduser);
    epurelist(map_all, &listsessions);
    addList(map_all, &listsessions, Item::Update);
}

void SessionsOperatoires::setcurrentsession(SessionOperatoire *sess)
{
    m_currentsession = sess;
}

void SessionsOperatoires::SupprimeSessionOperatoire(SessionOperatoire *session)
{
    Supprime(map_all, session);
}

SessionOperatoire* SessionsOperatoires::CreationSessionOperatoire(QHash<QString, QVariant> sets)
{
    SessionOperatoire *session = Q_NULLPTR;
    int idsession = 0;
    DataBase::I()->locktables(QStringList() << TBL_SESSIONSOPERATOIRES);
    idsession = DataBase::I()->selectMaxFromTable(CP_ID_SESSIONOPERATOIRE, TBL_SESSIONSOPERATOIRES, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idsession;
        sets[CP_ID_SESSIONOPERATOIRE] = idsession;
        result = DataBase::I()->InsertSQLByBinds(TBL_SESSIONSOPERATOIRES, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer cette session opératoire dans la base!"));
        return session;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_SESSIONOPERATOIRE] = idsession;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDUSER_SESSIONOPERATOIRE)       data[champ] = itset.value().toInt();
        else if (champ == CP_DATE_SESSIONOPERATOIRE)    data[champ] = itset.value().toString();
        else if (champ == CP_IDLIEU_SESSIONOPERATOIRE)  data[champ] = itset.value().toInt();
        else if (champ == CP_IDAIDE_SESSIONOPERATOIRE)  data[champ] = itset.value().toInt();
    }
    session = new SessionOperatoire(data);
    if (session != Q_NULLPTR)
        map_all->insert(session->id(), session);
    return session;
}

TypesInterventions::TypesInterventions(QObject *parent) : ItemsList(parent)
{
    map_all = new QMap<int, TypeIntervention*>();
}

QMap<int, TypeIntervention*>* TypesInterventions::typeinterventions() const
{
    return map_all;
}

TypeIntervention* TypesInterventions::getById(int id, bool reload)
{
    QMap<int, TypeIntervention*>::const_iterator itref = map_all->constFind(id);
    if( itref == map_all->constEnd() )
    {
        TypeIntervention* itm = DataBase::I()->loadTypeInterventionById(id);
        if (itm != Q_NULLPTR)
            add( map_all, itm );
        auto it = map_all->constFind(id);
        return (it != map_all->cend()? const_cast<TypeIntervention*>(it.value()) : Q_NULLPTR);
    }
    else if (reload)
    {
        TypeIntervention* itm = DataBase::I()->loadTypeInterventionById(id);
        if (itm)
        {
            itref.value()->setData(itm->datas());
            delete itm;
        }
    }
    return itref.value();
}

/*!
 * \brief TypeInterventions::initListe
 * Charge l'ensemble des TypeInterventions
 * et les ajoute à la classe TypeInterventions
 */
void TypesInterventions::initListe()
{
    QList<TypeIntervention*> listtypesinterventions = DataBase::I()->loadTypeInterventions();
    epurelist(map_all, &listtypesinterventions);
    addList(map_all, &listtypesinterventions, Item::Update);
}


void TypesInterventions::SupprimeTypeIntervention(TypeIntervention *typeintervention)
{
    Supprime(map_all, typeintervention);
}

TypeIntervention* TypesInterventions::CreationTypeIntervention(QHash<QString, QVariant> sets)
{
    TypeIntervention *typ = Q_NULLPTR;
    int idtypeintervention = 0;
    DataBase::I()->locktables(QStringList() << TBL_TYPESINTERVENTIONS);
    idtypeintervention = DataBase::I()->selectMaxFromTable(CP_ID_TYPINTERVENTION, TBL_TYPESINTERVENTIONS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idtypeintervention;
        sets[CP_ID_TYPINTERVENTION] = idtypeintervention;
        result = DataBase::I()->InsertSQLByBinds(TBL_TYPESINTERVENTIONS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer ce type d'intervention dans la base!"));
        return typ;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_TYPINTERVENTION] = idtypeintervention;
    QString champ;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_TYPEINTERVENTION_TYPINTERVENTION)   data[champ] = itset.value().toString();
        else if (champ == CP_CODECCAM_TYPINTERVENTION)      data[champ] = itset.value().toString();
        else if (champ == CP_DUREE_TYPINTERVENTION)         data[champ] = itset.value().toString();
    }
    typ = new TypeIntervention(data);
    if (typ != Q_NULLPTR)
        map_all->insert(typ->id(), typ);
    return typ;
}
