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

#ifndef DLG_PARAMCONNEXION_H
#define DLG_PARAMCONNEXION_H

#include <QMessageBox>
#include <QNetworkInterface>
#include <QSqlDatabase>
#include <QtSql>
#include <QTimer>
#include "upmessagebox.h"

/* Cette classe sert à paramétrer la connexion au serveur
 * DIFFERENTE POUR RUFUS ET RUFUSADMIN
*/

namespace Ui {
class dlg_paramconnexion;
}

class dlg_paramconnexion : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_paramconnexion(bool OKAccesDistant, QWidget *parent = Q_NULLPTR);
    ~dlg_paramconnexion();
    Ui::dlg_paramconnexion *ui;
    QSqlDatabase    getdatabase();

private:
    bool            VerifFiche();
    QTimer          *gTimer;
    bool            gVisible;
    QSqlDatabase    db;
    bool            TestConnexion();
    QString         gIPAvecZero, gIPSansZero, gClient, gServeur;

private slots:
    void    Slot_CalcIP(QString IP);
    void    Slot_Clign();
    void    Slot_HelpMsg();
    void    Slot_MAJIP();
    void    Slot_RegleAffichage();
    void    Slot_Test();
    void    Slot_Verif();
};

#endif // DLG_PARAMCONNEXION_H
