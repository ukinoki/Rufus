/* (C) 2016 LAINE SERGE
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

#include "dlg_paiement.h"
#include "gbl_datas.h"
#include "icons.h"
#include "ui_dlg_paiement.h"
#include "utils.h"

/*
 * la fiche est appelée de 3 façons différentes:
 * . choix de menu dans le menu Comptabilité de Rufus
 * . choix "Enregistre le paiement" dans le menu contextuel de l'accueil
 * . bouton "Enregistrer le paiement" dans la consultation
*/
// BUG : a faire ensuite PAIEMENT
dlg_paiement::dlg_paiement(QList<int> ListidActeAPasser, int Mode, Procedures *procAPasser, int PaiementAModifer, int idUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_paiement)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    proc            = procAPasser;
    db              = DataBase::getInstance();
    gListidActe     = ListidActeAPasser;
    m_userConnected  = proc->getUserConnected();
    gidUserACrediter = -1;
    //ui->UserscomboBox->setEnabled(proc->getUserConnected().isSecretaire());
    QFont font = qApp->font();
    font.setBold(true);
    font.setItalic(true);
    font.setPointSize(font.pointSize()+5);
    ui->Utilisateurlabel->setFont(font);
    font.setItalic(false);
    font.setPointSize(font.pointSize()+12);
    ui->PasdePaiementlabel->setFont(font);

    if (Mode < 2)
        gFicheMode = Direct;
    else
        gFicheMode = Tiers;
    FermeALaFin = (gListidActe.at(0) > 0);
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionPaiement").toByteArray());

    m_listeParents = Datas::I()->users->parents(); // les colonnes -> iduser, userlogin, soignant, responsableactes, UserEnregHonoraires, idCompteEncaissHonoraires

    if (idUser == 0)        //1. il y a un ou pusieurs actes à enregistrer
    {
        // la fiche a été appelée par le bouton "enregistrer le paiement"
        if (gListidActe.at(0)>0)
        {
            //TODO : SQL
            QString req = "select iduser, UserComptable from " NOM_TABLE_ACTES
                          " where idacte = " + QString::number(gListidActe.at(0));
            QSqlQuery quer(req, db->getDataBase());
            db->traiteErreurRequete(quer,req,"");
            quer.first();
            gidUserACrediter = quer.value(1).toInt();
        }
        // la fiche a été appelée par le menu et il n'y a pas d'acte prédéterminé à enregistrer
        else if (m_userConnected->isLiberal())
            gidUserACrediter = m_userConnected->id();
        else if (m_userConnected->isSalarie() && !m_userConnected->isAssistant())// l'utilisateur est un soignant salarie et responsable
            gidUserACrediter = proc->UserParent();
        else if (m_userConnected->isRemplacant())                                // l'utilisateur est un soignant remplacant et responsable
            gidUserACrediter = proc->UserParent();
        else if(m_userConnected->isSecretaire())
            gidUserACrediter = m_listeParents->first()->id();
    }
    else // la fiche a été appelée par elle-même pour modifier un enregistrement
    {
        gidUserACrediter = idUser;
        ui->UserscomboBox->setEnabled(false);
    }
    if (gidUserACrediter == -1)
    {
        InitOK = false;
        return;
    }

    gDataUser                               = Datas::I()->users->getUserById(gidUserACrediter);
    if (gDataUser != nullptr)
    {
        gNomUser                            = gDataUser->getLogin();
        gidCompteBancaireParDefaut          = gDataUser->getIdCompteEncaissHonoraires();
        proc                                ->setListeComptesEncaissmtUser(gidUserACrediter);
        glistComptesEncaissmt               = proc->getListeComptesEncaissmtUser();
        glistComptesEncaissmtAvecDesactive  = proc->getListeComptesEncaissmtUserAvecDesactive();
    }
    if( gDataUser == nullptr || glistComptesEncaissmt->rowCount() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche de paiement"), tr("Les paramètres ne sont pas trouvés pour le compte ") + proc->getLogin(gidUserACrediter));
        InitOK = false;
        return;
    }


    // On reconstruit le combobox des utilisateurs avec la liste des utilisateurs qui encaissent des honoraires et qui travaillent encore
    int index = 0;
    for( QMap<int, User*>::const_iterator itParent = m_listeParents->constBegin(); itParent != m_listeParents->constEnd(); ++itParent )
    {
        ui->UserscomboBox->addItem(itParent.value()->getLogin(), QString::number(itParent.value()->id()) );
        if( gidUserACrediter != itParent.value()->id())
            ++index;
    }
    if(index>=m_listeParents->size())
        ui->UserscomboBox->setCurrentIndex(0);
    else
        ui->UserscomboBox->setCurrentIndex(index);

    // idem pour les comptes
    RegleComptesComboBox();
    ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));

    connect (ui->AnnulupPushButton,                     SIGNAL(clicked()),                              this,           SLOT (Slot_Annul()));
    connect (ui->BanqueChequecomboBox,                  SIGNAL(editTextChanged(QString)),               this,           SLOT (Slot_EnableOKButton()));
    connect (ui->CarteCreditradioButton,                SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));
    connect (ui->ChequeradioButton,                     SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));
    connect (ui->CommissionlineEdit,                    SIGNAL(editingFinished()),                      this,           SLOT (Slot_ConvertitDoubleMontant()));
    connect (ui->CommissionlineEdit,                    SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKButton()));
    connect (ui->ComptesupComboBox,                     SIGNAL(currentIndexChanged(int)),               this,           SLOT (Slot_EnableOKButton()));
    connect (ui->dateEdit,                              SIGNAL(dateChanged(QDate)),                     this,           SLOT (Slot_EnableOKButton()));
    connect (ui->EnAttentecheckBox,                     SIGNAL(toggled(bool)),                          this,           SLOT (Slot_EnableOKButton()));
    connect (ui->EspecesradioButton,                    SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));
    connect (ui->GratuitradioButton,                    SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));
    connect (ui->ImpayeradioButton,                     SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));
    connect (ui->ListActesupPushButton,                 SIGNAL(clicked()),                              this,           SLOT (Slot_VoirListeActes()));
    connect (ui->ListPaiementsTiersupPushButton,        SIGNAL(clicked()),                              this,           SLOT (Slot_VoirListePaiementsTiers()));
    connect (ui->ListeupTableWidget,                    SIGNAL(customContextMenuRequested(QPoint)),     this,           SLOT (Slot_ModifGratuit(QPoint)));
    connect (ui->ListeupTableWidget,                    SIGNAL(itemEntered(QTableWidgetItem*)),         this,           SLOT (Slot_AfficheDDN(QTableWidgetItem*)));
    connect (ui->MontantlineEdit,                       SIGNAL(editingFinished()),                      this,           SLOT (Slot_ConvertitDoubleMontant()));
    connect (ui->MontantlineEdit,                       SIGNAL(textEdited(QString)),                    this,           SLOT (Slot_EnableOKButton()));
    connect (ui->OrdreupPushButton,                     SIGNAL(clicked()),                              this,           SLOT (Slot_ClassementListes()));
    connect (ui->NouvPaiementupPushButton,              SIGNAL(clicked()),                              this,           SLOT (Slot_EnregistrePaiementDirect()));
    connect (ui->NouvTiersupPushButton,                 SIGNAL(clicked()),                              this,           SLOT (Slot_EnregistrePaiementTiers()));
    connect (ui->TierscomboBox,                         SIGNAL(editTextChanged(QString)),               this,           SLOT (Slot_RecopieNomTiers(QString)));
    connect (ui->TiersradioButton,                      SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));
    connect (ui->TireurChequelineEdit,                  SIGNAL(editingFinished()),                      this,           SLOT (Slot_MajusculeCreerNom()));
    connect (ui->UserscomboBox,                         SIGNAL(currentIndexChanged(int)),               this,           SLOT (Slot_ChangeUtilisateur()));
    connect (ui->OKupPushButton,                        SIGNAL(clicked()),                              this,           SLOT (Slot_ValidePaiement()));
    connect (ui->VirementradioButton,                   SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));


    ui->TireurChequelineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->TireurChequelineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MontantlineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QDoubleValidator *val= new QDoubleValidator(this);
    val->setDecimals(2);
    ui->MontantlineEdit->setValidator(val);
    ui->CommissionlineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->CommissionlineEdit->setValidator(val);
    QRegExp val2 = QRegExp("[A-Z]*");
    ui->BanqueChequecomboBox->setValidator(new QRegExpValidator(val2));
    ui->BanqueChequecomboBox->lineEdit()->setMaxLength(10);
    ui->TierscomboBox->lineEdit()->setMaxLength(30);

    ReconstruitListeBanques();
    ReconstruitListeTiers();
    gOrdreTri = Chronologique;

    ui->RecImageLabel->setVisible(false);
    gtimerRecord = new QTimer(this);
    gtimerRecord->start(750);
    connect (gtimerRecord,                              SIGNAL(timeout()),                              this,           SLOT (Slot_AfficheRecord()));

    SupprimerBouton = Q_NULLPTR;
    ui->DetailupTableWidget->installEventFilter(this);
    ui->VerrouilleParlabel->setVisible(false);
    gtimerAfficheActeVerrouilleClignotant = new QTimer();
    gtimerAfficheActeVerrouille = new QTimer();
    QList<UpPushButton *> allUpButtons = ui->Buttonsframe->findChildren<UpPushButton *>();
    for (int n = 0; n <  allUpButtons.size(); n++)
    {
        allUpButtons.at(n)->installEventFilter(this);
        allUpButtons.at(n)->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Large);
    }
    installEventFilter(this);
    ui->OKupPushButton->setShortcut(QKeySequence("Meta+Return"));
    ui->AnnulupPushButton->setShortcut(QKeySequence("F12"));
    ModeModif   = "";
    ModifPaiementEnCours = false;
    ModifLigneRecettePossible = true;
    ui->PasdePaiementlabel->setVisible(false);
    gtextureGris = QBrush(Qt::gray,Qt::Dense4Pattern);
    gtextureNoir = QBrush(Qt::NoBrush);

    ui->RecImageLabel->setPixmap( Icons::pxEnregistrer() );

    if (PaiementAModifer == 0)
    {
        AppeleParFichePaiement = false;
        switch (gFicheMode) {
        case Direct:{
            if (gListidActe.at(0) != 0)
            {
                gMode = EnregistrePaiementDirect;
                Slot_EnregistrePaiementDirect();
            }
            else
            {
                gMode = Accueil;
                ui->RecImageLabel->setVisible(false);
                Slot_RegleAffichageFiche();
            }
            break;}
        case Tiers:{
            gMode = Accueil;
            ui->RecImageLabel->setVisible(false);
            Slot_RegleAffichageFiche();
            break;}
        default:{
            gMode = Accueil;
            Slot_RegleAffichageFiche();
            break;}
        }
    }
    else
    {
        AppeleParFichePaiement = true;
        ModifPaiementTiers(PaiementAModifer);
    }
    InitOK = true;
}

dlg_paiement::~dlg_paiement()
{
    delete ui;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Acte Verrouillé ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_AfficheActeVerrouille()
{
    disconnect(gtimerAfficheActeVerrouilleClignotant, SIGNAL(timeout()),this,SLOT(Slot_AfficheActeVerrouilleClignotant()));
    gtimerAfficheActeVerrouilleClignotant->stop();
    ui->VerrouilleParlabel->setVisible(false);
    for (int i= 0; i != ui->SalleDAttenteupTableWidget->rowCount(); i++)
    {
        UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->SalleDAttenteupTableWidget->cellWidget(i,1));
        if (Check)
            Check->setToggleable(true);
    }
    for (int i= 0; i != ui->ListeupTableWidget->rowCount(); i++)
    {
        UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->ListeupTableWidget->cellWidget(i,1));
        if (Check)
            Check->setToggleable(true);
    }


}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Affiche la DDN dans un tooltip ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_AfficheDDN(QTableWidgetItem *titem)
{
    if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
    {
        if (titem->column() == 3)
        {
            int ro = titem->row();
            QString req = "Select PATDDN from " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_ACTES " act  where pat.idpat = act .idpat and act.idacte = "
                    + ui->ListeupTableWidget->item(ro,0)->text();
            QSqlQuery quer(req,db->getDataBase());
            if (quer.size()>0)
            {
                quer.first();
                QToolTip::showText(cursor().pos(),quer.value(0).toDate().toString(tr("dd-MM-yyyy")));
            }
        }
        else
            QToolTip::showText(cursor().pos(),"");
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Acte Verrouillé ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_AfficheActeVerrouilleClignotant()
{
    ui->VerrouilleParlabel->setVisible(!ui->VerrouilleParlabel->isVisible());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Bouton Record -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_AfficheRecord()
{
    bool radioButtonClicked = false;
    QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
    for (int n = 0; n <  allRButtons.size(); n++)
        if (allRButtons.at(n)->isChecked())
        {
            radioButtonClicked = true;
            n = allRButtons.size();
        }
    if ((gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers) && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || radioButtonClicked || ui->DetailupTableWidget->rowCount() > 0))
        ui->RecImageLabel->setVisible(!ui->RecImageLabel->isVisible());
    else
        ui->RecImageLabel->setVisible(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Clic sur AnnulupPushButton ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_Annul()
{
    QString requete;
    ui->PasdePaiementlabel->setVisible(false);
    if (gMode == Accueil)
    {
        reject();
    }
    else if (ModifPaiementEnCours)
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Voulez vous vraiment annuler la modification de cette écriture?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton(tr("Annuler la modification"));
        UpSmallButton NoBouton;
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton()  != &OKBouton)
            return;       

        // Restaurer l'écriture à partir des Qlist
        // 1.       restaurer la ligne de recette
        if (ModifLigneRecettePossible)
        {
            // restaurer la ligne de recette
            requete = "INSERT INTO " NOM_TABLE_RECETTES " VALUES (";
            for (int i = 0; i < LigneRecetteAModifier.size(); i++)
            {
                requete += LigneRecetteAModifier.at(i);
                if (i < LigneRecetteAModifier.size() -1)
                    requete += ",";
             }
            requete += ")";

            QSqlQuery RestaureRecetteQuery (requete,db->getDataBase());
            db->traiteErreurRequete(RestaureRecetteQuery,requete,"");
            LigneRecetteAModifier.clear();

            //restaurer la ligne de commission s'il y en a eu une
            if (LigneDepenseAModifier.size() > 0)
            {
                requete = "INSERT INTO " NOM_TABLE_DEPENSES " VALUES (";
                for (int i = 0; i < LigneDepenseAModifier.size(); i++)
                {
                    requete += LigneDepenseAModifier.at(i);
                    if (i < LigneDepenseAModifier.size() -1)
                        requete += ",";
                }
                requete += ")";
                //UpMessageBox::Watch(this,requete);
                QSqlQuery RestaureDepenseQuery (requete,db->getDataBase());
                db->traiteErreurRequete(RestaureDepenseQuery,requete,"");
                if (LigneCommissionCompteAModifier.size() > 0)
                {
                    requete = "INSERT INTO " NOM_TABLE_LIGNESCOMPTES " VALUES (";
                    for (int i = 0; i < LigneCommissionCompteAModifier.size(); i++)
                    {
                        requete += LigneCommissionCompteAModifier.at(i);
                        if (i < LigneCommissionCompteAModifier.size() -1)
                            requete += ",";
                    }
                    requete += ")";
                    //UpMessageBox::Watch(this,requete);
                    QSqlQuery RestaureCommissionCompteQuery (requete,db->getDataBase());
                    db->traiteErreurRequete(RestaureCommissionCompteQuery,requete,"");
                    LigneCommissionCompteAModifier.clear();
                }
                LigneDepenseAModifier.clear();
            }

            // restaurer la ligne de compte s'il y en a eu une
            if (LigneCompteAModifier.size() > 0)
            {
                requete = "INSERT INTO " NOM_TABLE_LIGNESCOMPTES " VALUES (";
                for (int i = 0; i < LigneCompteAModifier.size(); i++)
                {
                    requete += LigneCompteAModifier.at(i);
                    if (i < LigneCompteAModifier.size() -1)
                        requete += ",";
                }
                requete += ")";

                QSqlQuery RestaureLigneCompteQuery (requete,db->getDataBase());
                db->traiteErreurRequete(RestaureLigneCompteQuery,requete,"");
                LigneCompteAModifier.clear();
            }

       }

        // 2.       restaurer les lignes de paiement
        if (ListeActesAModifier.size() > 0)
        {
            requete = "INSERT INTO " NOM_TABLE_LIGNESPAIEMENTS " (idActe, idRecette, Paye) VALUES ";
            for (int i = 0; i < ListeActesAModifier.size(); i++)
            {
                requete += "(" + QString::number(ListeActesAModifier.at(i)) + "," +  QString::number(idRecette) + "," + MontantActesAModifier.at(i) + ")";
                if (i < ListeActesAModifier.size() -1)
                    requete += ",";
            }
            //UpMessageBox::Watch(this,requete);
            QSqlQuery RestaureLignesPaiementsQuery (requete,db->getDataBase());
            db->traiteErreurRequete(RestaureLignesPaiementsQuery,requete,"");
        }

        // 3.       restaurer les types de paiement quand il s'agit d'un paiement direct
        if (gMode == EnregistrePaiementDirect)
        {
            for (int i = 0; i < ListeActesAModifier.size(); i++)
            {
                requete = "select idacte FROM " NOM_TABLE_TYPEPAIEMENTACTES " where idActe = " + QString::number(ListeActesAModifier.at(i));
                QSqlQuery TypePaiementQuery (requete,db->getDataBase());
                db->traiteErreurRequete(TypePaiementQuery,requete,"");
                if (TypePaiementQuery.size() == 0)
                {
                    requete = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe,TypePaiement) VALUES "
                              "(" + QString::number(ListeActesAModifier.at(i)) + ",'" + ModePaiementDirectAModifier + "')";
                    QSqlQuery RestaureTypesPaiementsQuery (requete,db->getDataBase());
                    db->traiteErreurRequete(RestaureTypesPaiementsQuery,requete,"");
                }
            }
        }
        if (AppeleParFichePaiement) reject();
        ModifPaiementEnCours = false;
        gMode = Accueil;
        disconnect (ui->ListeupTableWidget,           SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
        disconnect (ui->SalleDAttenteupTableWidget,   SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
        ui->RecImageLabel->setVisible(false);
        Slot_RegleAffichageFiche();
        ui->AnnulupPushButton->setText("Annuler");
        TraiteparCloseFlag = false;
    }
    else
    {
        bool radioButtonClicked = false;
        QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
        for (int n = 0; n <  allRButtons.size(); n++)
            if (allRButtons.at(n)->isChecked())
            {
                radioButtonClicked = true;
                n = allRButtons.size();
            }
        if (
           ((gMode == EnregistrePaiementDirect) && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || radioButtonClicked || ui->DetailupTableWidget->rowCount() > 0))
           ||
           ((gMode == EnregistrePaiementTiers) && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || radioButtonClicked || ui->DetailupTableWidget->rowCount() > 0))
           )
        {
            UpMessageBox msgbox;
            msgbox.setText(tr("Voulez vous vraiment annuler cette écriture?"));
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton(tr("Annuler l'écriture"));
            UpSmallButton NoBouton;
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != &OKBouton)
                return;
            ui->AnnulupPushButton->setText(tr("Annuler"));
            if (FermeALaFin)
            {
                NettoieVerrousCompta();
                reject();
            }
            RemetToutAZero();
            return;
        }
        else
        {
            gMode = Accueil;
            disconnect (ui->ListeupTableWidget,           SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
            disconnect (ui->SalleDAttenteupTableWidget,   SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
            ui->RecImageLabel->setVisible(false);
            Slot_RegleAffichageFiche();
            TraiteparCloseFlag = false;
        }
    }
    ModifLigneRecettePossible = true;
    NettoieVerrousCompta();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Recalcule le total de la table Details -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_CalculTotalDetails()
{
    QTableWidget        *TableDetails = ui->DetailupTableWidget;;
    double Total = 0;
    if (TableDetails->rowCount() > 0)
    {
        for (int k = 0; k < TableDetails->rowCount(); k++)
        {
            QLineEdit* Line = dynamic_cast<QLineEdit*>(TableDetails->cellWidget(k,TableDetails->columnCount()-2));
            if (Line)
                Total = Total + QLocale().toDouble(Line->text());
            else
                Total = Total + QLocale().toDouble(TableDetails->item(k,TableDetails->columnCount()-2)->text());
        }
    }
    QString TotalRemise;
    TotalRemise = QLocale().toString(Total,'f',2);
    ui->TotallineEdit->setText(TotalRemise);
    if (gMode == EnregistrePaiementDirect)
    {
        Slot_EnableOKButton();
        if (ModifLigneRecettePossible && !ui->GratuitradioButton->isChecked() && !ui->ImpayeradioButton->isChecked())
            ui->MontantlineEdit->setText(TotalRemise);
        if (ui->ImpayeradioButton->isChecked())
        {
            QString Montant;
            double TotalImpaye = 0;
            for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                TotalImpaye += QLocale().toDouble(ui->DetailupTableWidget->item(i,5)->text());
            Montant = QLocale().toString(TotalImpaye,'f',2);
            ui->MontantlineEdit->setText(Montant);
        }
    }
    if (gMode == EnregistrePaiementTiers)
        Slot_EnableOKButton();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_ChangeUtilisateur()
{
    gDataUser = Datas::I()->users->getUserById(ui->UserscomboBox->currentData().toInt());
    if (gDataUser != nullptr)
    {
        gNomUser                            = gDataUser->getLogin();
        gidCompteBancaireParDefaut          = gDataUser->getIdCompteEncaissHonoraires();
        proc                                ->setListeComptesEncaissmtUser(gidCompteBancaireParDefaut);
        glistComptesEncaissmt               = proc->getListeComptesEncaissmtUser();
        glistComptesEncaissmtAvecDesactive  = proc->getListeComptesEncaissmtUserAvecDesactive();
    }
    if (gDataUser == nullptr || glistComptesEncaissmt->rowCount() == 0)
    {
        UpMessageBox::Watch                 (this,tr("Impossible de changer d'utilisateur!") , tr("Les paramètres de") + ui->UserscomboBox->currentText() + tr("ne sont pas retrouvés"));
        disconnect (ui->UserscomboBox,      SIGNAL(currentIndexChanged(int)),   this,   SLOT (Slot_ChangeUtilisateur()));
        ui->UserscomboBox                   ->setCurrentIndex(ui->UserscomboBox->findData(gidUserACrediter));
        connect (ui->UserscomboBox,         SIGNAL(currentIndexChanged(int)),   this,   SLOT (Slot_ChangeUtilisateur()));
        gDataUser                           = Datas::I()->users->getUserById(ui->UserscomboBox->currentData().toInt());
        gNomUser                            = gDataUser->getLogin();
        gidCompteBancaireParDefaut          = gDataUser->getIdCompteEncaissHonoraires();
        proc                                ->setListeComptesEncaissmtUser(gidUserACrediter);
        glistComptesEncaissmt               = proc->getListeComptesEncaissmtUser();
        glistComptesEncaissmtAvecDesactive  = proc->getListeComptesEncaissmtUserAvecDesactive();
        return;
    }

    gidUserACrediter =  ui->UserscomboBox->currentData().toInt();
    RegleComptesComboBox();
    ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));
    AppeleParFichePaiement = false;
    switch (gFicheMode) {
    case Direct:{
        if (gListidActe.at(0) != 0)
        {
            gMode = EnregistrePaiementDirect;
            Slot_EnregistrePaiementDirect();
        }
        else
        {
            gMode = Accueil;
            ui->RecImageLabel->setVisible(false);
            Slot_RegleAffichageFiche();
        }
        break;}
    case Tiers:{
        gMode = Accueil;
        ui->RecImageLabel->setVisible(false);
        Slot_RegleAffichageFiche();
        break;}
    default:{
        gMode = Accueil;
        Slot_RegleAffichageFiche();
        break;}
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Modifie l'ordre de tri des tables ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_ClassementListes()
{
    if (gOrdreTri == Chronologique)
    {
        ui->OrdreupPushButton->setText(tr("Ordre chronologique"));
        gOrdreTri = Alphabetique;
    }
    else
    {
        ui->OrdreupPushButton->setText(tr("Ordre alphabétique"));
        gOrdreTri = Chronologique;
    }
    TrieListe(ui->ListeupTableWidget);
}

void dlg_paiement::Slot_ConvertitDoubleMontant()
{
    QLineEdit * Emetteur = qobject_cast<QLineEdit*> (sender());
    QString b;
    b = QLocale().toString(QLocale().toDouble(Emetteur->text()),'f',2);
    Emetteur->setText(b);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enable AnnulButton -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_EnableOKButton()
{
    ui->OKupPushButton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer un nouveau paiement direct -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_EnregistrePaiementDirect()
{
    gMode = EnregistrePaiementDirect;
    RemplitLesTables(gMode);
    if (ui->ListeupTableWidget->rowCount() == 0
            && ui->DetailupTableWidget->rowCount() == 0
            && ui->SalleDAttenteupTableWidget->rowCount() == 0)
    {
        gMode = Accueil;
        disconnect (ui->ListeupTableWidget,           SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
        disconnect (ui->SalleDAttenteupTableWidget,   SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
        ui->RecImageLabel->setVisible(false);
        Slot_RegleAffichageFiche();
        TraiteparCloseFlag = false;
    }
    else
    {
        Slot_RegleAffichageFiche();
        if (ui->DetailupTableWidget->rowCount() > 0)
            ui->OKupPushButton->setEnabled(true);
        ui->CarteCreditradioButton->setChecked(true);
        RegleAffichageTypePaiementframe(true,true);
        ui->PaiementgroupBox->setFocus();
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer un nouveau paiement par tiers ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_EnregistrePaiementTiers()
{
    gMode = EnregistrePaiementTiers;
    RemplitLesTables(gMode);
    if (ui->ListeupTableWidget->rowCount() == 0
            && ui->DetailupTableWidget->rowCount() == 0)
    {
        gMode = Accueil;
        disconnect (ui->ListeupTableWidget,           SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
        disconnect (ui->SalleDAttenteupTableWidget,   SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
        ui->RecImageLabel->setVisible(false);
        Slot_RegleAffichageFiche();
        TraiteparCloseFlag = false;
    }
    else
    {
        Slot_RegleAffichageFiche();
        ui->VirementradioButton->setChecked(true);
        RegleAffichageTypePaiementframe(true,true);
        ui->PaiementgroupBox->setFocus();
    }
}

void dlg_paiement::Slot_MajusculeCreerNom()
{
    QString nom;
    QLineEdit *UiNom;
    UiNom = ui->TireurChequelineEdit;
    nom = Utils::trimcapitilize(UiNom->text(),false);
    UiNom->setText(nom);
    Slot_EnableOKButton();
}


/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Créer le menu contextuel pour transformer un acte gratuit en acte payant                                                  --------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_ModifGratuit(QPoint pos)
{
    QTableWidgetItem *titem = dynamic_cast<QTableWidgetItem*>(ui->ListeupTableWidget->itemAt(pos));
    if (titem)
    {
        int ro = titem->row();
        if (ui->ListeupTableWidget->item(ro,5)->text() == tr("Gratuit"))
        {
            QMenu           *menuContextuel = new QMenu(this);

            QAction         *pAction_ModifGratuit = menuContextuel->addAction(tr("Ne plus considérer comme gratuit")) ;
            connect (pAction_ModifGratuit, &QAction::triggered,    [=] {ModifGratuitChoixMenu("Modifier");});

            // ouvrir le menu
            menuContextuel->exec(QCursor::pos());
            delete menuContextuel;
        }
        titem = Q_NULLPTR;
        delete titem;
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Transfomer un acte gratuit en acte payant (modifier le montant et supprimer la ligne correspondante de typespaiementActes) -------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::ModifGratuitChoixMenu(QString Choix)
{
    if (Choix == "Modifier")
    {
        QList<QTableWidgetSelectionRange>  RangeeSelectionne = ui->ListeupTableWidget->selectedRanges();
        if (RangeeSelectionne.size() > 0)
        {
            int ab = RangeeSelectionne.at(0).topRow();
            UpLineEdit *LigneMontant = new UpLineEdit();
            LigneMontant->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            QDoubleValidator *val= new QDoubleValidator(this);
            val->setDecimals(2);
            LigneMontant->setValidator(val);
            LigneMontant->setText("0,00");
            LigneMontant->setRowTable(ab);
            LigneMontant->setColumnTable(4);
            ui->ListeupTableWidget->takeItem(ab,4);
            ui->ListeupTableWidget->setCellWidget(ab,4,LigneMontant);
            LigneMontant->installEventFilter(this);
            LigneMontant->setFocus();
            LigneMontant->selectAll();
        }
    }
}

void dlg_paiement::Slot_VoirListeActes()
{
    gMode = VoirListeActes;
    RemplitLesTables(gMode);
    Slot_RegleAffichageFiche();
    ui->ListeupTableWidget->setCurrentCell(0,1);
    ui->ListeupTableWidget->setFocus();
}

void dlg_paiement::Slot_VoirListePaiementsTiers()
{
    gMode = VoirListePaiementsTiers;
    RemplitLesTables(gMode);
    Slot_RegleAffichageFiche();
    ui->ListeupTableWidget->setCurrentCell(0,1);
    ui->ListeupTableWidget->setFocus();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer un nouveau paiement direct -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_RecopieNomTiers(QString b)
{
    if (gMode == EnregistrePaiementTiers && ui->TireurChequelineEdit->isEnabled())
        ui->TireurChequelineEdit->setText(b);
    Slot_EnableOKButton();
}

void dlg_paiement::RegleComptesComboBox(bool ActiveSeult)
{
    QStandardItemModel *model = (ActiveSeult? glistComptesEncaissmt : glistComptesEncaissmtAvecDesactive);
    ui->ComptesupComboBox->clear();
    for (int i=0; i<model->rowCount(); i++)
        ui->ComptesupComboBox->addItem(model->item(i,0)->text(), model->item(i,1)->text());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des différents Widget en fonction du mode de fonctionnement ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_RegleAffichageFiche()
{
    ui->AnnulupPushButton->setText("Annuler");
    ui->ListActesupPushButton->move(10,60);
    ui->NouvPaiementupPushButton->move(10,110);
    ui->NouvTiersupPushButton->move(10,110);
    ui->ListPaiementsTiersupPushButton->move(10,60);
    ui->PasdePaiementlabel->setGeometry(110,20,530,280);

    bool g = false;
    ui->dateEdit->setDate(QDate::currentDate());

    if (gMode == Accueil)
    {
        setFixedWidth(680);
        setFixedHeight(HAUTEUR_MINI);
        ui->Utilisateurlabel    ->setGeometry(10,15,470,20);
        ui->line0               ->setGeometry(10,50,480,15);
        ui->line_4              ->setGeometry(140,70,20,190);
        ui->Buttonsframe        ->setGeometry(490,10,200,250);
        ui->AnnulupPushButton   ->move(10,150);
        ui->UserscomboBox       ->setEnabled(m_userConnected->isSecretaire());

        QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
        for (int n = 0; n <  allRButtons.size(); n++)
        {
            allRButtons.at(n)->setAutoExclusive(false);
            allRButtons.at(n)->setChecked(false);
            allRButtons.at(n)->setAutoExclusive(true);
        }
        ui->Detailsframe                ->setVisible(false);
        ui->line2                       ->setVisible(false);
        ui->ActesEnAttentelabel_3       ->setVisible(false);
        ui->ActesEnAttentelabel         ->setText(tr("Salle d'attente"));
        ui->ActesEnAttentelabel_2       ->setVisible(false);
        ui->SalleDAttenteupTableWidget  ->setVisible(false);
        ui->ListeupTableWidget          ->setVisible(false);
        ui->OrdreupPushButton           ->setVisible(false);
        ui->OKupPushButton              ->setText(tr("Fermer"));
        ui->OKupPushButton              ->setIcon(Icons::icOK());
        ui->OKupPushButton              ->setIconSize(QSize(30,30));
        if (gFicheMode == Direct)
        {
            ui->Utilisateurlabel                ->setText(tr("Gestion des paiements directs"));
            ui->ListActesupPushButton           ->setVisible(true);
            ui->NouvPaiementupPushButton        ->setVisible(true);
            ui->NouvTiersupPushButton           ->setVisible(false);
            ui->ListPaiementsTiersupPushButton  ->setVisible(false);
        }
        else
        {
            ui->Utilisateurlabel                ->setText(tr("Gestion des tiers payants"));
            ui->NouvTiersupPushButton           ->setVisible(true);
            ui->ListPaiementsTiersupPushButton  ->setVisible(true);
            ui->ListActesupPushButton           ->setVisible(false);
            ui->NouvPaiementupPushButton        ->setVisible(false);
        }
        g = true;
        if (SupprimerBouton!=Q_NULLPTR)
                SupprimerBouton ->setVisible(false);
        RegleComptesComboBox();
        ui->ComptesupComboBox               ->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));
    }
    else
    {
        setMinimumSize(QSize(800,950));
        setMaximumSize(QSize(800,950));
        ui->Utilisateurlabel                ->setGeometry(10,15,590,20);
        ui->line0                           ->setGeometry(10,50,600,15);
        ui->line_4                          ->setGeometry(140,70,20,190);
        ui->UserscomboBox                   ->setEnabled(false);
        ui->Detailsframe                    ->setVisible(true);
        ui->line2                           ->setVisible(true);
        ui->ActesEnAttentelabel_3           ->setVisible(true);
        ui->ListeupTableWidget              ->setVisible(true);
        ui->OrdreupPushButton               ->setVisible(true);
        ui->NouvTiersupPushButton           ->setVisible(false);
        ui->ListPaiementsTiersupPushButton  ->setVisible(false);
        ui->ListActesupPushButton           ->setVisible(false);
        ui->NouvPaiementupPushButton        ->setVisible(false);
        ui->OrdreupPushButton               ->move(605,610);
        ui->OrdreupPushButton               ->setIcon(Icons::icTri());
        ui->OrdreupPushButton               ->setIconSize(QSize(20,20));
        ui->Buttonsframe                    ->setGeometry(610,10,190,250);

        switch (gMode) {
        case EnregistrePaiementDirect:
        {
            ui->Utilisateurlabel            ->setText(tr("Gestion des paiements directs - Enregistrer un paiement"));
            ui->OKupPushButton              ->setText(tr("Valider\net fermer"));
            ui->OKupPushButton              ->setIcon(Icons::icOK());
            ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->AnnulupPushButton           ->move(10,150);
            ui->Detailsframe                ->setGeometry(5,280,790,670);
            ui->line2                       ->setGeometry(144,0,636,20);
            ui->DetailupTableWidget         ->setGeometry(0,25,690,160);
            ui->ActesEnAttentelabel         ->setText(tr("Salle d'attente"));
            ui->ActesEnAttentelabel         ->move(0,200);
            ui->ActesEnAttentelabel_2       ->move(0,335);
            ui->ActesEnAttentelabel_2       ->setVisible(true);
            ui->ActesEnAttentelabel_2       ->setText(tr("Actes en attente de paiement"));
            ui->ActesEnAttentelabel_2       ->setVisible(true);
            ui->SalleDAttenteupTableWidget  ->setVisible(true);
            ui->SalleDAttenteupTableWidget  ->setGeometry(0,225,690,100);
            ui->ListeupTableWidget          ->setGeometry(0,360,690,250);
            ui->Totallabel                  ->move(565,190);
            ui->TotallineEdit               ->move(615,190);
            ui->PaiementgroupBox            ->setFocusProxy(ui->CarteCreditradioButton);
            RegleComptesComboBox();
            ui->ComptesupComboBox           ->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));
            break;
        }
        case VoirListeActes:
        {
            ui->Utilisateurlabel            ->setText(tr("Gestion des paiements directs - Tous les actes effectués"));
            ui->OKupPushButton              ->setText(tr("Modifier"));
            ui->OKupPushButton              ->setIcon(Icons::icMarteau());
            ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->AnnulupPushButton           ->move(10,150);
            ui->Detailsframe                ->setGeometry(5,280,790,670);
            ui->line2                       ->setGeometry(144,0,636,20);
            ui->DetailupTableWidget         ->setGeometry(0,25,790,160);
            ui->ActesEnAttentelabel         ->setText(tr("Paiements pour cet acte"));
            ui->ActesEnAttentelabel         ->move(0,190);
            ui->ActesEnAttentelabel_2       ->setVisible(true);
            ui->ActesEnAttentelabel_2       ->move(0,315);
            ui->ActesEnAttentelabel_2       ->setText(tr("Tous les actes ayant reçu un paiement ou en attente de paiement"));
            ui->ActesEnAttentelabel_2       ->setVisible(true);
            ui->SalleDAttenteupTableWidget  ->setVisible(true);
            ui->SalleDAttenteupTableWidget  ->setGeometry(0,210,555,100);
            ui->ListeupTableWidget          ->setGeometry(0,335,790,270);
            ui->Totallabel                  ->move(665,190);
            ui->TotallineEdit               ->move(715,190);
            RegleComptesComboBox(false);
            break;
        }
        case EnregistrePaiementTiers:
        {
            ui->Utilisateurlabel            ->setText(tr("Gestion des tiers payants - Enregistrer un tiers payant"));
            ui->OKupPushButton              ->setText(tr("Valider\net fermer"));
            ui->OKupPushButton              ->setIcon(Icons::icOK());
            ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->AnnulupPushButton           ->move(10,150);
            ui->Detailsframe                ->setGeometry(5,280,790,670);
            ui->line2                       ->setGeometry(144,0,636,20);
            ui->DetailupTableWidget         ->setGeometry(0,25,790,160);
            ui->ActesEnAttentelabel         ->setText(tr("Actes enregistrés en tiers payant en attente de paiement"));
            ui->ActesEnAttentelabel         ->move(0,215);
            ui->ActesEnAttentelabel_2       ->move(0,335);
            ui->ActesEnAttentelabel_2       ->setVisible(false);
            ui->SalleDAttenteupTableWidget  ->setVisible(false);
            ui->ListeupTableWidget          ->setGeometry(0,240,790,370);
            ui->Totallabel                  ->move(665,190);
            ui->TotallineEdit               ->move(715,190);
            ui->PaiementgroupBox            ->setFocusProxy(ui->VirementradioButton);
            RegleComptesComboBox();
            ui->ComptesupComboBox           ->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));
            break;
        }
        case VoirListePaiementsTiers:
        {
            ui->Utilisateurlabel            ->setText(tr("Gestion des tiers payants - Liste des paiements"));
            ui->OKupPushButton              ->setText("Modifier");
            ui->OKupPushButton              ->setIcon(Icons::icMarteau());
            ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->AnnulupPushButton           ->move(10,150);
            ui->Detailsframe                ->setGeometry(5,280,790,670);
            ui->line2                       ->setGeometry(144,0,636,20);
            ui->DetailupTableWidget         ->setGeometry(0,25,790,160);
            ui->ActesEnAttentelabel         ->setText(tr("Tous les paiements enregistrés en tiers payant"));
            ui->ActesEnAttentelabel         ->move(0,215);
            ui->ActesEnAttentelabel_2       ->setVisible(false);
            ui->ActesEnAttentelabel_2       ->setText(tr("Tous les actes ayant reçu un paiement ou en attente de paiement"));
            ui->SalleDAttenteupTableWidget  ->setVisible(false);
            ui->ListeupTableWidget          ->setGeometry(0,240,555,370);
            ui->Totallabel                  ->move(665,190);
            ui->TotallineEdit               ->move(715,190);
            RegleComptesComboBox(false);
            break;
        }
        default:
            break;
        }
    }
    ui->AnnulupPushButton->setVisible(!g);
    ui->TypePaiementframe->setVisible(!g);
    RegleAffichageTypePaiementframe(false);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des widget dans le cadre DetailsPaiement en fonction du mode de fonctionnement et du radiobutton sélectionné ------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_RegleAffichageTypePaiementframe()
{
    RegleAffichageTypePaiementframe(true, true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Retrouve le rang de la ligne selectionnée et modifie les tables en fonction ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_RenvoieRangee(bool Coche)
{

    switch (gMode) {
    case EnregistrePaiementDirect:
    case EnregistrePaiementTiers:
    {
        QTableWidget*           TableOrigine = dynamic_cast<QTableWidget*>(focusWidget());
        if (!TableOrigine) return;
        UpCheckBox* Check = dynamic_cast<UpCheckBox*>(sender());
        if(Check)
        {
            int R = Check->getRowTable();
            if (Check->parent()->parent() == ui->DetailupTableWidget)
                VideDetailsTable(R);
            else
                CompleteDetailsTable(TableOrigine,R,Coche);
        }
        if (ModifPaiementEnCours)
            ui->OKupPushButton->setEnabled(true);
        break;
    }
    case VoirListeActes:
    case VoirListePaiementsTiers:
    {
        QTableWidget*           TableOrigine = dynamic_cast<QTableWidget*>(sender());
        if (!TableOrigine) return;
        QList<QTableWidgetSelectionRange>  RangeeSelectionne = TableOrigine->selectedRanges();
        if (RangeeSelectionne.size() > 0)
        {
            int ab = RangeeSelectionne.at(0).topRow();
            CompleteDetailsTable(TableOrigine, ab,true);
        }
        break;
    }
    default:
        break;
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des widget dans le cadre DetailsPaiement en fonction du mode de fonctionnement et du radiobutton sélectionné ------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_SupprimerPaiement()
{
    UpMessageBox msgbox;
    msgbox.setText(tr("Voulez vous vraiment supprimer les informations de cette écriture?"));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer les informations"));
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != OKBouton)
        return;
    if (AppeleParFichePaiement) reject();
    ModifPaiementEnCours = false;
    RemetToutAZero();
    gMode = Accueil;
    disconnect (ui->ListeupTableWidget,           SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
    disconnect (ui->SalleDAttenteupTableWidget,   SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
    ui->RecImageLabel->setVisible(false);
    Slot_RegleAffichageFiche();
    TraiteparCloseFlag = false;
    ModifLigneRecettePossible = true;
    ui->AnnulupPushButton->setText("Annuler");
    NettoieVerrousCompta();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- ValidupPushButton cliqué en fonction du mode de fonctionnement -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::Slot_ValidePaiement()
{
    QString requete;
    switch (gMode) {
    case Accueil:
    {
        Slot_Annul();
        break;
    }
    case EnregistrePaiementDirect:
    case EnregistrePaiementTiers:
    {
        if (VerifCoherencePaiement())
            switch (EnregistreRecette()) {
            case Impossible:
            case OK:
            {
                if (AppeleParFichePaiement || FermeALaFin) reject();
                ModifPaiementEnCours = false;
                RemetToutAZero();
                gMode = Accueil;
                disconnect (ui->ListeupTableWidget,           SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
                disconnect (ui->SalleDAttenteupTableWidget,   SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
                ui->RecImageLabel->setVisible(false);
                ModifLigneRecettePossible = true;
                Slot_RegleAffichageFiche();
                if (SupprimerBouton!=Q_NULLPTR) SupprimerBouton->setVisible(false);
                ui->AnnulupPushButton->setText("Annuler");
                NettoieVerrousCompta();
                break;
            }
            default:
                break;
            }
       break;
    }
    case VoirListePaiementsTiers:
    case VoirListeActes:
        /* Il s'agit de modifier un paiement dèjà enregistré.
        Plusieurs cas de figure
        .1 Il s'agit d'un acte pour lequel aucune recette n'a été enregistrée (acte enregistré en tiers et n'ayant pas encore été réglé ou acte gratuit ou impayé).
        .2 Il s'agit d'un paiement direct pour lequel une recette a été enregistrée.
        .3 Il s'agit d'un paiement par tiers  pour lequel une recette a été enregistrée.
        */
    {
        NettoieVerrousCompta();
        ModifPaiementEnCours = false;
        int     idActe;
        QString ModePaiement;
        QDate   DateActe;

        ui->PasdePaiementlabel->setVisible(false);

     /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
     //      Cas n°1. il s'agit d'un acte pour lequel aucun versement n'a été enregistré pour le moment (paiement par tiers (CB, CPAM...etc...) sans paiement enregistré)
     -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        if (gMode == VoirListeActes
                && ui->ListeupTableWidget->selectedRanges().size() > 0
                && ui->SalleDAttenteupTableWidget->rowCount() == 0)
        {
            gMode = EnregistrePaiementDirect;
            QList<QTableWidgetSelectionRange>  RangeeSelectionne = ui->ListeupTableWidget->selectedRanges();
                int ab          = RangeeSelectionne.at(0).topRow();
                ModePaiement    = ui->ListeupTableWidget->item(ab,5)->text();
                idActe          = ui->ListeupTableWidget->item(ab,0)->text().toInt();
                DateActe        = QDate::fromString(ui->ListeupTableWidget->item(ab,1)->text(),tr("dd-MM-yyyy"));
            requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(idActe);
            QSqlQuery SupprimTypPaiementActe(requete,db->getDataBase());
            db->traiteErreurRequete(SupprimTypPaiementActe,requete,"");
            gListidActe.clear();
            gListidActe << idActe;
            RemplitLesTables(gMode);
            Slot_RegleAffichageFiche();
            if (ModePaiement == tr("Gratuit"))
                ui->GratuitradioButton->setChecked(true);
            else if (ModePaiement == tr("Carte bancaire"))
                ui->CarteCreditradioButton->setChecked(true);
            else if (ModePaiement == tr("Chèque"))
                ui->ChequeradioButton->setChecked(true);
            else if (ModePaiement != tr("Non enregistré"))
            {
                ui->TiersradioButton->setChecked(true);
                ui->TypeTierscomboBox->setCurrentText(ModePaiement);
            }
            ui->dateEdit->setDate(DateActe);
            RegleAffichageTypePaiementframe(true,false);
            gListidActe.clear();
            ui->OKupPushButton->setEnabled(true);
            ui->AnnulupPushButton->setText(tr("Annuler ce paiement"));
            ui->dateEdit->setFocus();
            ui->dateEdit->setSelectedSection(QDateTimeEdit::DaySection);
            return;
        }

     /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
     //     Cas n°2. Une ligne de recette a été enregistrée
     -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        else
        {
            QTableWidget*   TableOrigine    = Q_NULLPTR;
            // On retrouve l'idRecette de LignesRecettes correspondant au paiement à modifier
            if (gMode == VoirListeActes)
                TableOrigine = ui->SalleDAttenteupTableWidget;
            else
                TableOrigine = ui->ListeupTableWidget;
            QList<QTableWidgetSelectionRange>  RangeeSelectionne = TableOrigine->selectedRanges();
            int ab      = RangeeSelectionne.at(0).topRow();
            idRecette   = TableOrigine->item(ab,0)->text().toInt();
            requete = "SELECT idRecette FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + QString::number(idRecette);
            QSqlQuery ChercheRecetteQuery (requete,db->getDataBase());
            db->traiteErreurRequete(ChercheRecetteQuery,requete,"");
            if (ChercheRecetteQuery.size() == 0)
            {
                UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier ce paiement pour le moment"),
                                     tr("Il est en cours de modification par un autre utilisateur."));
                RemplitLesTables(gMode);
                return;
            }
            requete = "SELECT idActe FROM " NOM_TABLE_LIGNESPAIEMENTS
                    " WHERE idRecette = " + QString::number(idRecette) +
                    " AND idActe IN (SELECT idActe FROM " NOM_TABLE_VERROUCOMPTAACTES " WHERE PosePar != " + QString::number(m_userConnected->id()) + ")";
            QSqlQuery ChercheActesVerrouillesQuery (requete,db->getDataBase());
            db->traiteErreurRequete(ChercheActesVerrouillesQuery,requete,"");
            if (ChercheActesVerrouillesQuery.size() > 0)
            {
                UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier ce paiement pour le moment."),
                                     tr("Certains actes qui le composent sont actuellement verrouillés par d'autres utilisateurs."));
                return;
            }

            // on détermine si le paiement cliqué est un tiers payant ou un paiement direct
            requete =   "SELECT TiersPayant FROM " NOM_TABLE_RECETTES " WHERE TiersPayant = 'O' and idRecette = " + QString::number(idRecette);
            QSqlQuery RetrouveTiersPayantQuery(requete,db->getDataBase());
            if (db->traiteErreurRequete(RetrouveTiersPayantQuery,requete,"")) return;
            if (RetrouveTiersPayantQuery.size() > 0)
                 ModeModif = "ModificationPaiementTiers";
             else
                 ModeModif = "ModificationPaiementDirect";
        }

    /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //      Cas n°3  //il s'agit d'un paiement direct => chèque ou espèces
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        if (ModeModif == "ModificationPaiementDirect")
        {
           // on fait la liste des actes et des montants payés à partir de DetailupTableWidget
            gMode = EnregistrePaiementDirect;
            ModifPaiementEnCours = true;
            ListeActesAModifier.clear();
            MontantActesAModifier.clear();
            for (int i = 0; i < ui->DetailupTableWidget->rowCount(); i++)
            {
                ListeActesAModifier     << ui->DetailupTableWidget->item(i,0)->text().toInt();
                MontantActesAModifier   << QString::number(QLocale().toDouble(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-2)->text()),'f',2);
                if(ui->EspecesradioButton->isChecked())     ModePaiementDirectAModifier = "E";
                if(ui->ChequeradioButton->isChecked())      ModePaiementDirectAModifier = "C";
            }

            requete = "SELECT idRecette, idUser, DatePaiement, DateEnregistrement, Montant, ModePaiement, TireurCheque,"
                      "CompteVirement, BanqueCheque, TiersPayant, NomTiers, Commission, Monnaie, idRemise, EnAttente, EnregistrePar, TypeRecette, datediff(DateEnregistrement,NOW()) as Delai FROM "
                      NOM_TABLE_RECETTES " WHERE idRecette = " + QString::number(idRecette);
            QSqlQuery RetrouveRecetteQuery(requete,db->getDataBase());
            if (db->traiteErreurRequete(RetrouveRecetteQuery,requete,"")) return;

            /* Verifier si on peut modifier la recette - impossible si:
             . la date d'enregistrement remonte à plus de 90 jours
             . c'est un chèque et il a été déposé en banque*/
            RetrouveRecetteQuery.first();
            if (RetrouveRecetteQuery.value(17).toInt() > 90)                                                            //             . la date d'enregistrement remonte à plus de 90 jours
            {
                UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier "
                                             "les données comptables de ce paiement"),
                                             tr("Il a été enregistré il y a plus de 90 jours!"));
                ModifLigneRecettePossible = false;
            }
            else if (RetrouveRecetteQuery.value(5).toString() == "C" && RetrouveRecetteQuery.value(13).toInt() > 0)     //             . c'est un chèque et il a été déposé en banque
            {
                UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier les données comptables de ce paiement"), tr("Le chèque a été déposé en banque!"));
                ModifLigneRecettePossible = false;
            }
            else
            {
                // nettoyer LignesRecetteset le mettre en mémoire au cas où on devrait le restaurer
                LigneRecetteAModifier.clear();
                LigneRecetteAModifier << RetrouveRecetteQuery.value(0).toString();                                              //idRecette
                LigneRecetteAModifier << RetrouveRecetteQuery.value(1).toString();                                              //idUser
                if (RetrouveRecetteQuery.value(2).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(2).toDate().toString("yyyy-MM-dd") + "'";         //DatePaiement
                if (RetrouveRecetteQuery.value(3).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(3).toDate().toString("yyyy-MM-dd") + "'";         //DateEnregistrement
                LigneRecetteAModifier << RetrouveRecetteQuery.value(4).toString();                                              //Montant
                if (RetrouveRecetteQuery.value(5).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(5).toString() + "'";                              //ModePaiement
                if (RetrouveRecetteQuery.value(6).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(6).toString()) + "'";   //TireurCheque
                if (RetrouveRecetteQuery.value(7).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << RetrouveRecetteQuery.value(7).toString() ;                                         //CompteVirement
                if (RetrouveRecetteQuery.value(8).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(8).toString()) + "'";   //BanqueCheque
                if (RetrouveRecetteQuery.value(9).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(9).toString()) + "'";   //TiersPayant
                if (RetrouveRecetteQuery.value(10).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(10).toString()) + "'";  //NomTiers
                if (RetrouveRecetteQuery.value(11).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << RetrouveRecetteQuery.value(11).toString();                                         //Commission
                if (RetrouveRecetteQuery.value(12).toString().isEmpty())
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(12).toString() + "'";                             //Monnaie
                if (RetrouveRecetteQuery.value(13).toInt() == 0)
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << RetrouveRecetteQuery.value(13).toString();                                         //IdRemise
                if (RetrouveRecetteQuery.value(14).toInt() == 0)
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << RetrouveRecetteQuery.value(14).toString();                                         //EnAttente
                if (RetrouveRecetteQuery.value(15).toInt() == 0)
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << RetrouveRecetteQuery.value(15).toString();                                         //EnregistrePar
                if (RetrouveRecetteQuery.value(16).toInt() == 0)
                    LigneRecetteAModifier << "null";
                else
                    LigneRecetteAModifier << RetrouveRecetteQuery.value(16).toString();                                         //TypeRecette
                requete = "DELETE FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + QString::number(idRecette);
                QSqlQuery SupprimLignesRecettesQuery(requete,db->getDataBase());
                db->traiteErreurRequete(SupprimLignesRecettesQuery,requete,"");

                if (SupprimerBouton == Q_NULLPTR)
                {
                    SupprimerBouton = new UpPushButton(ui->Buttonsframe);
                    SupprimerBouton->setText("Supprimer");
                    SupprimerBouton->setIcon(Icons::icPoubelle());
                    SupprimerBouton->setIconSize(QSize(30,30));
                    SupprimerBouton->setGeometry(10,40,180,55);
                    SupprimerBouton->setObjectName("SupprimerupPushButton");
                    connect (SupprimerBouton, SIGNAL(clicked()), this, SLOT (Slot_SupprimerPaiement()));
                }
                SupprimerBouton->setVisible(true);
            }

            //Nettoyer typesPaiementsActes
            // rechercher s'il y a eu plusieurs lignes de paiements pour cet acte et s'il n'y en a qu'une détruire les infos de type paiement actes
            bool GratuitImpayeVisible = true;
            for (int i = 0; i < ListeActesAModifier.size(); i++)
            {
                requete = "SELECT idActe FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idActe = " + QString::number(ListeActesAModifier.at(i));
                QSqlQuery ChercheLignesPaiements (requete,db->getDataBase());
                db->traiteErreurRequete(ChercheLignesPaiements,requete,"");
                if (ChercheLignesPaiements.size() > 1)
                    GratuitImpayeVisible = false;
                if (ChercheLignesPaiements.size() == 1)
                {
                    requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + QString::number(ListeActesAModifier.at(i));
                    QSqlQuery DeleteTypePaiementQuery (requete,db->getDataBase());
                    db->traiteErreurRequete(DeleteTypePaiementQuery,requete,"");
                }
            }

            // Nettoyer LignesPaiements
            requete = "DELETE FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idRecette = " + QString::number(idRecette);
            QSqlQuery SupprimLignesPaiementsQuery(requete,db->getDataBase());
            db->traiteErreurRequete(SupprimLignesPaiementsQuery,requete,"");

            gListidActe = ListeActesAModifier;
            RemplitLesTables(gMode);
            Slot_RegleAffichageFiche();
            RegleAffichageTypePaiementframe(true,true);
            ui->dateEdit->setDate(RetrouveRecetteQuery.value(2).toDate());
            if (!GratuitImpayeVisible)
            {
                ui->GratuitradioButton->setEnabled(false);
                ui->ImpayeradioButton->setEnabled(false);
            }
            for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
            {
                QList<QTableWidgetItem*> items;
                items = ui->DetailupTableWidget->findItems(QString::number(ListeActesAModifier.at(i)),Qt::MatchExactly);
                int ik = items.at(0)->row();
                QString B = QLocale().toString(MontantActesAModifier.at(i).toDouble(),'f',2);
                UpLineEdit* Paye = static_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(ik,ui->DetailupTableWidget->columnCount()-2));
                Paye->setText(B);
            }
            gListidActe.clear();
            ui->AnnulupPushButton->setText(tr("Annuler et\nrétablir l'écriture"));
            ui->OKupPushButton->setEnabled(false);
            if (ModifLigneRecettePossible)
            {
                ui->dateEdit->setFocus();
                ui->dateEdit->setSelectedSection(QDateTimeEdit::DaySection);
            }
            ModeModif   = "";
            return;
        }

     /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
     //      Cas n°4  //il s'agit d'un paiement par tiers => virement, chèque ou espèces
     -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
        if (ModeModif == "ModificationPaiementTiers")
        {
            int idActeAModifier = 0;
            if (gMode == VoirListeActes)
            {
                QList<QTableWidgetSelectionRange>  RangeeSelectionne = ui->ListeupTableWidget->selectedRanges();
                idActeAModifier      = ui->ListeupTableWidget->item(RangeeSelectionne.at(0).topRow(),0)->text().toInt();
            }
            if (gFicheMode == Direct)
            {
                QList<QDialog *> allQDialogs = this->parent()->findChildren<QDialog *>();
                if (allQDialogs.size()>0)
                    for (int n = 0; n <  allQDialogs.size(); n++)
                    {
                        dlg_paiement* QDialogGestionTiers = dynamic_cast<dlg_paiement*>(allQDialogs.at(n));
                        if (QDialogGestionTiers)
                        {
                            if (QDialogGestionTiers->gFicheMode == Tiers)
                            {
                                if (QDialogGestionTiers->gMode == EnregistrePaiementTiers)
                                {
                                    UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier ce paiement pour le moment"), tr("Un autre tiers payant est en cours d'enregistrement"));
                                    return;
                                }
                                else
                                    QDialogGestionTiers->close();
                            }
                        }
                    }
                int Mode = 4;
                ListeActesAModifier.clear();
                ListeActesAModifier << 0;
                dlg_paiement *Dlg_PaimtTiers          = new dlg_paiement(ListeActesAModifier, Mode, proc, idRecette, gidUserACrediter, this);
                if(Dlg_PaimtTiers->getInitOK())
                {
                    Dlg_PaimtTiers->setWindowTitle(tr("Gestion des tiers payants"));
                    Dlg_PaimtTiers->exec();
                    Slot_VoirListeActes();
                    if (idActeAModifier > 0)
                    {
                        QList<QTableWidgetItem*> items;
                        items = ui->ListeupTableWidget->findItems(QString::number(idActeAModifier),Qt::MatchExactly);
                        ui->ListeupTableWidget->setCurrentCell(items.at(0)->row(),1);
                    }
                    Slot_RenvoieRangee();
                }
            }
            else //on est en mode tiers -  déclenche directement ModifPaiementTiers(idRecette)
                ModifPaiementTiers(idRecette);
        }
        else
            ModifPaiementEnCours = false;
        break;
    }
    default:
        break;
    }
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void dlg_paiement::keyPressEvent ( QKeyEvent * event )
{
//    UpMessageBox::Watch(this,focusWidget()->objectName());
        switch (event->key()) {
        case Qt::Key_Tab:
//           UpMessageBox::Watch(this,focusWidget()->objectName());
            break;}
}

void dlg_paiement::closeEvent(QCloseEvent *event)
{
    proc->gsettingsIni->setValue("PositionsFiches/PositionPaiement", saveGeometry());
    event->accept();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiement::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        if (obj->inherits("UpLineEdit"))
        {
            if (obj->parent()->parent() == ui->DetailupTableWidget  && (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers))
            {
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                ValeurAvantChangement = Line->text();
                ValeurMaxi = ui->DetailupTableWidget->item(Line->getRowTable(),Line->getColumnTable()-1)->text();
                Line->selectAll();
            }
        }
    }
    if (event->type() == QEvent::FocusOut)
    {
        if (obj->inherits("UpLineEdit"))
        {
            if (obj->parent()->parent() == ui->DetailupTableWidget  && (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers))
            {
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                if (QLocale().toDouble(Line->text()) > QLocale().toDouble(ValeurMaxi))
                {
                    QSound::play(NOM_ALARME);
                    ValeurAvantChangement = QLocale().toString(QLocale().toDouble(ValeurMaxi),'f',2);   // Montant payé
                }
                else
                    ValeurAvantChangement = QLocale().toString(QLocale().toDouble(Line->text()),'f',2);   // Montant payé
                Line->setText(ValeurAvantChangement);
            }
            if (obj->parent()->parent() == ui->ListeupTableWidget  && gMode == VoirListeActes)
            {
                QTableWidgetItem *pItem1 = new QTableWidgetItem;
                QTableWidgetItem *pItem2 = new QTableWidgetItem;
                pItem1->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                ui->ListeupTableWidget->removeCellWidget(Line->getRowTable(),Line->getColumnTable());
                if (QLocale().toDouble(Line->text()) > 0)
                {
                    QString idActeAMettreAJour;
                    idActeAMettreAJour = ui->ListeupTableWidget->item(Line->getRowTable(),0)->text()
;                   QString requete = "UPDATE " NOM_TABLE_ACTES " SET ActeMontant = '" + Line->text() + "' WHERE idActe = " + idActeAMettreAJour;
                    QSqlQuery MAJActeQuery (requete,db->getDataBase());
                    db->traiteErreurRequete(MAJActeQuery,requete,"");
                    requete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " WHERE idActe = " + idActeAMettreAJour;
                    QSqlQuery MAJTypPaiemtQuery (requete,db->getDataBase());
                    db->traiteErreurRequete(MAJTypPaiemtQuery,requete,"");
                    QString B = QLocale().toString(QLocale().toDouble(Line->text()),'f',2);
                    pItem1->setText(B);
                    ui->ListeupTableWidget->setItem(Line->getRowTable(),Line->getColumnTable(),pItem1);
                    pItem2->setTextAlignment(Qt::AlignCenter);
                    pItem2->setText(tr("Non enregistré"));
                    ui->ListeupTableWidget->setItem(Line->getRowTable(),Line->getColumnTable()+1,pItem2);
                    for (int i = 0; i < ui->ListeupTableWidget->columnCount(); i++)
                        ui->ListeupTableWidget->item(Line->getRowTable(),i)->setForeground(gtextureNoir);
                    CompleteDetailsTable(ui->ListeupTableWidget, Line->getRowTable(),true);
                }
                else
                {
                    pItem1->setText("0,00");
                    pItem1->setForeground(gtextureGris);
                    ui->ListeupTableWidget->setItem(Line->getRowTable(),Line->getColumnTable(),pItem1);
                }
            }
        }
    }
    if (event->type() == QMouseEvent::MouseButtonDblClick)
    {
        if (obj->inherits("UpCheckBox"))
        {
            return true; //permet d'éviter un cafouillage
        }
    }
    if (event->type() == QMouseEvent::MouseButtonPress)
    {
        if (obj->inherits("UpCheckBox"))
        {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                // permet de bloquer le changement d'état du checkbox s'il est verrouillé par un autre utilisateur
                UpCheckBox* CheckBox = static_cast<UpCheckBox*>(obj);
                if (CheckBox->Toggleable())
                {
                    if ((gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
                            && (CheckBox->parent()->parent() == ui->ListeupTableWidget || CheckBox->parent()->parent() == ui->SalleDAttenteupTableWidget)
                            && !CheckBox->isChecked())
                    {
                        QTableWidget *TableAVerifier = static_cast<QTableWidget*>(CheckBox->parent()->parent());
                        if (!VerifVerrouCompta(TableAVerifier,CheckBox->getRowTable()))
                            return true;
                    }
                    CheckBox->setChecked(!CheckBox->isChecked());
                    emit CheckBox->uptoggled(CheckBox->isChecked());
                }
                return true;
            }
        }
    }
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
        if ((keyEvent->key()==Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
                && keyEvent->modifiers() == Qt::NoModifier)
        {
            if (obj->inherits("UpLineEdit"))
            {
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                if (obj->parent()->parent() == ui->DetailupTableWidget  && (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers))
                {
                    if (QLocale().toDouble(Line->text()) > QLocale().toDouble(ValeurMaxi))
                    {
                        QSound::play(NOM_ALARME);
                        ValeurAvantChangement = QLocale().toString(QLocale().toDouble(ValeurMaxi),'f',2);   // Montant payé
                    }
                    else
                        ValeurAvantChangement = QLocale().toString(QLocale().toDouble(Line->text()),'f',2);   // Montant payé
                    Line->setText(ValeurAvantChangement);
                    int AD = ui->DetailupTableWidget->currentRow();
                    if (AD <  ui->DetailupTableWidget->rowCount()-1)          // si on n'est pas sur la dernière rangée
                        ui->DetailupTableWidget->setCurrentCell(AD + 1,5);
                    else                                                    // sinon, on revient à la rangée de départ
                        ui->DetailupTableWidget->setCurrentCell(0,5);
                    return true;
                }
                else
                    ui->ListeupTableWidget->setFocus();
            }
            else if (!obj->inherits("UpPushButton") && obj->objectName() != "dlg_paiement")
            {
                QKeyEvent *newevent = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Tab , Qt::NoModifier);
                QCoreApplication::postEvent (obj, newevent);
                return QWidget::eventFilter(obj, newevent);
            }

        }
        if (keyEvent->key() == Qt::Key_F12)
        {
            Slot_Annul();
            return true;
        }
        if (keyEvent->key()==Qt::Key_Escape)
        {
            if (gMode == Accueil && !TraiteparCloseFlag) reject();
            ui->dateEdit->setFocus();
            if (!TraiteparCloseFlag)
                Slot_Annul();
            TraiteparCloseFlag = false;
            return true;
        }
     }
    if (event->type() == QEvent::Close)
    {
        if (gMode != Accueil)
        {
            event->ignore();
            Slot_Annul();
            TraiteparCloseFlag = true;
            return true;
        }
     }
    return QWidget::eventFilter(obj, event);
}

bool dlg_paiement::getInitOK()
{
    return InitOK;
}

void dlg_paiement::setInitOK(bool init)
{
    InitOK = init;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Définit l'architecture des TableView (SelectionMode, nombre de colonnes, avec Widgets UpcheckBox et UplineBox) ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::DefinitArchitectureTableView(QTableWidget *TableARemplir, int TypeTable)
{
    QStringList         LabelARemplir;
    QString             A;
    int                 ColCount = 0;

    // il faut deconnecter la table du signal itemSelectionChanged(), sinon, l'appel de TableARemplir->clear()
    // provoque un plantage. La table est vidée mais le slot est quand même appelé par le signal
    if (TableARemplir == ui->SalleDAttenteupTableWidget && gMode == VoirListeActes)
        disconnect (TableARemplir,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
    TableARemplir->clear();
    TableARemplir->verticalHeader()->hide();

    // ces 2 lignes sont nécessairs pour éviter un pb d'affichage des tables quand on les redéfinit
    TableARemplir->setRowCount(0);
    TableARemplir->setColumnCount(0);
    
    switch (TypeTable) {
    case Paiements:
    {
        ColCount = 7;
        TableARemplir->setColumnCount(ColCount);
        TableARemplir->setSelectionMode(QAbstractItemView::SingleSelection);

        LabelARemplir << "";
        LabelARemplir << tr("Date");
        LabelARemplir << tr("Payeur");
        LabelARemplir << tr("Mode de paiement");
        LabelARemplir << tr("Enregistré le");
        LabelARemplir << tr("Payé");
        LabelARemplir << "Classementpardate";

        TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
        TableARemplir->horizontalHeader()->setVisible(true);
        int li = 0;                                                                         // Réglage de la largeur et du nombre des colonnes
        TableARemplir->setColumnWidth(li,25);                                               // idActe ou idPaiement
        li++;
        TableARemplir->setColumnWidth(li,90);                                               // Date
        li++;
        TableARemplir->setColumnWidth(li,163);                                              // Payeur
        li++;
        TableARemplir->setColumnWidth(li,130);                                               // Mode de paiement
        li++;
        TableARemplir->setColumnWidth(li,95);                                               // Enregistré le
        li++;
        TableARemplir->setColumnWidth(li,75);                                               // Payé
        li++;
        TableARemplir->setColumnWidth(li,70);                                               // ActeDate ou PaiementDate

        break;
    }
    default:
    {
        if (TableARemplir == ui->DetailupTableWidget)
        {
            ColCount = 9;
            if (gMode != EnregistrePaiementDirect && !(gMode == VoirListeActes && TypeTable == ActesDirects))  ColCount = 10;
            TableARemplir->setColumnCount(ColCount);
            TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);

            LabelARemplir << "";
            LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
            if (gMode != EnregistrePaiementDirect && !(gMode == VoirListeActes && TypeTable == ActesDirects))
            {
                if (TypeTable == ActesTiers)
                    LabelARemplir << tr("Type tiers");
                else
                    LabelARemplir << tr("Mode paiement");
            }
            LabelARemplir << tr("Reste dû");
            if (gMode == EnregistrePaiementDirect)
                LabelARemplir << tr("A payer");
            else
                LabelARemplir << tr("Payé");
            LabelARemplir << "Classementpardate";
            TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
            TableARemplir->horizontalHeader()->setVisible(true);
//----------------------------------------------------------------------------------------------------------// Réglage de la largeur et du nombre des colonnes
            int li = 0;
            TableARemplir->setColumnWidth(li,25);                                               // idActe ou idPaiement
            li++;
            if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
                TableARemplir->setColumnWidth(li,20);                                           // Checkbox
            else
                TableARemplir->setColumnWidth(li,0);                                            // Checkbox
            li++;
            TableARemplir->setColumnWidth(li,90);                                               // Date
            li++;
            TableARemplir->setColumnWidth(li,193);                                              // Nom Prenom ou Payeur
            li++;
            TableARemplir->setColumnWidth(li,160);                                              // Cotation
            li++;
            switch (gMode) {
            case VoirListeActes:
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                if (TypeTable == ActesTiers)                                                        // Type tiers
                {
                    TableARemplir->setColumnWidth(li,120);                                           // Type tiers
                    li ++;
                }
                TableARemplir->setColumnWidth(li,75);                                               // Reste dû
                break;
            }
            case VoirListePaiementsTiers:
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                if (TypeTable == ActesTiers)                                                        // Type tiers
                {
                    TableARemplir->setColumnWidth(li,120);
                    li ++;
                }
                TableARemplir->setColumnWidth(li,75);                                               // Reste dû
                break;
            }
            default:
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                if (TypeTable == ActesTiers)                                                        // Type tiers
                {
                    TableARemplir->setColumnWidth(li,100);
                    li ++;
                }
                if (gMode == VoirListeActes && TypeTable == ActesDirects)                           // Mode de paiement
                {
                    TableARemplir->setColumnWidth(li,120);
                    li ++;
                }
                TableARemplir->setColumnWidth(li,75);                                               // Reste dû
                break;
            }
            }
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // Réglé ou A payer
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // ActeDate
        }

        if (TableARemplir == ui->SalleDAttenteupTableWidget)
        {
            ColCount = 9;
            TableARemplir->setColumnCount(ColCount);
            TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);

            LabelARemplir << "";
            LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
            LabelARemplir << tr("Payé");
            LabelARemplir << tr("Reste dû");
            LabelARemplir << "Classementpardate";
            TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
            TableARemplir->horizontalHeader()->setVisible(true);
            int li = 0;                                                                         // Réglage de la largeur et du nombre des colonnes
            TableARemplir->setColumnWidth(li,25);                                               // idActe ou idPaiement
            li++;
            TableARemplir->setColumnWidth(li,20);                                           // Checkbox
            li++;
            TableARemplir->setColumnWidth(li,90);                                           // Date
            li++;
            TableARemplir->setColumnWidth(li,193);                                              // Nom Prenom ou Payeur
            li++;
            TableARemplir->setColumnWidth(li,160);                                              // Cotation
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // Montant
            TableARemplir->setColumnWidth(li,75);                                               // Payé
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // Reste dû
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // ActeDate
        }

        if (TableARemplir == ui->ListeupTableWidget)
        {
            ColCount = 8;
            if (gMode != EnregistrePaiementDirect)  ColCount = 9;
            if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
                ColCount ++;
            TableARemplir->setColumnCount(ColCount);
            if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
                TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);
            else
                TableARemplir->setSelectionMode(QAbstractItemView::SingleSelection);

            LabelARemplir << "";
            if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
                LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
            if (gMode == EnregistrePaiementTiers)
                LabelARemplir << tr("Type tiers");
            if (gMode == VoirListeActes)
                LabelARemplir << tr("Mode paiement");
            if (gMode == EnregistrePaiementDirect)
                LabelARemplir << tr("Impayé");
            else
                LabelARemplir << tr("Payé");
            LabelARemplir << tr("Reste dû");
            LabelARemplir << "Classementpardate";
            TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
            TableARemplir->horizontalHeader()->setVisible(true);
            int li = 0;                                                                         // Réglage de la largeur et du nombre des colonnes
            TableARemplir->setColumnWidth(li,25);                                               // idActe ou idPaiement
            li++;
            if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
            {
                TableARemplir->setColumnWidth(li,20);                                           // Checkbox
                li++;
                TableARemplir->setColumnWidth(li,90);                                           // Date
            }
            else
                TableARemplir->setColumnWidth(li,90);
            li++;
            TableARemplir->setColumnWidth(li,193);                                              // Nom Prenom ou Payeur
            li++;
            TableARemplir->setColumnWidth(li,160);                                              // Cotation
            li++;
            if (gMode == VoirListeActes)
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                if ((TypeTable == ActesTiers)                                                       // Mode de paiement ou Type tiers
                        || (gMode == VoirListeActes && TypeTable == ActesDirects))
                {
                    TableARemplir->setColumnWidth(li,120);
                    li ++;
                }
                TableARemplir->setColumnWidth(li,75);                                               // Reste dû
            }
            else
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                if (TypeTable == ActesTiers)                                                        // Type tiers
                {
                    TableARemplir->setColumnWidth(li,100);
                    li ++;
                }
                if (gMode == VoirListeActes && TypeTable == ActesDirects)                           // Mode de paiement
                {
                    TableARemplir->setColumnWidth(li,120);
                    li ++;
                }
                TableARemplir->setColumnWidth(li,75);                                               // Impayé
            }
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // Réglé
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // ActeDate
        }
        break;
    }
    }
    TableARemplir->setColumnHidden(0,true);
    TableARemplir->setColumnHidden(ColCount-1,true);
    //TableARemplir->setShowGrid(false);
    TableARemplir->setGridStyle(Qt::SolidLine);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer une recette dans les tables Recettes, LignesPaiements , TypePiaiemntACte et mettre à jour SalleDAttente -----------------------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
int dlg_paiement::EnregistreRecette()
{
    QStringList locklist;
    locklist << NOM_TABLE_RECETTES << NOM_TABLE_LIGNESCOMPTES << NOM_TABLE_PATIENTS <<
                NOM_TABLE_ACTES << NOM_TABLE_DEPENSES << NOM_TABLE_SALLEDATTENTE << NOM_TABLE_RUBRIQUES2035  <<
                NOM_TABLE_LIGNESPAIEMENTS << NOM_TABLE_TYPEPAIEMENTACTES << NOM_TABLE_ARCHIVESBANQUE << NOM_TABLE_USERSCONNECTES;
    if (!db->createtransaction(locklist))
        return Impossible;

    if (((ui->EspecesradioButton->isChecked() || ui->ChequeradioButton->isChecked()) && gMode == EnregistrePaiementDirect)
       ||
        ((ui->EspecesradioButton->isChecked() || ui->VirementradioButton->isChecked() || ui->ChequeradioButton->isChecked()) && gMode == EnregistrePaiementTiers))
    {
        //1.  Mise à jour LignesRecettes ===============================================================================================================================================================================
        if (ModifLigneRecettePossible)
        {
            QString idCompte = "";
            // Mise à jour lignesRecettes
            QString EnregRecetterequete = "INSERT INTO " NOM_TABLE_RECETTES
                    " (idUser, DatePaiement, DateEnregistrement, Montant, ModePaiement, TireurCheque, BanqueCheque, EnAttente, CompteVirement, EnregistrePar, TypeRecette, TiersPayant, NomTiers, Commission) VALUES (";
            EnregRecetterequete +=  QString::number(gidUserACrediter);                                                  // idUser
            EnregRecetterequete +=  ", '" + ui->dateEdit->date().toString("yyyy-MM-dd");                                // DatePaiement
            EnregRecetterequete +=  "', DATE(NOW())";                                                                   // DateEnregistrement
            EnregRecetterequete +=  ", " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text()));              // Montant
            if (ui->ChequeradioButton->isChecked())
            {
                EnregRecetterequete += ",'C";                                                                           // Mode de paiement = chèque
                EnregRecetterequete += "','" + Utils::correctquoteSQL(ui->TireurChequelineEdit->text());               // Tireur chèque
                EnregRecetterequete += "','" + Utils::correctquoteSQL(ui->BanqueChequecomboBox->currentText());        // BanqueCheque
                if (ui->EnAttentecheckBox->isChecked())                                                                 // EnAttente
                    EnregRecetterequete += "',1";
                else
                    EnregRecetterequete += "',null";
                EnregRecetterequete += ",null";                                                                         // CompteVirement
            }
            else if (ui->CarteCreditradioButton->isChecked())                                                           // Mode de paiement = carte de crédit
                EnregRecetterequete += ",'B',null,null,null,null";
            else if (ui->EspecesradioButton->isChecked())                                                               // Mode de paiement = espèces
                EnregRecetterequete += ",'E',null,null,null,null";
            else if (ui->VirementradioButton->isChecked())                                                              // Mode de paiement = virement
            {
                EnregRecetterequete += ",'V',null,null,null";
                idCompte = ui->ComptesupComboBox->currentData().toString();
                EnregRecetterequete += "," + idCompte;
            }

            EnregRecetterequete += "," + QString::number(m_userConnected->id());                                                      // EnregistrePar
            EnregRecetterequete += ",1";                                                                                // TypeRecette
            if (gMode == EnregistrePaiementDirect)                                                                      // TiersPayant
                EnregRecetterequete += ",null";
            else
                EnregRecetterequete += ",'O'";

            QString NomTiers = "";
            switch (gMode) {
            case EnregistrePaiementDirect:{
                if (ui->ChequeradioButton->isChecked())
                    NomTiers = ui->TireurChequelineEdit->text();
                break;
            }
            case EnregistrePaiementTiers:{
                if ((ui->ChequeradioButton->isChecked()) || (ui->VirementradioButton->isChecked()))
                    NomTiers = ui->TierscomboBox->currentText();
                break;
            }
            default:{
                if (ui->DetailupTableWidget->rowCount() > 0)
                {
                    QString ChercheNomPat = "SELECT PatNom FROM " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_ACTES " act"
                                            " WHERE pat.idPat = act.idPat"
                                            " AND act.idActe = " + ui->DetailupTableWidget->item(0,0)->text();
                    QSqlQuery ChercheNomPatQuery (ChercheNomPat, db->getDataBase());
                    if (db->traiteErreurRequete(ChercheNomPatQuery,ChercheNomPat,""))
                    {
                        db->rollback();
                        return Impossible;
                    }
                    ChercheNomPatQuery.first();
                    NomTiers = ChercheNomPatQuery.value(0).toString();
                }
                break;
            }

            }
            EnregRecetterequete += ",'" + Utils::correctquoteSQL(NomTiers);
            if (ui->CommissionlineEdit->text() =="")
                EnregRecetterequete += "',null)";
            else
                EnregRecetterequete += "'," + QString::number(QLocale().toDouble(ui->CommissionlineEdit->text())) +")";
            //proc->Edit(EnregRecetterequete);
            QSqlQuery EnregRecetteQuery (EnregRecetterequete,db->getDataBase());
            if (db->traiteErreurRequete(EnregRecetteQuery,EnregRecetterequete,"Impossible d'enregistrer cette ligne de recette"))
            {
                db->rollback();
                return Impossible;
            }


            QString ChercheMaxrequete = "SELECT Max(idRecette) FROM " NOM_TABLE_RECETTES;
            QSqlQuery ChercheMaxidRecetteQuery (ChercheMaxrequete,db->getDataBase());
            if (db->traiteErreurRequete(ChercheMaxidRecetteQuery, ChercheMaxrequete,""))
            {
                db->rollback();
                return Impossible;
            }
            ChercheMaxidRecetteQuery.first();
            if (ChercheMaxidRecetteQuery.value(0).toInt() > 0)
                idRecette = ChercheMaxidRecetteQuery.value(0).toInt();

            //2. Mise à hour LignesComptes ======================================================================================================================================================
            if (ui->VirementradioButton->isChecked())
            {
                QString InsertComptrequete = "INSERT INTO " NOM_TABLE_LIGNESCOMPTES "(idLigne, idCompte, idRec, LigneDate, LigneLibelle,  LigneMontant, LigneDebitCredit, LigneTypeOperation) VALUES ("
                        + QString::number(proc->getMAXligneBanque()) + "," + idCompte + "," + QString::number(idRecette) + ", '" + ui->dateEdit->date().toString("yyyy-MM-dd")
                        + "', 'Virement créditeur " + Utils::correctquoteSQL(ui->TierscomboBox->currentText()) + "',"
                        + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) + ",1,'Virement créditeur')";
                QSqlQuery CompleteLigneCompteQuery (InsertComptrequete,db->getDataBase());
                if (db->traiteErreurRequete(CompleteLigneCompteQuery,InsertComptrequete,""))
                {
                    db->rollback();
                    return Impossible;
                }

            }
            //3. Mise à hour Depenses  et LignesComptes s'il y a eu une commission sur le virement ==============================================================================================
            if (QLocale().toDouble(ui->CommissionlineEdit->text()) > 0)
            {
                QString SelectMaxrequete = "select max(iddep) + 1 from " NOM_TABLE_DEPENSES;
                QSqlQuery RecupereMaxQuery (SelectMaxrequete,db->getDataBase());
                if (db->traiteErreurRequete(RecupereMaxQuery,SelectMaxrequete,""))
                {
                    db->rollback();
                    return Impossible;
                }
                RecupereMaxQuery.first();
                QString max = RecupereMaxQuery.value(0).toString();

                QString InsertDeprequete = "INSERT INTO " NOM_TABLE_DEPENSES "(iddep, idUser, DateDep, RefFiscale, Objet, Montant, FamFiscale, idRec, ModePaiement,Compte) VALUES (";
                InsertDeprequete += max;                                                                                        // idDep
                InsertDeprequete +=  "," + QString::number(gidUserACrediter);                                                   // idUser
                InsertDeprequete +=  ", '" + ui->dateEdit->date().toString("yyyy-MM-dd");                                       // DateDep
                // on va rechercher l'id2035:
                // si c'est une carte de crédit, l'id2035 correspondra à "frais financiers", sinon, ce sera "honoraires rétrocédés"
                QString intitule2035 = "Honoraires rétrocédés";
                if (ui->TierscomboBox->currentText() == "CB")
                    intitule2035= "Frais financiers";
                InsertDeprequete +=  "', '" + Utils::correctquoteSQL(intitule2035);                                            // RefFiscale
                InsertDeprequete +=  "', 'Commission " + Utils::correctquoteSQL(ui->TierscomboBox->currentText());             // Objet
                InsertDeprequete +=  "', " +  QString::number(QLocale().toDouble(ui->CommissionlineEdit->text()));              // Montant
                QString chercheFamFiscale = "select Famfiscale from " NOM_TABLE_RUBRIQUES2035 " where reffiscale = '" + Utils::correctquoteSQL(intitule2035) +"'";
                QSqlQuery cherchefamfiscalequery (chercheFamFiscale,db->getDataBase());
                if (db->traiteErreurRequete(cherchefamfiscalequery,chercheFamFiscale,""))
                {
                    db->rollback();
                    return Impossible;
                }
                if (cherchefamfiscalequery.size() > 0)
                {
                    cherchefamfiscalequery.first();
                    InsertDeprequete += ", '" + Utils::correctquoteSQL(cherchefamfiscalequery.value(0).toString()) + "'";
                }
                else
                    InsertDeprequete += ",''";                                                                                  // Famfiscale
                InsertDeprequete += ", " + QString::number(idRecette);                                                          // idRec
                InsertDeprequete += ", 'P'";                                                                                    // ModePaiement = P pour prélèvement
                InsertDeprequete += ", " + idCompte + ")";
                QSqlQuery CompleteDepensesQuery (InsertDeprequete,db->getDataBase());
                if (db->traiteErreurRequete(CompleteDepensesQuery,InsertDeprequete,""))
                {
                    db->rollback();
                    return Impossible;
                }
                if (ui->VirementradioButton->isChecked())
                {
                    QString Commission = "Commission";
                    if (ui->TierscomboBox->currentText() == "CB")
                        Commission += " CB";
                    QString InsertComrequete = "INSERT INTO " NOM_TABLE_LIGNESCOMPTES "(idCompte, idDep, idRec, LigneDate, LigneLibelle,  LigneMontant, LigneDebitCredit, LigneTypeOperation) VALUES ("
                            + idCompte + "," + max + "," + QString::number(idRecette) + ", '" + ui->dateEdit->date().toString("yyyy-MM-dd")
                            + "', '" + Commission + "'," + QString::number(QLocale().toDouble(ui->CommissionlineEdit->text())) + ",0,'Prélèvement')";
                    QSqlQuery CompleteLigneCompteQuery (InsertComrequete,db->getDataBase());
                    if (db->traiteErreurRequete(CompleteLigneCompteQuery,InsertComrequete,""))
                    {
                        db->rollback();
                        return Impossible;
                    }
                }
             }
        }
        else
        {
            bool updatelignerecettes = false;
            QString Updaterequete = "UPDATE " NOM_TABLE_RECETTES " SET ";
             if (ui->ChequeradioButton->isChecked())
             {
                 updatelignerecettes = true;
                 Updaterequete += "TireurCheque = '" + Utils::correctquoteSQL(ui->TireurChequelineEdit->text()) + "', ";                   // Tireur chèque
                 Updaterequete += "BanqueCheque = '" + Utils::correctquoteSQL(ui->BanqueChequecomboBox->currentText()) + "'";              // BanqueCheque
             }
            QString NomTiers = "";
            switch (gMode) {
            case EnregistrePaiementDirect:{
                if (ui->ChequeradioButton->isChecked())
                    NomTiers = ui->TireurChequelineEdit->text();
                break;
            }
            case EnregistrePaiementTiers:{
                if ((ui->ChequeradioButton->isChecked()) || (ui->VirementradioButton->isChecked()))
                    NomTiers = ui->TierscomboBox->currentText();
                break;
            }
            default:
                break;
            }
            if (updatelignerecettes && NomTiers != "")
                Updaterequete += ", ";
            if (NomTiers != "")
            {
                Updaterequete += "NomTiers = '" + Utils::correctquoteSQL(NomTiers) + "'";
                updatelignerecettes = true;
            }
            if (updatelignerecettes)
            {
                Updaterequete += " WHERE idRecette = " + QString::number(idRecette);
                //        UpMessageBox::Watch(this,Updaterequete);
                QSqlQuery UpdateRecetteQuery (Updaterequete,db->getDataBase());
                if (db->traiteErreurRequete(UpdateRecetteQuery,Updaterequete,tr("Impossible de mettre à jour cette ligne de recette")))
                {
                    db->rollback();
                    return Impossible;
                }
            }
        }

        //4. Mise à jour LignesPaiements ========================================================================================================================================================
        if (ui->DetailupTableWidget->rowCount() > 0)
        {
            for (int i = 0; i != ui->DetailupTableWidget->rowCount(); i++)
            {
                QString ActeAInserer = ui->DetailupTableWidget->item(i,0)->text();
                QString PayeAInserer = "0";
                int ColonneMontantPaye;
                if (gMode ==EnregistrePaiementDirect)
                    ColonneMontantPaye = 7;
                else
                    ColonneMontantPaye = 8;
                UpLineEdit* Line = dynamic_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ColonneMontantPaye));
                if (Line)
                    PayeAInserer = QString::number(QLocale().toDouble(Line->text()));
                //UpMessageBox::Watch(this,PayeAInserer);
                QString UpdatePmtrequete = "INSERT INTO " NOM_TABLE_LIGNESPAIEMENTS " (idActe,Paye,idRecette) VALUES ("
                        + ActeAInserer + "," + PayeAInserer + "," + QString::number(idRecette) +")";
                //UpMessageBox::Watch(this,UpdatePmtrequete);
                QSqlQuery InsertLignesPaiementsQuery (UpdatePmtrequete,db->getDataBase());
                if (db->traiteErreurRequete(InsertLignesPaiementsQuery,UpdatePmtrequete,tr("Impossible de mettre à jour la table LignesPaiements")))
                {
                    db->rollback();
                    return Impossible;
                }
            }
        }
    }

    //5.  Mise à jour TypepaiementActes ============================================================================================================================================================
    if (gMode == EnregistrePaiementDirect)
    {
        if (ui->GratuitradioButton->isChecked())
        {
            for (int i = 0; i != ui->DetailupTableWidget->rowCount(); i++)
            {
                if (QLocale().toDouble(ui->DetailupTableWidget->item(i,5)->text()) > 0)
                {
                    QString Msg = tr("Vous tentez d'enregistrer comme gratuit\nl'acte concernant ")
                                   + ui->DetailupTableWidget->item(i,3)->text() + "\n"
                                   + tr("alors que son montant est égal à ")
                                   + ui->DetailupTableWidget->item(i,5)->text() + " " + tr("euros") + "\n"
                                   + tr("Forcer cet enregistrement annulera le montant de l'acte!")
                                   + "\n" + tr("Confirmez vous la saisie?");
                    UpMessageBox msgbox;
                    msgbox.setText(tr("Forcer un acte gratuit?"));
                    msgbox.setInformativeText(Msg);
                    msgbox.setIcon(UpMessageBox::Warning);
                    UpSmallButton OKBouton(tr("Forcer l'enregistrement"));
                    UpSmallButton NoBouton(tr("Non"));
                    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
                    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
                    msgbox.exec();
                    if (msgbox.clickedButton() != &OKBouton)
                        return Annul;
                }
            }
            for (int i = 0; i != ui->DetailupTableWidget->rowCount(); i++)
            {
                if (QLocale().toDouble(ui->DetailupTableWidget->item(i,5)->text()) > 0)
                {
                    QString DelPmtrequete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " where idActe = " + ui->DetailupTableWidget->item(i,0)->text();
                    QSqlQuery DeleteTypPaiementQuery (DelPmtrequete,db->getDataBase());
                    if (db->traiteErreurRequete(DeleteTypPaiementQuery,DelPmtrequete,tr("Impossible de supprimer le patient de la table TypePaiementActes")))
                    {
                        db->rollback();
                        return Impossible;
                    }
                    QString InsPmtrequete = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe, TypePaiement) VALUES (" + ui->DetailupTableWidget->item(i,0)->text() + ",'G')";
                    QSqlQuery InsertTypPaiementQuery (InsPmtrequete,db->getDataBase());
                    if (db->traiteErreurRequete(InsertTypPaiementQuery,InsPmtrequete,""))
                    {
                        db->rollback();
                        return Impossible;
                    }
                    QString UpdPmtrequete = "UPDATE " NOM_TABLE_ACTES " SET ActeMontant = 0 WHERE idActe = " + ui->DetailupTableWidget->item(i,0)->text();
                    QSqlQuery UpdateMontantActeQuery (UpdPmtrequete,db->getDataBase());
                    if (db->traiteErreurRequete(UpdateMontantActeQuery,UpdPmtrequete,""))
                    {
                        db->rollback();
                        return Impossible;
                    }
                    ui->DetailupTableWidget->item(i,5)->setText("0,00");
                }
            }
        }

        for (int i = 0; i != ui->DetailupTableWidget->rowCount(); i++)
        {
            QString ActeAInserer = ui->DetailupTableWidget->item(i,0)->text();
            QString Del2Pmtrequete = "DELETE FROM " NOM_TABLE_TYPEPAIEMENTACTES " where idActe = " + ActeAInserer;
            QSqlQuery DeleteTypPaiementQuery (Del2Pmtrequete,db->getDataBase());
            if (db->traiteErreurRequete(DeleteTypPaiementQuery,Del2Pmtrequete,tr("Impossible de supprimer le patient de la table TypePaiementActes")))
            {
                db->rollback();
                return Impossible;
            }
        }
        QString ModePaiement;
        QString TypeTiers = "null";
        if (ui->TiersradioButton->isChecked())
        {
            ModePaiement    = "'T'";
            TypeTiers       = "'" + Utils::correctquoteSQL(ui->TypeTierscomboBox->currentText()) + "'";
        }
        else if (ui->CarteCreditradioButton->isChecked())
        {
            ModePaiement    = "'T'";
            TypeTiers       = "'CB'";
        }
        else if (ui->EspecesradioButton->isChecked())
            ModePaiement = "'E'";
        else if (ui->GratuitradioButton->isChecked())
            ModePaiement = "'G'";
        else if (ui->ChequeradioButton->isChecked())
            ModePaiement = "'C'";
        else if (ui->ImpayeradioButton->isChecked())
            ModePaiement = "'I'";
        for (int i = 0; i != ui->DetailupTableWidget->rowCount(); i++)
        {
            QString ActeAInserer = ui->DetailupTableWidget->item(i,0)->text();
            QString Ins2Pmtrequete = "INSERT INTO " NOM_TABLE_TYPEPAIEMENTACTES " (idActe,TypePaiement,Tiers) VALUES ("
                    + ActeAInserer + "," + ModePaiement + "," + TypeTiers +")";
            QSqlQuery InsertTypePaiementQuery (Ins2Pmtrequete,db->getDataBase());
            if (db->traiteErreurRequete(InsertTypePaiementQuery,Ins2Pmtrequete,tr("Impossible de mettre à jour la table LignesPaiements")))
            {
                db->rollback();
                return Impossible;
            }
        }
    }

    //6. Mise à jour Salle d'attente ==============================================================================================================================================================================
    if (gMode == EnregistrePaiementDirect)
        for (int i = 0; i != ui->DetailupTableWidget->rowCount(); i++)
        {
            QString ActeAInserer = ui->DetailupTableWidget->item(i,0)->text();
            QString DelSDatrequete = "DELETE FROM " NOM_TABLE_SALLEDATTENTE " where idActeAPayer = " + ActeAInserer;
            QSqlQuery DeleteSalDatQuery (DelSDatrequete,db->getDataBase());
            if (db->traiteErreurRequete(DeleteSalDatQuery,DelSDatrequete,tr("Impossible de supprimer le patient de la salle d'attente")))
            {
                db->rollback();
                return Impossible;
            }
        }
    db->commit();
    proc->MAJTcpMsgEtFlagSalDat();
    return OK;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir la DetailsTable en fonction du mode appelé -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::CompleteDetailsTable(QTableWidget *TableSource, int Rangee, bool Coche)
{
    QTableWidgetItem        *pItem1, *pItem2, *pItem3, *pItem4, *pItem5, *pItem6, *pItem7, *pItem8;
    QDoubleValidator        *val;
    QTableWidget            *TableDetails   = ui->DetailupTableWidget;
    QTableWidget            *TableSalDat    = ui->SalleDAttenteupTableWidget;
    QTableWidget            *TableOrigine = TableSource;
    QFontMetrics            fm(qApp->font());
    QString                 requete;

    switch (gMode) {
    case EnregistrePaiementDirect:
    {
        if (Coche)
        {
            int i   = TableDetails->rowCount();
            TableDetails->insertRow(i);
            UpCheckBox *CheckItem = new UpCheckBox();
            pItem1 = new QTableWidgetItem() ;
            pItem2 = new QTableWidgetItem() ;
            pItem3 = new QTableWidgetItem() ;
            pItem4 = new QTableWidgetItem() ;
            pItem5 = new QTableWidgetItem() ;
            pItem6 = new QTableWidgetItem() ;
            pItem7 = new QTableWidgetItem() ;

            pItem1->setText(TableOrigine->item(Rangee,0)->text());//idActe
            CheckItem->setCheckState(Qt::Checked);
            CheckItem->setFocusPolicy(Qt::NoFocus);
            connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
            CheckItem->installEventFilter(this);
            pItem2->setText(TableOrigine->item(Rangee,2)->text());//Date
            pItem3->setText(TableOrigine->item(Rangee,3)->text());//Nom Prenom
            pItem4->setText(TableOrigine->item(Rangee,4)->text());//Cotation
            pItem5->setText(TableOrigine->item(Rangee,5)->text());//Montant
            QString ResteDu = QLocale().toString(QLocale().toDouble(TableOrigine->item(Rangee,7)->text()),'f',2);
            pItem6->setText(ResteDu);//Reste dû
            pItem7->setText(TableOrigine->item(Rangee,8)->text());//Classementpardate

            UpLineEdit *LigneMontant = new UpLineEdit();
            LigneMontant->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            val= new QDoubleValidator(this);
            val->setDecimals(2);
            LigneMontant->setValidator(val);
            LigneMontant->setText(ResteDu);
            LigneMontant->setColumnTable(7);
            LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                   "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
            connect (LigneMontant, SIGNAL(textChanged(QString)), this, SLOT(Slot_CalculTotalDetails()));
            LigneMontant->installEventFilter(this);

            pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
            pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

            TableDetails->setItem(i,0,pItem1);                  //idActe
            TableDetails->setCellWidget(i,1,CheckItem);         //Check
            TableDetails->setItem(i,2,pItem2);                  //Date
            TableDetails->setItem(i,3,pItem3);                  //Nom Prenom
            TableDetails->setItem(i,4,pItem4);                  //Cotation
            TableDetails->setItem(i,5,pItem5);                  //Montant
            TableDetails->setItem(i,6,pItem6);                  //Reste dû
            TableDetails->setCellWidget(i,7,LigneMontant);      //Payé
            TableDetails->setItem(i,8,pItem7);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique

            TableDetails->setRowHeight(i,int(fm.height()*1.1));

            PoseVerrouCompta(TableOrigine->item(Rangee,0)->text().toInt());
         }
        else
        {
            RetireVerrouCompta(TableOrigine->item(Rangee,0)->text().toInt());

            QList<QTableWidgetItem*> items;
            items = TableDetails->findItems(TableOrigine->item(Rangee,0)->text(),Qt::MatchExactly);
            if (!items.isEmpty())
            {
                /* les lignes suivantes sont utiles pour contourner un bug de Qt
                Si on fait un removeRow et qu il y a des cellwidget dans le row,
                et que le nombre de lignes du tablewidget est supérieur à ce que peut afficher le scrollarea
                et que la dernière ligne est visible dans le scrollarea
                ils ne disparaissent pas et passent dans le row suivant
                alors la dernière ligne perd la visibilité de ses cellwidget
                et c'est le foutoir...
                On est obligé de reconstruire complètement la table*/
                TableDetails->removeCellWidget(items.at(0)->row(),1);
                TableDetails->removeCellWidget(items.at(0)->row(),TableDetails->columnCount()-2);
                TableDetails->removeRow(items.at(0)->row());
                QStringList ListeActesARemettreEnDetails, ListeMontantsARemettreEnDetails;
                QStringList ListeDates, ListeNoms, ListeCotations;
                QStringList ListeMontants, ListeResteDu, ListeDatesTri;

                for (int k = 0; k < ui->DetailupTableWidget->rowCount();k++)
                {
                    ListeActesARemettreEnDetails    << ui->DetailupTableWidget->item(k,0)->text();
                    ListeDates                      << ui->DetailupTableWidget->item(k,2)->text();
                    ListeNoms                       << ui->DetailupTableWidget->item(k,3)->text();
                    ListeCotations                  << ui->DetailupTableWidget->item(k,4)->text();
                    ListeMontants                   << ui->DetailupTableWidget->item(k,5)->text();
                    ListeResteDu                    << ui->DetailupTableWidget->item(k,6)->text();
                    UpLineEdit* Line = dynamic_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(k,ui->DetailupTableWidget->columnCount()-2));
                    if (Line)
                        ListeMontantsARemettreEnDetails << Line->text();
                    else
                        ListeMontantsARemettreEnDetails << "0,00";
                    ListeDatesTri                   << ui->DetailupTableWidget->item(k,8)->text();
                }
                int NbLignesAVirer = ui->DetailupTableWidget->rowCount();
                ui->DetailupTableWidget->clearContents();
                ui->DetailupTableWidget->setRowCount(0);
                ui->DetailupTableWidget->setRowCount(ListeActesARemettreEnDetails.size());
                for (int l = 0; l < NbLignesAVirer ;l++)
                {
                    UpCheckBox *CheckItem = new UpCheckBox();
                    pItem1 = new QTableWidgetItem() ;
                    pItem2 = new QTableWidgetItem() ;
                    pItem3 = new QTableWidgetItem() ;
                    pItem4 = new QTableWidgetItem() ;
                    pItem5 = new QTableWidgetItem() ;
                    pItem6 = new QTableWidgetItem() ;
                    pItem7 = new QTableWidgetItem() ;
                    pItem1->setText(ListeActesARemettreEnDetails.at(l));//idActe
                    CheckItem->setCheckState(Qt::Checked);
                    CheckItem->setFocusPolicy(Qt::NoFocus);
                    connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
                    CheckItem->installEventFilter(this);
                    pItem2->setText(ListeDates.at(l));          //Date
                    pItem3->setText(ListeNoms.at(l));           //Nom Prenom
                    pItem4->setText(ListeCotations.at(l));      //Cotation
                    pItem5->setText(ListeMontants.at(l));       //Montant
                    pItem6->setText(ListeResteDu.at(l));        //ResteDu
                    pItem7->setText(ListeDatesTri.at(l));       //Classementpardate

                    pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

                    UpLineEdit *LigneMontant = new UpLineEdit();
                    LigneMontant->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    val = new QDoubleValidator(this);
                    val->setDecimals(2);
                    LigneMontant->setValidator(val);
                    LigneMontant->setText(ListeMontantsARemettreEnDetails.at(l));
                    LigneMontant->setColumnTable(7);
                    LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                           "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                    connect (LigneMontant, SIGNAL(textChanged(QString)), this, SLOT(Slot_CalculTotalDetails()));
                    LigneMontant->installEventFilter(this);

                    TableDetails->setItem(l,0,pItem1);                  //idActe
                    TableDetails->setCellWidget(l,1,CheckItem);         //Check
                    TableDetails->setItem(l,2,pItem2);                  //Date
                    TableDetails->setItem(l,3,pItem3);                  //Nom Prenom
                    TableDetails->setItem(l,4,pItem4);                  //Cotation
                    TableDetails->setItem(l,5,pItem5);                  //Montant
                    TableDetails->setItem(l,6,pItem6);                  //Reste dû
                    TableDetails->setCellWidget(l,7,LigneMontant);      //Payé
                    TableDetails->setItem(l,8,pItem7);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique

                    TableDetails->setRowHeight(l,int(fm.height()*1.1));
                }
            }
        }
        TrieListe(TableDetails);
        break;
    }
    case EnregistrePaiementTiers:
    {
        if (Coche)
        {
            int i   = 0;//TableDetails->rowCount();
            TableDetails->insertRow(0);
            UpCheckBox *CheckItem = new UpCheckBox();
            pItem1 = new QTableWidgetItem() ;
            pItem2 = new QTableWidgetItem() ;
            pItem3 = new QTableWidgetItem() ;
            pItem4 = new QTableWidgetItem() ;
            pItem5 = new QTableWidgetItem() ;
            pItem6 = new QTableWidgetItem() ;
            pItem7 = new QTableWidgetItem() ;
            pItem8 = new QTableWidgetItem() ;

            pItem1->setText(TableOrigine->item(Rangee,0)->text());//idActe
            CheckItem->setCheckState(Qt::Checked);
            CheckItem->setFocusPolicy(Qt::NoFocus);
            connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
            CheckItem->installEventFilter(this);
            pItem2->setText(TableOrigine->item(Rangee,2)->text());//Date
            pItem3->setText(TableOrigine->item(Rangee,3)->text());//Nom Prenom
            pItem4->setText(TableOrigine->item(Rangee,4)->text());//Cotation
            pItem5->setText(TableOrigine->item(Rangee,5)->text());//Montant
            pItem8->setText(TableOrigine->item(Rangee,6)->text()); //Type tiers
            pItem8->setTextAlignment(Qt::AlignCenter);
            pItem7->setText(TableOrigine->item(Rangee,9)->text());//Classementpardate

            QString ResteDu = QLocale().toString(QLocale().toDouble(TableOrigine->item(Rangee,8)->text()),'f',2);

            pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
            pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
            pItem6->setText(ResteDu);

            UpLineEdit *LigneMontant = new UpLineEdit();
            LigneMontant->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            val= new QDoubleValidator(this);
            val->setDecimals(2);
            LigneMontant->setValidator(val);
            LigneMontant->setText(ResteDu);
            LigneMontant->setColumnTable(8);
            connect (LigneMontant, SIGNAL(textChanged(QString)), this, SLOT(Slot_CalculTotalDetails()));
            LigneMontant->installEventFilter(this);

            TableDetails->setItem(i,0,pItem1);                  //idActe
            TableDetails->setCellWidget(i,1,CheckItem);         //Check
            TableDetails->setItem(i,2,pItem2);                  //Date
            TableDetails->setItem(i,3,pItem3);                  //Nom Prenom
            TableDetails->setItem(i,4,pItem4);                  //Cotation
            TableDetails->setItem(i,5,pItem5);                  //Montant
            TableDetails->setItem(i,6,pItem8);                  //Type tiers
            TableDetails->setItem(i,7,pItem6);                  //Reste dû
            TableDetails->setCellWidget(i,8,LigneMontant);      //Payé
            TableDetails->setItem(i,9,pItem7);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique

            TableDetails->setRowHeight(i,int(fm.height()*1.1));

            PoseVerrouCompta(TableOrigine->item(Rangee,0)->text().toInt());
         }
        else
        {
            RetireVerrouCompta(TableOrigine->item(Rangee,0)->text().toInt());

            QList<QTableWidgetItem*> items;
            items = TableDetails->findItems(TableOrigine->item(Rangee,0)->text(),Qt::MatchExactly);
            if (!items.isEmpty())
            {
                /* les lignes suivantes sont utiles pour contourner un bug de Qt
                Si on fait un removeRow et qu il y a des cellwidget dans le row,
                et que le nombre de lignes du tablewidget est supérieur à ce que peut afficher le scrollarea
                et que la dernière ligne est visible dans le scrollarea
                ils ne disparaissent pas et passent dans le row suivant
                alors la dernière ligne perd la visibilité de ses cellwidget
                et c'est le foutoir...
                On est obligé de reconstruire complètement la table*/
                TableDetails->removeCellWidget(items.at(0)->row(),1);
                TableDetails->removeCellWidget(items.at(0)->row(),TableDetails->columnCount()-2);
                TableDetails->removeRow(items.at(0)->row());
                QStringList ListeActesARemettreEnDetails, ListeMontantsARemettreEnDetails;
                QStringList ListeDates, ListeNoms, ListeCotations;
                QStringList ListeMontants, ListeTypesTiers, ListeResteDu, ListeDatesTri;

                for (int k = 0; k < ui->DetailupTableWidget->rowCount();k++)
                {
                    ListeActesARemettreEnDetails    << ui->DetailupTableWidget->item(k,0)->text();
                    ListeDates                      << ui->DetailupTableWidget->item(k,2)->text();
                    ListeNoms                       << ui->DetailupTableWidget->item(k,3)->text();
                    ListeCotations                  << ui->DetailupTableWidget->item(k,4)->text();
                    ListeMontants                   << ui->DetailupTableWidget->item(k,5)->text();
                    ListeTypesTiers                 << ui->DetailupTableWidget->item(k,6)->text();
                    ListeResteDu                    << ui->DetailupTableWidget->item(k,7)->text();
                    UpLineEdit* Line = dynamic_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(k,ui->DetailupTableWidget->columnCount()-2));
                    if (Line)
                        ListeMontantsARemettreEnDetails << Line->text();
                    else
                        ListeMontantsARemettreEnDetails << "0,00";
                    ListeDatesTri                   << ui->DetailupTableWidget->item(k,9)->text();
                }
                int NbLignesAVirer = ui->DetailupTableWidget->rowCount();
                ui->DetailupTableWidget->clearContents();
                ui->DetailupTableWidget->setRowCount(0);
                ui->DetailupTableWidget->setRowCount(ListeActesARemettreEnDetails.size());
                for (int l = 0; l < NbLignesAVirer ;l++)
                {
                    UpCheckBox *CheckItem = new UpCheckBox();
                    pItem1 = new QTableWidgetItem() ;
                    pItem2 = new QTableWidgetItem() ;
                    pItem3 = new QTableWidgetItem() ;
                    pItem4 = new QTableWidgetItem() ;
                    pItem5 = new QTableWidgetItem() ;
                    pItem6 = new QTableWidgetItem() ;
                    pItem7 = new QTableWidgetItem() ;
                    pItem8 = new QTableWidgetItem() ;
                    pItem1->setText(ListeActesARemettreEnDetails.at(l));//idActe
                    CheckItem->setCheckState(Qt::Checked);
                    CheckItem->setFocusPolicy(Qt::NoFocus);
                    connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
                    CheckItem->installEventFilter(this);
                    pItem2->setText(ListeDates.at(l));          //Date
                    pItem3->setText(ListeNoms.at(l));           //Nom Prenom
                    pItem4->setText(ListeCotations.at(l));      //Cotation
                    pItem5->setText(ListeMontants.at(l));       //Montant
                    pItem6->setText(ListeTypesTiers.at(l));     //Type tiers
                    pItem7->setText(ListeResteDu.at(l));        //ResteDu
                    pItem8->setText(ListeDatesTri.at(l));       //Classementpardate

                    pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    pItem6->setTextAlignment(Qt::AlignCenter);
                    pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

                    UpLineEdit *LigneMontant = new UpLineEdit();
                    LigneMontant->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    val= new QDoubleValidator(this);
                    val->setDecimals(2);
                    LigneMontant->setValidator(val);
                    LigneMontant->setText(ListeMontantsARemettreEnDetails.at(l));
                    LigneMontant->setColumnTable(8);
                    LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                           "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                    connect (LigneMontant, SIGNAL(textChanged(QString)), this, SLOT(Slot_CalculTotalDetails()));
                    LigneMontant->installEventFilter(this);

                    TableDetails->setItem(l,0,pItem1);                  //idActe
                    TableDetails->setCellWidget(l,1,CheckItem);         //Check
                    TableDetails->setItem(l,2,pItem2);                  //Date
                    TableDetails->setItem(l,3,pItem3);                  //Nom Prenom
                    TableDetails->setItem(l,4,pItem4);                  //Cotation
                    TableDetails->setItem(l,5,pItem5);                  //Montant
                    TableDetails->setItem(l,6,pItem6);                  //Type tiers
                    TableDetails->setItem(l,7,pItem7);                  //Reste dû
                    TableDetails->setCellWidget(l,8,LigneMontant);      //Payé
                    TableDetails->setItem(l,9,pItem8);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique

                    TableDetails->setRowHeight(l,int(fm.height()*1.1));
                }
            }
        }
        TrieListe(TableDetails);
        break;
    }
    case VoirListeActes:
    {
        ui->DetailupTableWidget->clearContents();
        ui->DetailupTableWidget->setRowCount(0);
        if (TableOrigine == ui->ListeupTableWidget)
        {
            // Remplir la table SalDat avec les Paiements correspondants à l'acte mis en surbrillance dans la table liste
            QString TextidActe   = TableOrigine->item(Rangee,0)->text();
            requete =     "SELECT rec.idRecette, DatePaiement, DateEnregistrement, rec.Montant, ModePaiement, TireurCheque, CompteVirement, BanqueCheque, TiersPayant, NomTiers, Commission, Monnaie, idRemise, EnAttente, EnregistrePar, TypeRecette, RCDate, Paye\n"
                          " FROM " NOM_TABLE_LIGNESPAIEMENTS " lig, " NOM_TABLE_RECETTES " rec\n"
                          " LEFT OUTER JOIN " NOM_TABLE_REMISECHEQUES " AS rc ON rc.idRemCheq = rec.idRemise\n"
                          " WHERE idUser = " + QString::number(gidUserACrediter) + "\n"
                          " AND lig.idRecette = rec.idRecette\n"
                          " AND lig.idActe = " + TextidActe + "\n"
                          " ORDER BY DatePaiement DESC, NomTiers";
            QSqlQuery ChercheLignesRecettesQuery (requete,db->getDataBase());
            //UpMessageBox::Watch(this,TextidActe + "\n" +requete);

            db->traiteErreurRequete(ChercheLignesRecettesQuery,requete,"");
            RemplirTableWidget(TableSalDat,"Paiements",ChercheLignesRecettesQuery,false,Qt::Unchecked);
            if (ChercheLignesRecettesQuery.size() == 0)
            {
                ui->PasdePaiementlabel->setVisible(true);
                ui->PaiementgroupBox->setVisible(false);
                break;
            }
            else
            {
                ui->PasdePaiementlabel->setVisible(false);
                ui->TypePaiementframe->setVisible(true);
                ui->PaiementgroupBox->setVisible(true);
                if (ChercheLignesRecettesQuery.size() > 0)
                    connect (TableSalDat,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
                QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
                for (int n = 0; n <  allRButtons.size(); n++)
                {
                    allRButtons.at(n)->setAutoExclusive(false);
                    allRButtons.at(n)->setChecked(false);
                    allRButtons.at(n)->setAutoExclusive(true);
                }
                ui->MontantlineEdit->setText("0,00");
                ui->TierscomboBox->clearEditText();
                ui->BanqueChequecomboBox->clearEditText();
                ui->TireurChequelineEdit->clear();
                ui->EnAttentecheckBox->setCheckState(Qt::Unchecked);
                ui->ComptesupComboBox->clearEditText();
                ui->CommissionlineEdit->clear();
                ui->dateEdit->setDate(QDate::currentDate());
                ui->TotallineEdit->setText("0,00");

                if (ChercheLignesRecettesQuery.size() != 1)
                    break;
                else
                {
                    Rangee = 0;
                    TableOrigine = ui->SalleDAttenteupTableWidget;
                    disconnect (ui->SalleDAttenteupTableWidget,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
                    TableOrigine->setRangeSelected(QTableWidgetSelectionRange(0,0,0,TableOrigine->columnCount()-1),true);
                    connect (ui->SalleDAttenteupTableWidget,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
                }
            }
         }
        // la table d'origine est SalleDAttentetetableWidget ou (c'est ListeupTableWidget et il n'y a qu'un seul enregistrement dans SalleDAttenteupTableWidget)
        // on détermine si le paiement cliqué est un tiers payant ou un paiement direct
        QString TextidRecette   = TableOrigine->item(Rangee,0)->text();
        requete =   "SELECT TiersPayant FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + TextidRecette;
        QSqlQuery RetrouveTiersPayantQuery(requete,db->getDataBase());
        if (db->traiteErreurRequete(RetrouveTiersPayantQuery,requete,"")) return;
        if (RetrouveTiersPayantQuery.size() > 0)
        {
            RetrouveTiersPayantQuery.first();
            if (RetrouveTiersPayantQuery.value(0).toString() == "O")
            {
                DefinitArchitectureTableView(ui->DetailupTableWidget,ActesTiers);
                ui->DetailupTableWidget->setGeometry(0,25,790,160);
                ui->Totallabel->move(665,190);
                ui->TotallineEdit->move(715,190);
                gTypeTable = ActesTiers;
            }
            else
            {
                DefinitArchitectureTableView(ui->DetailupTableWidget,ActesDirects);
                ui->DetailupTableWidget->setGeometry(0,25,670,160);
                ui->Totallabel->move(465,190);
                ui->TotallineEdit->move(595,190);
                gTypeTable = ActesDirects;
            }
        }
        [[clang::fallthrough]];/* => pas de break, on continue avec le code de VoirListePaiementsTiers */
        //no break
    }
    case VoirListePaiementsTiers:      // on reconstruit la liste des actes inclus dans le paiement en surbrillance
    {
        ui->DetailupTableWidget->clearContents();
        ui->DetailupTableWidget->setRowCount(0);
        QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
        for (int n = 0; n <  allRButtons.size(); n++)
        {
            allRButtons.at(n)->setAutoExclusive(false);
            allRButtons.at(n)->setChecked(false);
            allRButtons.at(n)->setAutoExclusive(true);
        }
        if (gMode == VoirListePaiementsTiers) gTypeTable = ActesTiers;

        QString TextidRecette   = TableOrigine->item(Rangee,0)->text();

        requete =   "SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, Paye, TypePaiement, Tiers, TotalPaye\n"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_LIGNESPAIEMENTS " lig, " NOM_TABLE_TYPEPAIEMENTACTES " typ,\n"
                    " (SELECT lig.idActe, SUM(lig.paye) as TotalPaye FROM " NOM_TABLE_LIGNESPAIEMENTS " lig,\n"
                    " (SELECT idActe FROM " NOM_TABLE_LIGNESPAIEMENTS
                    " WHERE idRecette = " + TextidRecette + ") AS Result\n"
                    " WHERE lig.idActe = Result.idActe GROUP BY lig.idActe) AS calc\n"
                    " WHERE act.idActe = lig.idActe\n"
                    " AND typ.idActe = act.idActe\n"
                    " AND calc.idActe = act.idActe\n"
                    " AND lig.idRecette = " + TextidRecette + "\n"
                    " AND act.idPat = pat.idPat\n"
                    " ORDER BY ActeDate DESC, PatNom, PatPrenom";

        //UpMessageBox::Watch(this,requete);
        QSqlQuery ChercheDetailsPaiementQuery (requete,db->getDataBase());
        db->traiteErreurRequete(ChercheDetailsPaiementQuery,requete,"");

        RemplirTableWidget(TableDetails,"Actes",ChercheDetailsPaiementQuery,false,Qt::Unchecked);

        // Remplir les infos sur la recette concernée
        requete =   "SELECT idRecette, idUser, DatePaiement, DateEnregistrement, Montant, ModePaiement, TireurCheque, CompteVirement, BanqueCheque, TiersPayant, NomTiers, Commission, Monnaie, idRemise, EnAttente, EnregistrePar, TypeRecette FROM " NOM_TABLE_RECETTES
                    " WHERE idRecette = " + TextidRecette;
        QSqlQuery ChercheDetailsRecetteQuery (requete,db->getDataBase());
        db->traiteErreurRequete(ChercheDetailsRecetteQuery,requete,"");
        ChercheDetailsRecetteQuery.first();
        ui->dateEdit->setDate(ChercheDetailsRecetteQuery.value(2).toDate());
        QRadioButton *RadioAClicker = Q_NULLPTR;
        QString mp = ChercheDetailsRecetteQuery.value(5).toString();
        if (mp == "V")
        {
            if (ChercheDetailsRecetteQuery.value(9).toString() ==  "O" && ChercheDetailsRecetteQuery.value(10).toString() == "CB" && gMode == VoirListeActes)
                RadioAClicker = ui->CarteCreditradioButton;
            else
                RadioAClicker = ui->VirementradioButton;
            QString Commission = QLocale().toString(ChercheDetailsRecetteQuery.value(11).toDouble(),'f',2);
            ui->CommissionlineEdit->setText(Commission);
        }
        if (mp == "E") RadioAClicker = ui->EspecesradioButton;
        if (mp == "C") RadioAClicker = ui->ChequeradioButton;
        if (RadioAClicker != Q_NULLPTR)
            RadioAClicker->setChecked(true);
        ui->ComptesupComboBox->clearEditText();
        if (ChercheDetailsRecetteQuery.value(7).toString() != "0")
        {
            ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(ChercheDetailsRecetteQuery.value(7)));
            //qDebug() << ChercheDetailsRecetteQuery.value(7).toString() + " - " + ui->ComptesupComboBox->currentData().toString() + " - " + ui->ComptesupComboBox->currentText();
        }
        ui->TierscomboBox->setCurrentText(ChercheDetailsRecetteQuery.value(10).toString());
        if (mp == "C")
        {
            ui->EnAttentecheckBox->setChecked(ChercheDetailsRecetteQuery.value(14).toString() == "1");
            ui->TireurChequelineEdit->setText(ChercheDetailsRecetteQuery.value(6).toString());
            ui->BanqueChequecomboBox->setCurrentText(ChercheDetailsRecetteQuery.value(8).toString());
        }
        QString Montant = QLocale().toString(ChercheDetailsRecetteQuery.value(4).toDouble(),'f',2);
        ui->MontantlineEdit->setText(Montant);
        break;
    }
    } // fin switch
    Slot_CalculTotalDetails();
    RegleAffichageTypePaiementframe(false);
    RegleAffichageTypePaiementframe(true,false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Modifie un paiement par tiers ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::ModifPaiementTiers(int idRecetteAModifier)

{
    gFicheMode= Tiers;
    ModifPaiementEnCours    = true;
    gMode                   = EnregistrePaiementTiers;
    idRecette               = idRecetteAModifier;
    QString                 requete;
    ListeActesAModifier.clear();
    MontantActesAModifier.clear();

    // on fait la liste des actes et des montants payés de la recette à modifier
    QString Retrouverequete = "SELECT idActe, Paye FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idRecette = " + QString::number(idRecette);
    QSqlQuery RetrouveLignesPaiementsQuery (Retrouverequete,db->getDataBase());
    db->traiteErreurRequete(RetrouveLignesPaiementsQuery,Retrouverequete,"");
    RetrouveLignesPaiementsQuery.first();
    for (int i = 0; i < RetrouveLignesPaiementsQuery.size(); i++)
    {
        ListeActesAModifier     << RetrouveLignesPaiementsQuery.value(0).toInt();
        MontantActesAModifier   << RetrouveLignesPaiementsQuery.value(1).toString();
        RetrouveLignesPaiementsQuery.next();
    }


    QString RetrouveRecetterequete = "SELECT idRecette, idUser, DatePaiement, DateEnregistrement, Montant,"                 // 0, 1, 2, 3, 4
                                     " ModePaiement, TireurCheque, CompteVirement, BanqueCheque, TiersPayant,"              // 5, 6, 7, 8 ,9
                                     " NomTiers, Commission, Monnaie, idRemise, EnAttente,"                                 // 10, 11, 12, 13, 14
                                     " EnregistrePar, TypeRecette, datediff(DateEnregistrement,NOW()) as Delai"             // 15, 16, 17
                                     " FROM " NOM_TABLE_RECETTES
                                     " WHERE idRecette = " + QString::number(idRecette);
    QSqlQuery RetrouveRecetteQuery(RetrouveRecetterequete,db->getDataBase());
    if (db->traiteErreurRequete(RetrouveRecetteQuery,RetrouveRecetterequete,"")) return;
    RetrouveRecetteQuery.first();

    // Remplir les infos sur la recette concernée
    ui->dateEdit->setDate(RetrouveRecetteQuery.value(2).toDate());                                                          // DatePaiement
    QRadioButton *RadioAClicker = Q_NULLPTR;
    QString mp = RetrouveRecetteQuery.value(5).toString();                                                                  // ModePaiement
    if (mp == "V")
    {
            RadioAClicker = ui->VirementradioButton;
            QString Commission = QLocale().toString(RetrouveRecetteQuery.value(11).toDouble(),'f',2);
            ui->CommissionlineEdit->setText(Commission);
    }
    if (mp == "E") RadioAClicker = ui->EspecesradioButton;
    if (mp == "C") RadioAClicker = ui->ChequeradioButton;
    if (RadioAClicker != Q_NULLPTR)
        RadioAClicker->setChecked(true);
    if (!RetrouveRecetteQuery.value(7).isNull())                                                                            // CompteVirement
        ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(RetrouveRecetteQuery.value(7)));
    ui->TierscomboBox->setCurrentText(RetrouveRecetteQuery.value(10).toString());                                           //  TiersPayant
    if (mp == "C")
    {
        ui->EnAttentecheckBox->setChecked(RetrouveRecetteQuery.value(14).toString() == "1");                                // EnAttente
        ui->TireurChequelineEdit->setText(RetrouveRecetteQuery.value(6).toString());                                        // TireurCheque
        ui->BanqueChequecomboBox->setCurrentText(RetrouveRecetteQuery.value(8).toString());                                 // BanqueCheque
    }
    QString Montant = QLocale().toString(RetrouveRecetteQuery.value(4).toDouble(),'f',2);                                   // Montant
    ui->MontantlineEdit->setText(Montant);

    /* Verifier si on peut modifier la recette - impossible si:
 . la date d'enregistrement remonte à plus de 90 jours
 . c'est un chèque et il a été déposé en banque
 . c'est un virement et il a été pointé sur le compte*/
    if (RetrouveRecetteQuery.value(17).toInt() > 90)                                                            //             . la date d'enregistrement remonte à plus de 90 jours
    {
        UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier les données comptables de ce paiement"),
                                  tr("Il a été enregistré il y a plus de 90 jours!"));
        ModifLigneRecettePossible = false;
    }
    else if (RetrouveRecetteQuery.value(5).toString() == "C" && RetrouveRecetteQuery.value(13).toInt() > 0)     //             . c'est un chèque et il a été déposé en banque

    {
        UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier les données comptables de ce paiement"),
                                tr("Le chèque a été déposé en banque!"));
        ModifLigneRecettePossible = false;
    }
    else
    {
        bool Consolide = false;
        requete = "SELECT idLigne, idCompte, idDep, idRec, idRecSpec,"                      // 0, 1, 2, 3, 4
                  " idRemCheq, LigneDate, LigneLibelle, LigneMontant, LigneDebitCredit,"    // 5, 6, 7, 8, 9
                  " LigneTypeOperation, LigneConsolide"                                     // 10, 11
                  " FROM " NOM_TABLE_LIGNESCOMPTES
                  " WHERE idRec = " + QString::number(idRecette) + " and LigneDebitCredit = 1";
        QSqlQuery ChercheVirementCompteQuery (requete,db->getDataBase());
        db->traiteErreurRequete(ChercheVirementCompteQuery,requete,"");
        if (ChercheVirementCompteQuery.size() > 0)
        {
            ChercheVirementCompteQuery.first();
            if (ChercheVirementCompteQuery.value(11).toInt() == 1)
                Consolide = true;
        }
        if (!Consolide)
        {
            requete = "SELECT idLigne FROM " NOM_TABLE_ARCHIVESBANQUE " WHERE idRec = " + QString::number(idRecette);
            QSqlQuery ChercheVirementArchiveQuery (requete,db->getDataBase());
            db->traiteErreurRequete(ChercheVirementArchiveQuery,requete,"");
            if (ChercheVirementArchiveQuery.size() > 0)
                Consolide = true;
        }
        if (Consolide)
        {
            UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier les données comptables de ce paiement"),
                                     tr("Le virement a été enregistré en banque!"));
            ModifLigneRecettePossible = false;
        }
        else
        {
            // nettoyer LinesRecettes et le mettre en mémoire au cas où on devrait le restaurer
            LigneRecetteAModifier.clear();
            LigneRecetteAModifier << RetrouveRecetteQuery.value(0).toString();                                              //idRecette
            LigneRecetteAModifier << RetrouveRecetteQuery.value(1).toString();                                              //idUser
            if (RetrouveRecetteQuery.value(2).toString().isEmpty())                                                         //DatePaiement
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(2).toDate().toString("yyyy-MM-dd") + "'";
            if (RetrouveRecetteQuery.value(3).toString().isEmpty())                                                         //DateEnregistrement
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(3).toDate().toString("yyyy-MM-dd") + "'";
            LigneRecetteAModifier << RetrouveRecetteQuery.value(4).toString();                                              //Montant
            if (RetrouveRecetteQuery.value(5).toString().isEmpty())                                                         //ModePaiement
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(5).toString() + "'";
            if (RetrouveRecetteQuery.value(6).toString().isEmpty())                                                         //TireurCheque
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(6).toString()) + "'";
            if (RetrouveRecetteQuery.value(7).toString().isEmpty())                                                         //CompteVirement
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << RetrouveRecetteQuery.value(7).toString() ;
            if (RetrouveRecetteQuery.value(8).toString().isEmpty())                                                         //BanqueCheque
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(8).toString()) + "'";
            if (RetrouveRecetteQuery.value(9).toString().isEmpty())                                                         //TiersPayant
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(9).toString()) + "'";
            if (RetrouveRecetteQuery.value(10).toString().isEmpty())                                                        //NomTiers
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + Utils::correctquoteSQL(RetrouveRecetteQuery.value(10).toString()) + "'";
            if (RetrouveRecetteQuery.value(11).toString().isEmpty())                                                        //Commission
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << RetrouveRecetteQuery.value(11).toString();
            if (RetrouveRecetteQuery.value(12).toString().isEmpty())                                                        //Monnaie
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << "'" + RetrouveRecetteQuery.value(12).toString() + "'";
            if (RetrouveRecetteQuery.value(13).toInt() == 0)                                                                //IdRemise
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << RetrouveRecetteQuery.value(13).toString();
            if (RetrouveRecetteQuery.value(14).toInt() == 0)                                                                //EnAttente
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << RetrouveRecetteQuery.value(14).toString();
            if (RetrouveRecetteQuery.value(15).toInt() == 0)                                                                //EnregistrePar
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << RetrouveRecetteQuery.value(15).toString();
            if (RetrouveRecetteQuery.value(16).toInt() == 0)                                                                //TypeRecette
                LigneRecetteAModifier << "null";
            else
                LigneRecetteAModifier << RetrouveRecetteQuery.value(16).toString();
            requete = "DELETE FROM " NOM_TABLE_RECETTES " WHERE idRecette = " + QString::number(idRecette);
            QSqlQuery SupprimLignesRecettesQuery(requete,db->getDataBase());
            db->traiteErreurRequete(SupprimLignesRecettesQuery,requete,"");

            // nettoyer LignesComptes et le mettre en mémoire au cas où on devrait le restaurer
            LigneCompteAModifier.clear();
            if (ChercheVirementCompteQuery.size() > 0)
            {
                LigneCompteAModifier << ChercheVirementCompteQuery.value(0).toString();                                         //idLigne
                LigneCompteAModifier << ChercheVirementCompteQuery.value(1).toString();                                         //idCompte
                if (ChercheVirementCompteQuery.value(2).isNull())                                                               //idDep
                    LigneCompteAModifier << "null";
                else
                    LigneCompteAModifier << ChercheVirementCompteQuery.value(2).toString();
                if (ChercheVirementCompteQuery.value(3).isNull())                                                               //idRec
                    LigneCompteAModifier << "null";
                else
                    LigneCompteAModifier << ChercheVirementCompteQuery.value(3).toString();
                if (ChercheVirementCompteQuery.value(4).isNull())                                                               //idRecSpec
                    LigneCompteAModifier << "null";
                else
                    LigneCompteAModifier << ChercheVirementCompteQuery.value(4).toString();
                if (ChercheVirementCompteQuery.value(5).isNull())                                                               //idRemCheq
                    LigneCompteAModifier << "null";
                else
                    LigneCompteAModifier << ChercheVirementCompteQuery.value(5).toString();
                LigneCompteAModifier << "'" + ChercheVirementCompteQuery.value(6).toDate().toString("yyyy-MM-dd") + "'";        //LigneDate
                LigneCompteAModifier << "'" + Utils::correctquoteSQL(ChercheVirementCompteQuery.value(7).toString()) + "'";    //Lignelibelle
                LigneCompteAModifier << ChercheVirementCompteQuery.value(8).toString();                                         //LigneMontant
                LigneCompteAModifier << ChercheVirementCompteQuery.value(9).toString();                                         //LigneDebiCredit
                LigneCompteAModifier << "'" + Utils::correctquoteSQL(ChercheVirementCompteQuery.value(10).toString()) + "'";   //LigneTypeOperation
                if (ChercheVirementCompteQuery.value(11).toInt() == 0)                                                          //LigneConsolide
                    LigneCompteAModifier << "null";
                else
                    LigneCompteAModifier << ChercheVirementCompteQuery.value(11).toString();


                requete = "DELETE FROM " NOM_TABLE_LIGNESCOMPTES " WHERE idRec = " + QString::number(idRecette) + " AND LigneDebitCredit = 1";

                QSqlQuery SupprimLignesComptesQuery(requete,db->getDataBase());
                db->traiteErreurRequete(SupprimLignesComptesQuery,requete,"");

            }
            //Nettoyer Depenses  et comptes au besoin si une commission avait été enregistrée
            if (RetrouveRecetteQuery.value(11).toDouble() > 0)
            {
                  if (mp == "V")
                {
                    //Nettoyer et mettre en mémoire les commissions et les dépenses correspondantes éventuelles
                      QString CommCompterequete = "SELECT idLigne, idCompte, idDep, idRec, LigneDate,"
                                                  " LigneLibelle, LigneMontant, LigneDebitCredit, LigneTypeOperation, LigneConsolide"
                                                  " FROM " NOM_TABLE_LIGNESCOMPTES
                                                  " WHERE idRec = " + QString::number(idRecette) + " and idDep > 0";
                      QSqlQuery CommissionCompteQuery (CommCompterequete,db->getDataBase());
                      db->traiteErreurRequete(CommissionCompteQuery,CommCompterequete,"");
                      LigneCommissionCompteAModifier.clear();
                      if (CommissionCompteQuery.size() > 0)
                      {
                          CommissionCompteQuery.first();
                          LigneCommissionCompteAModifier << CommissionCompteQuery.value(0).toString();                                      //idLigne
                          LigneCommissionCompteAModifier << CommissionCompteQuery.value(1).toString();                                      //idCompte
                          LigneCommissionCompteAModifier << CommissionCompteQuery.value(2).toString();                                      //idDep
                          LigneCommissionCompteAModifier << CommissionCompteQuery.value(3).toString();                                      //idRec
                          LigneCommissionCompteAModifier << "null";                                                                         //idRecSpec
                          LigneCommissionCompteAModifier << "null";                                                                         //idRemCheq
                          LigneCommissionCompteAModifier << "'" + CommissionCompteQuery.value(4).toDate().toString("yyyy-MM-dd") + "'";     //LigneDate
                          LigneCommissionCompteAModifier << "'" + Utils::correctquoteSQL(CommissionCompteQuery.value(5).toString()) + "'"; //Lignelibelle
                          LigneCommissionCompteAModifier << CommissionCompteQuery.value(6).toString();                                      //LigneMontant
                          LigneCommissionCompteAModifier << CommissionCompteQuery.value(7).toString();                                      //LigneDebiCredit
                          LigneCommissionCompteAModifier << "'" + Utils::correctquoteSQL(CommissionCompteQuery.value(8).toString()) + "'"; //LigneTypeOperation
                          if (CommissionCompteQuery.value(9).toInt() == 0)
                              LigneCommissionCompteAModifier << "null";
                          else
                              LigneCommissionCompteAModifier << CommissionCompteQuery.value(9).toString();                                  //LigneConsolide

                          requete = "DELETE FROM " NOM_TABLE_LIGNESCOMPTES " WHERE idligne = " + CommissionCompteQuery.value(0).toString();
                          QSqlQuery SupprimLignesComptesQuery(requete,db->getDataBase());
                          db->traiteErreurRequete(SupprimLignesComptesQuery,requete,"");
                      }

                      QString Depenserequete = "select idDep, idUser, DateDep, Reffiscale, Objet,"
                                               " Montant, Famfiscale, Nooperation, Monnaie, idRec,"
                                               " ModePaiement, Compte, NoCheque from " NOM_TABLE_DEPENSES
                                               " where idDep = " + CommissionCompteQuery.value(2).toString();

                      QSqlQuery DepenseQuery (Depenserequete,db->getDataBase());
                      LigneDepenseAModifier.clear();
                      if (DepenseQuery.size() > 0)
                      {
                          DepenseQuery.first();
                          LigneDepenseAModifier << DepenseQuery.value(0).toString();                                            //idDep
                          LigneDepenseAModifier << DepenseQuery.value(1).toString();                                            //idUser
                          if (DepenseQuery.value(2).toString().isEmpty())
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << "'" + DepenseQuery.value(2).toDate().toString("yyyy-MM-dd") + "'";       //DateDep
                          if (DepenseQuery.value(3).toString().isEmpty())
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << "'" + Utils::correctquoteSQL(DepenseQuery.value(3).toString()) + "'"; //RefFiscale
                          if (DepenseQuery.value(4).toString().isEmpty())
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << "'" + Utils::correctquoteSQL(DepenseQuery.value(4).toString()) + "'"; //Objet
                          LigneDepenseAModifier << DepenseQuery.value(5).toString();                                            //Montant
                          if (DepenseQuery.value(6).toString().isEmpty())
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << "'" + Utils::correctquoteSQL(DepenseQuery.value(6).toString()) + "'"; //FamFiscale
                          if (DepenseQuery.value(7).toInt() == 0)
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << DepenseQuery.value(7).toString();                                       //Nooperation
                          if (DepenseQuery.value(8).toString().isEmpty())
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << "'" + Utils::correctquoteSQL(DepenseQuery.value(8).toString()) + "'"; //Monnaie
                          if (DepenseQuery.value(9).toInt() == 0)
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << DepenseQuery.value(9).toString();                                        //idRec
                          if (DepenseQuery.value(10).toString().isEmpty())
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << "'" + Utils::correctquoteSQL(DepenseQuery.value(10).toString()) + "'";//ModePaiement
                          if (DepenseQuery.value(11).toInt() == 0)
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << DepenseQuery.value(11).toString();                                       //Compte
                          if (DepenseQuery.value(12).toInt() == 0)
                              LigneDepenseAModifier << "null";
                          else
                              LigneDepenseAModifier << DepenseQuery.value(12).toString();                                       //NoCheque

                          requete = "DELETE FROM " NOM_TABLE_DEPENSES " WHERE idDep = " + DepenseQuery.value(0).toString();
                          QSqlQuery SupprimDepenseQuery(requete,db->getDataBase());
                          db->traiteErreurRequete(SupprimDepenseQuery,requete,"");
                      }
                  }
            }
            if (SupprimerBouton == Q_NULLPTR)
            {
                SupprimerBouton = new UpPushButton(ui->Buttonsframe);
                SupprimerBouton->setText(tr("Supprimer"));
                SupprimerBouton->setIcon(Icons::icPoubelle());
                SupprimerBouton->setIconSize(QSize(30,30));
                SupprimerBouton->move(10,10);
                SupprimerBouton->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Large);
                SupprimerBouton->setObjectName("SupprimerupPushButton");
                SupprimerBouton->setVisible(true);
                connect (SupprimerBouton, SIGNAL(clicked()), this, SLOT (Slot_SupprimerPaiement()));
            }
            else SupprimerBouton->setVisible(true);

        }
    }

    // Nettoyer LignesPaiements
    requete = "DELETE FROM " NOM_TABLE_LIGNESPAIEMENTS " WHERE idRecette = " + QString::number(idRecette);
    QSqlQuery SupprimLignesPaiementsQuery(requete,db->getDataBase());
    db->traiteErreurRequete(SupprimLignesPaiementsQuery,requete,"");

    gListidActe = ListeActesAModifier;
    RemplitLesTables(gMode);
    Slot_RegleAffichageFiche();
    ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(RetrouveRecetteQuery.value(7)));
    ui->dateEdit->setDate(RetrouveRecetteQuery.value(2).toDate());
    RegleAffichageTypePaiementframe(false);
    RegleAffichageTypePaiementframe(true,false);
    for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
    {
        QList<QTableWidgetItem*> items;
        items = ui->DetailupTableWidget->findItems(QString::number(ListeActesAModifier.at(i)),Qt::MatchExactly);
        int ik = items.at(0)->row();
        QString B = QLocale().toString(MontantActesAModifier.at(i).toDouble(),'f',2);
        UpLineEdit* Paye = static_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(ik,ui->DetailupTableWidget->columnCount()-2));
        Paye->setText(B);
    }
    gListidActe.clear();
    ui->AnnulupPushButton->setText(tr("Annuler et\nrétablir l'écriture"));
    ui->OKupPushButton->setEnabled(false);
    if (ModifLigneRecettePossible)
    {
        ui->dateEdit->setFocus();
        ui->dateEdit->setSelectedSection(QDateTimeEdit::DaySection);
    }
    else
    {
        ui->TireurChequelineEdit->setEnabled(false);
        ui->Tireurlabel->setEnabled(false);
        ui->BanqueChequecomboBox->setEnabled(false);
        ui->Banquelabel->setEnabled(false);
    }
    ModeModif   = "";
    //qDebug() << ui->ComptesupComboBox->currentText() + " - " + ui->ComptesupComboBox->currentData().toString();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Supprime de la liste des actes à afficher les actes verrouillés par d'autres utilisateur ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::NettoieVerrousListeActesAAfficher()
{
    if (gListidActe.size() > 0)
    {
        for (int i=0; i < gListidActe.size(); i++)
        {
            QString ChercheVerrou = "SELECT UserLogin FROM " NOM_TABLE_VERROUCOMPTAACTES " ver, " NOM_TABLE_UTILISATEURS " uti," NOM_TABLE_ACTES " act"
                    " WHERE act.idActe = "  + QString::number(gListidActe.at(i)) +
                    " AND ver.idActe = act.idActe"
                    " AND PosePar = uti.idUser";
            QSqlQuery ChercheVerrouQuery (ChercheVerrou,db->getDataBase());
            db->traiteErreurRequete(ChercheVerrouQuery, ChercheVerrou,"");
         }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Supprime les éventuels verrous laissés par erreur dans VerrousCompta ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::NettoieVerrousCompta()
{
    QString NettoieVerrousComptaActesRequete = "delete from " NOM_TABLE_VERROUCOMPTAACTES " where PosePar = " + QString::number(m_userConnected->id()) + " or PosePar is null";
    QSqlQuery NettoieVerrousComptaActesQuery (NettoieVerrousComptaActesRequete, db->getDataBase());
    db->traiteErreurRequete(NettoieVerrousComptaActesQuery,NettoieVerrousComptaActesRequete,"");
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Pose un verrou sur un acte ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::PoseVerrouCompta(int ActeAVerrouiller)
{
    QString verrourequete = "select idActe from " NOM_TABLE_VERROUCOMPTAACTES " where idActe = " + QString::number(ActeAVerrouiller);
    //UpMessageBox::Watch(this,verrourequete);
    QSqlQuery verifVerrouQuery (verrourequete,db->getDataBase());
    if (verifVerrouQuery.size() == 0)
    {
        QString VerrouilleEnreg= "INSERT INTO " NOM_TABLE_VERROUCOMPTAACTES
                " (idActe,DateTimeVerrou, PosePar)"
                " VALUES ("  + QString::number(ActeAVerrouiller) +
                ", NOW() ,"  + QString::number(m_userConnected->id()) + ")";
        QSqlQuery verrouilleAttentePaiementQuery (VerrouilleEnreg,db->getDataBase());
        db->traiteErreurRequete(verrouilleAttentePaiementQuery,VerrouilleEnreg,"");
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Retire un verrou sur un acte ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::RetireVerrouCompta(int ActeADeverrouiller)
{
    QString VerrouilleEnreg= "DELETE FROM " NOM_TABLE_VERROUCOMPTAACTES
            " WHERE idActe = " + QString::number(ActeADeverrouiller);
    QSqlQuery verrouilleAttentePaiementQuery (VerrouilleEnreg,db->getDataBase());
    db->traiteErreurRequete(verrouilleAttentePaiementQuery,VerrouilleEnreg,"");
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Banques dans le combobox Banques --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::ReconstruitListeBanques()
{
    QStringList ListBanques;

    ui->BanqueChequecomboBox->clear();
    QString req = "SELECT idBanqueAbrege FROM " NOM_TABLE_BANQUES " ORDER by idBanqueAbrege";
    QSqlQuery ListBanquesQuery (req,db->getDataBase());
    db->traiteErreurRequete(ListBanquesQuery,req,"");
    for (int i = 0; i < ListBanquesQuery.size(); i++)
    {
        ListBanquesQuery.seek(i);
        ListBanques << ListBanquesQuery.value(0).toString();
    }
    ui->BanqueChequecomboBox->insertItems(0,ListBanques);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Banques dans le combobox Banques --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::ReconstruitListeTiers()
{
    QStringList ListTiers, ListeTypeTiers;

    ui->TierscomboBox->clear();
    QString requete = "SELECT NomTiers FROM " NOM_TABLE_TIERS;
    QSqlQuery   ListTiersQuery (requete,db->getDataBase());
    db->traiteErreurRequete(ListTiersQuery,requete,"");
    for (int i = 0; i < ListTiersQuery.size(); i++)
    {
        ListTiersQuery.seek(i);
        ListTiers << ListTiersQuery.value(0).toString();
    }
    ui->TierscomboBox->insertItems(0,ListTiers);

    ui->TypeTierscomboBox->clear();
    requete = "SELECT Tiers FROM " NOM_TABLE_LISTETIERS;
    QSqlQuery   TiersQuery (requete,db->getDataBase());
    db->traiteErreurRequete(TiersQuery,requete,"");
    for (int i = 0; i < TiersQuery.size(); i++)
    {
        TiersQuery.seek(i);
        ListeTypeTiers << TiersQuery.value(0).toString();
    }
    ui->TypeTierscomboBox->insertItems(0,ListeTypeTiers);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Redessine les infos de paiement en fonction du mode de paiement choisi -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::RegleAffichageTypePaiementframe(bool VerifierEmetteur, bool AppeleParClicK)
{
    QAbstractButton *Emetteur = Q_NULLPTR;
    QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
    for (int n = 0; n <  allRButtons.size(); n++)
        if (allRButtons.at(n)->isChecked())
        {
            Emetteur = allRButtons.at(n);
            n = allRButtons.size();
        }

    if (!VerifierEmetteur)  // aucun RadioButton n'est coché, c'est le programme qui a appelé la méthode
                        // (changement de mode ou affichage des données d'un paiement déjà enregistré)
    {
        for (int i=0; i<ui->TypePaiementframe->findChildren<QWidget*>().size(); i++)
            ui->TypePaiementframe->findChildren<QWidget*>().at(i)->setEnabled(true);
        ui->TiersPayeurlabel->setVisible(false);
        ui->TierscomboBox->setVisible(false);
        ui->TypeTierslabel->setVisible(false);
        ui->TypeTierscomboBox->setVisible(false);
        ui->Banquelabel->setVisible(false);
        ui->BanqueChequecomboBox->setVisible(false);
        ui->Tireurlabel->setVisible(false);
        ui->TireurChequelineEdit->setVisible(false);
        ui->EnAttentelabel->setVisible(false);
        ui->EnAttentecheckBox->setVisible(false);
        ui->Commissionlabel->setVisible(false);
        ui->CommissionlineEdit->setVisible(false);
        ui->ComptesupComboBox->setVisible(false);
        ui->CompteCreditelabel->setVisible(false);
        ui->dateEdit->setVisible(false);

        if (gMode == Accueil)
        {
            ui->TypePaiementframe->setVisible(false);
            ui->MontantlineEdit->setText("0,00");
            ui->TierscomboBox->clearEditText();
            ui->BanqueChequecomboBox->clearEditText();
            ui->TireurChequelineEdit->clear();
            ui->EnAttentecheckBox->setCheckState(Qt::Unchecked);
            ui->ComptesupComboBox->clearEditText();
            ui->CommissionlineEdit->clear();
            ui->dateEdit->setDate(QDate::currentDate());
            ui->TotallineEdit->setText("0,00");
        }
        else
        {
            ui->TypePaiementframe->setVisible(true);
            ui->PaiementgroupBox->setVisible(true);
            ui->MontantlineEdit->setVisible(true);
            ui->Montantlabel->setVisible(true);
            ui->dateEdit->setVisible(true);
            switch (gMode) {
            case EnregistrePaiementDirect:
            {
                ui->PaiementgroupBox->setGeometry(0,5,150,185);
                ui->ImpayeradioButton->setVisible(true);
                ui->GratuitradioButton->setVisible(true);
                ui->CarteCreditradioButton->setVisible(true);
                ui->VirementradioButton->setVisible(false);
                ui->TiersradioButton->setVisible(true);
                ui->PaiementgroupBox->setFocusProxy(ui->CarteCreditradioButton);
                ui->ChequeradioButton->move(5,55);
                ui->EspecesradioButton->move(5,80);
                ui->CarteCreditradioButton->move(5,30);
                ui->TiersradioButton->move(5,105);
                ui->GratuitradioButton->move(5,130);
                ui->ImpayeradioButton->move(5,155);
                break;
            }
            case VoirListeActes:
            {
                ui->PaiementgroupBox->setGeometry(0,35,150,135);
                ui->ImpayeradioButton->setVisible(true);
                ui->GratuitradioButton->setVisible(true);
                ui->CarteCreditradioButton->setVisible(true);
                ui->VirementradioButton->setVisible(true);
                ui->VirementradioButton->move(5,105);
                ui->TiersradioButton->setVisible(false);
                ui->ChequeradioButton->move(5,55);
                ui->EspecesradioButton->move(5,80);
                ui->CarteCreditradioButton->move(5,30);
                break;
            }
            case EnregistrePaiementTiers:
            {
                ui->TierscomboBox->setVisible(true);
                ui->PaiementgroupBox->setGeometry(0,35,150,110);
                ui->ImpayeradioButton->setVisible(false);
                ui->GratuitradioButton->setVisible(false);
                ui->CarteCreditradioButton->setVisible(false);
                ui->VirementradioButton->setVisible(true);
                ui->VirementradioButton->move(5,30);
                ui->TiersradioButton->setVisible(false);
                ui->PaiementgroupBox->setFocusProxy(ui->VirementradioButton);
                ui->ChequeradioButton->move(5,55);
                ui->EspecesradioButton->move(5,80);
                break;
            }
            case VoirListePaiementsTiers:
            {
                ui->PaiementgroupBox->setGeometry(0,35,150,110);
                ui->ImpayeradioButton->setVisible(false);
                ui->GratuitradioButton->setVisible(false);
                ui->CarteCreditradioButton->setVisible(false);
                ui->VirementradioButton->setVisible(true);
                ui->VirementradioButton->move(5,30);
                ui->TiersradioButton->setVisible(false);
                ui->ChequeradioButton->move(5,55);
                ui->EspecesradioButton->move(5,80);
                break;
            }
            default:
                break;
            }
        }
    }
    else  // c'est le click sur un des radiobutton qui a appelé la méthode
    {
        if (Emetteur == ui->CarteCreditradioButton)
        {
            ui->TypeTierslabel      ->setVisible(false);
            ui->TypeTierscomboBox   ->setVisible(false);
            ui->EnAttentelabel      ->setVisible(false);
            ui->EnAttentecheckBox   ->setVisible(false);
            ui->TireurChequelineEdit->setVisible(false);
            ui->Tireurlabel         ->setVisible(false);
            ui->Banquelabel         ->setVisible(false);
            ui->BanqueChequecomboBox->setVisible(false);
            if (gMode == VoirListeActes || gMode == VoirListePaiementsTiers)
            {
                ui->TiersPayeurlabel    ->setVisible(true);
                ui->TierscomboBox       ->setVisible(true);
                ui->Montantlabel        ->setVisible(true);
                ui->MontantlineEdit     ->setVisible(true);
                ui->Montantlabel        ->setEnabled(true);
                ui->MontantlineEdit     ->setEnabled(true);
                ui->ComptesupComboBox   ->setVisible(true);
                ui->ComptesupComboBox   ->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));
                ui->CompteCreditelabel  ->setVisible(true);
                ui->Commissionlabel     ->setVisible(true);
                ui->CommissionlineEdit  ->setVisible(true);

            }
            else
            {
                ui->TiersPayeurlabel    ->setVisible(false);
                ui->TierscomboBox       ->setVisible(false);
                ui->Montantlabel        ->setVisible(false);
                ui->MontantlineEdit     ->setVisible(false);
                ui->Montantlabel        ->setEnabled(true);
                ui->MontantlineEdit     ->setEnabled(true);
                ui->ComptesupComboBox   ->setVisible(false);
                ui->CompteCreditelabel  ->setVisible(false);
                ui->Commissionlabel     ->setVisible(false);
                ui->CommissionlineEdit  ->setVisible(false);
                if (gMode == EnregistrePaiementDirect && AppeleParClicK)
                {
                    for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                    {
                        QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                        Paye->setReadOnly(false);
                        Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                        Slot_CalculTotalDetails();
                    }
                }
            }
        }
        else if (Emetteur == ui->ChequeradioButton)
        {
            ui->TireurChequelineEdit->setVisible(true);
            ui->Tireurlabel         ->setVisible(true);
            ui->Banquelabel         ->setVisible(true);
            ui->BanqueChequecomboBox->setVisible(true);
            ui->TiersPayeurlabel    ->setVisible(gMode == EnregistrePaiementTiers || gMode == VoirListePaiementsTiers);
            ui->TierscomboBox       ->setVisible(gMode == EnregistrePaiementTiers || gMode == VoirListePaiementsTiers);
            ui->TypeTierslabel      ->setVisible(false);
            ui->TypeTierscomboBox   ->setVisible(false);
            ui->EnAttentelabel      ->setVisible(true);
            ui->EnAttentecheckBox   ->setVisible(true);
            ui->Montantlabel        ->setVisible(true);
            ui->MontantlineEdit     ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->ComptesupComboBox   ->setVisible(false);
            ui->CompteCreditelabel  ->setVisible(false);
            // Si la Table Details n'est pas vide , on récupère le nom du premier de la liste pour remplir la case Tireur du chèque;
            if (gMode == EnregistrePaiementDirect && AppeleParClicK)
                if (ui->DetailupTableWidget->rowCount() > 0)
                {
                    QString req = "SELECT PatNom FROM " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_ACTES " act"
                                  " WHERE pat.idPat = act.idPat and idActe = " + ui->DetailupTableWidget->item(0,0)->text();
                    QSqlQuery ChercheNomPatQuery (req,db->getDataBase());
                    db->traiteErreurRequete(ChercheNomPatQuery,req,"");
                    ChercheNomPatQuery.first();
                    ui->TireurChequelineEdit->setText(ChercheNomPatQuery.value(0).toString());
                }
            if (gMode == VoirListeActes)
                if (ui->DetailupTableWidget->rowCount() > 0)
                {
                    ui->TiersPayeurlabel->setVisible(false);
                    ui->TierscomboBox->setVisible(false);
                    if (gTypeTable == ActesTiers)
                    {
                            ui->TiersPayeurlabel->setVisible(true);
                            ui->TierscomboBox->setVisible(true);
                    }
                }
            if (gMode == EnregistrePaiementTiers)
            {
                if (AppeleParClicK)
                    ui->TireurChequelineEdit->setText(ui->TierscomboBox->currentText());
            }
            ui->Commissionlabel->setVisible(false);
            ui->CommissionlineEdit->setVisible(false);

            if ((gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers) && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setReadOnly(false);
                    Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                    Slot_CalculTotalDetails();
                }
            }
        }
        else if (Emetteur == ui->TiersradioButton)
        {
            ui->EnAttentelabel      ->setVisible(false);
            ui->EnAttentecheckBox   ->setVisible(false);
            ui->TireurChequelineEdit->setVisible(false);
            ui->Tireurlabel         ->setVisible(false);
            ui->Banquelabel         ->setVisible(false);
            ui->BanqueChequecomboBox->setVisible(false);
            ui->TiersPayeurlabel    ->setVisible(false);
            ui->TierscomboBox       ->setVisible(false);
            ui->TypeTierslabel      ->setVisible(true);
            ui->TypeTierscomboBox   ->setVisible(true);
            ui->MontantlineEdit     ->setText("0,00");
            ui->Montantlabel        ->setVisible(false);
            ui->MontantlineEdit     ->setVisible(false);
            ui->ComptesupComboBox   ->setVisible(false);
            ui->CompteCreditelabel  ->setVisible(false);
            ui->Commissionlabel     ->setVisible(false);
            ui->CommissionlineEdit  ->setVisible(false);
            if ((gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers) && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                    Paye->setReadOnly(true);
                }
            }
        }
        else if (Emetteur == ui->EspecesradioButton)
        {
            ui->EnAttentelabel      ->setVisible(false);
            ui->EnAttentecheckBox   ->setVisible(false);
            ui->TireurChequelineEdit->setVisible(false);
            ui->Tireurlabel         ->setVisible(false);
            ui->Banquelabel         ->setVisible(false);
            ui->BanqueChequecomboBox->setVisible(false);
            ui->TiersPayeurlabel    ->setVisible(false);
            ui->TierscomboBox       ->setVisible(false);
            ui->TypeTierslabel      ->setVisible(false);
            ui->TypeTierscomboBox   ->setVisible(false);
            ui->Montantlabel        ->setVisible(true);
            ui->MontantlineEdit     ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->ComptesupComboBox   ->setVisible(false);
            ui->CompteCreditelabel  ->setVisible(false);
            ui->Commissionlabel     ->setVisible(false);
            ui->CommissionlineEdit  ->setVisible(false);
            if ((gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers) && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setReadOnly(false);
                    Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                    Slot_CalculTotalDetails();
                }
            }
        }
        else if (Emetteur == ui->GratuitradioButton)
        {
            ui->EnAttentelabel      ->setVisible(false);
            ui->EnAttentecheckBox   ->setVisible(false);
            ui->TireurChequelineEdit->setVisible(false);
            ui->Tireurlabel         ->setVisible(false);
            ui->Banquelabel         ->setVisible(false);
            ui->BanqueChequecomboBox->setVisible(false);
            ui->TiersPayeurlabel    ->setVisible(false);
            ui->TierscomboBox       ->setVisible(false);
            ui->TypeTierslabel      ->setVisible(false);
            ui->TypeTierscomboBox   ->setVisible(false);
            ui->MontantlineEdit     ->setText("0,00");
            ui->Montantlabel        ->setVisible(false);
            ui->MontantlineEdit     ->setVisible(false);
            ui->ComptesupComboBox   ->setVisible(false);
            ui->CompteCreditelabel  ->setVisible(false);
            ui->Commissionlabel     ->setVisible(false);
            ui->CommissionlineEdit  ->setVisible(false);
            if (gMode == EnregistrePaiementDirect && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setText("0,00");
                    Paye->setReadOnly(true);
                    Slot_CalculTotalDetails();
                }
            }
        }
        else if (Emetteur == ui->ImpayeradioButton)
        {
            ui->EnAttentelabel      ->setVisible(false);
            ui->EnAttentecheckBox   ->setVisible(false);
            ui->TireurChequelineEdit->setVisible(false);
            ui->Tireurlabel         ->setVisible(false);
            ui->Banquelabel         ->setVisible(false);
            ui->BanqueChequecomboBox->setVisible(false);
            ui->TiersPayeurlabel    ->setVisible(false);
            ui->TierscomboBox       ->setVisible(false);
            ui->TypeTierslabel      ->setVisible(false);
            ui->TypeTierscomboBox   ->setVisible(false);
            ui->Montantlabel        ->setVisible(true);
            ui->Montantlabel        ->setEnabled(false);
            ui->MontantlineEdit     ->setVisible(true);
            ui->MontantlineEdit     ->setEnabled(false);
            ui->ComptesupComboBox   ->setVisible(false);
            ui->CompteCreditelabel  ->setVisible(false);
            ui->Commissionlabel     ->setVisible(false);
            ui->CommissionlineEdit  ->setVisible(false);
            if (gMode == EnregistrePaiementDirect && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setText("0,00");
                    Paye->setReadOnly(true);
                }
            }
        }
        else if (Emetteur == ui->VirementradioButton)
        {
            ui->EnAttentelabel      ->setVisible(false);
            ui->EnAttentecheckBox   ->setVisible(false);
            ui->TireurChequelineEdit->setVisible(false);
            ui->Tireurlabel         ->setVisible(false);
            ui->Banquelabel         ->setVisible(false);
            ui->BanqueChequecomboBox->setVisible(false);
            ui->TiersPayeurlabel    ->setVisible(true);
            ui->TierscomboBox       ->setVisible(true);
            ui->TypeTierslabel      ->setVisible(false);
            ui->TypeTierscomboBox   ->setVisible(false);
            ui->Montantlabel        ->setVisible(true);
            ui->MontantlineEdit     ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->ComptesupComboBox   ->setVisible(true);
            ui->CompteCreditelabel  ->setVisible(true);
            ui->Commissionlabel     ->setVisible(gMode!= EnregistrePaiementDirect);
            ui->CommissionlineEdit  ->setVisible(gMode!=EnregistrePaiementDirect);
            if (gMode == EnregistrePaiementTiers && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setReadOnly(false);
                    Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                }
            }
        }
        ui->OKupPushButton->setEnabled(true);
    }

    ui->TypePaiementframe   ->setEnabled(gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers);
    ui->PaiementgroupBox    ->setVisible(!(gMode == Accueil || gMode == VoirListeActes));
    switch (gMode) {
    case EnregistrePaiementDirect:
    case EnregistrePaiementTiers:
    {
        ui->OKupPushButton      ->setEnabled(QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || (Emetteur !=Q_NULLPTR) || ui->DetailupTableWidget->rowCount() > 0);
        ui->PaiementgroupBox    ->setEnabled(ModifLigneRecettePossible);
        ui->MontantlineEdit     ->setEnabled(ModifLigneRecettePossible);
        ui->Montantlabel        ->setEnabled(ModifLigneRecettePossible);
        ui->CommissionlineEdit  ->setEnabled(ModifLigneRecettePossible);
        ui->Commissionlabel     ->setEnabled(ModifLigneRecettePossible);
        ui->EnAttentecheckBox   ->setEnabled(ModifLigneRecettePossible);
        ui->EnAttentelabel      ->setEnabled(ModifLigneRecettePossible);
        ui->dateEdit            ->setEnabled(ModifLigneRecettePossible);
        break;
    }
    case VoirListeActes:
    {
        if (ui->SalleDAttenteupTableWidget->rowCount() > 0)
            ui->OKupPushButton  ->setEnabled(ui->SalleDAttenteupTableWidget->selectedRanges().size() > 0);
        else if (ui->ListeupTableWidget->selectedRanges().size() > 0)
            ui->OKupPushButton  ->setEnabled(ui->ListeupTableWidget->item(ui->ListeupTableWidget->selectedRanges().at(0).topRow(),1)->foreground() == Qt::NoBrush);
        else ui->OKupPushButton ->setEnabled(false);
        ui->TypePaiementframe   ->setVisible(!ui->PasdePaiementlabel->isVisible());
        ui->PaiementgroupBox    ->setVisible(ui->SalleDAttenteupTableWidget->selectedRanges().size()>0);
        break;
    }
    case VoirListePaiementsTiers:
    {
        if (ui->SalleDAttenteupTableWidget->isVisible())
        {
            if (ui->SalleDAttenteupTableWidget->rowCount() > 0)
                ui->OKupPushButton  ->setEnabled(ui->SalleDAttenteupTableWidget->selectedRanges().size() > 0);
            else if (ui->ListeupTableWidget->selectedRanges().size() > 0)
                ui->OKupPushButton  ->setEnabled(ui->ListeupTableWidget->item(ui->ListeupTableWidget->selectedRanges().at(0).topRow(),1)->foreground() == Qt::NoBrush);
            else ui->OKupPushButton ->setEnabled(false);
            ui->TypePaiementframe   ->setVisible(!ui->PasdePaiementlabel->isVisible());
            ui->TypePaiementframe   ->setVisible(ui->SalleDAttenteupTableWidget->selectedRanges().size()>0);
            ui->PaiementgroupBox    ->setVisible(ui->SalleDAttenteupTableWidget->selectedRanges().size()>0);
        }
        else
            ui->OKupPushButton      ->setEnabled(ui->ListeupTableWidget->selectedRanges().size() > 0);
        break;
    }
    case Accueil:
    {
        ui->OKupPushButton->setEnabled(true);
        break;
    }
    default:
        break;
    }
    if (!(ui->VirementradioButton->isChecked() || ui->CarteCreditradioButton->isChecked()))
        ui->CommissionlineEdit->setText("0,00");
}


/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reinitialise tous les champs de saisie --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::RemetToutAZero()
{
    QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
    for (int n = 0; n <  allRButtons.size(); n++)
    {
        allRButtons.at(n)->setAutoExclusive(false);
        allRButtons.at(n)->setChecked(false);
        allRButtons.at(n)->setAutoExclusive(true);
    }
    int cols = ui->DetailupTableWidget->columnCount();
    int rows = ui->DetailupTableWidget->rowCount();
    for (int i= 0; i<rows; i++)
        for (int k=0; k<cols; k++)
            ui->DetailupTableWidget ->removeCellWidget(i,k);
    ui->DetailupTableWidget         ->clearContents();
    ui->DetailupTableWidget         ->setRowCount(0);
    ui->MontantlineEdit             ->setText("0,00");
    ui->TierscomboBox               ->clearEditText();
    ui->ComptesupComboBox           ->clearEditText();
    ui->CommissionlineEdit          ->setText("0,00");
    ui->BanqueChequecomboBox        ->clearEditText();
    ui->TireurChequelineEdit        ->clear();
    ui->EnAttentecheckBox           ->setCheckState(Qt::Unchecked);
    gListidActe.clear();
    RemplitLesTables(gMode);
    ui->OKupPushButton              ->setEnabled(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les TableWidget ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::RemplirTableWidget(QTableWidget *TableARemplir, QString TypeTable, QSqlQuery TableQuery, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas)
{
    QTableWidgetItem    *pItem1, *pItem2, *pItem3, *pItem4, *pItem5, *pItem6, *pItem7, *pItem8, *pItem9;
    QDoubleValidator *val;
    QString             A;
    QFontMetrics fm(qApp->font());

    disconnect (TableARemplir,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
    TableARemplir->clearContents();
    TableARemplir->verticalHeader()->hide();

    // cette ligne est nécessaire pour éviter un pb d'affichage des tables quand on les redéfiniit
    TableARemplir->setRowCount(0);
    TableARemplir->setRowCount(TableQuery.size());

    TableQuery.first();

    if (TypeTable == "Actes")                                                               // Table affichant des actes
    {
        for (int i = 0; i < TableQuery.size(); i++)
            {
                int col = 0;

                A = TableQuery.value(0).toString();                                                 // idACte
                pItem1 = new QTableWidgetItem() ;
                pItem1->setText(A);
                TableARemplir->setItem(i,col,pItem1);
                col++;

                if (AvecUpcheckBox)
                {
                    UpCheckBox *CheckItem = new UpCheckBox();
                    if (gListidActe.contains(TableQuery.value(0).toInt()))
                        CheckItem->setCheckState(Qt::Checked);
                    else
                        CheckItem->setCheckState(CheckedOuPas);
                    CheckItem->setRowTable(i);
                    CheckItem->setFocusPolicy(Qt::NoFocus);
                    connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
                    CheckItem->installEventFilter(this);
                    TableARemplir->setCellWidget(i,col,CheckItem);
                }
                if (TableARemplir == ui->DetailupTableWidget || AvecUpcheckBox)
                col++;

                A = TableQuery.value(1).toDate().toString(tr("dd-MM-yyyy"));                            // Date
                pItem2 = new QTableWidgetItem() ;
                pItem2->setText(A);
                TableARemplir->setItem(i,col,pItem2);
                col++;

                A = TableQuery.value(2).toString() + " " + TableQuery.value(3).toString();          // Nom Prenom
                pItem3 = new QTableWidgetItem() ;
                pItem3->setText(A);
                TableARemplir->setItem(i,col,pItem3);
                col++;

                A = TableQuery.value(4).toString();                                                 // Cotation
                pItem4 = new QTableWidgetItem() ;
                pItem4->setText(A);
                TableARemplir->setItem(i,col,pItem4);
                col++;

                QString Montant;
                if (TableQuery.value(6).toString() == "F")
                    Montant = QLocale().toString(TableQuery.value(5).toDouble()/6.55957,'f',2);                         // Montant en F converti en euros
                else
                    Montant = QLocale().toString(TableQuery.value(5).toDouble(),'f',2);                                 // Montant
                pItem5 = new QTableWidgetItem() ;
                pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pItem5->setText(Montant);
                TableARemplir->setItem(i,col,pItem5);
                col++;

                if (gMode == EnregistrePaiementDirect)
                {
                    A = TableQuery.value(7).toString();
                    if (TableARemplir == ui->ListeupTableWidget)
                    {
                        if (A == "I")
                        {
                            QLabel * lbl = new QLabel();
                            lbl->setAlignment(Qt::AlignCenter);
                            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxBlackCheck 15,15
                            TableARemplir->setCellWidget(i,col,lbl);                                    // Impayé (O/N)
                        }
                        col++;
                        QString ResteDu;
                        if (TableQuery.value(6).toString() == "F")
                            ResteDu = QLocale().toString((TableQuery.value(5).toDouble()-TableQuery.value(9).toDouble())/6.55957,'f',2);  // ResteDu en F converti en euros
                        else
                            ResteDu = QLocale().toString((TableQuery.value(5).toDouble()-TableQuery.value(9).toDouble()),'f',2);          // ResteDu
                        pItem6 = new QTableWidgetItem() ;
                        pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem6->setText(ResteDu);
                        pItem6->setForeground(QBrush(QColor(Qt::magenta)));
                        TableARemplir->setItem(i,col,pItem6);
                        col++;
                    }
                    if (TableARemplir == ui->SalleDAttenteupTableWidget)
                    {
                        pItem7 = new QTableWidgetItem() ;
                        pItem7->setText("0,00");
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        TableARemplir->setItem(i,col,pItem7);                                           // Payé
                        col++;
                        pItem6 = new QTableWidgetItem() ;
                        pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem6->setText(Montant);
                        pItem6->setForeground(QBrush(QColor(Qt::magenta)));
                        TableARemplir->setItem(i,col,pItem6);
                        col++;
                    }
                    if (TableARemplir == ui->DetailupTableWidget)
                    {
                        QString DejaPaye;
                        if (TableQuery.value(6).toString() == "F")
                            DejaPaye = QLocale().toString(TableQuery.value(7).toDouble()/6.55957,'f',2);
                        else
                            DejaPaye = QLocale().toString(TableQuery.value(7).toDouble(),'f',2);
                        pItem7 = new QTableWidgetItem() ;
                        pItem7->setText(QLocale().toString((QLocale().toDouble(Montant)-QLocale().toDouble(DejaPaye)),'f',2));
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        TableARemplir->setItem(i,col,pItem7);                                           // Reste dû
                        col++;
                        UpLineEdit *LigneMontant = new UpLineEdit();
                        LigneMontant->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        val = new QDoubleValidator(this);
                        val->setDecimals(2);
                        LigneMontant->setValidator(val);
                        LigneMontant->setText(Montant);
                        LigneMontant->setRowTable(i);
                        LigneMontant->setColumnTable(col);
                        LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                        TableARemplir->setCellWidget(i,col,LigneMontant);                               // Payé
                        connect (LigneMontant, SIGNAL(textChanged(QString)), this, SLOT(Slot_CalculTotalDetails()));
                        LigneMontant->installEventFilter(this);
                        col++;
                    }
                }

                if (gMode == EnregistrePaiementTiers)
                {
                    A = TableQuery.value(8).toString();
                    pItem6 = new QTableWidgetItem() ;
                    pItem6->setTextAlignment(Qt::AlignCenter);
                    pItem6->setText(A);
                    // col = 6 / Type tiers
                    TableARemplir->setItem(i,col,pItem6);                                           // Tiers
                    col++;
                    if (TableARemplir == ui->ListeupTableWidget)
                    {
                        if (TableQuery.value(6).toString() == "F")
                            A = QLocale().toString((QLocale().toDouble(Montant) - TableQuery.value(7).toDouble())/6.55957,'f',2);   // Payé en F converti en euros
                        else
                            A = QLocale().toString(QLocale().toDouble(Montant) - TableQuery.value(7).toDouble(),'f',2);             // Payé
                        pItem7 = new QTableWidgetItem;
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem7->setText(A);
                        // col = 7 Reste du
                        TableARemplir->setItem(i,col,pItem7);
                        col++;

                        pItem8 = new QTableWidgetItem;
                        pItem8->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        if (TableQuery.value(6).toString() == "F")
                            A = QLocale().toString(TableQuery.value(7).toDouble()/6.55957,'f',2);
                        else
                            A = QLocale().toString(TableQuery.value(7).toDouble(),'f',2);
                        pItem8->setText(A);
                        // col = 8 Payé
                        TableARemplir->setItem(i,col,pItem8);                                           // Reste dû
                    }
                    else
                    {
                        if (TableQuery.value(6).toString() == "F")
                            A = QLocale().toString(TableQuery.value(7).toDouble()/6.55957,'f',2);                     // Reste dû en F converti en euros
                        else
                            A = QLocale().toString(TableQuery.value(7).toDouble(),'f',2);                             // Reste dû
                        pItem7 = new QTableWidgetItem;
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem7->setText(A);
                        // col = 7 Reste du
                        TableARemplir->setItem(i,col,pItem7);
                        col++;
                        UpLineEdit *LignePaye = new UpLineEdit();
                        val = new QDoubleValidator(this);
                        val->setDecimals(2);
                        LignePaye->setValidator(val);
                        LignePaye->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        LignePaye->setText(A);                                                    // Payé
                        LignePaye->setRowTable(i);
                        LignePaye->setColumnTable(col);
                        LignePaye->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                        connect (LignePaye, SIGNAL(textChanged(QString)), this, SLOT(Slot_CalculTotalDetails()));
                        LignePaye->installEventFilter(this);
                        TableARemplir->setCellWidget(i,col,LignePaye);
                    }
                    col++;
                }

                if (gMode == VoirListeActes)
                {
                    A = TableQuery.value(8).toString();
                    if (A == "T") A = TableQuery.value(9).toString();
                    if (A == "E") A = tr("Espèces");
                    if (A == "C") A = tr("Chèque");
                    if (A == "I") A = tr("Impayé");
                    if (A == "G") A = tr("Gratuit");
                    if (A == "CB")  A = tr("Carte bancaire");
                    pItem6 = new QTableWidgetItem() ;
                    if (TableARemplir == ui->ListeupTableWidget)
                    {
                        pItem6->setTextAlignment(Qt::AlignCenter);
                        pItem6->setText(A);
                        TableARemplir->setItem(i,col,pItem6);                                           // Type paiement
                        col++;
                        A = QLocale().toString(TableQuery.value(7).toDouble(),'f',2);
                        pItem7 = new QTableWidgetItem;
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem7->setText(A);
                        TableARemplir->setItem(i,col,pItem7);                                  // Payé
                        col++;
                        QString B = QLocale().toString((TableQuery.value(5).toDouble() - TableQuery.value(7).toDouble()),'f',2);
                        pItem8 = new QTableWidgetItem;
                        pItem8->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem8->setText(B);
                        if (QLocale().toDouble(B) > 0) pItem8->setForeground(QBrush(Qt::magenta));
                        TableARemplir->setItem(i,col,pItem8);                                  // Payé
                        col++;
                        QString ab ="";
                        if (pItem6->text() == tr("Gratuit") && Montant == "0,00")
                        {
                            pItem2->setForeground(gtextureGris);
                            pItem3->setForeground(gtextureGris);
                            pItem4->setForeground(gtextureGris);
                            pItem5->setForeground(gtextureGris);
                            pItem6->setForeground(gtextureGris);
                            pItem7->setForeground(gtextureGris);
                            pItem8->setForeground(gtextureGris);
                            ab = tr("Vous ne pouvez pas modifier cet acte!\nSon montant est nul.");
                        }
                        pItem2->setToolTip(ab);
                        pItem3->setToolTip(ab);
                        pItem4->setToolTip(ab);
                        pItem5->setToolTip(ab);
                        pItem6->setToolTip(ab);
                        pItem7->setToolTip(ab);
                        pItem8->setToolTip(ab);
                    }
                    if (TableARemplir == ui->DetailupTableWidget)
                    {
                        if (gTypeTable == ActesTiers)
                        {
                            A = TableQuery.value(9).toString();
                            if (A == "CB")  A = tr("Carte Bancaire");
                            pItem6->setTextAlignment(Qt::AlignCenter);
                            pItem6->setText(A);
                            TableARemplir->setItem(i,col,pItem6);                                                   // Type paiement
                            col++;
                        }
                        QString Paye;
                        if (TableQuery.value(6).toString() == "F")
                            Paye = QLocale().toString(TableQuery.value(7).toDouble()/6.55957,'f',2);                  // Paye en F converti en euros
                        else
                            Paye = QLocale().toString(TableQuery.value(7).toDouble(),'f',2);                          // Paye
                        QString TotalPaye;
                        if (TableQuery.value(6).toString() == "F")
                            TotalPaye = QLocale().toString(TableQuery.value(10).toDouble()/6.55957,'f',2);            // TotalPaye en F converti en euros
                        else
                            TotalPaye = QLocale().toString(TableQuery.value(10).toDouble(),'f',2);                    // TotalPaye
                        QString ResteDu = QLocale().toString(QLocale().toDouble(Montant)-QLocale().toDouble(TotalPaye),'f',2);
                        pItem7 = new QTableWidgetItem() ;
                        pItem7->setText(ResteDu);
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        TableARemplir->setItem(i,col,pItem7);                                                       // Reste dû
                        col++;
                        pItem8 = new QTableWidgetItem() ;
                        pItem8->setText(Paye);
                        pItem8->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        TableARemplir->setItem(i,col,pItem8);                                                       // Reste dû
                        col++;
                    }
                }
                if (gMode == VoirListePaiementsTiers)
                {
                    pItem6 = new QTableWidgetItem() ;
                    if (TableARemplir == ui->ListeupTableWidget)
                    {
                        A = TableQuery.value(8).toString();
                        if (A == "T") A = TableQuery.value(9).toString();
                        if (A == "E") A = tr("Espèces");
                        if (A == "C") A = tr("Chèque");
                        if (A == "CB")  A = tr("Carte bancaire");
                        pItem6->setTextAlignment(Qt::AlignCenter);
                        pItem6->setText(A);
                        TableARemplir->setItem(i,col,pItem6);                                                       // Type paiement
                        col++;
                        A = QLocale().toString(TableQuery.value(7).toDouble(),'f',2);
                        pItem7 = new QTableWidgetItem;
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem7->setText(A);
                        TableARemplir->setItem(i,col,pItem7);                                                       // Payé
                        col++;
                    }
                    if (TableARemplir == ui->DetailupTableWidget)
                    {
                        A = TableQuery.value(9).toString();
                        if (A == "CB")  A = tr("Carte bancaire");
                        pItem6->setTextAlignment(Qt::AlignCenter);
                        pItem6->setText(A);
                        TableARemplir->setItem(i,col,pItem6);                                                       // Type paiement
                        col++;
                        QString Paye;
                        if (TableQuery.value(6).toString() == "F")
                            Paye = QLocale().toString(TableQuery.value(7).toDouble()/6.55957,'f',2);                  // Paye en F converti en euros
                        else
                            Paye = QLocale().toString(TableQuery.value(7).toDouble(),'f',2);                          // Paye
                        QString TotalPaye;
                        if (TableQuery.value(6).toString() == "F")
                            TotalPaye = QLocale().toString(TableQuery.value(10).toDouble()/6.55957,'f',2);            // TotalPaye en F converti en euros
                        else
                            TotalPaye = QLocale().toString(TableQuery.value(10).toDouble(),'f',2);                    // TotalPaye
                        QString ResteDu = QLocale().toString(QLocale().toDouble(Montant)- QLocale().toDouble(TotalPaye),'f',2);
                        pItem7 = new QTableWidgetItem() ;
                        pItem7->setText(ResteDu);
                        if (QLocale().toDouble(ResteDu) > 0) pItem7->setForeground(QBrush(Qt::magenta));
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        TableARemplir->setItem(i,col,pItem7);                                                       // Reste dû
                        col++;
                        pItem8 = new QTableWidgetItem() ;
                        pItem8->setText(Paye);
                        pItem8->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        TableARemplir->setItem(i,col,pItem8);                                                       // Payé
                        col++;
                    }
                }                

                A = TableQuery.value(1).toDate().toString("yyyy-MM-dd");                                        // ClassementparDate
                pItem9 = new QTableWidgetItem() ;
                pItem9->setText(A);
                TableARemplir->setItem(i,col,pItem9);

                TableARemplir->setRowHeight(i,int(fm.height()*1.1));

                TableQuery.next();
            }
    }

    if (TypeTable == "Paiements")                                                                   // Table affichant des paiements
    {
        for (int i = 0; i < TableQuery.size(); i++)
            {
                int col = 0;

                A = TableQuery.value(0).toString();                                                 // idRecette
                pItem1 = new QTableWidgetItem() ;
                pItem1->setText(A);
                TableARemplir->setItem(i,col,pItem1);
                col++;

                if (AvecUpcheckBox)
                {
                    UpCheckBox *CheckItem = new UpCheckBox();
                    if (gListidActe.contains(TableQuery.value(0).toInt()))
                        CheckItem->setCheckState(Qt::Checked);
                    else
                        CheckItem->setCheckState(CheckedOuPas);
                    CheckItem->setRowTable(i);
                    CheckItem->setFocusPolicy(Qt::NoFocus);
                    connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
                    CheckItem->installEventFilter(this);
                    TableARemplir->setCellWidget(i,col,CheckItem);
                    col++;
                }

                A = TableQuery.value(1).toDate().toString(tr("dd-MM-yyyy"));                            // Date
                pItem2 = new QTableWidgetItem() ;
                pItem2->setText(A);
                TableARemplir->setItem(i,col,pItem2);
                col++;

                QString mp = TableQuery.value(4).toString();
                if (mp == "V" && TableQuery.value(9).toString() == "CB")
                    A = tr("Virement carte bancaire");
                else
                    A = TableQuery.value(9).toString();                                                 // Payeur
                pItem3 = new QTableWidgetItem() ;
                pItem3->setText(A);
                TableARemplir->setItem(i,col,pItem3);
                col++;

                if (mp == "V") A = tr("Virement");
                if (mp == "E") A = tr("Espèces");
                if (mp == "C") A = tr("Chèque");                                                   // Type paiement
                pItem4 = new QTableWidgetItem() ;
                pItem4->setText(A);
                pItem4->setTextAlignment(Qt::AlignCenter);
                TableARemplir->setItem(i,col,pItem4);
                col++;


                if (mp == "C")
                    A = TableQuery.value(16).toDate().toString(tr("dd-MM-yyyy"));                        // Date validation
                else
                    A = TableQuery.value(2).toDate().toString(tr("dd-MM-yyyy"));
                pItem5 = new QTableWidgetItem() ;
                pItem5->setText(A);
                TableARemplir->setItem(i,col,pItem5);
                col++;

                if (TableQuery.value(11).toString() == "F")
                    A = QLocale().toString(TableQuery.value(17).toDouble()/6.55957,'f',2);            // Payé en F converti en euros
                else
                    A = QLocale().toString(TableQuery.value(17).toDouble(),'f',2);                     // Payé
                pItem6 = new QTableWidgetItem() ;
                pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pItem6->setText(A);
                TableARemplir->setItem(i,col,pItem6);
                col++;

                A = TableQuery.value(1).toDate().toString("yyyy-MM-dd");                             // ClassementparDate
                pItem7 = new QTableWidgetItem() ;
                pItem7->setText(A);
                TableARemplir->setItem(i,col,pItem7);

                TableARemplir->setRowHeight(i,int(fm.height()*1.1));
                TableQuery.next();
            }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les TableListe et SalDat en fonction du mode appelé ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::RemplitLesTables(int Mode)
{
    QTableWidget        *TableListe = ui->ListeupTableWidget;
    QTableWidget        *TableDetails = ui->DetailupTableWidget;
    QTableWidget        *TableSalDat = ui->SalleDAttenteupTableWidget;
    QString             requete;
    disconnect (TableListe,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));

    QString user =  " AND act.UserComptable = ";
    if (m_userConnected->isLiberal())
        // l'utilisateur est un soignant liberal et responsable - il enregistre ses actes et ceux de ses éventuels salariés
        user = " AND act.UserComptable = "  + QString::number(gidUserACrediter) + "\n";
    else if (m_userConnected->isSalarie() && !m_userConnected->isAssistant())
        // l'utilisateur est un soignant salarie et responsable
        user = " AND act.UserComptable = "  + QString::number(m_userConnected->getIdUserComptable()) + "\n"
               " AND act.UserParent = "     + QString::number(m_userConnected->id()) + "\n";
    else if (m_userConnected->isRemplacant())
        // l'utilisateur est un remplacant
        user = " AND act.UserComptable = "  + QString::number(m_userConnected->getIdUserComptable()) + "\n"
               " AND act.UserParent = "     + QString::number(m_userConnected->id()) + "\n";
    else if (m_userConnected->isSecretaire())
        // l'utilisateur est un secretaire
        user = " AND act.UserComptable = "  + QString::number(gidUserACrediter) + "\n";
    else
        user = " AND act.UserComptable = "  + QString::number(gidUserACrediter) + "\n";

    switch (Mode)
    {

    /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Enregistrement d'un paiement par tiers --------------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    case EnregistrePaiementTiers:
    {
        /*
        1. Remplissage TableListe -- tous les paiemenst en attente en dehors de ceux de la salle d'attente
        càd ceux:
        . pour lesquels la somme des montants versés est inférieure au prix de l'acte
        . ou dont le type de paiement enregistré est "tiers"
        */
        DefinitArchitectureTableView(TableListe, ActesTiers);
        requete =   "select * from (\n"
                    "SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, Actemontant -SUM(Paye) as tot, Tiers\n"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ, " NOM_TABLE_LIGNESPAIEMENTS " lig\n"
                    " WHERE act.idActe = typ.idActe\n"
                    " AND lig.idActe = act.idActe\n"
                    " AND TypePaiement = 'T'\n"
                    " AND act.idPat = pat.idPat\n";
        requete +=  user;
        requete +=  " group by act.idacte) as mar\n"
                    " where tot > 0\n"
                    " union\n\n"                                    // tous les actes enregistrés en tiers pour lesquels le paiement est incomplet

                    " (SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, ActeMontant as tot, Tiers\n"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ\n"
                    " WHERE act.idActe = typ.idActe\n"
                    " AND act.idacte not in (select idacte from " NOM_TABLE_LIGNESPAIEMENTS ")\n"
                    " AND TypePaiement = 'T'\n"
                    " AND act.idPat = pat.idPat\n";
        requete +=  user + ")\n";
        requete +=  " order by acteDate desc, PatNom, PatPrenom";
        //UpMessageBox::Watch(this,requete);

        QSqlQuery ListeActesQuery (requete,db->getDataBase());
        db->traiteErreurRequete(ListeActesQuery,requete,"");
        RemplirTableWidget(TableListe,"Actes", ListeActesQuery, true, Qt::Unchecked);
        ui->DetailupTableWidget->setRowCount(0);
        ui->DetailupTableWidget->setColumnCount(0);
        ui->DetailupTableWidget->horizontalHeader()->hide();

        //Remplissage de la table Details en cas de modification de paiement en cours
        DefinitArchitectureTableView(TableDetails,ActesTiers);
        if (gListidActe.size() > 0 && ModifPaiementEnCours)
        {
            NettoieVerrousListeActesAAfficher();
            QString CriteresRequete = " AND act.idActe IN (";
            for (int u = 0; u < gListidActe.size(); u++)
            {
                PoseVerrouCompta(gListidActe.at(u));
                if (u < gListidActe.size()-1)
                    CriteresRequete += QString::number(gListidActe.at(u)) + ", ";
                else
                    CriteresRequete += QString::number(gListidActe.at(u));
             }
            requete =
                    "select * from (\n"
                    "SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, Actemontant -SUM(Paye) as tot, Tiers\n"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ, " NOM_TABLE_LIGNESPAIEMENTS " lig\n"
                    " WHERE act.idActe = typ.idActe\n"
                    " AND lig.idActe = act.idActe\n"
                    " AND act.idPat = pat.idPat\n"
                    + CriteresRequete +
                    ")\n group by act.idacte) as mar\n"

                    " union\n\n"

                    " (SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, ActeMontant as tot, Tiers\n"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ\n"
                    " WHERE act.idActe = typ.idActe\n"
                    " AND act.idacte not in (select idacte from " NOM_TABLE_LIGNESPAIEMENTS ")\n"
                    + CriteresRequete +
                    ")\n AND act.idPat = pat.idPat)\n"
                    " order by acteDate desc, PatNom, PatPrenom";

            //UpMessageBox::Watch(this,requete);
            QSqlQuery DetailsQuery (requete,db->getDataBase());
            db->traiteErreurRequete(DetailsQuery,requete,"");
            RemplirTableWidget(TableDetails,"Actes", DetailsQuery, true, Qt::Checked);
        }
        break;
    }

        /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        -- Enregistrement d'un paiement direct -----------------------------------------------------------------------------------------------------------------------------------------------------
        ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    case EnregistrePaiementDirect:
        {
            /*
        1. Remplissage TableListe -- tous les paiemenst en attente en dehors de ceux de la salle d'attente
        càd ceux:
        . pour lesquels (aucune ligne de paiement n'a été enregistrée
        . OU le type de paiement enregistré est "impayé")
        . ET qui ne sont pas en salle d'attente en attente de paiement
        . ET dont le montant de l'acte n'est pas nul
        */
        DefinitArchitectureTableView(TableListe,ActesDirects);
        NettoieVerrousListeActesAAfficher();

        requete =
                    "SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, '' as TypePaiement, ActeMontant as ResteDu , SUM(Paye) as Regle FROM "
                    NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_LIGNESPAIEMENTS " lig\n"
                    " WHERE "
                    " act.idActe IN (SELECT idActe from (\n"
                    " SELECT  lig.idActe, SUM(Paye) as TotalPaye,ActeMontant  FROM " NOM_TABLE_LIGNESPAIEMENTS " as lig, " NOM_TABLE_ACTES " as act2\n"
                    " where lig.idActe= act2.idActe\n"
                    " group by idacte) as blue\n"
                    " where blue.acteMontant > blue.totalpaye and blue.totalpaye > 0)\n"
                    " AND act.idActe NOT IN (SELECT saldat.idActeAPayer FROM " NOM_TABLE_SALLEDATTENTE " saldat where saldat.idActeAPayer is not null)\n"
                    " AND act.idActe NOT IN (SELECT Typ.idActe FROM " NOM_TABLE_TYPEPAIEMENTACTES " Typ where Typ.TypePaiement = 'T')\n"
                    " AND act.idPat = pat.idPat\n"
                    " AND act.idActe = lig.idActe\n"
                    " AND ActeDate > AddDate(NOW(),-730)\n";
        requete +=  user;
        requete +=  " AND ActeMontant > 0\n"
                    " GROUP BY act.idActe\n";                        // tous les actes dont le paiement est incomplet

        requete +=  " UNION \n\n"
                    " SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, '' as TypePaiement, ActeMontant as ResteDu , 0 as Regle FROM "
                    NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat"
                    " WHERE "
                    " act.idacte not in (select idacte from " NOM_TABLE_LIGNESPAIEMENTS ")\n"
                    " AND act.idacte not in (select idacte from " NOM_TABLE_TYPEPAIEMENTACTES " WHERE TypePaiement = 'I' OR TypePaiement = 'T')\n"
                    " AND act.idActe NOT IN (SELECT saldat.idActeAPayer FROM " NOM_TABLE_SALLEDATTENTE " saldat where saldat.idActeAPayer is not null)\n"
                    " AND act.idPat = pat.idPat\n"
                    " AND ActeDate > AddDate(NOW(),-730)\n";
        requete +=  user;
        requete +=  " AND ActeMontant > 0\n"
                    " GROUP BY act.idActe\n";                        // tous les actes pour lesquels aucun renseignement de paiement n'a été enregistré

        requete +=  " UNION \n\n"
                    " SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, TypePaiement, ActeMontant as ResteDu, 0 as Regle"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ\n"
                    " WHERE typ.TypePaiement = 'I'\n"
                    " AND typ.idActe = act.idActe\n"
                    " AND act.idActe NOT IN (SELECT saldat.idActeAPayer FROM " NOM_TABLE_SALLEDATTENTE " saldat where saldat.idActeAPayer is not null)\n"
                    " AND act.idPat = pat.idPat\n"
                    " AND ActeDate > AddDate(NOW(),-730)\n";
        requete +=  user;
        requete +=  " ORDER BY ActeDate DESC, PatNom, PatPrenom";   // tous les actes impayés

        //proc->Edit(requete);
        QSqlQuery ListeActesQuery (requete,db->getDataBase());
        db->traiteErreurRequete(ListeActesQuery,requete,"");
        RemplirTableWidget(TableListe,"Actes", ListeActesQuery, true, Qt::Unchecked);

        //2. Remplissage TableSalDat
        DefinitArchitectureTableView(TableSalDat,ActesDirects);
        requete =   "SELECT idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, ActeMontant as CalculPaiement FROM \n"
                    NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat \n"
                    " WHERE idActe IN (SELECT saldat.idActeAPayer FROM " NOM_TABLE_SALLEDATTENTE " saldat) \n"
                    " AND act.idPat = pat.idPat \n";
        requete +=  user;
        requete +=  " AND ActeMontant > 0 \n"
                    " ORDER BY PatNom, PatPrenom";
        //proc->Edit(requete);
        QSqlQuery SalDatQuery (requete,db->getDataBase());
        db->traiteErreurRequete(SalDatQuery,requete,"");
        RemplirTableWidget(TableSalDat,"Actes", SalDatQuery, true, Qt::Unchecked);

        //3. Remplissage TableDetails
        DefinitArchitectureTableView(TableDetails,ActesDirects);
        if (gListidActe.size() > 0)
        {
            for (int i = 0; i < TableSalDat->rowCount();i++)
            {
                UpCheckBox* Check = dynamic_cast<UpCheckBox*>(TableSalDat->cellWidget(i,1));
                if(Check)
                {
                    if (Check->checkState() == Qt::Checked)
                    {
                        int R = Check->getRowTable();
                        CompleteDetailsTable(TableSalDat,R,Check->checkState());
                        PoseVerrouCompta(TableSalDat->item(i,0)->text().toInt());
                    }
                }
            }
            for (int i = 0; i < TableListe->rowCount();i++)
            {
                UpCheckBox* Check = dynamic_cast<UpCheckBox*>(TableListe->cellWidget(i,1));
                if(Check)
                {
                    if (Check->checkState()==Qt::Checked)
                    {
                        int R = Check->getRowTable();
                        CompleteDetailsTable(TableListe,R,Check->checkState());
                     }
                }
            }
          }
       break;
    }

        /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Modification d'un paiement direct -------------------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    case VoirListeActes:
    {
        // On sélectionne tous les actes sans exception, sauf les gratuits et les impayés
        DefinitArchitectureTableView(TableListe, ActesTiers);
        requete =   "select * from (\n"
                    "SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, SUM(Paye) as tot, TypePaiement, Tiers\n"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ, " NOM_TABLE_LIGNESPAIEMENTS " lig\n"
                    " WHERE act.idActe = typ.idActe\n"
                    " AND lig.idActe = act.idActe\n"
                    " AND TypePaiement NOT IN ('G','I')\n"
                    " AND act.idPat = pat.idPat\n"
                    " AND ActeDate > AddDate(NOW(),-730)\n";
        requete +=  user;
        requete +=  " group by act.idacte) as mar\n"
                    " union\n\n"

                    " (SELECT act.idActe, ActeDate, PatNom, PatPrenom, ActeCotation, ActeMontant, ActeMonnaie, 0 as tot, TypePaiement, Tiers\n"
                    " FROM " NOM_TABLE_ACTES " act, " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_TYPEPAIEMENTACTES " typ\n"
                    " WHERE act.idActe = typ.idActe\n"
                    " AND act.idacte not in (select idacte from " NOM_TABLE_LIGNESPAIEMENTS ")\n"
                    " AND TypePaiement NOT IN ('I')\n"
                    " AND ActeDate > AddDate(NOW(),-730)\n"
                    " AND act.idPat = pat.idPat\n";
        requete +=  " AND act.UserComptable = " + QString::number(gidUserACrediter) + ")\n";
        requete +=  " order by acteDate desc, PatNom, PatPrenom";

        //UpMessageBox::Watch(this,requete);

        QSqlQuery ListeActesQuery (requete,db->getDataBase());
        db->traiteErreurRequete(ListeActesQuery,requete,"");
        RemplirTableWidget(TableListe,"Actes", ListeActesQuery, false, Qt::Unchecked);
        if (TableListe->rowCount() > 0)
            connect (TableListe,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));

        DefinitArchitectureTableView(TableSalDat,Paiements);
        DefinitArchitectureTableView(TableDetails,ActesTiers);
        break;
    }

    /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Modification d'un paiement direct ou par tiers ------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    case VoirListePaiementsTiers:      // on reconstruit la liste des tiers
    {
        // On sélectionne tous les paiements sans exception
        DefinitArchitectureTableView(TableListe,Paiements);
        requete =   "SELECT idRecette, DatePaiement, DateEnregistrement, Montant, ModePaiement, TireurCheque, CompteVirement, BanqueCheque, TiersPayant, NomTiers, Commission, Monnaie, idRemise, EnAttente, EnregistrePar, TypeRecette, RCDate, Montant as Paye FROM " NOM_TABLE_RECETTES
                "\n LEFT OUTER JOIN (SELECT RCDate, idRemCheq FROM " NOM_TABLE_REMISECHEQUES ") AS rc\n"
                " ON rc.idRemCheq = idRemise\n"
                " WHERE idUser = " + QString::number(gidUserACrediter) +
                "\n AND TiersPayant = 'O'\n"
                " ORDER BY DatePaiement DESC, NomTiers";

        //UpMessageBox::Watch(this,requete);
        QSqlQuery ListePaiementsQuery (requete,db->getDataBase());
        db->traiteErreurRequete(ListePaiementsQuery,requete,"");
        RemplirTableWidget(TableListe,"Paiements", ListePaiementsQuery, false, Qt::Unchecked);

        ui->DetailupTableWidget->setRowCount(0);
        ui->DetailupTableWidget->setColumnCount(0);
        ui->DetailupTableWidget->horizontalHeader()->hide();
        connect (TableListe,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
        if (gMode == VoirListePaiementsTiers)
            DefinitArchitectureTableView(TableDetails,ActesTiers);
        else
            DefinitArchitectureTableView(TableDetails,ActesDirects);
        break;
     }
    default:
        break;

    }
    Slot_CalculTotalDetails();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Trie une table en remettant à jour les index des UpcheckBox et des UplineEdit ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::TrieListe(QTableWidget *TableATrier )
{
    int ncol = TableATrier->columnCount();
    int ColonneATrier;

    if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
        ColonneATrier = 3;
    else
        ColonneATrier = 2;

    if (TableATrier == ui->ListeupTableWidget)
    {
        if (gOrdreTri == Chronologique)
        {
            TableATrier->sortItems(ncol - 1,Qt::DescendingOrder);
            ui->OrdreupPushButton->setIcon(Icons::icTri());
            ui->OrdreupPushButton->setIconSize(QSize(20,20));
        }
        else
        {
            TableATrier->sortItems(ColonneATrier,Qt::AscendingOrder);
            ui->OrdreupPushButton->setIcon(Icons::icDate());
            ui->OrdreupPushButton->setIconSize(QSize(20,20));
        }
    }
    if (TableATrier == ui->DetailupTableWidget)
    {
        TableATrier->sortItems(ColonneATrier,Qt::AscendingOrder);
        TableATrier->sortItems(ncol - 1,Qt::DescendingOrder);
    }

    if (gMode == EnregistrePaiementDirect || gMode == EnregistrePaiementTiers)
    {
        int ColonneMontantPaye  = TableATrier->columnCount()- 2;
        int NombreRangees       = TableATrier->rowCount();
        for (int i= 0; i < NombreRangees; i++)
        {
            UpCheckBox* Check = dynamic_cast<UpCheckBox*>(TableATrier->cellWidget(i,1));
            //            QString nom = TableATrier->item(i,3)->text();
            if (Check)
            {
                disconnect(Check,       SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));    // il faut faire cette manip sinon on n'arrive pas à modifier le checkstate
                Check->setRowTable(i);
                connect(Check,          SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
            }
            if (TableATrier == ui->DetailupTableWidget)
            {
                UpLineEdit* Line = dynamic_cast<UpLineEdit*>(TableATrier->cellWidget(i,ColonneMontantPaye));
                if (Line)
                {
                    disconnect (Line,   SIGNAL(textChanged(QString)), this,     SLOT(Slot_CalculTotalDetails()));         // même remarque
                    Line->setRowTable(i);
                    connect (Line,      SIGNAL(textChanged(QString)), this,     SLOT(Slot_CalculTotalDetails()));
                }
            }
        }
    }
    }

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Verifie la cohérence d'un paiement -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiement::VerifCoherencePaiement()
{
    QString Msg = "";
    bool A          = true;
    bool AfficheMsg = true;
    bool Reponse    = true;

    while (Reponse) {
        // On n'a coché aucun mode de paiement
        if (ModifLigneRecettePossible &&
                (
                !(
                (ui->CarteCreditradioButton->isChecked()
                || ui->ChequeradioButton->isChecked()
                || ui->EspecesradioButton->isChecked()
                || ui->TiersradioButton->isChecked()
                || ui->ImpayeradioButton->isChecked()
                || ui->GratuitradioButton->isChecked())
                    &&  gMode == EnregistrePaiementDirect)
                &&
                !(
                (ui->ChequeradioButton->isChecked()
                || ui->EspecesradioButton->isChecked()
                || ui->VirementradioButton->isChecked())
                    &&  gMode == EnregistrePaiementTiers)
                )
           )
        {
            Msg = tr("Vous avez oublié de cocher le mode de paiement!");
            A = false;
            break;
        }
        // On a coché gratuit, impayé ou tiers et sélectionné aucun acte
        if (ui->DetailupTableWidget->rowCount() == 0 && (ui->GratuitradioButton->isChecked() || ui->ImpayeradioButton->isChecked() || ui->TiersradioButton->isChecked()))
        {
            Msg = tr("Vous n'avez enregistré aucun acte pour cet enregistrement!");
            A = false;
            break;
        }
        // Le montant inscrit n'est pas nul et l'acte est gratuit ou l'inverse
        if (ModifLigneRecettePossible && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 && ui->GratuitradioButton->isChecked()))
        {
            Msg = tr("Vous avez enregistré un montant supérieur à 0 pour un acte gratuit!");
            ui->MontantlineEdit->setFocus();
            A = false;
            break;
        }
        if (ModifLigneRecettePossible && (ui->MontantlineEdit->isVisible() && !(QLocale().toDouble(ui->MontantlineEdit->text()) > 0) && !(ui->GratuitradioButton->isChecked())))
        {
            Msg = tr("Vous avez oublié d'indiquer le montant!");
            ui->MontantlineEdit->setFocus();
            A = false;
            break;
        }
        // Le montant inscrit est supérieur au montant calculé
        if (ui->MontantlineEdit->isVisible() && (QLocale().toDouble(ui->MontantlineEdit->text()) > QLocale().toDouble(ui->TotallineEdit->text())))
        {
            if (ui->DetailupTableWidget->rowCount() > 0 && !ui->ImpayeradioButton->isChecked())
                Msg = tr("Le montant enregistré est supérieur au montant total calculé!\nEnregistrer quand même?");
            else if (ui->DetailupTableWidget->rowCount() == 0)
                Msg = tr("Il n'y a aucun acte enregistré!\nEnregistrer quand même?");
            else break;
            UpMessageBox msgbox;
            msgbox.setText(tr("Montants différents!"));
            msgbox.setInformativeText(Msg);
            msgbox.setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton(tr("Enregistrer"));
            UpSmallButton NoBouton(tr("Non"));
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != &OKBouton)
            {
                A = false;
                AfficheMsg = false;
                break;
            }
        }
        // Le montant inscrit est inférieur au montant calculé
        if (ui->MontantlineEdit->isVisible() && (QLocale().toDouble(ui->MontantlineEdit->text()) < QLocale().toDouble(ui->TotallineEdit->text())))
        {
            Msg = tr("Le montant versé est différent de la somme calculée!");
            ui->MontantlineEdit->setFocus();
            A = false;
            break;
        }
        // On a coché Chèque et on a oublié le nom de la banque
        if (ModifLigneRecettePossible && (ui->BanqueChequecomboBox->currentText() == "" && ui->ChequeradioButton->isChecked()))
        {
            Msg = tr("Il manque le nom de la banque!");
            ui->BanqueChequecomboBox->setFocus();
            ui->BanqueChequecomboBox->showPopup();
            A = false;
            break;
        }
        // On a coché Chèque et on vérifie que le nom de la banque est enregistré
        if (ModifLigneRecettePossible
                && (ui->BanqueChequecomboBox->currentText() != ""
                && ui->ChequeradioButton->isChecked())
                && ui->BanqueChequecomboBox->findText(ui->BanqueChequecomboBox->currentText()) == -1)
        {
            A = false;
            AfficheMsg = false;
            QString Banq = ui->BanqueChequecomboBox->currentText();
            QString Msg2 = Banq + "\n" + tr("Cette banque est inconnue!\nVoulez vous l'enregistrer?");
            UpMessageBox msgbox;
            msgbox                      .setText(tr("Banque inconnue!"));
            msgbox                      .setInformativeText(Msg2);
            msgbox                      .setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton(tr("Enregistrer"));
            UpSmallButton NoBouton(tr("Ne pas enregistrer"));
            UpSmallButton OupsBouton(tr("Annuler et revenir à la saisie"));
            msgbox.addButton(&OupsBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&NoBouton,   UpSmallButton::DONOTRECORDBUTTON);
            msgbox.addButton(&OKBouton,   UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() == &OKBouton)
            {
                Dlg_Banq = new dlg_banque(this, Banq.toUpper());
                if (Dlg_Banq->exec()>0)
                {
                    ReconstruitListeBanques();
                    ui->BanqueChequecomboBox->setCurrentText(Banq);
                    A = true;
                }
                else if (UpMessageBox::Question(this, tr("Continuer sans enregistrer la banque"), ui->BanqueChequecomboBox->currentText() + " ?")
                         == UpSmallButton::STARTBUTTON)
                    A = true;
            }
            else if (msgbox.clickedButton() == &NoBouton)
                A = true;
            if (!A)
                break;
        }

        // On a coché Chèque et on a oublié de noter le nom du tireur
        if (ModifLigneRecettePossible && (ui->TireurChequelineEdit->text() == "" && ui->ChequeradioButton->isChecked()))
        {
            Msg = tr("Il manque le nom du tireur du chèque!");
            ui->TireurChequelineEdit->setFocus();
            A = false;
            break;
        }
        // On a coché Tiers et on a oublié de choisir le Tiers
        if (ModifLigneRecettePossible && (ui->TypeTierscomboBox->currentText() == "" && ui->TiersradioButton->isChecked()))
        {
            Msg = tr("Il manque le type de tiers payant!");
            ui->TypeTierscomboBox->setFocus();
            ui->TypeTierscomboBox->showPopup();
            A = false;
            break;
        }
        if (ModifLigneRecettePossible
                && (
                    ui->TierscomboBox->currentText() == ""
                    && gMode == EnregistrePaiementTiers
                    && (ui->VirementradioButton->isChecked() || ui->ChequeradioButton->isChecked())
                   )
           )
        {
            Msg = tr("Il manque le nom du tiers payant!");
            ui->TierscomboBox->setFocus();
            ui->TierscomboBox->showPopup();
            A = false;
            break;
        }
        // On a coché Virement, le tiers est carte de crédit et on a oublié de renseigner la commission
        if (ModifLigneRecettePossible && (ui->TierscomboBox->currentText() == "CB" && QLocale().toDouble(ui->CommissionlineEdit->text()) ==  0.0 && ui->VirementradioButton->isChecked()))
        {
            Msg = tr("Vous avez oublié de mentionner le montant de la comission bancaire pour ce paiement par carte de crédit!");
            ui->Commissionlabel->setFocus();
            A = false;
            break;
        }
        // Le montant de la commission dépasse le montant du paiement commission
        if (ModifLigneRecettePossible && (QLocale().toDouble(ui->CommissionlineEdit->text()) > QLocale().toDouble(ui->MontantlineEdit->text())))
        {
            Msg = tr("Vous avez mentionné un montant pour la comission bancaire supérieur au montant du paiement!");
            ui->Commissionlabel->setFocus();
            A = false;
            break;
        }
        // On n'a enregistré aucune ligne correspondant à un acte
        if (ui->DetailupTableWidget->rowCount() == 0)
        {
            Msg = tr("Vous n'avez enregistré aucun acte\npour cette recette!\nConfirmez vous la saisie?");
            UpMessageBox msgbox;
            msgbox      .setText(tr("Aucun acte enregistré!"));
            msgbox      .setInformativeText(Msg);
            msgbox      .setIcon(UpMessageBox::Warning);
            UpSmallButton OKBouton(tr("Je confirme"));
            UpSmallButton NoBouton(tr("Annuler"));
            msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
            msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
            msgbox.exec();
            if (msgbox.clickedButton() != &OKBouton)
            {
                A = false;
                AfficheMsg = false;
                break;
            }
        }
        Reponse = false;
    }
    if (!A && AfficheMsg) UpMessageBox::Watch(this,Msg);
    return A;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Verifie si l'enregistrement coché est verrouillé par un autre utilisateur ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiement::VerifVerrouCompta(QTableWidget *TableAVerifier, int Rangee)
{
    if (gtimerAfficheActeVerrouilleClignotant->isActive())
        return false;
    QString ChercheVerrou = "SELECT UserLogin FROM " NOM_TABLE_VERROUCOMPTAACTES ", " NOM_TABLE_UTILISATEURS
                     " WHERE idActe = "  + TableAVerifier->item(Rangee,0)->text() +
                     " AND PosePar = idUser";
    //UpMessageBox::Watch(this,ChercheVerrou);
    QSqlQuery ChercheVerrouQuery (ChercheVerrou,db->getDataBase());
    db->traiteErreurRequete(ChercheVerrouQuery, ChercheVerrou,"");
    if (ChercheVerrouQuery.size() > 0)
    {
        ChercheVerrouQuery.first();
        ui->VerrouilleParlabel->setText(tr("Acte Verrouillé par ") + ChercheVerrouQuery.value(0).toString());
        ui->VerrouilleParlabel->setGeometry(200,190,250,16);
        ui->VerrouilleParlabel->setStyleSheet("color: magenta");
        ui->VerrouilleParlabel->setVisible(true);
        gtimerAfficheActeVerrouilleClignotant->start(100);
        connect(gtimerAfficheActeVerrouilleClignotant, SIGNAL(timeout()),this,SLOT(Slot_AfficheActeVerrouilleClignotant()));
        gtimerAfficheActeVerrouille->start(2000);
        gtimerAfficheActeVerrouille->setSingleShot(true);
        if (gMode == EnregistrePaiementDirect)
            for (int i= 0; i != ui->SalleDAttenteupTableWidget->rowCount(); i++)
            {
                UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->SalleDAttenteupTableWidget->cellWidget(i,1));
                if (Check)
                    Check->setToggleable(false);
            }
        for (int i= 0; i != ui->ListeupTableWidget->rowCount(); i++)
        {
            UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->ListeupTableWidget->cellWidget(i,1));
            if (Check)
                Check->setToggleable(false);
        }
        QSound::play(NOM_ALARME);
        connect(gtimerAfficheActeVerrouille, SIGNAL(timeout()),this,SLOT(Slot_AfficheActeVerrouille()));
        return false;
    }
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Retirer de la table Details un élément décoché et le décocher dans la table Liste ou SalDat où il figure -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiement::VideDetailsTable(int Rangee)
{
    bool                    idActeTrouve = false;
    QTableWidget            *TableDetails = ui->DetailupTableWidget;
    QTableWidget            *TableListe = ui->ListeupTableWidget;
    QTableWidget            *TableSalDat = ui->SalleDAttenteupTableWidget;

    // on récupère l'idActe de l'item concerné
    QString ActeAVirer = TableDetails->item(Rangee,0)->text();
    //UpMessageBox::Watch(this,TableDetails->item(Rangee,3)->text()+"\n"+ActeAVirer);
    //on décoche les items correspondants dans Tableliste et TableSalDat
    if (gMode == EnregistrePaiementDirect)
    {
        QList<QTableWidgetItem*> items;
        items = TableSalDat->findItems(ActeAVirer,Qt::MatchExactly);
        for (int j = 0; j < items.size(); j++)
        {
            int i = items.at(j)->row();
            idActeTrouve = true;
            UpCheckBox *CheckItem = new UpCheckBox;
            CheckItem->setCheckState(Qt::Unchecked);
            CheckItem->setRowTable(i);
            CheckItem->setFocusPolicy(Qt::NoFocus);
            connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
            CheckItem->installEventFilter(this);
            TableSalDat->removeCellWidget(i,1);
            TableSalDat->setCellWidget(i,1,CheckItem);
            RetireVerrouCompta(ActeAVirer.toInt());
        }
    }
    if (!idActeTrouve)
    {
        QList<QTableWidgetItem*> items;
        items = TableListe->findItems(ActeAVirer,Qt::MatchExactly);
        for (int j = 0; j < items.size(); j++)
        {
            int i = items.at(j)->row();
            idActeTrouve = true;
            UpCheckBox *CheckItem = new UpCheckBox;
            CheckItem->setCheckState(Qt::Unchecked);
            CheckItem->setRowTable(i);
            CheckItem->setFocusPolicy(Qt::NoFocus);
            connect(CheckItem,      SIGNAL(uptoggled(bool)),      this,       SLOT (Slot_RenvoieRangee(bool)));
            CheckItem->installEventFilter(this);
            TableListe->removeCellWidget(i,1);
            TableListe->setCellWidget(i,1,CheckItem);
            RetireVerrouCompta(ActeAVirer.toInt());
        }
    }
    if (!idActeTrouve) RemplitLesTables(gMode);
    // on supprime la rangée de la tableDetails et on reindexe les upcheckbox et les uplinetext
    TableDetails->removeRow(Rangee);
    TrieListe(ui->DetailupTableWidget);
    Slot_CalculTotalDetails();
    RegleAffichageTypePaiementframe();
}

