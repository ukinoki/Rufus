#ifndef LOG_H
#define LOG_H

#include <QDate>
#include <QDir>
#include <QString>
#include "macros.h"
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
    static void ERROR(QString msg, QString infoMsg = "")
    {
        trace("ERROR", msg, infoMsg);
    }
    static void trace(QString type, QString msg, QString infoMsg = "")
    {
        QDir DirRssces;
        QString dirlog = QDir::homePath() + NOMDIR_RUFUS NOMDIR_LOGS;
        if (!DirRssces.exists(dirlog))
            DirRssces.mkdir(dirlog);
        QString datelog = QDate::currentDate().toString("yyyy-MM-dd");
        QString fileName(dirlog +"/logs_" + datelog + ".txt");
        QFile testfile(fileName);
        if( testfile.open(QIODevice::ReadWrite) )
        {
            QTextStream out(&testfile);
            QString timelog = QTime::currentTime().toString();
            out << type << " : " << msg << " : " << timelog;
        }
        //else UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'ouvrir le fichier\n") + fileName);

        if( showMessage() )
            UpMessageBox::Watch(Q_NULLPTR, msg, infoMsg);
    }
};


#endif // LOG_H
