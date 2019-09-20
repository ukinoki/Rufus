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

#ifndef CLS_MESUREREFRACTION_H
#define CLS_MESUREREFRACTION_H

#include "cls_refraction.h"

class MesureRefraction : public QObject
{
public:
    MesureRefraction();
    void setdata(Refraction *ref);
    void setdata(MesureRefraction *mesure);

private:
    Refraction::Mesure m_typemesure;//!> le type de mesure effectuée : frontofocometre, autorefractomètre, acuité ou prescription
    double m_sphereOD = 0;          //!> sphere OD
    double m_cylindreOD = 0;        //!> cylindre OD
    int m_axecylindreOD = 0;        //!> axe OD
    QString m_avlOD = "";           //!> acuité de loin OD
    double m_addVPOD = 0;           //!> addition de près OD
    QString m_avpPOD = "";          //!> acuité de près OD
    double m_prismeOD = 0;          //!> prismeOD
    int m_baseprismeOD = 0;         //!> base prisme OD en degré
    QString m_formuleOD = "";       //!> formule de réfraction OD
    double m_sphereOG = 0;          //!> sphere OG
    double m_cylindreOG = 0;        //!> cylindre OG
    int m_axecylindreOG = 0;        //!> axe OG
    QString m_avlOG = "";           //!> acuité de loin OG
    double m_addVPOG = 0;           //!> addition de près OG
    QString m_avpPOG = "";          //!> acuité de près OG
    double m_prismeOG = 0;          //!> prismeOG
    int m_baseprismeOG = 0;         //!> base prisme OG en degré
    QString m_formuleOG = "";       //!> formule de réfraction OG
    int m_ecartIP = 0;              //!> ecart interpuppilaire
    bool m_cleandatas = true;       //!> les données sont vierges

public:
    Refraction::Mesure typemesure() const { return m_typemesure; }    //!> le type de mesure effectuée : frontofocometre, autorefractomètre, acuité ou prescription
    double sphereOD() const { return m_sphereOD; }                    //!> sphere OD
    double cylindreOD() const { return m_cylindreOD; }                //!> cylindre OD
    int axecylindreOD() const { return m_axecylindreOD; }             //!> axe OD
    QString avlOD() const { return m_avlOD; }                         //!> acuité de loin OD
    double addVPOD() const { return m_addVPOD; }                      //!> addition de près OD
    QString avpOD() const { return m_avpPOD; }                        //!> acuité de près OD
    double prismeOD() const { return m_prismeOD; }                    //!> prismeOD
    int baseprismeOD() const { return m_baseprismeOD; }               //!> base prisme OD en degré
    QString formuleOD() const { return m_formuleOD; }                 //!> formule de réfraction OD
    double sphereOG() const { return m_sphereOG; }                    //!> sphere OG
    double cylindreOG() const { return m_cylindreOG; }                //!> cylindre OG
    int axecylindreOG() const { return m_axecylindreOG; }             //!> axe OG
    QString avlOG() const { return m_avlOG; }                         //!> acuité de loin OG
    double addVPOG() const { return m_addVPOG; }                      //!> addition de près OG
    QString avpOG() const { return m_avpPOG; }                        //!> acuité de près OG
    double prismeOG() const { return m_prismeOG; }                    //!> prismeOG
    int baseprismeOG() const { return m_baseprismeOG; }               //!> base prisme OG en degré
    QString formuleOG() const { return m_formuleOG; }                 //!> formule de réfraction OG
    int ecartIP() const { return m_ecartIP; }                         //!> ecart interpuppilaire

    void setmesure(Refraction::Mesure mesure)           { m_typemesure = mesure; }                                    //!> le type de mesure effectuée : frontofocometre, autorfractomètre, acuité ou prescription
    void setsphereOD(double val)            { m_sphereOD = val; m_cleandatas = false; }                             //!> sphere OD
    void setcylindreOD(double val)          { m_cylindreOD = val; m_cleandatas = false; }                           //!> cylindre OD
    void setaxecylindreOD(int axe)          { m_axecylindreOD = axe; m_cleandatas = false; }                        //!> axe OD
    void setavlOD(QString txt)              { m_avlOD = txt; m_cleandatas = false; }                                //!> acuité de loin OD
    void setaddVPOD(double val)             { m_addVPOD = val; m_cleandatas = false; }                              //!> addition de près OD
    void setavpOD(QString txt)              { m_avpPOD = txt; m_cleandatas = false; }                               //!> acuité de près OD
    void setprismeOD(double val)            { m_prismeOD = val; m_cleandatas = false; }                             //!> prismeOD
    void setbaseprismeOD(int base)          { m_baseprismeOD = base; m_cleandatas = false; }                        //!> base prisme OD en degré
    void setformuleOD(QString txt)          { m_formuleOD = txt; m_cleandatas = false; }                            //!> formule de réfraction OD
    void setsphereOG(double val)            { m_sphereOG = val; m_cleandatas = false; }                             //!> sphere OG
    void setcylindreOG(double val)          { m_cylindreOG = val; m_cleandatas = false; }                           //!> cylindre OG
    void setaxecylindreOG(int axe)          { m_axecylindreOG = axe; m_cleandatas = false; }                        //!> axe OG
    void setavlOG(QString txt)              { m_avlOG = txt; m_cleandatas = false; }                                //!> acuité de loin OG
    void setaddVPOG(double val)             { m_addVPOG = val; m_cleandatas = false; }                              //!> addition de près OG
    void setavpOG(QString txt)              { m_avpPOG = txt; m_cleandatas = false; }                               //!> acuité de près OG
    void setprismeOG(double val)            { m_prismeOG = val; m_cleandatas = false; }                             //!> prismeOG
    void setbaseprismeOG(int base)          { m_baseprismeOG = base; m_cleandatas = false; }                        //!> base prisme OG en degré
    void setformuleOG(QString txt)          { m_formuleOG = txt; m_cleandatas = false; }                            //!> formule de réfraction OG
    void setecartIP(int val)                { m_ecartIP = val; m_cleandatas = false; }                              //!> ecart interpuppilaire

    bool isdataclean()                      { return m_cleandatas; }
    void cleandatas()
    {
        m_sphereOD = 0;
        m_cylindreOD = 0;
        m_axecylindreOD = 0;
        m_avlOD = "";
        m_addVPOD = 0;
        m_avpPOD = "";
        m_prismeOD = 0;
        m_baseprismeOD = 0;
        m_formuleOD = "";
        m_sphereOG = 0;
        m_cylindreOG = 0;
        m_axecylindreOG = 0;
        m_avlOG = "";
        m_addVPOG = 0;
        m_avpPOG = "";
        m_prismeOG = 0;
        m_baseprismeOG = 0;
        m_formuleOG = "";
        m_ecartIP = 0;
        m_cleandatas = true;
    }
};

#endif // CLS_MESUREREFRACTION_H
