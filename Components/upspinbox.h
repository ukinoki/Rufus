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

#ifndef UPSPINBOX_H
#define UPSPINBOX_H

#include <QSpinBox>
#include <QKeyEvent>
#include <cmath>            // sert à introduire la fonction floor()

class UpSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit        UpSpinBox(QWidget *parent = Q_NULLPTR);
    ~UpSpinBox();
    void            setvaleuravant(int ValPrec);
    int             valeuravant() const;
    enum UpDown     {Up,Down};      Q_ENUM(UpDown)
    void            setAutorCorrigeAxe(bool);
    void            CorrigeAxe(int UpDown);

private:
    int             m_valeuravant;
    bool            eventFilter(QObject *, QEvent *);
    bool            gCorrigeAxe;

};

#endif // UPSPINBOX_H
