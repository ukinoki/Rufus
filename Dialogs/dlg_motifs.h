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
    void                DeplaceVersRow(int id, int anc, int nouv);
    void                RecalculeLesRows();
    void                RemplirTableWidget();
    UpCheckBox*         UpchkFromTableW(QTableWidget*, int row, int col);
    void                SupprimMotif();
    void                CreeMotif();
    WidgetButtonFrame   *widgButtons;
    QMap<int,Motif*>    *m_motifs;
    Motif*              getMotifFromRow(int row);
    void                SetMotifToRow(Motif *mtf, int row);

private slots:
    void                Slot_ActualiseDetails();
    void                Slot_ChoixButtonFrame(int);
    void                Slot_DropMotif(QByteArray);
    void                Slot_ModifMotif(QString);
    void                Slot_ModifRaccouci(QString);
    void                Slot_ModifCouleur();
    void                Slot_ModifPD();
    void                Slot_ModifUtil();
    void                Slot_ParDefaut();
    void                Slot_Utiliser(bool);
    void                Slot_EnregistreMotifs();
};

#endif // DLG_MOTIFS_H
