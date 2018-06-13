#ifndef LOG_H
#define LOG_H

#include <QDir>
#include <QString>
#include "upmessagebox.h"


/**
* \file Log.h
* \brief Cette classe permet d'écrire des logs visuels eet dans des fichiers physiques
* \author Alexanre.D
* \version 0.1
* \date 12 juin 2018
*
* Cette classe est utilisée de manière uniquement static.
*
*/

class Logs
{
public:
    static bool showMessage() { return true; }
    static void ERROR(QString msg, QString infoMsg)
    {
        trace("ERROR", msg, infoMsg);
    }
    static void trace(QString type, QString msg, QString infoMsg)
    {
        QDir DirRssces;
        if (!DirRssces.exists(QDir::homePath() + "/Documents/Rufus/Ressources"))
            DirRssces.mkdir(QDir::homePath() + "/Documents/Rufus/Ressources");
        //TODO : Ajouter date au nom du fichier
        QString fileName("logs.txt");
        QFile testfile(fileName);
        if( testfile.open(QIODevice::ReadWrite) )
        {
            QTextStream out(&msg);
            //TODO : Ajouter heure
            out << type << " : " << msg;
        }
        //else UpMessageBox::Watch(0, tr("Impossible d'ouvrir le fichier\n") + fileName);

        if( showMessage() )
            UpMessageBox::Watch(0, msg, infoMsg);
    }
};


#endif // LOG_H
