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

void DataBase::init(QSettings const &setting, int mode)
{
    m_mode = mode;
    if( m_mode == Poste )
        m_server = "localhost";
    else
        m_server = setting.value(getBase() + "/Serveur").toString();

    m_port = setting.value(getBase() + "/Port").toInt();

    m_useSSL = (m_mode == Distant);
}

void DataBase::initFromFirstConnexion(QString mode, QString Server, int Port, bool SSL)
{
    if (mode == "BDD_POSTE") m_mode = Poste;
    else if (mode == "BDD_LOCAL") m_mode = ReseauLocal;
    else if (mode == "BDD_DISTANT") m_mode = Distant;

    m_server = Server;
    m_port = Port;
    m_useSSL = SSL;
}

int DataBase::getMode() const
{
    return m_mode;
}
QString DataBase::getBase() const
{
    return getBaseFromInt( m_mode );
}
QString DataBase::getBaseFromInt( int mode ) const
{
    if (mode == ReseauLocal)
        return "BDD_LOCAL";

    if (mode == Distant)
        return "BDD_DISTANT";

    return "BDD_POSTE"; //m_mode == Poste
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

void DataBase::unlocktables()
{
    StandardSQL("UNLOCK TABLES;");
}

bool DataBase::testconnexionbase() // une requete simple pour vérifier que la connexion à la base fontionne toujours
{
    QString req = "select AdresseTCPServeur from " NOM_TABLE_PARAMSYSTEME;
    return StandardSQL(req);
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
                           QHash<QString, QString> hash,
                           QString where,
                           QString errormsg)
{
    QString req = "update " + nomtable + " set";
    for (QHash<QString, QString>::const_iterator itset = hash.constBegin(); itset != hash.constEnd(); ++itset)
        req += " " + itset.key() + " = " + (itset.value().toLower()=="null"? "null," : "'" + Utils::correctquoteSQL(itset.value()) + "',");
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
    for (QHash<QString, QString>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
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
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
        champs  += itset.key() + ",";
        champs2  += ":" + itset.key() + ",";
    }
    champs = champs.left(champs.size()-1);
    champs2 = champs2.left(champs2.size()-1);
    QString prepare = "insert into " + nomtable + " (" + champs +  + ") values (" + champs2 + ")";
    query.prepare(prepare);
    for (QHash<QString, QVariant>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
    {
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
        QList<QVariantList> recordslist = db->StandardSelectSQL("Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient), ok);
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
     QVariantList recorddata = db->getFirstRecordFromStandardSelectSQL("Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient), ok);
     if (!ok)                                // erreur;
     if (recorddata.size()==0)                 // réponse vide
    */
    QList<QVariantList> listreponses = StandardSelectSQL(req , OK, errormsg);
    if(listreponses.size()>0)
        return listreponses.at(0);
    else
        return QVariantList();
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
                  " FROM " NOM_TABLE_UTILISATEURS " u "
                  " LEFT JOIN " NOM_TABLE_USERSCONNECTES " uc on uc.idUSer = u.idUser "
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
    m_userConnected->setData( loadUserData(m_userConnected->id()));
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
            " OPTAM, cpt.nomcompteabrege, cpt2.nomcompteabrege, cpt2.iduser as usercptdefaut"           // 30,31,32,33
            " from " NOM_TABLE_UTILISATEURS " usr "
            " left outer join " NOM_TABLE_COMPTES " cpt on usr.idcompteencaisshonoraires = cpt.idCompte"
            " left outer join " NOM_TABLE_COMPTES " cpt2 on usr.idCompteParDefaut = cpt2.idCompte"
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
    userData["idCompteParDefaut"]           = usrdata.at(12).toInt();
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
    userData["idCompteEncaissHonoraires"]   = usrdata.at(28).toInt();
    if( userData["idCompteEncaissHonoraires"].isNull() )
    {
        userData["idUserEncaissHonoraires"] = -1;
    }
    else
    {
        userData["idUserEncaissHonoraires"]    = idUser;
        userData["nomCompteEncaissHonoraires"] = usrdata.at(31).toString();
        userData["nomUserEncaissHonoraires"]   = usrdata.at(2).toString();
    }

    if( !usrdata.at(32).isNull() )
    {
        userData["nomCompteParDefaut"]  = usrdata.at(32).toString();
        userData["usercptdefaut"]       = usrdata.at(33).toInt();
    }
    return userData;
}

QJsonObject DataBase::loadAdminData()
{
    QJsonObject userData{};
    QString req = "select iduser from " NOM_TABLE_UTILISATEURS " where userlogin = '" NOM_ADMINISTRATEURDOCS "'";
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
                  " from " NOM_TABLE_UTILISATEURS " usr "
                  " left outer join " NOM_TABLE_COMPTES " cpt on usr.idcompteencaisshonoraires = cpt.idCompte "
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
 * Correspondants
*/
QList<Correspondant*> DataBase::loadCorrespondants()                             // tous les correspondants sans exception
{
    QList<Correspondant*> correspondants;
    QString req = "SELECT idCor, CorNom, CorPrenom, CorSexe, cormedecin, corspecialite FROM " NOM_TABLE_CORRESPONDANTS " order by cornom, corprenom";

    QList<QVariantList> corlist = StandardSelectSQL(req,ok);
    if(!ok || corlist.size()==0)
        return correspondants;
    for (int i=0; i<corlist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"]             = corlist.at(i).at(0).toInt();
        jData["nom"]            = corlist.at(i).at(1).toString();
        jData["prenom"]         = corlist.at(i).at(2).toString();
        jData["sexe"]           = corlist.at(i).at(3).toString();
        jData["medecin"]        = (corlist.at(i).at(4).toInt()==1);
        jData["generaliste"]    = (corlist.at(i).at(5).toInt() > 0);
        jData["isAllLoaded"]    = false;
        Correspondant *cor = new Correspondant(jData);
        correspondants << cor;
    }
    return correspondants;
}

QList<Correspondant*> DataBase::loadCorrespondantsALL()                             // tous les correspondants sans exception avec plus de renseignements
{
    QList<Correspondant*> correspondants;
    QString req = "SELECT idCor, CorNom, CorPrenom, nomspecialite as metier, CorAdresse1,"
                        " CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone,"
                        " CorSexe, cormedecin, corspecialite"
                        " FROM " NOM_TABLE_CORRESPONDANTS ", " NOM_TABLE_SPECIALITES
                        " where cormedecin = 1 and corspecialite = idspecialite"
                  " union "
                  "SELECT idCor, CorNom, CorPrenom, corautreprofession as metier, CorAdresse1,"
                        " CorAdresse2, CorAdresse3, CorCodepostal, CorVille, CorTelephone,"
                        " CorSexe, cormedecin, -1 as corspecialite"
                        " FROM " NOM_TABLE_CORRESPONDANTS
                        " where cormedecin <> 1 or cormedecin is null"
                  " order by metier, cornom, corprenom";
    QList<QVariantList> corlist = StandardSelectSQL(req,ok);
    if(!ok || corlist.size()==0)
        return correspondants;
    for (int i=0; i<corlist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"]             = corlist.at(i).at(0).toInt();
        jData["nom"]            = corlist.at(i).at(1).toString();
        jData["prenom"]         = corlist.at(i).at(2).toString();
        jData["metier"]         = corlist.at(i).at(3).toString();
        jData["adresse1"]       = corlist.at(i).at(4).toString();
        jData["adresse2"]       = corlist.at(i).at(5).toString();
        jData["adresse3"]       = corlist.at(i).at(6).toString();
        jData["codepostal"]     = corlist.at(i).at(7).toString();
        jData["ville"]          = corlist.at(i).at(8).toString();
        jData["telephone"]      = corlist.at(i).at(9).toString();
        jData["sexe"]           = corlist.at(i).at(10).toString();
        jData["medecin"]        = (corlist.at(i).at(11).toInt() == 1);
        jData["generaliste"]    = (corlist.at(i).at(12).toInt() == 0);
        jData["specialite"]     = corlist.at(i).at(12).toInt();
        jData["isAllLoaded"]    = false;
        Correspondant *cor = new Correspondant(jData);
        correspondants << cor;
    }
    return correspondants;
}

QJsonObject DataBase::loadCorrespondantData(int idcor)                             // toutes les données d'un correspondant
{
    QJsonObject jData{};
    QString req = "SELECT CorNom, CorPrenom, nomspecialite as metier, CorAdresse1, CorAdresse2,"
                        " CorAdresse3, CorCodepostal, CorVille, CorTelephone, CorSexe,"
                        " cormedecin, CorPortable, CorFax, CorMail, CorSpecialite"
                        " FROM " NOM_TABLE_CORRESPONDANTS ", " NOM_TABLE_SPECIALITES
                        " where cormedecin = 1 and corspecialite = idspecialite"
                        " and idcor = " + QString::number(idcor) +
                  " union "
                  "SELECT CorNom, CorPrenom, CorAutreProfession as metier, CorAdresse1, CorAdresse2,"
                        " CorAdresse3, CorCodepostal, CorVille, CorTelephone, CorSexe,"
                        " cormedecin, CorPortable, CorFax, CorMail, -1 as CorSpecialite"
                        " FROM " NOM_TABLE_CORRESPONDANTS
                        " where cormedecin <> 1 or cormedecin is null"
                        " and idcor = " + QString::number(idcor);
    QVariantList cordata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données de l'utilisateur"));
    if (!ok)
        return jData;

    if(cordata.size()==0)
        return jData;

    for (int i=0; i<cordata.size(); ++i)
    {
        jData["id"]         = idcor;
        jData["nom"]        = cordata.at(0).toString();
        jData["prenom"]     = cordata.at(1).toString();
        jData["metier"]     = cordata.at(2).toString();
        jData["adresse1"]   = cordata.at(3).toString();
        jData["adresse2"]   = cordata.at(4).toString();
        jData["adresse3"]   = cordata.at(5).toString();
        jData["codepostal"] = cordata.at(6).toString();
        jData["ville"]      = cordata.at(7).toString();
        jData["telephone"]  = cordata.at(8).toString();
        jData["sexe"]       = cordata.at(9).toString();
        jData["medecin"]    = (cordata.at(10).toInt()==1);
        jData["portable"]   = cordata.at(11).toString();
        jData["fax"]        = cordata.at(12).toString();
        jData["mail"]       = cordata.at(13).toString();
        jData["specialite"] = cordata.at(14).toInt();
        jData["isAllLoaded"]= true;
    }
    return jData;
}

void DataBase::SupprCorrespondant(int idcor)
{
    QString id = QString::number(idcor);
    StandardSQL("delete from " NOM_TABLE_CORRESPONDANTS " where idcor = " + id);
    StandardSQL("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg  = null where idcormedmg  = " + id);
    StandardSQL("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe1 = null where idcormedspe1 = " + id);
    StandardSQL("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe2 = null where idcormedspe2 = " + id);
    StandardSQL("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe3 = null where idcormedspe3 = " + id);
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
                  " compression, lienversfichier, formatdoc, Importance from " NOM_TABLE_IMPRESSIONS
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
        jData["id"] = doclist.at(i).at(0).toInt();
        jData["idpat"] = pat->id();
        jData["typedoc"] = doclist.at(i).at(1).toString();
        jData["soustypedoc"] = doclist.at(i).at(2).toString();
        jData["titre"] = doclist.at(i).at(3).toString();
        jData["dateimpression"] = QDateTime(doclist.at(i).at(4).toDate(), doclist.at(i).at(4).toTime()).toMSecsSinceEpoch();
        jData["compression"] = doclist.at(i).at(5).toInt();
        jData["lienversfichier"] = doclist.at(i).at(6).toString();
        jData["formatdoc"] = doclist.at(i).at(7).toString();
        jData["importance"] = doclist.at(i).at(8).toInt();
        DocExterne *doc = new DocExterne(jData);
        docsexternes << doc;
    }
    return docsexternes;
}

QJsonObject DataBase::loadDocExterneData(int idDoc)
{
    QJsonObject docexterneData{};
    QString req = "Select idImpression, idUser, idPat, TypeDoc, SousTypeDoc,"
                  " Titre, TextEntete, TextCorps, TextOrigine, TextPied,"
                  " Dateimpression, compression, lienversfichier, ALD, UserEmetteur,"
                  " formatdoc, Importance from " NOM_TABLE_IMPRESSIONS
                  " where idimpression = " + QString::number(idDoc);
    QVariantList docdata = getFirstRecordFromStandardSelectSQL(req, ok);
    if (!ok || docdata.size()==0)
        return docexterneData;
    docexterneData["isallloaded"] = true;

    docexterneData["id"] = docdata.at(0).toInt();
    docexterneData["iduser"] = docdata.at(1).toInt();
    docexterneData["idpat"] = docdata.at(2).toInt();
    docexterneData["typedoc"] = docdata.at(3).toString();
    docexterneData["soustypedoc"] = docdata.at(4).toString();

    docexterneData["titre"] = docdata.at(5).toString();
    docexterneData["textentete"] = docdata.at(6).toString();
    docexterneData["textcorps"] = docdata.at(7).toString();
    docexterneData["textorigine"] = docdata.at(8).toString();
    docexterneData["textpied"] = docdata.at(9).toString();

    docexterneData["dateimpression"] = QDateTime(docdata.at(10).toDate(), docdata.at(10).toTime()).toMSecsSinceEpoch();
    docexterneData["compression"] = docdata.at(11).toInt();
    docexterneData["lienversfichier"] = docdata.at(12).toString();
    docexterneData["ALD"] = (docdata.at(13).toInt()==1);
    docexterneData["useremetteur"] = docdata.at(14).toString();

    docexterneData["formatdoc"] = docdata.at(15).toString();
    docexterneData["importance"] = docdata.at(16).toInt();

    return docexterneData;
}

void DataBase::SupprDocExterne(DocExterne* doc)
{
    SupprRecordFromTable(doc->id(), "idImpression", NOM_TABLE_IMPRESSIONS);
}


/*
 * Documents
*/
QList<Document*> DataBase::loadDocuments()
{
    QList<Document*> documents;
    QString req = "Select idDocument, TextDocument, ResumeDocument, ConclusionDocument, idUser,"
                  " DocPublic, Prescription, Editable, Medical from " NOM_TABLE_COURRIERS
                  " WHERE (idUser = " + QString::number(getUserConnected()->id()) + " Or (DocPublic = 1 and iduser <> " + QString::number(getUserConnected()->id()) + "))"
                  " ORDER BY ResumeDocument";
    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return documents;
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
        Document *doc = new Document(jData);
        documents << doc;
    }
    return documents;
}

void DataBase::SupprDocument(Document* doc)
{
    SupprRecordFromTable(doc->id(), "idDocument", NOM_TABLE_COURRIERS);
}


/*
 * MetaDocuments
*/
QList<MetaDocument*> DataBase::loadMetaDocuments()
{
    QList<MetaDocument*> metadocuments;
    QString     req =  "SELECT ResumeMetaDocument, idMetaDocument, idUser, Public, TextMetaDocument"
                       " FROM "  NOM_TABLE_METADOCUMENTS
                       " WHERE idUser = " + QString::number(getUserConnected()->id());
                req += " UNION \n";
                req += "select ResumeMetaDocument, idMetaDocument, idUser, Public, TextMetaDocument from " NOM_TABLE_METADOCUMENTS
                       " where idMetaDocument not in\n"
                       " (select met.idMetaDocument from " NOM_TABLE_METADOCUMENTS " as met, "
                       NOM_TABLE_JOINTURESDOCS " as joi, "
                       NOM_TABLE_COURRIERS " as doc\n"
                       " where joi.idmetadocument = met.idMetaDocument\n"
                       " and joi.idDocument = doc.iddocument\n"
                       " and doc.docpublic is null)\n";
                req += " ORDER BY ResumeMetaDocument;";
    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return metadocuments;
    for (int i=0; i<doclist.size(); ++i)
    {
        QJsonObject jData{};
        jData["idmetadocument"] = doclist.at(i).at(1).toInt();
        jData["texte"] = doclist.at(i).at(4).toString();
        jData["resume"] = doclist.at(i).at(0).toString();
        jData["iduser"] = doclist.at(i).at(2).toInt();
        jData["public"] = (doclist.at(i).at(3).toInt()==1);
        MetaDocument *metadoc = new MetaDocument(jData);
        metadocuments << metadoc;
    }
    return metadocuments;
}

void DataBase::SupprMetaDocument(Document* doc)
{
    SupprRecordFromTable(doc->id(), "idDocument", NOM_TABLE_METADOCUMENTS);
}




/*******************************************************************************************************************************************************************
 ********* COMPTABILITÊ ********************************************************************************************************************************************
********************************************************************************************************************************************************************/
/*
 * Comptes
*/
QList<Compte*> DataBase::loadComptesByUser(int idUser)
{
    int idcptprefer=-1;
    QString req =
            " select idcomptepardefaut from " NOM_TABLE_UTILISATEURS
            " where iduser = " + QString::number(idUser);
    QVariantList cptdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(ok && cptdata.size()>0 )
        idcptprefer= cptdata.at(0).toInt();

    QList<Compte*> comptes;
    req = "SELECT idCompte, cmpt.idBanque, idUser, IBAN, intitulecompte, NomCompteAbrege, SoldeSurDernierReleve, partage, desactive, NomBanque "
          " FROM " NOM_TABLE_COMPTES " as cmpt "
          " left outer join " NOM_TABLE_BANQUES " as bank on cmpt.idbanque = bank.idbanque "
          " WHERE idUser = " + QString::number(idUser);
    QList<QVariantList> cptlist = StandardSelectSQL(req,ok);
    if(!ok || cptlist.size()==0)
        return comptes;
    for (int i=0; i<cptlist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"] = cptlist.at(i).at(0).toInt();
        jData["idbanque"] = cptlist.at(i).at(1).toInt();
        jData["iduser"] = cptlist.at(i).at(2).toInt();
        jData["IBAN"] = cptlist.at(i).at(3).toString();
        jData["IntituleCompte"] = cptlist.at(i).at(4).toString();
        jData["nom"] = cptlist.at(i).at(5).toString();
        jData["solde"] = cptlist.at(i).at(6).toDouble();
        jData["partage"] = (cptlist.at(i).at(7).toInt() == 1);
        jData["desactive"] = (cptlist.at(i).at(8).toInt() == 1);
        jData["NomBanque"] = cptlist.at(i).at(9).toString();
        jData["prefere"] = (cptlist.at(i).at(0).toInt() == idcptprefer);
        Compte *cpt = new Compte(jData);
        comptes << cpt;
    }

    return comptes;
}

int DataBase::getIdMaxTableComptesTableArchives()
{
    int a(0), b(0);
    a = selectMaxFromTable("idligne", NOM_TABLE_ARCHIVESBANQUE, ok);
    b = selectMaxFromTable("idligne", NOM_TABLE_LIGNESCOMPTES, ok);
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
                        " FROM " NOM_TABLE_DEPENSES " dep"
                        " left join " NOM_TABLE_FACTURES " fac on dep.idFacture = fac.idFacture"
                        " left join " NOM_TABLE_RUBRIQUES2035 " rub on dep.RefFiscale = rub.RefFiscale"
                        " WHERE dep.idUser = " + QString::number(idUser);
    QList<QVariantList> deplist = StandardSelectSQL(req,ok);
    if(!ok || deplist.size()==0)
        return depenses;
    for (int i=0; i<deplist.size(); ++i)
    {
        QJsonObject jData{};
        jData["iddepense"]      = deplist.at(i).at(0).toInt();
        jData["iduser"]         = idUser;
        jData["date"]           = deplist.at(i).at(1).toDate().toString("yyyy-MM-dd");
        jData["reffiscale"]     = deplist.at(i).at(2).toString();
        jData["objet"]          = deplist.at(i).at(3).toString();
        jData["montant"]        = deplist.at(i).at(4).toDouble();
        jData["famfiscale"]     = deplist.at(i).at(5).toString();
        jData["monnaie"]        = deplist.at(i).at(6).toString();
        jData["idrecette"]      = deplist.at(i).at(7).toInt();
        jData["modepaiement"]   = deplist.at(i).at(8).toString();
        jData["compte"]         = deplist.at(i).at(9).toInt();
        jData["nocheque"]       = deplist.at(i).at(10).toInt();
        jData["idfacture"]      = deplist.at(i).at(11).toInt();
        jData["lienfacture"]    = deplist.at(i).at(12).toString();
        jData["echeancier"]     = (deplist.at(i).at(13).toInt()==1);
        jData["objetecheancier"]= deplist.at(i).at(14).toString();
        jData["objetecheancier"]= deplist.at(i).at(14).toString();
        jData["idrubrique"]     = deplist.at(i).at(15).toInt();
        Depense *dep = new Depense(jData);
        depenses << dep;
    }

    return depenses;
}

void DataBase::loadDepenseArchivee(Depense *dep)
{
    bool archivee = false;
    QString req = "select idLigne from " NOM_TABLE_ARCHIVESBANQUE " where idDep = " + QString::number(dep->id());
    QVariantList arcdata = getFirstRecordFromStandardSelectSQL(req,ok);
    archivee = ok && arcdata.size() > 0;
    if (!archivee)  // pour les anciens enregistrements qui étaient archivés sans l'id...
    {
        req = "select idligne from " NOM_TABLE_ARCHIVESBANQUE
              " where LigneDate = '" + dep->date().toString("yyyy-MM-dd") +
              "' and LigneLibelle = '" + Utils::correctquoteSQL(dep->objet()) +
              "' and LigneMontant = " + QString::number(dep->montant());
        arcdata = getFirstRecordFromStandardSelectSQL(req,ok);
        archivee = ok && arcdata.size() > 0;
    }
    dep->setArchivee(archivee);
}

QStringList DataBase::ListeRubriquesFiscales()
{
    QStringList ListeRubriques;
    QString req = "SELECT reffiscale from " NOM_TABLE_RUBRIQUES2035 " where FamFiscale is not null and famfiscale <> 'Prélèvement personnel' order by reffiscale";
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
    QString req = "select idDep from " NOM_TABLE_DEPENSES " where DateDep " + op + "'" + date.toString("yyyy-MM-dd") +
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
                  " LigneDebitCredit, LigneTypeoperation, LigneDateConsolidation, idArchive from " NOM_TABLE_ARCHIVESBANQUE
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
    QString req = "SELECT idBanque, idBanqueAbrege, NomBanque, CodeBanque FROM " NOM_TABLE_BANQUES;
    QList<QVariantList> banqlist = StandardSelectSQL(req,ok);
    if(!ok || banqlist.size()==0)
        return banques;
    for (int i=0; i<banqlist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"] = banqlist.at(i).at(0).toInt();
        jData["idbanqueabrege"] = banqlist.at(i).at(1).toString();
        jData["nombanque"] = banqlist.at(i).at(2).toString();
        jData["codebanque"] = banqlist.at(i).at(3).toInt();
        Banque *bq = new Banque(jData);
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
    QString req = "SELECT idtIERS, Nomtiers, AdresseTiers, Codepostaltiers, Villetiers, Telephonetiers, FaxTiers from " NOM_TABLE_TIERS;
    QList<QVariantList> tierslist = StandardSelectSQL(req,ok);
    if(!ok || tierslist.size()==0)
        return listetiers;
    for (int i=0; i<tierslist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"] = tierslist.at(i).at(0).toInt();
        jData["nomtiers"] = tierslist.at(i).at(1).toInt();
        jData["adressetiers"] = tierslist.at(i).at(2).toString();
        jData["codepostaltiers"] = tierslist.at(i).at(3).toString();
        jData["villetiers"] = tierslist.at(i).at(4).toString();
        jData["telephonetiers"] = tierslist.at(i).at(5).toString();
        jData["faxtiers"] = tierslist.at(i).at(5).toString();
        Tiers *tiers = new Tiers(jData);
        listetiers << tiers;
    }

    return listetiers;
}

QList<TypeTiers*> DataBase::loadTypesTiers()
{
    QList<TypeTiers*> types;
    QString req = "SELECT Tiers FROM " NOM_TABLE_LISTETIERS;
    QList<QVariantList> tierslist = StandardSelectSQL(req,ok);
    if(!ok || tierslist.size()==0)
        return types;
    for (int i=0; i<tierslist.size(); ++i)
    {
        QJsonObject jData{};
        jData["typetiers"] = tierslist.at(i).at(0).toString();
        TypeTiers *type = new TypeTiers(jData);
        types << type;
    }
    return types;
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
          " FROM " NOM_TABLE_COTATIONS " cot left join " NOM_TABLE_CCAM " cc on cot.typeacte= cc.codeccam"
          " where idUser = " + QString::number(iduser) + " and typeacte in (select codeccam from " NOM_TABLE_CCAM ")"
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
        cotations << cotation;
    }
    req = " SELECT idcotation, Typeacte, MontantOPTAM, MontantNonOPTAM, MontantPratique, CCAM, Frequence, tip"
          " FROM "  NOM_TABLE_COTATIONS
          " where idUser = " + QString::number(iduser) +
          " and typeacte not in (select codeccam from  " NOM_TABLE_CCAM ")"
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
        cotations << cotation;
    }
    return cotations;
}

QStringList DataBase::loadTypesCotations()
{
    QStringList listcotations;
    QString req = "select distinct typeacte as code from " NOM_TABLE_COTATIONS
                  " union "
                  " select codeccam as code from " NOM_TABLE_CCAM
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
    QString  req = "SELECT idMotifsRDV, Motif, Raccourci, Couleur, Duree, ParDefaut, Utiliser, NoOrdre FROM "  NOM_TABLE_MOTIFSRDV " ORDER BY NoOrdre";
    QList<QVariantList> mtflist = StandardSelectSQL(req,ok);
    if(!ok || mtflist.size()==0)
        return motifs;
    for (int i=0; i<mtflist.size(); ++i)
    {
        QJsonObject jmotif{};
        jmotif["id"] = mtflist.at(i).at(0).toInt();
        jmotif["motif"] = mtflist.at(i).at(1).toString();
        jmotif["raccourci"] = mtflist.at(i).at(2).toString();
        jmotif["couleur"] = mtflist.at(i).at(3).toString();
        jmotif["duree"] = mtflist.at(i).at(4).toInt();
        jmotif["pardefaut"] = (mtflist.at(i).at(5).toInt()==1);
        jmotif["utiliser"] = (mtflist.at(i).at(6).toInt()==1);
        jmotif["noordre"] = mtflist.at(i).at(7).toInt();
        Motif *motif = new Motif(jmotif);
        motifs << motif;
    }
    return motifs;
}

/*
 * Sites
*/
QList<Site*> DataBase::loadSitesAll()
{
    QString req = "select idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, "
                    "LieuCodePostal, LieuVille, LieuTelephone, LieuFax "
                  "from " NOM_TABLE_LIEUXEXERCICE;
    return loadSites( req );
}
QList<Site*> DataBase::loadSitesByUser(int idUser)
{
    QString req = "select joint.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, "
                  "LieuCodePostal, LieuVille, LieuTelephone, LieuFax "
                  "from " NOM_TABLE_JOINTURESLIEUX " joint "
                  "left join " NOM_TABLE_LIEUXEXERCICE " lix on joint.idlieu = lix.idLieu "
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
                  "from " NOM_TABLE_VILLES;
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
        villes << ville;
    }
    return villes;
}


/*
 * Gestion des Patients
*/
void DataBase::loadSocialDataPatient(Patient* patient, bool &ok)
{
    QString req = "SELECT PatAdresse1, PatAdresse2, PatAdresse3, PatCodePostal, PatVille,"
                  " PatTelephone, PatPortable, PatMail, PatNNI, PatALD,"
                  " PatCMU, PatProfession FROM " NOM_TABLE_DONNEESSOCIALESPATIENTS
                  " WHERE idPat = " + QString::number(patient->id());

    QVariantList patlist = getFirstRecordFromStandardSelectSQL(req, ok);
    if(!ok || patlist.size()==0)
        return;
    QJsonObject jData{};
    jData["adresse1"]       = patlist.at(0).toString();
    jData["adresse2"]       = patlist.at(1).toString();
    jData["adresse3"]       = patlist.at(2).toString();
    jData["codepostal"]     = patlist.at(3).toString();
    jData["ville"]          = patlist.at(4).toString();
    jData["telephone"]      = patlist.at(5).toString();
    jData["portable"]       = patlist.at(6).toString();
    jData["mail"]           = patlist.at(7).toString();
    jData["NNI"]            = patlist.at(4).toLongLong();
    jData["ALD"]            = (patlist.at(9).toInt() == 1);
    jData["CMU"]            = (patlist.at(10).toInt() == 1);
    jData["profession"]     = patlist.at(11).toString();
    jData["isSocialLoaded"] = true;
    patient->addSocialData(jData);
}

void DataBase::loadMedicalDataPatient(Patient* patient, bool &ok)
{
    QString req = "select idCorMedMG, idCorMedSpe1, idCorMedSpe2, idCorMedSpe3, idCorNonMed,"
                  " RMPAtcdtsPersos, RMPTtGeneral, RMPAtcdtsFamiliaux, RMPAtcdtsOphs, Tabac,"
                  " Autrestoxiques, Gencorresp, Important, Resume, RMPTtOphs FROM " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS
                  " WHERE idPat = " + QString::number(patient->id());

    QVariantList patlist = getFirstRecordFromStandardSelectSQL(req, ok);
    if(!ok || patlist.size()==0)
        return;
    QJsonObject jData{};
    jData["idMG"]               = patlist.at(0).toInt();
    jData["idSpe1"]             = patlist.at(1).toInt();
    jData["idSpe2"]             = patlist.at(2).toInt();
    jData["idSpe3"]             = patlist.at(3).toInt();
    jData["idCornonMG"]         = patlist.at(4).toInt();
    jData["AtcdtsPerso"]        = patlist.at(5).toString();
    jData["TtGeneral"]          = patlist.at(6).toString();
    jData["AtcdtsFamiliaux"]    = patlist.at(7).toString();
    jData["AtcdstOph"]          = patlist.at(8).toString();
    jData["Tabac"]              = patlist.at(9).toString();
    jData["Toxiques"]           = patlist.at(10).toString();
    jData["GenCorresp"]         = patlist.at(11).toString();
    jData["Important"]          = patlist.at(12).toString();
    jData["Resume"]             = patlist.at(13).toString();
    jData["TtOph"]              = patlist.at(14).toString();
    jData["isMedicalLoaded"]    = true;
    patient->addMedicalData(jData);
}

Patient* DataBase::loadPatientById(int idPat, bool all)
{
    Patient *patient = new Patient();
    QString req = "SELECT idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " NOM_TABLE_PATIENTS " where idPat = " + QString::number(idPat);
    QVariantList patdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if( !ok || patdata.size()==0 )
        return Q_NULLPTR;
    QJsonObject jData{};
    jData["id"] = patdata.at(0).toInt();
    jData["nom"] = patdata.at(1).toString();
    jData["prenom"] = patdata.at(2).toString();
    jData["sexe"] = patdata.at(4).toString();
    jData["dateDeNaissance"] = patdata.at(3).toDate().toString("yyyy-MM-dd");
    jData["datecreation"] = patdata.at(5).toDate().toString("yyyy-MM-dd");
    jData["idcreateur"] = patdata.at(6).toInt();
    jData["isMedicalLoaded"] = all;
    jData["isSocialLoaded"] = all;
    patient->setData(jData);
    if (all)
    {
        bool ok;
        loadSocialDataPatient(patient, ok);
        loadMedicalDataPatient(patient, ok);
    }
    return patient;
}

QList<Patient*>* DataBase::loadPatientsAll(QString nom, QString prenom, bool filtre)
{
    QList<Patient*> *listpatients = new QList<Patient*>();
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
    if (m_mode == Distant)
        clauselimit = " limit 1000";
    QString req = "SELECT idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " NOM_TABLE_PATIENTS +
                  clausewhere +
                  clauselimit;
    //qDebug() << req;
    QList<QVariantList> patlist = StandardSelectSQL(req,ok);
    if( !ok || patlist.size()==0 )
        return listpatients;
    for (int i=0; i<patlist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"] = patlist.at(i).at(0).toInt();
        jData["nom"] = patlist.at(i).at(1).toString();
        jData["prenom"] = patlist.at(i).at(2).toString();
        jData["sexe"] = patlist.at(i).at(4).toString();
        jData["dateDeNaissance"] = patlist.at(i).at(3).toDate().toString("yyyy-MM-dd");
        jData["datecreation"] = patlist.at(i).at(5).toDate().toString("yyyy-MM-dd");
        jData["idcreateur"] = patlist.at(i).at(6).toInt();
        jData["isMedicalLoaded"] = false;
        jData["isSocialLoaded"] = false;
        Patient *patient = new Patient(jData);
        listpatients->append(patient);
    }
    return listpatients;
}

QList<Patient*>* DataBase::loadPatientsByDDN(QDate DDN)
{
    QList<Patient*> *listpatients = new QList<Patient*>();
    QString req = "SELECT idPat, PatNom, PatPrenom, PatDDN, Sexe, PatCreele, PatCreePar FROM " NOM_TABLE_PATIENTS
                  " WHERE PatDDN = '" + DDN.toString("yyyy-MM-dd") + "'";
    //qDebug() << req;
    QList<QVariantList> patlist = StandardSelectSQL(req,ok);
    if( !ok || patlist.size()==0 )
        return listpatients;
    for (int i=0; i<patlist.size(); ++i)
    {
        QJsonObject jData{};
        jData["id"] = patlist.at(i).at(0).toInt();
        jData["nom"] = patlist.at(i).at(1).toString();
        jData["prenom"] = patlist.at(i).at(2).toString();
        jData["sexe"] = patlist.at(i).at(4).toString();
        jData["dateDeNaissance"] = patlist.at(i).at(3).toDate().toString("yyyy-MM-dd");
        jData["datecreation"] = patlist.at(i).at(5).toDate().toString("yyyy-MM-dd");
        jData["idcreateur"] = patlist.at(i).at(6).toInt();
        jData["isMedicalLoaded"] = false;
        jData["isSocialLoaded"] = false;
        Patient *patient = new Patient(jData);
        listpatients->append(patient);
    }
    return listpatients;
}

Patient* DataBase::CreationPatient(QString nom, QString prenom, QDate datedenaissance, QString sexe)
{
    bool ok;
    QString req;
    locktables(QStringList() << NOM_TABLE_PATIENTS << NOM_TABLE_DONNEESSOCIALESPATIENTS << NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS);
    req =   "INSERT INTO " NOM_TABLE_PATIENTS
            " (PatNom, PatPrenom, PatDDN, PatCreele, PatCreePar, Sexe) "
            " VALUES ('" +
            Utils::correctquoteSQL(nom) + "', '" +
            Utils::correctquoteSQL(prenom) + "', '" +
            datedenaissance.toString("yyyy-MM-dd") +
            "', NOW(), '" +
            QString::number(getUserConnected()->id()) +"' , '" +
            sexe +
            "');";
    if (!StandardSQL(req, tr("Impossible de créer le dossier")))
        return Q_NULLPTR;

    // Récupération de l'idPatient créé ------------------------------------
    QString recuprequete =  "SELECT  idPat, PatNom, PatPrenom FROM " NOM_TABLE_PATIENTS
            " WHERE PatNom = '" + Utils::correctquoteSQL(nom) +
            "' AND PatPrenom = '" + Utils::correctquoteSQL(prenom) +
            "' AND PatDDN = '" + datedenaissance.toString("yyyy-MM-dd") + "'";
    QVariantList patdata = getFirstRecordFromStandardSelectSQL(recuprequete, ok, tr("Impossible de sélectionner les enregistrements"));
    if (!ok ||  patdata.size() == 0)
        return Q_NULLPTR;
    Patient *pat = loadPatientById(patdata.at(0).toInt());
    req = "INSERT INTO " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    StandardSQL(req,tr("Impossible de créer les données sociales"));
    req = "INSERT INTO " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " (idPat) VALUES ('" + QString::number(pat->id()) + "')";
    StandardSQL(req,tr("Impossible de créer les renseignements médicaux"));
    unlocktables();
    return pat;
}

void DataBase::UpdateCorrespondant(Patient *pat, typecorrespondant type, Correspondant *cor)
{
    QString id = (cor != Q_NULLPTR ? QString::number(cor->id()) : "null");
    QString field;
    switch (type) {
    case MG:
        field = "idCorMedMG";
        break;
    case Spe1:
        field = "idCorMedSpe1";
        break;
    case Spe2:
        field = "idCorMedSpe2";
    }
    StandardSQL("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set " + field + " = " + id +
                " where idpat = " + QString::number(pat->id()));
    if (cor != Q_NULLPTR && type == MG)
        pat->setmg(cor->id());
}


/*
 * MDP
*/
//Pas normal, les mots de passes doivent etre chiffrés
QString DataBase::getMDPAdmin()
{
    QString mdp ("");
    QVariantList mdpdata = getFirstRecordFromStandardSelectSQL("select mdpadmin from " NOM_TABLE_PARAMSYSTEME,ok);
    if( !ok || mdpdata.size()==0 )
        StandardSQL("update " NOM_TABLE_PARAMSYSTEME " set mdpadmin = '" NOM_MDPADMINISTRATEUR "'");
    else if (mdpdata.at(0) == "")
        StandardSQL("update " NOM_TABLE_PARAMSYSTEME " set mdpadmin = '" NOM_MDPADMINISTRATEUR "'");
    return (mdpdata.at(0).toString() != ""? mdpdata.at(0).toString() : NOM_MDPADMINISTRATEUR);
}


/*
 * Actes
*/
QString DataBase::loadActeRequest(int idActe, int idPat)
{
    QString subRequestRankAct = "SELECT idActe, idPat, "
                                  " CASE WHEN @prevRank = idPat THEN @curRank := @curRank + 1 WHEN @prevRank := idPat THEN @curRank := 1 END AS rank "
                                " FROM " NOM_TABLE_ACTES ", (SELECT @curRank := 0, @prevRank := NULL) r "
                                " ORDER BY idPat, idActe ";
    QString subRequestMinMaxAct = "SELECT min(idActe) as idActeMin, max(idActe) as idActeMax, count(idActe) as total, idPat "
                                " FROM " NOM_TABLE_ACTES
                                " GROUP BY idPat ";
    QString requete = "SELECT act.idActe, act.idPat, act.idUser, "
                        " act.ActeDate, act.ActeMotif, act.ActeTexte, act.ActeConclusion, "
                        " act.ActeCourrierAFaire, act.ActeCotation, act.ActeMontant, act.ActeMonnaie, "
                        " act.CreePar, act.UserComptable, act.UserParent, "
                        " pat.PatDDN, ll2.rank, ll.idActeMin, ll.idActeMax, ll.total, "
                        " tpm.TypePaiement, tpm.Tiers "
                      " FROM " NOM_TABLE_ACTES " act "
                      " LEFT JOIN " NOM_TABLE_PATIENTS " pat on pat.idPat = act.idPat "
                      " JOIN ( "+ subRequestMinMaxAct + " ) ll on ll.idPat = act.idPat "
                      " JOIN ( "+ subRequestRankAct + " ) ll2 on ll2.idPat = act.idPat and ll2.idActe = act.idActe "
                      " LEFT JOIN " NOM_TABLE_TYPEPAIEMENTACTES " tpm on tpm.idActe = act.idActe ";
    if( idActe > 0 )
        requete += " WHERE act.idActe = '" + QString::number(idActe) + "'";
    else if( idPat > 0 )
    {
        requete += " WHERE act.idPat = '" + QString::number(idPat) + "' "
                   " ORDER BY act.idActe DESC";
    }

    return requete;
}
QJsonObject DataBase::loadActeData(QVariantList actdata)
{
    QJsonObject data{};
    data["id"] = actdata.at(0).toInt();
    data["idPatient"] = actdata.at(1).toInt();
    data["idUser"] = actdata.at(2).toInt();
    data["date"] = QDateTime(actdata.at(3).toDate()).toMSecsSinceEpoch();
    data["motif"] = actdata.at(4).toString();
    data["texte"] = actdata.at(5).toString();
    data["conclusion"] = actdata.at(6).toString();
    data["courrierStatus"] = actdata.at(7).toString();
    data["cotation"] = actdata.at(8).toString();
    data["montant"] = actdata.at(9).toDouble();
    data["monnaie"] = actdata.at(10).toString();
    data["idCreatedBy"] = actdata.at(11).toInt();
    data["idUserComptable"] = actdata.at(12).toInt();
    data["idUserParent"] = actdata.at(13).toInt();

    if( actdata.at(14).isNull() )
        data["agePatient"] = -1;
    else
        data["agePatient"] = QDateTime(actdata.at(14).toDate()).toMSecsSinceEpoch();

    data["noActe"] = actdata.at(15).toInt();
    data["idActeMin"] = actdata.at(16).toInt();
    data["idActeMax"] = actdata.at(17).toInt();
    data["nbActes"] = actdata.at(18).toInt();

    if( actdata.at(19).isNull() )
        data["paiementType"] = "";
    else
        data["paiementType"] = actdata.at(19).toString();

    if( actdata.at(20).isNull() )
        data["paiementTiers"] = "";
    else
        data["paiementTiers"] = actdata.at(20).toString();

    return data;
}

Acte* DataBase::loadActeById(int idActe)
{
    Acte *acte = new Acte(idActe, 0, 0);
    QString req = loadActeRequest(idActe, 0);
    QVariantList actdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if( !ok || actdata.size()==0 )
        return Q_NULLPTR;
    QJsonObject data = loadActeData(actdata);
    acte->setData(data);
    return acte;
}

QMap<int, Acte*> DataBase::loadActesByPat(Patient *pat)
{
    QMap<int, Acte*> list;
    if( pat == Q_NULLPTR )
        return list;
    QString req = loadActeRequest(0, pat->id());
    QList<QVariantList> actlist = StandardSelectSQL(req,ok);
    if(!ok || actlist.size()==0)
        return list;
    for (int i=0; i<actlist.size(); ++i)
    {
        QJsonObject data = loadActeData(actlist.at(i));
        Acte *acte = new Acte();
        acte->setData(data);
        list[acte->id()] = acte;
    }
    return list;
}

double DataBase::getActePaye(int idActe)
{
    double montant = 0.0;
    // on récupère les lignes de paiement
    QString req = " SELECT lp.Paye, lr.Monnaie "
                  " FROM " NOM_TABLE_LIGNESPAIEMENTS " lp "
                  " LEFT JOIN " NOM_TABLE_RECETTES " lr on lr.idRecette = lp.idRecette "
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
