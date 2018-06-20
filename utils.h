#ifndef UTILS_H
#define UTILS_H

#include <QRegExp>

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
};

#endif // UTILS_H
