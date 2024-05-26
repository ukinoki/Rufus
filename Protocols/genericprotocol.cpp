#include "genericprotocol.h"

void GenericProtocol::EnregistreFileDatasXML(QDomDocument xml, TypeMesure typmesure)

{
    const QByteArray codecname = "UTF16LE";
    //QTextCodec *codec = QTextCodec::codecForName(codecname);
    QString Adress ("");
    QString typfile("");
    if (typmesure == MesureAutoref)
    {
        Adress = settings().value(Param_Poste_PortRefracteur_DossierEchange_Autoref).toString();
        typfile = "ARK";
    }
    else if (typmesure == MesureFronto)
    {
        Adress = settings().value(Param_Poste_PortRefracteur_DossierEchange_Fronto).toString();
        typfile = "LM";
    }
    else
        return;
    QDir Dir(Adress);
    if (!Dir.exists(Adress))
        Dir.mkdir(Adress);
    QStringList listfiles = Dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for(int i = 0; i < listfiles.size(); ++i)
    {
        QFile file(Adress + "/" + listfiles.at(i));
        Utils::removeWithoutPermissions(file);
    }
    QString filename = Adress + "/" + typfile + "_" + codecname + "_" + QString::number(Datas::I()->patients->currentpatient()->id()) + ".xml";
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream( &file );
        stream.setEncoding(QStringConverter::Utf16LE);         /*! Impose le codec UTF16LE que les Nidek exigent pour les fichiers xml */
        QString strxml = xml.toString();
        stream << strxml;
        file.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                            | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                            | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                            | QFileDevice::ReadUser   | QFileDevice::WriteUser);
        file.close();
    }
}
