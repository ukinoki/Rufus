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

#ifndef UPDOUBLESPINBOX_H
#define UPDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QSound>
#include <cmath>            // sert à introduire la fonction floor()
#include "upmessagebox.h"

class UpDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit UpDoubleSpinBox(QWidget *parent = Q_NULLPTR);
    ~UpDoubleSpinBox();
    void                    setvaleuravant(double valprec);
    double                  valeuravant() const;
    void                    setAutorCorrigeDioptr(bool);
    enum gUpDown            {Up,Down,Near};    Q_ENUM(gUpDown)
    void                    CorrigeDioptrie(int UpDownNear);

private:
    bool                    eventFilter(QObject *, QEvent *);
    double                  m_valeuravant;
    bool                    m_correctiondioptries;
};

#endif // UPDOUBLESPINBOX_H
