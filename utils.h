#ifndef UTILS_H
#define UTILS_H

#include <QRegExp>

class Utils
{
public:
    static QRegExp const rgx_AlphaNumeric;
    static QRegExp const rgx_IPV4;


    static void Pause(int msec = 1000);
};

#endif // UTILS_H
