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

#include "upspinbox.h"

UpSpinBox::UpSpinBox(QWidget *parent) : QSpinBox(parent)
{
    ValeurAvant = 0;
    installEventFilter(this);
    setKeyboardTracking(false);
    gCorrigeAxe = true;
    setContextMenuPolicy(Qt::NoContextMenu);
}
UpSpinBox::~UpSpinBox()
{

}
bool UpSpinBox::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        setValeurAvant(value());
    if (event->type() == QEvent::KeyPress)
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
            if (gCorrigeAxe) CorrigeAxe(Up);
            return true;
        }
        if(keyEvent->key()==Qt::Key_Down)
        {
            if (gCorrigeAxe) CorrigeAxe(Down);
            return true;
        }
    }
    return QWidget::eventFilter(obj,event);
}

//---------------------------------------------------------------------------------
// Corrige saisie axe dans un spinbox
//---------------------------------------------------------------------------------
void UpSpinBox::CorrigeAxe(int UpDown)
{
    setValue(valueFromText(text()));
    int a = value()/singleStep();
    switch (UpDown) {
    case Up:
        setValue((a+1)*singleStep());
        break;
    case Down:
        if ((value() - a*singleStep()) == 0)
            setValue((a-1)*singleStep());
        else
            setValue(a*singleStep());
        break;
    default:
        break;
    }
}

void UpSpinBox::setAutorCorrigeAxe(bool Autor)
{
    gCorrigeAxe = Autor;
}

void UpSpinBox::setValeurAvant(int ValPrec)
{
    ValeurAvant = ValPrec;
}
int UpSpinBox::getValeurAvant() const
{
    return ValeurAvant;
}
