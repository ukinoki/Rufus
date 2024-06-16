#ifndef GENERICPROTOCOL_H
#define GENERICPROTOCOL_H

#include <QObject>
#include <QDomDocument>
#include "gbl_datas.h"
#include "utils.h"
#include <QSettings>
#include "log.h"

class GenericProtocol : public QObject
{
    Q_OBJECT
private:
public:

    QSettings  settings() {return QSettings(PATH_FILE_INI, QSettings::IniFormat);}

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

protected:
    void        EnregistreFileDatasXML(QDomDocument xml, TypeMesure typmesure);
    void        SaveFileXML(QDomDocument xml, QString filename, QStringConverter::Encoding encoding);
    void        PrepareFolder(QString folder);
    void        logmesure(QString mesure)  { Logs::LogToFile("refraction.txt", mesure); }

signals:
    void        newmesure(TypeMesure);             //! signal d'insertion d'une nouvelle mesure dans la base et de mise à jour des fiches rufus.cpp et dlg_refraction.cpp

};
Q_DECLARE_OPERATORS_FOR_FLAGS(GenericProtocol::TypesMesures)

#endif // GENERICPROTOCOL_H
