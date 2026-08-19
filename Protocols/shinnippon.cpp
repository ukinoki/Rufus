#include "shinnippon.h"

ShinNippon* ShinNippon::instance = nullptr;

ShinNippon* ShinNippon::I()
{
    if( !instance )
        instance = new ShinNippon();
    return instance;
}

ShinNippon::ShinNippon() {}

void ShinNippon::LectureDonneesCOMRefracteur(QString Mesure, TypesMesures flag)
{
    Logs::LogToFile("MesuresRefracteur" + QDateTime::currentDateTime().toString("yyyyMMdd HH:mm:ss") + ".txt", Mesure);

    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString mAddOD      = "+0.00";
    QString mAddOG      = "+0.00";
    QString AVLOD(""), AVLOG(""), AVPOD(""), AVPOG("");
    QString PD          = "";
    int     idx;

    QString mesureOD(""), mesureOG("");
    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    QString mTOOD(""), mTOOG("");

    /*!
*SHIN-NIPPON DR-900 00003 2011/11/02
#RT
#VB 12.00
#WB 40 M
# R - 5.00 - 0.25 120               - refraction OD     ici -5.00(-0.25 à 120°)
# L - 2.00 - 0.50 50                - refraction OG         -2.00(-0.50 à 50°)
#AR + 6.25                          - addition VP OD
#AL + 1.25                          - addition VP OG
#PR O 2.50 D 9.00                   - prisme VL OD base: O = out I = int D = down U = up -> ici base 2.50 temporal 9.00 inférieur (ça ne fairt pas un gros prisme??)
#PL I 1.50 U 1.00
#Pr I 3.00 U 1.00                   - prisme VP OD      ici 3.00 base nasale 1.00 base sup
#Pl O 0.00 U 3.50
#pB 64.0                            - EIP
#pb 58.5                            - EIP en VP
#vR 0.1                             - acuité OD VL
#vL 0.032                           - acuité OG VL
#vB <0.032                          - acuité bino VL
#vr 0.1                             - acuité OD VP
#vl 0.2                             - acuité OG VP
#vb 0.3                             - acuité bino VP
#CV 5.0 4.0 3.0
#Cv 1.0 0.0 1.0
#DV 2.0 1.0
#Dv 2.0 3.0
#dV 0.5 1.0
#dv 3.0 3.5
#uV 0.5 0.0
#uv 2.0 2.5
#WF 2
#aA <0.5
#HA =
#SS 13'12


Le code de saut de ligne dans le sens DR-900 vers PC est « ¥r¥n » -> 0xA5 r 0xA5 n
# (SOX) commence une section (LM, RM, KM RT) et $ la termine

*/

    //! Données du FRONTO ---------------------------------------------------------------------------------------------------------------------
    if (flag.testFlag(MesureFronto))                //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur
    {
        MesureRefraction        *oldMesureFronto = new MesureRefraction();
        oldMesureFronto         ->setdatas(Datas::I()->mesurefronto);
        Datas::I()->mesurefronto->cleandatas();
        idx                     = Mesure.indexOf("LM");
        QString SectionFronto   = Mesure.right(Mesure.length()-idx);
        idx                     = Mesure.indexOf("$");
        SectionFronto           = Mesure.left(idx + 1);
        //qDebug() << "Procedures::LectureDonneesRefracteur(QString Mesure) - SectionFronto = " << SectionFronto;
        //Edit(SectionFronto + "\nOK");
        PD                      = SectionFronto.mid(SectionFronto.indexOf("pB")+4,2);
        Datas::I()->mesurefronto->setecartIP(PD.toInt());
        // OEIL DROIT -----------------------------------------------------------------------------
        if (SectionFronto.contains(" R"))
        {
            mesureOD     = SectionFronto.mid(SectionFronto.indexOf(" R")+4,17)   .replace(" ","0");
            mSphereOD    = mesureOD.mid(0,6);
            mCylOD       = mesureOD.mid(7,6);
            mAxeOD       = mesureOD.mid(14,3);
            if (SectionFronto.indexOf("AR")>-1)
                mAddOD   = SectionFronto.mid(SectionFronto.indexOf("AR")+4,6)    .replace(" ","0");
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (SectionFronto.contains(" L"))
        {
            mesureOG     = SectionFronto.mid(SectionFronto.indexOf(" L")+4,17)   .replace(" ","0");
            mSphereOG    = mesureOG.mid(0,6);
            mCylOG       = mesureOG.mid(7,6);
            mAxeOG       = mesureOG.mid(14,3);
            if (SectionFronto.indexOf("AL")>-1)
                mAddOG   = SectionFronto.mid(SectionFronto.indexOf("AR")+4,6)    .replace(" ","0");
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
        //debugMesureRefraction(Datas::I()->mesurefronto);
        if (Datas::I()->mesurefronto->isDifferent(oldMesureFronto) && !Datas::I()->mesurefronto->isdataclean())
            emit newmesure(MesureFronto);
        delete oldMesureFronto;
    }

    //! Données de l'AUTOREF - REFRACTION et KERATOMETRIE ----------------------------------------------------------------------------------------------
    if (flag.testFlag(MesureKerato))                //!=> il y a une mesure de keratométrie et l'autoref est connecté directement à la box du refraacteur
    {
        Keratometrie  *oldMesureKerato = new Keratometrie();
        oldMesureKerato->setdatas(Datas::I()->mesurekerato);
        Datas::I()->mesurekerato->cleandatas();
        idx                     = Mesure.indexOf("KM");
        QString SectionKerato   = Mesure.right(Mesure.length()-idx);
        idx                     = Mesure.indexOf("$");
        SectionKerato           = Mesure.left(idx + 1);
        //Edit(SectionKerato + "\nOK");
        // OEIL DROIT -----------------------------------------------------------------------------
        if (SectionKerato.contains("KR"))
        {
            mesureOD            = SectionKerato.mid(SectionKerato.indexOf("KR")+4,46)   .replace(" ","0");
            K1OD                = mesureOD.mid(0,5);
            K2OD                = mesureOD.mid(6,5);
            AxeKOD              = mesureOD.mid(31,3).toInt();
            Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
            Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
            Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
            Datas::I()->mesurekerato->setdioptriesK1OD(mesureOD.mid(12,5).toDouble());
            Datas::I()->mesurekerato->setdioptriesK2OD(mesureOD.mid(18,5).toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (SectionKerato.contains("KL"))
        {
            mesureOG            = SectionKerato.mid(SectionKerato.indexOf("CL")+4,46)   .replace(" ","0");
            K1OG                = mesureOG.mid(0,5);
            K2OG                = mesureOG.mid(6,5);
            AxeKOG              = mesureOG.mid(31,3).toInt();
            Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
            Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
            Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
            Datas::I()->mesurekerato->setdioptriesK1OG(mesureOG.mid(12,5).toDouble());
            Datas::I()->mesurekerato->setdioptriesK2OG(mesureOG.mid(18,5).toDouble());
        }
        if (Datas::I()->mesurekerato->isDifferent(oldMesureKerato) && !Datas::I()->mesurekerato->isdataclean())
            emit newmesure(MesureKerato);
        delete oldMesureKerato;
    }
    if (flag.testFlag(MesureAutoref))               //!=> il y a une mesure de refractometrie et l'autoref est directement branché sur la box du refracteur
    {
        MesureRefraction        *oldMesureAutoref = new MesureRefraction();
        oldMesureAutoref        ->setdatas(Datas::I()->mesureautoref);
        Datas::I()->mesureautoref->cleandatas();
        idx                     = Mesure.indexOf("RM");
        QString SectionAutoref  = Mesure.right(Mesure.length()-idx);
        idx                     = Mesure.indexOf("$");
        SectionAutoref          = Mesure.left(idx + 1);
        //Edit(SectionAutoref + "\nOK");
        PD                      = SectionAutoref.mid(SectionAutoref.indexOf("pB")+4,2);
        Datas::I()->mesureautoref->setecartIP(PD.toInt());
        // OEIL DROIT -----------------------------------------------------------------------------
        if (SectionAutoref.contains(" R"))
        {
            mesureOD     = SectionAutoref.mid(SectionAutoref.indexOf(" R")+4,17)   .replace(" ","0");
            mSphereOD    = mesureOD.mid(0,6);
            mCylOD       = mesureOD.mid(7,6);
            mAxeOD       = mesureOD.mid(14,3);
            Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (SectionAutoref.contains(" L"))
        {
            mesureOG     = SectionAutoref.mid(SectionAutoref.indexOf(" L")+4,17)   .replace(" ","0");
            mSphereOG    = mesureOG.mid(0,6);
            mCylOG       = mesureOG.mid(7,6);
            mAxeOG       = mesureOG.mid(14,3);
            Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
        }
        //debugMesureRefraction(Datas::I()->mesureautoref);
        if (Datas::I()->mesureautoref->isDifferent(oldMesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
            emit newmesure(MesureAutoref);
        delete oldMesureAutoref;
    }

    //! Données du REFRACTEUR --------------------------------------------------------------------------------------------------------------------
    if (flag.testFlag(MesureRefracteur))            //!=> il y a une mesure de refraction
    {
        idx                         = Mesure.indexOf("RT");
        QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);
        //qDebug() << "Procedures::LectureDonneesRefracteur(QString Mesure) - SectionRefracteur = " << SectionRefracteur;

        PD               = SectionRefracteur.mid(SectionRefracteur.indexOf("pB")+4,2);
        Datas::I()->mesureacuite->setecartIP(PD.toInt());
        // OEIL DROIT -----------------------------------------------------------------------------
        if (SectionRefracteur.contains(" R"))
        {
            mesureOD     = SectionRefracteur.mid(SectionRefracteur.indexOf(" R")+4,17)   .replace(" ","0");
            mSphereOD    = mesureOD.mid(0,6);
            mCylOD       = mesureOD.mid(7,6);
            mAxeOD       = mesureOD.mid(14,3);
            if (SectionRefracteur.indexOf("AR")>-1)
                mAddOD   = SectionRefracteur.mid(SectionRefracteur.indexOf("AR")+4,6)    .replace(" ","0");
            if (SectionRefracteur.indexOf("vR")>-1)
                AVLOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("vR")+4,5)    .replace(" ","0");
            if (SectionRefracteur.indexOf("vr")>-1)
                AVPOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("vr")+4,5)    .replace(" ","0");
            Datas::I()->mesureacuite->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesureacuite->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            Datas::I()->mesureacuite->setaddVPOD(mAddOD.toDouble());
            Datas::I()->mesureacuite->setavlOD(AVLOD);
            Datas::I()->mesureacuite->setavpOD(AVPOD);
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (SectionRefracteur.contains(" L"))
        {
            mesureOG     = SectionRefracteur.mid(SectionRefracteur.indexOf(" L")+4,17)   .replace(" ","0");
            mSphereOG    = mesureOG.mid(0,6);
            mCylOG       = mesureOG.mid(7,6);
            mAxeOG       = mesureOG.mid(14,3);
            if (SectionRefracteur.indexOf("AL")>-1)
                mAddOG   = SectionRefracteur.mid(SectionRefracteur.indexOf("AR")+4,6)    .replace(" ","0");
            if (SectionRefracteur.indexOf("vL")>-1)
                AVLOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("vL")+4,5)    .replace(" ","0");
            if (SectionRefracteur.indexOf("vl")>-1)
                AVPOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("vl")+4,5)    .replace(" ","0");
            Datas::I()->mesureacuite->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesureacuite->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            Datas::I()->mesureacuite->setaddVPOG(mAddOG.toDouble());
            Datas::I()->mesureacuite->setavlOG(AVLOG);
            Datas::I()->mesureacuite->setavpOG(AVPOG);
        }
    }
}

QByteArray ShinNippon::RegleRefracteurCOM(TypesMesures flag)
{
/*!
*
#PC
# R - 5.00 - 0.25 120
# L - 2.00 - 0.50 50
#AR 5.00
#AL - 2.25
#PR 10.20 243
#PL 0.60 308
#Pr 1.40 45
#Pl 2.80 45
#pB 64.0
#pb 58.5
$
#LM
# R - 1.93 - 1.09 120
# L - 2.04 - 0.75 0
#AR + 0.02 - 0.01
#AL - 0.03 + 0.02
#PR O 0.11 D 0.18
#PL O 0.03 D 1.15
#pB 64.5
$
#RM
# R - 5.13 - 1.05 95
# L - 3.98 - 2.08 180
#pB 68.0
#pb 59.5
$
@3dd4


Le code de début est * (SOH)
Le code de début de ligne est # (STX)
Le code de fin de section (LM, RM, KM ou RT) est $
Le code de fin est @ suivi de la somme de contrôle puis ¥n
Le code de saut de ligne du PC vers le DR-900 est « ¥r¥n » (-> 0xA5 r 0xA5 n), ou « ¥r » (-> 0xA5 r) ou « ¥n » (-> 0xA5 n)
Le code de saut de ligne après le code d'erreur doit être « ¥n » (-> 0xA5 n)

ERROR DETECTION CODE
It sends the checksum as the error detection code after sending all of the data (after "@").
Add up all of the data from the beginning ("*") to the end ("@"), and calculate the
checksum of 2byte. The calculated value is converted to the ASCII code, so it is 4byte. It
is considered that data transmission is completed by sending the linefeed code after
checksum.
*/

//(Question; In the total checksum, is it nesessary to incorporate all "#" and "¥r¥n"?

/*!
The auto phoropter does not understand the code from the beginning to the end if the
error detection code is not sent, the code is not matched or the linefeed code after the
error detection code is not sent.
E.g.) In case of *ABC...(midstream is omitted)..@
0x2a + 0x41 + 0x42 + 0x43 + ... + 0x40(@) = 0x12AB (checksum is 2byte)
After the terminal symbol ("@"), send "12AB¥n"(the part of checksum is
converted to the ASCIT code, so it is 4 byte.)


TRADUCTION
L'envoi de data au refracteur ShinNippon est suivi d'un code de détection d'erreur - ici 3dd4

Le PC envoie la somme de contrôle comme code de détection d'erreur après avoir envoyé toutes les données (après « @ » - 3dd4 dans l'exemple ci-dessus).
Additionnez toutes les données du début (« * ») à la fin (« @ ») et calculez la somme de contrôle de 2 octets.

La valeur calculée est convertie en code ASCII, soit 4 octets.
On considère que la transmission des données est terminée par l'envoi du code de saut de ligne après la somme de contrôle.

Le refracteur ne comprend pas le code du début à la fin
 si le code de détection d'erreur n'est pas envoyé,
 si le code ne correspond pas
 ou si le code de saut de ligne après le code de détection d'erreur n'est pas envoyé.

Par exemple, dans le cas de *ABC... (le milieu du code est omis)..@
0x2a + 0x41 + 0x42 + 0x43 + ... + 0x40(@) = 0x12AB (la somme de contrôle est de 2 octets)
Après le symbole terminal (« @ »), envoyez « 12AB¥n » (la partie de la somme de contrôle est convertie en code ASCIT, elle est donc de 4 octets).



Traduit avec DeepL.com (version gratuite)
*/


    QString AxeOD, AxeOG;
    QString AddOD, AddOG;
    QString SphereOD, SphereOG;
    QString CylindreOD, CylindreOG;
    QString SCAOD, SCAOG;
    QByteArray DTRbuff = QByteArray();
    auto initvariables = [&] ()
    {
        AxeOD  = "180";
        AxeOG  = "180";
        AddOD  = "+00.00";
        AddOG  = "+00.00";
        SphereOD  = "+00.00";
        SphereOG  = "+00.00";
        CylindreOD  = "+00.00";
        CylindreOG  = "+00.00";
    };
    auto convertdioptriesShinNippon = [&] (QString &finalvalue, double originvalue)
    {
        if (originvalue > 0)
            finalvalue = (originvalue < 10? "+ " : "+") + QString::number(originvalue,'f',2);
        else if (originvalue < 0)
            finalvalue = (originvalue > -10? QString::number(originvalue,'f',2).replace("-", "- ") : QString::number(originvalue,'f',2));
        else if (originvalue == 0)
            finalvalue = "  0.00";
    };
    auto addLFCRShinNippon = [&] (QByteArray &buff, bool shortlf = true)
    {
        if (!shortlf)
        {
            buff.append(YEN);
            buff.append(Utils::StringToArray("r"));
        }
        buff.append(YEN);
        buff.append(Utils::StringToArray("n"));
    };


    DTRbuff.append(SOH);                                    //SOH -> start of header

    /*! réglage de l'autoref */
    if (flag.testFlag(MesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
    {
        initvariables();
        convertdioptriesShinNippon(SphereOD, Datas::I()->mesureautoref->sphereOD());
        convertdioptriesShinNippon(SphereOG, Datas::I()->mesureautoref->sphereOG());
        convertdioptriesShinNippon(CylindreOD, Datas::I()->mesureautoref->cylindreOD());
        convertdioptriesShinNippon(CylindreOG, Datas::I()->mesureautoref->cylindreOG());

        SCAOD       = " " + SphereOD + " " + CylindreOD + " " + AxeOD;
        SCAOG       = " " + SphereOG + " " + CylindreOG + " " + AxeOG;

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray("RM"));         //section autoref
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray(" R"+ SCAOD));  //SD
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray(" L"+ SCAOG));  //SG
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        if (Datas::I()->mesureautoref->ecartIP() > 0)
        {
            DTRbuff.append(STX);                                //STX -> start of text
            DTRbuff.append(Utils::StringToArray("pB "+ QString::number(Datas::I()->mesureautoref->ecartIP(),'f',1)));  //SD
            addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon
        }
        DTRbuff.append(Utils::StringToArray("$"));
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon
    }

    /*! réglage du fronto */
    if (flag.testFlag(MesureFronto) && !Datas::I()->mesurefronto->isdataclean())
    {
        initvariables();

        convertdioptriesShinNippon(SphereOD,    Datas::I()->mesurefronto->sphereOD());
        convertdioptriesShinNippon(SphereOG,    Datas::I()->mesurefronto->sphereOG());
        convertdioptriesShinNippon(CylindreOD,  Datas::I()->mesurefronto->cylindreOD());
        convertdioptriesShinNippon(CylindreOG,  Datas::I()->mesurefronto->cylindreOG());
        convertdioptriesShinNippon(AddOD,       Datas::I()->mesureautoref->addVPOD());
        convertdioptriesShinNippon(AddOG,       Datas::I()->mesureautoref->addVPOG());

        SCAOD       = " " + SphereOD + " " + CylindreOD + " " + AxeOD;
        SCAOG       = " " + SphereOG + " " + CylindreOG + " " + AxeOG;
        AddOD       = " " + AddOD;
        AddOG       = " " + AddOG;

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray("LM"));         //section fronto
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray(" R"+ SCAOD));  //SD
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray(" L"+ SCAOG));  //SG
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray("AR"+ AddOD));  //SD
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray("AL"+ AddOG));  //SG
        addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

        if (Datas::I()->mesurefronto->ecartIP() > 0)
        {
            DTRbuff.append(Utils::StringToArray("pB "+ QString::number(Datas::I()->mesurefronto->ecartIP(),'f',1)));   //SD
            addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon
        }
    }
    DTRbuff.append(Utils::StringToArray("$"));
    addLFCRShinNippon(DTRbuff);                         //Linefeed + carriage return ShinNippon

    /*!
        qDebug() << "RegleRefracteur() - DTRBuff = " << QString(DTRbuff).toLocal8Bit() << "RegleRefracteur() - DTRBuff.size() = " << QString(DTRbuff).toLocal8Bit().size();
        QString nompat = "";
        Patient *pat = Datas::I()->patients->currentpatient();
        if (pat)
            nompat = pat->prenom() + " " + pat->nom().toUpper();

        Logs::LogToFile("PortSerieRefracteur.txt", "Datas = " + QString(DTRbuff).toLocal8Bit() + " - "
                        + QDateTime().toString("dd-MM-yyyy HH:mm:ss")
                        + (nompat != ""? " - " : "") + nompat);
        QFile com("/Users/serge/Desktop/" + nompat + ".txt");
        if (com.exists())
            com.remove();
        QByteArray ba;
        if (com.open(QIODevice::ReadWrite))
        {
            QTextStream out(&com);
            out << QString(DTRbuff).toLocal8Bit();
            com.close();
        }
        if (com.open(QIODevice::ReadOnly))
        {
            ba = com.readAll();
            com.close();
        }
        qDebug() << ba;
        //*/
    //qDebug() << DTRbuff;
    return DTRbuff;
}

