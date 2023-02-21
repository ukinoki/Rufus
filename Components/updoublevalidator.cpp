/* (C) 2020 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "updoublevalidator.h"

upDoubleValidator::upDoubleValidator(double bottom, double top, int decimals, QObject * parent) :
    QDoubleValidator(bottom, top, decimals, parent)
{
}

QValidator::State upDoubleValidator::validate(QString &s, int &) const {
    //qDebug() << s;
    if (s.isEmpty() || (s.startsWith("-") && s.length() == 1)) {
        // allow empty field or standalone minus sign
        return QValidator::Intermediate;
    }
    // check length of decimal places
    QString point = locale().decimalPoint();
    if(s.indexOf(point) != -1) {
        int lengthDecimals = s.length() - s.indexOf(point) - 1;
        if (lengthDecimals > decimals()) {
            return QValidator::Invalid;
        }
    }
    // check range of value
    bool isNumber;
    double value = locale().toDouble(s, &isNumber);
    //qDebug() << "isNumber " << isNumber << " - value " << value<< " - bottom() " << bottom() << " - top() " << top();
    if (isNumber && bottom() <= value && value <= top()) {
        return QValidator::Acceptable;
    }
    return QValidator::Invalid;
}

