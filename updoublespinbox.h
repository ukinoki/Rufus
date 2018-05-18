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

#ifndef UPDOUBLESPINBOX_H
#define UPDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QSound>
#include <QMessageBox>
#include <cmath>            // sert à introduire la fonction floor()
#include "upmessagebox.h"

class UpDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit UpDoubleSpinBox(QWidget *parent = Q_NULLPTR);
    ~UpDoubleSpinBox();
    void                    setValeurAvant(double valprec);
    double                  getValeurAvant() const;
    void                    setAutorCorrigeDioptr(bool);
    int                     gUpDown;
    enum gUpDown            {Up,Down,Near};
    bool                    gCorrigeDioptr;
    void                    CorrigeDioptrie(int UpDownNear);

private:
    bool                    eventFilter(QObject *, QEvent *);
    double                  ValeurAvant;
};

#endif // UPDOUBLESPINBOX_H
