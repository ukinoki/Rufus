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

#ifndef DLG_BILANRECETTES_H
#define DLG_BILANRECETTES_H



#include <QLocale>
#include "procedures.h"
#include "uptableview.h"
#include "upgroupbox.h"
#include "upradiobutton.h"

class dlg_bilanrecettes : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_bilanrecettes(QWidget *parent = Q_NULLPTR);
    ~dlg_bilanrecettes();
    bool                        getInitOK();
    enum Mode                   {SUPERVISEUR, COMPTABLE};    Q_ENUM(Mode)

private:
    Procedures                  *proc;
    DataBase                    *db;

    QDate                       m_debut, m_fin;
    bool                        m_initok;
    double                      TotalMontant;
    double                      TotalRecu;
    double                      TotalRecEsp;
    double                      TotalRecBanq;
    double                      TotalApport;
    double                      TotalAutresRec;
    double                      TotalAutresRecEsp;
    double                      TotalAutresRecBanq;
    double                      GdTotalEsp;
    double                      GdTotalBanq;

    Mode                        m_mode;
    QStandardItemModel          *m_recettesmodel;
    QSortFilterProxyModel       *m_sortfilterrecettesmodel;

    QCheckBox                   *wdg_supervcheckbox;
    QGroupBox                   *wdg_classmtupgrpbox;
    QHBoxLayout                 *wdg_lblbox, *wdg_hboxsup;
    UpComboBox                  *wdg_supervcombobox;
    UpLabel                     *wdg_label;
    UpTableView                 *wdg_bigtable;
    UpLabel                     *wdg_totalmontantlbl, *wdg_totalrecetteslbl;
    UpLabel                     *wdg_totalapportslbl, *wdg_grandtotallbl;
    UpPushButton                *wdg_choixperiodebouton;
    UpPushButton                *wdg_exportbouton;
    UpRadioButton               *wdg_comptableradiobouton;
    UpRadioButton               *wdg_superviseurradiobouton;

    void                        CalcBilan();
    void                        CalcSuperviseursEtComptables(); //Etablit la liste des superviseurs et des comptables pour la période du bilan
    void                        CalculeTotal();
    void                        ChangeMode(enum Mode);
    void                        ExportTable();
    void                        FiltreTable(int idx);
    Recette*                    getRecetteFromRow(int row);
    Recette*                    getRecetteFromIndex(QModelIndex idx);
    Recette*                    getRecetteFromSelectionInTable();
    void                        ImprimeEtat();
    void                        NouvPeriode();
    void                        RemplitLaTable();
};

#endif // DLG_BILANRECETTES_H
