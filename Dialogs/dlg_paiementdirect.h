/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_PAIEMENTDIRECT_H
#define DLG_PAIEMENTDIRECT_H

#include <QDialog>
#include <QSqlQuery>
#include "dlg_banque.h"
#include "procedures.h"
#include "database.h"
#include "gbl_datas.h"
#include "icons.h"
#include "utils.h"

namespace Ui {
class dlg_paiementdirect;
}

/*!
 * \brief l'ensemble des informations concernant un paiments direct ( = en attente d'être enregistré )
 * rappel pour un acte
 * iduser           = le soignant responsable médical de l'acte
 * creepar          = le soignant qui a cree l'acte
 * userparent       = le soignant remplacé si iduser est un remplaçant
 * usercomptable    = le user qui comptabilise l'acte
 */

class dlg_paiementdirect : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_paiementdirect(QList<int> ListidActeAPasser, Procedures *procAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_paiementdirect();
    bool                        getInitOK();

private:
    Ui::dlg_paiementdirect      *ui;
    Procedures                  *proc;
    DataBase                    *db;

    bool                        FermeALaFin;
    bool                        InitOK;
    bool                        ModifLigneRecettePossible;
    bool                        ModifPaiementEnCours;
    bool                        TraiteparCloseFlag;
    int                         gidComptableACrediter;
    int                         gidCompteBancaireParDefaut;
    int                         gidRecette;
    int                     gMode;
        enum gMode              {Accueil, EnregistrePaiement, ModifiePaiement, VoirListeActes};
    int                     gOrdreTri;
        enum gOrdreTri          {Alphabetique, Chronologique};
    int                     gTypeTable;
        enum gTypeTable         {ActesDirects,ActesTiers,Paiements};                    // définit les 3 types de tables possibles dans la fiche
    int                     ResultEnregRecette;
        enum ResultEnregRecette {Impossible, Annul, OK};
    QString                     ModeModif, ModePaiementDirectAModifier;
    QString                     ValeurAvantChangement, ValeurMaxi;

    QBrush                      gtextureGris, gtextureNoir;
    QList<int>                  gListidActe;
    QList<int>                  ListeActesAModifier;
    QList<QString>              LigneCommissionCompteAModifier;
    QList<QString>              LigneCompteAModifier;
    QList<QString>              LigneDepenseAModifier;
    QList<QString>              LigneRecetteAModifier;
    QList<QString>              MontantActesAModifier;
    QMap<int, User*>            *m_listeComptables;
    QMap<int, Banque*>          *m_listeBanques;
    QStandardItemModel          *glistComptesEncaissmt;
    QStandardItemModel          *glistComptesEncaissmtAvecDesactive;
    QTimer                      *gtimerRecord, *gtimerAfficheActeVerrouille, *gtimerAfficheActeVerrouilleClignotant;
    UpPushButton                *SupprimerBouton;
    User                        *m_userConnected, *UserComptableACrediter;
    QList<TypeTiers*>           *m_typestiers;

    dlg_banque                  *Dlg_Banq;

    void                        closeEvent(QCloseEvent *event);
    bool                        eventFilter(QObject *obj, QEvent *event)  ;
    void                        CompleteDetailsTable(UpTableWidget *TableSource, int Rangee, bool Coche = true);
    void                        DefinitArchitectureTableView(UpTableWidget *TableARemplir, int TypeTable = 0);
    int                         EnregistreRecette();
    void                        ModifGratuitChoixMenu(QString Choix);
    void                        NettoieVerrousCompta();
    void                        PoseVerrouCompta(int ActeAVerrouiller);
    void                        ReconstruitListeBanques();
    void                        ReconstruitListeTiers();
    void                        RegleAffichageTypePaiementframe(bool VerifierEmetteur = true, bool AppeleParClicK = false);
    void                        RegleComptesComboBox(bool ActiveSeult = true);
    void                        RemetToutAZero();
    void                        RemplirTableWidget(QTableWidget *TableARemplir, QString TypeTable, QSqlQuery TableQuery, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas);
    void                        RemplitLesTables(int Mode);
    void                        ResizePaiementGroupBox();
    void                        RetireVerrouCompta(int ActeADeverrouiller);
    void                        TrieListe(UpTableWidget *TableATrier);
    bool                        VerifCoherencePaiement();
    bool                        VerifVerrouCompta(UpTableWidget *TableAVerifier, int Rangee);
    void                        VideDetailsTable(int Rangee);

    void                        Annuler();
private slots:
    void                        Slot_AfficheActeVerrouille();
    void                        Slot_AfficheActeVerrouilleClignotant();
    void                        Slot_AfficheDDN(QTableWidgetItem*);
    void                        Slot_AfficheRecord();
    void                        Slot_CalculTotalDetails();
    void                        Slot_ChangeUtilisateur();
    void                        Slot_ClassementListes(int col);
    void                        Slot_ConvertitDoubleMontant();
    void                        Slot_EnableOKButton();
    void                        Slot_EnregistrePaiement();
    void                        Slot_FiltreListe(QString filtre);
    void                        Slot_Majuscule();
    void                        Slot_ModifGratuit(QPoint pos);
    void                        Slot_ModifiePaiement();
    void                        Slot_RegleAffichageFiche();
    void                        Slot_RegleAffichageTypePaiementframe();
    void                        Slot_RenvoieRangee(bool Coche = true);
    void                        Slot_SupprimerPaiement();
    void                        Slot_ValidePaiement();
    void                        Slot_VoirListeActes();

};

#endif // DLG_PAIEMENTDIRECT_H
