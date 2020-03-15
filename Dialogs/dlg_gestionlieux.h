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

#ifndef DLG_GESTIONLIEUX_H
#define DLG_GESTIONLIEUX_H

#include <QColorDialog>
#include <QHeaderView>
#include <QTableView>
#include "database.h"
#include "gbl_datas.h"
#include "upstandarditem.h"
#include "widgetbuttonframe.h"

/* sert à gérer les lieux d'exercice
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN*/

class dlg_GestionLieux : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_GestionLieux(QWidget *parent = Q_NULLPTR);
    ~dlg_GestionLieux();
    enum Mode               {Modif, Nouv, Suppr}; Q_ENUM(Mode)

private:
    DataBase                *db;
    QTableView              *wdg_bigtable;
    UpDialog                *dlg_lieu;
    UpLabel                 *wdg_adressuplbl;
    UpPushButton            *wdg_couleurpushbutt;
    UpPushButton            *wdg_nouvcouleurpushbutt;
    UpLineEdit              *wdg_nomlineedit;
    UpLineEdit              *wdg_adress1lineedit;
    UpLineEdit              *wdg_adress2lineedit;
    UpLineEdit              *wdg_adress3lineedit;
    UpLineEdit              *wdg_CPlineedit;
    UpLineEdit              *wdg_villelineedit;
    UpLineEdit              *wdg_tellineedit;
    UpLineEdit              *wdg_faxlineedit;
    QString                 str_nouvcolor;

    QStandardItemModel      *m_tabmodel = Q_NULLPTR;
    WidgetButtonFrame       *wdg_buttonframe;
    int                     m_idlieuserveur;
    bool                    m_ok;
    void                    AfficheDetails(QModelIndex, QModelIndex);
    void                    ChoixButtonFrame();
    void                    CreerLieu();
    void                    enregNouvLieu();
    void                    enregModifLieu();
    Site*                   getSiteFromIndex(QModelIndex idx);
    void                    ModifCouleur();
    void                    ModifLieu();
    void                    SupprLieu();
    void                    ModifLieuxDialog(Mode mode);
    void                    ReconstruitModel();
    bool                    ValidationFiche();
};

#endif // DLG_GESTIONLIEUX_H
