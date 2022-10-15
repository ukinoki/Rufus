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

#ifndef DLG_COMPTES_H
#define DLG_COMPTES_H

#include "procedures.h"
#include "updoublevalidator.h"

namespace Ui {
class dlg_comptes;
}

class dlg_comptes : public QDialog
{
    Q_OBJECT
public:
    explicit            dlg_comptes(QWidget *parent = Q_NULLPTR);
    ~dlg_comptes();
    Ui::dlg_comptes     *ui;
    bool                initOK() const;
    enum ModeArchives   {TOUT, PARARCHIVE};       Q_ENUM(ModeArchives)

private:
    Procedures      *proc = Procedures::I();
    DataBase        *db = DataBase::I();
    int             m_intervalledate = 180;
    QDate           m_dateencours = QDate::currentDate();

    double          m_soldesurreleve;
    int             m_idcompte;
    Archives        *m_archivescptencours;
    Compte          *m_compteencours;
    QList<int>      m_comptesusr;
    QDate           m_debut, m_fin;
    bool            m_initok;
    ModeArchives    m_modearchives;

    QTableWidget    *wdg_bigtable;
    UpDialog        *dlg_archives;
    UpTableWidget   *wdg_tablearchives;
    QComboBox       *wdg_listarchivescombo;
    UpLabel         *wdg_lbltitre;
    UpSmallButton   *wdg_loupbouton, *wdg_flechehtbouton;

    void            closeEvent(QCloseEvent *event);
    bool            eventFilter(QObject *obj, QEvent *event)  ;
    void            CalculeTotal();
    void            DefinitArchitetureTable();
    void            RemplitLaTable(int idcompte);
    void            RemplirTableArchives();
    void            RedessineFicheArchives();
    void            ModifMontant(LigneCompte *lign);
    void            SupprimerEcriture(LigneCompte *lign);
    void            AnnulArchive();
    void            Archiver();
    void            AnnulConsolidations();
    void            ChangeCompte(int idcompte);
    void            ContextMenuTableWidget(UpLabel *lbl);
    void            RenvoieRangee(UpCheckBox *Check);
    LigneCompte*    getLigneCompteFromRow(int id);
    int             getRowFromLigneCompte(LigneCompte *lign);
    void            SetLigneCompteToRow(LigneCompte *lign, int row);
    void            VoirArchives();
};

#endif // DLG_COMPTES_H
