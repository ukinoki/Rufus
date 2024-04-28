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

#ifndef CLS_REFRACTION_H
#define CLS_REFRACTION_H

#include "cls_item.h"

/*!
 * \brief Refraction class
 * l'ensemble des informations concernant la réfraction d'un patient
 */
class Refraction : public Item
{
    Q_OBJECT
public:
    explicit Refraction(QJsonObject data = QJsonObject(), QObject *parent = Q_NULLPTR);
    enum Mesure {Fronto, Autoref, Acuite, Prescription, NoMesure};      Q_ENUM(Mesure)
    enum Distance {Loin, Pres, AllDistance, Inconnu};                   Q_ENUM(Distance)
    enum Cycloplegie {Dilatation, NoDilatation, NoLoSo};                Q_ENUM(Cycloplegie)
    void setData(QJsonObject data);

private:
    int m_idpat;                    //!> l'id du patient
    int m_idacte;                   //!> l'id de l'acte aucours duquel la mesure a été faite
    QDate m_daterefraction;         //!> la date de la refraction
    Mesure m_typemesure;            //!> le type de mesure effectuée : frontofocometre, autorefractomètre, acuité ou prescription
    Distance m_distance = Inconnu;  //!> la distance de mesure: loin, près, les 2
    Cycloplegie m_dilate = NoLoSo;  //!> examen sous cycloplegie ou non
    bool m_isODmesure = false;      //!> l'OD a été mesuré
    double m_sphereOD = 0;          //!> sphere OD
    double m_cylindreOD = 0;        //!> cylindre OD
    int m_axecylindreOD = 0;        //!> axe OD
    QString m_avlOD;                //!> acuité de loin OD
    double m_addVPOD = 0;           //!> addition de près OD
    QString m_avpPOD;               //!> acuité de près OD
    double m_prismeOD = 0;          //!> prismeOD
    int m_baseprismeOD = 0;         //!> base prisme OD en degré
    QString m_baseprismetextOD;     //!> base prisme OD en texte (nasal, temporal, supérieur, inférieur
    bool m_haspressonOD = false;    //!> un presson est utilisé sur l'OD
    bool m_hasdepoliOD = false;     //!> un dépoli est utilisé sur l'OD
    bool m_hasplanOD = false;       //!> un verre plan est utilisé sur l'OD
    int m_ryserOD = 0;              //!> puissance Ryser OD
    QString m_formuleOD;            //!> formule de réfraction OD
    bool m_isOGmesure = false;      //!> l'OG a été mesuré
    double m_sphereOG = 0;          //!> sphere OG
    double m_cylindreOG = 0;        //!> cylindre OG
    int m_axecylindreOG = 0;        //!> axe OG
    QString m_avlOG;                //!> acuité de loin OG
    double m_addVPOG = 0;           //!> addition de près OG
    QString m_avpPOG;               //!> acuité de près OG
    double m_prismeOG = 0;          //!> prismeOG
    int m_baseprismeOG = 0;         //!> base prisme OG en degré
    QString m_baseprismetextOG;     //!> base prisme OG en texte (nasal, temporal, supérieur, inférieur
    bool m_haspressonOG = false;    //!> un presson est utilisé sur l'OG
    bool m_hasdepoliOG = false;     //!> un dépoli est utilisé sur l'OG
    bool m_hasplanOG = false;       //!> un verre plan est utilisé sur l'OG
    int m_ryserOG = 0;              //!> puissance Ryser OG
    QString m_formuleOG;            //!> formule de réfraction OG
    QString m_commentaireordo;      //!> commentaire de l'ordonnace de verres
    Distance m_typeverres;          //!> la distance d'utilisation des verres
    Cote m_oeil;                    //!> l'oeil pour lequel les verres sont prescrits
    int m_monture;                  //!> le nombre de montures
    bool m_isverreteinte;           //!> les verres sont teintés
    int m_ecartIP = 0;              //!> ecart interpuppilaire

public:
    int idpat() { return m_idpat; }                             //!> l'id du patient
    int idacte() { return m_idacte; }                           //!> l'id de l'acte aucours duquel la mesure a été faite
    QDate daterefraction() { return m_daterefraction; }         //!> la date de la refraction
    Mesure typemesure() { return m_typemesure; }                //!> le type de mesure effectuée : frontofocometre, autorefractomètre, acuité ou prescription
    Distance distance() { return m_distance; }                  //!> la distance de mesure: loin, près, les 2
    bool isdilate() { return m_dilate == Dilatation; }          //!> examen sous cycloplegie ou non
    bool isODmesure() { return m_isODmesure; }                  //!> l'OD a été mesuré
    double sphereOD() { return m_sphereOD; }                    //!> sphere OD
    double cylindreOD() { return m_cylindreOD; }                //!> cylindre OD
    int axecylindreOD() { return m_axecylindreOD; }             //!> axe OD
    QString avlOD() { return m_avlOD; }                         //!> acuité de loin OD
    double addVPOD() { return m_addVPOD; }                      //!> addition de près OD
    QString avpOD() { return m_avpPOD; }                        //!> acuité de près OD
    double prismeOD() { return m_prismeOD; }                    //!> prismeOD
    int baseprismeOD() { return m_baseprismeOD; }               //!> base prisme OD en degré
    QString baseprismetextOD() { return m_baseprismetextOD; }   //!> base prisme OD en texte (nasal, temporal, supérieur, inférieur
    bool haspressonOD() { return m_haspressonOD; }              //!> un presson est utilisé sur l'OD
    bool hasdepoliOD() { return m_hasdepoliOD; }                //!> un dépoli est utilisé sur l'OD
    bool hasplanOD() { return m_hasplanOD; }                    //!> un verre plan est utilisé sur l'OD
    int ryserOD() { return m_ryserOD; }                         //!> puissance Ryser OD
    QString formuleOD() { return m_formuleOD; }                 //!> formule de réfraction OD
    bool isOGmesure() { return m_isOGmesure; }                  //!> l'OG a été mesuré
    double sphereOG() { return m_sphereOG; }                    //!> sphere OG
    double cylindreOG() { return m_cylindreOG; }                //!> cylindre OG
    int axecylindreOG() { return m_axecylindreOG; }             //!> axe OG
    QString avlOG() { return m_avlOG; }                         //!> acuité de loin OG
    double addVPOG() { return m_addVPOG; }                      //!> addition de près OG
    QString avpOG() { return m_avpPOG; }                        //!> acuité de près OG
    double prismeOG() { return m_prismeOG; }                    //!> prismeOG
    int baseprismeOG() { return m_baseprismeOG; }               //!> base prisme OG en degré
    QString baseprismetextOG() { return m_baseprismetextOG; }   //!> base prisme OG en texte (nasal, temporal, supérieur, inférieur
    bool haspressonOG() { return m_haspressonOG; }              //!> un presson est utilisé sur l'OG
    bool hasdepoliOG() { return m_hasdepoliOG; }                //!> un dépoli est utilisé sur l'OG
    bool hasplanOG() { return m_hasplanOG; }                    //!> un verre plan est utilisé sur l'OG
    int ryserOG() { return m_ryserOG; }                         //!> puissance Ryser OG
    QString formuleOG() { return m_formuleOG; }                 //!> formule de réfraction OG
    QString commentaireordo() { return m_commentaireordo; }     //!> commentaire de l'ordonnace de verres
    Distance typeverres() { return m_typeverres; }              //!> la distance d'utilisation des verres
    Cote oeil() { return m_oeil; }                              //!> l'oeil pour lequel les verres sont prescrits
    int monture() { return m_monture; }                         //!> le nombre de montures
    bool isverreteinte() { return m_isverreteinte; }            //!> les verres sont teintés
    int ecartIP() { return m_ecartIP; }                         //!> ecart interpuppilaire

    void setidpat(int id)                   { m_idpat = id;
                                              m_data[CP_IDPAT_REFRACTIONS] = id; }
    void setidacte(int id)                  { m_idacte = id;
                                              m_data[CP_IDACTE_REFRACTIONS] = id; }
    void setdaterefraction(QDate date)      { m_daterefraction = date;
                                              m_data[CP_DATE_REFRACTIONS] = date.toString("yyyy-MM-dd"); }
    void settypemesure(Mesure mesure)       { m_typemesure = mesure;
                                              m_data[CP_TYPEMESURE_REFRACTIONS] = ConvertMesure(mesure); }
    void setdistance(Distance distance)     { m_distance = distance;
                                              m_data[CP_DISTANCEMESURE_REFRACTIONS] = ConvertDistance(distance); }
    void setisdilate(bool logic)            { m_dilate = (logic? Dilatation : NoDilatation);
                                              m_data[CP_CYCLOPLEGIE_REFRACTIONS] = logic; }
    void setODmesure(bool logic)            { m_isODmesure = logic;
                                              m_data[CP_ODMESURE_REFRACTIONS] = logic; }
    void setsphereOD(double val)            { m_sphereOD = val;
                                              m_data[CP_SPHEREOD_REFRACTIONS] = val; }
    void setcylindreOD(double val)          { m_cylindreOD = val;
                                              m_data[CP_CYLINDREOD_REFRACTIONS] = val; }
    void setaxecylindreOD(int axe)          { m_axecylindreOD = axe;
                                              m_data[CP_AXECYLOD_REFRACTIONS] = axe; }
    void setavlOD(QString txt)              { m_avlOD = txt;
                                              m_data[CP_AVLOD_REFRACTIONS] = txt; }
    void setaddVPOD(double val)             { m_addVPOD = val;
                                              m_data[CP_ADDVPOD_REFRACTIONS] = val; }
    void setavpOD(QString txt)              { m_avpPOD = txt;
                                              m_data[CP_AVPOD_REFRACTIONS] = txt; }
    void setprismeOD(double val)            { m_prismeOD = val;
                                              m_data[CP_PRISMEOD_REFRACTIONS] = val; }
    void setbaseprismeOD(int base)          { m_baseprismeOD = base;
                                              m_data[CP_BASEPRISMEOD_REFRACTIONS] = base; }
    void setbaseprismetextOD(QString txt)   { m_baseprismetextOD = txt;
                                              m_data[CP_BASEPRISMETEXTOD_REFRACTIONS] = txt; }
    void sethaspressonOD(bool logic)        { m_haspressonOD = (logic? True : False);
                                              m_data[CP_PRESSONOD_REFRACTIONS] = logic; }
    void sethasdepoliOD(bool logic)         { m_hasdepoliOD = (logic? True : False);
                                              m_data[CP_DEPOLIOD_REFRACTIONS] = logic; }
    void sethasplanOD(bool logic)           { m_hasplanOD = (logic? True : False);
                                              m_data[CP_PLANOD_REFRACTIONS] = logic; }
    void setryserOD(int val)                { m_ryserOD = val;
                                              m_data[CP_RYSEROD_REFRACTIONS] = val; }
    void setformuleOD(QString txt)          { m_formuleOD = txt;
                                              m_data[CP_FORMULEOD_REFRACTIONS] = txt; }
    void setOGmesure(bool logic)            { m_isOGmesure = logic;
                                              m_data[CP_OGMESURE_REFRACTIONS] = logic; }
    void setsphereOG(double val)            { m_sphereOG = val;
                                              m_data[CP_SPHEREOG_REFRACTIONS] = val; }
    void setcylindreOG(double val)          { m_cylindreOG = val;
                                              m_data[CP_CYLINDREOG_REFRACTIONS] = val; }
    void setaxecylindreOG(int axe)          { m_axecylindreOG = axe;
                                              m_data[CP_AXECYLOG_REFRACTIONS] = axe; }
    void setavlOG(QString txt)              { m_avlOG = txt;
                                              m_data[CP_AVLOG_REFRACTIONS] = txt; }
    void setaddVPOG(double val)             { m_addVPOG = val;
                                              m_data[CP_ADDVPOG_REFRACTIONS] = val; }
    void setavpOG(QString txt)              { m_avpPOG = txt;
                                              m_data[CP_AVPOG_REFRACTIONS] = txt; }
    void setprismeOG(double val)            { m_prismeOG = val;
                                              m_data[CP_PRISMEOG_REFRACTIONS] = val; }
    void setbaseprismeOG(int base)          { m_baseprismeOG = base;
                                              m_data[CP_BASEPRISMEOG_REFRACTIONS] = base; }
    void setbaseprismetextOG(QString txt)   { m_baseprismetextOG = txt;
                                              m_data[CP_BASEPRISMETEXTOG_REFRACTIONS] = txt; }
    void sethaspressonOG(bool logic)        { m_haspressonOG = (logic? True : False);
                                              m_data[CP_PRESSONOG_REFRACTIONS] = logic; }
    void sethasdepoliOG(bool logic)         { m_hasdepoliOG = (logic? True : False);
                                              m_data[CP_DEPOLIOG_REFRACTIONS] = logic; }
    void sethasplanOG(bool logic)           { m_hasplanOG = (logic? True : False);
                                              m_data[CP_PLANOG_REFRACTIONS] = logic; }
    void setryserOG(int val)                { m_ryserOG = val;
                                              m_data[CP_RYSEROG_REFRACTIONS] = val; }
    void setformuleOG(QString txt)          { m_formuleOG = txt;
                                              m_data[CP_FORMULEOG_REFRACTIONS] = txt; }
    void setcommentaireordo(QString txt)    { m_commentaireordo = txt;
                                              m_data[CP_COMMENTAIREORDO_REFRACTIONS] = txt; }
    void settypeverres(Distance distance)   { m_typeverres = distance;
                                              m_data[CP_TYPEVERRES_REFRACTIONS] = ConvertDistance(distance); }
    void setoeil(Cote oeil)          { m_oeil = oeil;
                                              m_data[CP_OEIL_REFRACTIONS] = ConvertCote(oeil); }
    void setmonture(int n)                  { m_monture = n;
                                              m_data[CP_MONTURE_REFRACTIONS] = n; }
    void setisverreteinte(bool logic)       { m_isverreteinte = logic;
                                              m_data[CP_VERRETEINTE_REFRACTIONS] = logic; }
    void setecartIP(int val)                { m_ecartIP = val;
                                              m_data[CP_PD_REFRACTIONS] = val; }

    void setdataclean(Refraction::Mesure mesure = Refraction::NoMesure);

    static Distance     ConvertDistance(QString distance);
    static QString      ConvertDistance(Refraction::Distance distance);
    static Mesure       ConvertMesure(QString Mesure);
    static QString      ConvertMesure(Refraction::Mesure Mesure);
    void                ConversionCylindreNeg();
};

#endif // CLS_REFRACTION_H
