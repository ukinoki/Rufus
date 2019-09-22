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

#ifndef DLG_DEPENSES_H
#define DLG_DEPENSES_H


#include "dlg_comptes.h"
#include "dlg_docsscanner.h"
#include "gbl_datas.h"
#include "ui_dlg_depenses.h"

namespace Ui {
class dlg_depenses;
}

class dlg_depenses : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_depenses(QWidget *parent = Q_NULLPTR);
    ~dlg_depenses();
    Ui::dlg_depenses            *ui;
    bool                        initOK() const;
    enum Mode                   {Lire, Modifier, Enregistrer, TableVide};       Q_ENUM(Mode)

private:
    DataBase                    *db             = DataBase::I();
    Procedures                  *proc           = Procedures::I();
    bool                        m_accesdistant  = (db->getMode()==Utils::Distant);
    User                        *m_userencours  = Q_NULLPTR;
    QMap<int, User*>            *map_usersliberaux = Datas::I()->users->liberaux();
    bool                        m_initok         = true;

    Depense                     *m_depenseencours;

    QList<QImage>               m_listeimages;
    QStringList                 m_listemoyensdepaiement;
    Mode                        m_mode;

    dlg_comptes                 *Dlg_Cmpt;
    dlg_docsscanner             *Dlg_DocsScan;
    UpDialog                    *dlg_ask;

    UpTableWidget               *wdg_bigtable;
    UpPushButton                *wdg_annuluppushbutton;
    UpPushButton                *wdg_enreguppushbutton;
    UpPushButton                *wdg_modifieruppushbutton;
    UpPushButton                *wdg_supprimeruppushbutton;
    QHBoxLayout                 *wdg_boxbuttlayout;

    void                        closeEvent(QCloseEvent *event);
    void                        keyPressEvent ( QKeyEvent * event );
    void                        AfficheFacture(Depense *dep = Q_NULLPTR);
    void                        CalculTotalDepenses();
    void                        ChoixMenu(QString);
    void                        DefinitArchitectureBigTable();
    void                        EnregistreDepense();
    void                        EnregistreFacture(QString typedoc);
    void                        ExportTable();
    void                        PrintTable();
    void                        FiltreTable();
    Depense*                    getDepenseFromRow(int row);
    bool                        initializeUserSelected();
    void                        ReconstruitListeAnnees();
    void                        ReconstruitListeRubriques(int idx = 0);
    void                        RegleAffichageFiche(Mode);
    void                        RegleComptesComboBox(bool ActiveSeult = true);
    void                        RemplitBigTable();
    void                        SetDepenseToRow(Depense *dep, int row);
    void                        EffaceFacture();
    void                        SupprimeFacture(Depense*dep);

    void                        AnnulEnreg();
    void                        ChangeUser(int idx);
    void                        ChoixPaiement();
    void                        ConvertitDoubleMontant();
    void                        EnableModifiepushButton();
    void                        GererDepense(QPushButton *widgsender);
    void                        GestionComptes();
    void                        MenuContextuel();
    void                        MetAJourFiche();
    void                        ModifierDepense();
    void                        RedessineBigTable();
    void                        SupprimerDepense();
    void                        ZoomDoc();
};

#endif // DLG_DEPENSES_H
