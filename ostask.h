#ifndef OSTASK_H
#define OSTASK_H

#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QEventLoop>

class OsTask : public QObject
{
   Q_OBJECT
public slots:
    void        executeScript(const QString &script)
    {
        //qDebug() << script;
        QEventLoop loop;
        QProcess dumpProcess(parent());
        dumpProcess.start(script);
        while (dumpProcess.waitForFinished(1000000000))
        {
            loop.processEvents();
        }
        int a = 99;
        if (dumpProcess.exitStatus() == QProcess::NormalExit)
            a = dumpProcess.exitCode();
        emit resultReady(a);
    }

signals:
    void        resultReady(const int &result);
};

class Controller : public QObject
{
    Q_OBJECT
    QThread OsTaskThread;
private:
    OsTask *m_task = Q_NULLPTR;
public:
    ~Controller()
    {
        OsTaskThread.quit();
        OsTaskThread.wait();
    }
    void execute(const QString &script)
    {
        disconnect(SIGNAL(operate(const QString &)));
        OsTaskThread.disconnect();
        if (m_task != Q_NULLPTR)
            delete m_task;
        m_task = new OsTask();
        m_task->moveToThread(&OsTaskThread);
        connect(&OsTaskThread,  &QThread::finished,     m_task, &QObject::deleteLater);
        connect(this,           &Controller::operate,   m_task, &OsTask::executeScript);
        connect(m_task,         &OsTask::resultReady,   this,   &Controller::handleResults);
        OsTaskThread.start();
        emit operate(script);
    }
private slots:
    void handleResults(const int &a) { emit result(a);}
signals:
    void operate(const QString &);
    void result(const int &a);
};
#endif // OSTASK_H
