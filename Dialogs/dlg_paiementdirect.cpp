#include "dlg_paiementdirect.h"
#include "ui_dlg_paiementdirect.h"

dlg_paiementdirect::dlg_paiementdirect(QList<int> ListidActeAPasser, Procedures *procAPasser, int PaiementAModifer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_paiementdirect)
{
  ui->setupUi(this);
  ui->Loupelabel->setPixmap(Icons::pxLoupe().scaled(30,30)); //WARNING : icon scaled : pxLoupe 20,20
  ui->Loupelabel->setText("");
  connect(ui->AnnulupPushButton, &QPushButton::clicked, this, &dlg_paiementdirect::reject);
  //setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

  proc                      = procAPasser;
  db                        = DataBase::getInstance();
  gListidActe               = ListidActeAPasser;
  gidComptableACrediter     = -1;

  QFont font = qApp->font();
  font.setBold(true);
  font.setItalic(true);
  font.setPointSize(font.pointSize()+5);

  ui->Utilisateurlabel->setFont(font);
  font.setItalic(false);
  font.setPointSize(font.pointSize()+12);
  ui->PasdePaiementlabel->setFont(font);
  ui->PasdePaiementlabel->setText(tr("Pas de\npaiement enregistré\npour cet acte"));
  ui->PasdePaiementlabel->setFixedSize(420,190);

  FermeALaFin = (gListidActe.at(0) > 0);

  restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionPaiement").toByteArray());

  m_listeComptables         = Datas::I()->users->comptables();
  m_listeParents            = Datas::I()->users->parents();
  m_userConnected           = proc->getUserConnected();

  ui->UserscomboBox->setEnabled(m_userConnected->isSecretaire());

  if (gListidActe.size() > 0)        //1. il y a un ou pusieurs actes à enregistrer - l'appel a été fait depuis la 'accueil ou par le bouton enregistrepaiement
  {
      // la fiche a été appelée par le bouton "enregistrer le paiement"
      if (gListidActe.at(0)>0)
          gidComptableACrediter = db->loadActeById(gListidActe.at(0))->idComptable();
      // la fiche a été appelée par le menu et il n'y a pas d'acte prédéterminé à enregistrer
      gidComptableACrediter = m_userConnected->getUserComptable()->id();     // -2 si le user est une secrétaire et qu'il n'y a pas de comptable
  }

  if (gidComptableACrediter == -1)
  {
      InitOK = false;
      return;
  }

  UserComptableACrediter                               = Datas::I()->users->getUserById(gidComptableACrediter);
  if (UserComptableACrediter != Q_NULLPTR)
  {
      gidCompteBancaireParDefaut          = UserComptableACrediter->getIdCompteEncaissHonoraires();
      proc                                ->setListeComptesEncaissmtUser(UserComptableACrediter->id());
      glistComptesEncaissmt               = proc->getListeComptesEncaissmtUser();
      glistComptesEncaissmtAvecDesactive  = proc->getListeComptesEncaissmtUserAvecDesactive();
  }
  if( UserComptableACrediter == Q_NULLPTR || glistComptesEncaissmt->rowCount() == 0)
  {
      UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche de paiement"), tr("Les paramètres ne sont pas trouvés pour le compte ") + UserComptableACrediter->getLogin());
      InitOK = false;
      return;
  }


  // On reconstruit le combobox des utilisateurs avec la liste des utilisateurs qui encaissent des honoraires et qui travaillent encore
  int index = 0;
  for( QMap<int, User*>::const_iterator itParent = m_listeParents->constBegin(); itParent != m_listeParents->constEnd(); ++itParent )
  {
      ui->UserscomboBox->addItem(itParent.value()->getLogin(), QString::number(itParent.value()->id()) );
      if( gidComptableACrediter != itParent.value()->id())
          ++index;
  }
  if(index>=m_listeParents->size())
      ui->UserscomboBox->setCurrentIndex(0);
  else
      ui->UserscomboBox->setCurrentIndex(index);
  // idem pour les comptes
  RegleComptesComboBox();
  ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));
/*

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
  connect (ui->ImpayeradioButton,                     SIGNAL(clicked()),                              this,           SLOT (Slot_RegleAffichageTypePaiementframe()));*/
  connect (ui->ListActesupPushButton,                 SIGNAL(clicked()),                              this,           SLOT (Slot_VoirListeActes()));
  //connect (ui->ListPaiementsTiersupPushButton,        SIGNAL(clicked()),                              this,           SLOT (Slot_VoirListePaiementsTiers()));
  connect (ui->ListeupTableWidget,                    SIGNAL(customContextMenuRequested(QPoint)),     this,           SLOT (Slot_ModifGratuit(QPoint)));
  connect (ui->ListeupTableWidget,                    SIGNAL(itemEntered(QTableWidgetItem*)),         this,           SLOT (Slot_AfficheDDN(QTableWidgetItem*)));
  /*connect (ui->MontantlineEdit,                       SIGNAL(editingFinished()),                      this,           SLOT (Slot_ConvertitDoubleMontant()));
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
*/


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

  m_listeBanques = Datas::I()->banques->banques();
  ReconstruitListeBanques();
  m_typestiers = Datas::I()->typestiers->typestiers();
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
      if (gListidActe.at(0) != 0)
      {
          gMode = EnregistrePaiementDirect;
          //Slot_EnregistrePaiementDirect();
      }
      else
      {
          gMode = Accueil;
          ui->RecImageLabel->setVisible(false);
          Slot_RegleAffichageFiche();
      }
  }
  else
  {
      AppeleParFichePaiement = true;
      //ModifPaiementTiers(PaiementAModifer);
  }
  InitOK = true;
}

dlg_paiementdirect::~dlg_paiementdirect()
{
    delete ui;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Bouton Record -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::Slot_AfficheRecord()
{
    bool radioButtonClicked = false;
    QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
    for (int n = 0; n <  allRButtons.size(); n++)
        if (allRButtons.at(n)->isChecked())
        {
            radioButtonClicked = true;
            n = allRButtons.size();
        }
    if ((gMode == EnregistrePaiementDirect) && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || radioButtonClicked || ui->DetailupTableWidget->rowCount() > 0))
        ui->RecImageLabel->setVisible(!ui->RecImageLabel->isVisible());
    else
        ui->RecImageLabel->setVisible(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Recalcule le total de la table Details -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::Slot_CalculTotalDetails()
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
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enable AnnulButton -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::Slot_EnableOKButton()
{
    ui->OKupPushButton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des différents Widget en fonction du mode de fonctionnement ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::Slot_RegleAffichageFiche()
{
    ui->dateEdit->setDate(QDate::currentDate());
    ui->Detailsframe                ->setVisible(gMode!=Accueil);
    ui->line_2                      ->setVisible(gMode!=Accueil);
    ui->line_4                      ->setVisible(gMode!=Accueil);

    if (gMode == Accueil)
    {
        setFixedWidth(680);
        setFixedHeight(HAUTEUR_MINI);
        ui->UserscomboBox       ->setEnabled(m_userConnected->isSecretaire());

        QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
        for (int n = 0; n <  allRButtons.size(); n++)
        {
            allRButtons.at(n)->setAutoExclusive(false);
            allRButtons.at(n)->setChecked(false);
            allRButtons.at(n)->setAutoExclusive(true);
        }
        ui->OKupPushButton              ->setVisible(false);
        ui->Utilisateurlabel            ->setText(tr("Gestion des paiements directs"));
        ui->ListActesupPushButton       ->setVisible(true);
        ui->NouvPaiementupPushButton    ->setVisible(true);
        ui->AnnulupPushButton            ->setVisible(true);
        ui->TypePaiementframe           ->setVisible(false);
        if (SupprimerBouton!=Q_NULLPTR)
                SupprimerBouton ->setVisible(false);
    }
    else
    {
        setFixedHeight(950);
        setFixedWidth(810);
        ui->UserscomboBox                   ->setEnabled(false);
        ui->ListActesupPushButton           ->setVisible(true);
        ui->NouvPaiementupPushButton        ->setVisible(true);

        switch (gMode) {
        case EnregistrePaiementDirect:
        {
            ui->Utilisateurlabel            ->setText(tr("Gestion des paiements directs - Enregistrer un paiement"));
            ui->OKupPushButton              ->setText(tr("Valider\net fermer"));
            ui->OKupPushButton              ->setIcon(Icons::icOK());
            ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->ActesEnAttentelabel         ->setText(tr("Salle d'attente"));
            ui->ActesEnAttentelabel_2       ->setText(tr("Actes en attente de paiement"));
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
            ui->ActesEnAttentelabel         ->setText(tr("Paiements pour cet acte"));
            ui->ActesEnAttentelabel_2       ->setText(tr("Tous les actes ayant reçu un paiement ou en attente de paiement"));
            RegleComptesComboBox(false);
            break;
        }
        default:
            break;
        }
    }
    RegleAffichageTypePaiementframe(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Retrouve le rang de la ligne selectionnée et modifie les tables en fonction ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::Slot_RenvoieRangee(bool Coche)
{

    switch (gMode) {
    case EnregistrePaiementDirect:
    {
        UpTableWidget*           TableOrigine = dynamic_cast<UpTableWidget*>(focusWidget());
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
    {
        UpTableWidget*           TableOrigine = dynamic_cast<UpTableWidget*>(sender());
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


void dlg_paiementdirect::Slot_VoirListeActes()
{
    gMode = VoirListeActes;
    RemplitLesTables(gMode);
    Slot_RegleAffichageFiche();
    ui->ListeupTableWidget->setCurrentCell(0,1);
    ui->ListeupTableWidget->setFocus();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Types de tiers payant dans le combobox TypeTiers --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ReconstruitListeTiers()
{
    QStringList ListTiers;

    ui->TypeTierscomboBox->clear();
    QList<TypeTiers*>::const_iterator ittyp;
    // toute la manip qui suit sert à remetre les banques par ordre aplhabétique - si vous trouvez plus simple, ne vous génez pas
    QStandardItemModel *model = new QStandardItemModel();
    for( ittyp = m_typestiers->constBegin(); ittyp != m_typestiers->constEnd(); ++ittyp )
    {
        TypeTiers *typ = const_cast<TypeTiers*>(*ittyp);
        QList<QStandardItem *> items;
        items << new QStandardItem(typ->typetiers());
            model->appendRow(items);
    }
    model->sort(0);
    for(int i=0; i<model->rowCount(); i++)
        ui->TypeTierscomboBox->addItem(model->item(i)->text());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir la DetailsTable en fonction du mode appelé -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::CompleteDetailsTable(UpTableWidget *TableSource, int Rangee, bool Coche)
{
    QTableWidgetItem        *pItem1, *pItem2, *pItem3, *pItem4, *pItem5, *pItem6, *pItem7;
    QDoubleValidator        *val;
    UpTableWidget           *TableDetails   = ui->DetailupTableWidget;
    UpTableWidget           *TableSalDat    = ui->SalleDAttenteupTableWidget;
    UpTableWidget           *TableOrigine = TableSource;
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
                          " WHERE idUser = " + QString::number(UserComptableACrediter->id()) + "\n"
                          " AND lig.idRecette = rec.idRecette\n"
                          " AND lig.idActe = " + TextidActe + "\n"
                          " ORDER BY DatePaiement DESC, NomTiers";
            QSqlQuery ChercheLignesRecettesQuery (requete, DataBase::getInstance()->getDataBase());
            //UpMessageBox::Watch(this,TextidActe + "\n" +requete);

            DataBase::getInstance()->traiteErreurRequete(ChercheLignesRecettesQuery,requete,"");
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
        QSqlQuery RetrouveTiersPayantQuery(requete, DataBase::getInstance()->getDataBase());
        if (DataBase::getInstance()->traiteErreurRequete(RetrouveTiersPayantQuery,requete,"")) return;
        if (RetrouveTiersPayantQuery.size() > 0)
        {
            RetrouveTiersPayantQuery.first();
            if (RetrouveTiersPayantQuery.value(0).toString() == "O")
            {
                DefinitArchitectureTableView(ui->DetailupTableWidget,ActesTiers);
                gTypeTable = ActesTiers;
            }
            else
            {
                DefinitArchitectureTableView(ui->DetailupTableWidget,ActesDirects);
                gTypeTable = ActesDirects;
            }
            ui->DetailupTableWidget->clearContents();
            ui->DetailupTableWidget->setRowCount(0);
            QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
            for (int n = 0; n <  allRButtons.size(); n++)
            {
                allRButtons.at(n)->setAutoExclusive(false);
                allRButtons.at(n)->setChecked(false);
                allRButtons.at(n)->setAutoExclusive(true);
            }
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
            QSqlQuery ChercheDetailsPaiementQuery (requete, DataBase::getInstance()->getDataBase());
            DataBase::getInstance()->traiteErreurRequete(ChercheDetailsPaiementQuery,requete,"");

            RemplirTableWidget(TableDetails,"Actes",ChercheDetailsPaiementQuery,false,Qt::Unchecked);

            // Remplir les infos sur la recette concernée
            requete =   "SELECT idRecette, idUser, DatePaiement, DateEnregistrement, Montant, ModePaiement, TireurCheque, CompteVirement, BanqueCheque, TiersPayant, NomTiers, Commission, Monnaie, idRemise, EnAttente, EnregistrePar, TypeRecette FROM " NOM_TABLE_RECETTES
                        " WHERE idRecette = " + TextidRecette;
            QSqlQuery ChercheDetailsRecetteQuery (requete, DataBase::getInstance()->getDataBase());
            DataBase::getInstance()->traiteErreurRequete(ChercheDetailsRecetteQuery,requete,"");
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
    }
    } // fin switch
    Slot_CalculTotalDetails();
    RegleAffichageTypePaiementframe(false);
    RegleAffichageTypePaiementframe(true,false);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Définit l'architecture des TableView (SelectionMode, nombre de colonnes, avec Widgets UpcheckBox et UplineBox) ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::DefinitArchitectureTableView(UpTableWidget *TableARemplir, int TypeTable)
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
            if (gMode == EnregistrePaiementDirect)
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
            if (gMode == EnregistrePaiementDirect)
                ColCount ++;
            TableARemplir->setColumnCount(ColCount);
            if (gMode == EnregistrePaiementDirect)
                TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);
            else
                TableARemplir->setSelectionMode(QAbstractItemView::SingleSelection);

            LabelARemplir << "";
            if (gMode == EnregistrePaiementDirect)
                LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
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
            if (gMode == EnregistrePaiementDirect)
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
    TableARemplir->setGridStyle(Qt::SolidLine);
    TableARemplir->FixLargeurTotale();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Banques dans le combobox Banques --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ReconstruitListeBanques()
{
    ui->BanqueChequecomboBox->clear();
    QMap<int, Banque*>::const_iterator itbanq;
    // toute la manip qui suit sert à remetre les banques par ordre aplhabétique - si vous trouvez plus simple, ne vous génez pas
    QStandardItemModel *model = new QStandardItemModel();
    for( itbanq = m_listeBanques->constBegin(); itbanq != m_listeBanques->constEnd(); ++itbanq )
    {
        Banque *bq = const_cast<Banque*>(*itbanq);
        QList<QStandardItem *> items;
        items << new QStandardItem(bq->NomBanqueAbrege()) << new QStandardItem(QString::number(bq->id()));
            model->appendRow(items);
    }
    model->sort(0);
    for(int i=0; i<model->rowCount(); i++)
        ui->BanqueChequecomboBox->addItem(model->item(i)->text(), model->item(i,1)->text());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Pose un verrou sur un acte ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::PoseVerrouCompta(int ActeAVerrouiller)
{
    QString verrourequete = "select idActe from " NOM_TABLE_VERROUCOMPTAACTES " where idActe = " + QString::number(ActeAVerrouiller);
    //UpMessageBox::Watch(this,verrourequete);
    QSqlQuery verifVerrouQuery (verrourequete, DataBase::getInstance()->getDataBase());
    if (verifVerrouQuery.size() == 0)
    {
        QString VerrouilleEnreg= "INSERT INTO " NOM_TABLE_VERROUCOMPTAACTES
                " (idActe,DateTimeVerrou, PosePar)"
                " VALUES ("  + QString::number(ActeAVerrouiller) +
                ", NOW() ,"  + QString::number(m_userConnected->id()) + ")";
        QSqlQuery verrouilleAttentePaiementQuery (VerrouilleEnreg, DataBase::getInstance()->getDataBase());
        DataBase::getInstance()->traiteErreurRequete(verrouilleAttentePaiementQuery,VerrouilleEnreg,"");
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Redessine les infos de paiement en fonction du mode de paiement choisi -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RegleAffichageTypePaiementframe(bool VerifierEmetteur, bool AppeleParClicK)
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
        ui->Tierswidget         ->setVisible(false);
        ui->TypeTierswidget     ->setVisible(false);
        ui->Banquewidget        ->setVisible(false);
        ui->TireurChequewidget  ->setVisible(false);
        ui->AttenteChequewidget ->setVisible(false);
        ui->Commissionwidget    ->setVisible(false);
        ui->CompteCreditewidget ->setVisible(false);
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
                ui->ImpayeradioButton->setVisible(true);
                ui->GratuitradioButton->setVisible(true);
                ui->CarteCreditradioButton->setVisible(true);
                ui->VirementradioButton->setVisible(false);
                ui->TiersradioButton->setVisible(true);
                ui->PaiementgroupBox->setFocusProxy(ui->CarteCreditradioButton);
                break;
            }
            case VoirListeActes:
            {
                ui->ImpayeradioButton->setVisible(true);
                ui->GratuitradioButton->setVisible(true);
                ui->CarteCreditradioButton->setVisible(true);
                ui->VirementradioButton->setVisible(true);
                ui->TiersradioButton->setVisible(false);
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
            ui->TypeTierswidget     ->setVisible(false);
            ui->AttenteChequewidget ->setVisible(false);
            ui->TireurChequewidget  ->setVisible(false);
            ui->Banquewidget        ->setVisible(false);
            if (gMode == VoirListeActes)
            {
                ui->Tierswidget         ->setVisible(true);
                ui->Montantlabel        ->setVisible(true);
                ui->MontantlineEdit     ->setVisible(true);
                ui->Montantlabel        ->setEnabled(true);
                ui->MontantlineEdit     ->setEnabled(true);
                ui->ComptesupComboBox   ->setCurrentIndex(ui->ComptesupComboBox->findData(gidCompteBancaireParDefaut));
                ui->CompteCreditewidget ->setVisible(true);
                ui->Commissionwidget    ->setVisible(true);

            }
            else
            {
                ui->Tierswidget         ->setVisible(false);
                ui->Montantlabel        ->setVisible(false);
                ui->MontantlineEdit     ->setVisible(false);
                ui->Montantlabel        ->setEnabled(true);
                ui->MontantlineEdit     ->setEnabled(true);
                ui->CompteCreditewidget ->setVisible(false);
                ui->Commissionwidget    ->setVisible(false);
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
            ui->TireurChequewidget  ->setVisible(true);
            ui->Banquewidget        ->setVisible(true);
            ui->TypeTierswidget     ->setVisible(false);
            ui->AttenteChequewidget ->setVisible(true);
            ui->Montantlabel        ->setVisible(true);
            ui->MontantlineEdit     ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->CompteCreditewidget ->setVisible(false);
            // Si la Table Details n'est pas vide , on récupère le nom du premier de la liste pour remplir la case Tireur du chèque;
            if (gMode == EnregistrePaiementDirect && AppeleParClicK)
                if (ui->DetailupTableWidget->rowCount() > 0)
                {
                    QString req = "SELECT PatNom FROM " NOM_TABLE_PATIENTS " pat, " NOM_TABLE_ACTES " act"
                                  " WHERE pat.idPat = act.idPat and idActe = " + ui->DetailupTableWidget->item(0,0)->text();
                    QSqlQuery ChercheNomPatQuery (req, DataBase::getInstance()->getDataBase());
                    DataBase::getInstance()->traiteErreurRequete(ChercheNomPatQuery,req,"");
                    ChercheNomPatQuery.first();
                    ui->TireurChequelineEdit->setText(ChercheNomPatQuery.value(0).toString());
                }
            if (gMode == VoirListeActes)
                if (ui->DetailupTableWidget->rowCount() > 0)
                    ui->Tierswidget         ->setVisible(gTypeTable == ActesTiers);
            ui->Commissionwidget    ->setVisible(false);

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
        else if (Emetteur == ui->TiersradioButton)
        {
            ui->AttenteChequewidget ->setVisible(false);
            ui->TireurChequewidget  ->setVisible(false);
            ui->Banquewidget        ->setVisible(false);
            ui->Tierswidget         ->setVisible(false);
            ui->TypeTierswidget     ->setVisible(true);
            ui->MontantlineEdit     ->setText("0,00");
            ui->Montantlabel        ->setVisible(false);
            ui->MontantlineEdit     ->setVisible(false);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            if (gMode == EnregistrePaiementDirect && AppeleParClicK)
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
            ui->AttenteChequewidget ->setVisible(false);
            ui->TireurChequewidget  ->setVisible(false);
            ui->Banquewidget        ->setVisible(false);
            ui->Tierswidget         ->setVisible(false);
            ui->TypeTierswidget     ->setVisible(false);
            ui->Montantlabel        ->setVisible(true);
            ui->MontantlineEdit     ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
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
        else if (Emetteur == ui->GratuitradioButton)
        {
            ui->AttenteChequewidget ->setVisible(false);
            ui->TireurChequewidget  ->setVisible(false);
            ui->Banquewidget        ->setVisible(false);
            ui->Tierswidget         ->setVisible(false);
            ui->TypeTierswidget     ->setVisible(false);
            ui->MontantlineEdit     ->setText("0,00");
            ui->Montantlabel        ->setVisible(false);
            ui->MontantlineEdit     ->setVisible(false);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
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
            ui->AttenteChequewidget ->setVisible(false);
            ui->TireurChequewidget  ->setVisible(false);
            ui->Banquewidget        ->setVisible(false);
            ui->Tierswidget         ->setVisible(false);
            ui->TypeTierswidget     ->setVisible(false);
            ui->Montantlabel        ->setVisible(true);
            ui->Montantlabel        ->setEnabled(false);
            ui->MontantlineEdit     ->setVisible(true);
            ui->MontantlineEdit     ->setEnabled(false);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
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
            ui->AttenteChequewidget ->setVisible(false);
            ui->TireurChequewidget  ->setVisible(false);
            ui->Banquewidget        ->setVisible(false);
            ui->Tierswidget         ->setVisible(true);
            ui->TypeTierswidget     ->setVisible(false);
            ui->Montantlabel        ->setVisible(true);
            ui->MontantlineEdit     ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->CompteCreditewidget ->setVisible(gMode!=EnregistrePaiementDirect);
        }
        ui->OKupPushButton->setEnabled(true);
    }

    ui->TypePaiementframe   ->setEnabled(gMode == EnregistrePaiementDirect);
    ui->PaiementgroupBox    ->setVisible(!(gMode == Accueil || gMode == VoirListeActes));
    switch (gMode) {
    case EnregistrePaiementDirect:
    {
        ui->OKupPushButton      ->setEnabled(QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || (Emetteur !=Q_NULLPTR) || ui->DetailupTableWidget->rowCount() > 0);
        ui->PaiementgroupBox    ->setEnabled(ModifLigneRecettePossible);
        ui->MontantlineEdit     ->setEnabled(ModifLigneRecettePossible);
        ui->Montantlabel        ->setEnabled(ModifLigneRecettePossible);
        ui->Commissionwidget    ->setVisible(ModifLigneRecettePossible);
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
    QFontMetrics fm         = QFontMetrics(qApp->font());
    int hauteurligne        = int(fm.height()*1.2);
    int nbbuttons = ui->PaiementgroupBox->findChildren<QRadioButton*>().size();
    int nbbuttonsvisibles = 0;
    for (int i=0; i<nbbuttons; ++i)
        if (ui->PaiementgroupBox->findChildren<QRadioButton*>().at(i)->isVisible())
            ++nbbuttonsvisibles;
    ui->PaiementgroupBox    ->setFixedHeight(nbbuttonsvisibles*hauteurligne+5);

}

void dlg_paiementdirect::RegleComptesComboBox(bool ActiveSeult)
{
    QStandardItemModel *model = (ActiveSeult? glistComptesEncaissmt : glistComptesEncaissmtAvecDesactive);
    ui->ComptesupComboBox->clear();
    for (int i=0; i<model->rowCount(); i++)
        ui->ComptesupComboBox->addItem(model->item(i,0)->text(), model->item(i,1)->text());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les TableListe et SalDat en fonction du mode appelé ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RemplitLesTables(int Mode)
{
    UpTableWidget        *TableListe = ui->ListeupTableWidget;
    UpTableWidget        *TableDetails = ui->DetailupTableWidget;
    UpTableWidget        *TableSalDat = ui->SalleDAttenteupTableWidget;
    QString             requete;
    disconnect (TableListe,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));

    QString user =  " AND act.UserComptable = ";
    if (m_userConnected->isLiberal())
        // l'utilisateur est un soignant liberal et responsable - il enregistre ses actes et ceux de ses éventuels salariés
        user = " AND act.UserComptable = "  + QString::number(UserComptableACrediter->id()) + "\n";
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
        user = " AND act.UserComptable = "  + QString::number(UserComptableACrediter->id()) + "\n";
    else
        user = " AND act.UserComptable = "  + QString::number(UserComptableACrediter->id()) + "\n";

    switch (Mode)
    {

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
        QSqlQuery ListeActesQuery (requete, DataBase::getInstance()->getDataBase());
        DataBase::getInstance()->traiteErreurRequete(ListeActesQuery,requete,"");
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
        QSqlQuery SalDatQuery (requete, DataBase::getInstance()->getDataBase());
        DataBase::getInstance()->traiteErreurRequete(SalDatQuery,requete,"");
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
    -- Affichage de la liste des actes -------------------------------------------------------------------------------------------------------------------------------------------------------
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
        requete +=  " AND act.UserComptable = " + QString::number(UserComptableACrediter->id()) + ")\n";
        requete +=  " order by acteDate desc, PatNom, PatPrenom";

        //UpMessageBox::Watch(this,requete);

        QSqlQuery ListeActesQuery (requete, DataBase::getInstance()->getDataBase());
        DataBase::getInstance()->traiteErreurRequete(ListeActesQuery,requete,"");
        RemplirTableWidget(TableListe,"Actes", ListeActesQuery, false, Qt::Unchecked);
        if (TableListe->rowCount() > 0)
            connect (TableListe,    SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));

        DefinitArchitectureTableView(TableSalDat,Paiements);
        DefinitArchitectureTableView(TableDetails,ActesTiers);
        break;
    }

    default:
        break;

    }
    Slot_CalculTotalDetails();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les TableWidget ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RemplirTableWidget(QTableWidget *TableARemplir, QString TypeTable, QSqlQuery TableQuery, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas)
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
-- Retirer de la table Details un élément décoché et le décocher dans la table Liste ou SalDat où il figure -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::VideDetailsTable(int Rangee)
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

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Retire un verrou sur un acte ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RetireVerrouCompta(int ActeADeverrouiller)
{
    QString VerrouilleEnreg= "DELETE FROM " NOM_TABLE_VERROUCOMPTAACTES
            " WHERE idActe = " + QString::number(ActeADeverrouiller);
    QSqlQuery verrouilleAttentePaiementQuery (VerrouilleEnreg, DataBase::getInstance()->getDataBase());
    DataBase::getInstance()->traiteErreurRequete(verrouilleAttentePaiementQuery,VerrouilleEnreg,"");
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Trie une table en remettant à jour les index des UpcheckBox et des UplineEdit ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::TrieListe(UpTableWidget *TableATrier )
{
    int ncol = TableATrier->columnCount();
    int ColonneATrier;

    if (gMode == EnregistrePaiementDirect)
        ColonneATrier = 3;
    else
        ColonneATrier = 2;

    if (TableATrier == ui->ListeupTableWidget)
    {
        if (gOrdreTri == Chronologique)
            TableATrier->sortItems(ncol - 1,Qt::DescendingOrder);
        else
            TableATrier->sortItems(ColonneATrier,Qt::AscendingOrder);
    }
    if (TableATrier == ui->DetailupTableWidget)
    {
        TableATrier->sortItems(ColonneATrier,Qt::AscendingOrder);
        TableATrier->sortItems(ncol - 1,Qt::DescendingOrder);
    }

    if (gMode == EnregistrePaiementDirect)
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
    -- Verifie si l'enregistrement coché est verrouillé par un autre utilisateur ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiementdirect::VerifVerrouCompta(UpTableWidget *TableAVerifier, int Rangee)
{
    if (gtimerAfficheActeVerrouilleClignotant->isActive())
        return false;
    QString ChercheVerrou = "SELECT UserLogin FROM " NOM_TABLE_VERROUCOMPTAACTES ", " NOM_TABLE_UTILISATEURS
                     " WHERE idActe = "  + TableAVerifier->item(Rangee,0)->text() +
                     " AND PosePar = idUser";
    //UpMessageBox::Watch(this,ChercheVerrou);
    QSqlQuery ChercheVerrouQuery (ChercheVerrou, DataBase::getInstance()->getDataBase());
    DataBase::getInstance()->traiteErreurRequete(ChercheVerrouQuery, ChercheVerrou,"");
    if (ChercheVerrouQuery.size() > 0)
    {
        ChercheVerrouQuery.first();
        ui->VerrouilleParlabel->setText(tr("Acte Verrouillé par ") + ChercheVerrouQuery.value(0).toString());
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



