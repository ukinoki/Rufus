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

#include "cls_interventions.h"

Interventions::Interventions(QObject *parent) : ItemsList(parent)
{
    map_interventions = new QMap<int, Intervention*>();
}

QMap<int, Intervention*>* Interventions::interventions() const
{
    return map_interventions;
}

Intervention* Interventions::getById(int id)
{
    QMap<int, Intervention*>::const_iterator itref = map_interventions->find(id);
    if( itref == map_interventions->constEnd() )
    {
        Intervention * ref = DataBase::I()->loadInterventionById(id);
        if (ref != Q_NULLPTR)
            add( map_interventions, ref );
        return ref;
    }
    return itref.value();
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
    epurelist(map_interventions, &listInterventions);
    addList(map_interventions, &listInterventions, Item::Update);
}

void Interventions::SupprimeIntervention(Intervention* intervention)
{
    Supprime(map_interventions, intervention);
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
    QVariant value;
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
        map_interventions->insert(intervention->id(), intervention);
    return intervention;
}

SessionsOperatoires::SessionsOperatoires(QObject *parent) : ItemsList(parent)
{
    map_sessions = new QMap<int, SessionOperatoire*>();
}

QMap<int, SessionOperatoire*>* SessionsOperatoires::sessions() const
{
    return map_sessions;
}

SessionOperatoire* SessionsOperatoires::getById(int id)
{
    QMap<int, SessionOperatoire*>::const_iterator itref = map_sessions->find(id);
    if( itref == map_sessions->constEnd() )
    {
        SessionOperatoire * ref = DataBase::I()->loadSessionOpById(id);
        if (ref != Q_NULLPTR)
            add( map_sessions, ref );
        return ref;
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
    epurelist(map_sessions, &listsessions);
    addList(map_sessions, &listsessions, Item::Update);
}


void SessionsOperatoires::SupprimeSessionOperatoire(SessionOperatoire *session)
{
    Supprime(map_sessions, session);
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
    QVariant value;
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
        map_sessions->insert(session->id(), session);
    return session;
}

IOLs::IOLs(QObject *parent) : ItemsList(parent)
{
    map_all     = new QMap<int, IOL*>();
}

QMap<int, IOL*>* IOLs::iols() const
{
    return map_all;
}

IOL* IOLs::getById(int id)
{
    QMap<int, IOL*>::const_iterator itref = map_all->find(id);
    if( itref == map_all->constEnd() )
    {
        IOL* ref = DataBase::I()->loadIOLById(id);
        if (ref != Q_NULLPTR)
            add( map_all, ref, Item::Update );
        return ref;
    }
    return itref.value();
}

/*!
 * \brief IOLs::initListe
 * Charge l'ensemble des IOLs d'un fabricant
 * et les ajoute à la classe IOLS
 */
void IOLs::initListeByManufacturerId(int id)
{
    QList<IOL*> listIOLs = DataBase::I()->loadIOLsByManufacturerId(id);
//    qDebug() << "listIOLS.size() =  " << listIOLs.size();
    ItemsList::clearAll(map_all);
    addList(map_all, &listIOLs);
}

/*!
 * \brief IOLs::initListe
 * Charge l'ensemble des IOLs
 * et les ajoute à la classe IOLS
 */
void IOLs::initListe()
{
    QList<IOL*> listIOLs = DataBase::I()->loadIOLs();
    ItemsList::clearAll(map_all);
    addList(map_all, &listIOLs);
}

void IOLs::SupprimeIOL(IOL *iol)
{
    Supprime(map_all, iol);
}

IOL* IOLs::CreationIOL(QHash<QString, QVariant> sets)
{
    IOL *iol = Q_NULLPTR;
    int idiol = 0;
    DataBase::I()->locktables(QStringList() << TBL_IOLS);
    idiol = DataBase::I()->selectMaxFromTable(CP_ID_IOLS, TBL_IOLS, m_ok);
    bool result = ( m_ok );
    if (result)
    {
        ++ idiol;
        sets[CP_ID_IOLS] = idiol;
        result = DataBase::I()->InsertSQLByBinds(TBL_IOLS, sets);
    }
    DataBase::I()->unlocktables();
    if (!result)
    {
        UpMessageBox::Watch(Q_NULLPTR,tr("Impossible d'enregistrer cet implant dans la base!"));
        return iol;
    }
    QJsonObject  data = QJsonObject{};
    data[CP_ID_IOLS] = idiol;
    QString champ;
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_IDMANUFACTURER_IOLS)              data[champ] = itset.value().toInt();
        else if (champ == CP_MODELNAME_IOLS)              data[champ] = itset.value().toString();
        else if (champ == CP_INACTIF_IOLS)                data[champ] = (itset.value().toString() == 1);
    }
    iol = new IOL(data);
    if (iol != Q_NULLPTR)
        map_all->insert(iol->id(), iol);
//    qDebug() << iol->id();
    return iol;
}

TypesInterventions::TypesInterventions(QObject *parent) : ItemsList(parent)
{
    map_typeinterventions = new QMap<int, TypeIntervention*>();
}

QMap<int, TypeIntervention*>* TypesInterventions::typeinterventions() const
{
    return map_typeinterventions;
}

TypeIntervention* TypesInterventions::getById(int id)
{
    QMap<int, TypeIntervention*>::const_iterator itref = map_typeinterventions->find(id);
    if( itref == map_typeinterventions->constEnd() )
    {
        TypeIntervention* ref = DataBase::I()->loadTypeInterventionById(id);
        if (ref != Q_NULLPTR)
            add( map_typeinterventions, ref );
        return ref;
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
    epurelist(map_typeinterventions, &listtypesinterventions);
    addList(map_typeinterventions, &listtypesinterventions, Item::Update);
}


void TypesInterventions::SupprimeTypeIntervention(TypeIntervention *typeintervention)
{
    Supprime(map_typeinterventions, typeintervention);
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
    QVariant value;
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champ  = itset.key();
        if (champ == CP_TYPEINTERVENTION_TYPINTERVENTION)   data[champ] = itset.value().toString();
        else if (champ == CP_CODECCAM_TYPINTERVENTION)      data[champ] = itset.value().toString();
        else if (champ == CP_DUREE_TYPINTERVENTION)         data[champ] = itset.value().toString();
    }
    typ = new TypeIntervention(data);
    if (typ != Q_NULLPTR)
        map_typeinterventions->insert(typ->id(), typ);
    return typ;
}
