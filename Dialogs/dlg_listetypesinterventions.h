/* (C) 2022 LAINE SERGE
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

#ifndef CLG_LISTETYPESINTERVENTIONS_H
#define CLG_LISTETYPESINTERVENTIONS_H

#include "procedures.h"
#include "updelegate.h"
#include "upstandarditemmodel.h"
#include "uptableview.h"

class dlg_listetypesinterventions : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_listetypesinterventions(TypeIntervention* typ = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    ~dlg_listetypesinterventions();
    QList<int>          listtypDepart() const;
    enum Mode           {Selection, Modification, Creation};     Q_ENUM(Mode)
    TypeIntervention*   currenttype() {return m_currenttype;}

private:
    DataBase            *db = DataBase::I();
    Mode                m_mode;
    QList<int>          m_listidtypesdepart;
    UpTableView         *wdg_tblview;
    WidgetButtonFrame   *wdg_buttonframe;
    QHBoxLayout         *wdg_editlayout;
    UpDialog            *dlg_ask;
    UpStandardItemModel *m_model = Q_NULLPTR;
    QHash<QString, QVariant>    m_listbinds;
    TypeIntervention    *m_currenttype = Q_NULLPTR;
    QString             m_textdelegate = "";
    QString             m_ccamdelegate = "";

    void                keyPressEvent   (QKeyEvent * event );
    void                Annulation();
    bool                ChercheDoublon(QString str, int row);
    void                ChoixButtonFrame();
    void                ChoixMenuContextuel(QString);
    void                ConfigMode(Mode mode, TypeIntervention *typ = Q_NULLPTR);
    void                DisableLines();
    void                EnableButtons(TypeIntervention *typ = Q_NULLPTR);
    void                EnableLines(int row = -1);
    bool                EnregistreType(TypeIntervention *typ);
    TypeIntervention*   getTypeFromIndex(QModelIndex idx);
    void                MenuContextuel();
    void                RemplirTableView();
    void                selectcurrenttype(TypeIntervention *typ, QAbstractItemView::ScrollHint hint = QAbstractItemView::EnsureVisible);
    void                settypeToRow(TypeIntervention *typ, int row, bool resizecolumn = true);
    void                Supprimetype(TypeIntervention *typ);
    void                Validation();
    void                Veriftyp();

signals:
    void closedelegate();

};

#endif // DLG_LISTETYPESINTERVENTIONS_H
