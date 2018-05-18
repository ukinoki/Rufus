/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_MOTIFS_H
#define DLG_MOTIFS_H

#include <QColorDialog>
#include <QToolTip>
#include <QVBoxLayout>

#include "procedures.h"
#include "upcheckbox.h"
#include "uplineedit.h"
#include "uplabel.h"
#include "uptablewidget.h"
#include "widgetbuttonframe.h"



namespace Ui {
class dlg_motifs;
}

class dlg_motifs : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_motifs(Procedures *prc, QWidget *parent = Q_NULLPTR);
    ~dlg_motifs();

private:
    Ui::dlg_motifs      *ui;
    Procedures          *proc;
    void                DeplaceVersRow(int id, int anc, int nouv);
    void                RecalculeLesRows();
    void                RemplirTableWidget();
    UpCheckBox*         UpchkFromTableW(QTableWidget*, int row, int col);
    void                SupprimMotif();
    void                CreeMotif();
    WidgetButtonFrame   *widgButtons;

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
