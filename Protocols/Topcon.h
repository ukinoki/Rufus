#ifndef TOPCON_H
#define TOPCON_H

#include "genericprotocol.h"

class Topcon :  public GenericProtocol
{
    Q_OBJECT
private:
    Topcon();
    static Topcon *instance;

public:
    static Topcon*        I();

    void LectureDonneesXMLnsCommon(QDomDocument docxml);
    void LectureDonneesXMLnsSBJ(QDomDocument docxml);
    void LectureDonneesXMLRefracteur(QDomDocument docxml, QString nameRF);

    void RegleRefracteurXML(TypesMesures flag, QString nameRF);
    void EnregistreFileDatasXML(QDomDocument xml, TypeMesure typmesure);

};


#endif // TOPCON_H
