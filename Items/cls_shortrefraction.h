#ifndef CLS_SHORTREFRACTION_H
#define CLS_SHORTREFRACTION_H

#include "cls_refraction.h"

class ShortRefraction : public Item
{
public:
    ShortRefraction();
    ShortRefraction(Refraction *ref);

private:
    Refraction::Mesure m_typemesure;//!> le type de mesure effectuée : frontofocometre, autorefractomètre, acuité ou prescription
    double m_sphereOD = 0;          //!> sphere OD
    double m_cylindreOD = 0;        //!> cylindre OD
    int m_axecylindreOD = 0;        //!> axe OD
    QString m_avlOD;                //!> acuité de loin OD
    double m_addVPOD = 0;           //!> addition de près OD
    QString m_avpPOD;               //!> acuité de près OD
    double m_prismeOD = 0;          //!> prismeOD
    int m_baseprismeOD = 0;         //!> base prisme OD en degré
    QString m_formuleOD;            //!> formule de réfraction OD
    double m_sphereOG = 0;          //!> sphere OG
    double m_cylindreOG = 0;        //!> cylindre OG
    int m_axecylindreOG = 0;        //!> axe OG
    QString m_avlOG;                //!> acuité de loin OG
    double m_addVPOG = 0;           //!> addition de près OG
    QString m_avpPOG;               //!> acuité de près OG
    double m_prismeOG = 0;          //!> prismeOG
    int m_baseprismeOG = 0;         //!> base prisme OG en degré
    QString m_formuleOG;            //!> formule de réfraction OG
    int m_ecartIP = 0;              //!> ecart interpuppilaire

public:
    Refraction::Mesure typemesure() { return m_typemesure; }    //!> le type de mesure effectuée : frontofocometre, autorefractomètre, acuité ou prescription
    double sphereOD() { return m_sphereOD; }                    //!> sphere OD
    double cylindreOD() { return m_cylindreOD; }                //!> cylindre OD
    int axecylindreOD() { return m_axecylindreOD; }             //!> axe OD
    QString avlOD() { return m_avlOD; }                         //!> acuité de loin OD
    double addVPOD() { return m_addVPOD; }                      //!> addition de près OD
    QString avpOD() { return m_avpPOD; }                        //!> acuité de près OD
    double prismeOD() { return m_prismeOD; }                    //!> prismeOD
    int baseprismeOD() { return m_baseprismeOD; }               //!> base prisme OD en degré
    QString formuleOD() { return m_formuleOD; }                 //!> formule de réfraction OD
    double sphereOG() { return m_sphereOG; }                    //!> sphere OG
    double cylindreOG() { return m_cylindreOG; }                //!> cylindre OG
    int axecylindreOG() { return m_axecylindreOG; }             //!> axe OG
    QString avlOG() { return m_avlOG; }                         //!> acuité de loin OG
    double addVPOG() { return m_addVPOG; }                      //!> addition de près OG
    QString avpOG() { return m_avpPOG; }                        //!> acuité de près OG
    double prismeOG() { return m_prismeOG; }                    //!> prismeOG
    int baseprismeOG() { return m_baseprismeOG; }               //!> base prisme OG en degré
    QString formuleOG() { return m_formuleOG; }                 //!> formule de réfraction OG
    int ecartIP() { return m_ecartIP; }                         //!> ecart interpuppilaire

    void setmesure(Refraction::Mesure mesure)           { m_typemesure = mesure; }              //!> le type de mesure effectuée : frontofocometre, autorfractomètre, acuité ou prescription
    void setsphereOD(double val)            { m_sphereOD = val; }                               //!> sphere OD
    void setcylindreOD(double val)          { m_cylindreOD = val; }                             //!> cylindre OD
    void setaxecylindreOD(int axe)          { m_axecylindreOD = axe; }                          //!> axe OD
    void setavlOD(QString txt)              { m_avlOD = txt; }                                  //!> acuité de loin OD
    void setaddVPOD(double val)             { m_addVPOD = val; }                                //!> addition de près OD
    void setavpOD(QString txt)              { m_avpPOD = txt; }                                 //!> acuité de près OD
    void setprismeOD(double val)            { m_prismeOD = val; }                               //!> prismeOD
    void setbaseprismeOD(int base)          { m_baseprismeOD = base; }                          //!> base prisme OD en degré
    void setformuleOD(QString txt)          { m_formuleOD = txt; }                              //!> formule de réfraction OD
    void setsphereOG(double val)            { m_sphereOG = val; }                               //!> sphere OG
    void setcylindreOG(double val)          { m_cylindreOG = val; }                             //!> cylindre OG
    void setaxecylindreOG(int axe)          { m_axecylindreOG = axe; }                          //!> axe OG
    void setavlOG(QString txt)              { m_avlOG = txt; }                                  //!> acuité de loin OG
    void setaddVPOG(double val)             { m_addVPOG = val; }                                //!> addition de près OG
    void setavpOG(QString txt)              { m_avpPOG = txt; }                                 //!> acuité de près OG
    void setprismeOG(double val)            { m_prismeOG = val; }                               //!> prismeOG
    void setbaseprismeOG(int base)          { m_baseprismeOG = base; }                          //!> base prisme OG en degré
    void setformuleOG(QString txt)          { m_formuleOG = txt; }                              //!> formule de réfraction OG
    void setecartIP(int val)                { m_ecartIP = val; }                                //!> ecart interpuppilaire
};

#endif // CLS_SHORTREFRACTION_H
