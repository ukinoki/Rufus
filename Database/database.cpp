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

DataBase* DataBase::getInstance()
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
    m_port =Port;
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

bool DataBase::traiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage)
{
    if (query.lastError().type() != QSqlError::NoError)
    {
        Logs::ERROR(ErrorMessage, tr("\nErreur\n") + query.lastError().text() +  tr("\nrequete = ") + requete);
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
    unlocktables();
    QSqlQuery ("SET AUTOCOMMIT = 0;", m_db );
    QString lockrequete = "LOCK TABLES " + ListTables.at(0) + " " + ModeBlocage;
    for (int i = 1; i < ListTables.size(); i++)
        lockrequete += "," + ListTables.at(i) + " " + ModeBlocage;
    return !traiteErreurRequete(QSqlQuery(lockrequete, m_db ),lockrequete, tr("Impossible de bloquer les tables en mode ") + ModeBlocage);
}

void DataBase::commit()
{
    QSqlQuery ("COMMIT;", m_db );
    unlocktables();
    QString commitrequete = "SET AUTOCOMMIT = 1;";
    traiteErreurRequete(QSqlQuery(commitrequete,m_db ), commitrequete, tr("Impossible de valider les mofifications"));
}

void DataBase::rollback()
{
    QSqlQuery ("ROLLBACK;", m_db );
    unlocktables();
    QString rollbackrequete = "SET AUTOCOMMIT = 1;";
    traiteErreurRequete(QSqlQuery(rollbackrequete, m_db ),rollbackrequete,"");
}

bool DataBase::locktables(QStringList ListTables, QString ModeBlocage)
{
    unlocktables();
    QString lockrequete = "LOCK TABLES " + ListTables.at(0) + " " + ModeBlocage;
    for (int i = 1; i < ListTables.size(); i++)
        lockrequete += "," + ListTables.at(i) + " " + ModeBlocage;
    QSqlQuery lockquery (lockrequete, m_db );
    return !traiteErreurRequete(lockquery,lockrequete, tr("Impossible de bloquer les tables en mode ") + ModeBlocage);
}

void DataBase::unlocktables()
{
    QString requete = "UNLOCK TABLES;";
    traiteErreurRequete(QSqlQuery (requete, m_db ), requete,"");
}

bool DataBase::testconnexionbase() // une requete simple pour vérifier que la connexion à la base fontionne toujours
{
    QString req = "select AdresseTCPServeur from " NOM_TABLE_PARAMSYSTEME;
    QSqlQuery testbasequery(req, DataBase::getInstance()->getDataBase());
    return (testbasequery.lastError().type()==QSqlError::NoError);
}

int DataBase::selectMaxFromTable(QString nomchamp, QString nomtable, QString errormsg)
{
    QString req = "select max(" + nomchamp + ") from " + nomtable;
    QSqlQuery query(req, getDataBase());
    if( traiteErreurRequete(query, req, errormsg) || !query.first())
        return -1;
    return query.value(0).toInt();
}

bool DataBase::SupprRecordFromTable(int id, QString nomChamp, QString nomtable, QString errormsg)
{
    QString req = "delete from " + nomtable + " where " + nomChamp + " = " + QString::number(id);
    return StandardSQL(req, errormsg);
}

QList<QList<QVariant>> DataBase::SelectRecordsFromTable(QStringList listselectChamp,
                                                        QString nomtable,
                                                        bool &OK,
                                                        QString where,
                                                        QString orderby,
                                                        bool distinct,
                                                        QString errormsg)
{
    QList<QList<QVariant>> listreponses;
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
    for (QHash<QString, QString>::const_iterator itset = sets.constBegin(); itset != sets.constEnd(); ++itset)
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
    QSqlQuery query = QSqlQuery(getDataBase());
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
    if (query.lastError().type() != QSqlError::NoError)
    {
        Logs::ERROR(errormsg, tr("\nErreur\n") + query.lastError().text());
        return false;
    }
    return true;
}

bool DataBase::StandardSQL(QString req , QString errormsg)
{
    QSqlQuery query(req, getDataBase());
    return !traiteErreurRequete(query, req, errormsg);
}

QList<QList<QVariant>> DataBase::StandardSelectSQL(QString req , bool &OK, QString errormsg)
{
    /*
    exemple:
        bool ok = true;
        QList<QList<QVariant>> list = db->StandardSelectSQL("Select idImpression from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient), ok);
        if (!ok)                                // erreur;
        if (list.size()==0)                     // réponse vide
     */
    QList<QList<QVariant>> listreponses;
    QSqlQuery query(req, getDataBase());
    QSqlRecord rec = query.record();
    if( traiteErreurRequete(query, req, errormsg))
    {
        OK = false;
        return listreponses;
    }
    OK = true;
    if( !query.first())
        return listreponses;
    do
    {
        QList<QVariant> record;
        for (int i=0; i<rec.count(); ++i)
            record << query.value(i);
        listreponses << record;
    } while (query.next());
    return listreponses;
}

QList<QVariant> DataBase::getFirstRecordFromStandardSelectSQL(QString req , bool &OK, QString errormsg)
{
    return StandardSelectSQL(req , OK, errormsg).at(0);
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
    QSqlQuery query(req, getDataBase() );
    if( query.lastError().type() != QSqlError::NoError )
    {
        jrep["code"] = -3;
        jrep["request"] = req;
        return jrep;
    }

    if( !query.first() )
    {
        jrep["code"] = -1;
        return jrep;
    }

    if( !query.value(userOffset).isNull() &&query.value(userOffset).toString() != QHostInfo::localHostName().left(60) )
    {
        jrep["code"] = -6;
        jrep["poste"] = query.value(userOffset).toString();
        // cette erreur n'est pas exploitée parce que dans certaines structures, un même utilisateur peut travailler sur plusieurs postes
        //return jrep;
    }

    jrep["code"] = 0;

    QJsonObject userData{};
    userData["id"] = query.value(0).toInt();
    userData["nom"] = query.value(1).toString();
    userData["prenom"] = query.value(2).toString();
    query.finish();

    m_userConnected = new User(login, password, userData);
    m_userConnected->setData( loadUserData(m_userConnected->id()) );
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
            " OPTAM, cpt.nomcompteabrege, cpt2.nomcompteabrege "                                        // 30,31,32
            " from " NOM_TABLE_UTILISATEURS " usr "
            " left outer join " NOM_TABLE_COMPTES " cpt on usr.idcompteencaisshonoraires = cpt.idCompte"
            " left outer join " NOM_TABLE_COMPTES " cpt2 on usr.idCompteParDefaut = cpt2.idCompte"
            " where usr.idUser = " + QString::number(idUser);

            //+ "  and userdesactive is null";
            // SL cette ligne est retirée parce qu'elle bloque l'affichage des utilisateurs désactivés dans dlg_gestionsusers

    QSqlQuery  query(req, getDataBase());
    if( traiteErreurRequete(query, req, tr("Impossible de retrouver les données de l'utilisateur")) )
        return userData;

    if( !query.first() )
        return userData;

    userData["isAllLoaded"]                 = true;
    userData["id"]                          = idUser;
    userData["droits"]                      = query.value(0).isNull() ? "" : query.value(0).toString();
    userData["AGA"]                         = (query.value(1).toInt() == 1);
    userData["login"]                       = query.value(2).isNull() ? "" : query.value(2).toString();
    userData["fonction"]                    = query.value(3).isNull() ? "" : query.value(3).toString();
    userData["titre"]                       = query.value(4).isNull() ? "" : query.value(4).toString();
    userData["nom"]                         = query.value(5).isNull() ? "" : query.value(5).toString();
    userData["prenom"]                      = query.value(6).isNull() ? "" : query.value(6).toString();
    userData["mail"]                        = query.value(7).isNull() ? "" : query.value(7).toString();
    userData["numPS"]                       = query.value(8).toInt();
    userData["specialite"]                  = query.value(9).isNull() ? "" : query.value(9).toString();
    userData["noSpecialite"]                = query.value(10).toInt();
    userData["numCO"]                       = query.value(11).isNull() ? "" : query.value(11).toString();
    userData["idCompteParDefaut"]           = query.value(12).toInt();
    userData["enregHonoraires"]             = query.value(13).toInt();
    userData["password"]                    = query.value(14).toString();
    userData["portable"]                    = query.value(15).isNull() ? "" : query.value(15).toString();
    userData["poste"]                       = query.value(16).toInt();
    userData["web"]                         = query.value(17).isNull() ? "" : query.value(17).toString();
    userData["memo"]                        = query.value(18).isNull() ? "" : query.value(18).toString();
    userData["desactive"]                   = (query.value(19).toInt() == 1);
    userData["policeEcran"]                 = query.value(20).isNull() ? "" : query.value(20).toString();
    userData["policeAttribut"]              = query.value(21).isNull() ? "" : query.value(21).toString();
    userData["secteur"]                     = query.value(22).toInt();
    userData["OPTAM"]                       = (query.value(30).toInt() == 1);
    userData["soignant"]                    = query.value(23).toInt();
    userData["responsableActes"]            = query.value(24).toInt();
    userData["cotation"]                    = (query.value(25).toInt() == 1);
    userData["employeur"]                   = query.value(26).toInt();
    userData["dateDerniereConnexion"]       = QDateTime(query.value(27).toDate(), query.value(27).toTime()).toMSecsSinceEpoch();
    userData["medecin"]                     = query.value(29).toInt();
    userData["idCompteEncaissHonoraires"]   = query.value(28).toInt();

    if( userData["idCompteEncaissHonoraires"].isNull() )
    {
        userData["idUserEncaissHonoraires"] = -1;
    }
    else
    {
        userData["idUserEncaissHonoraires"]    = idUser;
        userData["nomCompteEncaissHonoraires"] = query.value(31).toString();
        userData["nomUserEncaissHonoraires"]   = query.value(2).toString();
    }

    if( !query.value(32).isNull() )
        userData["nomCompteParDefaut"] = query.value(32).toString();

    query.finish();
    return userData;
}
QList<User*> DataBase::loadUsersAll()
{
    QList<User*> users;
    QString req = "select usr.iduser, usr.userlogin, usr.soignant, usr.responsableactes, "          //0,1,2,3
                    " usr.UserEnregHonoraires, usr.idCompteEncaissHonoraires, usr.UserCCAM, "       //4,5,6
                    " usr.UserEmployeur, cpt.nomcompteabrege "                                      //7,8
                  " from " NOM_TABLE_UTILISATEURS " usr "
                  " left outer join " NOM_TABLE_COMPTES " cpt on usr.idcompteencaisshonoraires = cpt.idCompte "
                  " where userdesactive is null";

    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return users;

    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["login"] = query.value(1).toString();
        jData["soignant"] = query.value(2).toInt();
        jData["responsableActes"] = query.value(3).toInt();
        jData["enregHonoraires"] = query.value(4).toInt();
        jData["idCompteEncaissHonoraires"] = query.value(5).toInt();
        jData["cotation"] = (query.value(6).toInt() == 1);
        jData["employeur"] = query.value(7).toInt();
        jData["nomCompteAbrege"] = query.value(8).toString();
        User *usr = new User(jData);
        users << usr;
    } while( query.next() );

    return users;
}

QJsonObject DataBase::loadUserDatabyLogin(QString login)
{
    QJsonObject userData{};

    QString req = "select iduser from " NOM_TABLE_UTILISATEURS " where UserLogin = '" + login + "'";
    QSqlQuery  query(req, getDataBase());
    if( traiteErreurRequete(query, req, tr("Impossible de retrouver les données de l'utilisateur")) )
        return userData;

    if( !query.first() )
        return userData;
    return loadUserData(query.value(0).toInt());
}

/*
 * Correspondants
*/
QList<Correspondant*> DataBase::loadCorrespondants()                             // tous les correspondants sans exception
{
    QList<Correspondant*> correspondants;
    QString req = "SELECT idCor, CorNom, CorPrenom, CorSexe, cormedecin FROM " NOM_TABLE_CORRESPONDANTS " order by cornom, corprenom";

    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return correspondants;
    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["nom"] = query.value(1).toString();
        jData["prenom"] = query.value(2).toString();
        jData["sexe"] = query.value(3).toString();
        jData["generaliste"] = (query.value(4).toInt()==1);
        Correspondant *cor = new Correspondant(jData);
        correspondants << cor;
    } while( query.next() );
    return correspondants;
}

QList<Correspondant*> DataBase::loadCorrespondantsALL()                             // tous les correspondants sans exception avec plus de renseignements
{
    QList<Correspondant*> correspondants;
    QString req = "SELECT idCor, CorNom, CorPrenom, nomspecialite as metier, CorAdresse1, CorAdresse2, CorAdresse3,"
                  " CorCodepostal, CorVille, CorTelephone, CorSexe, cormedecin FROM " NOM_TABLE_CORRESPONDANTS ", " NOM_TABLE_SPECIALITES
            " where cormedecin = 1 and corspecialite = idspecialite"
            " union"
            " SELECT idCor, CorNom, CorPrenom, corautreprofession as metier, CorAdresse1, CorAdresse2, CorAdresse3,"
            " CorCodepostal, CorVille, CorTelephone, CorSexe, cormedecin FROM " NOM_TABLE_CORRESPONDANTS
            " where cormedecin <> 1 or cormedecin is null"
            " order by metier, cornom, corprenom";
    QSqlQuery query(req,DataBase::getInstance()->getDataBase());
    if( traiteErreurRequete(query, req) || !query.first())
        return correspondants;
    do
    {
        QJsonObject jData{};
        jData["id"]         = query.value(0).toInt();
        jData["nom"]        = query.value(1).toString();
        jData["prenom"]     = query.value(2).toString();
        jData["metier"]     = query.value(3).toString();
        jData["adresse1"]   = query.value(4).toString();
        jData["adresse2"]   = query.value(5).toString();
        jData["adresse3"]   = query.value(6).toString();
        jData["codepostal"] = query.value(7).toString();
        jData["ville"]      = query.value(8).toString();
        jData["telephone"]  = query.value(9).toString();
        jData["sexe"]       = query.value(10).toString();
        jData["generaliste"] = (query.value(11).toInt()==1);
        Correspondant *cor = new Correspondant(jData);
        correspondants << cor;
    } while( query.next() );
    return correspondants;
}

void DataBase::SupprCorrespondant(int idcor)
{
    QString id = QString::number(idcor);
    QSqlQuery       ("delete from " NOM_TABLE_CORRESPONDANTS " where idcor = " + id, DataBase::getInstance()->getDataBase());
    QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg  = null where idcormedmg  = " + id, getDataBase());
    QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormespe1 = null where idcormespe1 = " + id, getDataBase());
    QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormespe2 = null where idcormespe2 = " + id, getDataBase());
    QSqlQuery       ("update " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormespe3 = null where idcormespe3 = " + id, getDataBase());
}

/*
 * DocsExternes
*/
QList<DocExterne*> DataBase::loadDoscExternesByPatientAll(int idpatient)
{
    QList<DocExterne*> docsexternes;
    QString req = "Select idImpression, TypeDoc, SousTypeDoc, Titre, Dateimpression,"
                  " compression, lienversfichier, formatdoc, Importance from " NOM_TABLE_IMPRESSIONS
                  " where idpat = " + QString::number(idpatient);
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return docsexternes;
    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["idpat"] = idpatient;
        jData["typedoc"] = query.value(1).toString();
        jData["soustypedoc"] = query.value(2).toString();
        jData["titre"] = query.value(3).toString();
        jData["dateimpression"] = QDateTime(query.value(4).toDate(), query.value(4).toTime()).toMSecsSinceEpoch();
        jData["compression"] = query.value(5).toInt();
        jData["lienversfichier"] = query.value(6).toString();
        jData["formatdoc"] = query.value(7).toString();
        jData["importance"] = query.value(8).toInt();
        DocExterne *doc = new DocExterne(jData);
        docsexternes << doc;
    } while( query.next() );
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
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return docexterneData;
    if( !query.first() )
        return docexterneData;
    docexterneData["isallloaded"] = true;

    docexterneData["id"] = query.value(0).toInt();
    docexterneData["iduser"] = query.value(1).toInt();
    docexterneData["idpat"] = query.value(2).toInt();
    docexterneData["typedoc"] = query.value(3).toString();
    docexterneData["soustypedoc"] = query.value(4).toString();

    docexterneData["titre"] = query.value(5).toString();
    docexterneData["textentete"] = query.value(6).toString();
    docexterneData["textcorps"] = query.value(7).toString();
    docexterneData["textorigine"] = query.value(8).toString();
    docexterneData["textpied"] = query.value(9).toString();

    docexterneData["dateimpression"] = QDateTime(query.value(10).toDate(), query.value(10).toTime()).toMSecsSinceEpoch();
    docexterneData["compression"] = query.value(11).toInt();
    docexterneData["lienversfichier"] = query.value(12).toString();
    docexterneData["ALD"] = (query.value(13).toInt()==1);
    docexterneData["useremetteur"] = query.value(14).toString();

    docexterneData["formatdoc"] = query.value(15).toString();
    docexterneData["importance"] = query.value(16).toInt();
    query.finish();
    return docexterneData;

}

void DataBase::SupprDocExterne(int iddoc)
{
    QString id = QString::number(iddoc);
    QSqlQuery       ("delete from " NOM_TABLE_IMPRESSIONS " where idimpression = " + id, DataBase::getInstance()->getDataBase());
}




/*******************************************************************************************************************************************************************
 ********* COMPTABILITÊ ********************************************************************************************************************************************
********************************************************************************************************************************************************************/
/*
 * Comptes
*/
QList<Compte*> DataBase::loadComptesAllUsers()
{
    QList<Compte*> comptes;
    QString req = "SELECT idCompte, cmpt.idBanque, idUser, IBAN, intitulecompte, NomCompteAbrege, SoldeSurDernierReleve, partage, desactive, NomBanque "
                  " FROM " NOM_TABLE_COMPTES " as cmpt "
                  " left outer join " NOM_TABLE_BANQUES " as bank on cmpt.idbanque = bank.idbanque ";
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return comptes;
    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["idbanque"] = query.value(1).toInt();
        jData["iduser"] = query.value(2).toInt();
        jData["IBAN"] = query.value(3).toString();
        jData["IntituleCompte"] = query.value(4).toString();
        jData["nom"] = query.value(5).toString();
        jData["solde"] = query.value(6).toDouble();
        jData["partage"] = (query.value(7).toInt() == 1);
        jData["desactive"] = (query.value(8).toInt() == 1);
        jData["NomBanque"] = query.value(9).toString();
        Compte *cpt = new Compte(jData);
        comptes << cpt;
    } while( query.next() );

    return comptes;
}

QList<Compte*> DataBase::loadComptesByUser(int idUser)
{
    QList<Compte*> comptes;
    QString req = "SELECT idCompte, cmpt.idBanque, idUser, IBAN, intitulecompte, NomCompteAbrege, SoldeSurDernierReleve, partage, desactive, NomBanque "
                  " FROM " NOM_TABLE_COMPTES " as cmpt "
                  " left outer join " NOM_TABLE_BANQUES " as bank on cmpt.idbanque = bank.idbanque "
                  " WHERE idUser = " + QString::number(idUser);
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return comptes;
    int idcptprefer=-1;
    QString chercheComptePrefereRequete =
            " select idcomptepardefaut from " NOM_TABLE_UTILISATEURS
            " where iduser = " + QString::number(idUser);
    QSqlQuery chercheComptePreferQuery (chercheComptePrefereRequete, getDataBase());
    if (chercheComptePreferQuery.size()>0)
    {
        chercheComptePreferQuery.first();
        idcptprefer= chercheComptePreferQuery.value(0).toInt();
    }

    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["idbanque"] = query.value(1).toInt();
        jData["iduser"] = query.value(2).toInt();
        jData["IBAN"] = query.value(3).toString();
        jData["IntituleCompte"] = query.value(4).toString();
        jData["nom"] = query.value(5).toString();
        jData["solde"] = query.value(6).toDouble();
        jData["partage"] = (query.value(7).toInt() == 1);
        jData["desactive"] = (query.value(8).toInt() == 1);
        jData["NomBanque"] = query.value(9).toString();
        jData["prefere"] = (query.value(0).toInt() == idcptprefer);
        Compte *cpt = new Compte(jData);
        comptes << cpt;
    } while( query.next() );

    return comptes;
}

int DataBase::getMaxLigneBanque()
{
    int a(0), b(0);
    QString req = "select max(idligne) from " NOM_TABLE_ARCHIVESBANQUE;
    QSqlQuery quer(req, getDataBase());
    if (quer.size()>0){
        quer.first();
        a = quer.value(0).toInt();
    }
    req = "select max(idligne) from " NOM_TABLE_LIGNESCOMPTES;
    QSqlQuery quer2(req, getDataBase());
    if (quer2.size()>0){
        quer2.first();
        if (quer2.value(0).toInt()>a)
            b = quer2.value(0).toInt();
    }
    return (((a<b)?b:a)+1);
}


/*
 * Depenses
*/
QList<Depense*> DataBase::loadDepensesByUser(int idUser)
{
    QList<Depense*> depenses;
    QString req = "SELECT idDep, DateDep , RefFiscale, Objet, Montant,"
                        " FamFiscale, Monnaie, idRec, ModePaiement, Compte,"
                        " NoCheque, dep.idFacture, LienFichier, Echeancier, Intitule"
                        " FROM " NOM_TABLE_DEPENSES " dep left join " NOM_TABLE_FACTURES " fac on dep.idFacture = fac.idFacture"
                        " WHERE dep.idUser = " + QString::number(idUser);
    QSqlQuery query (req,getDataBase());
    if( traiteErreurRequete(query, req) || !query.first())
        return depenses;

    do
    {
        QJsonObject jData{};
        jData["iddepense"]      = query.value(0).toInt();
        jData["iduser"]         = idUser;
        jData["date"]           = query.value(1).toDate().toString("yyyy-MM-dd");
        jData["reffiscale"]     = query.value(2).toString();
        jData["objet"]          = query.value(3).toString();
        jData["montant"]        = query.value(4).toDouble();
        jData["famfiscale"]     = query.value(5).toString();
        jData["monnaie"]        = query.value(6).toString();
        jData["idrecette"]      = query.value(7).toInt();
        jData["modepaiement"]   = query.value(8).toString();
        jData["compte"]         = query.value(9).toInt();
        jData["nocheque"]       = query.value(10).toInt();
        jData["idfacture"]      = query.value(11).toInt();
        jData["lienfacture"]    = query.value(12).toString();
        jData["echeancier"]     = (query.value(13).toInt()==1);
        jData["objetecheancier"]= query.value(14).toString();
        Depense *dep = new Depense(jData);
        depenses << dep;
    } while( query.next() );

    return depenses;
}

void DataBase::loadDepenseArchivee(Depense *dep)
{
    bool archivee = (QSqlQuery ("select idLigne from " NOM_TABLE_ARCHIVESBANQUE
                                  " where idDep = " + QString::number(dep->id()),
                       getDataBase())
                       .size() > 0);
    if (!archivee)  // pour les anciens enregistrements qui étaient archivés sans l'id...
    {
        archivee = (QSqlQuery("select idligne from " NOM_TABLE_ARCHIVESBANQUE
                                " where LigneDate = '" + dep->date().toString("yyyy-MM-dd")
                                + "' and LigneLibelle = '" + Utils::correctquoteSQL(dep->objet())
                                + "' and LigneMontant = " + QString::number(dep->montant()),
                      getDataBase())
                      .size() > 0);
    }
    dep->setArchivee(archivee);
}

QStringList DataBase::ListeRubriquesFiscales()
{
    QString req = "SELECT reffiscale from " NOM_TABLE_RUBRIQUES2035 " where FamFiscale is not null and famfiscale <> 'Prélèvement personnel'";
    QSqlQuery query (req, getDataBase());
    QStringList ListeRubriques;
    ListeRubriques << tr("Prélèvement personnel");
    for (int i = 0; i < query.size(); i++)
    {
            query.seek(i);
            ListeRubriques << query.value(0).toString();
    }
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
    QSqlQuery query (req,getDataBase());
    if( traiteErreurRequete(query, req) || !query.first())
        return listdepenses;
    do
    {
        QMap<int, Depense*>::const_iterator itDepense = m_listDepenses.find(query.value(0).toInt());
        if (itDepense != m_listDepenses.constEnd())
        {
            Depense *dep = itDepense.value();
            listdepenses << dep;
        }
    } while( query.next() );
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
   QSqlQuery query (req,getDataBase());
    if( traiteErreurRequete(query, req) || !query.first())
        return archives;
    do
    {
       QJsonObject jData{};
        jData["idligne"]                = query.value(0).toInt();
        jData["idcompte"]               = query.value(1).toInt();
        jData["iddepense"]              = query.value(2).toInt();
        jData["idrecette"]              = query.value(3).toInt();
        jData["idrecettespeciale"]      = query.value(4).toInt();
        jData["idremisecheque"]         = query.value(5).toInt();
        jData["lignedate"]              = query.value(6).toDate().toString("yyyy-MM-dd");
        jData["lignelibelle"]           = query.value(7).toString();
        jData["montant"]                = (query.value(9).toInt()==1? query.value(8).toDouble() : query.value(8).toDouble()*-1);
        jData["lignetypeoperation"]     = query.value(10).toString();
        jData["lignedateconsolidation"] = query.value(11).toDate().toString("yyyy-MM-dd");
        jData["idarchive"]              = query.value(12).toInt();
        Archive *arc = new Archive(jData);
        archives << arc;
    } while( query.next() );

    return archives;
}

/*
 * Banques
*/
QList<Banque*> DataBase::loadBanques()
{
    QList<Banque*> banques;
    QString req = "SELECT idBanque, idBanqueAbrege, NomBanque, CodeBanque FROM " NOM_TABLE_BANQUES;
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return banques;
    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["idbanqueabrege"] = query.value(1).toString();
        jData["nombanque"] = query.value(2).toString();
        jData["codebanque"] = query.value(3).toInt();
        Banque *bq = new Banque(jData);
        banques << bq;
    } while( query.next() );
    return banques;
}

/*
 * Tiers
*/
QList<Tiers*> DataBase::loadTiersPayants()
{
    QList<Tiers*> listetiers;
    QString req = "SELECT idtIERS, Nomtiers, AdresseTiers, Codepostaltiers, Villetiers, Telephonetiers, FaxTiers from " NOM_TABLE_TIERS;
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return listetiers;
    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["nomtiers"] = query.value(1).toInt();
        jData["adressetiers"] = query.value(2).toString();
        jData["codepostaltiers"] = query.value(3).toString();
        jData["villetiers"] = query.value(4).toString();
        jData["telephonetiers"] = query.value(5).toString();
        jData["faxtiers"] = query.value(5).toString();
        Tiers *tiers = new Tiers(jData);
        listetiers << tiers;
    } while( query.next() );

    return listetiers;
}

QList<TypeTiers*> DataBase::loadTypesTiers()
{
    QList<TypeTiers*> types;
    QString req = "SELECT Tiers FROM " NOM_TABLE_LISTETIERS;
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return types;
    do
    {
        QJsonObject jData{};
        jData["typetiers"] = query.value(0).toString();
        TypeTiers *type = new TypeTiers(jData);
        types << type;
    } while( query.next() );
    return types;
}


/*******************************************************************************************************************************************************************
 ***** FIN COMPTABILITÊ ********************************************************************************************************************************************
********************************************************************************************************************************************************************/

/*
 * Cotations
*/
QList<Cotation*> DataBase::loadCotations()
{
    QString  req = " select idcotation, Typeacte, MontantOPTAM, MontantNonOPTAM, MontantPratique, CCAM, idUser, Frequence from " NOM_TABLE_COTATIONS;
    QList<Cotation*> cotations;
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return cotations;
    do
    {
        QJsonObject jcotation{};
        jcotation["id"] = query.value(0).toInt();
        jcotation["typeacte"] = query.value(1).toString();
        jcotation["montantoptam"] = query.value(2).toDouble();
        jcotation["montantnonoptam"] = query.value(3).toDouble();
        jcotation["montantpratique"] = query.value(4).toDouble();
        jcotation["ccam"] = (query.value(5).toInt()==1);
        jcotation["iduser"] = query.value(6).toInt();
        jcotation["frequence"] = query.value(7).toInt();
        jcotation["descriptif"] = "";
        Cotation *cotation = new Cotation(jcotation);
        cotations << cotation;
    } while( query.next() );
    return cotations;
}

/*
 * Cotations
*/
QList<Cotation*> DataBase::loadCotationsByUser(int iduser)
{
    int k = 0;

    QList<Cotation*> cotations;
    QString  req = "SELECT idcotation, Typeacte, MontantOPTAM, MontantNonOPTAM, MontantPratique, CCAM, Frequence, nom"
          " FROM " NOM_TABLE_COTATIONS " cot left join " NOM_TABLE_CCAM " cc on cot.typeacte= cc.codeccam"
          " where idUser = " + QString::number(iduser) + " and typeacte in (select codeccam from " NOM_TABLE_CCAM ")"
          " order by typeacte";
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req))
        return cotations;
    if (query.first())
    do
    {
        ++k;
        QJsonObject jcotation{};
        jcotation["id"] = k;
        jcotation["idcotation"] = query.value(0).toInt();
        jcotation["typeacte"] = query.value(1).toString();
        jcotation["montantoptam"] = query.value(2).toDouble();
        jcotation["montantnonoptam"] = query.value(3).toDouble();
        jcotation["montantpratique"] = query.value(4).toDouble();
        jcotation["ccam"] = (query.value(5).toInt()==1);
        jcotation["iduser"] = iduser;
        jcotation["frequence"] = query.value(6).toInt();
        jcotation["descriptif"] = query.value(7).toString();
        Cotation *cotation = new Cotation(jcotation);
        cotations << cotation;
    } while( query.next() );
    req = " SELECT idcotation, Typeacte, MontantOPTAM, MontantNonOPTAM, MontantPratique, CCAM, Frequence, null as nom"
          " FROM "  NOM_TABLE_COTATIONS
          " where idUser = " + QString::number(iduser) +
          " and typeacte not in (select codeccam from  " NOM_TABLE_CCAM ")"
          " order by typeacte";
    QSqlQuery query1(req, getDataBase() );
    if( traiteErreurRequete(query1, req) || !query1.first())
        return cotations;
    do
    {
        k++;
        QJsonObject jcotation{};
        jcotation["id"] = k;
        jcotation["idcotation"] = query1.value(0).toInt();
        jcotation["typeacte"] = query1.value(1).toString();
        jcotation["montantoptam"] = query1.value(2).toDouble();
        jcotation["montantnonoptam"] = query1.value(3).toDouble();
        jcotation["montantpratique"] = query1.value(4).toDouble();
        jcotation["ccam"] = (query1.value(5).toInt()==1);
        jcotation["iduser"] = iduser;
        jcotation["frequence"] = query1.value(6).toInt();
        jcotation["descriptif"] = query1.value(7).toString();
        Cotation *cotation = new Cotation(jcotation);
        cotations << cotation;
    } while( query1.next() );

    return cotations;
}

QStringList DataBase::loadTypesCotations()
{
    QStringList listcotations;
    QString req = "select typeacte as code from " NOM_TABLE_COTATIONS
                  " union "
                  " select codeccam as code from " NOM_TABLE_CCAM
                  " order by code asc";
    QSqlQuery query(req, getDataBase());
    if( traiteErreurRequete(query, req) || !query.first())
        return listcotations;
    do
    {
        listcotations << query.value(0).toString();
    } while (query.next());
    return listcotations;
}

/*
 * Motifs
*/
QList<Motif*> DataBase::loadMotifs()
{
    QString  req = "SELECT idMotifsRDV, Motif, Raccourci, Couleur, Duree, ParDefaut, Utiliser, NoOrdre FROM "  NOM_TABLE_MOTIFSRDV " ORDER BY NoOrdre";
    QList<Motif*> motifs;
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return motifs;
    do
    {
        QJsonObject jmotif{};
        jmotif["id"] = query.value(0).toInt();
        jmotif["motif"] = query.value(1).toString();
        jmotif["raccourci"] = query.value(2).toString();
        jmotif["couleur"] = query.value(3).toString();
        jmotif["duree"] = query.value(4).toInt();
        jmotif["pardefaut"] = (query.value(5).toInt()==1);
        jmotif["utiliser"] = (query.value(6).toInt()==1);
        jmotif["noordre"] = query.value(7).toInt();
        Motif *motif = new Motif(jmotif);
        motifs << motif;
    } while( query.next() );
    return motifs;
}

/*
 * Sites
*/
QList<Site*> DataBase::loadSitesAll()
{
    QString req = "select joint.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, "
                    "LieuCodePostal, LieuVille, LieuTelephone, LieuFax "
                  "from " NOM_TABLE_JOINTURESLIEUX;

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
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return etabs;

    do
    {
        QJsonObject jEtab{};
        jEtab["idLieu"] = query.value(0).toInt();
        jEtab["nomLieu"] = query.value(1).toString();
        jEtab["adresse1"] = query.value(2).toString();
        jEtab["adresse2"] = query.value(3).toString();
        jEtab["adresse3"] = query.value(4).toString();
        jEtab["codePostal"] = query.value(5).toInt();
        jEtab["ville"] = query.value(6).toString();
        jEtab["telephone"] = query.value(7).toString();
        jEtab["fax"] = query.value(8).toString();
        Site *etab = new Site(jEtab);
        etabs << etab;
    } while( query.next() );

    return etabs;
}


/*
 * Villes
*/
Villes* DataBase::loadVillesAll()
{
    Villes *villes = new Villes();

    QString req = "select ville_id, codePostal, ville "
                  "from " NOM_TABLE_VILLES;
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return villes;

    do
    {
        QJsonObject jEtab{};
        jEtab["ville_id"] = query.value(0).toInt();
        jEtab["codePostal"] = query.value(1).toString();
        jEtab["ville"] = query.value(2).toString();
        Ville *ville = new Ville(jEtab);
        villes->addVille(ville);
    } while( query.next() );

    return villes;
}


/*
 * Gestion des Patients
*/
QList<Patient*> DataBase::loadPatientAll()
{
    QList<Patient*> patients;
    QString req = "select IdPat, PatNom, PatPrenom, PatDDN, Sexe, "          //0,1,2,3,4                                   //7,8
                  " from " NOM_TABLE_PATIENTS " usr "
                  " left outer join " NOM_TABLE_COMPTES " cpt on usr.idcompteencaisshonoraires = cpt.idCompte "
                  " ORDER BY PatNom, PatPrenom, PatDDN ";

    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return patients;
    do
    {
        QJsonObject jData{};
        jData["id"] = query.value(0).toInt();
        jData["nom"] = query.value(1).toString();
        jData["prenom"] = query.value(2).toString();
        jData["sexe"] = query.value(4).toString();
        jData["dateDeNaissance"] = QDateTime(query.value(3).toDate()).toMSecsSinceEpoch();
        Patient *patient = new Patient(jData);
        patients << patient;
    } while( query.next() );

    return patients;
}
Patient* DataBase::loadPatientById(int idPat)
{
    Patient *patient = new Patient();
    QString req = "select IdPat, PatNom, PatPrenom, PatDDN, Sexe from " NOM_TABLE_PATIENTS " where idPat = " + QString::number(idPat);
    QSqlQuery query(req, getDataBase() );
    if( traiteErreurRequete(query, req) || !query.first())
        return patient;
    QJsonObject jData{};
    jData["id"] = query.value(0).toInt();
    jData["nom"] = query.value(1).toString();
    jData["prenom"] = query.value(2).toString();
    jData["sexe"] = query.value(4).toString();
    jData["dateDeNaissance"] = QDateTime(query.value(3).toDate()).toMSecsSinceEpoch();
    patient->setData(jData);

    return patient;
}

/*
 * MDP
*/
//Pas normal, les mots de passes doivent etre chiffrés
QString DataBase::getMDPAdmin()
{
    QSqlQuery mdpquer("select mdpadmin from " NOM_TABLE_PARAMSYSTEME, getDataBase() );
    mdpquer.first();
    if (mdpquer.value(0).toString() == "")
        QSqlQuery("update " NOM_TABLE_PARAMSYSTEME " set mdpadmin = '" NOM_MDPADMINISTRATEUR "'", getDataBase() );
    return (mdpquer.value(0).toString() != ""? mdpquer.value(0).toString() : NOM_MDPADMINISTRATEUR);
}


/*
 * Actes
*/
QString DataBase::createActeRequest(int idActe, int idPat)
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
QJsonObject DataBase::extractActeData(QSqlQuery query)
{
    QJsonObject data{};
    data["id"] = query.value(0).toInt();
    data["idPatient"] = query.value(1).toInt();
    data["idUser"] = query.value(2).toInt();
    data["date"] = QDateTime(query.value(3).toDate()).toMSecsSinceEpoch();
    data["motif"] = query.value(4).toString();
    data["texte"] = query.value(5).toString();
    data["conclusion"] = query.value(6).toString();
    data["courrierStatus"] = query.value(7).toString();
    data["cotation"] = query.value(8).toString();
    data["montant"] = query.value(9).toDouble();
    data["monnaie"] = query.value(10).toString();
    data["idCreatedBy"] = query.value(11).toInt();
    data["idUserComptable"] = query.value(12).toInt();
    data["idUserParent"] = query.value(13).toInt();

    if( query.value(14).isNull() )
        data["agePatient"] = -1;
    else
        data["agePatient"] = QDateTime(query.value(14).toDate()).toMSecsSinceEpoch();

    data["noActe"] = query.value(15).toInt();
    data["idActeMin"] = query.value(16).toInt();
    data["idActeMax"] = query.value(17).toInt();
    data["nbActes"] = query.value(18).toInt();

    if( query.value(19).isNull() )
        data["paiementType"] = "";
    else
        data["paiementType"] = query.value(19).toString();

    if( query.value(20).isNull() )
        data["paiementTiers"] = "";
    else
        data["paiementTiers"] = query.value(20).toString();

    return data;
}
Acte* DataBase::loadActeById(int idActe)
{
    Acte *acte = new Acte(idActe, 0, 0);

    if( idActe == 0 )
        return acte;
    QString requete = createActeRequest(idActe, 0);
    QSqlQuery query(requete, getDataBase());
    if( traiteErreurRequete(query, requete) || !query.first() )
        return acte;

    QJsonObject data = extractActeData(query);
    acte->setData(data);
    return acte;
}
QMap<int, Acte*> DataBase::loadActesByIdPat(int idPat)
{
    QMap<int, Acte*> list;
    if( idPat == 0 )
        return list;

    QString requete = createActeRequest(0, idPat);
    QSqlQuery query(requete, getDataBase());
    if( traiteErreurRequete(query, requete) || !query.first() )
        return list;

    do
    {
        QJsonObject data = extractActeData(query);
        Acte *acte = new Acte();
        acte->setData(data);
        list[acte->id()] = acte;
    } while( query.next() );

    return list;
}
double DataBase::getActeMontant(int idActe)
{
    double montant = 0.0;
    // on récupère les lignes de paiement
    QString req = " SELECT lp.Paye, lr.Monnaie "
                  " FROM " NOM_TABLE_LIGNESPAIEMENTS " lp "
                  " LEFT JOIN " NOM_TABLE_RECETTES " lr on lr.idRecette = lp.idRecette "
                  " WHERE idActe = " + QString::number(idActe);
    QSqlQuery query(req, getDataBase());
    DataBase::getInstance()->traiteErreurRequete(query, req, "");
    if( !query.first() )
        return montant;
    do
    {
        if (query.value(1).toString() == "F")
            montant += (query.value(0).toDouble() / 6.55957);
        else
            montant += query.value(0).toDouble();
    } while( query.next() );

    return montant;
}
