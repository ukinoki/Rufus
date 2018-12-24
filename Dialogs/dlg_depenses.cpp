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

static inline double mmToInches(double mm) { return mm * 0.039370147; }

dlg_depenses::dlg_depenses(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_depenses)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    proc        = Procedures::I();
    db          = DataBase::getInstance();
    ui->UserscomboBox->setEnabled(proc->getUserConnected()->isSecretaire() );

    m_listUserLiberaux = Datas::I()->users->liberaux();

    int index = 0;
    bool foundUser = false;
    int currentIdUser = proc->getUserConnected()->id(); //Utilisateur connecte
    for( QMap<int, User*>::const_iterator itUser = m_listUserLiberaux->constBegin(); itUser != m_listUserLiberaux->constEnd(); ++itUser )
    {
        User *user = const_cast<User*>(itUser.value());
        ui->UserscomboBox->addItem(user->getLogin(), QString::number(user->id()) );
        if( !foundUser )
        {
            if(currentIdUser != user->id())
                ++index;
            else
                foundUser = true;
        }
    }
    if(index>=m_listUserLiberaux->size())
        ui->UserscomboBox->setCurrentIndex(0);
    else
        ui->UserscomboBox->setCurrentIndex(index);

    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionDepenses").toByteArray());

    InitOK = initializeUserSelected();
    if( !InitOK )
        return;

    setMaximumSize(1642,800);

    gBigTable       = new UpTableWidget(this);
    ui->horizontalLayout_3->addWidget(gBigTable);

    ModifierupPushButton = new UpPushButton(ui->frame);
    ModifierupPushButton->setFixedHeight(46);
    ModifierupPushButton->setText(tr("Modifier"));
    ModifierupPushButton->setIcon(Icons::icMarteau());
    ModifierupPushButton->setIconSize(QSize(25,25));

    SupprimerupPushButton = new UpPushButton(ui->frame);
    SupprimerupPushButton->setFixedHeight(46);
    SupprimerupPushButton->setText(tr("Supprimer"));
    SupprimerupPushButton->setIcon(Icons::icPoubelle());
    SupprimerupPushButton->setIconSize(QSize(25,25));

    EnregupPushButton = new UpPushButton(ui->frame);
    EnregupPushButton->setFixedHeight(46);
    EnregupPushButton->setText(tr("Valider"));
    EnregupPushButton->setIcon(Icons::icOK());
    EnregupPushButton->setIconSize(QSize(25,25));

    AnnulupPushButton = new UpPushButton(ui->frame);
    AnnulupPushButton->setFixedHeight(46);
    AnnulupPushButton->setText(tr("Annuler"));
    AnnulupPushButton->setIcon(Icons::icAnnuler());
    AnnulupPushButton->setIconSize(QSize(25,25));

    boxbutt = new QHBoxLayout();
    boxbutt->addWidget(AnnulupPushButton);
    boxbutt->addSpacerItem(new QSpacerItem(0,0));
    boxbutt->addWidget(EnregupPushButton);
    boxbutt->addSpacerItem(new QSpacerItem(0,0));
    boxbutt->addWidget(SupprimerupPushButton);
    boxbutt->addSpacerItem(new QSpacerItem(0,0));
    boxbutt->addWidget(ModifierupPushButton);
    boxbutt->setSpacing(2);
    boxbutt->setContentsMargins(0,0,0,0);
    ui->frame->layout()->addItem(boxbutt);

    ui->frame->setStyleSheet("QFrame#frame{border: 1px solid gray; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 210, 210, 50));}");
    ui->VisuDocupTableWidget->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30

    QStringList ListeRubriques = db->ListeRubriquesFiscales();
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

    int margemm         = proc->TailleTopMarge(); // exprimé en mm
    printer             = new QPrinter(QPrinter::HighResolution);
    printer             ->setFullPage(true);
    rect                = printer->paperRect();
    rect.adjust(mmToInches(margemm) * printer->logicalDpiX(),
                mmToInches(margemm) * printer->logicalDpiY(),
                -mmToInches(margemm) * printer->logicalDpiX(),
                -mmToInches(margemm) * printer->logicalDpiY());

    DefinitArchitectureBigTable();

    ReconstruitListeRubriques();
    ReconstruitListeAnnees();
    QString year = QDate::currentDate().toString("yyyy");
    int idx = ui->AnneecomboBox->findText(year);
    ui->AnneecomboBox->disconnect();
    ui->AnneecomboBox->setCurrentIndex(idx==-1? 0 : idx);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=](int) {RedessineBigTable();});
    RedessineBigTable();

    connect (ui->GestionComptesupPushButton,    &QPushButton::clicked,          this,   &dlg_depenses::GestionComptes);
    connect (ui->NouvelleDepenseupPushButton,   &QPushButton::clicked,          this,   [=] {GererDepense(ui->NouvelleDepenseupPushButton);});
    connect (ModifierupPushButton,              &QPushButton::clicked,          this,   [=] {GererDepense(ModifierupPushButton);});
    connect (ui->OKupPushButton,                &QPushButton::clicked,          this,   &dlg_depenses::accept);
    connect (ui->Rubriques2035comboBox,         QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                this,   [=](int) {FiltreTable();});
    connect (ui->FactureupPushButton,           &QPushButton::clicked,          this,   [=] {ScanDoc(FACTURE);});
    connect (ui->EcheancierupPushButton,        &QPushButton::clicked,          this,   [=] {ScanDoc(ECHEANCIER);});
    connect (ui->ExportupPushButton,            &QPushButton::clicked,          this,   &dlg_depenses::ExportTable);
    connect (ui->MontantlineEdit,               &QLineEdit::editingFinished,    this,   &dlg_depenses::ConvertitDoubleMontant);
    connect (ui->PaiementcomboBox,              QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                this,   [=](int) {ChoixPaiement();});
    connect (ui->ObjetlineEdit,                 &QLineEdit::textEdited,         this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->MontantlineEdit,               &QLineEdit::textEdited,         this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->DateDepdateEdit,               &QDateEdit::dateChanged,        this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->PaiementcomboBox,              &QComboBox::currentTextChanged, this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->RefFiscalecomboBox,            &QComboBox::currentTextChanged, this,   &dlg_depenses::EnableModifiepushButton);
    connect (SupprimerupPushButton,             &QPushButton::clicked,          this,   &dlg_depenses::SupprimerDepense);
    connect (ui->UserscomboBox,                 QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                this,   [=](int) {ChangeUser(ui->UserscomboBox->currentIndex());});
    connect (ui->VisuDocupTableWidget,          SIGNAL(zoom()),                 this,   SLOT(ZoomDoc()));
    connect (EnregupPushButton,                 &QPushButton::clicked,          this,   &dlg_depenses::ModifierDepense);
    connect (AnnulupPushButton,                 &QPushButton::clicked,          this,   &dlg_depenses::AnnulEnreg);

    gBigTable->setFocus();
    ui->ExportupPushButton->setEnabled(gBigTable->rowCount()>0);

    //ui->Facturewidget->setVisible(false);
    //ui->VisuDocupTableWidget->setVisible(false);

    InitOK= true;
}

dlg_depenses::~dlg_depenses()
{
    delete ui;
}

void dlg_depenses::ExportTable()
{
    QByteArray ExportEtat;
    QString sep = "\t";                                                                                                            // séparateur
    if (UpMessageBox::Question(this,
                               tr("Exportation de la table des dépenses"),
                               tr("Voulez-vous exporter la table des dépenses?\n(Format CSV, langue anglais(USA), champs séparés par tabulation)"))
            != UpSmallButton::STARTBUTTON)
        return;
    ExportEtat.append(tr("Date") + sep + tr("Dépense") + sep + tr("Montant") + sep + tr("Mode de paiement") + sep + tr("Rubrique 2035") + sep + tr("Famille fiscale") + "\n");
    for (int i=0;i< gBigTable->rowCount(); i++)
    {
        if (!gBigTable->isRowHidden(i))
        {
            QString date = QDate::fromString(gBigTable->item(i,7)->text(), "yyyy-MM-dd").toString("dd/MM/yyyy");
            ExportEtat.append(date + sep);                                                                              // Date - col = 7
            ExportEtat.append(static_cast<UpLabel*>(gBigTable->cellWidget(i,2))->text() + sep);                         // Libelle - col = 2
            double montant = QLocale().toDouble(static_cast<UpLabel*>(gBigTable->cellWidget(i,3))->text().remove(" "));
            ExportEtat.append(QString::number(montant) + sep);                                                          // Montant - col = 3
            ExportEtat.append(static_cast<UpLabel*>(gBigTable->cellWidget(i,4))->text() + sep);                         // Mode de paiement - col = 4
            ExportEtat.append(static_cast<UpLabel*>(gBigTable->cellWidget(i,5))->text() + sep);                         // Rubrique 2035 - col = 5
            ExportEtat.append(static_cast<UpLabel*>(gBigTable->cellWidget(i,6))->text());                               // Famille fiscale - col = 6
            ExportEtat.append("\n");
        }
    }
    QString ExportFileName = QDir::homePath() + NOMDIR_RUFUS + "/"
                            + tr("Depenses") + " " + ui->UserscomboBox->currentText() + " "
                            + tr("Année") + " " + ui->AnneecomboBox->currentText()
                            + ".csv";
    QFile   ExportFile(ExportFileName);
    bool exportOK = true;
    QString msg = tr("Nom du fichier d'export") + "\n" + ExportFileName;
    ExportFile.remove();
    if (ExportFile.open(QIODevice::Append))
    {
        QTextStream out(&ExportFile);
        out << ExportEtat;
        ExportFile.close();
    }
    else
        exportOK = false;
    UpMessageBox::Watch(this, (exportOK? tr("Exportation réussie") : tr("Echec exportation")), (exportOK? msg : tr("Les données n'ont pas pu être exportées")));
}

void dlg_depenses::RegleComptesComboBox(bool ActiveSeult)
{
    ui->ComptesupComboBox->clear();
    QMultiMap<int, Compte*> model = (ActiveSeult? gDataUser->getComptes()->comptes() : gDataUser->getComptes()->comptesAll());
    for( QMultiMap<int, Compte*>::const_iterator itCompte = model.constBegin(); itCompte != model.constEnd(); ++itCompte )
       ui->ComptesupComboBox->addItem(itCompte.value()->nom(), QString::number(itCompte.value()->id()) );
}

void    dlg_depenses::RegleAffichageFiche(enum gMode mode)
{
    gMode = mode;
    Depense *dep = (gBigTable->rowCount()>0? getDepenseFromRow(gBigTable->currentRow()) : Q_NULLPTR);

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
    ui->frame               ->setVisible(gMode != TableVide);

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
    ui->GestionComptesupPushButton  ->setEnabled(gMode == Lire || gMode == TableVide);
    EnregupPushButton               ->setVisible(!(gMode == Lire || gMode == TableVide));
    AnnulupPushButton               ->setVisible(!(gMode == Lire || gMode == TableVide));
    ui->Facturewidget         ->setVisible(gMode == Lire);
    ui->EcheancierupPushButton      ->setVisible(gMode == Lire);
    ui->NouvelleDepenseupPushButton ->setEnabled((gMode == Lire || gMode == TableVide) && gDataUser->getComptes()->comptes().size() );
    QString ttip = "";
    if( gDataUser->getComptes()->comptes().size() == 0)
        ttip = tr("Vous ne pouvez pas enregistrer de dépenses.\nAucun compte bancaire n'est enregistré.");
    ui->NouvelleDepenseupPushButton->setToolTip(ttip);
    SupprimerupPushButton   ->setVisible(gMode == Lire);
    ModifierupPushButton    ->setVisible(gMode == Lire);
    gBigTable               ->setEnabled(gMode == Lire);

    ui->UserscomboBox        ->setEnabled(proc->getUserConnected()->isSecretaire() && gMode==Lire);


    switch (gMode) {
    case TableVide:
        ui->OKupPushButton      ->setShortcut(QKeySequence("Meta+Return"));
        ModifierupPushButton    ->setShortcut(QKeySequence());
        EnregupPushButton       ->setShortcut(QKeySequence());
        [[clang::fallthrough]];// => pas de break, on continue avec le code de Lire
    case Lire: {
        EnregupPushButton       ->setText(tr("Modifier"));
        RegleComptesComboBox(false);
        break;
    }
    case Modifier: {
        gBigTable->disconnect();
        EnregupPushButton       ->setText("Valider");
        int compte = -1;
        ui->ComptesupComboBox->setVisible(dep->modepaiement()!="E");
        if (dep->modepaiement() != "E")            // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte pour savoir si on peut la modifier
        {
            compte = ui->ComptesupComboBox->currentData().toInt();
            ui->ComptesupComboBox   ->setVisible(true);
            if (dep->isArchivee() == Depense::NoLoSo)
                db->loadDepenseArchivee(dep);
            bool modifiable = (dep->isArchivee() == Depense::Non);
            ui->DateDeplabel        ->setEnabled(modifiable);
            ui->DateDepdateEdit     ->setEnabled(modifiable);
            ui->Montantlabel        ->setEnabled(modifiable);
            ui->MontantlineEdit     ->setEnabled(modifiable);
            ui->Paiementlabel       ->setEnabled(modifiable);
            ui->PaiementcomboBox    ->setEnabled(modifiable);
            ui->ComptesupComboBox   ->setEnabled(modifiable);
        }
        ui->OKupPushButton->setShortcut(QKeySequence());
        ModifierupPushButton->setShortcut(QKeySequence());
        EnregupPushButton->setShortcut(QKeySequence("Meta+Return"));
        RegleComptesComboBox();
        ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(compte));
        break;
    }
    case Enregistrer: {
        gBigTable->disconnect();
        ui->DateDepdateEdit     ->setDate(QDate::currentDate());
        ui->ObjetlineEdit       ->setText("");
        ui->MontantlineEdit     ->setText("0,00");
        ui->ComptesupComboBox   ->setVisible(!(ui->PaiementcomboBox->currentText() == tr("Espèces") || ui->PaiementcomboBox->currentText() == ""));
        ui->RefFiscalecomboBox  ->setCurrentText("");
        EnregupPushButton       ->setText(tr("Enregistrer"));
        ui->OKupPushButton      ->setShortcut(QKeySequence());
        ModifierupPushButton->setShortcut(QKeySequence());
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
    gBigTable->disconnect();
    RegleAffichageFiche(Lire);
    MetAJourFiche();
    connect (gBigTable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur courant ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_depenses::initializeUserSelected()
{
    int id = ui->UserscomboBox->currentData().toInt();
    gDataUser = m_listUserLiberaux->find(id).value();
    proc->initListeDepenses(gDataUser->id());
    Comptes *comptes = new Comptes();
    comptes->addCompte( db->loadComptesByUser(gDataUser->id()) );
    gDataUser->setComptes( comptes );
    if( gDataUser->getComptes()->comptesAll().size() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de continuer!"), tr("Pas de compte bancaire enregistré pour ") + gDataUser->getLogin());
        return false;
    }

    QJsonObject data = db->loadUserData(gDataUser->id());
    if(data.isEmpty())
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche paiement!"), tr("Les paramètres de ")
                             + gDataUser->getLogin() + tr("ne sont pas retrouvés"));
        return false;
    }
    gDataUser->setData( data ); //ON charge le reste des données

    if (gDataUser->getIdCompteParDefaut() <= 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir le journal des dépenses!"), tr("Pas de compte bancaire enregistré pour ")
                                     + gDataUser->getLogin());
        return false;
    }

    return true;
}
void dlg_depenses::ChangeUser(int)
{
    InitOK = initializeUserSelected();
    if( !InitOK )
        return;

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
    QString b = ui->MontantlineEdit->text();
    b = QLocale().toString(QLocale().toDouble(b),'f',2);
    ui->MontantlineEdit->setText(b);
}

void dlg_depenses::EnableModifiepushButton()
{
    ModifierupPushButton->setEnabled(true);
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
    else if (ui->ComptesupComboBox->currentIndex()==-1 && ui->PaiementcomboBox->currentText()!= tr("Espèces"))
        Erreur = tr("le compte bancaire");

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
        else if (Erreur == tr("le compte bancaire"))
        {
            ui->ComptesupComboBox->setFocus();
            ui->ComptesupComboBox->showPopup();
        }
        return;
    }

    QList<Depense*> veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->getDepenses(), ui->DateDepdateEdit->date(),
                                                              ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), gDataUser->id(),
                                                              DataBase::Egal);

    // vérifier que cette dépense n'a pas été déjà saisie
    if (veriflistdepenses.size() > 0)
    {
        pb = tr("Elle a déjà été saisie");
        OnSauteLaQuestionSuivante = true;
    }

    if (!OnSauteLaQuestionSuivante)
    {
        if (QDate::currentDate() > ui->DateDepdateEdit->date().addDays(90))
            pb = tr("Elle date de plus de 3 mois");
        veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->getDepenses(), ui->DateDepdateEdit->date().addDays(-180),
                                                  ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), gDataUser->id(),
                                                  DataBase::Sup);
        if (veriflistdepenses.size() > 0)
        {
            Depense *dep = veriflistdepenses.last();
            if (pb != "")
                pb += "\n";
            pb += tr("Une dépense semblable a été saisie le ") + dep->date().toString("dd MMM yyyy");
        }
    }

    if (pb != "")
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Il y a un problème avec cette dépense!" ));
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

    // Insertion de l'écriture dans la table depenses
    QString Paiement, idDep, m;
    Paiement = ui->PaiementcomboBox->currentText();
    if (Paiement == tr("Espèces"))              m = "E";
    else if (Paiement == tr("Virement"))        m = "V";
    else if (Paiement == tr("Carte de crédit")) m = "B";
    else if (Paiement == tr("Chèque"))          m = "C";
    else if (Paiement == tr("Prélèvement"))     m = "P";
    else if (Paiement == tr("TIP"))             m = "T";

    bool ok = true;
    QList<QList<QVariant>> listfamfiscale = db->SelectRecordsFromTable(QStringList() << "Famfiscale",
                                                                       NOM_TABLE_RUBRIQUES2035, ok,
                                                                       "where reffiscale = '" + Utils::correctquoteSQL(ui->RefFiscalecomboBox->currentText()) + "'");
    QString FamFiscale = listfamfiscale.at(0).at(0).toString();
    QString idCompte = ui->ComptesupComboBox->currentData().toString();
    db->locktables(QStringList() << NOM_TABLE_DEPENSES << NOM_TABLE_ARCHIVESBANQUE << NOM_TABLE_LIGNESCOMPTES);

    QHash<QString, QString> listsets;
    listsets.insert("DateDep",       ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
    listsets.insert("idUser",       QString::number(gDataUser->id()));
    listsets.insert("Objet",        ui->ObjetlineEdit->text());
    listsets.insert("Montant",      QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
    listsets.insert("RefFiscale",   ui->RefFiscalecomboBox->currentText());
    listsets.insert("FamFiscale",   FamFiscale);
    listsets.insert("ModePaiement", m);
    listsets.insert("Compte",       (m!="E"? idCompte : "null"));
    DataBase:: getInstance()->InsertIntoTable(NOM_TABLE_DEPENSES, listsets);

    idDep   = QString::number(db->selectMaxFromTable("idDep", NOM_TABLE_DEPENSES));

    // insertion de l'écriture dans la table lignescomptes quand il s'agit d'une opération bancaire
    if (m != "E")
    {
        if (Paiement == tr("Virement")) Paiement = tr("Virement débiteur");
        int a = db->getMaxLigneBanque();
        QHash<QString, QString> listsets;
        listsets.insert("idLigne",              QString::number(a));
        listsets.insert("idCompte",             idCompte);
        listsets.insert("idDep",                idDep);
        listsets.insert("LigneDate",            ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
        listsets.insert("Lignelibelle",         ui->ObjetlineEdit->text());
        listsets.insert("LigneMontant",         QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
        listsets.insert("LigneDebitCredit",     "0");
        listsets.insert("LigneTypeoperation",   Paiement);
        DataBase:: getInstance()->InsertIntoTable(NOM_TABLE_LIGNESCOMPTES, listsets);
    }
    db->commit();

    QJsonObject jData{};
    jData["iddepense"]      = idDep.toInt();
    jData["iduser"]         = gDataUser->id();
    jData["date"]           = ui->DateDepdateEdit->date().toString("yyyy-MM-dd");
    jData["reffiscale"]     = ui->RefFiscalecomboBox->currentText();
    jData["objet"]          = ui->ObjetlineEdit->text();
    jData["montant"]        = QLocale().toDouble(ui->MontantlineEdit->text());
    jData["famfiscale"]     = FamFiscale;
    jData["monnaie"]        = QString();
    jData["idrecette"]      = QVariant().toInt();
    jData["modepaiement"]   = m;
    jData["compte"]         = (m!="E"? idCompte.toInt() : QVariant().toInt());
    jData["nocheque"]       = QVariant().toInt();

    Depense *dep = new Depense(jData);
    Datas::I()->depenses->addDepense(dep);

    gBigTable->insertRow(gBigTable->rowCount());
    SetDepenseToRow(dep, gBigTable->rowCount()-1);
    gBigTable->sortByColumn(7);

    gBigTable->setEnabled(true);
    SupprimerupPushButton->setVisible(true);
    ModifierupPushButton->setVisible(true);
    ui->NouvelleDepenseupPushButton->setEnabled(true);

    QString annee =  QString::number(dep->annee());
    if (ui->AnneecomboBox->currentText() != annee)
    {
        if (ui->AnneecomboBox->findText(annee))
            ReconstruitListeAnnees();
        ui->AnneecomboBox->setCurrentText(annee);
    }
    bool rubrique2035trouvee = false;
    for (int i=0; i<ui->Rubriques2035comboBox->count(); ++i)
    {
        if (ui->Rubriques2035comboBox->itemText(i) == ui->RefFiscalecomboBox->currentText())
        {
            rubrique2035trouvee = true;
            break;
        }
    }
    if (!rubrique2035trouvee)
        ReconstruitListeRubriques();
    CalculTotalDepenses();
    gMode = Lire;
    connect (gBigTable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
    for (int i=0; i< gBigTable->rowCount(); i++)
        if (getDepenseFromRow(i)->id() == dep->id()){
            gBigTable->setCurrentCell(i,1);
            gBigTable->scrollTo(gBigTable->model()->index(i,1), QAbstractItemView::PositionAtCenter);
            break;
        }
    RegleAffichageFiche(Lire);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Gerer une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::GererDepense(QPushButton *widgsender)
{
    if (widgsender == ModifierupPushButton)
    {
        if (gMode == Lire)
            RegleAffichageFiche(Modifier);
        else
            RegleAffichageFiche(Lire);
    }
    else
        RegleAffichageFiche(Enregistrer);
}

void dlg_depenses::MenuContextuel()
{
    QMenu *menu;
    menu = new QMenu(this);

    if (ui->Rubriques2035comboBox->currentIndex() == 0)
    {
        QAction *pAction_RecopieDep = menu->addAction(tr("Effectuer une copie de cette dépense à la date d'aujourd'hui"));
        connect (pAction_RecopieDep, &QAction::triggered,    this,   [=] {ChoixMenu("Copie");});
    }
    QAction *pAction_ChercheVal = menu->addAction(tr("Rechercher une valeur"));
    connect (pAction_ChercheVal, &QAction::triggered,    this,   [=] {ChoixMenu("ChercheVal");});

    // ouvrir le menu
    menu->deleteLater();
    menu->exec(cursor().pos());
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
        UpSmallButton OKBouton;
        OKBouton.setUpButtonStyle(UpSmallButton::STARTBUTTON);
        msgbox.addButton(&OKBouton, QMessageBox::AcceptRole);
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
        SupprimerupPushButton           ->setVisible(false);
        ModifierupPushButton            ->setVisible(false);
        EnregupPushButton               ->setVisible(true);
        AnnulupPushButton               ->setVisible(true);
        ui->NouvelleDepenseupPushButton ->setEnabled(false);
        gBigTable                       ->setEnabled(false);
        gBigTable                       ->disconnect();
        ui->DateDepdateEdit             ->setDate(QDate::currentDate());
        EnregupPushButton               ->setText("Enregistrer");
        ui->OKupPushButton              ->setShortcut(QKeySequence());
        ModifierupPushButton            ->setShortcut(QKeySequence());
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
    Depense *dep = getDepenseFromRow(gBigTable->currentRow());

    if (dep->isArchivee() == Depense::NoLoSo)
        db->loadDepenseArchivee(dep);
    if (dep->isArchivee() == Depense::Oui)
    {
        UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
        return;
    }
    UpMessageBox msgbox;
    msgbox.setText(tr("Supprimer une dépense!"));
    msgbox.setInformativeText(tr("Confirmer la suppression de\n") + dep->date().toString("dd MMM yyyy") + "\n" + dep->objet() + "\n" + QLocale().toString(dep->montant(),'f',2) + "?");
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton(tr("Supprimer"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton)
        return;

    //On supprime l'écriture
    db->SupprRecordFromTable(dep->id(), "idDep", NOM_TABLE_LIGNESCOMPTES);
    db->SupprRecordFromTable(dep->id(), "idDep", NOM_TABLE_DEPENSES);
    Datas::I()->depenses->getDepenses()->remove(dep->id());

    if (gBigTable->rowCount() == 1)
    {
        ReconstruitListeAnnees();
        QString year = QDate::currentDate().toString("yyyy");
        int idx = ui->AnneecomboBox->findText(year);
        ui->AnneecomboBox->disconnect();
        ui->AnneecomboBox->setCurrentIndex(idx==-1? 0 : idx);
        RedessineBigTable();
        connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=](int) {RedessineBigTable();});
    }
    else for (int i = 0; i< gBigTable->rowCount(); i++)
        if (getDepenseFromRow(i) == Q_NULLPTR)
        {
            gBigTable->removeRow(i);
            if (i < gBigTable->rowCount() - 1)
                gBigTable->setCurrentCell(i,1);
            else
                gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
            break;
        }
    CalculTotalDepenses();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Recalcule le total des dépenses -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::CalculTotalDepenses()
{
    double Total = 0;
    if (gBigTable->rowCount() > 0)
        for (int k = 0; k < gBigTable->rowCount(); k++)
            Total = Total + QLocale().toDouble(static_cast<UpLabel*>(gBigTable->cellWidget(k,3))->text());
    QString TotalRemise;
    TotalRemise = QLocale().toString(Total,'f',2);
    QString AnneeRubrique2035 = tr("Total général");
    if (ui->Rubriques2035comboBox->currentText() != "<Aucun>")
        AnneeRubrique2035 = tr("Total ") + ui->Rubriques2035comboBox->currentText();
    ui->TotallineEdit->setText(AnneeRubrique2035 + " " + ui->AnneecomboBox->currentText() + " -> " + TotalRemise);
    ui->ExportupPushButton->setEnabled(gBigTable->rowCount()>0);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Ouvre la gestion des comptes -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::GestionComptes()
{
    Dlg_Cmpt          = new dlg_comptes();
    if (Dlg_Cmpt->getInitOK())
        Dlg_Cmpt->exec();
}

void dlg_depenses::ZoomDoc()
{
    connect (proc, &Procedures::PrintImage, this, &dlg_depenses::ImprimeFacture);
    connect (proc, &Procedures::DeleteImage, this, &dlg_depenses::SupprimeFacture);
    QMap<QString,QVariant> doc = proc->CalcImage(m_depenseencours->id(), FACTURE, true, true);
    proc->EditImage(doc,
                    (m_depenseencours->isecheancier()? m_depenseencours->objetecheancier() : m_depenseencours->objet()),
                    UpDialog::ButtonSuppr | UpDialog::ButtonPrint | UpDialog::ButtonOK);
}

void dlg_depenses::SupprimeFacture()
{
    UpMessageBox::Question(this, tr("Suppression de facture"), tr("Supprimer la facture ") + m_depenseencours->objet() + "?");
}

void dlg_depenses::Print(QPrinter *Imprimante, QImage image)
{
    QPainter PrintingPreView(Imprimante);
    QPixmap pix         = QPixmap::fromImage(image).scaledToWidth(int(rect.width()),Qt::SmoothTransformation);
    PrintingPreView.drawImage(QPoint(0,0),pix.toImage());
}

bool dlg_depenses::ImprimeFacture()
{
    bool AvecPrevisu = true;
    QMap<QString,QVariant> doc = proc->CalcImage(m_depenseencours->id(), FACTURE, true, true);
    QByteArray bapdf = doc.value("ba").toByteArray();
    if (doc.value("type").toString() == PDF)     // le document est un pdf ou un document texte
    {
        Poppler::Document* document = Poppler::Document::loadFromData(bapdf);
        if (!document || document->isLocked()) {
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            delete document;
            return false;
        }
        if (document == Q_NULLPTR) {
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
            delete document;
            return false;
        }

        document->setRenderHint(Poppler::Document::TextAntialiasing);
        int numpages = document->numPages();
        for (int i=0; i<numpages ;i++)
        {
            Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
            if (pdfPage == Q_NULLPTR) {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                delete document;
                return false;
            }
            QImage image = pdfPage->renderToImage(600,600);
            if (image.isNull()) {
                UpMessageBox::Watch(this,tr("Impossible de retrouver les pages du document"));
                delete document;
                return false;
            }
            // ... use image ...
            if (i == 0)
            {
                if (AvecPrevisu)
                {
                    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
                    connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(printer, image);});
                    dialog->exec();
                    delete dialog;
                }
                else
                {
                    QPrintDialog *dialog = new QPrintDialog(printer, this);
                    if (dialog->exec() != QDialog::Rejected)
                        Print(printer, image);
                    delete dialog;
                }
            }
            else
                Print(printer, image);
            delete pdfPage;
        }
        delete document;
    }
    if (doc.value("type").toString() == JPG)     // le document est un jpg
    {
        QPixmap pix;
        pix.loadFromData(bapdf);
        QImage image= pix.toImage();
        if (AvecPrevisu)
        {
            QPrintPreviewDialog *dialog = new QPrintPreviewDialog(printer, this);
            connect(dialog, &QPrintPreviewDialog::paintRequested, this,   [=] {Print(printer, image);});
            dialog->exec();
            delete dialog;
        }
        else
        {
            QPrintDialog *dialog = new QPrintDialog(printer, this);
            if (dialog->exec() != QDialog::Rejected)
                Print(printer, image);
            delete dialog;
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Met à jour la fiche --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::MetAJourFiche()
{
    ui->frame->setVisible(gBigTable->rowCount() > 0 && !gBigTable->isRowHidden(gBigTable->currentRow()));
    if (gMode == Lire && gBigTable->rowCount() > 0 && !gBigTable->isRowHidden(gBigTable->currentRow()))
    {
        ui->DateDepdateEdit     ->disconnect();
        ui->RefFiscalecomboBox  ->disconnect();
        ui->PaiementcomboBox    ->disconnect();
        if (gBigTable->selectedRanges().size() == 0)
            gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);

        m_depenseencours = getDepenseFromRow(gBigTable->currentRow());

        ui->ObjetlineEdit->setText(m_depenseencours->objet());
        ui->DateDepdateEdit->setDate(m_depenseencours->date());
        ui->MontantlineEdit->setText(QLocale().toString(m_depenseencours->montant(),'f',2));
        QString A = m_depenseencours->modepaiement();                                                         // Mode de paiement - col = 4
        QString B = "";
        if (A == "E")           A = tr("Espèces");
        else
        {
            QMap<int, Compte*>::iterator compteFind = gDataUser->getComptes()->comptesAll().find(m_depenseencours->comptebancaire());
            if( compteFind == gDataUser->getComptes()->comptesAll().constEnd() )
            {
                //ATTENTION ERROR
            }
            B       = compteFind.value()->nom();
            if (A == "B")       A = tr("Carte de crédit");
            else if (A == "T")  A = tr("TIP");
            else if (A == "V")  A = tr("Virement");
            else if (A == "P")  A = tr("Prélèvement");
            else if (A == "C")  A = tr("Chèque");
        }
        ui->PaiementcomboBox    ->setCurrentText(A);
        ui->ComptesupComboBox   ->setCurrentIndex(ui->ComptesupComboBox->findText(B));
        ui->ComptesupComboBox   ->setVisible(B != "");
        ui->RefFiscalecomboBox  ->setCurrentText(m_depenseencours->reffiscale());
        bool hasfacture = m_depenseencours->lienfacture()!="";
        ui->FactureupPushButton     ->setVisible(!hasfacture);
        ui->EcheancierupPushButton  ->setVisible(!hasfacture);
        ui->VisuDocupTableWidget    ->setVisible(hasfacture);
        if (m_depenseencours->lienfacture() != "")
        {
            QMap<QString,QVariant> doc = proc->CalcImage(m_depenseencours->id(), FACTURE, true, true);
            glistImg =  ui->VisuDocupTableWidget->AfficheDoc(doc, true);
        }
        if (m_depenseencours->modepaiement() != "E")            // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte pour savoir si on peut la modifier
        {
            if (m_depenseencours->isArchivee() == Depense::NoLoSo)
                db->loadDepenseArchivee(m_depenseencours);
            //qDebug() << m_depenseencours->objet() + " - id " + QString::number(m_depenseencours->id()) + " - archivée = " + (m_depenseencours->isArchivee()==Depense::Oui? "Oui" : (m_depenseencours->isArchivee()==Depense::Non? "Non" : "NoloSo"));
            if (gBigTable->selectedRanges().size() > 0)
                SupprimerupPushButton->setEnabled(m_depenseencours->isArchivee() == Depense::Non);
        }
        else
            SupprimerupPushButton->setEnabled(true);
        connect (ui->DateDepdateEdit,       &QDateEdit::dateChanged,        this,   [=] {EnableModifiepushButton();});
        connect (ui->PaiementcomboBox,      &QComboBox::currentTextChanged, this,   [=] {EnableModifiepushButton(); ChoixPaiement();});
        connect (ui->RefFiscalecomboBox,    &QComboBox::currentTextChanged, this,   [=] {EnableModifiepushButton();});
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
    Depense *dep = getDepenseFromRow(gBigTable->currentRow());
    QString idDep = QString::number(dep->id());
    QDate datedepart = dep->date();

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
    else if (ui->ComptesupComboBox->currentIndex()==-1 && ui->PaiementcomboBox->currentText()!= tr("Espèces"))
        Erreur = tr("le compte bancaire");

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
        else if (Erreur == tr("le compte bancaire"))
        {
            ui->ComptesupComboBox->setFocus();
            ui->ComptesupComboBox->showPopup();
        }
        return;
    }

    // vérifier que cette dépense n'a pas été déjà saisie
    QList<Depense*> veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->getDepenses(), ui->DateDepdateEdit->date(),
                                                              ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), gDataUser->id(),
                                                              DataBase::Egal);
    if (veriflistdepenses.size() > 0){
        for (QList<Depense*>::const_iterator itDepense = veriflistdepenses.constBegin(); itDepense != veriflistdepenses.constEnd(); ++itDepense){
            if (veriflistdepenses.last()->id() == idDep){
                pb = tr("Elle a déjà été saisie");
                OnSauteLaQuestionSuivante = true;
                break;
    }}}

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
        veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->getDepenses(), ui->DateDepdateEdit->date().addDays(-1),
                                                  ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), gDataUser->id(),
                                                  DataBase::Inf);
        if (veriflistdepenses.size() > 0)
        {
            if (pb != "")
                pb += "\n";
            pb += tr("Une dépense semblable a été saisie\nle ") + veriflistdepenses.last()->date().toString("dd MMM yyyy");
        }
    }

    if (pb != "")
    {
        UpMessageBox msgbox;
        msgbox.setText(tr("Il y a un problème avec cette dépense!"));
        msgbox.setInformativeText(pb + "\n" + tr("Confirmer la saisie?"));
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton(pb == tr("Elle a déjà été saisie")? "OK" : tr("Oui"));
        UpSmallButton NoBouton(tr("Non"));
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton || pb == tr("Elle a déjà été saisie"))
            return;
    }
    // Correction de l'écriture dans la table depenses
    QString Paiement, m;
    Paiement = ui->PaiementcomboBox->currentText();
    if (Paiement == tr("Espèces"))              m = "E";
    else if (Paiement == tr("Virement"))        m = "V";
    else if (Paiement == tr("Carte de crédit")) m = "B";
    else if (Paiement == tr("Chèque"))          m = "C";
    else if (Paiement == tr("Prélèvement"))     m = "P";
    else if (Paiement == tr("TIP"))             m = "T";
    bool ok = true;
    QList<QList<QVariant>> listfamfiscale = db->SelectRecordsFromTable(QStringList() << "Famfiscale",
                                                                       NOM_TABLE_RUBRIQUES2035, ok,
                                                                       "where reffiscale = '" + Utils::correctquoteSQL(ui->RefFiscalecomboBox->currentText()) + "'");
    QString FamFiscale = listfamfiscale.at(0).at(0).toString();
    QString idCompte = ui->ComptesupComboBox->currentData().toString();
    if (listfamfiscale.size() > 0)                // l'écriture existe et on la modifie
    {
        QHash<QString, QString> listsets;
        listsets.insert("DateDep",      ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
        listsets.insert("Objet",        ui->ObjetlineEdit->text());
        listsets.insert("Montant",      QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
        listsets.insert("RefFiscale",   ui->RefFiscalecomboBox->currentText());
        listsets.insert("FamFiscale",   FamFiscale);
        listsets.insert("ModePaiement", m);
        listsets.insert("Compte",       (m!="E"? idCompte : "null"));
        DataBase:: getInstance()->UpdateTable(NOM_TABLE_DEPENSES,
                                              listsets,
                                              "where idDep = " + idDep);

        QJsonObject jData{};
        jData["iddepense"]      = idDep.toInt();
        jData["iduser"]         = dep->iduser();
        jData["date"]           = ui->DateDepdateEdit->date().toString("yyyy-MM-dd");
        jData["reffiscale"]     = ui->RefFiscalecomboBox->currentText();
        jData["objet"]          = ui->ObjetlineEdit->text();
        jData["montant"]        = QLocale().toDouble(ui->MontantlineEdit->text());
        jData["famfiscale"]     = FamFiscale;
        jData["monnaie"]        = dep->monnaie();
        jData["idrecette"]      = dep->idrecette();
        jData["modepaiement"]   = m;
        jData["compte"]         = (m!="E"? idCompte.toInt() : QVariant().toInt());
        jData["nocheque"]       = dep->nocheque();
        dep->setData(jData);
    }

    // Correction de l'écriture dans la table lignescomptes
    if (Paiement == tr("Espèces"))
        db->SupprRecordFromTable(dep->id(), "idDep", NOM_TABLE_LIGNESCOMPTES);
    else
    {
        Paiement = ui->PaiementcomboBox->currentText();
        if (Paiement == tr("Virement")) Paiement = tr("Virement débiteur");

        // on recherche si l'écriture existe dans lignescomptes et si c'est le cas, on la modifie
        QList<QList<QVariant>> listlignescomptes = db->SelectRecordsFromTable(QStringList() << "idLigne",
                                                                              NOM_TABLE_LIGNESCOMPTES, ok,
                                                                              "where idDep = " + idDep);
        if (listlignescomptes.size() > 0)                // l'écriture existe et on la modifie
        {
           QHash<QString, QString> listsets;
           listsets.insert("LigneDate",             ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
           listsets.insert("LigneLibelle",          ui->ObjetlineEdit->text());
           listsets.insert("LigneMontant",          QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
           listsets.insert("LigneDebitCredit",      "0");
           listsets.insert("LigneTypeOperation",    Paiement);
           listsets.insert("idCompte",              (m!="E"? idCompte : "null"));
           DataBase:: getInstance()->UpdateTable(NOM_TABLE_LIGNESCOMPTES,
                                                 listsets,
                                                 "where idDep = " + idDep);
        }
        else           // on n'a pas trouvé la ligne, on la recherche dans les archives
        {
            QList<QList<QVariant>> listlignesarchives = db->SelectRecordsFromTable(QStringList() << "idLigne",
                                                                                   NOM_TABLE_ARCHIVESBANQUE, ok,
                                                                                   "where idDep = " + idDep);
            if (listlignesarchives.size() > 0)                // l'écriture existe et on la modifie
            {
                QHash<QString, QString> listsets;
                listsets.insert("LigneDate",            ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
                listsets.insert("LigneLibelle",         ui->ObjetlineEdit->text());
                listsets.insert("LigneDebitCredit",     "0");
                listsets.insert("LigneTypeOperation",   Paiement);
                DataBase:: getInstance()->UpdateTable(NOM_TABLE_ARCHIVESBANQUE,
                                                      listsets,
                                                      " where idDep = " + idDep);
            }
            else        // l'écriture n'existait ni dans lignescomptes ni dans archives
                        // => c'était une dépense en espèces
                        // on l'enregistre dans lignescomptes
            {
                QHash<QString, QString> listsets;
                listsets.insert("idCompte",             idCompte);
                listsets.insert("idDep",                idDep);
                listsets.insert("LigneDate",            ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
                listsets.insert("Lignelibelle",         ui->ObjetlineEdit->text());
                listsets.insert("LigneMontant",         QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
                listsets.insert("LigneDebitCredit",     "0");
                listsets.insert("LigneTypeoperation",   Paiement);
                DataBase:: getInstance()->InsertIntoTable(NOM_TABLE_LIGNESCOMPTES, listsets);
            }
        }
    }
    gBigTable->setEnabled(true);
    int row = -1;
    bool rubrique2035trouvee = false;
    for (int i=0; i<ui->Rubriques2035comboBox->count(); ++i)
    {
        if (ui->Rubriques2035comboBox->itemText(i) == ui->RefFiscalecomboBox->currentText())
        {
            rubrique2035trouvee = true;
            break;
        }
    }
    if (!rubrique2035trouvee)
        ReconstruitListeRubriques();
    int year = dep->annee();
    if (ui->AnneecomboBox->currentText() != QString::number(year))
    {
        if (ui->AnneecomboBox->findText(QString::number(year)) < 0)
            ReconstruitListeAnnees();
        ui->AnneecomboBox->setCurrentText(QString::number(year));
        for (int i=0; i< gBigTable->rowCount(); i++)
            if (getDepenseFromRow(i)->id() == dep->id()){
                row = i;
                break;
            }
    }
    else
    {
        // Mettre à jour l'affichage dans la table
        for (int i=0; i< gBigTable->rowCount(); i++)
            if (getDepenseFromRow(i)->id() == dep->id()){
                row = i;
                break;
            }
        QString A;
        static_cast<UpLabel*>(gBigTable->cellWidget(row,1))->setText(dep->date().toString(tr("d MMM yyyy") + " "));             // Date - col = 1
        static_cast<UpLabel*>(gBigTable->cellWidget(row,2))->setText(" " + dep->objet());                                       // Objet - col = 2

        if (dep->monnaie() == "F")
            A = QLocale().toString(dep->montant()/6.55957,'f',2);                                                               // Montant en F converti en euros
        else
            A = QLocale().toString(dep->montant(),'f',2);                                                                       // Montant - col = 3
        static_cast<UpLabel*>(gBigTable->cellWidget(row,3))->setText(A + " ");

        A = dep->modepaiement();                                                                                                // Mode de paiement - col = 4
        QString B = "";
        QString C = "";
        if (A == "E")  A = tr("Espèces");
        else
        {
            QMultiMap<int, Compte*>::const_iterator cptFind = gDataUser->getComptes()->comptesAll().find(dep->comptebancaire());
            if( cptFind != gDataUser->getComptes()->comptesAll().constEnd() )
                B = cptFind.value()->nom();
            if (A == "B")       A = tr("Carte de crédit");
            else if (A == "T")  A = tr("TIP");
            else if (A == "V")  A = tr("Virement");
            else if (A == "P")  A = tr("Prélèvement");
            else if (A == "C") { A = tr("Chèque");  if (dep->nocheque() > 0) C += " " + QString::number(dep->nocheque());}
        }
        A += " " + B + " " + C;
        static_cast<UpLabel*>(gBigTable->cellWidget(row,4))->setText(" " + A);
        static_cast<UpLabel*>(gBigTable->cellWidget(row,5))->setText(" " + dep->reffiscale());                                  // Ref fiscale - col = 5
        static_cast<UpLabel*>(gBigTable->cellWidget(row,6))->setText(" " + dep->famillefiscale());                              // Famille fiscale - col = 6
        A = dep->date().toString("yyyy-MM-dd");
        gBigTable->item(row,7)->setText(dep->date().toString("yyyy-MM-dd"));                                                    // ClassementparDate - col = 7
    }

    CalculTotalDepenses();
    SupprimerupPushButton->setEnabled(gBigTable->rowCount()>0);
    ModifierupPushButton->setEnabled(gBigTable->rowCount()>0);
    FiltreTable();
    gBigTable->setCurrentCell(row,1);
    if (dep->date() != datedepart)
    {
        gBigTable->sortByColumn(7);
        for (int i=0; i< gBigTable->rowCount(); i++)
            if (getDepenseFromRow(i)->id() == dep->id()){
                gBigTable->scrollTo(gBigTable->model()->index(i,1), QAbstractItemView::PositionAtCenter);
                break;
            }
    }
    gMode = Lire;
    RegleAffichageFiche(Lire);
    MetAJourFiche();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::RedessineBigTable()
{
    RemplitBigTable();
    gBigTable->disconnect();
    if (gBigTable->rowCount() > 0)
    {
        gBigTable->setCurrentCell(gBigTable->rowCount()-1,1);
        RegleAffichageFiche(Lire);
        MetAJourFiche();
        FiltreTable();
    }
    else
        RegleAffichageFiche(TableVide);
    connect (gBigTable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
    SupprimerupPushButton->setEnabled(gBigTable->rowCount()>0);
    ModifierupPushButton->setEnabled(gBigTable->rowCount()>0);
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
    int                 ColCount = 8;
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
    gBigTable->setColumnWidth(li,0);                                                // idDepense
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

    gBigTable->setColumnHidden(ColCount-1,true);

    gBigTable->setGridStyle(Qt::SolidLine);
    gBigTable->FixLargeurTotale();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Filtre la table en fonction des rubriques fiscales --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::FiltreTable()
{
    int rowdep = gBigTable->currentRow();
    int row = rowdep;

    QString filtre = ui->Rubriques2035comboBox->currentText();
    int idrubrique = ui->Rubriques2035comboBox->currentData().toInt();
    int idx = ui->Rubriques2035comboBox->findData(idrubrique);
    for (int i=0; i<gBigTable->rowCount(); i++)
    {
        if (ui->Rubriques2035comboBox->currentIndex()==0)
            gBigTable->setRowHidden(i,false);
        else
        {
            QString rub = static_cast<UpLabel*>(gBigTable->cellWidget(i,5))->text().mid(1);
            gBigTable->setRowHidden(i, rub!=filtre);
        }
    }
    if (idx>0 && gBigTable->isRowHidden(row))
    {
        do row--;
        while (gBigTable->isRowHidden(row) && row>0);       // on cherche l'enregistrement précédent qui n'est pas caché
        if (gBigTable->isRowHidden(row) && row == 0)        // si on est remonté au début de la table et qu'on n'a pas trouvé d'enregistrement non caché, on recherche vers la fin
        {
            row = rowdep;
            do row++;
            while (gBigTable->isRowHidden(row) && row < (gBigTable->rowCount()-1));
        }
    }
    if (!gBigTable->isRowHidden(row))
    {
        gBigTable->selectRow(row);
        gBigTable->setCurrentCell(row,1);
        Utils::Pause(1);            // si on ne fait pas ça, le scroll marche mal
        gBigTable->scrollToItem(gBigTable->item(row,0), QAbstractItemView::PositionAtCenter);
        if (row != rowdep)
        {
            gBigTable->disconnect();
            MetAJourFiche();
            connect (gBigTable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
        }
    }
    CalculTotalDepenses();
 }

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Annees dans le combobox Annees --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::ReconstruitListeAnnees()
{
    ui->AnneecomboBox->disconnect();
    QStringList ListeAnnees;

    for( QHash<int, Depense*>::const_iterator itDepense = Datas::I()->depenses->getDepenses()->constBegin(); itDepense != Datas::I()->depenses->getDepenses()->constEnd(); ++itDepense )
    {
        Depense *dep = const_cast<Depense*>(itDepense.value());
        if (!ListeAnnees.contains(QString::number(dep->annee())))
            ListeAnnees << QString::number(dep->annee());
    }
    ListeAnnees.sort();
    ui->AnneecomboBox->disconnect();
    ui->AnneecomboBox->clear();
    if (ListeAnnees.size()==0)
        ListeAnnees << QDate::currentDate().toString("yyyy");
    ui->AnneecomboBox->insertItems(0,ListeAnnees);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=](int) {RedessineBigTable();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des rubriques 2035  de l'utilisateur dans le combobox Rubriques --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::ReconstruitListeRubriques(int idx)
{
    ui->Rubriques2035comboBox->clear();
    bool ok = true;
    QString req = "select distinct dep.reffiscale, idRubrique from " NOM_TABLE_DEPENSES " dep"
                  " left join " NOM_TABLE_RUBRIQUES2035 " rub"
                  " on dep.RefFiscale = rub.Reffiscale"
                  " where idUser = " + QString::number(gDataUser->id()) +
                  " ORDER BY reffiscale";
    QList<QList<QVariant>> ListeRubriques = db->StandardSelectSQL(req, ok);
    ListeRubriques.insert(0, (QList<QVariant>() << tr("<Aucun>") << -1));
    for (int i = 0; i < ListeRubriques.size(); i++)
        ui->Rubriques2035comboBox->insertItem(i,ListeRubriques.at(i).at(0).toString(), ListeRubriques.at(i).at(1));
    ui->Rubriques2035comboBox->setCurrentIndex(idx);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::RemplitBigTable()
{
    gBigTable->disconnect();
    gBigTable->clearContents();
    gBigTable->setRowCount(0);
    QList<Depense*> listDepenses;

    for( QHash<int, Depense*>::const_iterator itDepense = Datas::I()->depenses->getDepenses()->constBegin(); itDepense != Datas::I()->depenses->getDepenses()->constEnd(); ++itDepense )
    {
        Depense *dep = const_cast<Depense*>(itDepense.value());
        if (dep->annee() == ui->AnneecomboBox->currentText().toInt())
            listDepenses << dep;
    }

    int i=0;
    if (listDepenses.size() == 0)
    {
        RegleAffichageFiche(TableVide);
        return;
    }
    gBigTable->setRowCount(listDepenses.size());
    for (QList<Depense*>::const_iterator itdep = listDepenses.constBegin() ; itdep != listDepenses.constEnd() ; ++itdep)
    {
        Depense *dep = const_cast<Depense*>(*itdep);
        SetDepenseToRow(dep, i);
        ++i;
    }
    gBigTable->sortItems(7);
    SupprimerupPushButton->setEnabled(false);
    connect (gBigTable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
}

Depense* dlg_depenses::getDepenseFromRow(int row)
{
    return Datas::I()->depenses->getDepenseById(gBigTable->item(row,0)->text().toInt());
}

void dlg_depenses::ScanDoc(QString typedoc)
{
    if (m_depenseencours == Q_NULLPTR)
        return;
    if (typedoc == FACTURE)
        Dlg_DocsScan = new dlg_docsscanner(m_depenseencours->id(), dlg_docsscanner::Facture, m_depenseencours->objet(), this);
    else if (typedoc == ECHEANCIER)
        Dlg_DocsScan = new dlg_docsscanner(m_depenseencours->id(), dlg_docsscanner::Echeancier, "", this);
    if (Dlg_DocsScan->exec() > 0)
    {
        QMap<QString, QVariant> map = Dlg_DocsScan->getdataFacture();
        int idfact = map["idfacture"].toInt();
        if (idfact>-1)
        {
            QString req = "update " NOM_TABLE_DEPENSES " set idFacture = " + QString::number(idfact) + " where idDep = " + QString::number(m_depenseencours->id());
            QSqlQuery (req, db->getDataBase());

            m_depenseencours->setidfacture(idfact);
            m_depenseencours->setlienfacture(Dlg_DocsScan->getdataFacture()["lien"].toString());
            m_depenseencours->setecheancier(Dlg_DocsScan->getdataFacture()["echeancier"].toBool());
            ui->FactureupPushButton     ->setVisible(false);
            ui->EcheancierupPushButton  ->setVisible(false);
            ui->VisuDocupTableWidget    ->setVisible(true);
            QMap<QString,QVariant> doc = proc->CalcImage(m_depenseencours->id(), FACTURE, true, true);
            glistImg = ui->VisuDocupTableWidget->AfficheDoc(doc, true);
        }
    }
    delete  Dlg_DocsScan;
}

void dlg_depenses::SetDepenseToRow(Depense *dep, int row)
{
    //+++ ne pas utiliser insertRow() qui est très lent au fur et à mesure qu'on vide et remplit la table
    QTableWidgetItem    *pItem7;
    QString             A;
    QTableWidgetItem *pitem0;
    UpLabel *label1, *label2, *label3, *label4, *label5, *label6;
    int col = 0;
    int id = dep->id();

    pitem0 = new QTableWidgetItem;
    label1 = new UpLabel;
    label2 = new UpLabel;
    label3 = new UpLabel;
    label4 = new UpLabel;
    label5 = new UpLabel;
    label6 = new UpLabel;

    label1->setId(id);
    label2->setId(id);
    label3->setId(id);
    label4->setId(id);
    label5->setId(id);
    label6->setId(id);

    label1->setContextMenuPolicy(Qt::CustomContextMenu);
    label2->setContextMenuPolicy(Qt::CustomContextMenu);
    label3->setContextMenuPolicy(Qt::CustomContextMenu);
    label4->setContextMenuPolicy(Qt::CustomContextMenu);
    label5->setContextMenuPolicy(Qt::CustomContextMenu);
    label6->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(label1,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label2,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label3,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label4,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label5,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label6,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});

    A = QString::number(id);                                                                    // idDepense - col = 0
    pitem0->setText(A);
    gBigTable->setItem(row,col,pitem0);
    col++;

    A = dep->date().toString(tr("d MMM yyyy"));                                                 // Date - col = 1
    label1->setText(A + " ");
    label1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    gBigTable->setCellWidget(row,col,label1);
    col++;

    label2->setText(" " + dep->objet());                                                        // Objet - col = 2
    gBigTable->setCellWidget(row,col,label2);
    col++;

    if (dep->monnaie() == "F")
        A = QLocale().toString(dep->montant()/6.55957,'f',2);// Montant en F converti en euros
    else
        A = QLocale().toString(dep->montant(),'f',2);                                           // Montant - col = 3
    label3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    label3->setText(A + " ");
    gBigTable->setCellWidget(row,col,label3);
    col++;

    A = dep->modepaiement();                                                                    // Mode de paiement - col = 4
    QString B = "";
    QString C = "";
    if (A == "E")           A = tr("Espèces");
    else
    {
        QMultiMap<int, Compte*>::const_iterator cptFind = gDataUser->getComptes()->comptesAll().find(dep->comptebancaire());
        if( cptFind != gDataUser->getComptes()->comptesAll().constEnd() )
            B = cptFind.value()->nom();
        if (A == "B")       A = tr("Carte de crédit");
        else if (A == "T")  A = tr("TIP");
        else if (A == "V")  A = tr("Virement");
        else if (A == "P")  A = tr("Prélèvement");
        else if (A == "C") {A = tr("Chèque");   if (dep->nocheque() > 0) C += " " + QString::number(dep->nocheque());}
    }
    A += " " + B + " " + C;
    label4->setText(" " + A);
    gBigTable->setCellWidget(row,col,label4);
    col++;

    A = dep->reffiscale();                                                                      // Rubrique2035 - col = 5
    label5->setText(" " + A);
    gBigTable->setCellWidget(row,col,label5);
    col++;

    A = dep->famillefiscale();                                                                  // Famille fiscale - col = 6
    label6->setText(" " + A);
    gBigTable->setCellWidget(row,col,label6);
    col++;

    A = dep->date().toString("yyyy-MM-dd");                                                     // ClassementparDate - col = 7
    pItem7 = new QTableWidgetItem() ;
    pItem7->setText(A);
    gBigTable->setItem(row,col,pItem7);

    gBigTable->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));
}


