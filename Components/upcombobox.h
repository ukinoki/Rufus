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

#ifndef UPCOMBOBOX_H
#define UPCOMBOBOX_H

#include <QComboBox>
#include <QLineEdit>
#include <QSound>
#include "upmessagebox.h"

class UpComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit UpComboBox(QWidget *parent = Q_NULLPTR);
    ~UpComboBox();
    void        setiD(int valprec);
    int         iD() const;
    void        setIndexParDefaut(int defaut);
    int         IndexParDefaut() const;
    void        setvaleuravant(QString valprec);
    QString     valeuravant() const;
    void        setvaleurapres(QString valpost);
    QString     valeurapres() const;
    void        setchamp(QString champcorrespondant);
    QString     champ() const;
    void        setTable(QString tablecorrespondant);
    QString     table() const;
    void        setImmediateToolTip(QString Msg);
    void        clearImmediateToolTip();

private:
    QLineEdit   *line;
    bool        eventFilter(QObject *, QEvent *);
    QString     m_valeuravant, m_valeurapres, m_champ, m_table, m_tooltipmsg;
    int         m_id, m_indexpardefaut;

signals:
    void        mouseDoubleClick(int row);

protected:
    void        mouseDoubleClickEvent(QMouseEvent *e);

};

#endif // UPCOMBOBOX_H
