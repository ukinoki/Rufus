/* (C) 2016 LAINE SERGE

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "conversionbase.h"

conversionbase::conversionbase(Procedures *proc, QString BaseAConvertir, QObject *parent) :
    QObject(parent)
{
    QString NomBase = BaseAConvertir;
    QSqlDatabase db = DataBase::getInstance()->getDataBase();
    /*COnvertir une base ophtalogic
    */
    //proc->RestaureBase(true,false,true);
    QString req = "select schema_name from information_schema.schemata"
                  " where schema_name <> 'information_schema'"
                  " and schema_name <> 'mysql'"
                  " and schema_name <> 'sys'"
                  " and schema_name <> 'performance_schema'"
                  " and schema_name <> '" NOM_BASE_CCAM
                  "' and schema_name <> '" NOM_BASE_CONSULTS
                  "' and schema_name <> '" NOM_BASE_COMPTA
                  "' and schema_name <> '" NOM_BASE_OPHTA "'";
    QSqlQuery quer(req,db);
    if (quer.size()==0)
    {
        UpMessageBox::Watch(0,"pas de base ophtalogic retrouvée");
        return;
    }
    else
    {
        NomBase = "";
        quer.first();
        for (int i=0; i<quer.size();i++)
        {
            if (QMessageBox::question(0,"Conversion d'une base Ophtalogic","Tenter de convertir la base " + quer.value(0).toString() + "?") == QMessageBox::Yes)
            {
                NomBase = quer.value(0).toString();
                break;
            }
        }
        if (NomBase=="")
            return;
        QSqlQuery("delete from " NOM_TABLE_PATIENTS,db);
        QSqlQuery("delete from " NOM_TABLE_DONNEESSOCIALESPATIENTS,db);
        QSqlQuery("delete from " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS,db);
        QSqlQuery("delete from " NOM_TABLE_ACTES,db);
        QSqlQuery("delete from " NOM_TABLE_TYPEPAIEMENTACTES,db);
        QSqlQuery("delete from " NOM_TABLE_LIGNESPAIEMENTS,db);
        QSqlQuery("delete from " NOM_TABLE_RECETTES,db);
        QSqlQuery("delete from " NOM_TABLE_CORRESPONDANTS,db);
        QSqlQuery("delete from " NOM_TABLE_REFRACTION,db);
        QSqlQuery("delete from " NOM_TABLE_IMPRESSIONS,db);

        QDate DDN,DateCreation;
        QString listpat, insertreq, listsocpat, listrmppat;
        QString idPat, idActe, idUser, Sexe;
        QString ALD, NNI, ref;
        QTime HeureCreation;
        QString txtCs;
        int max = 4;

        // Prescriptions
        proc->Message("importation des prescripions",1000);
        req = "select imp.Numéropatient, dateconsultation, objetordonnance, TypeOrdonnance, Abrègè, nompatient, prénom from (\n"
              "select ord.Numéropatient, ord.dateconsultation, objetordonnance, TypeOrdonnance, Abrègè\n from "  + NomBase + ".ordonnancespatients ord, "
                +  NomBase + ".consultations cs\n"
               " where ord.numéropatient = cs.numéropatient and ord.dateconsultation = cs.dateconsultation\n"
               " order by numéropatient) as imp\n"
               " left outer join (select numéropatient, nompatient, prénom from " + NomBase + ".patient) as pat\n"
               " on imp.numéropatient = pat.numéropatient";
        QSqlQuery ordoquery(req,db);
        QString prenom, nom, Entete, Corps, Pied;
        QString Typeprescription, Prescription;
        bool ALDQ;
        ordoquery.first();
        int b=0;
        for (int i=0; i< ordoquery.size(); i++)
        {
            DateCreation    = QDate::fromString(ordoquery.value(1).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            Typeprescription = ordoquery.value(3).toString();
            if (Typeprescription.toInt()>3)
                Typeprescription = "";
            if (Typeprescription == "1")
                Typeprescription = "Lentilles";
            else if (Typeprescription == "0")
                Typeprescription = "Prescription lunettes";
            else if (Typeprescription == "2")
                Typeprescription = "Prescription";
            Prescription= (Typeprescription=="3" ? "" : Typeprescription);
            if (Typeprescription == "3")
                Typeprescription = "Certificat - Demande examen";
            idUser = ordoquery.value(4).toString();
            if (idUser=="XXXXXX")
                idUser="1";
            else if (idUser=="YYYYYY")
                idUser="2";
            else
                idUser="21";
            ALDQ = ordoquery.value(2).toString().contains("affection de longue durée");
            idPat   = ordoquery.value(0).toString();
            nom     = ordoquery.value(5).toString();
            prenom  = Utils::trimcapitilize(ordoquery.value(6).toString());
            //création de l'entête
            Entete = (ALDQ? proc->ImpressionEntete(DateCreation, nullptr).value("ALD") : proc->ImpressionEntete(DateCreation, nullptr).value("Norm"));
            Entete.replace("{{TITRE1}}"            , "");
            Entete.replace("{{TITRE}}"             , "");
            Entete.replace("{{DDN}}"               , "");
            if (Prescription!= "")
            {
                Entete.replace("{{PRENOM PATIENT}}"    , prenom);
                Entete.replace("{{NOM PATIENT}}"       , nom.toUpper());
            }
            else
            {
                Entete.replace("{{PRENOM PATIENT}}"    , "");
                Entete.replace("{{NOM PATIENT}}"       , "");
            }

            //création du pied
            Pied = proc->ImpressionPied(false, ALDQ);

            // creation du corps
            Corps = proc->ImpressionCorps(ordoquery.value(2).toString(), ALDQ);
            Corps.remove("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>");
            Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
            Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");


            QSqlQuery query = QSqlQuery(db);
            // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
            query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, Titre, TextEntete, TextCorps, TextPied, Dateimpression, UserEmetteur, ALD)"
                                                               " values(:iduser, :idpat, :typeDoc, :titre, :textEntete, :textCorps, :textPied, :dateimpression, :useremetteur, :ald)");//, :ald)");
            query.bindValue(":iduser", idUser);
            query.bindValue(":idpat", idPat);
            query.bindValue(":typeDoc", Typeprescription);
            query.bindValue(":titre", Prescription);
            query.bindValue(":textEntete",Entete);
            query.bindValue(":textCorps", Corps);
            query.bindValue(":textPied", Pied);
            query.bindValue(":dateimpression", DateCreation.toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
            query.bindValue(":useremetteur", idUser);
            QVariant ALD100 = QVariant(QVariant::String);
            if (ALDQ) ALD100 = "1";
            query.bindValue(":ald", ALD100);
            query.exec();
            DataBase::getInstance()->erreurRequete(query,"problème pour enregistrer une prescription du patient " + nom.toUpper() + " " + prenom, "");
            ordoquery.next();
            if (b==100)
                b=0;
            if (b==0)
            {
                QTime dieTime= QTime::currentTime().addMSecs(2);
                while (QTime::currentTime() < dieTime)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
                proc->Message("importation des prescriptions - patient n° " + idPat);
            }
            b+=1;
        }
        proc->Message("table " NOM_TABLE_IMPRESSIONS " importée",1000);

        // Refractions
        proc->Message("importation des réfractions",1000);
        QString QuelleMesure;
        req = "select NuméroPatient, DateConsultation, HeureConsultation, Idx, SphèreDroit"                     //0,1,2,3,4
              ", CylindreDroit, AxeDroit, SphèreGauche, CylindreGauche, AxeGauche"                              //5,6,7,8,9
              ", AddDroit, AddGauche, AVLoinDroit, AVLoinGauche, AVPrèsDroit"                                   //10,11,12,13,14
              ", AVPrèsGauche, PD, PuissanceDroit, PuissanceGauche, BaseDroit"                                  //15,16,17,18,19
              ", BaseGauche, NuméroConsultation "                                                               //20,21
              " from " + NomBase + ".examenréfraction";
        QSqlQuery refquery(req,db);
        refquery.first();
        for (int i = 0; i<refquery.size();i++)
        {
            DateCreation    = QDate::fromString(refquery.value(1).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            switch (refquery.value(3).toInt()) {
            case 1:
                QuelleMesure = "P";
                break;
            case 2:
                QuelleMesure = "A";
                break;
            case 3:
                QuelleMesure = "R";
                break;
            case 4:
                QuelleMesure = "O";
                break;
            default:
                break;
            }
            ref = "(" + refquery.value(0).toString() + "," + refquery.value(21).toString() + ",'" + DateCreation.toString("yyyy-MM-dd") + "','" + QuelleMesure + "',"
                    + QString::number(QLocale().toDouble(refquery.value(4).toString()),'f',2) + ","
                    + QString::number(QLocale().toDouble(refquery.value(5).toString()),'f',2) + ","
                    + QString::number(refquery.value(6).toInt()) + ","
                    + QString::number(QLocale().toDouble(refquery.value(7).toString()),'f',2) + ","
                    + QString::number(QLocale().toDouble(refquery.value(8).toString()),'f',2) + ","
                    + QString::number(refquery.value(9).toInt()) + ","
                    + QString::number(QLocale().toDouble(refquery.value(10).toString()),'f',2) + ","
                    + QString::number(QLocale().toDouble(refquery.value(11).toString()),'f',2) + ",'"
                    + refquery.value(12).toString() + "','"
                    + refquery.value(13).toString() + "','"
                    + refquery.value(14).toString() + "','"
                    + refquery.value(15).toString() + "'"
                    + ")";
            insertreq = "insert into " NOM_TABLE_REFRACTION " (idPat, idActe, DateRefraction, QuelleMesure, SphereOD, CylindreOD, AxeCylindreOD, SphereOG, CylindreOG, AxeCylindreOG, AddVPOD, AddVPOG,"
                                                            "AVLOD, AVLOG, AVPOD, AVPOG) values " + ref;
            DataBase::getInstance()->erreurRequete(QSqlQuery(insertreq,db),insertreq,"");
            refquery.next();
        }
        proc->Message("table " NOM_TABLE_REFRACTION " importée",1000);

        // - Importation des données patients -------------------------------------------------------------------------------------------------------------------------------------------
        req = "select Numéropatient, nompatient, prénom, datenaiss, sexe, DateCréation,"
              " Adresse, CPostal, Ville, Téléphone, NuméroSS, ALD, Profession,"
              " TraitGen,TraitOph, AntécédentsObs, ANtécédentsFam"
              " from " + NomBase + ".patient order by numéropatient";
        QSqlQuery pat(req,db);
        //proc->Edit(req);
        max = pat.size();
        pat.first();
        proc->Message("importation des données patients",1000);
        for (int i=0; i<max; i++)
        {
            DDN             = QDate::fromString(pat.value(3).toString().left(10),"yyyy-MM-dd");
            if (!DDN.isValid()) DDN = QDate::fromString("2000-01-01","yyyy-MM-dd");
            DateCreation    = QDate::fromString(pat.value(5).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            idPat = pat.value(0).toString();
            Sexe = pat.value(4).toString();
            if (Sexe=="H") Sexe = "M";
            listpat = "(" + idPat + ",'"
                            + Utils::correctquoteSQL(pat.value(1).toString()).remove("!") + "','"
                            + Utils::correctquoteSQL(pat.value(2).toString()) + "','"
                            + DDN.toString("yyyy-MM-dd") + "','"
                            + Sexe + "','"
                            + DateCreation.toString("yyyy-MM-dd")
                            + "',1)";
            insertreq = "insert into " NOM_TABLE_PATIENTS " (idpat,patnom,patprenom,patDDN, Sexe, patCreele, Patcreepar) values \n" + listpat;
            //proc->Edit(insertreq);
            DataBase::getInstance()->erreurRequete(QSqlQuery(insertreq,db),insertreq,"");


            if (pat.value(10).toString() == "")     NNI = "null";   else NNI = QString::number(pat.value(10).toInt());
            if (pat.value(11).toString() == "-1")   ALD = "1";      else ALD = "0";
            listsocpat = "(" + idPat + ",'"
                            + Utils::correctquoteSQL(pat.value(6).toString().left(80)) + "','"
                            + pat.value(7).toString().left(5) + "','"
                            + Utils::correctquoteSQL(pat.value(8).toString().left(40)) + "','"
                            + Utils::correctquoteSQL(pat.value(9).toString().left(17)) + "',"
                            + NNI + ","
                            + ALD + ",'"
                            + Utils::correctquoteSQL(pat.value(12).toString().left(45)) + "')";
            insertreq = "insert into " NOM_TABLE_DONNEESSOCIALESPATIENTS " (idpat,patAdresse1,Patcodepostal,patville,pattelephone,patNNI,patALD,patprofession) values \n" + listsocpat;
            //proc->Edit(insertreq);
            DataBase::getInstance()->erreurRequete(QSqlQuery(insertreq,db),insertreq,"");

            if (Utils::correctquoteSQL(pat.value(13).toString())!= ""
                    || Utils::correctquoteSQL(pat.value(14).toString())!= ""
                    || Utils::correctquoteSQL(pat.value(15).toString())!= ""
                    || Utils::correctquoteSQL(pat.value(16).toString())!= ""
                    )
            {
                listrmppat = "(" + idPat + ",'"
                        + Utils::correctquoteSQL(pat.value(13).toString()) + "','"
                        + Utils::correctquoteSQL(pat.value(14).toString()) + "','"
                        + Utils::correctquoteSQL(pat.value(15).toString()) + "','"
                        + Utils::correctquoteSQL(pat.value(16).toString()) + "')";
                insertreq = "insert into " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " (idpat,RMPTtGeneral, RMPTtOphs, RMPAtcdtsOPhs, RMPAtcdtsFamiliaux) values \n" + listrmppat;
                //proc->Edit(insertreq);
                DataBase::getInstance()->erreurRequete(QSqlQuery(insertreq,db),insertreq,"");
            }

            pat.next();
        }
        req = " select idPat, Patnom from " NOM_TABLE_PATIENTS;
        QSqlQuery patnom(req,db);
        patnom.first();
        for (int i=0;i<patnom.size();i++)
        {
            req = "update " NOM_TABLE_PATIENTS " set Patnom = '" + Utils::correctquoteSQL(Utils::trimcapitilize(patnom.value(1).toString())) + "' where idpat = "  + patnom.value(0).toString();
            DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
            patnom.next();
        }

        proc->Message("table " NOM_TABLE_PATIENTS " importée",1000);
        proc->Message("table " NOM_TABLE_DONNEESSOCIALESPATIENTS " importée",1000);
        proc->Message("table " NOM_TABLE_RENSEIGNEMENTSMEDICAUXPATIENTS " importée",1000);

        // - Importation des actes -------------------------------------------------------------------------------------------------------------------------------------------
        proc->Message("importation des actes - date, motif et diagnostic",1000);
        req = "select NuméroConsultation, numéroPatient, DateConsultation, Motif, Diagnostic, HeureConsultation"
              " from " + NomBase + ".examensymdiagmotif order by numéroconsultation";
        QSqlQuery acte(req,db);
        //proc->Edit(req);
        max = acte.size();
        acte.first();
        for (int i=0; i<max; i++)
        {
            DateCreation    = QDate::fromString(acte.value(2).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            HeureCreation    = QTime::fromString(acte.value(5).toString(),"HH:mm");
            if (!HeureCreation.isValid()) HeureCreation = QTime::fromString("00:00","HH:mm");
            idActe = acte.value(0).toString();
            idPat = acte.value(1).toString();
            listpat = "(" + idActe + "," + idPat + ",'"
                            + DateCreation.toString("yyyy-MM-dd") + "','"
                            + Utils::correctquoteSQL(acte.value(3).toString()) + "','"
                            + Utils::correctquoteSQL(acte.value(4).toString()) + "','"
                            + HeureCreation.toString("HH:mm")
                            + "')";
            insertreq = "insert into " NOM_TABLE_ACTES " (idActe,idpat,ActeDate,Actemotif, ActeConclusion, ActeHeure) values \n" + listpat;
            //proc->Edit(insertreq);
            DataBase::getInstance()->erreurRequete(QSqlQuery(insertreq,db),insertreq,"");

            acte.next();
        }

        // Corps des consultations et honoraires
        proc->Message("importation des actes - texte des consultations",1000);
        req = "select NuméroConsultation, numéroPatient, Abrégé, DateConsultation, TexteConsultation, TotalActesE, HeureConsultation"
              " from " + NomBase + ".consultations order by numéroconsultation";
        QSqlQuery csquery(req,db);
        max = csquery.size();
        //max = 4;
        csquery.first();
        for (int i=0; i<max; i++)
        {
            idActe = csquery.value(0).toString();
            idPat = csquery.value(1).toString();
            idUser = csquery.value(2).toString();
            if (idUser=="XXXXXX")
                idUser="1";
            else if (idUser=="YYYYYY")
                idUser="2";
            else
                idUser="21";
            txtCs = csquery.value(4).toString().remove("\\par ");
            txtCs.remove("\\cf1");
            txtCs.remove("\\cf0");
            txtCs.remove("\\tab");
            txtCs.remove("\\pard");
            txtCs.remove("\\brdrt");
            txtCs.remove("\\brdrs");
            txtCs.remove("\\brdrw20");
            txtCs.remove("\\brdrl");
            txtCs.remove("\\brdrr");
            txtCs.remove("\\keepn");
            txtCs.remove("\\qc");
            txtCs.remove("\\ul");
            txtCs.remove("\fs32");
            txtCs.remove("\\ulnone");
            txtCs.remove("\\fs24");
            txtCs.remove("\\lang1024");
            txtCs.remove("\\lang1036");
            txtCs.remove("\\plain");
            txtCs.remove("\\f2");
            txtCs.remove("\\fs20");
            txtCs.remove("\\f1");
            txtCs.remove("\\fs32");
            txtCs.remove("\\ltrpar");
            txtCs.remove("\\nowidctlpar");
            txtCs.remove("\\ltrparnone");
            txtCs.remove("rdrb");
            txtCs.remove("\\f0");

            int j = txtCs.indexOf("Observation");
            txtCs = txtCs.right(txtCs.length()-j);
            j = txtCs.indexOf("Motif :");
            if (j>-1)
            {
                txtCs = txtCs.right(txtCs.length()-j-7);
                j = txtCs.indexOf(":");
                QString AModif = txtCs.left(j+1);
                int idxlastlne = AModif.lastIndexOf("\n");
                QString Modif = AModif.right(AModif.length()-idxlastlne);
                txtCs = txtCs.right(txtCs.length()-j);
                txtCs = Modif + txtCs;
                //proc->Edit("Amodif = " + AModif + "\nidx = " + QString::number(idxlastlne) + "\ntxtCs = " + txtCs + "\nModif = " + Modif + "\nModiffinal = " + Modif+txtCs);
            }
            txtCs.replace("\\'e9","é");
            txtCs.replace("\\'e8","è");
            txtCs.replace("\\'e0","à");
            txtCs.replace("\\'f4","ô");
            txtCs.replace("\\'b0","°");
            txtCs.replace("\n\\b0","\\b0");
            txtCs.replace("\\b0 ","\\b0");
            txtCs.replace("\\b0\n","\\b0");
            txtCs.replace("\\b0\n","\\b0");
            txtCs.replace("\\b0 ","\\b0");
            txtCs.replace("\\b0","\\b0\n");
            txtCs.replace("\n\\b0","\\b0");
            txtCs.replace("\n ","\n");
            txtCs.remove("}");
            QTextEdit txtEdit;
            txtEdit.setText(txtCs);
            txtCs = txtEdit.toHtml();
            txtCs.remove("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n");
            txtCs.remove("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>");
           txtCs.replace("\\b0","</font></b>");
           txtCs.replace("\\b","<font color = \"blue\"><b>");
           txtCs.replace("<font color = \"blue\"><b> </p>\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<font color = \"blue\"><b>");
           txtCs.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
           txtCs.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");

            req = "select idActe from " NOM_TABLE_ACTES " where idActe = " + idActe;
            QSqlQuery seekcs(req,db);
            if (seekcs.size()>0)
            {
                req = "update " NOM_TABLE_ACTES " set Actetexte = '" + Utils::correctquoteSQL(txtCs) + "', idUser = " + idUser + ", ActeMontant = " + csquery.value(5).toString() + ", CreePar = " + idUser +
                        " where idActe = " + idActe;
                //proc->Edit(req);
                DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
            }
            else
            {
                DateCreation    = QDate::fromString(csquery.value(3).toString().left(10),"yyyy-MM-dd");
                if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
                HeureCreation    = QTime::fromString(csquery.value(6).toString(),"HH:mm");
                if (!HeureCreation.isValid()) HeureCreation = QTime::fromString("00:00","HH:mm");
                listpat = "(" + idActe + "," + idPat + "," + idUser + ",'"
                            + DateCreation.toString("yyyy-MM-dd") + "','"
                            + Utils::correctquoteSQL(txtCs) + "',"
                            + csquery.value(5).toString() + ",'"
                            + HeureCreation.toString("HH:mm") + "',"
                            + idUser
                            + ")";
                insertreq = "insert into " NOM_TABLE_ACTES " (idActe, idPat, idUser, ActeDate, ActeTexte, ActeMontant, ActeHeure, CreePar) values \n" + listpat;
                //proc->Edit(insertreq);
                DataBase::getInstance()->erreurRequete(QSqlQuery(insertreq,db),insertreq,"");
            }
            csquery.next();
        }


        // Cotations
        proc->Message("importation des actes - cotations",1000);
        req = "select Numéroconsultation, acte from "  + NomBase + ".actespatients order by numéroconsultation";
        QSqlQuery cotquery(req,db);
        cotquery.first();
        for (int i=0; i< cotquery.size(); i++)
        {
            req = "update " NOM_TABLE_ACTES " set actecotation = '" + Utils::correctquoteSQL(cotquery.value(1).toString().left(20)) +
                    "' where idacte = " + cotquery.value(0).toString();
            DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
            cotquery.next();
        }
        QSqlQuery("update " NOM_TABLE_ACTES " set actecotation = 'xxx' where actecotation is null",db);
        proc->Message("table " NOM_TABLE_ACTES " importée",1000);

        // Paiements
        proc->Message("paiements - espèces pour tout le monde",1000);
        req = "select idActe, actemontant, idUser, actedate from " NOM_TABLE_ACTES;
        QSqlQuery pai(req,db);
        pai.first();
        for (int i=0; i< pai.size(); i++)
        {
            DateCreation    = QDate::fromString(pai.value(3).toString().left(10),"yyyy-MM-dd");
            QString typepai = "E";
            if (pai.value(1).toDouble()==0)
                typepai = "G";
            req = "insert into " NOM_TABLE_TYPEPAIEMENTACTES " (idacte, typepaiement) values (" + pai.value(0).toString() + ",'" + typepai + "')";
            DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
            if (typepai=="E")
            {
                req = "insert into " NOM_TABLE_LIGNESPAIEMENTS " (idacte, idRecette, Paye) values (" + pai.value(0).toString() + "," + pai.value(0).toString() + "," + pai.value(1).toString() + ")";
                DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
                req = "insert into " NOM_TABLE_RECETTES " (idRecette, idUser, DatePaiement, DateEnregistrement, Montant, Modepaiement,Monnaie, EnregistrePar,TypeRecette) values (" +
                        pai.value(0).toString() + "," + pai.value(2).toString() + ",'" + DateCreation.toString("yyyy-MM-dd") + "','" + DateCreation.toString("yyyy-MM-dd") + "'," +
                        pai.value(1).toString() + ",'E','E'," + pai.value(2).toString() + ",1)";
                DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
            }
            pai.next();
        }
        proc->Message("table " NOM_TABLE_TYPEPAIEMENTACTES " importée",1000);
        proc->Message("table " NOM_TABLE_LIGNESPAIEMENTS " importée",1000);
        proc->Message("table " NOM_TABLE_RECETTES " importée",1000);

        // Correspondants
        proc->Message("médecins correspondants",1000);
        req = "select NuméroConfrère, Nom, Prénom, Adresse, CPostal, Ville, Téléphone, Fax, Spécialité from " + NomBase + ".Confrères";
        QSqlQuery med(req,db);
        med.first();
        for (int i=0; i< med.size(); i++)
        {
            req = "insert into " NOM_TABLE_CORRESPONDANTS " (idCor, CorNom, CorPrenom, CorAdresse1, CorCodePostal, CorVille, CorTelephone, CorFax, CorMedecin, CorSpecialite) values (" +
                    med.value(0).toString() +
                    ",'" + Utils::correctquoteSQL(med.value(1).toString().left(70)) +
                    "','" + Utils::correctquoteSQL(med.value(2).toString().left(70)) +
                    "','" + Utils::correctquoteSQL(med.value(3).toString().left(70)) +
                    "','" + Utils::correctquoteSQL(med.value(4).toString().left(5)) +
                    "','" + Utils::correctquoteSQL(med.value(5).toString().left(40)) +
                    "','" + Utils::correctquoteSQL(med.value(6).toString().left(17)) +
                    "','" + Utils::correctquoteSQL(med.value(7).toString().left(17)) +
                    "',1,'" + Utils::correctquoteSQL(med.value(8).toString().left(45)) + "')";
            DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
            med.next();
        }
        req = "update " NOM_TABLE_CORRESPONDANTS " set corspecialite = 0 where corspecialite = 'Généraliste'";
        DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
        req = " select idCor, cornom from " NOM_TABLE_CORRESPONDANTS;
        QSqlQuery cor(req,db);
        cor.first();
        for (int i=0;i<cor.size();i++)
        {
            req = "update " NOM_TABLE_CORRESPONDANTS " set cornom = '" + Utils::correctquoteSQL(Utils::trimcapitilize(cor.value(1).toString())) + "' where idcor = "  + cor.value(0).toString();
            DataBase::getInstance()->erreurRequete(QSqlQuery(req,db),req,"");
            cor.next();
        }
        proc->Message("table " NOM_TABLE_CORRESPONDANTS " importée",1000);
    }
}
