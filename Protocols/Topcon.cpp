#include "Topcon.h"

Topcon* Topcon::instance = Q_NULLPTR;

Topcon* Topcon::I()
{
    if( !instance )
        instance = new Topcon();
    return instance;
}

Topcon::Topcon() {}


void Topcon::LectureDonneesXMLRefracteur(QDomDocument docxml, QString nameRF)
{
    if( nameRF == "TOPCON CV-5000")
    {
        LectureDonneesXMLnsSBJ(docxml);
    }
}


void Topcon::LectureDonneesXMLnsCommon(QDomDocument docxml)
{
/*
 * Sample namespace Common from Topcon CV5000PC
 *
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<Ophthalmology xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:nsCommon="http://www.Topcon.or.jp/standardized/namespaces/Common" xmlns:nsSBJ="http://www.Topcon.or.jp/standardized/namespaces/SBJ" xsi:schemaLocation="http://www.Topcon.or.jp/standardized/namespaces/Common Common_schema.xsd http://www.Topcon.or.jp/standardized/namespaces/SBJ SBJ_schema.xsd">
    <nsCommon:Common>
        <nsCommon:Company>TOPCON</nsCommon:Company>
        <nsCommon:ModelName>CV-5000</nsCommon:ModelName>
        <nsCommon:MachineNo>192168118</nsCommon:MachineNo>
        <nsCommon:ROMVersion>2.21.00</nsCommon:ROMVersion>
        <nsCommon:Version>1.2</nsCommon:Version>
        <nsCommon:Date>2024-05-11</nsCommon:Date>
        <nsCommon:Time>20:52:16.712</nsCommon:Time>
        <nsCommon:Patient>
            <nsCommon:No.>12</nsCommon:No.>
            <nsCommon:ID>12</nsCommon:ID>
            <nsCommon:FirstName/>
            <nsCommon:MiddleName/>
            <nsCommon:LastName/>
            <nsCommon:Sex/>
            <nsCommon:Age/>
            <nsCommon:DOB/>
            <nsCommon:NameJ1/>
            <nsCommon:NameJ2/>
        </nsCommon:Patient>
        <Duration>00:07:10</Duration>
        <DateFormat>D_M_Y</DateFormat>
    </nsCommon:Common>
</Ophthalmology>
*/

}

void Topcon::LectureDonneesXMLnsSBJ(QDomDocument docxml)
{
/*
 * Sample namespace SBJ from Topcon CV5000PC
 *
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<Ophthalmology xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:nsCommon="http://www.joia.or.jp/standardized/namespaces/Common" xmlns:nsSBJ="http://www.joia.or.jp/standardized/namespaces/SBJ" xsi:schemaLocation="http://www.joia.or.jp/standardized/namespaces/Common Common_schema.xsd http://www.joia.or.jp/standardized/namespaces/SBJ SBJ_schema.xsd">
    <nsCommon:Common>
        <nsCommon:Company>TOPCON</nsCommon:Company>
        <nsCommon:ModelName>CV-5000</nsCommon:ModelName>
        <nsCommon:MachineNo>192168118</nsCommon:MachineNo>
        <nsCommon:ROMVersion>2.21.00</nsCommon:ROMVersion>
        <nsCommon:Version>1.2</nsCommon:Version>
        <nsCommon:Date>2024-05-13</nsCommon:Date>
        <nsCommon:Time>19:48:59.562</nsCommon:Time>
        <nsCommon:Patient>
            <nsCommon:No.>13</nsCommon:No.>
            <nsCommon:ID>13</nsCommon:ID>
            <nsCommon:FirstName/>
            <nsCommon:MiddleName/>
            <nsCommon:LastName/>
            <nsCommon:Sex/>
            <nsCommon:Age/>
            <nsCommon:DOB/>
            <nsCommon:NameJ1/>
            <nsCommon:NameJ2/>
        </nsCommon:Patient>
        <Duration>00:03:12</Duration>
        <DateFormat>D_M_Y</DateFormat>
    </nsCommon:Common>
    <nsSBJ:Measure type="SBJ">
        <nsSBJ:RefractionTest>
            <nsSBJ:Type No="1">
                <nsSBJ:TypeName>Full Correction</nsSBJ:TypeName>
                <nsSBJ:ExamDistance No="1">
                    <nsSBJ:Distance unit="cm">500,000</nsSBJ:Distance>
                    <nsSBJ:RefractionData>
                        <nsSBJ:R>
                            <nsSBJ:Sph unit="D">0,25</nsSBJ:Sph>
                            <nsSBJ:Cyl unit="D">-1,50</nsSBJ:Cyl>
                            <nsSBJ:Axis unit="deg">180</nsSBJ:Axis>
                            <nsSBJ:HPri unit="prism"/>
                            <nsSBJ:HBase/>
                            <nsSBJ:VPri unit="prism"/>
                            <nsSBJ:VBase/>
                            <nsSBJ:Prism unit="prism"/>
                            <nsSBJ:Angle unit="deg"/>
                        </nsSBJ:R>
                        <nsSBJ:L>
                            <nsSBJ:Sph unit="D">-0,25</nsSBJ:Sph>
                            <nsSBJ:Cyl unit="D">0,00</nsSBJ:Cyl>
                            <nsSBJ:Axis unit="deg">180</nsSBJ:Axis>
                            <nsSBJ:HPri unit="prism"/>
                            <nsSBJ:HBase/>
                            <nsSBJ:VPri unit="prism"/>
                            <nsSBJ:VBase/>
                            <nsSBJ:Prism unit="prism"/>
                            <nsSBJ:Angle unit="deg"/>
                        </nsSBJ:L>
                        <nsSBJ:VD unit="mm">12,00</nsSBJ:VD>
                    </nsSBJ:RefractionData>
                    <nsSBJ:PD>
                        <nsSBJ:R unit="mm">32,00</nsSBJ:R>
                        <nsSBJ:L unit="mm">32,00</nsSBJ:L>
                        <nsSBJ:B unit="mm">64,00</nsSBJ:B>
                    </nsSBJ:PD>
                </nsSBJ:ExamDistance>
                <nsSBJ:ExamDistance No="2">
                    <nsSBJ:Distance unit="cm">67,000</nsSBJ:Distance>
                    <nsSBJ:RefractionData>
                        <nsSBJ:R>
                            <nsSBJ:Sph unit="D">3,25</nsSBJ:Sph>
                            <nsSBJ:Cyl unit="D">-1,50</nsSBJ:Cyl>
                            <nsSBJ:Axis unit="deg">180</nsSBJ:Axis>
                            <nsSBJ:HPri unit="prism"/>
                            <nsSBJ:HBase/>
                            <nsSBJ:VPri unit="prism"/>
                            <nsSBJ:VBase/>
                            <nsSBJ:Prism unit="prism"/>
                            <nsSBJ:Angle unit="deg"/>
                        </nsSBJ:R>
                        <nsSBJ:L>
                            <nsSBJ:Sph unit="D">2,25</nsSBJ:Sph>
                            <nsSBJ:Cyl unit="D">0,00</nsSBJ:Cyl>
                            <nsSBJ:Axis unit="deg">180</nsSBJ:Axis>
                            <nsSBJ:HPri unit="prism"/>
                            <nsSBJ:HBase/>
                            <nsSBJ:VPri unit="prism"/>
                            <nsSBJ:VBase/>
                            <nsSBJ:Prism unit="prism"/>
                            <nsSBJ:Angle unit="deg"/>
                        </nsSBJ:L>
                        <nsSBJ:VD unit="mm">12,00</nsSBJ:VD>
                    </nsSBJ:RefractionData>
                    <nsSBJ:PD>
                        <nsSBJ:R unit="mm">31,00</nsSBJ:R>
                        <nsSBJ:L unit="mm">31,00</nsSBJ:L>
                        <nsSBJ:B unit="mm">62,00</nsSBJ:B>
                    </nsSBJ:PD>
                </nsSBJ:ExamDistance>
            </nsSBJ:Type>
        </nsSBJ:RefractionTest>
    </nsSBJ:Measure>
</Ophthalmology>
*/


    Logs::LogToFile("MesuresRefracteur.txt", docxml.toByteArray());



/*
    QString PDD(""), PDG("");

 *  Not confirmed
    QString mAddOD      = "+0.00";
    QString mAddOG      = "+0.00";
    QString AVLOD(""), AVLOG("");

    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    QString mTOOD(""), mTOOG("");
*/

    // For debug purposes
    int num=0;

    QString nsURI("nsSBJ"); // Maybe Subjective?
    QDomElement xml = docxml.documentElement();


    // Find Type with TypeName tag = "Full Correction"
    QDomElement elemType;

    QList<QDomElement> typeNames = Utils::XMLgetElementsByTagnameNS(xml, nsURI, "TypeName" );
    num=typeNames.count();
    for(int i=0; i<num; i++) {
        QDomElement elemTypeName = typeNames.at(i);
        QString text = elemTypeName.text();
        if( text == "Full Correction"){
            QDomNode parentNode = elemTypeName.parentNode();
            if( parentNode.isElement()) {
                elemType = parentNode.toElement();
                break;
            }
        }
    }


    if(!elemType.isNull())
    {
        //variables for ADD calculation
        // vl = loin (far), vp = pres (near)
        double vlSphOD = 0.0;
        double vlSphOG = 0.0;
        double vpSphOD = 0.0;
        double vpSphOG = 0.0;
        bool readedVL = false;
        bool readedVP = false;

        // Get ExamDistances (VL et VP)
        QList<QDomElement> ExamDistances = Utils::XMLgetElementsByTagnameNS(xml, nsURI, "ExamDistance" );
        num=ExamDistances.count();
        for(int i=0; i<num; i++)
        {
            QString mode = ""; // empty, VL or VP
            QDomElement ExamDistance = ExamDistances.at(i);
            if( ! ExamDistance.isNull())
            {
                QDomElement distanceElement = Utils::XMLfirstElementByTagNameNS(ExamDistance, nsURI, "Distance");
                if(!distanceElement.isNull() )
                {
                    QString distanceUnits = "cm";
                    if(distanceElement.hasAttribute("unit"))
                    {
                        distanceUnits=distanceElement.attribute("unit");
                    }
                    QString distanceValue = distanceElement.text().replace(",",".");
                    double distance = distanceValue.toDouble();
                    if(distanceUnits == "mm") // I don't know if that happens, but...
                    {
                        distance /= 10; // convert to cm.
                    }
                    if (distance > 100) {
                        mode="VL";
                    } else {
                        mode="VP";
                    }
                }
            }
            if( mode == "VL")
            {
                // Get  RefractionData
                QDomElement RefractionData =  Utils::XMLfirstElementByTagNameNS(ExamDistance, nsURI, "RefractionData");
                if(!RefractionData.isNull())
                {
                    QString mSphereOD   = "+00.00";
                    QString mCylOD      = "+00.00";
                    QString mAxeOD      = "000";
                    QString mSphereOG   = "+00.00";
                    QString mCylOG      = "+00.00";
                    QString mAxeOG      = "000";


                    QDomElement R =  Utils::XMLfirstElementByTagNameNS(RefractionData, nsURI,"R");
                    if(!R.isNull())
                    {
                        mSphereOD = Utils::XMLfirstElementValueByTagNameNS(R, nsURI, "Sph");
                        mCylOD = Utils::XMLfirstElementValueByTagNameNS(R, nsURI, "Cyl");
                        mAxeOD = Utils::XMLfirstElementValueByTagNameNS(R, nsURI, "Axis");
                    }
                    QDomElement L =  Utils::XMLfirstElementByTagNameNS(RefractionData, nsURI,"L");
                    if(!L.isNull())
                    {
                        mSphereOG = Utils::XMLfirstElementValueByTagNameNS(L, nsURI, "Sph");
                        mCylOG = Utils::XMLfirstElementValueByTagNameNS(L, nsURI, "Cyl");
                        mAxeOG = Utils::XMLfirstElementValueByTagNameNS(L, nsURI, "Axis");
                    }
                    mSphereOD = mSphereOD.replace(",",".");
                    mSphereOG = mSphereOG.replace(",",".");

                    mCylOD = mCylOD.replace(",",".");
                    mCylOG = mCylOG.replace(",",".");

                    vlSphOD = mSphereOD.toDouble();
                    vlSphOG = mSphereOG.toDouble();
                    Datas::I()->mesureacuite->setsphereOD(vlSphOD);
                    Datas::I()->mesureacuite->setsphereOG(vlSphOG);

                    Datas::I()->mesureacuite->setcylindreOD(mCylOD.toDouble());
                    Datas::I()->mesureacuite->setcylindreOG(mCylOG.toDouble());

                    Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                    Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                    readedVL = true;
                }
                // PD
                QDomElement elemPD =  Utils::XMLfirstElementByTagNameNS(RefractionData, nsURI,"PD");
                if(!elemPD.isNull())
                {
                    //PDD = Utils::XMLfirstElementValueByTagNameNS(PDis, nsURI, "R");
                    //PDG = Utils::XMLfirstElementValueByTagNameNS(PDis, nsURI, "L");

                    QString PD          = "";
                    PD = Utils::XMLfirstElementValueByTagNameNS(elemPD, nsURI, "B");
                    PD = PD.replace(",",".");
                    Datas::I()->mesureacuite->setecartIP(static_cast<int>(std::round(PD.toDouble())));
                }

            }// if(mode == "VL")
            else if(mode == "VP")
            {
                // Get  RefractionData
                QDomElement RefractionData =  Utils::XMLfirstElementByTagNameNS(ExamDistance, nsURI, "RefractionData");
                if(!RefractionData.isNull())
                {
                    QString mSphereOD   = "+00.00";
                    QString mCylOD      = "+00.00";
                    QString mAxeOD      = "000";
                    QString mSphereOG   = "+00.00";
                    QString mCylOG      = "+00.00";
                    QString mAxeOG      = "000";

                    QDomElement R =  Utils::XMLfirstElementByTagNameNS(RefractionData, nsURI,"R");
                    if(!R.isNull())
                    {
                        mSphereOD = Utils::XMLfirstElementValueByTagNameNS(R, nsURI, "Sph");
                        mCylOD = Utils::XMLfirstElementValueByTagNameNS(R, nsURI, "Cyl");
                        mAxeOD = Utils::XMLfirstElementValueByTagNameNS(R, nsURI, "Axis");
                    }
                    QDomElement L =  Utils::XMLfirstElementByTagNameNS(RefractionData, nsURI,"L");
                    if(!L.isNull())
                    {
                        mSphereOG = Utils::XMLfirstElementValueByTagNameNS(L, nsURI, "Sph");
                        mCylOG = Utils::XMLfirstElementValueByTagNameNS(L, nsURI, "Cyl");
                        mAxeOG = Utils::XMLfirstElementValueByTagNameNS(L, nsURI, "Axis");
                    }

                    mSphereOD = mSphereOD.replace(",",".");
                    mCylOD = mCylOD.replace(",",".");
                    mSphereOG = mSphereOG.replace(",",".");
                    mCylOG = mCylOG.replace(",",".");

                    vpSphOD = mSphereOD.toDouble();
                    vpSphOG = mSphereOG.toDouble();

                    readedVP = true;

                }
                QDomElement AV =  Utils::XMLfirstElementByTagNameNS(RefractionData, nsURI,"VA");
                if(!AV.isNull())
                {
                    QString AVLOD(""), AVLOG("");
                    AVLOD = Utils::XMLfirstElementValueByTagNameNS(AV, nsURI, "R");
                    AVLOD = AVLOD.replace(",",".").replace(" ","0");
                    AVLOG = Utils::XMLfirstElementValueByTagNameNS(AV, nsURI, "L");
                    AVLOG = AVLOD.replace(",",".").replace(" ","0");

                    Datas::I()->mesureacuite->setavlOD(AVLOD);
                    Datas::I()->mesureacuite->setavlOG(AVLOG);
                }
                //<nsSBJ:VA>
                //<nsSBJ:R/>
                //<nsSBJ:L/>
                //<nsSBJ:B unit="Decimal"> 7.5</nsSBJ:B>
                //</nsSBJ:VA>

            } // if(mode == "VP")

        } // ExamDistances

        // ADD calculation if both readed
        if( readedVL and readedVP ){
            Datas::I()->mesureacuite->setaddVPOD(vpSphOD - vlSphOD);
            Datas::I()->mesureacuite->setaddVPOG(vpSphOG - vlSphOG);
        }
    }
/*
        Datas::I()->mesureacuite->setavlOD(AVLOD);
        Datas::I()->mesureacuite->setavlOG(AVLOG);
*/
}


/*
 * ******************************************************************************
 * ******************************************************************************
 *
 *
 *
 *   REGLE REFRACTEUR
 *
 *   Work in progress ...
 *
 *
 * ******************************************************************************
 * ******************************************************************************
 */




void Topcon::RegleRefracteurXML(int flg, QString nameRF)
{

    // I don't know how to pass Procedures::Types
    /*!
     *  I'd create in Protocols, a protocolgeneric.h class with TypeMesures and TypeMesure inside
     *  Then Tomey.h and Topcon.h inherits from this class
     *  and migrate definition of TypeMesures and TypeMesure from Procedures.h to ProtocolGeneric:.h
     *  and #include "protocolgeneric.h" in procedures.h
     */
    //TypesMesures flag(flg);

    /*! LE FRONTO */
    bool ExistMesureFronto =  !Datas::I()->mesurefronto->isdataclean();
    if (ExistMesureFronto)
    {
        QDomDocument LMxml("");
        QDomElement ophtalmology = LMxml.createElement("Ophtalmology");
        LMxml.appendChild(ophtalmology);
        {
            QDomElement Common = LMxml.createElement("Common");
            ophtalmology.appendChild(Common);
            {
                QDomElement company = LMxml.createElement("Company");
                Common.appendChild(company);
                company.appendChild(LMxml.createTextNode("RUFUS"));
                QDomElement model = LMxml.createElement("ModelName");
                Common.appendChild(model);
                model.appendChild(LMxml.createTextNode("Rufus"));
                Common.appendChild(LMxml.createElement("MachineNo"));
                Common.appendChild(LMxml.createElement("ROMVersion"));
                QDomElement version = LMxml.createElement("Version");
                Common.appendChild(version);
                version.appendChild(LMxml.createTextNode("Rufus"));
                QDomElement date = LMxml.createElement("Date");
                Common.appendChild(date);
                date.appendChild(LMxml.createTextNode(QDate::currentDate().toString(QObject::tr("yyyy/MM/dd"))));
                QDomElement time = LMxml.createElement("Time");
                Common.appendChild(time);
                time.appendChild(LMxml.createTextNode(QTime::currentTime().toString(QObject::tr("HH:mm:ss"))));
                QDomElement patient = LMxml.createElement("Patient");
                Common.appendChild(patient);
                {
                    QDomElement patientno = LMxml.createElement("No.");
                    patient.appendChild(patientno);
                    {
                        QDomText id = LMxml.createTextNode(Datas::I()->patients->currentpatient()? QString::number(Datas::I()->patients->currentpatient()->id()) : "0");
                        patientno.appendChild(id);
                    }
                    QDomElement patientid = LMxml.createElement("ID");
                    patient.appendChild(patientid);
                    {
                        QDomText id = LMxml.createTextNode(Datas::I()->patients->currentpatient()? QString::number(Datas::I()->patients->currentpatient()->id()) : "0");
                        patientid.appendChild(id);
                    }
                    patient.appendChild(LMxml.createElement("FirstName"));
                    patient.appendChild(LMxml.createElement("MiddleName"));
                    patient.appendChild(LMxml.createElement("LastName"));
                    patient.appendChild(LMxml.createElement("Sex"));
                    patient.appendChild(LMxml.createElement("Age"));
                    patient.appendChild(LMxml.createElement("DOB"));
                    patient.appendChild(LMxml.createElement("NameJ1"));
                    patient.appendChild(LMxml.createElement("NameJ2"));
                }
                QDomElement oper = LMxml.createElement("Operator");
                Common.appendChild(oper);
                {
                    oper.appendChild(LMxml.createElement("No."));
                    oper.appendChild(LMxml.createElement("ID"));
                }
            }
            QDomElement Measure = LMxml.createElement("Measure");
            Measure.setAttribute("Type","LM");
            ophtalmology.appendChild(Measure);
            {
                Measure.appendChild(LMxml.createElement("MeasureMode"));
                Measure.appendChild(LMxml.createElement("DiopterStep"));
                Measure.appendChild(LMxml.createElement("AxisStep"));
                Measure.appendChild(LMxml.createElement("CylinderMode"));
                Measure.appendChild(LMxml.createElement("PrismDiopterStep"));
                Measure.appendChild(LMxml.createElement("PrismBaseStep"));
                Measure.appendChild(LMxml.createElement("PrismMode"));
                Measure.appendChild(LMxml.createElement("AddMode"));
                QDomElement lm = LMxml.createElement("LM");
                Measure.appendChild(lm);
                {
                    QDomElement s = LMxml.createElement("S");
                    lm.appendChild(s);
                    {
                        s.appendChild(LMxml.createElement("Sphere"));
                        s.appendChild(LMxml.createElement("Cylinder"));
                        s.appendChild(LMxml.createElement("Axis"));
                        s.appendChild(LMxml.createElement("SE"));
                        s.appendChild(LMxml.createElement("ADD"));
                        s.appendChild(LMxml.createElement("ADD2"));
                        s.appendChild(LMxml.createElement("NearSphere"));
                        s.appendChild(LMxml.createElement("NearSPhere2"));
                        s.appendChild(LMxml.createElement("Prism"));
                        s.appendChild(LMxml.createElement("PrismBase"));
                        s.appendChild(LMxml.createElement("PrismX"));
                        s.appendChild(LMxml.createElement("PrismY"));
                        s.appendChild(LMxml.createElement("Prism"));
                        s.appendChild(LMxml.createElement("UVTransmittance"));
                        s.appendChild(LMxml.createElement("ConfidenceIndex"));
                    }
                    QDomElement r = LMxml.createElement("R");
                    lm.appendChild(r);
                    {
                        QDomElement sph = LMxml.createElement("Sphere");
                        sph.setAttribute("unit","D");
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->sphereOD(),'f',2));
                            sph.appendChild(val);
                        }
                        r.appendChild(sph);
                        QDomElement cyl = LMxml.createElement("Cylinder");
                        cyl.setAttribute("unit","D");
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->cylindreOD(),'f',2));
                            cyl.appendChild(val);
                        }
                        r.appendChild(cyl);
                        QDomElement ax = LMxml.createElement("Axis");
                        ax.setAttribute("unit","deg");
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->axecylindreOD()));
                            ax.appendChild(val);
                        }
                        r.appendChild(ax);
                        r.appendChild(LMxml.createElement("SE"));
                        QDomElement add = LMxml.createElement("ADD");
                        r.appendChild(add);
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2));
                            add.appendChild(val);
                        }
                        r.appendChild(add);
                        r.appendChild(LMxml.createElement("ADD2"));
                        r.appendChild(LMxml.createElement("NearSphere"));
                        r.appendChild(LMxml.createElement("NearSPhere2"));
                        r.appendChild(LMxml.createElement("Prism"));
                        r.appendChild(LMxml.createElement("PrismBase"));
                        r.appendChild(LMxml.createElement("PrismX"));
                        r.appendChild(LMxml.createElement("PrismY"));
                        r.appendChild(LMxml.createElement("Prism"));
                        r.appendChild(LMxml.createElement("UVTransmittance"));
                        r.appendChild(LMxml.createElement("ConfidenceIndex"));
                    }
                    QDomElement l = LMxml.createElement("L");
                    lm.appendChild(l);
                    {
                        QDomElement sph = LMxml.createElement("Sphere");
                        sph.setAttribute("unit","D");
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->sphereOG(),'f',2));
                            sph.appendChild(val);
                        }
                        l.appendChild(sph);
                        QDomElement cyl = LMxml.createElement("Cylinder");
                        cyl.setAttribute("unit","D");
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->cylindreOG(),'f',2));
                            cyl.appendChild(val);
                        }
                        l.appendChild(cyl);
                        QDomElement ax = LMxml.createElement("Axis");
                        ax.setAttribute("unit","deg");
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->axecylindreOG()));
                            ax.appendChild(val);
                        }
                        l.appendChild(ax);
                        l.appendChild(LMxml.createElement("SE"));
                        QDomElement add = LMxml.createElement("ADD");
                        l.appendChild(add);
                        {
                            QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2));
                            add.appendChild(val);
                        }
                        l.appendChild(add);
                        l.appendChild(LMxml.createElement("ADD2"));
                        l.appendChild(LMxml.createElement("NearSphere"));
                        l.appendChild(LMxml.createElement("NearSPhere2"));
                        l.appendChild(LMxml.createElement("Prism"));
                        l.appendChild(LMxml.createElement("PrismBase"));
                        l.appendChild(LMxml.createElement("PrismX"));
                        l.appendChild(LMxml.createElement("PrismY"));
                        l.appendChild(LMxml.createElement("Prism"));
                        l.appendChild(LMxml.createElement("UVTransmittance"));
                        l.appendChild(LMxml.createElement("ConfidenceIndex"));
                    }
                }
                QDomElement pd = LMxml.createElement("PD");
                Measure.appendChild(pd);
                {
                    QDomElement pdb = LMxml.createElement("Distance");
                    pdb.setAttribute("Unit","mm");
                    if (Datas::I()->mesurefronto->ecartIP()>0)
                    {
                        QDomText val = LMxml.createTextNode(QString::number(Datas::I()->mesurefronto->ecartIP()));
                        pdb.appendChild(val);
                    }
                    pd.appendChild(pdb);
                    QDomElement pdr = LMxml.createElement("DistanceR");
                    pdr.setAttribute("Unit","mm");
                    pd.appendChild(pdr);
                    QDomElement pdl = LMxml.createElement("DistanceR");
                    pdl.setAttribute("Unit","mm");
                    pd.appendChild(pdl);
                }
                QDomElement nidek = LMxml.createElement("NIDEK");
                Measure.appendChild(nidek);
                {
                    QDomElement s = LMxml.createElement("S");
                    nidek.appendChild(s);
                    {
                        QDomElement length = LMxml.createElement("Length");
                        length.setAttribute("unit","mm");
                        s.appendChild(length);
                        QDomElement chwidth = LMxml.createElement("Length");
                        chwidth.setAttribute("unit","mm");
                        s.appendChild(chwidth);
                        QDomElement chlength = LMxml.createElement("Length");
                        chlength.setAttribute("unit","mm");
                        s.appendChild(chlength);
                        s.appendChild(LMxml.createElement("Index"));
                        QDomElement gtrs = LMxml.createElement("GreenTransmittance");
                        gtrs.setAttribute("unit","%");
                        s.appendChild(gtrs);
                    }
                    QDomElement r = LMxml.createElement("R");
                    nidek.appendChild(r);
                    {
                        QDomElement length = LMxml.createElement("Length");
                        length.setAttribute("unit","mm");
                        r.appendChild(length);
                        QDomElement chwidth = LMxml.createElement("Length");
                        chwidth.setAttribute("unit","mm");
                        r.appendChild(chwidth);
                        QDomElement chlength = LMxml.createElement("Length");
                        chlength.setAttribute("unit","mm");
                        r.appendChild(chlength);
                        r.appendChild(LMxml.createElement("Index"));
                        QDomElement gtrs = LMxml.createElement("GreenTransmittance");
                        gtrs.setAttribute("unit","%");
                        r.appendChild(gtrs);
                    }
                    QDomElement l = LMxml.createElement("L");
                    nidek.appendChild(l);
                    {
                        QDomElement length = LMxml.createElement("Length");
                        length.setAttribute("unit","mm");
                        l.appendChild(length);
                        QDomElement chwidth = LMxml.createElement("Length");
                        chwidth.setAttribute("unit","mm");
                        l.appendChild(chwidth);
                        QDomElement chlength = LMxml.createElement("Length");
                        chlength.setAttribute("unit","mm");
                        l.appendChild(chlength);
                        l.appendChild(LMxml.createElement("Index"));
                        QDomElement gtrs = LMxml.createElement("GreenTransmittance");
                        gtrs.setAttribute("unit","%");
                        l.appendChild(gtrs);
                    }
                    QDomElement netprism = LMxml.createElement("NetPrism");
                    nidek.appendChild(netprism);
                    {
                        QDomElement neth = LMxml.createElement("NetHPrism");
                        neth.setAttribute("unit","pri");
                        neth.setAttribute("base","in");
                        netprism.appendChild(neth);
                        QDomElement netv = LMxml.createElement("NetVPrism");
                        netv.setAttribute("unit","pri");
                        netv.setAttribute("base","out");
                        netprism.appendChild(netv);
                    }
                    QDomElement inside = LMxml.createElement("Inside");
                    nidek.appendChild(netprism);
                    {
                        QDomElement r = LMxml.createElement("InsideR");
                        r.setAttribute("unit","mm");
                        inside.appendChild(r);
                        QDomElement l = LMxml.createElement("InsideL");
                        l.setAttribute("unit","mm");
                        inside.appendChild(l);
                    }
                }
            }
        }
        //EnregistreFileDatasXML(LMxml, MesureFronto);
    }

    /*! L'AUTOREF */
    bool ExistMesureAutoref = !Datas::I()->mesureautoref->isdataclean();
    if (ExistMesureAutoref)
    {
        QDomDocument ARxml("");
        QDomElement Data = ARxml.createElement("Data");
        ARxml.appendChild(Data);
        QDomElement company = ARxml.createElement("Company");
        Data.appendChild(company);
        {
            QDomText t = ARxml.createTextNode("NIDEK");
            company.appendChild(t);
        }
        QDomElement model = ARxml.createElement("ModelName");
        Data.appendChild(model);
        {
            QDomText u = ARxml.createTextNode("ARK-1s");
            model.appendChild(u);
        }
        QDomElement rom = ARxml.createElement("ROMVersion");
        Data.appendChild(rom);
        QDomElement date = ARxml.createElement("Date");
        Data.appendChild(date);
        {
            QDomText w = ARxml.createTextNode(QDate::currentDate().toString(QObject::tr("yyyy/MM/dd")));
            date.appendChild(w);
        }
        QDomElement time = ARxml.createElement("Time");
        Data.appendChild(time);
        {
            QDomText x = ARxml.createTextNode(QTime::currentTime().toString(QObject::tr("HH:mm:ss")));
            time.appendChild(x);
        }
        QDomElement patient = ARxml.createElement("Patient");
        Data.appendChild(patient);
        {
            QDomElement patientno = ARxml.createElement("No.");
            patient.appendChild(patientno);
            {
                QDomText id = ARxml.createTextNode(Datas::I()->patients->currentpatient()? QString::number(Datas::I()->patients->currentpatient()->id()) : "0");
                patientno.appendChild(id);
            }
            QDomElement patientid = ARxml.createElement("ID");
            patient.appendChild(patientid);
        }
        QDomElement comment = ARxml.createElement("Comment");
        Data.appendChild(comment);
        QDomElement vd = ARxml.createElement("VD");
        Data.appendChild(vd);
        QDomElement wd = ARxml.createElement("WorkingDistance");
        Data.appendChild(wd);
        QDomElement ds = ARxml.createElement("DiopterStep");
        Data.appendChild(ds);
        QDomElement as = ARxml.createElement("AxisStep");
        Data.appendChild(as);
        QDomElement cm = ARxml.createElement("CylinderMode");
        Data.appendChild(cm);
        QDomElement ri = ARxml.createElement("RefractiveIndex");
        Data.appendChild(ri);
        QDomElement r = ARxml.createElement("R");
        Data.appendChild(r);
        {
            QDomElement ar = ARxml.createElement("AR");
            r.appendChild(ar);
            {
                QDomElement arlist = ARxml.createElement("ARList");
                ar.appendChild(arlist);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    arlist.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    arlist.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    arlist.appendChild(ax);
                    QDomElement cm = ARxml.createElement("CataractMode");
                    arlist.appendChild(cm);
                    QDomElement ci = ARxml.createElement("ConfidenceIndex");
                    arlist.appendChild(ci);
                    QDomElement se = ARxml.createElement("SE");
                    arlist.appendChild(se);
                }
                QDomElement armed = ARxml.createElement("ARMedian");
                ar.appendChild(armed);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    armed.appendChild(sph);
                    {
                        QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->sphereOD(),'f',2));
                        sph.appendChild(val);
                    }
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    armed.appendChild(cyl);
                    {
                        QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->cylindreOD(),'f',2));
                        cyl.appendChild(val);
                    }
                    QDomElement ax = ARxml.createElement("Axis");
                    armed.appendChild(ax);
                    {
                        QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->axecylindreOD()));
                        ax.appendChild(val);
                    }
                    QDomElement se = ARxml.createElement("SE");
                    armed.appendChild(se);
                }
                QDomElement tl = ARxml.createElement("TrialLens");
                ar.appendChild(tl);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    tl.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    tl.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    tl.appendChild(ax);
                }
                QDomElement cl = ARxml.createElement("ContactLens");
                ar.appendChild(cl);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    cl.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    cl.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    cl.appendChild(ax);
                    QDomElement se = ARxml.createElement("SE");
                    cl.appendChild(se);
                }
                QDomElement img = ARxml.createElement("RingImage");
                ar.appendChild(img);

            }
            QDomElement va = ARxml.createElement("VA");
            r.appendChild(va);
            {
                QDomElement ucva = ARxml.createElement("UCVA");
                va.appendChild(ucva);
                QDomElement bcva = ARxml.createElement("BCVA");
                va.appendChild(bcva);
                QDomElement lva = ARxml.createElement("LVA");
                va.appendChild(lva);
                QDomElement gva = ARxml.createElement("GVA");
                va.appendChild(gva);
                QDomElement nva = ARxml.createElement("NVA");
                va.appendChild(nva);
                QDomElement wd = ARxml.createElement("WorkingDistance");
                va.appendChild(wd);
            }
            QDomElement sr = ARxml.createElement("SR");
            r.appendChild(sr);
            {
                QDomElement sph = ARxml.createElement("Sphere");
                sr.appendChild(sph);
                QDomElement cyl = ARxml.createElement("Cylinder");
                sr.appendChild(cyl);
                QDomElement ax = ARxml.createElement("Axis");
                sr.appendChild(ax);
                QDomElement se = ARxml.createElement("SE");
                sr.appendChild(se);
                QDomElement add = ARxml.createElement("ADD");
                sr.appendChild(add);
                QDomElement wd = ARxml.createElement("WorkingDistance");
                sr.appendChild(wd);
            }
            QDomElement lm = ARxml.createElement("LM");
            r.appendChild(lm);
            {
                lm.appendChild(ARxml.createElement("Sphere"));
                lm.appendChild(ARxml.createElement("Cylinder"));
                lm.appendChild(ARxml.createElement("Axis"));
                lm.appendChild(ARxml.createElement("ADD"));
                lm.appendChild(ARxml.createElement("ADD2"));
            }
            QDomElement km = ARxml.createElement("KM");
            r.appendChild(km);
            {
                QDomElement kmlist = ARxml.createElement("KMList");
                kmlist.setAttribute("No","1");
                km.appendChild(kmlist);
                {
                    QDomElement r1 = ARxml.createElement("R1");
                    kmlist.appendChild(r1);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r1.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r1.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r1.appendChild(ax);
                    }
                    QDomElement r2 = ARxml.createElement("R2");
                    kmlist.appendChild(r2);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r2.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r2.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r2.appendChild(ax);
                    }
                    QDomElement avg = ARxml.createElement("Average");
                    kmlist.appendChild(avg);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        avg.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        avg.appendChild(pwr);
                    }
                    QDomElement kmc = ARxml.createElement("KMCylinder");
                    kmlist.appendChild(kmc);
                    {
                        QDomElement pwr = ARxml.createElement("Power");
                        kmc.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        kmc.appendChild(ax);
                    }
                }
                QDomElement kmmed = ARxml.createElement("KMMedian");
                km.appendChild(kmmed);
                {
                    QDomElement r1 = ARxml.createElement("R1");
                    kmmed.appendChild(r1);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r1.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r1.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r1.appendChild(ax);
                    }
                    QDomElement r2 = ARxml.createElement("R2");
                    kmmed.appendChild(r2);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r2.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r2.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r2.appendChild(ax);
                    }
                    QDomElement avg = ARxml.createElement("Average");
                    kmmed.appendChild(avg);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        avg.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        avg.appendChild(pwr);
                    }
                    QDomElement kmc = ARxml.createElement("KMCylinder");
                    kmmed.appendChild(kmc);
                    {
                        QDomElement pwr = ARxml.createElement("Power");
                        kmc.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        kmc.appendChild(ax);
                    }
                }
            }
            QDomElement cs = ARxml.createElement("CS");
            r.appendChild(cs);
            {
                QDomElement cslist = ARxml.createElement("CSList");
                cslist.setAttribute("No","1");
                cs.appendChild(cslist);
                {
                    QDomElement sz = ARxml.createElement("Size");
                    cslist.appendChild(sz);
                }
            }
            QDomElement ps = ARxml.createElement("PS");
            r.appendChild(ps);
            {
                QDomElement pslist = ARxml.createElement("PSList");
                pslist.setAttribute("No","1");
                ps.appendChild(pslist);
                {
                    QDomElement sz = ARxml.createElement("Size");
                    pslist.appendChild(sz);
                    QDomElement lp = ARxml.createElement("Lamp");
                    pslist.appendChild(lp);
                }
            }
            QDomElement ac = ARxml.createElement("AC");
            r.appendChild(ac);
            {
                QDomElement sph = ARxml.createElement("Sphere");
                ac.appendChild(sph);
                QDomElement maxps = ARxml.createElement("MaxPS");
                ac.appendChild(maxps);
                QDomElement minps = ARxml.createElement("MinPS");
                ac.appendChild(minps);
                QDomElement img = ARxml.createElement("AccImage");
                ac.appendChild(img);
            }
            QDomElement ri = ARxml.createElement("RI");
            r.appendChild(ri);
            {
                QDomElement coih = ARxml.createElement("COIH");
                ri.appendChild(coih);
                QDomElement coia = ARxml.createElement("COIA");
                ri.appendChild(coia);
                QDomElement poi = ARxml.createElement("POI");
                ri.appendChild(poi);
                QDomElement rimg = ARxml.createElement("RetroImage");
                ri.appendChild(rimg);
            }
        }
        QDomElement l = ARxml.createElement("L");
        Data.appendChild(l);
        {
            QDomElement ar = ARxml.createElement("AR");
            l.appendChild(ar);
            {
                QDomElement arlist = ARxml.createElement("ARList");
                ar.appendChild(arlist);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    arlist.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    arlist.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    arlist.appendChild(ax);
                    QDomElement cm = ARxml.createElement("CataractMode");
                    arlist.appendChild(cm);
                    QDomElement ci = ARxml.createElement("ConfidenceIndex");
                    arlist.appendChild(ci);
                    QDomElement se = ARxml.createElement("SE");
                    arlist.appendChild(se);
                }
                QDomElement armed = ARxml.createElement("ARMedian");
                ar.appendChild(armed);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    armed.appendChild(sph);
                    {
                        QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->sphereOG(),'f',2));
                        sph.appendChild(val);
                    }
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    armed.appendChild(cyl);
                    {
                        QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->cylindreOG(),'f',2));
                        cyl.appendChild(val);
                    }
                    QDomElement ax = ARxml.createElement("Axis");
                    armed.appendChild(ax);
                    {
                        QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->axecylindreOG()));
                        ax.appendChild(val);
                    }
                    QDomElement se = ARxml.createElement("SE");
                    armed.appendChild(se);
                }
                QDomElement tl = ARxml.createElement("TrialLens");
                ar.appendChild(tl);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    tl.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    tl.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    tl.appendChild(ax);
                }
                QDomElement cl = ARxml.createElement("ContactLens");
                ar.appendChild(cl);
                {
                    QDomElement sph = ARxml.createElement("Sphere");
                    cl.appendChild(sph);
                    QDomElement cyl = ARxml.createElement("Cylinder");
                    cl.appendChild(cyl);
                    QDomElement ax = ARxml.createElement("Axis");
                    cl.appendChild(ax);
                    QDomElement se = ARxml.createElement("SE");
                    cl.appendChild(se);
                }
                QDomElement img = ARxml.createElement("RingImage");
                ar.appendChild(img);

            }
            QDomElement va = ARxml.createElement("VA");
            l.appendChild(va);
            {
                QDomElement ucva = ARxml.createElement("UCVA");
                va.appendChild(ucva);
                QDomElement bcva = ARxml.createElement("BCVA");
                va.appendChild(bcva);
                QDomElement lva = ARxml.createElement("LVA");
                va.appendChild(lva);
                QDomElement gva = ARxml.createElement("GVA");
                va.appendChild(gva);
                QDomElement nva = ARxml.createElement("NVA");
                va.appendChild(nva);
                QDomElement wd = ARxml.createElement("WorkingDistance");
                va.appendChild(wd);
            }
            QDomElement sr = ARxml.createElement("SR");
            l.appendChild(sr);
            {
                QDomElement sph = ARxml.createElement("Sphere");
                sr.appendChild(sph);
                QDomElement cyl = ARxml.createElement("Cylinder");
                sr.appendChild(cyl);
                QDomElement ax = ARxml.createElement("Axis");
                sr.appendChild(ax);
                QDomElement se = ARxml.createElement("SE");
                sr.appendChild(se);
                QDomElement add = ARxml.createElement("ADD");
                sr.appendChild(add);
                QDomElement wd = ARxml.createElement("WorkingDistance");
                sr.appendChild(wd);
            }
            QDomElement lm = ARxml.createElement("LM");
            l.appendChild(lm);
            {
                lm.appendChild(ARxml.createElement("Sphere"));
                lm.appendChild(ARxml.createElement("Cylinder"));
                lm.appendChild(ARxml.createElement("Axis"));
                lm.appendChild(ARxml.createElement("ADD"));
                lm.appendChild(ARxml.createElement("ADD2"));
            }
            QDomElement km = ARxml.createElement("KM");
            l.appendChild(km);
            {
                QDomElement kmlist = ARxml.createElement("KMList");
                kmlist.setAttribute("No","1");
                km.appendChild(kmlist);
                {
                    QDomElement r1 = ARxml.createElement("R1");
                    kmlist.appendChild(r1);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r1.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r1.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r1.appendChild(ax);
                    }
                    QDomElement r2 = ARxml.createElement("R2");
                    kmlist.appendChild(r2);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r2.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r2.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r2.appendChild(ax);
                    }
                    QDomElement avg = ARxml.createElement("Average");
                    kmlist.appendChild(avg);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        avg.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        avg.appendChild(pwr);
                    }
                    QDomElement kmc = ARxml.createElement("KMCylinder");
                    kmlist.appendChild(kmc);
                    {
                        QDomElement pwr = ARxml.createElement("Power");
                        kmc.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        kmc.appendChild(ax);
                    }
                }
                QDomElement kmmed = ARxml.createElement("KMMedian");
                km.appendChild(kmmed);
                {
                    QDomElement r1 = ARxml.createElement("R1");
                    kmmed.appendChild(r1);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r1.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r1.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r1.appendChild(ax);
                    }
                    QDomElement r2 = ARxml.createElement("R2");
                    kmmed.appendChild(r2);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        r2.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        r2.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        r2.appendChild(ax);
                    }
                    QDomElement avg = ARxml.createElement("Average");
                    kmmed.appendChild(avg);
                    {
                        QDomElement rd = ARxml.createElement("Radius");
                        avg.appendChild(rd);
                        QDomElement pwr = ARxml.createElement("Power");
                        avg.appendChild(pwr);
                    }
                    QDomElement kmc = ARxml.createElement("KMCylinder");
                    kmmed.appendChild(kmc);
                    {
                        QDomElement pwr = ARxml.createElement("Power");
                        kmc.appendChild(pwr);
                        QDomElement ax = ARxml.createElement("Axis");
                        kmc.appendChild(ax);
                    }
                }
            }
            QDomElement cs = ARxml.createElement("CS");
            l.appendChild(cs);
            {
                QDomElement cslist = ARxml.createElement("CSList");
                cslist.setAttribute("No","1");
                cs.appendChild(cslist);
                {
                    QDomElement sz = ARxml.createElement("Size");
                    cslist.appendChild(sz);
                }
            }
            QDomElement ps = ARxml.createElement("PS");
            l.appendChild(ps);
            {
                QDomElement pslist = ARxml.createElement("PSList");
                pslist.setAttribute("No","1");
                ps.appendChild(pslist);
                {
                    QDomElement sz = ARxml.createElement("Size");
                    pslist.appendChild(sz);
                    QDomElement lp = ARxml.createElement("Lamp");
                    pslist.appendChild(lp);
                }
            }
            QDomElement ac = ARxml.createElement("AC");
            l.appendChild(ac);
            {
                QDomElement sph = ARxml.createElement("Sphere");
                ac.appendChild(sph);
                QDomElement maxps = ARxml.createElement("MaxPS");
                ac.appendChild(maxps);
                QDomElement minps = ARxml.createElement("MinPS");
                ac.appendChild(minps);
                QDomElement img = ARxml.createElement("AccImage");
                ac.appendChild(img);
            }
            QDomElement ri = ARxml.createElement("RI");
            l.appendChild(ri);
            {
                QDomElement coih = ARxml.createElement("COIH");
                ri.appendChild(coih);
                QDomElement coia = ARxml.createElement("COIA");
                ri.appendChild(coia);
                QDomElement poi = ARxml.createElement("POI");
                ri.appendChild(poi);
                QDomElement rimg = ARxml.createElement("RetroImage");
                ri.appendChild(rimg);
            }
        }
        QDomElement pd = ARxml.createElement("PD");
        Data.appendChild(pd);
        {
            QDomElement pdl = ARxml.createElement("PDList");
            pdl.setAttribute("No","1");
            pd.appendChild(pdl);
            {
                QDomElement fpd = ARxml.createElement("FarPD");
                pdl.appendChild(fpd);
                if (Datas::I()->mesurefronto->ecartIP()>0)
                {
                    QDomText val = ARxml.createTextNode(QString::number(Datas::I()->mesureautoref->ecartIP()));
                    fpd.appendChild(val);
                }
                QDomElement rpd = ARxml.createElement("RPD");
                pdl.appendChild(rpd);
                QDomElement lpd = ARxml.createElement("LPD");
                pdl.appendChild(lpd);
                QDomElement npd = ARxml.createElement("NearPD");
                pdl.appendChild(npd);
            }
        }
    }
}
