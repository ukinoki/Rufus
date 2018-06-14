#include "cls_acte.h"
#include "database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

DataBase* DataBase::instance = 0;

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
    UpMessageBox::Watch(0,
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
void DataBase::setUserConnected( User *user )
{
    m_userConnected = user;
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
        return NULL;

    QSqlDatabase::removeDatabase(basename);
    return m_db.lastError().text();
}

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

    if( !query.value(userOffset).isNull() )
    {
        jrep["code"] = -6;
        jrep["poste"] = query.value(userOffset).toString();
        return jrep;
    }

    jrep["code"] = 0;

    QJsonObject userData{};
    userData["id"] = query.value(0).toInt();
    userData["nom"] = query.value(1).toString();
    userData["prenom"] = query.value(2).toString();

    m_userConnected = new User(login, password, userData);
    return jrep;
}
QJsonObject DataBase::loadUser(int idUser)
{
    QJsonObject userData{};

    QString req = "select UserDroits, UserAGA, UserLogin, UserFonction, UserTitre, "                        // 0,1,2,3,4
            " UserNom, UserPrenom, UserMail, UserNumPS, UserSpecialite,"                                    // 5,6,7,8,9
            " UserNoSpecialite, UserNumCO, idCompteParDefaut, UserEnregHonoraires, UserMDP,"                // 10,11,12,13,14
            " UserPortable, UserPoste, UserWeb, UserMemo, UserDesactive,"                                   // 15,16,17,18,19
            " UserPoliceEcran, UserPoliceAttribut, UserSecteur, Soignant, ResponsableActes,"                // 20,21,22,23,24
            " UserCCAM, UserEmployeur, DateDerniereConnexion, idCompteEncaissHonoraires, Medecin,"          // 25,26,27,28,29
            " OPTAM"                                                                                        // 30
            " from " NOM_TABLE_UTILISATEURS
            " where idUser = " + QString::number(idUser);
    QSqlQuery  query(req, getDataBase());
    if( traiteErreurRequete(query, tr("Impossible de retrouver les données de l'utilisateur"), req) )
        return userData;

    if( !query.first() )
        return userData;

    userData["idUser"]                      = idUser;
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

    if( !query.value(26).isNull()
       && (userData["soignant"].toInt() == 1 || userData["soignant"].toInt() == 2 || userData["soignant"].toInt() == 3) )         //le user est un soignant et il y a un employeur
    {
        req = "select idCompteEncaissHonoraires from " NOM_TABLE_UTILISATEURS " where iduser = " + query.value(26).toString();
        QSqlQuery cptquer(req, getDataBase());
        if( cptquer.first() )
            userData["idCompteEncaissHonoraires"] = cptquer.value(0).toInt();
        cptquer.finish();
    }

    if( userData["idCompteEncaissHonoraires"].isNull() )
    {
        userData["idUserEncaissHonoraires"]     = -1;
        userData["nomCompteEncaissHonoraires"]  = "";
        userData["nomUserEncaissHonoraires"]    = "";
    }
    else
    {
        req = "select cpt.iduser, nomcompteabrege, userlogin from " NOM_TABLE_COMPTES  " cpt"
              " left outer join " NOM_TABLE_UTILISATEURS " usr on  usr.iduser = cpt.iduser"
              " where idcompte = " + userData["idCompteEncaissHonoraires"].toString();
        QSqlQuery usrencaisquer(req, getDataBase());
        if( usrencaisquer.first() )
        {
            userData["idUserEncaissHonoraires"]     = usrencaisquer.value(0).toInt();
            userData["nomCompteEncaissHonoraires"]  = usrencaisquer.value(1).toString();
            userData["nomUserEncaissHonoraires"]    = usrencaisquer.value(2).toString();
            usrencaisquer.finish();
        }
    }

    if( !query.value(12).isNull() )
    {
        req = "select nomcompteabrege from " NOM_TABLE_COMPTES
              " where idcompte = " + userData["idCompteParDefaut"].toString();
        QSqlQuery usrcptquer(req, getDataBase());
        if( usrcptquer.first() )
            userData["nomCompteParDefaut"]  = usrcptquer.value(0).toString();
    }
    query.finish();
    return userData;
}

/*
 * Etablissements
*/
QList<Etablissement*> DataBase::loadUserEtablissements(int idUser)
{
    QList<Etablissement*> etabs;

    QString req = "select joint.idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3, "
                    "LieuCodePostal, LieuVille, LieuTelephone, LieuFax "
                  "from " NOM_TABLE_JOINTURESLIEUX " joint "
                  "left join " NOM_TABLE_LIEUXEXERCICE " lix on joint.idlieu = lix.idLieu "
                  "where iduser = " + QString::number(idUser);

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
        Etablissement *etab = new Etablissement(jEtab);
        etabs << etab;
    } while( query.next() );

    return etabs;
}

/*
 * Gestion des Patients
*/



/*
 * Gestion des Actes
*/
Acte* DataBase::loadActeById(int idActe)
{
    Acte *acte = new Acte(idActe, 0, 0);

    if( idActe == 0 )
        return acte;
    QString subRequestRankAct = "SELECT idActe, idPat, "
                                  "CASE WHEN @prevRank = idPat THEN @curRank := @curRank + 1 WHEN @prevRank := idPat THEN @curRank := 1 END AS rank "
                                  " FROM " NOM_TABLE_ACTES ", (SELECT @curRank := 0, @prevRank := NULL) r "
                                  " ORDER BY idPat, idActe ";
    QString subRequestMinMaxAct = "SELECT min(idActe) as idActeMin, max(idActe) as idActeMax, count(idActe) as total, idPat "
                                " FROM " NOM_TABLE_ACTES
                                " GROUP BY idPat ";
    QString requete = "SELECT act.idActe, act.idPat, act.idUser, "
                        " act.ActeDate, act.ActeMotif, act.ActeTexte, act.ActeConclusion, "
                        " act.ActeCourrierAFaire, act.ActeCotation, act.ActeMontant, act.ActeMonnaie, "
                        " act.CreePar, "
                        " pat.PatDDN, ll2.rank, ll.idActeMin, ll.idActeMax, ll.total, "
                        " tpm.TypePaiement, tpm.Tiers "
                      " FROM " NOM_TABLE_ACTES " act "
                      " LEFT JOIN " NOM_TABLE_PATIENTS " pat on pat.idPat = act.idPat "
                      " JOIN ( "+ subRequestMinMaxAct + " ) ll on ll.idPat = act.idPat "
                      " JOIN ( "+ subRequestRankAct + " ) ll2 on ll2.idPat = act.idPat and ll2.idActe = act.idActe "
                      " LEFT JOIN " NOM_TABLE_TYPEPAIEMENTACTES " tpm on tpm.idActe = act.idActe "
                      " WHERE act.idActe = '" + QString::number(idActe) + "'";
    QSqlQuery query(requete, getDataBase());
    if( traiteErreurRequete(query, requete) || !query.first() )
        return acte;

    QJsonObject data{};
    data["date"] = QDateTime(query.value(3).toDate()).toMSecsSinceEpoch();
    data["motif"] = query.value(4).toString();
    data["texte"] = query.value(5).toString();
    data["conclusion"] = query.value(6).toString();
    data["courrierStatus"] = query.value(7).toString();
    data["idCreatedBy"] = query.value(11).toInt();
    data["idPatient"] = query.value(2).toInt();
    data["cotation"] = query.value(8).toString();
    data["monnaie"] = query.value(10).toString();
    data["montant"] = query.value(9).toDouble();

    if( query.value(12).isNull() )
        data["agePatient"] = -1;
    else
        data["agePatient"] = QDateTime(query.value(12).toDate()).toMSecsSinceEpoch();

    data["noActe"] = query.value(13).toInt();
    data["idActeMin"] = query.value(14).toInt();
    data["idActeMax"] = query.value(15).toInt();
    data["nbActes"] = query.value(16).toInt();

    if( query.value(17).isNull() )
        data["paiementType"] = "";
    else
        data["paiementType"] = query.value(17).toString();

    if( query.value(18).isNull() )
        data["montant"] = "";
    else
        data["montant"] = query.value(18).toString();
    acte->setData(data);
    return acte;
}
QMap<int, Acte*> DataBase::loadActeByIdPat(int idPat)
{
    QMap<int, Acte*> list;
    if( idPat == 0 )
        return list;

    QString subRequestMinMaxAct = "SELECT idActe, idPat, "
                                  "CASE WHEN @prevRank = idPat THEN @curRank := @curRank + 1 WHEN @prevRank := idPat THEN @curRank := 1 END AS rank, "
                                  " FROM " NOM_TABLE_ACTES " (SELECT @curRank := 0, @prevRank := NULL) r "
                                  " ORDER BY idPat, idActe ";
    QString subRequestRankAct = "SELECT min(idActe) as idActeMin, max(idActe) as idActeMax, count(idActe) as total, idPat "
                                " FROM " NOM_TABLE_ACTES
                                " GROUP BY idPat ";
    QString requete = "SELECT act.idActe, act.idPat, act.idUser, "
                        " act.ActeDate, act.ActeMotif, act.ActeTexte, act.ActeConclusion, "
                        " act.ActeCourrierAFaire, act.ActeCotation, act.ActeMontant, act.ActeMonnaie, "
                        " act.CreePar, "
                        " pat.PatDDN, ll2.rank, ll.idActeMin, ll.idActeMax, ll.total, "
                        " tpm.TypePaiement, tpm.Tiers "
                      " FROM " NOM_TABLE_ACTES " act "
                      " LEFT JOIN " NOM_TABLE_PATIENTS " pat on pat.idPat = act.idPat "
                      " JOIN ( "+ subRequestMinMaxAct + " ) ll on ll.idPat = act.idPat "
                      " JOIN ( "+ subRequestRankAct + " ) ll2 on ll2.idPat = act.idPat and ll2.idActe = act.idActe "
                      " LEFT JOIN " NOM_TABLE_TYPEPAIEMENTACTES " tpm on tpm.idActe = act.idActe "
                      " WHERE act.idPat = '" + QString::number(idPat) + "' "
                      " ORDER BY idActe DESC";
    QSqlQuery query(requete, getDataBase());
    if( traiteErreurRequete(query, requete) || !query.first() )
        return list;

    do
    {
        Acte *acte = new Acte(query.value(0).toInt(), query.value(16).toInt(), query.value(13).toInt());

        QJsonObject data{};
        data["date"] = QDateTime(query.value(3).toDate()).toMSecsSinceEpoch();
        data["motif"] = query.value(4).toString();
        data["texte"] = query.value(5).toString();
        data["conclusion"] = query.value(6).toString();
        data["courrierStatus"] = query.value(7).toString();
        data["idCreatedBy"] = query.value(11).toInt();
        data["idPatient"] = query.value(2).toInt();
        data["cotation"] = query.value(8).toString();
        data["monnaie"] = query.value(10).toString();
        data["montant"] = query.value(9).toDouble();

        if( query.value(12).isNull() )
            data["agePatient"] = -1;
        else
            data["agePatient"] = QDateTime(query.value(12).toDate()).toMSecsSinceEpoch();

        data["idActeMin"] = query.value(14).toInt();
        data["idActeMax"] = query.value(15).toInt();

        if( query.value(17).isNull() )
            data["paiementType"] = "";
        else
            data["paiementType"] = query.value(17).toString();

        if( query.value(18).isNull() )
            data["montant"] = "";
        else
            data["montant"] = query.value(18).toString();
        acte->setData(data);
        list[acte->id()] = acte;
    }while( query.next() );

    return list;
}
