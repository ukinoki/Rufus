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

#ifndef UPLABEL_H
#define UPLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QToolTip>

class UpLabel : public QLabel
{
    Q_OBJECT
public:
    explicit UpLabel(QWidget *parent = Q_NULLPTR, QString txt = QString());
    ~UpLabel();
    void        setId(int id);
    int         getId() const;
    void        setRow(int id);
    void        setImmediateToolTip(QString);
    int         getRow() const;
private:
    bool        eventFilter(QObject *obj, QEvent *event)  ;
    int         id;
    int         row;
    QString     gToolTipMsg;
    void        AfficheToolTip();
signals:
    void        clicked(int a);
    void        enter(int a);
    void        dblclick(int a);
};
#endif // UPLABEL_H
