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

#ifndef dlg_paiementtiers_H
#define dlg_paiementtiers_H

#include <QMessageBox>
#include <QToolTip>
#include "procedures.h"
#include "uplineedit.h"
#include "upcheckbox.h"
#include "dlg_gestionbanques.h"
#include "ui_dlg_gestionbanques.h"
#include "flags.h"

namespace Ui {
class dlg_paiementtiers;
}

class dlg_paiementtiers : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_paiementtiers(QWidget *parent = Q_NULLPTR);
    ~dlg_paiementtiers();
    Ui::dlg_paiementtiers   *ui;
    bool                    initOK() const;
    enum Mode               {Accueil, EnregistrePaiementTiers, VoirListePaiementsTiers};    Q_ENUM(Mode)
    enum OrdreTri           {Alphabetique, Chronologique};                                  Q_ENUM(OrdreTri)
    enum TypeTable          {Actes,Paiements};                                              Q_ENUM(TypeTable)
    enum ResultEnregRecette {Impossible, Annul, OK};                                        Q_ENUM(ResultEnregRecette)

private slots:
    void                Slot_AfficheActeVerrouille();
    void                Slot_AfficheActeVerrouilleClignotant();
    void                Slot_AfficheDDN(QTableWidgetItem*);
    void                Slot_AfficheRecord();
    void                Slot_Annul();
    void                Slot_CalculTotalDetails();
    void                Slot_ChangeUtilisateur();
    void                Slot_ClassementListes(int col);
    void                Slot_ConvertitDoubleMontant();
    void                Slot_EnableOKButton();
    void                Slot_EnregistrePaiementTiers();
    void                Slot_FiltreListe(QString filtre);
    void                Slot_MajusculeCreerNom();
    void                Slot_RecopieNomTiers(QString);
    void                Slot_SupprimerPaiement();
    void                Slot_VoirListePaiementsTiers();
    void                Slot_RegleAffichageFiche();
    void                Slot_RegleAffichageTypePaiementframe();
    void                Slot_RenvoieRangee(bool Coche = true);
    void                Slot_ValidePaiement();

private:
    Procedures              *proc   = Procedures::I();
    DataBase                *db     = DataBase::I();

    Mode                    m_mode;
    OrdreTri                m_ordretri;
    dlg_gestionbanques      *Dlg_Banq;

    bool                    m_ok;
    bool                    m_initok;
    bool                    m_traiteparcloseflag;
    bool                    m_modifpaiementencours;
    bool                    m_modiflignerecettepossible;
    int                     m_idrecette;
    QString                 m_valeuravantchangement, m_vleurmaxi;
    QList<int>              m_listidactes;
    QList<int>              m_listactesamodifier;
    QList<QString>          m_lignecommissioncompteamodifier;
    QList<QString>          m_lignecompteamodifier;
    QList<QString>          m_lignerecetteamodifier;
    QList<QString>          m_lignedepenseamodifier;
    QList<QString>          m_montantactesamodifier;

    QTimer                  *t_timerrecord, *t_timerafficheacteverrouille, *t_timerafficheacteverrouilleclignotant;
    User                    *m_useracrediter    = Q_NULLPTR;
    User                    *m_userconnected    = Datas::I()->users->userconnected();

    QMap<int, User*>        *map_comptables;


    void                closeEvent(QCloseEvent *event);
    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                CompleteDetailsTable(QTableWidget *TableOrigine, int Rangee, bool Coche = true);
    void                DefinitArchitectureTableView(UpTableWidget *TableARemplir, TypeTable Type);
    ResultEnregRecette  EnregistreRecette();
    void                ModifPaiementTiers(int idRecetteAModifier);
    void                NettoieVerrousListeActesAAfficher();
    void                NettoieVerrousCompta();
    void                PoseVerrouCompta(int ActeAVerrouiller);
    void                RetireVerrouCompta(int ActeADeverrouiller);
    void                ReconstruitListeBanques();
    void                ReconstruitListeTiers();
    void                RegleAffichageTypePaiementframe(bool VerifierEmetteur = true, bool AppeleParClicK = false);
    void                RegleComptesComboBox(bool avecLesComptesInactifs = false);
    void                RemetToutAZero();
    void                RemplirTableWidget(UpTableWidget *TableARemplir, TypeTable Type, QList<QVariantList> reclist, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas);
    void                RemplitLesTables();
    void                TrieListe(QTableWidget *TableATrier);
    bool                VerifCoherencePaiement();
    bool                VerifVerrouCompta(QTableWidget *TableAVerifier, int Rangee);
    void                VideDetailsTable(int Rangee);
};

#endif // dlg_paiementtiers_H
