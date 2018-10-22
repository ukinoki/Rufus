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

#ifndef UPMESSAGEBOX_H
#define UPMESSAGEBOX_H

#include "updialog.h"
#include "uplabel.h"
#include "uppushbutton.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QTextEdit>
#include "macros.h"

class UpMessageBox : public UpDialog
{
    Q_OBJECT
public:
    explicit        UpMessageBox(QWidget *parent = Q_NULLPTR);
    ~UpMessageBox();
    static int      Watch       (QWidget*, QString Text = "", QString InfoText = "", Buttons Butts = UpDialog::ButtonOK);
    static int      Question    (QWidget*, QString Text = "", QString InfoText = "", Buttons Butts = UpDialog::ButtonCancel | UpDialog::ButtonOK, QStringList textlist = QStringList());
    static void     Information (QWidget*, QString Text = "", QString InfoText = "");
    int             Icon;
    enum            Icon   {Quest, Info, Warning, Critical, Print};
    void            addButton(UpSmallButton *button, enum UpSmallButton::StyleBouton);
    void            addButton(UpPushButton *button);
    void            removeButton(UpSmallButton *);
    UpSmallButton*  clickedButton();
    UpPushButton*   clickedpushbutton();
    void            setIcon(enum Icon icn);
    void            setIconPixmap(QPixmap);
    void            setText(QString);
    void            setInformativeText(QString);
    void            setDefaultButton(QPushButton*);

private:
    QSize           CalcSize(QString);
    UpLabel         *lblIcon, *Textedt, *InfoTextedt;
    QHBoxLayout     *infolayout;
    QVBoxLayout     *textlayout, *globallayout;
    UpSmallButton   *ReponsSmallButton;
    UpPushButton    *ReponsPushButton;
    void            Repons(QPushButton *butt);
};

#endif // UPMESSAGEBOX_H
