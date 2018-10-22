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

#include "dlg_recettesspeciales.h"
#include "gbl_datas.h"
#include "icons.h"
#include "ui_dlg_recettesspeciales.h"
#include "cls_compte.h"

dlg_recettesspeciales::dlg_recettesspeciales(Procedures *procAPasser, QWidget *parent) :
    QDialog(parent), ui(new Ui::dlg_recettesspeciales)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    proc        = procAPasser;
    m_db        = DataBase::getInstance();
    db          = m_db->getDataBase();

    ui->Userlabel->setText(tr("Recettes spéciales de ") + m_db->getUserConnected()->getLogin());

    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionDepenses").toByteArray());

    InitOK = initializeUserSelected();
    if( !InitOK )
        return;

    setMaximumSize(1642,800);

    gBigTable       = new UpTableWidget(this);
    ui->horizontalLayout_3->addWidget(gBigTable);
    EnregupPushButton = new UpPushButton(ui->frame);
    EnregupPushButton->setGeometry(155,285,150,50);
    EnregupPushButton->setText("Valider");
    EnregupPushButton->setIcon(Icons::icOK());
    EnregupPushButton->setIconSize(QSize(30,30));
    EnregupPushButton->setVisible(true);

    AnnulupPushButton = new UpPushButton(ui->frame);
    AnnulupPushButton->setGeometry(5,285,150,50);
    AnnulupPushButton->setText("Annuler");
    AnnulupPushButton->setIcon(Icons::icAnnuler());
    AnnulupPushButton->setIconSize(QSize(30,30));
    AnnulupPushButton->setVisible(true);

    ui->frame->setStyleSheet("QFrame#frame{border: 1px solid gray; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 210, 210, 50));}");

    QString req = "SELECT idBanqueAbrege, idBanque FROM " NOM_TABLE_BANQUES " ORDER by idBanqueAbrege";
    QSqlQuery ListBanquesQuery (req,db);
    m_db->traiteErreurRequete(ListBanquesQuery,req,"");
    ListBanquesQuery.first();
    do
        ui->BanqChequpComboBox->addItem(ListBanquesQuery.value(0).toString(),ListBanquesQuery.value(1).toInt());
    while (ListBanquesQuery.next());

    //TODO : SQL
    QStringList ListeRubriques;
    ListeRubriques << tr("Apport praticien") << tr("Divers et autres recettes");
    ui->RefFiscalecomboBox->insertItems(0,ListeRubriques);
    ui->RefFiscalecomboBox->setCurrentText(ListeRubriques.at(0));

    glistMoyensDePaiement << NOM_CHEQUE;
    glistMoyensDePaiement << NOM_ESPECES;
    glistMoyensDePaiement << NOM_VIREMENT;
    ui->PaiementcomboBox->insertItems(0,glistMoyensDePaiement );
    ui->PaiementcomboBox->setCurrentText(glistMoyensDePaiement.at(0));

    QDoubleValidator *val= new QDoubleValidator(this);
    val->setDecimals(2);
    ui->MontantlineEdit->setValidator(val);

    QList<UpPushButton *> allUpButtons = this->findChildren<UpPushButton *>();
    for (int n = 0; n <  allUpButtons.size(); n++)
        allUpButtons.at(n)->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Mid);
    ui->GestionComptesupPushButton->resize(290,48);

    DefinitArchitectureBigTable();

    ReconstruitListeAnnees();

    connect (ui->GestionComptesupPushButton,    &QPushButton::clicked,          [=] {GestionComptes();});
    connect (ui->NouvelleRecetteupPushButton,   &QPushButton::clicked,          [=] {GererRecette(ui->NouvelleRecetteupPushButton);});
    connect (ui->ModifierupPushButton,          &QPushButton::clicked,          [=] {GererRecette(ui->ModifierupPushButton);});
    connect (ui->OKupPushButton,                &QPushButton::clicked,          [=] {accept();});
    connect (ui->MontantlineEdit,               &QLineEdit::editingFinished,    [=] {ConvertitDoubleMontant();});
    connect (ui->PaiementcomboBox,              QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                [=](int) {ChoixPaiement();});
    connect (ui->ObjetlineEdit,                 &QLineEdit::textEdited,         [=] {EnableModifiepushButton();});
    connect (ui->MontantlineEdit,               &QLineEdit::textEdited,         [=] {EnableModifiepushButton();});
    connect (ui->DateRecdateEdit,               &QDateEdit::dateChanged,        [=] {EnableModifiepushButton();});
    connect (ui->PaiementcomboBox,              &QComboBox::currentTextChanged, [=] {EnableModifiepushButton();});
    connect (ui->RefFiscalecomboBox,            &QComboBox::currentTextChanged, [=] {EnableModifiepushButton();});
    connect (ui->SupprimerupPushButton,         &QPushButton::clicked,          [=] {SupprimerRecette();});

    connect (EnregupPushButton,                 &QPushButton::clicked,          [=] {gMode == Enregistrer? EnregistreRecette() : ModifierRecette();});
    connect (AnnulupPushButton,                 &QPushButton::clicked,          [=] {AnnulEnreg();});

    QString year = QDate::currentDate().toString("yyyy");
    int idx = ui->AnneecomboBox->findText(year);
    ui->AnneecomboBox->setCurrentIndex(idx==-1? ui->AnneecomboBox->count()-1 : idx);
    connect (ui->AnneecomboBox,                 QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {RedessineBigTable();});
    RedessineBigTable();

    gBigTable->setFocus();

    RegleComptesComboBox(false);

    setFixedWidth(gBigTable->width() + ui->frame->width() + layout()->margin()*3);
    InitOK = true;
}

void    dlg_recettesspeciales::RegleAffichageFiche(enum gMode mode)
{
    gMode = mode;
    ui->DateRecdateEdit     ->setVisible(gMode != TableVide);
    ui->ObjetlineEdit       ->setVisible(gMode != TableVide);
    ui->MontantlineEdit     ->setVisible(gMode != TableVide);
    ui->PaiementcomboBox    ->setVisible(gMode != TableVide);

    ui->BanqChequpComboBox  ->setVisible(gMode != TableVide);
    ui->Tireurlabel         ->setVisible(gMode != TableVide);
    ui->Comptelabel         ->setVisible(gMode != TableVide);
    ui->BanqueChequelabel   ->setVisible(gMode != TableVide);
    ui->TireurlineEdit      ->setVisible(gMode != TableVide);
    ui->ComptesupComboBox   ->setVisible(gMode != TableVide);
    ui->RefFiscalecomboBox  ->setVisible(gMode != TableVide);
    ui->DateRecettelabel    ->setVisible(gMode != TableVide);
    ui->Objetlabel          ->setVisible(gMode != TableVide);
    ui->Montantlabel        ->setVisible(gMode != TableVide);
    ui->Paiementlabel       ->setVisible(gMode != TableVide);
    ui->RefFiscalelabel     ->setVisible(gMode != TableVide);

    ui->DateRecdateEdit     ->setEnabled(gMode != Lire);
    ui->ObjetlineEdit       ->setEnabled(gMode != Lire);
    ui->MontantlineEdit     ->setEnabled(gMode != Lire);
    ui->Tireurlabel         ->setEnabled(gMode != Lire);
    ui->BanqueChequelabel   ->setEnabled(gMode != Lire);
    ui->TireurlineEdit      ->setEnabled(gMode != Lire);
    ui->PaiementcomboBox    ->setEnabled(gMode != Lire);
    ui->BanqChequpComboBox  ->setEnabled(gMode != Lire);
    ui->ComptesupComboBox   ->setEnabled(gMode != Lire);
    ui->Comptelabel         ->setEnabled(gMode != Lire);
    ui->RefFiscalecomboBox  ->setEnabled(gMode != Lire);
    ui->DateRecettelabel    ->setEnabled(gMode != Lire);
    ui->Objetlabel          ->setEnabled(gMode != Lire);
    ui->Montantlabel        ->setEnabled(gMode != Lire);
    ui->Paiementlabel       ->setEnabled(gMode != Lire);
    ui->RefFiscalelabel     ->setEnabled(gMode != Lire);
    ui->OKupPushButton      ->setEnabled(gMode == Lire || gMode == TableVide);
    ui->GestionComptesupPushButton  ->setEnabled(gMode == Lire || gMode == TableVide);
    ui->SupprimerupPushButton       ->setVisible(gMode == Lire);
    ui->ModifierupPushButton        ->setVisible(gMode == Lire);
    int sz = gDataUser->getComptes()->comptes().size();
    ui->NouvelleRecetteupPushButton ->setEnabled((gMode == Lire || gMode == TableVide) && sz>0);
    ui->NouvelleRecetteupPushButton->setToolTip((gMode == Lire || gMode == TableVide) && sz>0? "" : tr("Vous ne pouvez pas enregistrer de recettes.\nAucun compte bancaire n'est enregistré."));
    EnregupPushButton       ->setVisible(!(gMode == Lire || gMode == TableVide));
    AnnulupPushButton       ->setVisible(!(gMode == Lire || gMode == TableVide));
    gBigTable               ->setEnabled(gMode == Lire);
    if (gBigTable->rowCount()==0 && gMode== Enregistrer)
    {
        ui->TireurlineEdit->setVisible(false);
        ui->Tireurlabel->setVisible(false);
        ui->BanqChequpComboBox->setVisible(false);
        ui->BanqueChequelabel->setVisible(false);
        ui->PaiementcomboBox->setCurrentText(NOM_VIREMENT);
    }
    RegleComptesComboBox(gMode==Enregistrer);
    MetAJourFiche();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Clic sur AnnulupPushButton ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::AnnulEnreg()
{
    RegleAffichageFiche(gBigTable->rowCount()>0? Lire : TableVide);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur courant ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_recettesspeciales::initializeUserSelected()
{
    gDataUser = m_db->getUserConnected();

    Comptes *comptes = new Comptes();
    comptes->addCompte( m_db->loadComptesByUser(gDataUser->id()) );
    gDataUser->setComptes( comptes );
    if( gDataUser->getComptes()->comptesAll().size() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de continuer!"), tr("Pas de compte bancaire enregistré pour ") + gDataUser->getLogin());
        return false;
    }

    QJsonObject data = m_db->loadUserData(gDataUser->id());
    if(data.isEmpty())
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche recettes spéciales!"), tr("Les paramètres de ")
                             + gDataUser->getLogin() + tr("ne sont pas retrouvés"));
        return false;
    }
    gDataUser->setData( data ); //ON charge le reste des données

    if (gDataUser->getIdCompteParDefaut() <= 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche recettes spéciales!"), tr("Pas de compte bancaire enregistré pour ")
                                     + gDataUser->getLogin());
        return false;
    }

    return true;
}

void dlg_recettesspeciales::ChoixPaiement()
{
    ui->BanqChequpComboBox  ->setVisible(ui->PaiementcomboBox->currentText() == NOM_CHEQUE);
    ui->BanqueChequelabel   ->setVisible(ui->PaiementcomboBox->currentText() == NOM_CHEQUE);
    ui->Tireurlabel         ->setVisible(ui->PaiementcomboBox->currentText() == NOM_CHEQUE);
    ui->TireurlineEdit      ->setVisible(ui->PaiementcomboBox->currentText() == NOM_CHEQUE);
    ui->ComptesupComboBox   ->setVisible(ui->PaiementcomboBox->currentText() == NOM_VIREMENT || ui->PaiementcomboBox->currentText() == NOM_ESPECES);
    ui->Comptelabel         ->setVisible(ui->PaiementcomboBox->currentText() == NOM_VIREMENT || ui->PaiementcomboBox->currentText() == NOM_ESPECES);
}

void dlg_recettesspeciales::ConvertitDoubleMontant()
{
    QString b = ui->MontantlineEdit->text();
    b = QLocale().toString(QLocale().toDouble(b),'f',2);
    ui->MontantlineEdit->setText(b);
}

void dlg_recettesspeciales::EnableModifiepushButton()
{
    ui->ModifierupPushButton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::EnregistreRecette()
{
    bool OnSauteLaQuestionSuivante = false;
    QString pb = "";
    QString Paiement;
    Paiement = ui->PaiementcomboBox->currentText();

    if (!VerifSaisie())
        return;

    // vérifier que cette recette n'a pas été déjà saisie
    QString requete = "select DateRecette from " NOM_TABLE_RECETTESSPECIALES " where DateRecette = '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") +
            "'and Libelle = '"  + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
            "'and Montant = "   + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
            " and idUser = "    + QString::number(gDataUser->id());
    QSqlQuery ChercheRecQuery (requete,db);
    if (ChercheRecQuery.size() > 0)
    {
        pb = tr("Elle a déjà été saisie");
        OnSauteLaQuestionSuivante = true;
    }

    if (!OnSauteLaQuestionSuivante)
    {
        if (QDate::currentDate() > ui->DateRecdateEdit->date().addDays(90))
            pb = tr("Elle date de plus de 3 mois");
        requete = "select DateRecette from " NOM_TABLE_RECETTESSPECIALES " where DateRecette > '" + ui->DateRecdateEdit->date().addDays(-180).toString("yyyy-MM-dd") +
                "'and Libelle = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                "'and Montant = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                " and idUser = " + QString::number(gDataUser->id());
        QSqlQuery ChercheDep2Query (requete,db);
        if (ChercheDep2Query.size() > 0)
        {
            ChercheDep2Query.last();
            if (pb != "")
                pb += "\n";
            pb += tr("Une recette semblable a été saisie le ") + ChercheDep2Query.value(0).toDate().toString("dd MMM yyyy");
        }
    }

    if (pb != "")
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Il y a un problème avec cette recette!" ));
        msgbox.setInformativeText(pb + "\n" + tr("Confirmer la saisie?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton(tr("Confirmer"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            return;
    }

    // Insertion de l'écriture dans la table autresrecettes
    QString idRec, m;
    if (Paiement == NOM_ESPECES)              m = "E";
    else if (Paiement == NOM_VIREMENT)        m = "V";
    else if (Paiement == NOM_CHEQUE)          m = "C";

    QStringList listtables;
    listtables << NOM_TABLE_RECETTESSPECIALES << NOM_TABLE_ARCHIVESBANQUE << NOM_TABLE_LIGNESCOMPTES;
    if (!m_db->locktables(listtables))
        return;
    QString insertRecrequete = "insert into " NOM_TABLE_RECETTESSPECIALES " (DateRecette, idUser, Libelle, Montant, TypeRecette, Paiement, CompteVirement, TireurCheque, BanqueCheque)"
            " VALUES ('" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") +
            "', " + QString::number(gDataUser->id()) +
            ", '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
            "', " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
            ", '" + proc->CorrigeApostrophe(ui->RefFiscalecomboBox->currentText()) +
            "', '" + m +
            "', " + (m=="V"||m=="E"? ui->ComptesupComboBox->currentData().toString()    : "null") +
            ", "  + (m=="C"? "'" + ui->TireurlineEdit->text() + "'"  : "null") +
            ", "  + (m=="C"? "'" + ui->BanqChequpComboBox->currentText() + "'"  : "null") + ")";

    QSqlQuery EnregistreDepenseQuery (insertRecrequete,db);
    if (m_db->traiteErreurRequete(EnregistreDepenseQuery,insertRecrequete,tr("Impossible d'enregister cete recette!")))
    {
        m_db->rollback();
        return;
    }

    requete     = "select max(idRecette) from " NOM_TABLE_RECETTESSPECIALES;
    QSqlQuery   ChercheidDepQuery (requete,db);
    ChercheidDepQuery.first();
    idRec       = ChercheidDepQuery.value(0).toString();

    // insertion de l'écriture dans la table lignescomptes quand il s'agit d'un virement ou d'un dépôt d'espèces
    if (Paiement == NOM_VIREMENT || Paiement == NOM_ESPECES)
    {
        Paiement = (Paiement == NOM_VIREMENT? tr("Virement crébiteur") : tr("Dépôt espèces"));
        int a = proc->getMAXligneBanque();
        requete = "insert into " NOM_TABLE_LIGNESCOMPTES "(idLigne, idCompte, idRecSpec, LigneDate, Lignelibelle, LigneMontant, LigneDebitCredit, LigneTypeoperation) VALUES (" +
                    QString::number(a) + "," +
                    ui->ComptesupComboBox->currentData().toString() +
                    "," + idRec +
                    ", '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") +
                    "', '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                    "', "  + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                    ", 1, '" + Paiement + "')";
        QSqlQuery EnregLigneCompeQuery(requete,db);
        if (m_db->traiteErreurRequete(EnregLigneCompeQuery,requete,tr("Transaction annulée!\nImpossible d'enregister cette opération sur le compte bancaire!")))
        {
                m_db->rollback();
                return;
        }
    }
    m_db->commit();

    gBigTable->setEnabled(true);
    ui->SupprimerupPushButton->setVisible(true);
    ui->ModifierupPushButton->setVisible(true);
    ui->NouvelleRecetteupPushButton->setEnabled(true);

    int annee =  ui->DateRecdateEdit->date().year();
    if (ui->AnneecomboBox->currentText() != QString::number(annee))
    {
        if (ui->AnneecomboBox->findText(QString::number(annee)) == -1)
            ReconstruitListeAnnees();
        ui->AnneecomboBox->disconnect();
        ui->AnneecomboBox->setCurrentText(QString::number(annee));
        connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {RedessineBigTable();});
    }
    RedessineBigTable();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Gerer une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::GererRecette(QPushButton *widgsender)
{
    if (widgsender == ui->ModifierupPushButton)
    {
        if (gMode == Lire)
            RegleAffichageFiche(Modifier);
        else
            RegleAffichageFiche(gBigTable->rowCount()>0? Lire : TableVide);
    }
    else
        RegleAffichageFiche(Enregistrer);
}

void dlg_recettesspeciales::MenuContextuel(UpLabel *labelClicked)
{
    int idRecAOuvrir = labelClicked->getId();

    QMenu *menu;
    menu = new QMenu(this);

    QAction *pAction_RecopieDep = menu->addAction(tr("Effectuer une copie de cette recette à la date d'aujourd'hui"));
    connect (pAction_RecopieDep, &QAction::triggered,    [=] {ChoixMenu(QString::number(idRecAOuvrir));});
    QAction *pAction_ChercheVal = menu->addAction(tr("Rechercher une valeur"));
    connect (pAction_ChercheVal, &QAction::triggered,    [=] {ChoixMenu("ChercheVal");});

    // ouvrir le menu
    menu->exec(cursor().pos());
}

void dlg_recettesspeciales::ChoixMenu(QString choix)
{
    if (choix == "ChercheVal")
    {
        QMessageBox msgbox;
        msgbox.setText("...");
        msgbox.setDetailedText("...");
        msgbox.setInformativeText(tr("Entrez la valeur à rechercher..."));
        msgbox.setTextInteractionFlags(Qt::TextEditable);
        UpSmallButton OKBouton;
        OKBouton.setUpButtonStyle(UpSmallButton::STARTBUTTON);
        msgbox.addButton(&OKBouton, QMessageBox::AcceptRole);
        msgbox.exec();
        proc->EnChantier();
    }
    else
    {
        RegleAffichageFiche(Enregistrer);
        gMode = Enregistrer;
        ui->DateRecdateEdit             ->setEnabled(true);
        ui->ObjetlineEdit               ->setEnabled(true);
        ui->MontantlineEdit             ->setEnabled(true);
        ui->PaiementcomboBox            ->setEnabled(true);
        ui->BanqChequpComboBox          ->setEnabled(true);
        ui->BanqueChequelabel           ->setEnabled(true);
        ui->Comptelabel                 ->setEnabled(true);
        ui->ComptesupComboBox           ->setEnabled(true);
        ui->RefFiscalecomboBox          ->setEnabled(true);
        ui->DateRecettelabel            ->setEnabled(true);
        ui->Objetlabel                  ->setEnabled(true);
        ui->Montantlabel                ->setEnabled(true);
        ui->Paiementlabel               ->setEnabled(true);
        ui->RefFiscalelabel             ->setEnabled(true);
        ui->OKupPushButton              ->setEnabled(false);
        ui->GestionComptesupPushButton  ->setEnabled(false);
        ui->SupprimerupPushButton       ->setVisible(false);
        ui->ModifierupPushButton        ->setVisible(false);
        ui->NouvelleRecetteupPushButton ->setEnabled(false);
        EnregupPushButton               ->setVisible(true);
        AnnulupPushButton               ->setVisible(true);
        gBigTable                       ->setEnabled(false);
        gBigTable->disconnect();
        ui->DateRecdateEdit             ->setDate(QDate::currentDate());
        EnregupPushButton               ->setText("Enregistrer");
        ui->OKupPushButton              ->setShortcut(QKeySequence());
        ui->ModifierupPushButton        ->setShortcut(QKeySequence());
        EnregupPushButton               ->setShortcut(QKeySequence("Meta+Return"));
    }
}

void dlg_recettesspeciales::RegleComptesComboBox(bool ActiveSeult)
{
    ui->ComptesupComboBox->clear();

    QMultiMap<int, Compte*> model = (ActiveSeult? gDataUser->getComptes()->comptes() : gDataUser->getComptes()->comptesAll());
    for( QMultiMap<int, Compte*>::const_iterator itCompte = model.constBegin(); itCompte != model.constEnd(); ++itCompte )
       ui->ComptesupComboBox->addItem(itCompte.value()->nom(), QString::number(itCompte.value()->id()) );
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Supprimer une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::SupprimerRecette()
{
    if (gBigTable->selectedRanges().size() == 0) return;
    // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte
    QString VerifArchivesrequete = " select idRecSpec from " NOM_TABLE_ARCHIVESBANQUE " where idRecSpec = " + QString::number(idRecEnCours);
    QSqlQuery VerifArchivQuery (VerifArchivesrequete,db);
    if (VerifArchivQuery.size() > 0)
    {
        UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
        return;
    }

    QString VerifRemiserequete = " select idRemise from " NOM_TABLE_RECETTESSPECIALES " where idRecette = " + QString::number(idRecEnCours);
    QSqlQuery VerifRemiseQuery (VerifRemiserequete,db);
    if (VerifRemiseQuery.size() > 0)
    {
        VerifRemiseQuery.first();
        if (VerifRemiseQuery.value(0).toInt()>0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
            return;
        }
    }

    QDate   Dateop;
    QString Libelle;
    double  montant;
    QString requete1 = "select DateRecette, Libelle, Montant from " NOM_TABLE_RECETTESSPECIALES " where idrecette = " + QString::number(idRecEnCours);
    QSqlQuery query1 (requete1,db);
    if (query1.size() > 0)
    {
        query1.first();
        Dateop = query1.value(0).toDate();
        Libelle = query1.value(1).toString();
        montant = query1.value(2).toDouble();
        QString requete2 = "select idligne from " NOM_TABLE_ARCHIVESBANQUE " where LigneDate = '" + Dateop.toString("yyyy-MM-dd")
                + "' and LigneLibelle = '" + proc->CorrigeApostrophe(Libelle) + "' and LigneMontant = " + QString::number(montant);
        QSqlQuery query2 (requete2,db);
        if (query2.size()> 0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
            return;
        }
        QString daterec = Dateop.toString("dd MMM yyyy");
        UpMessageBox msgbox;
        msgbox.setText(tr("Supprimer une recette!"));
        msgbox.setInformativeText(tr("Confirmer la suppression de\n") + daterec + "\n" + Libelle + "\n" + QLocale().toString(montant,'f',2) + "?");
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton(tr("Supprimer"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            return;

        //On supprime l'écriture
        QString SupprComptesrequete = "delete from " NOM_TABLE_LIGNESCOMPTES " where idrecspec = " + QString::number(idRecEnCours);
        QSqlQuery (SupprComptesrequete,db);

        QString SupprDepenserequete = "delete from " NOM_TABLE_RECETTESSPECIALES " where idrecette = " + QString::number(idRecEnCours);
        QSqlQuery (SupprDepenserequete,db);

        if (gBigTable->rowCount() == 1)
        {
            ui->AnneecomboBox->disconnect();
            RedessineBigTable();
            ReconstruitListeAnnees();
            QString year = QDate::currentDate().toString("yyyy");
            int idx = ui->AnneecomboBox->findText(year);
            ui->AnneecomboBox->setCurrentIndex(idx==-1? ui->AnneecomboBox->count()-1 : idx);
            connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {RedessineBigTable();});
        }
        else
        {
            for (int i = 0; i< gBigTable->rowCount(); i++)
            {
                UpLabel *iddeplbl = static_cast<UpLabel *>(gBigTable->cellWidget(i,0));
                if (iddeplbl->text() == QString::number(idRecEnCours))
                {
                    gBigTable->removeRow(i);
                    if (i < gBigTable->rowCount() - 1)
                        gBigTable->setCurrentCell(i,1);
                    else
                        gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
                    i = gBigTable->rowCount();
                }
            }
        }
        CalculTotalRecettes();
        RegleAffichageFiche(gBigTable->rowCount()>0? Lire : TableVide);
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Recalcule le total des recettes -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::CalculTotalRecettes()
{
    double Total = 0;
    if (gBigTable->rowCount() > 0)
    {
        for (int k = 0; k < gBigTable->rowCount(); k++)
        {
            UpLabel* Line = dynamic_cast<UpLabel*>(gBigTable->cellWidget(k,3));
            if (Line)
                Total += QLocale().toDouble(Line->text());
        }
    }
    QString TotalRecettes = QLocale().toString(Total,'f',2);
    ui->TotallineEdit->setText(tr("Total ")  + ui->AnneecomboBox->currentText() + " -> " + TotalRecettes);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Ouvre la gestion des comptes -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::GestionComptes()
{
    Dlg_Cmpt          = new dlg_comptes(proc);
    if (Dlg_Cmpt->getInitOK())
        Dlg_Cmpt->exec();
    gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Met à jour la fiche --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::MetAJourFiche()
{
    if (gMode == Lire)
    {
        ui->ModifierupPushButton        ->setEnabled(QDate::currentDate() < ui->DateRecdateEdit->date().addDays(365));
        ui->SupprimerupPushButton       ->setEnabled(QDate::currentDate() < ui->DateRecdateEdit->date().addDays(365));
    }
    if (gBigTable->rowCount() > 0)
    {
        ui->DateRecdateEdit     ->disconnect();
        ui->RefFiscalecomboBox  ->disconnect();
        ui->PaiementcomboBox    ->disconnect();

        idRecEnCours = static_cast<UpLabel*>(gBigTable->cellWidget(gBigTable->currentRow(),0))->text().toInt();

        //TODO : SQL
        QString MetAJourrequete = "select DateRecette, Libelle, Montant, Paiement, idremise, TypeRecette, CompteVirement, BanqueCheque, TireurCheque from " NOM_TABLE_RECETTESSPECIALES " where idRecette = " + QString::number(idRecEnCours);
        QSqlQuery ChercheRecQuery (MetAJourrequete,db);
        ChercheRecQuery.first();

        ui->ObjetlineEdit->setText(ChercheRecQuery.value(1).toString());
        ui->DateRecdateEdit->setDate(ChercheRecQuery.value(0).toDate());
        ui->MontantlineEdit->setText(QLocale().toString(ChercheRecQuery.value(2).toDouble(),'f',2));
        QString A = ChercheRecQuery.value(3).toString();                                                         // Mode de paiement - col = 4
        QString B = "";
        ui->Comptelabel->setVisible(false);
        ui->ComptesupComboBox->setVisible(false);
        ui->BanqChequpComboBox->setVisible(false);
        ui->BanqueChequelabel   ->setVisible(false);
        ui->Tireurlabel         ->setVisible(false);
        ui->TireurlineEdit      ->setVisible(false);
        ui->ComptesupComboBox   ->setCurrentIndex(-1);
        ui->BanqChequpComboBox  ->setCurrentIndex(-1);

        if (A == "E")
            A = NOM_ESPECES;
        else if (A == "V")
        {
            if (ChercheRecQuery.value(6).toInt()>0)
            {
                QMap<int, Compte*>::iterator compteFind = gDataUser->getComptes()->comptesAll().find(ChercheRecQuery.value(6).toInt());
                if( compteFind != gDataUser->getComptes()->comptesAll().constEnd() )
                {
                    B = compteFind.value()->nom();
                    ui->Comptelabel->setVisible(true);
                    ui->ComptesupComboBox->setVisible(true);
                    ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(ChercheRecQuery.value(6).toInt()));
                }
            }
            A = NOM_VIREMENT;
        }
        else if (A == "C")
        {
            if (ChercheRecQuery.value(7).toString() != "")
            {
                B = ChercheRecQuery.value(7).toString();
                ui->BanqChequpComboBox  ->setVisible(true);
                ui->BanqueChequelabel   ->setVisible(true);
                ui->BanqChequpComboBox  ->setCurrentIndex(ui->BanqChequpComboBox->findText(B));
            }
            if (ChercheRecQuery.value(8).toString() != "")
            {
                B = ChercheRecQuery.value(8).toString();
                ui->Tireurlabel     ->setVisible(true);
                ui->TireurlineEdit  ->setVisible(true);
                ui->TireurlineEdit  ->setText(B);
            }
            A = NOM_CHEQUE;
        }
        ui->PaiementcomboBox    ->setCurrentIndex(ui->PaiementcomboBox->findText(A));
        ui->RefFiscalecomboBox  ->setCurrentText(ChercheRecQuery.value(5).toString());

        connect (ui->DateRecdateEdit,       &QDateEdit::dateChanged,                                [=] {EnableModifiepushButton();});
        connect (ui->PaiementcomboBox,      &QComboBox::currentTextChanged,                         [=] {EnableModifiepushButton();});
        connect (ui->PaiementcomboBox,      QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {ChoixPaiement();});
        connect (ui->RefFiscalecomboBox,    &QComboBox::currentTextChanged,                         [=] {EnableModifiepushButton();});

        QString Paiement = ui->PaiementcomboBox->currentText();
        bool modifiable;

        switch (gMode) {
        case Enregistrer:
            ui->ObjetlineEdit->clear();
            ui->MontantlineEdit->setText(QLocale().toString(0.00,'f',2));
            ui->DateRecdateEdit->setDate(QDate::currentDate());
            ui->Comptelabel->setVisible(true);
            ui->ComptesupComboBox->setVisible(true);
            ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(gDataUser->getIdCompteParDefaut()));
            ui->PaiementcomboBox->setCurrentText(NOM_VIREMENT);
            break;
        case Modifier:
            if (Paiement == NOM_VIREMENT)
                // on recherche si l'écriture existe dans archivesbanques et si c'est le cas, on ne peut pas modifier le montant
            {
                modifiable = QSqlQuery("select idLigne from " NOM_TABLE_ARCHIVESBANQUE " where idrecspec = " + QString::number(idRecEnCours),db).size() == 0;
                ui->MontantlineEdit->setEnabled(modifiable);
                ui->PaiementcomboBox->setEnabled(modifiable);
                ui->Comptelabel->setVisible(modifiable);
                ui->ComptesupComboBox->setVisible(modifiable);
                ui->Comptelabel->setEnabled(modifiable);
                ui->ComptesupComboBox->setEnabled(modifiable);
            }
            else if (Paiement == NOM_CHEQUE)
                // on recherche si le chéque a été déposé et si c'est le cas, on ne peut pas modifier le montant
            {
                QString requete = "select idRemise from " NOM_TABLE_RECETTESSPECIALES " where idrecspec = " + QString::number(idRecEnCours);
                QSqlQuery cherchequery(requete,db);
                cherchequery.first();
                modifiable = !cherchequery.value(0).isNull();
                ui->MontantlineEdit->setEnabled(modifiable);
                ui->PaiementcomboBox->setEnabled(modifiable);
                ui->BanqChequpComboBox->setEnabled(modifiable);
                ui->BanqueChequelabel->setEnabled(modifiable);
                ui->Tireurlabel->setEnabled(modifiable);
                ui->TireurlineEdit->setEnabled(modifiable);
                ui->TireurlineEdit->setVisible(modifiable);
                ui->BanqChequpComboBox->setVisible(modifiable);
                ui->BanqueChequelabel->setVisible(modifiable);
                ui->Tireurlabel->setVisible(modifiable);
            }
            break;
        default:
            break;
        }
    }
    else
    {
        ui->ModifierupPushButton        ->setVisible(false);
        ui->SupprimerupPushButton       ->setVisible(false);
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Modifier une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::ModifierRecette()
{
    QString idRec       = QString::number(idRecEnCours);
    QString Paiement    = ui->PaiementcomboBox->currentText();

    if (!VerifSaisie())
        return;
    // on reconstruit les renseignements de la recette à modifier
    QString req = "select paiement, comptevirement, banquecheque, idremise from " NOM_TABLE_RECETTESSPECIALES " where idrecette = " + idRec;
    QSqlQuery ancquer(req, db);
    if (ancquer.size()==0)
    {
        EnregistreRecette();
        return;
    }
    ancquer.first();
    QString ancpaiement = ancquer.value(0).toString();

    if (ancpaiement == "E")
     {
        req = "delete from " NOM_TABLE_RECETTESSPECIALES " where idrecette = " + idRec;
        //qDebug() << req;
        QSqlQuery(req, db);
        EnregistreRecette();
     }
     else if (ancpaiement == "C")
     {
         // le cheque a été remis en banque, on se contente de mettre à jour la date, la rubrique fiscale et l'intitulé dans autresrecettes
         if (ancquer.value(3).toInt()>0)
         {
             req = "update " NOM_TABLE_RECETTESSPECIALES " set "
                   "DateRecette = '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") + "', "
                   "Libelle = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +"', "
                   "TypeRecette = '" + ui->RefFiscalecomboBox->currentText() + "'"
                   " where idrecette = " + idRec;
             QSqlQuery(req,db);
         }
         else
         // le cheque n'a pas été remis en banque, on remet tout à jour
         {
             req = "delete from " NOM_TABLE_RECETTESSPECIALES " where idrecette = " + idRec;
             //qDebug() << req;
             QSqlQuery(req, db);
             EnregistreRecette();
         }
     }
     else if (ancpaiement == "V")
     {
         QString req = "select idLigne from " NOM_TABLE_ARCHIVESBANQUE " where idrecspec = " + idRec;
         QSqlQuery archvirquer(req, db);
         if (archvirquer.size()>0)
         {
             // le virement a été enregistré en banque, on se contente de mettre à jour la date, la rubrique fiscale et l'intitulé dans autresrecettes et archivesbanques
             archvirquer.first();
             QString idligne = archvirquer.value(0).toString();
             req = "update " NOM_TABLE_ARCHIVESBANQUE " set "
                   "LigneDate = '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") + "', "
                   "LigneLibelle = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) + "'"
                   "LigneTypeOperation = '" + proc->CorrigeApostrophe(ui->RefFiscalecomboBox->currentText()) + "'"
                   " where idligne = " + idligne;
             QSqlQuery quer1(req,db);
             m_db->traiteErreurRequete(quer1,req);
             req = "update " NOM_TABLE_RECETTESSPECIALES " set "
                   "DateRecette = '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") + "', "
                   "Libelle = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) + "'"
                   "TypeRecette = '" + proc->CorrigeApostrophe(ui->RefFiscalecomboBox->currentText()) + "'"
                   " where idrecette = " + idRec;
             QSqlQuery quer2(req,db);
             m_db->traiteErreurRequete(quer2,req);
         }
         else
         // le virement n'a pas été enregistré en banque
         {
             req = "delete from " NOM_TABLE_RECETTESSPECIALES " where idrecette = " + idRec;
             //qDebug() << req;
             QSqlQuery(req, db);
             EnregistreRecette();
         }
    }
    RedessineBigTable(idRec.toInt());
    RegleAffichageFiche(gBigTable->rowCount()>0? Lire : TableVide);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {RedessineBigTable();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::RedessineBigTable(int idRec)
{
    RemplitBigTable();
    CalculTotalRecettes();
    if (gBigTable->rowCount() > 0)
    {
        if (idRec == -1)
            idRec = gBigTable->rowCount()-1;
        else
        {
            bool trouve = false;
            for (int row=0; row< gBigTable->rowCount(); row++)
            {
                UpLabel* idReclbl = static_cast<UpLabel*>(gBigTable->cellWidget(row,0));
                if (idReclbl->text() == QString::number(idRec))
                {
                    gBigTable->setCurrentCell(row,1);
                    row = gBigTable->rowCount();
                    trouve = true;
                }
            }
            if (!trouve)    gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
            gMode = Lire;
        }
        gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
        gBigTable->scrollTo(gBigTable->model()->index(gBigTable->model()->rowCount()-1,1));
    }
    else
        RegleAffichageFiche(TableVide);
    ui->SupprimerupPushButton   ->setEnabled(gBigTable->rowCount()>0);
    ui->ModifierupPushButton    ->setEnabled(gBigTable->rowCount()>0);
}

void dlg_recettesspeciales::closeEvent(QCloseEvent *event)
{
    proc->gsettingsIni->setValue("PositionsFiches/PositionDepenses",saveGeometry());
    event->accept();
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void dlg_recettesspeciales::keyPressEvent ( QKeyEvent * event )
{
    switch (event->key()) {
    case Qt::Key_F12:{
        if (gMode == Lire)
            reject();
        else
            AnnulEnreg();
       break;}
    default: break;
    }
}

bool dlg_recettesspeciales::getInitOK()
{
    return InitOK;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Definit les colonnes de BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::DefinitArchitectureBigTable()
{
    int                 ColCount;

    ColCount = 7;
    gBigTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    gBigTable->setContextMenuPolicy(Qt::CustomContextMenu);
    gBigTable->setPalette(QPalette(Qt::white));
    gBigTable->setEditTriggers(QAbstractItemView::AnyKeyPressed
                                 |QAbstractItemView::DoubleClicked
                                 |QAbstractItemView::EditKeyPressed
                                 |QAbstractItemView::SelectedClicked);
    gBigTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gBigTable->verticalHeader()->setVisible(false);
    gBigTable->setFocusPolicy(Qt::StrongFocus);
    gBigTable->setColumnCount(ColCount);
    gBigTable->setSelectionMode(QAbstractItemView::SingleSelection);

    QStringList LabelARemplir;
    LabelARemplir << "";
    LabelARemplir << tr("Date");
    LabelARemplir << tr("Recette");
    LabelARemplir << tr("Montant");
    LabelARemplir << tr("Mode de paiement");
    LabelARemplir << tr("Rubrique fiscale");
    LabelARemplir << tr("Classement par date");

    gBigTable->setHorizontalHeaderLabels(LabelARemplir);
    gBigTable->horizontalHeader()->setVisible(true);
    gBigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    gBigTable->setColumnWidth(li,25);                                               // idRecette
    li++;
    gBigTable->setColumnWidth(li,100);                                              // DateRecette affichage européen
    li++;
    gBigTable->setColumnWidth(li,400);                                              // Libelle
    li++;
    gBigTable->setColumnWidth(li,100);                                              // Montant
    li++;
    gBigTable->setColumnWidth(li,160);                                              // ModePaiement
    li++;
    gBigTable->setColumnWidth(li,300);                                              // Rubrique fiscqale
    li++;
    gBigTable->setColumnWidth(li,0);                                                // DepDate

    gBigTable->setColumnHidden(0,true);
    gBigTable->setColumnHidden(ColCount-1,true);

    gBigTable->setGridStyle(Qt::SolidLine);
    gBigTable->FixLargeurTotale();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Annees dans le combobox Annees --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::ReconstruitListeAnnees()
{
    QString requete = "SELECT distinct Annee from (SELECT year(DateRecette) as Annee FROM " NOM_TABLE_RECETTESSPECIALES " WHERE idUser = " + QString::number(gDataUser->id()) + ") as ghf order by Annee";
    QSqlQuery ChercheAnneesQuery (requete,db);
    QStringList ListeAnnees;
    for (int i = 0; i < ChercheAnneesQuery.size(); i++)
    {
            ChercheAnneesQuery.seek(i);
            ListeAnnees << ChercheAnneesQuery.value(0).toString();
    }
    ui->AnneecomboBox->disconnect();
    ui->AnneecomboBox->clear();
    ui->AnneecomboBox->insertItems(0,ListeAnnees);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {RedessineBigTable();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::RemplitBigTable()
{
    //TODO : SQL
    QTableWidgetItem    *pItem8;
    QString             A;
    UpLabel *label0, *label1, *label2, *label3, *label4, *label5, *label6;
    gBigTable->disconnect();
    gBigTable->clearContents();
    QString Recrequete = "SELECT idRecette, idUser, year(DateRecette) as Annee, DateRecette, Libelle, Montant,"
                         "TypeRecette, Nooperation, Monnaie, Paiement, CompteVirement, BanqueCheque FROM " NOM_TABLE_RECETTESSPECIALES
                         " WHERE idUser = " + QString::number(gDataUser->id());
    if (ui->AnneecomboBox->currentText() != "")
        Recrequete += " AND year(DateRecette) = " + ui->AnneecomboBox->currentText();
    Recrequete += " ORDER BY DateRecette, Libelle";
    //proc->Edit(Recrequete);
    QSqlQuery EnumRecettesQuery (Recrequete,db);
    if (m_db->traiteErreurRequete(EnumRecettesQuery,Recrequete,"Impossible de retrouver la table des depenses"))
        return;
    gBigTable->setRowCount(EnumRecettesQuery.size());
    EnumRecettesQuery.first();

    for (int i = 0; i < EnumRecettesQuery.size(); i++)
        {
            int col = 0;
            int id = EnumRecettesQuery.value(0).toInt();

            label0 = new UpLabel;
            label1 = new UpLabel;
            label2 = new UpLabel;
            label3 = new UpLabel;
            label4 = new UpLabel;
            label5 = new UpLabel;
            label6 = new UpLabel;

            label0->setId(id);                      // idRecette
            label1->setId(id);                      // idRecette
            label2->setId(id);                      // idRecette
            label3->setId(id);                      // idRecette
            label4->setId(id);                      // idRecette
            label5->setId(id);                      // idRecette
            label6->setId(id);                      // idRecette

            label0->setContextMenuPolicy(Qt::CustomContextMenu);
            label1->setContextMenuPolicy(Qt::CustomContextMenu);
            label2->setContextMenuPolicy(Qt::CustomContextMenu);
            label3->setContextMenuPolicy(Qt::CustomContextMenu);
            label4->setContextMenuPolicy(Qt::CustomContextMenu);
            label5->setContextMenuPolicy(Qt::CustomContextMenu);
            label6->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(label0,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(label0);});
            connect(label1,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(label1);});
            connect(label2,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(label2);});
            connect(label3,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(label3);});
            connect(label4,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(label4);});
            connect(label5,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(label5);});
            connect(label6,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(label6);});

            A = EnumRecettesQuery.value(0).toString();                                                                  // idRecette - col = 0
            label0->setText(A);
            gBigTable->setCellWidget(i,col,label0);
            col++;

            A = EnumRecettesQuery.value(3).toDate().toString(tr("d MMM yyyy"));                                         // DateRecette col = 1
            label1->setText(A + " ");
            label1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            gBigTable->setCellWidget(i,col,label1);
            col++;

            label2->setText(" " + EnumRecettesQuery.value(4).toString());                                               // Libelle - col = 2
            gBigTable->setCellWidget(i,col,label2);
            col++;

            if (EnumRecettesQuery.value(8).toString() == "F")
                A = QLocale().toString(EnumRecettesQuery.value(5).toDouble()/6.55957,'f',2);                            // Montant en F converti en euros
            else
                A = QLocale().toString(EnumRecettesQuery.value(5).toDouble(),'f',2);                                    // Montant - col = 3
            label3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            label3->setText(A + " ");                                                                                   // Montant - col = 3
            gBigTable->setCellWidget(i,col,label3);
            col++;

            A = EnumRecettesQuery.value(9).toString();                                                                  // Paiement - col = 4
            QString B = "";
            QString C = "";
            if (A == "E")
                A = NOM_ESPECES;
            else if (A == "V")
            {
                if (EnumRecettesQuery.value(10).toInt() > 0)
                {
                    QMultiMap<int, Compte*>::const_iterator cptFind = gDataUser->getComptes()->comptesAll().find(EnumRecettesQuery.value(10).toInt());
                    if( cptFind != gDataUser->getComptes()->comptesAll().constEnd() )
                        B = cptFind.value()->nom();
                }
                A = NOM_VIREMENT + (B==""? "" : " " + B);
            }
            else if (A == "C")
            {
                B = EnumRecettesQuery.value(11).toString();
                A = NOM_CHEQUE + (B==""? "" : " " + B);
            }
            label4->setText(" " + A);
            gBigTable->setCellWidget(i,col,label4);
            col++;

            A = EnumRecettesQuery.value(6).toString();                                                                  // Famille fiscale - col = 5
            label6->setText(" " + A);
            gBigTable->setCellWidget(i,col,label6);
            col++;

            A = EnumRecettesQuery.value(3).toDate().toString("yyyy-MM-dd");                                             // ClassementparDate - col = 6
            pItem8 = new QTableWidgetItem() ;
            pItem8->setText(A);
            gBigTable->setItem(i,col,pItem8);

            gBigTable->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));

            EnumRecettesQuery.next();
        }
    ui->SupprimerupPushButton->setEnabled(false);
    connect (gBigTable,     &QTableWidget::itemSelectionChanged, [=] {RegleAffichageFiche(Lire);});
}

//  Vérifer que la saisie est complète et cohérente
bool dlg_recettesspeciales::VerifSaisie()
{
    QString Paiement = ui->PaiementcomboBox->currentText();

    QString Erreur = "";
    if (ui->ObjetlineEdit->text() == "")
        Erreur = tr("l'objet de la recette");
    else if (!(QLocale().toDouble(ui->MontantlineEdit->text()) > 0))
        Erreur = tr("le montant");
    else if (ui->PaiementcomboBox->currentText() == "")
        Erreur = tr("le mode de paiement");
    else if (ui->RefFiscalecomboBox->currentText() == "")
        Erreur = tr("la rubrique fiscale");
    else if (Paiement == NOM_VIREMENT)
    {
        if (ui->ComptesupComboBox->currentText()=="")
            Erreur = tr("le compte crédité par le virement");
    }
    else if (Paiement == NOM_ESPECES)
    {
        if (ui->ComptesupComboBox->currentText()=="")
            Erreur = tr("le compte crédité par ce versement d'espèces");
    }
    else if (Paiement == NOM_CHEQUE)
    {
        if (ui->BanqChequpComboBox->currentText()=="")
            Erreur = tr("la banque émettrice du chèque");
        if (ui->TireurlineEdit->text()=="")
            Erreur = tr("l'a banque émettrice'émetteur du chèque");
    }

    if (Erreur != "")
    {
        UpMessageBox::Watch(this,tr("vous avez oublié de renseigner ") + Erreur);
        if (Erreur == tr("le motif"))
            ui->ObjetlineEdit->setFocus();
        else if (Erreur == tr("le montant"))
            ui->MontantlineEdit->setFocus();
        else if (Erreur == tr("le mode de paiement"))
            ui->PaiementcomboBox->setFocus();
        else if (Erreur == tr("la rubrique fiscale"))
            ui->RefFiscalecomboBox->setFocus();
        else if (Erreur == tr("le compte crédité par le virement"))
            ui->ComptesupComboBox->setFocus();
        else if (Erreur == tr("la banque émettrice du chèque"))
            ui->BanqChequpComboBox->setFocus();
        else if (Erreur == tr("l'a banque émettrice'émetteur du chèque"))
            ui->TireurlineEdit->setFocus();
        return false;
    }
    return true;
}
