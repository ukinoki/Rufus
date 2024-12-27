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

SerialThread::SerialThread(QSerialPort *PortProc, QString NomApp)
{
    Port = PortProc;
    m_nomapp        = NomApp;
    m_thread = new QThread(this);
    moveToThread(m_thread);
    connect(m_thread,   &QThread::started,          this, [&]{
                                                                connect(Port,   &QSerialPort::readyRead, this, &SerialThread::LitPort);
                                                                connect(Port,   &QSerialPort::errorOccurred, this, [&] (QSerialPort::SerialPortError error){qDebug() << "erreur portCOM " + m_nomapp << Utils::EnumDescription(QMetaEnum::fromType<QSerialPort::SerialPortError>(), error);});
                                                              });
    connect(this,       &SerialThread::finished,    m_thread,   &QThread::quit);
    connect(this,       &SerialThread::finished,    this,       &QObject::deleteLater);
    connect(m_thread,   &QThread::finished,         m_thread,   &QThread::deleteLater);
    QTimer *t_timer = new QTimer(0);
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
        qDebug() << "reception" << QString::fromLocal8Bit(b);
        emit newdatacom(QString::fromLocal8Bit(b));
        t_timer->stop();
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

