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

#ifndef DLG_GESTIONBANQUES_H
#define DLG_GESTIONBANQUES_H

#include "updialog.h"
#include "uplabel.h"
#include "upmessagebox.h"
#include "uptablewidget.h"
#include "widgetbuttonframe.h"
#include "database.h"
#include "gbl_datas.h"
#include "utils.h"

/* sert à gérer les noms des organismes bancaires
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN*/

namespace Ui {
class dlg_gestionbanques;
}

class dlg_gestionbanques : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_gestionbanques(QWidget *parent = Q_NULLPTR, QString nouvbanqueabrege = "");
    ~dlg_gestionbanques();
    Ui::dlg_gestionbanques  *ui;
    enum Mode           {Norm, Modif, Nouv, Suppr};    Q_ENUM(Mode)


private:
    DataBase            *db = DataBase::I();
    UpTableWidget       *wdg_bigtable;
    WidgetButtonFrame   *wdg_buttonframe;
    QStandardItemModel  *m_model = Q_NULLPTR;
    bool                m_fermeapresvalidation;
    Mode                m_mode;

    void            RemetEnNorm();
    void            RemplirTableWidget();
    void            NouvBanque();
    void            ModifBanque();
    void            SupprBanque();

    void            AnnuleModifBanque();
    void            AfficheBanque();
    void            ValideModifBanque();
    void            ChoixButtonFrame();
};

#endif // DLG_GESTIONBANQUES_H
