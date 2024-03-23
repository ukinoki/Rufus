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
    enum        StyleBouton {BACKBUTTON, CALENDARBUTTON, CANCELBUTTON, CLOSEBUTTON, COPYBUTTON,
                             DONOTRECORDBUTTON, EDITBUTTON, HOMEBUTTON, LOUPEBUTTON, NOBUTTON, OUPSBUTTON, QWANTBUTTON, PRINTBUTTON ,RECEPTIONBUTTON, RECORDBUTTON, STARTBUTTON, SUPPRBUTTON,PDFBUTTON};
                Q_ENUM(StyleBouton)
    void        setUpButtonStyle(enum StyleBouton);
    StyleBouton ButtonStyle() const;
    void        setiD(int);
    int         iD() const;
    void        setText(QString);
    void        setdata(QVariant var);
    QVariant    data() const;
    void        setImmediateToolTip(QString Msg, bool  affichettipmemesidisabled = false);

private:
    bool        eventFilter(QObject *obj, QEvent *event)  ;
    int         m_id;
    StyleBouton m_style;
    QVariant    m_luggage;
    void        AfficheToolTip();
    bool        AfficheToolTipMemeSiDisabled = false;
    QString     m_tooltipmsg;

signals:
    void        clicked(int a);
    void        clicked(QVariant var);
};

#endif // UPSMALLBUTTON_H
