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

#include "serialthread.h"

/*SerialThread::SerialThread(QSerialPort *PortProc)
{
    moveToThread(&thread);
    Port            = PortProc;
    connect(Port,   &QSerialPort::readyRead, this, &SerialThread::LitPort);
    thread.start();
}

void SerialThread::LitPort()
{
    reponseData = Port->readAll();
    while (Port->waitForReadyRead(100))
        reponseData.append(Port->readAll());
    QString ReponsePort(reponseData);
    if (ReponsePort != "")
    {
        Port->clear();
        reponseData.clear();
        emit reponse(ReponsePort);
    }
}*/

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
    connect(Port,   &QSerialPort::readyRead, this, &SerialThread::LitPort);
}

void SerialThread::LitPort()
{
    QByteArray reponseData = Port->readAll();
    while (Port->waitForReadyRead(100))
        reponseData += Port->readAll();
    QString ReponsePort(reponseData);   
#ifdef Q_OS_WIN
        Utils::writeDataToFileDateTime(reponseData, "Received.bin","c:/outils/log/Phoromat/");
#endif
    if (ReponsePort != "")
    {
        emit newdatacom(ReponsePort);
        Port->clear();
    }
}
