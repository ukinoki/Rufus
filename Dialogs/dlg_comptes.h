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

#ifndef DLG_COMPTES_H
#define DLG_COMPTES_H

#include <QLocale>

#include "procedures.h"
#include "upcheckbox.h"
#include "uplabel.h"
#include "cls_compte.h"
#include "cls_archivebanque.h"
#include "gbl_datas.h"

namespace Ui {
class dlg_comptes;
}

class dlg_comptes : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_comptes(QWidget *parent = Q_NULLPTR);
    ~dlg_comptes();
    Ui::dlg_comptes *ui;
    bool            getInitOK();
    void            setInitOK(bool init);
    enum ModeArchives {TOUT, PARARCHIVE};       Q_ENUM(ModeArchives)

private:
    int             intervalledate;
    QDate           dateencours;
    void            closeEvent(QCloseEvent *event);
    bool            eventFilter(QObject *obj, QEvent *event)  ;
    double          SoldeSurReleve;
    int             idCompte;
    int             gidLigneASupprimer;
    Archives        *archivescptencours;
    Compte          *CompteEnCours;
    QList<Compte*>  *comptesusr;
    DataBase        *db;
    Procedures      *proc;
    QDate           Debut, Fin;
    QTableWidget    *gBigTable;
    bool            InitOK;
    void            CalculeTotal();
    void            DefinitArchitetureTable();
    void            InsertLigneSurLaTable(QVariantList ligne, int row);
    void            RemplitLaTable(int idCompteAVoir);

    UpDialog        *gArchives;
    UpTableWidget   *gTableArchives;
    QComboBox       *glistArchCombo;
    UpLabel         *glbltitre;
    UpSmallButton   *gloupButton, *gFlecheHtButton;
    ModeArchives    gModeArchives;
    void            RemplirTableArchives();
    void            RedessineFicheArchives();

    void            SupprimerEcriture(QString);
    // anciens slots
    void            AnnulArchive();
    void            Archiver();
    void            AnnulConsolidations();
    void            ChangeCompte(int idCompteAVoir);
    void            ContextMenuTableWidget(UpLabel *lbl);
    void            RenvoieRangee(bool Coche, UpCheckBox *Check);
    void            VoirArchives();
};

#endif // DLG_COMPTES_H
