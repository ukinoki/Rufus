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

#include "upmessagebox.h"
#include "database.h"

namespace Ui {
class dlg_identificationuser;
}

class dlg_identificationuser : public QDialog
{
    Q_OBJECT
public:
    explicit dlg_identificationuser(QWidget *parent = Q_NULLPTR);
    ~dlg_identificationuser();
    Ui::dlg_identificationuser *ui;
    enum LoginResult { NoConnexion, NoUser, CorruptedBase, CorruptedUser, OK}; Q_ENUM(LoginResult)
    LoginResult             loginresult() const;

private:
    DataBase                *db = DataBase::I();
    bool                    eventFilter(QObject *, QEvent *);
    LoginResult             ControleDonnees();
    LoginResult             m_loginresult = NoConnexion;
    void                    Validation();
};

#endif // DLG_IDENTIFICATIONUSER_H
