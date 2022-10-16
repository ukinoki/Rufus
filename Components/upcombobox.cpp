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

#include "upcombobox.h"

UpComboBox::UpComboBox(QWidget *parent) : QComboBox (parent)
{
    m_valeuravant     = "";
    m_valeurapres     = "";
    m_champ           = "";
    m_table           = "";
    m_id              = -1;
    m_indexpardefaut  = -1;
    m_tooltipmsg     = "";
    setContextMenuPolicy(Qt::NoContextMenu);
    installEventFilter(this);
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),  this, &UpComboBox::clearImmediateToolTip);
}

UpComboBox::~UpComboBox()
{

}

void UpComboBox::clearImmediateToolTip()
{
    if (currentIndex()==-1) setImmediateToolTip("");
}

// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------
bool UpComboBox::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
        setvaleuravant(currentText());
    if (event->type() == QEvent::FocusOut)
    {
        if (lineEdit()!=Q_NULLPTR)
        {
            if (lineEdit()->text() != "")
            {
                if (!lineEdit()->hasAcceptableInput())
                {
                    //QString ab = lineEdit()->text();
                    setCurrentText(valeuravant());
                    //QRegExpValidator const * reg = static_cast<QRegExpValidator const*>(lineEdit()->validator());
                    //UpMessageBox::Watch(this,reg->regExp().pattern() + "\n'" + ab + "'");
                }
            }
            else
                setCurrentIndex(-1);
        }
    }
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Delete)            // on tape SUPPR on remet à zero si on est à la fin du lineEdit
            if (lineEdit()->cursorPosition() == lineEdit()->text().length())
                lineEdit()->setText(itemText(m_indexpardefaut));
        if (keyEvent->key() == Qt::Key_Escape)
            if (currentText() != valeuravant())
            {
                setCurrentText(valeuravant());
                setCurrentIndex(findText(currentText()));
                return true;
            }
        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
            if (currentText() != "")
            {
                int idx = findText(currentText(),Qt::MatchStartsWith);
                setCurrentIndex(idx);
            }
    }
    if (event->type() == QEvent::Enter)
    {
        if (m_tooltipmsg != "" && isEnabled())
            QToolTip::showText(cursor().pos(),m_tooltipmsg);
    }
    return QWidget::eventFilter(obj, event);
}

void UpComboBox::mouseDoubleClickEvent(QMouseEvent *)
{
    emit mouseDoubleClick(currentIndex());
}


void UpComboBox::setiD(int valprec)
{
    m_id = valprec;
}

int UpComboBox::iD() const
{
    return m_id;
}

void UpComboBox::setIndexParDefaut(int defaut)
{
    if (defaut < count())
        m_indexpardefaut = defaut;
    else
        m_indexpardefaut = 0;
}

int UpComboBox::IndexParDefaut() const
{
    if (m_indexpardefaut < count())
        return m_indexpardefaut;
    else return 0;
}

void UpComboBox::setvaleuravant(QString valprec)
{
    m_valeuravant = valprec;
}

QString UpComboBox::valeuravant() const
{
    return m_valeuravant;
}

void UpComboBox::setvaleurapres(QString valpost)
{
    m_valeuravant = valpost;
}

QString UpComboBox::valeurapres() const
{
    return m_valeurapres;
}

void UpComboBox::setchamp(QString champcorrespondant)
{
    m_champ = champcorrespondant;
}

QString UpComboBox::champ() const
{
    return m_champ;
}

void UpComboBox::setTable(QString tablecorrespondant)
{
    m_table = tablecorrespondant;
}

QString UpComboBox::table() const
{
    return m_table;
}

void UpComboBox::setImmediateToolTip(QString Msg)
{
    m_tooltipmsg = Msg;
}

