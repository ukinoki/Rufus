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
#include "dlg_message.h"


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
        QString fileName(dirlog + "/" + datelog + "_errorlog.txt");
        QFile testfile(fileName);
        if( testfile.open(QIODevice::Append) )
        {
            QTextStream out(&testfile);
            QString timelog = QTime::currentTime().toString();
            out << timelog << " - " << type << " : " << msg << (infoMsg==""? "" : " : " + infoMsg) << "\n";
            testfile.close();
        }
        //else UpMessageBox::Watch(Q_NULLPTR, tr("Impossible d'ouvrir le fichier\n") + fileName);

        if( showMessage() )
            UpMessageBox::Show(Q_NULLPTR, msg, infoMsg);
    }
    static void MSGSOCKET(QString msg, QString infoMsg = "")
    {
        tracesocket("MSG", msg, infoMsg);
    }
    static void tracesocket(QString type, QString msg, QString infoMsg = "")
    {
        QDir DirRssces;
        QString dirlog = QDir::homePath() + NOMDIR_RUFUS NOMDIR_LOGS;
        if (!DirRssces.exists(dirlog))
            DirRssces.mkdir(dirlog);
        QString datelog = QDate::currentDate().toString("yyyy-MM-dd");
        QString fileName(dirlog + "/" + datelog + "_tcpmsglog.txt");
        QFile testfile(fileName);
        if( testfile.open(QIODevice::Append) )
        {
            QTextStream out(&testfile);
            QString timelog = QTime::currentTime().toString();
            out << timelog << " - " << type << "\n     -> " << msg.replace(TCPMSG_Separator, ":::") << (infoMsg==""? "" : " : " + infoMsg.replace(TCPMSG_Separator, ":::")) << "\n";
            testfile.close();
        }
    }
};


#endif // LOG_H
