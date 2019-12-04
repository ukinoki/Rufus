/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LOG_H
#define LOG_H

#include <QDate>
#include <QDir>
#include <QString>
#include "macros.h"
#include "upmessagebox.h"

/*!
* \file Log.h
* \brief Cette classe permet d'écrire des logs visuels et dans des fichiers physiques
* \author Alexandre.D
* \version 0.1
* \date 12 juin 2018
* Cette classe est utilisée de manière uniquement static.
*/

class Logs
{
public:
    static void ERROR(QString msg, QString infoMsg = "")
    {
        trace("ERROR", msg, infoMsg);
    }
    static void trace(QString type, QString msg, QString infoMsg = "")
    {
        QDir DirRssces;
        QString dirlog = QDir::homePath() + DIR_RUFUS DIR_LOGS;
        if (!DirRssces.exists(dirlog))
            DirRssces.mkdir(dirlog);
        QString datelog = QDate::currentDate().toString("yyyy-MM-dd");
        QString fileName(dirlog + "/" + datelog + "_errorlog.txt");
        QFile testfile(fileName);
        if( testfile.open(QIODevice::Append) )
        {
            QTextStream out(&testfile);
            QString timelog = QTime::currentTime().toString();
            out << timelog << " - " << type << " : " << msg << (infoMsg==""? "" : " : " + infoMsg) << "\n";
            testfile.close();
        }
    }
    static void LogSktMessage(QString msg, QString infoMsg = "")
    {
        QDir DirRssces;
        QString dirlog = QDir::homePath() + DIR_RUFUS DIR_LOGS;
        if (!DirRssces.exists(dirlog))
            DirRssces.mkdir(dirlog);
        QString datelog = QDate::currentDate().toString("yyyy-MM-dd");
        QString fileName(dirlog + "/" + datelog + "_tcpmsglog.txt");
        QFile testfile(fileName);
        if( testfile.open(QIODevice::Append) )
        {
            QTextStream out(&testfile);
            QString timelog = QTime::currentTime().toString();
            out << timelog << " - " << "MSG\t-> " << msg.replace(TCPMSG_Separator, ":::") << (infoMsg == ""? "" : " : " + infoMsg.replace(TCPMSG_Separator, ":::")) << "\n";
            testfile.close();
        }
    }
    static void LogToFile(QString NomFichier, QString msg)
    {
        //syntaxe = LogToFile("test.txt", texte);
        QDir DirRssces;
        QString dirlog = QDir::homePath() + DIR_RUFUS DIR_LOGS;
        if (!DirRssces.exists(dirlog))
            DirRssces.mkdir(dirlog);
        QString datelog = QDate::currentDate().toString("yyyy-MM-dd");
        QString fileName(dirlog + "/" + datelog + "_" + NomFichier);
        QFile testfile(fileName);
        if (testfile.open(QIODevice::Append))
        {
            QTextStream out(&testfile);
            QString timelog = QTime::currentTime().toString();
            out << timelog << " - " << msg << "\n";
            testfile.close();
        }
    }
    //!> supprime les fichiers de logs antérieurs à J - anciennete jours
    static void EpureLogs(int anciennete = 7)
    {
        QDir dirlogs = QDir(QDir::homePath() + DIR_RUFUS DIR_LOGS);
        QStringList listfiles = dirlogs.entryList();
        for (int i=0; i<listfiles.size(); ++i)
        {
            QFile file(listfiles.at(i));
            QDate datefile = QDate::fromString(file.fileName().left(10), "yyyy-MM-dd");
            if (datefile < QDate::currentDate().addDays(-anciennete))
                QFile::remove(QDir::homePath() + DIR_RUFUS DIR_LOGS + "/" + file.fileName());
        }
    }
};

#endif // LOG_H
