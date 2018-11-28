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
  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

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
*/

  ui->TireurChequelineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  ui->TireurChequelineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
  ui->MontantlineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  QDoubleValidator *val= new QDoubleValidator(this);
  val->setDecimals(2);
  ui->MontantlineEdit->setValidator(val);

  QRegExp val2 = QRegExp("[A-Z]*");
  ui->BanqueChequecomboBox->setValidator(new QRegExpValidator(val2));
  ui->BanqueChequecomboBox->lineEdit()->setMaxLength(10);

  ReconstruitListeBanques();
  /*ReconstruitListeTiers();
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
  */
}

dlg_paiementdirect::~dlg_paiementdirect()
{
    delete ui;
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

