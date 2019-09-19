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

#ifndef CLS_REFRACTION_H
#define CLS_REFRACTION_H

#include "cls_item.h"

class Refraction : public Item
{
    Q_OBJECT
public:
    explicit Refraction(QJsonObject data = QJsonObject(), QObject *parent = Q_NULLPTR);
    enum Mesure {Fronto, Autoref, Acuite, Prescription, NoMesure};      Q_ENUM(Mesure)
    enum Distance {Loin, Pres, AllDistance, Inconnu};                   Q_ENUM(Distance)
    enum Oeil {Droit, Gauche, Les2};                                    Q_ENUM(Oeil)
    enum Cycloplegie {Dilatation, NoDilatation, NoLoSo};                Q_ENUM(Cycloplegie)
    void setData(QJsonObject data);

private:
    int m_idpat;                    //!> l'id du patient
    int m_idacte;                   //!> l'id de l'acte aucours duquel la mesure a été faite
    QDate m_daterefraction;         //!> la date de la refraction
    Mesure m_typemesure;            //!> le type de mesure effectuée : frontofocometre, autorefractomètre, acuité ou prescription
    Distance m_distance = Inconnu;  //!> la distance de mesure: loin, près, les 2
    Cycloplegie m_dilate = NoLoSo;  //!> examen sous cycloplegie ou non
    bool m_isODmesure;              //!> l'OD a été mesuré
    double m_sphereOD = 0;          //!> sphere OD
    double m_cylindreOD = 0;        //!> cylindre OD
    int m_axecylindreOD = 0;        //!> axe OD
    QString m_avlOD;                //!> acuité de loin OD
    double m_addVPOD = 0;           //!> addition de près OD
    QString m_avpPOD;               //!> acuité de près OD
    double m_prismeOD = 0;          //!> prismeOD
    int m_baseprismeOD = 0;         //!> base prisme OD en degré
    QString m_baseprismetextOD;     //!> base prisme OD en texte (nasal, temporal, supérieur, inférieur
    Logic m_haspressonOD = Null;    //!> un presson est utilisé sur l'OD
    Logic m_hasdepoliOD = Null;     //!> un dépoli est utilisé sur l'OD
    Logic m_hasplanOD = Null;       //!> un verre plan est utilisé sur l'OD
    int m_ryserOD = 0;              //!> puissance Ryser OD
    QString m_formuleOD;            //!> formule de réfraction OD
    bool m_isOGmesure;              //!> l'OG a été mesuré
    double m_sphereOG = 0;          //!> sphere OG
    double m_cylindreOG = 0;        //!> cylindre OG
    int m_axecylindreOG = 0;        //!> axe OG
    QString m_avlOG;                //!> acuité de loin OG
    double m_addVPOG = 0;           //!> addition de près OG
    QString m_avpPOG;               //!> acuité de près OG
    double m_prismeOG = 0;          //!> prismeOG
    int m_baseprismeOG = 0;         //!> base prisme OG en degré
    QString m_baseprismetextOG;     //!> base prisme OG en texte (nasal, temporal, supérieur, inférieur
    Logic m_haspressonOG = Null;    //!> un presson est utilisé sur l'OG
    Logic m_hasdepoliOG = Null;     //!> un dépoli est utilisé sur l'OG
    Logic m_hasplanOG = Null;       //!> un verre plan est utilisé sur l'OG
    int m_ryserOG = 0;              //!> puissance Ryser OG
    QString m_formuleOG;            //!> formule de réfraction OG
    QString m_commentaireordo;      //!> commentaire de l'ordonnace de verres
    Distance m_typeverres;          //!> la distance d'utilisation des verres
    Oeil m_oeil;                    //!> l'oeil pour lequel les verres sont prescrits
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
    bool haspressonOD() { return m_haspressonOD == True; }      //!> un presson est utilisé sur l'OD
    bool hasdepoliOD() { return m_hasdepoliOD == True; }        //!> un dépoli est utilisé sur l'OD
    bool hasplanOD() { return m_hasplanOD == True; }            //!> un verre plan est utilisé sur l'OD
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
    bool haspressonOG() { return m_haspressonOG == True; }      //!> un presson est utilisé sur l'OG
    bool hasdepoliOG() { return m_hasdepoliOG == True; }        //!> un dépoli est utilisé sur l'OG
    bool hasplanOG() { return m_hasplanOG == True; }            //!> un verre plan est utilisé sur l'OG
    int ryserOG() { return m_ryserOG; }                         //!> puissance Ryser OG
    QString formuleOG() { return m_formuleOG; }                 //!> formule de réfraction OG
    QString commentaireordo() { return m_commentaireordo; }     //!> commentaire de l'ordonnace de verres
    Distance typeverres() { return m_typeverres; }              //!> la distance d'utilisation des verres
    Oeil oeil() { return m_oeil; }                              //!> l'oeil pour lequel les verres sont prescrits
    int monture() { return m_monture; }                         //!> le nombre de montures
    bool isverreteinte() { return m_isverreteinte; }            //!> les verres sont teintés
    int ecartIP() { return m_ecartIP; }                         //!> ecart interpuppilaire

    void setidpat(int id)                   { m_idpat = id; }                                   //!> l'id du patient
    void setidacte(int id)                  { m_idacte = id; }                                  //!> l'id de l'acte aucours duquel la mesure a été faite
    void setdaterefraction(QDate date)      { m_daterefraction = date; }                        //!> la date de la refraction
    void setmesure(Mesure mesure)           { m_typemesure = mesure; }                          //!> le type de mesure effectuée : frontofocometre, autorfractomètre, acuité ou prescription
    void setdistance(Distance distance)     { m_distance = distance; }                          //!> la distance de mesure: loin, près, les 2
    void setisdilate(bool logic)            { m_dilate = (logic? Dilatation : NoDilatation); }  //!> examen sous cycloplegie ou non
    void setisODmesure(bool logic)          { m_isODmesure = logic; }                           //!> l'OD a été mesuré
    void setsphereOD(double val)            { m_sphereOD = val; }                               //!> sphere OD
    void setcylindreOD(double val)          { m_cylindreOD = val; }                             //!> cylindre OD
    void setaxecylindreOD(int axe)          { m_axecylindreOD = axe; }                          //!> axe OD
    void setavlOD(QString txt)              { m_avlOD = txt; }                                  //!> acuité de loin OD
    void setaddVPOD(double val)             { m_addVPOD = val; }                                //!> addition de près OD
    void setavpOD(QString txt)              { m_avpPOD = txt; }                                 //!> acuité de près OD
    void setprismeOD(double val)            { m_prismeOD = val; }                               //!> prismeOD
    void setbaseprismeOD(int base)          { m_baseprismeOD = base; }                          //!> base prisme OD en degré
    void setbaseprismetextOD(QString txt)   { m_baseprismetextOD = txt; }                       //!> base prisme OD en texte (nasal, temporal, supérieur, inférieur
    void sethaspressonOD(bool logic)        { m_haspressonOD = (logic? True : False); }         //!> un presson est utilisé sur l'OD
    void sethasdepoliOD(bool logic)         { m_hasdepoliOD = (logic? True : False); }          //!> un dépoli est utilisé sur l'OD
    void sethasplanOD(bool logic)           { m_hasplanOD = (logic? True : False); }            //!> un verre plan est utilisé sur l'OD
    void setryserOD(int val)                { m_ryserOD = val; }                                //!> puissance Ryser OD
    void setformuleOD(QString txt)          { m_formuleOD = txt; }                              //!> formule de réfraction OD
    void setisOGmesure(bool logic)          { m_isOGmesure = logic; }                           //!> l'OG a été mesuré
    void setsphereOG(double val)            { m_sphereOG = val; }                               //!> sphere OG
    void setcylindreOG(double val)          { m_cylindreOG = val; }                             //!> cylindre OG
    void setaxecylindreOG(int axe)          { m_axecylindreOG = axe; }                          //!> axe OG
    void setavlOG(QString txt)              { m_avlOG = txt; }                                  //!> acuité de loin OG
    void setaddVPOG(double val)             { m_addVPOG = val; }                                //!> addition de près OG
    void setavpOG(QString txt)              { m_avpPOG = txt; }                                 //!> acuité de près OG
    void setprismeOG(double val)            { m_prismeOG = val; }                               //!> prismeOG
    void setbaseprismeOG(int base)          { m_baseprismeOG = base; }                          //!> base prisme OG en degré
    void setbaseprismetextOG(QString txt)   { m_baseprismetextOG = txt; }                       //!> base prisme OG en texte (nasal, temporal, supérieur, inférieur
    void sethaspressonOG(bool logic)        { m_haspressonOG = (logic? True : False); }         //!> un presson est utilisé sur l'OG
    void sethasdepoliOG(bool logic)         { m_hasdepoliOG = (logic? True : False); }          //!> un dépoli est utilisé sur l'OG
    void sethasplanOG(bool logic)           { m_hasplanOG = (logic? True : False); }            //!> un verre plan est utilisé sur l'OG
    void setryserOG(int val)                { m_ryserOG = val; }                                //!> puissance Ryser OG
    void setformuleOG(QString txt)          { m_formuleOG = txt; }                              //!> formule de réfraction OG
    void setcommentaireordo(QString txt)    { m_commentaireordo = txt; }                        //!> commentaire de l'ordonnace de verres
    void settypeverres(Distance distance)   { m_typeverres = distance; }                        //!> la distance d'utilisation des verres
    void setoeil(Oeil oeil)                 { m_oeil = oeil; }                                  //!> l'oeil pour lequel les verres sont prescrits
    void setmonture(int n)                  { m_monture = n; }                                  //!> le nombre de montures
    void setisverreteinte(bool logic)       { m_isverreteinte = logic; }                        //!> les verres sont teintés
    void setecartIP(int val)                { m_ecartIP = val; }                                //!> ecart interpuppilaire

    void setdataclean(Refraction::Mesure mesure = Refraction::NoMesure);

    static Distance    ConvertDistance(QString distance);
    static Mesure      ConvertMesure(QString Mesure);
    static QString     ConvertMesure(Refraction::Mesure Mesure);
};

#endif // CLS_REFRACTION_H
