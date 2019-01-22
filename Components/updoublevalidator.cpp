/* (C) 2018 LAINE SERGE
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

QValidator::State upDoubleValidator::validate(QString &s, int &i) const
{
    bool ok;
    i = 0;
    double d = QLocale().toDouble(s, &ok);

    if (s.isEmpty() || (ok && d==0.0))
        return QValidator::Intermediate;

    if (ok && d >= bottom() && d <= top())
        return QValidator::Acceptable;
    else
        return QValidator::Invalid;
}
