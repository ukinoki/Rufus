/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
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
    enum        StyleBouton {RECEPTIONBUTTON, BACKBUTTON, CANCELBUTTON, CLOSEBUTTON, COPYBUTTON, DONOTRECORDBUTTON, EDITBUTTON, NOBUTTON, PRINTBUTTON ,RECORDBUTTON, STARTBUTTON, SUPPRBUTTON, OUPSBUTTON};
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
    QIcon       giconBack,
                giconCancel,    giconCancelpushed,
                giconClose,     giconClosepushed,
                giconCopy,
                giconDoNotRecord,
                giconEdit,
                giconOK,        giconOKpushed,
                giconOups,
                giconPrint,
                giconReception, giconRecord,
                giconSuppr;
    int         id;
    QVariant    gLuggage;
    void        AfficheToolTip();
    QString     gToolTipMsg;

signals:
    void        clicked(int a);
    void        clicked(QVariant var);
};

#endif // UPSMALLBUTTON_H
