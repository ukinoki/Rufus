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

#include "importdocsexternesthread.h"

ImportDocsExternesThread::ImportDocsExternesThread()
{
    moveToThread(&m_thread);
    m_acces           = (db->ModeAccesDataBase()!=Utils::Distant? Local : Distant);
    m_thread          .start();
}

void ImportDocsExternesThread::RapatrieDocumentsThread(AppareilImagerie *appareil, QString nomfiledoc)
{
    if (m_encours)
        return;
    if (appareil == Q_NULLPTR)
    {
        m_encours = false;
        return;
    }
    m_encours = true;
    QDate m_currentdate = db->ServerDate();
    m_listmsg.clear();
    m_datetransfer = m_currentdate.toString("yyyy-MM-dd");
    if (!DefinitDossiersImagerie())
    {
        m_encours = false;
        return;
    }
    /* Retrouver
     * Titre du document
     * Date du document
     * contenu du fichier
     * idpatient
     */
    //qDebug() << appareil->nomappareil();
    QString NomDirDoc   = appareil->nomdossierechange();
    QString Titredoc    = appareil->titreexamen();
    if (NomDirDoc == "")
        NomDirDoc = "Triumph Speed Triple 1050 2011";

    // Titre du document------------------------------------------------------------------------------------------------------------------------------------------------
    QString Typedoc     = Titredoc;
    QString SousTypeDoc = Titredoc;
    QString Appareil    = appareil->nomappareil();
    QString path_file_origin = NomDirDoc + "/" + nomfiledoc;
    QFile   jnaltrsferfile(proc->AbsolutePathDirImagerie() + NOM_DIR_IMAGES + "/" + m_datetransfer + "/0JournalTransferts - " + m_datetransfer + ".txt");
    if (jnaltrsferfile.open(QIODevice::Append))
    {
        QTextStream out(&jnaltrsferfile);
        out << m_currentdate.toString("yyyy-MM-dd") << QTime::currentTime().toString() << " - " + tr ("Rapatriement de ") << Titredoc << " - " << nomfiledoc << " - " << Utils::hostName() << "\n" ;
        jnaltrsferfile.close();
    }
    QString commentechec;

    file_origin.setFileName(path_file_origin);
    Utils::copyWithPermissions(file_origin, proc->AbsolutePathDirImagerie() + NOM_DIR_ORIGINAUX NOM_DIR_IMAGES + "/" + m_datetransfer + "/" + nomfiledoc);

    QString datetimecreation = QFileInfo(file_origin).birthTime().toString("yyyyMMdd-HHmmss");

    // Date et type du document------------------------------------------------------------------------------------------------------------------------------------------------
    QString datestring  = "";
    if (Appareil == "TOPCON ALADDIN")
    {
        QDateTime datefic   = QFileInfo(path_file_origin).birthTime();
        datestring          = datefic.toString("yyyyMMdd");
        Titredoc            = "Biométrie - Aladdin";
        Typedoc             = "Biométrie";
        SousTypeDoc         = "Aladdin";
    }
    else if (Appareil == "TOPCON ALADDIN II")
    {
        //1051_MIGUEL_JEAN-ROBERT_01-06-1948_Aladdin_06_06_2018_13_16.pdf
        QStringList listn   = nomfiledoc.split("_");
        int n               = listn.size();
        if (n>4)
        {
            QString jour        = listn.at(n-5);
            QString mois        = listn.at(n-4);
            QString annee       = listn.at(n-3);
            if (QDate(annee.toInt(), mois.toInt(), jour.toInt()).isValid())
                datestring      = annee + mois + jour;
        }
        if (datestring == "")
            datestring      = m_currentdate.toString("yyyyMMdd");
        Titredoc            = "Biométrie - Aladdin";
        Typedoc             = "Biométrie";
        SousTypeDoc         = "Aladdin";
    }
    else if (Appareil == "TOPCON TRITON")
    {
        if (nomfiledoc.split("_").size()>3)
        {
            datestring  = nomfiledoc.split("_").at(3);
            datestring  = datestring.left(8);
        }
        else
             datestring      = m_currentdate.toString("yyyyMMdd");
        Titredoc    = "OCT - Topcon";
        Typedoc     = "OCT";
        SousTypeDoc = "Topcon";
    }
    else if (Appareil == "CANON CR-2")
    {
        if (nomfiledoc.split("_").size()>1)
        {
            datestring  = nomfiledoc.split("_").at(1);
            datestring  = datestring.left(8);
        }
        else
             datestring      = m_currentdate.toString("yyyyMMdd");
        Titredoc    = "RNM - Canon";
        Typedoc     = "RNM";
        QString cote = "";
         if (nomfiledoc.split("_").size()>2)
             cote = ((nomfiledoc.split("_").at(2) == "R")? tr("OD") : tr("OG"));
        SousTypeDoc = "Canon " + cote;
    }
    else if (Appareil == "OTI SLO") {
        datestring  = nomfiledoc.mid(nomfiledoc.indexOf("-")+1,8);
        Titredoc    = "OCT - OTI";
        Typedoc     = "OCT";
        SousTypeDoc = "OTI";
    }
    else if (Appareil == "OPTOVUE")
    {
        QStringList list = nomfiledoc.split("_");
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
        if (nomfiledoc.split("_").size()>1)
        {
            datestring = nomfiledoc.split("_").at(1);
            datestring = datestring.left(8);
        }
        Titredoc    = "OCT - Nidek";
        Typedoc     = "OCT";
        SousTypeDoc = "Nidek";
    }
    else if (Appareil == "CANON-OCT")
    {
        if (nomfiledoc.split("_").size()>1)
        {
            datestring = nomfiledoc.split("_").at(1);
            datestring = datestring.left(8);
        }
        Titredoc    = "OCT - Canon";
        Typedoc     = "OCT";
        QString cote = (nomfiledoc.contains("BothEyes")? tr("ODG") : ((nomfiledoc.split("_").at(4) == "R")? tr("OD") : tr("OG")));
        QString typeexam = nomfiledoc.split("_").at(3);
        if (typeexam == "OCTA")     typeexam = "AngioOCT";
        if (typeexam == "Disc3D")   typeexam = "Glaucome";
        SousTypeDoc = "Canon " + typeexam + " " + cote;
        if (typeexam == "Disc3D")   typeexam = "Glaucome";
        datetimecreation = datestring + "-" + nomfiledoc.split("_").at(2);
        QStringList filters;
        filters << "*.exd";
        QStringList filesnames;
        filesnames = QDir(NomDirDoc).entryList(filters, QDir::Files);
        for (int j=0 ; j < filesnames.size(); j++)
        {
            QFile exdfile(NomDirDoc + "/" + filesnames.at(j));
            Utils::removeWithoutPermissions(exdfile);
        }
    }
    else if (Appareil == "NIDEK-RNM")
    {
        if (nomfiledoc.split("_").size()>1)
        {
            datestring = nomfiledoc.split("_").at(1);
            datestring = datestring.left(8);
        }
        Titredoc    = "RNM - Nidek";
        Typedoc     = "RNM";
        SousTypeDoc = "Nidek";
    }
    else if (Appareil == "SPECTRALIS")
    {
        if (nomfiledoc.split("_").size()>1)
        {
            datestring = nomfiledoc.split("_").at(1);
            datestring = datestring.left(8);
        }
        Titredoc    = "OCT - Heidelberg";
        Typedoc     = "OCT";
        SousTypeDoc = "Heidelberg";
    }
    else if (Appareil == "NAVIS-EX")
    {
        if (nomfiledoc.split("_").size()>1)
        {
            datestring = nomfiledoc.split("_").at(1);
            datestring = datestring.left(8);
        }
        QString AbregeTitre = nomfiledoc.split("_").at(3);
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

        QRegularExpression re;
        re.setPattern("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}_[0-9]{2}_[0-9]{2}Z-"); //! correspond aux sections 2,3,et 4 avant 2019 et 3,4 et 5 après
        if (nomfiledoc.contains(re))
        {
            int idxdate;
            Titredoc    = "RNM - Eidon ";
            Typedoc     = "RNM";
            idxdate = nomfiledoc.indexOf(re);
            datestring = nomfiledoc.mid(idxdate,10).replace("-","");
            datetimecreation = datestring + "-" + nomfiledoc.mid(idxdate + 11,8).replace("_","");
            QString details (""), cote("");
            if (nomfiledoc.split(re).size()>1)
                details = nomfiledoc.split(re).at(1);
            else
            {
                commentechec =  tr("nom invalide");
                EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
                return;
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
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
            return;
        }
    }
    else if (Appareil == "OPTOS Daytona series")
    {
        if (nomfiledoc.split("-").size()>1)
        {
            datestring = nomfiledoc.split("-").at(1);
            datestring = datestring.left(8);
        }
        Titredoc    = "RNM - Optos";
        Typedoc     = "RNM";
        SousTypeDoc = "Optos";
    }
    else if (Appareil == "ZEISS CIRRUS 5000")
    {
        if (nomfiledoc.split("_").size()>7)
        {
            datestring = nomfiledoc.split("_").at(7);
            datestring = datestring.left(8);
        }
        Titredoc    = "OCT - Zeiss";
        Typedoc     = "OCT";
        SousTypeDoc = "Zeiss";
    }
    else if (Appareil == "ION Imaging")
    {
        //! 37214_0D_20200522_1848188838.01.e.jpg
        if (nomfiledoc.split("_").size()>2)
            datestring = nomfiledoc.split("_").at(2);
        Titredoc    = "Photo - ION";
        Typedoc     = "Imagerie SA";
        SousTypeDoc = "ION";
    }
    else if (Appareil == "TOPCON TRC-NW400")
    {
        //! 20240126_23_R_012.JPG
        if (nomfiledoc.split("_").size()>2)
            datestring = nomfiledoc.split("_").at(0);
        Titredoc    = "RNM TRC-NW400";
        Typedoc     = "RNM";
        QString cote = nomfiledoc.split("_").at(2);
        cote = (cote=="R")? tr("OD") : tr("OG");
        SousTypeDoc = "TRC-NW400 " + cote;
    }
    else if (Appareil == "ESSILOR Retina 550")
    {
        //! ALI BACAR- Noeline -P2077839101-_OD.jpg
            datestring = QDate::currentDate().toString("yyyyMMdd");
        Titredoc    = "RNM - Retina 550";
        Typedoc     = "RNM";
        QString cote ("");
        if (nomfiledoc.split("_").size()>0)
            cote = nomfiledoc.split("_").at(1);
        cote = (cote=="OD")? tr("OD") : tr("OG");
        SousTypeDoc = "Retina 550";
        if (cote != "")
            SousTypeDoc += " " + cote;
    }

    if (!QDate().fromString(datestring,"yyyyMMdd").isValid())
    {
        commentechec =  tr("date invalide") + " -> " + datestring;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
        return;
    }

    datestring = QDate().fromString(datestring,"yyyyMMdd").toString("yyyy-MM-dd");

    // Format du document------------------------------------------------------------------------------------------------------------------------------------------------
    QString suffix = QFileInfo(file_origin).suffix().toLower();
    if (!Utils::isFormatRecognized(file_origin))
    {
        commentechec = tr("format invalide") + " -> " + suffix;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
        return;
    }

    // Contenu du document------------------------------------------------------------------------------------------------------------------------------------------------
    QByteArray ba;
    QString szorigin, szfinal;
    qint64 sz = file_origin.size();
    szorigin = Utils::getExpressionSize(sz);
    if ((suffix == JPG || suffix == JPEG || suffix == PNG) && sz > TAILLEMAXIIMAGES)
        Utils::CompressFileToJPG(path_file_origin, false);
    file_origin.setFileName(path_file_origin);
    file_origin.open(QIODevice::ReadOnly);
    ba = file_origin.readAll();
    file_origin.close();
    szfinal = Utils::getExpressionSize(file_origin.size());

    // IdPatient------------------------------------------------------------------------------------------------------------------------------------------------
    QString req(""), idPatient("");
    if (Appareil == "TOPCON ALADDIN")   {
        QStringList listn = nomfiledoc.split("_");
        if (listn.size()<5)
        {
            commentechec =  tr("Impossible d'ouvrir le fichier");
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
            return;
        }
        QString nom     = Utils::capitilize(listn.at(0));
        QString prenom  = Utils::capitilize(listn.at(1));
        QString jour    = Utils::capitilize(listn.at(2));
        QString mois    = Utils::capitilize(listn.at(3));
        QString annee   = Utils::capitilize(listn.at(4));
        req             = "select " CP_IDPAT_PATIENTS " from " TBL_PATIENTS
                        " where " CP_NOM_PATIENTS " like '" + nom + "'"
                        " and " CP_PRENOM_USR " like '" + prenom  + "'"
                        " and " CP_DDN_PATIENTS " = '" + annee + "-" + mois + "-" + jour + "'";
        //qDebug() << req;
        QVariantList patlst = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
        if (!m_ok || patlst.size()==0)
        {
            commentechec =  tr("Impossible d'ouvrir le fichier");
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
            return;
        }
        idPatient = patlst.at(0).toString();
        //qDebug() << idPatient;
    }
    else if (Appareil == "TOPCON ALADDIN II")
        idPatient           = nomfiledoc.split("_").at(0);
    else if (Appareil == "TOPCON TRITON")
        idPatient           = nomfiledoc.split("_").at(2);
    else if (Appareil == "CANON CR-2")
        idPatient           = nomfiledoc.split("_").at(0);
    else if (Appareil == "OTI SLO")
        idPatient           = nomfiledoc.split("-").at(0);
    else if (Appareil == "OPTOVUE")
    {
        if (nomfiledoc.split("__").size()>0)
        {
            idPatient       = nomfiledoc.split("__").at(1);
            if (idPatient.split("_").size()>0)
                idPatient   = idPatient.split("_").at(0);
        }
    }
    else if (Appareil == "NIDEK-OCT")   {
        idPatient           = nomfiledoc.split("_").at(0);
    }
    else if (Appareil == "CANON-OCT")   {
        idPatient           = nomfiledoc.split("_").at(0);
    }
    else if (Appareil == "NIDEK-RNM")   {
        idPatient           = nomfiledoc.split("_").at(0);
    }
    else if (Appareil == "SPECTRALIS")   {
        idPatient           = nomfiledoc.split("_").at(0);
    }
    else if (Appareil == "NAVIS-EX")   {
        idPatient           = nomfiledoc.split("_").at(0);
    }
    else if (Appareil == "EIDON")   {
        if (nomfiledoc.split("-").at(0).toInt()>0)
            idPatient           = nomfiledoc.split("-").at(0);
        else
            idPatient           = nomfiledoc.split("-").at(2);
    }
    else if (Appareil == "OPTOS Daytona series")   {
        idPatient           = nomfiledoc.split("-").at(0);
    }
    else if (Appareil == "ZEISS CIRRUS 5000")
    {
        idPatient           = nomfiledoc.split("_").at(3);
    }
    else if (Appareil == "ION Imaging")
    {
        //! 37214_0D_20200522_1848188838.01.e.jpg
        idPatient           = nomfiledoc.split("_").at(0);
    }
    else if (Appareil == "TOPCON TRC-NW400")
    {
        //! 20240126_23_R_012.JPG
        idPatient           = nomfiledoc.split("_").at(1);
    }
    else if (Appareil == "ESSILOR Retina 550")
    {
        //! ALI BACAR- Noeline -P2077839101-_OD.jpg
        idPatient           = nomfiledoc.split("_").at(1);
    }

    bool b=true;
    if (idPatient.toInt(&b)<1)
    {
        commentechec = tr("idPatient invalide") + " -> " + idPatient;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
        return;
    }
    if (!b)
    {
        commentechec = tr("idPatient invalide") + " -> " + idPatient;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
        return;
    }
    QString identpat;
    QVariantList patlst = db->getFirstRecordFromStandardSelectSQL("select " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS " from " TBL_PATIENTS " where " CP_IDPAT_PACHY " = " + idPatient, m_ok);
    if (!m_ok || patlst.size()==0)
    {
        commentechec =  tr("Pas de patient pour cet idPatient") + " -> " + idPatient;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
        return;
    }
    identpat = patlst.at(0).toString() + " " + patlst.at(1).toString();
    //qDebug() << identpat;

    /*! _______________________________________________________________________________________________________________________________________________________
     * Enregistrement du fichier dans la base
     * Si on est sur le réseau local, l'enregistrement se fait dans la table Impressions et le fichier est copié à son adresse définitive
     * Si on est en accès distant, l'enregistrement se fait dans la table Impressions et le contenu du fichier est copié dans le champ blob de la table de la table
     * _______________________________________________________________________________________________________________________________________________________
     */
    int idimpr = db->selectMaxFromTable(CP_ID_DOCSEXTERNES,  TBL_DOCSEXTERNES, m_ok)+1;
    QString NomFileFinal = idPatient + "_"
            + Typedoc + "_"
            + SousTypeDoc + "_"
            + datetimecreation
            + "-" + QString::number(idimpr)
            + "." + QFileInfo(file_origin).suffix().toLower();

    if (m_acces == Local)
    {
        req = "insert into " TBL_DOCSEXTERNES " (" CP_ID_DOCSEXTERNES ", " CP_IDUSER_DOCSEXTERNES ",  " CP_IDPAT_DOCSEXTERNES ",  " CP_TYPEDOC_DOCSEXTERNES ",  " CP_SOUSTYPEDOC_DOCSEXTERNES ", " CP_TITRE_DOCSEXTERNES ", " CP_DATE_DOCSEXTERNES ","
                CP_IDEMETTEUR_DOCSEXTERNES ", " CP_LIENFICHIER_DOCSEXTERNES ", " CP_EMISORRECU_DOCSEXTERNES ", " CP_FORMATDOC_DOCSEXTERNES ", " CP_IDLIEU_DOCSEXTERNES ")"
                                                                                                                                                                       " values("
                + QString::number(idimpr) + ", "
                + QString::number(Datas::I()->users->userconnected()->id()) + ", "
                + idPatient + ", '"
                + Typedoc + "', '"
                + SousTypeDoc + "', '"
                + Titredoc + "', '"
                + datestring + " " + QTime::currentTime().toString("HH:mm:ss") + "', "
                + QString::number(Datas::I()->users->userconnected()->id()) + ", '"
                + "/" + m_datetransfer + "/" + NomFileFinal + "', "
                + "0" + ", '"
                IMAGERIE "', "
                + QString::number(Datas::I()->sites->idcurrentsite()) + ")";

        if(db->StandardSQL(req))
        {
            QString pathDocFinal = proc->AbsolutePathDirImagerie() + NOM_DIR_IMAGES + "/" + m_datetransfer + "/" + NomFileFinal;
            Utils::copyWithPermissions(file_origin,pathDocFinal);
            if (jnaltrsferfile.open(QIODevice::Append))
            {
                QTextStream out(&jnaltrsferfile);
                out << m_currentdate.toString("yyyy-MM-dd") << QTime::currentTime().toString() << Titredoc << " - " << nomfiledoc << " - " << idPatient << " - " << identpat << " - " << Utils::hostName() << "\n" ;
                jnaltrsferfile.close();
            }
            if (Utils::removeWithoutPermissions(file_origin))
            {
                QString msg = tr("Enregistrement d'un cliché") + " <font color=\"red\"><b>" + Titredoc + "</b></font>"
                              " " + tr("pour") + " <font color=\"green\"><b>" + identpat + "</b></font> " + tr("dans la base de données");
                if ((suffix == JPG || suffix == JPEG || suffix == PNG) && szorigin != szfinal)
                    msg += "<br />" + tr("le cliché a été compressé de ") + szorigin + tr(" à ") + szfinal;
                else
                    msg += "<br />" + tr("la taille du fichier est de ") + szorigin;
                m_listmsg << msg;
            }
            else
                db->SupprRecordFromTable(idimpr, CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES);
        }
        else
        {
            commentechec = tr("impossible d'enregistrer le fichier");
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
        }
    }
    else if (m_acces == Distant)
    {
        if (suffix == PDF)
            suffix = CP_PDF_DOCSEXTERNES;
        else if (suffix== JPG || suffix == JPEG || suffix == PNG)
            suffix = CP_JPG_DOCSEXTERNES;
        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        QHash<QString, QVariant> listbinds;
        listbinds[CP_ID_DOCSEXTERNES]           = idimpr;
        listbinds[CP_IDUSER_DOCSEXTERNES]       = Datas::I()->users->userconnected()->id();
        listbinds[CP_IDPAT_DOCSEXTERNES]        = idPatient;
        listbinds[CP_TYPEDOC_DOCSEXTERNES]      = Typedoc;
        listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES]  = SousTypeDoc;
        listbinds[CP_TITRE_DOCSEXTERNES]        = Titredoc;
        listbinds[CP_DATE_DOCSEXTERNES]         = datestring + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds[CP_IDEMETTEUR_DOCSEXTERNES]   = Datas::I()->users->userconnected()->id();
        listbinds[suffix]                    = ba;
        listbinds[CP_EMISORRECU_DOCSEXTERNES]   = "0";
        listbinds[CP_FORMATDOC_DOCSEXTERNES]    = IMAGERIE;
        listbinds[CP_IDLIEU_DOCSEXTERNES]       = Datas::I()->sites->idcurrentsite();
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if(doc != Q_NULLPTR)
        {
            delete doc;
            if (jnaltrsferfile.open(QIODevice::Append))
            {
                QTextStream out(&jnaltrsferfile);
                out << Titredoc << " - " << nomfiledoc << " - " << idPatient << " - " << identpat << " - " << Utils::hostName() << "\n" ;
                jnaltrsferfile.close();
            }
            if (Utils::removeWithoutPermissions(file_origin))
            {
                QString msg = tr("Enregistrement d'un cliché") + " <font color=\"red\"><b>" + Titredoc + "</b></font>"
                              " " + tr("pour") + " <font color=\"green\"><b>" + identpat + "</b></font> " + tr("dans la base de données");
                if (suffix == CP_JPG_DOCSEXTERNES && szorigin != szfinal)
                    msg += "<br />" + tr("le cliché a été compressé de ") + szorigin + tr(" à ") + szfinal;
                else
                    msg += "<br />" + tr("la taille du fichier est de ") + szorigin;
                m_listmsg << msg;
            }
            else
                db->SupprRecordFromTable(idimpr, CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES);
        }
        else
        {
            commentechec = tr("impossible d'enregistrer ") + nomfiledoc;
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + Utils::hostName());
        }
    }

    if (m_listmsg.size()>0)
        emit emitmsg(m_listmsg, 3000);
    m_encours = false;
}

/*! \brief bool ImportDocsExternesThread::DefinitDossiersImagerie()
 * Définit l'emplacement des dossiers utilisés
 * \param m_pathdirstockageimagerie =   l'emplacement baseURL où seront stockés les fichiers d'imagerie (directement sur le serveur en mode monoposte ou réseau local - sur un dossier du client en mode sitant
 * \param m_pathdirtransfer =         le resolved URL de l'emplacement de stockage définitif des fichiers
 * \param m_pathdirechectransfer =      le resolved URL de l'emplacement où sont transférés les fichiers qui n'ont pas pu être intégrés dans la BDD
 * \param m_pathdiroriginOKtransfer =   le resolved URL de l'emplacement de stockage des copies des fichiers image d'origine
 */
bool ImportDocsExternesThread::DefinitDossiersImagerie()
{

    if (!Utils::mkpath(proc->AbsolutePathDirImagerie() + NOM_DIR_IMAGES + "/" + m_datetransfer))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>"
                      + proc->AbsolutePathDirImagerie() + NOM_DIR_IMAGES + "/" + m_datetransfer
                      + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000);
        return false;
    }

    if (Utils::EchecDir() == "")
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + Utils::EchecDir() + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000);
        return false;
    }

    if (!Utils::mkpath(proc->AbsolutePathDirImagerie() + NOM_DIR_ORIGINAUX NOM_DIR_IMAGES + "/" + m_datetransfer))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>"
                      + proc->AbsolutePathDirImagerie() + NOM_DIR_ORIGINAUX NOM_DIR_IMAGES + "/" + m_datetransfer
                      + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000);
        return false;
    }
    return true;
}

void ImportDocsExternesThread::EchecImport(QString txt)
{
    QString msg = tr("Impossible d'enregistrer le fichier ") + "<font color=\"red\"><b>" + QFileInfo(file_origin).fileName() + "</b></font>" + tr(" dans la base de données");
    QStringList listmsg;
    listmsg << msg;
    emit emitmsg(listmsg, 3000);

    Utils::copyWithPermissions(file_origin, Utils::EchecDir() + "/" + QFileInfo(file_origin).fileName());
    Utils::removeWithoutPermissions(file_origin);
    QString echectrsfername         = Utils::EchecDir() + "/0EchecTransferts - " + m_datetransfer + ".txt";
    QFile   echectrsfer(echectrsfername);
    if (echectrsfer.open(QIODevice::Append))
    {
        QTextStream out(&echectrsfer);
        QDateTime dt = db->ServerDateTime();
        out << dt.date().toString("yyyy-MM-dd") << dt.time().toString() << txt << "\n" ;
        echectrsfer.close();
    }
    m_encours = false;
}
