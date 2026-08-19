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

#ifndef DLG_LISTEIOLS_H
#define DLG_LISTEIOLS_H

#include "updialog.h"
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include "rangeslider.h"
#include <QTreeView>
#include "gbl_datas.h"
#include "updelegate.h"
#include "updoublespinbox.h"
#include "widgetbuttonframe.h"
#include "dlg_identificationiol.h"

class dlg_listeiols : public UpDialog
{
    Q_OBJECT
public:
    explicit                dlg_listeiols(bool onlyactifs = false, QWidget *parent = nullptr);
    ~dlg_listeiols();
    int                     idcurrentIOL() const;
    bool                    listeIOLsmodifiee() const;

private:
    bool                    m_listemodifiee = false;
    bool                    m_onlyactifs;
    QList<int>              m_listidiolsutilises;
    QStandardItemModel      *m_IOLsmodel            = nullptr;
    QStandardItemModel      *m_manufacturersmodel   = nullptr;
    UpComboBox              *wdg_manufacturerscombo = nullptr;
    UpCheckBox              *wdg_pwrchk             = nullptr;
    UpComboBox              *wdg_typebox            = nullptr;
    UpCheckBox              *wdg_prechargechk       = nullptr;
    UpCheckBox              *wdg_jaunechk           = nullptr;
    UpCheckBox              *wdg_clairchk           = nullptr;
    UpCheckBox              *wdg_singlepiecechk     = nullptr;
    UpCheckBox              *wdg_twopiecechk        = nullptr;
    UpCheckBox              *wdg_toricchk           = nullptr;
    UpCheckBox              *wdg_edofchk            = nullptr;
    UpCheckBox              *wdg_multifocalchk      = nullptr;
    UpCheckBox              *wdg_monofocalchk       = nullptr;
    UpLabel                 *wdg_minpwrlbl          = nullptr;
    UpLabel                 *wdg_maxpwrlbl          = nullptr;
    RangeSlider             *wdg_rangepwrslider     = nullptr;
    QWidget                 *wdg_pwrslider          = new QWidget();
    double                  m_minpwr                = 10.00;
    double                  m_maxpwr                = 30.00;
    UpPushButton            *wdg_annulfiltresbut    = nullptr;
    QTreeView               *wdg_itemstree          = nullptr;
    WidgetButtonFrame       *wdg_buttonframe        = nullptr;
    TreeViewDelegate        m_treedelegate;
    IOL                     *m_currentIOL           = nullptr;
    bool                    m_filterbypwr           = false;
    QLabel                  *m_IOLtotalcount        = new QLabel;

    bool                    eventFilter(QObject *obj, QEvent *event);
    void                    Annulerlesfiltres();
    void                    ChoixButtonFrame();
    void                    connectFiltersSignals();
    void                    disconnectFiltersSignals();
     void                   Enablebuttons(QModelIndex idx);
    void                    EnregistreNouveauIOL();
    IOL*                    getIOLFromIndex(QModelIndex idx);
    void                    ReconstruitListeManufacturers();                // la liste des fabricants
    void                    ModifIOL(IOL *iol);
    void                    scrollToIOL(IOL *iol);
    void                    SupprIOL(IOL *iol);
    void                    ReconstruitTreeViewIOLs(QString filtre = "");

};

#endif // DLG_LISTEIOLS_H
