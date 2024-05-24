#ifndef GENERICPROTOCOL_H
#define GENERICPROTOCOL_H

#include <QObject>

class GenericProtocol : public QObject
{
    Q_OBJECT
public:
    enum TypeMesure {
        MesureNone        = 0x0,
        MesureAll         = 0x1,
        MesureFronto      = 0x2,
        MesureAutoref     = 0x4,
        MesureKerato      = 0x8,
        MesureRefracteur  = 0x10,
        MesureTono        = 0x20,
        MesurePachy       = 0x40
    };  Q_ENUM(TypeMesure)
    Q_DECLARE_FLAGS(TypesMesures, TypeMesure)

};
Q_DECLARE_OPERATORS_FOR_FLAGS(GenericProtocol::TypesMesures)

#endif // GENERICPROTOCOL_H
