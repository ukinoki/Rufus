#include "utils.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTime>

void Utils::Pause(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}
