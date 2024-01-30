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

#ifndef UPMESSAGEBOX_H
#define UPMESSAGEBOX_H

#include "updialog.h"
#include "uplabel.h"
#include "uppushbutton.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDesktopServices>
#include <QTextEdit>
#include "macros.h"
#include "utils.h"

class UpMessageBox : public UpDialog
{
    Q_OBJECT
public:
    explicit        UpMessageBox(QWidget *parent = Q_NULLPTR);
    ~UpMessageBox();
    static void                         Show        (QWidget*, QString Text = "", QString InfoText = "");
    static UpSmallButton::StyleBouton   Watch(QWidget*, QString Text = "", QString InfoText = "", Buttons Butts = UpDialog::ButtonOK, QString link = "");
    static UpSmallButton::StyleBouton   Question    (QWidget*, QString Text = "", QString InfoText = "", Buttons Butts = UpDialog::ButtonCancel | UpDialog::ButtonOK, QStringList titresboutonslist = QStringList());
    static void                         Information (QWidget*, QString Text = "", QString InfoText = "");
    enum                                Icon   {Quest, Info, Warning, Critical, Print}; Q_ENUM(Icon)
    enum                                Movie   {QuestionGif, InfoGif, WarningGif}; Q_ENUM(Movie)
    void                                addButton(UpSmallButton *button, enum UpSmallButton::StyleBouton);
    void                                addButton(UpPushButton *button);
    void                                removeButton(UpSmallButton *);
    UpSmallButton*                      clickedButton() const;
    UpPushButton*                       clickedpushbutton() const;
    void                                setIcon(enum Icon icn, bool animatedIcon = true);
    void                                setIconPixmap(QPixmap);
    void                                setText(QString);
    void                                setInformativeText(QString);
    void                                setDefaultButton(QPushButton*);

private:
    UpLabel         *wdg_iconlbl, *wdg_texteditlbl, *wdg_infolbl;
    QHBoxLayout     *wdg_infolayout;
    QVBoxLayout     *wdg_textlayout;
    QMovie          *m_movie;
    UpSmallButton   *wdg_ReponsSmallButton;
    UpPushButton    *wdg_ReponsPushButton;
    void            Repons(QPushButton *butt);
    void            setAnimatedIcon(Movie movie);
};

#endif // UPMESSAGEBOX_H
