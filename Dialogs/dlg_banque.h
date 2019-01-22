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

#ifndef DLG_BANQUE_H
#define DLG_BANQUE_H

#include <QSqlDatabase>
#include <QtSql>

#include "updialog.h"
#include "uplabel.h"
#include "upmessagebox.h"
#include "uptablewidget.h"
#include "widgetbuttonframe.h"
#include "database.h"
#include "gbl_datas.h"
#include "utils.h"
#include "cls_banque.h"

/* sert à gérer les noms des organismes bancaires
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN*/

namespace Ui {
class dlg_banque;
}

class dlg_banque : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_banque(QWidget *parent = Q_NULLPTR, QString nouvbanqueabrege = "");
    ~dlg_banque();
    Ui::dlg_banque  *ui;
    UpTableWidget   *uptablebanq;

private:
    int                                     gMode;
    bool                                    gFermeApresValidation;
    enum gMode                              {Norm, Modif, Nouv, Suppr};
    QMap<int,Banque*>                       *m_banques;
    DataBase                                *db;
    WidgetButtonFrame                       *widgButtons;

    void            RemetEnNorm();
    void            RemplirTableView();
    void            NouvBanque();
    void            ModifBanque();
    void            SupprBanque();

    void            AnnuleModifBanque();
    void            AfficheBanque();
    void            MAJBanques();               // met à jour la liste des banques
    void            ValideModifBanque();
    void            ChoixButtonFrame(int);
};

#endif // DLG_BANQUE_H
