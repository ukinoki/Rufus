/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "functordatauser.h"

QMap<QString,QVariant> FunctorDataUser::operator ()(int iduser, int idLieu, QSqlDatabase db)
{
    QString req = "select UserDroits, UserAGA, UserLogin, UserFonction, UserTitre, "                        // 0,1,2,3,4
            " UserNom, UserPrenom, UserMail, UserNumPS, UserSpecialite,"                                    // 5,6,7,8,9
            " UserNoSpecialite, UserNumCO, idCompteParDefaut, UserEnregHonoraires, UserMDP,"                // 10,11,12,13,14
            " UserPortable, UserPoste, UserWeb, UserMemo, UserDesactive,"                                   // 15,16,17,18,19
            " UserPoliceEcran, UserPoliceAttribut, UserSecteur, Soignant, ResponsableActes,"                // 20,21,22,23,24
            " UserCCAM, UserEmployeur, DateDerniereConnexion, idCompteEncaissHonoraires, Medecin,"          // 25,26,27,28,29
            " OPTAM"                                                                                        // 30
            " from " NOM_TABLE_UTILISATEURS
            " where idUser = " + QString::number(iduser);
    //qDebug() << req;
    QSqlQuery  RetrouveDataUserQuery (req,db);
    if (RetrouveDataUserQuery.lastError().type() != QSqlError::NoError)
    {
        UpMessageBox::Watch(0, "Impossible de retrouver les données de l'utilisateur", "\nErreur\n" + RetrouveDataUserQuery.lastError().text() +  "\nrequete = " + req);
        //qDebug() << req;
    }
    //TraiteErreurRequete(RetrouveDataUserQuery, req);
    if (RetrouveDataUserQuery.size() == 0)
    {
        UpMessageBox::Watch(0, QObject::tr("Impossible de retrouver cet utilisateur!"));
        DataUser["Success"]            = false;
        return DataUser;
    }
    RetrouveDataUserQuery.first();
    DataUser["Success"]                     = true;
    DataUser["idUser"]                      = iduser;
    DataUser["Droits"]                      = RetrouveDataUserQuery.value(0);
    DataUser["AGA"]                         = (RetrouveDataUserQuery.value(1).toInt() == 1);
    DataUser["UserLogin"]                   = RetrouveDataUserQuery.value(2);
    DataUser["Fonction"]                    = RetrouveDataUserQuery.value(3);
    DataUser["Titre"]                       = RetrouveDataUserQuery.value(4);
    DataUser["Nom"]                         = RetrouveDataUserQuery.value(5);
    DataUser["Prenom"]                      = RetrouveDataUserQuery.value(6);
    DataUser["Mail"]                        = RetrouveDataUserQuery.value(7);
    if (RetrouveDataUserQuery.value(8).toString() == QVariant(QVariant::String))
        DataUser["NumPS"]                   = "";
    else
        DataUser["NumPS"]                   = RetrouveDataUserQuery.value(8);
    DataUser["Specialite"]                  = RetrouveDataUserQuery.value(9);
    DataUser["NoSpecialite"]                = RetrouveDataUserQuery.value(10);
    DataUser["NumCO"]                       = RetrouveDataUserQuery.value(11);
    DataUser["idCompteParDefaut"]           = RetrouveDataUserQuery.value(12);
    DataUser["EnregHonoraires"]             = RetrouveDataUserQuery.value(13);
    DataUser["MDP"]                         = RetrouveDataUserQuery.value(14);
    DataUser["Portable"]                    = RetrouveDataUserQuery.value(15);
    DataUser["Poste"]                       = RetrouveDataUserQuery.value(16);
    DataUser["Web"]                         = RetrouveDataUserQuery.value(17);
    DataUser["Memo"]                        = RetrouveDataUserQuery.value(18);
    DataUser["Desactive"]                   = (RetrouveDataUserQuery.value(19).toInt() == 1);
    DataUser["PoliceEcran"]                 = RetrouveDataUserQuery.value(20);
    DataUser["PoliceAttribut"]              = RetrouveDataUserQuery.value(21);
    DataUser["Secteur"]                     = RetrouveDataUserQuery.value(22);
    DataUser["OPTAM"]                       = (RetrouveDataUserQuery.value(30).toInt() == 1);
    DataUser["Soignant"]                    = RetrouveDataUserQuery.value(23);
    DataUser["ResponsableActes"]            = RetrouveDataUserQuery.value(24);
    DataUser["Cotation"]                    = (RetrouveDataUserQuery.value(25).toInt()==1);
    DataUser["Employeur"]                   = RetrouveDataUserQuery.value(26);
    DataUser["DateDerniereConnexion"]       = QDateTime(RetrouveDataUserQuery.value(27).toDate(), RetrouveDataUserQuery.value(27).toTime());
    DataUser["Medecin"]                     = RetrouveDataUserQuery.value(29);

    if (RetrouveDataUserQuery.value(26).toInt() != QVariant(QVariant::Int)
       && (DataUser["Soignant"].toInt() == 1 || DataUser["Soignant"].toInt() == 2 || DataUser["Soignant"].toInt() == 3))         //le user est un soignant et il y a un employeur
    {
        req = "select idCompteEncaissHonoraires from " NOM_TABLE_UTILISATEURS " where iduser = " + RetrouveDataUserQuery.value(26).toString();
        QSqlQuery cptquer(req, db);
        cptquer.first();
        DataUser["idCompteEncaissHonoraires"]   = cptquer.value(0);
    }
    else if (RetrouveDataUserQuery.value(28).toInt() != QVariant(QVariant::Int))
        DataUser["idCompteEncaissHonoraires"]   = RetrouveDataUserQuery.value(28);

    if (DataUser["idCompteEncaissHonoraires"].toInt() != QVariant(QVariant::Int))
    {
        req = "select cpt.iduser, nomcompteabrege, userlogin from " NOM_TABLE_COMPTES  " cpt"
              " left outer join " NOM_TABLE_UTILISATEURS " usr on  usr.iduser = cpt.iduser"
              " where idcompte = " + DataUser["idCompteEncaissHonoraires"].toString();
        QSqlQuery usrencaisquer(req, db);
        usrencaisquer.first();
        DataUser["idUserEncaissHonoraires"]     = usrencaisquer.value(0);
        DataUser["NomCompteEncaissHonoraires"]  = usrencaisquer.value(1);
        DataUser["NomUserEncaissHonoraires"]    = usrencaisquer.value(2);
    }
    else
    {
        DataUser["idUserEncaissHonoraires"]     = -1;
        DataUser["NomCompteEncaissHonoraires"]  = -1;
        DataUser["NomUserEncaissHonoraires"]    = "";
    }
    if (RetrouveDataUserQuery.value(12).toInt() != QVariant(QVariant::Int))
    {
        req = "select nomcompteabrege from " NOM_TABLE_COMPTES
              " where idcompte = " + DataUser["idCompteParDefaut"].toString();
        QSqlQuery usrcptquer(req, db);
        usrcptquer.first();
        DataUser["NomCompteParDefaut"]  = usrcptquer.value(0);
    }

    QString lxreq = "select idLieu, NomLieu, LieuAdresse1, LieuAdresse2, LieuAdresse3,"    // 0,1,2,3,4
                    " LieuCodePostal, LieuVille, LieuTelephone, LieuFax"                  // 5,6,7,8
                    " from " NOM_TABLE_LIEUXEXERCICE
                    " where idLieu = " + QString::number(idLieu);
    //qDebug() << lxreq;
    QSqlQuery  RetrouveDataLieuQuery (lxreq,db);
    if (RetrouveDataLieuQuery.lastError().type() != QSqlError::NoError)
    {
        UpMessageBox::Watch(0, "Pas d'adresse enregistrée pour cet utilisateur", "\nErreur\n" + RetrouveDataLieuQuery.lastError().text() +  "\nrequete = " + lxreq);
        //qDebug() << lxreq;
    }
    if (RetrouveDataLieuQuery.size()==0)
    {
        UpMessageBox::Watch(0, QObject::tr("Pas d'adresse enregistrée pour cet utilisateur!"),  QObject::tr("Corrigez cette erreur et relancez le programme!"));
        DataUser["Success"]            = true;
        return DataUser;
    }
    RetrouveDataLieuQuery.first();
    DataUser["idLieu"]                      = RetrouveDataLieuQuery.value(0);
    DataUser["NomLieu"]                     = RetrouveDataLieuQuery.value(1);
    DataUser["Adresse1"]                    = RetrouveDataLieuQuery.value(2);
    DataUser["Adresse2"]                    = RetrouveDataLieuQuery.value(3);
    DataUser["Adresse3"]                    = RetrouveDataLieuQuery.value(4);
    if (RetrouveDataLieuQuery.value(5).toString() == QVariant(QVariant::String))
        DataUser["CodePostal"]              = "";
    else
        DataUser["CodePostal"]              = RetrouveDataLieuQuery.value(5);
    DataUser["Ville"]                       = RetrouveDataLieuQuery.value(6);
    DataUser["Telephone"]                   = RetrouveDataLieuQuery.value(7);
    DataUser["Fax"]                         = RetrouveDataLieuQuery.value(8);

    return DataUser;
}
