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

#include "importdocsexternesthread.h"

ImportDocsExternesThread::ImportDocsExternesThread(Procedures *proced)
{
    moveToThread(&thread);
    a=0;
    proc            = proced;
    EnCours         = false;
    db              = DataBase::I();
    gAcces           = (db->getMode()!=DataBase::Distant? Local : Distant);
    idLieuExercice  = Datas::I()->users->userconnected()->sitedetravail()->id();
    thread          .start();
}

void ImportDocsExternesThread::RapatrieDocumentsThread(QList<QVariantList> listdocs)
{
    /* req = "select distinct list.TitreExamen, list.NomAPPareil from " TBL_APPAREILSCONNECTESCENTRE " appcon, " TBL_LISTEAPPAREILS " list"
          " where list.idappareil = appcon.idappareil and idLieu = " + QString::number(idlieuExercice);

    -> listdocs.at(i).at(0) = le titre de l'examen
    -> listdocs.at(i).at(1) = le nom de l'appareil*/
    if (EnCours)
        return;
    EnCours = true;
    listmsg.clear();
    datetransfer = QDate::currentDate().toString("yyyy-MM-dd");
    if (!DefinitDossiers())
    {
        EnCours = false;
        return;
    }
    for (int itr=0; itr<listdocs.size(); itr++)
    {
        QString NomDirDoc = proc->getDossierDocuments(listdocs.at(itr).at(1).toString(), db->getMode());  // le dossier où sont exportés les documents d'un appareil donné
        if (NomDirDoc == "")
            NomDirDoc = "Triumph Speed Triple 1050 2011";
        if (QDir(NomDirDoc).exists())
        {
            /* Retrouver
                 * Titre du document
                 * Date du document
                 * contenu du fichier
                 * idpatient
                */
            // Titre du document------------------------------------------------------------------------------------------------------------------------------------------------
            QString Titredoc    = listdocs.at(itr).at(0).toString();
            QString Typedoc     = Titredoc;
            QString SousTypeDoc = Titredoc;
            QString Appareil    = listdocs.at(itr).at(1).toString();
            QStringList listfich = QDir(NomDirDoc).entryList(QDir::Files | QDir::NoDotAndDotDot);
            for (int k=0; k<listfich.size(); k++)
            {
                QString nomdoc  = listfich.at(k);
                if (nomdoc.contains("smbtest"))
                    continue;
                QString CheminFichierImage      = NomDirDoc + "/" + nomdoc;
                QFile   jnaltrsferfile(CheminOKTransfrDir + "/0JournalTransferts - " + datetransfer + ".txt");
                QString commentechec;

                FichierOrigine.setFileName(CheminFichierImage);
                QString datetimecreation = QFileInfo(FichierOrigine).created().toString("yyyyMMdd-HHmmss");

                // Date et type du document------------------------------------------------------------------------------------------------------------------------------------------------
                QString datestring  = "";
                if (Appareil == "TOPCON ALADDIN")
                {
                    QDateTime datefic   = QFileInfo(FichierOrigine).created();
                    datestring          = datefic.toString("yyyyMMdd");
                    Titredoc            = "Biométrie - Aladdin";
                    Typedoc             = "Biométrie";
                    SousTypeDoc         = "Aladdin";
                }
                else if (Appareil == "TOPCON ALADDIN II")
                {
                    //1051_MIGUEL_JEAN-ROBERT_01-06-1948_Aladdin_06_06_2018_13_16.pdf
                    QStringList listn   = nomdoc.split("_");
                    int n               = listn.size();
                    QString jour        = listn.at(n-5);
                    QString mois        = listn.at(n-4);
                    QString annee       = listn.at(n-3);
                    datestring          = annee + mois + jour;
                    Titredoc            = "Biométrie - Aladdin";
                    Typedoc             = "Biométrie";
                    SousTypeDoc         = "Aladdin";
                }
                else if (Appareil == "TOPCON TRITON")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring  = nomdoc.split("_").at(3);
                        datestring  = datestring.left(8);
                    }
                    Titredoc    = "OCT - Topcon";
                    Typedoc     = "OCT";
                    SousTypeDoc = "Topcon";
                }
                else if (Appareil == "CANON CR-2")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring  = nomdoc.split("_").at(1);
                        datestring  = datestring.left(8);
                    }
                    Titredoc    = "RNM - Canon";
                    Typedoc     = "RNM";
                    QString cote = ((nomdoc.split("_").at(2) == "R")? tr("OD") : tr("OG"));
                    SousTypeDoc = "Canon " + cote;
                }
                else if (Appareil == "OTI SLO") {
                    datestring  = nomdoc.mid(nomdoc.indexOf("-")+1,8);
                    Titredoc    = "OCT - OTI";
                    Typedoc     = "OCT";
                    SousTypeDoc = "OTI";
                }
                else if (Appareil == "OPTOVUE")
                {
                    QStringList list = nomdoc.split("_");
                    if (list.size()>7)
                    {
                        datestring  = list.at(7);
                        datestring  = datestring.replace("-","");
                    }
                    Titredoc    = "OCT - Optovue";
                    Typedoc     = "OCT";
                    SousTypeDoc = "Optovue";
                }
                else if (Appareil == "NIDEK-OCT")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring = nomdoc.split("_").at(1);
                        datestring = datestring.left(8);
                    }
                    Titredoc    = "OCT - Nidek";
                    Typedoc     = "OCT";
                    SousTypeDoc = "Nidek";
                }
                else if (Appareil == "CANON-OCT")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring = nomdoc.split("_").at(1);
                        datestring = datestring.left(8);
                    }
                    Titredoc    = "OCT - Canon";
                    Typedoc     = "OCT";
                    QString cote = (nomdoc.contains("BothEyes")? tr("ODG") : ((nomdoc.split("_").at(4) == "R")? tr("OD") : tr("OG")));
                    QString typeexam = nomdoc.split("_").at(3);
                    if (typeexam == "OCTA")     typeexam = "AngioOCT";
                    if (typeexam == "Disc3D")   typeexam = "Glaucome";
                    SousTypeDoc = "Canon " + typeexam + " " + cote;
                    if (typeexam == "Disc3D")   typeexam = "Glaucome";
                    datetimecreation = datestring + "-" + nomdoc.split("_").at(2);
                }
                else if (Appareil == "NIDEK-RNM")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring = nomdoc.split("_").at(1);
                        datestring = datestring.left(8);
                    }
                    Titredoc    = "RNM - Nidek";
                    Typedoc     = "RNM";
                    SousTypeDoc = "Nidek";
                }
                else if (Appareil == "SPECTRALIS")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring = nomdoc.split("_").at(1);
                        datestring = datestring.left(8);
                    }
                    Titredoc    = "OCT - Heidelberg";
                    Typedoc     = "OCT";
                    SousTypeDoc = "Heidelberg";
                }
                else if (Appareil == "NAVIS-EX")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring = nomdoc.split("_").at(1);
                        datestring = datestring.left(8);
                    }
                    QString AbregeTitre = nomdoc.split("_").at(3);
                    if (AbregeTitre == "OT") {
                        Titredoc    = "OCT - Nidek";
                        Typedoc     = "OCT";
                        SousTypeDoc = "Nidek";
                    }
                    else if (AbregeTitre == "AO") {
                        Titredoc    = "OCT SA - Nidek";
                        Typedoc     = "OCT SA";
                        SousTypeDoc = "Nidek";
                    }
                    else if (AbregeTitre == "CO") {
                        Titredoc    = "RNM - Nidek";
                        Typedoc     = "RNM";
                        SousTypeDoc = "Nidek";
                    }
                }
                else if (Appareil == "EIDON")
                {
                    // il faut découper le nom de fichier en retrouvant l'emplacement du module dateTheure ( 2017-06-14T17_39_21Z- )dans le nom
                    // parce que si on découpe à partir du début en se basant sur les tirets le décompte plant si on met des tirets dans le nom ou le prénom p.e.,
                    // la fonction plante dans ses découpages de QStringList et le programme avec
                    /*
                     * avant 2019
                     * 368_Zammit - Sauveur - 2017 - 06 - 14T17_39_21Z - eidon_20129 - right - 0 - visible - 2017 - 06 - 14T17_39_26Z -    - report.pdf
                     * 368_Zammit - Sauveur - 2017 - 06 - 16T12_27_13Z - eidon_20129 - right - 0 - af      - 2017 - 06 - 16T12_27_18Z -    - image.jpg
                     * 0          - 1       - 2    - 3  - 4            - 5           - 6     - 7 - 8       - 9    - 10 - 11           - 12 - 13

                     * version 2019
                     * 9303  - Caro_Fernando  - - 2019 - 01 - 31T12_13_04Z - eidon_20005 - right - 0 - visible - 2019 - 01 - 31T12_13_03Z - 2019 - 01 - 31_121345 - image.jpg
                     * 15179 - Viottolo_Louis - - 2019 - 01 - 31T13_31_04Z - eidon_20005 - right - 0 - visible - 2019 - 01 - 31T13_31_03Z - 2019 - 01 - 31_133138 - image.jpg
                     * 0     - 1              -2- 3    - 4  - 5            - 6           - 7     - 8 - 9       - 10   - 11 - 12           - 13   - 14 - 15        - 16
                    */

                    QRegExp re("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}_[0-9]{2}_[0-9]{2}Z-"); // correspond aux sections 2,3,et 4 avant 2019 eet 3,4 et 5 aprçs
                    if (nomdoc.contains(re))
                    {
                        int idxdate;
                        Titredoc    = "RNM - Eidon ";
                        Typedoc     = "RNM";
                        idxdate = nomdoc.indexOf(re);
                        datestring = nomdoc.mid(idxdate,10).replace("-","");
                        datetimecreation = datestring + "-" + nomdoc.mid(idxdate + 11,8).replace("_","");
                        QString details (""), cote("");
                        if (nomdoc.split(re).size()>1)
                            details = nomdoc.split(re).at(1);
                        else
                        {
                            commentechec =  tr("nom invalide");
                            EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                            continue;
                        }
                        if (details.split("-").size()>0)
                            cote = ((details.split("-").at(1)=="right")? tr("OD") : tr("OG"));
                        SousTypeDoc = "Eidon " + cote;
                        if (details.split("-").size()>3)
                        {
                            QString typecliche = details.split("-").at(3);
                            if (typecliche == "infrared")
                                SousTypeDoc += " IR";
                            else if (typecliche == "af")
                                SousTypeDoc += " Autofluo";
                        }
                    }
                    else
                    {
                        commentechec =  tr("nom invalide");
                        EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                        continue;
                    }
                }
                else if (Appareil == "OPTOS Daytona series")
                {
                    if (nomdoc.split("-").size()>1)
                    {
                        datestring = nomdoc.split("-").at(1);
                        datestring = datestring.left(8);
                    }
                    Titredoc    = "RNM - Optos";
                    Typedoc     = "RNM";
                    SousTypeDoc = "Optos";
                }
                else if (Appareil == "ZEISS CIRRUS 5000")
                {
                    if (nomdoc.split("_").size()>1)
                    {
                        datestring = nomdoc.split("_").at(7);
                        datestring = datestring.left(8);
                    }
                    Titredoc    = "OCT - Zeiss";
                    Typedoc     = "OCT";
                    SousTypeDoc = "Zeiss";
                }
                if (!QDate().fromString(datestring,"yyyyMMdd").isValid())
                {
                    commentechec =  tr("date invalide") + " -> " + datestring;
                    EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    continue;
                }

                datestring = QDate().fromString(datestring,"yyyyMMdd").toString("yyyy-MM-dd");
                // Format du document------------------------------------------------------------------------------------------------------------------------------------------------
                QString formatdoc = QFileInfo(listfich.at(k)).suffix().toLower();
                if (formatdoc != "pdf" && formatdoc != "jpg" && formatdoc != "jpeg")
                {
                    commentechec = tr("format invalide") + " -> " + formatdoc;
                    EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    continue;
                }
                // Contenu du document------------------------------------------------------------------------------------------------------------------------------------------------
                QByteArray ba;
                QString nomfichresize = NomDirStockageProv + "/resize" + nomdoc;
                QString szorigin, szfinal;
                // on vide le dossier provisoire
                QStringList listfichresize = QDir(NomDirStockageProv).entryList(QDir::Files | QDir::NoDotAndDotDot);
                for (int t=0; t<listfichresize.size(); t++)
                {
                    QString nomdocrz  = listfichresize.at(t);
                    QString CheminFichierResize = NomDirStockageProv + "/" + nomdocrz;
                    QFile(CheminFichierResize).remove();
                }
                if (FichierOrigine.open(QIODevice::ReadOnly))
                {
                    double sz = FichierOrigine.size();
                    if (sz/(1024*1024) > 1)
                        szorigin = QString::number(sz/(1024*1024),'f',1) + "Mo";
                    else
                        szorigin = QString::number(sz/1024,'f',1) + "Ko";
                    szfinal = szorigin;
                    FichierOrigine.copy(nomfichresize);
                    FichierImage.setFileName(nomfichresize);
                    if (formatdoc == "jpg" && sz > TAILLEMAXIIMAGES)
                    {
                        QImage  img(nomfichresize);
                        FichierImage.remove();
                        QPixmap pixmap;
                        pixmap = pixmap.fromImage(img.scaledToWidth(2560,Qt::SmoothTransformation));
                        int     tauxcompress = 90;
                        while (sz > TAILLEMAXIIMAGES && tauxcompress > 1)
                        {
                            pixmap.save(nomfichresize, "jpeg",tauxcompress);
                            sz = FichierImage.size();
                            tauxcompress -= 10;
                        }
                        if (sz/(1024*1024) > 1)
                            szfinal = QString::number(sz/(1024*1024),'f',0) + "Mo";
                        else
                            szfinal = QString::number(sz/1024,'f',0) + "Ko";
                    }
                    FichierImage.open(QIODevice::ReadOnly);
                    ba = FichierImage.readAll();
                }
                else
                {
                    commentechec =  tr("Impossible d'ouvrir le fichier");
                    EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    continue;
                }
                //qDebug()<< "ba.size() = " + QString::number(ba.size());

                // IdPatient------------------------------------------------------------------------------------------------------------------------------------------------
                QString req(""), idPatient("");
                if (Appareil == "TOPCON ALADDIN")   {
                    QStringList listn = nomdoc.split("_");
                    if (listn.size()<5)
                    {
                        commentechec =  tr("Impossible d'ouvrir le fichier");
                        EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                        continue;
                    }
                    QString nom     = Utils::capitilize(listn.at(0));
                    QString prenom  = Utils::capitilize(listn.at(1));
                    QString jour    = Utils::capitilize(listn.at(2));
                    QString mois    = Utils::capitilize(listn.at(3));
                    QString annee   = Utils::capitilize(listn.at(4));
                    req             = "select idpat from " TBL_PATIENTS
                                      " where patnom like '" + nom + "'"
                                      " and patprenom like '" + prenom  + "'"
                                      " and patDDN = '" + annee + "-" + mois + "-" + jour + "'";
                    //qDebug() << req;
                    QVariantList patlst = db->getFirstRecordFromStandardSelectSQL(req, ok);
                    if (!ok || patlst.size()==0)
                    {
                        commentechec =  tr("Impossible d'ouvrir le fichier");
                        EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                        continue;
                    }
                    idPatient = patlst.at(0).toString();
                    //qDebug() << idPatient;
                }
                else if (Appareil == "TOPCON ALADDIN II")
                    idPatient           = nomdoc.split("_").at(0);
                else if (Appareil == "TOPCON TRITON")
                    idPatient           = nomdoc.split("_").at(2);
                else if (Appareil == "CANON CR-2")
                    idPatient           = nomdoc.split("_").at(0);
                else if (Appareil == "OTI SLO")
                    idPatient           = nomdoc.split("-").at(0);
                else if (Appareil == "OPTOVUE")
                {
                    if (nomdoc.split("__").size()>0)
                    {
                        idPatient       = nomdoc.split("__").at(1);
                        if (idPatient.split("_").size()>0)
                            idPatient   = idPatient.split("_").at(0);
                    }
                }
                else if (Appareil == "NIDEK-OCT")   {
                    idPatient           = nomdoc.split("_").at(0);
                }
                else if (Appareil == "CANON-OCT")   {
                    idPatient           = nomdoc.split("_").at(0);
                }
                else if (Appareil == "NIDEK-RNM")   {
                    idPatient           = nomdoc.split("_").at(0);
                }
                else if (Appareil == "SPECTRALIS")   {
                    idPatient           = nomdoc.split("_").at(0);
                }
                else if (Appareil == "NAVIS-EX")   {
                    idPatient           = nomdoc.split("_").at(0);
                }
                else if (Appareil == "EIDON")   {
                    if (nomdoc.split("-").at(0).toInt()>0)
                        idPatient           = nomdoc.split("-").at(0);
                    else
                        idPatient           = nomdoc.split("-").at(2);
                }
                else if (Appareil == "OPTOS Daytona series")   {
                    idPatient           = nomdoc.split("-").at(0);
                }
                else if (Appareil == "ZEISS CIRRUS 5000")
                {
                    idPatient           = nomdoc.split("_").at(3);
                }
                bool b=true;
                if (idPatient.toInt(&b)<1)
                {
                    commentechec = tr("idPatient invalide") + " -> " + idPatient;
                    EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    continue;
                }
                if (!b)
                {
                    commentechec = tr("idPatient invalide") + " -> " + idPatient;
                    EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    continue;
                }
                QString identpat;
                QVariantList patlst = db->getFirstRecordFromStandardSelectSQL("select patnom, patprenom from " TBL_PATIENTS " where idpat = " + idPatient, ok);
                if (!ok || patlst.size()==0)
                {
                    commentechec =  tr("Pas de patient pour cet idPatient") + " -> " + idPatient;
                    EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    continue;
                }
                identpat = patlst.at(0).toString() + " " + patlst.at(1).toString();
                //qDebug() << identpat;

                /* _______________________________________________________________________________________________________________________________________________________
                 * Enregistrement du fichier dans la base
                 * Si on est sur le réseau local, l'enregistrement se fait dans la table Impressions et le fichier est copié à son adresse définitive
                 * Si on est en accès distant, l'enregistrement se fait dans la table Impressions et le contenu du fichier est copié dans le champ blob de la table de la table
                 * _______________________________________________________________________________________________________________________________________________________
                */
                int idimpr = db->selectMaxFromTable("idimpression",  TBL_IMPRESSIONS, ok)+1;

                QString NomFileDoc = idPatient + "_"
                        + Typedoc + "_"
                        + SousTypeDoc + "_"
                        + datetimecreation
                        + "-" + QString::number(idimpr)
                        + "." + QFileInfo(nomdoc).suffix();

                if (gAcces == Local)
                {
                    req = "insert into " TBL_IMPRESSIONS " (idimpression, idUser,  idpat,  TypeDoc,  SousTypeDoc, Titre, Dateimpression,"
                                                               " UserEmetteur, lienversfichier, EmisRecu, FormatDoc, idLieu)"
                                                               " values("
                            + QString::number(idimpr) + ", "
                            + QString::number(Datas::I()->users->userconnected()->id()) + ", "
                            + idPatient + ", '"
                            + Typedoc + "', '"
                            + SousTypeDoc + "', '"
                            + Titredoc + "', '"
                            + datestring + " " + QTime::currentTime().toString("HH:mm:ss") + "', "
                            + QString::number(Datas::I()->users->userconnected()->id()) + ", '"
                            + "/" + datetransfer + "/" + NomFileDoc + "', "
                            + "0" + ", '"
                            IMAGERIE "', "
                            + QString::number(idLieuExercice) + ")";

                    if(db->StandardSQL(req))
                    {
                        QString CheminOKTransfrDoc          = CheminOKTransfrDir + "/" + NomFileDoc;
                        QString CheminOKTransfrDocOrigin    = CheminOKTransfrDirOrigin + "/" + nomdoc;
                        FichierImage.copy(CheminOKTransfrDoc);
                        QFile CC(CheminOKTransfrDoc);
                        CC.open(QIODevice::ReadWrite);
                        CC.setPermissions(QFileDevice::ReadOther
                                          | QFileDevice::ReadGroup
                                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
                        FichierImage.remove();
                        FichierOrigine.copy(CheminOKTransfrDocOrigin);
                        QFile CO(CheminOKTransfrDocOrigin);
                        CO.open(QIODevice::ReadWrite);
                        CO.setPermissions(QFileDevice::ReadOther
                                          | QFileDevice::ReadGroup
                                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
                        if (jnaltrsferfile.open(QIODevice::Append))
                        {
                            QTextStream out(&jnaltrsferfile);
                            out << Titredoc << " - " << nomdoc << " - " << idPatient << " - " << identpat << " - " << QHostInfo::localHostName() << "\n" ;
                            jnaltrsferfile.close();
                        }
                        if (FichierOrigine.remove())
                        {
                            QString msg = tr("Enregistrement d'un cliché") + " <font color=\"red\"><b>" + Titredoc + "</b></font>"
                                          " " + tr("pour") + " <font color=\"green\"><b>" + identpat + "</b></font> " + tr("dans la base de données");
                            if (formatdoc == "jpg" && szorigin != szfinal)
                                msg += "<br />" + tr("le cliché a été compressé de ") + szorigin + tr(" à ") + szfinal;
                            else
                                msg += "<br />" + tr("la taille du fichier est de ") + szorigin;
                            //qDebug() << "ba size = "  + QString::number(ba.size()) << "ba compressé size = " + QString::number(qCompress(ba).size());
                            listmsg << msg;
                            //qDebug() << "xx = " + QString::number(xx) << "x = " + QString::number(xx-DlgMess->width()-50) << "yy = " + QString::number(yy)  << "y = " + QString::number(yy-DlgMess->height()*(k+1))  << "itr = " << QString::number(k);
                        }
                        else
                            db->SupprRecordFromTable(idimpr, "idimpression", TBL_IMPRESSIONS);
                    }
                    else
                    {
                        commentechec = tr("impossible d'enregistrer ") + nomdoc;
                        EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    }
                }
                else if (gAcces == Distant)
                {
                    if (formatdoc == "pdf")
                        formatdoc = CP_PDF_IMPRESSIONS;
                    else if (formatdoc== "jpg" || formatdoc == "jpeg")
                        formatdoc = CP_JPG_IMPRESSIONS;
                    // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
                    QHash<QString, QVariant> listbinds;
                    listbinds[CP_IDIMPRESSION_IMPRESSIONS] =    idimpr;
                    listbinds[CP_IDUSER_IMPRESSIONS] =          Datas::I()->users->userconnected()->id();
                    listbinds[CP_IDPAT_IMPRESSIONS] =           idPatient;
                    listbinds[CP_TYPEDOC_IMPRESSIONS] =         Typedoc;
                    listbinds[CP_SOUSTYPEDOC_IMPRESSIONS] =     SousTypeDoc;
                    listbinds[CP_TITRE_IMPRESSIONS] =           Titredoc;
                    listbinds[CP_DATE_IMPRESSIONS] =            datestring + " " + QTime::currentTime().toString("HH:mm:ss");
                    listbinds[CP_IDEMETTEUR_IMPRESSIONS] =      Datas::I()->users->userconnected()->id();
                    listbinds[formatdoc] =                      ba;
                    listbinds[CP_EMISORRECU_IMPRESSIONS] =      "0";
                    listbinds[CP_FORMATDOC_IMPRESSIONS] =       IMAGERIE;
                    listbinds[CP_IDLIEU_IMPRESSIONS] =          idLieuExercice;
                    DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
                    if(doc != Q_NULLPTR)
                    {
                        delete doc;
                        FichierImage.remove();
                        QString CheminOKTransfrDocOrigin    = CheminOKTransfrDirOrigin + "/" + nomdoc;
                        FichierOrigine.copy(CheminOKTransfrDocOrigin);
                        QFile CO(CheminOKTransfrDocOrigin);
                        CO.open(QIODevice::ReadWrite);
                        CO.setPermissions(QFileDevice::ReadOther
                                          | QFileDevice::ReadGroup
                                          | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                                          | QFileDevice::ReadUser   | QFileDevice::WriteUser);
                        if (jnaltrsferfile.open(QIODevice::Append))
                        {
                            QTextStream out(&jnaltrsferfile);
                            out << Titredoc << " - " << nomdoc << " - " << idPatient << " - " << identpat << " - " << QHostInfo::localHostName() << "\n" ;
                            jnaltrsferfile.close();
                        }
                        if (FichierOrigine.remove())
                        {
                            QString msg = tr("Enregistrement d'un cliché") + " <font color=\"red\"><b>" + Titredoc + "</b></font>"
                                          " " + tr("pour") + " <font color=\"green\"><b>" + identpat + "</b></font> " + tr("dans la base de données");
                            if (formatdoc == "jpg" && szorigin != szfinal)
                                msg += "<br />" + tr("le cliché a été compressé de ") + szorigin + tr(" à ") + szfinal;
                            else
                                msg += "<br />" + tr("la taille du fichier est de ") + szorigin;
                            //qDebug() << "ba size = "  + QString::number(ba.size()) << "ba compressé size = " + QString::number(qCompress(ba).size());
                            listmsg << msg;
                            //qDebug() << "xx = " + QString::number(xx) << "x = " + QString::number(xx-DlgMess->width()-50) << "yy = " + QString::number(yy)  << "y = " + QString::number(yy-DlgMess->height()*(k+1))  << "itr = " << QString::number(k);
                        }
                        else
                            db->SupprRecordFromTable(idimpr, "idimpression", TBL_IMPRESSIONS);
                    }
                    else
                    {
                        commentechec = tr("impossible d'enregistrer ") + nomdoc;
                        EchecImport(Titredoc + " - " + nomdoc + " - " + commentechec + " - " + QHostInfo::localHostName());
                    }
                }
            }
        }
    }

    if (listmsg.size()>0)
        emit emitmsg(listmsg, 3000, true);
    EnCours = false;
}

bool ImportDocsExternesThread::DefinitDossiers()
{
    QString NomOnglet;
    if (db->getMode() == DataBase::Poste)
    {
        NomOnglet = tr("Monoposte");
        NomDirStockageImagerie = db->parametres()->dirimagerie();
    }
    if (db->getMode() == DataBase::ReseauLocal)
    {
        NomOnglet = tr("Réseau local");
        NomDirStockageImagerie  = proc->gsettingsIni->value("BDD_LOCAL/DossierImagerie").toString();
    }
    if (db->getMode() == DataBase::Distant)
    {
        NomOnglet = tr("Accès distant");
        NomDirStockageImagerie  = proc->gsettingsIni->value("BDD_DISTANT/DossierImagerie").toString();
    }

    NomDirStockageProv      = NomDirStockageImagerie + DIR_PROV;
    if (!Utils::mkpath(NomDirStockageProv))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + NomDirStockageProv + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000, false);
        return false;
    }
    CheminOKTransfrDir      = NomDirStockageImagerie + DIR_IMAGES + "/" + datetransfer;
    if (!Utils::mkpath(CheminOKTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDir + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000, false);
        return false;
    }
    CheminEchecTransfrDir   = NomDirStockageImagerie + DIR_ECHECSTRANSFERTS;
    if (!Utils::mkpath(CheminEchecTransfrDir))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminEchecTransfrDir + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000, false);
        return false;
    }

    if (gAcces == Local)
    {
        CheminOKTransfrDirOrigin    = NomDirStockageImagerie + DIR_ORIGINAUX DIR_IMAGES + "/" + datetransfer;
        if (!Utils::mkpath(CheminOKTransfrDirOrigin))
        {
            QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + CheminOKTransfrDirOrigin + "</b></font>" + tr(" invalide");
            QStringList listmsg;
            listmsg << msg;
            emit emitmsg(listmsg, 3000, false);
            return false;
        }
    }
    return true;
}

void ImportDocsExternesThread::EchecImport(QString txt)
{
    QString msg = tr("Impossible d'enregistrer le fichier ") + "<font color=\"red\"><b>" + QFileInfo(FichierOrigine).fileName() + "</b></font>" + tr(" dans la base de données");
    QStringList listmsg;
    listmsg << msg;
    emit emitmsg(listmsg, 3000, false);

    QString CheminEchecTransfrDoc   = CheminEchecTransfrDir + "/" + QFileInfo(FichierOrigine).fileName();
    FichierOrigine.copy(CheminEchecTransfrDoc);
    FichierOrigine.remove();
    QString echectrsfername         = CheminEchecTransfrDir + "/0EchecTransferts - " + datetransfer + ".txt";
    QFile   echectrsfer(echectrsfername);
    if (echectrsfer.open(QIODevice::Append))
    {
        QTextStream out(&echectrsfer);
        out << txt << "\n" ;
        echectrsfer.close();
    }
}
