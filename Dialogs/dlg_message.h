/* (C) 2020 LAINE SERGE
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
#include <QHBoxLayout>

#include <QSystemTrayIcon>
#include <QTimer>
#include <QDebug>
#include <QDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QRegularExpression>
#include <QScreen>
#include <QTextEdit>
#include <QTime>

#include "icons.h"
#include "macros.h"

/* Cette classe sert à afficher un message dans une fenêtre pop-up, sans bouton, sans bandeau de titre
Les paramètres sont :
QString mess    = le contenu du message - on peut mettre du html
int pause       = la durée d'affichage du message en ms (1000 par défaut)
bool bottom     = si true le message est affiché en bas à droite de l'écran, sinon, en plein centre (en bas à droite par défaut)
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN
*/
class ShowMessage : public QObject
{
    Q_OBJECT
private:
    qintptr         idprioritymessage;
    QWidget         *m_parent = Q_NULLPTR;
    static ShowMessage *instance;
    ShowMessage();
    void            LogMessage(QString msg);
    bool            epureFontFamily(QString &text);     //! copied from utils.h to avoid circular reference

public:
    static ShowMessage* I();
    void SplashMessage(QString msg, int duree = 3000);
    void SplashMessage(QStringList listmsg, int duree = 3000)
    {
        for (int i=0; i<listmsg.size(); i++)
            SplashMessage(listmsg.at(i), duree);
    }
    void PriorityMessage(QString msg, qintptr &idmessage, int duree = 0, QWidget *parent= Q_NULLPTR);
    void ClosePriorityMessage(qintptr idmsg) { emit closeprioiritydlg(idmsg); }

signals:
    void closeprioiritydlg(qintptr iddlg);

};

#endif // DLG_MESSAGE_H
