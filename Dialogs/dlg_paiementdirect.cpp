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

#include "dlg_paiementdirect.h"
#include "ui_dlg_paiementdirect.h"

dlg_paiementdirect::dlg_paiementdirect(QList<int> ListidActeAPasser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_paiementdirect)
{
    ui->setupUi(this);
    ui->Loupelabel->setPixmap(Icons::pxLoupe().scaled(30,30)); //WARNING : icon scaled : pxLoupe 20,20
    ui->Loupelabel->setText("");
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowTitle(tr("Gestion des paiements directs"));

    m_listidactes             = ListidActeAPasser;

    QFont font = qApp->font();
    font.setBold(true);
    font.setItalic(true);
    font.setPointSize(font.pointSize()+5);
    QFontMetrics fm         = QFontMetrics(font);
    int maxheight = int(fm.height());

    ui->Titrelabel->setFont(font);
    font.setItalic(false);
    font.setPointSize(font.pointSize()+12);
    ui->PasdePaiementlabel->setFont(font);
    ui->PasdePaiementlabel->setText(tr("Pas de\npaiement enregistré\npour cet acte"));
    ui->PasdePaiementlabel->setFixedSize(360,190);

    ui->Datewidget            ->setMaximumHeight(maxheight);
    ui->Montantwidget         ->setMaximumHeight(maxheight);
    ui->Tierswidget           ->setMaximumHeight(maxheight);
    ui->TypeTierswidget       ->setMaximumHeight(maxheight);
    ui->Banquewidget          ->setMaximumHeight(maxheight);
    ui->TireurChequewidget    ->setMaximumHeight(maxheight);
    ui->AttenteChequewidget   ->setMaximumHeight(maxheight);
    ui->Commissionwidget      ->setMaximumHeight(maxheight);
    ui->CompteCreditewidget   ->setMaximumHeight(maxheight);

    m_fermealafin = (m_listidactes.size() > 0);

    restoreGeometry(proc->settings()->value(Position_Fiche Nom_fiche_Paiement).toByteArray());

    map_comptables         = Datas::I()->users->comptablesActes();
    if( map_comptables->size() == 0 )
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche de paiement"), tr("L'utilisateur n'est pas valide"));
        m_initok = false;
        return;
    }

    ui->ListeupTableWidget->horizontalHeader()->setSectionsClickable(true);
    connect (ui->ListeupTableWidget->horizontalHeader(), &QHeaderView::sectionClicked,  this,   &dlg_paiementdirect::ClassementListes);

    connect (ui->AnnulupPushButton,         &QPushButton::clicked,                      this,   &dlg_paiementdirect::Annuler);
    connect (ui->ListActesupPushButton,     &QPushButton::clicked,                      this,   &dlg_paiementdirect::ListeActes);
    connect (ui->ModifierupPushButton,      &QPushButton::clicked,                      this,   &dlg_paiementdirect::ModifiePaiement);
    connect (ui->NouvPaiementupPushButton,  &QPushButton::clicked,                      this,   &dlg_paiementdirect::EnregistreNouveauPaiement);
    connect (ui->OKupPushButton,            &QPushButton::clicked,                      this,   &dlg_paiementdirect::ValidePaiement);
    connect (ui->SupprimerupPushButton,     &QPushButton::clicked,                      this,   &dlg_paiementdirect::SupprimerPaiement);

    connect (ui->BanqueChequecomboBox,      &QComboBox::editTextChanged,                this,   &dlg_paiementdirect::EnableOKButton);
    connect (ui->ComptesupComboBox,         QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                        this,   &dlg_paiementdirect::EnableOKButton);
    connect (ui->dateEdit,                  &QDateEdit::dateChanged,                    this,   &dlg_paiementdirect::EnableOKButton);
    connect (ui->EnAttentecheckBox,         &QCheckBox::toggled,                        this,   &dlg_paiementdirect::EnableOKButton);
    connect (ui->MontantlineEdit,           &QLineEdit::editingFinished,                this,   &dlg_paiementdirect::ConvertitDoubleMontant);
    connect (ui->MontantlineEdit,           &QLineEdit::textEdited,                     this,   &dlg_paiementdirect::EnableOKButton);
    connect (ui->TireurChequelineEdit,      &QLineEdit::editingFinished,                this,   &dlg_paiementdirect::Majuscule);

    connect (ui->CarteCreditradioButton,    &QRadioButton::clicked,                     this,   &dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->ChequeradioButton,         &QRadioButton::clicked,                     this,   &dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->EspecesradioButton,        &QRadioButton::clicked,                     this,   &dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->GratuitradioButton,        &QRadioButton::clicked,                     this,   &dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->ImpayeradioButton,         &QRadioButton::clicked,                     this,   &dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->TiersradioButton,          &QRadioButton::clicked,                     this,   &dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->VirementradioButton,       &QRadioButton::clicked,                     this,   &dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton);

    connect (ui->ListeupTableWidget,        &QTableWidget::customContextMenuRequested,  this,   &dlg_paiementdirect::ModifGratuit);
    connect (ui->CherchePatientupLineEdit,  &QLineEdit::textEdited,                     this,   &dlg_paiementdirect::FiltreListe);

    ui->TireurChequelineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->TireurChequelineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MontantlineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QDoubleValidator *val= new QDoubleValidator(this);
    val->setDecimals(2);
    ui->MontantlineEdit->setValidator(val);
    ui->CommissionlineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->BanqueChequecomboBox->setValidator(new QRegExpValidator(Utils::rgx_MajusculeSeul));
    ui->BanqueChequecomboBox->lineEdit()->setMaxLength(10);
    ui->TierscomboBox->lineEdit()->setMaxLength(30);

    map_banques = Datas::I()->banques->banques();
    ReconstruitListeBanques();
    m_typestiers = Datas::I()->typestiers->typestiers();
    ReconstruitListeTiers();

    m_ordretri = Chronologique;

    ui->RecImageLabel->setVisible(false);
    t_timerrecord = new QTimer(this);
    t_timerrecord->start(750);
    connect (t_timerrecord, &QTimer::timeout,   this,   &dlg_paiementdirect::AfficheRecord);

    ui->DetailupTableWidget->installEventFilter(this);
    ui->VerrouilleParlabel->setVisible(false);
    t_timerafficheacteverrouilleclignotant = new QTimer(this);
    t_timerafficheacteverrouille = new QTimer(this);
    QList<UpPushButton *> allUpButtons = ui->Buttonsframe->findChildren<UpPushButton *>();
    for (int n = 0; n <  allUpButtons.size(); n++)
    {
        allUpButtons.at(n)->installEventFilter(this);
        allUpButtons.at(n)->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Large);
    }
    installEventFilter(this);
    ui->OKupPushButton->setShortcut(QKeySequence("Meta+Return"));
    ui->AnnulupPushButton->setShortcut(QKeySequence("F12"));
    m_modiflignerecettepossible = true;
    m_modifpaiementencours = false;
    ui->PasdePaiementlabel->setVisible(false);
    m_textureGray = QBrush(Qt::gray,Qt::Dense4Pattern);

    ui->RecImageLabel->setPixmap( Icons::pxEnregistrer() );
    
    ui->ComptablescomboBox->setEnabled(Datas::I()->users->getById(currentuser()->idcomptableactes()) == Q_NULLPTR && map_comptables->size()>1);

    // On reconstruit le combobox des utilisateurs avec la liste des comptables
    if( map_comptables->size() > 1 )
    {
        int nulitem = -2;
        ui->ComptablescomboBox->addItem(tr("Tout le monde"), nulitem);
    }
    for (auto it = map_comptables->constBegin(); it != map_comptables->constEnd(); ++it)
    {
        User* usr = const_cast<User*>(it.value());
        ui->ComptablescomboBox->addItem(usr->login(), QString::number(usr->id()) );
    }
    // on cherche le comptable à créditer
    if (m_listidactes.size() > 0)                     // il y a un ou pusieurs actes à enregistrer - l'appel a été fait depuis l'accueil ou par le bouton enregistrepaiement
    {
        Acte *act = Datas::I()->actes->getById(m_listidactes.at(0));
        if (act != Q_NULLPTR)
            m_useracrediter = Datas::I()->users->getById(act->idComptable());
    }
    else                                            // la fiche a été appelée par le menu et il n'y a pas d'acte prédéterminé à enregistrer
        m_useracrediter = (map_comptables->size() == 1? map_comptables->cbegin().value() : Datas::I()->users->getById(currentuser()->idcomptableactes()));     // -2 si le user est une secrétaire et qu'il n'y a pas de comptable

    if( m_useracrediter == Q_NULLPTR)
    {
        if (currentuser()->isSecretaire())
            m_useracrediter = map_comptables->cbegin().value();
    }
    if( m_useracrediter == Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche de paiement"), tr("Le comptable n'est pas retrouvé"));
        m_initok = false;
        return;
    }

    proc->MAJComptesBancaires(m_useracrediter);
    if( m_useracrediter != Q_NULLPTR && m_useracrediter->listecomptesbancaires().size() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche de paiement"), tr("Les paramètres ne sont pas trouvés pour le compte ") + m_useracrediter->login());
        m_initok = false;
        return;
    }

    ChangeComptable(m_useracrediter);
    connect (ui->ComptablescomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &dlg_paiementdirect::ChangeComptableBox);

    if (m_listidactes.size() > 0)
        EnregistreNouveauPaiement();
    else
    {
        m_mode = Accueil;
        ui->RecImageLabel->setVisible(false);
        RegleAffichageFiche();
    }
    m_initok = true;
}

dlg_paiementdirect::~dlg_paiementdirect()
{
    delete ui;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Bouton Record -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::AfficheRecord()
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
        m_mode == EnregistrePaiement
        && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || radioButtonClicked || ui->DetailupTableWidget->rowCount() > 0)
       )
        ui->RecImageLabel->setVisible(!ui->RecImageLabel->isVisible());
    else
        ui->RecImageLabel->setVisible(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Acte Verrouillé ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::AfficheActeVerrouille()
{
    disconnect(t_timerafficheacteverrouilleclignotant,  &QTimer::timeout,   this,   &dlg_paiementdirect::AfficheActeVerrouilleClignotant);
    t_timerafficheacteverrouilleclignotant->stop();
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
-- Timer affichage Acte Verrouillé ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::AfficheActeVerrouilleClignotant()
{
    ui->VerrouilleParlabel->setVisible(!ui->VerrouilleParlabel->isVisible());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Affiche la DDN dans un tooltip ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::AfficheToolTip(QTableWidgetItem *titem)
{
    if (m_mode==VoirListeActes)
    {
        if (titem->column() == 2)
        {
            QStringList list = ui->ListeupTableWidget->item(titem->row(),2)->data(Qt::UserRole).toStringList();
            QString tip = list.at(0) + "\n" + list.at(1) + "\n" + list.at(2);
            QToolTip::showText(cursor().pos(), tip);
        }
        else
            QToolTip::showText(cursor().pos(),"");
    }
    else if (m_mode==EnregistrePaiement)
    {
        if (titem->column() == 3)
        {
            QStringList list = ui->ListeupTableWidget->item(titem->row(),3)->data(Qt::UserRole).toStringList();
            QString tip = list.at(0) + "\n" + list.at(1) + "\n" + list.at(2);
            QToolTip::showText(cursor().pos(), tip);
        }
        else
            QToolTip::showText(cursor().pos(),"");
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Clic sur AnnulupPushButton ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::Annuler()
{
    QString requete;
    ui->PasdePaiementlabel->setVisible(false);
    if (m_mode == Accueil)
        reject();
    else if (m_modifpaiementencours)
    {
        UpMessageBox msgbox(this);
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
        if (m_modiflignerecettepossible)
        {
            // restaurer la ligne de recette
            requete = "INSERT INTO " TBL_RECETTES " VALUES (";
            for (int i = 0; i < m_lignerecetteamodifier.size(); i++)
            {
                requete += m_lignerecetteamodifier.at(i);
                if (i < m_lignerecetteamodifier.size() -1)
                    requete += ",";
             }
            requete += ")";
            db->StandardSQL(requete);
            m_lignerecetteamodifier.clear();

            //restaurer la ligne de commission s'il y en a eu une
            if (m_lignedepenseamodifier.size() > 0)
            {
                requete = "INSERT INTO " TBL_DEPENSES " VALUES (";
                for (int i = 0; i < m_lignedepenseamodifier.size(); i++)
                {
                    requete += m_lignedepenseamodifier.at(i);
                    if (i < m_lignedepenseamodifier.size() -1)
                        requete += ",";
                }
                requete += ")";
                db->StandardSQL(requete);
                m_lignedepenseamodifier.clear();
            }

            // restaurer la ligne de compte s'il y en a eu une
            if (m_lignecompteamodifier.size() > 0)
            {
                requete = "INSERT INTO " TBL_LIGNESCOMPTES " VALUES (";
                for (int i = 0; i < m_lignecompteamodifier.size(); i++)
                {
                    requete += m_lignecompteamodifier.at(i);
                    if (i < m_lignecompteamodifier.size() -1)
                        requete += ",";
                }
                requete += ")";
                db->StandardSQL(requete);
                m_lignecompteamodifier.clear();
            }

       }

        // 2.       restaurer les lignes de paiement
        if (m_listidactesamodifier.size() > 0)
        {
            requete = "INSERT INTO " TBL_LIGNESPAIEMENTS " (" CP_IDACTE_LIGNEPAIEMENT ", " CP_IDRECETTE_LIGNEPAIEMENT ", " CP_PAYE_LIGNEPAIEMENT ") VALUES ";
            for (int i = 0; i < m_listidactesamodifier.size(); i++)
            {
                requete += "(" + QString::number(m_listidactesamodifier.at(i)) + "," +  QString::number(m_idrecette) + "," + m_montantactesamodifier.at(i) + ")";
                if (i < m_listidactesamodifier.size() -1)
                    requete += ",";
            }
            db->StandardSQL(requete);
        }

        // 3.       restaurer les types de paiement quand il s'agit d'un paiement direct
        for (int i = 0; i < m_listidactesamodifier.size(); i++)
        {
            requete = "select " CP_IDACTE_TYPEPAIEMENTACTES " FROM " TBL_TYPEPAIEMENTACTES " where " CP_IDACTE_TYPEPAIEMENTACTES " = " + QString::number(m_listidactesamodifier.at(i));
            QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
            if (m_ok && actdata.size() == 0)
            {
                requete = "INSERT INTO " TBL_TYPEPAIEMENTACTES " (" CP_IDACTE_TYPEPAIEMENTACTES "," CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ") VALUES "
                          "(" + QString::number(m_listidactesamodifier.at(i)) + ",'" + m_modepaiementdirectamodifier + "')";
                db->StandardSQL(requete);
            }
        }
        m_mode = Accueil;
        m_modifpaiementencours = false;
        ui->RecImageLabel->setVisible(false);
        RegleAffichageFiche();
        ui->AnnulupPushButton->setText("Annuler");
        m_traiteparcloseflag = false;
    }
    else
    {
        if (m_mode == EnregistrePaiement && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || ui->DetailupTableWidget->rowCount() > 0))
        {
            UpMessageBox msgbox(this);
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
            if (m_fermealafin)
            {
                NettoieVerrousCompta();
                reject();
            }
            RemetToutAZero();
            return;
        }
        else
        {
            m_mode = Accueil;
            ui->RecImageLabel->setVisible(false);
            RegleAffichageFiche();
            m_traiteparcloseflag = false;
        }
    }
    m_modiflignerecettepossible = true;
    NettoieVerrousCompta();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Recalcule le total de la table Details -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::CalculTotalDetails()
{
    double Total = 0;
    if (ui->DetailupTableWidget->rowCount() > 0)
    {
        for (int k = 0; k < ui->DetailupTableWidget->rowCount(); k++)
        {
            QLineEdit* Line = dynamic_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(k,ui->DetailupTableWidget->columnCount()-2));
            if (Line)
                Total = Total + QLocale().toDouble(Line->text());
            else
            {
                QString mnt = ui->DetailupTableWidget->item(k,ui->DetailupTableWidget->columnCount()-2)->text();
                Total = Total + QLocale().toDouble(mnt);
            }
        }
    }
    QString TotalRemise;
    TotalRemise = QLocale().toString(Total,'f',2);
    ui->TotallineEdit->setText(TotalRemise);
    if (m_mode == EnregistrePaiement)
    {
        EnableOKButton();
        if (m_modiflignerecettepossible && !ui->GratuitradioButton->isChecked() && !ui->ImpayeradioButton->isChecked())
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
-- Change l'utilisateur comptable depuis le combobox -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ChangeComptableBox()
{
    User* usr = Datas::I()->users->getById(ui->ComptablescomboBox->currentData().toInt());
    if (usr != Q_NULLPTR)
        ChangeComptable(usr, true);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur comptable -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ChangeComptable(User* comptable, bool depuislecombo)
{
    m_useracrediter = comptable;
    if (!depuislecombo)
    {
        disconnect (ui->ComptablescomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &dlg_paiementdirect::ChangeComptableBox);
        ui->ComptablescomboBox  ->setCurrentIndex(ui->ComptablescomboBox->findData(comptable->id()));
        connect (ui->ComptablescomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &dlg_paiementdirect::ChangeComptableBox);
    }

    if (m_useracrediter != Q_NULLPTR)
        proc->MAJComptesBancaires(m_useracrediter);
    if (ui->DetailupTableWidget->rowCount()>-1)
    if (ui->DetailupTableWidget->rowCount()>-1)
    RegleComptesComboBox();
    FiltreLesTables();
}

void dlg_paiementdirect::FiltreLesTables()
{
    for (int i= 0; i<ui->ListeupTableWidget->rowCount(); ++i)
        ui->ListeupTableWidget->setRowHidden(i, (ui->ListeupTableWidget->item(i,0)->data(Qt::UserRole).toInt() != m_useracrediter->id()) && m_useracrediter != Q_NULLPTR);
    for (int i= 0; i<ui->SalleDAttenteupTableWidget->rowCount(); ++i)
        ui->SalleDAttenteupTableWidget->setRowHidden(i, (ui->SalleDAttenteupTableWidget->item(i,0)->data(Qt::UserRole).toInt() != m_useracrediter->id()) && m_useracrediter != Q_NULLPTR);
    if (ui->ListeupTableWidget->rowNoHiddenCount()==0 && ui->SalleDAttenteupTableWidget->rowNoHiddenCount() == 0)
    {
        ui->TypePaiementframe->setVisible(false);
        ui->PasdePaiementlabel->setVisible(false);
        ui->SalleDAttenteupTableWidget->clearContents();
        ui->DetailupTableWidget->clearContents();
    }
    else
    {
        ui->ListeupTableWidget->scrollToItem(ui->ListeupTableWidget->item(ui->ListeupTableWidget->FirstRowNoHidden(),0), QAbstractItemView::PositionAtTop);
        if (m_mode==VoirListeActes)
            ui->ListeupTableWidget->selectRow(ui->ListeupTableWidget->FirstRowNoHidden());
    }
}

void dlg_paiementdirect::ConvertitDoubleMontant()
{
    QString b = QLocale().toString(QLocale().toDouble(ui->MontantlineEdit->text()),'f',2);
    ui->MontantlineEdit->setText(b);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Modifie l'ordre de tri des tables ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ClassementListes(int col)
{
    if (m_mode==VoirListeActes && col!= 2) return;
    if (m_mode==EnregistrePaiement && col!= 3) return;
    if (m_ordretri == Chronologique)
        m_ordretri = Alphabetique;
    else
        m_ordretri = Chronologique;
    TrieListe(ui->ListeupTableWidget);
    FiltreListe(ui->CherchePatientupLineEdit->text());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enable AnnulButton -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::EnableOKButton()
{
    ui->OKupPushButton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer un nouveau paiement direct -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::EnregistreNouveauPaiement()
{
    m_mode = EnregistrePaiement;
    bool ok = true;
    RemplitLesTables(ok);
    if (ui->ListeupTableWidget->rowCount() == 0
            && ui->DetailupTableWidget->rowCount() == 0
            && ui->SalleDAttenteupTableWidget->rowCount() == 0)
    {
        m_mode = Accueil;
        ui->RecImageLabel->setVisible(false);
        RegleAffichageFiche();
        m_traiteparcloseflag = false;
    }
    else
    {
        RegleAffichageFiche();
        if (ui->DetailupTableWidget->rowCount() > 0)
            ui->OKupPushButton->setEnabled(true);
        ui->CarteCreditradioButton->setChecked(true);
        RegleAffichageTypePaiementframe(true,true);
        ui->PaiementgroupBox->setFocus();
    }
}

void dlg_paiementdirect::Majuscule()
{
    ui->TireurChequelineEdit->setText(Utils::trimcapitilize(ui->TireurChequelineEdit->text(),false));
    EnableOKButton();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Filtrer la liste                                                 --------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::FiltreListe(QString filtre)
{
    QList<QTableWidgetItem*> listitems = ui->ListeupTableWidget->findItems(filtre, Qt::MatchStartsWith);
    if (listitems.size()>0)
    {
        int col = (m_mode == VoirListeActes? 2 : 3);
        for (int i=0; i<listitems.size(); ++i)
        {
            if (listitems.at(i)->column() == col)
            {
                if (m_mode == VoirListeActes)
                {
                    ui->ListeupTableWidget->scrollToItem(listitems.at(i), QAbstractItemView::PositionAtCenter);
                    ui->ListeupTableWidget->selectRow(listitems.at(i)->row());
                }
                else
                ui->ListeupTableWidget->scrollToItem(listitems.at(i), QAbstractItemView::PositionAtTop);
                break;
            }
        }
    }
    else
        ui->ListeupTableWidget->clearSelection();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Créer le menu contextuel pour transformer un acte gratuit en acte payant                                                  --------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ModifGratuit(QPoint pos)
{
    QTableWidgetItem *titem = dynamic_cast<QTableWidgetItem*>(ui->ListeupTableWidget->itemAt(pos));
    if (titem)
    {
        int ro = titem->row();
        if (ui->ListeupTableWidget->item(ro,5)->text() == tr("Gratuit"))
        {
            QMenu           *menuContextuel = new QMenu(this);
            QAction         *pAction_ModifGratuit = menuContextuel->addAction(tr("Ne plus considérer comme gratuit")) ;
            connect (pAction_ModifGratuit, &QAction::triggered, this,   [&] {ModifGratuitChoixMenu("Modifier");});

            // ouvrir le menu
            menuContextuel->exec(QCursor::pos());
            delete menuContextuel;
        }
    }
}

void dlg_paiementdirect::ModifiePaiement()
{
    QString requete;
    QString ModePaiement;
    QDate   DateActe;
    /* Il s'agit de modifier un paiement dèjà enregistré.
        Plusieurs cas de figure
        .1 Il s'agit d'un acte pour lequel aucune recette n'a été enregistrée (acte enregistré en tiers et n'ayant pas encore été réglé ou acte gratuit ou impayé).
        .2 Il s'agit d'un paiement direct pour lequel une recette a été enregistrée.
        .3 Il s'agit d'un paiement par tiers  pour lequel une recette a été enregistrée.
        */
    NettoieVerrousCompta();
    m_modifpaiementencours = false;

     /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
     //      Cas n°1. il s'agit d'un acte pour lequel aucun versement n'a été enregistré pour le moment (paiement par tiers (CB, CPAM...etc...) sans paiement enregistré)
     -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    if (ui->ListeupTableWidget->selectedRanges().size() > 0
            && ui->SalleDAttenteupTableWidget->rowCount() == 0)
    {
        m_mode = EnregistrePaiement;
        QList<QTableWidgetSelectionRange>  RangeeSelectionne = ui->ListeupTableWidget->selectedRanges();
        int ab          = RangeeSelectionne.at(0).topRow();
        ModePaiement    = ui->ListeupTableWidget->item(ab,5)->text();
        int idActe      = ui->ListeupTableWidget->item(ab,0)->text().toInt();
        DateActe        = QDate::fromString(ui->ListeupTableWidget->item(ab,1)->text(),tr("dd-MM-yyyy"));
        db->SupprRecordFromTable(idActe, CP_IDACTE_TYPEPAIEMENTACTES, TBL_TYPEPAIEMENTACTES);
        m_listidactes.clear();
        m_listidactes << idActe;
        bool ok = true;
        RemplitLesTables(ok);
        RegleAffichageFiche();
        if (ModePaiement == tr("Gratuit"))
            ui->GratuitradioButton->setChecked(true);
        else if (ModePaiement == tr("Carte bancaire"))
            ui->CarteCreditradioButton->setChecked(true);
        else if (ModePaiement == tr(CHEQUE))
            ui->ChequeradioButton->setChecked(true);
        else if (ModePaiement != tr("Non enregistré"))
        {
            ui->TiersradioButton->setChecked(true);
            ui->TypeTierscomboBox->setCurrentText(ModePaiement);
        }
        ui->dateEdit->setDate(DateActe);
        RegleAffichageTypePaiementframe(true,false);
        m_listidactes.clear();
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
        // On retrouve l'idRecette de LignesRecettes correspondant au paiement à modifier
        int ab      = ui->SalleDAttenteupTableWidget->selectedRanges().at(0).topRow();
        m_idrecette  = ui->SalleDAttenteupTableWidget->item(ab,0)->text().toInt();
        requete = "SELECT " CP_ID_LIGNRECETTES " FROM " TBL_RECETTES " WHERE " CP_ID_LIGNRECETTES " = " + QString::number(m_idrecette);
        QVariantList recdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
        if (m_ok && recdata.size() == 0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier ce paiement pour le moment"),
                                tr("Il est en cours de modification par un autre utilisateur."));
            bool ok = true;
            RemplitLesTables(ok);
            FiltreLesTables();
            return;
        }
        requete = "SELECT " CP_IDACTE_LIGNEPAIEMENT " FROM " TBL_LIGNESPAIEMENTS
                " WHERE " CP_IDRECETTE_LIGNEPAIEMENT " = " + QString::number(m_idrecette) +
                " AND " CP_IDACTE_LIGNEPAIEMENT " IN (SELECT idActe FROM " TBL_VERROUCOMPTAACTES " WHERE PosePar != " + QString::number(currentuser()->id()) + ")";
        QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
        if (m_ok && actdata.size() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier ce paiement pour le moment."),
                                tr("Certains actes qui le composent sont actuellement verrouillés par d'autres utilisateurs."));
            return;
        }

        // on fait la liste des actes et des montants payés à partir de DetailupTableWidget
        m_mode = EnregistrePaiement;
        m_modifpaiementencours = true;
        m_listidactesamodifier.clear();
        m_montantactesamodifier.clear();
        for (int i = 0; i < ui->DetailupTableWidget->rowCount(); i++)
        {
            m_listidactesamodifier     << ui->DetailupTableWidget->item(i,0)->text().toInt();
            m_montantactesamodifier   << QString::number(QLocale().toDouble(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-2)->text()),'f',2);
            if(ui->EspecesradioButton->isChecked())     m_modepaiementdirectamodifier = ESP;
            if(ui->ChequeradioButton->isChecked())      m_modepaiementdirectamodifier = CHQ;
        }
        requete = "SELECT "
                    CP_ID_LIGNRECETTES ", "
                    CP_IDUSER_LIGNRECETTES ", "
                    CP_DATE_LIGNRECETTES ", "
                    CP_DATEENREGISTREMENT_LIGNRECETTES ", "
                    CP_MONTANT_LIGNRECETTES ", "
                    CP_MODEPAIEMENT_LIGNRECETTES ","
                    CP_TIREURCHEQUE_LIGNRECETTES ", "
                    CP_IDCPTEVIREMENT_LIGNRECETTES ", "
                    CP_BANQUECHEQUE_LIGNRECETTES ", "
                    CP_TIERSPAYANT_LIGNRECETTES ", "
                    CP_NOMPAYEUR_LIGNRECETTES ","
                    CP_COMMISSION_LIGNRECETTES ", "
                    CP_MONNAIE_LIGNRECETTES ", "
                    CP_IDREMISECHQ_LIGNRECETTES ", "
                    CP_CHQENATTENTE_LIGNRECETTES ", "
                    CP_IDUSERENREGISTREUR_LIGNRECETTES ", "
                    CP_TYPERECETTE_LIGNRECETTES ","
                    " datediff(" CP_DATEENREGISTREMENT_LIGNRECETTES ", NOW()) as Delai"
                    " FROM " TBL_RECETTES
                    " WHERE " CP_ID_LIGNRECETTES " = " + QString::number(m_idrecette);
        QVariantList recettedata = db->getFirstRecordFromStandardSelectSQL(requete,m_ok);
        if (!m_ok || recettedata.size()==0)
            return;
        //qDebug() << requete;

        /* Verifier si on peut modifier la recette - impossible si:
             . la date d'enregistrement remonte à plus de 90 jours
             . c'est un chèque et il a été déposé en banque*/
        if (recettedata.at(17).toInt() > 90)                                                            //             . la date d'enregistrement remonte à plus de 90 jours
        {
            UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier "
                                        "les données comptables de ce paiement"),
                                tr("Il a été enregistré il y a plus de 90 jours!"));
            m_modiflignerecettepossible = false;
        }
        else if (recettedata.at(5).toString() == CHQ && recettedata.at(13).toInt() > 0)     //             . c'est un chèque et il a été déposé en banque
        {
            UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier les données comptables de ce paiement"), tr("Le chèque a été déposé en banque!"));
            m_modiflignerecettepossible = false;
        }
        else
        {
            // nettoyer LignesRecettes et le mettre en mémoire au cas où on devrait le restaurer
            m_lignerecetteamodifier.clear();
            m_lignerecetteamodifier << recettedata.at(0).toString();                                              //idRecette
            m_lignerecetteamodifier << recettedata.at(1).toString();                                              //idUser
            if (recettedata.at(2).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + recettedata.at(2).toDate().toString("yyyy-MM-dd") + "'";         //DatePaiement
            if (recettedata.at(3).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + recettedata.at(3).toDate().toString("yyyy-MM-dd") + "'";         //DateEnregistrement
            m_lignerecetteamodifier << recettedata.at(4).toString();                                              //Montant
            if (recettedata.at(5).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + recettedata.at(5).toString() + "'";                              //ModePaiement
            if (recettedata.at(6).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + Utils::correctquoteSQL(recettedata.at(6).toString()) + "'";     //TireurCheque
            if (recettedata.at(7).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << recettedata.at(7).toString() ;                                         //CompteVirement
            if (recettedata.at(8).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + Utils::correctquoteSQL(recettedata.at(8).toString()) + "'";     //BanqueCheque
            if (recettedata.at(9).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + Utils::correctquoteSQL(recettedata.at(9).toString()) + "'";     //TiersPayant
            if (recettedata.at(10).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + Utils::correctquoteSQL(recettedata.at(10).toString()) + "'";    //NomTiers
            if (recettedata.at(11).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << recettedata.at(11).toString();                                         //Commission
            if (recettedata.at(12).toString().isEmpty())
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + recettedata.at(12).toString() + "'";                             //Monnaie
            if (recettedata.at(13).toInt() == 0)
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << recettedata.at(13).toString();                                         //IdRemise
            if (recettedata.at(14).toInt() == 0)
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << recettedata.at(14).toString();                                         //EnAttente
            if (recettedata.at(15).toInt() == 0)
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << recettedata.at(15).toString();                                         //EnregistrePar
            if (recettedata.at(16).toInt() == 0)
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << recettedata.at(16).toString();                                         //TypeRecette
            db->SupprRecordFromTable(m_idrecette, CP_ID_LIGNRECETTES, TBL_RECETTES);
            ui->SupprimerupPushButton->setVisible(true);
        }

        //Nettoyer typesPaiementsActes
        // rechercher s'il y a eu plusieurs lignes de paiements pour cet acte et s'il n'y en a qu'une détruire les infos de type paiement actes
        bool GratuitImpayeVisible = true;
        for (int i = 0; i < m_listidactesamodifier.size(); i++)
        {
            requete = "SELECT " CP_IDACTE_LIGNEPAIEMENT " FROM " TBL_LIGNESPAIEMENTS " WHERE " CP_IDACTE_LIGNEPAIEMENT " = " + QString::number(m_listidactesamodifier.at(i));
            QList<QVariantList> actlist = db->StandardSelectSQL(requete, m_ok);
            if (actlist.size() > 1)
                GratuitImpayeVisible = false;
            if (actlist.size() == 1)
                db->SupprRecordFromTable(m_listidactesamodifier.at(i), CP_IDACTE_TYPEPAIEMENTACTES, TBL_TYPEPAIEMENTACTES);
        }

        // Nettoyer LignesPaiements
        db->SupprRecordFromTable(m_idrecette, CP_IDRECETTE_LIGNEPAIEMENT, TBL_LIGNESPAIEMENTS);

        m_listidactes = m_listidactesamodifier;
        bool ok = true;
        RemplitLesTables(ok);
        RegleAffichageFiche();
        ui->SupprimerupPushButton->setVisible(true);
        RegleAffichageTypePaiementframe(true,true);
        ui->dateEdit->setDate(recettedata.at(2).toDate());
        if (!GratuitImpayeVisible)
        {
            ui->GratuitradioButton->setEnabled(false);
            ui->ImpayeradioButton->setEnabled(false);
        }
        for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
        {
            QList<QTableWidgetItem*> items;
            items = ui->DetailupTableWidget->findItems(QString::number(m_listidactesamodifier.at(i)),Qt::MatchExactly);
            int ik = items.at(0)->row();
            QString B = QLocale().toString(m_montantactesamodifier.at(i).toDouble(),'f',2);
            UpLineEdit* Paye = static_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(ik,ui->DetailupTableWidget->columnCount()-2));
            Paye->setText(B);
        }
        m_listidactes.clear();
        ui->AnnulupPushButton->setText(tr("Annuler et\nrétablir l'écriture"));
        ui->OKupPushButton->setEnabled(false);
        if (m_modiflignerecettepossible)
        {
            ui->dateEdit->setFocus();
            ui->dateEdit->setSelectedSection(QDateTimeEdit::DaySection);
        }
        return;
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des différents Widget en fonction du mode de fonctionnement ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RegleAffichageFiche()
{
    ui->dateEdit                    ->setDate(m_currentdate);
    ui->Detailsframe                ->setVisible(m_mode!=Accueil);
    ui->line_2                      ->setVisible(m_mode!=Accueil);
    ui->line_4                      ->setVisible(m_mode!=Accueil);
    ui->AnnulupPushButton           ->setVisible(m_mode!=Accueil);
    ui->NouvPaiementupPushButton    ->setVisible(m_mode==Accueil);
    ui->ListActesupPushButton       ->setVisible(m_mode==Accueil);
    ui->OKupPushButton              ->setVisible(m_mode!=VoirListeActes);
    ui->ModifierupPushButton        ->setVisible(m_mode==VoirListeActes);
    ui->PasdePaiementlabel          ->setVisible(false);
    ui->Comptablelabel              ->setVisible(m_mode!=Accueil);
    ui->ComptablescomboBox          ->setVisible(m_mode!=Accueil);
    ui->ComptablescomboBox          ->setEnabled(Datas::I()->users->getById(currentuser()->idcomptableactes()) == Q_NULLPTR
                                                 && (m_mode == VoirListeActes || (m_mode == EnregistrePaiement && ui->DetailupTableWidget->rowCount()==0))
                                                 && map_comptables->size()>1);
    ui->SupprimerupPushButton       ->setVisible(false);
    ui->CherchePatientupLineEdit    ->clear();

    disconnect (ui->SalleDAttenteupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
    disconnect (ui->ListeupTableWidget,         &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
    if (m_mode == Accueil)
    {
        setFixedWidth(680);
        setFixedHeight(HAUTEUR_MINI);

        QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
        for (int n = 0; n <  allRButtons.size(); n++)
        {
            allRButtons.at(n)->setAutoExclusive(false);
            allRButtons.at(n)->setChecked(false);
            allRButtons.at(n)->setAutoExclusive(true);
        }
        ui->OKupPushButton              ->setText(tr("Fermer"));
        ui->OKupPushButton              ->setIcon(Icons::icOK());
        ui->Titrelabel                  ->setText(tr("Gestion des paiements directs"));
        ui->TypePaiementframe           ->setVisible(false);
    }
    else
    {
        setFixedHeight(950);
        setFixedWidth(ui->ListeupTableWidget->width() + layout()->contentsMargins().left() + layout()->contentsMargins().right());

        switch (m_mode) {
        case EnregistrePaiement:
        {
            ui->Titrelabel                  ->setText(tr("Gestion des paiements directs - Enregistrer un paiement"));
            ui->OKupPushButton              ->setText(tr("Valider\net fermer"));
            ui->OKupPushButton              ->setIcon(Icons::icOK());
            ui->ActesEnAttentelabel         ->setText(tr("Salle d'attente"));
            ui->ActesEnAttentelabel_2       ->setText(tr("Actes en attente de paiement"));
            ui->PaiementgroupBox            ->setFocusProxy(ui->CarteCreditradioButton);
            RegleComptesComboBox();
            ui->ComptesupComboBox       ->setCurrentIndex(ui->ComptesupComboBox->findData(m_useracrediter->idcomptepardefaut()));
            break;
        }
        case VoirListeActes:
        {
            ui->Titrelabel                  ->setText(tr("Gestion des paiements directs - Tous les actes effectués"));
            ui->ActesEnAttentelabel         ->setText(tr("Paiements pour cet acte"));
            ui->ActesEnAttentelabel_2       ->setText(tr("Tous les actes ayant reçu un paiement ou en attente de paiement"));
            RegleComptesComboBox(false);
            connect (ui->SalleDAttenteupTableWidget, &QTableWidget::itemSelectionChanged, this, [=] {RenvoieRangee();});
            connect (ui->ListeupTableWidget,         &QTableWidget::itemSelectionChanged, this, [=] {RenvoieRangee();});
            break;
        }
        default:
            break;
        }
    }
    RegleAffichageTypePaiementframe(false);
    FiltreLesTables();
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des widget dans le cadre DetailsPaiement en fonction du mode de fonctionnement et du radiobutton sélectionné ------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RegleAffichageTypePaiementframeDepuisBouton()
{
    RegleAffichageTypePaiementframe(true, true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Retrouve le rang de la ligne selectionnée et modifie les tables en fonction ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RenvoieRangee(bool Coche)
{
    switch (m_mode) {
    case EnregistrePaiement:
    {
        UpTableWidget* TableOrigine = dynamic_cast<UpTableWidget*>(focusWidget());
        if (!TableOrigine) return;
        UpCheckBox* Check = dynamic_cast<UpCheckBox*>(sender());
        if(Check)
        {
            int R = Check->rowTable();
            if (Check->parent()->parent() == ui->DetailupTableWidget)
                VideDetailsTable(R);
            else
                CompleteDetailsTable(TableOrigine,R,Coche);
        }
        if (m_modifpaiementencours)
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

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des widget dans le cadre DetailsPaiement en fonction du mode de fonctionnement et du radiobutton sélectionné ------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::SupprimerPaiement()
{
    UpMessageBox msgbox(this);
    msgbox.setText(tr("Voulez vous vraiment supprimer les informations de cette écriture?"));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton;
    OKBouton.setText(tr("Supprimer les informations"));
    UpSmallButton NoBouton;
    NoBouton.setText(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton)
        return;
    RemetToutAZero();
    m_modifpaiementencours = false;
    m_mode = Accueil;
    ui->RecImageLabel->setVisible(false);
    RegleAffichageFiche();
    m_traiteparcloseflag = false;
    m_modiflignerecettepossible = true;
    ui->AnnulupPushButton->setText("Annuler");
    NettoieVerrousCompta();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- ValidupPushButton cliqué en fonction du mode de fonctionnement -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ValidePaiement()
{
    switch (m_mode) {
    case Accueil:
    {
        Annuler();
        break;
    }
    case EnregistrePaiement:
    {
        if (VerifCoherencePaiement())
        {
            switch (EnregistreRecette()) {
            case Impossible:
            case OK:
            {
                if (m_fermealafin) reject();
                RemetToutAZero();
                m_modifpaiementencours = false;
                m_mode = Accueil;
                ui->RecImageLabel->setVisible(false);
                m_modiflignerecettepossible = true;
                RegleAffichageFiche();
                ui->SupprimerupPushButton->setVisible(false);
                ui->AnnulupPushButton->setText("Annuler");
                NettoieVerrousCompta();
                break;
            }
            default:
                break;
            }
        }
       break;
    }
    default:
        break;
    }
}
void dlg_paiementdirect::ListeActes()
{
    bool ok = true;
    m_mode = VoirListeActes;
    RemplitLesTables(ok);
    if (ok)
    {
        RegleAffichageFiche();
        ui->ListeupTableWidget->setCurrentCell(0,1);
        ui->ListeupTableWidget->setFocus();
    }
    else
        m_mode = Accueil;
}

void dlg_paiementdirect::closeEvent(QCloseEvent *event)
{
    proc->settings()->setValue(Position_Fiche Nom_fiche_Paiement, saveGeometry());
    event->accept();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiementdirect::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        if (obj->inherits("UpLineEdit"))
        {
            if (obj->parent()->parent() == ui->DetailupTableWidget  && m_mode == EnregistrePaiement)
            {
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                m_valeuravantchangement = Line->text();
                m_valeurmaxi = ui->DetailupTableWidget->item(Line->Row(),Line->Column()-1)->text();
                Line->selectAll();
            }
        }
    }
    if (event->type() == QEvent::FocusOut)
    {
        if (obj->inherits("UpLineEdit"))
        {
            if (obj->parent()->parent() == ui->DetailupTableWidget  && m_mode == EnregistrePaiement)
            {
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                if (QLocale().toDouble(Line->text()) > QLocale().toDouble(m_valeurmaxi))
                {
                    QSound::play(NOM_ALARME);
                    m_valeuravantchangement = QLocale().toString(QLocale().toDouble(m_valeurmaxi),'f',2);   // Montant payé
                }
                else
                    m_valeuravantchangement = QLocale().toString(QLocale().toDouble(Line->text()),'f',2);   // Montant payé
                Line->setText(m_valeuravantchangement);
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
                    if (m_mode == EnregistrePaiement
                        && (CheckBox->parent()->parent() == ui->ListeupTableWidget || CheckBox->parent()->parent() == ui->SalleDAttenteupTableWidget)
                        && !CheckBox->isChecked())
                    {
                        UpTableWidget *TableAVerifier = static_cast<UpTableWidget*>(CheckBox->parent()->parent());
                        if (!VerifVerrouCompta(TableAVerifier,CheckBox->rowTable()))
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
                if (obj->parent()->parent() == ui->DetailupTableWidget  && m_mode == EnregistrePaiement)
                {
                    if (QLocale().toDouble(Line->text()) > QLocale().toDouble(m_valeurmaxi))
                    {
                        QSound::play(NOM_ALARME);
                        m_valeuravantchangement = QLocale().toString(QLocale().toDouble(m_valeurmaxi),'f',2);   // Montant payé
                    }
                    else
                        m_valeuravantchangement = QLocale().toString(QLocale().toDouble(Line->text()),'f',2);   // Montant payé
                    Line->setText(m_valeuravantchangement);
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
            Annuler();
            return true;
        }
        if (keyEvent->key()==Qt::Key_Escape)
        {
            if (m_mode == Accueil && !m_traiteparcloseflag) reject();
            ui->dateEdit->setFocus();
            if (!m_traiteparcloseflag)
                Annuler();
            m_traiteparcloseflag = false;
            return true;
        }
     }
    if (event->type() == QEvent::Close)
    {
        if (m_mode != Accueil)
        {
            event->ignore();
            Annuler();
            m_traiteparcloseflag = true;
            return true;
        }
     }
    return QWidget::eventFilter(obj, event);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Types de tiers payant dans le combobox TypeTiers --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ReconstruitListeTiers()
{
    QStringList ListTiers;

    ui->TypeTierscomboBox->clear();
    // toute la manip qui suit sert à remetre les banques par ordre aplhabétique - si vous trouvez plus simple, ne vous génez pas
    QStandardItemModel *model = new QStandardItemModel(this);
    foreach (TypeTiers* typ, *m_typestiers)
    {
        QList<QStandardItem *> items;
        items << new QStandardItem(typ->typetiers());
            model->appendRow(items);
    }
    model->sort(0);
    for(int i=0; i<model->rowCount(); i++)
        ui->TypeTierscomboBox->addItem(model->item(i)->text());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
--  Remplir la DetailsTable en fonction du mode appelé
    on a cliqué un checkbox dans les tables Liste ou SalleDAttente
    et on recopie l'enregistrement dans la table details --
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::CompleteDetailsTable(UpTableWidget *TableSource, int Rangee, bool Coche)
{
    UpTableWidget           *TableOrigine = TableSource;
    QFontMetrics            fm(qApp->font());
    QString                 requete;
    bool                    ok;

    switch (m_mode) {
    case EnregistrePaiement:            // La table est remplie par un clic dans le checkbox de sélection
    {
        QDoubleValidator        *val;
        QTableWidgetItem        *pItem1, *pItem2, *pItem3, *pItem4, *pItem5, *pItem6, *pItem7;
        pItem1 = new QTableWidgetItem() ;
        pItem2 = new QTableWidgetItem() ;
        pItem3 = new QTableWidgetItem() ;
        pItem4 = new QTableWidgetItem() ;
        pItem5 = new QTableWidgetItem() ;
        pItem6 = new QTableWidgetItem() ;
        pItem7 = new QTableWidgetItem() ;
        if (Coche)
        {
            int i = ui->DetailupTableWidget->rowCount();
            ui->DetailupTableWidget->insertRow(i);
            UpCheckBox *CheckItem = new UpCheckBox();

            pItem1->setText(TableOrigine->item(Rangee,0)->text());  //idActe
            pItem1->setData(Qt::UserRole,TableOrigine->item(Rangee,0)->data(Qt::UserRole));//UserComptable
            CheckItem->setCheckState(Qt::Checked);
            CheckItem->setFocusPolicy(Qt::NoFocus);
            connect(CheckItem,  &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);
            CheckItem->installEventFilter(this);
            pItem2->setText(TableOrigine->item(Rangee,2)->text());  //Date
            pItem3->setText(TableOrigine->item(Rangee,3)->text());  //Nom Prenom
            pItem3->setData(Qt::UserRole,TableOrigine->item(Rangee,3)->data(Qt::UserRole));//Nom
            pItem4->setText(TableOrigine->item(Rangee,4)->text());  //Cotation
            pItem5->setText(TableOrigine->item(Rangee,5)->text());  //Montant
            QString ResteDu = QLocale().toString(QLocale().toDouble(TableOrigine->item(Rangee,7)->text()),'f',2);
            pItem6->setText(ResteDu);                               //Reste dû
            pItem7->setText(TableOrigine->item(Rangee,8)->text());  //Classementpardate

            UpLineEdit *LigneMontant = new UpLineEdit();
            LigneMontant->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            val= new QDoubleValidator(this);
            val->setDecimals(2);
            LigneMontant->setValidator(val);
            LigneMontant->setText(ResteDu);
            LigneMontant->setColumn(7);
            LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                   "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
            connect (LigneMontant,  &QLineEdit::textChanged,    this,   &dlg_paiementdirect::CalculTotalDetails);
            LigneMontant->installEventFilter(this);

            pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
            pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

            ui->DetailupTableWidget->setItem(i,0,pItem1);                  //idActe
            ui->DetailupTableWidget->setCellWidget(i,1,CheckItem);         //Check
            ui->DetailupTableWidget->setItem(i,2,pItem2);                  //Date
            ui->DetailupTableWidget->setItem(i,3,pItem3);                  //Nom Prenom
            ui->DetailupTableWidget->setItem(i,4,pItem4);                  //Cotation
            ui->DetailupTableWidget->setItem(i,5,pItem5);                  //Montant
            ui->DetailupTableWidget->setItem(i,6,pItem6);                  //Reste dû
            ui->DetailupTableWidget->setCellWidget(i,7,LigneMontant);      //Payé
            ui->DetailupTableWidget->setItem(i,8,pItem7);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique

            ui->DetailupTableWidget->setRowHeight(i,int(fm.height()*1.1));

            PoseVerrouCompta(TableOrigine->item(Rangee,0)->text().toInt());
         }
        else
        {
            RetireVerrouCompta(TableOrigine->item(Rangee,0)->text().toInt());

            QList<QTableWidgetItem*> items;
            items = ui->DetailupTableWidget->findItems(TableOrigine->item(Rangee,0)->text(),Qt::MatchExactly);
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
                ui->DetailupTableWidget->removeCellWidget(items.at(0)->row(),1);
                ui->DetailupTableWidget->removeCellWidget(items.at(0)->row(),ui->DetailupTableWidget->columnCount()-2);
                ui->DetailupTableWidget->removeRow(items.at(0)->row());
                QStringList ListeActesARemettreEnDetails, ListeMontantsARemettreEnDetails, ListeIdComptables;
                QStringList ListeDates, ListeNoms, ListeCotations;
                QStringList ListeMontants, ListeResteDu, ListeDatesTri;

                for (int k = 0; k < ui->DetailupTableWidget->rowCount();k++)
                {
                    ListeActesARemettreEnDetails    << ui->DetailupTableWidget->item(k,0)->text();
                    ListeIdComptables               << ui->DetailupTableWidget->item(k,0)->data(Qt::UserRole).toString();
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
                    pItem1->setText(ListeActesARemettreEnDetails.at(l));        //idActe
                    pItem1->setData(Qt::UserRole, ListeIdComptables.at(l));     //idComptableActes
                    CheckItem->setCheckState(Qt::Checked);
                    CheckItem->setFocusPolicy(Qt::NoFocus);
                    connect(CheckItem,  &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);
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
                    LigneMontant->setColumn(7);
                    LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                           "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                    connect (LigneMontant,  &QLineEdit::textChanged,    this,   &dlg_paiementdirect::CalculTotalDetails);
                    LigneMontant->installEventFilter(this);

                    ui->DetailupTableWidget->setItem(l,0,pItem1);                  //idActe
                    ui->DetailupTableWidget->setCellWidget(l,1,CheckItem);         //Check
                    ui->DetailupTableWidget->setItem(l,2,pItem2);                  //Date
                    ui->DetailupTableWidget->setItem(l,3,pItem3);                  //Nom Prenom
                    ui->DetailupTableWidget->setItem(l,4,pItem4);                  //Cotation
                    ui->DetailupTableWidget->setItem(l,5,pItem5);                  //Montant
                    ui->DetailupTableWidget->setItem(l,6,pItem6);                  //Reste dû
                    ui->DetailupTableWidget->setCellWidget(l,7,LigneMontant);      //Payé
                    ui->DetailupTableWidget->setItem(l,8,pItem7);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique

                    ui->DetailupTableWidget->setRowHeight(l,int(fm.height()*1.1));
                }
            }
        }
        TrieListe(ui->DetailupTableWidget);

        // si DetailsupTableWidget n'est pas vide, on filtre les 2 autres tables pour n'afficher que les enregistrements avec le même comptable
        if (ui->DetailupTableWidget->rowCount() == 0)
        {
            // si la liste des actes n'était pas vide au départ, il est possible que le comptable du user connecte soit différent du comptable des actes de la liste
            if (currentuser()->idcomptableactes() > 0 && currentuser()->idcomptableactes() != m_useracrediter->id())
            {
                m_useracrediter = Q_NULLPTR;
                bool ok = true;
                RemplitLesTables(ok);
            }
            m_useracrediter   = (map_comptables->size() == 1? map_comptables->cbegin().value() : Datas::I()->users->getById(currentuser()->idcomptableactes()));     // -2 si le user est une secrétaire et qu'il n'y a pas de comptable
            if( m_useracrediter == Q_NULLPTR)
            {
                if (currentuser()->isSecretaire())
                    m_useracrediter = map_comptables->cbegin().value();
            }
            ui->TireurChequelineEdit->setText("");
        }
        else
        {
            m_useracrediter = Datas::I()->users->getById(ui->DetailupTableWidget->item(0,0)->data(Qt::UserRole).toInt());
            if (ui->ChequeradioButton->isChecked() && ui->TireurChequelineEdit->text() == "")
                ui->TireurChequelineEdit->setText(ui->DetailupTableWidget->item(0,3)->data(Qt::UserRole).toStringList().at(3));
        }

        ChangeComptable(m_useracrediter);

        ui->ComptablescomboBox->setEnabled(currentuser()->isSecretaire() && (m_mode == EnregistrePaiement && ui->DetailupTableWidget->rowCount()==0));
        ui->ComptablescomboBox          ->setEnabled(Datas::I()->users->getById(currentuser()->idcomptableactes()) == Q_NULLPTR
                                                     && ui->DetailupTableWidget->rowCount()==0
                                                     && map_comptables->size()>1);
        break;
    }
    case VoirListeActes:                // La table est remplie par la sélection d'une ligne dans listuptablewidget ou salleDAttenteupTablewidget
    {
        ui->DetailupTableWidget->clearContents();
        ui->DetailupTableWidget->setRowCount(0);
        if (TableOrigine == ui->ListeupTableWidget)
        {
            // Remplir la table SalDat avec les Paiements correspondants à l'acte mis en surbrillance dans la table liste
            QString TextidActe   = TableOrigine->item(Rangee,0)->text();
            requete =     "SELECT "
                            "rec." CP_ID_LIGNRECETTES ","
                            CP_DATE_LIGNRECETTES ", "
                            CP_DATEENREGISTREMENT_LIGNRECETTES ", "
                            "rec." CP_MONTANT_LIGNRECETTES ", "
                            CP_MODEPAIEMENT_LIGNRECETTES ","
                            CP_TIREURCHEQUE_LIGNRECETTES ", "
                            CP_IDCPTEVIREMENT_LIGNRECETTES ", "
                            CP_BANQUECHEQUE_LIGNRECETTES ", "
                            CP_TIERSPAYANT_LIGNRECETTES ", "
                            CP_NOMPAYEUR_LIGNRECETTES ", "
                            CP_COMMISSION_LIGNRECETTES ","
                            CP_MONNAIE_LIGNRECETTES ", "
                            CP_IDREMISECHQ_LIGNRECETTES ", "
                            CP_CHQENATTENTE_LIGNRECETTES ", "
                            CP_IDUSERENREGISTREUR_LIGNRECETTES ", "
                            CP_TYPERECETTE_LIGNRECETTES ", "
                            CP_DATE_REMCHEQ ", "
                            CP_PAYE_LIGNEPAIEMENT
                          " FROM " TBL_LIGNESPAIEMENTS " lig, " TBL_RECETTES " rec\n"
                          " LEFT OUTER JOIN " TBL_REMISECHEQUES " AS rc ON rc." CP_ID_REMCHEQ " = rec." CP_IDREMISECHQ_LIGNRECETTES "\n"
                          " WHERE lig." CP_IDRECETTE_LIGNEPAIEMENT " = rec." CP_ID_LIGNRECETTES "\n"
                          " AND lig." CP_IDACTE_LIGNEPAIEMENT " = " + TextidActe + "\n"
                          " ORDER BY " CP_DATE_LIGNRECETTES " DESC, " CP_NOMPAYEUR_LIGNRECETTES;
            QList<QVariantList> reclist = db->StandardSelectSQL(requete, ok);
            RemplirTableWidget(ui->SalleDAttenteupTableWidget,"Paiements",reclist,false,Qt::Unchecked);
            if (reclist.size() == 0)
            {
                ui->PasdePaiementlabel->setVisible(true);
                ui->PaiementgroupBox->setVisible(false);
                ui->ModifierupPushButton->setEnabled(true);
                break;
            }
            else
            {
                ui->PasdePaiementlabel->setVisible(false);
                ui->TypePaiementframe->setVisible(true);
                ui->PaiementgroupBox->setVisible(true);
                connect (ui->SalleDAttenteupTableWidget, &QTableWidget::itemSelectionChanged, this, [=] {RenvoieRangee();});
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
                ui->dateEdit->setDate(m_currentdate);
                ui->TotallineEdit->setText("0,00");

                if (reclist.size() != 1)
                {
                    ui->ModifierupPushButton->setEnabled(false);
                    break;
                }
                else
                {
                    Rangee = 0;
                    TableOrigine = ui->SalleDAttenteupTableWidget;
                    disconnect (ui->SalleDAttenteupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
                    TableOrigine->selectRow(0);
                    connect (ui->SalleDAttenteupTableWidget,    &QTableWidget::itemSelectionChanged, this, [=] {RenvoieRangee();});
                }
            }
         }
        // la table d'origine est SalleDAttentetetableWidget ou (c'est ListeupTableWidget et il n'y a qu'un seul enregistrement dans SalleDAttenteupTableWidget)
        // on détermine si le paiement cliqué est un tiers payant ou un paiement direct
        QString TextidRecette   = TableOrigine->item(Rangee,0)->text();
        requete =   "SELECT " CP_TIERSPAYANT_LIGNRECETTES " FROM " TBL_RECETTES " WHERE " CP_ID_LIGNRECETTES " = " + TextidRecette;
        QVariantList tiersdata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
        if (!ok)
            return;
        if (tiersdata.size() > 0)
        {
            if (tiersdata.at(0).toString() == "O")
            {
                DefinitArchitectureTable(ui->DetailupTableWidget,ActesTiers);
                ui->ModifierupPushButton->setEnabled(false);
                m_typetable = ActesTiers;
            }
            else
            {
                DefinitArchitectureTable(ui->DetailupTableWidget,ActesDirects);
                ui->ModifierupPushButton->setEnabled(true);
                m_typetable = ActesDirects;
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

            requete =   "SELECT act." CP_ID_ACTES ", " CP_DATE_ACTES ", " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_COTATION_ACTES ", "
                        CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", " CP_PAYE_LIGNEPAIEMENT ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_TIERS_TYPEPAIEMENTACTES ", "
                        "TotalPaye\n"
                        " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_LIGNESPAIEMENTS " lig, " TBL_TYPEPAIEMENTACTES " typ,\n"
                        " (SELECT lig." CP_IDACTE_LIGNEPAIEMENT ", SUM(lig." CP_PAYE_LIGNEPAIEMENT ") as TotalPaye FROM " TBL_LIGNESPAIEMENTS " lig,\n"
                        " (SELECT " CP_IDACTE_LIGNEPAIEMENT " FROM " TBL_LIGNESPAIEMENTS
                        " WHERE " CP_IDRECETTE_LIGNEPAIEMENT " = " + TextidRecette + ") AS Result\n"
                        " WHERE lig." CP_IDACTE_LIGNEPAIEMENT " = Result." CP_IDACTE_LIGNEPAIEMENT " GROUP BY lig." CP_IDACTE_LIGNEPAIEMENT ") AS calc\n"
                        " WHERE act." CP_ID_ACTES " = lig." CP_IDACTE_LIGNEPAIEMENT "\n"
                        " AND typ." CP_IDACTE_TYPEPAIEMENTACTES " = act." CP_ID_ACTES "\n"
                        " AND calc." CP_IDACTE_LIGNEPAIEMENT " = act." CP_ID_ACTES "\n"
                        " AND lig." CP_IDRECETTE_LIGNEPAIEMENT " = " + TextidRecette + "\n"
                        " AND act." CP_IDPAT_ACTES " = pat." CP_IDPAT_PATIENTS "\n"
                        " ORDER BY " CP_DATE_ACTES " DESC, " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS;

            //UpMessageBox::Watch(this,requete);
            QList<QVariantList> pmtlist = db->StandardSelectSQL(requete, ok);

            RemplirTableWidget(ui->DetailupTableWidget,"Actes",pmtlist,false,Qt::Unchecked);

            // Remplir les infos sur la recette concernée
            requete =   "SELECT "
                        CP_ID_LIGNRECETTES ", "
                        CP_IDUSER_LIGNRECETTES ", "
                        CP_DATE_LIGNRECETTES ", "
                        CP_DATEENREGISTREMENT_LIGNRECETTES ", "
                        CP_MONTANT_LIGNRECETTES ", "
                        CP_MODEPAIEMENT_LIGNRECETTES ","
                        CP_TIREURCHEQUE_LIGNRECETTES ", "
                        CP_IDCPTEVIREMENT_LIGNRECETTES ", "
                        CP_BANQUECHEQUE_LIGNRECETTES ", "
                        CP_TIERSPAYANT_LIGNRECETTES ", "
                        CP_NOMPAYEUR_LIGNRECETTES ", "
                        CP_COMMISSION_LIGNRECETTES ","
                        CP_MONNAIE_LIGNRECETTES ", "
                        CP_IDREMISECHQ_LIGNRECETTES ", "
                        CP_CHQENATTENTE_LIGNRECETTES ", "
                        CP_IDUSERENREGISTREUR_LIGNRECETTES ", "
                        CP_TYPERECETTE_LIGNRECETTES
                        " FROM " TBL_RECETTES
                        " WHERE " CP_ID_LIGNRECETTES " = " + TextidRecette;
            QVariantList recdata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
            if (!ok || recdata.size()==0)
                return;
            ui->dateEdit->setDate(recdata.at(2).toDate());
            QRadioButton *RadioAClicker = Q_NULLPTR;
            QString mp = recdata.at(5).toString();
            if (mp == VRMT)
            {
                if (recdata.at(9).toString() ==  "O" && recdata.at(10).toString() == "CB" && m_mode == VoirListeActes)
                    RadioAClicker = ui->CarteCreditradioButton;
                else
                    RadioAClicker = ui->VirementradioButton;
                QString Commission = QLocale().toString(recdata.at(11).toDouble(),'f',2);
                ui->CommissionlineEdit->setText(Commission);
            }
            if (mp == ESP) RadioAClicker = ui->EspecesradioButton;
            if (mp == CHQ) RadioAClicker = ui->ChequeradioButton;
            if (RadioAClicker != Q_NULLPTR)
                RadioAClicker->setChecked(true);
            ui->ComptesupComboBox->clearEditText();
            if (recdata.at(7).toString() != "0")
            {
                ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(recdata.at(7)));
                //qDebug() << recdata.at(7).toString() + " - " + ui->ComptesupComboBox->currentData().toString() + " - " + ui->ComptesupComboBox->currentText();
            }
            ui->TierscomboBox->setCurrentText(recdata.at(10).toString());
            if (mp == CHQ)
            {
                ui->EnAttentecheckBox->setChecked(recdata.at(14).toString() == "1");
                ui->TireurChequelineEdit->setText(recdata.at(6).toString());
                ui->BanqueChequecomboBox->setCurrentText(recdata.at(8).toString());
            }
            QString Montant = QLocale().toString(recdata.at(4).toDouble(),'f',2);
            ui->MontantlineEdit->setText(Montant);
            break;
        }
        break;
    }
    default:
        break;
    } // fin switch

    CalculTotalDetails();
    RegleAffichageTypePaiementframe(false);
    RegleAffichageTypePaiementframe(true,false);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Définit l'architecture des TableView (SelectionMode, nombre de colonnes, avec Widgets UpcheckBox et UplineBox) ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::DefinitArchitectureTable(UpTableWidget *TableARemplir, TypeTable typetable)
{
    QStringList         LabelARemplir;
    int                 ColCount = 0;

    // il faut deconnecter la table du signal itemSelectionChanged(), sinon, l'appel de TableARemplir->clear()
    // provoque un plantage. La table est vidée mais le slot est quand même appelé par le signal
    if (TableARemplir == ui->SalleDAttenteupTableWidget && m_mode == VoirListeActes)
        disconnect (ui->SalleDAttenteupTableWidget,    &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
    TableARemplir->clear();
    TableARemplir->verticalHeader()->hide();

    // ces 2 lignes sont nécessairs pour éviter un pb d'affichage des tables quand on les redéfinit
    TableARemplir->setRowCount(0);
    TableARemplir->setColumnCount(0);

    switch (typetable) {
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
        TableARemplir->setColumnWidth(li,95);                                               // Date
        li++;
        TableARemplir->setColumnWidth(li,200);                                              // Payeur
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
            if (m_mode != EnregistrePaiement && !(m_mode == VoirListeActes && typetable == ActesDirects))  ColCount = 10;
            TableARemplir->setColumnCount(ColCount);
            TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);

            LabelARemplir << "";
            LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
            if (m_mode != EnregistrePaiement && !(m_mode == VoirListeActes && typetable == ActesDirects))
            {
                if (typetable == ActesTiers)
                    LabelARemplir << tr("Type tiers");
                else
                    LabelARemplir << tr("Mode paiement");
            }
            LabelARemplir << tr("Reste dû");
            if (m_mode == EnregistrePaiement)
                LabelARemplir << tr("A payer");
            else
                LabelARemplir << tr("Payé");
            LabelARemplir << "Classementpardate";
            TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
            TableARemplir->horizontalHeader()->setVisible(true);
//----------------------------------------------------------------------------------------------------------// Réglage de la largeur et du nombre des colonnes
            int li = 0;
            TableARemplir->setColumnWidth(li,25);                                               // 0 - idActe ou idPaiement
            li++;
            if (m_mode == EnregistrePaiement)
                TableARemplir->setColumnWidth(li,20);                                           // 1 = Checkbox
            else
                TableARemplir->setColumnWidth(li,0);
            li++;
            TableARemplir->setColumnWidth(li,95);                                               // 2 - Date
            li++;
            TableARemplir->setColumnWidth(li,200);                                              // 3 - Nom Prenom ou Payeur
            li++;
            TableARemplir->setColumnWidth(li,170);                                              // 4 - Cotation
            li++;
            switch (m_mode) {
            case VoirListeActes:
            {
                TableARemplir->setColumnWidth(li,75);                                               // 5 - Montant
                li++;
                if (typetable == ActesTiers)                                                        // 6 - Type tiers
                {
                    TableARemplir->setColumnWidth(li,120);                                          // 7 -Type tiers
                    li ++;
                }
                TableARemplir->setColumnWidth(li,75);                                               // 7 ou 8 - Reste dû
                break;
            }
            default:
            {
                TableARemplir->setColumnWidth(li,75);                                               // 5 - Montant
                li++;
                if (typetable == ActesTiers)                                                        // 6 -Type tiers
                {
                    TableARemplir->setColumnWidth(li,100);
                    li ++;
                }
                if (m_mode == VoirListeActes && typetable == ActesDirects)                           // Mode de paiement
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
            TableARemplir->setColumnWidth(li,25);                                               // 0 - idActe ou idPaiement
            li++;
            TableARemplir->setColumnWidth(li,20);                                               // 1 - Checkbox
            li++;
            TableARemplir->setColumnWidth(li,95);                                               // 2 - Date
            li++;
            TableARemplir->setColumnWidth(li,200);                                              // 3 - Nom Prenom ou Payeur
            li++;
            TableARemplir->setColumnWidth(li,170);                                              // 4 - Cotation
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // 5 - Montant
            TableARemplir->setColumnWidth(li,75);                                               // 6 - Payé
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // 7 - Reste dû
            li++;
            TableARemplir->setColumnWidth(li,75);                                               // 8 - ActeDate
        }

        if (TableARemplir == ui->ListeupTableWidget)
        {
            ColCount = 8;
            if (m_mode != EnregistrePaiement)  ColCount = 9;
            if (m_mode == EnregistrePaiement)
                ColCount ++;
            TableARemplir->setColumnCount(ColCount);
            if (m_mode == EnregistrePaiement)
                TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);
            else
                TableARemplir->setSelectionMode(QAbstractItemView::SingleSelection);

            LabelARemplir << "";
            if (m_mode == EnregistrePaiement)
                LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
            if (m_mode == VoirListeActes)
                LabelARemplir << tr("Mode paiement");
            if (m_mode == EnregistrePaiement)
                LabelARemplir << tr(IMPAYE);
            else
                LabelARemplir << tr("Payé");
            LabelARemplir << tr("Reste dû");
            LabelARemplir << "Classementpardate";
            TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
            TableARemplir->horizontalHeader()->setVisible(true);
            int li = 0;                                                                         // Réglage de la largeur et du nombre des colonnes
            TableARemplir->setColumnWidth(li,25);                                               // idActe ou idPaiement
            li++;
            if (m_mode == EnregistrePaiement)
            {
                TableARemplir->setColumnWidth(li,20);                                           // Checkbox
                li++;
                TableARemplir->setColumnWidth(li,95);                                           // Date
            }
            else
                TableARemplir->setColumnWidth(li,95);
            li++;
            TableARemplir->setColumnWidth(li,200);                                              // Nom Prenom ou Payeur
            li++;
            TableARemplir->setColumnWidth(li,170);                                              // Cotation
            li++;
            if (m_mode == VoirListeActes)
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                if ((typetable == ActesTiers)                                                       // Mode de paiement ou Type tiers
                        || (m_mode == VoirListeActes && typetable == ActesDirects))
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
                if (typetable == ActesTiers)                                                        // Type tiers
                {
                    TableARemplir->setColumnWidth(li,100);
                    li ++;
                }
                if (m_mode == VoirListeActes && typetable == ActesDirects)                           // Mode de paiement
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
    // toute la manip qui suit sert à remettre les banques par ordre alphabétique - si vous trouvez plus simple, ne vous génez pas
    QStandardItemModel *model = new QStandardItemModel(this);
    for (auto it = map_banques->constBegin(); it != map_banques->constEnd(); ++it)
    {
        Banque* bq = const_cast<Banque*>(it.value());
        QList<QStandardItem *> items;
        items << new QStandardItem(bq->nomabrege()) << new QStandardItem(QString::number(bq->id()));
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
    QString verrourequete = "select idActe from " TBL_VERROUCOMPTAACTES " where idActe = " + QString::number(ActeAVerrouiller);
    //qDebug() << verrourequete;
    QVariantList actdata = db->getFirstRecordFromStandardSelectSQL(verrourequete, m_ok);
    if (!m_ok)
        return;
    if (actdata.size() == 0)
    {
        QString VerrouilleEnreg= "INSERT INTO " TBL_VERROUCOMPTAACTES
                " (idActe,DateTimeVerrou, PosePar)"
                " VALUES ("  + QString::number(ActeAVerrouiller) +
                ", NOW() ,"  + QString::number(currentuser()->id()) + ")";
        db->StandardSQL(VerrouilleEnreg);
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

    if (VerifierEmetteur)  // c'est le click sur un des radiobutton qui a appelé la méthode
    {
        if (Emetteur == ui->CarteCreditradioButton)
        {
            ui->TypeTierswidget     ->setVisible(false);
            ui->AttenteChequewidget ->setVisible(false);
            ui->TireurChequewidget  ->setVisible(false);
            ui->Banquewidget        ->setVisible(false);
            ui->Tierswidget         ->setVisible(m_mode == VoirListeActes);
            ui->Montantwidget       ->setVisible(m_mode == VoirListeActes);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->CompteCreditewidget ->setVisible(m_mode == VoirListeActes);
            ui->Commissionwidget    ->setVisible(m_mode == VoirListeActes);
            if (m_mode == VoirListeActes)
                ui->ComptesupComboBox       ->setCurrentIndex(ui->ComptesupComboBox->findData(m_useracrediter->idcomptepardefaut()));
            else if (m_mode == EnregistrePaiement && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setReadOnly(false);
                    Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                    CalculTotalDetails();
                }
            }
        }
        else if (Emetteur == ui->ChequeradioButton)
        {
            ui->TireurChequewidget  ->setVisible(true);
            ui->Banquewidget        ->setVisible(true);
            ui->TypeTierswidget     ->setVisible(false);
            ui->AttenteChequewidget ->setVisible(true);
            ui->Montantwidget       ->setVisible(true);
            ui->Montantwidget       ->setEnabled(true);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            // Si la Table Details n'est pas vide , on récupère le nom du premier de la liste pour remplir la case Tireur du chèque;
            if (m_mode == EnregistrePaiement && AppeleParClicK)
                if (ui->DetailupTableWidget->rowCount() > 0)
                {
                    QString req = "SELECT " CP_NOM_PATIENTS " FROM " TBL_PATIENTS " pat, " TBL_ACTES " act"
                                  " WHERE pat." CP_IDPAT_PATIENTS " = act." CP_IDPAT_ACTES " and " CP_ID_ACTES " = " + ui->DetailupTableWidget->item(0,0)->text();
                    QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
                    if (m_ok && patdata.size()>0)
                        ui->TireurChequelineEdit->setText(patdata.at(0).toString());
                }
            if (m_mode == VoirListeActes)
                if (ui->DetailupTableWidget->rowCount() > 0)
                    ui->Tierswidget         ->setVisible(m_typetable == ActesTiers);

            if (m_mode == EnregistrePaiement && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setReadOnly(false);
                    Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                    CalculTotalDetails();
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
            ui->Montantwidget       ->setVisible(false);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            if (m_mode == EnregistrePaiement && AppeleParClicK)
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
            ui->Montantwidget       ->setVisible(true);
            ui->Montantwidget       ->setEnabled(true);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            if (m_mode == EnregistrePaiement && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setReadOnly(false);
                    Paye->setText(ui->DetailupTableWidget->item(i,ui->DetailupTableWidget->columnCount()-3)->text());
                    CalculTotalDetails();
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
            ui->Montantwidget       ->setVisible(false);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            if (m_mode == EnregistrePaiement && AppeleParClicK)
            {
                for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
                {
                    QLineEdit* Paye = static_cast<QLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ui->DetailupTableWidget->columnCount()-2));
                    Paye->setText("0,00");
                    Paye->setReadOnly(true);
                    CalculTotalDetails();
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
            ui->Montantwidget       ->setVisible(true);
            ui->Montantwidget       ->setEnabled(false);
            ui->CompteCreditewidget ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            ui->Commissionwidget    ->setVisible(false);
            if (m_mode == EnregistrePaiement&& AppeleParClicK)
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
            ui->Montantwidget       ->setVisible(true);
            ui->Montantwidget       ->setEnabled(true);
            ui->CompteCreditewidget ->setVisible(true);
            ui->Commissionwidget    ->setVisible(m_mode!= EnregistrePaiement);
        }
        ui->OKupPushButton->setEnabled(true);
    }
    else  // aucun RadioButton n'est coché, c'est le programme qui a appelé la méthode
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

        if (m_mode == Accueil)
        {
            ui->TypePaiementframe->setVisible(false);
            ui->MontantlineEdit->setText("0,00");
            ui->TierscomboBox->clearEditText();
            ui->BanqueChequecomboBox->clearEditText();
            ui->TireurChequelineEdit->clear();
            ui->EnAttentecheckBox->setCheckState(Qt::Unchecked);
            ui->ComptesupComboBox->clearEditText();
            ui->CommissionlineEdit->clear();
            ui->dateEdit->setDate(m_currentdate);
            ui->TotallineEdit->setText("0,00");
        }
        else
        {
            ui->TypePaiementframe->setVisible(true);
            ui->PaiementgroupBox->setVisible(true);
            ui->MontantlineEdit->setVisible(true);
            ui->Montantlabel->setVisible(true);
            ui->dateEdit->setVisible(true);
            switch (m_mode) {
            case EnregistrePaiement:
            {
                ui->CarteCreditradioButton->setVisible(true);
                ui->ChequeradioButton->setVisible(true);
                ui->EspecesradioButton->setVisible(true);
                ui->TiersradioButton->setVisible(true);
                ui->ImpayeradioButton->setVisible(true);
                ui->GratuitradioButton->setVisible(true);
                ui->VirementradioButton->setVisible(false);
                ui->PaiementgroupBox->setFocusProxy(ui->CarteCreditradioButton);
                break;
            }
            case VoirListeActes:
            {
                ui->CarteCreditradioButton->setVisible(true);
                ui->ChequeradioButton->setVisible(true);
                ui->EspecesradioButton->setVisible(true);
                ui->TiersradioButton->setVisible(false);
                ui->ImpayeradioButton->setVisible(false);
                ui->GratuitradioButton->setVisible(false);
                ui->VirementradioButton->setVisible(true);
                break;
            }
            default:
                break;
            }
        }
    }

    ui->TypePaiementframe   ->setEnabled(m_mode == EnregistrePaiement);
    ui->PaiementgroupBox    ->setVisible(!(m_mode == Accueil || m_mode == VoirListeActes));
    switch (m_mode) {
    case EnregistrePaiement:
    {
        ui->OKupPushButton      ->setEnabled(QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || (Emetteur !=Q_NULLPTR) || ui->DetailupTableWidget->rowCount() > 0);
        ui->PaiementgroupBox    ->setEnabled(m_modiflignerecettepossible);
        ui->MontantlineEdit     ->setEnabled(m_modiflignerecettepossible);
        ui->Montantlabel        ->setEnabled(m_modiflignerecettepossible);
        ui->Commissionwidget    ->setEnabled(m_modiflignerecettepossible);
        ui->EnAttentecheckBox   ->setEnabled(m_modiflignerecettepossible);
        ui->EnAttentelabel      ->setEnabled(m_modiflignerecettepossible);
        ui->dateEdit            ->setEnabled(m_modiflignerecettepossible);
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
    }
    if (!(ui->VirementradioButton->isChecked() || ui->CarteCreditradioButton->isChecked()))
        ui->CommissionlineEdit->setText("0,00");
}

void dlg_paiementdirect::RegleComptesComboBox(bool avecLesComptesInactifs)
{
    ui->ComptesupComboBox->clear();
    foreach (int idcpt, m_useracrediter->listecomptesbancaires(avecLesComptesInactifs))
    {
        Compte *cpt = Datas::I()->comptes->getById(idcpt);
        if (cpt != Q_NULLPTR)
            ui->ComptesupComboBox->addItem(m_useracrediter->login() + "/" + cpt->nomabrege(), cpt->id());
    }
    ui->ComptesupComboBox       ->setCurrentIndex(ui->ComptesupComboBox->findData(m_useracrediter->idcomptepardefaut()));
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les TableListe et SalDat en fonction du mode appelé ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RemplitLesTables(bool &ok)
{
    QString             requete;
    disconnect (ui->SalleDAttenteupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
    disconnect (ui->SalleDAttenteupTableWidget, &QTableWidget::itemEntered, this, Q_NULLPTR);
    disconnect (ui->ListeupTableWidget,         &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
    disconnect (ui->ListeupTableWidget,         &QTableWidget::itemEntered, this, Q_NULLPTR);

    switch (m_mode)
    {

    /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Enregistrement d'un paiement direct -----------------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    case EnregistrePaiement:
        {
            /*
        1. Remplissage ui->ListeupTableWidget -- tous les paiements en attente en dehors de ceux de la salle d'attente
        càd ceux:
        . pour lesquels (aucune ligne de paiement n'a été enregistrée
        . OU le type de paiement enregistré est IMPAYE)
        . ET qui ne sont pas en salle d'attente en attente de paiement
        . ET dont le montant de l'acte n'est pas nul
        */
        DefinitArchitectureTable(ui->ListeupTableWidget, ActesDirects);

        requete =
                    "SELECT act." CP_ID_ACTES ", " CP_DATE_ACTES ", " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_COTATION_ACTES ","                                                    // 0, 1, 2, 3, 4
                    CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", '' as " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_MONTANT_ACTES " as ResteDu , SUM(" CP_PAYE_LIGNEPAIEMENT ") as Regle, "     // 5, 6, 7, 8, 9
                    CP_DDN_PATIENTS ", " CP_IDUSERCOMPTABLE_ACTES ", " CP_IDUSER_ACTES                                                                                                      // 10, 11, 12
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_LIGNESPAIEMENTS " lig\n"
                    " WHERE "
                    " act." CP_ID_ACTES " IN (SELECT " CP_IDACTE_LIGNEPAIEMENT " from (\n"
                    " SELECT  lig." CP_IDACTE_LIGNEPAIEMENT ", SUM(" CP_PAYE_LIGNEPAIEMENT ") as TotalPaye, " CP_MONTANT_ACTES "  FROM " TBL_LIGNESPAIEMENTS " as lig, " TBL_ACTES " as act2\n"
                    " where lig." CP_IDACTE_LIGNEPAIEMENT " = act2." CP_ID_ACTES "\n"
                    " group by " CP_IDACTE_LIGNEPAIEMENT ") as blue\n"
                    " where blue." CP_MONTANT_ACTES " > blue.totalpaye and blue.totalpaye > 0)\n"
                    " AND act." CP_ID_ACTES " NOT IN (SELECT saldat." CP_IDACTEAPAYER_SALDAT " FROM " TBL_SALLEDATTENTE " saldat where saldat." CP_IDACTEAPAYER_SALDAT " is not null)\n"
                    " AND act." CP_ID_ACTES " NOT IN (SELECT Typ." CP_IDACTE_TYPEPAIEMENTACTES " FROM " TBL_TYPEPAIEMENTACTES " Typ where Typ." CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " = 'T')\n"
                    " AND act." CP_IDPAT_ACTES " = pat." CP_IDPAT_PATIENTS "\n"
                    " AND act." CP_ID_ACTES " = lig.idActe\n"
                    " AND " CP_DATE_ACTES " > AddDate(NOW(),-730)\n";
        requete +=  " AND " CP_MONTANT_ACTES " > 0\n"
                    " GROUP BY act." CP_ID_ACTES "\n";                        // tous les actes dont le paiement est incomplet

        requete +=  " UNION \n\n"
                    " SELECT act." CP_ID_ACTES ", " CP_DATE_ACTES ", " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_COTATION_ACTES ","                       // 0, 1, 2, 3, 4
                    CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", '' as " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_MONTANT_ACTES " as ResteDu , 0 as Regle, "      // 5, 6, 7, 8, 9
                    CP_DDN_PATIENTS ", " CP_IDUSERCOMPTABLE_ACTES ", " CP_IDUSER_ACTES                                                                          // 10, 11, 12
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat"
                    " WHERE "
                    " act." CP_ID_ACTES " not in (select " CP_IDACTE_LIGNEPAIEMENT " from " TBL_LIGNESPAIEMENTS ")\n"
                    " AND act." CP_ID_ACTES " not in (select " CP_IDACTE_TYPEPAIEMENTACTES " from " TBL_TYPEPAIEMENTACTES " WHERE " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " = 'I' OR " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " = 'T')\n"
                    " AND act." CP_ID_ACTES " NOT IN (SELECT saldat." CP_IDACTEAPAYER_SALDAT " FROM " TBL_SALLEDATTENTE " saldat where saldat." CP_IDACTEAPAYER_SALDAT " is not null)\n"
                    " AND act." CP_IDPAT_ACTES " = pat." CP_IDPAT_PATIENTS "\n"
                    " AND " CP_DATE_ACTES " > AddDate(NOW(),-730)\n";
        requete +=  " AND " CP_MONTANT_ACTES " > 0\n"
                    " GROUP BY act." CP_ID_ACTES "\n";                        // tous les actes pour lesquels aucun renseignement de paiement n'a été enregistré et qui ne sont pas en salle d'attente

        requete +=  " UNION \n\n"
                    " SELECT act." CP_ID_ACTES ", " CP_DATE_ACTES ", " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_COTATION_ACTES ","               // 0, 1, 2, 3, 4
                    CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_MONTANT_ACTES " as ResteDu, 0 as Regle, "     // 5, 6, 7, 8, 9
                    CP_DDN_PATIENTS ", " CP_IDUSERCOMPTABLE_ACTES ", " CP_IDUSER_ACTES                                                                  // 10, 11, 12
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ\n"
                    " WHERE typ." CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " = 'I'\n"
                    " AND typ." CP_IDACTE_TYPEPAIEMENTACTES " = act." CP_ID_ACTES "\n"
                    " AND act." CP_ID_ACTES " NOT IN (SELECT saldat." CP_IDACTEAPAYER_SALDAT " FROM " TBL_SALLEDATTENTE " saldat where saldat." CP_IDACTEAPAYER_SALDAT " is not null)\n"
                    " AND act." CP_IDPAT_ACTES " = pat." CP_IDPAT_PATIENTS "\n"
                    " AND " CP_DATE_ACTES " > AddDate(NOW(),-730)\n";
        requete +=  " ORDER BY " CP_DATE_ACTES " DESC, " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS;   // tous les actes impayés

        //proc->Edit(requete);
        bool ok;
        QList<QVariantList> actlist = db->StandardSelectSQL(requete, ok);
        RemplirTableWidget(ui->ListeupTableWidget, "Actes", actlist, true, Qt::Unchecked);
        if (ui->ListeupTableWidget->rowCount() > 0)
            connect (ui->ListeupTableWidget,    &QTableWidget::itemEntered, this,   [=] (QTableWidgetItem *item) {AfficheToolTip(item);});

        //2. Remplissage ui->SalleDAttenteupTableWidget
        DefinitArchitectureTable(ui->SalleDAttenteupTableWidget,ActesDirects);
        requete =   "SELECT " CP_ID_ACTES ", " CP_DATE_ACTES ", " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS ", " CP_COTATION_ACTES ","            // 0, 1, 2, 3, 4
                    CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", " CP_MONTANT_ACTES " as CalculPaiement, " CP_IDUSERCOMPTABLE_ACTES ","            // 5, 6, 7, 8, 9
                    CP_IDUSER_ACTES                                                                                                             // 10
                    " FROM \n" TBL_ACTES " act, " TBL_PATIENTS " pat \n"
                    " WHERE " CP_ID_ACTES " IN (SELECT saldat." CP_IDACTEAPAYER_SALDAT " FROM " TBL_SALLEDATTENTE " saldat) \n"
                    " AND act." CP_IDPAT_ACTES " = pat." CP_IDPAT_PATIENTS " \n";
        requete +=  " AND " CP_MONTANT_ACTES " > 0 \n"
                    " ORDER BY " CP_DATE_ACTES " DESC, " CP_NOM_PATIENTS ", " CP_PRENOM_PATIENTS;
        //proc->Edit(requete);
        actlist = db->StandardSelectSQL(requete, ok);
        RemplirTableWidget(ui->SalleDAttenteupTableWidget,"Actes", actlist, true, Qt::Unchecked);

        //3. Remplissage ui->DetailupTableWidget
        DefinitArchitectureTable(ui->DetailupTableWidget,ActesDirects);
        if (m_listidactes.size() > 0)
        {
            for (int i = 0; i < ui->SalleDAttenteupTableWidget->rowCount();i++)
            {
                UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->SalleDAttenteupTableWidget->cellWidget(i,1));
                if(Check)
                {
                    if (Check->checkState() == Qt::Checked)
                    {
                        int R = Check->rowTable();
                        CompleteDetailsTable(ui->SalleDAttenteupTableWidget,R,Check->checkState());
                        PoseVerrouCompta(ui->SalleDAttenteupTableWidget->item(i,0)->text().toInt());
                    }
                }
            }
            for (int i = 0; i < ui->ListeupTableWidget->rowCount();i++)
            {
                UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->ListeupTableWidget->cellWidget(i,1));
                if(Check)
                {
                    if (Check->checkState()==Qt::Checked)
                    {
                        int R = Check->rowTable();
                        CompleteDetailsTable(ui->ListeupTableWidget,R,Check->checkState());
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
        requete =   "select * from (\n"
                    "SELECT act." CP_ID_ACTES ", " CP_DATE_ACTES ", PatNom, PatPrenom, " CP_COTATION_ACTES ","                                                                  // 0, 1, 2, 3, 4
                    CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", SUM(" CP_PAYE_LIGNEPAIEMENT ") as tot, " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_TIERS_TYPEPAIEMENTACTES ","    // 5, 6, 7, 8, 9
                    " PatDDN, " CP_IDUSERCOMPTABLE_ACTES ", " CP_IDUSER_ACTES "\n"                                                                                              // 10, 11, 12
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ, " TBL_LIGNESPAIEMENTS " lig\n"
                    " WHERE act." CP_ID_ACTES " = typ." CP_IDACTE_TYPEPAIEMENTACTES "\n"
                    " AND lig." CP_IDACTE_LIGNEPAIEMENT " = act." CP_ID_ACTES "\n"
                    " AND " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " NOT IN ('G','I')\n"
                    " AND act." CP_IDPAT_ACTES " = pat.idPat\n"
                    " AND " CP_DATE_ACTES " > AddDate(NOW(),-730)\n";
        requete +=  " group by act." CP_ID_ACTES ") as mar\n"
                    " union\n\n"

                    " SELECT act." CP_ID_ACTES ", " CP_DATE_ACTES ", PatNom, PatPrenom, " CP_COTATION_ACTES ","                                 // 0, 1, 2, 3, 4
                    CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", 0 as tot, " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_TIERS_TYPEPAIEMENTACTES "," // 5, 6, 7, 8, 9
                    " PatDDN, " CP_IDUSERCOMPTABLE_ACTES ", " CP_IDUSER_ACTES "\n"                                                              // 10, 11, 12
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ\n"
                    " WHERE act." CP_ID_ACTES " = typ." CP_IDACTE_TYPEPAIEMENTACTES "\n"
                    " AND act." CP_ID_ACTES " not in (select " CP_IDACTE_LIGNEPAIEMENT " from " TBL_LIGNESPAIEMENTS ")\n"
                    " AND " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES " NOT IN ('I')\n"
                    " AND " CP_DATE_ACTES " > AddDate(NOW(),-730)\n"
                    " AND act." CP_IDPAT_ACTES " = pat.idPat\n";
        requete +=  " order by " CP_DATE_ACTES " desc, PatNom, PatPrenom";

        //UpMessageBox::Watch(this,requete);
        QList<QVariantList> actlist = db->StandardSelectSQL(requete, m_ok);
        if (actlist.size() == 0)
        {
            UpMessageBox::Watch(this, tr("Pas d'acte enregistré"));
            ok = false;
            return;
        }
        DefinitArchitectureTable(ui->ListeupTableWidget, ActesTiers);
        RemplirTableWidget(ui->ListeupTableWidget,"Actes", actlist, false, Qt::Unchecked);
        if (ui->ListeupTableWidget->rowCount() > 0)
        {
            connect (ui->ListeupTableWidget,            &QTableWidget::itemSelectionChanged, this, [=] {RenvoieRangee();});
            connect (ui->ListeupTableWidget,            &QTableWidget::itemEntered, this,   [=] (QTableWidgetItem *item) {AfficheToolTip(item);});
            connect (ui->SalleDAttenteupTableWidget,    &QTableWidget::itemEntered, this,   [=] (QTableWidgetItem *item) {AfficheToolTip(item);});
        }
        DefinitArchitectureTable(ui->SalleDAttenteupTableWidget,Paiements);
        DefinitArchitectureTable(ui->DetailupTableWidget,ActesTiers);
        break;
    }

    default:
        break;

    }
    CalculTotalDetails();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les TableWidget ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RemplirTableWidget(QTableWidget *TableARemplir, QString TypeTable, QList<QVariantList> Tablelist, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas)
{
    QTableWidgetItem    *pItem1, *pItem2, *pItem3, *pItem4, *pItem5, *pItem6, *pItem7, *pItem8, *pItem9;
    QDoubleValidator *val;
    QString             A;
    QFontMetrics fm(qApp->font());

    if (TableARemplir == ui->SalleDAttenteupTableWidget)
        disconnect (ui->SalleDAttenteupTableWidget,    &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
    TableARemplir->clearContents();
    TableARemplir->verticalHeader()->hide();

    // cette ligne est nécessaire pour éviter un pb d'affichage des tables quand on les redéfiniit
    TableARemplir->setRowCount(0);
    TableARemplir->setRowCount(Tablelist.size());

    if (TypeTable == "Actes")                                                                       // Table affichant des actes
    {
        for (int i = 0; i < Tablelist.size(); i++)
            {
                int col = 0;

                A = Tablelist.at(i).at(0).toString();                                                 // idACte
                pItem1 = new QTableWidgetItem() ;
                pItem1->setText(A);
                if (TableARemplir == ui->ListeupTableWidget)
                    pItem1->setData(Qt::UserRole,Tablelist.at(i).at(11).toInt());                     // l'id comptable est passé en data
                else if (TableARemplir == ui->SalleDAttenteupTableWidget)
                    pItem1->setData(Qt::UserRole,Tablelist.at(i).at(8).toInt());
                TableARemplir->setItem(i,col,pItem1);
                col++;

                if (AvecUpcheckBox)
                {
                    UpCheckBox *CheckItem = new UpCheckBox();
                    if (m_listidactes.contains(Tablelist.at(i).at(0).toInt()))
                        CheckItem->setCheckState(Qt::Checked);
                    else
                        CheckItem->setCheckState(CheckedOuPas);
                    CheckItem->setRowTable(i);
                    CheckItem->setFocusPolicy(Qt::NoFocus);
                    connect(CheckItem,        &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);
                    CheckItem->installEventFilter(this);
                    TableARemplir->setCellWidget(i,col,CheckItem);
                }
                if (TableARemplir == ui->DetailupTableWidget || AvecUpcheckBox)
                    col++;

                A = Tablelist.at(i).at(1).toDate().toString(tr("dd-MM-yyyy"));                       // Date
                pItem2 = new QTableWidgetItem() ;
                pItem2->setText(A);
                TableARemplir->setItem(i,col,pItem2);
                col++;

                A = Tablelist.at(i).at(2).toString() + " " + Tablelist.at(i).at(3).toString();          // Nom Prenom
                pItem3 = new QTableWidgetItem() ;
                pItem3->setText(A);
                if (TableARemplir == ui->ListeupTableWidget)
                {
                    QString superviseur (""), comptable("");
                    if (Datas::I()->users->getById(Tablelist.at(i).at(11).toInt()) != Q_NULLPTR)
                        superviseur = Datas::I()->users->getById(Tablelist.at(i).at(12).toInt())->login();
                    if (Datas::I()->users->getById(Tablelist.at(i).at(11).toInt()) != Q_NULLPTR)
                        comptable = Datas::I()->users->getById(Tablelist.at(i).at(12).toInt())->login();
                    pItem3->setData(Qt::UserRole, QStringList()
                    << tr("superviseur -> ") + superviseur
                    << tr("comptable -> ") + comptable
                    << tr("DDN ") + Tablelist.at(i).at(10).toDate().toString(tr("dd-MM-yyyy"))
                    << Tablelist.at(i).at(2).toString());                                             // Nom
                }
                TableARemplir->setItem(i,col,pItem3);
                col++;

                A = Tablelist.at(i).at(4).toString();                                                 // Cotation
                pItem4 = new QTableWidgetItem() ;
                pItem4->setText(A);
                TableARemplir->setItem(i,col,pItem4);
                col++;

                QString Montant;
                if (Tablelist.at(i).at(6).toString() == "F")
                    Montant = QLocale().toString(Tablelist.at(i).at(5).toDouble()/6.55957,'f',2);                         // Montant en F converti en euros
                else
                    Montant = QLocale().toString(Tablelist.at(i).at(5).toDouble(),'f',2);                                 // Montant
                pItem5 = new QTableWidgetItem() ;
                pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pItem5->setText(Montant);
                TableARemplir->setItem(i,col,pItem5);
                col++;

                if (m_mode == EnregistrePaiement)
                {
                    A = Tablelist.at(i).at(7).toString();
                    if (TableARemplir == ui->ListeupTableWidget)
                    {
                        if (A == IMP)
                        {
                            QLabel * lbl = new QLabel();
                            lbl->setAlignment(Qt::AlignCenter);
                            lbl->setPixmap(Icons::pxBlackCheck().scaled(15,15)); //WARNING : icon scaled : pxBlackCheck 15,15
                            TableARemplir->setCellWidget(i,col,lbl);                                    // Impayé (O/N)
                        }
                        col++;
                        QString ResteDu;
                        if (Tablelist.at(i).at(6).toString() == "F")
                            ResteDu = QLocale().toString((Tablelist.at(i).at(5).toDouble()-Tablelist.at(i).at(9).toDouble())/6.55957,'f',2);  // ResteDu en F converti en euros
                        else
                            ResteDu = QLocale().toString((Tablelist.at(i).at(5).toDouble()-Tablelist.at(i).at(9).toDouble()),'f',2);          // ResteDu
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
                        if (Tablelist.at(i).at(6).toString() == "F")
                            DejaPaye = QLocale().toString(Tablelist.at(i).at(7).toDouble()/6.55957,'f',2);
                        else
                            DejaPaye = QLocale().toString(Tablelist.at(i).at(7).toDouble(),'f',2);
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
                        LigneMontant->setRow(i);
                        LigneMontant->setColumn(col);
                        LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                               "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                        TableARemplir->setCellWidget(i,col,LigneMontant);                               // Payé
                        connect (LigneMontant,  &QLineEdit::textChanged,    this,   &dlg_paiementdirect::CalculTotalDetails);
                        LigneMontant->installEventFilter(this);
                        col++;
                    }
                }

                if (m_mode == VoirListeActes)
                {
                    A = Tablelist.at(i).at(8).toString();
                    if (A == TRS) A = Tablelist.at(i).at(9).toString();
                    if (A == ESP) A = tr(ESPECES);
                    if (A == CHQ) A = tr(CHEQUE);
                    if (A == IMP) A = tr(IMPAYE);
                    if (A == GRAT) A = tr("Gratuit");
                    if (A == "CB")  A = tr("Carte bancaire");
                    if (TableARemplir == ui->ListeupTableWidget)
                    {
                        pItem6 = new QTableWidgetItem() ;
                        pItem6->setTextAlignment(Qt::AlignCenter);
                        pItem6->setText(A);
                        TableARemplir->setItem(i,col,pItem6);                                           // Type paiement
                        col++;
                        A = QLocale().toString(Tablelist.at(i).at(7).toDouble(),'f',2);
                        pItem7 = new QTableWidgetItem;
                        pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem7->setText(A);
                        TableARemplir->setItem(i,col,pItem7);                                  // Payé
                        col++;
                        QString B = QLocale().toString((Tablelist.at(i).at(5).toDouble() - Tablelist.at(i).at(7).toDouble()),'f',2);
                        pItem8 = new QTableWidgetItem;
                        pItem8->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                        pItem8->setText(B);
                        if (QLocale().toDouble(B) > 0) pItem8->setForeground(QBrush(Qt::magenta));
                        TableARemplir->setItem(i,col,pItem8);                                  // Payé
                        col++;
                        QString ab ="";
                        if (pItem6->text() == tr("Gratuit") && Montant == "0,00")
                        {
                            pItem2->setForeground(m_textureGray);
                            pItem3->setForeground(m_textureGray);
                            pItem4->setForeground(m_textureGray);
                            pItem5->setForeground(m_textureGray);
                            pItem6->setForeground(m_textureGray);
                            pItem7->setForeground(m_textureGray);
                            pItem8->setForeground(m_textureGray);
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
                        if (m_typetable == ActesTiers)
                        {
                            A = Tablelist.at(i).at(9).toString();
                            if (A == "CB")  A = tr("Carte Bancaire");
                            pItem6 = new QTableWidgetItem() ;
                            pItem6->setTextAlignment(Qt::AlignCenter);
                            pItem6->setText(A);
                            TableARemplir->setItem(i,col,pItem6);                                                   // Type paiement
                            col++;
                        }
                        QString Paye;
                        if (Tablelist.at(i).at(6).toString() == "F")
                            Paye = QLocale().toString(Tablelist.at(i).at(7).toDouble()/6.55957,'f',2);                  // Paye en F converti en euros
                        else
                            Paye = QLocale().toString(Tablelist.at(i).at(7).toDouble(),'f',2);                          // Paye
                        QString TotalPaye;
                        if (Tablelist.at(i).at(6).toString() == "F")
                            TotalPaye = QLocale().toString(Tablelist.at(i).at(10).toDouble()/6.55957,'f',2);            // TotalPaye en F converti en euros
                        else
                            TotalPaye = QLocale().toString(Tablelist.at(i).at(10).toDouble(),'f',2);                    // TotalPaye
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

                A = Tablelist.at(i).at(1).toDate().toString("yyyy-MM-dd");                                        // ClassementparDate
                pItem9 = new QTableWidgetItem() ;
                pItem9->setText(A);
                TableARemplir->setItem(i,col,pItem9);

                TableARemplir->setRowHeight(i,int(fm.height()*1.1));
            }
    }

    if (TypeTable == "Paiements")                                                                   // Table affichant des paiements
    {
        for (int i = 0; i < Tablelist.size(); i++)
            {
                int col = 0;

                A = Tablelist.at(i).at(0).toString();                                                 // idRecette
                pItem1 = new QTableWidgetItem() ;
                pItem1->setText(A);
                TableARemplir->setItem(i,col,pItem1);
                col++;

                if (AvecUpcheckBox)
                {
                    UpCheckBox *CheckItem = new UpCheckBox();
                    if (m_listidactes.contains(Tablelist.at(i).at(0).toInt()))
                        CheckItem->setCheckState(Qt::Checked);
                    else
                        CheckItem->setCheckState(CheckedOuPas);
                    CheckItem->setRowTable(i);
                    CheckItem->setFocusPolicy(Qt::NoFocus);
                    connect(CheckItem,        &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);
                    CheckItem->installEventFilter(this);
                    TableARemplir->setCellWidget(i,col,CheckItem);
                    col++;
                }

                A = Tablelist.at(i).at(1).toDate().toString(tr("dd-MM-yyyy"));                        // Date
                pItem2 = new QTableWidgetItem() ;
                pItem2->setText(A);
                TableARemplir->setItem(i,col,pItem2);
                col++;

                QString mp = Tablelist.at(i).at(4).toString();
                if (mp == VRMT && Tablelist.at(i).at(9).toString() == "CB")
                    A = tr("Virement carte bancaire");
                else
                    A = Tablelist.at(i).at(9).toString();                                             // Payeur
                pItem3 = new QTableWidgetItem() ;
                pItem3->setText(A);
                TableARemplir->setItem(i,col,pItem3);
                col++;

                if (mp == VRMT) A = tr(VIREMENT);
                if (mp == ESP) A = tr(ESPECES);
                if (mp == CHQ) A = tr(CHEQUE);                                                    // Type paiement
                pItem4 = new QTableWidgetItem() ;
                pItem4->setText(A);
                pItem4->setTextAlignment(Qt::AlignCenter);
                TableARemplir->setItem(i,col,pItem4);
                col++;


                if (mp == CHQ)
                    A = Tablelist.at(i).at(16).toDate().toString(tr("dd-MM-yyyy"));                   // Date validation
                else
                    A = Tablelist.at(i).at(2).toDate().toString(tr("dd-MM-yyyy"));
                pItem5 = new QTableWidgetItem() ;
                pItem5->setText(A);
                TableARemplir->setItem(i,col,pItem5);
                col++;

                if (Tablelist.at(i).at(11).toString() == "F")
                    A = QLocale().toString(Tablelist.at(i).at(17).toDouble()/6.55957,'f',2);          // Payé en F converti en euros
                else
                    A = QLocale().toString(Tablelist.at(i).at(17).toDouble(),'f',2);                  // Payé
                pItem6 = new QTableWidgetItem() ;
                pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pItem6->setText(A);
                TableARemplir->setItem(i,col,pItem6);
                col++;

                A = Tablelist.at(i).at(1).toDate().toString("yyyy-MM-dd");                            // ClassementparDate
                pItem7 = new QTableWidgetItem() ;
                pItem7->setText(A);
                TableARemplir->setItem(i,col,pItem7);

                TableARemplir->setRowHeight(i,int(fm.height()*1.1));
            }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Retirer de la table Details un élément décoché et le décocher dans la table Liste ou SalDat où il figure -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::VideDetailsTable(int Rangee)
{
    bool                    idActeTrouve = false;

    // on récupère l'idActe de l'item concerné
    QString ActeAVirer = ui->DetailupTableWidget->item(Rangee,0)->text();
    //UpMessageBox::Watch(this,ui->DetailupTableWidget->item(Rangee,3)->text()+"\n"+ActeAVirer);
    //on décoche les items correspondants dans ui->ListeupTableWidget et ui->SalleDAttenteupTableWidget
    if (m_mode == EnregistrePaiement)
    {
        QList<QTableWidgetItem*> items;
        items = ui->SalleDAttenteupTableWidget->findItems(ActeAVirer,Qt::MatchExactly);
        for (int j = 0; j < items.size(); j++)
        {
            int i = items.at(j)->row();
            idActeTrouve = true;
            UpCheckBox *CheckItem = new UpCheckBox;
            CheckItem->setCheckState(Qt::Unchecked);
            CheckItem->setRowTable(i);
            CheckItem->setFocusPolicy(Qt::NoFocus);
            connect(CheckItem,        &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);
            CheckItem->installEventFilter(this);
            ui->SalleDAttenteupTableWidget->removeCellWidget(i,1);
            ui->SalleDAttenteupTableWidget->setCellWidget(i,1,CheckItem);
            RetireVerrouCompta(ActeAVirer.toInt());
        }
    }
    if (!idActeTrouve)
    {
        QList<QTableWidgetItem*> items;
        items = ui->ListeupTableWidget->findItems(ActeAVirer,Qt::MatchExactly);
        for (int j = 0; j < items.size(); j++)
        {
            int i = items.at(j)->row();
            idActeTrouve = true;
            UpCheckBox *CheckItem = new UpCheckBox;
            CheckItem->setCheckState(Qt::Unchecked);
            CheckItem->setRowTable(i);
            CheckItem->setFocusPolicy(Qt::NoFocus);
            connect(CheckItem,        &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);
            CheckItem->installEventFilter(this);
            ui->ListeupTableWidget->removeCellWidget(i,1);
            ui->ListeupTableWidget->setCellWidget(i,1,CheckItem);
            RetireVerrouCompta(ActeAVirer.toInt());
        }
    }
    if (!idActeTrouve)
    {
        bool ok = true;
        RemplitLesTables(ok);
    }
    FiltreLesTables();

    // on supprime la rangée de ui->DetailupTableWidget et on reindexe les upcheckbox et les uplinetext
    ui->DetailupTableWidget->removeRow(Rangee);
    TrieListe(ui->DetailupTableWidget);
    CalculTotalDetails();
    RegleAffichageTypePaiementframe();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Retire un verrou sur un acte ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RetireVerrouCompta(int ActeADeverrouiller)
{
    db->SupprRecordFromTable(ActeADeverrouiller, "idActe", TBL_VERROUCOMPTAACTES);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer une recette dans les tables Recettes, LignesPaiements , TypePiaiemntACte et mettre à jour SalleDAttente -----------------------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
dlg_paiementdirect::ResultEnregRecette dlg_paiementdirect::EnregistreRecette()
{
    QStringList locklist;
    locklist << TBL_RECETTES << TBL_LIGNESCOMPTES << TBL_PATIENTS <<
                TBL_ACTES << TBL_DEPENSES << TBL_SALLEDATTENTE << TBL_RUBRIQUES2035  <<
                TBL_LIGNESPAIEMENTS << TBL_TYPEPAIEMENTACTES << TBL_ARCHIVESBANQUE << TBL_USERSCONNECTES;
    if (!db->createtransaction(locklist))
        return Impossible;

    if ((ui->EspecesradioButton->isChecked() || ui->ChequeradioButton->isChecked()) && m_mode == EnregistrePaiement)

    {
        //1.  Mise à jour LignesRecettes ===============================================================================================================================================================================
        if (m_modiflignerecettepossible)
        {
            QString idCompte = "";
            // Mise à jour lignesRecettes
            QString EnregRecetterequete = "INSERT INTO " TBL_RECETTES
                    " ("
                    CP_IDUSER_LIGNRECETTES ", "
                    CP_DATE_LIGNRECETTES ", "
                    CP_DATEENREGISTREMENT_LIGNRECETTES ", "
                    CP_MONTANT_LIGNRECETTES ", "
                    CP_MODEPAIEMENT_LIGNRECETTES ","
                    CP_TIREURCHEQUE_LIGNRECETTES ", "
                    CP_BANQUECHEQUE_LIGNRECETTES ", "
                    CP_CHQENATTENTE_LIGNRECETTES ", "
                    CP_IDCPTEVIREMENT_LIGNRECETTES ", "
                    CP_IDUSERENREGISTREUR_LIGNRECETTES ", "
                    CP_TYPERECETTE_LIGNRECETTES ", "
                    CP_NOMPAYEUR_LIGNRECETTES
                    ") VALUES (";
            EnregRecetterequete +=  QString::number(m_useracrediter->id());                                             // idUser
            EnregRecetterequete +=  ", '" + ui->dateEdit->date().toString("yyyy-MM-dd");                                // DatePaiement
            EnregRecetterequete +=  "', DATE(NOW())";                                                                   // DateEnregistrement
            EnregRecetterequete +=  ", " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text()));            // Montant
            if (ui->ChequeradioButton->isChecked())
            {
                EnregRecetterequete += ",'" CHQ;                                                                        // Mode de paiement = chèque
                EnregRecetterequete += "','" + Utils::correctquoteSQL(ui->TireurChequelineEdit->text());                // Tireur chèque
                EnregRecetterequete += "','" + Utils::correctquoteSQL(ui->BanqueChequecomboBox->currentText());         // BanqueCheque
                if (ui->EnAttentecheckBox->isChecked())                                                                 // EnAttente
                    EnregRecetterequete += "',1";
                else
                    EnregRecetterequete += "',null";
                EnregRecetterequete += ",null";                                                                         // CompteVirement
            }
            else if (ui->CarteCreditradioButton->isChecked())                                                           // Mode de paiement = carte de crédit
                EnregRecetterequete += ",'" CB "',null,null,null,null";
            else if (ui->EspecesradioButton->isChecked())                                                               // Mode de paiement = espèces
                EnregRecetterequete += ",'" ESP "',null,null,null,null";
            else if (ui->VirementradioButton->isChecked())                                                              // Mode de paiement = virement
            {
                EnregRecetterequete += ",'" VRMT "',null,null,null";
                idCompte = ui->ComptesupComboBox->currentData().toString();
                EnregRecetterequete += "," + idCompte;
            }

            EnregRecetterequete += "," + QString::number(currentuser()->id());                                          // EnregistrePar
            EnregRecetterequete += ",1";                                                                                // TypeRecette

            QString NomTiers = "";
            switch (m_mode) {
            case EnregistrePaiement:
            {
                if (ui->ChequeradioButton->isChecked())
                    NomTiers = ui->TireurChequelineEdit->text();
                break;
            }
            default:{
                if (ui->DetailupTableWidget->rowCount() > 0)
                {
                    QString ChercheNomPat = "SELECT PatNom FROM " TBL_PATIENTS " pat, " TBL_ACTES " act"
                                            " WHERE pat.idPat = act." CP_IDPAT_ACTES ""
                                            " AND act." CP_ID_ACTES " = " + ui->DetailupTableWidget->item(0,0)->text();
                    QVariantList patdata = db->getFirstRecordFromStandardSelectSQL(ChercheNomPat, m_ok);
                    if (!m_ok)
                    {
                        db->rollback();
                        return Impossible;
                    }
                    NomTiers = patdata.at(0).toString();
                }
                break;
            }

            }
            EnregRecetterequete += ",'" + Utils::correctquoteSQL(NomTiers) + "')";
            //proc->Edit(EnregRecetterequete);
            if (!db->StandardSQL(EnregRecetterequete, tr("Impossible d'enregistrer cette ligne de recette")))
            {
                db->rollback();
                return Impossible;
            }

            QString ChercheMaxrequete = "SELECT Max(" CP_ID_LIGNRECETTES ") FROM " TBL_RECETTES;
            QVariantList recdata = db->getFirstRecordFromStandardSelectSQL(ChercheMaxrequete, m_ok);
            if (!m_ok)
            {
                db->rollback();
                return Impossible;
            }
            if (recdata.at(0).toInt() > 0)
                m_idrecette = recdata.at(0).toInt();

            //2. Mise à jour LignesComptes ======================================================================================================================================================
            if (ui->VirementradioButton->isChecked())
            {
                QString InsertComptrequete = "INSERT INTO " TBL_LIGNESCOMPTES "("
                        CP_ID_LIGNCOMPTES ", "
                        CP_IDCOMPTE_LIGNCOMPTES ", "
                        CP_IDREC_LIGNCOMPTES ", "
                        CP_DATE_LIGNCOMPTES ", "
                        CP_LIBELLE_LIGNCOMPTES ", "
                        CP_MONTANT_LIGNCOMPTES ", "
                        CP_DEBITCREDIT_LIGNCOMPTES ", "
                        CP_TYPEOPERATION_LIGNCOMPTES
                        ") VALUES (" +
                        QString::number(db->getIdMaxTableComptesTableArchives()) + "," +
                        idCompte + "," +
                        QString::number(m_idrecette) + ", '" +
                        ui->dateEdit->date().toString("yyyy-MM-dd") + "', "
                        "'Virement créditeur " + Utils::correctquoteSQL(ui->TierscomboBox->currentText()) + "'," +
                        QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                        ",1, "
                        "'Virement créditeur')";
                if (!db->StandardSQL(InsertComptrequete))
                {
                    db->rollback();
                    return Impossible;
                }
            }
        }
        else
        {
            bool updatelignerecettes = false;
            QString Updaterequete = "UPDATE " TBL_RECETTES " SET ";
             if (ui->ChequeradioButton->isChecked())
             {
                 updatelignerecettes = true;
                 Updaterequete += CP_TIREURCHEQUE_LIGNRECETTES " = '" + Utils::correctquoteSQL(ui->TireurChequelineEdit->text()) + "', ";                   // Tireur chèque
                 Updaterequete += CP_BANQUECHEQUE_LIGNRECETTES " = '" + Utils::correctquoteSQL(ui->BanqueChequecomboBox->currentText()) + "'";              // BanqueCheque
             }
            QString NomTiers = "";
            switch (m_mode) {
            case EnregistrePaiement:
            {
                if (ui->ChequeradioButton->isChecked())
                    NomTiers = ui->TireurChequelineEdit->text();
                break;
            }
            default:
                break;
            }
            if (updatelignerecettes && NomTiers != "")
                Updaterequete += ", ";
            if (NomTiers != "")
            {
                Updaterequete += CP_NOMPAYEUR_LIGNRECETTES " = '" + Utils::correctquoteSQL(NomTiers) + "'";
                updatelignerecettes = true;
            }
            if (updatelignerecettes)
            {
                Updaterequete += " WHERE " CP_ID_LIGNRECETTES " = " + QString::number(m_idrecette);
                //        UpMessageBox::Watch(this,Updaterequete);
                if (db->StandardSQL(Updaterequete,tr("Impossible de mettre à jour cette ligne de recette")))
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
                if (m_mode ==EnregistrePaiement)
                    ColonneMontantPaye = 7;
                else
                    ColonneMontantPaye = 8;
                UpLineEdit* Line = dynamic_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ColonneMontantPaye));
                if (Line)
                    PayeAInserer = QString::number(QLocale().toDouble(Line->text()));
                //UpMessageBox::Watch(this,PayeAInserer);
                QString UpdatePmtrequete = "INSERT INTO " TBL_LIGNESPAIEMENTS " (" CP_IDACTE_LIGNEPAIEMENT "," CP_PAYE_LIGNEPAIEMENT "," CP_IDRECETTE_LIGNEPAIEMENT ") VALUES ("
                        + ActeAInserer + "," + PayeAInserer + "," + QString::number(m_idrecette) +")";
                //UpMessageBox::Watch(this,UpdatePmtrequete);
                if (!db->StandardSQL(UpdatePmtrequete,tr("Impossible de mettre à jour la table LignesPaiements")))
                {
                    db->rollback();
                    return Impossible;
                }
            }
        }
    }

    //5.  Mise à jour TypepaiementActes ============================================================================================================================================================
    if (m_mode == EnregistrePaiement)
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
                    UpMessageBox msgbox(this);
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
                    if (!db->SupprRecordFromTable(ui->DetailupTableWidget->item(i,0)->text().toInt(), CP_IDACTE_TYPEPAIEMENTACTES, TBL_TYPEPAIEMENTACTES))
                    {
                        db->rollback();
                        return Impossible;
                    }
                    QString InsPmtrequete = "INSERT INTO " TBL_TYPEPAIEMENTACTES " (" CP_IDACTE_TYPEPAIEMENTACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ") VALUES (" + ui->DetailupTableWidget->item(i,0)->text() + ",'G')";
                    if (!db->StandardSQL(InsPmtrequete))
                    {
                        db->rollback();
                        return Impossible;
                    }
                    QString UpdPmtrequete = "UPDATE " TBL_ACTES " SET " CP_MONTANT_ACTES " = 0 WHERE " CP_ID_ACTES " = " + ui->DetailupTableWidget->item(i,0)->text();
                    if (!db->StandardSQL(UpdPmtrequete))
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
            QString Del2Pmtrequete = "DELETE FROM " TBL_TYPEPAIEMENTACTES " where " CP_IDACTE_TYPEPAIEMENTACTES " = " + ActeAInserer;
            if (!db->StandardSQL(Del2Pmtrequete,tr("Impossible de supprimer le patient de la table TypePaiementActes")))
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
            QString Ins2Pmtrequete = "INSERT INTO " TBL_TYPEPAIEMENTACTES " (" CP_IDACTE_TYPEPAIEMENTACTES ", " CP_TYPEPAIEMENT_TYPEPAIEMENTACTES ", " CP_TIERS_TYPEPAIEMENTACTES ") VALUES ("
                    + ActeAInserer + "," + ModePaiement + "," + TypeTiers +")";
            if (!db->StandardSQL(Ins2Pmtrequete,tr("Impossible de mettre à jour la table LignesPaiements")))
            {
                db->rollback();
                return Impossible;
            }
        }
    }

    //6. Mise à jour Salle d'attente ==============================================================================================================================================================================
    if (m_mode == EnregistrePaiement)
        for (int i = 0; i != ui->DetailupTableWidget->rowCount(); i++)
        {
            QString ActeAInserer = ui->DetailupTableWidget->item(i,0)->text();
            if (!db->SupprRecordFromTable(ActeAInserer.toInt(), CP_IDACTEAPAYER_SALDAT, TBL_SALLEDATTENTE, tr("Impossible de supprimer le patient de la salle d'attente")))
            {
                db->rollback();
                return Impossible;
            }
            else
            {
                QList<PatientEnCours*> listpatients;
                for (auto it = Datas::I()->patientsencours->patientsencours()->constBegin(); it != Datas::I()->patientsencours->patientsencours()->constEnd(); ++it)
                {
                    PatientEnCours *pat = const_cast<PatientEnCours*>(it.value());
                    if (pat != Q_NULLPTR)
                        if (pat->idacteapayer() == ActeAInserer.toInt())
                            listpatients << pat;
                }
                for (int i=0; i<listpatients.size();++i)
                    Datas::I()->patientsencours->SupprimePatientEnCours(listpatients.at(i));
            }
        }
    db->commit();
    Flags::I()->MAJFlagSalleDAttente();
    return OK;
}

bool dlg_paiementdirect::initOK() const
{
    return m_initok;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Transfomer un acte gratuit en acte payant (modifier le montant et supprimer la ligne correspondante de typespaiementActes) -------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::ModifGratuitChoixMenu(QString Choix)
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
            LigneMontant->setRow(ab);
            LigneMontant->setColumn(4);
            ui->ListeupTableWidget->takeItem(ab,4);
            ui->ListeupTableWidget->setCellWidget(ab,4,LigneMontant);
            LigneMontant->installEventFilter(this);
            LigneMontant->setFocus();
            LigneMontant->selectAll();
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Supprime les éventuels verrous laissés par erreur dans VerrousCompta ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::NettoieVerrousCompta()
{
    db->StandardSQL("delete from " TBL_VERROUCOMPTAACTES " where PosePar = " + QString::number(currentuser()->id()) + " or PosePar is null");
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reinitialise tous les champs de saisie --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::RemetToutAZero()
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
    m_listidactes.clear();
    bool ok = true;
    RemplitLesTables(ok);
    FiltreLesTables();
    ui->OKupPushButton              ->setEnabled(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Trie une table en remettant à jour les index des UpcheckBox et des UplineEdit ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementdirect::TrieListe(UpTableWidget *TableATrier )
{
    int ncol = TableATrier->columnCount();
    int ColonneATrier;

    if (m_mode == EnregistrePaiement)
        ColonneATrier = 3;
    else
        ColonneATrier = 2;

    if (TableATrier == ui->ListeupTableWidget)
    {
        if (m_ordretri == Chronologique)
            TableATrier->sortItems(ncol - 1,Qt::DescendingOrder);
        else
            TableATrier->sortItems(ColonneATrier,Qt::AscendingOrder);
    }
    if (TableATrier == ui->DetailupTableWidget)
    {
        TableATrier->sortItems(ColonneATrier,Qt::AscendingOrder);
        TableATrier->sortItems(ncol - 1,Qt::DescendingOrder);
    }

    if (m_mode == EnregistrePaiement)
    {
        int ColonneMontantPaye  = TableATrier->columnCount()- 2;
        int NombreRangees       = TableATrier->rowCount();
        for (int i= 0; i < NombreRangees; i++)
        {
            UpCheckBox* Check = dynamic_cast<UpCheckBox*>(TableATrier->cellWidget(i,1));
            //            QString nom = TableATrier->item(i,3)->text();
            if (Check)
            {
                disconnect(Check,       &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);    // il faut faire cette manip sinon on n'arrive pas à modifier le checkstate
                Check->setRowTable(i);
                connect(Check,          &UpCheckBox::uptoggled, this,  &dlg_paiementdirect::RenvoieRangee);
            }
            if (TableATrier == ui->DetailupTableWidget)
            {
                UpLineEdit* Line = dynamic_cast<UpLineEdit*>(TableATrier->cellWidget(i,ColonneMontantPaye));
                if (Line)
                {
                    disconnect (Line,   &QLineEdit::textChanged,    this,   &dlg_paiementdirect::CalculTotalDetails);         // même remarque
                    Line->setRow(i);
                    connect (Line,      &QLineEdit::textChanged,    this,   &dlg_paiementdirect::CalculTotalDetails);
                }
            }
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Verifie la cohérence d'un paiement -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiementdirect::VerifCoherencePaiement()
{
    QString Msg = "";
    bool A          = true;
    bool AfficheMsg = true;
    bool Reponse    = true;

    while (Reponse) {
        // On n'a coché aucun mode de paiement
        if (m_modiflignerecettepossible &&
                !(
                (ui->CarteCreditradioButton->isChecked()
                || ui->ChequeradioButton->isChecked()
                || ui->EspecesradioButton->isChecked()
                || ui->TiersradioButton->isChecked()
                || ui->ImpayeradioButton->isChecked()
                || ui->GratuitradioButton->isChecked())
                &&  m_mode == EnregistrePaiement
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
        if (m_modiflignerecettepossible && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 && ui->GratuitradioButton->isChecked()))
        {
            Msg = tr("Vous avez enregistré un montant supérieur à 0 pour un acte gratuit!");
            ui->MontantlineEdit->setFocus();
            A = false;
            break;
        }
        if (m_modiflignerecettepossible && (ui->MontantlineEdit->isVisible() && !(QLocale().toDouble(ui->MontantlineEdit->text()) > 0) && !(ui->GratuitradioButton->isChecked())))
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
            UpMessageBox msgbox(this);
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
        if (m_modiflignerecettepossible && (ui->BanqueChequecomboBox->currentText() == "" && ui->ChequeradioButton->isChecked()))
        {
            Msg = tr("Il manque le nom de la banque!");
            ui->BanqueChequecomboBox->setFocus();
            ui->BanqueChequecomboBox->showPopup();
            A = false;
            break;
        }
        // On a coché Chèque et on vérifie que le nom de la banque est enregistré
        if (m_modiflignerecettepossible
                && (ui->BanqueChequecomboBox->currentText() != ""
                && ui->ChequeradioButton->isChecked())
                && ui->BanqueChequecomboBox->findText(ui->BanqueChequecomboBox->currentText()) == -1)
        {
            A = false;
            AfficheMsg = false;
            QString Banq = ui->BanqueChequecomboBox->currentText();
            QString Msg2 = Banq + "\n" + tr("Cette banque est inconnue!\nVoulez vous l'enregistrer?");
            UpMessageBox msgbox(this);
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
                dlg_gestionbanques *Dlg_Banq = new dlg_gestionbanques(this, Banq.toUpper());
                if (Dlg_Banq->exec() == QDialog::Accepted)
                {
                    ReconstruitListeBanques();
                    ui->BanqueChequecomboBox->setCurrentText(Banq);
                    A = true;
                }
                else if (UpMessageBox::Question(this, tr("Continuer sans enregistrer la banque"), ui->BanqueChequecomboBox->currentText() + " ?")
                         == UpSmallButton::STARTBUTTON)
                    A = true;
                delete Dlg_Banq;
            }
            else if (msgbox.clickedButton() == &NoBouton)
                A = true;
            if (!A)
                break;
        }

        // On a coché Chèque et on a oublié de noter le nom du tireur
        if (m_modiflignerecettepossible && (ui->TireurChequelineEdit->text() == "" && ui->ChequeradioButton->isChecked()))
        {
            Msg = tr("Il manque le nom du tireur du chèque!");
            ui->TireurChequelineEdit->setFocus();
            A = false;
            break;
        }
        // On a coché Tiers et on a oublié de choisir le Tiers
        if (m_modiflignerecettepossible && (ui->TypeTierscomboBox->currentText() == "" && ui->TiersradioButton->isChecked()))
        {
            Msg = tr("Il manque le type de tiers payant!");
            ui->TypeTierscomboBox->setFocus();
            ui->TypeTierscomboBox->showPopup();
            A = false;
            break;
        }
        // On n'a enregistré aucune ligne correspondant à un acte
        if (ui->DetailupTableWidget->rowCount() == 0)
        {
            Msg = tr("Vous n'avez enregistré aucun acte\npour cette recette!\nConfirmez vous la saisie?");
            UpMessageBox msgbox(this);
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
bool dlg_paiementdirect::VerifVerrouCompta(UpTableWidget *TableAVerifier, int Rangee)
{
    if (t_timerafficheacteverrouilleclignotant->isActive())
        return false;
    QString ChercheVerrou = "SELECT " CP_LOGIN_USR " FROM " TBL_VERROUCOMPTAACTES ", " TBL_UTILISATEURS
                     " WHERE idActe = "  + TableAVerifier->item(Rangee,0)->text() +
                     " AND PosePar = " CP_ID_USR ;
    //UpMessageBox::Watch(this,ChercheVerrou);
    QVariantList verroudata = db->getFirstRecordFromStandardSelectSQL(ChercheVerrou, m_ok);
    if (m_ok && verroudata.size() > 0)
    {
        ui->VerrouilleParlabel->setText(tr("Acte Verrouillé par ") + verroudata.at(0).toString());
        ui->VerrouilleParlabel->setStyleSheet("color: magenta");
        ui->VerrouilleParlabel->setVisible(true);
        t_timerafficheacteverrouilleclignotant->start(100);
        connect(t_timerafficheacteverrouilleclignotant,  &QTimer::timeout,   this,   &dlg_paiementdirect::AfficheActeVerrouilleClignotant);
        t_timerafficheacteverrouille->start(2000);
        t_timerafficheacteverrouille->setSingleShot(true);
        if (m_mode == EnregistrePaiement)
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
        connect(t_timerafficheacteverrouille,   &QTimer::timeout,   this,   &dlg_paiementdirect::AfficheActeVerrouille);
        return false;
    }
    return true;
}



