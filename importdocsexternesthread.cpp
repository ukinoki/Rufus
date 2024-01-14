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

    QString NomDirDoc   = appareil->nomdossierechange();
    QString Titredoc    = appareil->titreexamen();
    if (NomDirDoc == "")
        NomDirDoc = "Triumph Speed Triple 1050 2011";

    // Titre du document------------------------------------------------------------------------------------------------------------------------------------------------
    QString Typedoc     = Titredoc;
    QString SousTypeDoc = Titredoc;
    QString Appareil    = appareil->nomappareil();
    QString CheminFichierImage = NomDirDoc + "/" + nomfiledoc;
    QFile   jnaltrsferfile(m_pathdirOKtransfer + "/0JournalTransferts - " + m_datetransfer + ".txt");
    if (jnaltrsferfile.open(QIODevice::Append))
    {
        QTextStream out(&jnaltrsferfile);
        out << m_currentdate.toString("yyyy-MM-dd") << QTime::currentTime().toString() << " - " + tr ("Rapatriement de ") << Titredoc << " - " << nomfiledoc << " - " << QHostInfo::localHostName() << "\n" ;
        jnaltrsferfile.close();
    }
    QString commentechec;

    file_origine.setFileName(CheminFichierImage);
    QString datetimecreation = QFileInfo(file_origine).birthTime().toString("yyyyMMdd-HHmmss");

    // Date et type du document------------------------------------------------------------------------------------------------------------------------------------------------
    QString datestring  = "";
    if (Appareil == "TOPCON ALADDIN")
    {
        QDateTime datefic   = QFileInfo(file_origine).birthTime();
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
                EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
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
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
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

    if (!QDate().fromString(datestring,"yyyyMMdd").isValid())
    {
        commentechec =  tr("date invalide") + " -> " + datestring;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
        return;
    }

    datestring = QDate().fromString(datestring,"yyyyMMdd").toString("yyyy-MM-dd");

    // Format du document------------------------------------------------------------------------------------------------------------------------------------------------
    QString formatdoc = QFileInfo(nomfiledoc).suffix().toLower();
    if (formatdoc != "pdf" && formatdoc != "jpg" && formatdoc != "jpeg")
    {
        commentechec = tr("format invalide") + " -> " + formatdoc;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
        return;
    }

    // Contenu du document------------------------------------------------------------------------------------------------------------------------------------------------
    QByteArray ba;
    QString nomfichresize = m_pathdirstockageprovisoire + "/resize" + nomfiledoc;
    QString szorigin, szfinal;
    // on vide le dossier provisoire
    QStringList listfichresize = QDir(m_pathdirstockageprovisoire).entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (int t=0; t<listfichresize.size(); t++)
    {
        QString nomdocrz  = listfichresize.at(t);
        QString CheminFichierResize = m_pathdirstockageprovisoire + "/" + nomdocrz;
        QFile file(CheminFichierResize);
        Utils::removeWithoutPermissions(file);
    }
    if (file_origine.open(QIODevice::ReadOnly))
    {
        double sz = file_origine.size();
        if (sz/(1024*1024) > 1)
            szorigin = QString::number(sz/(1024*1024),'f',1) + "Mo";
        else
            szorigin = QString::number(sz/1024,'f',1) + "Ko";
        szfinal = szorigin;
        Utils::copyWithPermissions(file_origine,nomfichresize);
        file_image.setFileName(nomfichresize);
        if (formatdoc == "jpg" && sz > TAILLEMAXIIMAGES)
        {
            QImage  img(nomfichresize);
            Utils::removeWithoutPermissions(file_image);
            QPixmap pixmap;
            pixmap = pixmap.fromImage(img.scaledToWidth(2560,Qt::SmoothTransformation));
            int     tauxcompress = 90;
            while (sz > TAILLEMAXIIMAGES && tauxcompress > 1)
            {
                pixmap.save(nomfichresize, "jpeg",tauxcompress);
                sz = file_image.size();
                tauxcompress -= 10;
            }
            if (sz/(1024*1024) > 1)
                szfinal = QString::number(sz/(1024*1024),'f',0) + "Mo";
            else
                szfinal = QString::number(sz/1024,'f',0) + "Ko";
        }
        file_image.open(QIODevice::ReadOnly);
        ba = file_image.readAll();
    }
    else
    {
        commentechec =  tr("Impossible d'ouvrir le fichier");
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
        return;
    }
    //qDebug()<< "ba.size() = " + QString::number(ba.size());

    // IdPatient------------------------------------------------------------------------------------------------------------------------------------------------
    QString req(""), idPatient("");
    if (Appareil == "TOPCON ALADDIN")   {
        QStringList listn = nomfiledoc.split("_");
        if (listn.size()<5)
        {
            commentechec =  tr("Impossible d'ouvrir le fichier");
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
            return;
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
        QVariantList patlst = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
        if (!m_ok || patlst.size()==0)
        {
            commentechec =  tr("Impossible d'ouvrir le fichier");
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
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

    bool b=true;
    if (idPatient.toInt(&b)<1)
    {
        commentechec = tr("idPatient invalide") + " -> " + idPatient;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
        return;
    }
    if (!b)
    {
        commentechec = tr("idPatient invalide") + " -> " + idPatient;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
        return;
    }
    QString identpat;
    QVariantList patlst = db->getFirstRecordFromStandardSelectSQL("select patnom, patprenom from " TBL_PATIENTS " where idpat = " + idPatient, m_ok);
    if (!m_ok || patlst.size()==0)
    {
        commentechec =  tr("Pas de patient pour cet idPatient") + " -> " + idPatient;
        EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
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

    QString NomFileDoc = idPatient + "_"
            + Typedoc + "_"
            + SousTypeDoc + "_"
            + datetimecreation
            + "-" + QString::number(idimpr)
            + "." + QFileInfo(nomfiledoc).suffix();

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
                + "/" + m_datetransfer + "/" + NomFileDoc + "', "
                + "0" + ", '"
                IMAGERIE "', "
                + QString::number(Datas::I()->sites->idcurrentsite()) + ")";

        if(db->StandardSQL(req))
        {
            QString CheminOKTransfrDoc          = m_pathdirOKtransfer + "/" + NomFileDoc;
            QString CheminOKTransfrDocOrigin    = m_pathdirOKtransferorigin + "/" + nomfiledoc;
            Utils::copyWithPermissions(file_image,CheminOKTransfrDoc);
            Utils::removeWithoutPermissions(file_image);
            Utils::copyWithPermissions(file_origine, CheminOKTransfrDocOrigin);
            if (jnaltrsferfile.open(QIODevice::Append))
            {
                QTextStream out(&jnaltrsferfile);
                out << m_currentdate.toString("yyyy-MM-dd") << QTime::currentTime().toString() << Titredoc << " - " << nomfiledoc << " - " << idPatient << " - " << identpat << " - " << QHostInfo::localHostName() << "\n" ;
                jnaltrsferfile.close();
            }
            if (Utils::removeWithoutPermissions(file_origine))
            {
                QString msg = tr("Enregistrement d'un cliché") + " <font color=\"red\"><b>" + Titredoc + "</b></font>"
                                                                                                         " " + tr("pour") + " <font color=\"green\"><b>" + identpat + "</b></font> " + tr("dans la base de données");
                if (formatdoc == "jpg" && szorigin != szfinal)
                    msg += "<br />" + tr("le cliché a été compressé de ") + szorigin + tr(" à ") + szfinal;
                else
                    msg += "<br />" + tr("la taille du fichier est de ") + szorigin;
                //qDebug() << "ba size = "  + QString::number(ba.size()) << "ba compressé size = " + QString::number(qCompress(ba).size());
                m_listmsg << msg;
                //qDebug() << "xx = " + QString::number(xx) << "x = " + QString::number(xx-DlgMess->width()-50) << "yy = " + QString::number(yy)  << "y = " + QString::number(yy-DlgMess->height()*(k+1))  << "itr = " << QString::number(k);
            }
            else
                db->SupprRecordFromTable(idimpr, CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES);
        }
        else
        {
            commentechec = tr("impossible d'enregistrer le fichier");
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
        }
    }
    else if (m_acces == Distant)
    {
        if (formatdoc == "pdf")
            formatdoc = CP_PDF_DOCSEXTERNES;
        else if (formatdoc== "jpg" || formatdoc == "jpeg")
            formatdoc = CP_JPG_DOCSEXTERNES;
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
        listbinds[formatdoc]                    = ba;
        listbinds[CP_EMISORRECU_DOCSEXTERNES]   = "0";
        listbinds[CP_FORMATDOC_DOCSEXTERNES]    = IMAGERIE;
        listbinds[CP_IDLIEU_DOCSEXTERNES]       = Datas::I()->sites->idcurrentsite();
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if(doc != Q_NULLPTR)
        {
            delete doc;
            Utils::removeWithoutPermissions(file_image);
            QString CheminOKTransfrDocOrigin    = m_pathdirOKtransferorigin + "/" + nomfiledoc;
            Utils::copyWithPermissions(file_origine, CheminOKTransfrDocOrigin);
            if (jnaltrsferfile.open(QIODevice::Append))
            {
                QTextStream out(&jnaltrsferfile);
                out << Titredoc << " - " << nomfiledoc << " - " << idPatient << " - " << identpat << " - " << QHostInfo::localHostName() << "\n" ;
                jnaltrsferfile.close();
            }
            if (Utils::removeWithoutPermissions(file_origine))
            {
                QString msg = tr("Enregistrement d'un cliché") + " <font color=\"red\"><b>" + Titredoc + "</b></font>"
                                                                                                         " " + tr("pour") + " <font color=\"green\"><b>" + identpat + "</b></font> " + tr("dans la base de données");
                if (formatdoc == "jpg" && szorigin != szfinal)
                    msg += "<br />" + tr("le cliché a été compressé de ") + szorigin + tr(" à ") + szfinal;
                else
                    msg += "<br />" + tr("la taille du fichier est de ") + szorigin;
                //qDebug() << "ba size = "  + QString::number(ba.size()) << "ba compressé size = " + QString::number(qCompress(ba).size());
                m_listmsg << msg;
                //qDebug() << "xx = " + QString::number(xx) << "x = " + QString::number(xx-DlgMess->width()-50) << "yy = " + QString::number(yy)  << "y = " + QString::number(yy-DlgMess->height()*(k+1))  << "itr = " << QString::number(k);
            }
            else
                db->SupprRecordFromTable(idimpr, CP_ID_DOCSEXTERNES, TBL_DOCSEXTERNES);
        }
        else
        {
            commentechec = tr("impossible d'enregistrer ") + nomfiledoc;
            EchecImport(Titredoc + " - " + nomfiledoc + " - " + commentechec + " - " + QHostInfo::localHostName());
        }
    }

    if (m_listmsg.size()>0)
        emit emitmsg(m_listmsg, 3000);
    m_encours = false;
}

/*! \brief bool ImportDocsExternesThread::DefinitDossiersImagerie()
 * Définit l'emplacement des dossiers utilisés
 * \param m_pathdirstockageimagerie =   l'emplacement baseURL où seront stockés les fichiers d'imagerie (directement sur le serveur en mode monoposte ou réseau local - sur un dossier du client en mode sitant
 *`\param m_pathdirstockageprovisoire = l'emplacement où les appareils d'imagerie vont écrire les fichiers qu'ils émettent en attendant leur intégration dans la BDD
 * \param m_pathdirOKtransfer =         le resolved URL de l'emplacement de stockage définitif des fichiers
 * \param m_pathdirechectransfer =      le resolved URL de l'emplacement où sont transférés les fichiers qui n'ont pas pu être intégrés dans la BDD
 * \param m_pathdiroriginOKtransfer =   le resolved URL de l'emplacement de stockage des copies des fichiers image d'origine
 */
bool ImportDocsExternesThread::DefinitDossiersImagerie()
{
    m_pathdirstockageimagerie   = proc->DefinitDossierImagerie();
    m_pathdirstockageprovisoire = m_pathdirstockageimagerie + NOM_DIR_PROV;
    if (!Utils::mkpath(m_pathdirstockageprovisoire))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + m_pathdirstockageprovisoire + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000);
        return false;
    }
    m_pathdirOKtransfer      = m_pathdirstockageimagerie + NOM_DIR_IMAGES + "/" + m_datetransfer;
    if (!Utils::mkpath(m_pathdirOKtransfer))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + m_pathdirOKtransfer + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000);
        return false;
    }
    m_pathdirechectransfer   = m_pathdirstockageimagerie + NOM_DIR_ECHECSTRANSFERTS;
    if (!Utils::mkpath(m_pathdirechectransfer))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + m_pathdirechectransfer + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000);
        return false;
    }
    m_pathdirOKtransferorigin    = m_pathdirstockageimagerie + NOM_DIR_ORIGINAUX NOM_DIR_IMAGES + "/" + m_datetransfer;
    if (!Utils::mkpath(m_pathdirOKtransferorigin))
    {
        QString msg = tr("Dossier de sauvegarde ") + "<font color=\"red\"><b>" + m_pathdirOKtransferorigin + "</b></font>" + tr(" invalide");
        QStringList listmsg;
        listmsg << msg;
        emit emitmsg(listmsg, 3000);
        return false;
    }
    return true;
}

void ImportDocsExternesThread::EchecImport(QString txt)
{
    QString msg = tr("Impossible d'enregistrer le fichier ") + "<font color=\"red\"><b>" + QFileInfo(file_origine).fileName() + "</b></font>" + tr(" dans la base de données");
    QStringList listmsg;
    listmsg << msg;
    emit emitmsg(listmsg, 3000);

    QString CheminEchecTransfrDoc   = m_pathdirechectransfer + "/" + QFileInfo(file_origine).fileName();
    Utils::copyWithPermissions(file_origine, CheminEchecTransfrDoc);
    Utils::removeWithoutPermissions(file_origine);
    QString echectrsfername         = m_pathdirechectransfer + "/0EchecTransferts - " + m_datetransfer + ".txt";
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

