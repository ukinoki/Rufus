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

#ifndef DLG_MOTIFS_H
#define DLG_MOTIFS_H

#include <QColorDialog>
#include <QToolTip>
#include <QVBoxLayout>
#include <QSqlQuery>

#include "upcheckbox.h"
#include "uplineedit.h"
#include "uplabel.h"
#include "uptablewidget.h"
#include "widgetbuttonframe.h"
#include "gbl_datas.h"
#include "database.h"
#include "updialog.h"



namespace Ui {
class dlg_motifs;
}

class dlg_motifs : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_motifs(QWidget *parent = Q_NULLPTR);
    ~dlg_motifs();

private:
    Ui::dlg_motifs      *ui;
    WidgetButtonFrame   *wdg_buttonframe;
    QMap<int,Motif*>    *map_motifs;
    void                ActualiseDetails();
    void                ChoixButtonFrame();
    void                CreeMotif();
    void                DeplaceVersRow(int id, int anc, int nouv);
    void                DropMotif(QByteArray);
    void                EnregistreMotifs();
    Motif*              getMotifFromRow(int row) const;
    void                ModifCouleur();
    void                ModifPD();
    void                ModifMotif();
    void                ModifRaccouci();
    void                ModifUtil();
    void                ParDefaut(UpCheckBox *check);
    void                RecalculeLesRows();
    void                RemplirTableWidget();
    void                SetMotifToRow(Motif *mtf, int row);
    void                SupprimMotif();
    UpCheckBox*         UpchkFromTableW(QTableWidget*, int row, int col) const;
    void                Utiliser(UpCheckBox *check, bool a);
};

#endif // DLG_MOTIFS_H
