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

#ifndef UPSYSTEMTRAYICON_H
#define UPSYSTEMTRAYICON_H

#include <QSystemTrayIcon>
#include <QTimer>
#include <QTime>
#include <QStandardItem>

class UpSystemTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit    UpSystemTrayIcon();
    void        showMessage(QString title, QString msg, QIcon icon, int duree);
private:
    QList<QMap<QString, QVariant>>  list_messages; //!> chaque QMap de la liste contient ["titre"] , ["texte"], ["duree"]
    QList<QIcon>                    list_icons;
    void                            showListMessages();
};

#endif // UPSYSTEMTRAYICON_H
