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

#ifndef DLG_LISTEMOTSCLES_H
#define DLG_LISTEMOTSCLES_H

#include "procedures.h"
#include "updelegate.h"
#include "uptableview.h"

class dlg_listemotscles : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_listemotscles(QWidget *parent = Q_NULLPTR);
    ~dlg_listemotscles();
    QList<int>          listMCDepart() const;
    enum Mode           {Selection, Modification, Creation};     Q_ENUM(Mode)

private:
    DataBase            *db = DataBase::I();
    Patient             *m_currentpatient = Datas::I()->patients->currentpatient();
    Mode                m_mode;
    QList<int>          m_listidmotsclesdepart;
    UpTableView         *wdg_tblview;
    WidgetButtonFrame   *wdg_buttonframe;
    QHBoxLayout         *wdg_editlayout;
    UpDialog            *dlg_ask;
    QStandardItemModel  *m_model = Q_NULLPTR;
    QHash<QString, QVariant>    m_listbinds;
    MotCle              *m_currentmotcle = Q_NULLPTR;

    void                keyPressEvent   (QKeyEvent * event );
    void                Annulation();
    bool                ChercheDoublon(QString str, int row);
    void                ChoixButtonFrame();
    void                ChoixMenuContextuel(QString);
    void                ConfigMode(Mode mode, MotCle *mc = Q_NULLPTR);
    void                DisableLines();
    void                Enablebuttons(QModelIndex idx);
    void                EnableLines(int row = -1);
    void                EnregistreMotCle(MotCle *mc);
    MotCle*             getMotCleFromIndex(QModelIndex idx);
    int                 getRowFromMotCle(MotCle *mc);
    void                MenuContextuel();
    void                RemplirTableView();
    void                selectcurrentMotCle(MotCle *mc);
    void                setMotCleToRow(MotCle *mc, int row);
    void                SupprimeMotCle(MotCle *mc);
    void                Validation();
    void                VerifMC();

};

#endif // DLG_LISTEMOTSCLES_H
