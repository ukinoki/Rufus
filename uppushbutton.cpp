/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "uppushbutton.h"
#include <QApplication>
#include "icons.h"

UpPushButton::UpPushButton(QWidget *parent) : QPushButton(parent)
{
    setAutoDefault(false);
    setFocusPolicy(Qt::StrongFocus);
    gToolTipMsg = "";
    installEventFilter(this);
    setFlat(false);
    setStyleSheet("UpPushButton {border: 1px solid gray; border-radius: 5px; margin-left: 5px; margin-right: 5px;  margin-top: 3px; margin-bottom: 3px;"
                  " padding-left: 8px; padding-right: 8px; padding-top: 3px; padding-bottom: 3px; qproperty-iconSize: 30px 30px; qproperty-flat: false;"
                  " color : #000000;"
                  " background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde);}"
                  " UpPushButton:focus {color : #000000; border: 2px solid rgb(164, 205, 255); border-radius: 5px;}"
                  " UpPushButton:pressed {color : gray; background-color: rgb(175, 175, 175);}"
                  " UpPushButton:!enabled {color : gray;}");
    setContextMenuPolicy(Qt::NoContextMenu);
}

UpPushButton::~UpPushButton()
{
}

bool UpPushButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        if (gToolTipMsg != "")
            if (isEnabled())
                QToolTip::showText(cursor().pos(),gToolTipMsg);
    }
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key()==Qt::Key_Return  && keyEvent->modifiers() == Qt::NoModifier) || keyEvent->key() == Qt::Key_Enter)
        {
            emit clicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void UpPushButton::setImmediateToolTip(QString Msg)
{
    gToolTipMsg = Msg;
}

void UpPushButton::setUpButtonStyle(enum StyleBouton Style, enum TailleBouton Taille)
{
    int width;
    switch (Taille) {
    case Large:
        width = 170;
        break;
    case Mid:
        width = 140;
        break;
    default:
        width = 115;
        break;
    }
    resize(width,52);

    switch (Style) {
    case OKBUTTON:
        setShortcut(QKeySequence("Meta+Return"));
        setIcon(Icons::icOK());
        setText("OK");
        move(parentWidget()->size().width()-width-8,parentWidget()->size().height()-60);
        break;
    case ANNULBUTTON:
        setShortcut(QKeySequence("F12"));
        setIcon(Icons::icAnnuler());
        setText(tr("Annuler"));
        move(parentWidget()->size().width()-width-width-8,parentWidget()->size().height()-60);
        break;
    case IMPRIMEBUTTON:
        setIcon(Icons::icImprimer());
        setText(tr("Imprimer"));
        move(parentWidget()->size().width()-width-width-width-8,parentWidget()->size().height()-60);
        break;
    default:
        setShortcut(QKeySequence());
        break;
    }
}

void UpPushButton::setId(int idadef)
{
    id = idadef;
}

int UpPushButton::getId() const
{
    return id;
}
