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

#include "updoublespinbox.h"

UpDoubleSpinBox::UpDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    ValeurAvant = 0;
    installEventFilter(this);
    setKeyboardTracking(false);
    gCorrigeDioptr = true;
    setContextMenuPolicy(Qt::NoContextMenu);
}
UpDoubleSpinBox::~UpDoubleSpinBox()
{
}

bool UpDoubleSpinBox::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )           setValeurAvant(value());
    if (event->type() == QEvent::FocusOut )
    {
        setValue(valueFromText(text()));    // cette acrobatie est nécessaire parce que la fonction value() ne marche pas
                                            // et retourne la valeur en entrant et pas celle en sortant...
        if (gCorrigeDioptr) CorrigeDioptrie(Near);
    }
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape)
            if (valueFromText(text()) != getValeurAvant())    {
                setValue(getValeurAvant());
                return true;
            }
        if (keyEvent->key() == Qt::Key_Delete)            // on tape SUPPR on remet à zero
        {
            setValue(0);
            selectAll();
            return true;
        }
        if(keyEvent->key()==Qt::Key_Up)
        {
            if (gCorrigeDioptr) CorrigeDioptrie(Up);
            return true;
        }
        if(keyEvent->key()==Qt::Key_Down)
        {
            if (gCorrigeDioptr) CorrigeDioptrie(Down);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

//---------------------------------------------------------------------------------
// Corrige saisie dioptrie dans un doublespinbox
//---------------------------------------------------------------------------------
void UpDoubleSpinBox::CorrigeDioptrie(int UpDownNear)
{
    int a,b ;
    double c;
    a =  floor(value()/singleStep());
    switch (UpDownNear) {
    case Up:
        setValue(a*singleStep()+singleStep());
        break;
    case Down:
        if (a == value()/singleStep())
            setValue(a*singleStep()-singleStep());
        else
            setValue(a*singleStep());
        break;
    case Near:
        b = value()*10;
        c = value()/singleStep()*10;
        if (a != value()/singleStep())
        {
            if (((singleStep() == 0.25) && ((abs(b)%10 != 2 && abs(b)%10 != 7) || b != value()*10))
                    || singleStep() == 0.50)
                {
                    QSound::play("://goutte.wav");
                    UpMessageBox::Watch(this,tr("Valeur non conforme"),tr("La valeur sera arrondie à la\nvaleur conforme la plus proche!"));
                }
            setValue(round(c/10)*singleStep());
        }
        break;
    default:
        break;
    }
    if (value() >= 0)
        setPrefix("+");
    else setPrefix("");
}

void UpDoubleSpinBox::setAutorCorrigeDioptr(bool Autor)
{
    gCorrigeDioptr = Autor;
}

void UpDoubleSpinBox::setValeurAvant(double valprec)
{
    ValeurAvant = valprec;
}

double UpDoubleSpinBox::getValeurAvant() const
{
    return ValeurAvant;
}

