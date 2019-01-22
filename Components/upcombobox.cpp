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

#include "upcombobox.h"

UpComboBox::UpComboBox(QWidget *parent) : QComboBox (parent)
{
    ValeurAvant     = "";
    ValeurApres     = "";
    Champ           = "";
    Table           = "";
    id              = -1;
    IndexParDefaut  = -1;
    gToolTipMsg     = "";
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
        setValeurAvant(currentText());
    if (event->type() == QEvent::FocusOut)
    {
        if (lineEdit()!=Q_NULLPTR)
        {
            if (lineEdit()->text() != "")
            {
                if (!lineEdit()->hasAcceptableInput())
                {
                    QString ab = lineEdit()->text();
                    setCurrentText(getValeurAvant());
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
                lineEdit()->setText(itemText(IndexParDefaut));
        if (keyEvent->key() == Qt::Key_Escape)
            if (currentText() != getValeurAvant())
            {
                setCurrentText(getValeurAvant());
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
        if (gToolTipMsg != "" && isEnabled())
            QToolTip::showText(cursor().pos(),gToolTipMsg);
    }
    return QWidget::eventFilter(obj, event);
}

void UpComboBox::mouseDoubleClickEvent(QMouseEvent *)
{
    emit mouseDoubleClick(currentIndex());
}


void UpComboBox::setid(int valprec)
{
    id = valprec;
}

int UpComboBox::getid() const
{
    return id;
}

void UpComboBox::setIndexParDefaut(int defaut)
{
    if (defaut < count())
        IndexParDefaut = defaut;
    else
        IndexParDefaut = 0;
}

int UpComboBox::getIndexParDefaut() const
{
    if (IndexParDefaut < count())
        return IndexParDefaut;
    else return 0;
}

void UpComboBox::setValeurAvant(QString valprec)
{
    ValeurAvant = valprec;
}

QString UpComboBox::getValeurAvant() const
{
    return ValeurAvant;
}

void UpComboBox::setValeurApres(QString valpost)
{
    ValeurAvant = valpost;
}

QString UpComboBox::getValeurApres() const
{
    return ValeurApres;
}

void UpComboBox::setChampCorrespondant(QString champcorrespondant)
{
    Champ = champcorrespondant;
}

QString UpComboBox::getChampCorrespondant() const
{
    return Champ;
}

void UpComboBox::setTableCorrespondant(QString tablecorrespondant)
{
    Table = tablecorrespondant;
}

QString UpComboBox::getTableCorrespondant() const
{
    return Table;
}

void UpComboBox::setImmediateToolTip(QString Msg)
{
    gToolTipMsg = Msg;
}

