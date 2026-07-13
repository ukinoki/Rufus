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
    explicit                dlg_listeiols(bool onlyactifs = false, QWidget *parent = Q_NULLPTR);
    ~dlg_listeiols();
    int                     idcurrentIOL() const;
    bool                    listeIOLsmodifiee() const;

private:
    bool                    m_listemodifiee = false;
    bool                    m_onlyactifs;
    QList<int>              m_listidiolsutilises;
    QStandardItemModel      *m_IOLsmodel            = Q_NULLPTR;
    QStandardItemModel      *m_manufacturersmodel   = Q_NULLPTR;
    UpComboBox              *wdg_manufacturerscombo = Q_NULLPTR;
    UpCheckBox              *wdg_pwrchk             = Q_NULLPTR;
    UpComboBox              *wdg_typebox            = Q_NULLPTR;
    UpCheckBox              *wdg_prechargechk       = Q_NULLPTR;
    UpCheckBox              *wdg_jaunechk           = Q_NULLPTR;
    UpCheckBox              *wdg_clairchk           = Q_NULLPTR;
    UpCheckBox              *wdg_singlepiecechk     = Q_NULLPTR;
    UpCheckBox              *wdg_twopiecechk        = Q_NULLPTR;
    UpCheckBox              *wdg_toricchk           = Q_NULLPTR;
    UpCheckBox              *wdg_edofchk            = Q_NULLPTR;
    UpCheckBox              *wdg_multifocalchk      = Q_NULLPTR;
    UpCheckBox              *wdg_monofocalchk       = Q_NULLPTR;
    UpLabel                 *wdg_minpwrlbl          = Q_NULLPTR;
    UpLabel                 *wdg_maxpwrlbl          = Q_NULLPTR;
    RangeSlider             *wdg_rangepwrslider     = Q_NULLPTR;
    QWidget                 *wdg_pwrslider          = new QWidget();
    double                  m_minpwr                = 10.00;
    double                  m_maxpwr                = 30.00;
    UpPushButton            *wdg_annulfiltresbut    = Q_NULLPTR;
    QTreeView               *wdg_itemstree          = Q_NULLPTR;
    WidgetButtonFrame       *wdg_buttonframe        = Q_NULLPTR;
    TreeViewDelegate        m_treedelegate;
    IOL                     *m_currentIOL           = Q_NULLPTR;
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
    void                    ImportListeIOLS(QDomDocument docxml);
    void                    ReconstruitListeManufacturers();                // la liste des fabricants
    void                    ModifIOL(IOL *iol);
    void                    scrollToIOL(IOL *iol);
    void                    SupprIOL(IOL *iol);
    void                    ReconstruitTreeViewIOLs(QString filtre = "");
    void                    HasNewVersion();

};

#endif // DLG_LISTEIOLS_H
