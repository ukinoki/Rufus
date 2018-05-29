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

#ifndef DLG_DEPENSES_H
#define DLG_DEPENSES_H

#include <QLocale>
#include <QMenu>

#include "uptablewidget.h"
#include "dlg_comptes.h"

namespace Ui {
class dlg_depenses;
}

class dlg_depenses : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_depenses(Procedures *procAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_depenses();
    Ui::dlg_depenses            *ui;
    bool                        getInitOK();

private:
    dlg_comptes                 *Dlg_Cmpt;
    int                         r,c, idDepEnCours;
    int                         gidUser;
    int                         gidUserADebiter;
    Procedures                  *proc;
    QSqlDatabase                db;
    QString                     gNomUser;               //le nom de l'utilisateur dont on affiche les dépenses
    QStringList                 glistMoyensDePaiement;
    QStringList                 glistRefFiscales;
    QMap<QString,QVariant>      gDataUser;
    QStandardItemModel          *glistComptes;
    QStandardItemModel          *glistComptesAvecDesactive;
    QStandardItemModel          *gListeLiberauxModel;
    UpTableWidget               *gBigTable;
    UpPushButton                *EnregupPushButton, *AnnulupPushButton;
    bool                        InitOK;
    int                         gMode;
    enum gMode                  {Lire, Modifier, Enregistrer, TableVide};

    void                        closeEvent(QCloseEvent *event);
    void                        keyPressEvent ( QKeyEvent * event );
    void                        DefinitArchitectureBigTable();
    void                        ReconstruitListeAnnees();
    void                        ReconstruitListeRubriques();
    void                        ReconstruitListeToutesRubriques();
    void                        RegleAffichageFiche(enum gMode);
    void                        RegleComptesComboBox(bool ActiveSeult = true);
    void                        RemplitBigTable();
    void                        ChoixMenu(QString);

    //anciens slots
    void                        AnnulEnreg();

private slots:
    void                        Slot_ChangeUser(int idx);
    void                        Slot_CalculTotalDepenses();
    void                        Slot_ChercheFamilleFiscale(QString RefFiscale);
    void                        Slot_ChoixPaiement(QString choix);
    void                        Slot_ConvertitDoubleMontant();
    void                        Slot_EnableModifiepushButton();
    void                        Slot_EnregistreDepense();
    void                        Slot_GererDepense();
    void                        Slot_GestionComptes();
    void                        Slot_MenuContextuel();
    void                        Slot_MetAJourFiche();
    void                        Slot_ModifierDepense();
    void                        Slot_RedessineBigTable();
    void                        Slot_SupprimerDepense();
};

#endif // DLG_DEPENSES_H
