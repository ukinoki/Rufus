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

#ifndef DLG_FONTDIALOG_H
#define DLG_FONTDIALOG_H

#include <QFontDatabase>
#include "upmessagebox.h"
#include <QTreeWidget>

class dlg_fontdialog : public UpDialog
{
    Q_OBJECT

public:
    explicit            dlg_fontdialog(QString nomSettings, QString m_position, QWidget *parent = Q_NULLPTR);
                        ~dlg_fontdialog();
    void                setFont(QFont);
    QFont               font() const;
    void                setFontAttribut(QString);
    QString             fontAttribut() const;

private:
    QFont               m_font;
    QString             m_fontattribut;
    QTreeWidget         *wdg_treewidget;
    UpLabel             *wdg_lbl;
    QFrame              *wdg_frame;
    UpPushButton        *wdg_uppushbutton;
    void                FermeFiche();
    void                Redessinelabel(QTreeWidgetItem*);
};

#endif // DLG_FONTDIALOG_H
