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

#include "database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

DataBase* DataBase::instance = Q_NULLPTR;

DataBase* DataBase::I()
{
    if( !instance )
        instance = new DataBase();
    return instance;
}

DataBase::DataBase() {}

void DataBase::initParametresConnexionSQL(QString Server, int Port)
{
    m_server = Utils::calcIP(Server, false);
    m_port = Port;
}

void DataBase::setModeacces(const Utils::ModeAcces &modeacces)
{
    m_modeacces = modeacces;
}

Utils::ModeAcces DataBase::ModeAccesDataBase() const
{
    return m_modeacces;
}

QString DataBase::AdresseServer() const
{
    return m_server;
}
int DataBase::port() const
{
    return m_port;
}

void DataBase::InfosConnexionSQL()
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
    bool useSSL = (m_modeacces == Utils::Distant);
    QString connectSSLoptions = "";
    if (useSSL)
    {
        QString dirkey = "/etc/mysql";
        QSettings *m_settings = new QSettings(PATH_FILE_INI, QSettings::IniFormat);
        if (m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL").toString() != "")
            dirkey = m_settings->value(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL").toString();
        else
            m_settings->setValue(Utils::getBaseFromMode(Utils::Distant) + "/DossierClesSSL",dirkey);
        QDir dirtorestore(dirkey);
        //qDebug() << dirtorestore.absolutePath();
        QStringList listfichiers = dirtorestore.entryList(QStringList() << "*.pem");
        for (int t=0; t<listfichiers.size(); t++)
        {
            QString nomfich  = listfichiers.at(t);
            if (nomfich == "client-key.pem")
                connectSSLoptions += "SSL_KEY=" + dirkey + "/client-key.pem;";
            if (nomfich == "client-cert.pem")
                connectSSLoptions += "SSL_CERT=" + dirkey + "/client-cert.pem;";
            if (nomfich == "ca-cert.pem")
                connectSSLoptions += "SSL_CA=" + dirkey + "/ca-cert.pem;";
        }
    }
    QString connectOptions = connectSSLoptions + "MYSQL_OPT_RECONNECT=1";
    m_db.setConnectOptions(connectOptions);

    m_db.setUserName(login + (useSSL ? "SSL" : ""));
    m_db.setPassword(password);
    Logs::LogSQL("Serveur      - " + m_db.hostName());
    Logs::LogSQL("databaseName - " + m_db.databaseName());
    Logs::LogSQL("Login        - " + m_db.userName());
    Logs::LogSQL("port         - " + QString::number(m_db.port()));

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

/*!
 * \brief DataBase::selectMaxFromTable
 * \param nomchamp
 * \param nomtable
 * \param ok
 * \param errormsg
 * \return la valeur maximale du champ int d'une table
 * sur une table vide, la fonction renvoie 0 parce que la requete "select max(id) from table" renvoie une valeur null mais le nombre de réponses est de 1
 */
int DataBase::selectMaxFromTable(QString nomchamp, QString nomtable, bool &ok, QString errormsg)
{
    QString req = "select max(" + nomchamp + ") from " + nomtable;
    QVariantList data = getFirstRecordFromStandardSelectSQL(req, ok, errormsg);
    if(!ok || data.size()==0)
        return -1;
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
    //qDebug() << "DataBase::SelectRecordsFromTable" << req;
    return StandardSelectSQL(req, OK, errormsg);
}

bool DataBase::UpdateTable(QString nomtable,
                           QHash<QString, QVariant> sets,
                           QString where,
                           QString errormsg)
{
    QString req = "update " + nomtable + " set";
    QHashIterator<QString, QVariant> itset(sets);
    while (itset.hasNext())
    {
        itset.next();
        QString clause  = " " + itset.key() + " = " + (itset.value().toString().toLower()=="null" || itset.value() == QVariant() || itset.value().toString() == ""? "null," : "'" + Utils::correctquoteSQL(itset.value().toString()) + "',");
        //qDebug() << "itset.value().toString() = " << itset.value().toString();
        //qDebug() << "clause = " << clause;
        req += clause;
    }
    req = req.left(req.size()-1); //retire la virgule de la fin
    req += " " + where;
    //qDebug() << req;
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
        valeurs += (itset.value().toLower()=="null" || itset.value() == QVariant()? "null," : "'" + Utils::correctquoteSQL(itset.value()) + "',");
    }
    champs = champs.left(champs.size()-1) + ") values (";
    valeurs = valeurs.left(valeurs.size()-1) + ")";
    req += champs + valeurs;
    return StandardSQL(req, errormsg);
}

bool DataBase::InsertSQLByBinds(QString nomtable,
                                QHash<QString, QVariant> sets,
                                QString errormsg)                               /*! ++++++ si on veut entrer une valeur null la bindvalue doit être mise à QVariant() - "null" ne marche pas */
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
    //qDebug() << prepare;
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
    QSqlQuery query(req, m_db);
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
    QSqlQuery query(req, m_db);
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
     if (recorddata.size()==0)               // réponse vide
    */
    //qDebug() << req;
    QList<QVariantList> listreponses = StandardSelectSQL(req , OK, errormsg);
    if (listreponses.size()>0)
        return listreponses.at(0);
    else
        return QVariantList();
}

void DataBase::VideDatabases()
{
    UpSystemTrayIcon::I()->showMessage(tr("Messages"), tr("Suppression de l'ancienne base Rufus en cours"), Icons::icSunglasses(), 3000);
    StandardSQL ("drop database if exists " DB_COMPTA );
    StandardSQL ("drop database if exists " DB_OPHTA );
    StandardSQL ("drop database if exists " DB_CONSULTS );
    StandardSQL ("drop database if exists " DB_IMAGES );
}


/*
 * Parametres système
*/
void DataBase::initParametresSysteme()
{
    if (!m_db.isOpen())
        return;
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
        return ;
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
        initParametresSysteme();
    return m_parametres;
}
void DataBase::setmdpadmin(QString mdp)
{
    if (!m_db.isOpen())
        return;
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_MDPADMIN_PARAMSYSTEME " = '" +  mdp + "'");
    bool ok;
    QVariantList mdpdata = getFirstRecordFromStandardSelectSQL("select " CP_ID_USR " from " TBL_UTILISATEURS " where " CP_LOGIN_USR " = '" NOM_ADMINISTRATEUR "'", ok);
    if (!ok || mdpdata.size()==0)
        StandardSQL("insert into " TBL_UTILISATEURS " (" CP_NOM_USR ", " CP_LOGIN_USR ", " CP_MDP_USR ") values ('" NOM_ADMINISTRATEUR "', '" NOM_ADMINISTRATEUR "', '" + mdp + "')");
    else
        StandardSQL("update " TBL_UTILISATEURS " set " CP_MDP_USR " = '" + mdp + "' where " CP_LOGIN_USR " = '" NOM_ADMINISTRATEUR "'");
    parametres()->setmdpadmin(mdp);
}
void DataBase::setnumcentre(int id)
{
    if (!m_db.isOpen())
        return;
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_NUMCENTRE_PARAMSYSTEME " = " + QString::number(id));
    parametres()->setnumcentre(id);
}
void DataBase::setidlieupardefaut(int id)
{
    if (!m_db.isOpen())
        return;
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_IDLIEUPARDEFAUT_PARAMSYSTEME " = " + QString::number(id));
    parametres()->setidlieupardefaut(id);
}
void DataBase::setdocscomprimes(bool one)
{
    if (!m_db.isOpen())
        return;
    QString a = (one? "'1'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_DOCSCOMPRIMES_PARAMSYSTEME " = " + a);
    parametres()->setdocscomprimes(one);
}
void DataBase::setversionbase(int version)
{
    if (!m_db.isOpen())
        return;
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_VERSIONBASE_PARAMSYSTEME " = " + QString::number(version));
    parametres()->setversionbase(version);
}
void DataBase::setsanscompta(bool one)
{
    if (!m_db.isOpen())
        return;
    QString a = (!one? "'1'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_SANSCOMPTA_PARAMSYSTEME " = " + a);
    parametres()->setsanscompta(one);
}
void DataBase::setadresseserveurlocal(QString  adress)
{
    if (!m_db.isOpen())
        return;
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_ADRESSELOCALSERVEUR_PARAMSYSTEME " = " + value);
    parametres()->setadresseserveurlocal(adress);
}
void DataBase::setadresseserveurdistant(QString adress)
{
    if (!m_db.isOpen())
        return;
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_ADRESSEDISTANTSERVEUR_PARAMSYSTEME " = " + value);
    parametres()->setadresseserveurdistant(adress);
}
void DataBase::setdirimagerie(QString adress)
{
    if (!m_db.isOpen())
        return;
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_DIRIMAGERIE_PARAMSYSTEME " = " + value);
    parametres()->setdirimagerieserveur(adress);
}
void DataBase::setdaysbkup(Utils::Days days)
{
    if (!m_db.isOpen())
        return;
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
    parametres()->setdaysbkup(days);
}
void DataBase::setheurebkup(QTime time)
{
    if (!m_db.isOpen())
        return;
    QString value = (time != QTime()? "'" + time.toString("HH:mm:ss") + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_HEUREBKUP_PARAMSYSTEME " = " + value);
    parametres()->setheurebkup(time);
}
void DataBase::setdirbkup(QString adress)
{
    if (!m_db.isOpen())
        return;
    QString value = (adress != ""? "'" + Utils::correctquoteSQL(adress) + "'" : "null");
    StandardSQL("update " TBL_PARAMSYSTEME " set " CP_DIRBKUP_PARAMSYSTEME " = " + value);
    parametres()->setdirbkup(adress);
}

/*
 * Donnes ophtapatient
*/
void DataBase::initDonnesOphtaPatient(int idpat)
{
    if (m_donneesophtapatient == Q_NULLPTR)
        m_donneesophtapatient = new DonneesOphtaPatient();
    QJsonObject data{};
    QString req = "select " CP_ID_DATAOPHTA ", " CP_SPHEREOD_DATAOPHTA ", " CP_CYLINDREOD_DATAOPHTA ", " CP_AXECYLINDREOD_DATAOPHTA ", " CP_DATEREFRACTIONOD_DATAOPHTA ", "
                            CP_SPHEREOG_DATAOPHTA ", " CP_CYLINDREOG_DATAOPHTA ", " CP_AXECYLINDREOG_DATAOPHTA ", " CP_DATEREFRACTIONOG_DATAOPHTA ", " CP_ECARTIP_DATAOPHTA
                            " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idpat) + " and " CP_MESURE_DATAOPHTA " = 'A' order by idmesure asc" ;
    //qDebug() << req;
    QVariantList ophtadata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données opthalmologiques du patient"));
    if(ok && ophtadata.size() > 0)
    {
        data[CP_ID_DATAOPHTA "A"]               = ophtadata.at(0).toInt();
        data[CP_IDPATIENT_DATAOPHTA]            = idpat;

        data[CP_SPHEREOD_DATAOPHTA "A"]         = ophtadata.at(1).toDouble();
        data[CP_CYLINDREOD_DATAOPHTA "A"]       = ophtadata.at(2).toDouble();
        data[CP_AXECYLINDREOD_DATAOPHTA "A"]    = ophtadata.at(3).toInt();
        data[CP_DATEREFRACTIONOD_DATAOPHTA "A"] = ophtadata.at(4).toDate().toString("yyyy-MM-dd");
        data[CP_SPHEREOG_DATAOPHTA "A"]         = ophtadata.at(5).toDouble();
        data[CP_CYLINDREOG_DATAOPHTA "A"]       = ophtadata.at(6).toDouble();
        data[CP_AXECYLINDREOG_DATAOPHTA "A"]    = ophtadata.at(7).toInt();
        data[CP_DATEREFRACTIONOG_DATAOPHTA "A"] = ophtadata.at(8).toDate().toString("yyyy-MM-dd");
        data[CP_ECARTIP_DATAOPHTA "A"]          = ophtadata.at(9).toInt();
    }
    else
    {
        m_donneesophtapatient->cleandatas();
        return;
    }

    req = "select " CP_K1OD_DATAOPHTA  ", " CP_K2OD_DATAOPHTA ", " CP_AXEKOD_DATAOPHTA ", " CP_K1OG_DATAOPHTA ", " CP_K2OG_DATAOPHTA ", "
                    CP_AXEKOG_DATAOPHTA ", " CP_MODEMESUREKERATO_DATAOPHTA ", " CP_DATEKERATO_DATAOPHTA ", " CP_DIOTRIESK1OD_DATAOPHTA ", " CP_DIOTRIESK2OD_DATAOPHTA ", "
                    CP_DIOTRIESK1OG_DATAOPHTA ", " CP_DIOTRIESK2OG_DATAOPHTA
                    " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idpat) + " order by idmesure asc" ;
    ophtadata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données opthalmologiques du patient"));
    if(ok && ophtadata.size() > 0)
    {
        data[CP_K1OD_DATAOPHTA]                 = ophtadata.at(0).toDouble();
        data[CP_K2OD_DATAOPHTA]                 = ophtadata.at(1).toDouble();
        data[CP_AXEKOD_DATAOPHTA]               = ophtadata.at(2).toInt();
        data[CP_K1OG_DATAOPHTA]                 = ophtadata.at(3).toDouble();
        data[CP_K2OG_DATAOPHTA]                 = ophtadata.at(4).toDouble();
        data[CP_AXEKOG_DATAOPHTA]               = ophtadata.at(5).toInt();
        data[CP_MODEMESUREKERATO_DATAOPHTA]     = ophtadata.at(6).toString();
        data[CP_DATEKERATO_DATAOPHTA]           = ophtadata.at(7).toDate().toString("yyyy-MM-dd");
        data[CP_DIOTRIESK1OD_DATAOPHTA]         = ophtadata.at(8).toDouble();
        data[CP_DIOTRIESK2OD_DATAOPHTA]         = ophtadata.at(9).toDouble();
        data[CP_DIOTRIESK1OG_DATAOPHTA]         = ophtadata.at(10).toDouble();
        data[CP_DIOTRIESK2OG_DATAOPHTA]         = ophtadata.at(11).toDouble();
    }

    req = "select " CP_ID_DATAOPHTA ", " CP_DISTANCE_DATAOPHTA ", " CP_SPHEREOD_DATAOPHTA ", " CP_CYLINDREOD_DATAOPHTA ", " CP_AXECYLINDREOD_DATAOPHTA ", "
                    CP_AVLOD_DATAOPHTA ", " CP_ADDVPOD_DATAOPHTA ", " CP_AVPOD_DATAOPHTA ", " CP_DATEREFRACTIONOD_DATAOPHTA ", " CP_SPHEREOG_DATAOPHTA ", "
                    CP_CYLINDREOG_DATAOPHTA ", " CP_AXECYLINDREOG_DATAOPHTA ", " CP_AVLOG_DATAOPHTA ", " CP_ADDVPOG_DATAOPHTA ", " CP_AVPOG_DATAOPHTA ", "
                    CP_DATEREFRACTIONOG_DATAOPHTA ", " CP_ECARTIP_DATAOPHTA
                    " from " TBL_DONNEES_OPHTA_PATIENTS " where " CP_IDPATIENT_DATAOPHTA " = " + QString::number(idpat) + " and " CP_MESURE_DATAOPHTA " = 'R' order by idmesure asc" ;
    ophtadata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données opthalmologiques du patient"));
    if(ok && ophtadata.size() > 0)
    {
        data[CP_ID_DATAOPHTA]               = ophtadata.at(0).toInt();
        data[CP_DISTANCE_DATAOPHTA]         = ophtadata.at(1).toString();
        data[CP_SPHEREOD_DATAOPHTA]         = ophtadata.at(2).toDouble();
        data[CP_CYLINDREOD_DATAOPHTA]       = ophtadata.at(3).toDouble();
        data[CP_AXECYLINDREOD_DATAOPHTA]    = ophtadata.at(4).toInt();
        data[CP_AVLOD_DATAOPHTA]            = ophtadata.at(5).toString();
        data[CP_ADDVPOD_DATAOPHTA]          = ophtadata.at(6).toDouble();
        data[CP_AVPOD_DATAOPHTA]            = ophtadata.at(7).toString();
        data[CP_DATEREFRACTIONOD_DATAOPHTA] = ophtadata.at(8).toDate().toString("yyyy-MM-dd");
        data[CP_SPHEREOG_DATAOPHTA]         = ophtadata.at(9).toDouble();
        data[CP_CYLINDREOG_DATAOPHTA]       = ophtadata.at(10).toDouble();
        data[CP_AXECYLINDREOG_DATAOPHTA]    = ophtadata.at(11).toInt();
        data[CP_AVLOG_DATAOPHTA]            = ophtadata.at(12).toString();
        data[CP_ADDVPOG_DATAOPHTA]          = ophtadata.at(13).toDouble();
        data[CP_AVPOG_DATAOPHTA]            = ophtadata.at(14).toString();
        data[CP_DATEREFRACTIONOG_DATAOPHTA] = ophtadata.at(15).toDate().toString("yyyy-MM-dd");
        data[CP_ECARTIP_DATAOPHTA]          = ophtadata.at(16).toInt();
    }
    m_donneesophtapatient->setData(data);
    return;
}

DonneesOphtaPatient* DataBase::donneesOphtaPatient()
{
    return m_donneesophtapatient;
}
/*
 * Users
*/
DataBase::QueryResult DataBase::verifExistUser(QString login, QString password)
{
    QString req = "SELECT " CP_ID_USR
                  " FROM " TBL_UTILISATEURS
                  " WHERE " CP_LOGIN_USR " = '" + login + "' "
                  " AND " CP_MDP_USR " = '" + Utils::calcSHA1(password) + "'"
                  " AND " CP_ISDESACTIVE_USR " is null ";
    QVariantList usrdata = getFirstRecordFromStandardSelectSQL(req, ok);
    if(!ok)
        return Error;
    if(usrdata.size() == 1)
    {
        m_iduserConnected = usrdata.at(0).toInt();
        return OK;
    }
    else    /*! on essaie avec un mot de passe en clair */
    {
        req = "SELECT " CP_ID_USR
              " FROM " TBL_UTILISATEURS
              " WHERE " CP_LOGIN_USR " = '" + login + "' "
              " AND " CP_MDP_USR " = '" + password + "'"
              " AND " CP_ISDESACTIVE_USR " is null ";
        usrdata = getFirstRecordFromStandardSelectSQL(req, ok);
        if(!ok)
            return Error;
        else if(usrdata.size()==0)
            return Empty;
        else
        {
            m_iduserConnected = usrdata.at(0).toInt();
            return OK;
        }
    }
}

DataBase::QueryResult DataBase::calcidUserConnected(QString login, QString password)
{
    QString req = "SELECT " CP_ID_USR
                  " FROM " TBL_UTILISATEURS
                  " WHERE " CP_LOGIN_USR " = '" + login + "' "
                  " AND " CP_MDP_USR " = '" + Utils::calcSHA1(password) + "'"
                  " AND " CP_ISDESACTIVE_USR " is null ";
    QVariantList usrdata = getFirstRecordFromStandardSelectSQL(req, ok);
    if(!ok)
        return Error;
    if(usrdata.size() == 1)
    {
        m_iduserConnected = usrdata.at(0).toInt();
        return OK;
    }
    else  /*! on essaie avec un mot de passe en clair et si le mot de passe n'est pas crypté, on le crypte */
    {
        req = "SELECT " CP_ID_USR
              " FROM " TBL_UTILISATEURS
              " WHERE " CP_LOGIN_USR " = '" + login + "' "
              " AND " CP_MDP_USR " = '" + password + "'"
              " AND " CP_ISDESACTIVE_USR " is null ";
        usrdata = getFirstRecordFromStandardSelectSQL(req, ok);
        if(!ok)
            return Error;
        else if(usrdata.size()==0)
            return Empty;
        else
        {
            req = "Update " TBL_UTILISATEURS " set " CP_MDP_USR " = '" + Utils::calcSHA1(password) + "' where " CP_ID_USR " = " + usrdata.at(0).toString();
            if (StandardSQL(req))
            {
                m_iduserConnected = usrdata.at(0).toInt();
                return OK;
            }
            else
                return Empty;
        }
    }
}

QJsonObject DataBase::loadUserData(int idUser)
{
    QJsonObject userData{};

    QString req = "select " CP_DROITS_USR ", " CP_ISAGA_USR ", " CP_LOGIN_USR ", " CP_FONCTION_USR ", " CP_TITRE_USR ", "                               // 0,1,2,3,4
            CP_NOM_USR ", " CP_PRENOM_USR ", " CP_MAIL_USR ", " CP_NUMPS_USR ", " CP_SPECIALITE_USR ", "                                                // 5,6,7,8,9
            CP_IDSPECIALITE_USR ", " CP_NUMCO_USR ", " CP_IDCOMPTEPARDEFAUT_USR ", " CP_ENREGHONORAIRES_USR ", " CP_MDP_USR ", "                        // 10,11,12,13,14
            CP_PORTABLE_USR ", " CP_POSTE_USR ", " CP_WEBSITE_USR ", " CP_MEMO_USR ", " CP_ISDESACTIVE_USR ","                                          // 15,16,17,18,19
            CP_POLICEECRAN_USR ", " CP_POLICEATTRIBUT_USR ", " CP_SECTEUR_USR ", " CP_SOIGNANTSTATUS_USR ", " CP_RESPONSABLEACTES_USR ", "              // 20,21,22,23,24
            CP_CCAM_USR ", " CP_IDEMPLOYEUR_USR ", " CP_DATEDERNIERECONNEXION_USR ", " CP_IDCOMPTEENCAISSEMENTHONORAIRES_USR ", " CP_ISMEDECIN_USR ", " // 25,26,27,28,29
            CP_ISOPTAM_USR ", " CP_DATECREATIONMDP_USR                                                                                                  // 30, 31
            " from " TBL_UTILISATEURS
            " where " CP_ID_USR " = " + QString::number(idUser);

            //+ "  and userdesactive is null";
            // SL cette ligne est retirée parce qu'elle bloque l'affichage des utilisateurs désactivés dans dlg_gestionsusers
    //qDebug() << req;

    QVariantList usrdata = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données de l'utilisateur"));
    if (!ok)
        return userData;

    if(usrdata.size()==0)
        return userData;

    userData[CP_ID_USR]                             = idUser;
    userData[CP_DROITS_USR]                         = usrdata.at(0).isNull() ? "" : usrdata.at(0).toString();
    userData[CP_ISAGA_USR]                          = (usrdata.at(1).toInt() == 1);
    userData[CP_LOGIN_USR]                          = usrdata.at(2).isNull() ? "" : usrdata.at(2).toString();
    userData[CP_FONCTION_USR]                       = usrdata.at(3).isNull() ? "" : usrdata.at(3).toString();
    userData[CP_TITRE_USR]                          = usrdata.at(4).isNull() ? "" : usrdata.at(4).toString();
    userData[CP_NOM_USR]                            = usrdata.at(5).isNull() ? "" : usrdata.at(5).toString();
    userData[CP_PRENOM_USR]                         = usrdata.at(6).isNull() ? "" : usrdata.at(6).toString();
    userData[CP_MAIL_USR]                           = usrdata.at(7).isNull() ? "" : usrdata.at(7).toString();
    userData[CP_NUMPS_USR]                          = usrdata.at(8).toLongLong();
    userData[CP_SPECIALITE_USR]                     = usrdata.at(9).isNull() ? "" : usrdata.at(9).toString();
    userData[CP_IDSPECIALITE_USR]                   = usrdata.at(10).toInt();
    userData[CP_NUMCO_USR]                          = usrdata.at(11).isNull() ? "" : usrdata.at(11).toString();
    userData[CP_IDCOMPTEPARDEFAUT_USR]              = (usrdata.at(12).isNull()? -1 : usrdata.at(12).toInt());
    userData[CP_ENREGHONORAIRES_USR]                = usrdata.at(13).toInt();
    userData[CP_MDP_USR]                            = usrdata.at(14).toString();
    userData[CP_PORTABLE_USR]                       = usrdata.at(15).isNull() ? "" : usrdata.at(15).toString();
    userData[CP_POSTE_USR]                          = usrdata.at(16).toInt();
    userData[CP_WEBSITE_USR]                        = usrdata.at(17).isNull() ? "" : usrdata.at(17).toString();
    userData[CP_MEMO_USR]                           = usrdata.at(18).isNull() ? "" : usrdata.at(18).toString();
    userData[CP_ISDESACTIVE_USR]                    = (usrdata.at(19).toInt() == 1);
    userData[CP_POLICEECRAN_USR]                    = usrdata.at(20).isNull() ? "" : usrdata.at(20).toString();
    userData[CP_POLICEATTRIBUT_USR]                 = usrdata.at(21).isNull() ? "" : usrdata.at(21).toString();
    userData[CP_SECTEUR_USR]                        = usrdata.at(22).toInt();
    userData[CP_ISOPTAM_USR]                        = (usrdata.at(30).toInt() == 1);
    userData[CP_SOIGNANTSTATUS_USR]                 = usrdata.at(23).toInt();
    userData[CP_RESPONSABLEACTES_USR]               = usrdata.at(24).toInt();
    userData[CP_CCAM_USR]                           = (usrdata.at(25).toInt() == 1);
    userData[CP_IDEMPLOYEUR_USR]                    = usrdata.at(26).toInt();
    userData[CP_DATEDERNIERECONNEXION_USR]          = QDateTime(usrdata.at(27).toDate(), usrdata.at(27).toTime()).toMSecsSinceEpoch();
    userData[CP_ISMEDECIN_USR]                      = usrdata.at(29).toInt();
    userData[CP_IDCOMPTEENCAISSEMENTHONORAIRES_USR] = (usrdata.at(28).isNull()? -1 : usrdata.at(28).toInt());
    userData[CP_DATECREATIONMDP_USR]                = usrdata.at(31).toDate().toString("yyyy-MM-dd");
    return userData;
}

QJsonObject DataBase::loadAdminData()
{
    QJsonObject userData{};
    QString req = "select " CP_ID_USR " from " TBL_UTILISATEURS " where " CP_LOGIN_USR " = '" NOM_ADMINISTRATEUR "'";
    QVariantList usrid = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver les données de l'administrateur"));
    if (!ok || usrid.size()==0)
        return userData;
    return loadUserData(usrid.at(0).toInt());
}

QList<User*> DataBase::loadUsersShortListe()
{
    QList<User*> users;
    QString req = "select " CP_ID_USR ", " CP_LOGIN_USR  " from " TBL_UTILISATEURS;

    QList<QVariantList> usrlist = StandardSelectSQL(req, ok);
    if( !ok || usrlist.size()==0 )
        return users;
    for (int i=0; i<usrlist.size(); ++i)
    {
        QVariantList usrdata = usrlist.at(i);
        QJsonObject userData{};
        userData[CP_ID_USR]                             = usrdata.at(0).toInt();
        userData[CP_LOGIN_USR]                          = usrdata.at(1).toString();
        User *usr = new User(userData);
        users << usr;
    }
    return users;
}

QList<User*> DataBase::loadUsers()
{
    QList<User*> users;
    QString req = "select " CP_DROITS_USR ", " CP_ISAGA_USR ", " CP_LOGIN_USR ", " CP_FONCTION_USR ", " CP_TITRE_USR ", "                               // 0,1,2,3,4
            CP_NOM_USR ", " CP_PRENOM_USR ", " CP_MAIL_USR ", " CP_NUMPS_USR ", " CP_SPECIALITE_USR ", "                                                // 5,6,7,8,9
            CP_IDSPECIALITE_USR ", " CP_NUMCO_USR ", " CP_IDCOMPTEPARDEFAUT_USR ", " CP_ENREGHONORAIRES_USR ", " CP_MDP_USR ", "                        // 10,11,12,13,14
            CP_PORTABLE_USR ", " CP_POSTE_USR ", " CP_WEBSITE_USR ", " CP_MEMO_USR ", " CP_ISDESACTIVE_USR ","                                          // 15,16,17,18,19
            CP_POLICEECRAN_USR ", " CP_POLICEATTRIBUT_USR ", " CP_SECTEUR_USR ", " CP_SOIGNANTSTATUS_USR ", " CP_RESPONSABLEACTES_USR ", "              // 20,21,22,23,24
            CP_CCAM_USR ", " CP_IDEMPLOYEUR_USR ", " CP_DATEDERNIERECONNEXION_USR ", " CP_IDCOMPTEENCAISSEMENTHONORAIRES_USR ", " CP_ISMEDECIN_USR ", " // 25,26,27,28,29
            CP_ISOPTAM_USR ", " CP_ID_USR ", " CP_DATECREATIONMDP_USR                                                                                   // 30,31,32
            " from " TBL_UTILISATEURS;

    QList<QVariantList> usrlist = StandardSelectSQL(req, ok);
    if( !ok || usrlist.size()==0 )
        return users;
    for (int i=0; i<usrlist.size(); ++i)
    {
        QVariantList usrdata = usrlist.at(i);
        QJsonObject userData{};
        userData[CP_ID_USR]                             = usrdata.at(31).toInt();
        userData[CP_DROITS_USR]                         = usrdata.at(0).isNull() ? "" : usrdata.at(0).toString();
        userData[CP_ISAGA_USR]                          = (usrdata.at(1).toInt() == 1);
        userData[CP_LOGIN_USR]                          = usrdata.at(2).isNull() ? "" : usrdata.at(2).toString();
        userData[CP_FONCTION_USR]                       = usrdata.at(3).isNull() ? "" : usrdata.at(3).toString();
        userData[CP_TITRE_USR]                          = usrdata.at(4).isNull() ? "" : usrdata.at(4).toString();
        userData[CP_NOM_USR]                            = usrdata.at(5).isNull() ? "" : usrdata.at(5).toString();
        userData[CP_PRENOM_USR]                         = usrdata.at(6).isNull() ? "" : usrdata.at(6).toString();
        userData[CP_MAIL_USR]                           = usrdata.at(7).isNull() ? "" : usrdata.at(7).toString();
        userData[CP_NUMPS_USR]                          = usrdata.at(8).toLongLong();
        userData[CP_SPECIALITE_USR]                     = usrdata.at(9).isNull() ? "" : usrdata.at(9).toString();
        userData[CP_IDSPECIALITE_USR]                   = usrdata.at(10).toInt();
        userData[CP_NUMCO_USR]                          = usrdata.at(11).isNull() ? "" : usrdata.at(11).toString();
        userData[CP_IDCOMPTEPARDEFAUT_USR]              = (usrdata.at(12).isNull()? -1 : usrdata.at(12).toInt());
        userData[CP_ENREGHONORAIRES_USR]                = usrdata.at(13).toInt();
        userData[CP_MDP_USR]                            = usrdata.at(14).toString();
        userData[CP_PORTABLE_USR]                       = usrdata.at(15).isNull() ? "" : usrdata.at(15).toString();
        userData[CP_POSTE_USR]                          = usrdata.at(16).toInt();
        userData[CP_WEBSITE_USR]                        = usrdata.at(17).isNull() ? "" : usrdata.at(17).toString();
        userData[CP_MEMO_USR]                           = usrdata.at(18).isNull() ? "" : usrdata.at(18).toString();
        userData[CP_ISDESACTIVE_USR]                    = (usrdata.at(19).toInt() == 1);
        userData[CP_POLICEECRAN_USR]                    = usrdata.at(20).isNull() ? "" : usrdata.at(20).toString();
        userData[CP_POLICEATTRIBUT_USR]                 = usrdata.at(21).isNull() ? "" : usrdata.at(21).toString();
        userData[CP_SECTEUR_USR]                        = usrdata.at(22).toInt();
        userData[CP_ISOPTAM_USR]                        = (usrdata.at(30).toInt() == 1);
        userData[CP_SOIGNANTSTATUS_USR]                 = usrdata.at(23).toInt();
        userData[CP_RESPONSABLEACTES_USR]               = usrdata.at(24).toInt();
        userData[CP_CCAM_USR]                           = (usrdata.at(25).toInt() == 1);
        userData[CP_IDEMPLOYEUR_USR]                    = usrdata.at(26).toInt();
        userData[CP_DATEDERNIERECONNEXION_USR]          = QDateTime(usrdata.at(27).toDate(), usrdata.at(27).toTime()).toMSecsSinceEpoch();
        userData[CP_ISMEDECIN_USR]                      = usrdata.at(29).toInt();
        userData[CP_IDCOMPTEENCAISSEMENTHONORAIRES_USR] = (usrdata.at(28).isNull()? -1 : usrdata.at(28).toInt());
        userData[CP_DATECREATIONMDP_USR]                = usrdata.at(32).toDate().toString("yyyy-MM-dd");
        User *usr = new User(userData);
        users << usr;
    }
    return users;
}

/*
 * Postes connectes
*/
QJsonObject DataBase::loadPosteConnecteData(int iduser, QString macadress)
{
    QJsonObject posteData{};
    QString req = "select NomPosteConnecte, AccesDistant, UserSuperviseur,"
                  " UserComptable, UserParent, idLieu, HeureDerniereConnexion, idPat, IPAdress"
                  " from " TBL_USERSCONNECTES
                  " where idUser = " + QString::number(iduser) +
                  " and " CP_MACADRESS_USRCONNECT " = '" + macadress + "'";
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
        posteData["stringid"]                               = macadress.split(" ").at(0) + " - " + QString::number(iduser);
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
    QString req = "Select " CP_ID_DOCSEXTERNES ", " CP_TYPEDOC_DOCSEXTERNES ", " CP_SOUSTYPEDOC_DOCSEXTERNES ", " CP_TITRE_DOCSEXTERNES ", " CP_DATE_DOCSEXTERNES ","
                  CP_COMPRESSION_DOCSEXTERNES ", " CP_LIENFICHIER_DOCSEXTERNES ", " CP_FORMATDOC_DOCSEXTERNES ", " CP_IMPORTANCE_DOCSEXTERNES " from " TBL_DOCSEXTERNES
                  " where " CP_IDPAT_DOCSEXTERNES " = " + QString::number(pat->id());
//#ifdef Q_OS_LINUX
//    req += " and " CP_FORMATDOC_DOCSEXTERNES " <> '" VIDEO "'";
//#endif

    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return docsexternes;
    for (int i=0; i<doclist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ISALLLOADED]                   = false;
        jData[CP_ID_DOCSEXTERNES]               = doclist.at(i).at(0).toInt();
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
    QString req = "Select " CP_ID_DOCSEXTERNES ", " CP_IDUSER_DOCSEXTERNES ", " CP_IDPAT_DOCSEXTERNES ", " CP_TYPEDOC_DOCSEXTERNES ", " CP_SOUSTYPEDOC_DOCSEXTERNES ","
                  CP_TITRE_DOCSEXTERNES ", " CP_TEXTENTETE_DOCSEXTERNES ", " CP_TEXTCORPS_DOCSEXTERNES ", " CP_TEXTORIGINE_DOCSEXTERNES ", " CP_TEXTPIED_DOCSEXTERNES ","
                  CP_DATE_DOCSEXTERNES ", " CP_COMPRESSION_DOCSEXTERNES ", " CP_LIENFICHIER_DOCSEXTERNES ", " CP_ALD_DOCSEXTERNES ", " CP_IDEMETTEUR_DOCSEXTERNES ","
                  CP_FORMATDOC_DOCSEXTERNES ", " CP_IMPORTANCE_DOCSEXTERNES ", " CP_IDREFRACTION_DOCSEXTERNES " from " TBL_DOCSEXTERNES
                  " where " CP_ID_DOCSEXTERNES " = " + QString::number(idDoc);
    QVariantList docdata = getFirstRecordFromStandardSelectSQL(req, ok);
    if (!ok || docdata.size()==0)
        return jData;
    jData[CP_ISALLLOADED]                   = true;

    jData[CP_ID_DOCSEXTERNES]               = docdata.at(0).toInt();
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
    QString req = "Select " CP_ID_IMPRESSIONS ", " CP_TEXTE_IMPRESSIONS ", " CP_RESUME_IMPRESSIONS ", " CP_CONCLUSION_IMPRESSIONS ", " CP_IDUSER_IMPRESSIONS ","
                  CP_DOCPUBLIC_IMPRESSIONS ", " CP_PRESCRIPTION_IMPRESSIONS ", " CP_EDITABLE_IMPRESSIONS ", " CP_MEDICAL_IMPRESSIONS " from " TBL_IMPRESSIONS
                  " WHERE (" CP_IDUSER_IMPRESSIONS " = " + QString::number(m_iduserConnected) + " Or (" CP_DOCPUBLIC_IMPRESSIONS " = 1 and " CP_IDUSER_IMPRESSIONS " <> " + QString::number(m_iduserConnected) + "))"
                  " ORDER BY " CP_RESUME_IMPRESSIONS;
    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return impressions;
    for (int i=0; i<doclist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_IMPRESSIONS]            = doclist.at(i).at(0).toInt();
        jData[CP_TEXTE_IMPRESSIONS]         = doclist.at(i).at(1).toString();
        jData[CP_RESUME_IMPRESSIONS]        = doclist.at(i).at(2).toString();
        jData[CP_CONCLUSION_IMPRESSIONS]    = doclist.at(i).at(3).toString();
        jData[CP_IDUSER_IMPRESSIONS]        = doclist.at(i).at(4).toInt();
        jData[CP_DOCPUBLIC_IMPRESSIONS]     = (doclist.at(i).at(5).toInt()==1);
        jData[CP_PRESCRIPTION_IMPRESSIONS]  = (doclist.at(i).at(6).toInt()==1);
        jData[CP_EDITABLE_IMPRESSIONS ]     = (doclist.at(i).at(7).toInt()==1);
        jData[CP_MEDICAL_IMPRESSIONS]       = (doclist.at(i).at(8).toInt()==1);
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
    QString     req =  "SELECT " CP_RESUME_DOSSIERIMPRESSIONS " , " CP_ID_DOSSIERIMPRESSIONS " , " CP_IDUSER_DOSSIERIMPRESSIONS ", " CP_PUBLIC_DOSSIERIMPRESSIONS ", " CP_TEXTE_DOSSIERIMPRESSIONS
                       " FROM "  TBL_DOSSIERSIMPRESSIONS
                       " WHERE " CP_IDUSER_DOSSIERIMPRESSIONS " = " + QString::number(m_iduserConnected);
                req += " UNION \n";
                req += "select " CP_RESUME_DOSSIERIMPRESSIONS ", " CP_ID_DOSSIERIMPRESSIONS ", " CP_IDUSER_DOSSIERIMPRESSIONS ", " CP_PUBLIC_DOSSIERIMPRESSIONS ", " CP_TEXTE_DOSSIERIMPRESSIONS " from " TBL_DOSSIERSIMPRESSIONS
                       " where " CP_IDUSER_DOSSIERIMPRESSIONS " not in\n"
                       " (select met." CP_ID_DOSSIERIMPRESSIONS " from " TBL_DOSSIERSIMPRESSIONS " as met, "
                       TBL_JOINTURESIMPRESSIONS " as joi, "
                       TBL_IMPRESSIONS " as doc\n"
                       " where joi." CP_IDMETADOCUMENT_JOINTURESIMPRESSIONS " = met." CP_ID_DOSSIERIMPRESSIONS "\n"
                       " and joi." CP_IDDOCUMENT_JOINTURESIMPRESSIONS " = doc." CP_ID_IMPRESSIONS "\n"
                       " and doc." CP_DOCPUBLIC_IMPRESSIONS " is null)\n";
                req += " ORDER BY " CP_RESUME_DOSSIERIMPRESSIONS ";";
//    qDebug() << req;
    QList<QVariantList> doclist = StandardSelectSQL(req,ok);
    if(!ok || doclist.size()==0)
        return dossiers;
    for (int i=0; i<doclist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_DOSSIERIMPRESSIONS]     = doclist.at(i).at(1).toInt();
        jData[CP_TEXTE_DOSSIERIMPRESSIONS]  = doclist.at(i).at(4).toString();
        jData[CP_RESUME_DOSSIERIMPRESSIONS] = doclist.at(i).at(0).toString();
        jData[CP_IDUSER_DOSSIERIMPRESSIONS] = doclist.at(i).at(2).toInt();
        jData[CP_PUBLIC_DOSSIERIMPRESSIONS] = (doclist.at(i).at(3).toInt()==1);
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
    QList<QVariantList> cptlist = StandardSelectSQL(req,ok);
    if(!ok || cptlist.size()==0)
        return listcomptes;
    for (int i=0; i<cptlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_COMPTES]  = cptlist.at(i).at(0).toInt();
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
    QList<QVariantList> cptlist = StandardSelectSQL(req,ok);
    if(!ok || cptlist.size()==0)
        return jData;
    for (int i=0; i<cptlist.size(); ++i)
    {
        jData[CP_ID_COMPTES]  = id;
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
 * LignesComptes
*/
QList<LigneCompte*> DataBase::loadLignesComptesByCompte(int idcompte)
{
    QList<LigneCompte*> listlignes = QList<LigneCompte*>();
    bool ok;
    QList<QVariantList> lignlist = SelectRecordsFromTable(QStringList() << CP_ID_LIGNCOMPTES
                                                                        << CP_IDDEP_LIGNCOMPTES
                                                                        << CP_IDREC_LIGNCOMPTES
                                                                        << CP_IDRECSPEC_LIGNCOMPTES
                                                                        << CP_IDREMCHEQ_LIGNCOMPTES
                                                                        << CP_DATE_LIGNCOMPTES
                                                                        << CP_LIBELLE_LIGNCOMPTES
                                                                        << CP_MONTANT_LIGNCOMPTES
                                                                        << CP_DEBITCREDIT_LIGNCOMPTES
                                                                        << CP_TYPEOPERATION_LIGNCOMPTES
                                                                        << CP_CONSOLIDE_LIGNCOMPTES,
                                                                        TBL_LIGNESCOMPTES,
                                                                        ok,
                                                                        "where " CP_IDCOMPTE_LIGNCOMPTES " = " + QString::number(idcompte),
                                                                        "order by " CP_DATE_LIGNCOMPTES ", " CP_LIBELLE_LIGNCOMPTES ", " CP_MONTANT_LIGNCOMPTES );
    if(!ok || lignlist.size()==0)
        return listlignes;
    for (int i=0; i<lignlist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_LIGNCOMPTES]       = lignlist.at(i).at(0).toInt();
        jData[CP_IDCOMPTE_LIGNCOMPTES]      = idcompte;
        jData[CP_IDDEP_LIGNCOMPTES]         = lignlist.at(i).at(1).toInt();
        jData[CP_IDREC_LIGNCOMPTES]         = lignlist.at(i).at(2).toString();
        jData[CP_IDRECSPEC_LIGNCOMPTES]     = lignlist.at(i).at(3).toString();
        jData[CP_IDREMCHEQ_LIGNCOMPTES]     = lignlist.at(i).at(4).toString();
        jData[CP_DATE_LIGNCOMPTES]          = lignlist.at(i).at(5).toDate().toString("yyyy-MM-dd");
        jData[CP_LIBELLE_LIGNCOMPTES]       = lignlist.at(i).at(6).toString();
        jData[CP_MONTANT_LIGNCOMPTES]       = lignlist.at(i).at(7).toDouble();
        jData[CP_DEBITCREDIT_LIGNCOMPTES]   = (lignlist.at(i).at(8).toInt() == 1);
        jData[CP_TYPEOPERATION_LIGNCOMPTES] = lignlist.at(i).at(9).toString();
        jData[CP_CONSOLIDE_LIGNCOMPTES]     = (lignlist.at(i).at(10).toInt() == 1);
        LigneCompte *lign = new LigneCompte(jData);
        if (lign != Q_NULLPTR)
            listlignes << lign;
    }
    return listlignes;
}

QJsonObject DataBase::loadLigneCompteDataById(int id)
{
    QJsonObject jData{};
    bool ok;
    QString req = "SELECT " CP_IDCOMPTE_LIGNCOMPTES ", "
            CP_IDDEP_LIGNCOMPTES ", "
            CP_IDREC_LIGNCOMPTES ", "
            CP_IDRECSPEC_LIGNCOMPTES ", "
            CP_IDREMCHEQ_LIGNCOMPTES ", "
            CP_DATE_LIGNCOMPTES ", "
            CP_LIBELLE_LIGNCOMPTES ", "
            CP_MONTANT_LIGNCOMPTES ", "
            CP_DEBITCREDIT_LIGNCOMPTES ", "
            CP_TYPEOPERATION_LIGNCOMPTES ", "
            CP_CONSOLIDE_LIGNCOMPTES " FROM "
            TBL_LIGNESCOMPTES
            " where " CP_ID_LIGNCOMPTES " = " + QString::number(id);
    QVariantList lign = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || lign.size()==0)
        return jData;
    jData[CP_ID_LIGNCOMPTES]       = id;
    jData[CP_IDCOMPTE_LIGNCOMPTES]      = lign.at(0).toInt();
    jData[CP_IDDEP_LIGNCOMPTES]         = lign.at(1).toInt();
    jData[CP_IDREC_LIGNCOMPTES]         = lign.at(2).toString();
    jData[CP_IDRECSPEC_LIGNCOMPTES]     = lign.at(3).toString();
    jData[CP_IDREMCHEQ_LIGNCOMPTES]     = lign.at(4).toString();
    jData[CP_DATE_LIGNCOMPTES]          = lign.at(5).toDate().toString("yyyy-MM-dd");
    jData[CP_LIBELLE_LIGNCOMPTES]       = lign.at(6).toString();
    jData[CP_MONTANT_LIGNCOMPTES]       = lign.at(7).toDouble();
    jData[CP_DEBITCREDIT_LIGNCOMPTES]   = (lign.at(8).toInt() == 1);
    jData[CP_TYPEOPERATION_LIGNCOMPTES] = lign.at(9).toString();
    jData[CP_CONSOLIDE_LIGNCOMPTES]     = (lign.at(10).toInt() == 1);
    return jData;
}

/*
 * Depenses
*/
QList<Depense*> DataBase::loadDepensesByUser(int idUser)
{
    QList<Depense*> depenses;
    QString req = "SELECT " CP_ID_DEPENSES ", " CP_DATE_DEPENSES " , dep." CP_REFFISCALE_DEPENSES ", " CP_OBJET_DEPENSES ", " CP_MONTANT_DEPENSES ","
                        " dep." CP_FAMILLEFISCALE_DEPENSES ", " CP_MONNAIE_DEPENSES ", " CP_IDRECETTE_DEPENSES ", " CP_MODEPAIEMENT_DEPENSES ", " CP_COMPTE_DEPENSES ","
                          CP_NUMCHEQUE_DEPENSES ", dep." CP_IDFACTURE_DEPENSES ", " CP_LIENFICHIER_FACTURES ", " CP_ECHEANCIER_FACTURES ", " CP_INTITULE_FACTURES ","
                        " idRubrique"
                        " FROM " TBL_DEPENSES " dep"
                        " left join " TBL_FACTURES " fac on dep." CP_IDFACTURE_DEPENSES " = fac." CP_ID_FACTURES
                        " left join " TBL_RUBRIQUES2035 " rub on dep." CP_REFFISCALE_DEPENSES " = rub.RefFiscale"
                        " WHERE dep." CP_IDUSER_DEPENSES " = " + QString::number(idUser);
    //qDebug() << req;
    QList<QVariantList> deplist = StandardSelectSQL(req,ok);
    if(!ok || deplist.size()==0)
        return depenses;
    for (int i=0; i<deplist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_DEPENSES]      = deplist.at(i).at(0).toInt();
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
        jData[CP_LIENFICHIER_FACTURES]    = deplist.at(i).at(12).toString();
        jData[CP_ECHEANCIER_FACTURES]     = (deplist.at(i).at(13).toInt()==1);
        jData[CP_INTITULE_FACTURES]       = deplist.at(i).at(14).toString();
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
    //qDebug() << req;
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
        jData[CP_ID_BANQUES] = banqlist.at(i).at(0).toInt();
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
    QString req = "SELECT " CP_ID_TIERS ", " CP_NOM_TIERS ", " CP_ADRESSE1_TIERS ", " CP_ADRESSE2_TIERS ", " CP_ADRESSE3_TIERS ", "
                            CP_CODEPOSTAL_TIERS ", " CP_VILLE_TIERS ", " CP_TELEPHONE_TIERS ", " CP_FAX_TIERS ", " CP_MAIL_TIERS ", " CP_WEB_TIERS " from " TBL_TIERS;
    QList<QVariantList> tierslist = StandardSelectSQL(req,ok);
    if(!ok || tierslist.size()==0)
        return listetiers;
    for (int i=0; i<tierslist.size(); ++i)
    {
        QJsonObject jData{};
        jData[CP_ID_TIERS]          = tierslist.at(i).at(0).toInt();
        jData[CP_NOM_TIERS]         = tierslist.at(i).at(1).toString();
        jData[CP_ADRESSE1_TIERS]    = tierslist.at(i).at(2).toString();
        jData[CP_ADRESSE2_TIERS]    = tierslist.at(i).at(3).toString();
        jData[CP_ADRESSE3_TIERS]    = tierslist.at(i).at(4).toString();
        jData[CP_CODEPOSTAL_TIERS]  = tierslist.at(i).at(5).toString();
        jData[CP_VILLE_TIERS]       = tierslist.at(i).at(6).toString();
        jData[CP_TELEPHONE_TIERS]   = tierslist.at(i).at(7).toString();
        jData[CP_FAX_TIERS]         = tierslist.at(i).at(8).toString();
        jData[CP_MAIL_TIERS]        = tierslist.at(i).at(9).toString();
        jData[CP_WEB_TIERS]         = tierslist.at(i).at(10).toString();
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
QList<Recette*> DataBase::loadRecettesByPeriod(QDate datedebut, QDate datefin)
{
    QList<Recette*> listerecettes;
        //---------------------------------------------- Tous les actes effectués par tout le monde durant la période, sauf les impayés et les gratuits
        QString req =
        "select res1.idActe, res1.actedate, res1.nom, res1.actecotation, res1.acteMontant, res1.actemonnaie, res1.TypePaiement,"
        " res1.Tiers, Paye, res1.iduser, res1.userparent, res1.usercomptable, null as montantautresrecettes, null as typeautresrecettes from\n "
        "(\n"
            "select\n"
            " act." CP_ID_ACTES ", " CP_DATE_ACTES ", concat(patnom, ' ', patprenom) as nom, " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES
            ", TypePaiement, Tiers, " CP_IDUSER_ACTES ", " CP_IDUSERPARENT_ACTES ", " CP_IDUSERCOMPTABLE_ACTES " from \n"
            TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ\n"
            " where act." CP_IDPAT_ACTES " = pat.idpat\n"
            " and act." CP_ID_ACTES " = typ.idacte\n"
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
            jData["date"] = recetteslist.at(i).at(1).toDate().toString("yyyy-MM-dd");
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
    jData[CP_ID_LIGNRECETTES]            = id;
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
                    " (select " CP_ID_ACTES " from " TBL_ACTES " where idpat = " + QString::number(pat->id()) + ")";
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
    QString req = "select lieux." CP_ID_SITE ", " CP_NOM_SITE ", " CP_ADRESSE1_SITE ", " CP_ADRESSE2_SITE ", " CP_ADRESSE3_SITE ", "
                  CP_CODEPOSTAL_SITE ", " CP_VILLE_SITE ", " CP_TELEPHONE_SITE ", " CP_FAX_SITE ", " CP_COULEUR_SITE ", iduser"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint." CP_IDLIEU_JOINTSITE " = lieux." CP_ID_SITE
                  " where " CP_IDUSER_JOINTSITE " = " + QString::number(iduser) +
                  " union"
                  " (select lieux." CP_ID_SITE ", " CP_NOM_SITE ", " CP_ADRESSE1_SITE ", " CP_ADRESSE2_SITE ", " CP_ADRESSE3_SITE ", "
                  CP_CODEPOSTAL_SITE ", " CP_VILLE_SITE ", " CP_TELEPHONE_SITE ", " CP_FAX_SITE ", " CP_COULEUR_SITE ", null as iduser"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint." CP_IDLIEU_JOINTSITE " = lieux." CP_ID_SITE
                  " where " CP_IDUSER_JOINTSITE " <> " + QString::number(iduser) + " and " CP_IDUSER_JOINTSITE " is not null"
                  " and lieux." CP_ID_SITE " not in"
                  " (select lieux." CP_ID_SITE
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint." CP_IDLIEU_JOINTSITE " = lieux." CP_ID_SITE
                  " where " CP_IDUSER_JOINTSITE " = " + QString::number(iduser) + ")";
                  " union"
                  " (select lieux." CP_ID_SITE ", " CP_NOM_SITE ", " CP_ADRESSE1_SITE ", " CP_ADRESSE2_SITE ", " CP_ADRESSE3_SITE ", "
                  CP_CODEPOSTAL_SITE ", " CP_VILLE_SITE ", " CP_TELEPHONE_SITE ", " CP_FAX_SITE ", " CP_COULEUR_SITE ", null as iduser"
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint." CP_IDLIEU_JOINTSITE " = lieux." CP_ID_SITE
                  " where " CP_IDUSER_JOINTSITE " is not null"
                  " and lieux." CP_ID_SITE " not in"
                  " (select lieux." CP_ID_SITE
                  " from " TBL_LIEUXEXERCICE " lieux left join " TBL_JOINTURESLIEUX " joint"
                  " on joint." CP_IDLIEU_JOINTSITE " = lieux." CP_ID_SITE
                  " where " CP_IDUSER_JOINTSITE " = " + QString::number(iduser) + ")";
    /*!< cette requête sert à recenser tous les lieux de travail avec le champ iduser positionné
     *  à l'id du user en cours s'il est utilisé par l'user en cours,
     *  à -1 s'il est utilisé par d'autres utilisateurs mais pas le user en cours
     *  et à null s'il n'est utilisé par personne
     * il doit y avoir moyen de faire plus simple mais je ne sais pas comment
     */
    //qDebug() << req;

    req = "select " CP_ID_SITE ", " CP_NOM_SITE ", " CP_ADRESSE1_SITE ", " CP_ADRESSE2_SITE ", " CP_ADRESSE3_SITE ", "
          CP_CODEPOSTAL_SITE ", " CP_VILLE_SITE ", " CP_TELEPHONE_SITE ", " CP_FAX_SITE ", " CP_COULEUR_SITE
          " from " TBL_LIEUXEXERCICE;
    return loadSites( req );
}
QList<int> DataBase::loadidSitesByUser(int idUser)
{
    QList<int> listid = QList<int>();
    QString req = "select " CP_IDLIEU_JOINTSITE " from " TBL_JOINTURESLIEUX " where " CP_IDUSER_JOINTSITE " = " + QString::number(idUser);
    QList<QVariantList> listidsites = StandardSelectSQL(req, ok);
    if (ok && listidsites.size()>0)
        for (int i=0; i<listidsites.size(); ++i)
            listid << listidsites.at(i).at(0).toInt();
    return listid;
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
        jEtab[CP_ID_SITE]           = sitlist.at(i).at(0).toInt();
        jEtab[CP_NOM_SITE]          = sitlist.at(i).at(1).toString();
        jEtab[CP_ADRESSE1_SITE]     = sitlist.at(i).at(2).toString();
        jEtab[CP_ADRESSE2_SITE]     = sitlist.at(i).at(3).toString();
        jEtab[CP_ADRESSE3_SITE]     = sitlist.at(i).at(4).toString();
        jEtab[CP_CODEPOSTAL_SITE]   = sitlist.at(i).at(5).toInt();
        jEtab[CP_VILLE_SITE]        = sitlist.at(i).at(6).toString();
        jEtab[CP_TELEPHONE_SITE]    = sitlist.at(i).at(7).toString();
        jEtab[CP_FAX_SITE]          = sitlist.at(i).at(8).toString();
        jEtab[CP_COULEUR_SITE]      = sitlist.at(i).at(9).toString();
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
QString DataBase::getMDPAdmin()
{
    QString mdp ("");
    QVariantList mdpdata = getFirstRecordFromStandardSelectSQL("select mdpadmin from " TBL_PARAMSYSTEME,ok);
    if( !ok || mdpdata.size()==0 )
        StandardSQL("update " TBL_PARAMSYSTEME " set mdpadmin = '" + Utils::calcSHA1(MDP_ADMINISTRATEUR) + "'");
    else if (mdpdata.at(0) == "")
        StandardSQL("update " TBL_PARAMSYSTEME " set mdpadmin = '" + Utils::calcSHA1(MDP_ADMINISTRATEUR) + "'");
    return (mdpdata.at(0).toString() != ""? mdpdata.at(0).toString() : Utils::calcSHA1(MDP_ADMINISTRATEUR));
}


/*
 * Actes
*/
QJsonObject DataBase::loadActeData(QVariantList actdata)
{
    QJsonObject data{};
    data[CP_ID_ACTES] = actdata.at(0).toInt();
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
        data[CP_TYPEPAIEMENT_TYPEPAIEMENTACTES] = "";
    else
        data[CP_TYPEPAIEMENT_TYPEPAIEMENTACTES] = actdata.at(14).toString();

    if( actdata.at(15).isNull() )
        data[CP_TIERS_TYPEPAIEMENTACTES] = "";
    else
        data[CP_TIERS_TYPEPAIEMENTACTES] = actdata.at(15).toString();
    data[CP_NUMCENTRE_ACTES] = actdata.at(16).toInt();
    data[CP_IDLIEU_ACTES] = actdata.at(17).toInt();
    data[CP_HEURE_ACTES] = actdata.at(18).toTime().toString("HH:mm:ss");
    data[CP_SUPERVISEURREMPLACANT_ACTES] = (actdata.at(19) == 1);
    data[CP_ID_LIGNPRGOPERATOIRE] = actdata.at(20).toInt();

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
    QString req = "SELECT act." CP_ID_ACTES ", act." CP_IDPAT_ACTES ", act." CP_IDUSER_ACTES ",  act." CP_DATE_ACTES ", act." CP_MOTIF_ACTES ","
                  " act." CP_TEXTE_ACTES ", act." CP_CONCLUSION_ACTES ", act." CP_COURRIERAFAIRE_ACTES ", act." CP_COTATION_ACTES ", act." CP_MONTANT_ACTES ","
                  " act." CP_MONNAIE_ACTES ", act." CP_IDUSERCREATEUR_ACTES ", act." CP_IDUSERCOMPTABLE_ACTES ", act." CP_IDUSERPARENT_ACTES ","
                  " tpm." CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", tpm." CP_TIERS_TYPEPAIEMENTACTES ", " CP_NUMCENTRE_ACTES ", " CP_IDLIEU_ACTES ", act." CP_HEURE_ACTES ","
                  " act." CP_SUPERVISEURREMPLACANT_ACTES ", lign." CP_ID_LIGNPRGOPERATOIRE
                  " FROM " TBL_ACTES " act "
                  " LEFT JOIN " TBL_TYPEPAIEMENTACTES " tpm on tpm." CP_IDACTE_TYPEPAIEMENTACTES " = act." CP_ID_ACTES
                  " LEFT JOIN " TBL_LIGNESPRGOPERATOIRES " lign on lign." CP_IDACTE_LIGNPRGOPERATOIRE " = act." CP_ID_ACTES
                  " WHERE act." CP_ID_ACTES " = '" + QString::number(idActe) + "'";
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
    QString req = "SELECT act." CP_ID_ACTES ", act." CP_IDPAT_ACTES ", act." CP_IDUSER_ACTES ",  act." CP_DATE_ACTES ", act." CP_MOTIF_ACTES ","
                  " act." CP_TEXTE_ACTES ", act." CP_CONCLUSION_ACTES ", act." CP_COURRIERAFAIRE_ACTES ", act." CP_COTATION_ACTES ", act." CP_MONTANT_ACTES ","
                  " act." CP_MONNAIE_ACTES ", act." CP_IDUSERCREATEUR_ACTES ", act." CP_IDUSERCOMPTABLE_ACTES ", act." CP_IDUSERPARENT_ACTES ","
                  " tpm." CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", tpm." CP_TIERS_TYPEPAIEMENTACTES ", act." CP_NUMCENTRE_ACTES ", " CP_IDLIEU_ACTES ", act." CP_HEURE_ACTES ","
                  " act." CP_SUPERVISEURREMPLACANT_ACTES ", lign." CP_ID_LIGNPRGOPERATOIRE
                  " FROM " TBL_ACTES " act "
                  " LEFT JOIN " TBL_TYPEPAIEMENTACTES " tpm on tpm." CP_IDACTE_TYPEPAIEMENTACTES " = act." CP_ID_ACTES
                  " LEFT JOIN " TBL_LIGNESPRGOPERATOIRES " lign on lign." CP_IDACTE_LIGNPRGOPERATOIRE " = act." CP_ID_ACTES
                  " WHERE act." CP_IDPAT_ACTES " = '" + QString::number(pat->id()) + "' "
                  " ORDER BY act." CP_ID_ACTES " DESC";
    QList<QVariantList> actlist = StandardSelectSQL(req,ok);
    //qDebug() << req;
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
        data[CP_ID_ACTES] = actlist.at(i).at(0).toInt();
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
    QString req = " SELECT lp." CP_PAYE_LIGNEPAIEMENT ", lr." CP_MONNAIE_LIGNRECETTES
                  " FROM " TBL_LIGNESPAIEMENTS " lp "
                  " LEFT JOIN " TBL_RECETTES " lr on lp." CP_IDRECETTE_LIGNEPAIEMENT " = lr." CP_ID_LIGNRECETTES
                  " WHERE " CP_IDACTE_LIGNEPAIEMENT " = " + QString::number(idActe);
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
int DataBase::getidActeCorrespondant(int idpat, QDate date)
{
    QString req = "select " CP_ID_ACTES " from " TBL_ACTES " where " CP_IDPAT_ACTES " = " + QString::number(idpat) + " and " CP_DATE_ACTES " = '" + date.toString("yyyy-MM-dd") + "'";
    //qDebug() << req;
    QVariantList listid = getFirstRecordFromStandardSelectSQL(req, ok, tr("Impossible de retrouver un acte correspondant"));
    if (listid.size() > 0)
        return listid.at(0).toInt();
    else return 0;
}


/*
 * Refractions
*/

QJsonObject DataBase::loadRefractionData(QVariantList refdata)           //! attribue la liste des datas à une refraction
{
    QJsonObject data{};
    data[CP_ID_REFRACTIONS]                 = refdata.at(0).toInt();
    data[CP_IDPAT_REFRACTIONS]              = refdata.at(1).toInt();
    data[CP_IDACTE_REFRACTIONS]             = refdata.at(2).toInt();
    data[CP_DATE_REFRACTIONS]               = refdata.at(3).toDate().toString("yyyy-MM-dd");
    data[CP_TYPEMESURE_REFRACTIONS]         = refdata.at(4).toString();
    data[CP_DISTANCEMESURE_REFRACTIONS]     = refdata.at(5).toString();
    if (refdata.at(6) != QVariant())
        data[CP_CYCLOPLEGIE_REFRACTIONS]    = (refdata.at(6).toInt() == 1);
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
    if (refdata.at(17) != QVariant())
        data[CP_PRESSONOD_REFRACTIONS]      = (refdata.at(17).toInt() == 1);
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
    if (refdata.at(32) != QVariant())
        data[CP_PRESSONOG_REFRACTIONS]      = (refdata.at(32).toInt() == 1);
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
    data["isODmesure"]                      = (refdata.at(8) != QVariant());
    data["isOGmesure"]                      = (refdata.at(23) != QVariant());
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

/*
 * Commentaires lunettes
*/

QJsonObject DataBase::loadCommentLunetData(QVariantList comdata)           //! attribue la liste des datas à un commentaire lunette
{
    QJsonObject data{};
    data[CP_ID_COMLUN]                  = comdata.at(0).toInt();
    data[CP_TEXT_COMLUN]                = comdata.at(1).toString();
    data[CP_RESUME_COMLUN]              = comdata.at(2).toString();
    data[CP_IDUSER_COMLUN]              = comdata.at(3).toInt();
    data[CP_PARDEFAUT_COMLUN]           = (comdata.at(4).toInt() == 1);
    data[CP_PUBLIC_COMLUN]              = (comdata.at(5).toInt() == 1);
    return data;
}

CommentLunet* DataBase::loadCommentLunetById(int id)                 //! charge un commentaire lunette
{
    bool ok;
    CommentLunet* com = Q_NULLPTR;
    QString req = "SELECT " CP_ID_COMLUN ", " CP_TEXT_COMLUN ", " CP_RESUME_COMLUN ", " CP_IDUSER_COMLUN ", " CP_PARDEFAUT_COMLUN ", " CP_PUBLIC_COMLUN " FROM " TBL_COMMENTAIRESLUNETTES
            " WHERE " CP_ID_COMLUN " = " + QString::number(id)
            + " order by " CP_RESUME_COMLUN;
    QVariantList comdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || comdata.size()==0)
        return com;
    QJsonObject data = loadCommentLunetData(comdata);
    com = new CommentLunet(data);
    return com;
}

QList<CommentLunet*> DataBase::loadCommentsLunetsByListidUser(QList<int> listid)        //! charge les commentaires utilisés par un groupe d'utilisateurs
{
    QList<CommentLunet*> listcom = QList<CommentLunet*>();
    if (listid.size() == 0)
        return listcom;
    QString req = "SELECT " CP_ID_COMLUN ", " CP_TEXT_COMLUN ", " CP_RESUME_COMLUN ", " CP_IDUSER_COMLUN ", " CP_PARDEFAUT_COMLUN ", " CP_PUBLIC_COMLUN " FROM " TBL_COMMENTAIRESLUNETTES
            " WHERE " CP_IDUSER_COMLUN " = " + QString::number(listid.at(0));
    if (listid.size()>1)
    {
        for (int i=1; i<listid.size(); ++i)
            req += " OR " CP_IDUSER_COMLUN " = " + QString::number(listid.at(i));
        req += " order by " CP_RESUME_COMLUN;
    }
    //qDebug() << req;
    QList<QVariantList> listdata = StandardSelectSQL(req,ok);
    if(!ok || listdata.size()==0)
        return listcom;
    for (int i=0; i<listdata.size(); ++i)
    {
        QJsonObject data = loadCommentLunetData(listdata.at(i));
        CommentLunet *com = new CommentLunet(data);
        if (com)
            listcom << com;
    }
    return listcom;
}

/*
 * Sessions opératoires
*/

QJsonObject DataBase::loadSessionOpData(QVariantList sessiondata)           //! attribue la liste des datas d'une session opératoire
{
    QJsonObject data{};
    data[CP_ID_SESSIONOPERATOIRE]                   = sessiondata.at(0).toInt();
    data[CP_DATE_SESSIONOPERATOIRE]                 = sessiondata.at(1).toDate().toString("yyyy-MM-dd");
    data[CP_IDUSER_SESSIONOPERATOIRE]               = sessiondata.at(2).toInt();
    data[CP_IDAIDE_SESSIONOPERATOIRE]               = sessiondata.at(3).toInt();
    data[CP_IDLIEU_SESSIONOPERATOIRE]               = sessiondata.at(4).toInt();
    data[CP_INCIDENT_SESSIONOPERATOIRE]             = sessiondata.at(5).toString();
    return data;
}

QList<SessionOperatoire*> DataBase::loadSessionsOpByUserId(int id)                  //! charge toutes les sessions opératoires d'un user
{
    QList<SessionOperatoire*> list = QList<SessionOperatoire*> ();
    QString req =   "SELECT " CP_ID_SESSIONOPERATOIRE ", " CP_DATE_SESSIONOPERATOIRE ", " CP_IDUSER_SESSIONOPERATOIRE ", " CP_IDAIDE_SESSIONOPERATOIRE ", " CP_IDLIEU_SESSIONOPERATOIRE ", " CP_INCIDENT_SESSIONOPERATOIRE  // 0-1-2-3-4
                    " FROM " TBL_SESSIONSOPERATOIRES
                    " WHERE " CP_IDUSER_SESSIONOPERATOIRE " = " + QString::number(id) +
                    " order by " CP_DATE_SESSIONOPERATOIRE " asc";
    QList<QVariantList> interventionlist = StandardSelectSQL(req,ok);

  if(!ok || interventionlist.size()==0)
        return list;
    for (int i=0; i<interventionlist.size(); ++i)
    {
        QVariantList sessiondata = interventionlist.at(i);
        QJsonObject data = loadSessionOpData(sessiondata);
        SessionOperatoire *session = new SessionOperatoire(data);
        if (session != Q_NULLPTR)
            list << session;
    }
    return list;
}

SessionOperatoire* DataBase::loadSessionOpById(int idsession)                   //! charge une Intervention définie par son id - utilisé pour renouveler les données en cas de modification
{
    SessionOperatoire *session = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_SESSIONOPERATOIRE ", " CP_DATE_SESSIONOPERATOIRE ", " CP_IDUSER_SESSIONOPERATOIRE ", " CP_IDAIDE_SESSIONOPERATOIRE ", " CP_IDLIEU_SESSIONOPERATOIRE ", " CP_INCIDENT_SESSIONOPERATOIRE // 0-1-2-3-4
                    " FROM " TBL_SESSIONSOPERATOIRES
                    " WHERE " CP_ID_SESSIONOPERATOIRE " = " + QString::number(idsession) ;
    QVariantList sessiondata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || sessiondata.size()==0)
        return session;
    QJsonObject data = loadSessionOpData(sessiondata);
    session= new SessionOperatoire(data);
    return session;
}

/*
 * Interventions
*/

QJsonObject DataBase::loadInterventionData(QVariantList interventiondata)           //! attribue la liste des datas à une intervention
{
    QJsonObject data{};
    data[CP_ID_LIGNPRGOPERATOIRE]                   = interventiondata.at(0).toInt();
    data[CP_HEURE_LIGNPRGOPERATOIRE]                = interventiondata.at(1).toTime().toString("HH:mm:ss");
    data[CP_IDPATIENT_LIGNPRGOPERATOIRE]            = interventiondata.at(2).toInt();
    data[CP_IDSESSION_LIGNPRGOPERATOIRE]            = interventiondata.at(3).toInt();
    data[CP_TYPEANESTH_LIGNPRGOPERATOIRE]           = interventiondata.at(4).toString();
    data[CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE]   = interventiondata.at(5).toInt();
    data[CP_COTE_LIGNPRGOPERATOIRE]                 = interventiondata.at(6).toString();
    data[CP_IDIOL_LIGNPRGOPERATOIRE]                = interventiondata.at(7).toInt();
    data[CP_PWRIOL_LIGNPRGOPERATOIRE]               = interventiondata.at(8).toDouble();
    data[CP_CYLIOL_LIGNPRGOPERATOIRE]               = interventiondata.at(9).toDouble();
    data[CP_OBSERV_LIGNPRGOPERATOIRE]               = interventiondata.at(10).toString();
    data[CP_IDACTE_LIGNPRGOPERATOIRE]               = interventiondata.at(11).toInt();
    data[CP_INCIDENT_LIGNPRGOPERATOIRE]             = interventiondata.at(12).toString();
    return data;
}

QList<Intervention*> DataBase::loadInterventionsBySessionId(int id)                  //! charge toutes les Interventions d'un patient
{
    QList<Intervention*> list = QList<Intervention*> ();
    QString req =   "SELECT " CP_ID_LIGNPRGOPERATOIRE ", " CP_HEURE_LIGNPRGOPERATOIRE ", " CP_IDPATIENT_LIGNPRGOPERATOIRE ", " CP_IDSESSION_LIGNPRGOPERATOIRE ", " CP_TYPEANESTH_LIGNPRGOPERATOIRE ", "  // 0-1-2-3-4
                              CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE ", " CP_COTE_LIGNPRGOPERATOIRE ", " CP_IDIOL_LIGNPRGOPERATOIRE ", " CP_PWRIOL_LIGNPRGOPERATOIRE ", " CP_CYLIOL_LIGNPRGOPERATOIRE ", " // 5-6-7-8-9
                              CP_OBSERV_LIGNPRGOPERATOIRE ", " CP_IDACTE_LIGNPRGOPERATOIRE ", " CP_INCIDENT_LIGNPRGOPERATOIRE // 10-11-12
                    " FROM " TBL_LIGNESPRGOPERATOIRES
                    " WHERE " CP_IDSESSION_LIGNPRGOPERATOIRE " = " + QString::number(id) +
                    " order by " CP_HEURE_LIGNPRGOPERATOIRE " asc";
    QList<QVariantList> interventionlist = StandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || interventionlist.size()==0)
        return list;
    for (int i=0; i<interventionlist.size(); ++i)
    {
        QJsonObject data = loadInterventionData(interventionlist.at(i));
        Intervention *intervention = new Intervention(data);
        if (intervention != Q_NULLPTR)
            list << intervention;
    }
    return list;
}

Intervention* DataBase::loadInterventionById(int idintervention)                   //! charge une Intervention définie par son id - utilisé pour renouveler les données en cas de modification
{
    Intervention *intervention = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_LIGNPRGOPERATOIRE ", " CP_HEURE_LIGNPRGOPERATOIRE ", " CP_IDPATIENT_LIGNPRGOPERATOIRE ", " CP_IDSESSION_LIGNPRGOPERATOIRE ", " CP_TYPEANESTH_LIGNPRGOPERATOIRE ", "  // 0-1-2-3-4
                              CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE ", " CP_COTE_LIGNPRGOPERATOIRE ", " CP_IDIOL_LIGNPRGOPERATOIRE ", " CP_PWRIOL_LIGNPRGOPERATOIRE ", " CP_CYLIOL_LIGNPRGOPERATOIRE ", " // 5-6-7-8-9
                              CP_OBSERV_LIGNPRGOPERATOIRE ", " CP_IDACTE_LIGNPRGOPERATOIRE ", " CP_INCIDENT_LIGNPRGOPERATOIRE // 10-11-12
                    " FROM " TBL_LIGNESPRGOPERATOIRES ;
                    " WHERE " CP_ID_LIGNPRGOPERATOIRE " = " + QString::number(idintervention) ;
    QVariantList interventiondata = getFirstRecordFromStandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || interventiondata.size()==0)
        return intervention;
    QJsonObject data = loadInterventionData(interventiondata);
    intervention = new Intervention(data);
    return intervention;
}

Intervention* DataBase::loadInterventionByDateIdPatient(QDate date, int idpatient)  //! charge une Intervention définie par sa date et l'iddu patietnt
{
    Intervention *intervention = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_LIGNPRGOPERATOIRE ", " CP_HEURE_LIGNPRGOPERATOIRE ", " CP_IDPATIENT_LIGNPRGOPERATOIRE ", lign." CP_IDSESSION_LIGNPRGOPERATOIRE ", " CP_TYPEANESTH_LIGNPRGOPERATOIRE ", "  // 0-1-2-3-4
                              CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE ", " CP_COTE_LIGNPRGOPERATOIRE ", " CP_IDIOL_LIGNPRGOPERATOIRE ", " CP_PWRIOL_LIGNPRGOPERATOIRE ", " CP_CYLIOL_LIGNPRGOPERATOIRE ", " // 5-6-7-8-9
                              CP_OBSERV_LIGNPRGOPERATOIRE ", " CP_IDACTE_LIGNPRGOPERATOIRE ", lign." CP_INCIDENT_LIGNPRGOPERATOIRE ", " CP_DATE_SESSIONOPERATOIRE // 10-11-12
                    " FROM " TBL_LIGNESPRGOPERATOIRES " lign, " TBL_SESSIONSOPERATOIRES " sess "
                    " WHERE lign." CP_IDSESSION_LIGNPRGOPERATOIRE " = sess." CP_ID_SESSIONOPERATOIRE
                    " AND " CP_IDPATIENT_LIGNPRGOPERATOIRE " = " + QString::number(idpatient) +
                    " AND " CP_DATE_SESSIONOPERATOIRE " = '" + date.toString("yyyy-MM-dd") + "'" ;
    QVariantList interventiondata = getFirstRecordFromStandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || interventiondata.size()==0)
        return intervention;
    QJsonObject data = loadInterventionData(interventiondata);
    intervention = new Intervention(data);
    return intervention;
}

/*
 * IOLs
*/

QJsonObject DataBase::loadIOLData(QVariantList ioldata)                     //! attribue la liste des datas à un IOL
{
    QJsonObject data{};
    data[CP_ID_IOLS]                = ioldata.at(0).toInt();
    data[CP_IDMANUFACTURER_IOLS]    = ioldata.at(1).toInt();
    data[CP_MODELNAME_IOLS]         = ioldata.at(2).toString();
    data[CP_DIAOPT_IOLS]            = ioldata.at(3).toDouble();
    data[CP_DIAALL_IOLS]            = ioldata.at(4).toDouble();
    data[CP_ACD_IOLS]               = ioldata.at(5).toDouble();
    data[CP_MINPWR_IOLS]            = ioldata.at(6).toDouble();
    data[CP_MAXPWR_IOLS]            = ioldata.at(7).toDouble();
    data[CP_PWRSTEP_IOLS]           = ioldata.at(8).toDouble();
    data[CP_MINCYL_IOLS]            = ioldata.at(9).toDouble();
    data[CP_MAXCYL_IOLS]            = ioldata.at(10).toDouble();
    data[CP_CYLSTEP_IOLS]           = ioldata.at(11).toDouble();
    data[CP_CSTEAOPT_IOLS]          = ioldata.at(12).toDouble();
    data[CP_CSTEAECHO_IOLS]         = ioldata.at(13).toDouble();
    data[CP_HAIGISA0_IOLS]          = ioldata.at(14).toDouble();
    data[CP_HAIGISA1_IOLS]          = ioldata.at(15).toDouble();
    data[CP_HAIGISA2_IOLS]          = ioldata.at(16).toDouble();
    data[CP_HOLL1_IOLS]             = ioldata.at(17).toDouble();
    data[CP_DIAINJECTEUR_IOLS]      = ioldata.at(18).toDouble();
    data[CP_ARRAYIMG_IOLS]          = QLatin1String(ioldata.at(19).toByteArray().toBase64());
    data[CP_MATERIAU_IOLS]          = ioldata.at(20).toString();
    data[CP_REMARQUE_IOLS]          = ioldata.at(21).toString();
    data[CP_INACTIF_IOLS]           = (ioldata.at(22) == 1);
    data[CP_PRECHARGE_IOLS]         = (ioldata.at(23) == 1);
    data[CP_JAUNE_IOLS]             = (ioldata.at(24) == 1);
    data[CP_MULTIFOCAL_IOLS]        = (ioldata.at(25) == 1);
    data[CP_TYPIMG_IOLS]            = ioldata.at(26).toString();
    data[CP_EDOF_IOLS]              = (ioldata.at(27) == 1);
    data[CP_TORIC_IOLS]             = (ioldata.at(28) == 1);
    data[CP_TYP_IOLS]               = ioldata.at(29).toInt();
    return data;
}

QList<IOL*> DataBase::loadIOLs()                                            //! charge tous les IOLS
{
    QString reqdel = "delete from " TBL_IOLS " where " CP_MODELNAME_IOLS " is null or " CP_MODELNAME_IOLS " = \"\""
                     " or " CP_IDMANUFACTURER_IOLS " is null or " CP_IDMANUFACTURER_IOLS " not in (select " CP_ID_MANUFACTURER " from " TBL_MANUFACTURERS ")";
    //qDebug() << reqdel;
    StandardSQL(reqdel, "erreu del");
    QList<IOL*> list = QList<IOL*> ();
    QString req =   "SELECT " CP_ID_IOLS ", " CP_IDMANUFACTURER_IOLS ", " CP_MODELNAME_IOLS ", " CP_DIAOPT_IOLS ", " CP_DIAALL_IOLS", "         // 0-1-2-3-4
                    CP_ACD_IOLS ", " CP_MINPWR_IOLS ", " CP_MAXPWR_IOLS ", " CP_PWRSTEP_IOLS ", " CP_MINCYL_IOLS ", "                           // 5-6-7-8-9
                    CP_MAXCYL_IOLS ", " CP_CYLSTEP_IOLS ", " CP_CSTEAOPT_IOLS ", " CP_CSTEAECHO_IOLS ", " CP_HAIGISA0_IOLS ", "                 // 10-11-12-13-14
                    CP_HAIGISA1_IOLS ", " CP_HAIGISA2_IOLS ", " CP_HOLL1_IOLS ", " CP_DIAINJECTEUR_IOLS ", " CP_ARRAYIMG_IOLS ", "                   // 15-16-17-18-19
                    CP_MATERIAU_IOLS ", " CP_REMARQUE_IOLS ", " CP_INACTIF_IOLS ", " CP_PRECHARGE_IOLS ", " CP_JAUNE_IOLS ", "                  // 20-21-22-23-24
                    CP_MULTIFOCAL_IOLS ", " CP_TYPIMG_IOLS ", " CP_EDOF_IOLS ", " CP_TORIC_IOLS ", " CP_TYP_IOLS                                // 25-26-27-28-29
                    " FROM " TBL_IOLS
                    " order by " CP_IDMANUFACTURER_IOLS;
    QList<QVariantList> iollist = StandardSelectSQL(req,ok);
    if(!ok || iollist.size()==0)
        return list;
    for (int i=0; i<iollist.size(); ++i)
    {
        QJsonObject data = loadIOLData(iollist.at(i));
        IOL *iol = new IOL(data);
        if (iol != Q_NULLPTR)
            list << iol;
    }
    return list;
}

QList<IOL*> DataBase::loadIOLsByManufacturerId(int id)                       //! charge tous les IOLS d'un fabricant
{
    QList<IOL*> list = QList<IOL*> ();
    QString req =   "SELECT " CP_ID_IOLS ", " CP_IDMANUFACTURER_IOLS ", " CP_MODELNAME_IOLS ", " CP_DIAOPT_IOLS ", " CP_DIAALL_IOLS", "         // 0-1-2-3-4
                    CP_ACD_IOLS ", " CP_MINPWR_IOLS ", " CP_MAXPWR_IOLS ", " CP_PWRSTEP_IOLS ", " CP_MINCYL_IOLS ", "                           // 5-6-7-8-9
                    CP_MAXCYL_IOLS ", " CP_CYLSTEP_IOLS ", " CP_CSTEAOPT_IOLS ", " CP_CSTEAECHO_IOLS ", " CP_HAIGISA0_IOLS ", "                 // 10-11-12-13-14
                    CP_HAIGISA1_IOLS ", " CP_HAIGISA2_IOLS ", " CP_HOLL1_IOLS ", " CP_DIAINJECTEUR_IOLS ", " CP_ARRAYIMG_IOLS ", "                   // 15-16-17-18-19
                    CP_MATERIAU_IOLS ", " CP_REMARQUE_IOLS ", " CP_INACTIF_IOLS ", " CP_PRECHARGE_IOLS ", " CP_JAUNE_IOLS ", "                  // 20-21-22-23-24
                    CP_MULTIFOCAL_IOLS ", " CP_TYPIMG_IOLS ", " CP_EDOF_IOLS ", " CP_TORIC_IOLS ", " CP_TYP_IOLS                                // 25-26-27-28-29
                    " FROM " TBL_IOLS
                    " where " CP_IDMANUFACTURER_IOLS " = " + QString::number(id) +
                    " order by " CP_IDMANUFACTURER_IOLS;
    //qDebug() << req;
    QList<QVariantList> iollist = StandardSelectSQL(req,ok);
    if(!ok || iollist.size()==0)
        return list;
    for (int i=0; i<iollist.size(); ++i)
    {
        QJsonObject data = loadIOLData(iollist.at(i));
        IOL *iol = new IOL(data);
        if (iol != Q_NULLPTR)
            list << iol;
    }
    return list;
}

IOL* DataBase::loadIOLById(int idiol)                   //! charge un IOL défini par son id - utilisé pour renouveler les données en cas de modification
{
    QString reqdel = "delete from " TBL_IOLS " where " CP_MODELNAME_IOLS " is null or " CP_MODELNAME_IOLS " = \"\""
                     " or " CP_IDMANUFACTURER_IOLS " is null or " CP_IDMANUFACTURER_IOLS " not in (select " CP_ID_MANUFACTURER " from " TBL_MANUFACTURERS ")";
    //qDebug() << reqdel;
    StandardSQL(reqdel, "");
    IOL *iol = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_IOLS ", " CP_IDMANUFACTURER_IOLS ", " CP_MODELNAME_IOLS ", " CP_DIAOPT_IOLS ", " CP_DIAALL_IOLS", "         // 0-1-2-3-4
                    CP_ACD_IOLS ", " CP_MINPWR_IOLS ", " CP_MAXPWR_IOLS ", " CP_PWRSTEP_IOLS ", " CP_MINCYL_IOLS ", "                           // 5-6-7-8-9
                    CP_MAXCYL_IOLS ", " CP_CYLSTEP_IOLS ", " CP_CSTEAOPT_IOLS ", " CP_CSTEAECHO_IOLS ", " CP_HAIGISA0_IOLS ", "                 // 10-11-12-13-14
                    CP_HAIGISA1_IOLS ", " CP_HAIGISA2_IOLS ", " CP_HOLL1_IOLS ", " CP_DIAINJECTEUR_IOLS ", " CP_ARRAYIMG_IOLS ", "              // 15-16-17-18-19
                    CP_MATERIAU_IOLS ", " CP_REMARQUE_IOLS ", " CP_INACTIF_IOLS ", " CP_PRECHARGE_IOLS ", " CP_JAUNE_IOLS ", "                  // 20-21-22-23-24
                    CP_MULTIFOCAL_IOLS ", " CP_TYPIMG_IOLS ", " CP_EDOF_IOLS ", " CP_TORIC_IOLS ", " CP_TYP_IOLS                                // 25-26-27-28-29
                    " FROM " TBL_IOLS
                    " WHERE " CP_ID_IOLS " = " + QString::number(idiol) ;
    //qDebug() << req;
    QVariantList ioldata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || ioldata.size()==0)
        return iol;
    return new IOL(loadIOLData(ioldata));
}

void DataBase::UpDateIOL(int id, QHash<QString, QVariant> sets)
{
    UpdateTable(TBL_IOLS, sets, " where " CP_ID_IOLS " = " + QString::number(id),tr("Impossible de modifier l'IOL"));

    QByteArray ba = sets[CP_ARRAYIMG_IOLS].toByteArray();
    QSqlQuery query = QSqlQuery(m_db);
    QString prepare = "update " TBL_IOLS " set " CP_ARRAYIMG_IOLS " = :" CP_ARRAYIMG_IOLS " where " CP_ID_IOLS " = " + QString::number(id);
    query.prepare(prepare);
    query.bindValue(":" CP_ARRAYIMG_IOLS, ba);
    query.exec();
    if (query.lastError().type() != QSqlError::NoError)
        Logs::ERROR("erreur", tr("\nErreur\n") + query.lastError().text());
    query.finish();
}

/*
 * TypeInterventions
*/

QJsonObject DataBase::loadTypeInterventionData(QVariantList typeinterventiondata)         //! attribue la liste des datas à un TypeIntervention
{
    QJsonObject data{};
    data[CP_ID_TYPINTERVENTION]                 = typeinterventiondata.at(0).toInt();
    data[CP_TYPEINTERVENTION_TYPINTERVENTION]   = typeinterventiondata.at(1).toString();
    data[CP_CODECCAM_TYPINTERVENTION]           = typeinterventiondata.at(2).toString();
    return data;
}

QList<TypeIntervention*> DataBase::loadTypeInterventions()                       //! charge tous les TypeInterventions
{
    QList<TypeIntervention*> list = QList<TypeIntervention*> ();
    QString req =   "SELECT " CP_ID_TYPINTERVENTION ", " CP_TYPEINTERVENTION_TYPINTERVENTION ", " CP_CODECCAM_TYPINTERVENTION // 0-1-2
                    " FROM " TBL_TYPESINTERVENTIONS " order by " CP_ID_TYPINTERVENTION;
    QList<QVariantList> typeinterventionlist = StandardSelectSQL(req,ok);
    if(!ok || typeinterventionlist.size()==0)
        return list;
    for (int i=0; i<typeinterventionlist.size(); ++i)
    {
        QJsonObject data = loadTypeInterventionData(typeinterventionlist.at(i));
        TypeIntervention *typeintervention = new TypeIntervention(data);
        if (typeintervention != Q_NULLPTR)
            list << typeintervention;
    }
    return list;
}

TypeIntervention* DataBase::loadTypeInterventionById(int idtypeintervention)                   //! charge un TypeIntervention défini par son id - utilisé pour renouveler les données en cas de modification
{
    TypeIntervention *typeintervention = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_TYPINTERVENTION ", " CP_TYPEINTERVENTION_TYPINTERVENTION ", " CP_CODECCAM_TYPINTERVENTION // 0-1-2
                    " FROM " TBL_TYPESINTERVENTIONS
                    " WHERE " CP_ID_TYPINTERVENTION " = " + QString::number(idtypeintervention) ;
    QVariantList typeinterventiondata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || typeinterventiondata.size()==0)
        return typeintervention;
    QJsonObject data = loadTypeInterventionData(typeinterventiondata);
    typeintervention = new TypeIntervention(data);
    return typeintervention;
}

/*
 * Manufacturers
*/

QJsonObject DataBase::loadManufacturerData(QVariantList Mandata)         //! attribue la liste des datas à un fabricant
{
    QJsonObject data{};
    data[CP_ID_MANUFACTURER]            = Mandata.at(0).toInt();
    data[CP_NOM_MANUFACTURER]           = Mandata.at(1).toString();
    data[CP_ADRESSE1_MANUFACTURER]      = Mandata.at(2).toString();
    data[CP_ADRESSE2_MANUFACTURER]      = Mandata.at(3).toString();
    data[CP_ADRESSE3_MANUFACTURER]      = Mandata.at(4).toString();
    data[CP_CODEPOSTAL_MANUFACTURER]    = Mandata.at(5).toString();
    data[CP_VILLE_MANUFACTURER]         = Mandata.at(6).toString();
    data[CP_TELEPHONE_MANUFACTURER]     = Mandata.at(7).toString();
    data[CP_FAX_MANUFACTURER]           = Mandata.at(8).toString();
    data[CP_PORTABLE_MANUFACTURER]      = Mandata.at(9).toString();
    data[CP_WEBSITE_MANUFACTURER]       = Mandata.at(10).toString();
    data[CP_MAIL_MANUFACTURER]          = Mandata.at(11).toString();
    data[CP_INACTIF_MANUFACTURER]       = (Mandata.at(12).toInt() == 1);
    data[CP_DISTRIBUEPAR_MANUFACTURER]  = Mandata.at(13).toInt();
    data[CP_IDRUFUS_MANUFACTURER]       = Mandata.at(14).toInt();
    return data;
}

QList<Manufacturer*> DataBase::loadManufacturers()                       //! charge tous les fabricants
{
    QList<Manufacturer*> list = QList<Manufacturer*> ();
    QString req =   "SELECT " CP_ID_MANUFACTURER ", " CP_NOM_MANUFACTURER ", " CP_ADRESSE1_MANUFACTURER ", " CP_ADRESSE2_MANUFACTURER ", " CP_ADRESSE3_MANUFACTURER ", "
                              CP_CODEPOSTAL_MANUFACTURER ", " CP_VILLE_MANUFACTURER ", " CP_TELEPHONE_MANUFACTURER ", " CP_FAX_MANUFACTURER ", " CP_PORTABLE_MANUFACTURER ", " CP_WEBSITE_MANUFACTURER ", "
                              CP_MAIL_MANUFACTURER ", " CP_INACTIF_MANUFACTURER ", " CP_DISTRIBUEPAR_MANUFACTURER ", " CP_IDRUFUS_MANUFACTURER
                    " FROM " TBL_MANUFACTURERS " order by " CP_NOM_MANUFACTURER;
    QList<QVariantList> Manufacturerlist = StandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || Manufacturerlist.size()==0)
        return list;
    for (int i=0; i<Manufacturerlist.size(); ++i)
    {
        QJsonObject data = loadManufacturerData(Manufacturerlist.at(i));
        Manufacturer *Man = new Manufacturer(data);
        if (Man != Q_NULLPTR)
            list << Man;
    }
    return list;
}

Manufacturer* DataBase::loadManufacturerById(int idManufacturer)                   //! charge un fabricant défini par son id - utilisé pour renouveler les données en cas de modification
{
    Manufacturer *Man = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_MANUFACTURER ", " CP_NOM_MANUFACTURER ", " CP_ADRESSE1_MANUFACTURER ", " CP_ADRESSE2_MANUFACTURER ", " CP_ADRESSE3_MANUFACTURER ", "
                              CP_CODEPOSTAL_MANUFACTURER ", " CP_VILLE_MANUFACTURER ", " CP_TELEPHONE_MANUFACTURER ", " CP_FAX_MANUFACTURER ", " CP_PORTABLE_MANUFACTURER ", " CP_WEBSITE_MANUFACTURER ", "
                              CP_MAIL_MANUFACTURER ", " CP_INACTIF_MANUFACTURER ", " CP_DISTRIBUEPAR_MANUFACTURER ", " CP_IDRUFUS_MANUFACTURER
                    " FROM " TBL_MANUFACTURERS
                    " WHERE " CP_ID_MANUFACTURER " = " + QString::number(idManufacturer);
    QVariantList Manufacturerdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || Manufacturerdata.size()==0)
        return Man;
    QJsonObject data = loadManufacturerData(Manufacturerdata);
    Man = new Manufacturer(data);
    return Man;
}

/*
 * Commerciaux
*/

QJsonObject DataBase::loadCommercialData(QVariantList comdata)         //! attribue la liste des datas à un fabricant
{
    QJsonObject data{};
    data[CP_ID_COM]             = comdata.at(0).toInt();
    data[CP_NOM_COM]            = comdata.at(1).toString();
    data[CP_PRENOM_COM]         = comdata.at(2).toString();
    data[CP_STATUT_COM]         = comdata.at(3).toString();
    data[CP_MAIL_COM]           = comdata.at(4).toString();
    data[CP_TELEPHONE_COM]      = comdata.at(5).toString();
    data[CP_IDMANUFACTURER_COM] = comdata.at(6).toInt();
    return data;
}

QList<Commercial*> DataBase::loadCommercials()                       //! charge tous les fabricants
{
    QList<Commercial*> list = QList<Commercial*> ();
    QString req =   "SELECT " CP_ID_COM ", " CP_NOM_COM ", " CP_PRENOM_COM ", " CP_STATUT_COM ", " CP_MAIL_COM ", "
                              CP_TELEPHONE_COM ", " CP_IDMANUFACTURER_COM
                    " FROM " TBL_COMMERCIALS " order by " CP_NOM_COM ", " CP_PRENOM_COM;
    QList<QVariantList> commerciallist = StandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || commerciallist.size()==0)
        return list;
    for (int i=0; i<commerciallist.size(); ++i)
    {
        QJsonObject data = loadCommercialData(commerciallist.at(i));
        Commercial *Com = new Commercial(data);
        if (Com != Q_NULLPTR)
            list << Com;
    }
    return list;
}

Commercial* DataBase::loadCommercialById(int idcommercial)                   //! charge un commercial défini par son id - utilisé pour renouveler les données en cas de modification
{
    Commercial *Com = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_COM ", " CP_NOM_COM ", " CP_PRENOM_COM ", " CP_STATUT_COM ", " CP_MAIL_COM ", "
                              CP_TELEPHONE_COM ", " CP_IDMANUFACTURER_COM
                    " FROM " TBL_COMMERCIALS
                    " WHERE " CP_ID_COM " = " + QString::number(idcommercial);
    QVariantList Commercialdata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || Commercialdata.size()==0)
        return Com;
    QJsonObject data = loadCommercialData(Commercialdata);
    Com = new Commercial(data);
    return Com;
}

QList<Commercial *> DataBase::loadCommercialsByIdManufacturer(int idmanufacturer)
{
    QList<Commercial*> list = QList<Commercial*> ();
    QString req =   "SELECT " CP_ID_COM ", " CP_NOM_COM ", " CP_PRENOM_COM ", " CP_STATUT_COM ", " CP_MAIL_COM ", "
                              CP_TELEPHONE_COM ", " CP_IDMANUFACTURER_COM
                    " FROM " TBL_COMMERCIALS
                    " WHERE " CP_IDMANUFACTURER_COM " = " + QString::number(idmanufacturer) +
                    " order by " CP_NOM_COM ", " CP_PRENOM_COM;
    QList<QVariantList> commerciallist = StandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || commerciallist.size()==0)
        return list;
    for (int i=0; i<commerciallist.size(); ++i)
    {
        QJsonObject data = loadCommercialData(commerciallist.at(i));
        Commercial *Com = new Commercial(data);
        if (Com != Q_NULLPTR)
            list << Com;
    }
    return list;
}

/*
 * Manufacturers
*/

QJsonObject DataBase::loadMotCleData(QVariantList Motcledata)         //! attribue la liste des datas à un mot clé
{
    QJsonObject data{};
    data[CP_ID_MOTCLE]      = Motcledata.at(0).toInt();
    data[CP_TEXT_MOTCLE]    = Motcledata.at(1).toString();
    return data;
}

QList<MotCle*> DataBase::loadMotsCles()                       //! charge tous les mots clés
{
    QList<MotCle*> list = QList<MotCle*> ();
    QString req =   "SELECT " CP_ID_MOTCLE ", " CP_TEXT_MOTCLE
                    " FROM " TBL_MOTSCLES " order by " CP_TEXT_MOTCLE;
    QList<QVariantList> MotClelist = StandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || MotClelist.size()==0)
        return list;
    for (int i=0; i<MotClelist.size(); ++i)
    {
        QJsonObject data = loadMotCleData(MotClelist.at(i));
        MotCle *Motcle = new MotCle(data);
        if (Motcle)
            list << Motcle;
    }
    return list;
}

MotCle* DataBase::loadMotCleById(int idMotcle)                   //! charge un mot clé défini par son id - utilisé pour renouveler les données en cas de modification
{
    MotCle *Motcle = Q_NULLPTR;
    QString req =   "SELECT " CP_ID_MOTCLE ", " CP_TEXT_MOTCLE
                    " FROM " TBL_MOTSCLES
                    " WHERE " CP_ID_MOTCLE " = " + QString::number(idMotcle);
    QVariantList MotCledata = getFirstRecordFromStandardSelectSQL(req,ok);
    if(!ok || MotCledata.size()==0)
        return Motcle;
    QJsonObject data = loadMotCleData(MotCledata);
    Motcle = new MotCle(data);
    return Motcle;
}

QList<int> DataBase::loadListIdMotsClesByPat(int idpat)                              //! chagre les id des mots clés utilisés par un patient
{
    QList<int> listid = QList<int> ();
    QString req =   "SELECT " CP_IDMOTCLE_JOINTURESMOTSCLES
                    " FROM " TBL_MOTSCLESJOINTURES
                    " WHERE " CP_IDPATIENT_JOINTURESMOTSCLES " = " + QString::number(idpat);
    QList<QVariantList> idslist = StandardSelectSQL(req,ok);
    //qDebug() << req;
    if(!ok || idslist.size()==0)
        return listid;
    for (int i=0; i<idslist.size(); ++i)
        listid << idslist.at(i).at(0).toInt();
    return listid;
}
