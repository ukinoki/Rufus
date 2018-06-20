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

#ifndef DLG_FONTDIALOG_H
#define DLG_FONTDIALOG_H

#include <QFontDatabase>
#include "upmessagebox.h"
#include <QTreeWidget>

class dlg_fontdialog : public UpDialog
{
    Q_OBJECT

public:
    explicit            dlg_fontdialog(QString nomSettings, QString Position, QWidget *parent = Q_NULLPTR);
                        ~dlg_fontdialog();
    void                setFont(QFont);
    QFont               getFont() const;
    void                setFontAttribut(QString);
    QString             getFontAttribut() const;

private slots:
    void                Slot_Redessinelabel(QTreeWidgetItem*);
    void                Slot_FermeFiche();

private:
    QFont               gFontDialog;
    QString             gFontAttribut;
    QTreeWidget         *TreeWidget;
    UpLabel             *lbl;
    QFrame              *frame;
    UpPushButton        *button;
};

#endif // DLG_FONTDIALOG_H
