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

//
// Polymorphism QDomDocument  QDomElement + 2 QString
//
QDomElement CreateNode(QDomDocument doc, QDomElement parent, QString nsURI, QString name)
{
    QDomElement child = doc.createElement(nsURI+ ":" + name);
    parent.appendChild(child);
    return child;
}

//
// Polymorphism QDomDocument  QDomElement + 3 QString (add Text Node with Value)
//
QDomElement CreateNode(QDomDocument doc, QDomElement parent, QString nsURI, QString name, QString value)
{
    QDomElement child =CreateNode(doc, parent, nsURI, name);
    if( value != "")
    {
        child.appendChild(doc.createTextNode(value));
    }
    return child;
}

//
// Polymorphism QDomDocument  QDomElement + 4 QString (add atttibute) + 1 QString optional value
//
QDomElement CreateNode(QDomDocument doc, QDomElement parent, QString nsURI, QString attribute , QString valueAtt, QString name , QString value = "")
{
    QDomElement child =CreateNode(doc, parent, nsURI, name, value);
    if( attribute != "" && valueAtt != "")
    {
        child.setAttribute(attribute, valueAtt);
    }
    return child;
}


void AddCommon(QDomDocument LMxml, QDomElement ophtalmology)
{
/*
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
*/
    QString nsURI="nsCommon";
    QString value = ""; // For convenience

    QDomElement elemCommon = CreateNode(LMxml, ophtalmology, nsURI, "Common");

    CreateNode(LMxml, elemCommon, nsURI, "Company", "RUFUS");
    CreateNode(LMxml, elemCommon, nsURI, "ModelName", "Rufus");
    CreateNode(LMxml, elemCommon, nsURI, "MachineNo", "1");
    CreateNode(LMxml, elemCommon, nsURI, "ROMVersion", "1.0");
    CreateNode(LMxml, elemCommon, nsURI, "Version", "1.2");

    value = QDate::currentDate().toString(QObject::tr("yyyy/MM/dd"));
    CreateNode(LMxml, elemCommon, nsURI, "Date", value);

    value = QTime::currentTime().toString(QObject::tr("HH:mm:ss"));
    CreateNode(LMxml, elemCommon, nsURI, "Time", value);

    QDomElement elePatient = CreateNode(LMxml, elemCommon, nsURI, "Patient");

    value = Datas::I()->patients->currentpatient()? QString::number(Datas::I()->patients->currentpatient()->id()) : "0";
    CreateNode(LMxml, elePatient, nsURI, "No.", value);
    CreateNode(LMxml, elePatient, nsURI, "ID", value);

    CreateNode(LMxml, elePatient, nsURI, "FirstName");
    CreateNode(LMxml, elePatient, nsURI, "MiddleName");
    CreateNode(LMxml, elePatient, nsURI, "LastName");
    CreateNode(LMxml, elePatient, nsURI, "Sex");
    CreateNode(LMxml, elePatient, nsURI, "Age");
    CreateNode(LMxml, elePatient, nsURI, "DOB");
    CreateNode(LMxml, elePatient, nsURI, "NameJ1");
    CreateNode(LMxml, elePatient, nsURI, "NameJ2");

    // QDomElement eleOperator = CreateNode(LMxml, elemCommon, nsURI, "Operator");
    CreateNode(LMxml, elePatient, nsURI, "No.");
    CreateNode(LMxml, elePatient, nsURI, "ID");

    CreateNode(LMxml, elemCommon, nsURI, "Duration", "00:00:30");
    CreateNode(LMxml, elemCommon, nsURI, "DateFormat", "D_M_Y");
}

// Add Pupillary Distance to Measure Element
void AddPD(QDomDocument RMxml, QDomElement elemMeasure, QString nsURI)
{
    // Measure -> PD
    if (Datas::I()->mesureautoref->ecartIP()>0)
    {
        QString value=QString::number(Datas::I()->mesureautoref->ecartIP());
        QDomElement elePD = CreateNode(RMxml, elemMeasure, nsURI, "PD");
        CreateNode(RMxml, elePD, nsURI, "unit", "mm", "Distance", value);

        // Measure -> PD VOID data
        CreateNode(RMxml, elePD, nsURI, "unit", "mm", "DistanceR");
        CreateNode(RMxml, elePD, nsURI, "unit", "mm", "DistanceL");
        CreateNode(RMxml, elePD, nsURI, "unit", "mm", "Near");
        CreateNode(RMxml, elePD, nsURI, "unit", "mm", "NearR");
        CreateNode(RMxml, elePD, nsURI, "unit", "mm", "NearL");
    }
}

void AddLMVoidData(QDomDocument LMxml, QDomElement eleS, QString nsURI)
{
    CreateNode(LMxml, eleS, nsURI, "unit", "D", "SE");
    CreateNode(LMxml, eleS, nsURI, "unit", "D", "ADD2");
    CreateNode(LMxml, eleS, nsURI, "unit", "D", "NearSphere");
    CreateNode(LMxml, eleS, nsURI, "unit", "D", "NearSphere2");
    CreateNode(LMxml, eleS, nsURI, "unit", "pri", "Prism");
    CreateNode(LMxml, eleS, nsURI, "unit", "deg", "PrismBase");
    CreateNode(LMxml, eleS, nsURI, "unit", "pri", "PrismX");
    CreateNode(LMxml, eleS, nsURI, "unit", "pri", "PrismY");
    CreateNode(LMxml, eleS, nsURI, "unit", "%", "UVTransmittance");
    CreateNode(LMxml, eleS, nsURI, "ConfidenceIndex");
}


void AddLMEyeVoidData(QDomDocument LMxml, QDomElement eleS, QString nsURI)
{
    /*
    <nsSBJ:HPri unit="prism"/>
    <nsSBJ:HBase/>
    <nsSBJ:VPri unit="prism"/>
    <nsSBJ:VBase/>
    <nsSBJ:Prism unit="prism"/>
    <nsSBJ:Angle unit="deg"/>
    */
    CreateNode(LMxml, eleS, nsURI, "unit", "prism", "HPri");
    CreateNode(LMxml, eleS, nsURI, "HBase");
    CreateNode(LMxml, eleS, nsURI, "unit", "prism", "VPri");
    CreateNode(LMxml, eleS, nsURI, "VBase");
    CreateNode(LMxml, eleS, nsURI, "unit", "prism", "Prism");
    CreateNode(LMxml, eleS, nsURI, "unit", "deg", "Angle");

}



void AddLMJOIA(QDomDocument LMxml, QDomElement ophtalmology)
{
/*
<nsLM:Measure type="LM">
    <nsLM:MeasureMode>Progressive</nsLM:MeasureMode>
    <nsLM:DiopterStep unit="D">0.25</nsLM:DiopterStep>
    <nsLM:AxisStep unit="deg">1</nsLM:AxisStep>
    <nsLM:CylinderMode>-</nsLM:CylinderMode>
    <nsLM:PrismDiopterStep unit="pri">0.25</nsLM:PrismDiopterStep>
    <nsLM:PrismBaseStep unit="deg">1</nsLM:PrismBaseStep>
    <nsLM:PrismMode>xy</nsLM:PrismMode>
    <nsLM:AddMode>add</nsLM:AddMode>
    <nsLM:LM>
        <nsLM:S>
            <nsLM:Sphere unit="D">-3.00</nsLM:Sphere>
            <nsLM:Cylinder unit="D">-11.00</nsLM:Cylinder>
            <nsLM:Axis unit="deg">180</nsLM:Axis>
            <nsLM:SE unit="D">-8.50</nsLM:SE>
            <nsLM:ADD unit="D">1.50</nsLM:ADD>
            <nsLM:ADD2 unit="D">2.00</nsLM:ADD2>
            <nsLM:NearSphere unit="D">-1.50</nsLM:NearSphere>
            <nsLM:NearSphere2 unit="D">-1.00</nsLM:NearSphere2>
            <nsLM:Prism unit="pri">0.25</nsLM:Prism>
            <nsLM:PrismBase unit="deg">102</nsLM:PrismBase>
            <nsLM:PrismX unit="pri" base="in">0.00</nsLM:PrismX>
            <nsLM:PrismY unit="pri" base="up">0.25</nsLM:PrismY>
            <nsLM:UVTransmittance unit="%">5</nsLM:UVTransmittance>
            <nsLM:ConfidenceIndex>CYL OVERFLOW</nsLM:ConfidenceIndex>
        </nsLM:S>
        <nsLM:R>
            <nsLM:Sphere unit="D">-3.00</nsLM:Sphere>
            <nsLM:Cylinder unit="D">0.00</nsLM:Cylinder>
            <nsLM:Axis unit="deg">0</nsLM:Axis>
            <nsLM:SE unit="D">-3.00</nsLM:SE>
            <nsLM:ADD unit="D">1.50</nsLM:ADD>
            <nsLM:ADD2 unit="D">2.00</nsLM:ADD2>
            <nsLM:NearSphere unit="D">-1.50</nsLM:NearSphere>
            <nsLM:NearSphere2 unit="D">-1.00</nsLM:NearSphere2>
            <nsLM:Prism unit="pri">0.25</nsLM:Prism>
            <nsLM:PrismBase unit="deg">102</nsLM:PrismBase>
            <nsLM:PrismX unit="pri" base="in">0.00</nsLM:PrismX>
            <nsLM:PrismY unit="pri" base="up">0.25</nsLM:PrismY>
            <nsLM:UVTransmittance unit="%">5</nsLM:UVTransmittance>
            <nsLM:ConfidenceIndex></nsLM:ConfidenceIndex>
        </nsLM:R>
        <nsLM:L>
            <nsLM:Sphere unit="D">-2.00</nsLM:Sphere>
            <nsLM:Cylinder unit="D">-1.00</nsLM:Cylinder>
            <nsLM:Axis unit="deg">176</nsLM:Axis>
            <nsLM:SE unit="D">-2.50</nsLM:SE>
            <nsLM:ADD unit="D">1.75</nsLM:ADD>
            <nsLM:ADD2 unit="D">2.00</nsLM:ADD2>
            <nsLM:NearSphere unit="D">-0.25</nsLM:NearSphere>
            <nsLM:NearSphere2 unit="D">0.00</nsLM:NearSphere2>
            <nsLM:Prism unit="pri">2.50</nsLM:Prism>
            <nsLM:PrismBase unit="deg">90</nsLM:PrismBase>
            <nsLM:PrismX unit="pri" base="out">0.00</nsLM:PrismX>
            <nsLM:PrismY unit="pri" base="up">2.50</nsLM:PrismY>
            <nsLM:UVTransmittance unit="%">5</nsLM:UVTransmittance>
            <nsLM:ConfidenceIndex></nsLM:ConfidenceIndex>
        </nsLM:L>
    </nsLM:LM>
    <nsLM:PD>
        <nsLM:Distance unit="mm">58.5</nsLM:Distance>
        <nsLM:DistanceR unit="mm">29.5</nsLM:DistanceR>
        <nsLM:DistanceL unit="mm">29.0</nsLM:DistanceL>
        <nsLM:Near unit="mm"></nsLM:Near>
        <nsLM:NearR unit="mm"></nsLM:NearR>
        <nsLM:NearL unit="mm"></nsLM:NearL>
    </nsLM:PD>
</nsLM:Measure>
*/
    QString nsURI="nsLM";
    QString value = ""; // For convenience

    // Measure
    //QDomElement CreateNode(QDomDocument doc, QDomElement parent, QString nsURI, QString attribute , QString valueAtt, QString name , QString value = "")
    QDomElement elemMeasure = CreateNode(LMxml, ophtalmology, nsURI, "type", "LM", "Measure", "");

    // Measure VOID children
    CreateNode(LMxml, elemMeasure, nsURI, "MeasureMode");
    CreateNode(LMxml, elemMeasure, nsURI, "unit", "D", "DiopterStep");
    CreateNode(LMxml, elemMeasure, nsURI, "unit", "deg", "AxisStep");
    CreateNode(LMxml, elemMeasure, nsURI, "CylinderMode");
    CreateNode(LMxml, elemMeasure, nsURI, "unit", "pri", "PrismDiopterStep");
    CreateNode(LMxml, elemMeasure, nsURI, "unit", "deg", "PrismBaseStep");
    CreateNode(LMxml, elemMeasure, nsURI, "PrismMode");
    CreateNode(LMxml, elemMeasure, nsURI, "AddMode");
    CreateNode(LMxml, elemMeasure, nsURI, "LensLabel");

    // Measure -> LM
    QDomElement eleLM = CreateNode(LMxml, elemMeasure, nsURI, "LM");
    // Measure -> LM -> S
    QDomElement eleS = CreateNode(LMxml, eleLM, nsURI, "S");

    // Measure -> LM -> S   VOID data
    CreateNode(LMxml, eleS, nsURI, "unit", "D", "Sphere");
    CreateNode(LMxml, eleS, nsURI, "unit", "D", "Cylinder");
    CreateNode(LMxml, eleS, nsURI, "unit", "deg", "Axis");
    CreateNode(LMxml, eleS, nsURI, "unit", "D", "ADD");
    // Rest of VOID (or default) data
    AddLMVoidData(LMxml, eleS, nsURI);

    // Measure -> LM -> R
    QDomElement eleR = CreateNode(LMxml, eleLM, nsURI, "R");

    value=QString::number(Datas::I()->mesurefronto->sphereOD(),'f',2);
    CreateNode(LMxml, eleR, nsURI, "unit", "D", "Sphere", value);

    value=QString::number(Datas::I()->mesurefronto->cylindreOD(),'f',2);
    CreateNode(LMxml, eleR, nsURI, "unit", "D", "Cylinder", value);

    value=QString::number(Datas::I()->mesurefronto->axecylindreOD(),'f',2);
    CreateNode(LMxml, eleR, nsURI, "unit", "D", "Axis", value);

    value=QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2);
    CreateNode(LMxml, eleR, nsURI, "unit", "D", "ADD", value);

    // Measure -> LM -> R   VOID data
    AddLMVoidData(LMxml, eleR, nsURI);


    // Measure -> LM -> L
    QDomElement eleL = CreateNode(LMxml, eleLM, nsURI, "L");

    value=QString::number(Datas::I()->mesurefronto->sphereOG(),'f',2);
    CreateNode(LMxml, eleL, nsURI, "unit", "D", "Sphere", value);

    value=QString::number(Datas::I()->mesurefronto->cylindreOG(),'f',2);
    CreateNode(LMxml, eleL, nsURI, "unit", "D", "Cylinder", value);

    value=QString::number(Datas::I()->mesurefronto->axecylindreOG(),'f',2);
    CreateNode(LMxml, eleL, nsURI, "unit", "D", "Axis", value);

    value=QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2);
    CreateNode(LMxml, eleL, nsURI, "unit", "D", "ADD", value);

    // Measure -> LM -> L   VOID data
    AddLMVoidData(LMxml, eleL, nsURI);


    // Measure -> PD
    AddPD(LMxml, elemMeasure, nsURI);
}



void AddLM(QDomDocument LMxml, QDomElement ophtalmology)
{

    QString nsURI="nsSBJ";
    QString value = ""; // For convenience

    // Measure
    QDomElement elemMeasure = CreateNode(LMxml, ophtalmology, nsURI, "type", "SBJ", "Measure", "");
    QDomElement elemRefTest = CreateNode(LMxml, elemMeasure, nsURI, "RefractionTest", "");
    QDomElement elemType1 = CreateNode(LMxml, elemRefTest, nsURI, "No", "1", "Type", "");
    CreateNode(LMxml, elemType1, nsURI, "TypeName", "Current Spectacles"); // Fronto for TopCon
    QDomElement elemExamDistance1 = CreateNode(LMxml, elemType1, nsURI, "No", "1", "ExamDistance", "");


    // ExamDistance No 1 (Far vision)
    CreateNode(LMxml, elemExamDistance1, nsURI, "unit", "cm", "Distance", "500.00");
    QDomElement elemRefractionData = CreateNode(LMxml, elemExamDistance1, nsURI, "RefractionData", "");

    // RefractionData 1 -> R
    QDomElement eleR = CreateNode(LMxml, elemRefractionData, nsURI, "R", "");

    value=QString::number(Datas::I()->mesurefronto->sphereOD(),'f',2);
    CreateNode(LMxml, eleR, nsURI, "unit", "D", "Sph", value);

    value=QString::number(Datas::I()->mesurefronto->cylindreOD(),'f',2);
    CreateNode(LMxml, eleR, nsURI, "unit", "D", "Cyl", value);

    value=QString::number(Datas::I()->mesurefronto->axecylindreOD(),'f',2);
    CreateNode(LMxml, eleR, nsURI, "unit", "D", "Axis", value);


    // RefractionData 1 -> R   VOID data
    AddLMEyeVoidData(LMxml, eleR, nsURI);


    // RefractionData 1 -> L
    QDomElement eleL = CreateNode(LMxml, elemRefractionData, nsURI, "L", "");

    value=QString::number(Datas::I()->mesurefronto->sphereOG(),'f',2);
    CreateNode(LMxml, eleL, nsURI, "unit", "D", "Sph", value);

    value=QString::number(Datas::I()->mesurefronto->cylindreOG(),'f',2);
    CreateNode(LMxml, eleL, nsURI, "unit", "D", "Cyl", value);

    value=QString::number(Datas::I()->mesurefronto->axecylindreOG(),'f',2);
    CreateNode(LMxml, eleL, nsURI, "unit", "D", "Axis", value);


    // RefractionData 1 -> L   VOID data
    AddLMEyeVoidData(LMxml, eleL, nsURI);

    // <nsSBJ:VD unit="mm">12,00</nsSBJ:VD>
    CreateNode(LMxml, elemRefractionData, nsURI, "unit", "mm", "VD", "12.0");

    // RefractionData 1 -> PD
    AddPD(LMxml, elemExamDistance1, nsURI);



    // ExamDistance No 2 (Near vision)
    QDomElement elemExamDistance2 = CreateNode(LMxml, elemType1, nsURI, "No", "2", "ExamDistance", "");
    CreateNode(LMxml, elemExamDistance2, nsURI, "unit", "cm", "Distance", "67.00");
    QDomElement elemRefractionData2 = CreateNode(LMxml, elemExamDistance2, nsURI, "RefractionData", "");



    // RefractionData 2 -> R
    QDomElement eleR2 = CreateNode(LMxml, elemRefractionData2, nsURI, "R", "");


    // ADD  R
    value=QString::number(Datas::I()->mesurefronto->addVPOD() + Datas::I()->mesurefronto->sphereOD(),'f',2);
    CreateNode(LMxml, eleR2, nsURI, "unit", "D", "Sph", value);

    // Needed ... ???

    value=QString::number(Datas::I()->mesurefronto->cylindreOD(),'f',2);
    CreateNode(LMxml, eleR2, nsURI, "unit", "D", "Cyl", value);

    value=QString::number(Datas::I()->mesurefronto->axecylindreOD(),'f',2);
    CreateNode(LMxml, eleR2, nsURI, "unit", "D", "Axis", value);

    // RefractionData 2 -> R   VOID data
    AddLMEyeVoidData(LMxml, eleR2, nsURI);






    // RefractionData 2 -> L
    QDomElement eleL2 = CreateNode(LMxml, elemRefractionData2, nsURI, "L", "");


    // ADD  R
    value=QString::number(Datas::I()->mesurefronto->addVPOG() + Datas::I()->mesurefronto->sphereOG(),'f',2);
    CreateNode(LMxml, eleL2, nsURI, "unit", "D", "Sph", value);

    // Needed ... ???
    value=QString::number(Datas::I()->mesurefronto->cylindreOG(),'f',2);
    CreateNode(LMxml, eleL2, nsURI, "unit", "D", "Cyl", value);

    value=QString::number(Datas::I()->mesurefronto->axecylindreOG(),'f',2);
    CreateNode(LMxml, eleL2, nsURI, "unit", "D", "Axis", value);

    // RefractionData 1 -> L   VOID data
    AddLMEyeVoidData(LMxml, eleL2, nsURI);

    // <nsSBJ:VD unit="mm">12,00</nsSBJ:VD>
    CreateNode(LMxml, elemRefractionData2, nsURI, "unit", "mm", "VD", "12.0");

}





void AddRM(QDomDocument RMxml, QDomElement ophtalmology)
{
    QString nsURI="nsREF";
    QString value = ""; // For convenience

    // Measure
    //QDomElement CreateNode(QDomDocument doc, QDomElement parent, QString nsURI, QString attribute , QString valueAtt, QString name , QString value = "")
    QDomElement elemMeasure = CreateNode(RMxml, ophtalmology, nsURI, "type", "REF", "Measure", "");

    // Measure VOID children
    CreateNode(RMxml, elemMeasure, nsURI, "unit", "mm", "VD", "14.00");
    CreateNode(RMxml, elemMeasure, nsURI, "unit", "D", "DiopterStep");
    CreateNode(RMxml, elemMeasure, nsURI, "unit", "deg", "AxisStep");
    CreateNode(RMxml, elemMeasure, nsURI, "CylinderMode", "-");

    // Measure -> REF
    QDomElement eleREF = CreateNode(RMxml, elemMeasure, nsURI, "REF");
    // Measure -> REF -> R
    QDomElement eleR = CreateNode(RMxml, eleREF, nsURI, "R");
    // Measure -> REF -> R -> List
    QDomElement eleListR = CreateNode(RMxml, eleR, nsURI, "No", "1","List");

    value = QString::number(Datas::I()->mesureautoref->sphereOD(),'f',2);
    CreateNode(RMxml, eleListR, nsURI, "unit", "D", "Sphere", value);
    value = QString::number(Datas::I()->mesureautoref->cylindreOD(),'f',2);
    CreateNode(RMxml, eleListR, nsURI, "unit", "D", "Cylinder", value);
    value = QString::number(Datas::I()->mesureautoref->axecylindreOD());
    CreateNode(RMxml, eleListR, nsURI, "unit", "deg", "Axis", value);

    CreateNode(RMxml, eleListR, nsURI, "unit", "D", "SE");
    CreateNode(RMxml, eleListR, nsURI, "CataractMode");
    CreateNode(RMxml, eleListR, nsURI, "IOLMode");
    CreateNode(RMxml, eleListR, nsURI, "ConfidenceIndex");


    // Measure -> REF -> L
    QDomElement eleL = CreateNode(RMxml, eleREF, nsURI, "L");
    // Measure -> REF -> L -> List
    QDomElement eleListL = CreateNode(RMxml, eleL, nsURI, "No", "1","List");

    value = QString::number(Datas::I()->mesureautoref->sphereOG(),'f',2);
    CreateNode(RMxml, eleListL, nsURI, "unit", "D", "Sphere", value);
    value = QString::number(Datas::I()->mesureautoref->cylindreOG(),'f',2);
    CreateNode(RMxml, eleListL, nsURI, "unit", "D", "Cylinder", value);
    value = QString::number(Datas::I()->mesureautoref->axecylindreOG());
    CreateNode(RMxml, eleListL, nsURI, "unit", "deg", "Axis", value);

    CreateNode(RMxml, eleListL, nsURI, "unit", "D", "SE");
    CreateNode(RMxml, eleListL, nsURI, "CataractMode");
    CreateNode(RMxml, eleListL, nsURI, "IOLMode");
    CreateNode(RMxml, eleListL, nsURI, "ConfidenceIndex");

    // Measure -> PD
    AddPD(RMxml, elemMeasure, nsURI);
}




void Topcon::RegleRefracteurXML(TypesMesures flag, QString nameRF)
{
/*
     QString filename = Adress + "/" + typfile + "_" + codecname + "_" + QString::number(Datas::I()->patients->currentpatient()->id()) + ".xml";
*/
    /*! LE FRONTO */
    bool ExistMesureFronto =  flag.testFlag(MesureFronto) && !Datas::I()->mesurefronto->isdataclean();

    /*
     *  Te lens meter XML file isn't recognized by the CV-5000 PC
     *  Its generation is disabled.
        ExistMesureFronto = false;
     */

    if (ExistMesureFronto)
    {
        QDomDocument LMxml("");

        QDomElement eleOphtalmology = LMxml.createElement("Ophthalmology");
        eleOphtalmology.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        eleOphtalmology.setAttribute("xmlns:nsCommon", "http://www.joia.or.jp/standardized/namespaces/Common");
        eleOphtalmology.setAttribute("xmlns:nsSBJ", "http://www.joia.or.jp/standardized/namespaces/SBJ");
        eleOphtalmology.setAttribute("xsi:schemaLocation", "http://www.joia.or.jp/standardized/namespaces/Common Common_schema.xsd http://www.joia.or.jp/standardized/namespaces/SBJ SBJ_schema.xsd");

        /*
        eleOphtalmology.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        eleOphtalmology.setAttribute("xmlns:nsCommon", "http://www.joia.or.jp/standardized/namespaces/Common");
        eleOphtalmology.setAttribute("xmlns:nsLM", "http://www.joia.or.jp/standardized/namespaces/LM");
        eleOphtalmology.setAttribute("xsi:schemaLocation", "http://www.joia.or.jp/standardized/namespaces/Common Common_schema.xsd http://www.joia.or.jp/standardized/namespaces/LM LM_schema.xsd");
        */

        LMxml.appendChild(eleOphtalmology);

        AddCommon(LMxml, eleOphtalmology);
        AddLM(LMxml, eleOphtalmology);
        EnregistreFileDatasXML(LMxml, MesureFronto);
    }
    /*! L'AUTOREF */
    bool ExistMesureAutoref =  flag.testFlag(MesureAutoref) && !Datas::I()->mesureautoref->isdataclean();
    if (ExistMesureAutoref)
    {
        QDomDocument RMxml("");
        QDomElement eleOphtalmology = RMxml.createElement("Ophthalmology");
        eleOphtalmology.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        eleOphtalmology.setAttribute("xmlns:nsCommon", "http://www.joia.or.jp/standardized/namespaces/Common");
        eleOphtalmology.setAttribute("xmlns:nsREF", "http://www.joia.or.jp/standardized/namespaces/REF");
        eleOphtalmology.setAttribute("xsi:schemaLocation", "http://www.joia.or.jp/standardized/namespaces/Common Common_schema.xsd http://www.joia.or.jp/standardized/namespaces/REF REF_schema.xsd");
        RMxml.appendChild(eleOphtalmology);

        AddCommon(RMxml, eleOphtalmology);
        AddRM(RMxml, eleOphtalmology);
        EnregistreFileDatasXML(RMxml, MesureAutoref);
    }
}


void Topcon::EnregistreFileDatasXML(QDomDocument xml, TypeMesure typmesure)

{
    QString folder ("");
    if (typmesure == MesureAutoref)
    {
        folder = settings().value(Param_Poste_PortRefracteur_DossierEchange_Autoref).toString();
    }
    else if (typmesure == MesureFronto)
    {
        folder = settings().value(Param_Poste_PortRefracteur_DossierEchange_Fronto).toString();
    }
    else
    {
        return;
    }

    PrepareFolder(folder);
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("yyyyMMdd_hhmmss");
    QString filename = folder + "/" + QString::number(Datas::I()->patients->currentpatient()->id()) + "_" + formattedTime + "_RUFUS_V1_Rufus.xml";

    QDomNode node = xml.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"" );
    xml.insertBefore( node, xml.firstChild() );


    SaveFileXML(xml, filename, QStringConverter::Utf8 );
}
