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
#include "uptablewidget.h"
#include "upgroupbox.h"
#include "upradiobutton.h"

class dlg_bilanrecettes : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_bilanrecettes(QWidget *parent = Q_NULLPTR);
    ~dlg_bilanrecettes();
    bool                        getInitOK();

private:
    Procedures                  *proc;
    QDate                       Debut, Fin;
    DataBase                    *db;
    QList<QVariantList>         gBilan;
    QCheckBox                   *gSupervcheckBox;
    QGroupBox                   *ClassmtupGrpBox;
    QHBoxLayout                 *glblbox, *hboxsup;
    UpComboBox                  *gSupervBox;
    UpLabel                     *gLabel;
    UpTableWidget               *gBigTable;
    UpLabel                     *TotalMontantlbl, *TotalReclbl;
    UpLabel                     *TotalApportlbl, *GdTotalReclbl;
    UpPushButton                *ChxPeriodButt;
    UpPushButton                *ExportButt;
    UpRadioButton               *ComptableRadio;
    UpRadioButton               *SupervRadio;
    User                        *gidUser;
    void                        CalcBilan();
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
    int                         gMode;
    enum gMode                  {SUPERVISEUR, COMPTABLE};
    void                        CalcSuperviseursEtComptables(); //Etablit la liste des superviseurs et des comptables pour la période du bilan
    void                        CalculeTotal();
    void                        ChangeMode(enum gMode);
    void                        DefinitArchitetureTable();
    void                        ExportTable();
    void                        FiltreTable(int idx);
    void                        ImprimeEtat();
    void                        NouvPeriode();
    void                        RemplitLaTable();
    bool                        InitOK;
};

#endif // DLG_BILANRECETTES_H
