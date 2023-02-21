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

#ifndef DLG_RECETTESSPECIALES_H
#define DLG_RECETTESSPECIALES_H

#include "uptablewidget.h"
#include "dlg_comptes.h"

namespace Ui {
class dlg_recettesspeciales;
}

class dlg_recettesspeciales : public QDialog
{
    Q_OBJECT
public:
    explicit dlg_recettesspeciales(QWidget *parent = Q_NULLPTR);
    Ui::dlg_recettesspeciales   *ui;
    bool                        initOK() const;
    enum Mode                   {Lire, Modifier, Enregistrer, TableVide};    Q_ENUM(Mode)

private:
    Procedures                  *proc   = Procedures::I();
    DataBase                    *db     = DataBase::I();
    QDate                       m_currentdate = db->ServerDate();

    int                         m_idrecetteencours;
    QStringList                 m_listemoyensdepaiement;
    QStringList                 m_listerecettesfiscales;
    bool                        m_initok;
    Mode                        m_mode;
    User*                       currentuser() { return Datas::I()->users->userconnected(); }
    UpTableWidget               *wdg_bigtable;
    UpPushButton                *wdg_enreguppushbutton, *wdg_annuluppushbutton;

    void                        closeEvent(QCloseEvent *event);
    void                        keyPressEvent ( QKeyEvent * event );
    void                        CalculTotalRecettes();
    void                        DefinitArchitectureBigTable();
    void                        EnregistreRecette();
    void                        ReconstruitListeAnnees();
    void                        RegleComptesComboBox(bool ActiveSeult = true);
    void                        RegleAffichageFiche(enum Mode, bool majfiche = true);
    void                        RemplitBigTable();
    void                        ChoixMenu(int id = -1);
    bool                        initializeUserSelected();

    void                        AnnulEnreg();
    void                        ChoixPaiement();
    void                        ConvertitDoubleMontant();
    void                        EnableModifiepushButton();
    void                        GererRecette(QPushButton *widgsender);
    void                        GestionComptes();
    void                        MenuContextuel(int id);
    void                        MetAJourFiche();
    void                        ModifierRecette();
    void                        RedessineBigTable(int idRec = -1);
    void                        SupprimerRecette();
    bool                        VerifSaisie();
};

#endif // DLG_RECETTESSPECIALES_H
