#ifndef NIDEK_H
#define NIDEK_H

#include "genericprotocol.h"

class Nidek : public GenericProtocol
{
    Q_OBJECT
private:
    Nidek();
    static Nidek        *instance;

public:
    static Nidek*        I();

    //! XML
    void        LectureDonneesXMLAutoref(QDomDocument docxml, QString nameARK);
    void        LectureDonneesXMLFronto(QDomDocument docxml);
    void        LectureDonneesXMLRefracteur(QDomDocument docxml, TypesMesures flag);

    void        RegleRefracteurXML(TypesMesures flag, QString nameRF);

    //! PORT COM
    QByteArray  OKtoReceive(QString mesure);
    QByteArray  RequestToSend();

    void        LectureDonneesCOMAutoref(QString Mesure, QString nameARK);
    void        LectureDonneesCOMFronto(QString Mesure);
    void        LectureDonneesCOMRefracteur(QString Mesure, TypesMesures flag);

    QByteArray  RegleRefracteurCOM(TypesMesures flag);

};

#endif // NIDEK_H
