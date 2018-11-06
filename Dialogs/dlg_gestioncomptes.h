/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_GESTIONCOMPTES_H
#define DLG_GESTIONCOMPTES_H

#include "dlg_banque.h"
#include "ui_dlg_banque.h"
#include "upcombobox.h"
#include <QRadioButton>
#include "cls_user.h"
#include "functormajpremierelettre.h"
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
    explicit dlg_gestioncomptes(User *DataUser,
                                bool societe, bool AfficheLeSolde = true, QWidget *parent = Q_NULLPTR);
    ~dlg_gestioncomptes();

private:
    Ui::dlg_gestioncomptes  *ui;
    DataBase                *db;
    User                    *gDataUser;
    Comptes                 *comptesusr;
    Compte                  *CompteEnCours;
    dlg_banque              *Dlg_Banq;
    bool                    createurducompte;
    bool                    gAfficheLeSolde;
    bool                    gSociete;
    bool                    gVisible;
    int                     gidUser, gMode;
    int                     gidCompteParDefaut;
    enum gMode              {Norm, Modif, Nouv};
    QString                 gUserLogin;
    QTimer                  *gTimer;
    void                    closeEvent(QCloseEvent *);
    QString                 CorrigeApostrophe(QString RechAp);
    void                    MetAJourListeBanques();
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
