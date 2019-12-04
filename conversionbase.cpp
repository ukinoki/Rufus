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


#include "conversionbase.h"

conversionbase::conversionbase(Procedures *proc, QString BaseAConvertir, QObject *parent) :
    QObject(parent)
{
    QString NomBase = BaseAConvertir;
    db = DataBase::I();
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
                  "' and schema_name <> '" DB_OPHTA "'";
    QList<QVariantList> lgclist = db->StandardSelectSQL(req,ok);
    if (lgclist.size()==0)
    {
        UpMessageBox::Watch(Q_NULLPTR,"pas de base ophtalogic retrouvée");
        return;
    }
    else
    {
        NomBase = "";
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
