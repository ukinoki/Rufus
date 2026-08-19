/* (C) 2024 LAINE SERGE
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

#include "upserialport.h"

upSerialPort::upSerialPort(QString portname, QString NomApp, QObject *parent) : QSerialPort(portname, parent)
{
    m_nomapp        = NomApp;
    if (t_timer == nullptr)
    {
        t_timer = new QTimer (this);
        t_timer->setInterval(100);
    }
    connect(this,   &QSerialPort::readyRead, this, &upSerialPort::readDataPort);
    connect(this,   &QSerialPort::errorOccurred, this, [&] (QSerialPort::SerialPortError error){
        QMetaEnum metaenum = QMetaEnum::fromType<QSerialPort::SerialPortError>();
        if (QString(metaenum.valueToKey(error)).toLocal8Bit() != "NoError")
            qDebug() << "erreur portCOM " + m_nomapp << metaenum.valueToKey(error);
    });
}

void upSerialPort::readDataPort()
{
    m_repons = "";
    t_timer->start();
    connect (t_timer, &QTimer::timeout, this, &upSerialPort::readdatas);
}

void upSerialPort::readdatas()
{
    QByteArray dataprov = readAll();
    QByteArray b(Utils::cleanByteArray(dataprov));
    QString addreponse = QString::fromLocal8Bit(b);
    m_repons += addreponse;
    if (dataprov.size() == 0)
    {
        t_timer->stop();
        t_timer->disconnect();
        qDebug() << "respons" << m_repons;
        emit newdatacom(m_repons);
        clear();
    }
}

void upSerialPort::writeDatas(QByteArray datas, QString msgdebug)
{
    qDebug() << "envoi" << QString::fromLocal8Bit(Utils::cleanByteArray(datas)) << msgdebug;
    write(datas);
    flush();
    #ifdef Q_OS_WIN
    int timetowaitms = int (datas.size()*8*1000 / baudRate());
    timetowaitms += 10;
    waitForBytesWritten(timetowaitms);
    #endif
}
