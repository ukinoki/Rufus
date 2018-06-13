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

#include "dlg_depenses.h"
#include "icons.h"
#include "ui_dlg_depenses.h"

dlg_depenses::dlg_depenses(Procedures *procAPasser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_depenses)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    proc        = procAPasser;
    db          = DataBase::getInstance()->getDataBase();
    gidUserADebiter     = -1;
    ui->UserscomboBox   ->setEnabled(proc->getDataUser()->isSecretaire() );
    gListeLiberauxModel  = proc->getListeLiberaux(); // les colonnes -> iduser, userlogin, soignant, responsableactes, UserEnregHonoraires, idCompteEncaissHonoraires

    for (int i=0; i<gListeLiberauxModel->rowCount(); i++)
        ui->UserscomboBox->addItem(gListeLiberauxModel->item(i,1)->text(), gListeLiberauxModel->item(i,0)->text());
    if (ui->UserscomboBox->findData(QString::number(proc->getDataUser()->id()))>-1)
        ui->UserscomboBox->setCurrentIndex(ui->UserscomboBox->findData(QString::number(proc->getDataUser()->id())));
    else
        ui->UserscomboBox->setCurrentIndex(0);

    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionDepenses").toByteArray());

    gidUserADebiter             = ui->UserscomboBox->currentData().toInt();
    gNomUser                    = proc->getLogin((gidUserADebiter));
    proc                        ->setListeComptesUser(gidUserADebiter);
    glistComptes                = proc->getListeComptesUser();
    glistComptesAvecDesactive   = proc->getListeComptesUserAvecDesactive();
    if (glistComptesAvecDesactive->rowCount() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de continuer!"), tr("Pas de compte bancaire enregistré pour ") + gNomUser);
        InitOK = false;
        return;
    }

    gDataUser = proc->setDataOtherUser(gidUserADebiter);
    if (gDataUser == nullptr)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche paiement!"), tr("Les paramètres de ")
                             + gNomUser + tr("ne sont pas retrouvés"));
        InitOK = false;
        return;
    }
    if (gDataUser->getIdCompteParDefaut() <= 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir le journal des dépenses!"), tr("Pas de compte bancaire enregistré pour ")
                                     + gNomUser);
        InitOK = false;
        return;
    }

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

    QString requete = "SELECT reffiscale from " NOM_TABLE_RUBRIQUES2035 " where FamFiscale is not null and famfiscale <> 'Prélèvement personnel'";
    QSqlQuery ChercheRubriquesQuery (requete,db);
    QStringList ListeRubriques;
    ListeRubriques << tr("Prélèvement personnel");
    for (int i = 0; i < ChercheRubriquesQuery.size(); i++)
    {
            ChercheRubriquesQuery.seek(i);
            ListeRubriques << ChercheRubriquesQuery.value(0).toString();
    }
    ui->RefFiscalecomboBox->insertItems(0,ListeRubriques);
    ui->RefFiscalecomboBox->setCurrentText(ListeRubriques.at(0));

    glistMoyensDePaiement << tr("Carte de crédit");
    glistMoyensDePaiement << tr("Chèque");
    glistMoyensDePaiement << tr("Espèces");
    glistMoyensDePaiement << tr("Prélèvement");
    glistMoyensDePaiement << tr("Virement");
    glistMoyensDePaiement << tr("TIP");
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

    ReconstruitListeRubriques();
    ReconstruitListeToutesRubriques();
    ReconstruitListeAnnees();

    connect (ui->GestionComptesupPushButton,    &QPushButton::clicked,          [=] {GestionComptes();});
    connect (ui->NouvelleDepenseupPushButton,   &QPushButton::clicked,          [=] {GererDepense(ui->NouvelleDepenseupPushButton);});
    connect (ui->ModifierupPushButton,          &QPushButton::clicked,          [=] {GererDepense(ui->ModifierupPushButton);});
    connect (ui->OKupPushButton,                &QPushButton::clicked,          [=] {accept();});
    connect (ui->Rubriques2035comboBox,         QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                [=](int) {RedessineBigTable();});
    connect (ui->MontantlineEdit,               &QLineEdit::editingFinished,    [=] {ConvertitDoubleMontant();});
    connect (ui->PaiementcomboBox,              QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                [=](int) {ChoixPaiement();});
    connect (ui->ObjetlineEdit,                 &QLineEdit::textEdited,         [=] {EnableModifiepushButton();});
    connect (ui->MontantlineEdit,               &QLineEdit::textEdited,         [=] {EnableModifiepushButton();});
    connect (ui->DateDepdateEdit,               &QDateEdit::dateChanged,        [=] {EnableModifiepushButton();});
    connect (ui->PaiementcomboBox,              &QComboBox::currentTextChanged, [=] {EnableModifiepushButton();});
    connect (ui->RefFiscalecomboBox,            &QComboBox::currentTextChanged, [=] {EnableModifiepushButton();});
    connect (ui->SupprimerupPushButton,         &QPushButton::clicked,          [=] {SupprimerDepense();});
    connect (ui->UserscomboBox,                 QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                [=](int) {ChangeUser(ui->UserscomboBox->currentIndex());});

    connect (EnregupPushButton,                 &QPushButton::clicked,          [=] {ModifierDepense();});
    connect (AnnulupPushButton,                 &QPushButton::clicked,          [=] {AnnulEnreg();});

    QString year = QDate::currentDate().toString("yyyy");
    int idx = ui->AnneecomboBox->findText(year);
    ui->AnneecomboBox->setCurrentIndex(idx==-1? 0 : idx);

    gBigTable->setFocus();

    MetAJourFiche();
    InitOK= true;
}

dlg_depenses::~dlg_depenses()
{
    delete ui;
}

void dlg_depenses::RegleComptesComboBox(bool ActiveSeult)
{
    QStandardItemModel *model = (ActiveSeult? glistComptes : glistComptesAvecDesactive);
    ui->ComptesupComboBox->clear();
    for (int i=0; i<model->rowCount(); i++)
        ui->ComptesupComboBox->addItem(model->item(i,1)->text(), model->item(i,0)->text());
}

void    dlg_depenses::RegleAffichageFiche(enum gMode mode)
{
    gMode = mode;
    ui->DateDepdateEdit     ->setVisible(gMode != TableVide);
    ui->ObjetlineEdit       ->setVisible(gMode != TableVide);
    ui->MontantlineEdit     ->setVisible(gMode != TableVide);
    ui->PaiementcomboBox    ->setVisible(gMode != TableVide);

    ui->ComptesupComboBox   ->setVisible(gMode != TableVide);
    ui->RefFiscalecomboBox  ->setVisible(gMode != TableVide);
    ui->DateDeplabel        ->setVisible(gMode != TableVide);
    ui->Objetlabel          ->setVisible(gMode != TableVide);
    ui->Montantlabel        ->setVisible(gMode != TableVide);
    ui->Paiementlabel       ->setVisible(gMode != TableVide);
    ui->RefFiscalelabel     ->setVisible(gMode != TableVide);

    ui->DateDepdateEdit     ->setEnabled(gMode != Lire);
    ui->ObjetlineEdit       ->setEnabled(gMode != Lire);
    ui->MontantlineEdit     ->setEnabled(gMode != Lire);
    ui->PaiementcomboBox    ->setEnabled(gMode != Lire);
    ui->ComptesupComboBox   ->setEnabled(gMode != Lire);
    ui->RefFiscalecomboBox  ->setEnabled(gMode != Lire);
    ui->DateDeplabel        ->setEnabled(gMode != Lire);
    ui->Objetlabel          ->setEnabled(gMode != Lire);
    ui->Montantlabel        ->setEnabled(gMode != Lire);
    ui->Paiementlabel       ->setEnabled(gMode != Lire);
    ui->RefFiscalelabel     ->setEnabled(gMode != Lire);
    ui->OKupPushButton      ->setEnabled(gMode == Lire || gMode == TableVide);
    ui->GestionComptesupPushButton->setEnabled(gMode == Lire || gMode == TableVide);
    ui->SupprimerupPushButton->setVisible(gMode == Lire);
    ui->ModifierupPushButton->setVisible(gMode == Lire);
    ui->NouvelleDepenseupPushButton->setEnabled((gMode == Lire || gMode == TableVide) && glistComptes->rowCount()>0);
    QString ttip = (glistComptes->rowCount()==0?
                    tr("Vous ne pouvez pas enregistrer de dépenses.\n"
                       "Aucun compte bancaire n'est enregistré.") : "");
    ui->NouvelleDepenseupPushButton->setToolTip(ttip);
    EnregupPushButton       ->setVisible(!(gMode == Lire || gMode == TableVide));
    AnnulupPushButton       ->setVisible(!(gMode == Lire || gMode == TableVide));
    gBigTable               ->setEnabled(gMode == Lire);

    ui->UserscomboBox        ->setEnabled(proc->getDataUser()->isSecretaire() && gMode==Lire);


    switch (gMode) {
    case TableVide:
        ui->OKupPushButton      ->setShortcut(QKeySequence("Meta+Return"));
        ui->ModifierupPushButton->setShortcut(QKeySequence());
        EnregupPushButton       ->setShortcut(QKeySequence());
        // => pas de break, on continue avec le code de Lire
    case Lire: {
        EnregupPushButton       ->setText(tr("Modifier"));
        RegleComptesComboBox(false);
        break;
    }
    case Modifier: {
        disconnect (gBigTable, 0,0,0);
        EnregupPushButton       ->setText("Valider");
        ui->ComptesupComboBox   ->setVisible(ui->PaiementcomboBox->currentText() != tr("Espèces"));
        if (ui->PaiementcomboBox->currentText() != tr("Espèces"))            // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte pour savoir si on peut la modifier
        {
            bool modifiable = true;
            QString VerifArchivesrequete = " select * from " NOM_TABLE_ARCHIVESBANQUE " where idDep = " + QString::number(idDepEnCours);
            QSqlQuery VerifArchivQuery (VerifArchivesrequete,db);
            if (VerifArchivQuery.size() > 0)
                modifiable = false;
            if (modifiable)
            {
                QDate   Dateop;
                QString Libelle;
                double  montant;
                QString requete1 = "select DateDep, Objet, Montant from " NOM_TABLE_DEPENSES " where iddep = " + QString::number(idDepEnCours);
                QSqlQuery query1 (requete1,db);
                if (query1.size() > 0)
                {
                    query1.first();
                    Dateop = query1.value(0).toDate();
                    Libelle = query1.value(1).toString();
                    montant = query1.value(2).toDouble();
                    QString requete2 = "select idligne from " NOM_TABLE_ARCHIVESBANQUE " where LigneDate = '" + Dateop.toString("yyyy-MM-dd") +
                            "' and LigneLibelle = '" + proc->CorrigeApostrophe(Libelle) + "' and LigneMontant = " + QString::number(montant);
                    QSqlQuery query2 (requete2,db);
                    if (query2.size()> 0)
                        modifiable = false;
                }
            }
            ui->DateDeplabel        ->setEnabled(modifiable);
            ui->DateDepdateEdit     ->setEnabled(modifiable);
            ui->Montantlabel        ->setEnabled(modifiable);
            ui->MontantlineEdit     ->setEnabled(modifiable);
            ui->Paiementlabel       ->setEnabled(modifiable);
            ui->PaiementcomboBox    ->setEnabled(modifiable);
            ui->ComptesupComboBox   ->setEnabled(modifiable);
        }
        ui->OKupPushButton->setShortcut(QKeySequence());
        ui->ModifierupPushButton->setShortcut(QKeySequence());
        EnregupPushButton->setShortcut(QKeySequence("Meta+Return"));
        RegleComptesComboBox();
        break;
    }
    case Enregistrer: {
        disconnect (gBigTable, 0,0,0);
        ui->DateDepdateEdit     ->setDate(QDate::currentDate());
        ui->ObjetlineEdit       ->setText("");
        ui->MontantlineEdit     ->setText("0,00");
        ui->ComptesupComboBox   ->setVisible(!(ui->PaiementcomboBox->currentText() == tr("Espèces") || ui->PaiementcomboBox->currentText() == ""));
        ui->RefFiscalecomboBox  ->setCurrentText("");
        EnregupPushButton       ->setText(tr("Enregistrer"));
        ui->OKupPushButton      ->setShortcut(QKeySequence());
        ui->ModifierupPushButton->setShortcut(QKeySequence());
        EnregupPushButton       ->setShortcut(QKeySequence("Meta+Return"));
        RegleComptesComboBox();
        ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(QString::number(gDataUser->getIdCompteParDefaut())));
        break;
    }
    default:
        break;
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Clic sur AnnulupPushButton ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::AnnulEnreg()
{
    RegleAffichageFiche(Lire);
    MetAJourFiche();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur courant ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::ChangeUser(int)
{
    gidUserADebiter             = ui->UserscomboBox->currentData().toInt();
    gNomUser                    = proc->getLogin((gidUserADebiter));
    proc                        ->setListeComptesUser(gidUserADebiter);
    glistComptes                = proc->getListeComptesUser();
    glistComptesAvecDesactive   = proc->getListeComptesUserAvecDesactive();
    if (glistComptesAvecDesactive->rowCount() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de continuer!"), tr("Pas de compte bancaire enregistré pour ") + gNomUser);
        InitOK = false;
        return;
    }
    gDataUser = proc->setDataOtherUser(gidUserADebiter);
    if (gDataUser == nullptr)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche paiement!"), tr("Les paramètres de ")
                             + gNomUser + tr("ne sont pas retrouvés"));
        InitOK = false;
        return;
    }
    if (gDataUser->getIdCompteParDefaut() <= 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir le journal des dépenses!"), tr("Pas de compte bancaire enregistré pour ")
                                     + gNomUser);
        InitOK = false;
        return;
    }
    RegleComptesComboBox(false);
    ReconstruitListeAnnees();
    ReconstruitListeRubriques();
    QString year = QDate::currentDate().toString("yyyy");
    int idx = ui->AnneecomboBox->findText(year);
    ui->AnneecomboBox->setCurrentIndex(idx>-1? 0 : idx);
}

void dlg_depenses::ChoixPaiement()
{
    ui->ComptesupComboBox->setVisible(ui->PaiementcomboBox->currentText() != tr("Espèces") && ui->PaiementcomboBox->currentText() != "");
}

void dlg_depenses::ConvertitDoubleMontant()
{
    QLineEdit * Emetteur = qobject_cast<QLineEdit*> (sender());
    QString b = Emetteur->text();
    b = QLocale().toString(QLocale().toDouble(b),'f',2);
    Emetteur->setText(b);
}

void dlg_depenses::EnableModifiepushButton()
{
    ui->ModifierupPushButton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- ENregistrer une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::EnregistreDepense()
{
    bool OnSauteLaQuestionSuivante = false;
    QString pb = "";

    //  Vérifer que la saisie est complète et cohérente
    QString Erreur = "";
    if (ui->ObjetlineEdit->text() == "")
        Erreur = tr("l'objet de la dépense");
    else if (!(QLocale().toDouble(ui->MontantlineEdit->text()) > 0))
        Erreur = tr("le montant");
    else if (ui->PaiementcomboBox->currentText() == "")
        Erreur = tr("le mode de paiement");
    else if (ui->RefFiscalecomboBox->currentText() == "")
        Erreur = tr("la rubrique fiscale");

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
        return;
    }

    // vérifier que cette dépense n'a pas été déjà saisie
    QString requete = "select DateDep from " NOM_TABLE_DEPENSES " where DateDep = '" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
            "'and Objet = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
            "'and Montant = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
            " and idUser = " + QString::number(gidUserADebiter);
    QSqlQuery ChercheDepQuery (requete,db);
    if (ChercheDepQuery.size() > 0)
    {
        pb = tr("Elle a déjà été saisie");
        OnSauteLaQuestionSuivante = true;
    }

    if (!OnSauteLaQuestionSuivante)
    {
        if (QDate::currentDate() > ui->DateDepdateEdit->date().addDays(90))
            pb = tr("Elle date de plus de 3 mois");
        requete = "select DateDep from " NOM_TABLE_DEPENSES " where Datedep > '" + ui->DateDepdateEdit->date().addDays(-180).toString("yyyy-MM-dd") +
                "'and Objet = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                "'and Montant = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                " and idUser = " + QString::number(gidUserADebiter);
        QSqlQuery ChercheDep2Query (requete,db);
        if (ChercheDep2Query.size() > 0)
        {
            ChercheDep2Query.last();
            if (pb != "")
                pb += "\n";
            pb += tr("Une dépense semblable a été saisie le ") + ChercheDep2Query.value(0).toDate().toString("dd MMM yyyy");
        }
    }

    if (pb != "")
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Il y a un problème avec cette dépense!" ));
        msgbox.setInformativeText(pb + "\n" + tr("Confirmer la saisie?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setText(tr("Confirmer"));
        UpSmallButton *NoBouton = new UpSmallButton();
        NoBouton->setText(tr("Annuler"));
        msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != OKBouton)
        {
            delete NoBouton;
            delete OKBouton;
            return;
        }
        delete NoBouton;
        delete OKBouton;
    }

    // Insertion de l'écriture dans la table depenses
    QString Paiement, idDep, m;
    Paiement = proc->CorrigeApostrophe(ui->PaiementcomboBox->currentText());
    if (Paiement == tr("Espèces"))              m = "E";
    else if (Paiement == tr("Virement"))        m = "V";
    else if (Paiement == tr("Carte de crédit")) m = "B";
    else if (Paiement == tr("Chèque"))          m = "C";
    else if (Paiement == tr("Prélèvement"))     m = "P";
    else if (Paiement == tr("TIP"))             m = "T";

    QString FamFiscale;
    requete = "select Famfiscale from " NOM_TABLE_RUBRIQUES2035 " where reffiscale = '"
            + proc->CorrigeApostrophe(ui->RefFiscalecomboBox->currentText()) +"'";
    QSqlQuery cherchefamfiscalequery (requete,db);
    DataBase::getInstance()->traiteErreurRequete(cherchefamfiscalequery,requete,"");
    if (cherchefamfiscalequery.size() > 0)
    {
        cherchefamfiscalequery.first();
        FamFiscale = cherchefamfiscalequery.value(0).toString();
    }
    QString idCompte = ui->ComptesupComboBox->currentData().toString();
    QString insertdeprequete = "insert into " NOM_TABLE_DEPENSES " (DateDep, idUser, Objet, Montant, RefFiscale, FamFiscale, ModePaiement, Compte)"
            " VALUES ('" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
            "', " + QString::number(gidUserADebiter) +
            ", '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
            "', " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
            ", '" + proc->CorrigeApostrophe(ui->RefFiscalecomboBox->currentText()) +
            "', '" + FamFiscale +
            "', '" + m +
            "', " + (m!="E"? idCompte : "null") + ")";
    QSqlQuery ("LOCK TABLES '" NOM_TABLE_DEPENSES " WRITE, " NOM_TABLE_ARCHIVESBANQUE " WRITE, " NOM_TABLE_LIGNESCOMPTES "' WRITE",db);
    QSqlQuery EnregistreDepenseQuery (insertdeprequete,db);
    DataBase::getInstance()->traiteErreurRequete(EnregistreDepenseQuery,insertdeprequete,tr("Impossible d'enregister cete dépense!"));
    requete     = "select max(idDep) from " NOM_TABLE_DEPENSES;
    QSqlQuery   ChercheidDepQuery (requete,db);
    ChercheidDepQuery.first();
    idDep       = ChercheidDepQuery.value(0).toString();
    // insertion de l'écriture dans la table lignescomptes quand il s'agit d'une opération bancaire
    if (Paiement != tr("Espèces"))
    {
        if (Paiement == tr("Virement")) Paiement = tr("Virement débiteur");
        int a = proc->getMAXligneBanque();
        requete = "insert into " NOM_TABLE_LIGNESCOMPTES "(idLigne, idCompte, idDep, LigneDate, Lignelibelle, LigneMontant, LigneDebitCredit, LigneTypeoperation) VALUES (" +
                    QString::number(a) + "," +
                    idCompte +
                    "," + idDep +
                    ", '" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
                    "', '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                    "', "  + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                    ", 0, '" + Paiement + "')";
        QSqlQuery EnregLigneCompeQuery(requete,db);
        DataBase::getInstance()->traiteErreurRequete(EnregLigneCompeQuery,requete,tr("Impossible d'enregister cette opération sur le compte bancaire!"));
    }
    QSqlQuery("UNLOCK TABLES", db);

    gBigTable->setEnabled(true);
    ui->SupprimerupPushButton->setVisible(true);
    ui->ModifierupPushButton->setVisible(true);
    ui->NouvelleDepenseupPushButton->setEnabled(true);

    int annee =  ui->DateDepdateEdit->date().year();
    if (ui->AnneecomboBox->currentText() != QString::number(annee))
    {
        if (ui->AnneecomboBox->findText(QString::number(annee)) == -1)
            ReconstruitListeAnnees();
        ui->AnneecomboBox->setCurrentText(QString::number(annee));
    }
    RedessineBigTable();
    RegleAffichageFiche(Lire);
    for (int i=0; i < gBigTable->rowCount(); i++)
    {
        UpLabel* idDeplbl = static_cast<UpLabel*>(gBigTable->cellWidget(i,0));
        if (idDeplbl->text() == idDep){
            gBigTable->setCurrentCell(i,1);
            idDepEnCours = idDep.toInt();
            i = gBigTable->rowCount();
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Gerer une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::GererDepense(QPushButton *widgsender)
{
    if (widgsender == ui->ModifierupPushButton)
    {
        if (gMode == Lire)
            RegleAffichageFiche(Modifier);
        else
            RegleAffichageFiche(Lire);
    }
    else
        RegleAffichageFiche(Enregistrer);
}

void dlg_depenses::MenuContextuel(QPoint pos, UpLabel *labelClicked)
{
    int idDepAOuvrir = labelClicked->getId();

    QMenu *menu;
    menu = new QMenu(this);

    if (ui->Rubriques2035comboBox->currentIndex() == 0)
    {
        QAction *pAction_RecopieDep = menu->addAction(tr("Effectuer une copie de cette dépense à la date d'aujourd'hui"));
        connect (pAction_RecopieDep, &QAction::triggered,    [=] {ChoixMenu(QString::number(idDepAOuvrir));});
    }
    QAction *pAction_ChercheVal = menu->addAction(tr("Rechercher une valeur"));
    connect (pAction_ChercheVal, &QAction::triggered,    [=] {ChoixMenu("ChercheVal");});

    // ouvrir le menu
    menu->deleteLater();
    menu->exec(pos);
}

void dlg_depenses::ChoixMenu(QString choix)
{
    if (choix == "ChercheVal")
    {
        QMessageBox msgbox;
        msgbox.setText("...");
        msgbox.setDetailedText("...");
        msgbox.setInformativeText(tr("Entrez la valeur à rechercher..."));
        msgbox.setTextInteractionFlags(Qt::TextEditable);
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setUpButtonStyle(UpSmallButton::STARTBUTTON);
        msgbox.addButton(OKBouton, QMessageBox::AcceptRole);
        msgbox.exec();
        proc->EnChantier();
    }
    else
    {
        MetAJourFiche();
        gMode = Enregistrer;
        ui->DateDepdateEdit             ->setEnabled(true);
        ui->ObjetlineEdit               ->setEnabled(true);
        ui->MontantlineEdit             ->setEnabled(true);
        ui->PaiementcomboBox            ->setEnabled(true);
        ui->ComptesupComboBox           ->setEnabled(true);
        ui->RefFiscalecomboBox          ->setEnabled(true);
        ui->DateDeplabel                ->setEnabled(true);
        ui->Objetlabel                  ->setEnabled(true);
        ui->Montantlabel                ->setEnabled(true);
        ui->Paiementlabel               ->setEnabled(true);
        ui->RefFiscalelabel             ->setEnabled(true);
        ui->OKupPushButton              ->setEnabled(false);
        ui->GestionComptesupPushButton  ->setEnabled(false);
        ui->SupprimerupPushButton       ->setVisible(false);
        ui->ModifierupPushButton        ->setVisible(false);
        ui->NouvelleDepenseupPushButton ->setEnabled(false);
        EnregupPushButton               ->setVisible(true);
        AnnulupPushButton               ->setVisible(true);
        gBigTable                       ->setEnabled(false);
        disconnect (gBigTable, 0,0,0);
        ui->DateDepdateEdit             ->setDate(QDate::currentDate());
        EnregupPushButton               ->setText("Enregistrer");
        ui->OKupPushButton              ->setShortcut(QKeySequence());
        ui->ModifierupPushButton        ->setShortcut(QKeySequence());
        EnregupPushButton               ->setShortcut(QKeySequence("Meta+Return"));
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Supprimer une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::SupprimerDepense()
{
    if (gBigTable->selectedRanges().size() == 0) return;
    // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte
    QString VerifArchivesrequete = " select * from " NOM_TABLE_ARCHIVESBANQUE " where idDep = " + QString::number(idDepEnCours);
    QSqlQuery VerifArchivQuery (VerifArchivesrequete,db);
    if (VerifArchivQuery.size() > 0)
    {
        UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
        return;
    }

    QDate   Dateop;
    QString Libelle;
    double  montant;
    QString requete1 = "select DateDep, Objet, Montant from " NOM_TABLE_DEPENSES " where iddep = " + QString::number(idDepEnCours);
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
        QString datedep = Dateop.toString("dd MMM yyyy");
        UpMessageBox msgbox;
        msgbox.setText(tr("Supprimer une dépense!"));
        msgbox.setInformativeText(tr("Confirmer la suppression de\n") + datedep + "\n" + Libelle + "\n" + QLocale().toString(montant,'f',2) + "?");
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton *OKBouton = new UpSmallButton();
        OKBouton->setText(tr("Supprimer"));
        UpSmallButton *NoBouton = new UpSmallButton();
        NoBouton->setText(tr("Annuler"));
        msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != OKBouton)
        {
            delete NoBouton;
            delete OKBouton;
            return;
        }
        delete NoBouton;
        delete OKBouton;


        //On supprime l'écriture
        QString SupprComptesrequete = " delete from " NOM_TABLE_LIGNESCOMPTES " where idDep = " + QString::number(idDepEnCours);
        QSqlQuery (SupprComptesrequete,db);

        QString SupprDepenserequete = "delete from " NOM_TABLE_DEPENSES " where idDep = " + QString::number(idDepEnCours);
        QSqlQuery (SupprDepenserequete,db);

        if (gBigTable->rowCount() < 2)
        {
            ReconstruitListeAnnees();
            QString year = QDate::currentDate().toString("yyyy");
            int idx = ui->AnneecomboBox->findText(year);
            ui->AnneecomboBox->setCurrentIndex(idx==-1? 0 : idx);
        }
        else
        {
            for (int i = 0; i< gBigTable->rowCount(); i++)
            {
                UpLabel *iddeplbl = static_cast<UpLabel *>(gBigTable->cellWidget(i,0));
                if (iddeplbl->text() == QString::number(idDepEnCours))
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
        CalculTotalDepenses();
        MetAJourFiche();
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Recalcule le total des dépenses -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::CalculTotalDepenses()
{
    double Total = 0;
    if (gBigTable->rowCount() > 0)
    {
        for (int k = 0; k < gBigTable->rowCount(); k++)
        {
            UpLabel* Line = dynamic_cast<UpLabel*>(gBigTable->cellWidget(k,3));
            if (Line)
                Total = Total + QLocale().toDouble(Line->text());
            else
                Total = Total + QLocale().toDouble(gBigTable->item(k,3)->text());
        }
    }
    QString TotalRemise;
    TotalRemise = QLocale().toString(Total,'f',2);
    QString AnneeRubrique2035 = tr("Total général");
    if (ui->Rubriques2035comboBox->currentText() != "<Aucun>")
        AnneeRubrique2035 = tr("Total ") + ui->Rubriques2035comboBox->currentText();
    ui->TotallineEdit->setText(AnneeRubrique2035 + " " + ui->AnneecomboBox->currentText() + " -> " + TotalRemise);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Ouvre la gestion des comptes -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::GestionComptes()
{
    Dlg_Cmpt          = new dlg_comptes(proc, this);
    if (Dlg_Cmpt->getInitOK())
        Dlg_Cmpt->exec();
    gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Met à jour la fiche --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::MetAJourFiche()
{
    if (gMode == Lire && gBigTable->rowCount() > 0)
    {
        disconnect (ui->DateDepdateEdit,    0,0,0);
        disconnect (ui->RefFiscalecomboBox, 0,0,0);
        disconnect (ui->PaiementcomboBox,   0,0,0);

        UpLabel* idDeplbl = static_cast<UpLabel*>(gBigTable->cellWidget(gBigTable->currentRow(),0));
        idDepEnCours = idDeplbl->text().toInt();
        QString MetAJourrequete = "select DateDep, Objet, Montant, ModePaiement, compte, Reffiscale from " NOM_TABLE_DEPENSES " where idDep = " + idDeplbl->text();
        QSqlQuery ChercheDepQuery (MetAJourrequete,db);
        ChercheDepQuery.first();

        ui->ObjetlineEdit->setText(ChercheDepQuery.value(1).toString());
        ui->DateDepdateEdit->setDate(ChercheDepQuery.value(0).toDate());
        ui->MontantlineEdit->setText(QLocale().toString(ChercheDepQuery.value(2).toDouble(),'f',2));
        QString A = ChercheDepQuery.value(3).toString();                                                         // Mode de paiement - col = 4
        QString B = "";
        if (A == "E")           A = tr("Espèces");
        else
        {
            int row = glistComptesAvecDesactive->findItems(ChercheDepQuery.value(4).toString()).at(0)->row();
            B       = glistComptesAvecDesactive->item(row, 1)->text();
            if (A == "B")       A = tr("Carte de crédit");
            else if (A == "T")  A = tr("TIP");
            else if (A == "V")  A = tr("Virement");
            else if (A == "P")  A = tr("Prélèvement");
            else if (A == "C")  A = tr("Chèque");
        }
        ui->PaiementcomboBox    ->setCurrentText(A);
        ui->ComptesupComboBox   ->setCurrentIndex(ui->ComptesupComboBox->findText(B));
        ui->ComptesupComboBox   ->setVisible(B != "");
        ui->RefFiscalecomboBox  ->setCurrentText(ChercheDepQuery.value(5).toString());

        if (ChercheDepQuery.value(3).toString() != "E")            // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte pour savoir si on peut la modifier
        {
            bool modifiable = true;
            QString VerifArchivesrequete = " select idLigne from " NOM_TABLE_ARCHIVESBANQUE " where idDep = " + QString::number(idDepEnCours);
            QSqlQuery VerifArchivQuery (VerifArchivesrequete,db);
            if (VerifArchivQuery.size() > 0)
                modifiable = false;
            if (modifiable)
            {
                QDate   Dateop;
                QString Libelle;
                double  montant;
                QString requete1 = "select DateDep, Objet, Montant from " NOM_TABLE_DEPENSES " where iddep = " + QString::number(idDepEnCours);
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
                        modifiable = false;
                }
            }
            if (gBigTable->selectedRanges().size() > 0) ui->SupprimerupPushButton->setEnabled(modifiable);
        }
        else
            ui->SupprimerupPushButton->setEnabled(true);
        connect (ui->DateDepdateEdit,       &QDateEdit::dateChanged,        [=] {EnableModifiepushButton();});
        connect (ui->PaiementcomboBox,      &QComboBox::currentTextChanged, [=] {EnableModifiepushButton();});
        connect (ui->PaiementcomboBox,      QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                            [=](int) {ChoixPaiement();});
        connect (ui->RefFiscalecomboBox,    &QComboBox::currentTextChanged, [=] {EnableModifiepushButton();});
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Modifier une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::ModifierDepense()
{
    if (gMode == Enregistrer)
    {
        EnregistreDepense();
        return;
    }
    UpLabel* idDeplbl = static_cast<UpLabel*>(gBigTable->cellWidget(gBigTable->currentRow(),0));
    QString idDep = idDeplbl->text();

    bool OnSauteLaQuestionSuivante = false;
    QString pb = "";

    //  Vérifer que la saisie est complète et cohérente
    QString Erreur = "";
    if (ui->ObjetlineEdit->text() == "")
        Erreur = tr("l'objet de la dépense");
    else if (!(QLocale().toDouble(ui->MontantlineEdit->text()) > 0))
        Erreur = tr("le montant");
    else if (ui->PaiementcomboBox->currentText() == "")
        Erreur = tr("le mode de paiement");
    else if (ui->RefFiscalecomboBox->currentText() == "")
        Erreur = tr("la rubrique fiscale");

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
        return;
    }

    // vérifier que cette dépense n'a pas été déjà saisie
    QString requete = "select DateDep from " NOM_TABLE_DEPENSES " where DateDep = '" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
            "'and Objet = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
            "'and Montant = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
            " and iddep <> " + idDep +
            " and idUser = " + QString::number(proc->getDataUser()->getIdUserComptable());
    QSqlQuery ChercheDepQuery (requete,db);
    if (ChercheDepQuery.size() > 0)
    {
        pb = tr("Elle a déjà été saisie");
        OnSauteLaQuestionSuivante = true;
    }

    if (!OnSauteLaQuestionSuivante)
    {
        if (QDate::currentDate() > ui->DateDepdateEdit->date().addDays(90))
        {
            pb = tr("Elle date de plus de 3 mois");
            OnSauteLaQuestionSuivante = true;
        }
    }
    if (!OnSauteLaQuestionSuivante)
    {
        requete = "select DateDep from " NOM_TABLE_DEPENSES " where Datedep < '" + ui->DateDepdateEdit->date().addDays(-1).toString("yyyy-MM-dd") +
                "'and Objet = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                "'and Montant = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                " and idUser = " + QString::number(proc->getDataUser()->getIdUserComptable());
        QSqlQuery ChercheDep2Query (requete,db);
        if (ChercheDep2Query.size() > 0)
        {
            ChercheDep2Query.last();
            if (pb != "")
                pb += "\n";
            pb += tr("Une dépense semblable a été saisie\nle ") + ChercheDep2Query.value(0).toDate().toString("dd MMM yyyy");
        }
    }

    if (pb != "")
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Il y a un problème avec cette dépense!"));
        msgbox.setInformativeText(pb + "\n" + tr("Confirmer la saisie?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton *OKBouton = new UpSmallButton();
        if (pb == tr("Elle a déjà été saisie"))
            OKBouton->setText("OK");
        else
            OKBouton->setText(tr("Oui"));
        UpSmallButton *NoBouton = new UpSmallButton();
        NoBouton->setText("Non");
        msgbox.addButton(NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != OKBouton || pb == tr("Elle a déjà été saisie")){
            delete OKBouton;
            delete NoBouton;
            return;}
        delete OKBouton;
        delete NoBouton;
    }

    // Correction de l'écriture dans la table depenses
    QString Paiement, m;
    Paiement = proc->CorrigeApostrophe(ui->PaiementcomboBox->currentText());
    if (Paiement == tr("Espèces"))              m = "E";
    else if (Paiement == tr("Virement"))        m = "V";
    else if (Paiement == tr("Carte de crédit")) m = "B";
    else if (Paiement == tr("Chèque"))          m = "C";
    else if (Paiement == tr("Prélèvement"))     m = "P";
    else if (Paiement == tr("TIP"))             m = "T";

    QString FamFiscale;
    requete = "select Famfiscale from " NOM_TABLE_RUBRIQUES2035 " where reffiscale = '"
            + proc->CorrigeApostrophe(ui->RefFiscalecomboBox->currentText()) +"'";
    QSqlQuery cherchefamfiscalequery (requete,db);
    DataBase::getInstance()->traiteErreurRequete(cherchefamfiscalequery,requete,"");
    if (cherchefamfiscalequery.size() > 0)
    {
        cherchefamfiscalequery.first();
        FamFiscale = cherchefamfiscalequery.value(0).toString();
    }
    QString idCompte = ui->ComptesupComboBox->currentData().toString();
    QString modifdeprequete = "update " NOM_TABLE_DEPENSES
            " set DateDep = '" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
            "', Objet = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
            "', Montant = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
            ", RefFiscale = '" + proc->CorrigeApostrophe(ui->RefFiscalecomboBox->currentText()) +
            "', FamFiscale = '" + proc->CorrigeApostrophe(FamFiscale) +
            "', ModePaiement = '" + m +
            "', Compte = " + (m!="E"? idCompte : "null") +
            " where idDep = " + idDep;
    QSqlQuery ModifieDepenseQuery (modifdeprequete,db);
    DataBase::getInstance()->traiteErreurRequete(ModifieDepenseQuery,modifdeprequete,tr("Impossible d'enregister cete dépense!"));

    // Correction de l'écriture dans la table lignescomptes
    if (Paiement == tr("Espèces"))
    {
        QString requete = "delete from " NOM_TABLE_LIGNESCOMPTES " where iddep = " + idDep;
        QSqlQuery (requete,db);
    }
    else
    {
        Paiement = ui->PaiementcomboBox->currentText();
        if (Paiement == tr("Virement")) Paiement = tr("Virement débiteur");

        // on recherche si l'écriture existe dans lignescomptes et si c'est le cas, on la modifie
        QString requete = "select idLigne from " NOM_TABLE_LIGNESCOMPTES " where iddep = " + idDep;
        QSqlQuery cherchequery(requete,db);
        if (cherchequery.size() > 0)                // l'écriture existe et on la modifie
        {
            requete = "update " NOM_TABLE_LIGNESCOMPTES
                    " set LigneDate = '" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
                    "', LigneLibelle = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                    "', LigneMontant = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                    ", LigneDebitCredit = 0, LigneTypeOperation = '" + Paiement +
                    "', idCompte = " + (m!="E"? idCompte : "null") +
                    " where idDep = " +idDep;
            QSqlQuery (requete,db);
            //UpMessageBox::Watch(this,requete);
        }
        else           // on n'a pas trouvé la ligne, on la recherche dans les archives
        {
            QString Archrequete = "select idLigne from " NOM_TABLE_ARCHIVESBANQUE " where iddep = " + idDep;
            QSqlQuery Archcherchequery(Archrequete,db);
            if (Archcherchequery.size() > 0)                // l'écriture existe et on la modifie
            {
                QString Arch2requete = "update " NOM_TABLE_ARCHIVESBANQUE
                        " set LigneDate = '" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
                        "', LigneLibelle = '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                        "', LigneDebitCredit = 0, LigneTypeOperation = '" + Paiement +
                        "' where idDep = " +idDep;
                //UpMessageBox::Watch(this,Arch2requete);
                QSqlQuery (Arch2requete,db);
            }
            else        // l'écriture n'existait ni dans lignescomptes ni dans archives
                        // => c'était une dépense en espèces
                        // on l'enregistre dans lignescomptes
            {
                requete = "insert into " NOM_TABLE_LIGNESCOMPTES "(idCompte, idDep, LigneDate, Lignelibelle, LigneMontant, LigneDebitCredit, LigneTypeoperation) VALUES (" +
                            idCompte +
                            "," + idDep +
                            ", '" + ui->DateDepdateEdit->date().toString("yyyy-MM-dd") +
                            "', '" + proc->CorrigeApostrophe(ui->ObjetlineEdit->text()) +
                            "', "  + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                            ", 0, '" + Paiement + "')";
                QSqlQuery EnregLigneCompeQuery(requete,db);
                DataBase::getInstance()->traiteErreurRequete(EnregLigneCompeQuery,requete,tr("Impossible d'enregister cette opération sur le compte bancaire!"));
            }
        }
    }
    gBigTable->setEnabled(true);
    int year = ui->DateDepdateEdit->date().year();
    if (ui->AnneecomboBox->currentText() != QString::number(year))
    {
        if (ui->AnneecomboBox->findText(QString::number(year)) < 0)
            ReconstruitListeAnnees();
        ui->AnneecomboBox->setCurrentText(QString::number(year));
    }

    RedessineBigTable();
    bool trouve = false;
    for (int i=0; i< gBigTable->rowCount(); i++)
    {
        UpLabel* idDeplbl = static_cast<UpLabel*>(gBigTable->cellWidget(i,0));
        if (idDeplbl->text() == idDep){
            gBigTable->setCurrentCell(i,1);
            i = gBigTable->rowCount();
            trouve = true;
        }
    }
    if (!trouve)    gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
    gMode = Lire;
    MetAJourFiche();
    RegleAffichageFiche(Lire);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::RedessineBigTable()
{
    RemplitBigTable();
    CalculTotalDepenses();
    if (gBigTable->rowCount() > 0)
    {
        RegleAffichageFiche(Lire);
        gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
        gBigTable->scrollTo(gBigTable->model()->index(gBigTable->model()->rowCount()-1,1));
    }
    else
        RegleAffichageFiche(TableVide);
    ui->SupprimerupPushButton->setEnabled(gBigTable->rowCount()>0);
    ui->ModifierupPushButton->setEnabled(gBigTable->rowCount()>0);
}

void dlg_depenses::closeEvent(QCloseEvent *event)
{
    proc->gsettingsIni->setValue("PositionsFiches/PositionDepenses",saveGeometry());
    event->accept();
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void dlg_depenses::keyPressEvent ( QKeyEvent * event )
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

bool dlg_depenses::getInitOK()
{
    return InitOK;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Definit les colonnes de BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::DefinitArchitectureBigTable()
{
    int                 ColCount;

    ColCount = 8;
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
    LabelARemplir << tr("Dépense");
    LabelARemplir << tr("Montant");
    LabelARemplir << tr("Mode de paiement");
    LabelARemplir << tr("Rubrique 2035");
    LabelARemplir << tr("Famille rubrique");
    LabelARemplir << tr("Classement par date");

    gBigTable->setHorizontalHeaderLabels(LabelARemplir);
    gBigTable->horizontalHeader()->setVisible(true);
    gBigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    gBigTable->setColumnWidth(li,25);                                               // idDepense
    li++;
    gBigTable->setColumnWidth(li,100);                                              // DepDate affichage européen
    li++;
    gBigTable->setColumnWidth(li,330);                                              // DepNom
    li++;
    gBigTable->setColumnWidth(li,100);                                              // DepMontant
    li++;
    gBigTable->setColumnWidth(li,160);                                              // DepModePaiement
    li++;
    gBigTable->setColumnWidth(li,300);                                              // Rubrique 2035
    li++;
    gBigTable->setColumnWidth(li,293);                                              // Famille rubrique
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
void dlg_depenses::ReconstruitListeAnnees()
{
    QString requete = "SELECT distinct Annee from (SELECT year(DateDEP) as Annee FROM " NOM_TABLE_DEPENSES " WHERE idUser = " + QString::number(gidUserADebiter) + ") as ghf order by Annee";
    QSqlQuery ChercheAnneesQuery (requete,db);
    QStringList ListeAnnees;
    for (int i = 0; i < ChercheAnneesQuery.size(); i++)
    {
            ChercheAnneesQuery.seek(i);
            ListeAnnees << ChercheAnneesQuery.value(0).toString();
    }
    disconnect (ui->AnneecomboBox,  0,0,0);
    ui->AnneecomboBox->clear();
    ui->AnneecomboBox->insertItems(0,ListeAnnees);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int) {RedessineBigTable();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des rubriques 2035  de l'utilisateur dans le combobox Rubriques --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::ReconstruitListeRubriques()
{
    QString requete = "SELECT distinct reffiscale from " NOM_TABLE_DEPENSES " WHERE idUser = " + QString::number(gidUserADebiter) + " order by reffiscale";
    QSqlQuery ChercheRubriquesQuery (requete,db);
    QStringList ListeRubriques;
    ListeRubriques << "<Aucun>";
    for (int i = 0; i < ChercheRubriquesQuery.size(); i++)
    {
            ChercheRubriquesQuery.seek(i);
            ListeRubriques << ChercheRubriquesQuery.value(0).toString();
    }
    ui->Rubriques2035comboBox->insertItems(0,ListeRubriques);
    ui->Rubriques2035comboBox->setCurrentText(ListeRubriques.at(0));
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste de toutes les rubriques 2035 --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::ReconstruitListeToutesRubriques()
{
    QString requete = "SELECT distinct reffiscale from " NOM_TABLE_DEPENSES " order by reffiscale";
    QSqlQuery ChercheRubriquesQuery (requete,db);
    for (int i = 0; i < ChercheRubriquesQuery.size(); i++)
    {
        ChercheRubriquesQuery.seek(i);
        glistRefFiscales << ChercheRubriquesQuery.value(0).toString();
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::RemplitBigTable()
{
    QTableWidgetItem    *pItem8;
    QString             A;
    UpLabel *label0, *label1, *label2, *label3, *label4, *label5, *label6;
    disconnect (gBigTable, 0,0,0);
    gBigTable->clearContents();
    QString Deprequete = "SELECT idDep, idUser, year(DateDep) as Annee, DateDep , RefFiscale, Objet, Montant,"
                         "FamFiscale, Nooperation, Monnaie, ModePaiement, Compte, NoCheque FROM " NOM_TABLE_DEPENSES
                         " WHERE idUser = " + QString::number(gidUserADebiter);
    if (ui->AnneecomboBox->currentText() != "")
        Deprequete += " AND year(DateDep) = " + ui->AnneecomboBox->currentText();
    if (ui->Rubriques2035comboBox->currentText() != ui->Rubriques2035comboBox->itemText(0))
        Deprequete += " AND RefFiscale = '" + proc->CorrigeApostrophe(ui->Rubriques2035comboBox->currentText()) + "'";
    Deprequete += " ORDER BY DateDep, Objet";
    //proc->Edit(Deprequete);
    QSqlQuery EnumDepensesQuery (Deprequete,db);
    if (DataBase::getInstance()->traiteErreurRequete(EnumDepensesQuery,Deprequete,"Impossible de retrouver la table des depenses"))
        return;
    gBigTable->setRowCount(EnumDepensesQuery.size());
    EnumDepensesQuery.first();

    for (int i = 0; i < EnumDepensesQuery.size(); i++)
        {
            int col = 0;
            int id = EnumDepensesQuery.value(0).toInt();

            label0 = new UpLabel;
            label1 = new UpLabel;
            label2 = new UpLabel;
            label3 = new UpLabel;
            label4 = new UpLabel;
            label5 = new UpLabel;
            label6 = new UpLabel;

            label0->setId(id);                      // idDep
            label1->setId(id);                      // idDep
            label2->setId(id);                      // idDep
            label3->setId(id);                      // idDep
            label4->setId(id);                      // idDep
            label5->setId(id);                      // idDep
            label6->setId(id);                      // idDep

            label0->setContextMenuPolicy(Qt::CustomContextMenu);
            label1->setContextMenuPolicy(Qt::CustomContextMenu);
            label2->setContextMenuPolicy(Qt::CustomContextMenu);
            label3->setContextMenuPolicy(Qt::CustomContextMenu);
            label4->setContextMenuPolicy(Qt::CustomContextMenu);
            label5->setContextMenuPolicy(Qt::CustomContextMenu);
            label6->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(label0,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(cursor().pos(), label0);});
            connect(label1,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(cursor().pos(), label1);});
            connect(label2,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(cursor().pos(), label2);});
            connect(label3,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(cursor().pos(), label3);});
            connect(label4,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(cursor().pos(), label4);});
            connect(label5,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(cursor().pos(), label5);});
            connect(label6,  &QWidget::customContextMenuRequested,   [=] {MenuContextuel(cursor().pos(), label6);});

            A = EnumDepensesQuery.value(0).toString();                                                          // idDepense - col = 0
            label0->setText(A);
            gBigTable->setCellWidget(i,col,label0);
            col++;

            A = EnumDepensesQuery.value(3).toDate().toString(tr("d MMM yyyy"));
            label1->setText(A + " ");
            label1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            gBigTable->setCellWidget(i,col,label1);
            col++;

            label2->setText(" " + EnumDepensesQuery.value(5).toString());                                               // Objet - col = 2
            gBigTable->setCellWidget(i,col,label2);
            col++;

            if (EnumDepensesQuery.value(10).toString() == "F")
                A = QLocale().toString(EnumDepensesQuery.value(6).toDouble()/6.55957,'f',2);// Montant en F converti en euros
            else
                A = QLocale().toString(EnumDepensesQuery.value(6).toDouble(),'f',2);                                      // Montant - col = 3
            label3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            label3->setText(A + " ");                                                                                   // Montant - col = 3
            gBigTable->setCellWidget(i,col,label3);
            col++;

            A = EnumDepensesQuery.value(10).toString();                                                                 // Mode de paiement - col = 4
            QString B = "";
            QString C = "";
            if (A == "E")           A = tr("Espèces");
            else
            {
                QList<QStandardItem*> litemlist = glistComptesAvecDesactive->findItems(EnumDepensesQuery.value(11).toString());
                if (litemlist.size()>0)
                {
                    int row = litemlist.at(0)->row();
                    B = glistComptesAvecDesactive->item(row, 1)->text();
                }
                if (A == "B")       A = tr("Carte de crédit");
                else if (A == "T")  A = tr("TIP");
                else if (A == "V")  A = tr("Virement");
                else if (A == "P")  A = tr("Prélèvement");
                else if (A == "C") {
                    A = tr("Chèque ");
                    if (EnumDepensesQuery.value(12).toInt() > 0) C += EnumDepensesQuery.value(12).toString();}
            }
            A += " " + B + " " + C;
            label4->setText(" " + A);
            gBigTable->setCellWidget(i,col,label4);
            col++;

            A = EnumDepensesQuery.value(4).toString();                                                                  // Rubrique2035 - col = 5
            label5->setText(" " + A);
            gBigTable->setCellWidget(i,col,label5);
            col++;

            A = EnumDepensesQuery.value(7).toString();                                                                  // Famille fiscale - col = 6
            label6->setText(" " + A);
            gBigTable->setCellWidget(i,col,label6);
            col++;

            A = EnumDepensesQuery.value(3).toDate().toString("yyyy-MM-dd");                                             // ClassementparDate - col = 7
            pItem8 = new QTableWidgetItem() ;
            pItem8->setText(A);
            gBigTable->setItem(i,col,pItem8);

            gBigTable->setRowHeight(i,QFontMetrics(qApp->font()).height()*1.3);

            EnumDepensesQuery.next();
        }
    ui->SupprimerupPushButton->setEnabled(false);
    connect (gBigTable,     &QTableWidget::itemSelectionChanged, [=] {MetAJourFiche();});
}



