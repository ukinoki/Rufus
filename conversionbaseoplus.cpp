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


#include "conversionbaseoplus.h"

conversionbaseoplus::conversionbaseoplus(Procedures *proc, QString BaseAConvertir, QObject *parent) :
    QObject(parent)
{
    QString NomBase = BaseAConvertir;
    db = DataBase::I();
    /*COnvertir une base ophtalogic
    */
    //proc->RestaureBase(true,false,true);
    QString req = "select schema_name from information_schema.schemata"
                  " where schema_name = 'Oplus'";
    QList<QVariantList> lgclist = db->StandardSelectSQL(req,ok);
    if (lgclist.size()==0)
    {
        UpMessageBox::Watch(Q_NULLPTR,"pas de base O+ retrouvée");
        return;
    }
    NomBase = "";
    for (int i=0; i<lgclist.size();i++)
    {
        if (QMessageBox::question(Q_NULLPTR,"Conversion d'une base O+","Tenter de convertir la base " + lgclist.at(i).at(0).toString() + "?") == QMessageBox::Yes)
        {
            NomBase = lgclist.at(i).at(0).toString();
            break;
        }
    }
    if (NomBase=="")
        return;
    db->StandardSQL("delete from " TBL_PATIENTS);
    db->StandardSQL("delete from " TBL_DONNEESSOCIALESPATIENTS);
    db->StandardSQL("delete from " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    db->StandardSQL("delete from " TBL_ACTES);
    db->StandardSQL("delete from " TBL_TYPEPAIEMENTACTES);
    db->StandardSQL("delete from " TBL_LIGNESPAIEMENTS);
    db->StandardSQL("delete from " TBL_RECETTES);
    db->StandardSQL("delete from " TBL_CORRESPONDANTS);
    db->StandardSQL("delete from " TBL_REFRACTIONS);
    db->StandardSQL("delete from " TBL_DOCSEXTERNES);
    db->StandardSQL("delete from " TBL_DONNEES_OPHTA_PATIENTS);
    db->StandardSQL("delete from " TBL_UTILISATEURS " where " CP_NOM_USR " <> '" NOM_ADMINISTRATEUR "' and " CP_NOM_USR " <> 'Snow'" );

    // Utilisateurs
    req = "SELECT distinct prat_usuel FROM Oplus.identites_patients where prat_usuel <> \"\" order by prat_usuel asc;";
    QMap<int,QString> mapprat;
    QList<QVariantList> pratlist = db->StandardSelectSQL(req,ok);
    for (int i=0; i< pratlist.size(); i++)
    {
        QString nom = pratlist.at(i).at(0).toString();
        QString login = nom.replace("Dr ", "");
        login = login.replace(".", "");
        login = login.replace("'", "");
        login = login.replace(" ", "").toLower().left(8);
        req = "insert into " TBL_UTILISATEURS "(" CP_NOM_USR ", " CP_LOGIN_USR ", " CP_MDP_USR ") values ('" + Utils::correctquoteSQL(nom) + "', '" + login + "', 'bob')";
        db->StandardSQL(req);
        req = "select " CP_ID_USR " from " TBL_UTILISATEURS " where " CP_NOM_USR " = '" + nom + "'";
        int idusr = db->getFirstRecordFromStandardSelectSQL(req, ok).at(0).toInt();
        mapprat.insert(idusr, nom);
    }

    // Médecins traitants
    req = "SELECT distinct med_trait FROM Oplus.identites_patients where med_trait <> \"\";";
    QMap<QString, int> medtraitprat;
    QList<QVariantList> medlist = db->StandardSelectSQL(req,ok);
    int k = 1;
    for (int i=0; i< medlist.size(); i++)
    {
        QString nommed = Utils::trimcapitilize(medlist.at(i).at(0).toString());
        medtraitprat.insert(nommed, k);
        k++;
    }
    for (auto it = medtraitprat.constBegin(); it != medtraitprat.constEnd(); ++it)
    {
        req = "insert into " TBL_CORRESPONDANTS "(" CP_ID_CORRESP ", " CP_NOM_CORRESP ", " CP_PRENOM_CORRESP ", " CP_SEXE_CORRESP ", " CP_SPECIALITE_CORRESP ", " CP_ISMEDECIN_CORRESP  ") "
                                                                                                                                                                                        " values (" + QString::number(it.value()) +  ",'" + it.key() + "', 'x', 'F', 13, 1)";
        //qDebug() << req;
        db->StandardSQL(req);
    }

    // Patients
    UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des patients", Icons::icSunglasses(), 1000);
    req = "select nopat, sexe, nom_prenom, date_naiss,"
          " adr1, adr2, codepostal, ville, noSS, tel, tel_port, email, profession,"
          " prat_usuel, med_trait"
          " from Oplus.identites_patients";
    QList<QVariantList> patlist = db->StandardSelectSQL(req,ok);
    QList<int> listidpat;

    for (int i=0; i< patlist.size(); i++)
    {
        QString nom (""), sexe (""), ddn ("");
        int idpat;
        QString adr1 (""), adr2 (""), codepostal (""), ville (""), noSS (""), tel (""), tel_port (""), email (""), profession ("");

        QHash<QString, QVariant> listbinds;
        ddn = "1900-01-01";
        QDate date = QDate::fromString(patlist.at(i).at(3).toString().left(10),"dd/MM/yyyy");
        if (date.isValid())
            ddn = date.toString("yyyy-MM-dd");
        idpat = patlist.at(i).at(0).toInt();
        listidpat << idpat;
        nom = Utils::trimcapitilize(patlist.at(i).at(2).toString());
        sexe = "";
        if (patlist.at(i).at(1) == "F" || patlist.at(i).at(1) == "M")
            sexe = patlist.at(i).at(1).toString();
        listbinds[CP_IDPAT_PATIENTS] = idpat;
        listbinds[CP_DDN_PATIENTS] = date;
        listbinds[CP_NOM_PATIENTS] = nom;
        if (sexe != "")
            listbinds[CP_SEXE_PATIENTS] = sexe;
        db->InsertSQLByBinds(TBL_PATIENTS, listbinds, "problème avec le patient " + nom);
        listbinds.clear();

        adr1        = patlist.at(i).at(4).toString();
        adr2        = patlist.at(i).at(5).toString();
        codepostal  = patlist.at(i).at(6).toString();
        ville       = patlist.at(i).at(7).toString();
        noSS        = patlist.at(i).at(8).toString();
        tel         = patlist.at(i).at(9).toString();
        tel_port    = patlist.at(i).at(10).toString();
        email       = patlist.at(i).at(11).toString();
        profession  = patlist.at(i).at(12).toString();
        listbinds[CP_IDPAT_PATIENTS] = idpat;
        if (adr1 != "")
            listbinds[CP_ADRESSE1_DSP] = adr1;
        if (adr2 != "")
            listbinds[CP_ADRESSE2_DSP] = adr2;
        if (codepostal != "")
            listbinds[CP_CODEPOSTAL_DSP] = codepostal;
        if (ville != "")
            listbinds[CP_VILLE_DSP] = ville;
        if (noSS != "")
            listbinds[CP_NNI_DSP] = noSS;
        if (tel != "")
            listbinds[CP_TELEPHONE_DSP] = tel;
        if (tel_port != "")
            listbinds[CP_PORTABLE_DSP] = tel_port;
        if (email != "")
            listbinds[CP_MAIL_DSP] = email;
        if (profession != "")
            listbinds[CP_PROFESSION_DSP] = profession;
        db->InsertSQLByBinds(TBL_DONNEESSOCIALESPATIENTS, listbinds, "problème avec le patient " + nom);
        listbinds.clear();

        listbinds[CP_IDPAT_PATIENTS] = idpat;
        QString medtrait = Utils::trimcapitilize(patlist.at(i).at(14).toString());
        if (medtrait != "")
        {
            auto it = medtraitprat.find(medtrait);
            if  (it != medtraitprat.end())
            {
                int idmed = it.value();
                listbinds[CP_IDMG_RMP] = idmed;
            }
        }
        db->InsertSQLByBinds(TBL_RENSEIGNEMENTSMEDICAUXPATIENTS, listbinds, "problème avec le patient " + nom);
        listbinds.clear();
    }
    UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_PATIENTS " importée", Icons::icSunglasses(), 1000);

    for (int k=0; k< listidpat.size(); ++k)
    {
        int idpat =  listidpat.at(k);
        qDebug() << k << "/" <<  listidpat.size() << "idpat = " << idpat;
        QString strgidpat = QString::number(idpat);

        req = "SELECT rubrique, valeur, dateheure FROM Oplus.actes where nopat = " + QString::number(idpat);
        QList<QVariantList> actlist = db->StandardSelectSQL(req,ok);
        QList<QDateTime> listdates;
        for (int i=0; i< actlist.size(); i++)
        {
            QDateTime datetime = QDateTime::fromString(actlist.at(i).at(2).toString(),"dd/MM/yyyy HHmm");
            if (!datetime.isValid())
            {
                /*!
                    pidmm   = écart interpupillaire
                    atcd    = antécédents généraux
                    dgn     = diagnostic ophtalmo
                    dsp     =
                    ci      = contre indications
                    med     = médecin (? double emploi)
                    mail    = mail -> id.
                    secre   = remarque secrétaire
                    ald     = ald
                    sec     = ?
                */
                QString rubrique = actlist.at(i).at(0).toString();
                if (rubrique == "atcd")
                {
                    QString atcdt = actlist.at(i).at(1).toString().replace("^","\n");
                    req = "update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set " CP_ATCDTSPERSOS_RMP " =  '" + Utils::correctquoteSQL(atcdt) + "' where " CP_IDPAT_RMP " = " + strgidpat;
                    //qDebug() << req;
                    db->StandardSQL(req);
                }
                else if (rubrique == "dgn")
                {
                    QString atcdt = actlist.at(i).at(1).toString().replace("^","\n");
                    req = "update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set " CP_ATCDTSOPH_RMP " =  '" + Utils::correctquoteSQL(atcdt) + "' where " CP_IDPAT_RMP " = " + strgidpat;
                    //qDebug() << req;
                    db->StandardSQL(req);
                }
            }
            else if (!listdates.contains(datetime))
                listdates << datetime;
        }
        for (int l=0; l< listdates.size(); l++)
        {
            QDateTime datetime = listdates.at(l);
            QString actedate = datetime.date().toString("yyyy-MM-dd");
            QString acteheure = datetime.time().toString("HH:mm");
            QString txt = "";
            QString motif = "";
            for (int j=0; j< actlist.size(); j++)
            {
                QDateTime datetimeenreg = QDateTime::fromString(actlist.at(j).at(2).toString(),"dd/MM/yyyy HHmm");
                if (datetimeenreg == datetime)
                {
                    QString rubrique = actlist.at(j).at(1).toString();
                    if (rubrique == "m")
                        motif += actlist.at(j).at(2).toString().replace("^","\n");
                    else
                        txt += "[" + rubrique + "]\t" + actlist.at(j).at(2).toString().replace("^","\n\t") + "\n";
                }
            }
            req= "insert into " TBL_ACTES " (" CP_IDPAT_ACTES ", " CP_IDUSER_ACTES ", " CP_TEXTE_ACTES ", " CP_DATE_ACTES ", " CP_HEURE_ACTES ", " CP_IDUSERCREATEUR_ACTES  ", "
                    CP_COTATION_ACTES  ", " CP_MONTANT_ACTES", " CP_MONNAIE_ACTES ", " CP_IDUSERPARENT_ACTES ", " CP_IDLIEU_ACTES ", " CP_MOTIF_ACTES ")"
                    " values (" + strgidpat + ", 1, '" + Utils::correctquoteSQL(txt) + "', '" + actedate + "', '" + acteheure + "', 1, "
                    " 'Acte gratuit', 0.00 , 'E', 1, 1, '" + Utils::correctquoteSQL(motif) + "')";
            //qDebug() << req;
            db->StandardSQL(req);
        }
    }
    req = "select " CP_ID_ACTES " from " TBL_ACTES;
    QList<QVariantList> actslist = db->StandardSelectSQL(req,ok);
    QString values = "";
    for (int i=0; i< actslist.size(); i++)
    {
        values += "(" + actslist.at(i).at(0).toString() + ", 'G')";
        if (i<actslist.size()-1)
            values += ", ";
    }
    req = "insert into " TBL_TYPEPAIEMENTACTES " (" CP_IDACTE_TYPEPAIEMENTACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ") values " + values;
    qDebug() << req;
    db->StandardSQL(req);
}
