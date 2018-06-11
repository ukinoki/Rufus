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

#ifndef DLG_IDENTIFICATIONUSER_H
#define DLG_IDENTIFICATIONUSER_H

#include <QFileDialog>
#include <QHostInfo>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QtSql>

#include "upmessagebox.h"
#include "utils.h"

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
    //QMap<QString,QString>   getParamConnexion();
    //int                     getidUser();


private slots:
    void                    Slot_RetourOK();
    void                    Slot_RetourAnnul();

private:
    int                     ControleDonnees();
    //void                    Pause(int msec);
    int                     gidUser, gPort;
    //QMap<QString,QString>   gmap;
    QTimer                  *gTimerControl;
    bool                    gChgUsr;
    bool                    eventFilter(QObject *, QEvent *);
};

#endif // DLG_IDENTIFICATIONUSER_H
