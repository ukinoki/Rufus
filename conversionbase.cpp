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


#include "conversionbase.h"

conversionbase::conversionbase(QObject *parent) : QObject(parent)
{
    db = DataBase::I();
    proc = Procedures::I();
}

void conversionbase::conversionbaseophtalogic()
{
    /*COnvertir une base ophtalogic
    */
    //proc->RestaureBase(true,false,true);
    QString req = "select schema_name from information_schema.schemata"
                  " where schema_name <> 'information_schema'"
                  " and schema_name <> 'mysql'"
                  " and schema_name <> 'sys'"
                  " and schema_name <> 'performance_schema'"
                  " and schema_name <> '" DB_CCAM
                  "' and schema_name <> '" DB_CONSULTS
                  "' and schema_name <> '" DB_COMPTA
                  "' and schema_name <> '" DB_OPHTA
                  "' and schema_name <> 'Oplus'";
    QList<QVariantList> lgclist = db->StandardSelectSQL(req,ok);
    if (lgclist.size()==0)
    {
        UpMessageBox::Watch(Q_NULLPTR,"pas de base ophtalogic retrouvée");
        return;
    }
    else
    {
        QString NomBase = "";
        for (int i=0; i<lgclist.size();i++)
        {
            if (QMessageBox::question(Q_NULLPTR,"Conversion d'une base Ophtalogic","Tenter de convertir la base " + lgclist.at(i).at(0).toString() + "?") == QMessageBox::Yes)
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

        QDate DDN,DateCreation;
        QString listpat, insertreq, listsocpat, listrmppat;
        QString idPat, idActe, idUser, Sexe;
        QString ALD, NNI, ref;
        QTime HeureCreation;
        QString txtCs;
        int max = 4;

        // Prescriptions
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des prescripions", Icons::icSunglasses(), 1000);
        req = "select imp.Numéropatient, dateconsultation, objetordonnance, TypeOrdonnance, Abrègè, nompatient, prénom from (\n"
              "select ord.Numéropatient, ord.dateconsultation, objetordonnance, TypeOrdonnance, Abrègè\n from "  + NomBase + ".ordonnancespatients ord, "
                +  NomBase + ".consultations cs\n"
               " where ord.numéropatient = cs.numéropatient and ord.dateconsultation = cs.dateconsultation\n"
               " order by numéropatient) as imp\n"
               " left outer join (select numéropatient, nompatient, prénom from " + NomBase + ".patient) as pat\n"
               " on imp.numéropatient = pat.numéropatient";
        QList<QVariantList> ordolist = db->StandardSelectSQL(req,ok);
        QString prenom, nom, Entete, Corps, Pied;
        QString Typeprescription, Prescription;
        bool ALDQ;
        int b=0;
        for (int i=0; i< ordolist.size(); i++)
        {
            DateCreation    = QDate::fromString(ordolist.at(i).at(1).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            Typeprescription = ordolist.at(i).at(3).toString();
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
            idUser = ordolist.at(i).at(4).toString();
            if (idUser=="XXXXXX")
                idUser="1";
            else if (idUser=="YYYYYY")
                idUser="2";
            else
                idUser="21";
            ALDQ = ordolist.at(i).at(2).toString().contains("affection de longue durée");
            idPat   = ordolist.at(i).at(0).toString();
            nom     = ordolist.at(i).at(5).toString();
            prenom  = Utils::trimcapitilize(ordolist.at(i).at(6).toString());
            //création de l'entête
            Entete = (ALDQ? proc->CalcEnteteImpression(DateCreation, Q_NULLPTR).value("ALD") : proc->CalcEnteteImpression(DateCreation, Q_NULLPTR).value("Norm"));
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
            Pied = proc->CalcPiedImpression(Datas::I()->users->getById(idUser.toInt()),false, ALDQ);

            // creation du corps
            Corps = proc->CalcCorpsImpression(ordolist.at(i).at(2).toString(), ALDQ);
            Corps.remove("<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>");
            Corps.replace("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">","<p style=\" margin-top:0px; margin-bottom:0px;\">");
            Corps.remove("border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" ");

            QHash<QString, QVariant> listbinds;
            listbinds["iduser"] = idUser;
            listbinds["idpat"] = idPat;
            listbinds["typeDoc"] = Typeprescription;
            listbinds["titre"] = Prescription;
            listbinds["textEntete"] =Entete;
            listbinds["textCorps"] = Corps;
            listbinds["textPied"] = Pied;
            listbinds["dateimpression"] = DateCreation.toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
            listbinds["useremetteur"] = idUser;
            QVariant ALD100 = QVariant(QVariant::String);
            if (ALDQ) ALD100 = "1";
            listbinds["ald"] = ALD100;

            db->InsertSQLByBinds(TBL_DOCSEXTERNES, listbinds, "problème pour enregistrer une prescription du patient " + nom.toUpper() + " " + prenom);
            if (b==100)
                b=0;
            if (b==0)
            {
                QTime dieTime= QTime::currentTime().addMSecs(2);
                while (QTime::currentTime() < dieTime)
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
                UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des prescriptions - patient n° " + idPat, Icons::icSunglasses(), 3000);
            }
            b+=1;
        }
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_DOCSEXTERNES " importée", Icons::icSunglasses(), 1000);

        // Refractions
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des réfractions", Icons::icSunglasses(), 1000);
        QString QuelleMesure;
        req = "select NuméroPatient, DateConsultation, HeureConsultation, Idx, SphèreDroit"                     //0,1,2,3,4
              ", CylindreDroit, AxeDroit, SphèreGauche, CylindreGauche, AxeGauche"                              //5,6,7,8,9
              ", AddDroit, AddGauche, AVLoinDroit, AVLoinGauche, AVPrèsDroit"                                   //10,11,12,13,14
              ", AVPrèsGauche, PD, PuissanceDroit, PuissanceGauche, BaseDroit"                                  //15,16,17,18,19
              ", BaseGauche, NuméroConsultation "                                                               //20,21
              " from " + NomBase + ".examenréfraction";
        QList<QVariantList> reflist = db->StandardSelectSQL(req,ok);
        for (int i = 0; i<reflist.size();i++)
        {
            DateCreation    = QDate::fromString(reflist.at(i).at(1).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            switch (reflist.at(i).at(3).toInt()) {
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
            ref = "(" + reflist.at(i).at(0).toString() + "," + reflist.at(i).at(21).toString() + ",'" + DateCreation.toString("yyyy-MM-dd") + "','" + QuelleMesure + "',"
                    + QString::number(QLocale().toDouble(reflist.at(i).at(4).toString()),'f',2) + ","
                    + QString::number(QLocale().toDouble(reflist.at(i).at(5).toString()),'f',2) + ","
                    + QString::number(reflist.at(i).at(6).toInt()) + ","
                    + QString::number(QLocale().toDouble(reflist.at(i).at(7).toString()),'f',2) + ","
                    + QString::number(QLocale().toDouble(reflist.at(i).at(8).toString()),'f',2) + ","
                    + QString::number(reflist.at(i).at(9).toInt()) + ","
                    + QString::number(QLocale().toDouble(reflist.at(i).at(10).toString()),'f',2) + ","
                    + QString::number(QLocale().toDouble(reflist.at(i).at(11).toString()),'f',2) + ",'"
                    + reflist.at(i).at(12).toString() + "','"
                    + reflist.at(i).at(13).toString() + "','"
                    + reflist.at(i).at(14).toString() + "','"
                    + reflist.at(i).at(15).toString() + "'"
                    + ")";
            insertreq = "insert into " TBL_REFRACTIONS " (idPat, idActe, DateRefraction, QuelleMesure, SphereOD, CylindreOD, AxeCylindreOD, SphereOG, CylindreOG, AxeCylindreOG, AddVPOD, AddVPOG,"
                                                            "AVLOD, AVLOG, AVPOD, AVPOG) values " + ref;
            db->StandardSQL(insertreq);
        }
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_REFRACTIONS " importée", Icons::icSunglasses(), 1000);

        // - Importation des données patients -------------------------------------------------------------------------------------------------------------------------------------------
        req = "select Numéropatient, nompatient, prénom, datenaiss, sexe, DateCréation,"
              " Adresse, CPostal, Ville, Téléphone, NuméroSS, ALD, Profession,"
              " TraitGen,TraitOph, AntécédentsObs, ANtécédentsFam"
              " from " + NomBase + ".patient order by numéropatient";
        QList<QVariantList> patlist = db->StandardSelectSQL(req,ok);
        //proc->Edit(req);
        max = patlist.size();
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des données patients", Icons::icSunglasses(), 1000);
        for (int i=0; i<max; i++)
        {
            DDN             = QDate::fromString(patlist.at(i).at(3).toString().left(10),"yyyy-MM-dd");
            if (!DDN.isValid()) DDN = QDate::fromString("2000-01-01","yyyy-MM-dd");
            DateCreation    = QDate::fromString(patlist.at(i).at(5).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            idPat = patlist.at(i).at(0).toString();
            Sexe = patlist.at(i).at(4).toString();
            if (Sexe=="H") Sexe = "M";
            listpat = "(" + idPat + ",'"
                            + Utils::correctquoteSQL(patlist.at(i).at(1).toString()).remove("!") + "','"
                            + Utils::correctquoteSQL(patlist.at(i).at(2).toString()) + "','"
                            + DDN.toString("yyyy-MM-dd") + "','"
                            + Sexe + "','"
                            + DateCreation.toString("yyyy-MM-dd")
                            + "',1)";
            insertreq = "insert into " TBL_PATIENTS " (idpat,patnom,patprenom,patDDN, Sexe, patCreele, Patcreepar) values \n" + listpat;
            //proc->Edit(insertreq);
            db->StandardSQL(insertreq);


            if (patlist.at(i).at(10).toString() == "")     NNI = "null";   else NNI = QString::number(patlist.at(i).at(10).toInt());
            if (patlist.at(i).at(11).toString() == "-1")   ALD = "1";      else ALD = "0";
            listsocpat = "(" + idPat + ",'"
                            + Utils::correctquoteSQL(patlist.at(i).at(6).toString().left(80)) + "','"
                            + patlist.at(i).at(7).toString().left(5) + "','"
                            + Utils::correctquoteSQL(patlist.at(i).at(8).toString().left(40)) + "','"
                            + Utils::correctquoteSQL(patlist.at(i).at(9).toString().left(17)) + "',"
                            + NNI + ","
                            + ALD + ",'"
                            + Utils::correctquoteSQL(patlist.at(i).at(12).toString().left(45)) + "')";
            insertreq = "insert into " TBL_DONNEESSOCIALESPATIENTS " (idpat,patAdresse1,Patcodepostal,patville,pattelephone,patNNI,patALD,patprofession) values \n" + listsocpat;
            //proc->Edit(insertreq);
            db->StandardSQL(insertreq);

            if (Utils::correctquoteSQL(patlist.at(i).at(13).toString())!= ""
                    || Utils::correctquoteSQL(patlist.at(i).at(14).toString())!= ""
                    || Utils::correctquoteSQL(patlist.at(i).at(15).toString())!= ""
                    || Utils::correctquoteSQL(patlist.at(i).at(16).toString())!= ""
                    )
            {
                listrmppat = "(" + idPat + ",'"
                        + Utils::correctquoteSQL(patlist.at(i).at(13).toString()) + "','"
                        + Utils::correctquoteSQL(patlist.at(i).at(14).toString()) + "','"
                        + Utils::correctquoteSQL(patlist.at(i).at(15).toString()) + "','"
                        + Utils::correctquoteSQL(patlist.at(i).at(16).toString()) + "')";
                insertreq = "insert into " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " (idpat,RMPTtGeneral, RMPTtOphs, RMPAtcdtsOPhs, RMPAtcdtsFamiliaux) values \n" + listrmppat;
                //proc->Edit(insertreq);
                db->StandardSQL(insertreq);
            }
        }
        req = " select idPat, Patnom from " TBL_PATIENTS;
        patlist = db->StandardSelectSQL(req,ok);
        for (int i=0;i<patlist.size();i++)
        {
            req = "update " TBL_PATIENTS " set Patnom = '" + Utils::correctquoteSQL(Utils::trimcapitilize(patlist.at(i).at(1).toString())) + "' where idpat = "  + patlist.at(i).at(0).toString();
            db->StandardSQL(req);
        }

        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_PATIENTS " importée", Icons::icSunglasses(), 1000);
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_DONNEESSOCIALESPATIENTS " importée", Icons::icSunglasses(), 1000);
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " importée", Icons::icSunglasses(), 1000);

        // - Importation des actes -------------------------------------------------------------------------------------------------------------------------------------------
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des actes - date, motif et diagnostic", Icons::icSunglasses(), 1000);
        req = "select NuméroConsultation, numéroPatient, DateConsultation, Motif, Diagnostic, HeureConsultation"
              " from " + NomBase + ".examensymdiagmotif order by numéroconsultation";
        QList<QVariantList> actlist = db->StandardSelectSQL(req,ok);
        //proc->Edit(req);
        max = actlist.size();
        for (int i=0; i<max; i++)
        {
            DateCreation    = QDate::fromString(actlist.at(i).at(2).toString().left(10),"yyyy-MM-dd");
            if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
            HeureCreation    = QTime::fromString(actlist.at(i).at(5).toString(),"HH:mm");
            if (!HeureCreation.isValid()) HeureCreation = QTime::fromString("00:00","HH:mm");
            idActe = actlist.at(i).at(0).toString();
            idPat = actlist.at(i).at(1).toString();
            listpat = "(" + idActe + "," + idPat + ",'"
                            + DateCreation.toString("yyyy-MM-dd") + "','"
                            + Utils::correctquoteSQL(actlist.at(i).at(3).toString()) + "','"
                            + Utils::correctquoteSQL(actlist.at(i).at(4).toString()) + "','"
                            + HeureCreation.toString("HH:mm")
                            + "')";
            insertreq = "insert into " TBL_ACTES " (idActe,idpat,ActeDate,Actemotif, ActeConclusion, ActeHeure) values \n" + listpat;
            //proc->Edit(insertreq);
            db->StandardSQL(insertreq);
        }

        // Corps des consultations et honoraires
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des actes - texte des consultations", Icons::icSunglasses(), 1000);
        req = "select NuméroConsultation, numéroPatient, Abrégé, DateConsultation, TexteConsultation, TotalActesE, HeureConsultation"
              " from " + NomBase + ".consultations order by numéroconsultation";
        QList<QVariantList> cslist = db->StandardSelectSQL(req,ok);
        max = cslist.size();
        //max = 4;
        for (int i=0; i<max; i++)
        {
            idActe = cslist.at(i).at(0).toString();
            idPat = cslist.at(i).at(1).toString();
            idUser = cslist.at(i).at(2).toString();
            if (idUser=="XXXXXX")
                idUser="1";
            else if (idUser=="YYYYYY")
                idUser="2";
            else
                idUser="21";
            txtCs = cslist.at(i).at(4).toString().remove("\\par ");
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

            req = "select idActe from " TBL_ACTES " where idActe = " + idActe;
            QVariantList csdata = db->getFirstRecordFromStandardSelectSQL(req,ok);
            if (csdata.size()>0)
            {
                req = "update " TBL_ACTES " set Actetexte = '" + Utils::correctquoteSQL(txtCs) + "', idUser = " + idUser + ", ActeMontant = " + cslist.at(i).at(5).toString() + ", CreePar = " + idUser +
                        " where idActe = " + idActe;
                //proc->Edit(req);
                db->StandardSQL(req);
            }
            else
            {
                DateCreation    = QDate::fromString(cslist.at(i).at(3).toString().left(10),"yyyy-MM-dd");
                if (!DateCreation.isValid()) DateCreation = QDate::fromString("2000-01-01","yyyy-MM-dd");
                HeureCreation    = QTime::fromString(cslist.at(i).at(6).toString(),"HH:mm");
                if (!HeureCreation.isValid()) HeureCreation = QTime::fromString("00:00","HH:mm");
                listpat = "(" + idActe + "," + idPat + "," + idUser + ",'"
                            + DateCreation.toString("yyyy-MM-dd") + "','"
                            + Utils::correctquoteSQL(txtCs) + "',"
                            + cslist.at(i).at(5).toString() + ",'"
                            + HeureCreation.toString("HH:mm") + "',"
                            + idUser
                            + ")";
                insertreq = "insert into " TBL_ACTES " (idActe, idPat, idUser, ActeDate, ActeTexte, ActeMontant, ActeHeure, CreePar) values \n" + listpat;
                //proc->Edit(insertreq);
                db->StandardSQL(insertreq);
            }
        }


        // Cotations
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des actes - cotations", Icons::icSunglasses(), 1000);
        req = "select Numéroconsultation, acte from "  + NomBase + ".actespatients order by numéroconsultation";
        QList<QVariantList> cotlist = db->StandardSelectSQL(req,ok);
        for (int i=0; i< cotlist.size(); i++)
        {
            req = "update " TBL_ACTES " set actecotation = '" + Utils::correctquoteSQL(cotlist.at(i).at(1).toString().left(20)) +
                    "' where idacte = " + cotlist.at(i).at(0).toString();
            db->StandardSQL(req);
        }
        db->StandardSQL("update " TBL_ACTES " set actecotation = 'xxx' where actecotation is null");
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_ACTES " importée", Icons::icSunglasses(), 1000);

        // Paiements
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "paiements - espèces pour tout le monde", Icons::icSunglasses(), 1000);
        req = "select idActe, actemontant, idUser, actedate from " TBL_ACTES;
        QList<QVariantList> pmtlist = db->StandardSelectSQL(req,ok);
        for (int i=0; i< pmtlist.size(); i++)
        {
            DateCreation    = QDate::fromString(pmtlist.at(i).at(3).toString().left(10),"yyyy-MM-dd");
            QString typepai = "E";
            if (pmtlist.at(i).at(1).toDouble()==0.0)
                typepai = "G";
            req = "insert into " TBL_TYPEPAIEMENTACTES " (idacte, typepaiement) values (" + pmtlist.at(i).at(0).toString() + ",'" + typepai + "')";
            db->StandardSQL(req);
            if (typepai=="E")
            {
                req = "insert into " TBL_LIGNESPAIEMENTS " (idacte, idRecette, Paye) values (" + pmtlist.at(i).at(0).toString() + "," + pmtlist.at(i).at(0).toString() + "," + pmtlist.at(i).at(1).toString() + ")";
                db->StandardSQL(req);
                req = "insert into " TBL_RECETTES " (idRecette, idUser, DatePaiement, DateEnregistrement, Montant, Modepaiement,Monnaie, EnregistrePar,TypeRecette) values (" +
                        pmtlist.at(i).at(0).toString() + "," + pmtlist.at(i).at(2).toString() + ",'" + DateCreation.toString("yyyy-MM-dd") + "','" + DateCreation.toString("yyyy-MM-dd") + "'," +
                        pmtlist.at(i).at(1).toString() + ",'E','E'," + pmtlist.at(i).at(2).toString() + ",1)";
                db->StandardSQL(req);
            }
        }
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_TYPEPAIEMENTACTES " importée", Icons::icSunglasses(), 1000);
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_LIGNESPAIEMENTS " importée", Icons::icSunglasses(), 1000);
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_RECETTES " importée", Icons::icSunglasses(), 1000);

        // Correspondants
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "médecins correspondants", Icons::icSunglasses(), 1000);
        req = "select NuméroConfrère, Nom, Prénom, Adresse, CPostal, Ville, Téléphone, Fax, Spécialité from " + NomBase + ".Confrères";
        QList<QVariantList> corlist = db->StandardSelectSQL(req,ok);
        for (int i=0; i< corlist.size(); i++)
        {
            req = "insert into " TBL_CORRESPONDANTS " (idCor, CorNom, CorPrenom, CorAdresse1, CorCodePostal, CorVille, CorTelephone, CorFax, CorMedecin, CorSpecialite) values (" +
                    corlist.at(i).at(0).toString() +
                    ",'" + Utils::correctquoteSQL(corlist.at(i).at(1).toString().left(70)) +
                    "','" + Utils::correctquoteSQL(corlist.at(i).at(2).toString().left(70)) +
                    "','" + Utils::correctquoteSQL(corlist.at(i).at(3).toString().left(70)) +
                    "','" + Utils::correctquoteSQL(corlist.at(i).at(4).toString().left(5)) +
                    "','" + Utils::correctquoteSQL(corlist.at(i).at(5).toString().left(40)) +
                    "','" + Utils::correctquoteSQL(corlist.at(i).at(6).toString().left(17)) +
                    "','" + Utils::correctquoteSQL(corlist.at(i).at(7).toString().left(17)) +
                    "',1,'" + Utils::correctquoteSQL(corlist.at(i).at(8).toString().left(45)) + "')";
            db->StandardSQL(req);
        }
        req = "update " TBL_CORRESPONDANTS " set corspecialite = 0 where corspecialite = 'Généraliste'";
        db->StandardSQL(req);
        req = " select idCor, cornom from " TBL_CORRESPONDANTS;
        corlist = db->StandardSelectSQL(req,ok);
        for (int i=0;i<corlist.size();i++)
        {
            req = "update " TBL_CORRESPONDANTS " set cornom = '" + Utils::correctquoteSQL(Utils::trimcapitilize(corlist.at(i).at(1).toString())) + "' where idcor = "  + corlist.at(i).at(0).toString();
            db->StandardSQL(req);
        }
        UpSystemTrayIcon::I()->showMessage(tr("Messages"), "table " TBL_CORRESPONDANTS " importée", Icons::icSunglasses(), 1000);
    }
}

void conversionbase::conversionbaseoplus()
{
    /*! IMPORT OPLUS
     * Il y a 2 fichiers CSV et un dossier pieces_jointes contenant l'imagerie
     * 1. mettre tout dans un seul dossier (laine dans l'exemple suivant) qu'on recopiera dans le dossier imagerie de Rufus pour que MySQL puisse y accèder
        * actes.csv
        * identites_patients.csv
        * pieces_jointes (dosssier)
     * 2. Importer les fichiers csv dans une base de données mySQL Oplus
        * éxécuter le script MySQL suivant

    CREATE SCHEMA `Oplus`;
    Use `Oplus`;
    CREATE TABLE `Oplus`.`identites_patients` (
      `nopat` INT NOT NULL AUTO_INCREMENT,
      `sexe` VARCHAR(10) NULL,
      `titre` VARCHAR(4) NULL,
      `nom_prenom` VARCHAR(45) NULL,
      `adr1` VARCHAR(100) NULL,
      `adr2` VARCHAR(100) NULL,
      `codepostal` VARCHAR(5) NULL,
      `ville` VARCHAR(100) NULL,
      `date_naiss` VARCHAR(10) NULL,
      `noSS` VARCHAR(45) NULL,
      `nom_naiss` VARCHAR(45) NULL,
      `tel` VARCHAR(45) NULL,
      `tel_bur` VARCHAR(45) NULL,
      `tel_port` VARCHAR(45) NULL,
      `email` VARCHAR(45) NULL,
      `prat_usuel` VARCHAR(45) NULL,
      `med_trait` VARCHAR(45) NULL,
      `org` VARCHAR(45) NULL,
      `profession` VARCHAR(45) NULL,
      `divers` VARCHAR(45) NULL,
      PRIMARY KEY (`nopat`));
    LOAD DATA INFILE '/Users/serge/Documents/Rufus/laine/identites_patients.csv'
    INTO TABLE `Oplus`.`identites_patients`
    FIELDS
        TERMINATED BY '|'
    LINES
        TERMINATED BY '\n'
        IGNORE 1 LINES;
    CREATE TABLE `Oplus`.`actes` (
      `nopat` INT NOT NULL,
      `dateheure` VARCHAR(15) NULL,
      `rubrique` VARCHAR(15) NULL,
      `valeur` VARCHAR(1000) NULL);
    LOAD DATA INFILE '/Users/serge/Documents/Rufus/laine/actes.csv'
    INTO TABLE `Oplus`.`actes`
    FIELDS
        TERMINATED BY '|'
    LINES
        TERMINATED BY '\n'
        IGNORE 1 LINES;

        * ce srcipt plantera à de nombreuses reprises sur les irrégularités des champs des fichiers CSV
        * corriger l'erreur à chaque plantage et recommencer jusaqu'à ce que le script finisse par s'éxécuter sans échec
     * 3. faire une install vierge de Rufus en conservant les données d'utilisateur proposées par défaut
     * 4. redémarrer Rufus et éxécuter la fonction consersionbaseoplus qui va importer les données de la base MySQL Oplus créée par le script précédent pour les incorporer dans la base Rufus nouvellement créée
        * C'est très lent - environ 2 à 3 heures pour 350000 dossiers
     * 5. redémarrer Rufus et corriger les utilisateurs importés, les lieux d'exercice, l'emplacement du serveur...etc...
     *
     * Remarque : les documents émis par O+ au format Abiword (extension .zabw) ne peuvent pas être importés
     */

    QString NomBase = "";
    db = DataBase::I();

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
    dirimagerieoplus    = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                            tr("Choisissez le dossier dans lequel se trouvent les images O+\n"
                                                               "Le nom de dossier ne doit pas contenir d'espace"),
                                                            QDir::homePath() + NOM_DIR_RUFUS,
                                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    bool okdirimagerie = QDir(dirimagerieoplus).exists();
    dirimagerie = QDir::homePath() + NOM_DIR_RUFUS NOM_DIR_IMAGERIE NOM_DIR_IMAGES;

    db->StandardSQL("delete from " TBL_PATIENTS);
    db->StandardSQL("delete from " TBL_DONNEESSOCIALESPATIENTS);
    db->StandardSQL("delete from " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS);
    db->StandardSQL("delete from " TBL_ACTES);
    db->StandardSQL("delete from " TBL_TYPEPAIEMENTACTES);
    db->StandardSQL("delete from " TBL_CORRESPONDANTS);
    db->StandardSQL("delete from " TBL_REFRACTIONS);
    db->StandardSQL("delete from " TBL_DOCSEXTERNES);
    db->StandardSQL("delete from " TBL_UTILISATEURS " where " CP_NOM_USR " <> '" NOM_ADMINISTRATEUR "' and " CP_NOM_USR " <> 'Snow'" );

    /*! UTILISATEURS =========================================================================================================================================================================
     * Seul le nom de l'utilisateur est enregistré dans 0+, dans la table identites_patients, avec les risques de doublons
     * On crée donc un enregistrement pour chacun de ces utilisateurss dans la table Correspondants de Rufus
     * le nom et le login sont fixés arbitrairement
     * le mdp à "bob"
     * il n'y a aucun renseignement sur le statut
     * il faut donc compléter "à la main" par la suite tous les autres renseignements de statut
     */
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

    /*! MEDECINS TRAITANTS =========================================================================================================================================================================
     * Seul le nom du médecin est enregistré dans 0+, dans la table identites_patients, avec les risques de doublons
     * On crée donc un enregistrement pour chacun de ces médecins dans la table Correspondants de Rufus
     * le prénom est fixé arbitrairement à x
     * le sexe à M
     * pas d'adresse
     * tous généralistes
     * les risques de doublons persistent donc
     */
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

    /*! PATIENTS =========================================================================================================================================================================
     *
     */
    UpSystemTrayIcon::I()->showMessage(tr("Messages"), "importation des patients", Icons::icSunglasses(), 1000);
    req = "select nopat, sexe, nom_prenom, date_naiss,"
          " adr1, adr2, codepostal, ville, noSS, tel, tel_port, email, profession,"
          " prat_usuel, med_trait"
          " from Oplus.identites_patients";
    QList<QVariantList> patlist = db->StandardSelectSQL(req,ok);
    QMap<int, QString> mappat;

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
        nom = Utils::trimcapitilize(patlist.at(i).at(2).toString());
        mappat.insert(idpat, nom);
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

        adr1        = Utils::trimcapitilize(patlist.at(i).at(4).toString());
        adr2        = Utils::trimcapitilize(patlist.at(i).at(5).toString());
        codepostal  = patlist.at(i).at(6).toString();
        ville       = Utils::trimcapitilize(patlist.at(i).at(7).toString());
        noSS        = patlist.at(i).at(8).toString();
        tel         = patlist.at(i).at(9).toString();
        tel_port    = patlist.at(i).at(10).toString();
        email       = patlist.at(i).at(11).toString();
        profession  = Utils::trimcapitilize(patlist.at(i).at(12).toString());
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

    /*! ACTES ET IMAGERIE ============================================================================================================================================
     *
     */
    int l = 0;
    for (auto it = mappat.constBegin(); it != mappat.constEnd(); ++it)
    {
        ++l;
        int idpat =  it.key();
        qDebug() << l << "/" <<  mappat.size() << "idpat =" << idpat << "- patient = " << it.value();
        QString strgidpat = QString::number(idpat);

        req = "SELECT rubrique, valeur, dateheure FROM Oplus.actes where nopat = " + QString::number(idpat);
        //qDebug() << req;
        QList<QVariantList> actlist = db->StandardSelectSQL(req,ok);
        QList<QDateTime> listdates;
        for (int i=0; i< actlist.size(); i++)
        {
            QDateTime datetime = QDateTime::fromString(actlist.at(i).at(2).toString(),"dd/MM/yyyy HHmm");
            QString rubrique = actlist.at(i).at(0).toString();
            QString valeur = actlist.at(i).at(1).toString();
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
                if (rubrique == "atcd")
                {
                    QString atcdt = valeur.replace("^","\n");
                    req = "update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set " CP_ATCDTSPERSOS_RMP " =  '" + Utils::correctquoteSQL(atcdt) + "' where " CP_IDPAT_RMP " = " + strgidpat;
                    //qDebug() << req;
                    db->StandardSQL(req);
                }
                else if (rubrique == "dgn")
                {
                    QString atcdt = valeur.replace("^","\n");
                    req = "update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set " CP_ATCDTSOPH_RMP " =  '" + Utils::correctquoteSQL(atcdt) + "' where " CP_IDPAT_RMP " = " + strgidpat;
                    //qDebug() << req;
                    db->StandardSQL(req);
                }
            }
            else if (!listdates.contains(datetime))
                listdates << datetime;
        }
        UpTextEdit *uptxtedit = new UpTextEdit;
        for (int l=0; l< listdates.size(); l++)
        {
            QDateTime datetime = listdates.at(l);
            QString actedate = datetime.date().toString("yyyy-MM-dd");
            QString acteheure = datetime.time().toString("HH:mm");
            QString txt = "";
            QString motif = "", conclusion = "";
            for (int j=0; j< actlist.size(); j++)
            {
                QDateTime datetimeenreg = QDateTime::fromString(actlist.at(j).at(2).toString(),"dd/MM/yyyy HHmm");
                QString rubrique = actlist.at(j).at(0).toString();
                QString valeur = actlist.at(j).at(1).toString();
                if (datetimeenreg == datetime)
                {
                    QString larg =  "60";
                    QString retourligneavectab = "</td>" HTML_RETOURLIGNE "<td width=\"" + larg + "\"></td><td>";
                    QString retourlignesanstab = "</td>" HTML_RETOURLIGNE "<td>";
                    if (rubrique == "m")
                        motif += valeur.replace("^","\n");
                    else if (rubrique == "cc") //! une ligne cc corrrespond à la conclusion et est enregistrée en conclusion
                    {
                        valeur = valeur.replace("^",retourlignesanstab);
                        conclusion += HTML_RETOURLIGNE "<td>" + valeur + "</td></p>";
                    }
                    else if (rubrique == "ttn") //! une ligne ttn corrrespond à une prescription et est enregistrée en conclusion
                    {
                        rubrique = tr("Ordo:");
                        valeur = valeur.replace("^",retourligneavectab);
                        QString title = HTML_RETOURLIGNE "<td width=\"" + larg + "\"><b><font color = \"" COULEUR_TITRES "\">" + rubrique + "</font></b></td>";
                        conclusion += title + "<td>" + valeur + "</td></p>";
                    }
                    else if (rubrique == "vp") //! une ligne vp corrrespond à une prescription de verres et est enregistrée en conclusion
                    {
                        rubrique = tr("VP:");
                        valeur = valeur.replace("^",retourligneavectab);
                        QString title = HTML_RETOURLIGNE "<td width=\"" + larg + "\"><b><font color = \"" COULEUR_TITRES "\">" + rubrique + "</font></b></td>";
                        conclusion += title + "<td>" + valeur + "</td></p>";
                    }
                    /*! l'adresse des fichiers image est enregistrée dans la table actes
                     * dans les lignes des rubriques scan, icv et reti
                     */
                    else if ((rubrique == "icv" || rubrique == "scan" || rubrique == "reti")
                             && (valeur.contains(".tif") || valeur.contains(".jpg") || valeur.contains(".jpeg"))
                             && okdirimagerie)
                    {
                        QString nomfichier = valeur;
                        QString titre = (rubrique == "icv"? tr("CV"): (rubrique == "reti"? tr("RNM") : "scan"));
                        QString typedoc = titre;
                        QString sstypedoc = titre;
                        if (valeur.split(":").size()>1)
                        {
                            nomfichier = valeur.split(":").at(0);
                            sstypedoc = valeur.split(":").at(1);
                        }
                        QString nomdossierimage = strgidpat;
                        if (nomdossierimage.size() == 1)
                            nomdossierimage = "00000" +nomdossierimage;
                        else if (nomdossierimage.size() == 2)
                            nomdossierimage = "0000" +nomdossierimage;
                        else if (nomdossierimage.size() == 3)
                            nomdossierimage = "000" +nomdossierimage;
                        else if (nomdossierimage.size() == 4)
                            nomdossierimage = "00" +nomdossierimage;
                        else if (nomdossierimage.size() == 5)
                            nomdossierimage = "0" +nomdossierimage;
                        QString pathorigin = dirimagerieoplus + "/" + nomdossierimage +"/" + nomfichier;
                        QFile fileorigin(pathorigin);
                        if (!fileorigin.exists())
                            continue;
                        QString nomdossierdestination = dirimagerie + "/" + datetime.date().toString("yyyy-MM-dd");
                        Utils::mkpath(nomdossierdestination);
                        int idimpr = db->selectMaxFromTable(CP_ID_DOCSEXTERNES,  TBL_DOCSEXTERNES, ok)+1;
                        QString nomfiledestination = strgidpat + "_"
                                + typedoc + "_"
                                + sstypedoc + "_"
                                + datetime.toString("yyyyMMdd-HHmm")
                                + "-" + QString::number(idimpr);
                        QString suffix = QFileInfo(fileorigin).suffix();
                        if (suffix == "tif") //! CONVERTIT LES TIF EN JPG
                        {
                            if (!fileorigin.open(QIODevice::ReadOnly)) {
                                return;
                            }
                            QByteArray ar(fileorigin.size(), ' ');
                            fileorigin.read(ar.data(), ar.size());
                            QImage img;
                            img.loadFromData(ar);
                            if (!img.isNull())
                            {
                                nomfiledestination +=  ".jpg";
                                img.save(nomdossierdestination + "/" + nomfiledestination);
                            }
                        }
                        else
                        {
                            if (QFileInfo(fileorigin).suffix() == "jpeg")
                                suffix = "jpg";
                            nomfiledestination +=  "." + suffix;
                            fileorigin.copy(nomdossierdestination + "/" + nomfiledestination);
                        }
                        req = "insert into " TBL_DOCSEXTERNES " (" CP_ID_DOCSEXTERNES ", " CP_IDUSER_DOCSEXTERNES ",  " CP_IDPAT_DOCSEXTERNES ",  " CP_TYPEDOC_DOCSEXTERNES ",  " CP_SOUSTYPEDOC_DOCSEXTERNES ", " CP_TITRE_DOCSEXTERNES ", " CP_DATE_DOCSEXTERNES ","
                                                                   CP_IDEMETTEUR_DOCSEXTERNES ", " CP_LIENFICHIER_DOCSEXTERNES ", " CP_EMISORRECU_DOCSEXTERNES ", " CP_FORMATDOC_DOCSEXTERNES ", " CP_IDLIEU_DOCSEXTERNES ")"
                                                                   " values("
                                + QString::number(idimpr) + ", "
                                + "1, "
                                + strgidpat + ", '"
                                + typedoc + "', '"
                                + titre
                                + "', '"
                                + sstypedoc + "', '"
                                + datetime.date().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss") + "', "
                                +  "1, '"
                                + "/" + datetime.date().toString("yyyy-MM-dd") + "/" + nomfiledestination + "', "
                                + "0" + ", '"
                                IMAGERIE "', "
                                + QString::number(Datas::I()->sites->idcurrentsite()) + ")";
                        //qDebug() << req;
                        if(db->StandardSQL(req))
                        {
                            QFile CC(nomdossierdestination + "/" + nomfiledestination);
                            CC.open(QIODevice::ReadWrite);
                            CC.setPermissions(QFileDevice::ReadOther
                                              | QFileDevice::ReadGroup
                                              | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                              | QFileDevice::ReadUser   | QFileDevice::WriteUser);
                        }
                    }
                    else
                    {
                        if (rubrique == "lf")
                            rubrique = tr("LAF:");
                        else if (rubrique == "to")
                            rubrique = tr("TO:");
                        else if (rubrique == "av")
                            rubrique = tr("AV:");
                        else if (rubrique == "ra")
                            rubrique = tr("Autoref:");
                        else if (rubrique == "po")
                            rubrique = tr("Porte:");
                        else if (rubrique.left(3) == "ker")
                            rubrique = tr("Kerato:");
                        else if (rubrique == "fo")
                            rubrique = tr("FO:");
                        else if (rubrique == "oct")
                            rubrique = tr("OCT:");
                        else if (rubrique == "pachy")
                            rubrique = tr("Pachy:");
                        else
                            rubrique = "[" + rubrique + "]";

                        valeur = valeur.replace("^",retourligneavectab);
                        QString title = HTML_RETOURLIGNE "<td width=\"" + larg + "\"><b><font color = \"" COULEUR_TITRES "\">" + rubrique + "</font></b></td>";
                        txt += title + "<td>" + valeur + "</td></p>";
                    }
                }
            }
            uptxtedit->setText(conclusion);
            conclusion = uptxtedit->toHtml();
            Utils::nettoieHTML(conclusion);

            uptxtedit->setText(txt);
            txt = uptxtedit->toHtml();
            Utils::nettoieHTML(txt);

            uptxtedit->setText(motif);
            motif = uptxtedit->toHtml();
            Utils::nettoieHTML(motif);

            req= "insert into " TBL_ACTES " (" CP_IDPAT_ACTES ", " CP_IDUSER_ACTES ", " CP_TEXTE_ACTES ", " CP_DATE_ACTES ", " CP_HEURE_ACTES ", " CP_IDUSERCREATEUR_ACTES  ", "
                    CP_COTATION_ACTES  ", " CP_MONTANT_ACTES", " CP_MONNAIE_ACTES ", " CP_IDUSERPARENT_ACTES ", " CP_IDLIEU_ACTES ", " CP_MOTIF_ACTES ", " CP_CONCLUSION_ACTES ")"
                    " values (" + strgidpat + ", 1, '" + Utils::correctquoteSQL(txt) + "', '" + actedate + "', '" + acteheure + "', 1, "
                    " 'Acte gratuit', 0.00 , 'E', 1, 1, '" + Utils::correctquoteSQL(motif) + "', '" + Utils::correctquoteSQL(conclusion) + "')";
            //qDebug() << req;
            db->StandardSQL(req);
        }
        delete uptxtedit;
    }

    //! tous les actes importés sont enregistrés comme des actes gratuits
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
    //qDebug() << req;
    db->StandardSQL(req);
}
