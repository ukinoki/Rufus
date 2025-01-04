#ifndef SHINNIPPON_H
#define SHINNIPPON_H

#include "genericprotocol.h"

class ShinNippon : public GenericProtocol
{
    Q_OBJECT
private:
    ShinNippon();
    static ShinNippon        *instance;

public:
    static ShinNippon*        I();

    //! PORT COM
    void        LectureDonneesCOMRefracteur(QString Mesure, TypesMesures flag);
    QByteArray  RegleRefracteurCOM(TypesMesures flag);

};
#endif // SHINNIPPON_H
