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
#include "upradiobutton.h"

/* Cette classe sert à paramétrer la connexion au serveur
 * DIFFERENTE POUR RUFUS ET RUFUSADMIN
*/

namespace Ui {
class dlg_paramconnexion;
}

class dlg_paramconnexion : public QDialog
{
public:
    explicit dlg_paramconnexion(bool OKAccesDistant, QWidget *parent = Q_NULLPTR);
    ~dlg_paramconnexion();
    Ui::dlg_paramconnexion *ui;

private:
    bool            m_visible =  true;
    QString         m_IPaveczero = "";
    QString         m_IPsanszero = "";
    QString         m_client = "";
    QString         m_serveur = "";
    QTimer          *t_timer;
    QSqlDatabase    db;
    void            CalcIP(QString IP);
    void            Clign();
    void            HelpMsg();
    void            MAJIP();
    void            RegleAffichage(QRadioButton *butt);
    void            Test();
    void            Verif();
    bool            VerifFiche();
    bool            TestConnexion();
};

#endif // DLG_PARAMCONNEXION_H
