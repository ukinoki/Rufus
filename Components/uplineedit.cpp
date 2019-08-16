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

#include "uplineedit.h"
#include <QMessageBox>
UpLineEdit::UpLineEdit(QWidget *parent) : QLineEdit(parent)
{
    m_row        = -1;
    m_col     = -1;
    m_id              = -1;
    m_valeuravant     = "";
    m_valeurapres     = "";
    m_champ           = "";
    m_table           = "";
    m_datas        = QVariant();
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
}

UpLineEdit::~UpLineEdit()
{
}
// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------
bool UpLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        AfficheToolTip();
        return true;
    }
    if (event->type() == QEvent::FocusIn)
        setvaleuravant(text());
    if (event->type() == QEvent::FocusOut)
    {
        if (text() != m_valeuravant)
            emit TextModified(text());
        const QDoubleValidator *val= dynamic_cast<const QDoubleValidator*>(this->validator());
        if (val)
            if (text() != m_valeuravant)
                setText(QLocale().toString(QLocale().toDouble(text()),'f',2));
    }
    return QWidget::eventFilter(obj, event);
}
void UpLineEdit::enterEvent(QEvent *)
{
    if (m_row > -1)
        emit mouseEnter(m_row);
}

void UpLineEdit::mouseReleaseEvent(QMouseEvent *)
{
    if (m_row > -1)
        emit mouseRelease(m_row);
}

void UpLineEdit::mouseDoubleClickEvent(QMouseEvent *)
{
    if (m_row > -1)
        emit mouseDoubleClick(m_row);
}

void UpLineEdit::AfficheToolTip()
{
    if (m_tooltipmsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),m_tooltipmsg);
}

void UpLineEdit::setImmediateToolTip(QString Msg)
{
    m_tooltipmsg = Msg;
}

void UpLineEdit::setdatas(QVariant data)
{
    m_datas = data;
}

QVariant UpLineEdit::datas() const
{
    return m_datas;
}

void UpLineEdit::setiD(int Id)
{
    m_id = Id;
}
int UpLineEdit::iD() const
{
    return m_id;
}

void UpLineEdit::setRow(int val)
{
    m_row = val;
}
int UpLineEdit::Row() const
{
    return m_row;
}

void UpLineEdit::setColumn(int val)
{
    m_col = val;
}
int UpLineEdit::Column() const
{
    return m_col;
}
void UpLineEdit::setvaleuravant(QString valprec)
{
    m_valeuravant = valprec;
}

QString UpLineEdit::valeuravant() const
{
    return m_valeuravant;
}

void UpLineEdit::setvaleurapres(QString valpost)
{
    m_valeurapres = valpost;
}

QString UpLineEdit::valeurapres() const
{
    return m_valeurapres;
}

void UpLineEdit::setchamp(QString champcorrespondant)
{
    m_champ = champcorrespondant;
}

QString UpLineEdit::champ() const
{
    return m_champ;
}

void UpLineEdit::settable(QString tablecorrespondant)
{
    m_table = tablecorrespondant;
}

QString UpLineEdit::table() const
{
    return m_table;
}

