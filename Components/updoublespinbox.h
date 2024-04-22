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

#ifndef UPDOUBLESPINBOX_H
#define UPDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QSoundEffect>
#include <cmath>            // sert à introduire la fonction floor()

#include "utils.h"
class Utils;

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
    void                    CorrigeDioptrie(int UpDownNear);            /*! corrige une valeur entrée après la virgule à la valeur de step la plus proche - 21,37 devient 21,50 si le step est réglé à 0,50 */
    void                    PrefixePlus();                              //! convertit en QString signé + ou - la valeur
    void                    setValuewithPrefix(double ValeurDouble);

private:
    bool                    eventFilter(QObject *, QEvent *);
    double                  m_valeuravant;
    bool                    m_correctiondioptries;
};

#endif // UPDOUBLESPINBOX_H
