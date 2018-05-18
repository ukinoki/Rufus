/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UPDOUBLEVALIDATOR_H
#define UPDOUBLEVALIDATOR_H

#include <QDoubleValidator>
#include <QValidator>
#include <QLocale>

class upDoubleValidator: public QDoubleValidator
{
public:
    upDoubleValidator(double bottom, double top, int decimals, QObject * parent);
    QValidator::State validate(QString &s, int &i) const;
};

#endif // UPDOUBLEVALIDATOR_H
