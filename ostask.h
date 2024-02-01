#ifndef OSTASK_H
#define OSTASK_H

#include <QProcess>
#include <QThread>
#include <QDebug>

class OsTask : public QProcess
{
    Q_OBJECT

public:
    OsTask()
    {
        connect(this, &QProcess::finished, this, &OsTask::handleResults);
    }

    ~OsTask()
    {
        if (!isFinished()) {
            //qDebug() << "Waiting for Controller to finish...";
            waitForFinished(-1);
        }
    }

    bool isFinished()
    {
        return waitForFinished(1);
    }

    void execute(const QString &script)
    {
        if (!isFinished()) {
            //qDebug() << "Waiting for Controller to finish...";
            waitForFinished(-1);
        }

        startCommand(script);
    }

private slots:
    void handleResults(const int &exitCode)
    {
        int a = 99;
        //qDebug() << Utils::EnumDescription(QMetaEnum::fromType<QProcess::ExitStatus>(), dumpProcess.exitStatus());
        //qDebug() << Utils::EnumDescription(QMetaEnum::fromType<QProcess::ProcessError>(), dumpProcess.error());
        if (exitStatus() == QProcess::NormalExit)
            a = exitCode;
        emit result(a);
    }

signals:
    void result(const int &a);
};
#endif // OSTASK_H
