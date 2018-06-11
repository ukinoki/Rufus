#include "database.h"

#include <QSqlDatabase>

DataBase* DataBase::instance = 0;
DataBase* DataBase::getInstance()
{
    if( !instance )
    {
        instance = new DataBase();
    }

    return instance;
}

void DataBase::init(QSettings const &setting, int mode) : m_mode(mode)
{
    if( m_mode == Poste )
        m_server = "localhost";
    else
        m_server = setting->value(getBase() + "/Serveur").toString();

    m_port = setting->value(getBase() + "/Port").toInt();

    m_useSSL = (m_mode == Distant);
}

QString DataBase::getMode() const
{
    return m_mode;
}
QString DataBase::getBase() const
{
    if (m_mode == ReseauLocal)
        return "BDD_LOCAL";
    if (m_mode == Distant)
        return "BDD_DISTANT";

    return "BDD_POSTE"; //m_mode == Poste
}
QString DataBase::getServer() const
{
    return m_server;
}
QSqlDatabase DataBase::getDataBase(QString basename) const
{
    if( m_mapDB->contains(basename) )
        return m_mapDB[basename];

    return nullptr;
}

QString DataBase::getInformations()
{
    UpMessageBox::Watch(0,
        tr("Connexion à la base de données!"),
        tr("Vos paramètres de connexion") +
           "\n" + tr("Serveur     ") + "\n ->\t" + m_db.hostName() +
           "\n" + tr("databaseName") + "\n ->\t" + m_db.databaseName() +
           "\n" + tr("Login       ") + "\n ->\t" + m_db.userName() +
           "\n" + tr("password    ") + "\n ->\t" + m_db.password() +
           "\n" + tr("port        ") + "\n ->\t" + QString::number(m_db.port()));
}

User DataBase::getUser() const
{
    return &m_userConnected;
}



QString connectToDataBase(QString basename, QString login, QString password)
{
    QSqlDatabase db = m_mapDB[basename];
    if( db )
    {

    }

    db = QSqlDatabase::addDatabase("QMYSQL",basename);
    db.setHostName( m_server );
    db.setPort( m_port );

    QString connectOptions = (m_useSSL?
                              "SSL_KEY=/etc/mysql/client-key.pem;"
                              "SSL_CERT=/etc/mysql/client-cert.pem;"
                              "SSL_CA=/etc/mysql/ca-cert.pem;"
                              "MYSQL_OPT_RECONNECT=1"
                                 :
                              "MYSQL_OPT_RECONNECT=1");
    db.setConnectOptions(connectOptions);

    db.setUserName(login + (m_useSSL ? "SSL" : ""));
    db.setPassword(password);

    if( db.open() )
    {
        m_mapDB[basename] = db;
        return NULL;
    }

    QSqlDatabase::removeDatabase(basename);
    db.deleteLater();
    return db.lastError().text();
}

QJsonObject login(QString login, QString password)
{
    QJsonObject jrep{};
    int userOffset = 3;

    QString req = "SELECT u.idUser, u.UserNom, u.UserPrenom "
                  "uc.NomPosteconnecte "//TODO : récupérer tout le reste
                  " FROM %1 u "
                  " LEFT JOIN %2 uc on uc.idUSer = u.idUser "
                  " WHERE UserLogin = '%2' "
                  " AND UserMDP = '%3' ";
    req.args(NOM_TABLE_UTILISATEURS, NOM_TABLE_USERSCONNECTES, login, password);

    QSqlQuery query(req,getDataBase(NOM_BASE_CONSULT) );
    if( query.lastError().type() != QSqlError::NoError )
    {
        jrep["code"] = -3;
        jrep["request"] = req;
        return json;
    }

    if( !query.first() )
    {
        jrep["code"] = -4;
        return json;
    }

    if( !query.value(userOffset).isNull() )
    {
        jrep["code"] = -1;
        jrep["poste"] = query.value(userOffset).toString();
        return json;
    }

    jrep["code"] = 0;

    QJsonObject userData{};
    userData["id"] = query.value(0).toString();
    userData["nom"] = query.value(1).toString();
    userData["prenom"] = query.value(2).toString();
    m_userConnected = new User(login, password, userData);


    return json;
}



