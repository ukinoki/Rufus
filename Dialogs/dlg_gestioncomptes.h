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

#ifndef DLG_GESTIONCOMPTES_H
#define DLG_GESTIONCOMPTES_H

#include "dlg_gestionbanques.h"
#include "ui_dlg_gestionbanques.h"
#include "upcombobox.h"
#include <QRadioButton>
#include "cls_user.h"
#include "database.h"
#include "cls_compte.h"

/* sert à gérer les comptes bancaires des utilisateurs
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN*/

namespace Ui {
class dlg_gestioncomptes;
}

class dlg_gestioncomptes : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_gestioncomptes(User *user,
                                bool societe, bool AfficheLeSolde = true, QWidget *parent = Q_NULLPTR);
    ~dlg_gestioncomptes();
    enum Mode              {Norm, Modif, Nouv};    Q_ENUM(Mode)


private:
    Ui::dlg_gestioncomptes  *ui;
    DataBase                *db;
    User                    *m_userencours;
    Compte                  *m_comptencours;
    dlg_gestionbanques      *Dlg_Banq;
    bool                    gAfficheLeSolde;
    bool                    gSociete;
    bool                    gVisible;
    int                     gidUser;
    Mode                    gMode;
    QString                 gUserLogin;
    QTimer                  *gTimer;
    void                    closeEvent(QCloseEvent *);
    void                    ReconstruitComboBanques();
    void                    RemplirTableView(int idcompte = -1);
    bool                    VerifCompte();
    WidgetButtonFrame       *widgButtons;
    UpSmallButton           *NouvBanqupPushButton;
    void                    ModifCompte();
    void                    NouvCompte();
    void                    SupprCompte();

    void                    AfficheCompte(QTableWidgetItem *, QTableWidgetItem *);
    void                    AnnulModif();
    void                    Banques();
    void                    ChoixButtonFrame(int);
    void                    Clign();
    void                    CompteFactice();
    void                    DesactiveCompte();
    void                    Fermer();
    void                    ValidCompte();
};

#endif // DLG_GSETIONCOMPTES_H
