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

#include "database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

DataBase* DataBase::instance = Q_NULLPTR;

DataBase* DataBase::I()
{
    if( !instance )
    {
        instance = new DataBase();
    }
    return instance;
}

DataBase::DataBase() {}

void DataBase::init(QSettings const &setting, Utils::ModeAcces mode)
{
    m_mode = mode;
    if( m_mode == Utils::Poste )
        m_server = "localhost";
    else
        m_server = setting.value(getBase() + "/Serveur").toString();

    m_port = setting.value(getBase() + "/Port").toInt();

    m_useSSL = (m_mode == Utils::Distant);
}

void DataBase::initFromFirstConnexion(QString mode, QString Server, int Port, bool SSL)
{
    if (mode == Utils::getBaseFromMode(Utils::Poste))
        m_mode = Utils::Poste;
    else if (mode == Utils::getBaseFromMode(Utils::ReseauLocal))
        m_mode = Utils::ReseauLocal;
    else if (mode == Utils::getBaseFromMode(Utils::Distant))
        m_mode = Utils::Distant;

    m_server = Server;
    m_port = Port;
    m_useSSL = SSL;
}

Utils::ModeAcces DataBase::getMode() const
{
    return m_mode;
}
QString DataBase::getBase() const
{
    return Utils::getBaseFromMode( m_mode );
}
QString DataBase::getServer() const
{
    return m_server;
}
QSqlDatabase DataBase::getDataBase() const
{
    return m_db;
}
void DataBase::getInformations()
{
    UpMessageBox::Watch(Q_NULLPTR,
        tr("Connexion à la base de données!"),
        tr("Vos paramètres de connexion") +
           "\n" + tr("Serveur     ") + "\n ->\t" + m_db.hostName() +
           "\n" + tr("databaseName") + "\n ->\t" + m_db.databaseName() +
           "\n" + tr("Login       ") + "\n ->\t" + m_db.userName() +
           "\n" + tr("password    ") + "\n ->\t" + m_db.password() +
           "\n" + tr("port        ") + "\n ->\t" + QString::number(m_db.port()));
}

User* DataBase::getUserConnected() const
{
    return m_userConnected;
}

void DataBase::setUserConnected(User *usr)
{
    m_userConnected = usr;
}

bool DataBase::erreurRequete(QSqlError erreur, QString requete, QString ErrorMessage)
{
    if (erreur.type() != QSqlError::NoError)
    {
        Logs::ERROR(ErrorMessage, tr("\nErreur\n") + erreur.text() +  tr("\nrequete = ") + requete);
        return true;
    }
    return false;
}

QString DataBase::connectToDataBase(QString basename, QString login, QString password)
{

    m_db = QSqlDatabase::addDatabase("QMYSQL",basename);
    m_db.setHostName( m_server );
    m_db.setPort( m_port );

    QString connectOptions = (m_useSSL?
                              "SSL_KEY=/etc/mysql/client-key.pem;"
                              "SSL_CERT=/etc/mysql/client-cert.pem;"
                              "SSL_CA=/etc/mysql/ca-cert.pem;"
                              "MYSQL_OPT_RECONNECT=1"
                                 :
                              "MYSQL_OPT_RECONNECT=1");
    m_db.setConnectOptions(connectOptions);

    m_db.setUserName(login + (m_useSSL ? "SSL" : ""));
    m_db.setPassword(password);

    if( m_db.open() )
        return QString();

    QSqlDatabase::removeDatabase(basename);
    return m_db.lastError().text();
}

QDateTime DataBase::ServerDateTime()
{
    bool ok;
    QVariantList now = getFirstRecordFromStandardSelectSQL("select now()",ok);
    if (!ok || now.size() == 0)
        return QDateTime::currentDateTime();
    else if (!now.at(0).toDateTime().isValid())
        return QDateTime::currentDateTime();
    else
        return now.at(0).toDateTime();
}

bool DataBase::createtransaction(QStringList ListTables, QString ModeBlocage)
{
    bool a = true;
    QString req = "LOCK TABLES " + ListTables.at(0) + " " + ModeBlocage;
    for (int i = 1; i < ListTables.size(); i++)
        req += "," + ListTables.at(i) + " " + ModeBlocage;
    a = StandardSQL(req);
    if (a)
        StandardSQL("SET AUTOCOMMIT = 0;"
                    "START TRANSACTION;");
    return a;
}

void DataBase::commit()
{
    StandardSQL("COMMIT;"
                "SET AUTOCOMMIT = 1;"
                "UNLOCK TABLES;");
}

void DataBase::rollback()
{
    StandardSQL("ROLLBACK;"
                "SET AUTOCOMMIT = 1;"
                "UNLOCK TABLES;");

}

bool DataBase::locktables(QStringList ListTables, QString ModeBlocage)
{
    unlocktables();
    QString req = "LOCK TABLES " + ListTables.at(0) + " " + ModeBlocage;
    for (int i = 1; i < ListTables.size(); i++)
        req += "," + ListTables.at(i) + " " + ModeBlocage;
    return StandardSQL(req);
}

bool DataBase::locktable(QString Table, QString ModeBlocage)
{
    return locktables(QStringList() << Table, ModeBlocage);
}

void DataBase::unlocktables()
{
    StandardSQL("UNLOCK TABLES;");
}

int DataBase::selectMaxFromTable(QString nomchamp, QString nomtable, bool &ok, QString errormsg)
{
    QString req = "select max(" + nomchamp + ") from " + nomtable;
    QVariantList data = getFirstRecordFromStandardSelectSQL(req, ok, errormsg);
    if(!ok || data.size()==0)
        return 0;
    return data.at(0).toInt();
}

bool DataBase::SupprRecordFromTable(int id, QString nomChamp, QString nomtable, QString errormsg)
{
    QString req = "delete from " + nomtable + " where " + nomChamp + " = " + QString::number(id);
    return StandardSQL(req, errormsg);
}

QList<QVariantList> DataBase::SelectRecordsFromTable(QStringList listselectChamp,
                                                        QString nomtable,
                                                        bool &OK,
                                                        QString where,
                                                        QString orderby,
                                                        bool distinct,
                                                        QString errormsg)
{
    QList<QVariantList> listreponses;
    QString Distinct = (distinct? "distinct " : "");
    QString selectchamp;
    for (int i=0; i<listselectChamp.size(); ++i)
        selectchamp += listselectChamp.at(i) + ",";
    selectchamp = selectchamp.left(selectchamp.size()-1);
    QString req = "select " + Distinct + selectchamp + " from " + nomtable;
    if (where != "")
        req += " " + where;
    if (orderby != "")
        req += " " + orderby;
    return StandardSelectSQL(req, OK, errormsg);
}

bool DataBase::UpdateTable(QString nomtable,
                           QHash<QString, QString> sets,
                           QString where,
                           QString errormsg)
{
    QString req = "update " + nomtable + " set";
    QHashIterator<QString, QString> itset(sets);
    while (itset.hasNext())
    {
        itset.next();
        req += " " + itset.key() + " = " + (itset.value().toLower()=="null"? "null," : "'" + Utils::correctquoteSQL(itset.value()) + "',");
    }
    req = req.left(req.size()-1); //retire la virgule de la fin
    req += " " + where;
    return StandardSQL(req, errormsg);
}

bool DataBase::InsertIntoTable(QString nomtable,
                               QHash<QString, QString> sets,
                               QString errormsg)
{
    QString req = "insert into " + nomtable + " (";
    QString champs;
    QString valeurs;
    QHashIterator<QString, QString> itset(sets);
    while (itset.hasNext())
    {
        itset.next();
        champs  += itset.key() + ",";
        valeurs += (itset.value().toLower()=="null"? "null," : "'" + Utils::correctquoteSQL(itset.value()) + "',");
    }
    champs = champs.left(champs.size()-1) + ") values (";
    valeurs = valeurs.left(valeurs.size()-1) + ")";
    req += champs + valeurs;
    return StandardSQL(req, errormsg);
}

bool DataBase::InsertSQLByBinds(QString nomtable,
                                QHash<QString, QVariant> sets,
                                QString errormsg)
{
    QSqlQuery query = QSqlQuery(m_db);
    QString champs, champs2;
    QString valeurs;
    QHashIterator<QString, QVariant> itset(sets);
    while (itset.hasNext())
    {
        itset.next();
        champs  += itset.key() + ",";
        champs2  += ":" + itset.key() + ",";
    }
    champs = champs.left(champs.size()-1);
    champs2 = champs2.left(champs2.size()-1);
    QString prepare = "insert into " + nomtable + " (" + champs + ") values (" + champs2 + ")";
    query.prepare(prepare);
    itset.toFront();
    while (itset.hasNext())
    {
        itset.next();
        query.bindValue(":" + itset.key(), itset.value());
        //qDebug() << "query.bindValue("":" + itset.key() + "," + itset.value().toString() + ")";
    }
    query.exec();
    bool a = true;
    if (query.lastError().type() != QSqlError::NoError)
    {
        Logs::ERROR(errormsg, tr("\nErreur\n") + query.lastError().text());
        a =  false;
    }
    query.finish();
    return a;
}

bool DataBase::StandardSQL(QString req , QString errormsg)
{
    QSqlQuery query(req, getDataBase());
    bool a = !erreurRequete(query.lastError(), req, errormsg);
    query.finish();
    return a;
}

QList<QVariantList> DataBase::StandardSelectSQL(QString req , bool &OK, QString errormsg)
{
    /*
    exemple:
        bool ok = true;
        QList<QVariantList> recordslist = db->StandardSelectSQL("Select idImpression from " TBL_IMPRESSIONS " where idpat = " + QString::number(gidPatient), ok);
        if (!ok)                                // erreur;
        if (recordslist.size()==0)                     // réponse vide
     */
    QList<QVariantList> listreponses = QList<QVariantList>();
    QSqlQuery query(req, getDataBase());
    QSqlRecord rec = query.record();
    if( erreurRequete(query.lastError(), req, errormsg))
    {
        OK = false;
        query.finish();
        return listreponses;
    }
    OK = true;
    if( !query.first())
    {
        query.finish();
        return listreponses;
    }
    do
    {
        QVariantList record;
        for (int i=0; i<rec.count(); ++i)
            record << query.value(i);
        listreponses << record;
    } while (query.next());
    query.finish();
    return listreponses;
}

QVariantList DataBase::getFirstRecordFromStandardSelectSQL(QString req , bool &OK, QString errormsg)
{
    /*
     exemple:
     bool ok = true;
     QVariantList recorddata = db->getFirstRecordFromStandardSelectSQL("Select idImpression from " TBL_IMPRESSIONS " where idpat = " + QString::number(gidPatient), ok);
     if (!ok)                                // erreur;
     if (recorddata.size()==0)                 // réponse vide
    */
    QList<QVariantList> listreponses = StandardSelectSQL(req , OK, errormsg);
    if(listreponses.size()>0)
        return listreponses.at(0);
    else
        return QVariantList();
}

void DataBase::VideDatabases()
{
    dlg_message(tr("Suppression de l'ancienne base Rufus en cours"));
    StandardSQL ("drop database if exists " DB_COMPTA );
    StandardSQL ("drop database if exists " DB_OPHTA );
    StandardSQL ("drop database if exists " DB_CONSULTS );
    StandardSQL ("drop database if exists " DB_IMAGES );
}


/*
 * Parametres système
*/
void DataBase::initParametres()
{
    if (m_parametres == Q_NULLPTR)
        m_parametres = new ParametresSysteme();
    QJsonObject paramData{};

    QString req = "select " CP_MDPADMIN_PARAMSYSTEME ", " CP_NUMCENTRE_PARAMSYSTEME ", " CP_IDLIEUPARDEFAUT_PARAMSYSTEME ", " CP_DOCSCOMPRIMES_PARAMSYSTEME ", " CP_VERSIONBASE_PARAMSYSTEME ", "
                  CP_SANSCOMPTA_PARAMSYSTEME ", " CP_ADRESSELOCALSERVEUR_PARAMSYSTEME ", " CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME ", " CP_DIRIMAGERIE_PARAMSYSTEME ", "
                  CP_LUNDIBKUP_PARAMSYSTEME ", " CP_MARDIBKUP_PARAMSYSTEME ", " CP_MERCREDIBKUP_PARAMSYSTEME ", " CP_JEUDIBKUP_PARAMSYSTEME ", " CP_VENDREDIBKUP_PARAMSYSTEME ", "
                  CP_SAMEDIBKUP_PARAMSYSTEME ", " CP_DIMANCHEBKUP_PARAMSYSTEME ", " CP_HEUREBKUP_PARAMSYSTEME ", " CP_DIRBKUP_PARAMSYSTEME
                  " from " TBL_PARAMSYSTEME;
    QVariantList paramdata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les paramètres du système"));
    if(!ok || paramdata.size() == 0)
    {
        delete m_parametres;
        m_parametres = Q_NULLPTR;
        return ;
    }
    paramData[CP_MDPADMIN_PARAMSYSTEME]               = paramdata.at(0).toString();
    paramData[CP_NUMCENTRE_PARAMSYSTEME]              = paramdata.at(1).toInt();
    paramData[CP_IDLIEUPARDEFAUT_PARAMSYSTEME]        = paramdata.at(2).toInt();
    paramData[CP_DOCSCOMPRIMES_PARAMSYSTEME]          = (paramdata.at(3).toInt() == 1);
    paramData[CP_VERSIONBASE_PARAMSYSTEME]            = paramdata.at(4).toInt();
    paramData[CP_SANSCOMPTA_PARAMSYSTEME]             = (paramdata.at(5).toInt() == 1);
    paramData[CP_ADRESSELOCALSERVEUR_PARAMSYSTEME]    = paramdata.at(6).toString();
    paramData[CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME]  = paramdata.at(7).toString();
    paramData[CP_DIRIMAGERIE_PARAMSYSTEME]            = paramdata.at(8).toString();
    paramData[CP_LUNDIBKUP_PARAMSYSTEME]              = (paramdata.at(9).toInt() == 1);
    paramData[CP_MARDIBKUP_PARAMSYSTEME]              = (paramdata.at(10).toInt() == 1);
    paramData[CP_MERCREDIBKUP_PARAMSYSTEME]           = (paramdata.at(11).toInt() == 1);
    paramData[CP_JEUDIBKUP_PARAMSYSTEME]              = (paramdata.at(12).toInt() == 1);
    paramData[CP_VENDREDIBKUP_PARAMSYSTEME]           = (paramdata.at(13).toInt() == 1);
    paramData[CP_SAMEDIBKUP_PARAMSYSTEME]             = (paramdata.at(14).toInt() == 1);
    paramData[CP_DIMANCHEBKUP_PARAMSYSTEME]           = (paramdata.at(15).toInt() == 1);
    paramData[CP_HEUREBKUP_PARAMSYSTEME]              = paramdata.at(16).toTime().toString("HH:mm:ss");
    paramData[CP_DIRBKUP_PARAMSYSTEME]                = paramdata.at(17).toString();
    m_parametres->setData(paramData);
    return;
}

ParametresSysteme* DataBase::parametres()
{
    if (m_parametres == Q_NULLPTR)
        initParametres();
    return m_parametres;
}
void DataBase::setmdpadmin(QString mdp)
{
    QString value = (mdp != ""? "'" + Utils::correctquoteSQL(mdp) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_MDPADMIN_PARAMSYSTEME " = " + value);
    m_parametres->setmdpadmin(mdp);
}
void DataBase::setnumcentre(int id)
{
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_NUMCENTRE_PARAMSYSTEME " = " + QString::number(id));
    m_parametres->setnumcentre(id);
}
void DataBase::setidlieupardefaut(int id)
{
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_IDLIEUPARDEFAUT_PARAMSYSTEME " = " + QString::number(id));
    m_parametres->setidlieupardefaut(id);
}
void DataBase::setdocscomprimes(bool one)
{
    QString a = (one? "'1'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_DOCSCOMPRIMES_PARAMSYSTEME " = " + a);
    m_parametres->setdocscomprimes(one);
}
void DataBase::setversionbase(int version)
{
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_VERSIONBASE_PARAMSYSTEME " = " + QString::number(version));
    m_parametres->setversionbase(version);
}
void DataBase::setsanscompta(bool one)
{
    QString a = (!one? "'1'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_SANSCOMPTA_PARAMSYSTEME " = " + a);
    m_parametres->setsanscompta(one);
}
void DataBase::setadresseserveurlocal(QString  adress)
{
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_ADRESSELOCALSERVEUR_PARAMSYSTEME " = " + value);
    m_parametres->setadresseserveurlocal(adress);
}
void DataBase::setadresseserveurdistant(QString adress)
{
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME " = " + value);
    m_parametres->setadresseserveurdistant(adress);
}
void DataBase::setdirimagerie(QString adress)
{
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_DIRIMAGERIE_PARAMSYSTEME " = " + value);
    m_parametres->setdirimagerie(adress);
}
void DataBase::setdaysbkup(Utils::Days days)
{
    QString val;
    QString req = "update " TBL_PARAMSYSTEME " set ";
    val = (days.testFlag(Utils::Lundi)?     "'1'" : "null");
    req += CP_LUNDIBKUP_PARAMSYSTEME " = " + val + ", ";
    val = (days.testFlag(Utils::Mardi)?     "'1'" : "null");
    req += CP_MARDIBKUP_PARAMSYSTEME " = " + val + ", ";
    val = (days.testFlag(Utils::Mercredi)?  "'1'" : "null");
    req += CP_MERCREDIBKUP_PARAMSYSTEME " = " + val + ", ";
    val = (days.testFlag(Utils::Jeudi)?     "'1'" : "null");
    req += CP_JEUDIBKUP_PARAMSYSTEME " = " + val + ", ";
    val = (days.testFlag(Utils::Vendredi)?  "'1'" : "null");
    req += CP_VENDREDIBKUP_PARAMSYSTEME " = " + val + ", ";
    val = (days.testFlag(Utils::Samedi)?    "'1'" : "null");
    req += CP_SAMEDIBKUP_PARAMSYSTEME " = " + val + ", ";
    val = (days.testFlag(Utils::Dimanche)?  "'1'" : "null");
    req += CP_DIMANCHEBKUP_PARAMSYSTEME " = " + val;
    StandardSQL(req);
    m_parametres->setdaysbkup(days);
}
void DataBase::setheurebkup(QTime time)
{
    QString value = (time != QTime()? "'" + time.toString("HH:mm:ss") + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_HEUREBKUP_PARAMSYSTEME " = " + value);
    m_parametres->setheurebkup(time);
}
void DataBase::setdirbkup(QString adress)
{
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_DIRBKUP_PARAMSYSTEME " = " + value);
    m_parametres->setdirbkup(adress);
}

/*
 * Users
*/
QJsonObject DataBase::login(QString login, QString password)
{
    QJsonObject jrep{};
    int userOffset = 3;

    //TODO : SQL USER : récupérer tout le reste
    QString req = "SELECT u.idUser, u.UserNom, u.UserPrenom, "
                    "uc.NomPosteconnecte "
                  " FROM " TBL_UTILISATEURS " u "
                  " LEFT JOIN " TBL_USERSCONNECTES " uc on uc.idUSer = u.idUser "
                  " WHERE UserLogin = '" + login + "' "
                  " AND UserMDP = '" + password + "' ";
    QVariantList usrdata = getFirstRecordFromStandardSelectSQL(req, ok);
    if(!ok)
    {
        jrep["code"] = -3;
        jrep["request"] = req;
        return jrep;
    }

    if(usrdata.size()==0)
    {
        jrep["code"] = -1;
        return jrep;
    }

    if( !usrdata.at(userOffset).isNull() && usrdata.at(userOffset).toString() != QHostInfo::localHostName().left(60) )
    {
        jrep["code"] = -6;
        jrep["poste"] = usrdata.at(userOffset).toString();
        // cette erreur n'est pas exploitée parce que dans certaines structures, un même utilisateur peut travailler sur plusieurs postes
        //return jrep;
    }

    jrep["code"] = 0;

    QJsonObject userData{};
    userData["id"] = usrdata.at(0).toInt();
    userData["nom"] = usrdata.at(1).toString();
    userData["prenom"] = usrdata.at(2).toString();

    m_userConnected = new User(login, password, userData);
    m_userConnected->setData(loadUserData(usrdata.at(0).toInt()));
    return jrep;
}

QJsonObject DataBase::loadUserData(int idUser)
{
    QJsonObject userData{};

    QString req = "select UserDroits, UserAGA, UserLogin, UserFonction, UserTitre, "                    // 0,1,2,3,4
            " UserNom, UserPrenom, UserMail, UserNumPS, UserSpecialite,"                                // 5,6,7,8,9
            " UserNoSpecialite, UserNumCO, idCompteParDefaut, UserEnregHonoraires, UserMDP,"            // 10,11,12,13,14
            " UserPortable, UserPoste, UserWeb, UserMemo, UserDesactive,"                               // 15,16,17,18,19
            " UserPoliceEcran, UserPoliceAttribut, UserSecteur, Soignant, ResponsableActes,"            // 20,21,22,23,24
            " UserCCAM, UserEmployeur, DateDerniereConnexion, idCompteEncaissHonoraires, Medecin,"      // 25,26,27,28,29
            " OPTAM, cpt.nomcompteabrege"                                                               // 30,31
            " from " TBL_UTILISATEURS " usr "
            " left outer join " TBL_COMPTES " cpt on usr.idcompteencaisshonoraires = cpt.idCompte"
            " where usr.idUser = " + QString::number(idUser);

            //+ "  and userdesactive is null";
            // SL cette ligne est retirée parce qu'elle bloque l'affichage des utilisateurs désactivés dans dlg_gestionsusers

    QVariantList usrdata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données de l'utilisateur"));
    if (!ok)
        return userData;

    if(usrdata.size()==0)
        return userData;

    userData["isAllLoaded"]                 = true;
    userData["id"]                          = idUser;
    userData["droits"]                      = usrdata.at(0).isNull() ? "" : usrdata.at(0).toString();
    userData["AGA"]                         = (usrdata.at(1).toInt() == 1);
    userData["login"]                       = usrdata.at(2).isNull() ? "" : usrdata.at(2).toString();
    userData["fonction"]                    = usrdata.at(3).isNull() ? "" : usrdata.at(3).toString();
    userData["titre"]                       = usrdata.at(4).isNull() ? "" : usrdata.at(4).toString();
    userData["nom"]                         = usrdata.at(5).isNull() ? "" : usrdata.at(5).toString();
    userData["prenom"]                      = usrdata.at(6).isNull() ? "" : usrdata.at(6).toString();
    userData["mail"]                        = usrdata.at(7).isNull() ? "" : usrdata.at(7).toString();
    userData["numPS"]                       = usrdata.at(8).toLongLong();
    userData["specialite"]                  = usrdata.at(9).isNull() ? "" : usrdata.at(9).toString();
    userData["noSpecialite"]                = usrdata.at(10).toInt();
    userData["numCO"]                       = usrdata.at(11).isNull() ? "" : usrdata.at(11).toString();
    userData["idCompteParDefaut"]           = (usrdata.at(12).isNull()? -1 : usrdata.at(12).toInt());
    userData["enregHonoraires"]             = usrdata.at(13).toInt();
    userData["password"]                    = usrdata.at(14).toString();
    userData["portable"]                    = usrdata.at(15).isNull() ? "" : usrdata.at(15).toString();
    userData["poste"]                       = usrdata.at(16).toInt();
    userData["web"]                         = usrdata.at(17).isNull() ? "" : usrdata.at(17).toString();
    userData["memo"]                        = usrdata.at(18).isNull() ? "" : usrdata.at(18).toString();
    userData["desactive"]                   = (usrdata.at(19).toInt() == 1);
    userData["policeEcran"]                 = usrdata.at(20).isNull() ? "" : usrdata.at(20).toString();
    userData["policeAttribut"]              = usrdata.at(21).isNull() ? "" : usrdata.at(21).toString();
    userData["secteur"]                     = usrdata.at(22).toInt();
    userData["OPTAM"]                       = (usrdata.at(30).toInt() == 1);
    userData["soignant"]                    = usrdata.at(23).toInt();
    userData["responsableActes"]            = usrdata.at(24).toInt();
    userData["cotation"]                    = (usrdata.at(25).toInt() == 1);
    userData["employeur"]                   = usrdata.at(26).toInt();
    userData["dateDerniereConnexion"]       = QDateTime(usrdata.at(27).toDate(), usrdata.at(27).toTime()).toMSecsSinceEpoch();
    userData["medecin"]                     = usrdata.at(29).toInt();
    userData["idCompteEncaissHonoraires"]   = (usrdata.at(28).isNull()? -1 : usrdata.at(28).toInt());
    if( userData["idCompteEncaissHonoraires"].toInt() > -1 )
        userData["nomCompteEncaissHonoraires"] = usrdata.at(31).toString();
    return userData;
}

QJsonObject DataBase::loadAdminData()
{
    QJsonObject userData{};
    QString req = "select iduser from " TBL_UTILISATEURS " where userlogin = '" NOM_ADMINISTRATEURDOCS "'";
    QVariantList usrid = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données de l'administrateur"));
    if (!ok)
        return userData;
    if(usrid.size()==0)
        return userData;
    return loadUserData(usrid.at(0).toInt());
}

QList<User*> DataBase::loadUsers()
{
    QList<User*> users;
    QString req = "select usr.iduser, usr.userlogin, usr.soignant, usr.responsableactes, "          //0,1,2,3
                    " usr.UserEnregHonoraires, usr.idCompteEncaissHonoraires, usr.UserCCAM, "       //4,5,6
                    " usr.UserEmployeur, cpt.nomcompteabrege "                                      //7,8
                  " from " TBL_UTILISATEURS " usr "
                  " left outer join " TBL_COMPTES " cpt on usr.idcompteencaisshonoraires = cpt.idCompte "
                  " where userdesactive is null";
    QList<QVariantList> usrlist = StandardSelectSQL(req, ok);
    if( !ok || usrlist.size()==0 )
        return users;
    for (int i=0; i<usrlist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"]                         = usrlist.at(i).at(0).toInt();
        jData["login"]                      = usrlist.at(i).at(1).toString();
        jData["soignant"]                   = usrlist.at(i).at(2).toInt();
        jData["responsableActes"]           = usrlist.at(i).at(3).toInt();
        jData["enregHonoraires"]            = usrlist.at(i).at(4).toInt();
        jData["idCompteEncaissHonoraires"]  = usrlist.at(i).at(5).toInt();
        jData["cotation"]                   = (usrlist.at(i).at(6).toInt() == 1);
        jData["employeur"]                  = usrlist.at(i).at(7).toInt();
        jData["nomCompteAbrege"]            = usrlist.at(i).at(8).toString();
        jData["isAllLoaded"]                = false;
        User *usr = new User(jData);
        users << usr;
    }
    return users;
}

/*
 * Users connectes
*/
QJsonObject DataBase::loadPosteConnecteData(int iduser, QString macadress)
{
    QJsonObject posteData{};
    QString req = "select NomPosteConnecte, AccesDistant, UserSuperviseur,"
                  " UserComptable, UserParent, idLieu, HeureDerniereConnexion, idPat, IPAdress"
                  " from " TBL_USERSCONNECTES
                  " where idUser = " + QString::number(iduser) +
                  " and " CP_MACADRESS_USRCONNECT " = " + macadress;
    QList<QVariantList> postlist = StandardSelectSQL(req, ok);
    if( !ok || postlist.size()==0 )
        return posteData;
    for (int i=0; i<postlist.size(); ++i)
    {
        posteData[CP_IDUSER_USRCONNECT]                     = iduser;
        posteData[CP_NOMPOSTE_USRCONNECT]                   = postlist.at(i).at(0).toString();
        posteData[CP_MACADRESS_USRCONNECT]                  = macadress;
        posteData[CP_DISTANT_USRCONNECT]                    = postlist.at(i).at(1).toInt() == 1;
        posteData[CP_IDUSERSUPERVISEUR_USRCONNECT]          = postlist.at(i).at(2).toInt();
        posteData[CP_IDUSERCOMPTABLE_USRCONNECT]            = postlist.at(i).at(3).toInt();
        posteData[CP_IDUSERPARENT_USRCONNECT]               = postlist.at(i).at(4).toInt();
        posteData[CP_IDLIEU_USRCONNECT]                     = postlist.at(i).at(5).toInt();
        posteData[CP_HEUREDERNIERECONNECTION_USRCONNECT]    = QDateTime(postlist.at(i).at(6).toDate(), postlist.at(i).at(6).toTime()).toMSecsSinceEpoch();
        posteData[CP_IDPATENCOURS_USRCONNECT]               = postlist.at(i).at(7).toInt();
        posteData["stringid"]                               = macadress.split(" ").at(0) + " - " + postlist.at(i).at(0).toString();
        posteData[CP_IPADRESS_USRCONNECT]                   = postlist.at(i).at(8).toString();
    }
    return posteData;
}

QList<PosteConnecte*> DataBase::loadPostesConnectes()
{
    QList<PosteConnecte*> postes;
    QString req = "select idUser, NomPosteConnecte, MACAdressePosteConnecte, AccesDistant, UserSuperviseur,"
                  " UserComptable, UserParent, idLieu, HeureDerniereConnexion, idPat, IPAdress"
                  " from " TBL_USERSCONNECTES ;
    QList<QVariantList> postlist = StandardSelectSQL(req, ok);
    if( !ok || postlist.size()==0 )
        return postes;
    for (int i=0; i<postlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_IDUSER_USRCONNECT]                     = postlist.at(i).at(0).toInt();
        jData[CP_NOMPOSTE_USRCONNECT]                   = postlist.at(i).at(1).toString();
        jData[CP_MACADRESS_USRCONNECT]                  = postlist.at(i).at(2).toString();
        jData[CP_DISTANT_USRCONNECT]                    = postlist.at(i).at(3).toInt() == 1;
        jData[CP_IDUSERSUPERVISEUR_USRCONNECT]          = postlist.at(i).at(4).toInt();
        jData[CP_IDUSERCOMPTABLE_USRCONNECT]            = postlist.at(i).at(5).toInt();
        jData[CP_IDUSERPARENT_USRCONNECT]               = postlist.at(i).at(6).toInt();
        jData[CP_IDLIEU_USRCONNECT]                     = postlist.at(i).at(7).toInt();
        jData[CP_HEUREDERNIERECONNECTION_USRCONNECT]    = QDateTime(postlist.at(i).at(8).toDate(), postlist.at(i).at(8).toTime()).toMSecsSinceEpoch();
        jData[CP_IDPATENCOURS_USRCONNECT]               = postlist.at(i).at(9).toInt();
        jData["stringid"]                               = postlist.at(i).at(2).toString().split(" ").at(0) + " - " + postlist.at(i).at(0).toString();
        jData[CP_IPADRESS_USRCONNECT]                   = postlist.at(i).at(10).toString();
        PosteConnecte *post = new PosteConnecte(jData);
        postes << post;
    }
    return postes;
}

/*
 * Correspondants
*/
QList<Correspondant*> DataBase::loadCorrespondants()                             //! tous les correspondants
{
    QList<Correspondant*> correspondants;
    QString req =   "SELECT idCor, CorNom, CorPrenom, CorSexe, cormedecin, corspecialite, nomspecialite as metier FROM " TBL_CORRESPONDANTS ", " TBL_SPECIALITES
                    " where cormedecin = 1 and corspecialite = idspecialite"
                    " union "
                    "SELECT idCor, CorNom, CorPrenom, CorSexe, cormedecin, -1 as corspecialite, corautreprofession as metier from "  TBL_CORRESPONDANTS
                    " where cormedecin <> 1 or cormedecin is null"
                    " order by cornom, corprenom";

    QList<QVariantList> corlist = StandardSelectSQL(req,ok);
    if(!ok || corlist.size()==0)
        return correspondants;
    for (int i=0; i<corlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_CORRESP]             = corlist.at(i).at(0).toInt();
        jData[CP_NOM_CORRESP]            = corlist.at(i).at(1).toString();
        jData[CP_PRENOM_CORRESP]         = corlist.at(i).at(2).toString();
        jData[CP_SEXE_CORRESP]           = corlist.at(i).at(3).toString();
        jData[CP_ISMEDECIN]              = (corlist.at(i).at(4).toInt()==1);
        jData[CP_ISGENERALISTE]          = (corlist.at(i).at(5).toInt() > 0);
        jData[CP_METIER]                 = corlist.at(i).at(6).toString();
        jData[CP_ISALLLOADED]            = false;
        Correspondant *cor = new Correspondant(jData);
        correspondants << cor;
    }
    return correspondants;
}

QList<Correspondant*> DataBase::loadCorrespondantsALL()                             //! tous les correspondants avec plus de renseignements
{
    QList<Correspondant*> correspondants;
    QString req = "SELECT idCor, CorNom, CorPrenom, nomspecialite as metier, CorAdresse1,"
                        " CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone,"
                        " CorSexe, cormedecin, corspecialite"
                        " FROM " TBL_CORRESPONDANTS ", " TBL_SPECIALITES
                        " where cormedecin = 1 and corspecialite = idspecialite"
                  " union "
                  "SELECT idCor, CorNom, CorPrenom, corautreprofession as metier, CorAdresse1,"
                        " CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone,"
                        " CorSexe, cormedecin, -1 as corspecialite"
                        " FROM " TBL_CORRESPONDANTS
                        " where cormedecin <> 1 or cormedecin is null"
                  " order by metier, cornom, corprenom";
    QList<QVariantList> corlist = StandardSelectSQL(req,ok);
    if(!ok || corlist.size()==0)
        return correspondants;
    for (int i=0; i<corlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_CORRESP]             = corlist.at(i).at(0).toInt();
        jData[CP_NOM_CORRESP]            = corlist.at(i).at(1).toString();
        jData[CP_PRENOM_CORRESP]         = corlist.at(i).at(2).toString();
        jData[CP_METIER]                 = corlist.at(i).at(3).toString();
        jData[CP_ADRESSE1_CORRESP]       = corlist.at(i).at(4).toString();
        jData[CP_ADRESSE2_CORRESP]       = corlist.at(i).at(5).toString();
        jData[CP_ADRESSE3_CORRESP]       = corlist.at(i).at(6).toString();
        jData[CP_CODEPOSTAL_CORRESP]     = corlist.at(i).at(7).toString();
        jData[CP_VILLE_CORRESP]          = corlist.at(i).at(8).toString();
        jData[CP_TELEPHONE_CORRESP]      = corlist.at(i).at(9).toString();
        jData[CP_SEXE_CORRESP]           = corlist.at(i).at(10).toString();
        jData[CP_ISMEDECIN]              = (corlist.at(i).at(11).toInt() == 1);
        jData[CP_ISGENERALISTE]          = (corlist.at(i).at(12).toInt() == 0);
        jData[CP_SPECIALITE_CORRESP]     = corlist.at(i).at(12).toInt();
        jData[CP_ISALLLOADED]            = false;
        Correspondant *cor = new Correspondant(jData);
        correspondants << cor;
    }
    return correspondants;
}

QJsonObject DataBase::loadCorrespondantData(int idcor)                             //! toutes les données d'un correspondant
{
    QJsonObject jData{};
    QString req = "SELECT CorNom, CorPrenom, nomspecialite as metier, CorAdresse1, CorAdresse2,"
                        " CorAdresse3, CorCodepostal, CorVille, CorTelephone, CorSexe,"
                        " cormedecin, CorPortable, CorFax, CorMail, CorSpecialite"
                        " FROM " TBL_CORRESPONDANTS ", " TBL_SPECIALITES
                        " where cormedecin = 1 and corspecialite = idspecialite"
                        " and idcor = " + QString::number(idcor) +
                  " union "
                  "SELECT CorNom, CorPrenom, CorAutreProfession as metier, CorAdresse1, CorAdresse2,"
                        " CorAdresse3, CorCodepostal, CorVille, CorTelephone, CorSexe,"
                        " cormedecin, CorPortable, CorFax, CorMail, -1 as CorSpecialite"
                        " FROM " TBL_CORRESPONDANTS
                        " where cormedecin <> 1 or cormedecin is null"
                        " and idcor = " + QString::number(idcor);
    QVariantList cordata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données de l'utilisateur"));
    if (!ok)
        return jData;

    if(cordata.size()==0)
        return jData;

    for (int i=0; i<cordata.size(); ++i)
    {
        jData[CP_ID_CORRESP]         = idcor;
        jData[CP_NOM_CORRESP]        = cordata.at(0).toString();
        jData[CP_PRENOM_CORRESP]     = cordata.at(1).toString();
        jData[CP_METIER]             = cordata.at(2).toString();
        jData[CP_ADRESSE1_CORRESP]   = cordata.at(3).toString();
        jData[CP_ADRESSE2_CORRESP]   = cordata.at(4).toString();
        jData[CP_ADRESSE3_CORRESP]   = cordata.at(5).toString();
        jData[CP_CODEPOSTAL_CORRESP] = cordata.at(6).toString();
        jData[CP_VILLE_CORRESP]      = cordata.at(7).toString();
        jData[CP_TELEPHONE_CORRESP]  = cordata.at(8).toString();
        jData[CP_SEXE_CORRESP]       = cordata.at(9).toString();
        jData[CP_ISMEDECIN]          = (cordata.at(10).toInt()==1);
        jData[CP_PORTABLE_CORRESP]   = cordata.at(11).toString();
        jData[CP_FAX_CORRESP]        = cordata.at(12).toString();
        jData[CP_MAIL_CORRESP]       = cordata.at(13).toString();
        jData[CP_ISGENERALISTE]      = (cordata.at(14).toInt() == 0);
        jData[CP_SPECIALITE_CORRESP] = cordata.at(14).toInt();
        jData[CP_ISALLLOADED]        = true;
    }
    return jData;
}

/*
 * DocsExternes
*/
QList<DocExterne*> DataBase::loadDoscExternesByPatient(Patient *pat)
{
    QList<DocExterne*> docsexternes;
    if (pat == Q_NULLPTR)
        return QList<DocExterne*>();
    QString req = "Select idImpression, TypeDoc, SousTypeDoc, Titre, Dateimpression,"
                  " compression, lienversfichier, formatdoc, Importance from " TBL_DOCSEXTERNES
                  " where idpat = " + QString::number(pat->id());
#ifdef Q_OS_LINUX
    req += " and formatdoc <> '" VIDEO "'";
#endif

    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return docsexternes;
    for (int i=0; i<doclist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ISALLLOADED]                  = false;
        jData[CP_ID_DOCSEXTERNES]     = doclist.at(i).at(0).toInt();
        jData[CP_IDPAT_DOCSEXTERNES]            = pat->id();
        jData[CP_TYPEDOC_DOCSEXTERNES]          = doclist.at(i).at(1).toString();
        jData[CP_SOUSTYPEDOC_DOCSEXTERNES]      = doclist.at(i).at(2).toString();

        jData[CP_TITRE_DOCSEXTERNES]            = doclist.at(i).at(3).toString();
        jData[CP_DATE_DOCSEXTERNES]             = QDateTime(doclist.at(i).at(4).toDate(), doclist.at(i).at(4).toTime()).toMSecsSinceEpoch();
        jData[CP_COMPRESSION_DOCSEXTERNES]      = doclist.at(i).at(5).toInt();
        jData[CP_LIENFICHIER_DOCSEXTERNES]      = doclist.at(i).at(6).toString();
        jData[CP_FORMATDOC_DOCSEXTERNES]        = doclist.at(i).at(7).toString();
        jData[CP_IMPORTANCE_DOCSEXTERNES]       = doclist.at(i).at(8).toInt();
        DocExterne *doc = new DocExterne(jData);
        if (doc != Q_NULLPTR)
            docsexternes << doc;
    }
    return docsexternes;
}

QJsonObject DataBase::loadDocExterneData(int idDoc)
{
    QJsonObject jData{};
    QString req = "Select idImpression, idUser, idPat, TypeDoc, SousTypeDoc,"
                  " Titre, TextEntete, TextCorps, TextOrigine, TextPied,"
                  " Dateimpression, compression, lienversfichier, ALD, UserEmetteur,"
                  " formatdoc, Importance, idRefraction from " TBL_DOCSEXTERNES
                  " where idimpression = " + QString::number(idDoc);
    QVariantList docdata = getFirstRecordFromStandardSelectSQL(req, ok);
    if (!ok || docdata.size()==0)
        return jData;
    jData[CP_ISALLLOADED]                  = true;

    jData[CP_ID_DOCSEXTERNES]     = docdata.at(0).toInt();
    jData[CP_IDUSER_DOCSEXTERNES]           = docdata.at(1).toInt();
    jData[CP_IDPAT_DOCSEXTERNES]            = docdata.at(2).toInt();
    jData[CP_TYPEDOC_DOCSEXTERNES]          = docdata.at(3).toString();
    jData[CP_SOUSTYPEDOC_DOCSEXTERNES]      = docdata.at(4).toString();

    jData[CP_TITRE_DOCSEXTERNES]            = docdata.at(5).toString();
    jData[CP_TEXTENTETE_DOCSEXTERNES]       = docdata.at(6).toString();
    jData[CP_TEXTCORPS_DOCSEXTERNES]        = docdata.at(7).toString();
    jData[CP_TEXTORIGINE_DOCSEXTERNES]      = docdata.at(8).toString();
    jData[CP_TEXTPIED_DOCSEXTERNES]         = docdata.at(9).toString();

    jData[CP_DATE_DOCSEXTERNES]             = QDateTime(docdata.at(10).toDate(), docdata.at(10).toTime()).toMSecsSinceEpoch();
    jData[CP_COMPRESSION_DOCSEXTERNES]      = docdata.at(11).toInt();
    jData[CP_LIENFICHIER_DOCSEXTERNES]      = docdata.at(12).toString();
    jData[CP_ALD_DOCSEXTERNES]              = docdata.at(13).toInt();
    jData[CP_IDEMETTEUR_DOCSEXTERNES]       = docdata.at(14).toString();

    jData[CP_FORMATDOC_DOCSEXTERNES]        = docdata.at(15).toString();
    jData[CP_IMPORTANCE_DOCSEXTERNES]       = docdata.at(16).toInt();
    jData[CP_IDREFRACTION_DOCSEXTERNES]     = docdata.at(17).toInt();

    return jData;
}

/*
 * Impressions
*/
QList<Impression*> DataBase::loadImpressions()
{
    QList<Impression*> impressions;
    QString req = "Select idDocument, TextDocument, ResumeDocument, ConclusionDocument, idUser,"
                  " DocPublic, Prescription, Editable, Medical from " TBL_IMPRESSIONS
                  " WHERE (idUser = " + QString::number(getUserConnected()->id()) + " Or (DocPublic = 1 and iduser <> " + QString::number(getUserConnected()->id()) + "))"
                  " ORDER BY ResumeDocument";
    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return impressions;
    for (int i=0; i<doclist.size(); ++i)
    {
        QJsonObject jData{};
        jData["iddocument"] = doclist.at(i).at(0).toInt();
        jData["texte"] = doclist.at(i).at(1).toString();
        jData["resume"] = doclist.at(i).at(2).toString();
        jData["conclusion"] = doclist.at(i).at(3).toString();
        jData["iduser"] = doclist.at(i).at(4).toInt();
        jData["public"] = (doclist.at(i).at(5).toInt()==1);
        jData["prescription"] = (doclist.at(i).at(6).toInt()==1);
        jData["editable"] = (doclist.at(i).at(7).toInt()==1);
        jData["medical"] = (doclist.at(i).at(8).toInt()==1);
        Impression *doc = new Impression(jData);
        if (doc != Q_NULLPTR)
            impressions << doc;
    }
    return impressions;
}

/*
 * Dossiers impression
*/
QList<DossierImpression*> DataBase::loadDossiersImpressions()
{
    QList<DossierImpression*> dossiers;
    QString     req =  "SELECT ResumeMetaDocument, idMetaDocument, idUser, Public, TextMetaDocument"
                       " FROM "  TBL_DOSSIERSIMPRESSIONS
                       " WHERE idUser = " + QString::number(getUserConnected()->id());
                req += " UNION \n";
                req += "select ResumeMetaDocument, idMetaDocument, idUser, Public, TextMetaDocument from " TBL_DOSSIERSIMPRESSIONS
                       " where idMetaDocument not in\n"
                       " (select met.idMetaDocument from " TBL_DOSSIERSIMPRESSIONS " as met, "
                       TBL_JOINTURESDOCS " as joi, "
                       TBL_IMPRESSIONS " as doc\n"
                       " where joi.idmetadocument = met.idMetaDocument\n"
                       " and joi.idDocument = doc.iddocument\n"
                       " and doc.docpublic is null)\n";
                req += " ORDER BY ResumeMetaDocument;";
    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return dossiers;
    for (int i=0; i<doclist.size(); ++i)
    {
        QJsonObject jData{};
        jData["idmetadocument"] = doclist.at(i).at(1).toInt();
        jData["texte"] = doclist.at(i).at(4).toString();
        jData["resume"] = doclist.at(i).at(0).toString();
        jData["iduser"] = doclist.at(i).at(2).toInt();
        jData["public"] = (doclist.at(i).at(3).toInt()==1);
        DossierImpression *metadoc = new DossierImpression(jData);
        if (metadoc != Q_NULLPTR)
            dossiers << metadoc;
    }
    return dossiers;
}


/*******************************************************************************************************************************************************************
 ********* COMPTABILITÊ ********************************************************************************************************************************************
********************************************************************************************************************************************************************/
/*
 * Comptes
*/
QList<Compte*> DataBase::loadComptesAll()
{
    QList<Compte*> listcomptes = QList<Compte*>();
    bool ok;
    QString req = "SELECT idCompte, idBanque, idUser, IBAN, intitulecompte, NomCompteAbrege, SoldeSurDernierReleve, partage, desactive "
          " FROM " TBL_COMPTES;
    QList<QVariantList> cptlist = DataBase::I()->StandardSelectSQL(req,ok);
    if(!ok || cptlist.size()==0)
        return listcomptes;
    for (int i=0; i<cptlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_IDCOMPTE_COMPTES]  = cptlist.at(i).at(0).toInt();
        jData[CP_IDBANQUE_COMPTES]  = cptlist.at(i).at(1).toInt();
        jData[CP_IDUSER_COMPTES]    = cptlist.at(i).at(2).toInt();
        jData[CP_IBAN_COMPTES]      = cptlist.at(i).at(3).toString();
        jData[CP_INTITULE_COMPTES]  = cptlist.at(i).at(4).toString();
        jData[CP_NOMABREGE_COMPTES] = cptlist.at(i).at(5).toString();
        jData[CP_SOLDE_COMPTES]     = cptlist.at(i).at(6).toDouble();
        jData[CP_PARTAGE_COMPTES]   = (cptlist.at(i).at(7).toInt() == 1);
        jData[CP_DESACTIVE_COMPTES] = (cptlist.at(i).at(8).toInt() == 1);
        Compte *cpt = new Compte(jData);
        if (cpt != Q_NULLPTR)
            listcomptes << cpt;
    }
    return listcomptes;
}

QJsonObject DataBase::loadCompteDataById(int id)
{
    QJsonObject jData{};
    bool ok;
    QString req = "SELECT idCompte, idBanque, idUser, IBAN, intitulecompte, NomCompteAbrege, SoldeSurDernierReleve, partage, desactive "
          " FROM " TBL_COMPTES
          " where idCompte = " + QString::number(id);
    QList<QVariantList> cptlist = DataBase::I()->StandardSelectSQL(req,ok);
    if(!ok || cptlist.size()==0)
        return jData;
    for (int i=0; i<cptlist.size(); ++i)
    {
        jData[CP_IDCOMPTE_COMPTES]  = id;
        jData[CP_IDBANQUE_COMPTES]  = cptlist.at(i).at(1).toInt();
        jData[CP_IDUSER_COMPTES]    = cptlist.at(i).at(2).toInt();
        jData[CP_IBAN_COMPTES]      = cptlist.at(i).at(3).toString();
        jData[CP_INTITULE_COMPTES]  = cptlist.at(i).at(4).toString();
        jData[CP_NOMABREGE_COMPTES] = cptlist.at(i).at(5).toString();
        jData[CP_SOLDE_COMPTES]     = cptlist.at(i).at(6).toDouble();
        jData[CP_PARTAGE_COMPTES]   = (cptlist.at(i).at(7).toInt() == 1);
        jData[CP_DESACTIVE_COMPTES] = (cptlist.at(i).at(8).toInt() == 1);
    }
    return jData;
}

int DataBase::getIdMaxTableComptesTableArchives()
{
    int a(0), b(0);
    a = selectMaxFromTable("idligne", TBL_ARCHIVESBANQUE, ok);
    b = selectMaxFromTable("idligne", TBL_LIGNESCOMPTES, ok);
    return (((a<b)?b:a)+1);
}


/*
 * Depenses
*/
QList<Depense*> DataBase::loadDepensesByUser(int idUser)
{
    QList<Depense*> depenses;
    QString req = "SELECT idDep, DateDep , dep.RefFiscale, Objet, Montant,"
                        " dep.FamFiscale, Monnaie, idRec, ModePaiement, Compte,"
                        " NoCheque, dep.idFacture, LienFichier, Echeancier, Intitule,"
                        " idRubrique"
                        " FROM " TBL_DEPENSES " dep"
                        " left join " TBL_FACTURES " fac on dep.idFacture = fac.idFacture"
                        " left join " TBL_RUBRIQUES2035 " rub on dep.RefFiscale = rub.RefFiscale"
                        " WHERE dep.idUser = " + QString::number(idUser);
    //qDebug() << req;
    QList<QVariantList> deplist = StandardSelectSQL(req,ok);
    if(!ok || deplist.size()==0)
        return depenses;
    for (int i=0; i<deplist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_IDDEPENSE_DEPENSES]      = deplist.at(i).at(0).toInt();
        jData[CP_IDUSER_DEPENSES]         = idUser;
        jData[CP_DATE_DEPENSES]           = deplist.at(i).at(1).toDate().toString("yyyy-MM-dd");
        jData[CP_REFFISCALE_DEPENSES]     = deplist.at(i).at(2).toString();
        jData[CP_OBJET_DEPENSES]          = deplist.at(i).at(3).toString();
        jData[CP_MONTANT_DEPENSES]        = deplist.at(i).at(4).toDouble();
        jData[CP_FAMILLEFISCALE_DEPENSES] = deplist.at(i).at(5).toString();
        jData[CP_MONNAIE_DEPENSES]        = deplist.at(i).at(6).toString();
        jData[CP_IDRECETTE_DEPENSES]      = deplist.at(i).at(7).toInt();
        jData[CP_MODEPAIEMENT_DEPENSES]   = deplist.at(i).at(8).toString();
        jData[CP_COMPTE_DEPENSES]         = deplist.at(i).at(9).toInt();
        jData[CP_NUMCHEQUE_DEPENSES]      = deplist.at(i).at(10).toInt();
        jData[CP_IDFACTURE_DEPENSES]      = deplist.at(i).at(11).toInt();
        jData["lienfacture"]              = deplist.at(i).at(12).toString();
        jData["echeancier"]               = (deplist.at(i).at(13).toInt()==1);
        jData["objetecheancier"]          = deplist.at(i).at(14).toString();
        jData["idrubrique"]               = deplist.at(i).at(15).toInt();
        Depense *dep = new Depense(jData);
        if (dep != Q_NULLPTR)
            depenses << dep;
    }

    return depenses;
}

void DataBase::loadDepenseArchivee(Depense *dep)
{
    bool archivee = false;
    QString req = "select idLigne from " TBL_ARCHIVESBANQUE " where idDep = " + QString::number(dep->id());
    QVariantList arcdata = getFirstRecordFromStandardSelectSQL(req,ok);
    archivee = ok && arcdata.size() > 0;
    if (!archivee)  // pour les anciens enregistrements qui étaient archivés sans l'id...
    {
        req = "select idligne from " TBL_ARCHIVESBANQUE
              " where LigneDate = '" + dep->date().toString("yyyy-MM-dd") +
              "' and LigneLibelle = '" + Utils::correctquoteSQL(dep->objet()) +
              "' and LigneMontant = " + QString::number(dep->montant());
        arcdata = getFirstRecordFromStandardSelectSQL(req,ok);
        archivee = ok && arcdata.size() > 0;
    }
    dep->setarchivee(archivee);
}

QStringList DataBase::ListeRubriquesFiscales()
{
    QStringList ListeRubriques = QStringList();
    QString req = "SELECT reffiscale from " TBL_RUBRIQUES2035 " where FamFiscale is not null and famfiscale <> 'Prélèvement personnel' order by reffiscale";
    QList<QVariantList> rublist = StandardSelectSQL(req,ok);
    if(!ok || rublist.size()==0)
        return ListeRubriques;
    ListeRubriques << "Prélèvement personnel";
    for (int i=0; i<rublist.size(); ++i)
        ListeRubriques << rublist.at(i).at(0).toString();
    return ListeRubriques;
}

QList<Depense*> DataBase::VerifExistDepense(QMap<int, Depense *> m_listDepenses, QDate date, QString objet, double montant, int iduser, enum comparateur Comp)
{
    QString op = "=";
    if (Comp == DataBase::Sup)
        op = ">";
    else if (Comp == DataBase::Inf)
        op = "<";
    QList<Depense*> listdepenses;
    QString req = "select idDep from " TBL_DEPENSES " where DateDep " + op + "'" + date.toString("yyyy-MM-dd") +
            "'and Objet = '" + Utils::correctquoteSQL(objet) +
            "'and Montant = " + QString::number(montant) +
            " and idUser = " + QString::number(iduser) +
            " order by DateDep";
    QList<QVariantList> deplist = StandardSelectSQL(req,ok);
    if(!ok || deplist.size()==0)
        return listdepenses;
    for (int i=0; i<deplist.size(); ++i)
    {
        QMap<int, Depense*>::const_iterator itDepense = m_listDepenses.find(deplist.at(i).at(0).toInt());
        if (itDepense != m_listDepenses.constEnd())
        {
            Depense *dep = itDepense.value();
            listdepenses << dep;
        }
    }
    return listdepenses;
}

/*
 * Archives
*/
QList<Archive*> DataBase::loadArchiveByDate(QDate date, Compte *compte, int intervalle)
{
    QList<Archive*> archives;
    QString req = "select idLigne, idcompte, iddep, idrec, idrecspec, idremcheq, LigneDate, LigneLibelle, LigneMontant,"
                  " LigneDebitCredit, LigneTypeoperation, LigneDateConsolidation, idArchive from " TBL_ARCHIVESBANQUE
                  " where idCompte = " + QString::number(compte->id())
                + " and lignedateconsolidation > '" + date.addDays(-intervalle).toString("yyyy-MM-dd") + "'"
                + " and lignedateconsolidation <= '" + date.toString("yyyy-MM-dd") + "'";
    QList<QVariantList> arclist = StandardSelectSQL(req,ok);
    if(!ok || arclist.size()==0)
        return archives;
    for (int i=0; i<arclist.size(); ++i)
    {
       QJsonObject jData{};
        jData["idligne"]                = arclist.at(i).at(0).toInt();
        jData["idcompte"]               = arclist.at(i).at(1).toInt();
        jData["iddepense"]              = arclist.at(i).at(2).toInt();
        jData["idrecette"]              = arclist.at(i).at(3).toInt();
        jData["idrecettespeciale"]      = arclist.at(i).at(4).toInt();
        jData["idremisecheque"]         = arclist.at(i).at(5).toInt();
        jData["lignedate"]              = arclist.at(i).at(6).toDate().toString("yyyy-MM-dd");
        jData["lignelibelle"]           = arclist.at(i).at(7).toString();
        jData["montant"]                = (arclist.at(i).at(9).toInt()==1? arclist.at(i).at(8).toDouble() : arclist.at(i).at(8).toDouble()*-1);
        jData["lignetypeoperation"]     = arclist.at(i).at(10).toString();
        jData["lignedateconsolidation"] = arclist.at(i).at(11).toDate().toString("yyyy-MM-dd");
        jData["idarchive"]              = arclist.at(i).at(12).toInt();
        Archive *arc = new Archive(jData);
        if (arc != Q_NULLPTR)
            archives << arc;
    }
    return archives;
}

/*
 * Banques
*/
QList<Banque*> DataBase::loadBanques()
{
    QList<Banque*> banques;
    QString req = "SELECT idBanque, idBanqueAbrege, NomBanque, CodeBanque FROM " TBL_BANQUES " order by nomBanque";
    QList<QVariantList> banqlist = StandardSelectSQL(req,ok);
    if(!ok || banqlist.size()==0)
        return banques;
    for (int i=0; i<banqlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_IDBANQUE_BANQUES] = banqlist.at(i).at(0).toInt();
        jData[CP_NOMABREGE_BANQUES] = banqlist.at(i).at(1).toString();
        jData[CP_NOMBANQUE_BANQUES] = banqlist.at(i).at(2).toString();
        jData[CP_CODE_BANQUES] = banqlist.at(i).at(3).toInt();
        Banque *bq = new Banque(jData);
        if (bq != Q_NULLPTR)
            banques << bq;
    }
    return banques;
}

/*
 * Tiers
*/
QList<Tiers*> DataBase::loadTiersPayants()
{
    QList<Tiers*> listetiers;
    QString req = "SELECT idtIERS, Nomtiers, AdresseTiers, Codepostaltiers, Villetiers, Telephonetiers, FaxTiers from " TBL_TIERS;
    QList<QVariantList> tierslist = StandardSelectSQL(req,ok);
    if(!ok || tierslist.size()==0)
        return listetiers;
    for (int i=0; i<tierslist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"] = tierslist.at(i).at(0).toInt();
        jData["nomtiers"] = tierslist.at(i).at(1).toString();
        jData["adressetiers"] = tierslist.at(i).at(2).toString();
        jData["codepostaltiers"] = tierslist.at(i).at(3).toString();
        jData["villetiers"] = tierslist.at(i).at(4).toString();
        jData["telephonetiers"] = tierslist.at(i).at(5).toString();
        jData["faxtiers"] = tierslist.at(i).at(5).toString();
        Tiers *tiers = new Tiers(jData);
        if (tiers != Q_NULLPTR)
            listetiers << tiers;
    }

    return listetiers;
}

QList<TypeTiers*> DataBase::loadTypesTiers()
{
    QList<TypeTiers*> types;
    QString req = "SELECT Tiers FROM " TBL_LISTETIERS;
    QList<QVariantList> tierslist = StandardSelectSQL(req,ok);
    if(!ok || tierslist.size()==0)
        return types;
    for (int i=0; i<tierslist.size(); ++i)
    {
        QJsonObject jData{};
        jData["typetiers"] = tierslist.at(i).at(0).toString();
        TypeTiers *type = new TypeTiers(jData);
        if (type != Q_NULLPTR)
            types << type;
    }
    return types;
}

/*
 * Recettes
*/
QList<Recette*> DataBase::loadRecettesByDate(QDate datedebut, QDate datefin)
{
    QList<Recette*> listerecettes;
        //---------------------------------------------- Tous les actes effectués par tout le monde durant la période, sauf les impayés et les gratuits
        QString req =
        "select res1.idActe, res1.actedate, res1.nom, res1.actecotation, res1.acteMontant, res1.actemonnaie, res1.TypePaiement,"
        " res1.Tiers, Paye, res1.iduser, res1.userparent, res1.usercomptable, null as montantautresrecettes, null as typeautresrecettes from\n "
        "(\n"
            "select\n"
            " act." CP_IDACTE_ACTES ", " CP_DATE_ACTES ", concat(patnom, ' ', patprenom) as nom, " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES
            ", TypePaiement, Tiers, " CP_IDUSER_ACTES ", " CP_IDUSERPARENT_ACTES ", " CP_IDUSERCOMPTABLE_ACTES " from \n"
            TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ\n"
            " where act." CP_IDPAT_ACTES " = pat.idpat\n"
            " and act." CP_IDACTE_ACTES " = typ.idacte\n"
            " and " CP_DATE_ACTES " >= '" + datedebut.toString("yyyy-MM-dd") + "'\n"
            " and " CP_DATE_ACTES " <= '" + datefin.toString("yyyy-MM-dd") + "'\n"
            " order by " CP_DATE_ACTES ", nom\n"
        ")\n"
        " as res1\n"
        " left outer join\n"
        " (\n"
            "select rec.idrecette, paye, lig.idActe from \n"
            TBL_LIGNESPAIEMENTS " lig, " TBL_RECETTES " rec, " TBL_TYPEPAIEMENTACTES " typ2\n"
            " where lig.idrecette = rec.idrecette\n"
            " and lig.idActe = typ2.idacte\n"
            " and TypePaiement <> 'T'\n"
            " and TypePaiement <> 'G'\n"
            " and datepaiement >= '" + datedebut.toString("yyyy-MM-dd") + "'\n"
            " and datepaiement <= '" + datefin.toString("yyyy-MM-dd") + "'\n"
        ")\n"
        " as res3 on res1.idacte = res3.idActe\n";

        //----------------------------------------------- et tous les tiers payants encaissés durant cette même période
        req +=
        " union\n"

        " select null as idActe, DatePaiement as actedate, NomTiers as nom, null as actecotation, null as acteMontant, Monnaie as acteMonnaie, ModePaiement as TypePaiement,"
        " null as Tiers, Montant as paye, iduser, iduser as userparent, iduser as usercomptable, null as montantautresrecettes, null as typeautresrecettes from \n"
        TBL_RECETTES
        "\n where TiersPayant = 'O'\n"
        " and DatePaiement >= '" + datedebut.toString("yyyy-MM-dd") + "'\n"
        " and DatePaiement <= '" + datefin.toString("yyyy-MM-dd") + "'\n"

        " union\n"

        " select null as idActe, DateRecette as actedate, Libelle as nom, null as actecotation, null as acteMontant, Monnaie as acteMonnaie,"
        " Paiement as TypePaiement, null as Tiers, null as paye, null as iduser, null as userparent, iduser as usercomptable,"
        " montant as montantautresrecettes, Typerecette as typeautresrecettes from \n" TBL_RECETTESSPECIALES
        " \nwhere"
        " DateRecette >= '" + datedebut.toString("yyyy-MM-dd") + "'\n"
        " and DateRecette<= '" + datefin.toString("yyyy-MM-dd") + "'\n"
        " order by actedate, nom";

        //proc->Edit(req);
        //p... ça c'est de la requête
        QList<QVariantList> recetteslist = StandardSelectSQL(req,ok);
        if(!ok || recetteslist.size()==0)
            return listerecettes;
        for (int i=0; i<recetteslist.size(); ++i)
        {
            QJsonObject jData{};
            jData["id"] = i;
            jData["idacte"] = (recetteslist.at(i).at(0) == QVariant()? -1 : recetteslist.at(i).at(0).toInt());
            jData["date"] = recetteslist.at(i).at(1).toDate().toString("yyyy-MM-dd");;
            jData["payeur"] = recetteslist.at(i).at(2).toString();
            jData["cotationacte"] = recetteslist.at(i).at(3).toString();
            jData["montant"] = recetteslist.at(i).at(4).toDouble();
            jData["monnaie"] = recetteslist.at(i).at(5).toString();
            jData["modepaiement"] = recetteslist.at(i).at(6).toString();
            jData["typetiers"] = recetteslist.at(i).at(7).toString();
            jData["encaissement"] = recetteslist.at(i).at(8).toDouble();
            jData["iduser"] = recetteslist.at(i).at(9).toInt();
            jData["idparent"] = recetteslist.at(i).at(10).toInt();
            jData["idcomptable"] = recetteslist.at(i).at(11).toInt();
            jData["encaissementautrerecette"] = recetteslist.at(i).at(12).toDouble();
            jData["apportpraticien"] = (recetteslist.at(i).at(13).toString() == tr("Apport praticien"));
            jData["autrerecette"] = (recetteslist.at(i).at(13).toString() == tr("Divers et autres recettes"));
            Recette *recette = new Recette(jData);
            if (recette != Q_NULLPTR)
                listerecettes << recette;
        }
        return listerecettes;
}

/*
 * Recettes comptables
*/
RecetteComptable* DataBase::loadRecetteComptablebyId(int id)
{
    QString req =   "SELECT idUser, DatePaiement, DateEnregistrement, Montant, ModePaiement,"
                    " TireurCheque, CompteVirement, BanqueCheque, TiersPayant, NomTiers,"
                    " Commission, Monnaie, idRemise, EnAttente, EnregistrePar,"
                    " TypeRecette FROM " TBL_RECETTES
                    " WHERE idRecette = " + QString::number(id);
    QVariantList recette = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || recette.size()==0)
        return Q_NULLPTR;
    QJsonObject jData{};
    jData[CP_IDRECETTE_LIGNRECETTES]            = id;
    jData[CP_IDUSER_LIGNRECETTES]               = recette.at(0).toInt();
    jData[CP_DATE_LIGNRECETTES]                 = recette.at(1).toDate().toString("yyyy-MM-dd");
    jData[CP_DATEENREGISTREMENT_LIGNRECETTES]   = recette.at(2).toDate().toString("yyyy-MM-dd");
    jData[CP_MONTANT_LIGNRECETTES]              = recette.at(3).toDouble();
    jData[CP_MODEPAIEMENT_LIGNRECETTES]         = recette.at(4).toString();
    jData[CP_TIREURCHEQUE_LIGNRECETTES]         = recette.at(5).toString();
    jData[CP_IDCPTEVIREMENT_LIGNRECETTES]       = recette.at(6).toInt();
    jData[CP_BANQUECHEQUE_LIGNRECETTES]         = recette.at(7).toString();
    jData[CP_TIERSPAYANT_LIGNRECETTES]          = (recette.at(8).toString() == "O");
    jData[CP_NOMPAYEUR_LIGNRECETTES]            = recette.at(9).toString();
    jData[CP_COMMISSION_LIGNRECETTES]           = recette.at(10).toDouble();
    jData[CP_MONNAIE_LIGNRECETTES]              = recette.at(11).toString();
    jData[CP_IDREMISECHQ_LIGNRECETTES]          = recette.at(12).toInt();
    jData[CP_CHQENATTENTE_LIGNRECETTES]         = (recette.at(13).toInt() == 1);
    jData[CP_IDUSERENREGISTREUR_LIGNRECETTES]   = recette.at(14).toInt();
    jData[CP_TYPERECETTE_LIGNRECETTES]          = recette.at(15).toInt();
    RecetteComptable *rec = new RecetteComptable(jData);
    return rec;
}


/*
 * PaiementsTiers
*/
QList<PaiementTiers*> DataBase::loadPaiementTiersByUser(User* usr)
{
    QList<PaiementTiers*> listepaiements;
    if (usr == Q_NULLPTR)
        return listepaiements;
    QString req =   "SELECT idRecette, DatePaiement, DateEnregistrement, Montant, ModePaiement,"
                    " TireurCheque, CompteVirement, BanqueCheque, NomTiers, Commission,"
                    " Monnaie, idRemise, EnAttente, EnregistrePar, TypeRecette,"
                    " RCDate FROM " TBL_RECETTES
                "\n LEFT OUTER JOIN (SELECT RCDate, idRemCheq FROM " TBL_REMISECHEQUES ") AS rc\n"
                " ON rc.idRemCheq = idRemise\n"
                " WHERE idUser = " + QString::number(usr->id()) +
                "\n AND TiersPayant = 'O'\n"
                " ORDER BY DatePaiement DESC, NomTiers";
    QList<QVariantList> paiementslist = StandardSelectSQL(req,ok);
    if(!ok || paiementslist.size()==0)
        return listepaiements;
    for (int i=0; i<paiementslist.size(); ++i)
    {
        QJsonObject jData{};
        jData["idpaiement"]                 = paiementslist.at(i).at(0).toInt();
        jData["date"]                       = paiementslist.at(i).at(1).toDate().toString("yyyy-MM-dd");
        jData["dateenregistrement"]         = paiementslist.at(i).at(2).toDate().toString("yyyy-MM-dd");
        jData["montant"]                    = paiementslist.at(i).at(3).toDouble();
        jData["modepaiement"]               = paiementslist.at(i).at(4).toString();
        jData["tireurcheque"]               = paiementslist.at(i).at(5).toString();
        jData["comptevirement"]             = paiementslist.at(i).at(6).toInt();
        jData["banquecheque"]               = paiementslist.at(i).at(7).toString();
        jData["nomtiers"]                   = paiementslist.at(i).at(8).toString();
        jData["commission"]                 = paiementslist.at(i).at(9).toDouble();
        jData["monnaie"]                    = paiementslist.at(i).at(10).toString();
        jData["idremisecheque"]             = paiementslist.at(i).at(11).toInt();
        jData["chequeenattente"]            = (paiementslist.at(i).at(12).toInt() == 1);
        jData["iduserenregistreur"]         = paiementslist.at(i).at(13).toInt();
        jData["dateremisecheques"]          = paiementslist.at(i).at(14).toDate().toString("yyyy-MM-dd");
        jData["encaissement"]               = 0;
        PaiementTiers *pimt = new PaiementTiers(jData);
        if (pimt != Q_NULLPTR)
            listepaiements << pimt;
    }
    return listepaiements;
}


/*
 * Lignes  paiements
*/
QList<LignePaiement *> DataBase::loadlignespaiementsByPatient(Patient *pat)
{
    QList<LignePaiement*> listepaiements;
    if (pat == Q_NULLPTR)
        return listepaiements;
    bool ok;
    QString req =   "SELECT idActe, lig.idRecette, Paye, Monnaie FROM " TBL_LIGNESPAIEMENTS " as lig"
                    " inner join " TBL_RECETTES " rec on rec.idrecette = lig.idrecette"
                    " where idActe in"
                    " (select " CP_IDACTE_ACTES " from " TBL_ACTES " where idpat = " + QString::number(pat->id()) + ")";
    QList<QVariantList> paiementslist = StandardSelectSQL(req, ok);
    if(!ok || paiementslist.size()==0)
        return listepaiements;
    for (int i=0; i<paiementslist.size(); ++i)
    {
        QJsonObject jData{};
        QString stringid    = paiementslist.at(i).at(0).toString() + TCPMSG_Separator + paiementslist.at(i).at(1).toString();
        jData["stringid"]   = stringid;
        jData["paye"]       = paiementslist.at(i).at(2).toDouble();
        jData["monnaie"]    = paiementslist.at(i).at(3).toString();
        LignePaiement *pimt = new LignePaiement(jData);
        if (pimt != Q_NULLPTR)
            listepaiements << pimt;
    }
    return listepaiements;
}
/*******************************************************************************************************************************************************************
 ***** FIN COMPTABILITÊ ********************************************************************************************************************************************
********************************************************************************************************************************************************************/

/*
 * Cotations
*/
QList<Cotation*> DataBase::loadCotationsByUser(int iduser)
{
    int k = 0;

    QList<Cotation*> cotations;
    QString  req = "SELECT idcotation, Typeacte, OPTAM, nonOPTAM, MontantPratique, CCAM, Frequence, nom"
          " FROM " TBL_COTATIONS " cot left join " TBL_CCAM " cc on cot.typeacte= cc.codeccam"
          " where idUser = " + QString::number(iduser) + " and typeacte in (select codeccam from " TBL_CCAM ")"
          " order by typeacte";
    //qDebug() << req;
    QList<QVariantList> cotlist = StandardSelectSQL(req,ok);
    if(!ok)
        return cotations;
    for (int i=0; i<cotlist.size(); ++i)
    {
        ++k;
        QJsonObject jcotation{};
        jcotation["id"] = k;
        jcotation["idcotation"] = cotlist.at(i).at(0).toInt();
        jcotation["typeacte"] = cotlist.at(i).at(1).toString();
        jcotation["montantoptam"] = cotlist.at(i).at(2).toDouble();
        jcotation["montantnonoptam"] = cotlist.at(i).at(3).toDouble();
        jcotation["montantpratique"] = cotlist.at(i).at(4).toDouble();
        jcotation["ccam"] = (cotlist.at(i).at(5).toInt()==1);
        jcotation["iduser"] = iduser;
        jcotation["frequence"] = cotlist.at(i).at(6).toInt();
        jcotation["descriptif"] = cotlist.at(i).at(7).toString();
        Cotation *cotation = new Cotation(jcotation);
        if (cotation != Q_NULLPTR)
            cotations << cotation;
    }
    req = " SELECT idcotation, Typeacte, MontantOPTAM, MontantNonOPTAM, MontantPratique, CCAM, Frequence, tip"
          " FROM "  TBL_COTATIONS
          " where idUser = " + QString::number(iduser) +
          " and typeacte not in (select codeccam from  " TBL_CCAM ")"
          " order by typeacte";
    cotlist = StandardSelectSQL(req,ok);
    if(!ok || cotlist.size()==0)
        return cotations;
    for (int i=0; i<cotlist.size(); ++i)
    {
        k++;
        QJsonObject jcotation{};
        jcotation["id"] = k;
        jcotation["idcotation"] = cotlist.at(i).at(0).toInt();
        jcotation["typeacte"] = cotlist.at(i).at(1).toString();
        jcotation["montantoptam"] = cotlist.at(i).at(2).toDouble();
        jcotation["montantnonoptam"] = cotlist.at(i).at(3).toDouble();
        jcotation["montantpratique"] = cotlist.at(i).at(4).toDouble();
        jcotation["ccam"] = (cotlist.at(i).at(5).toInt()==1);
        jcotation["iduser"] = iduser;
        jcotation["frequence"] = cotlist.at(i).at(6).toInt();
        jcotation["descriptif"] = cotlist.at(i).at(7).toString();
        Cotation *cotation = new Cotation(jcotation);
        if (cotation != Q_NULLPTR)
            cotations << cotation;
    }
    return cotations;
}

QStringList DataBase::loadTypesCotations()
{
    QStringList listcotations;
    QString req = "select distinct typeacte as code from " TBL_COTATIONS
                  " union "
                  " select codeccam as code from " TBL_CCAM
                  " order by code asc";
    QList<QVariantList> cotlist = StandardSelectSQL(req,ok);
    if(!ok || cotlist.size()==0)
        return listcotations;
    for (int i=0; i<cotlist.size(); ++i)
        listcotations << cotlist.at(i).at(0).toString();
    return listcotations;
}

/*
 * Motifs
*/
QList<Motif*> DataBase::loadMotifs()
{
    QList<Motif*> motifs;
    QString  req = "SELECT idMotifsRDV, Motif, Raccourci, Couleur, Duree, ParDefaut, Utiliser, NoOrdre FROM "  TBL_MOTIFSRDV " ORDER BY NoOrdre";
    QList<QVariantList> mtflist = StandardSelectSQL(req,ok);
    if(!ok || mtflist.size()==0)
        return motifs;
    for (int i=0; i<mtflist.size(); ++i)
    {
        QJsonObject jmotif{};
        jmotif["id"]        = mtflist.at(i).at(0).toInt();
        jmotif["motif"]     = mtflist.at(i).at(1).toString();
        jmotif["raccourci"] = mtflist.at(i).at(2).toString();
        jmotif["couleur"]   = mtflist.at(i).at(3).toString();
        jmotif["duree"]     = mtflist.at(i).at(4).toInt();
        jmotif["pardefaut"] = (mtflist.at(i).at(5).toInt()==1);
        jmotif["utiliser"]  = (mtflist.at(i).at(6).toInt()==1);
        jmotif["noordre"]   = mtflist.at(i).at(7).toInt();
        Motif *motif = new Motif(jmotif);
        if (motif != Q_NULLPTR)
            motifs << motif;
    }
    return motifs;
}

/*
 * Sites
*/
QList<Site*> DataBase::loadSitesAll()
{
    int iduser = 1;
    QString req = "select lieux.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3,"
                  " LieuCodePostal, LieuVille, LieuTelephone, LieuFax, iduser"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint.idlieu = lieux.idLieu"
                  " where iduser = " + QString::number(iduser) +
                  " union"
                  " (select lieux.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3,"
                  " LieuCodePostal, LieuVille, LieuTelephone, LieuFax, null as iduser"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint.idlieu = lieux.idLieu"
                  " where iduser <> " + QString::number(iduser) + " and iduser is not null"
                  " and lieux.idlieu not in"
                  " (select lieux.idLieu"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint.idlieu = lieux.idLieu"
                  " where iduser = " + QString::number(iduser) + ")";
                  " union"
                  " (select lieux.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3,"
                  " LieuCodePostal, LieuVille, LieuTelephone, LieuFax, null as iduser"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint.idlieu = lieux.idLieu"
                  " where iduser is not null"
                  " and lieux.idlieu not in"
                  " (select lieux.idLieu"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint.idlieu = lieux.idLieu"
                  " where iduser = " + QString::number(iduser) + ")";
    /*!< cette requête sert à recenser tous les lieux de travail avec le champ iduser positionné
     *  à l'id du user en cours s'il est utilisé par l'user en cours,
     *  à -1 s'il est utilisé par d'autres utilisateurs mais pas le user en cours
     *  et à null s'il n'est utilisé par personne
     * il doit y avoir moyen de faire plus simple mais je ne sais pas comment
     */
    //qDebug() << req;

    req = "select idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, "
                    "LieuCodePostal, LieuVille, LieuTelephone, LieuFax "
                  "from " TBL_LIEUXEXERCICE;
    return loadSites( req );
}
QList<Site*> DataBase::loadSitesByUser(int idUser)
{
    QString req = "select joint.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, "
                  "LieuCodePostal, LieuVille, LieuTelephone, LieuFax "
                  "from " TBL_JOINTURESLIEUX " joint "
                  "left join " TBL_LIEUXEXERCICE " lix on joint.idlieu = lix.idLieu "
                  "where iduser = " + QString::number(idUser);
    return loadSites( req );
}
QList<Site*> DataBase::loadSites(QString req)
{
    QList<Site*> etabs;
    QList<QVariantList> sitlist = StandardSelectSQL(req,ok);
    if(!ok || sitlist.size()==0)
        return etabs;
    for (int i=0; i<sitlist.size(); ++i)
    {
        QJsonObject jEtab{};
        jEtab["idLieu"] = sitlist.at(i).at(0).toInt();
        jEtab["nomLieu"] = sitlist.at(i).at(1).toString();
        jEtab["adresse1"] = sitlist.at(i).at(2).toString();
        jEtab["adresse2"] = sitlist.at(i).at(3).toString();
        jEtab["adresse3"] = sitlist.at(i).at(4).toString();
        jEtab["codePostal"] = sitlist.at(i).at(5).toInt();
        jEtab["ville"] = sitlist.at(i).at(6).toString();
        jEtab["telephone"] = sitlist.at(i).at(7).toString();
        jEtab["fax"] = sitlist.at(i).at(8).toString();
        Site *etab = new Site(jEtab);
        if (etab != Q_NULLPTR)
            etabs << etab;
    }
    return etabs;
}

/*
 * Villes
*/
QList<Ville*> DataBase::loadVilles()
{
    QList<Ville*> villes;

    QString req = "select ville_id, codePostal, ville "
                  "from " TBL_VILLES;
    QList<QVariantList> villist = StandardSelectSQL(req,ok);
    if(!ok || villist.size()==0)
        return villes;
    for (int i=0; i<villist.size(); ++i)
    {
        QJsonObject jEtab{};
        jEtab["ville_id"] = villist.at(i).at(0).toInt();
        jEtab["codePostal"] = villist.at(i).at(1).toString();
        jEtab["ville"] = villist.at(i).at(2).toString();
        Ville *ville = new Ville(jEtab);
        if (ville != Q_NULLPTR)
            villes << ville;
    }
    return villes;
}


/*
 * Gestion des Patients présents dans le centre de soins
*/
PatientEnCours* DataBase::loadPatientEnCoursById(int idPat)
{
    PatientEnCours *pat = new PatientEnCours;
    QString req = "SELECT idPat, IdUser, Statut, HeureStatut,  HeureRDV,"
                  " HeureArrivee, Motif, Message, idActeAPayer, PosteExamen,"
                  " idUserEnCoursExam, idSalDat FROM " TBL_SALLEDATTENTE " where idPat = " + QString::number(idPat);
    QVariantList patdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if( !ok || patdata.size()==0 )
    {
        delete pat;
        pat = Q_NULLPTR;
        return pat;
    }
    QJsonObject jData = loadPatientEnCoursData(patdata);
    pat->setData(jData);
    return pat;
}

QJsonObject DataBase::loadPatientEnCoursDataById(int idPat)
{
    QString req = "SELECT idPat, IdUser, Statut, HeureStatut,  HeureRDV,"
                  " HeureArrivee, Motif, Message, idActeAPayer, PosteExamen,"
                  " idUserEnCoursExam, idSalDat FROM " TBL_SALLEDATTENTE " where idPat = " + QString::number(idPat);
    QVariantList patdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if( !ok || patdata.size()==0 )
        return QJsonObject();
    QJsonObject jData = loadPatientEnCoursData(patdata);
    return jData;
}

QJsonObject DataBase::loadPatientEnCoursData(QVariantList patdata)
{
    QJsonObject jData{};
    if( !ok || patdata.size()==0 )
        return jData;
    jData[CP_IDPAT_SALDAT]              = patdata.at(0).toInt();
    jData[CP_IDUSER_SALDAT]             = patdata.at(1).toInt();
    jData[CP_STATUT_SALDAT]             = patdata.at(2).toString();
    jData[CP_HEURESTATUT_SALDAT]        = patdata.at(3).toTime().toString("HH:mm:ss");
    jData[CP_HEURERDV_SALDAT]           = patdata.at(4).toTime().toString("HH:mm:ss");
    jData[CP_HEUREARRIVEE_SALDAT]       = patdata.at(5).toTime().toString("HH:mm:ss");
    jData[CP_MOTIF_SALDAT]              = patdata.at(6).toString();
    jData[CP_MESSAGE_SALDAT]            = patdata.at(7).toString();
    jData[CP_IDACTEAPAYER_SALDAT]       = patdata.at(8).toInt();
    jData[CP_POSTEEXAMEN_SALDAT]        = patdata.at(9).toString();
    jData[CP_IDUSERENCOURSEXAM_SALDAT]  = patdata.at(10).toInt();
    jData[CP_IDSALDAT_SALDAT]           = patdata.at(11).toInt();
    return jData;
}

QList<PatientEnCours *> DataBase::loadPatientsenCoursAll()
{
    QList<PatientEnCours*> listpat;
    QString req = "SELECT idPat, IdUser, Statut, HeureStatut,  HeureRDV,"
                  " HeureArrivee, Motif, Message, idActeAPayer, PosteExamen,"
                  " idUserEnCoursExam, idSalDat FROM " TBL_SALLEDATTENTE;
    QList<QVariantList> patlist = StandardSelectSQL(req, ok);
    if( !ok || patlist.size()==0 )
        return listpat;
    for (int i=0; i<patlist.size(); ++i)
    {
        QJsonObject jData = loadPatientEnCoursData(patlist.at(i));
        PatientEnCours *patient = new PatientEnCours(jData);
        if (patient != Q_NULLPTR)
            listpat << patient;
    }
    return listpat;
}


/*
 * Gestion des Patients
*/
void DataBase::loadSocialDataPatient(QJsonObject &jData, bool &ok)
{
    QString req = "SELECT PatAdresse1, PatAdresse2, PatAdresse3, PatCodePostal, PatVille,"
                  " PatTelephone, PatPortable, PatMail, PatNNI, PatALD,"
                  " PatCMU, PatProfession FROM " TBL_DONNEESSOCIALESPATIENTS
                  " WHERE idPat = " + QString::number(jData[CP_IDPAT_PATIENTS].toInt());
    QVariantList patlist = getFirstRecordFromStandardSelectSQL(req, ok);
    if(!ok || patlist.size()==0)
    {
        jData[CP_ISSOCIALlOADED] = false;
        return;
    }
    jData[CP_ADRESSE1_DSP]       = patlist.at(0).toString();
    jData[CP_ADRESSE2_DSP]       = patlist.at(1).toString();
    jData[CP_ADRESSE3_DSP]       = patlist.at(2).toString();
    jData[CP_CODEPOSTAL_DSP]     = patlist.at(3).toString();
    jData[CP_VILLE_DSP]          = patlist.at(4).toString();
    jData[CP_TELEPHONE_DSP]      = patlist.at(5).toString();
    jData[CP_PORTABLE_DSP]       = patlist.at(6).toString();
    jData[CP_MAIL_DSP]           = patlist.at(7).toString();
    jData[CP_NNI_DSP]            = patlist.at(8).toLongLong();
    jData[CP_ALD_DSP]            = (patlist.at(9).toInt() == 1);
    jData[CP_CMU_DSP]            = (patlist.at(10).toInt() == 1);
    jData[CP_PROFESSION_DSP]     = patlist.at(11).toString();
    jData[CP_ISSOCIALlOADED]     = true;
}

void DataBase::loadMedicalDataPatient(QJsonObject &jData, bool &ok)
{
    QString req = "select idCorMedMG, idCorMedSpe1, idCorMedSpe2, idCorMedSpe3, idCorNonMed,"
                  " RMPAtcdtsPersos, RMPTtGeneral, RMPAtcdtsFamiliaux, RMPAtcdtsOphs, Tabac,"
                  " Autrestoxiques, Gencorresp, Important, Resume, RMPTtOphs FROM " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS
                  " WHERE idPat = " + QString::number(jData[CP_IDPAT_PATIENTS].toInt());

    QVariantList patlist = getFirstRecordFromStandardSelectSQL(req, ok);
    if(!ok || patlist.size()==0)
    {
        jData[CP_ISMEDICALlOADED] = false;
        return;
    }
    jData[CP_IDMG_RMP]              = patlist.at(0).toInt();
    jData[CP_IDSPE1_RMP]            = patlist.at(1).toInt();
    jData[CP_IDSPE2_RMP]            = patlist.at(2).toInt();
    jData[CP_IDSPE3_RMP]            = patlist.at(3).toInt();
    jData[CP_IDCORNONMED_RMP]       = patlist.at(4).toInt();
    jData[CP_ATCDTSPERSOS_RMP]      = patlist.at(5).toString();
    jData[CP_TRAITMTGEN_RMP]        = patlist.at(6).toString();
    jData[CP_ATCDTSFAMLXS_RMP]      = patlist.at(7).toString();
    jData[CP_ATCDTSOPH_RMP]         = patlist.at(8).toString();
    jData[CP_TABAC_RMP]             = patlist.at(9).toString();
    jData[CP_AUTRESTOXIQUES_RMP]    = patlist.at(10).toString();
    jData[CP_GENCORRESPONDANT_RMP]  = patlist.at(11).toString();
    jData[CP_IMPORTANT_RMP]         = patlist.at(12).toString();
    jData[CP_RESUME_RMP]            = patlist.at(13).toString();
    jData[CP_TRAITMTOPH_RMP]        = patlist.at(14).toString();
    jData[CP_ISMEDICALlOADED]       = true;
}

QJsonObject DataBase::loadPatientAllData(int idPat)
{
    QJsonObject jData{};
    QString req = "SELECT PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " TBL_PATIENTS " where idPat = " + QString::number(idPat);
    QVariantList patdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if( !ok || patdata.size()==0 )
        return jData;
    jData[CP_IDPAT_PATIENTS] = idPat;
    jData[CP_NOM_PATIENTS] = patdata.at(0).toString();
    jData[CP_PRENOM_PATIENTS] = patdata.at(1).toString();
    jData[CP_DDN_PATIENTS] = patdata.at(2).toDate().toString("yyyy-MM-dd");
    jData[CP_SEXE_PATIENTS] = patdata.at(3).toString();
    jData[CP_DATECREATION_PATIENTS] = patdata.at(4).toDate().toString("yyyy-MM-dd");
    jData[CP_IDCREATEUR_PATIENTS] = patdata.at(5).toInt();
    loadMedicalDataPatient(jData, ok);
    loadSocialDataPatient(jData, ok);
    return jData;
}

Patient* DataBase::loadPatientById(int idPat, Patient *pat, Item::LOADDETAILS details)
{
    if (pat == Q_NULLPTR)
        pat = new Patient();
    QString req = "SELECT PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " TBL_PATIENTS " where idPat = " + QString::number(idPat);
    QVariantList patdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if( !ok || patdata.size()==0 )
        return Q_NULLPTR;
    QJsonObject jData{};
    jData[CP_IDPAT_PATIENTS]        = idPat;
    jData[CP_NOM_PATIENTS]          = patdata.at(0).toString();
    jData[CP_PRENOM_PATIENTS]       = patdata.at(1).toString();
    jData[CP_DDN_PATIENTS]          = patdata.at(2).toDate().toString("yyyy-MM-dd");
    jData[CP_SEXE_PATIENTS]         = patdata.at(3).toString();
    jData[CP_DATECREATION_PATIENTS] = patdata.at(4).toDate().toString("yyyy-MM-dd");
    jData[CP_IDCREATEUR_PATIENTS]   = patdata.at(5).toInt();
    if (details == Item::LoadDetails)
    {
        bool ok;
        loadMedicalDataPatient(jData, ok);
        loadSocialDataPatient(jData, ok);
    }
    pat->setData(jData);
    return pat;
}

qint64 DataBase::countPatientsAll(QString nom, QString prenom)
{
    QString clausewhere ("");
    if (Utils::correctquoteSQL(nom).length() > 0 || Utils::correctquoteSQL(prenom).length() > 0)
        clausewhere += " WHERE ";
    if (Utils::correctquoteSQL(nom).length() > 0)
        clausewhere += "PatNom like '" + Utils::correctquoteSQL(nom) + "%'";
    if (Utils::correctquoteSQL(prenom).length() > 0)
    {
        if (clausewhere != " WHERE ")
            clausewhere += " AND PatPrenom like '" + Utils::correctquoteSQL(prenom) + "%'";
        else
            clausewhere += "PatPrenom like '" + Utils::correctquoteSQL(prenom) + "%'";
    }
    QString req = "SELECT COUNT(idPat) FROM " TBL_PATIENTS + clausewhere;
    QVariantList patlist = getFirstRecordFromStandardSelectSQL(req,ok);
    return qint64(patlist.at(0).toULongLong());
}

QList<Patient*> DataBase::loadPatientsAll(QString nom, QString prenom, bool filtre)
{
    QList<Patient*> listpatients;
    QString clausewhere ("");
    QString like = (filtre? "like" : "=");
    QString clauselimit ("");
    if (Utils::correctquoteSQL(nom).length() > 0 || Utils::correctquoteSQL(prenom).length() > 0)
        clausewhere += " WHERE ";
    if (Utils::correctquoteSQL(nom).length() > 0)
        clausewhere += "PatNom " + like + " '" + Utils::correctquoteSQL(nom) + (filtre? "%" : "") + "'";
    if (Utils::correctquoteSQL(prenom).length() > 0)
    {
        if (clausewhere != " WHERE ")
            clausewhere += " AND PatPrenom " + like + " '" + Utils::correctquoteSQL(prenom) + (filtre? "%" : "") + "'";
        else
            clausewhere += "PatPrenom " + like + " '" + Utils::correctquoteSQL(prenom) + (filtre? "%" : "") + "'";
    }
    clauselimit = " limit 1000";
    QString req = "select idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar from"
                   " (select idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar from " TBL_PATIENTS
                   " force index(idx_nomprenom) order by patnom, patprenom) as idxpat";
    req += clausewhere;
    req += clauselimit;
    //qDebug() << req;
    QList<QVariantList> patlist = StandardSelectSQL(req,ok);
    if( !ok || patlist.size()==0 )
        return listpatients;
    for (int i=0; i<patlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_IDPAT_PATIENTS] = patlist.at(i).at(0).toInt();
        jData[CP_NOM_PATIENTS] = patlist.at(i).at(1).toString();
        jData[CP_PRENOM_PATIENTS] = patlist.at(i).at(2).toString();
        jData[CP_DDN_PATIENTS] = patlist.at(i).at(3).toDate().toString("yyyy-MM-dd");
        jData[CP_SEXE_PATIENTS] = patlist.at(i).at(4).toString();
        jData[CP_DATECREATION_PATIENTS] = patlist.at(i).at(5).toDate().toString("yyyy-MM-dd");
        jData[CP_IDCREATEUR_PATIENTS] = patlist.at(i).at(6).toInt();
        Patient *patient = new Patient(jData);
        if (patient != Q_NULLPTR)
            listpatients << patient;
    }
    return listpatients;
}

QList<Patient*> DataBase::loadPatientsByListId(QList<int> listid)
{
    QList<Patient*> listpatients =  QList<Patient*>();
    if (listid.size() == 0)
        return listpatients;
    QString stringlistid;
    for (int i = 0; i < listid.size(); ++i)
    {
        stringlistid += QString::number(listid.at(i));
        if (i < listid.size()-1)
            stringlistid += ",";
    }
    QString req = "select idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar from " TBL_PATIENTS " where idpat in (" + stringlistid + ")";
    //qDebug() << req;
    QList<QVariantList> patlist = StandardSelectSQL(req,ok);
    if( !ok || patlist.size()==0 )
        return listpatients;
    for (int i=0; i<patlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_IDPAT_PATIENTS] = patlist.at(i).at(0).toInt();
        jData[CP_NOM_PATIENTS] = patlist.at(i).at(1).toString();
        jData[CP_PRENOM_PATIENTS] = patlist.at(i).at(2).toString();
        jData[CP_DDN_PATIENTS] = patlist.at(i).at(3).toDate().toString("yyyy-MM-dd");
        jData[CP_SEXE_PATIENTS] = patlist.at(i).at(4).toString();
        jData[CP_DATECREATION_PATIENTS] = patlist.at(i).at(5).toDate().toString("yyyy-MM-dd");
        jData[CP_IDCREATEUR_PATIENTS] = patlist.at(i).at(6).toInt();
        Patient *patient = new Patient(jData);
        if (patient != Q_NULLPTR)
            listpatients << patient;
    }
    return listpatients;
}

QList<Patient *> DataBase::loadPatientsByDDN(QDate DDN)
{
    QList<Patient*> listpatients;
    QString req = "SELECT idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " TBL_PATIENTS
                  " WHERE PatDDN = '" + DDN.toString("yyyy-MM-dd") + "'";
    //qDebug() << req;
    QList<QVariantList> patlist = StandardSelectSQL(req,ok);
    if( !ok || patlist.size()==0 )
        return listpatients;
    for (int i=0; i<patlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_IDPAT_PATIENTS] = patlist.at(i).at(0).toInt();
        jData[CP_NOM_PATIENTS] = patlist.at(i).at(1).toString();
        jData[CP_PRENOM_PATIENTS] = patlist.at(i).at(2).toString();
        jData[CP_SEXE_PATIENTS] = patlist.at(i).at(4).toString();
        jData[CP_DDN_PATIENTS] = patlist.at(i).at(3).toDate().toString("yyyy-MM-dd");
        jData[CP_DATECREATION_PATIENTS] = patlist.at(i).at(5).toDate().toString("yyyy-MM-dd");
        jData[CP_IDCREATEUR_PATIENTS] = patlist.at(i).at(6).toInt();
        Patient *patient = new Patient(jData);
        if (patient != Q_NULLPTR)
            listpatients << patient;
    }
    return listpatients;
}

/*
 * MDP
*/
//Pas normal, les mots de passes doivent etre chiffrés
QString DataBase::getMDPAdmin()
{
    QString mdp ("");
    QVariantList mdpdata = getFirstRecordFromStandardSelectSQL("select mdpadmin from " TBL_PARAMSYSTEME,ok);
    if( !ok || mdpdata.size()==0 )
        StandardSQL("update " TBL_PARAMSYSTEME " set mdpadmin = '" NOM_MDPADMINISTRATEUR "'");
    else if (mdpdata.at(0) == "")
        StandardSQL("update " TBL_PARAMSYSTEME " set mdpadmin = '" NOM_MDPADMINISTRATEUR "'");
    return (mdpdata.at(0).toString() != ""? mdpdata.at(0).toString() : NOM_MDPADMINISTRATEUR);
}


/*
 * Actes
*/
QJsonObject DataBase::loadActeData(QVariantList actdata)
{
    QJsonObject data{};
    data[CP_IDACTE_ACTES] = actdata.at(0).toInt();
    data[CP_IDPAT_ACTES] = actdata.at(1).toInt();
    data[CP_IDUSER_ACTES] = actdata.at(2).toInt();
    data[CP_DATE_ACTES] = actdata.at(3).toDate().toString("yyyy-MM-dd");
    data[CP_MOTIF_ACTES] = actdata.at(4).toString();
    data[CP_TEXTE_ACTES] = actdata.at(5).toString();
    data[CP_CONCLUSION_ACTES] = actdata.at(6).toString();
    data[CP_COURRIERAFAIRE_ACTES] = actdata.at(7).toString();
    data[CP_COTATION_ACTES] = actdata.at(8).toString();
    data[CP_MONTANT_ACTES] = actdata.at(9).toDouble();
    data[CP_MONNAIE_ACTES] = actdata.at(10).toString();
    data[CP_IDUSERCREATEUR_ACTES] = actdata.at(11).toInt();
    data[CP_IDUSERCOMPTABLE_ACTES] = actdata.at(12).toInt();
    data[CP_IDUSERPARENT_ACTES] = actdata.at(13).toInt();
    if( actdata.at(14).isNull() )
        data["paiementType"] = "";
    else
        data["paiementType"] = actdata.at(14).toString();

    if( actdata.at(15).isNull() )
        data["paiementTiers"] = "";
    else
        data["paiementTiers"] = actdata.at(15).toString();
    data[CP_NUMCENTRE_ACTES] = actdata.at(16).toInt();
    data[CP_IDLIEU_ACTES] = actdata.at(17).toInt();
    data[CP_HEURE_ACTES] = actdata.at(18).toTime().toString("HH:mm:ss");
    data[CP_SUPERVISEURREMPLACANT_ACTES] = (actdata.at(19) == 1);

    return data;
}

Acte* DataBase::loadActeById(int idActe)
{
    Acte *acte = new Acte();
    QJsonObject data = loadActeAllData(idActe);
    if (data == QJsonObject{})
        return Q_NULLPTR;
    acte->setData(data);
    return acte;
}

QJsonObject DataBase::loadActeAllData(int idActe)
{
    QString req = "SELECT act." CP_IDACTE_ACTES ", act." CP_IDPAT_ACTES ", act." CP_IDUSER_ACTES ",  act." CP_DATE_ACTES ", act." CP_MOTIF_ACTES ","
                  " act." CP_TEXTE_ACTES ", act." CP_CONCLUSION_ACTES ", act." CP_COURRIERAFAIRE_ACTES ", act." CP_COTATION_ACTES ", act." CP_MONTANT_ACTES ","
                  " act." CP_MONNAIE_ACTES ", act." CP_IDUSERCREATEUR_ACTES ", act." CP_IDUSERCOMPTABLE_ACTES ", act." CP_IDUSERPARENT_ACTES ","
                  " tpm.TypePaiement, tpm.Tiers, act." CP_NUMCENTRE_ACTES ", " CP_IDLIEU_ACTES ", act." CP_HEURE_ACTES ","
                  " act." CP_SUPERVISEURREMPLACANT_ACTES
                  " FROM " TBL_ACTES " act "
                  " LEFT JOIN " TBL_TYPEPAIEMENTACTES " tpm on tpm.idActe = act.idActe "
                  " WHERE act." CP_IDACTE_ACTES " = '" + QString::number(idActe) + "'";
    QVariantList actdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if( !ok || actdata.size()==0 )
        return QJsonObject{};
    return loadActeData(actdata);
}

QList<Acte *> DataBase::loadActesByPat(Patient *pat)
{
    QList<Acte*> list;
    if( pat == Q_NULLPTR )
        return list;
    QString req = "SELECT act." CP_IDACTE_ACTES ", act." CP_IDPAT_ACTES ", act." CP_IDUSER_ACTES ",  act." CP_DATE_ACTES ", act." CP_MOTIF_ACTES ","
                  " act." CP_TEXTE_ACTES ", act." CP_CONCLUSION_ACTES ", act." CP_COURRIERAFAIRE_ACTES ", act." CP_COTATION_ACTES ", act." CP_MONTANT_ACTES ","
                  " act." CP_MONNAIE_ACTES ", act." CP_IDUSERCREATEUR_ACTES ", act." CP_IDUSERCOMPTABLE_ACTES ", act." CP_IDUSERPARENT_ACTES ","
                  " tpm.TypePaiement, tpm.Tiers, act." CP_NUMCENTRE_ACTES ", " CP_IDLIEU_ACTES ", act." CP_HEURE_ACTES ","
                  " act." CP_SUPERVISEURREMPLACANT_ACTES
                  " FROM " TBL_ACTES " act "
                  " LEFT JOIN " TBL_TYPEPAIEMENTACTES " tpm on tpm.idActe = act.idActe "
                  " WHERE act." CP_IDPAT_ACTES " = '" + QString::number(pat->id()) + "' "
                  " ORDER BY act." CP_IDACTE_ACTES " DESC";
    QList<QVariantList> actlist = StandardSelectSQL(req,ok);
    if(!ok || actlist.size()==0)
        return list;
    for (int i=0; i<actlist.size(); ++i)
    {
        QJsonObject data = loadActeData(actlist.at(i));
        Acte *acte = new Acte(data);
        if (acte != Q_NULLPTR)
            list << acte;
    }
    return list;
}

QList<Acte *> DataBase::loadIdActesByPat(Patient *pat)
{
    QList<Acte*> list;
    if( pat == Q_NULLPTR )
        return list;
    QString req = "SELECT idActe FROM " TBL_ACTES
                  " WHERE idPat = '" + QString::number(pat->id()) + "' "
                  " ORDER BY act.idActe DESC";
    QList<QVariantList> actlist = StandardSelectSQL(req,ok);
    if(!ok || actlist.size()==0)
        return list;
    for (int i=0; i<actlist.size(); ++i)
    {
        QJsonObject data{};
        data[CP_IDACTE_ACTES] = actlist.at(i).at(0).toInt();
        Acte *acte = new Acte(data);
        if (acte != Q_NULLPTR)
            list << acte;
    }
    return list;
}

double DataBase::getActePaye(int idActe)
{
    double montant = 0.0;
    // on récupère les lignes de paiement
    QString req = " SELECT lp.Paye, lr.Monnaie "
                  " FROM " TBL_LIGNESPAIEMENTS " lp "
                  " LEFT JOIN " TBL_RECETTES " lr on lr.idRecette = lp.idRecette "
                  " WHERE idActe = " + QString::number(idActe);
    QList<QVariantList> mtntlist = StandardSelectSQL(req,ok);
    if(!ok || mtntlist.size()==0)
        return montant;
    for (int i=0; i<mtntlist.size(); ++i)
    {
        if (mtntlist.at(i).at(1).toString() == "F")
            montant += (mtntlist.at(i).at(0).toDouble() / 6.55957);
        else
            montant += mtntlist.at(i).at(0).toDouble();
    }
    return montant;
}

/*
 * Refractions
*/

QJsonObject             DataBase::loadRefractionData(QVariantList refdata)           //! attribue la liste des datas à une refraction
{
    QJsonObject data{};
    data[CP_ID_REFRACTIONS]                 = refdata.at(0).toInt();
    data[CP_IDPAT_REFRACTIONS]              = refdata.at(1).toInt();
    data[CP_IDACTE_REFRACTIONS]             = refdata.at(2).toInt();
    data[CP_DATE_REFRACTIONS]               = refdata.at(3).toDate().toString("yyyy-MM-dd");
    data[CP_TYPEMESURE_REFRACTIONS]         = refdata.at(4).toString();
    data[CP_DISTANCEMESURE_REFRACTIONS]     = refdata.at(5).toString();
    data[CP_CYCLOPLEGIE_REFRACTIONS]        = (refdata.at(6).toInt() == 1);
    data[CP_ODMESURE_REFRACTIONS]           = (refdata.at(7).toInt() == 1);
    data[CP_SPHEREOD_REFRACTIONS]           = refdata.at(8).toDouble();
    data[CP_CYLINDREOD_REFRACTIONS]         = refdata.at(9).toDouble();
    data[CP_AXECYLOD_REFRACTIONS]           = refdata.at(10).toInt();
    data[CP_AVLOD_REFRACTIONS]              = refdata.at(11).toString();
    data[CP_ADDVPOD_REFRACTIONS]            = refdata.at(12).toDouble();
    data[CP_AVPOD_REFRACTIONS]              = refdata.at(13).toString();
    data[CP_PRISMEOD_REFRACTIONS]           = refdata.at(14).toDouble();
    data[CP_BASEPRISMEOD_REFRACTIONS]       = refdata.at(15).toInt();
    data[CP_BASEPRISMETEXTOD_REFRACTIONS]   = refdata.at(16).toString();
    data[CP_PRESSONOD_REFRACTIONS]          = (refdata.at(17).toInt() == 1);
    data[CP_DEPOLIOD_REFRACTIONS]           = (refdata.at(18).toInt() == 1);
    data[CP_PLANOD_REFRACTIONS]             = (refdata.at(19).toInt() == 1);
    data[CP_RYSEROD_REFRACTIONS]            = refdata.at(20).toInt();
    data[CP_FORMULEOD_REFRACTIONS]          = refdata.at(21).toString();
    data[CP_OGMESURE_REFRACTIONS]           = (refdata.at(22).toInt() == 1);
    data[CP_SPHEREOG_REFRACTIONS]           = refdata.at(23).toDouble();
    data[CP_CYLINDREOG_REFRACTIONS]         = refdata.at(24).toDouble();
    data[CP_AXECYLOG_REFRACTIONS]           = refdata.at(25).toInt();
    data[CP_AVLOG_REFRACTIONS]              = refdata.at(26).toString();
    data[CP_ADDVPOG_REFRACTIONS]            = refdata.at(27).toDouble();
    data[CP_AVPOG_REFRACTIONS]              = refdata.at(28).toString();
    data[CP_PRISMEOG_REFRACTIONS]           = refdata.at(29).toDouble();
    data[CP_BASEPRISMEOG_REFRACTIONS]       = refdata.at(30).toInt();
    data[CP_BASEPRISMETEXTOG_REFRACTIONS]   = refdata.at(31).toString();
    data[CP_PRESSONOG_REFRACTIONS]          = (refdata.at(32).toInt() == 1);
    data[CP_DEPOLIOG_REFRACTIONS]           = (refdata.at(33).toInt() == 1);
    data[CP_PLANOG_REFRACTIONS]             = (refdata.at(34).toInt() == 1);
    data[CP_RYSEROG_REFRACTIONS]            = refdata.at(35).toInt();
    data[CP_FORMULEOG_REFRACTIONS]          = refdata.at(36).toString();
    data[CP_COMMENTAIREORDO_REFRACTIONS]    = refdata.at(37).toString();
    data[CP_TYPEVERRES_REFRACTIONS]         = refdata.at(38).toString();
    data[CP_OEIL_REFRACTIONS]               = refdata.at(39).toString();
    data[CP_MONTURE_REFRACTIONS]            = refdata.at(40).toString();
    data[CP_VERRETEINTE_REFRACTIONS]        = (refdata.at(41).toInt() == 1);
    data[CP_PD_REFRACTIONS]                 = refdata.at(42).toInt();
    return data;
}

QList<Refraction*> DataBase::loadRefractionsByPatId(int id)                  //! charge toutes les refractions d'un patient
{
    QList<Refraction*> list = QList<Refraction*> ();
    QString req = "SELECT  idRefraction, idPat, idActe, DateRefraction, QuelleMesure, QuelleDistance, "           // 0-1-2-3-4-5
        " Cycloplegie, ODcoche, SphereOD, CylindreOD, AxeCylindreOD, AVLOD, "                   // 6-7-8-9-10-11
        " AddVPOD, AVPOD, PrismeOD, BasePrismeOD, BasePrismeTextOD, PressOnOD,"                 // 12-13-14-15-16-17
        " DepoliOD, PlanOD, RyserOD, FormuleOD, OGcoche, SphereOG, CylindreOG,"                 // 18-19-20-21-22-23-24
        " AxeCylindreOG, AVLOG, AddVPOG, AVPOG, PrismeOG, BasePrismeOG, "                       // 25-26-27-28-29-30
        " BasePrismeTextOG, PressOnOG, DepoliOG, PlanOG, RyserOG, FormuleOG, "                  // 31-32-34-35-36
        " CommentaireOrdoLunettes, QuelsVerres, QuelOeil, Monture, VerreTeinte, PD"             // 37-38-39-40-41-42
        " FROM " TBL_REFRACTIONS ;
    req += " WHERE  IdPat = " + QString::number(id) ;
    req += " order by idrefraction desc";
    QList<QVariantList> reflist = StandardSelectSQL(req,ok);
    if(!ok || reflist.size()==0)
        return list;
    for (int i=0; i<reflist.size(); ++i)
    {
        QJsonObject data = loadRefractionData(reflist.at(i));
        Refraction *ref = new Refraction(data);
        if (ref != Q_NULLPTR)
            list << ref;
    }
    return list;
}

Refraction* DataBase::loadRefractionById(int idref)                   //! charge une refraction définie par son id - utilisé pour renouveler les données en cas de modification
{
    Refraction *ref = Q_NULLPTR;
    QString req = "SELECT idRefraction, idPat, idActe, DateRefraction, QuelleMesure, QuelleDistance, "           // 0-1-2-3-4-5
        " Cycloplegie, ODcoche, SphereOD, CylindreOD, AxeCylindreOD, AVLOD, "                   // 6-7-8-9-10-11
        " AddVPOD, AVPOD, PrismeOD, BasePrismeOD, BasePrismeTextOD, PressOnOD,"                 // 12-13-14-15-16-17
        " DepoliOD, PlanOD, RyserOD, FormuleOD, OGcoche, SphereOG, CylindreOG,"                 // 18-19-20-21-22-23-24
        " AxeCylindreOG, AVLOG, AddVPOG, AVPOG, PrismeOG, BasePrismeOG, "                       // 25-26-27-28-29-30
        " BasePrismeTextOG, PressOnOG, DepoliOG, PlanOG, RyserOG, FormuleOG, "                  // 31-32-34-35-36
        " CommentaireOrdoLunettes, QuelsVerres, QuelOeil, Monture, VerreTeinte, PD"             // 37-38-39-40-41-42
        " FROM " TBL_REFRACTIONS ;
    req += " WHERE  IdRefraction = " + QString::number(idref) ;
    QVariantList refdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || refdata.size()==0)
        return ref;
    QJsonObject data = loadRefractionData(refdata);
    ref = new Refraction(data);
    return ref;
}
