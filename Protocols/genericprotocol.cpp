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
    {
        return;
    }

    PrepareFolder(Adress);

    QString filename = Adress + "/" + typfile + "_" + codecname + "_" + QString::number(Datas::I()->patients->currentpatient()->id()) + ".xml";
    SaveFileXML(xml, filename, QStringConverter::Utf16LE );
}

void GenericProtocol::PrepareFolder(QString folder)
{
    QDir Dir(folder);

    if (!Dir.exists(folder))
        Dir.mkdir(folder);

    QStringList listfiles = Dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for(int i = 0; i < listfiles.size(); ++i)
    {
        QFile file(folder + "/" + listfiles.at(i));
        Utils::removeWithoutPermissions(file);
    }
}

void GenericProtocol::SaveFileXML(QDomDocument xml, QString filename, QStringConverter::Encoding encoding)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream( &file );
        stream.setEncoding(encoding);         /*! Impose le codec UTF16LE que les Nidek exigent pour les fichiers xml */
        QString strxml = xml.toString();
        stream << strxml;
        file.setPermissions(QFileDevice::ReadOther    | QFileDevice::WriteOther
                            | QFileDevice::ReadGroup  | QFileDevice::WriteGroup
                            | QFileDevice::ReadOwner  | QFileDevice::WriteOwner
                            | QFileDevice::ReadUser   | QFileDevice::WriteUser);
        file.close();
    }

}
