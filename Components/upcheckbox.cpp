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

#include "upcheckbox.h"

UpCheckBox::UpCheckBox(QWidget *parent)
    : QCheckBox(parent)
{
    // Les membres sont initialisés dans le header (member-initialization).
    // Seules les initialisations non triviales restent ici.
    setContextMenuPolicy(Qt::NoContextMenu);
    setFont(qApp->font());
    installEventFilter(this);
}

UpCheckBox::UpCheckBox(const QString &text, QWidget *parent)
    : UpCheckBox(parent)   // délégation : constructeur de base exécuté en entier
{
    setText(text);
    // setFont() n'est pas répété : il a déjà été appelé par UpCheckBox(parent).
}

// ─────────────────────────────────────────────────────────────────────────────
void UpCheckBox::afficheToolTip()
{
    if (!m_tooltipmsg.isEmpty() && isEnabled())
        QToolTip::showText(cursor().pos(), m_tooltipmsg);
}

bool UpCheckBox::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        emit enter();
        afficheToolTip();
        // On ne consomme pas l'événement (return false) afin que
        // les widgets parents puissent également le recevoir.
        return false;
    }
    if (event->type() == QEvent::MouseButtonPress
     || event->type() == QEvent::MouseButtonDblClick) {
        if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton
         && !m_toggleable)
            return true;   // consommer le clic pour bloquer le basculement
    }
    return QWidget::eventFilter(obj, event);
}

// ─────────────────────────────────────────────────────────────────────────────
void UpCheckBox::setImmediateToolTip(const QString &msg)
{
    m_tooltipmsg = msg;
}

int  UpCheckBox::columntable() const              { return m_columntable; }
void UpCheckBox::setColumntable(int v)            { m_columntable = v; }
void UpCheckBox::setRowTable(int val)             { m_rowtable = val; }
int  UpCheckBox::rowTable() const                 { return m_rowtable; }
void UpCheckBox::setiD(int val)                   { m_id = val; }
int  UpCheckBox::iD() const                       { return m_id; }
void UpCheckBox::setToggleable(bool val)          { m_toggleable = val; }
bool UpCheckBox::Toggleable() const               { return m_toggleable; }
