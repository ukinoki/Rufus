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
#include "ascii.h"

#ifdef Q_OS_WIN

SerialThread::SerialThread(QSerialPort *PortProc)
{
    Port = PortProc;
    m_thread = new QThread(this);
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, [&]{
                                connect(Port,   &QSerialPort::readyRead, this, &SerialThread::LitPort);
                                connect(Port,   &QSerialPort::errorOccurred, this, [] (QSerialPort::SerialPortError error){qDebug() << "erreur portCOM " << Utils::EnumDescription(QMetaEnum::fromType<QSerialPort::SerialPortError>(), error);});
                                });

    t_timer = new QTimer(0);
    t_timer->setInterval(100);
    t_timer->moveToThread(m_thread);
    connect(t_timer,&QTimer::timeout, this, &SerialThread::readTimer);
}

void SerialThread::readTimer()
{
    reponseData = Port->readAll();
    char buffer[50];
    int numRead = Port->read(buffer, 50);
    if (numRead == 0)
    {
        QByteArray b(Utils::cleanByteArray(reponseData));
//        b.replace(CR,' ');  //replace CR (\n) for space
//        b.replace(LF,'|');  //replace CR (\n) for separator
        emit newdatacom(QString::fromLocal8Bit(b));
        t_timer->stop();
        //Utils::writeDataToFileDateTime(reponseData, "Received.bin","c:/outils/log");
    }
    else
        reponseData += Port->readAll();
    Port->clear();
}

void SerialThread::LitPort()
{
    t_timer->start();
}
void SerialThread::transaction()
{
    if (!m_thread->isRunning())
        m_thread->start();
}

#else
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
    connect(Port,   &QSerialPort::errorOccurred, this, [] (QSerialPort::SerialPortError error){qDebug() << "erreur portCOM " << Utils::EnumDescription(QMetaEnum::fromType<QSerialPort::SerialPortError>(), error);});
}

void SerialThread::LitPort()
{
    QByteArray reponseData = Port->readAll();
    while (Port->waitForReadyRead(100))
        reponseData += Port->readAll();
    QString ReponsePort(Utils::cleanByteArray(reponseData));
    if (ReponsePort != "")
    {
        emit newdatacom(ReponsePort);
        Port->clear();
    }
}
#endif
