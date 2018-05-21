/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_MESSAGE_H
#define DLG_MESSAGE_H

#include <QDialog>
#include <QThread>
#include "uplabel.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QDebug>
#include <QDialog>

#include "functormessage.h"

#include "uptextedit.h"

/* Cette classe sert à afficher un message dans une fenêtre popo-up, sans bouton, sans bandeau de titre
Les paramètres sont :
QString mess    = le contenu du message - on peut mettre du html
int pause       = la durée d'affichage du message en ms (1000 par défaut)
bool bottom     = si true le message est affiché en bas à droite de l'écran, sinon, en plein centre (en bas à droite par défaut)
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN
*/
class dlg_message : public QObject
{
    Q_OBJECT
public:
    explicit        dlg_message(QStringList listmsg, int pause = 1000, bool bottom = true);
private:
    void            delay(int msec);
    QThread         *thread;
    FunctorMessage  fmessage;
};

#endif // DLG_MESSAGE_H
