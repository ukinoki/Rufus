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

#ifndef UPCOMBOBOX_H
#define UPCOMBOBOX_H

#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSound>
#include "upmessagebox.h"

class UpComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit UpComboBox(QWidget *parent = Q_NULLPTR);
    ~UpComboBox();
    void        setid(int valprec);
    int         getid() const;
    void        setIndexParDefaut(int defaut);
    int         getIndexParDefaut() const;
    void        setValeurAvant(QString valprec);
    QString     getValeurAvant() const;
    void        setValeurApres(QString valpost);
    QString     getValeurApres() const;
    void        setChampCorrespondant(QString champcorrespondant);
    QString     getChampCorrespondant() const;
    void        setTableCorrespondant(QString tablecorrespondant);
    QString     getTableCorrespondant() const;
    void        setImmediateToolTip(QString Msg);
    void        clearImmediateToolTip();

private:
    QLineEdit   *line;
    bool        eventFilter(QObject *, QEvent *);
    QString     ValeurAvant, ValeurApres, Champ, Table, gToolTipMsg;
    int         id, IndexParDefaut;

signals:
    void        mouseDoubleClick(int row);

protected:
    void        mouseDoubleClickEvent(QMouseEvent *e);

};

#endif // UPCOMBOBOX_H
