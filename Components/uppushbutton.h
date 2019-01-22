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

#ifndef UPPUSHBUTTON_H
#define UPPUSHBUTTON_H

#include <QPushButton>
#include <QKeyEvent>
#include <QToolTip>

class UpPushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit    UpPushButton(QWidget *parent = Q_NULLPTR);
    explicit    UpPushButton(QString text, QWidget *parent = Q_NULLPTR);
    ~UpPushButton();

    int         StyleBouton;
    enum        StyleBouton {OKBUTTON,ANNULBUTTON,NORMALBUTTON,IMPRIMEBUTTON};
    int         TailleBouton;
    enum        TailleBouton {Large, Mid, Small};
    void        setUpButtonStyle(enum StyleBouton, enum TailleBouton);
    void        setId(int id);
    int         getId() const;
    void        setImmediateToolTip(QString Msg);

private:
    bool        eventFilter(QObject *obj, QEvent *event)  ;
    int         id;
    QString     gToolTipMsg;
};
#endif // UPPUSHBUTTON_H
