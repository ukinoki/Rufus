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

#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <utils.h>

#ifdef Q_OS_WIN
class SerialThread : public QObject
{
    Q_OBJECT

public:
    explicit        SerialThread(QSerialPort *PortProc, QString NomApp = "");
    void            transaction();

signals:
    void            newdatacom(const QString &s);

private:
    QByteArray      reponseData;
    QSerialPort     *Port;
    QString         m_nomapp = "";
    void            LitPort();
    QThread         *m_thread;
    QTimer          *t_timer;
    void            readTimer();
};

#else
class SerialThread : public QThread
{
    Q_OBJECT

public:
    explicit        SerialThread(QSerialPort *PortProc, QString NomApp = "");
    void            transaction();
    void            run() Q_DECL_OVERRIDE;

signals:
    void            newdatacom(const QString &s);

private:
    QByteArray      reponseData;
    QSerialPort     *Port;
    QString         m_nomapp = "";
    void            LitPort();
};
#endif
#endif // SERIALTHREAD_H

