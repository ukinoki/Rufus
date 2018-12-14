#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QRegExp>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QHostInfo>
#include "uplineedit.h"
#include "uplabel.h"
#include "upmessagebox.h"
#include <QInputDialog>

class Utils
{
public:
    static QRegExp const rgx_rx;
    static QRegExp const rgx_AlphaNumeric;
    static QRegExp const rgx_AlphaNumeric_3_15;
    static QRegExp const rgx_AlphaNumeric_5_15;

    static QRegExp const rgx_IPV4;
    static QRegExp const rgx_IPV4_mask;

    static QRegExp const rgx_mail;

    static QRegExp const rgx_adresse;
    static QRegExp const rgx_CP;
    static QRegExp const rgx_ville;
    static QRegExp const rgx_telephone;

    static QRegExp const rgx_tabac;
    static QRegExp const rgx_cotation;

    static QRegExp const rgx_recherche;


    static void Pause(int msec = 1000);
    static QString convertHTML(QString text);

    static QString trim(QString text, bool end=true);
    static QString capitilize(QString text);
    static QString trimcapitilize(QString, bool end = true, bool maj = true, bool lower = true);
    static QMap<QString, double> dir_size(const QString DirPath);
    static qint32  ArrayToInt(QByteArray source);
    static QByteArray IntToArray(int source);
    static QString getIpAdress();
    static QString getMACAdress();
    static QString correctquoteSQL(QString text);
    static bool VerifMDP(QString MDP, QString Msg, bool mdpverified=false);
};

#endif // UTILS_H
