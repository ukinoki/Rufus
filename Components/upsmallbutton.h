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

#ifndef UPSMALLBUTTON_H
#define UPSMALLBUTTON_H

#include <QPushButton>
#include <QKeyEvent>
#include <QToolTip>
#include <QDebug>

class UpSmallButton : public QPushButton
{
    Q_OBJECT

public:
    explicit    UpSmallButton(QString text, QWidget *parent = Q_NULLPTR);
    explicit    UpSmallButton(QWidget *parent = Q_NULLPTR);
    ~UpSmallButton();
    int         StyleBouton;
    enum        StyleBouton {BACKBUTTON, CALENDARBUTTON, CANCELBUTTON, CLOSEBUTTON, COPYBUTTON,
                             DONOTRECORDBUTTON, EDITBUTTON, HOMEBUTTON, LOUPEBUTTON, NOBUTTON, OUPSBUTTON, QWANTBUTTON, PRINTBUTTON ,RECEPTIONBUTTON, RECORDBUTTON, STARTBUTTON, SUPPRBUTTON};
    void        setUpButtonStyle(enum StyleBouton);
    int         ButtonStyle();
    void        setId(int);
    int         getId();
    void        setText(QString);
    void        setLuggage(QVariant var);
    QVariant    Luggage();
    void        setImmediateToolTip(QString Msg);

private:
    bool        eventFilter(QObject *obj, QEvent *event)  ;
    int         id;
    QVariant    gLuggage;
    void        AfficheToolTip();
    QString     gToolTipMsg;

signals:
    void        clicked(int a);
    void        clicked(QVariant var);
};

#endif // UPSMALLBUTTON_H
