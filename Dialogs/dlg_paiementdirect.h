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

#ifndef DLG_PAIEMENTDIRECT_H
#define DLG_PAIEMENTDIRECT_H

#include "dlg_gestionbanques.h"
#include "procedures.h"
#include "flags.h"

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
    explicit dlg_paiementdirect(QList<int> ListidActeAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_paiementdirect();
    bool                    initOK() const;
    enum Mode               {Accueil, EnregistrePaiement, VoirListeActes};   Q_ENUM(Mode)
    enum OrdreTri           {Alphabetique, Chronologique};                   Q_ENUM(OrdreTri)
    enum TypeTable          {ActesDirects,ActesTiers,Paiements};             Q_ENUM(TypeTable)                    // définit les 3 types de tables possibles dans la fiche
    enum ResultEnregRecette {Impossible, Annul, OK};                         Q_ENUM(ResultEnregRecette)

private:
    Ui::dlg_paiementdirect      *ui;
    Procedures                  *proc   = Procedures::I();
    DataBase                    *db     = DataBase::I();
    OrdreTri                    m_ordretri = Chronologique;

    Mode                        m_mode;
    TypeTable                   m_typetable;

    bool                        m_ok;
    bool                        m_fermealafin;
    bool                        m_initok;
    bool                        m_modiflignerecettepossible;
    bool                        m_modifpaiementencours;
    bool                        m_traiteparcloseflag;
    int                         m_idrecette;
    QString                     m_modepaiementdirectamodifier;
    QString                     m_valeuravantchangement, m_valeurmaxi;
    QList<int>                  m_listidactes;
    QList<int>                  m_listidactesamodifier;
    QList<QString>              m_lignecommissioncompteamodifier;
    QList<QString>              m_lignecompteamodifier;
    QList<QString>              m_lignedepenseamodifier;
    QList<QString>              m_lignerecetteamodifier;
    QList<QString>              m_montantactesamodifier;

    QMap<int, User*>            *map_comptables;
    QMap<int, Banque*>          *map_banques;
    QList<TypeTiers*>           *m_typestiers;
    User                        *m_userConnected = Datas::I()->users->userconnected();
    User                        *m_useracrediter = Q_NULLPTR;

    QBrush                      m_textureGray;
    QTimer                      *t_timerrecord, *t_timerafficheacteverrouille, *t_timerafficheacteverrouilleclignotant;
    UpPushButton                *wdg_supprimeuopushbutton;

    dlg_gestionbanques           *Dlg_Banq;

    void                        closeEvent(QCloseEvent *event);
    bool                        eventFilter(QObject *obj, QEvent *event)  ;
    void                        ChangeComptable(User *comptable, bool depuislecombo = false);
    void                        CompleteDetailsTable(UpTableWidget *TableSource, int Rangee, bool Coche = true);
    void                        DefinitArchitectureTable(UpTableWidget *TableARemplir, TypeTable typetable);
    ResultEnregRecette          EnregistreRecette();
    void                        FiltreLesTables();
    void                        ModifGratuitChoixMenu(QString Choix);
    void                        NettoieVerrousCompta();
    void                        PoseVerrouCompta(int ActeAVerrouiller);
    void                        ReconstruitListeBanques();
    void                        ReconstruitListeTiers();
    void                        RegleAffichageTypePaiementframe(bool VerifierEmetteur = true, bool AppeleParClicK = false);
    void                        RegleComptesComboBox(bool avecLesComptesInactifs = false);
    void                        RemetToutAZero();
    void                        RemplirTableWidget(QTableWidget *TableARemplir, QString TypeTable, QList<QVariantList> Tablelist, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas);
    void                        RemplitLesTables();
    void                        ResizePaiementGroupBox();
    void                        RetireVerrouCompta(int ActeADeverrouiller);
    void                        TrieListe(UpTableWidget *TableATrier);
    bool                        VerifCoherencePaiement();
    bool                        VerifVerrouCompta(UpTableWidget *TableAVerifier, int Rangee);
    void                        VideDetailsTable(int Rangee);
    void                        Annuler();

    void                        AfficheActeVerrouille();
    void                        AfficheActeVerrouilleClignotant();
    void                        AfficheToolTip(QTableWidgetItem*);
    void                        AfficheRecord();
    void                        CalculTotalDetails();
    void                        ClassementListes(int col);
    void                        ChangeComptableBox();
    void                        ConvertitDoubleMontant();
    void                        EnableOKButton();
    void                        EnregistreNouveauPaiement();
    void                        FiltreListe(QString filtre);
    void                        ListeActes();
    void                        Majuscule();
    void                        ModifGratuit(QPoint pos);
    void                        ModifiePaiement();
    void                        RegleAffichageFiche();
    void                        RegleAffichageTypePaiementframeDepuisBouton();
    void                        RenvoieRangee(bool Coche = true);
    void                        SupprimerPaiement();
    void                        ValidePaiement();
};

#endif // DLG_PAIEMENTDIRECT_H
