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

#ifndef DLG_LISTESITES_H
#define DLG_LISTESITES_H

#include <QColorDialog>
#include <QHeaderView>
#include <QTableView>
#include "database.h"
#include "gbl_datas.h"
#include "upstandarditem.h"
#include "uptableview.h"
#include "widgetbuttonframe.h"
#include "upsystemtrayicon.h"
#include "dlg_identificationsite.h"

class UpSystemTrayIcon;
/* sert à gérer les lieux d'exercice
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN*/

class dlg_listesites : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_listesites(QWidget *parent = nullptr);
    ~dlg_listesites();
    enum Mode               {Modif, Nouv, Suppr}; Q_ENUM(Mode)

private:
    DataBase                *db;
    UpTableView             *wdg_tblview;
    UpLabel                 *wdg_adressuplbl;
    UpPushButton            *wdg_couleurpushbutt;

    QStandardItemModel      *m_model = nullptr;
    WidgetButtonFrame       *wdg_buttonframe;
    int                     m_idlieuserveur;
    bool                    m_ok;
    void                    AfficheDetails(QModelIndex, QModelIndex);
    void                    ChoixButtonFrame();
    void                    CreerLieu();
    int                     getRowFromSite(Site* sit);
    Site*                   getSiteFromIndex(QModelIndex idx);
    void                    ModifCouleur();
    void                    ModifLieu();
    void                    SupprLieu();
    void                    ReconstruitModel();
};

#endif // DLG_LISTESITES_H
