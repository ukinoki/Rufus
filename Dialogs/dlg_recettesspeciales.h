/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_RECETTESSPECIALES_H
#define DLG_RECETTESSPECIALES_H

#include <QLocale>
#include <QMenu>

#include "uptablewidget.h"
#include "dlg_comptes.h"
#include "cls_user.h"

namespace Ui {
class dlg_recettesspeciales;
}

class dlg_recettesspeciales : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_recettesspeciales(QWidget *parent = Q_NULLPTR);
    Ui::dlg_recettesspeciales   *ui;
    bool                        getInitOK();

private:
    dlg_comptes                 *Dlg_Cmpt;
    int                         r,c, idRecEnCours;
    Procedures                  *proc;
    DataBase                    *db;
    QStringList                 glistMoyensDePaiement;
    QStringList                 glistRefFiscales;
    User                        *gDataUser;
    UpTableWidget               *gBigTable;
    UpPushButton                *EnregupPushButton, *AnnulupPushButton;

    bool                        InitOK;
    int                         gMode;
    enum gMode                  {Lire, Modifier, Enregistrer, TableVide};

    void                        closeEvent(QCloseEvent *event);
    void                        keyPressEvent ( QKeyEvent * event );
    void                        CalculTotalRecettes();
    void                        DefinitArchitectureBigTable();
    void                        EnregistreRecette();
    void                        ReconstruitListeAnnees();
    void                        RegleComptesComboBox(bool ActiveSeult = true);
    void                        RegleAffichageFiche(enum gMode);
    void                        RemplitBigTable();
    void                        ChoixMenu(QString);
    bool                        initializeUserSelected();

    void                        AnnulEnreg();
    void                        ChoixPaiement();
    void                        ConvertitDoubleMontant();
    void                        EnableModifiepushButton();
    void                        GererRecette(QPushButton *widgsender);
    void                        GestionComptes();
    void                        MenuContextuel(UpLabel *labelClicked);
    void                        MetAJourFiche();
    void                        ModifierRecette();
    void                        RedessineBigTable(int idRec = -1);
    void                        SupprimerRecette();
    bool                        VerifSaisie();
};

#endif // DLG_RECETTESSPECIALES_H
