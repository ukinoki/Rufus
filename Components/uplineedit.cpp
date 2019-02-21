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
    CanDepart       = true;
    PeutEtreVide    = true;
    RowTable        = -1;
    ColumnTable     = -1;
    id              = -1;
    ValeurAvant     = "";
    ValeurApres     = "";
    Champ           = "";
    Table           = "";
    linedata        = QVariant();
    installEventFilter(this);
    connect(this, &QLineEdit::textEdited,       this, &UpLineEdit::ReemitTextEdited);
    // connect(this, &QLineEdit::inputRejected,    this, [=] {QSound::play(NOM_ALARME);}); le signal inpuRejected n'est pas reconnu par osx...
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
        setValeurAvant(text());
    if (event->type() == QEvent::FocusOut)
    {
        if (text() != ValeurAvant)
            emit TextModified(text());
        const QDoubleValidator *val= dynamic_cast<const QDoubleValidator*>(this->validator());
        if (val)
            if (text() != ValeurAvant)
                setText(QLocale().toString(QLocale().toDouble(text()),'f',2));
    }
    return QWidget::eventFilter(obj, event);
}
void UpLineEdit::enterEvent(QEvent *)
{
    if (RowTable > -1)
        emit mouseEnter(RowTable);
}

void UpLineEdit::mouseReleaseEvent(QMouseEvent *)
{
    if (RowTable > -1)
        emit mouseRelease(RowTable);
}

void UpLineEdit::mouseDoubleClickEvent(QMouseEvent *)
{
    if (RowTable > -1)
        emit mouseDoubleClick(RowTable);
}

void UpLineEdit::AfficheToolTip()
{
    if (gToolTipMsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),gToolTipMsg);
}

void UpLineEdit::setImmediateToolTip(QString Msg)
{
    gToolTipMsg = Msg;
}

void UpLineEdit::setCanDepart(bool OK)
{
    CanDepart = OK;
}

bool UpLineEdit::getCanDepart() const
{
    return CanDepart;
}

void UpLineEdit::setData(QVariant data)
{
    linedata = data;
}

QVariant UpLineEdit::getData()
{
    return linedata;
}

void UpLineEdit::setPeutEtreVide(bool OK)
{
    PeutEtreVide = OK;
}

bool UpLineEdit::getPeutEtreVide()
{
    return PeutEtreVide;
}

void UpLineEdit::setId(int Id)
{
    id = Id;
}
int UpLineEdit::getId()
{
    return id;
}

void UpLineEdit::setRowTable(int val)
{
    RowTable = val;
}
int UpLineEdit::getRowTable() const
{
    return RowTable;
}

void UpLineEdit::setColumnTable(int val)
{
    ColumnTable = val;
}
int UpLineEdit::getColumnTable() const
{
    return ColumnTable;
}
void UpLineEdit::setValeurAvant(QString valprec)
{
    ValeurAvant = valprec;
}

QString UpLineEdit::getValeurAvant() const
{
    return ValeurAvant;
}

void UpLineEdit::setValeurApres(QString valpost)
{
    ValeurApres = valpost;
}

QString UpLineEdit::getValeurApres() const
{
    return ValeurApres;
}

void UpLineEdit::setChampCorrespondant(QString champcorrespondant)
{
    Champ = champcorrespondant;
}

QString UpLineEdit::getChampCorrespondant() const
{
    return Champ;
}

void UpLineEdit::setTableCorrespondant(QString tablecorrespondant)
{
    Table = tablecorrespondant;
}

QString UpLineEdit::getTableCorrespondant() const
{
    return Table;
}

void UpLineEdit::ReemitTextEdited()
{
    emit upTextEdited(text(), RowTable, ColumnTable);
}
