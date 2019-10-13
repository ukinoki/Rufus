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

#include "dlg_message.h"

Message* Message::instance = Q_NULLPTR;

void Message::LogMessage(QString msg)
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
        out << timelog << " - " << "MESSAGE" << " : " << msg << "\n";
        testfile.close();
    }
}
