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

#include "upsystemtrayicon.h"

UpSystemTrayIcon* UpSystemTrayIcon::instance =  Q_NULLPTR;
UpSystemTrayIcon* UpSystemTrayIcon::I()
{
    if( !instance )
        instance = new UpSystemTrayIcon(Icons::icSunglasses());
    return instance;
}

void UpSystemTrayIcon::showMessage(QString title, QString msg, QIcon icon, int duree)
{
    QMap<QString, QVariant> map_messages;
    map_messages["titre"] = title;
    map_messages["texte"] = msg;
    map_messages["duree"] = duree;
    list_messages   .append(map_messages);
    list_icons      .append(icon);
    if (!isVisible())
        showListMessages();
}

void UpSystemTrayIcon::showMessages(QString title, QStringList listmsg, QIcon icon, int duree)
{
    for (int i=0; i<listmsg.size(); ++i)
        showMessage(title, listmsg.at(i), icon, duree);
}

void UpSystemTrayIcon::showListMessages()
{
    if (list_messages.size() == 0 || list_icons.size() == 0)
    {
        hide();
        return;
    }
    if (!isVisible())
        show();
    int duree = list_messages.first()["duree"].toInt();
    QSystemTrayIcon::showMessage(list_messages.first()["titre"].toString(), list_messages.first()["texte"].toString(), list_icons.first(), duree);
    list_messages.removeAt(0);
    list_icons.removeAt(0);
    QTimer::singleShot(duree, this, &UpSystemTrayIcon::showListMessages);
}
