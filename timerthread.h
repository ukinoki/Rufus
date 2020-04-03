/* (C) 2020 LAINE SERGE
This file is part of RufusAdmin.

RufusAdmin is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H



/*! Cette classe permet d'éxécuter un timer dans un thread de manière à le rebdre indépendant de l'activité du programme
 * elle s'utilise comme un QTimer avec les mêmes commandes
    * stop
    * start(int interval)
    * start()
    * setinterval(int interval)
 * elle utilise le même signal timeout()
 * elle se pilote donc exactement comme un QTimer classique */


#include <QTimer>
#include <QThread>
#include <QTime>

class TimerController : public QObject
{
    Q_OBJECT
public:
    QThread TimerThread;
private:
    QTimer *m_timer = Q_NULLPTR;
public:
    ~TimerController() {
        TimerThread.quit();
        TimerThread.wait();
    }
    TimerController(QObject * parent = Q_NULLPTR) : QObject(parent)
    {
        TimerThread.disconnect();
        if (m_timer != Q_NULLPTR)
            delete m_timer;
        m_timer = new QTimer();
        m_timer->moveToThread(&TimerThread);
        m_timer->setTimerType(Qt::PreciseTimer);
        connect(&TimerThread,   &QThread::finished,                                 m_timer, &QObject::deleteLater);
        connect(this,           QOverload<int>::of(&TimerController::starttimer),   m_timer, QOverload<int>::of(&QTimer::start));
        connect(this,           QOverload<>::of(&TimerController::starttimer),      m_timer, QOverload<>::of(&QTimer::start));
        connect(this,           &TimerController::stoptimer,                        m_timer, &QTimer::stop);
        connect(this,           &TimerController::setintervaltimer,                 m_timer, QOverload<int>::of(&QTimer::setInterval));
        connect(m_timer,        &QTimer::timeout,                                   this,    &TimerController::setTimeOut);
        TimerThread.start();
    }
    void start(int interval)                { emit starttimer(interval); }
    void start()                            { emit starttimer(); }
    void stop()                             { emit stoptimer(); }
    void setInterval(int interval = 1000)   { emit setintervaltimer(interval); }

private slots:
    void setTimeOut() { emit timeout();}

signals:
    void starttimer(int interval);
    void starttimer();
    void stoptimer();
    void setintervaltimer (int interval);
    void timeout();
};

#endif // TIMERTHREAD_H
