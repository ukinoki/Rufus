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

#include "serialthread.h"
#include <QCoreApplication>

QT_USE_NAMESPACE

SerialThread::SerialThread(QSerialPort *PortProc)
{
    Port            = PortProc;
}

void SerialThread::transaction()
{
    if (!isRunning())
        start();
}

void SerialThread::run()
{
    connect(Port,   SIGNAL(readyRead()), this, SLOT(Slot_LitPort()));
}

void SerialThread::Slot_LitPort()
{
    QByteArray reponseData = Port->readAll();
    while (Port->waitForReadyRead(100))
        reponseData += Port->readAll();
    QString ReponsePort(reponseData);
    if (ReponsePort != "")
    {
        emit reponse(ReponsePort);
        Port->clear();
    }
}

