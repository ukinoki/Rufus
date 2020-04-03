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

#ifndef DLG_CHOIXDATE_H
#define DLG_CHOIXDATE_H

#include <QDialog>
#include <QMap>
#include "utils.h"

namespace Ui {
class dlg_choixdate;
}

class dlg_choixdate : public QDialog
{
    Q_OBJECT
public:
    explicit                    dlg_choixdate(QWidget *parent = Q_NULLPTR);
    ~dlg_choixdate();
    Ui::dlg_choixdate           *ui;
    QMap<Utils::Period, QDate>  mapdate() const { return map_date; }

private:
    QMap<Utils::Period, QDate>  map_date;
    bool                        eventFilter(QObject *obj, QEvent *event)  ;
    void                        AfficheDates(QWidget *widg);
    void                        ModifDate(QWidget *widg);
    void                        Fermefiche();
};

#endif // DLG_CHOIXDATE_H
