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

#ifndef DLG_PAIEMENT_H
#define DLG_PAIEMENT_H

#include <QMessageBox>
#include <QToolTip>
#include "procedures.h"
#include "uplineedit.h"
#include "upcheckbox.h"
#include "dlg_banque.h"
#include "ui_dlg_banque.h"

namespace Ui {
class dlg_paiement;
}

class dlg_paiement : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_paiement(QList<int> ListidActeAPasser, int Mode, Procedures *procAPasser, int PaiementAModifer=0, int idUser=0, QWidget *parent = Q_NULLPTR);
    ~dlg_paiement();
    Ui::dlg_paiement    *ui;
    bool                getInitOK();
    void                setInitOK(bool init);

private slots:
    void                Slot_AfficheActeVerrouille();
    void                Slot_AfficheActeVerrouilleClignotant();
    void                Slot_AfficheDDN(QTableWidgetItem*);
    void                Slot_AfficheRecord();
    void                Slot_Annul();
    void                Slot_CalculTotalDetails();
    void                Slot_ChangeUtilisateur();
    void                Slot_ClassementListes();
    void                Slot_ConvertitDoubleMontant();
    void                Slot_EnableOKButton();
    void                Slot_EnregistrePaiementDirect();
    void                Slot_EnregistrePaiementTiers();
    void                Slot_MajusculeCreerNom();
    void                Slot_ModifGratuit(QPoint pos);
    void                Slot_RecopieNomTiers(QString);
    void                Slot_SupprimerPaiement();
    void                Slot_VoirListeActes();
    void                Slot_VoirListePaiementsTiers();
    void                Slot_RegleAffichageFiche();
    void                Slot_RegleAffichageTypePaiementframe();
    void                Slot_RenvoieRangee(bool Coche = true);
    void                Slot_ValidePaiement();

private:
    void                    ModifGratuitChoixMenu(QString Choix);
    /*bool                    ophtalmo;
    bool                    orthoptist;
    bool                    autresoignant;
    bool                    secretaire;
    bool                    assistant;
    bool                    liberal;
    bool                    salarie;
    bool                    remplacant;
    bool                    pasremplacant;
    bool                    soignant;
    */
    bool                    AppeleParFichePaiement;
    bool                    FermeALaFin;
    bool                    InitOK;
    bool                    TraiteparCloseFlag;
    bool                    ModifPaiementEnCours;
    bool                    ModifLigneRecettePossible;
    //int                     gidUser;
    int                     gidUserACrediter;
    int                     gFicheMode;
    int                     idRecette;
        enum gFicheMode     {Tiers, Direct};
    int                     gMode;
        enum gMode          {Accueil, EnregistrePaiementDirect, VoirListeActes, EnregistrePaiementTiers, VoirListePaiementsTiers};
    int                     gOrdreTri;
        enum gOrdreTri      {Alphabetique, Chronologique};
    int                     gTypeTable;
        enum gTypeTable     {ActesDirects,ActesTiers,Paiements};                    // définit les 3 types de tables possibles dans la fiche
    dlg_banque              *Dlg_Banq;
    Procedures              *proc;
    QBrush                  gtextureGris, gtextureNoir;
    QList<int>              gListidActe;
    QList<int>              ListeActesAModifier;
    QList<QString>          LigneCommissionCompteAModifier;
    QList<QString>          LigneCompteAModifier;
    QList<QString>          LigneRecetteAModifier;
    QList<QString>          LigneDepenseAModifier;
    QList<QString>          MontantActesAModifier;
    User                    *gDataUser;
    User                    *m_userConnected;
    QStandardItemModel      *glistComptesEncaissmt;
    QStandardItemModel      *glistComptesEncaissmtAvecDesactive;
    QMap<int, User*>        *m_listeParents;

    QSqlDatabase            db;
    int                     gidCompteBancaireParDefaut;
    QString                 ModeModif, ModePaiementDirectAModifier;
    QString                 gNomUser;
    QString                 ValeurAvantChangement, ValeurMaxi;
    QTimer                  *gtimerRecord, *gtimerAfficheActeVerrouille, *gtimerAfficheActeVerrouilleClignotant;
    UpPushButton            *SupprimerBouton;

    void                closeEvent(QCloseEvent *event);
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                keyPressEvent ( QKeyEvent * event );
    void                CompleteDetailsTable(QTableWidget *TableSource, int Rangee, bool Coche = true);
    void                DefinitArchitectureTableView(QTableWidget *TableARemplir, int TypeTable = 0);
    int                 EnregistreRecette();
    int                 ResultEnregRecette;
        enum ResultEnregRecette {Impossible, Annul, OK};
    void                ModifPaiementTiers(int idRecetteAModifier);
    void                NettoieVerrousListeActesAAfficher();
    void                NettoieVerrousCompta();
    void                PoseVerrouCompta(int ActeAVerrouiller);
    void                RetireVerrouCompta(int ActeADeverrouiller);
    void                ReconstruitListeBanques();
    void                ReconstruitListeTiers();
    void                RegleAffichageTypePaiementframe(bool VerifierEmetteur = true, bool AppeleParClicK = false);
    void                RegleComptesComboBox(bool ActiveSeult = true);
    void                RemetToutAZero();
    void                RemplirTableWidget(QTableWidget *TableARemplir, QString TypeTable, QSqlQuery TableQuery, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas);
    void                RemplitLesTables(int Mode);
    void                TrieListe(QTableWidget *TableATrier);
    bool                VerifCoherencePaiement();
    bool                VerifVerrouCompta(QTableWidget *TableAVerifier, int Rangee);
    void                VideDetailsTable(int Rangee);
};

#endif // DLG_PAIEMENT_H
