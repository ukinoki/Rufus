#ifndef TOPCON_H
#define TOPCON_H

#include <QObject>
#include <QDomDocument>


class Topcon
{


private:
    Topcon();
    static Topcon *instance;

public:



    static Topcon*        I();

    static void LectureDonneesXMLnsCommon(QDomDocument docxml);
    static void LectureDonneesXMLnsSBJ(QDomDocument docxml);
    static void LectureDonneesXMLRefracteur(QDomDocument docxml, QString nameRF);

    static void RegleRefracteurXML(int flag, QString nameRF);

};


#endif // TOPCON_H
