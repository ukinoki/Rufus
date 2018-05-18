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

#ifndef DLG_CHOIXDATE_H
#define DLG_CHOIXDATE_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class dlg_choixdate;
}

class dlg_choixdate : public QDialog
{
    Q_OBJECT

public:
    explicit            dlg_choixdate(QWidget *parent = Q_NULLPTR);
    ~dlg_choixdate();
    Ui::dlg_choixdate   *ui;

private slots:
    void                Slot_AfficheDates();
    void                Slot_ModifDate();

private:
    bool                eventFilter(QObject *obj, QEvent *event)  ;


};

#endif // DLG_CHOIXDATE_H
