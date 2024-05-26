#include "nidek.h"

Nidek* Nidek::instance = Q_NULLPTR;

Nidek* Nidek::I()
{
    if( !instance )
        instance = new Nidek();
    return instance;
}

Nidek::Nidek() {}


void Nidek::LectureDonneesXMLAutoref(QDomDocument docxml, QString nameARK)
{
    /*! exemple de fichier xml pour un ARK-1s
     *
     *
<?xml version="1.0" encoding="UTF-16"?>
<?xml-stylesheet type="text/xsl" href="RKT_style.xsl"?>
<Data>
    <Company>NIDEK</Company>
    <ModelName>ARK-1s</ModelName>
    <ROMVersion>1.00.02 /5.05</ROMVersion>
    <Version>1.01</Version>
    <Date>2013/03/11</Date>
    <Time>16:03:07</Time>
    <Patient>
        <No.>0003</No.>
        <ID>4902205625223</ID>
    </Patient>
    <Comment> NIDEK ARK-1s</Comment>
    <VD>12.00 mm</VD>
    <WorkingDistance>40 cm</WorkingDistance>
    <DiopterStep>0.01D</DiopterStep>
    <AxisStep>1°</AxisStep>
    <CylinderMode>-</CylinderMode>
    <RefractiveIndex>1.3375</RefractiveIndex>
    <R>
        <AR>
            <ARList No = "1">
                <Sphere>-6.38</Sphere>
                <Cylinder>-0.63</Cylinder>
                <Axis>179</Axis>
                <CataractMode>ON</CataractMode>
                <ConfidenceIndex>9</ConfidenceIndex>
                <SE>-6.70</SE>
            </ARList>
            <ARList No = "2">
                <Error>COVR </Error>
            </ARList>
            <ARMedian>
                <Sphere>-6.38</Sphere>
                <Cylinder>-0.64</Cylinder>
                <Axis>177</Axis>
                <SE>-6.70</SE>
            </ARMedian>
            <TrialLens>
                <Sphere>-6.25</Sphere>
                <Cylinder>-0.75</Cylinder>
                <Axis>177</Axis>
            </TrialLens>
            <ContactLens>
                <Sphere>-5.93</Sphere>
                <Cylinder>-0.54</Cylinder>
                <Axis>177</Axis>
                <SE>-6.20</SE>
            </ContactLens>
            <RingImage>
                ARK_4902205625223 _20130311160307RA1.jpg
            </RingImage>
        </AR>
        <VA>
            <UCVA>&lt;0.1</UCVA>
            <BCVA>1.0</BCVA>
            <LVA>0.8</LVA>
            <GVA>0.5</GVA>
            <NVA>0.8</NVA>
            <WorkingDistance>35 cm</WorkingDistance>
        </VA>
        <SR>
            <Sphere>-6.25</Sphere>
            <Cylinder>-0.75</Cylinder>
            <Axis>177</Axis>
            <SE>-6.75</SE>
            <ADD>+1.75</ADD>
            <WorkingDistance>35 cm</WorkingDistance>
        </SR>
        <LM>
            <Sphere>-0.50</Sphere>
            <Cylinder>-0.00</Cylinder>
            <Axis>0</Axis>
            <ADD>+3.00</ADD>
            <ADD2>+3.50</ADD2>
        </LM>
        <KM>
            <KMList No = "1">
                <R1>
                    <Radius>7.56</Radius>
                    <Power>44.64</Power>
                    <Axis>179</Axis>
                </R1>
                <R2>
                    <Radius>7.29</Radius>
                    <Power>46.30</Power>
                    <Axis>89</Axis>
                </R2>
                <Average>
                    <Radius>7.43</Radius>
                    <Power>45.42</Power>
                </Average>
                <KMCylinder>
                    <Power>-1.66</Power>
                    <Axis>179</Axis>
                </KMCylinder>
            </KMList>
            <KMMedian>
                <R1>
                    <Radius>7.55</Radius>
                    <Power>44.70</Power>
                    <Axis>178</Axis>
                </R1>
                <R2>
                    <Radius>7.29</Radius>
                    <Power>46.30</Power>
                    <Axis>88</Axis>
                </R2>
                <Average>
                    <Radius>7.42</Radius>
                    <Power>45.49</Power>
                </Average>
                <KMCylinder>
                    <Power>-1.60</Power>
                    <Axis>178</Axis>
                </KMCylinder>
            </KMMedian>
        </KM>
        <CS>
            <CSList No = "1">
                <Size>12.1</Size>
            </CSList>
        </CS>
        <PS>
            <PSList No = "1">
                <Size>4.7</Size>
                <Lamp>ON</Lamp>
            </PSList>
        </PS>
        <AC>
            <Sphere>8.15</Sphere>
            <MaxPS>4.1</MaxPS>
            <MinPS>1.6</MinPS>
            <AccImage>ARK_4902205625223_20130311160307RC1.jpg</AccImage>
        </AC>
        <RI>
            <COIH>0.7</COIH>
            <COIA>1</COIA>
            <POI>0</POI>
            <RetroImage>ARK_4902205625223_20130311160307RI1.jpg</RetroImage>
        </RI>
    </R>
    <L>
    </L>
    <PD>
        <PDList No = "1">
            <FarPD>56</FarPD>
            <RPD>28</RPD>
            <LPD>28</LPD>
            <NearPD>53</NearPD>
         </PDList>
    </PD>
</Data>

*/

    bool autorefhaskerato    = (nameARK == "NIDEK ARK-1A"
                             || nameARK == "NIDEK ARK-1"
                             || nameARK == "NIDEK ARK-1S"
                             || nameARK == "NIDEK ARK-530A"
                             || nameARK == "NIDEK ARK-510A"
                             || nameARK == "NIDEK HandyRef-K"
                             || nameARK == "NIDEK TONOREF III"
                             || nameARK == "NIDEK ARK-30");
    bool autorefhastonopachy = (nameARK == "NIDEK TONOREF III");
    bool autorefhasipmesure = (nameARK != "NIDEK HandyRef-K"
                               || nameARK != "NIDEK ARK-30"
                               || nameARK != "NIDEK AR-20");
    bool istonorefIII = (nameARK == "NIDEK TONOREF III");
    QDomElement xml = docxml.documentElement();
    for (int i=0; i<xml.childNodes().size(); i++)
    {
        QDomElement childnode = xml.childNodes().at(i).toElement();
        if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
        {
            for (int j=0; j<childnode.childNodes().size(); j++)
            {
                QDomElement childRnode = childnode.childNodes().at(j).toElement();
                if (childRnode.tagName() == "AR")
                {
                    for (int k=0; k<childRnode.childNodes().size(); k++)
                    {
                        QDomElement childARnode = childRnode.childNodes().at(k).toElement();
                        if (childARnode.tagName() == "ARMedian")
                        {
                            for (int l=0; l<childARnode.childNodes().size(); l++)
                            {
                                QDomElement childARmednode = childARnode.childNodes().at(l).toElement();
                                if (childARmednode.tagName() == "Sphere")
                                    Datas::I()->mesureautoref->setsphereOD(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                if (childARmednode.tagName() == "Cylinder")
                                    Datas::I()->mesureautoref->setcylindreOD(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                if (childARmednode.tagName() == "Axis")
                                    Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(childARmednode.text().toInt()));
                            }
                        }
                    }
                }
            }
        }
        if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
        {
            for (int j=0; j<childnode.childNodes().size(); j++)
            {
                QDomElement childRnode = childnode.childNodes().at(j).toElement();
                if (childRnode.tagName() == "AR")
                {
                    for (int k=0; k<childRnode.childNodes().size(); k++)
                    {
                        QDomElement childARnode = childRnode.childNodes().at(k).toElement();
                        if (childARnode.tagName() == "ARMedian")
                        {
                            for (int l=0; l<childARnode.childNodes().size(); l++)
                            {
                                QDomElement childARmednode = childARnode.childNodes().at(l).toElement();
                                if (childARmednode.tagName() == "Sphere")
                                    Datas::I()->mesureautoref->setsphereOG(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                if (childARmednode.tagName() == "Cylinder")
                                    Datas::I()->mesureautoref->setcylindreOG(Utils::roundToNearestPointTwentyFive(childARmednode.text().toDouble()));
                                if (childARmednode.tagName() == "Axis")
                                    Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(childARmednode.text().toInt()));
                            }
                        }
                    }
                }
            }
        }
    }
    if (autorefhasipmesure)
    {
        Datas::I()->mesureautoref->setecartIP(0);
        for (int i=0; i<xml.childNodes().size(); i++)
        {
            QDomElement childnode = xml.childNodes().at(i).toElement();
            if (childnode.tagName() == "PD")                         /*! ECART INTERPUPILLAIRE  --------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement childPDnode = childnode.childNodes().at(j).toElement();
                    if (childPDnode.tagName() == "PDList" && childPDnode.attributeNode("No").value().toInt() == 1)
                    {
                        for (int k=0; k<childPDnode.childNodes().size(); k++)
                        {
                            QDomElement childPD1node = childPDnode.childNodes().at(k).toElement();
                            if (childPD1node.tagName() == "FarPD")
                            {
                                Datas::I()->mesureautoref->setecartIP(childPD1node.text().toInt());
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    if (autorefhaskerato)
    {
        QString DiametreKeratoAUtiliserTonorefIII = "2.4mm";
        //! Données de KERATOMETRIE --------------------------------------------------------------------------------------------------------
        for (int i=0; i<xml.childNodes().size(); i++)
        {
            QDomElement childnode = xml.childNodes().at(i).toElement();
            if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement K = childnode.childNodes().at(j).toElement();
                    if ((K.tagName() == "KM" && !istonorefIII)
                        || (K.tagName() == "KM" && istonorefIII && (K.attributeNode("condition").value() == "ø"+ DiametreKeratoAUtiliserTonorefIII || K.attributeNode("condition").value() == "Φ" + DiametreKeratoAUtiliserTonorefIII)))
                    {
                        for (int k=0; k<K.childNodes().size(); k++)
                        {
                            QDomElement KM = K.childNodes().at(k).toElement();
                            if (KM.tagName() == "KMMedian")
                            {
                                for (int l=0; l<KM.childNodes().size(); l++)
                                {
                                    QDomElement KMR = KM.childNodes().at(l).toElement();
                                    if (KMR.tagName() == "R1")
                                    {
                                        for (int m=0; m<KMR.childNodes().size(); m++)
                                        {
                                            QDomElement KMR1 = KMR.childNodes().at(m).toElement();
                                            if (KMR1.tagName() == "Radius" && KMR1.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setK1OD(KMR1.text().toDouble());
                                            if (KMR1.tagName() == "Power" && KMR1.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setdioptriesK1OD(round(KMR1.text().toDouble()*10)/10);
                                        }
                                    }
                                    if (KMR.tagName() == "R2")
                                    {
                                        for (int m=0; m<KMR.childNodes().size(); m++)
                                        {
                                            QDomElement KMR2 = KMR.childNodes().at(m).toElement();
                                            if (KMR2.tagName() == "Radius" && KMR2.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setK2OD(KMR2.text().toDouble());
                                            if (KMR2.tagName() == "Power" && KMR2.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setdioptriesK2OD(round(KMR2.text().toDouble()*10)/10);
                                        }
                                    }
                                    if (KMR.tagName() == "KMCylinder")
                                    {
                                        for (int m=0; m<KMR.childNodes().size(); m++)
                                        {
                                            QDomElement axeK = KMR.childNodes().at(m).toElement();
                                            if (axeK.tagName() == "Axis" && axeK.text().toInt() > 0)
                                                Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(axeK.text().toInt()));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement K = childnode.childNodes().at(j).toElement();
                    if ((K.tagName() == "KM" && !istonorefIII)
                        || (K.tagName() == "KM" && istonorefIII && (K.attributeNode("condition").value() == "ø" + DiametreKeratoAUtiliserTonorefIII || K.attributeNode("condition").value() == "Φ" + DiametreKeratoAUtiliserTonorefIII)))
                    {
                        for (int k=0; k<K.childNodes().size(); k++)
                        {
                            QDomElement KM = K.childNodes().at(k).toElement();
                            if (KM.tagName() == "KMMedian")
                            {
                                for (int l=0; l<KM.childNodes().size(); l++)
                                {
                                    QDomElement KMR = KM.childNodes().at(l).toElement();
                                    if (KMR.tagName() == "R1")
                                    {
                                        for (int m=0; m<KMR.childNodes().size(); m++)
                                        {
                                            QDomElement KMR1 = KMR.childNodes().at(m).toElement();
                                            if (KMR1.tagName() == "Radius" && KMR1.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setK1OG(KMR1.text().toDouble());
                                            if (KMR1.tagName() == "Power" && KMR1.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setdioptriesK1OG(Utils::roundToNearestPointTwentyFive(KMR1.text().toDouble()));
                                        }
                                    }
                                    if (KMR.tagName() == "R2")
                                    {
                                        for (int m=0; m<KMR.childNodes().size(); m++)
                                        {
                                            QDomElement KMR2 = KMR.childNodes().at(m).toElement();
                                            if (KMR2.tagName() == "Radius" && KMR2.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setK2OG(KMR2.text().toDouble());
                                            if (KMR2.tagName() == "Power" && KMR2.text().toDouble() > 0)
                                                Datas::I()->mesurekerato->setdioptriesK2OG(Utils::roundToNearestPointTwentyFive(KMR2.text().toDouble()));
                                        }
                                    }
                                    if (KMR.tagName() == "KMCylinder")
                                    {
                                        for (int m=0; m<KMR.childNodes().size(); m++)
                                        {
                                            QDomElement axeK = KMR.childNodes().at(m).toElement();
                                            if (axeK.tagName() == "Axis" && axeK.text().toInt() > 0)
                                                Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(axeK.text().toInt()));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (autorefhastonopachy)
    {
        //! Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
        for (int i=0; i<xml.childNodes().size(); i++)
        {
            QDomElement childnode = xml.childNodes().at(i).toElement();
            if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement TO = childnode.childNodes().at(j).toElement();
                    if (TO.tagName() == "NT")
                    {
                        for (int k=0; k<TO.childNodes().size(); k++)
                        {
                            QDomElement TOD = TO.childNodes().at(k).toElement();
                            if (TOD.tagName() == "NTAverage")
                            {
                                for (int l=0; l<TOD.childNodes().size(); l++)
                                {
                                    QDomElement TOunit = TOD.childNodes().at(l).toElement();
                                    if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                    {
                                        Datas::I()->mesuretono->setTOD(std::round(TOunit.text().toDouble()));
                                        Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                    }
                                }
                            }
                            if (TOD.tagName() == "CorrectedIOP")
                            {
                                for (int l=0; l<TOD.childNodes().size(); l++)
                                {
                                    QDomElement TOcor = TOD.childNodes().at(l).toElement();
                                    if (TOcor.tagName() == "Corrected")
                                    {
                                        for (int m=0; m<TOcor.childNodes().size(); m++)
                                        {
                                            QDomElement TOunit = TOcor.childNodes().at(m).toElement();
                                            if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                            {
                                                Datas::I()->mesuretono->setTODcorrigee(std::round(TOunit.text().toDouble()));
                                                Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement TO = childnode.childNodes().at(j).toElement();
                    if (TO.tagName() == "NT")
                    {
                        for (int k=0; k<TO.childNodes().size(); k++)
                        {
                            QDomElement TOG = TO.childNodes().at(k).toElement();
                            if (TOG.tagName() == "NTAverage")
                            {
                                for (int l=0; l<TOG.childNodes().size(); l++)
                                {
                                    QDomElement TOunit = TOG.childNodes().at(l).toElement();
                                    if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                    {
                                        Datas::I()->mesuretono->setTOG(std::round(TOunit.text().toDouble()));
                                        Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                    }
                                }
                            }
                            if (TOG.tagName() == "CorrectedIOP")
                            {
                                for (int l=0; l<TOG.childNodes().size(); l++)
                                {
                                    QDomElement TOcor = TOG.childNodes().at(l).toElement();
                                    if (TOcor.tagName() == "Corrected")
                                    {
                                        for (int m=0; m<TOcor.childNodes().size(); m++)
                                        {
                                            QDomElement TOunit = TOcor.childNodes().at(m).toElement();
                                            if (TOunit.tagName() == "mmHg" && std::round(TOunit.text().toDouble()) > 0)
                                            {
                                                Datas::I()->mesuretono->setTOGcorrigee(std::round(TOunit.text().toDouble()));
                                                Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        //! Données de PACHYMETRIE --------------------------------------------------------------------------------------------------------
        for (int i=0; i<xml.childNodes().size(); i++)
        {
            QDomElement childnode = xml.childNodes().at(i).toElement();
            if (childnode.tagName() == "R")                         /*! OEIL DROIT  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement pachy = childnode.childNodes().at(j).toElement();
                    if (pachy.tagName() == "PACHY")
                    {
                        for (int k=0; k<pachy.childNodes().size(); k++)
                        {
                            QDomElement pachyD = pachy.childNodes().at(k).toElement();
                            if (pachyD.tagName() == "PACHYAverage")
                            {
                                for (int l=0; l<pachyD.childNodes().size(); l++)
                                {
                                    QDomElement thickness = pachyD.childNodes().at(l).toElement();
                                    if (thickness.tagName() == "Thickness" && thickness.text().toInt() > 0)
                                    {
                                        Datas::I()->mesurepachy->setpachyOD(thickness.text().toInt());
                                        Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (childnode.tagName() == "L")                         /*! OEIL GAUCHE  ------------------------------------------------------------------*/
            {
                for (int j=0; j<childnode.childNodes().size(); j++)
                {
                    QDomElement pachy = childnode.childNodes().at(j).toElement();
                    if (pachy.tagName() == "PACHY")
                    {
                        for (int k=0; k<pachy.childNodes().size(); k++)
                        {
                            QDomElement pachyG = pachy.childNodes().at(k).toElement();
                            if (pachyG.tagName() == "PACHYAverage")
                            {
                                for (int l=0; l<pachyG.childNodes().size(); l++)
                                {
                                    QDomElement thickness = pachyG.childNodes().at(l).toElement();
                                    if (thickness.tagName() == "Thickness" && thickness.text().toInt() > 0)
                                    {
                                        Datas::I()->mesurepachy->setpachyOG(thickness.text().toInt());
                                        Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void Nidek::LectureDonneesXMLFronto(QDomDocument docxml)
{
    /*! exemple de fichier xml pour un ARK-1s
    *‌ il est conforme au standard de la Japan Ophthalmic Instrument Association (https://www.joia.or.jp/)
    * donc le code pour parser le xml est le même pour tous les appareils qui suivent ce standard p.e. Tomey Tl-6000 et TL-7000, Rodenstock AL 6600

<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="NIDEK_LM_Stylesheet.xsl" ?
<Ophthalmology>
<Common>
    <Company>NIDEK</Company>
    <ModelName>LM-1800PD</ModelName>
    <MachineNo></MachineNo>
    <ROMVersion>1.10</ROMVersion>
    <Version>NIDEK_V1.01</Version>
    <Date>2011-03-04</Date>
    <Time>08:08:03</Time>
    <Patient>
        <No.>0001</No.>
        <ID>pati</ID>
        <FirstName></FirstName>
        <MiddleName></MiddleName>
        <LastName></LastName>
        <Sex></Sex>
        <Age></Age>
        <DOB></DOB>
        <NameJ1></NameJ1>
        <NameJ2></NameJ2>
    </Patient>
    <Operator>
        <No.></No.>
        <ID>ope</ID>
    </Operator>
</Common>
<Measure Type="LM">
    <MeasureMode>AutoProgressive</MeasureMode>
    <DiopterStep unit="D">0.25</DiopterStep>
    <AxisStep unit="deg">1</AxisStep>
    <CylinderMode>-</CylinderMode>
    <PrismDiopterStep unit="pri">0.25</PrismDiopterStep>
    <PrismBaseStep unit="deg">1</PrismBaseStep>
    <PrismMode>xy</PrismMode>
    <AddMode>add</AddMode>
    <LM>
        <S>
            <Sphere unit="D">-2.00</Sphere>
            <Cylinder unit="D">-1.50</Cylinder>
            <Axis unit="deg">143</Axis>
            <SE unit="D"></SE>
            <ADD unit="D">1.75</ADD>
            <ADD2 unit="D"></ADD2>
            <NearSphere unit="D">0.00</NearSphere>
            <NearSphere2 unit="D"></NearSphere2>
            <Prism unit="pri">2.25</Prism>
            <PrismBase unit="deg">77</PrismBase>
            <PrismX unit="pri" base="in">0.50</PrismX>
            <PrismY unit="pri" base="up">2.00</PrismY>
            <UVTransmittance unit="%">5</UVTransmittance>
            <ConfidenceIndex></ConfidenceIndex>
        </S>
        <S>
            <Error>MEASUREMENT ERROR</Error>
        </S>
        <R>
            <Sphere unit="D">-2.00</Sphere>
            <Cylinder unit="D">-1.50</Cylinder>
            <Axis unit="deg">143</Axis>
            <SE unit="D"></SE>
            <ADD unit="D">1.75</ADD>
            <ADD2 unit="D"></ADD2>
            <NearSphere unit="D">0.00</NearSphere>
            <NearSphere2 unit="D"></NearSphere2>
            <Prism unit="pri">2.25</Prism>
            <PrismBase unit="deg">77</PrismBase>
            <PrismX unit="pri" base="in">0.50</PrismX>
            <PrismY unit="pri" base="up">2.00</PrismY>
            <UVTransmittance unit="%">5</UVTransmittance>
            <ConfidenceIndex></ConfidenceIndex>
        </R>
        <L>
            <Sphere unit="D">-2.00</Sphere>
            <Cylinder unit="D">-1.50</Cylinder>
            <Axis unit="deg">148</Axis>
            <SE unit="D"></SE>
            <ADD unit="D">1.75</ADD>
            <ADD2 unit="D"></ADD2>
            <NearSphere unit="D">-0.25</NearSphere>
            <NearSphere2 unit="D"></NearSphere2>
            <Prism unit="pri">1.50</Prism>
            <PrismBase unit="deg">61</PrismBase>
            <PrismX unit="pri" base="out">0.75</PrismX>
            <PrismY unit="pri" base="up">1.25</PrismY>
            <UVTransmittance unit="%">5</UVTransmittance>
            <ConfidenceIndex></ConfidenceIndex>
        </L>
    </LM>
    <PD>
        <Distance unit="mm">70.5</Distance>
        <DistanceR unit="mm">35.5</DistanceR>
        <DistanceL unit="mm">35.0</DistanceL>
    </PD>
    <NIDEK>
        <S>
            <Length unit="mm">23</Length>
            <ChannelWidth unit="mm">18</ChannelWidth>
            <ChannelLength unit="mm">23</ChannelLength>
            <Index></Index>
            <GreenTransmittance unit="%"></GreenTransmittance unit="%">
        </S>
        <R>
            <Length unit="mm">23</Length>
            <ChannelWidth unit="mm">18</ChannelWidth>
            <ChannelLength unit="mm">23</ChannelLength>
            <Index></Index>
            <GreenTransmittance unit="%"></GreenTransmittance unit="%">
        </R>
        <L>
            <Length unit="mm">22</Length>
            <ChannelWidth unit="mm">15</ChannelWidth>
            <ChannelLength unit="mm">20</ChannelLength>
            <Index></Index>
            <GreenTransmittance unit="%"></GreenTransmittance unit="%">
        </L>
        <NetPrism>
            <NetHPrism unit="pri" base="in">3</NetHPrism>
            <NetVPrism unit="pri" base="out">2</NetVPrism>
        </NetPrism>
        <Inside>
            <InsideR unit="mm">2.0</InsideR>
            <InsideL unit="mm">1.0</InsideL>
        </Inside>
    </NIDEK>
</Measure>
</Ophthalmology>
*/
    QDomElement xml = docxml.documentElement();
    for (int h=0; h<xml.childNodes().size(); h++)
    {
        QDomElement childnode = xml.childNodes().at(h).toElement();
        if (childnode.tagName() == "Measure")
        {
            for (int g=0; g<childnode.childNodes().size(); g++)
            {
                QDomElement measure = childnode.childNodes().at(g).toElement();
                if (measure.tagName() == "LM")
                {
                    for (int i=0; i<measure.childNodes().size(); i++)
                    {
                        QDomElement lm = measure.childNodes().at(i).toElement();
                        if (lm.tagName() == "R")                        /*! OEIL DROIT  ------------------------------------------------------------------*/
                        {
                            for (int l=0; l<lm.childNodes().size(); l++)
                            {
                                QDomElement valnode = lm.childNodes().at(l).toElement();
                                if (valnode.tagName() == "Sphere")
                                    Datas::I()->mesurefronto->setsphereOD(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                if (valnode.tagName() == "Cylinder")
                                    Datas::I()->mesurefronto->setcylindreOD(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                if (valnode.tagName() == "Axis")
                                    Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(valnode.text().toInt()));
                                if (valnode.tagName() == "ADD")
                                    Datas::I()->mesurefronto->setaddVPOD(Utils::roundToNearestFive(valnode.text().toInt()));
                            }
                        }
                        if (lm.tagName() == "L")                        /*! OEIL GAUCHE  ------------------------------------------------------------------*/
                        {
                            for (int l=0; l<lm.childNodes().size(); l++)
                            {
                                QDomElement valnode = lm.childNodes().at(l).toElement();
                                if (valnode.tagName() == "Sphere")
                                    Datas::I()->mesurefronto->setsphereOG(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                if (valnode.tagName() == "Cylinder")
                                    Datas::I()->mesurefronto->setcylindreOG(Utils::roundToNearestPointTwentyFive(valnode.text().toDouble()));
                                if (valnode.tagName() == "Axis")
                                    Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(valnode.text().toInt()));
                                if (valnode.tagName() == "ADD")
                                    Datas::I()->mesurefronto->setaddVPOG(Utils::roundToNearestFive(valnode.text().toInt()));
                            }
                        }
                    }
                }
                if (measure.tagName() == "PD")                          /*! ECART INTERPUPILLAIRE  --------------------------------------------------------*/
                {
                    for (int i=0; i<measure.childNodes().size(); i++)
                    {
                        QDomElement PD = measure.childNodes().at(i).toElement();
                        if (PD.tagName() == "Distance")
                            Datas::I()->mesurefronto->setecartIP(PD.text().toInt());
                    }
                }
            }
        }
    }
}

void Nidek::LectureDonneesXMLRefracteur(QDomDocument docxml, TypesMesures flag)
{
    /*! exemple de fichier xml pour un RT-6100
     *
     *
<?xml version="1.0" encoding="UTF-16"?>
<?xml-stylesheet type="text/xsl" href="NIDEK_RT_Stylesheet.xsl"?>
<Ophthalmology>
  <Common>
    <Company>NIDEK</Company>
    <ModelName>RT-6100</ModelName>
    <MachineNo>360008</MachineNo>
    <ROMVersion>1.1.4</ROMVersion>
    <Version>NIDEK_RT_V1.00</Version>
    <Date>2021-10-26</Date>
    <Time>17-16-48</Time>
    <Patient>
    <No>0138</No>
    <ID></ID>
      <FirstName/>
      <MiddleName/>
      <LastName/>
      <Sex/>
    <Age>40</Age>
      <DOB/>
      <NameJ1/>
      <NameJ2/>
    </Patient>
  </Common>
  <Measure Type="RT">
    <Phoropter>
      <DiopterStep unit="D"/>
      <AxisStep unit="deg"/>
      <CylinderMode>-</CylinderMode>
      <Corrected CorrectionType="LM_Base" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <SatisfactionLevel MaxLevel="5">4</SatisfactionLevel>
        <Satisfaction>Yes</Satisfaction>
        <R>
          <Sphere unit="D">1.00</Sphere>
          <Cylinder unit="D">-0.50</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm"/>
        </R>
        <L>
          <Sphere unit="D">1.50</Sphere>
          <Cylinder unit="D">-0.75</Cylinder>
          <Axis unit="deg">80</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>0.90</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm"/>
        </L>
        <B>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm"/>
        </B>
      </Corrected>
      <Corrected CorrectionType="REF_Base" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <R>
          <Sphere unit="D">0.50</Sphere>
          <Cylinder unit="D">-1.00</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </R>
        <L>
          <Sphere unit="D">1.00</Sphere>
          <Cylinder unit="D">-1.25</Cylinder>
          <Axis unit="deg">80</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </L>
        <B>
          <VA/>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">61.00</PD>
        </B>
      </Corrected>
      <Corrected CorrectionType="Full" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <R>
          <Sphere unit="D">1.25</Sphere>
          <Cylinder unit="D">-1.00</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </R>
        <L>
          <Sphere unit="D">1.50</Sphere>
          <Cylinder unit="D">-1.25</Cylinder>
          <Axis unit="deg">85</Axis>
          <ADD unit="D"/>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </L>
        <B>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">61.00</PD>
        </B>
      </Corrected>
      <Corrected CorrectionType="Best" Vision="Distant" Situation="Standard">
        <VD unit="mm"/>
        <WorkingDistance unit="cm">40</WorkingDistance>
        <R>
          <Sphere unit="D">0.75</Sphere>
          <Cylinder unit="D">-0.50</Cylinder>
          <Axis unit="deg">90</Axis>
          <ADD unit= "D">0.25</ADD>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </R>
        <L>
          <Sphere unit="D">1.50</Sphere>
          <Cylinder unit="D">-1.25</Cylinder>
          <Axis unit="deg">85</Axis>
          <ADD unit= "D">0.25</ADD>
          <Prism unit="pri"/>
          <PrismBase unit="deg"/>
          <PrismX unit="pri" base="in"/>
          <PrismY unit="pri" base="up"/>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">30.50</PD>
        </L>
        <B>
          <VA>1.00</VA>
          <ADDVA/>
          <PHVA/>
          <LIVA/>
          <PD unit="mm">61.00</PD>
        </B>
      </Corrected>
      <Other/>
    </Phoropter>
    <The21pointExams>
      <No07>
        <R>
        <Sphere unit="D">0.75</Sphere>
        <Cylinder unit="D">-0.50</Cylinder>
        <Axis unit="deg">90</Axis>
        </R>
        <L>
        <Sphere unit="D">1.50</Sphere>
        <Cylinder unit="D">-1.25</Cylinder>
        <Axis unit="deg">85</Axis>
        </L>
      </No07>
    </The21pointExams>
    <KM>
      <ReferenceInstance></ReferenceInstance>
      <DiopterStep Unit="D">0.25D</DiopterStep>
      <AxisStep Unit="deg">5°</AxisStep>
      <CylinderMode>-</CylinderMode>
      <R>
        <Median>
          <R1>
            <Radius>7.69</Radius>
            <Power>44.00</Power>
            <Axis>86</Axis>
          </R1>
          <R2>
            <Radius>7.58</Radius>
            <Power>44.50</Power>
            <Axis>176</Axis>
          </R2>
        </Median>
      </R>
      <L>
        <Median>
          <R1>
            <Radius>7.69</Radius>
            <Power>44.00</Power>
            <Axis>95</Axis>
          </R1>
          <R2>
            <Radius>7.53</Radius>
            <Power>44.75</Power>
            <Axis>5</Axis>
          </R2>
        </Median>
      </L>
    </KM>
    <TM>
      <ReferenceInstance></ReferenceInstance>
      <R>
        <Average>
          <IOP_mmHg unit="mmHg">12.20</IOP_mmHg>
          <IOP_Pa unit="kPa">1.61</IOP_Pa>
        </Average>
      </R>
      <L>
        <Average>
          <IOP_mmHg unit="mmHg">12.20</IOP_mmHg>
          <IOP_Pa unit="kPa">1.60</IOP_Pa>
        </Average>
      </L>
    </TM>
    <PACHY>
      <ReferenceInstance></ReferenceInstance>
      <R>
        <Average>
          <Thickness>581</Thickness>
        </Average>
      </R>
      <L>
        <Average>
          <Thickness>590</Thickness>
        </Average>
      </L>
    </PACHY>
    <Test_Time unit="sec">5</Test_Time>
    <SystemNo>0</SystemNo>
  </Measure>
</Ophthalmology>

*/
    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString mAddOD      = "+0.00";
    QString mAddOG      = "+0.00";
    QString AVLOD(""), AVLOG("");
    QString PD          = "";
    QString PDD(""), PDG("");

    QString K1OD("null"), K2OD("null"), K1OG("null"), K2OG("null");
    int     AxeKOD(0), AxeKOG(0);
    QString mTOOD(""), mTOOG("");

    QDomElement xml = docxml.documentElement();
    for (int i=0; i<xml.childNodes().size(); i++)
    {
        QDomElement childnode = xml.childNodes().at(i).toElement();
        if (childnode.tagName() == "Measure")
        {
            for (int j=0; j<childnode.childNodes().size(); j++)
            {
                QDomElement childnodemeasure = childnode.childNodes().at(j).toElement();
                // On essaie de récupérer les mesures de fronto, autoref, refractions subjective et fianle
                if (childnodemeasure.tagName() == "Phoropter")
                {
                    /*! On a 4 tags Corrected à récupérer avec 3 attributs pou chacun
                         * le premier attribut ("CorrectionType") peut être
                            * LM_Base pour fronto
                            * REF_Base pour autoref
                            * Full pour acuité subjective
                            * Best pour final
                         * Les 2 autres attributs sont
                            * La distance ("Vision"): Distant ou Near (mais tout est dans Distant, on ne prend pas Near ???)
                            * L'ambiance lumineuse de mesure ("Situation"): Standard ou Night - on ne prend que Standard
                         */

                    bool OKMesureFronto = false;
                    bool OKMesureAutoref = false;
                    bool OKMesureAcuite = false;
                    bool OKMesureFinal = false;
                    QDomElement childnodephoropter = childnodemeasure;
                    for (int k=0; k<childnodephoropter.childNodes().size(); k++)
                    {
                        QDomElement childnodephoropterit = childnodemeasure.childNodes().at(k).toElement();
                        int a = childnodephoropterit.attributes().count();
                        if (a > 0)
                        {
                            QStringList listattributesnames, listattributesvalues;
                            for (int b=0;b<a;b++)
                            {
                                listattributesnames     << childnodephoropterit.attributes().item(b).toAttr().name();
                                listattributesvalues    << childnodephoropterit.attributes().item(b).toAttr().value();
                            }

                            //! On essaie de récupérer une mesure Fronto
                            OKMesureFronto =   listattributesnames.contains("Vision")
                                             && listattributesnames.contains("CorrectionType")
                                             && listattributesnames.contains("Situation")
                                             && listattributesvalues.contains("Distant")
                                             && listattributesvalues.contains("LM_Base")
                                             && listattributesvalues.contains("Standard");
                            if (OKMesureFronto && flag.testFlag(MesureFronto))             //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur)
                            {
                                //qDebug() << "OK Fronto" << childnodephoropter.tagName() << " - " << childnodephoropter.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                MesureRefraction        *oldMesureFronto = new MesureRefraction();
                                oldMesureFronto         ->setdatas(Datas::I()->mesurefronto);
                                Datas::I()->mesurefronto->cleandatas();
                                for (int frit=0; frit<childnodephoropterit.childNodes().size(); frit++)
                                {
                                    QDomElement childnodefronto = childnodephoropterit.childNodes().at(frit).toElement();
                                    // OEIL DROIT -----------------------------------------------------------------------------
                                    if (childnodefronto.tagName() == "R")
                                    {
                                        for (int frR=0; frR<childnodefronto.childNodes().size(); frR++)
                                        {
                                            QDomElement childnodeitem = childnodefronto.childNodes().at(frR).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOD = childnodeitem.text();
                                        }
                                        Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
                                        Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
                                        Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                        Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
                                    }
                                    // OEIL GAUCHE -----------------------------------------------------------------------------
                                    if (childnodefronto.tagName() == "L")
                                    {
                                        for (int frG=0; frG<childnodefronto.childNodes().size(); frG++)
                                        {
                                            QDomElement childnodeitem = childnodefronto.childNodes().at(frG).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOG = childnodeitem.text();
                                        }
                                        Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
                                        Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
                                        Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                        Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
                                    }
                                    // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                    if (childnodefronto.tagName() == "B")
                                        for (int frEIP=0; frEIP<childnodefronto.childNodes().size(); frEIP++)
                                        {
                                            QDomElement childnodeitem = childnodefronto.childNodes().at(frEIP).toElement();
                                            if (childnodeitem.tagName() == "PD")
                                            {
                                                PD = childnodeitem.text();
                                                Datas::I()->mesurefronto->setecartIP(PD.toInt());
                                            }
                                        }
                                }
                                if (Datas::I()->mesurefronto->isDifferent(oldMesureFronto) && !Datas::I()->mesurefronto->isdataclean())
                                {
                                    emit newmesure(MesureFronto);
                                }
                                delete oldMesureFronto;
                                //if (!OKMesureFronto) qDebug() << "pas OK Fronto";
                            }

                            //! On essaie de récupérer une mesure Autoref
                            OKMesureAutoref =   listattributesnames.contains("Vision")
                                              && listattributesnames.contains("CorrectionType")
                                              && listattributesnames.contains("Situation")
                                              && listattributesvalues.contains("Distant")
                                              && listattributesvalues.contains("REF_Base")
                                              && listattributesvalues.contains("Standard");
                            if (OKMesureAutoref && flag.testFlag(MesureAutoref))             //!=> il y a une mesure pour l'autoref et l'autoref est directement branché sur la box du refracteur)
                            {
                                //qDebug() << "OK Autoref" << childnodephoropterit.tagName() << " - " << childnodephoropterit.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                MesureRefraction        *oldMesureAutoref = new MesureRefraction();
                                oldMesureAutoref         ->setdatas(Datas::I()->mesureautoref);
                                Datas::I()->mesureautoref->cleandatas();
                                for (int arfit=0; arfit<childnodephoropterit.childNodes().size(); arfit++)
                                {
                                    QDomElement childnodeautoref = childnodephoropterit.childNodes().at(arfit).toElement();
                                    // OEIL DROIT -----------------------------------------------------------------------------
                                    if (childnodeautoref.tagName() == "R")
                                    {
                                        for (int arfR=0; arfR<childnodeautoref.childNodes().size(); arfR++)
                                        {
                                            QDomElement childnodeitem = childnodeautoref.childNodes().at(arfR).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOD = childnodeitem.text();
                                        }
                                        Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
                                        Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
                                        Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                        Datas::I()->mesureautoref->setaddVPOD(mAddOD.toDouble());
                                    }
                                    // OEIL GAUCHE -----------------------------------------------------------------------------
                                    if (childnodeautoref.tagName() == "L")
                                    {
                                        for (int frG=0; frG<childnodeautoref.childNodes().size(); frG++)
                                        {
                                            QDomElement childnodeitem = childnodeautoref.childNodes().at(frG).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOG = childnodeitem.text();
                                        }
                                        Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
                                        Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
                                        Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                        Datas::I()->mesureautoref->setaddVPOG(mAddOG.toDouble());
                                    }
                                    // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                    if (childnodeautoref.tagName() == "B")
                                        for (int frEIP=0; frEIP<childnodeautoref.childNodes().size(); frEIP++)
                                        {
                                            QDomElement childnodeitem = childnodeautoref.childNodes().at(frEIP).toElement();
                                            if (childnodeitem.tagName() == "PD")
                                            {
                                                PD = childnodeitem.text();
                                                Datas::I()->mesureautoref->setecartIP(PD.toInt());
                                            }
                                        }
                                }
                                if (Datas::I()->mesureautoref->isDifferent(oldMesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
                                {
                                    emit newmesure(MesureAutoref);
                                }
                                delete oldMesureAutoref;
                                //if (!OKMesureAutoref) qDebug() << "pas OK Autoref";
                            }

                            //! On récupère la refraction subjective
                            OKMesureAcuite =   listattributesnames.contains("Vision")
                                             && listattributesnames.contains("CorrectionType")
                                             && listattributesnames.contains("Situation")
                                             && listattributesvalues.contains("Distant")
                                             && listattributesvalues.contains("Full")
                                             && listattributesvalues.contains("Standard");
                            if (OKMesureAcuite)
                            {
                                //qDebug() << "OK Acuite" << childnodephoropter.tagName() << " - " << childnodephoropter.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                Datas::I()->mesureacuite->cleandatas();
                                for (int AVit=0; AVit<childnodephoropterit.childNodes().size(); AVit++)
                                {
                                    QDomElement childnodeAV = childnodephoropterit.childNodes().at(AVit).toElement();
                                    // OEIL DROIT -----------------------------------------------------------------------------
                                    if (childnodeAV.tagName() == "R")
                                        for (int AVR=0; AVR<childnodeAV.childNodes().size(); AVR++)
                                        {
                                            QDomElement childnodeitem = childnodeAV.childNodes().at(AVR).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "VA")
                                                AVLOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "PD")
                                                PDD = childnodeitem.text();
                                        }
                                    // OEIL GAUCHE -----------------------------------------------------------------------------
                                    if (childnodeAV.tagName() == "L")
                                        for (int frG=0; frG<childnodeAV.childNodes().size(); frG++)
                                        {
                                            QDomElement childnodeitem = childnodeAV.childNodes().at(frG).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "VA")
                                                AVLOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "PD")
                                                PDG = childnodeitem.text();
                                        }
                                    // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                }
                                //qDebug() << PDG << PDD;
                                Datas::I()->mesureacuite->setsphereOD(mSphereOD.toDouble());
                                Datas::I()->mesureacuite->setcylindreOD(mCylOD.toDouble());
                                Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                Datas::I()->mesureacuite->setaddVPOD(mAddOD.toDouble());
                                Datas::I()->mesureacuite->setavlOD(AVLOD);
                                Datas::I()->mesureacuite->setsphereOG(mSphereOG.toDouble());
                                Datas::I()->mesureacuite->setcylindreOG(mCylOG.toDouble());
                                Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                Datas::I()->mesureacuite->setaddVPOG(mAddOG.toDouble());
                                Datas::I()->mesureacuite->setavlOG(AVLOG);
                                Datas::I()->mesureacuite->setecartIP(static_cast<int>(std::round(PDD.toDouble() + PDG.toDouble())));
                            }

                            //! On récupère la refraction Finale
                            OKMesureFinal =   listattributesnames.contains("Vision")
                                            && listattributesnames.contains("CorrectionType")
                                            && listattributesnames.contains("Situation")
                                            && listattributesvalues.contains("Distant")
                                            && listattributesvalues.contains("Best")
                                            && listattributesvalues.contains("Standard");
                            if (OKMesureFinal)
                            {
                                PDD = ""; PDG = "";
                                //qDebug() << "OK Final" << childnodephoropter.tagName() << " - " << childnodephoropter.attributes().count() << " - " << listattributesnames << listattributesvalues;
                                Datas::I()->mesurefinal->cleandatas();
                                for (int AVit=0; AVit<childnodephoropterit.childNodes().size(); AVit++)
                                {
                                    QDomElement childnodeAV = childnodephoropterit.childNodes().at(AVit).toElement();
                                    // OEIL DROIT -----------------------------------------------------------------------------
                                    if (childnodeAV.tagName() == "R")
                                        for (int AVR=0; AVR<childnodeAV.childNodes().size(); AVR++)
                                        {
                                            QDomElement childnodeitem = childnodeAV.childNodes().at(AVR).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "VA")
                                                AVLOD = childnodeitem.text();
                                            if (childnodeitem.tagName() == "PD")
                                                PDD = childnodeitem.text();
                                        }
                                    // OEIL GAUCHE -----------------------------------------------------------------------------
                                    if (childnodeAV.tagName() == "L")
                                        for (int frG=0; frG<childnodeAV.childNodes().size(); frG++)
                                        {
                                            QDomElement childnodeitem = childnodeAV.childNodes().at(frG).toElement();
                                            if (childnodeitem.tagName() == "Sphere")
                                                mSphereOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Cylinder")
                                                mCylOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "Axis")
                                                mAxeOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "ADD")
                                                mAddOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "VA")
                                                AVLOG = childnodeitem.text();
                                            if (childnodeitem.tagName() == "PD")
                                                PDG = childnodeitem.text();
                                        }
                                    // ECART INTERPUPILLAIRE -----------------------------------------------------------------------------
                                }
                                Datas::I()->mesurefinal->setsphereOD(mSphereOD.toDouble());
                                Datas::I()->mesurefinal->setcylindreOD(mCylOD.toDouble());
                                Datas::I()->mesurefinal->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                                Datas::I()->mesurefinal->setaddVPOD(mAddOD.toDouble());
                                Datas::I()->mesurefinal->setavlOD(AVLOD);
                                Datas::I()->mesurefinal->setsphereOG(mSphereOG.toDouble());
                                Datas::I()->mesurefinal->setcylindreOG(mCylOG.toDouble());
                                Datas::I()->mesurefinal->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                                Datas::I()->mesurefinal->setaddVPOG(mAddOG.toDouble());
                                Datas::I()->mesurefinal->setavlOG(AVLOG);
                                Datas::I()->mesurefinal->setecartIP(static_cast<int>(std::round(PDD.toDouble() + PDG.toDouble())));
                            }
                        }
                    }
                    //qDebug() << "OK Phoropter";
                }
                //! On essaie de récupérer une mesure de Keratométrie
                if (childnodemeasure.tagName() == "KM" && flag.testFlag(MesureKerato))             //!=> il y a une mesure pour l'autoref et l'autoref est directement branché sur la box du refracteur)
                {
                    Keratometrie  *oldMesureKerato = new Keratometrie();
                    oldMesureKerato->setdatas(Datas::I()->mesurekerato);
                    Datas::I()->mesurekerato->cleandatas();
                    QDomElement childnodekerato = childnodemeasure;
                    for (int KMit=0; KMit<childnodekerato.childNodes().size(); KMit++)
                    {
                        QDomElement childnodekeratoit = childnodekerato.childNodes().at(KMit).toElement();
                        // OEIL DROIT -----------------------------------------------------------------------------
                        if (childnodekeratoit.tagName() == "R")
                        {
                            for (int KMitR=0; KMitR<childnodekeratoit.childNodes().size(); KMitR++)
                            {
                                QDomElement childnodemedian = childnodekeratoit.childNodes().at(KMitR).toElement();
                                if (childnodemedian.tagName() == "Median")
                                    for (int KMitRMed=0; KMitRMed<childnodemedian.childNodes().size(); KMitRMed++)
                                    {
                                        QDomElement childnodeR = childnodemedian.childNodes().at(KMitRMed).toElement();
                                        if (childnodeR.tagName() == "R1")
                                        {
                                            for (int KMitRMedR1=0; KMitRMedR1<childnodeR.childNodes().size(); KMitRMedR1++)
                                            {
                                                QDomElement childnodeR1 = childnodeR.childNodes().at(KMitRMedR1).toElement();
                                                if (childnodeR1.tagName() == "Radius" && childnodeR1.text() !="")
                                                {
                                                    K1OD = childnodeR1.text();
                                                    Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                                                }
                                                if (childnodeR1.tagName() == "Axis" && childnodeR1.text() != "")
                                                {
                                                    AxeKOD = childnodeR1.text().toInt();
                                                    Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                                                }
                                            }
                                        }
                                        if (childnodeR.tagName() == "R2")
                                        {
                                            for (int frR2=0; frR2<childnodeR.childNodes().size(); frR2++)
                                            {
                                                QDomElement childnodeR2 = childnodeR.childNodes().at(frR2).toElement();
                                                if (childnodeR2.tagName() == "Radius" && childnodeR2.text() !="")
                                                {
                                                    K2OD = childnodeR2.text();
                                                    Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                                                }
                                            }
                                        }
                                    }
                            }
                        }
                        // OEIL GAUCHE -----------------------------------------------------------------------------
                        if (childnodekeratoit.tagName() == "L")
                        {
                            for (int KMitL=0; KMitL<childnodekeratoit.childNodes().size(); KMitL++)
                            {
                                QDomElement childnodemedian = childnodekeratoit.childNodes().at(KMitL).toElement();
                                if (childnodemedian.tagName() == "Median")
                                    for (int KMitLMed=0; KMitLMed<childnodemedian.childNodes().size(); KMitLMed++)
                                    {
                                        QDomElement childnodeL = childnodemedian.childNodes().at(KMitLMed).toElement();
                                        if (childnodeL.tagName() == "R1")
                                        {
                                            for (int KMitLMedR1=0; KMitLMedR1<childnodeL.childNodes().size(); KMitLMedR1++)
                                            {
                                                QDomElement childnodeR1 = childnodeL.childNodes().at(KMitLMedR1).toElement();
                                                if (childnodeR1.tagName() == "Radius" && childnodeR1.text() !="")
                                                {
                                                    K1OG = childnodeR1.text();
                                                    Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                                                }
                                                if (childnodeR1.tagName() == "Axis" && childnodeR1.text() != "")
                                                {
                                                    AxeKOG = childnodeR1.text().toInt();
                                                    Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                                                }
                                            }
                                        }
                                        if (childnodeL.tagName() == "R2")
                                        {
                                            for (int KMitLMedR2=0; KMitLMedR2<childnodeL.childNodes().size(); KMitLMedR2++)
                                            {
                                                QDomElement childnodeR2 = childnodeL.childNodes().at(KMitLMedR2).toElement();
                                                if (childnodeR2.tagName() == "Radius" && childnodeR2.text() !="")
                                                {
                                                    K2OG = childnodeR2.text();
                                                    Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                                                }
                                            }
                                        }
                                    }
                            }
                        }
                    }
                    if (Datas::I()->mesurekerato->isDifferent(oldMesureKerato) && !Datas::I()->mesurekerato->isdataclean())
                    {
                        emit newmesure(MesureKerato);
                    }
                    delete oldMesureKerato;
                }
                //! On essaie de récupérer une mesure de tonométrie
                if (childnodemeasure.tagName() == "TM" && flag.testFlag(MesureTono))             //!=> il y a une mesure pour la tono et l'autoref est directement branché sur la box du refracteur)
                {
                    Datas::I()->mesuretono->cleandatas();
                    QDomElement childnodetono = childnodemeasure;
                    for (int TOK=0; TOK<childnodetono.childNodes().size(); TOK++)
                    {
                        QDomElement childnodetonoit = childnodemeasure.childNodes().at(TOK).toElement();
                        // OEIL DROIT -----------------------------------------------------------------------------
                        if (childnodetonoit.tagName() == "R")
                            for (int TOR=0; TOR<childnodetonoit.childNodes().size(); TOR++)
                            {
                                QDomElement childnodeAverage = childnodetonoit.childNodes().at(TOR).toElement();
                                if (childnodeAverage.tagName() == "Average")
                                {
                                    for (int TOAVG=0; TOAVG<childnodeAverage.childNodes().size(); TOAVG++)
                                    {
                                        QDomElement childnodeR = childnodeAverage.childNodes().at(TOAVG).toElement();
                                        if (childnodeR.tagName() == "IOP_mmHg" && childnodeR.text() != "")
                                        {
                                            mTOOD = childnodeR.text();
                                            Datas::I()->mesuretono->setTOD(int(mTOOD.toDouble()));
                                        }
                                    }
                                }
                            }
                        // OEIL GAUCHE -----------------------------------------------------------------------------
                        if (childnodetonoit.tagName() == "L")
                            for (int TOL=0; TOL<childnodetonoit.childNodes().size(); TOL++)
                            {
                                QDomElement childnodeAverage = childnodetonoit.childNodes().at(TOL).toElement();
                                if (childnodeAverage.tagName() == "Average")
                                {
                                    for (int TOAVG=0; TOAVG<childnodeAverage.childNodes().size(); TOAVG++)
                                    {
                                        QDomElement childnodeL = childnodeAverage.childNodes().at(TOAVG).toElement();
                                        if (childnodeL.tagName() == "IOP_mmHg" && childnodeL.text() != "")
                                        {
                                            mTOOG = childnodeL.text();
                                            Datas::I()->mesuretono->setTOG(int(mTOOG.toDouble()));
                                        }
                                    }
                                }
                            }
                    }
                    if (!Datas::I()->mesuretono->isdataclean())
                    {
                        Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
                        //qDebug() << "TOD = " + QString::number(Datas::I()->mesuretono->TOD()) + "mmHg - TOG = " + QString::number(Datas::I()->mesuretono->TOG()) + "mmHg";
                        emit newmesure(MesureTono);                     //! depuis LectureDonneesRefracteur(QString Mesure)
                    }
                }
                //! On essaie de récupérer une mesure de pachymétrie
                if (childnodemeasure.tagName() == "PACHY" && flag.testFlag(MesurePachy))             //!=> il y a une mesure pour la pachy et l'autoref est directement branché sur la box du refracteur)
                {
                    Datas::I()->mesurepachy->cleandatas();
                    for (int pachyK=0; pachyK<childnodemeasure.childNodes().size(); pachyK++)
                    {
                        QDomElement childnodepachy = childnodemeasure.childNodes().at(pachyK).toElement();
                        // OEIL DROIT -----------------------------------------------------------------------------
                        if (childnodepachy.tagName() == "R")
                            for (int pachyR=0; pachyR<childnodepachy.childNodes().size(); pachyR++)
                            {
                                QDomElement childnodeAverage = childnodepachy.childNodes().at(pachyR).toElement();
                                if (childnodeAverage.tagName() == "Average")
                                    for (int pachyAVG=0; pachyAVG<childnodeAverage.childNodes().size(); pachyAVG++)
                                    {
                                        QDomElement childnodeR = childnodeAverage.childNodes().at(pachyAVG).toElement();
                                        if (childnodeR.tagName() == "Thickness" && childnodeR.text() != "")
                                            Datas::I()->mesurepachy->setpachyOD(int(childnodeR.text().toDouble()));
                                    }
                            }
                        // OEIL GAUCHE -----------------------------------------------------------------------------
                        if (childnodepachy.tagName() == "L")
                            for (int pachyL=0; pachyL<childnodepachy.childNodes().size(); pachyL++)
                            {
                                QDomElement childnodeAverage = childnodepachy.childNodes().at(pachyL).toElement();
                                if (childnodeAverage.tagName() == "Average")
                                    for (int pachyAVG=0; pachyAVG<childnodeAverage.childNodes().size(); pachyAVG++)
                                    {
                                        QDomElement childnodeL = childnodeAverage.childNodes().at(pachyAVG).toElement();
                                        if (childnodeL.tagName() == "Thickness" && childnodeL.text() != "")
                                            Datas::I()->mesurepachy->setpachyOG(int(childnodeL.text().toDouble()));
                                    }
                            }
                    }
                    if (!Datas::I()->mesurepachy->isdataclean())
                    {
                        Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
                        //qDebug() << "pachyOD = " + QString::number(Datas::I()->mesurepachy->pachyOD()) + " - pachyOG = " + QString::number(Datas::I()->mesurepachy->pachyOG());
                        emit newmesure(MesurePachy);                     //! depuis LectureDonneesXMLRefracteur(QString Mesure)
                    }
                }
            }
        }
    }

}


void Nidek::RegleRefracteurXML(TypesMesures flag, QString nameRF)
{
    /*! Il faut régler le réfracteur en créant un fichier xml pour l'autoref et un pour le fronto à partir des données du dossier du patient en cours
     * Il faut déposer ces fichiers dans le dossier réseau correspondant surveillé par le refracteur
     */

/*! LE FRONTO */
    bool ExistMesureFronto = flag.testFlag(MesureFronto) && !Datas::I()->mesurefronto->isdataclean();
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
                company.appendChild(LMxml.createTextNode("NIDEK"));
                QDomElement model = LMxml.createElement("ModelName");
                Common.appendChild(model);
                model.appendChild(LMxml.createTextNode("LM-1800PD"));
                Common.appendChild(LMxml.createElement("MachineNo"));
                Common.appendChild(LMxml.createElement("ROMVersion"));
                QDomElement version = LMxml.createElement("Version");
                Common.appendChild(version);
                version.appendChild(LMxml.createTextNode("NIDEK_V1.01"));
                QDomElement date = LMxml.createElement("Date");
                Common.appendChild(date);
                date.appendChild(LMxml.createTextNode(QDate::currentDate().toString(tr("yyyy/MM/dd"))));
                QDomElement time = LMxml.createElement("Time");
                Common.appendChild(time);
                time.appendChild(LMxml.createTextNode(QTime::currentTime().toString(tr("HH:mm:ss"))));
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
        EnregistreFileDatasXML(LMxml, MesureFronto);
    }

/*! L'AUTOREF */
    bool ExistMesureAutoref = flag.testFlag(MesureAutoref) && !Datas::I()->mesureautoref->isdataclean();
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
            QDomText w = ARxml.createTextNode(QDate::currentDate().toString(tr("yyyy/MM/dd")));
            date.appendChild(w);
        }
        QDomElement time = ARxml.createElement("Time");
        Data.appendChild(time);
        {
            QDomText x = ARxml.createTextNode(QTime::currentTime().toString(tr("HH:mm:ss")));
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

        /*! Pour le Glasspop, on met un délai de 3 secondes avant l'envoi des datas refracteur sinon il s'emmêle les crayons s'il y a des données données LM */
        if (nameRF == "NIDEK Glasspop" && ExistMesureFronto)
        {
            QTimer  xmlfiletimer;      /*! permet d'introduire un décalage entre l'enregistrement des données AR ET LM pour le Glasspop pour contourner un bug du Glasspop */
            xmlfiletimer.setSingleShot(true);
            xmlfiletimer.setInterval(3000);
            xmlfiletimer.start();
            connect(&xmlfiletimer, &QTimer::timeout, Q_NULLPTR, [=] { EnregistreFileDatasXML(ARxml, MesureAutoref); });
        }
        else
            EnregistreFileDatasXML(ARxml, MesureAutoref);
    }
}

QByteArray Nidek::OKtoReceive(QString mesure)
{
    /*! la séquence SendData = "\001CRL\002SD\017\004" SendDataNIDEK() est utilisée dans le système NIDEK en réponse à une demande d'envoi de données RequestToSendNIDEK() */
    QByteArray DTRbuff;
    DTRbuff.append(SOH);                            //SOH -> start of header
    DTRbuff.append(mesure.toLocal8Bit());           //CRL pour le refracteur, CLM pour le fronto, CRK ou CRM pour l'autoref
    DTRbuff.append(STX);                            //STX -> start of text
    DTRbuff.append(Utils::StringToArray("SD"));     //SD
    DTRbuff.append(ETB);                            //ETB -> end of text block  -> fin RTS
    DTRbuff.append(EOT);                            //EOT -> end of transmission
    //QByteArray reponse = QString(DTRbuff).toLocal8Bit();
    //reponse += "\r";                                    /*! +++ il faut rajouter \r à la séquence SendDataNIDEK("CRL") sinon ça ne marche pas .... */
    DTRbuff.append(CR);                             // \r = CR
    DTRbuff.replace("""","");
    //qDebug() << "SendDataNidek = " << QString(DTRbuff).toLocal8Bit();
    //return reponse;
    return DTRbuff;
}

QByteArray Nidek::RequestToSend()
{
    /*! la séquence SOH "C**" STX "RS" ETB EOT -> Data a envoyer ="\001C**\002RS\017\004" RequestToSendNIDEK() est utilisée dans le système NIDEK pour signifier qu'on veut envoyer des données
     * Si l'appareil cible est OK, il émet la réponse SendDataNIDEK() = "\001CRL\002SD\027\004\r"
     * pour signifier qu'il est prêt à recevoir les données
     * Dans Rufus, cette demande d'envoi est créée à l'ouverture d'un dossier patient et permet de régler le refracteur sur les données de ce patient */
    QByteArray DTSbuff;
    DTSbuff.append(SOH);                            //SOH -> start of header
    DTSbuff.append(Utils::StringToArray("C**"));    //C**
    DTSbuff.append(STX);                            //STX -> start of text
    DTSbuff.append(Utils::StringToArray("RS"));     //RS
    DTSbuff.append(ETB);                            //ETB -> end of text block  -> fin RTS
    DTSbuff.append(EOT);                            //EOT -> end of transmission
    //qDebug() << "RequestToSendNIDEK() = " << QString(DTSbuff).toLocal8Bit();
    //return  QString(DTSbuff).toLocal8Bit();
    return DTSbuff;
}

void Nidek::LectureDonneesCOMAutoref(QString Mesure, QString nameARK)
{
/*! NIDEK ARK-1a - exemple de fichier de sortie *//*
DrmIDNIDEK/ARK-1a
NO0225
DA27/MAR/2017.07:40
VD12.00
WD35
OL-05.25-00.75080
OR-04.75-00.75105
DL-00.25+00.00+05
DR-00.25+00.00-05
DRMIDNIDEK/ARK-1a
NO0225
DA27/MAR/2017.07:40
VD12.00
WD35
OL-05.00-00.75075
OL-05.00-00.750808 
OL-05.00-00.750758 
OL-05.00-00.750758 
OR-04.50-00.75110
OR-04.50-00.751109 
OR-04.50-00.751109 
OR-04.50-00.751109 
CL-04.75-00.50080
CR-04.25-00.75110
PD68????63
DKM L07.7607.7014007.73
DL43.5043.7514043.75-00.25
 L07.7607.7014007.73
DL43.5043.7514043.75-00.25
 L07.7607.7013507.73
DL43.5043.7513543.75-00.25
 L07.7407.6914507.72
DL43.5044.0014543.75-00.50
 R07.8407.7612007.80
DR43.0043.5012043.25-00.50
 R07.8407.7612507.80
DR43.0043.5012543.25-00.50
 R07.8407.7612007.80
DR43.0043.5012043.25-00.50
 R07.8307.7612007.80
DR43.0043.5012043.25-00.50
SL11.8
PL05.7N
SR11.8
PR05.9N
A40F

*//*! Exemple de fichier de sortie pour un TONOREFIII *//*

DRMIDNIDEK/TONOREF3
NO0002
DA31/MAR/2017.01:02
VD12.00
WD40
OL+00.00-00.250559
OL+00.00-00.250559
OR-00.50-00.25090
OR-00.25-00.500909
OR-00.50-00.250909
OR-00.50-00.250909
CL+00.00-00.25055
CR-00.50-00.25090
TL+00.00-00.25055
TR-00.50-00.25090
PD67????63
DKM
 L07.7607.7000007.73
DL43.5043.7500043.75-00.25
 L07.7707.7018007.74
DL43.5043.7518043.50-00.25
 L07.7607.6900007.73
DL43.5044.0000043.75-00.50
 L07.7607.7000007.73
DL43.5043.7500043.75-00.25
 R07.7207.6700507.70
DR43.7544.0000543.75-00.25
 R07.7207.6700507.70
DR43.7544.0000543.75-00.25
 R07.7207.6700507.70
DR43.7544.0000543.75-00.25
 R07.7307.6700507.70
DR43.7544.0000543.75-00.25
Dkm
 L07.7507.6401007.70
DL43.5044.2501043.75-00.75
 L07.7607.6501007.71
DL43.5044.0001043.75-00.50
 L07.7507.6401007.70
DL43.5044.2501043.75-00.75
 L07.7407.6301007.69
DL43.5044.2501044.00-00.75
 R07.7007.6200007.66
DR43.7544.2500044.00-00.50
 R07.6907.6100007.65
DR44.0044.2500044.00-00.25
 R07.7007.6200007.66
DR43.7544.2500044.00-00.50
 R07.7207.6300507.68
DR43.7544.2500544.00-00.50
SL12.2
PL05.0N
SR12.2
PR05.5N
DNT
 L01    17/2.3  AV17.0/2.30
DPM
 L01 0583  AV0583
 R01 0586  AV0586

*//*! Exemple de fichier de sortie avec seul l'OG mesuré pour un ARK1A *//*

Drm
0002IDNIDEK/ARK-1a
NO0272
DA10/APR/2017.08:58
VD12.00
WD35
OL+00.00-01.25095
DL+00.25-00.50-05
DRM
NIDEK/ARK-1a
NO0272
DA10/APR/2017.08:58
VD12.00
WD35
OL-00.25-00.75100
OL-00.25-00.751008
OL-00.25-00.751008
OL-00.25-00.751008
CL-00.25-00.75100
CR+00.00+00.00000
DKM
 L07.8907.8214507.86
DL42.7543.2514543.00-00.50
 L07.8807.8314507.86
DL42.7543.0014543.00-00.25
 L07.8907.8214007.86
DL42.7543.2514043.00-00.50
 L07.8907.8214507.86
DL42.7543.2514543.00-00.50
SL11.7
PL04.7N
000461E4
*/
    int     a(0);
    bool autorefhaskerato    = (nameARK == "NIDEK ARK-1A"
                             || nameARK == "NIDEK ARK-1"
                             || nameARK == "NIDEK ARK-1S"
                             || nameARK == "NIDEK ARK-530A"
                             || nameARK == "NIDEK ARK-510A"
                             || nameARK == "NIDEK HandyRef-K"
                             || nameARK == "NIDEK TONOREF III"
                             || nameARK == "NIDEK ARK-30");
    bool autorefhastonopachy = (nameARK == "NIDEK TONOREF III");
    bool autorefhasipmesure = (nameARK != "NIDEK HandyRef-K"
                               || nameARK != "NIDEK ARK-30"
                               || nameARK != "NIDEK AR-20");

    a               = Mesure.indexOf("VD");
    a               = Mesure.length() - a -1;
    QString Ref("");
    Ref             = Mesure.right(a);
    if (Ref != "")
    {
        a  = Ref.indexOf("OR");
        // OEIL DROIT -----------------------------------------------------------------------------
        if (a>=0)
        {
            QString mesureOD("");
            QString mSphereOD   = "+00.00";
            QString mCylOD      = "+00.00";
            QString mAxeOD      = "000";
            mesureOD            = Ref.mid(Ref.indexOf("OR")+2,15)   .replace(" ","0");
            mSphereOD           = mesureOD.mid(0,6);
            mCylOD              = mesureOD.mid(6,6);
            mAxeOD              = mesureOD.mid(12,3);
            Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        a  = Ref.indexOf("OL");
        if (a>=0)
        {
            QString mesureOG("");
            QString mSphereOG   = "+00.00";
            QString mCylOG      = "+00.00";
            QString mAxeOG      = "000";
            mesureOG            = Ref.mid(Ref.indexOf("OL")+2,15)   .replace(" ","0");
            mSphereOG           = mesureOG.mid(0,6);
            mCylOG              = mesureOG.mid(6,6);
            mAxeOG              = mesureOG.mid(12,3);
            Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
        }
        if (autorefhasipmesure)
        {
            a  = Ref.indexOf("PD");
            if (a >= 0) {
                QString PD      = Ref.mid(Ref.indexOf("PD")+2,2);
                Datas::I()->mesureautoref->setecartIP(PD.toInt());
            }
        }
        if (autorefhaskerato)
        {
            // Données de KERATOMETRIE --------------------------------------------------------------------------------------------------------
            if (Mesure.contains("DKM"))                 //=> il y a une mesure de keratometrie
            {
                a                   = Mesure.indexOf("DKM");
                a                   = Mesure.length() - a;
                QString K           = Mesure.right(a);
                // OEIL DROIT -----------------------------------------------------------------------------
                a  = Ref.indexOf(" R");
                if (a>=0)
                {
                    QString KOD("");
                    QString K1OD("null"), K2OD("null");
                    int     AxeKOD(0);
                    KOD                 = K.mid(K.indexOf(" R")+2,13);
                    K1OD                = KOD.mid(0,5);
                    K2OD                = KOD.mid(5,5);
                    AxeKOD              = KOD.mid(10,3).toInt();
                    Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                    Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                    Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                    QString mOD         = K.mid(K.indexOf("DR")+2,10).replace(" ","0");
                    Datas::I()->mesurekerato->setdioptriesK1OD(mOD.mid(0,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OD(mOD.mid(5,5).toDouble());
                }
                // OEIL GAUCHE ---------------------------------------------------------------------------
                a  = Ref.indexOf(" L");
                if (a>=0)
                {
                    QString KOG("");
                    QString K1OG("null"), K2OG("null");
                    int     AxeKOG(0);
                    KOG                 = K.mid(K.indexOf(" L")+2,13);
                    K1OG                = KOG.mid(0,5);
                    K2OG                = KOG.mid(5,5);
                    AxeKOG              = KOG.mid(10,3).toInt();
                    Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                    Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                    Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                    QString mOG         = K.mid(K.indexOf("DL")+2,10).replace(" ","0");
                    Datas::I()->mesurekerato->setdioptriesK1OG(mOG.mid(0,5).toDouble());
                    Datas::I()->mesurekerato->setdioptriesK2OG(mOG.mid(5,5).toDouble());
                }
            }
        }
    }
    if (autorefhastonopachy)
    {
        // Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
        if (Mesure.contains("DNT"))                 //=> il y a une mesure de tonometrie
        {
            QString TonoOD(""), TonoOG("");
            a                       = Mesure.indexOf("DNT");
            a                       = Mesure.length() - a -1;
            QString Tono            = Mesure.right(a);
            int b                   = Tono.indexOf("DPM");
            if (b>=0)
                Tono                = Tono.left(b-1);
            a = Tono.indexOf(" R");
            if (a>=0)
            {
                TonoOD          = Tono.right(Tono.length()-a-1);
                a               = TonoOD.indexOf("AV");
                TonoOD          = TonoOD.mid(a+2,2).replace(" ","0");
            }
            b= Tono.indexOf(" L");
            if (b>=0)
            {
                TonoOG          = Tono.right(Tono.length()-b-1);
                b               = TonoOG.indexOf("AV");
                TonoOG          = TonoOG.mid(b+2,2).replace(" ","0");
            }
            Datas::I()->mesuretono->setTOD(TonoOD.toInt());
            Datas::I()->mesuretono->setTOG(TonoOG.toInt());
            Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
        }
        // Données de PACHYMETRIE --------------------------------------------------------------------------------------------------------
        if (Mesure.contains("DPM"))                 //=> il y a une mesure de pachymetrie
        {
            QString PachyOD(""), PachyOG("");
            a                       = Mesure.indexOf("DPM");
            a                       = Mesure.length() - a;
            QString Pachy           = Mesure.right(a);
            if (Pachy.indexOf(" R")>=0)
                PachyOD         = Pachy.mid(Pachy.indexOf(" R")+6,3);
            if (Pachy.indexOf(" L")>=0)
                PachyOG         = Pachy.mid(Pachy.indexOf(" L")+6,3);
            Datas::I()->mesurepachy->setpachyOD(PachyOD.toInt());
            Datas::I()->mesurepachy->setpachyOG(PachyOG.toInt());
            Datas::I()->mesurepachy->setmodemesure(Pachymetrie::Optique);
        }
    }
}

void Nidek::LectureDonneesCOMFronto(QString Mesure)
{
    //qDebug() << "LectureDonneesCOMFronto(QString Mesure)" << Mesure;
    //Edit(Mesure);
    QString mSphereOD   = "+00.00";
    QString mCylOD      = "+00.00";
    QString mAxeOD      = "000";
    QString mSphereOG   = "+00.00";
    QString mCylOG      = "+00.00";
    QString mAxeOG      = "000";
    QString mAddOD      = "+0.00";
    QString mAddOG      = "+0.00";
    QString mesureOD, mesureOG;
    {
        // OEIL DROIT -----------------------------------------------------------------------------
        int idxOD = Mesure.indexOf(" R");
        if (idxOD > 0)
        {
            mesureOD            = Mesure.mid(idxOD+2,15)   .replace(" ","0");
            mSphereOD            = mesureOD.mid(0,6);
            mCylOD               = mesureOD.mid(6,6);
            mAxeOD               = mesureOD.mid(12,3);
            if (Mesure.indexOf("AR")>0)
                mAddOD           = Mesure.mid(Mesure.indexOf("AR")+2,5);
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        int idxOG = Mesure.indexOf(" L");
        if (idxOG > 0)
        {
            mesureOG            = Mesure.mid(idxOG+2,15)   .replace(" ","0");
            mSphereOG            = mesureOG.mid(0,6);
            mCylOG               = mesureOG.mid(6,6);
            mAxeOG               = mesureOG.mid(12,3);
            if (Mesure.indexOf("AL")>0)
                mAddOG           = Mesure.mid(Mesure.indexOf("AL")+2,5);
            Datas::I()->mesurefronto->setsphereOG(mSphereOG.toDouble());
            Datas::I()->mesurefronto->setcylindreOG(mCylOG.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
            Datas::I()->mesurefronto->setaddVPOG(mAddOG.toDouble());
        }
    }
}

void Nidek::LectureDonneesCOMRefracteur(QString Mesure, TypesMesures flag)
{
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
NIDEK RT-5100 ID             DA2016/12/30
@LM
 R+ 1.50- 1.25 15
 L+ 2.50- 1.00155
@RM
OR+ 1.25- 1.75170
OL+ 1.25- 0.50 20
PD57.530.527.0
@RT
fR- 1.50- 1.00 40
fL+ 2.50- 1.50135
nR+ 1.50- 1.00 40
nL+ 5.50- 1.50135
aR+ 3.00
aL+ 2.75
vR 1.25
vL 0.70
pD57.530.527.0
@NT
TR25.3
TL22.1
*/

    //! Données du FRONTO ---------------------------------------------------------------------------------------------------------------------
    if (flag.testFlag(MesureFronto))             //!=> il y a une mesure pour le fronto et le fronto est directement branché sur la box du refracteur
    {
        MesureRefraction        *oldMesureFronto = new MesureRefraction();
        oldMesureFronto         ->setdatas(Datas::I()->mesurefronto);
        Datas::I()->mesurefronto->cleandatas();
        idx                     = Mesure.indexOf("@LM");
        QString SectionFronto   = Mesure.right(Mesure.length()-idx);
        //Edit(SectionFronto + "\nOK");
        // OEIL DROIT -----------------------------------------------------------------------------
        if (SectionFronto.contains(" R"))
        {
            mesureOD     = SectionFronto.mid(SectionFronto.indexOf(" R")+2,15)   .replace(" ","0");
            mSphereOD    = mesureOD.mid(0,6);
            mCylOD       = mesureOD.mid(6,6);
            mAxeOD       = mesureOD.mid(12,3);
            mAddOD       = SectionFronto.mid(SectionFronto.indexOf("AR")+2,6)    .replace(" ","0");
            Datas::I()->mesurefronto->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesurefronto->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesurefronto->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
            Datas::I()->mesurefronto->setaddVPOD(mAddOD.toDouble());
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (SectionFronto.contains(" L"))
        {
            mesureOG     = SectionFronto.mid(SectionFronto.indexOf(" L")+2,15)   .replace(" ","0");
            mSphereOG    = mesureOG.mid(0,6);
            mCylOG       = mesureOG.mid(6,6);
            mAxeOG       = mesureOG.mid(12,3);
            mAddOG       = SectionFronto.mid(SectionFronto.indexOf("AL")+2,6)    .replace(" ","0");
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
    if (flag.testFlag(MesureKerato))                 //!=> il y a une mesure de keratométrie et l'autoref est connecté directement à la box du refraacteur
    {
        Keratometrie  *oldMesureKerato = new Keratometrie();
        oldMesureKerato->setdatas(Datas::I()->mesurekerato);
        Datas::I()->mesurekerato->cleandatas();
        idx                     = Mesure.indexOf("@KM");
        QString SectionKerato   = Mesure.right(Mesure.length()-idx);
        //Edit(SectionKerato + "\nOK");
        // OEIL DROIT -----------------------------------------------------------------------------
        if (SectionKerato.contains("CR"))
        {
            mesureOD            = SectionKerato.mid(SectionKerato.indexOf("CR")+2,13)   .replace(" ","0");
            K1OD                = mesureOD.mid(0,5);
            K2OD                = mesureOD.mid(5,5);
            AxeKOD              = mesureOD.mid(10,3).toInt();
            Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
            Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
            Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
            if (SectionKerato.contains("DR"))
            {
                mesureOD        = SectionKerato.mid(SectionKerato.indexOf("DR")+2,10)   .replace(" ","0");
                Datas::I()->mesurekerato->setdioptriesK1OD(mesureOD.mid(0,5).toDouble());
                Datas::I()->mesurekerato->setdioptriesK2OD(mesureOD.mid(5,5).toDouble());
            }
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (SectionKerato.contains("CL"))
        {
            mesureOG            = SectionKerato.mid(SectionKerato.indexOf("CL")+2,13)   .replace(" ","0");
            K1OG                = mesureOG.mid(0,5);
            K2OG                = mesureOG.mid(5,5);
            AxeKOG              = mesureOG.mid(10,3).toInt();
            Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
            Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
            Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
            if (SectionKerato.contains("DL"))
            {
                mesureOG        = SectionKerato.mid(SectionKerato.indexOf("DL")+2,10)   .replace(" ","0");
                Datas::I()->mesurekerato->setdioptriesK1OG(mesureOG.mid(0,5).toDouble());
                Datas::I()->mesurekerato->setdioptriesK2OG(mesureOG.mid(5,5).toDouble());
            }
        }
        if (Datas::I()->mesurekerato->isDifferent(oldMesureKerato) && !Datas::I()->mesurekerato->isdataclean())
            emit newmesure(MesureKerato);
        delete oldMesureKerato;
    }
    if (flag.testFlag(MesureAutoref))                 //!=> il y a une mesure de refractometrie et l'autoref est directement branché sur la box du refracteur
    {
        MesureRefraction        *oldMesureAutoref = new MesureRefraction();
        oldMesureAutoref        ->setdatas(Datas::I()->mesureautoref);
        Datas::I()->mesureautoref->cleandatas();
        idx                     = Mesure.indexOf("@RM");
        QString SectionAutoref  = Mesure.right(Mesure.length()-idx);
        //Edit(SectionAutoref);
        // OEIL DROIT -----------------------------------------------------------------------------
        if (SectionAutoref.contains("OR"))
        {
            mesureOD     = SectionAutoref.mid(SectionAutoref.indexOf("OR")+2,15)   .replace(" ","0");
            mSphereOD    = mesureOD.mid(0,6);
            mCylOD       = mesureOD.mid(6,6);
            mAxeOD       = mesureOD.mid(12,3);
            Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
            Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
            Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (SectionAutoref.contains("OL"))
        {
            mesureOG     = SectionAutoref.mid(SectionAutoref.indexOf("OL")+2,15)   .replace(" ","0");
            mSphereOG    = mesureOG.mid(0,6);
            mCylOG       = mesureOG.mid(6,6);
            mAxeOG       = mesureOG.mid(12,3);
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
    if (flag.testFlag(MesureRefracteur))                 //=> il y a une mesure de refraction
    {
        idx                         = Mesure.indexOf("@RT");
        QString SectionRefracteur   = Mesure.right(Mesure.length()-idx);
        //qDebug() << "Procedures::LectureDonneesRefracteur(QString Mesure) - SectionRefracteur = " << SectionRefracteur;

        // les données subjectives --------------------------------------------------------------------------------------------------------------
        if (SectionRefracteur.contains("fR") || SectionRefracteur.contains("fL"))
        {
            PD               = SectionRefracteur.mid(SectionRefracteur.indexOf("pD")+2,2);
            Datas::I()->mesureacuite->setecartIP(PD.toInt());
            // OEIL DROIT -----------------------------------------------------------------------------
            if (SectionRefracteur.contains("fR"))
            {
                mesureOD     = SectionRefracteur.mid(SectionRefracteur.indexOf("fR")+2,15)   .replace(" ","0");
                mSphereOD    = mesureOD.mid(0,6);
                mCylOD       = mesureOD.mid(6,6);
                mAxeOD       = mesureOD.mid(12,3);
                if (SectionRefracteur.indexOf("aR")>-1)
                    mAddOD   = SectionRefracteur.mid(SectionRefracteur.indexOf("aR")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("vR")>-1)
                    AVLOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("vR")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("yR")>-1)
                    AVPOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("yR")+2,5)    .replace(" ","0");
                Datas::I()->mesureacuite->setsphereOD(mSphereOD.toDouble());
                Datas::I()->mesureacuite->setcylindreOD(mCylOD.toDouble());
                Datas::I()->mesureacuite->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                Datas::I()->mesureacuite->setaddVPOD(mAddOD.toDouble());
                Datas::I()->mesureacuite->setavlOD(AVLOD);
                Datas::I()->mesureacuite->setavpOD(AVPOD);
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionRefracteur.contains("fL"))
            {
                mesureOG     = SectionRefracteur.mid(SectionRefracteur.indexOf("fL")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
                if (SectionRefracteur.indexOf("aL")>-1)
                    mAddOG   = SectionRefracteur.mid(SectionRefracteur.indexOf("aL")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("vL")>-1)
                    AVLOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("vL")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("yL")>-1)
                    AVPOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("yL")+2,5)    .replace(" ","0");
                Datas::I()->mesureacuite->setsphereOG(mSphereOG.toDouble());
                Datas::I()->mesureacuite->setcylindreOG(mCylOG.toDouble());
                Datas::I()->mesureacuite->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                Datas::I()->mesureacuite->setaddVPOG(mAddOG.toDouble());
                Datas::I()->mesureacuite->setavlOG(AVLOG);
                Datas::I()->mesureacuite->setavpOG(AVPOG);
            }
        }

        // les données finales --------------------------------------------------------------------------------------------------------------
        if (SectionRefracteur.contains("FR") || SectionRefracteur.contains("FL"))
        {
            PD               = SectionRefracteur.mid(SectionRefracteur.indexOf("PD")+2,2);
            Datas::I()->mesurefinal->setecartIP(PD.toInt());
            // OEIL DROIT -----------------------------------------------------------------------------
            if (SectionRefracteur.contains("FR"))
            {
                mesureOD     = SectionRefracteur.mid(SectionRefracteur.indexOf("FR")+2,15)   .replace(" ","0");
                mSphereOD    = mesureOD.mid(0,6);
                mCylOD       = mesureOD.mid(6,6);
                mAxeOD       = mesureOD.mid(12,3);
                if (SectionRefracteur.indexOf("AR")>-1)
                    mAddOD   = SectionRefracteur.mid(SectionRefracteur.indexOf("AR")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("VR")>-1)
                    AVLOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("VR")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("YR")>-1)
                    AVPOD    = SectionRefracteur.mid(SectionRefracteur.indexOf("YR")+2,5)    .replace(" ","0");

                Datas::I()->mesurefinal->setsphereOD(mSphereOD.toDouble());
                Datas::I()->mesurefinal->setcylindreOD(mCylOD.toDouble());
                Datas::I()->mesurefinal->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                Datas::I()->mesurefinal->setaddVPOD(mAddOD.toDouble());
                Datas::I()->mesurefinal->setavlOD(AVLOD);
                Datas::I()->mesurefinal->setavpOD(AVPOD);
            }
            // OEIL GAUCHE ---------------------------------------------------------------------------
            if (SectionRefracteur.contains("FL"))
            {
                mesureOG     = SectionRefracteur.mid(SectionRefracteur.indexOf("FL")+2,15)   .replace(" ","0");
                mSphereOG    = mesureOG.mid(0,6);
                mCylOG       = mesureOG.mid(6,6);
                mAxeOG       = mesureOG.mid(12,3);
                if (SectionRefracteur.indexOf("AL")>-1)
                    mAddOG   = SectionRefracteur.mid(SectionRefracteur.indexOf("AL")+2,6)    .replace(" ","0");
                if (SectionRefracteur.indexOf("VL")>-1)
                    AVLOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("VL")+2,5)    .replace(" ","0");
                if (SectionRefracteur.indexOf("YL")>-1)
                    AVPOG    = SectionRefracteur.mid(SectionRefracteur.indexOf("YL")+2,5)    .replace(" ","0");

                Datas::I()->mesurefinal->setsphereOG(mSphereOG.toDouble());
                Datas::I()->mesurefinal->setcylindreOG(mCylOG.toDouble());
                Datas::I()->mesurefinal->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                Datas::I()->mesurefinal->setaddVPOG(mAddOG.toDouble());
                Datas::I()->mesurefinal->setavlOG(AVLOG);
                Datas::I()->mesurefinal->setavpOG(AVPOG);
            }
        }
    }

    //! Données de TONOMETRIE --------------------------------------------------------------------------------------------------------
    if (flag.testFlag(MesureTono))                 //!=> il y a une mesure de tonometrie et l'autoref est branché sur la box du refracteur
    {
        Datas::I()->mesuretono->cleandatas();
        idx                     = Mesure.indexOf("@NT");
        QString SectionTono     = Mesure.right(Mesure.length()-idx-5);
        SectionTono             = SectionTono.left(SectionTono.indexOf("@"));
        logmesure("LectureDonneesRefracteur() - " + SectionTono);
        //Edit(SectionTono+ "\nOK");
        // OEIL DROIT -----------------------------------------------------------------------------
        mTOOD                   = SectionTono.mid(SectionTono.indexOf("TR")+2,4)   .replace(" ","0");
        // OEIL GAUCHE ---------------------------------------------------------------------------
        mTOOG                   = SectionTono.mid(SectionTono.indexOf("TL")+2,4)   .replace(" ","0");
        logmesure("LectureDonneesRefracteur() - Section Tono - mTOOD = " + mTOOD + " - mTOOG = " + mTOOG);
        Datas::I()->mesuretono->setTOD(int(mTOOD.toDouble()));
        Datas::I()->mesuretono->setTOG(int(mTOOG.toDouble()));
        Datas::I()->mesuretono->setmodemesure(Tonometrie::Air);
        logmesure("LectureDonneesRefracteur() - nouvelle mesure tono -> TOD = " + QString::number(Datas::I()->mesuretono->TOD()) + " - TOG = " + QString::number(Datas::I()->mesuretono->TOG()));
        emit newmesure(MesureTono);                     //! depuis Nidek::LectureDonneesRefracteur(QString Mesure)
    }
}

QByteArray Nidek::RegleRefracteurCOM(TypesMesures flag)
{
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
    auto convertdioptriesNIDEK = [&] (QString &finalvalue, double originvalue)
    {
        if (originvalue > 0)
            finalvalue = (originvalue < 10? "+0" : "+") + QString::number(originvalue,'f',2);
        else if (originvalue < 0)
            finalvalue = (originvalue > -10? QString::number(originvalue,'f',2).replace("-", "-0") : QString::number(originvalue,'f',2));
    };
    auto convertaxeNIDEK = [&] (QString &finalvalue, int originvalue)
    {
        if (originvalue < 10)       finalvalue = "  " + QString::number(originvalue);
        else if (originvalue < 100) finalvalue = " "  + QString::number(originvalue);
        else                        finalvalue = QString::number(originvalue);
    };
    DTRbuff.append(SOH);                                    //SOH -> start of header

    /*! réglage de l'autoref */
    if (flag.testFlag(MesureAutoref) && !Datas::I()->mesureautoref->isdataclean())
    {
        initvariables();
        convertaxeNIDEK(AxeOD, Datas::I()->mesureautoref->axecylindreOD());
        convertaxeNIDEK(AxeOG, Datas::I()->mesureautoref->axecylindreOG());
        convertdioptriesNIDEK(SphereOD, Datas::I()->mesureautoref->sphereOD());
        convertdioptriesNIDEK(SphereOG, Datas::I()->mesureautoref->sphereOG());
        convertdioptriesNIDEK(CylindreOD, Datas::I()->mesureautoref->cylindreOD());
        convertdioptriesNIDEK(CylindreOG, Datas::I()->mesureautoref->cylindreOG());

        SCAOD       = SphereOD + CylindreOD + AxeOD;
        SCAOG       = SphereOG + CylindreOG + AxeOG;
        SCAOD.replace("+0","+ ");
        SCAOD.replace("-0","- ");
        SCAOG.replace("+0","+ ");
        SCAOG.replace("-0","- ");
        DTRbuff.append(Utils::StringToArray("DRM"));        //section fronto
        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray("OR"+ SCAOD));  //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        DTRbuff.append(Utils::StringToArray("OL"+ SCAOG));  //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        if (Datas::I()->mesureautoref->ecartIP() > 0)
        {
            DTRbuff.append(Utils::StringToArray("PD"+ QString::number(Datas::I()->mesureautoref->ecartIP())));  //SD
            DTRbuff.append(ETB);                            //ETB -> end of text block
        }
    }

    /*! réglage du fronto */
    if (flag.testFlag(MesureFronto) && !Datas::I()->mesurefronto->isdataclean())
    {
        initvariables();

        convertaxeNIDEK(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
        convertaxeNIDEK(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
        convertdioptriesNIDEK(SphereOD, Datas::I()->mesurefronto->sphereOD());
        convertdioptriesNIDEK(SphereOG, Datas::I()->mesurefronto->sphereOG());
        convertdioptriesNIDEK(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
        convertdioptriesNIDEK(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

        SCAOD       = SphereOD + CylindreOD + AxeOD;
        SCAOG       = SphereOG + CylindreOG + AxeOG;
        SCAOD.replace("+0","+ ");
        SCAOD.replace("-0","- ");
        SCAOG.replace("+0","+ ");
        SCAOG.replace("-0","- ");
        AddOD       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2);
        AddOG       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2);
        DTRbuff.append(Utils::StringToArray("DLM"));        //section fronto
        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray(" R"+ SCAOD));  //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        DTRbuff.append(Utils::StringToArray(" L"+ SCAOG));  //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        DTRbuff.append(Utils::StringToArray("AR" + AddOD)); //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        DTRbuff.append(Utils::StringToArray("AL" + AddOG)); //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        if (Datas::I()->mesurefronto->ecartIP() > 0)
        {
            DTRbuff.append(Utils::StringToArray("PD"+ QString::number(Datas::I()->mesurefronto->ecartIP())));   //SD
            DTRbuff.append(ETB);                            //ETB -> end of text block
        }
    }
    DTRbuff.append(EOT);                                    //EOT -> end of transmission

    /*! réglage du fronto */
    if (flag.testFlag(MesureFronto) && !Datas::I()->mesurefronto->isdataclean())
    {
        initvariables();

        convertaxeNIDEK(AxeOD, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOD()));
        convertaxeNIDEK(AxeOG, Utils::roundToNearestFive(Datas::I()->mesurefronto->axecylindreOG()));
        convertdioptriesNIDEK(SphereOD, Datas::I()->mesurefronto->sphereOD());
        convertdioptriesNIDEK(SphereOG, Datas::I()->mesurefronto->sphereOG());
        convertdioptriesNIDEK(CylindreOD, Datas::I()->mesurefronto->cylindreOD());
        convertdioptriesNIDEK(CylindreOG, Datas::I()->mesurefronto->cylindreOG());

        SCAOD       = SphereOD + CylindreOD + AxeOD;
        SCAOG       = SphereOG + CylindreOG + AxeOG;
        SCAOD.replace("+0","+ ");
        SCAOD.replace("-0","- ");
        SCAOG.replace("+0","+ ");
        SCAOG.replace("-0","- ");
        AddOD       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOD(),'f',2);
        AddOG       = "+ " + QString::number(Datas::I()->mesurefronto->addVPOG(),'f',2);
        DTRbuff.append(Utils::StringToArray("DLM"));        //section fronto
        DTRbuff.append(STX);                                //STX -> start of text
        DTRbuff.append(Utils::StringToArray(" R"+ SCAOD));  //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        DTRbuff.append(Utils::StringToArray(" L"+ SCAOG));  //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        DTRbuff.append(Utils::StringToArray("AR" + AddOD)); //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        DTRbuff.append(Utils::StringToArray("AL" + AddOG)); //SD
        DTRbuff.append(ETB);                                //ETB -> end of text block
        if (Datas::I()->mesurefronto->ecartIP() > 0)
        {
            DTRbuff.append(Utils::StringToArray("PD"+ QString::number(Datas::I()->mesurefronto->ecartIP())));   //SD
            DTRbuff.append(ETB);                            //ETB -> end of text block
        }
    }
    DTRbuff.append(EOT);                                    //EOT -> end of transmission

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
