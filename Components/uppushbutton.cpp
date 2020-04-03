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

#include "uppushbutton.h"
#include <QApplication>
#include "icons.h"
#include "styles.h"

UpPushButton::UpPushButton(QWidget *parent) : QPushButton(parent)
{
    setAutoDefault(false);
    setFocusPolicy(Qt::StrongFocus);
    m_tooltipmsg = "";
    installEventFilter(this);
    setFlat(false);
    setStyleSheet(STYLE_UPPUSHBUTTON);
    setContextMenuPolicy(Qt::NoContextMenu);
}

UpPushButton::UpPushButton(QString text, QWidget *parent) : UpPushButton(parent)
{
    setText(text);
}

UpPushButton::~UpPushButton()
{
}

bool UpPushButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        if (m_tooltipmsg != "")
            if (isEnabled())
                QToolTip::showText(cursor().pos(),m_tooltipmsg);
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

int UpPushButton::data() const
{
    return m_data;
}

void UpPushButton::setData(int data)
{
    m_data = data;
}

void UpPushButton::setImmediateToolTip(QString Msg)
{
    m_tooltipmsg = Msg;
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

void UpPushButton::setiD(int idadef)
{
    m_id = idadef;
}

int UpPushButton::iD() const
{
    return m_id;
}
