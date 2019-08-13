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

#ifndef DLG_IDENTIFICATIONUSER_H
#define DLG_IDENTIFICATIONUSER_H

#include <QFileDialog>
#include <QHostInfo>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QtSql>

#include "upmessagebox.h"
#include "utils.h"
#include "database.h"

namespace Ui {
class dlg_identificationuser;
}

class dlg_identificationuser : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_identificationuser(bool ChgUser = false, QWidget *parent = Q_NULLPTR);
    ~dlg_identificationuser();
    Ui::dlg_identificationuser *ui;


private slots:
    void                    Slot_RetourOK();
    void                    Slot_RetourAnnul();

private:
    DataBase                *db;
    int                     m_iduser, m_sqlport;
    bool                    m_chgmtuser;
    bool                    eventFilter(QObject *, QEvent *);
    int                     ControleDonnees();
};

#endif // DLG_IDENTIFICATIONUSER_H
