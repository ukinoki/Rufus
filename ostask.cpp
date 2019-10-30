#include "ostask.h"

void OsTask::executeScript(const QString &script)
{
    //qDebug() << script;
    QProcess dumpProcess(parent());
    dumpProcess.start(script);
    dumpProcess.waitForFinished(1000000000);
    int a = 99;
    if (dumpProcess.exitStatus() == QProcess::NormalExit)
        a = dumpProcess.exitCode();
    emit resultReady(a);
}
