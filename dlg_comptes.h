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

#ifndef DLG_COMPTES_H
#define DLG_COMPTES_H

#include <QLocale>

#include "procedures.h"
#include "upcheckbox.h"
#include "uplabel.h"

namespace Ui {
class dlg_comptes;
}

class dlg_comptes : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_comptes(Procedures *procAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_comptes();
    Ui::dlg_comptes *ui;
    bool            getInitOK();
    void            setInitOK(bool init);

private:
    void            closeEvent(QCloseEvent *event);
    bool            eventFilter(QObject *obj, QEvent *event)  ;
    double          SoldeSurReleve;
    int             idCompte;
    int             gidLigneASupprimer;
    Procedures      *proc;
    QDate           Debut, Fin;
    QTableWidget    *gBigTable;
    bool            InitOK;
    void            DefinitArchitetureTable();
    void            RemplitLaTable(int idCompteAVoir);

private slots:
    void            Slot_AnnulArchive();
    void            Slot_Archiver();
    void            Slot_AnnulConsolidations();
    void            Slot_CalculeTotal();
    void            Slot_ChangeCompte(int idCompteAVoir);
    void            Slot_ContextMenuTableWidget();
    void            Slot_RenvoieRangee(bool Coche = true);
    void            Slot_SupprimerEcriture(QString);
};

#endif // DLG_COMPTES_H
