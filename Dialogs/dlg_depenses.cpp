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

#include "dlg_depenses.h"


dlg_depenses::dlg_depenses(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_depenses)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowIcon(Icons::icCreditCard());

    ui->UserscomboBox->setEnabled(Datas::I()->users->userconnected()->isSecretaire() );

    int index = 0;
    bool foundUser = false;
    int currentIdUser = Datas::I()->users->userconnected()->id(); //Utilisateur connecte
    foreach (User * user, *map_usersliberaux)
    {
        ui->UserscomboBox->addItem(user->login(), QString::number(user->id()) );
        if( !foundUser )
        {
            if(currentIdUser != user->id())
                ++index;
            else
                foundUser = true;
        }
    }
    if(index >= map_usersliberaux->size())
        ui->UserscomboBox->setCurrentIndex(0);
    else
        ui->UserscomboBox->setCurrentIndex(index);

    restoreGeometry(proc->settings()->value(Position_Fiche "Depenses").toByteArray());

    m_initok = initializeUserSelected();
    if( !m_initok )
        return;

    setMaximumHeight(800);

    wdg_bigtable       = new UpTableWidget(this);
    ui->horizontalLayout_3->addWidget(wdg_bigtable);

    wdg_modifieruppushbutton = new UpPushButton(ui->frame);
    wdg_modifieruppushbutton->setFixedHeight(46);
    wdg_modifieruppushbutton->setText(tr("Modifier"));
    wdg_modifieruppushbutton->setIcon(Icons::icMarteau());
    wdg_modifieruppushbutton->setIconSize(QSize(25,25));

    wdg_supprimeruppushbutton = new UpPushButton(ui->frame);
    wdg_supprimeruppushbutton->setFixedHeight(46);
    wdg_supprimeruppushbutton->setText(tr("Supprimer"));
    wdg_supprimeruppushbutton->setIcon(Icons::icPoubelle());
    wdg_supprimeruppushbutton->setIconSize(QSize(25,25));

    wdg_enreguppushbutton = new UpPushButton(ui->frame);
    wdg_enreguppushbutton->setFixedHeight(46);
    wdg_enreguppushbutton->setText(tr("Valider"));
    wdg_enreguppushbutton->setIcon(Icons::icOK());
    wdg_enreguppushbutton->setIconSize(QSize(25,25));

    wdg_annuluppushbutton = new UpPushButton(ui->frame);
    wdg_annuluppushbutton->setFixedHeight(46);
    wdg_annuluppushbutton->setText(tr("Annuler"));
    wdg_annuluppushbutton->setIcon(Icons::icAnnuler());
    wdg_annuluppushbutton->setIconSize(QSize(25,25));

    ui->PrintupSmallButton  ->setText("");
    ui->PrintupSmallButton  ->setUpButtonStyle(UpSmallButton::PRINTBUTTON);
    ui->PrintupSmallButton  ->setShortcut(QKeySequence("Meta+P"));

    wdg_boxbuttlayout = new QHBoxLayout();
    wdg_boxbuttlayout->addWidget(wdg_annuluppushbutton);
    wdg_boxbuttlayout->addSpacerItem(new QSpacerItem(0,0));
    wdg_boxbuttlayout->addWidget(wdg_enreguppushbutton);
    wdg_boxbuttlayout->addSpacerItem(new QSpacerItem(0,0));
    wdg_boxbuttlayout->addWidget(wdg_supprimeruppushbutton);
    wdg_boxbuttlayout->addSpacerItem(new QSpacerItem(0,0));
    wdg_boxbuttlayout->addWidget(wdg_modifieruppushbutton);
    wdg_boxbuttlayout->setSpacing(2);
    wdg_boxbuttlayout->setContentsMargins(0,5,0,0);
    ui->frame->layout()->addItem(wdg_boxbuttlayout);

    ui->frame->setStyleSheet("QFrame#frame{border: 1px solid gray; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 210, 210, 50));}");
    ui->VisuDocupTableWidget->setCursor(QCursor(Icons::pxZoomIn().scaled(30,30))); //WARNING : icon scaled : pxZoomIn 30,30

    QStringList ListeRubriques = db->ListeRubriquesFiscales();
    ui->RefFiscalecomboBox->insertItems(0,ListeRubriques);
    ui->RefFiscalecomboBox->setCurrentText(ListeRubriques.at(0));

    m_listemoyensdepaiement << tr(CARTECREDIT);
    m_listemoyensdepaiement << tr(CHEQUE);
    m_listemoyensdepaiement << tr(ESPECES);
    m_listemoyensdepaiement << tr(PRELEVEMENT);
    m_listemoyensdepaiement << tr(VIREMENT);
    m_listemoyensdepaiement << tr(TIP);
    ui->PaiementcomboBox->insertItems(0,m_listemoyensdepaiement );
    ui->PaiementcomboBox->setCurrentText(m_listemoyensdepaiement.at(0));

    QDoubleValidator *val= new QDoubleValidator(this);
    val->setDecimals(2);
    ui->MontantlineEdit->setValidator(val);
    ui->ObjetlineEdit->setValidator(new QRegExpValidator(Utils::rgx_intitulecompta));

    QList<UpPushButton *> allUpButtons = this->findChildren<UpPushButton *>();
    for (int n = 0; n <  allUpButtons.size(); n++)
        allUpButtons.at(n)->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Mid);
    ui->GestionComptesupPushButton->resize(290,48);

    DefinitArchitectureBigTable();

    ReconstruitListeRubriques();
    ReconstruitListeAnnees();
    QString year = QDate::currentDate().toString("yyyy");
    int idx = ui->AnneecomboBox->findText(year);
    ui->AnneecomboBox->disconnect();
    ui->AnneecomboBox->setCurrentIndex(idx==-1? ui->AnneecomboBox->count()-1 : idx);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   &dlg_depenses::RedessineBigTable);
    RedessineBigTable();

    connect (ui->GestionComptesupPushButton,    &QPushButton::clicked,          this,   &dlg_depenses::GestionComptes);
    connect (ui->NouvelleDepenseupPushButton,   &QPushButton::clicked,          this,   [=] {GererDepense(ui->NouvelleDepenseupPushButton);});
    connect (wdg_modifieruppushbutton,          &QPushButton::clicked,          this,   [=] {GererDepense(wdg_modifieruppushbutton);});
    connect (ui->OKupPushButton,                &QPushButton::clicked,          this,   &dlg_depenses::accept);
    connect (ui->Rubriques2035comboBox,         QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                this,   &dlg_depenses::FiltreTable);
    connect (ui->DetailsDepensesupPushButton,   &QPushButton::clicked,          this,   &dlg_depenses::AfficheDetailsDepenses);
    connect (ui->FactureupPushButton,           &QPushButton::clicked,          this,   [=] {EnregistreFacture(FACTURE);});
    connect (ui->EcheancierupPushButton,        &QPushButton::clicked,          this,   [=] {EnregistreFacture(ECHEANCIER);});
    connect (ui->ExportupPushButton,            &QPushButton::clicked,          this,   &dlg_depenses::ExportTable);
    connect (ui->ChercheMontantupPushButton,    &QPushButton::clicked,          this,   &dlg_depenses::RechercheValeur);
    connect (ui->PrintupSmallButton,            &QPushButton::clicked,          this,   &dlg_depenses::PrintTable);
    connect (ui->MontantlineEdit,               &QLineEdit::editingFinished,    this,   &dlg_depenses::ConvertitDoubleMontant);
    connect (ui->PaiementcomboBox,              QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                this,   &dlg_depenses::ChoixPaiement);
    connect (ui->ObjetlineEdit,                 &QLineEdit::textEdited,         this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->MontantlineEdit,               &QLineEdit::textEdited,         this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->DateDepdateEdit,               &QDateEdit::dateChanged,        this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->PaiementcomboBox,              &QComboBox::currentTextChanged, this,   &dlg_depenses::EnableModifiepushButton);
    connect (ui->RefFiscalecomboBox,            &QComboBox::currentTextChanged, this,   &dlg_depenses::EnableModifiepushButton);
    connect (wdg_supprimeruppushbutton,         &QPushButton::clicked,          this,   &dlg_depenses::SupprimerDepense);
    connect (ui->UserscomboBox,                 QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                this,   [=](int) {ChangeUser(ui->UserscomboBox->currentIndex());});
    connect (ui->VisuDocupTableWidget,          &UpTableWidget::zoom,           this,   &dlg_depenses::ZoomDoc);
    connect (ui->VisuFacturecheckBox,           &QCheckBox::clicked,            this,   [=] {AfficheFacture(m_depenseencours);});
    connect (wdg_enreguppushbutton,             &QPushButton::clicked,          this,   &dlg_depenses::ModifierDepense);
    connect (wdg_annuluppushbutton,             &QPushButton::clicked,          this,   &dlg_depenses::AnnulEnreg);

    wdg_bigtable            ->setFocus();
    ui->ExportupPushButton  ->setEnabled(wdg_bigtable->rowCount()>0);
    ui->PrintupSmallButton  ->setEnabled(wdg_bigtable->rowCount()>0);
    setFixedWidth(wdg_bigtable->width() + ui->VisuDocupTableWidget->width() + layout()->contentsMargins().left() + layout()->contentsMargins().right() +layout()->spacing());

    //ui->Facturewidget->setVisible(false);
    //ui->VisuDocupTableWidget->setVisible(false);

    m_initok = true;
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
                               tr("Voulez-vous exporter la table des dépenses?\n(Jeu de caractères Unicode (UTF8), Format CSV, langue anglais(USA), champs séparés par tabulation)"))
            != UpSmallButton::STARTBUTTON)
        return;
    ExportEtat.append(tr("Date") + sep + tr("Dépense") + sep + tr("id Rubrique 2035") + sep + tr("Montant") + sep + tr("Mode de paiement") + sep  + tr("Rubrique 2035") + sep + tr("Famille fiscale") + "\n");
    for (int i=0;i< wdg_bigtable->rowCount(); i++)
    {
        if (!wdg_bigtable->isRowHidden(i))
        {
            Depense *dep = getDepenseFromRow(i);
            if (dep->rubriquefiscale() != "Amortissements")
            {
                ExportEtat.append(dep->date().toString("dd/MM/yyyy") + sep);                                                // Date - col = 7
                ExportEtat.append(dep->objet() + sep);                                                                      // Libelle - col = 2
                ExportEtat.append(QString::number(dep->idrubriquefiscale()) + sep);                                         // id Rubrique 2035
                ExportEtat.append(QString::number(dep->montant()) + sep);                                                   // Montant - col = 3
                ExportEtat.append(Utils::ConvertitModePaiement(dep->modepaiement()) + sep);                                 // Mode de paiement - col = 4
                ExportEtat.append(dep->rubriquefiscale() + sep);                                                            // Rubrique 2035 - col = 5
                ExportEtat.append(dep->famillefiscale());                                                                   // Famille fiscale - col = 6
                ExportEtat.append("\n");
            }
        }
    }
    for (int i=0;i< wdg_bigtable->rowCount(); i++)
    {
        if (!wdg_bigtable->isRowHidden(i))
        {
            Depense *dep = getDepenseFromRow(i);
            if (dep->rubriquefiscale() == "Amortissements")
            {
                ExportEtat.append(dep->date().toString("dd/MM/yyyy") + sep);                                                // Date - col = 7
                ExportEtat.append(dep->objet() + sep);                                                                      // Libelle - col = 2
                ExportEtat.append(QString::number(dep->idrubriquefiscale()) + sep);                                         // id Rubrique 2035
                ExportEtat.append(QString::number(dep->montant()) + sep);                                                   // Montant - col = 3
                ExportEtat.append(Utils::ConvertitModePaiement(dep->modepaiement()) + sep);                                 // Mode de paiement - col = 4
                ExportEtat.append(dep->rubriquefiscale() + sep);                                                            // Rubrique 2035 - col = 5
                ExportEtat.append(dep->famillefiscale());                                                                   // Famille fiscale - col = 6
                ExportEtat.append("\n");
            }
        }
    }
    QString ExportFileName = PATH_DIR_RUFUS + "/"
                            + tr("Depenses") + " " + ui->UserscomboBox->currentText() + " "
                            + tr("Année") + " " + ui->AnneecomboBox->currentText()
                            + ".csv";
    QFile   ExportFile(ExportFileName);
    bool exportOK = true;
    QString msg = tr("Nom du fichier d'export") + "\n" + ExportFileName + "\nLes amortissements ont été placés en fin de fichier";
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

void dlg_depenses::PrintTable()
{
    QString            Entete, Pied;
    bool AvecDupli   = false;
    bool AvecPrevisu = true;
    bool AvecNumPage = false;

    User *userEntete = Q_NULLPTR;

    //création de l'entête
    userEntete = Datas::I()->users->getById(ui->UserscomboBox->currentData().toInt());

    if(userEntete == Q_NULLPTR)
    {
        UpMessageBox::Watch(this, tr("Impossible de retrouver les données de l'en-tête") , tr("Annulation de l'impression"));
        return;
    }
    Entete = proc->CalcEnteteImpression(QDate::currentDate(), userEntete).value("Norm");
    if (Entete == "") return;

    // NOTE : POURQUOI mettre ici "PRENOM PATIENT" alors que ce sont les données d'un User qui sont utilisées ???
    // REP : parce qu'on utilise le même entête que pour les ordonnances et qu'on va substituer les champs patient dans cet entête.
    // on pourrait faire un truc plus élégant (un entête spécifique pour cet état p.e.) mais je n'ai pas eu le temps de tout faire.
        Entete.replace("{{PRENOM PATIENT}}"    , ui->TotallineEdit->text());
    Entete.replace("{{NOM PATIENT}}"       , "");
    Entete.replace("{{TITRE1}}"            , windowTitle());
    Entete.replace("{{TITRE}}"             , "");
    Entete.replace("{{DDN}}"               , "");

    // création du pied
    Pied = proc->CalcPiedImpression(userEntete);
    if (Pied == "") return;

    // creation du corps
    double c = CORRECTION_td_width;
    QTextEdit *Etat_textEdit = new QTextEdit;
    QString test4 = "<html><head><style type=\"text/css\">p.p1 {font:70px; margin: 0px 0px 10px 100px;}"
                    "</style></head>"
                    "<body LANG=\"fr-FR\" DIR=\"LTR\">"
                    "<table width=\"" + QString::number(int(c*510)) + "\" border=\"1\"  cellspacing=\"0\" cellpadding=\"2\">";
    for (int i=0;i< wdg_bigtable->rowCount(); i++)
    {
        if (!wdg_bigtable->isRowHidden(i))
        {
            Depense *dep = getDepenseFromRow(i);
            if (dep->rubriquefiscale() != "Amortissements")
            {
                test4 += "<tr>"
                         "<td width=\"" + QString::number(int(c*45))  + "\"><span style=\"font-size:6pt\"><div align=\"right\">" + dep->date().toString(tr("d MMM yy")) + "</div></span></font></td>"   //! date
                         "<td width=\"" + QString::number(int(c*150)) + "\"><span style=\"font-size:6pt\">" + dep->objet() + "</span></td>"                                                             //! Libelle
                         "<td width=\"" + QString::number(int(c*150)) + "\"><span style=\"font-size:6pt\">" + dep->rubriquefiscale() + "</span></td>"                                                   //! rubrique fiscale
                         "<td width=\"" + QString::number(int(c*35))  + "\"><span style=\"font-size:6pt\"><div align=\"right\">" + QLocale().toString(dep->montant(),'f',2) + "</div></span></td>"      //! montant
                         "<td width=\"" + QString::number(int(c*60)) + "\"><span style=\"font-size:6pt\">" + Utils::ConvertitModePaiement(dep->modepaiement()) + "</span></td>"                         //! mode de paiement
                         "</tr>";
             }
        }
    }
    for (int i=0;i< wdg_bigtable->rowCount(); i++)
    {
        if (!wdg_bigtable->isRowHidden(i))
        {
            Depense *dep = getDepenseFromRow(i);
            if (dep->rubriquefiscale() == "Amortissements")
            {
                test4 += "<tr>"
                         "<td width=\"" + QString::number(int(c*45))  + "\"><span style=\"font-size:6pt\"><div align=\"right\">" + dep->date().toString(tr("d MMM yy")) + "</div></span></font></td>"   //! date
                         "<td width=\"" + QString::number(int(c*150)) + "\"><span style=\"font-size:6pt\">" + dep->objet() + "</span></td>"                                                             //! Libelle
                         "<td width=\"" + QString::number(int(c*150)) + "\"><span style=\"font-size:6pt\">" + dep->rubriquefiscale() + "</span></td>"                                                   //! rubrique fiscale
                         "<td width=\"" + QString::number(int(c*35))  + "\"><span style=\"font-size:6pt\"><div align=\"right\">" + QLocale().toString(dep->montant(),'f',2) + "</div></span></td>"      //! montant
                         "<td width=\"" + QString::number(int(c*60)) + "\"><span style=\"font-size:6pt\">" + Utils::ConvertitModePaiement(dep->modepaiement()) + "</span></td>"                         //! mode de paiement
                         "</tr>";
             }
        }
    }
    test4 += "</table>";
    test4 += "</body></html>";

    Etat_textEdit->setHtml(test4);

    proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                       proc->TaillePieddePage(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);
    delete Etat_textEdit;
    Etat_textEdit = Q_NULLPTR;

}

void dlg_depenses::RegleComptesComboBox(bool ActiveSeult)
{
    ui->ComptesupComboBox->clear();
    QListIterator<int> itcpt(m_userencours->listecomptesbancaires());
    while (itcpt.hasNext()) {
        Compte *cpt = Datas::I()->comptes->getById(itcpt.next());
        if (cpt){
            if (ActiveSeult)
            {
                if (!cpt->isDesactive())
                    ui->ComptesupComboBox->addItem(cpt->nomabrege(), QString::number(cpt->id()) );
            }
            else
                ui->ComptesupComboBox->addItem(cpt->nomabrege(), QString::number(cpt->id()) );
        }
    }
}

void    dlg_depenses::RegleAffichageFiche(enum Mode mode)
{
    m_mode = mode;

    ui->DateDepdateEdit     ->setVisible(m_mode != TableVide);
    ui->ObjetlineEdit       ->setVisible(m_mode != TableVide);
    ui->MontantlineEdit     ->setVisible(m_mode != TableVide);
    ui->PaiementcomboBox    ->setVisible(m_mode != TableVide);

    ui->ComptesupComboBox   ->setVisible(m_mode != TableVide);
    ui->RefFiscalecomboBox  ->setVisible(m_mode != TableVide);
    ui->DateDeplabel        ->setVisible(m_mode != TableVide);
    ui->Objetlabel          ->setVisible(m_mode != TableVide);
    ui->Montantlabel        ->setVisible(m_mode != TableVide);
    ui->Paiementlabel       ->setVisible(m_mode != TableVide);
    ui->RefFiscalelabel     ->setVisible(m_mode != TableVide);
    ui->frame               ->setVisible(m_mode != TableVide);

    ui->DateDepdateEdit     ->setEnabled(m_mode != Lire);
    ui->ObjetlineEdit       ->setEnabled(m_mode != Lire);
    ui->MontantlineEdit     ->setEnabled(m_mode != Lire);
    ui->PaiementcomboBox    ->setEnabled(m_mode != Lire);
    ui->ComptesupComboBox   ->setEnabled(m_mode != Lire);
    ui->RefFiscalecomboBox  ->setEnabled(m_mode != Lire);
    ui->DateDeplabel        ->setEnabled(m_mode != Lire);
    ui->Objetlabel          ->setEnabled(m_mode != Lire);
    ui->Montantlabel        ->setEnabled(m_mode != Lire);
    ui->Paiementlabel       ->setEnabled(m_mode != Lire);
    ui->RefFiscalelabel     ->setEnabled(m_mode != Lire);
    ui->OKupPushButton      ->setEnabled(m_mode == Lire || m_mode == TableVide);
    ui->GestionComptesupPushButton  ->setEnabled(m_mode == Lire || m_mode == TableVide);
    wdg_enreguppushbutton           ->setVisible(!(m_mode == Lire || m_mode == TableVide));
    wdg_annuluppushbutton           ->setVisible(!(m_mode == Lire || m_mode == TableVide));
    ui->Facturewidget               ->setVisible(m_mode == Lire);
    ui->NouvelleDepenseupPushButton ->setEnabled((m_mode == Lire || m_mode == TableVide) && m_userencours->listecomptesbancaires().size() > 0 );
    QString ttip = "";
    if( m_userencours->listecomptesbancaires().size() == 0)
        ttip = tr("Vous ne pouvez pas enregistrer de dépenses.\nAucun compte bancaire n'est enregistré.");
    ui->NouvelleDepenseupPushButton->setToolTip(ttip);
    wdg_supprimeruppushbutton       ->setVisible(m_mode == Lire);
    wdg_modifieruppushbutton        ->setVisible(m_mode == Lire);
    wdg_bigtable                    ->setEnabled(m_mode == Lire);
    ui->ChercheMontantupPushButton  ->setEnabled(m_mode == Lire);
    ui->ExportupPushButton          ->setEnabled(m_mode == Lire);
    ui->PrintupSmallButton          ->setEnabled(m_mode==Lire);
    ui->UserscomboBox               ->setEnabled(Datas::I()->users->userconnected()->isSecretaire() && m_mode==Lire);


    switch (m_mode) {
    case TableVide:
        ui->OKupPushButton      ->setShortcut(QKeySequence("Meta+Return"));
        wdg_modifieruppushbutton    ->setShortcut(QKeySequence());
        wdg_enreguppushbutton       ->setShortcut(QKeySequence());
        ui->FactureupPushButton     ->setVisible(false);
        ui->EcheancierupPushButton  ->setVisible(false);
        ui->VisuDocupTableWidget    ->setVisible(false);
        [[clang::fallthrough]];// => pas de break, on continue avec le code de Lire
    case Lire: {
        wdg_enreguppushbutton       ->setText(tr("Modifier"));
        RegleComptesComboBox(false);
        break;
    }
    case Modifier: {
        wdg_bigtable->disconnect();
        wdg_enreguppushbutton       ->setText("Valider");
        int compte = -1;
        Depense *dep = (wdg_bigtable->rowCount()>0? getDepenseFromRow(wdg_bigtable->currentRow()) : Q_NULLPTR);
        if (dep)
        {
            bool vis = (dep->modepaiement()!=ESP);
            ui->ComptesupComboBox->setVisible(vis);
            if (dep->modepaiement() != ESP)            // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte pour savoir si on peut la modifier
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
        }
        ui->OKupPushButton->setShortcut(QKeySequence());
        wdg_modifieruppushbutton->setShortcut(QKeySequence());
        wdg_enreguppushbutton->setShortcut(QKeySequence("Meta+Return"));
        RegleComptesComboBox();
        ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(compte));
        break;
    }
    case Enregistrer: {
        wdg_bigtable->disconnect();
        ui->DateDepdateEdit     ->setDate(QDate::currentDate());
        ui->ObjetlineEdit       ->setText("");
        ui->MontantlineEdit     ->setText("0,00");
        ui->ComptesupComboBox   ->setVisible(!(ui->PaiementcomboBox->currentText() == tr(ESPECES) || ui->PaiementcomboBox->currentText() == ""));
        ui->RefFiscalecomboBox  ->setCurrentText("");
        wdg_enreguppushbutton       ->setText(tr("Enregistrer"));
        ui->OKupPushButton      ->setShortcut(QKeySequence());
        wdg_modifieruppushbutton->setShortcut(QKeySequence());
        wdg_enreguppushbutton       ->setShortcut(QKeySequence("Meta+Return"));
        RegleComptesComboBox();
        ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(QString::number(m_userencours->idcomptepardefaut())));
        break;
    }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Clic sur AnnulupPushButton ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::AnnulEnreg()
{
    wdg_bigtable->disconnect();
    RegleAffichageFiche(Lire);
    MetAJourFiche();
    connect (wdg_bigtable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur courant ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_depenses::initializeUserSelected()
{
    int id = ui->UserscomboBox->currentData().toInt();
    m_userencours = map_usersliberaux->find(id).value();
    proc->MAJComptesBancaires(m_userencours);
    Datas::I()->depenses->initListeByUser(m_userencours->id());
    if( m_userencours->listecomptesbancaires().size() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de continuer!"), tr("Pas de compte bancaire enregistré pour ") + m_userencours->login());
        return false;
    }
    if (m_userencours->idcomptepardefaut() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir le journal des dépenses!"), tr("Pas de compte bancaire enregistré pour ")
                                     + m_userencours->login());
        return false;
    }

    return true;
}
void dlg_depenses::ChangeUser(int)
{
    m_initok = initializeUserSelected();
    if( !m_initok )
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
    ui->ComptesupComboBox->setVisible(ui->PaiementcomboBox->currentText() != tr(ESPECES) && ui->PaiementcomboBox->currentText() != "");
}

void dlg_depenses::ConvertitDoubleMontant()
{
    QString b = ui->MontantlineEdit->text();
    b = QLocale().toString(QLocale().toDouble(b),'f',2);
    ui->MontantlineEdit->setText(b);
}

void dlg_depenses::EnableModifiepushButton()
{
    wdg_modifieruppushbutton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::EnregistreDepense()
{
    bool OnSauteLaQuestionSuivante = false;
    QString pb = "";
    // Vérifier l'absence de slash dans l'intitulé
    if (!Utils::rgx_intitulecompta.exactMatch(ui->ObjetlineEdit->text()))
    {
        UpMessageBox::Watch(this,tr("l'intitulé n'est pas conforme"), tr("Il contient des caractères non admis"));
        return;
    }

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
    else if (ui->ComptesupComboBox->currentIndex()==-1 && ui->PaiementcomboBox->currentText()!= tr(ESPECES))
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

    QList<Depense*> veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->depenses(), ui->DateDepdateEdit->date(),
                                                              ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), m_userencours->id(),
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
        veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->depenses(), ui->DateDepdateEdit->date().addDays(-180),
                                                  ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), m_userencours->id(),
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
    QString Paiement, m;
    Paiement = ui->PaiementcomboBox->currentText();
    if (Paiement == tr(ESPECES))              m = ESP;
    else if (Paiement == tr(VIREMENT))        m = VRMT;
    else if (Paiement == tr(CARTECREDIT))     m = CB;
    else if (Paiement == tr(CHEQUE))          m = CHQ;
    else if (Paiement == tr(PRELEVEMENT))     m = PLVMT;
    else if (Paiement == tr(TIP))             m = TP;

    bool ok = true;
    QList<QVariantList> listfamfiscale = db->SelectRecordsFromTable(QStringList() << CP_FAMFISCALE_2035,
                                                                       TBL_RUBRIQUES2035, ok,
                                                                       "where " CP_REFFISCALE_2035 " = '" + Utils::correctquoteSQL(ui->RefFiscalecomboBox->currentText()) + "'");
    QString FamFiscale = listfamfiscale.at(0).at(0).toString();
    QString idCompte = ui->ComptesupComboBox->currentData().toString();

    Depense *dep = Datas::I()->depenses->CreationDepense(m_userencours->id(),                       //! idUser
                                        ui->DateDepdateEdit->date(),                                //! DateDep
                                        ui->RefFiscalecomboBox->currentText(),                      //! RefFiscale
                                        ui->ObjetlineEdit->text(),                                  //! Objet
                                        QLocale().toDouble(ui->MontantlineEdit->text()),            //! Montant
                                        FamFiscale,                                                 //! FamFiscale
                                        "",                                                         //! Monnaie
                                        0,                                                          //! idRec
                                        m,                                                          //! ModePaiement
                                        (m!=ESP? idCompte.toInt() : 0),                             //! Compte
                                        0,                                                          //! NoCheque
                                        0);                                                         //! idFacture
    // insertion de l'écriture dans la table lignescomptes quand il s'agit d'une opération bancaire
    if (m != ESP)
    {
        if (Paiement == tr(VIREMENT)) Paiement = tr("Virement débiteur");
        int a = db->getIdMaxTableComptesTableArchives();
        QHash<QString, QString> listsets;
        listsets.insert(CP_ID_LIGNCOMPTES,              QString::number(a));
        listsets.insert(CP_IDCOMPTE_LIGNCOMPTES,        idCompte);
        listsets.insert(CP_IDDEP_LIGNCOMPTES,           QString::number(dep->id()));
        listsets.insert(CP_DATE_LIGNCOMPTES,            ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
        listsets.insert(CP_LIBELLE_LIGNCOMPTES,         ui->ObjetlineEdit->text());
        listsets.insert(CP_MONTANT_LIGNCOMPTES,         QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
        listsets.insert(CP_DEBITCREDIT_LIGNCOMPTES,     "0");
        listsets.insert(CP_TYPEOPERATION_LIGNCOMPTES,   Paiement);
        db->InsertIntoTable(TBL_LIGNESCOMPTES, listsets);
    }

    wdg_bigtable->insertRow(wdg_bigtable->rowCount());
    SetDepenseToRow(dep, wdg_bigtable->rowCount()-1);
    wdg_bigtable->sortByColumn(7);

    wdg_bigtable->setEnabled(true);
    wdg_supprimeruppushbutton->setVisible(true);
    wdg_modifieruppushbutton->setVisible(true);
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
    m_mode = Lire;
    connect (wdg_bigtable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
    for (int i=0; i< wdg_bigtable->rowCount(); i++)
        if (getDepenseFromRow(i)->id() == dep->id()){
            wdg_bigtable->setCurrentCell(i,1);
            wdg_bigtable->scrollTo(wdg_bigtable->model()->index(i,1), QAbstractItemView::PositionAtCenter);
            break;
        }
    RegleAffichageFiche(Lire);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Gerer une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::GererDepense(QPushButton *widgsender)
{
    if (widgsender == wdg_modifieruppushbutton)
    {
        if (m_mode == Lire)
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
        connect (pAction_RecopieDep, &QAction::triggered,    this,   &dlg_depenses::CopieDepense);
        Depense *dep = getDepenseFromRow(wdg_bigtable->currentRow());
        if (dep)
            if (dep->idfacture()>0)
            {
                QAction *pAction_SupprFacture = menu->addAction(tr("Supprimer la facture de") + " " + dep->objet());
                connect (pAction_SupprFacture, &QAction::triggered,    this,   [=] {SupprimeFacture(dep);});
            }
    }
    QAction *pAction_ChercheVal = menu->addAction(tr("Rechercher une valeur"));
    connect (pAction_ChercheVal, &QAction::triggered,    this,   &dlg_depenses::RechercheValeur);

    // ouvrir le menu
    menu->deleteLater();
    menu->exec(cursor().pos());
}

void dlg_depenses::CopieDepense()
{
    MetAJourFiche();
    m_mode = Enregistrer;
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
    wdg_supprimeruppushbutton       ->setVisible(false);
    wdg_modifieruppushbutton        ->setVisible(false);
    wdg_enreguppushbutton           ->setVisible(true);
    wdg_annuluppushbutton           ->setVisible(true);
    ui->NouvelleDepenseupPushButton ->setEnabled(false);
    wdg_bigtable                    ->setEnabled(false);
    ui->ChercheMontantupPushButton  ->setEnabled(false);
    ui->ExportupPushButton          ->setEnabled(false);
    ui->PrintupSmallButton          ->setEnabled(false);
    wdg_bigtable                    ->disconnect();
    ui->DateDepdateEdit             ->setDate(QDate::currentDate());
    wdg_enreguppushbutton           ->setText("Enregistrer");
    ui->OKupPushButton              ->setShortcut(QKeySequence());
    wdg_modifieruppushbutton        ->setShortcut(QKeySequence());
    wdg_enreguppushbutton           ->setShortcut(QKeySequence("Meta+Return"));
    ui->EcheancierupPushButton      ->setVisible(false);
    ui->FactureupPushButton         ->setVisible(false);
    ui->VisuDocupTableWidget        ->setVisible(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Supprimer une dépense ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::SupprimerDepense()
{
    if (wdg_bigtable->selectedRanges().size() == 0) return;
    // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte
    Depense *dep = getDepenseFromRow(wdg_bigtable->currentRow());

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

    //On supprime la facture éventuelle
    if (dep->idfacture()>0)
        SupprimeFacture(dep);

    //On supprime l'écriture
    db->SupprRecordFromTable(dep->id(), CP_IDDEP_LIGNCOMPTES, TBL_LIGNESCOMPTES);
    Datas::I()->depenses->SupprimeDepense(dep);

    if (wdg_bigtable->rowCount() == 1)
    {
        ReconstruitListeAnnees();
        QString year = QDate::currentDate().toString("yyyy");
        int idx = ui->AnneecomboBox->findText(year);
        ui->AnneecomboBox->disconnect();
        ui->AnneecomboBox->setCurrentIndex(idx==-1? 0 : idx);
        RedessineBigTable();
        connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=](int) {RedessineBigTable();});
    }
    else for (int i = 0; i< wdg_bigtable->rowCount(); i++)
        if (getDepenseFromRow(i) == Q_NULLPTR)
        {
            wdg_bigtable->removeRow(i);
            if (i < wdg_bigtable->rowCount() - 1)
                wdg_bigtable->setCurrentCell(i,1);
            else
                wdg_bigtable->setCurrentCell(wdg_bigtable->rowCount()-1,1);
            break;
        }
    CalculTotalDepenses();
}

/*! -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 Affiche la ventilation des dépenses en fonction des modes de paiement: espèces, banque1, banque2...etc.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::AfficheDetailsDepenses()
{
    double TotalEspeces = 0, Global = 0;
    QMap<int, double> listcomptes;
    QString Total;
    QString tdwidth = "200";

    if (wdg_bigtable->rowCount() > 0)
        for (int k = 0; k < wdg_bigtable->rowCount(); k++)
            if (!wdg_bigtable->isRowHidden(k))
            {
                Depense*dep = getDepenseFromRow(k);
                if (dep)
                {
                    if (dep->modepaiement() == ESP)
                        TotalEspeces += dep->montant();
                    else
                    {
                        if (listcomptes.find(dep->comptebancaire()) == listcomptes.end())
                            listcomptes.insert(dep->comptebancaire(),dep->montant());
                        else
                        {
                            double total = listcomptes.find(dep->comptebancaire()).value();
                            total += dep->montant();
                            listcomptes.insert(dep->comptebancaire(),total);
                        }
                    }
                }
            }
    Total += HTML_RETOURLIGNE "<td width=\"" + tdwidth + "\"><font color = " COULEUR_TITRES "><b>"
            + tr(ESPECES)
            + " :</b></font></td><td align=\"right\" width=\"" + tdwidth + "\">" + QLocale().toString(TotalEspeces,'f',2) + "</td>";
    Global += TotalEspeces;
    for (auto it = listcomptes.begin(); it != listcomptes.end(); ++it)
    {
        Banque *bq = Datas::I()->banques->getById(it.key());
        if (bq)
        {
            Total += HTML_RETOURLIGNE "<td width=\"" + tdwidth + "\"><font color = " COULEUR_TITRES "><b>"
                    + bq->nom()
                    + " :</b></font></td><td align=\"right\" width=\"" + tdwidth + "\">" + QLocale().toString(it.value(),'f',2) + "</td>";
            Global += it.value();
        }
    }
    Total += HTML_RETOURLIGNE "<td width=\"" + tdwidth + "\"><font color = " COULEUR_TITRES "><b>"
            + tr("GLOBAL")
            + " :</b></font></td><td align=\"right\" width=\"" + tdwidth + "\">" + QLocale().toString(Global,'f',2) + "</td>";
    Total += "</p>";
    proc->Edit(Total, this->windowTitle(), false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Affiche la facture dans ui->VisuDocupTableWidget -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::AfficheFacture(Depense *dep)
{
    if (dep == Q_NULLPTR)
        dep = m_depenseencours;
    if (dep == Q_NULLPTR)
    {
        ui->FactureupPushButton     ->setVisible(false);
        ui->EcheancierupPushButton  ->setVisible(false);
        ui->VisuDocupTableWidget    ->setVisible(false);
        return;
    }
    bool hasfacture = dep->idfacture()>0;
    ui->FactureupPushButton     ->setVisible(!hasfacture);
    ui->EcheancierupPushButton  ->setVisible(!hasfacture);
    ui->VisuDocupTableWidget    ->setVisible(hasfacture);
    if (hasfacture)
    {
        if (!ui->VisuFacturecheckBox->isChecked())
        {
            ui->VisuDocupTableWidget->clear();
            ui->VisuDocupTableWidget->setColumnCount(1);
            ui->VisuDocupTableWidget->setRowCount(1);
            ui->VisuDocupTableWidget->setColumnWidth(0,ui->VisuDocupTableWidget->width()-2);
            ui->VisuDocupTableWidget->setRowHeight(0,ui->VisuDocupTableWidget->height()-2);
            ui->VisuDocupTableWidget->horizontalHeader()->setVisible(false);
            ui->VisuDocupTableWidget->verticalHeader()->setVisible(false);
            QWidget *widg = new QWidget();
            UpLabel* lab = new UpLabel(widg);
            lab->setText(tr("Voir") + "\n" + (dep->isecheancier()? tr("l'échéancier"): tr("la facture")));
            lab->setAlignment(Qt::AlignCenter);
            QGridLayout *lay = new QGridLayout(widg);
            lay->addWidget(lab,0,0,Qt::AlignCenter);
            lab->setFocusPolicy(Qt::ClickFocus);
            ui->VisuDocupTableWidget->setCellWidget(0,0,widg);
            connect(lab, &UpLabel::clicked, this,
            [=]
            {
              if (dep->lienfacture()!="")
                  emit ui->VisuDocupTableWidget->zoom();
              else
                  UpMessageBox::Watch(this, tr("La visualisation de cette facture ou échéancier n'est pas possible)"));
            });
        }
        else
        {
            QMap<QString,QVariant> doc;
            if (m_depenseencours->factureformat() == "" && m_depenseencours->factureblob() == QByteArray())
                proc->CalcImage(dep, true, true);
            doc.insert("ba", m_depenseencours->factureblob());
            doc.insert("type", m_depenseencours->factureformat());
            m_listeimages =  ui->VisuDocupTableWidget->AfficheDoc(doc, true);
        }
    }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Recalcule le total des dépenses -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::CalculTotalDepenses()
{
    double Total = 0;
    if (wdg_bigtable->rowCount() > 0)
        for (int k = 0; k < wdg_bigtable->rowCount(); k++)
            if (!wdg_bigtable->isRowHidden(k))
            {
                Depense*dep = getDepenseFromRow(k);
                if (dep) Total += dep->montant();
            }
    QString TotalRemise;
    TotalRemise = QLocale().toString(Total,'f',2);
    QString AnneeRubrique2035 = tr("Total général");
    if (ui->Rubriques2035comboBox->currentText() != "<Aucun>")
        AnneeRubrique2035 = tr("Total ") + ui->Rubriques2035comboBox->currentText();
    ui->TotallineEdit->setText(AnneeRubrique2035 + " " + ui->AnneecomboBox->currentText() + " -> " + TotalRemise);
    ui->ExportupPushButton->setEnabled(wdg_bigtable->rowCount()>0);
    ui->PrintupSmallButton->setEnabled(wdg_bigtable->rowCount()>0);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Ouvre la gestion des comptes -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::GestionComptes()
{
    dlg_comptes *Dlg_Cmpt = new dlg_comptes();
    if (Dlg_Cmpt->initOK())
        Dlg_Cmpt->exec();
}

void dlg_depenses::ZoomDoc()
{
    disconnect (proc, &Procedures::DelImage, this, &dlg_depenses::EffaceFacture);
    connect (proc, &Procedures::DelImage, this, &dlg_depenses::EffaceFacture);
    QMap<QString,QVariant> doc;
    doc.insert("ba", m_depenseencours->factureblob());
    doc.insert("type", m_depenseencours->factureformat());
    proc->CalcImage(m_depenseencours, true, true);
    proc->EditDocument(doc,
                    (m_depenseencours->isecheancier()? m_depenseencours->objetecheancier() : m_depenseencours->objet()),
                    (m_depenseencours->isecheancier()? tr("Echéancier") : tr("Facture")),
                    UpDialog::ButtonSuppr | UpDialog::ButtonPrint | UpDialog::ButtonOK);
}

void dlg_depenses::EffaceFacture()
{
    if (UpMessageBox::Question(this,
                           (m_depenseencours->isecheancier()? tr("Suppression d'échéancier") : tr("Suppression de facture")),
                           (m_depenseencours->isecheancier()? tr ("Confirmez la suppression du lien vers ") + "\n" + m_depenseencours->objetecheancier() : tr ("Confirmez la suppression de la facture ") + "\n" +  m_depenseencours->objet()),
                           UpDialog::ButtonCancel | UpDialog::ButtonOK,
                           QStringList() << "Oups!" << tr("Supprimer"))
            != UpSmallButton::STARTBUTTON)
        return;
        /* on ferme la fiche d'édition de la facture
         * on supprime la référence à la facture dans la dépense
         * on efface le contenu de ui->VisuDocupTableWidget, on la cache et on réaffiche les boutons d'ajout de facture et d'échéancier
         */
    /* on ferme la fiche d'édition de la facture*/
    emit proc->CloseEditDocument();
    SupprimeFacture(m_depenseencours);
    /* on efface le contenu de ui->VisuDocupTableWidget, on la cache et on réaffiche les boutons d'ajout de facture et d'échéancier*/
    ui->VisuDocupTableWidget->clear();
    bool hasfacture = m_depenseencours->lienfacture()!="";
    ui->FactureupPushButton     ->setVisible(!hasfacture);
    ui->EcheancierupPushButton  ->setVisible(!hasfacture);
    ui->VisuDocupTableWidget    ->setVisible(hasfacture);
}

void dlg_depenses::SupprimeFacture(Depense *dep)
{
        /* on remet à null les champs idfacture, jpg et pdf de la dépense
         * on vérifie si l'idfacture est utilisé par d'autres dépenses (cas d'un échéancier)
         * si non,
            * en cas d'accès distant
                * on inscrit l'idfacture dans la table FacturesASupprimer
            * sinon
                * on détruit l'enregistrement dans la table factures
                * on copie le fichier dans le dossier facturessanslien
                * on l'efface du dossier de factures
         * on remet à zero les idfacture et lienfacture de la dépense
         */

    /* on remet à null le champ idfacture de la dépense*/
    ItemsList::update(dep, CP_IDFACTURE_DEPENSES, QVariant());

    QString req;
    /* on vérifie si l'idfacture est utilisé par d'autres dépenses (cas d'un échéancier)*/
    bool supprimerlafacture = true;
    bool ok = true;
    /* si c'est un échéancier, et s'il est référencé par d'autres dépenses => on ne l'efface pas */
    if (dep->isecheancier())
    {
        foreach (Depense *dep1, *Datas::I()->depenses->depenses())
            if (dep1->idfacture() == dep->idfacture())
            {
                supprimerlafacture = false;
                break;
            }
    }
    if (supprimerlafacture)
    {
        /* on détruit l'enregistrement dans la table factures*/
        db->SupprRecordFromTable(dep->idfacture(), CP_ID_FACTURES, TBL_FACTURES);
        /* on inscrit le lien vers le fichier dans la table FacturesASupprimer
         * la fonction SupprimeDocsetFactures de Rufus ou RufusAdmin
         * se chargera de supprimer les fichiers du disque
         * et d'en faire une copie dans le dossier factures sans lien
         * On vérifie au préalable que cette facture ne vient pas d'être inscrite dans la table */
        if (db->StandardSelectSQL("select " CP_LIENFICHIER_FACTASUPPR " from " TBL_FACTURESASUPPRIMER " where " CP_LIENFICHIER_FACTASUPPR " = '" + dep->lienfacture() + "'", ok).size()==0)
            req = "insert into " TBL_FACTURESASUPPRIMER
                  " (" CP_LIENFICHIER_FACTASUPPR ")"
                  " values ('" + dep->lienfacture() + "')";
        db->StandardSQL(req);
    }
    /* on remet à zero les idfacture et lienfacture de la dépense*/
    dep->setlienfacture("");
    dep->setecheancier(false);
    dep->setfactureformat("");
    dep->setfactureblob(QByteArray());
    dep->setidfacture(0);
    SetDepenseToRow(m_depenseencours,wdg_bigtable->currentRow());
    AfficheFacture(dep);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Met à jour la fiche --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::MetAJourFiche()
{
    ui->frame->setVisible(wdg_bigtable->rowCount() > 0 && !wdg_bigtable->isRowHidden(wdg_bigtable->currentRow()));
    if (m_mode == Lire && wdg_bigtable->rowCount() > 0 && !wdg_bigtable->isRowHidden(wdg_bigtable->currentRow()))
    {
        ui->DateDepdateEdit     ->disconnect();
        ui->RefFiscalecomboBox  ->disconnect();
        ui->PaiementcomboBox    ->disconnect();
        if (wdg_bigtable->selectedRanges().size() == 0)
            wdg_bigtable->setCurrentCell(wdg_bigtable->rowCount()-1,1);

        m_depenseencours = getDepenseFromRow(wdg_bigtable->currentRow());

        ui->ObjetlineEdit->setText(m_depenseencours->objet());
        ui->DateDepdateEdit->setDate(m_depenseencours->date());
        ui->MontantlineEdit->setText(QLocale().toString(m_depenseencours->montant(),'f',2));
        QString A = m_depenseencours->modepaiement();                                                         // Mode de paiement - col = 4
        QString B = "";
        if (A == ESP)           A = tr(ESPECES);
        else
        {
            int idx = m_userencours->listecomptesbancaires(true).indexOf(m_depenseencours->comptebancaire());
            if( idx == -1 )
            {
                //ATTENTION ERROR
            }
            B = (Datas::I()->comptes->getById(m_userencours->listecomptesbancaires(true).at(idx)) != Q_NULLPTR? Datas::I()->comptes->getById(m_userencours->listecomptesbancaires(true).at(idx))->nomabrege() : "");
            A = Utils::ConvertitModePaiement(A);
        }
        ui->PaiementcomboBox    ->setCurrentText(A);
        ui->ComptesupComboBox   ->setCurrentIndex(ui->ComptesupComboBox->findText(B));
        ui->ComptesupComboBox   ->setVisible(B != "");
        ui->RefFiscalecomboBox  ->setCurrentText(m_depenseencours->rubriquefiscale());
        AfficheFacture(m_depenseencours);
        if (m_depenseencours->modepaiement() != ESP)            // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte pour savoir si on peut la modifier
        {
            if (m_depenseencours->isArchivee() == Depense::NoLoSo)
                db->loadDepenseArchivee(m_depenseencours);
            //qDebug() << m_depenseencours->objet() + " - id " + QString::number(m_depenseencours->id()) + " - archivée = " + (m_depenseencours->isArchivee()==Depense::Oui? "Oui" : (m_depenseencours->isArchivee()==Depense::Non? "Non" : "NoloSo"));
            if (wdg_bigtable->selectedRanges().size() > 0)
                wdg_supprimeruppushbutton->setEnabled(m_depenseencours->isArchivee() == Depense::Non);
        }
        else
            wdg_supprimeruppushbutton->setEnabled(true);
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
    if (m_mode == Enregistrer)
    {
        EnregistreDepense();
        return;
    }
    Depense *dep = getDepenseFromRow(wdg_bigtable->currentRow());
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
    else if (ui->ComptesupComboBox->currentIndex()==-1 && ui->PaiementcomboBox->currentText()!= tr(ESPECES))
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
    QList<Depense*> veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->depenses(), ui->DateDepdateEdit->date(),
                                                              ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), m_userencours->id(),
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
        veriflistdepenses = db->VerifExistDepense(*Datas::I()->depenses->depenses(), ui->DateDepdateEdit->date().addDays(-1),
                                                  ui->ObjetlineEdit->text(), QLocale().toDouble(ui->MontantlineEdit->text()), m_userencours->id(),
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
    if (Paiement == tr(ESPECES))              m = ESP;
    else if (Paiement == tr(VIREMENT))        m = VRMT;
    else if (Paiement == tr(CARTECREDIT))     m = CB;
    else if (Paiement == tr(CHEQUE))          m = CHQ;
    else if (Paiement == tr(PRELEVEMENT))     m = PLVMT;
    else if (Paiement == tr(TIP))             m = TP;
    bool ok = true;
    QList<QVariantList> listfamfiscale = db->SelectRecordsFromTable(QStringList() << CP_FAMFISCALE_2035,
                                                                       TBL_RUBRIQUES2035, ok,
                                                                       "where " CP_REFFISCALE_2035 " = '" + Utils::correctquoteSQL(ui->RefFiscalecomboBox->currentText()) + "'");
    QString FamFiscale = listfamfiscale.at(0).at(0).toString();
    if (listfamfiscale.size() > 0)                // l'écriture existe et on la modifie
    {
        ItemsList::update(dep, CP_DATE_DEPENSES,            ui->DateDepdateEdit->date());
        ItemsList::update(dep, CP_OBJET_DEPENSES,           ui->ObjetlineEdit->text());
        ItemsList::update(dep, CP_MONTANT_DEPENSES,         QLocale().toDouble(ui->MontantlineEdit->text()));
        ItemsList::update(dep, CP_REFFISCALE_DEPENSES,      ui->RefFiscalecomboBox->currentText());
        ItemsList::update(dep, CP_FAMILLEFISCALE_DEPENSES,  FamFiscale);
        ItemsList::update(dep, CP_MODEPAIEMENT_DEPENSES,    m);
        ItemsList::update(dep, CP_COMPTE_DEPENSES,          (m!=ESP? ui->ComptesupComboBox->currentData().toInt() : QVariant()));
    }
    
    // Correction de l'écriture dans la table lignescomptes
    if (Paiement == tr(ESPECES))
        db->SupprRecordFromTable(dep->id(), CP_IDDEP_LIGNCOMPTES, TBL_LIGNESCOMPTES);
    else
    {
        Paiement = ui->PaiementcomboBox->currentText();
        if (Paiement == tr(VIREMENT)) Paiement = tr("Virement débiteur");

        // on recherche si l'écriture existe dans lignescomptes et si c'est le cas, on la modifie
        QList<QVariantList> listlignescomptes = db->SelectRecordsFromTable(QStringList() << CP_ID_LIGNCOMPTES,
                                                                              TBL_LIGNESCOMPTES, ok,
                                                                              "where " CP_IDDEP_LIGNCOMPTES " = " + idDep);
        if (listlignescomptes.size() > 0)                // l'écriture existe et on la modifie
        {
           QHash<QString, QVariant> listsets;
           listsets.insert(CP_DATE_LIGNCOMPTES,             ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
           listsets.insert(CP_LIBELLE_LIGNCOMPTES,          ui->ObjetlineEdit->text());
           listsets.insert(CP_MONTANT_LIGNCOMPTES,          QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
           listsets.insert(CP_DEBITCREDIT_LIGNCOMPTES,      "0");
           listsets.insert(CP_TYPEOPERATION_LIGNCOMPTES,    Paiement);
           listsets.insert(CP_IDCOMPTE_LIGNCOMPTES,         (m!=ESP? ui->ComptesupComboBox->currentData().toString() : "null"));
           DataBase::I()->UpdateTable(TBL_LIGNESCOMPTES, listsets, "where " CP_IDDEP_LIGNCOMPTES " = " + idDep);
        }
        else           // on n'a pas trouvé la ligne, on la recherche dans les archives
        {
            QList<QVariantList> listlignesarchives = db->SelectRecordsFromTable(QStringList() << CP_ID_ARCHIVESCPT,
                                                                                   TBL_ARCHIVESBANQUE, ok,
                                                                                   "where " CP_IDDEP_ARCHIVESCPT " = " + idDep);
            if (listlignesarchives.size() > 0)                // l'écriture existe et on la modifie
            {
                QHash<QString, QVariant> listsets;
                listsets.insert(CP_DATE_ARCHIVESCPT,            ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
                listsets.insert(CP_LIBELLE_ARCHIVESCPT,         ui->ObjetlineEdit->text());
                listsets.insert(CP_DEBITCREDIT_ARCHIVESCPT,     "0");
                listsets.insert(CP_TYPEOPERATION_ARCHIVESCPT,   Paiement);
                DataBase:: I()->UpdateTable(TBL_ARCHIVESBANQUE,
                                            listsets,
                                            " where " CP_IDDEP_ARCHIVESCPT " = " + idDep);
            }
            else        // l'écriture n'existait ni dans lignescomptes ni dans archives
                        // => c'était une dépense en espèces
                        // on l'enregistre dans lignescomptes
            {
                QHash<QString, QString> listsets;
                listsets.insert(CP_IDCOMPTE_LIGNCOMPTES,        ui->ComptesupComboBox->currentData().toString());
                listsets.insert(CP_IDDEP_LIGNCOMPTES,           idDep);
                listsets.insert(CP_DATE_LIGNCOMPTES,            ui->DateDepdateEdit->date().toString("yyyy-MM-dd"));
                listsets.insert(CP_LIBELLE_LIGNCOMPTES,         ui->ObjetlineEdit->text());
                listsets.insert(CP_MONTANT_LIGNCOMPTES,         QString::number(QLocale().toDouble(ui->MontantlineEdit->text())));
                listsets.insert(CP_DEBITCREDIT_LIGNCOMPTES,     "0");
                listsets.insert(CP_TYPEOPERATION_LIGNCOMPTES,   Paiement);
                DataBase:: I()->InsertIntoTable(TBL_LIGNESCOMPTES, listsets);
            }
        }
    }
    wdg_bigtable->setEnabled(true);
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
        for (int i=0; i< wdg_bigtable->rowCount(); i++)
            if (getDepenseFromRow(i)->id() == dep->id()){
                row = i;
                break;
            }
    }
    else
    {
        // Mettre à jour l'affichage dans la table
        for (int i=0; i< wdg_bigtable->rowCount(); i++)
            if (getDepenseFromRow(i)->id() == dep->id()){
                row = i;
                break;
            }
        QString A;
        static_cast<UpLabel*>(wdg_bigtable->cellWidget(row,1))->setText(dep->date().toString(tr("d MMM yyyy") + " "));             // Date - col = 1
        static_cast<UpLabel*>(wdg_bigtable->cellWidget(row,2))->setText(" " + dep->objet());                                       // Objet - col = 2

        if (dep->monnaie() == "F")
            A = QLocale().toString(dep->montant()/6.55957,'f',2);                                                               // Montant en F converti en euros
        else
            A = QLocale().toString(dep->montant(),'f',2);                                                                       // Montant - col = 3
        static_cast<UpLabel*>(wdg_bigtable->cellWidget(row,3))->setText(A + " ");

        A = dep->modepaiement();                                                                                                // Mode de paiement - col = 4
        QString B = "";
        QString C = "";
        if (A == ESP)  A = tr(ESPECES);
        else
        {
            int idx = m_userencours->listecomptesbancaires(true).indexOf(dep->comptebancaire());
            if( idx == -1 )
            {
                //ATTENTION ERROR
            }
            B = (Datas::I()->comptes->getById(m_userencours->listecomptesbancaires(true).at(idx)) != Q_NULLPTR? Datas::I()->comptes->getById(m_userencours->listecomptesbancaires(true).at(idx))->nomabrege() : "");
            A = Utils::ConvertitModePaiement(A);
            if (A == tr(CHEQUE))
                if (dep->nocheque() > 0)
                    C += " " + QString::number(dep->nocheque());
        }
        A += " " + B + " " + C;
        static_cast<UpLabel*>(wdg_bigtable->cellWidget(row,4))->setText(" " + A);
        static_cast<UpLabel*>(wdg_bigtable->cellWidget(row,5))->setText(" " + dep->rubriquefiscale());                                  // Ref fiscale - col = 5
        static_cast<UpLabel*>(wdg_bigtable->cellWidget(row,6))->setText(" " + dep->famillefiscale());                              // Famille fiscale - col = 6
        A = dep->date().toString("yyyy-MM-dd");
        wdg_bigtable->item(row,7)->setText(dep->date().toString("yyyy-MM-dd"));                                                    // ClassementparDate - col = 7
    }

    CalculTotalDepenses();
    wdg_supprimeruppushbutton->setEnabled(wdg_bigtable->rowCount()>0);
    wdg_modifieruppushbutton->setEnabled(wdg_bigtable->rowCount()>0);
    FiltreTable();
    wdg_bigtable->setCurrentCell(row,1);
    if (dep->date() != datedepart)
    {
        wdg_bigtable->sortByColumn(7);
        for (int i=0; i< wdg_bigtable->rowCount(); i++)
            if (getDepenseFromRow(i)->id() == dep->id()){
                wdg_bigtable->scrollTo(wdg_bigtable->model()->index(i,1), QAbstractItemView::PositionAtCenter);
                break;
            }
    }
    m_mode = Lire;
    wdg_bigtable->disconnect();
    RegleAffichageFiche(Lire);
    MetAJourFiche();
    connect (wdg_bigtable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::RedessineBigTable()
{
    RemplitBigTable();
    wdg_bigtable->disconnect();
    m_depenseencours = Q_NULLPTR;
    if (wdg_bigtable->rowCount() > 0)
    {
        wdg_bigtable->setCurrentCell(wdg_bigtable->rowCount()-1,1);
        RegleAffichageFiche(Lire);
        MetAJourFiche();
        FiltreTable();
    }
    else
        RegleAffichageFiche(TableVide);
    connect (wdg_bigtable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
    //wdg_supprimeruppushbutton->setEnabled(wdg_bigtable->rowCount()>0);
    //wdg_modifieruppushbutton->setEnabled(wdg_bigtable->rowCount()>0);
}

void dlg_depenses::closeEvent(QCloseEvent *event)
{
    proc->settings()->setValue(Position_Fiche "Depenses",saveGeometry());
    event->accept();
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void dlg_depenses::keyPressEvent ( QKeyEvent * event )
{
    switch (event->key()) {
    case Qt::Key_F12:{
        if (m_mode == Lire)
            reject();
        else
            AnnulEnreg();
       break;}
    default: break;
    }
}

bool dlg_depenses::initOK() const
{
    return m_initok;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Definit les colonnes de BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::DefinitArchitectureBigTable()
{
    int                 ColCount = 9;
    wdg_bigtable->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_bigtable->setContextMenuPolicy(Qt::CustomContextMenu);
    wdg_bigtable->setPalette(QPalette(Qt::white));
    wdg_bigtable->setEditTriggers(QAbstractItemView::AnyKeyPressed
                                 |QAbstractItemView::DoubleClicked
                                 |QAbstractItemView::EditKeyPressed
                                 |QAbstractItemView::SelectedClicked);
    wdg_bigtable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_bigtable->verticalHeader()->setVisible(false);
    wdg_bigtable->setFocusPolicy(Qt::StrongFocus);
    wdg_bigtable->setColumnCount(ColCount);
    wdg_bigtable->setSelectionMode(QAbstractItemView::SingleSelection);

    QStringList LabelARemplir;
    LabelARemplir << "";
    LabelARemplir << tr("Date");
    LabelARemplir << tr("Dépense");
    LabelARemplir << tr("Montant");
    LabelARemplir << tr("Mode de paiement");
    LabelARemplir << tr("Rubrique 2035");
    LabelARemplir << tr("Famille rubrique");
    LabelARemplir << tr("Classement par date");
    LabelARemplir << "";

    wdg_bigtable->setHorizontalHeaderLabels(LabelARemplir);
    wdg_bigtable->horizontalHeader()->setVisible(true);
    wdg_bigtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    wdg_bigtable->setColumnWidth(li,0);                                                // 0 -idDepense
    li++;
    wdg_bigtable->setColumnWidth(li,100);                                              // 1 - DepDate affichage européen
    li++;
    wdg_bigtable->setColumnWidth(li,330);                                              // 2 - DepNom
    li++;
    wdg_bigtable->setColumnWidth(li,100);                                              // 3 - DepMontant
    li++;
    wdg_bigtable->setColumnWidth(li,160);                                              // 4 - DepModePaiement
    li++;
    wdg_bigtable->setColumnWidth(li,300);                                              // 5 - Rubrique 2035
    li++;
    wdg_bigtable->setColumnWidth(li,293);                                              // 6 - Famille rubrique
    li++;
    wdg_bigtable->setColumnWidth(li,0);                                                // 7 - DepDate
    li++;
    wdg_bigtable->setColumnWidth(li,45);                                               // 8 - point bleu si une facture est enregistrée

    wdg_bigtable->setColumnHidden(ColCount-2,true);

    wdg_bigtable->setGridStyle(Qt::SolidLine);
    wdg_bigtable->FixLargeurTotale();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Filtre la table en fonction des rubriques fiscales --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::FiltreTable()
{
    int rowdep = wdg_bigtable->currentRow();
    int row = rowdep;

    QString filtre = ui->Rubriques2035comboBox->currentText();
    int idrubrique = ui->Rubriques2035comboBox->currentData().toInt();
    int idx = ui->Rubriques2035comboBox->findData(idrubrique);
    for (int i=0; i<wdg_bigtable->rowCount(); i++)
    {
        if (ui->Rubriques2035comboBox->currentIndex()==0)
            wdg_bigtable->setRowHidden(i,false);
        else
        {
            Depense *dep = getDepenseFromRow(i);
            wdg_bigtable->setRowHidden(i, dep->idrubriquefiscale() != idrubrique);
        }
    }
    if (idx>0 && wdg_bigtable->isRowHidden(row))
    {
        do row--;
        while (wdg_bigtable->isRowHidden(row) && row>0);       // on cherche l'enregistrement précédent qui n'est pas caché
        if (wdg_bigtable->isRowHidden(row) && row == 0)        // si on est remonté au début de la table et qu'on n'a pas trouvé d'enregistrement non caché, on recherche vers la fin
        {
            row = rowdep;
            do row++;
            while (wdg_bigtable->isRowHidden(row) && row < (wdg_bigtable->rowCount()-1));
        }
    }
    if (!wdg_bigtable->isRowHidden(row))
    {
        wdg_bigtable->selectRow(row);
        wdg_bigtable->setCurrentCell(row,1);
        Utils::Pause(1);            // si on ne fait pas ça, le scroll marche mal
        wdg_bigtable->scrollToItem(wdg_bigtable->item(row,0), QAbstractItemView::PositionAtCenter);
        if (row != rowdep)
        {
            wdg_bigtable->disconnect();
            MetAJourFiche();
            connect (wdg_bigtable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
        }
    }
    CalculTotalDepenses();
 }


void dlg_depenses::RechercheValeur()
{
    auto completebox = [=] (UpDialog *dlg, UpComboBox *box, double montant)
    {
        box->clear();
        QStandardItemModel *listdep = new QStandardItemModel();
        foreach( Depense *dep, *Datas::I()->depenses->depenses())
        {
            if (dep->montant() == montant)
            {
                UpStandardItem *item = new UpStandardItem(dep->date().toString("d-MMM-yy") + " - " + dep->objet(), dep);
                listdep->insertRow(0,item);
            }
        }
        box->setVisible(listdep->rowCount() > 0);
        if (listdep->rowCount() > 0)
        {
            box->setModel(listdep);
            box->setEditable(false);
        }
        dlg->OKButton->setEnabled(box->isVisible());
    };
    UpDialog *dlg_ask           = new UpDialog(this);
    UpLineEdit *line            = new UpLineEdit(dlg_ask);
    UpLabel *label              = new UpLabel();
    UpComboBox *box             = new UpComboBox();
    upDoubleValidator *m_val    = new upDoubleValidator(0, 10000000 , 2, this);
    QList<double> listmontants;
    // toute la manip qui suit sert à remettre les patients en cours par ordre chronologique - si vous trouvez plus simple, ne vous génez pas

    QStandardItemModel *listmontant = new QStandardItemModel(this);
    foreach( Depense *dep, *Datas::I()->depenses->depenses())
    {
        if (!listmontants.contains(dep->montant()))
        {
            listmontants << dep->montant();
            UpStandardItem *item = new UpStandardItem(QLocale().toString(dep->montant(),'f',2), dep);
            listmontant->appendRow(item);
        }
    }
    listmontant->sort(0);
    QCompleter *compMOntantDepenses = new QCompleter(listmontant);
    compMOntantDepenses->setCompletionMode(QCompleter::InlineCompletion);
    line->setCompleter(compMOntantDepenses);
    dlg_ask     ->setModal(true);
    label       ->setText(tr("Entrez le montant à rechercher"));
    int labelwidth = Utils::CalcSize(label->text()).width();
    label       ->setFixedWidth(labelwidth);
    line        ->setAlignment(Qt::AlignRight);
    line        ->setMaxLength(9);
    line        ->setValidator(m_val);
    int linewidth = Utils::CalcSize("000000000").width();
    line        ->setFixedWidth(linewidth);
    connect(line, &QLineEdit::textEdited, line, [=]{completebox(dlg_ask, box,QLocale().toDouble(line->text()));});
    QHBoxLayout *hlay = new QHBoxLayout();
    hlay        ->insertSpacerItem(0, new QSpacerItem(5,5, QSizePolicy::Expanding));
    hlay        ->insertWidget(1,line);
    hlay        ->setSizeConstraint(QLayout::SetFixedSize);
    dlg_ask     ->dlglayout()->insertWidget(0,label);
    dlg_ask     ->dlglayout()->insertItem(1,hlay);
    dlg_ask     ->dlglayout()->insertWidget(2,box);
    box         ->setVisible(false);
    line        ->setFocus();

    dlg_ask     ->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlg_ask     ->dlglayout()->setSpacing(5);
    dlg_ask     ->setWindowTitle(tr("Recherche de montant"));
    connect(dlg_ask->OKButton,    &QPushButton::clicked, dlg_ask, &QDialog::accept);
    dlg_ask     ->OKButton->setEnabled(false);
    //dlg_ask->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    dlg_ask     ->setFixedWidth(labelwidth + 10);
    dlg_ask     ->setMaximumHeight(150);
    if (dlg_ask->exec() >0)
    {
        if (box->count() == 0)
            return;
        if (ui->Rubriques2035comboBox->currentIndex() > 0)
        {
            ui->Rubriques2035comboBox->setCurrentIndex(0);
            FiltreTable();
        }
        QStandardItemModel *model = dynamic_cast<QStandardItemModel*>(box->model());
        if (model != Q_NULLPTR)
        {
            QModelIndex idx = box->model()->index(box->currentIndex(),0);
            UpStandardItem *upitem = dynamic_cast<UpStandardItem *>(model->itemFromIndex(idx));
            if (upitem != Q_NULLPTR) {
                Depense *dep = Datas::I()->depenses->getById(upitem->item()->id());
                if (dep != Q_NULLPTR)
                {
                    if (dep->annee() != ui->AnneecomboBox->currentText().toInt())
                    {
                        int idx = ui->AnneecomboBox->findText(QString::number(dep->annee()));
                        ui->AnneecomboBox->disconnect();
                        ui->AnneecomboBox->setCurrentIndex(idx==-1? 0 : idx);
                        RemplitBigTable();
                        connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),    this,   [=](int) {RedessineBigTable();});
                    }
                    for (int i=0; i< wdg_bigtable->rowCount(); i++)
                        if (getDepenseFromRow(i) == dep){
                            m_depenseencours = dep;
                            wdg_bigtable->scrollTo(wdg_bigtable->model()->index(i,1), QAbstractItemView::PositionAtCenter);
                            wdg_bigtable->setCurrentCell(i,0);
                            break;
                        }
                    MetAJourFiche();
                }
            }
        }
    }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Annees dans le combobox Annees --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::ReconstruitListeAnnees()
{
    ui->AnneecomboBox->disconnect();
    QStringList ListeAnnees;
    for (auto it = Datas::I()->depenses->depenses()->constBegin(); it != Datas::I()->depenses->depenses()->constEnd(); ++it)
    {
        Depense* dep = const_cast<Depense*>(it.value());
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
    QString req = "select distinct dep." CP_REFFISCALE_DEPENSES ", idRubrique from " TBL_DEPENSES " dep"
                  " left join " TBL_RUBRIQUES2035 " rub"
                  " on dep." CP_REFFISCALE_DEPENSES " = rub." CP_REFFISCALE_2035
                  " where " CP_IDUSER_DEPENSES " = " + QString::number(m_userencours->id()) +
                  " ORDER BY " CP_REFFISCALE_DEPENSES;
    QList<QVariantList> ListeRubriques = db->StandardSelectSQL(req, ok);
    ListeRubriques.insert(0, (QVariantList() << tr("<Aucun>") << -1));
    for (int i = 0; i < ListeRubriques.size(); i++)
        ui->Rubriques2035comboBox->insertItem(i,ListeRubriques.at(i).at(0).toString(), ListeRubriques.at(i).at(1));
    ui->Rubriques2035comboBox->setCurrentIndex(idx);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_depenses::RemplitBigTable()
{
    wdg_bigtable->disconnect();
    wdg_bigtable->clearContents();
    wdg_bigtable->setRowCount(0);
    QList<Depense*> listDepenses;

    for (auto it = Datas::I()->depenses->depenses()->constBegin(); it != Datas::I()->depenses->depenses()->constEnd(); ++it)
    {
        Depense* dep = const_cast<Depense*>(it.value());
        if (dep->annee() == ui->AnneecomboBox->currentText().toInt())
            listDepenses << dep;
    }
    if (listDepenses.size() == 0)
    {
        RegleAffichageFiche(TableVide);
        return;
    }
    wdg_bigtable->setRowCount(listDepenses.size());
    int i=0;
    foreach(Depense *dep, listDepenses)
    {
        SetDepenseToRow(dep, i);
        ++i;
    }

    wdg_bigtable->sortItems(7);
    wdg_supprimeruppushbutton->setEnabled(false);
    connect (wdg_bigtable,     &QTableWidget::itemSelectionChanged, this,   [=] {MetAJourFiche();});
}

Depense* dlg_depenses::getDepenseFromRow(int row)
{
    return Datas::I()->depenses->getById(wdg_bigtable->item(row,0)->text().toInt());
}

void dlg_depenses::EnregistreFacture(QString typedoc)
{
    if (m_depenseencours == Q_NULLPTR)
        return;
    if (typedoc == FACTURE)
        EnregistreDocScanne(dlg_docsscanner::Facture);
    else if (typedoc == ECHEANCIER)
    {
        /* on recherche s'il y a d'autres échéanciers enregistrés dans la table factures pour cet utilisateur*/
        QString req = "select distinct dep." CP_IDFACTURE_DEPENSES ", " CP_INTITULE_FACTURES ", " CP_LIENFICHIER_FACTURES " from " TBL_DEPENSES " dep"
                      " left join " TBL_FACTURES " fac"
                      " on dep." CP_IDFACTURE_DEPENSES " = fac." CP_ID_FACTURES
                      " where " CP_ECHEANCIER_FACTURES " = 1"
                      " and " CP_IDUSER_DEPENSES " = " + QString::number(m_userencours->id()) +
                      " order by " CP_INTITULE_FACTURES;
        //qDebug() << req;
        bool ok = true;
        QList<QVariantList> ListeEch = db->StandardSelectSQL(req, ok);
        if (ListeEch.size()>0)
        {
            dlg_ask                         = new UpDialog(this);
            QListView   *listview           = new QListView(dlg_ask);
            listview->setMinimumWidth(200);
            listview->setMinimumHeight(150);
            UpSmallButton *creerecheancier  = new UpSmallButton();
            creerecheancier->setIcon(Icons::icAjouter());
            dlg_ask->dlglayout()->insertWidget(0,listview);
            dlg_ask      ->AjouteLayButtons(UpDialog::ButtonEdit | UpDialog::ButtonCancel | UpDialog::ButtonOK);
            dlg_ask      ->setWindowTitle(tr("Choisissez un échéancier"));
            dlg_ask      ->AjouteWidgetLayButtons(creerecheancier, false);
            dlg_ask->OKButton->setEnabled(false);
            dlg_ask->EditButton->setEnabled(false);

            QStandardItemModel *model = new QStandardItemModel(this);
            for (int i=0; i< ListeEch.size(); ++i)
            {
                model->setItem(i,0, new QStandardItem(ListeEch.at(i).at(0).toString()));    //! idfacture
                model->setItem(i,1, new QStandardItem(ListeEch.at(i).at(1).toString()));    //! Intitule
                model->setItem(i,2, new QStandardItem(ListeEch.at(i).at(2).toString()));    //! LienFichier
            }
            listview->setModel(model);
            UpLineDelegate *linedeleg = new UpLineDelegate();
            listview->setModelColumn(1);
            listview->setItemDelegate(linedeleg);
            connect(linedeleg, &UpLineDelegate::editingFinished, this, [=] {
                                                                            QStandardItem *item = model->itemFromIndex(listview->selectionModel()->selectedIndexes().at(0));
                                                                            QString oldintitule = ListeEch.at(item->row()).at(1).toString();
                                                                            int rowit           = item->row();
                                                                            int idech           = item->model()->item(rowit,0)->text().toInt();
                                                                            for (int i=0; i< model->rowCount(); ++i)
                                                                            {
                                                                                if (item->model()->item(i,1)->text() == item->text() && i != item->row())
                                                                                {
                                                                                    UpMessageBox::Watch(dlg_ask, tr("Nom déja utilisé"), tr("Le nom") + " " + item->text() + " " + tr("est déja utilisé"));
                                                                                    item->setText(oldintitule);
                                                                                    return;
                                                                                }
                                                                            }
                                                                            QString oldlien     = item->model()->item(rowit,2)->text();
                                                                            QString newlien     = oldlien;
                                                                            newlien             .replace(oldintitule, item->text());
                                                                            item->model()->item(rowit,2)->setText(newlien);
                                                                            QString req         = "update " TBL_FACTURES " set " CP_LIENFICHIER_FACTURES " = '" + newlien + "', " CP_INTITULE_FACTURES " = '" + item->text() + "' "
                                                                                                  " where " CP_ID_FACTURES " = " + QString::number(idech);
                                                                            DataBase::I()       ->StandardSQL(req);
                                                                            QString newfilename = Procedures::I()->AbsolutePathDirImagerie() +  NOM_DIR_FACTURES + newlien;
                                                                            QString oldfilename = Procedures::I()->AbsolutePathDirImagerie() +  NOM_DIR_FACTURES + oldlien;
                                                                            QFile(oldfilename)  .rename(newfilename);
                                                                            foreach (Depense *depacorriger, *Datas::I()->depenses->depenses())
                                                                            {
                                                                                if (depacorriger->idfacture() == idech)
                                                                                {
                                                                                    depacorriger->setlienfacture(newlien);
                                                                                    depacorriger->setobjetecheancier(item->text());
                                                                                    depacorriger->setfactureformat("");
                                                                                    depacorriger->setfactureblob(QByteArray());
                                                                                }
                                                                            }
                                                                        });
            connect (listview->selectionModel(), &QItemSelectionModel::selectionChanged, this,   [=] {
                                                                            dlg_ask->OKButton->setEnabled(listview->selectionModel()->selectedIndexes().size()>0);
                                                                            dlg_ask->EditButton->setEnabled(listview->selectionModel()->selectedIndexes().size()>0);
                                                                        });
            connect(dlg_ask->OKButton,       &QPushButton::clicked,  this, [=] { if (listview->selectionModel()->selectedIndexes().size() > 0) dlg_ask->accept(); });
            connect(creerecheancier,         &QPushButton::clicked,  this, [=] { dlg_ask->reject(); EnregistreDocScanne(dlg_docsscanner::Echeancier); dlg_ask->reject(); });
            connect(dlg_ask->CancelButton,   &QPushButton::clicked,  dlg_ask,   &UpDialog::reject);
            connect(dlg_ask->EditButton,     &QPushButton::clicked,  this, [=] { if (listview->selectionModel()->selectedIndexes().size() > 0) listview->edit(listview->selectionModel()->selectedIndexes().at(0)); });

            if (dlg_ask->exec() > 0)
            {
                int row             = listview->selectionModel()->selectedIndexes().at(0).row();
                int idfact          = static_cast<QStandardItemModel*>(listview->model())->item(row,0)->text().toInt();
                QString lienfichier = static_cast<QStandardItemModel*>(listview->model())->item(row,2)->text();
                QString objet       = static_cast<QStandardItemModel*>(listview->model())->item(row,1)->text();
                /* on a récupéré un idfacture à utiliser comme échéancier pour cette dépense*/
                ItemsList::update(m_depenseencours, CP_IDFACTURE_DEPENSES, idfact);
                m_depenseencours->setlienfacture(lienfichier);
                m_depenseencours->setecheancier(true);
                m_depenseencours->setobjetecheancier(objet);
                ui->FactureupPushButton     ->setVisible(false);
                ui->EcheancierupPushButton  ->setVisible(false);
                ui->VisuDocupTableWidget    ->setVisible(true);
                proc->CalcImage(m_depenseencours, true, true);
                QMap<QString,QVariant> doc;
                doc.insert("ba", m_depenseencours->factureblob());
                doc.insert("type", m_depenseencours->factureformat());
                m_listeimages = ui->VisuDocupTableWidget->AfficheDoc(doc, true);
                SetDepenseToRow(m_depenseencours,wdg_bigtable->currentRow());
            }
            delete model;
            return;
        }
        else
            EnregistreDocScanne(dlg_docsscanner::Echeancier);
    }
}

void dlg_depenses::EnregistreDocScanne(dlg_docsscanner::Mode mode)
{
    if (mode == dlg_docsscanner::Document)
        return;
    dlg_docsscanner *Dlg_DocsScan = new dlg_docsscanner(m_depenseencours, mode, m_depenseencours->objet(), this);
    if (!Dlg_DocsScan->initOK())
        return;
    if (Dlg_DocsScan->exec() > 0)
    {
        QMap<QString, QVariant> map = Dlg_DocsScan->getdataFacture();
        int idfact = map.value("idfacture").toInt();
        if (idfact>-1)
        {
            ItemsList::update(m_depenseencours, CP_IDFACTURE_DEPENSES, idfact);
            m_depenseencours->setlienfacture(map["lien"].toString());
            m_depenseencours->setecheancier(map["echeancier"].toBool());
            m_depenseencours->setobjetecheancier(map["objetecheancier"].toString());
            ui->FactureupPushButton     ->setVisible(false);
            ui->EcheancierupPushButton  ->setVisible(false);
            ui->VisuDocupTableWidget    ->setVisible(true);
            proc->CalcImage(m_depenseencours, true, true);
            QMap<QString,QVariant> doc;
            doc.insert("ba", m_depenseencours->factureblob());
            doc.insert("type", m_depenseencours->factureformat());
            m_listeimages = ui->VisuDocupTableWidget->AfficheDoc(doc, true);
            SetDepenseToRow(m_depenseencours,wdg_bigtable->currentRow());
        }
    }
    delete  Dlg_DocsScan;
    Dlg_DocsScan = Q_NULLPTR;
}

void dlg_depenses::SetDepenseToRow(Depense *dep, int row)
{
    //+++ ne pas utiliser insertRow() qui est très lent au fur et à mesure qu'on vide et remplit la table
    QTableWidgetItem    *pItem7;
    QString             A;
    QTableWidgetItem *pitem0;
    UpLabel *label1, *label2, *label3, *label4, *label5, *label6, *label7;
    int col = 0;
    int id = dep->id();

    pitem0 = new QTableWidgetItem;
    label1 = new UpLabel;
    label2 = new UpLabel;
    label3 = new UpLabel;
    label4 = new UpLabel;
    label5 = new UpLabel;
    label6 = new UpLabel;
    label7 = new UpLabel;

    label1->setiD(id);
    label2->setiD(id);
    label3->setiD(id);
    label4->setiD(id);
    label5->setiD(id);
    label6->setiD(id);
    label7->setiD(id);

    label1->setContextMenuPolicy(Qt::CustomContextMenu);
    label2->setContextMenuPolicy(Qt::CustomContextMenu);
    label3->setContextMenuPolicy(Qt::CustomContextMenu);
    label4->setContextMenuPolicy(Qt::CustomContextMenu);
    label5->setContextMenuPolicy(Qt::CustomContextMenu);
    label6->setContextMenuPolicy(Qt::CustomContextMenu);
    label7->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(label1,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label2,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label3,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label4,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label5,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label6,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});
    connect(label7,  &QWidget::customContextMenuRequested,   this,   [=] {MenuContextuel();});

    A = QString::number(id);                                                                    // idDepense - col = 0
    pitem0->setText(A);
    wdg_bigtable->setItem(row,col,pitem0);
    col++;

    A = dep->date().toString(tr("d MMM yyyy"));                                                 // Date - col = 1
    label1->setText(A + " ");
    label1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    wdg_bigtable->setCellWidget(row,col,label1);
    col++;

    label2->setText(" " + dep->objet());                                                        // Objet - col = 2
    wdg_bigtable->setCellWidget(row,col,label2);
    col++;

    if (dep->monnaie() == "F")
        A = QLocale().toString(dep->montant()/6.55957,'f',2);// Montant en F converti en euros
    else
        A = QLocale().toString(dep->montant(),'f',2);                                           // Montant - col = 3
    label3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    label3->setText(A + " ");
    wdg_bigtable->setCellWidget(row,col,label3);
    col++;

    A = dep->modepaiement();                                                                    // Mode de paiement - col = 4
    QString B = "";
    QString C = "";
    QString mode = Utils::ConvertitModePaiement(A);
    if (A != ESP)
    {
        int idx = m_userencours->listecomptesbancaires(true).indexOf(dep->comptebancaire());
        if( idx == -1 )
        {
            //ATTENTION ERROR
        }
        B = (Datas::I()->comptes->getById(m_userencours->listecomptesbancaires(true).at(idx)) != Q_NULLPTR? Datas::I()->comptes->getById(m_userencours->listecomptesbancaires(true).at(idx))->nomabrege() : "");
        if (A == tr(CHEQUE))
            if (dep->nocheque() > 0)
                C += " " + QString::number(dep->nocheque());
    }
    mode += " " + B + C;
    label4->setText(" " + mode);
    wdg_bigtable->setCellWidget(row,col,label4);
    col++;

    A = dep->rubriquefiscale();                                                                 // Rubrique2035 - col = 5
    label5->setText(" " + A);
    wdg_bigtable->setCellWidget(row,col,label5);
    col++;

    A = dep->famillefiscale();                                                                  // Famille fiscale - col = 6
    label6->setText(" " + A);
    wdg_bigtable->setCellWidget(row,col,label6);
    col++;

    A = dep->date().toString("yyyy-MM-dd");                                                     // ClassementparDate - col = 7 (colonne masquée)
    pItem7 = new QTableWidgetItem() ;
    pItem7->setText(A);
    wdg_bigtable->setItem(row,col,pItem7);
    col++;

    if (dep->idfacture()>0)                                                                     // une facture est enregistrée - col = 8
        label7->setPixmap(Icons::pxApres().scaled(10,10)); //WARNING : icon scaled : pxApres 10,10
    label7->setAlignment(Qt::AlignCenter);
    wdg_bigtable->setCellWidget(row,col,label7);

    wdg_bigtable->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));
}


