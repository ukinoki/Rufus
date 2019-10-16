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

#ifndef DLG_LISTEMOTSCLES_H
#define DLG_LISTEMOTSCLES_H

#include "procedures.h"

class dlg_listemotscles : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_listemotscles(QWidget *parent = Q_NULLPTR);
    ~dlg_listemotscles();
    QStringList         listMCDepart() const;
    enum Mode {Creation, Modif};    Q_ENUM(Mode)

private:
    DataBase            *db = DataBase::I();
    Patient             *m_currentpatient = Datas::I()->patients->currentpatient();
    QStringList         m_listemotscles, m_listidmotsclesdepart;

    QTableView          *wdg_bigtable;
    WidgetButtonFrame   *wdg_buttonframe;
    QHBoxLayout         *wdg_editlayout;
    UpDialog            *dlg_ask;
    QStandardItemModel  *m_model;
    QItemSelectionModel *m_selectionmodel;

    void                ChoixButtonFrame();
    void                DisableLines();
    void                Enablebuttons();
    void                RemplirTableView();
    void                CreationModifMC(enum Mode);
    void                SupprMC();
    void                Validation();
    void                VerifMC();

};

#endif // DLG_LISTEMOTSCLES_H
