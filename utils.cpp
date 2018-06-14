#include "utils.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTime>


/*
 * Initialization des varaibles static const
*/
QRegExp const Utils::rgx_rx = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
QRegExp const Utils::rgx_AlphaNumeric = QRegExp("[A-Za-z0-9]*");
QRegExp const Utils::rgx_AlphaNumeric_3_15 = QRegExp("[A-Za-z0-9]{3,15}$");
QRegExp const Utils::rgx_AlphaNumeric_5_15 = QRegExp("[A-Za-z0-9]{5,15}$");
QRegExp const Utils::rgx_IPV4 = QRegExp("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");
QRegExp const Utils::rgx_IPV4_mask = QRegExp("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\."
                              "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])");

QRegExp const Utils::rgx_mail = QRegExp("[A-Za-z0-9_-]+.[A-Za-z0-9_-]+@[A-Za-z0-9_-]+.[A-Za-z0-9_-]+");
                              //QRegExp("^[a-zA-Z][\\w\\.-]*[a-zA-Z0-9]@[a-zA-Z0-9][\\w\\.-]*[a-zA-Z0-9]\\.[a-zA-Z][a-zA-Z\\.]*[a-zA-Z]$");
                              //QRegExp("^[0-9a-zA-Z]+([0-9a-zA-Z]*[-._+])*[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)*([0-9a-zA-Z]*[.])[a-zA-Z]{2,6}$");

QRegExp const Utils::rgx_adresse = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9°, -]*");
QRegExp const Utils::rgx_CP = QRegExp("[0-9]{5}");
QRegExp const Utils::rgx_ville = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ -]*");
QRegExp const Utils::rgx_telephone = QRegExp("[0-9 ]*");

QRegExp const Utils::rgx_tabac = QRegExp("[0-9]{2}");
QRegExp const Utils::rgx_cotation = QRegExp("[xsA-Z0-9.+/]*");  // le x pour BZQKOO1x1.5 et le s pour Cs

QRegExp const Utils::rgx_recherche = QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-Z %-]*");


//AUTRE QRegExp trouvé
//QRegExp("^[A-Za-z0-9]{5,15}$")
//QRegExp("[éêëèÉÈÊËàâÂÀîïÏÎôöÔÖùÙçÇ'a-zA-ZŒœ0-9 -]+") -> qlinedelegate

//JAMAIS UTILISE
//QRegExp const Utils::rgx_AVP = QRegExp("[1,5|1.5|2|3|4|5|6|8|10|14|28|<28]"); // JE NE COMPRENDS PAS, car 1,5 1.5 10 14 et 28 ne sortiront jamais

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
