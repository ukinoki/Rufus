/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_message.h"

dlg_message::dlg_message(QStringList listmsg, int pause, bool bottom)
{
    thread = new QThread;
    thread->start();
    moveToThread(thread);
    fmessage(listmsg, pause, bottom);
    thread->exit();
}

void dlg_message::delay(int msec)
{
    QEventLoop loop;
    QTimer t;
    t.setSingleShot(true);
    connect(&t, SIGNAL(timeout()), &loop, SLOT(quit()));
    t.start(msec);
    loop.exec();
}
