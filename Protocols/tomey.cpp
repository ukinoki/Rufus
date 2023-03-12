/* (C) 2023 LAINE SERGE
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

#include "tomey.h"
#include "gbl_datas.h"

Tomey* Tomey::instance = Q_NULLPTR;

Tomey* Tomey::I()
{
    if( !instance )
        instance = new Tomey();
    return instance;
}

Tomey::Tomey() {}

QList<QString> getFieldsFromLine(QList<QString> list, int i)
{
    QString line = list.at(i); // Get line
    QList<QString> fields = line.split(",");
    return fields;
}


void Tomey::LectureDonneesRC5000Form(QString Mesure, QString nameARK)
{
    QList<QString> list= Mesure.split("\r\n"); //split in lines

    QString mode = "";
    int nlines = list.length();
    for( int i =0; i< nlines; i++ )
    {
        QList<QString> fields = getFieldsFromLine(list, i);
        if ( fields.at(0) == "[FM_IF]")
        {
            mode=fields.at(1); // REF ou KERATO
        }
        else if ( fields.at(0) == "[PD]" && mode == "REF")
        {
           if (  fields.length() >1)
           {
               bool ok;
               double PD = fields.at(1).toDouble(&ok);
               if( ok ) {
                   Datas::I()->mesureautoref->setecartIP(int(PD));
               }
           }
        }
        // OEIL DROIT -------------------------------------------------------- [FM_IF], REF
        else if ( fields.at(0) == "[N_R]" && mode == "REF")
        {
            bool ok;
            int n = fields.at(1).toInt(&ok);
            if ( ok && i+n < nlines )
            {
                for( int k=i; k<i+n; k++)
                {
                    QList<QString> fields2 = getFieldsFromLine(list, k+1); //get measure
                    if ( fields2.at(0) == "[POWER_R]" && fields2.at(1) == "A" && fields2.length() > 5)  // Average
                    {
                        QString mSphereOD   = fields2.at(2);
                        QString mCylOD      = fields2.at(3);
                        QString mAxeOD      = fields2.at(4);
                        Datas::I()->mesureautoref->setsphereOD(mSphereOD.toDouble());
                        Datas::I()->mesureautoref->setcylindreOD(mCylOD.toDouble());
                        Datas::I()->mesureautoref->setaxecylindreOD(Utils::roundToNearestFive(mAxeOD.toInt()));
                        break;
                    }
                }
                i+=n; //skip all measures
            }
        }
        // OEIL GAUCHE ------------------------------------------------------- [FM_IF], REF
        else if ( fields.at(0) == "[N_L]" && mode == "REF")
        {
            bool ok;
            int n = fields.at(1).toInt(&ok);
            if ( ok && i+n < nlines )
            {
                for( int k=i; k<i+n; k++)
                {
                    QList<QString> fields2 = getFieldsFromLine(list, k+1); //get measure
                    if ( fields2.at(0) == "[POWER_L]" && fields2.at(1) == "A" && fields2.length() > 5)  // Average
                    {
                        QString mSphereOG   = fields2.at(2);
                        QString mCylOG      = fields2.at(3);
                        QString mAxeOG      = fields2.at(4);
                        Datas::I()->mesureautoref->setsphereOG(mSphereOG.toDouble());
                        Datas::I()->mesureautoref->setcylindreOG(mCylOG.toDouble());
                        Datas::I()->mesureautoref->setaxecylindreOG(Utils::roundToNearestFive(mAxeOG.toInt()));
                        break;
                    }
                }
                i+=n; //skip all measures
            }
        }
        // OEIL DROIT -------------------------------------------------------- [FM_IF], KERATO
        // mode [FM_IF],KERATO,1-03-00
        // i    [N_R],4,1
        // i+1  [INF_R],1,
        // i+2  [K1_R], 8.38, 40.27,16
        // i+3  [K2_R], 8.19, 41.21,106
        // i+4  [AV_R], 8.29, 40.71
        // i+5  [CYL_R],-0.94,16
        // i+6  [KAI_R],  4.0,343,A,           --------------------- Rodenstock CX 2000 Not in Tomey Form specification.
        // i+7  [KRI_R],  2.0,A,               ---------------------  idem
        else if ( fields.at(0) == "[N_R]" && mode == "KERATO")
        {
            bool ok;
            i+=1;
            // read lines until found [INF_R],A,
            while ( i < nlines ){
                QList<QString> fields2 = getFieldsFromLine(list, i);
                if ( fields2.at(0) == "[INF_R]" && fields2.length() > 1)
                {
                    if(fields2.at(1) == "A")
                    {
                        i+=1;
                        while(i < nlines) {
                            fields2 = getFieldsFromLine(list, i);
                            if ( fields2.at(0) == "[K1_R]" and fields2.length() > 3)
                            {
                                double K1OD = fields2.at(1).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setK1OD(K1OD);
                                }
                                double K1DOD = fields2.at(2).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setdioptriesK1OD(K1DOD);
                                }
                                int    AxeKOD = fields2.at(3).toInt(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                                }
                            }
                            else if ( fields2.at(0) == "[K2_R]" and fields2.length() > 2)
                            {
                                double K2OD = fields2.at(1).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setK2OD(K2OD);
                                }
                                double K2ODD = fields2.at(2).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setdioptriesK2OD(K2ODD);
                                }
                            } else {
                                break;
                            }
                            i+=1;
                        }
                        break;
                    }
                }
                i+=1;
            }
        }
        // OEIL GAUCHE -------------------------------------------------------- [FM_IF], KERATO
        else if ( fields.at(0) == "[N_L]" && mode == "KERATO")
        {
            bool ok;
            // read lines until found [INF_L],A,
            i+=1;
            while ( i < nlines ){
                QList<QString> fields2 = getFieldsFromLine(list, i);
                if ( fields2.at(0) == "[INF_L]" && fields2.length() > 1)
                {
                    if(fields2.at(1) == "A")
                    {
                        i+=1;
                        while(i < nlines) {
                            fields2 = getFieldsFromLine(list, i);
                            if ( fields2.at(0) == "[K1_L]" and fields2.length() > 3)
                            {
                                double K1OG = fields2.at(1).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setK1OG(K1OG);

                                }
                                double K1DOG = fields2.at(2).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setdioptriesK1OG(K1DOG);
                                }
                                int    AxeKOG = fields2.at(3).toInt(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                                }
                            }
                            else if ( fields2.at(0) == "[K2_L]" and fields2.length() > 2)
                            {
                                double K2OG = fields2.at(1).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setK2OG(K2OG);
                                }
                                double K2OGD = fields2.at(2).toDouble(&ok);
                                if( ok )
                                {
                                    Datas::I()->mesurekerato->setdioptriesK2OG(K2OGD);
                                }
                            } else {
                                break;
                            }
                            i+=1;
                        }
                        break;
                    }
                }
                i+=1;
            }
        }
    }
}

void Tomey::LectureDonneesRC5000(QString Mesure, QString nameARK)
{
    int     a(0);


    /*! SORTIE EXEMPLE POUR UN TOMEY RC-5000
     * SOH =    SOH             //SOH -> start of header
     * STX =    STX             //STX -> start of text
     * CR =     CR              //CR -> carriage return
     * EOT =    EOT             //EOT -> end of transmission
     * La 1ere ligne commence par SOH, la dernière par EOT- représentés ici
     * Les autres lignes commencent par STX
     * Toutes les lignes se terminent par CR


SOH*1234                                    -> id (4 octets)
RK
0R+ 5.25- 0.25179                           -> AR Côté (R/L) Sphere (6o) Cylindre (6o) Axe (3o)
0L+ 3.00- 0.75 89
DB60.5                                      -> EIP (4o)
CR 5.0045.00 89 4.0040.00 75 4.50- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o)
CL 5.0045.00 89 4.0040.00 75 4.50- 0.25
*/
    a               = Mesure.indexOf("RK");
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
        a  = Ref.indexOf("DB");
        if (a >= 0) {
            QString PD      = Ref.mid(Ref.indexOf("DB")+2,2);
            Datas::I()->mesureautoref->setecartIP(PD.toInt());
        }
        // Données de KERATOMETRIE -------------------------------------------------------------------------------------------------------
        // OEIL DROIT -----------------------------------------------------------------------------
        a  = Ref.indexOf("CR");
        if (a>=0)
        {
            /*!->CR 5.0045.00 89 4.0040.00 75 4.50- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
            QString KOD("");
            QString K1OD("null"), K2OD("null");
            int     AxeKOD(0);
            KOD                 = Ref.mid(Ref.indexOf("CR")+2,37);
            /*!              1           2           3
             *!->01234|56789|012|34567|89012|345|67890|123456 */
            /*   mK1OD|dK1OD|Axe|mK2OD|dK2OD|Axe|mAvgK|Cylndr
             *!-> 5.00|45.00| 89| 4.00|40.00| 75| 4.50|- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
            KOD.replace(" ", "0");
            /*!->05.00|45.00|089|04.00|40.00|075|04.50|-00.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
            K1OD                = KOD.mid(0,5);
            K2OD                = KOD.mid(13,5);
            AxeKOD              = KOD.mid(10,3).toInt();
            if (K1OD.toDouble() != 0 && K2OD.toDouble() != 0)
            {
                Datas::I()->mesurekerato->setK1OD(K1OD.toDouble());
                Datas::I()->mesurekerato->setK2OD(K2OD.toDouble());
                Datas::I()->mesurekerato->setaxeKOD(Utils::roundToNearestFive(AxeKOD));
                Datas::I()->mesurekerato->setdioptriesK1OD(KOD.mid(5,5).toDouble());
                Datas::I()->mesurekerato->setdioptriesK2OD(KOD.mid(18,5).toDouble());
            }
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        a  = Ref.indexOf("CL");
        if (a>=0)
        {
            /*!->CL 5.0045.00 89 4.0040.00 75 4.50- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
            QString KOG("");
            QString K1OG("null"), K2OG("null");
            int     AxeKOG(0);
            KOG                 = Ref.mid(Ref.indexOf("CR")+2,37);
            /*!              1           2           3
             *!->01234|56789|012|34567|89012|345|67890|123456 */
            /*   mK1OG|dK1OG|Axe|mK2OG|dK2OG|Axe|mAvgK|Cylndr
             *!-> 5.00|45.00| 89| 4.00|40.00| 75| 4.50|- 0.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
            KOG.replace(" ", "0");
            /*!->05.00|45.00|089|04.00|40.00|075|04.50|-00.25     -> Kerato R1mm (5o) R1dioptries (5o) R1Axe (3o) R2mm (5o) R2dioptries (5o) R2Axe (3o) RAVGmm (5o) Cylindre (6o) */
            K1OG                = KOG.mid(0,5);
            K2OG                = KOG.mid(13,5);
            AxeKOG              = KOG.mid(10,3).toInt();
            if (K1OG.toDouble() != 0 && K2OG.toDouble() != 0)
            {
                Datas::I()->mesurekerato->setK1OG(K1OG.toDouble());
                Datas::I()->mesurekerato->setK2OG(K2OG.toDouble());
                Datas::I()->mesurekerato->setaxeKOG(Utils::roundToNearestFive(AxeKOG));
                Datas::I()->mesurekerato->setdioptriesK1OG(KOG.mid(5,5).toDouble());
                Datas::I()->mesurekerato->setdioptriesK2OG(KOG.mid(18,5).toDouble());
            }
        }
    }
}
