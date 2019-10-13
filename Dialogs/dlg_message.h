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

#ifndef DLG_MESSAGE_H
#define DLG_MESSAGE_H

#include <QDialog>
#include <QThread>
#include "uplabel.h"
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QDebug>
#include <QDialog>
#include <QScreen>
#include "icons.h"
#include "uptextedit.h"

/* Cette classe sert à afficher un message dans une fenêtre popo-up, sans bouton, sans bandeau de titre
Les paramètres sont :
QString mess    = le contenu du message - on peut mettre du html
int pause       = la durée d'affichage du message en ms (1000 par défaut)
bool bottom     = si true le message est affiché en bas à droite de l'écran, sinon, en plein centre (en bas à droite par défaut)
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN
*/
class Message : public QObject
{
    Q_OBJECT

public:
    static Message *instance;
    static Message* I()
    {
        if( !instance )
            instance = new Message();
        return instance;
    }
    void TrayMessage(QString msg, int duree = 3000)
    {
        ict_messageIcon->setIcon(Icons::icSunglasses());
        ict_messageIcon->show();
        ict_messageIcon->showMessage(tr("Messages"), msg, Icons::icSunglasses(), duree);
        QTimer::singleShot(duree + 200, this, [=]{ ict_messageIcon->hide();});
        LogMessage(msg);
    }
    void TrayMessage(QStringList listmsg, int duree = 3000)
    {
        for (int i=0; i<listmsg.size(); i++)
            TrayMessage(listmsg.at(i), duree);
    }

private:
    Message() {}
    QSystemTrayIcon *ict_messageIcon = new QSystemTrayIcon;
    void LogMessage(QString msg);

};

#endif // DLG_MESSAGE_H
