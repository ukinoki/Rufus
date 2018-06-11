#include "utils.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTime>


/*
 * Initialization des varaibles static const
*/
QRegExp const Utils::rgx_AlphaNumeric = QRegExp("[A-Za-z0-9]*");
QRegExp const Utils::rgx_IPV4 = QRegExp("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");



/*!
 *  \brief Pause
 *
 *  Methode qui permet d'attendre un certain temps (donné en paramètre)
 *
 *  \param msec : le temps à attendre en milliseconde
 *
 */
void Utils::Pause(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}
