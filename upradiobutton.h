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

#ifndef UPRADIOBUTTON_H
#define UPRADIOBUTTON_H

#include <QRadioButton>
#include <QMouseEvent>
#include <QToolTip>

class UpRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit    UpRadioButton(QWidget *parent = Q_NULLPTR);
    void        setImmediateToolTip(QString);
    void        setToggleable(bool val);
            // cette propriété permet de bloquer la modification d'état du RadioButton
            // la propriété checkable n'est pas utilisable car elle ne permet pas de bloquer le radiobutton en position checked
    bool        Toggleable() const;
    void        setiD(int val);
    int         iD() const;
private:
    int         gid;
    QString     gToolTipMsg;
    bool        eventFilter(QObject *obj, QEvent *event)  ;
    void        AfficheToolTip();
    bool        Toggable;
signals:
    void        enter();

};

#endif // UPRADIOBUTTON_H
