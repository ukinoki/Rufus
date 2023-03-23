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

#ifndef DLG_LISTEVILLES_H
#define DLG_LISTEVILLES_H

#include "uptableview.h"
#include "updelegate.h"
#include "procedures.h"
#include "upstandarditemmodel.h"

class dlg_listevilles : public UpDialog
{
    Q_OBJECT
public:
    dlg_listevilles(QWidget *parent = Q_NULLPTR);
    ~dlg_listevilles();

private:
    DataBase                    *db = DataBase::I();

    UpTableView                 *wdg_tblview;

    WidgetButtonFrame           *wdg_buttonframe;
    UpStandardItemModel         *m_model = Q_NULLPTR;
    Ville                       *m_currentville = Q_NULLPTR;
    QHash<QString, QVariant>    m_listbinds;
    QMap<int, Ville*>           *m_mapvilles = Datas::I()->villes->villes();
    QSortFilterProxyModel       *m_listnomsproxymodel  = Q_NULLPTR;
    QSortFilterProxyModel       *m_listCPsproxymodel    = Q_NULLPTR;
    QCompleter                  *m_complListVilles      = Q_NULLPTR;
    UpDialog                    *dlg_ask                = Q_NULLPTR;
    UpLineEdit                  *cpline                 = Q_NULLPTR;
    UpLineEdit                  *nomline                = Q_NULLPTR;
    bool                        ChercheDoublon(QString cp, QString nom);
    void                        ChoixButtonFrame();
    void                        ChoixMenuContextuel(QString);
    Ville*                      getVilleFromIndex(QModelIndex idx);
    void                        MenuContextuel();
    void                        RemplirTableView();
    void                        selectcurrentVille(Ville *ville, QAbstractItemView::ScrollHint hint = QAbstractItemView::EnsureVisible);
    void                        setVilleToRow(Ville *ville, int row);
    void                        SupprimmVille(Ville *ville);
    void                        EnregistreNouvelleVille();
    void                        ModifieVille(Ville *ville);
    void                        dialogville(QString cp, QString nom);
};

#endif // DLG_LISTEVILLES_H
