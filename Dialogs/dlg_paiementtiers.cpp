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

#include "dlg_paiementtiers.h"
#include "gbl_datas.h"
#include "icons.h"
#include "ui_dlg_paiementtiers.h"
#include "utils.h"

/*
 * la fiche est appelée par le choix de menu "Gestion des tierrs payants" dans le menu Comptabilité de Rufus
*/

dlg_paiementtiers::dlg_paiementtiers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_paiementtiers)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    //ui->UserscomboBox->setEnabled(db->getUserConnected().isSecretaire());
    QFont font = qApp->font();
    font.setBold(true);
    font.setItalic(true);
    font.setPointSize(font.pointSize()+5);
    ui->Utilisateurlabel->setFont(font);

    restoreGeometry(proc->settings()->value("PositionsFiches/PositionPaiement").toByteArray());

    map_comptables = Datas::I()->users->comptables(); // les colonnes -> iduser, userlogin, soignant, responsableactes, UserEnregHonoraires, idCompteEncaissHonoraires
    if (m_userconnected->isLiberal())
        m_useracrediter = m_userconnected;
    else if (m_userconnected->isSalarie() && !m_userconnected->isAssistant())// l'utilisateur est un soignant salarie et responsable
        m_useracrediter = Datas::I()->users->getById(m_userconnected->idcomptable());
    else if (m_userconnected->isRemplacant())                                // l'utilisateur est un soignant remplacant et responsable
    {
        User *parent = Datas::I()->users->getById(m_userconnected->idparent());
        if (parent->isLiberal())
            m_useracrediter = parent;
        else if (m_userconnected->isSalarie() && !m_userconnected->isAssistant())// l'utilisateur est un soignant salarie et responsable
            m_useracrediter = Datas::I()->users->getById(parent->idcomptable());
    }
    else if(m_userconnected->isSecretaire())
        m_useracrediter = Datas::I()->users->getById(map_comptables->firstKey());
    if (m_useracrediter == Q_NULLPTR)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche de paiement"), tr("L'utilisateur n'est pas valide"));
        m_initok = false;
        return;
    }
    proc        ->MAJComptesBancaires(m_useracrediter);
    if( m_useracrediter->listecomptesbancaires()->size() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche de paiement"), tr("Les paramètres ne sont pas trouvés pour le compte ") + m_useracrediter->login());
        m_initok = false;
        return;
    }


    // On reconstruit le combobox des utilisateurs avec la liste des utilisateurs qui encaissent des honoraires et qui travaillent encore
    foreach (User *usr, *map_comptables)
        ui->UserscomboBox->addItem(usr->login(), QString::number(usr->id()));
    int idx = ui->UserscomboBox->findData(m_useracrediter->id());
    ui->UserscomboBox->setCurrentIndex(idx==-1? 0 : idx);

    // idem pour les comptes
    RegleComptesComboBox();
    ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(m_useracrediter->idcomptepardefaut()));
    connect (ui->ListeupTableWidget->horizontalHeader(), &QHeaderView::sectionClicked,                  this,           &dlg_paiementtiers::ClassementListes);

    connect (ui->AnnulupPushButton,                     &QPushButton::clicked,                          this,           &dlg_paiementtiers::Annuler);
    connect (ui->BanqueChequecomboBox,                  &QComboBox::editTextChanged,                    this,           &dlg_paiementtiers::EnableOKButton);
    connect (ui->ChequeradioButton,                     &QRadioButton::clicked,                         this,           &dlg_paiementtiers::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->CommissionlineEdit,                    &QLineEdit::editingFinished,                    this,           [=] {ConvertitDoubleMontant(ui->CommissionlineEdit);});
    connect (ui->CommissionlineEdit,                    &QLineEdit::textEdited,                         this,           &dlg_paiementtiers::EnableOKButton);
    connect (ui->ComptesupComboBox,                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                                        this,           &dlg_paiementtiers::EnableOKButton);
    connect (ui->dateEdit,                              &QDateEdit::dateChanged,                        this,           &dlg_paiementtiers::EnableOKButton);
    connect (ui->EspecesradioButton,                    &QRadioButton::clicked,                         this,           &dlg_paiementtiers::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->ListPaiementsTiersupPushButton,        &QPushButton::clicked,                          this,           &dlg_paiementtiers::VoirListePaiements);
    connect (ui->ListeupTableWidget,                    &QTableWidget::itemEntered,                     this,           &dlg_paiementtiers::AfficheDDN);
    connect (ui->MontantlineEdit,                       &QLineEdit::editingFinished,                    this,           [=] {ConvertitDoubleMontant(ui->MontantlineEdit);});
    connect (ui->MontantlineEdit,                       &QLineEdit::textEdited,                         this,           &dlg_paiementtiers::EnableOKButton);
    connect (ui->NouvTiersupPushButton,                 &QPushButton::clicked,                          this,           &dlg_paiementtiers::EnregistreNouveauPaiement);
    connect (ui->SupprimupPushButton,                   &QPushButton::clicked,                          this,           &dlg_paiementtiers::SupprimerPaiement);
    connect (ui->TierscomboBox,                         &QComboBox::editTextChanged,                    this,           &dlg_paiementtiers::RecopieNomTiers);
    connect (ui->TireurChequelineEdit,                  &QLineEdit::editingFinished,                    this,           &dlg_paiementtiers::MajusculeCreerNom);
    connect (ui->UserscomboBox,                         QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                                        this,           &dlg_paiementtiers::ChangeUtilisateur);
    connect (ui->OKupPushButton,                        &QPushButton::clicked,                          this,           &dlg_paiementtiers::ValidePaiement);
    connect (ui->VirementradioButton,                   &QRadioButton::clicked,                         this,           &dlg_paiementtiers::RegleAffichageTypePaiementframeDepuisBouton);
    connect (ui->CherchePatientupLineEdit,              &QLineEdit::textEdited,                         this,           &dlg_paiementtiers::FiltreListe);


    ui->TireurChequelineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->TireurChequelineEdit->setValidator(new QRegExpValidator(Utils::rgx_rx,this));
    ui->MontantlineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QDoubleValidator *val= new QDoubleValidator(this);
    val->setDecimals(2);
    ui->MontantlineEdit->setValidator(val);
    ui->CommissionlineEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->CommissionlineEdit->setValidator(val);
    ui->BanqueChequecomboBox->setValidator(new QRegExpValidator(Utils::rgx_MajusculeSeul));
    ui->BanqueChequecomboBox->lineEdit()->setMaxLength(10);
    ui->TierscomboBox->lineEdit()->setMaxLength(30);
    ui->TierscomboBox->lineEdit()->setAlignment(Qt::AlignRight);
    ui->BanqueChequecomboBox->lineEdit()->setAlignment(Qt::AlignRight);
    ui->ComptesupComboBox->lineEdit()->setAlignment(Qt::AlignRight);
    ui->Loupelabel->setPixmap(Icons::pxLoupe().scaled(30,30)); //WARNING : icon scaled : pxLoupe 20,20
    ui->Loupelabel->setText("");

    ReconstruitListeBanques();
    ReconstruitListeTiers();
    m_ordretri = Chronologique;

    ui->RecImageLabel->setVisible(false);
    t_timerrecord = new QTimer(this);
    t_timerrecord->start(750);
    connect (t_timerrecord, &QTimer::timeout, this, &dlg_paiementtiers::AfficheRecord);

    ui->VerrouilleParlabel->setVisible(false);
    t_timerafficheacteverrouilleclignotant = new QTimer(this);
    t_timerafficheacteverrouille = new QTimer(this);
    QList<UpPushButton *> allUpButtons = ui->Buttonswidget->findChildren<UpPushButton *>();
    for (int n = 0; n <  allUpButtons.size(); n++)
    {
        allUpButtons.at(n)->installEventFilter(this);
        allUpButtons.at(n)->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Large);
    }
    installEventFilter(this);
    ui->OKupPushButton->setShortcut(QKeySequence("Meta+Return"));
    ui->AnnulupPushButton->setShortcut(QKeySequence("F12"));
    m_modifpaiementencours = false;
    m_modiflignerecettepossible = true;
    ui->RecImageLabel->setPixmap( Icons::pxEnregistrer() );

    m_mode = Accueil;
    ui->RecImageLabel->setVisible(false);
    RegleAffichageFiche();
    m_initok = true;
}

dlg_paiementtiers::~dlg_paiementtiers()
{
    delete ui;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Acte Verrouillé ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::AfficheActeVerrouille()
{
    disconnect(t_timerafficheacteverrouilleclignotant, &QTimer::timeout, this, &dlg_paiementtiers::AfficheActeVerrouilleClignotant);
    t_timerafficheacteverrouilleclignotant->stop();
    ui->VerrouilleParlabel->setVisible(false);
//    for (int i= 0; i != ui->ListeupTableWidget->rowCount(); i++)
//    {
//        UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->ListeupTableWidget->cellWidget(i,1));
//        if (Check)
//            Check->setToggleable(true);
//    }


}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Affiche la DDN dans un tooltip ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::AfficheDDN(QTableWidgetItem *titem)
{
    if (m_mode == EnregistrePaiementTiers)
    {
        if (titem->column() == 3)
        {
            int ro = titem->row();
            QString req = "Select PATDDN from " TBL_PATIENTS " pat, " TBL_ACTES " act  where pat.idpat = act .idpat and act.idacte = "
                    + ui->ListeupTableWidget->item(ro,0)->text();
            QVariantList ddndata = db->getFirstRecordFromStandardSelectSQL(req, m_ok);
            if (m_ok && ddndata.size()>0)
                QToolTip::showText(cursor().pos(),ddndata.at(0).toDate().toString(tr("dd-MM-yyyy")));
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Acte Verrouillé ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::AfficheActeVerrouilleClignotant()
{
    ui->VerrouilleParlabel->setVisible(!ui->VerrouilleParlabel->isVisible());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Timer affichage Bouton Record -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::AfficheRecord()
{
    bool radioButtonClicked = false;
    QList<QRadioButton *> allRButtons = ui->PaiementgroupBox->findChildren<QRadioButton *>();
    for (int n = 0; n <  allRButtons.size(); n++)
        if (allRButtons.at(n)->isChecked())
        {
            radioButtonClicked = true;
            n = allRButtons.size();
        }
    if ((m_mode == EnregistrePaiementTiers) && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || radioButtonClicked || ui->DetailupTableWidget->rowCount() > 0))
        ui->RecImageLabel->setVisible(!ui->RecImageLabel->isVisible());
    else
        ui->RecImageLabel->setVisible(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Clic sur AnnulupPushButton ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::Annuler()
{
    QString requete;
    if (m_mode == Accueil)
    {
        reject();
    }
    else if (m_modifpaiementencours)
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
                //UpMessageBox::Watch(this,requete);
                db->StandardSQL(requete);
                if (m_lignecommissioncompteamodifier.size() > 0)
                {
                    requete = "INSERT INTO " TBL_LIGNESCOMPTES " VALUES (";
                    for (int i = 0; i < m_lignecommissioncompteamodifier.size(); i++)
                    {
                        requete += m_lignecommissioncompteamodifier.at(i);
                        if (i < m_lignecommissioncompteamodifier.size() -1)
                            requete += ",";
                    }
                    requete += ")";
                    //UpMessageBox::Watch(this,requete);
                    db->StandardSQL(requete);
                    m_lignecommissioncompteamodifier.clear();
                }
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
        if (m_listactesamodifier.size() > 0)
        {
            requete = "INSERT INTO " TBL_LIGNESPAIEMENTS " (idActe, idRecette, Paye) VALUES ";
            for (int i = 0; i < m_listactesamodifier.size(); i++)
            {
                requete += "(" + QString::number(m_listactesamodifier.at(i)) + "," +  QString::number(m_idrecette) + "," + m_montantactesamodifier.at(i) + ")";
                if (i < m_listactesamodifier.size() -1)
                    requete += ",";
            }
            //UpMessageBox::Watch(this,requete);
            db->StandardSQL(requete);
        }

        // 3.       restaurer les types de paiement quand il s'agit d'un paiement direct
        m_modifpaiementencours = false;
        m_mode = Accueil;
        disconnect (ui->ListeupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
        ui->RecImageLabel->setVisible(false);
        RegleAffichageFiche();
        ui->AnnulupPushButton->setText("Annuler");
        m_traiteparcloseflag = false;
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
        if (m_mode == EnregistrePaiementTiers && (QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || radioButtonClicked || ui->DetailupTableWidget->rowCount() > 0))
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
            RemetToutAZero();
            return;
        }
        else
        {
            m_mode = Accueil;
            disconnect (ui->ListeupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
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
void dlg_paiementtiers::CalculTotalDetails()
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
                Total = Total + QLocale().toDouble(ui->DetailupTableWidget->item(k,ui->DetailupTableWidget->columnCount()-2)->text());
        }
    }
    QString TotalRemise;
    TotalRemise = QLocale().toString(Total,'f',2);
    ui->TotallineEdit->setText(TotalRemise);
    if (m_mode == EnregistrePaiementTiers)
        EnableOKButton();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::ChangeUtilisateur()
{
    int id = m_useracrediter->id();
    m_useracrediter = Datas::I()->users->getById(ui->UserscomboBox->currentData().toInt());
    if (m_useracrediter != Q_NULLPTR)
        proc        ->MAJComptesBancaires(m_useracrediter);
    if (m_useracrediter == Q_NULLPTR || m_useracrediter->listecomptesbancaires()->size() == 0)
    {
        UpMessageBox::Watch                 (this,tr("Impossible de changer d'utilisateur!") , tr("Les paramètres de") + ui->UserscomboBox->currentText() + tr("ne sont pas retrouvés"));
        disconnect (ui->UserscomboBox,      QOverload<int>::of(&QComboBox::currentIndexChanged), this, &dlg_paiementtiers::ChangeUtilisateur);
        ui->UserscomboBox                   ->setCurrentIndex(ui->UserscomboBox->findData(id));
        connect (ui->UserscomboBox,         QOverload<int>::of(&QComboBox::currentIndexChanged), this, &dlg_paiementtiers::ChangeUtilisateur);
        m_useracrediter                     = Datas::I()->users->getById(id);
        proc                                ->MAJComptesBancaires(m_useracrediter);
        return;
    }

    m_useracrediter =  Datas::I()->users->getById(ui->UserscomboBox->currentData().toInt());
    RegleComptesComboBox();
    ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(m_useracrediter->idcompteencaissementhonoraires()));
    m_mode = Accueil;
    ui->RecImageLabel->setVisible(false);
    RegleAffichageFiche();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Modifie l'ordre de tri des tables ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::ClassementListes(int col)
{
    if (m_mode==VoirListePaiementsTiers && col!= 2) return;
    if (m_mode==EnregistrePaiementTiers && col!= 3) return;
    if (m_ordretri == Chronologique)
        m_ordretri = Alphabetique;
    else
        m_ordretri = Chronologique;
    TrieListe(ui->ListeupTableWidget);
    FiltreListe(ui->CherchePatientupLineEdit->text());
}

void dlg_paiementtiers::ConvertitDoubleMontant(QLineEdit *line)
{
    QString b;
    b = QLocale().toString(QLocale().toDouble(line->text()),'f',2);
    line->setText(b);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enable AnnulButton -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::EnableOKButton()
{
    ui->OKupPushButton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer un nouveau paiement par tiers ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::EnregistreNouveauPaiement()
{
    m_mode = EnregistrePaiementTiers;
    RemplitLesTables();
    if (ui->ListeupTableWidget->rowCount() == 0
            && ui->DetailupTableWidget->rowCount() == 0)
    {
        m_mode = Accueil;
        disconnect (ui->ListeupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
        ui->RecImageLabel->setVisible(false);
        RegleAffichageFiche();
        m_traiteparcloseflag = false;
    }
    else
    {
        RegleAffichageFiche();
        ui->VirementradioButton->setChecked(true);
        RegleAffichageTypePaiementframe(true,true);
        ui->PaiementgroupBox->setFocus();
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Filtrer la liste                                                 --------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::FiltreListe(QString filtre)
{
    QList<QTableWidgetItem*> listitems = ui->ListeupTableWidget->findItems(filtre, Qt::MatchStartsWith);
    if (listitems.size()>0)
    {
        int col = (m_mode == VoirListePaiementsTiers? 2 : 3);
        for (int i=0; i<listitems.size(); ++i)
        {
            if (listitems.at(i)->column() == col)
            {
                if (m_mode == VoirListePaiementsTiers)
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

void dlg_paiementtiers::MajusculeCreerNom()
{
    QString nom;
    QLineEdit *UiNom;
    UiNom = ui->TireurChequelineEdit;
    nom = Utils::trimcapitilize(UiNom->text(),false);
    UiNom->setText(nom);
    EnableOKButton();
}


void dlg_paiementtiers::VoirListePaiements()
{
    m_mode = VoirListePaiementsTiers;
    RemplitLesTables();
    RegleAffichageFiche();
    ui->ListeupTableWidget->selectRow(0);
    CompleteDetailsTable(ui->ListeupTableWidget, 0);
    ui->ListeupTableWidget->setFocus();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer un nouveau paiement direct -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RecopieNomTiers(QString b)
{
    if (m_mode == EnregistrePaiementTiers && ui->TireurChequelineEdit->isEnabled())
        ui->TireurChequelineEdit->setText(b);
    EnableOKButton();
}

void dlg_paiementtiers::RegleComptesComboBox(bool avecLesComptesInactifs)
{
    ui->ComptesupComboBox->clear();
    foreach (int idcpt, *m_useracrediter->listecomptesbancaires(avecLesComptesInactifs))
    {
        Compte *cpt = Datas::I()->comptes->getById(idcpt);
        if (cpt != Q_NULLPTR)
            ui->ComptesupComboBox->addItem(m_useracrediter->login() + "/" + cpt->nomabrege(), cpt->id());
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des différents Widget en fonction du mode de fonctionnement ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RegleAffichageFiche()
{
    ui->AnnulupPushButton->setText("Annuler");

    bool g = false;
    ui->dateEdit->setDate(QDate::currentDate());
    ui->CherchePatientupLineEdit    ->clear();

    if (m_mode == Accueil)
    {
        setFixedWidth(680);
        setFixedHeight(HAUTEUR_MINI);
        ui->Utilisateurlabel    ->setGeometry(10,15,470,20);
        ui->line0               ->setGeometry(10,50,480,15);
        ui->UserscomboBox       ->setEnabled(m_userconnected->isSecretaire());

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
        ui->ListeupTableWidget          ->setVisible(false);
        ui->OKupPushButton              ->setText(tr("Fermer"));
        ui->OKupPushButton              ->setIcon(Icons::icOK());
        ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->Utilisateurlabel                ->setText(tr("Gestion des tiers payants"));
            ui->NouvTiersupPushButton           ->setVisible(true);
            ui->ListPaiementsTiersupPushButton  ->setVisible(true);
        g = true;
        ui->SupprimupPushButton         ->setVisible(false);
        RegleComptesComboBox();
        ui->ComptesupComboBox           ->setCurrentIndex(ui->ComptesupComboBox->findData(m_useracrediter->idcompteencaissementhonoraires()));
    }
    else
    {
        setFixedHeight(950);
        ui->Utilisateurlabel                ->setGeometry(10,15,590,20);
        ui->line0                           ->setGeometry(10,50,600,15);
        ui->UserscomboBox                   ->setEnabled(false);
        ui->Detailsframe                    ->setVisible(true);
        ui->line2                           ->setVisible(true);
        ui->ActesEnAttentelabel_3           ->setVisible(true);
        ui->ListeupTableWidget              ->setVisible(true);
        ui->NouvTiersupPushButton           ->setVisible(false);
        ui->ListPaiementsTiersupPushButton  ->setVisible(false);

        switch (m_mode) {
        case EnregistrePaiementTiers:
        {
            ui->Utilisateurlabel            ->setText(tr("Gestion des tiers payants - Enregistrer un tiers payant"));
            ui->OKupPushButton              ->setText(tr("Valider\net fermer"));
            ui->OKupPushButton              ->setIcon(Icons::icOK());
            ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->ActesEnAttentelabel         ->setText(tr("Actes enregistrés en tiers payant en attente de paiement"));
            ui->PaiementgroupBox            ->setFocusProxy(ui->VirementradioButton);
            RegleComptesComboBox();
            ui->ComptesupComboBox           ->setCurrentIndex(ui->ComptesupComboBox->findData(m_useracrediter->idcompteencaissementhonoraires()));
            break;
        }
        case VoirListePaiementsTiers:
        {
            ui->Utilisateurlabel            ->setText(tr("Gestion des tiers payants - Liste des paiements"));
            ui->OKupPushButton              ->setText("Modifier");
            ui->OKupPushButton              ->setIcon(Icons::icMarteau());
            ui->OKupPushButton              ->setIconSize(QSize(30,30));
            ui->ActesEnAttentelabel         ->setText(tr("Tous les paiements enregistrés en tiers payant"));
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
void dlg_paiementtiers::RegleAffichageTypePaiementframeDepuisBouton()
{
    RegleAffichageTypePaiementframe(true, true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Retrouve le rang de la ligne selectionnée et modifie les tables en fonction ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RenvoieRangee(bool Coche)
{
    QTableWidget*  TableOrigine = dynamic_cast<QTableWidget*>(focusWidget());
    if (TableOrigine != Q_NULLPTR)
    {
        switch (m_mode) {
        case EnregistrePaiementTiers:
        {
            UpCheckBox* Check = dynamic_cast<UpCheckBox*>(sender());
            if(Check)
            {
                int R = Check->rowTable();
                if (ui->DetailupTableWidget->isAncestorOf(Check))
                    VideDetailsTable(R);
                else
                    CompleteDetailsTable(TableOrigine,R,Coche);
            }
            if (m_modifpaiementencours)
                ui->OKupPushButton->setEnabled(true);
            break;
        }
        case VoirListePaiementsTiers:
        {
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
        TableOrigine = Q_NULLPTR;
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Regle l'affichage des widget dans le cadre DetailsPaiement en fonction du mode de fonctionnement et du radiobutton sélectionné ------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::SupprimerPaiement()
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
    m_modifpaiementencours = false;
    RemetToutAZero();
    m_mode = Accueil;
//    disconnect (ui->ListeupTableWidget,           SIGNAL(itemSelectionChanged()), this, SLOT(Slot_RenvoieRangee()));
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
void dlg_paiementtiers::ValidePaiement()
{
    QString requete;
    switch (m_mode) {
    case Accueil:
    {
        Annuler();
        break;
    }
    case EnregistrePaiementTiers:
    {
        if (VerifCoherencePaiement())
            switch (EnregistreRecette()) {
            case Impossible:
            case OK:
            {
                m_modifpaiementencours = false;
                RemetToutAZero();
                m_mode = Accueil;
                disconnect (ui->ListeupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
                ui->RecImageLabel->setVisible(false);
                m_modiflignerecettepossible = true;
                RegleAffichageFiche();
                ui->SupprimupPushButton->setVisible(false);
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
        /* Il s'agit de modifier un paiement par tiers  pour lequel une recette a été enregistrée. */
    {
        NettoieVerrousCompta();
        m_modifpaiementencours = false;
        QString ModePaiement;
        // On retrouve l'idRecette de LignesRecettes correspondant au paiement à modifier
        QList<QTableWidgetSelectionRange>  RangeeSelectionne = ui->ListeupTableWidget->selectedRanges();
        int ab      = RangeeSelectionne.at(0).topRow();
        m_idrecette   = ui->ListeupTableWidget->item(ab,0)->text().toInt();
        requete = "SELECT idRecette FROM " TBL_RECETTES " WHERE idRecette = " + QString::number(m_idrecette);
        QList<QVariantList> reclist = db->StandardSelectSQL(requete,m_ok);
        if (reclist.size() == 0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier ce paiement pour le moment"),
                                tr("Il est en cours de modification par un autre utilisateur."));
            RemplitLesTables();
            return;
        }
        requete = "SELECT idActe FROM " TBL_LIGNESPAIEMENTS
                " WHERE idRecette = " + QString::number(m_idrecette) +
                " AND idActe IN (SELECT idActe FROM " TBL_VERROUCOMPTAACTES " WHERE PosePar != " + QString::number(m_userconnected->id()) + ")";
        QList<QVariantList> actlist = db->StandardSelectSQL(requete,m_ok);
        if (actlist.size() > 0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas modifier ce paiement pour le moment."),
                                tr("Certains actes qui le composent sont actuellement verrouillés par d'autres utilisateurs."));
            return;
        }
        ModifPaiementTiers(m_idrecette);
        break;
        }
    }
}

void dlg_paiementtiers::closeEvent(QCloseEvent *event)
{
    proc->settings()->setValue("PositionsFiches/PositionPaiement", saveGeometry());
    event->accept();
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Interception des évènements internes -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiementtiers::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn )
    {
        if (obj->inherits("UpLineEdit"))
        {
            if (obj->parent()->parent() == ui->DetailupTableWidget  && m_mode == EnregistrePaiementTiers)
            {
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                m_valeuravantchangement = Line->text();
                m_vleurmaxi = ui->DetailupTableWidget->item(Line->Row(),Line->Column()-1)->text();
                Line->selectAll();
            }
        }
    }
    if (event->type() == QEvent::FocusOut)
    {
        if (obj->inherits("UpLineEdit"))
        {
            if (obj->parent()->parent() == ui->DetailupTableWidget  && m_mode == EnregistrePaiementTiers)
            {
                UpLineEdit* Line = static_cast<UpLineEdit*>(obj);
                if (QLocale().toDouble(Line->text()) > QLocale().toDouble(m_vleurmaxi))
                {
                    QSound::play(NOM_ALARME);
                    m_valeuravantchangement = QLocale().toString(QLocale().toDouble(m_vleurmaxi),'f',2);   // Montant payé
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
//                    if (gMode == EnregistrePaiementTiers
//                            && (CheckBox->parent()->parent() == ui->ListeupTableWidget)
//                            && !CheckBox->isChecked())
//                    {
//                        if (!VerifVerrouCompta(static_cast<QTableWidget*>(CheckBox->parent()->parent()),CheckBox->getRowTable()))
//                            return true;
//                    }
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
                if (obj->parent()->parent() == ui->DetailupTableWidget  && m_mode == EnregistrePaiementTiers)
                {
                    if (QLocale().toDouble(Line->text()) > QLocale().toDouble(m_vleurmaxi))
                    {
                        QSound::play(NOM_ALARME);
                        m_valeuravantchangement = QLocale().toString(QLocale().toDouble(m_vleurmaxi),'f',2);   // Montant payé
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
//                else
//                    ui->ListeupTableWidget->setFocus();
            }
            else if (!obj->inherits("UpPushButton") && obj->objectName() != "dlg_paiementtiers")
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

bool dlg_paiementtiers::initOK() const
{
    return m_initok;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Définit l'architecture des TableView (SelectionMode, nombre de colonnes, avec Widgets UpcheckBox et UplineBox) ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::DefinitArchitectureTableView(UpTableWidget *TableARemplir, TypeTable Type)
{
    QStringList         LabelARemplir;
    QString             A;
    int                 ColCount = 0;

    // il faut deconnecter la table du signal itemSelectionChanged(), sinon, l'appel de TableARemplir->clear()
    // provoque un plantage. La table est vidée mais le slot est quand même appelé par le signal
    TableARemplir->clear();
    TableARemplir->verticalHeader()->hide();

    // ces 2 lignes sont nécessaires pour éviter un pb d'affichage des tables quand on les redéfinit
    TableARemplir->setRowCount(0);
    TableARemplir->setColumnCount(0);
    
    switch (Type) {
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
        LabelARemplir << "";

        TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
        TableARemplir->horizontalHeader()->setVisible(true);
        int li = 0;
        TableARemplir->setColumnWidth(li,0);                                                // idActe ou idPaiement
        li++;
        TableARemplir->setColumnWidth(li,90);                                               // Date
        li++;
        TableARemplir->setColumnWidth(li,163);                                              // Payeur
        li++;
        TableARemplir->setColumnWidth(li,130);                                              // Mode de paiement
        li++;
        TableARemplir->setColumnWidth(li,95);                                               // Enregistré le
        li++;
        TableARemplir->setColumnWidth(li,75);                                               // Payé
        li++;
        TableARemplir->setColumnWidth(li,0);                                                // PaiementDate format yyyy-MM-dd pour le tri par date

        break;
    }
    default:
    {
        if (TableARemplir == ui->DetailupTableWidget)
        {
            ColCount = 9;
            if (m_mode == EnregistrePaiementTiers)
                ColCount ++;
            TableARemplir->setColumnCount(ColCount);
            TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);

            LabelARemplir << "";
            if (m_mode == EnregistrePaiementTiers)
                LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
            LabelARemplir << tr("Type tiers");
            LabelARemplir << tr("Reste dû");
            LabelARemplir << tr("Payé");
            LabelARemplir << "";
            TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
            TableARemplir->horizontalHeader()->setVisible(true);

            int li = 0;
            TableARemplir->setColumnWidth(li,0);                                                    // idActe ou idPaiement
            li++;
            if (m_mode == EnregistrePaiementTiers)
            {
                TableARemplir->setColumnWidth(li,20);                                               // Checkbox
                li++;
            }
            TableARemplir->setColumnWidth(li,90);                                                   // Date
            li++;
            TableARemplir->setColumnWidth(li,193);                                                  // Nom Prenom
            li++;
            TableARemplir->setColumnWidth(li,160);                                                  // Cotation
            li++;
            switch (m_mode) {
            case VoirListePaiementsTiers:
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                TableARemplir->setColumnWidth(li,120);                                              // Type tiers
                li ++;
                TableARemplir->setColumnWidth(li,75);                                               // Reste dû
                break;
            }
            default:
            {
                TableARemplir->setColumnWidth(li,75);                                               // Montant
                li++;
                TableARemplir->setColumnWidth(li,100);                                              // Type tiers
                li ++;
                TableARemplir->setColumnWidth(li,75);                                               // Reste dû
                break;
            }
            }
            li++;
            TableARemplir->setColumnWidth(li,75);                                                   // Payé
            li++;
            TableARemplir->setColumnWidth(li,0);                                                    // ActeDate format yyyy-MM-dd pour le tri par date
        }

        if (TableARemplir == ui->ListeupTableWidget)
        {
            ColCount = 9;
            if (m_mode == EnregistrePaiementTiers)
                ColCount ++;
            TableARemplir->setColumnCount(ColCount);
            if (m_mode == EnregistrePaiementTiers)
                TableARemplir->setSelectionMode(QAbstractItemView::NoSelection);
            else
                TableARemplir->setSelectionMode(QAbstractItemView::SingleSelection);

            LabelARemplir << "";
            if (m_mode == EnregistrePaiementTiers)
                LabelARemplir << "";
            LabelARemplir << tr("Date");
            LabelARemplir << tr("Nom Prénom");
            LabelARemplir << tr("Cotation");
            LabelARemplir << tr("Montant");
            LabelARemplir << tr("Type tiers");
            LabelARemplir << tr("Payé");
            LabelARemplir << tr("Reste dû");
            LabelARemplir << "";
            TableARemplir->setHorizontalHeaderLabels(LabelARemplir);
            TableARemplir->horizontalHeader()->setVisible(true);
            int li = 0;
            TableARemplir->setColumnWidth(li,0);                                                    // idActe
            li++;
            if (m_mode == EnregistrePaiementTiers)
            {
                TableARemplir->setColumnWidth(li,20);                                               // Checkbox
                li++;
                TableARemplir->setColumnWidth(li,90);                                               // Date
            }
            else
                TableARemplir->setColumnWidth(li,90);
            li++;
            TableARemplir->setColumnWidth(li,193);                                                  // Nom Prenom ou Payeur
            li++;
            TableARemplir->setColumnWidth(li,160);                                                  // Cotation
            li++;
            TableARemplir->setColumnWidth(li,75);                                                   // Montant
            li++;
            TableARemplir->setColumnWidth(li,100);                                                  // Typetiers
            li ++;
            TableARemplir->setColumnWidth(li,75);                                                   // Payé
            li++;
            TableARemplir->setColumnWidth(li,75);                                                   // Reste dû
            li++;
            TableARemplir->setColumnWidth(li,0);                                                    // ActeDate format yyyy-MM-dd pour le tri par date
        }
        break;
    }
    }
    TableARemplir->setColumnHidden(0,true);
    TableARemplir->setColumnHidden(ColCount-1,true);
    TableARemplir->setGridStyle(Qt::SolidLine);
    int l = TableARemplir->FixLargeurTotale();
    if (Type != Paiements)
        setFixedWidth(l + 20);
}


/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer une recette dans les tables Recettes, LignesPaiements , TypePiaiemntACte -----------------------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
dlg_paiementtiers::ResultEnregRecette dlg_paiementtiers::EnregistreRecette()
{
    QStringList locklist = QStringList() << TBL_RECETTES << TBL_LIGNESCOMPTES << TBL_DEPENSES << TBL_RUBRIQUES2035 << TBL_LIGNESPAIEMENTS;

    if (!db->createtransaction(locklist))
        return Impossible;

    if ((ui->EspecesradioButton->isChecked() || ui->VirementradioButton->isChecked() || ui->ChequeradioButton->isChecked()) && m_mode == EnregistrePaiementTiers)
    {
        //1.  Mise à jour LignesRecettes ===============================================================================================================================================================================
        if (m_modiflignerecettepossible)
        {
            QString idCompte = "";
            // Mise à jour lignesRecettes
            QString EnregRecetterequete = "INSERT INTO " TBL_RECETTES
                    " (idUser, DatePaiement, DateEnregistrement, Montant, ModePaiement, TireurCheque, BanqueCheque, EnAttente, CompteVirement, EnregistrePar, TypeRecette, TiersPayant, NomTiers, Commission) VALUES (";
            EnregRecetterequete +=  QString::number(m_useracrediter->id());                                                  // idUser
            EnregRecetterequete +=  ", '" + ui->dateEdit->date().toString("yyyy-MM-dd");                                // DatePaiement
            EnregRecetterequete +=  "', DATE(NOW())";                                                                   // DateEnregistrement
            EnregRecetterequete +=  ", " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text()));              // Montant
            if (ui->ChequeradioButton->isChecked())
            {
                EnregRecetterequete += ",'C";                                                                           // Mode de paiement = chèque
                EnregRecetterequete += "','" + Utils::correctquoteSQL(ui->TireurChequelineEdit->text());               // Tireur chèque
                EnregRecetterequete += "','" + Utils::correctquoteSQL(ui->BanqueChequecomboBox->currentText());        // BanqueCheque
                    EnregRecetterequete += "',null";
                EnregRecetterequete += ",null";                                                                         // CompteVirement
            }
            else if (ui->EspecesradioButton->isChecked())                                                               // Mode de paiement = espèces
                EnregRecetterequete += ",'E',null,null,null,null";
            else if (ui->VirementradioButton->isChecked())                                                              // Mode de paiement = virement
            {
                EnregRecetterequete += ",'V',null,null,null";
                idCompte = ui->ComptesupComboBox->currentData().toString();
                EnregRecetterequete += "," + idCompte;
            }

            EnregRecetterequete += "," + QString::number(m_userconnected->id());                                        // EnregistrePar
            EnregRecetterequete += ",1";                                                                                // TypeRecette
            EnregRecetterequete += ",'O'";                                                                              // Tierspayant

            QString NomTiers = ui->TierscomboBox->currentText();
            EnregRecetterequete += ",'" + Utils::correctquoteSQL(NomTiers);                                             // Nomtiers
            QString commission = (ui->CommissionlineEdit->text() ==""? "null" : QString::number(QLocale().toDouble(ui->CommissionlineEdit->text())));
            EnregRecetterequete += "'," + commission +")";                                                              // Commission
            //proc->Edit(EnregRecetterequete);
            if (!db->StandardSQL(EnregRecetterequete,"Impossible d'enregistrer cette ligne de recette"))
            {
                db->rollback();
                return Impossible;
            }


            QString ChercheMaxrequete = "SELECT Max(idRecette) FROM " TBL_RECETTES;
            QVariantList maxrecdata = db->getFirstRecordFromStandardSelectSQL(ChercheMaxrequete, m_ok);
            if (!m_ok || maxrecdata.size()==0)
            {
                db->rollback();
                return Impossible;
            }
            if (maxrecdata.at(0).toInt() > 0)
                m_idrecette = maxrecdata.at(0).toInt();

            //2. Mise à hour LignesComptes ======================================================================================================================================================
            if (ui->VirementradioButton->isChecked())
            {
                QString InsertComptrequete = "INSERT INTO " TBL_LIGNESCOMPTES "(idLigne, idCompte, idRec, LigneDate, LigneLibelle,  LigneMontant, LigneDebitCredit, LigneTypeOperation) VALUES ("
                        + QString::number(db->getIdMaxTableComptesTableArchives()) + "," + idCompte + "," + QString::number(m_idrecette) + ", '" + ui->dateEdit->date().toString("yyyy-MM-dd")
                        + "', 'Virement créditeur " + Utils::correctquoteSQL(ui->TierscomboBox->currentText()) + "',"
                        + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) + ",1,'Virement créditeur')";
                if (!db->StandardSQL(InsertComptrequete))
                {
                    db->rollback();
                    return Impossible;
                }

            }
            //3. Mise à hour Depenses  et LignesComptes s'il y a eu une commission sur le virement ==============================================================================================
            if (QLocale().toDouble(ui->CommissionlineEdit->text()) > 0)
            {
                QString SelectMaxrequete = "select max(iddep) + 1 from " TBL_DEPENSES;
                QVariantList maxdepdata = db->getFirstRecordFromStandardSelectSQL(SelectMaxrequete, m_ok);
                if (!m_ok || maxdepdata.size()==0)
                {
                    db->rollback();
                    return Impossible;
                }
                QString max = maxdepdata.at(0).toString();

                QString InsertDeprequete = "INSERT INTO " TBL_DEPENSES "(iddep, idUser, DateDep, RefFiscale, Objet, Montant, FamFiscale, idRec, ModePaiement,Compte) VALUES (";
                InsertDeprequete += max;                                                                                        // idDep
                InsertDeprequete +=  "," + QString::number(m_useracrediter->id());                                                   // idUser
                InsertDeprequete +=  ", '" + ui->dateEdit->date().toString("yyyy-MM-dd");                                       // DateDep
                // on va rechercher l'id2035:
                // si c'est une carte de crédit, l'id2035 correspondra à "frais financiers", sinon, ce sera "honoraires rétrocédés"
                QString intitule2035 = "Honoraires rétrocédés";
                if (ui->TierscomboBox->currentText() == "CB")
                    intitule2035= "Frais financiers";
                InsertDeprequete +=  "', '" + Utils::correctquoteSQL(intitule2035);                                            // RefFiscale
                InsertDeprequete +=  "', 'Commission " + Utils::correctquoteSQL(ui->TierscomboBox->currentText());             // Objet
                InsertDeprequete +=  "', " +  QString::number(QLocale().toDouble(ui->CommissionlineEdit->text()));              // Montant
                QString chercheFamFiscale = "select Famfiscale from " TBL_RUBRIQUES2035 " where reffiscale = '" + Utils::correctquoteSQL(intitule2035) +"'";
                QVariantList famfiscdata = db->getFirstRecordFromStandardSelectSQL(chercheFamFiscale, m_ok);
                if (!m_ok || famfiscdata.size()==0)
                {
                    db->rollback();
                    return Impossible;
                }
                InsertDeprequete += ", '" + Utils::correctquoteSQL(famfiscdata.at(0).toString()) + "'";
                InsertDeprequete += ", " + QString::number(m_idrecette);                                                          // idRec
                InsertDeprequete += ", 'P'";                                                                                    // ModePaiement = P pour prélèvement
                InsertDeprequete += ", " + idCompte + ")";
                if (!db->StandardSQL(InsertDeprequete))
                {
                    db->rollback();
                    return Impossible;
                }
                if (ui->VirementradioButton->isChecked())
                {
                    QString Commission = "Commission";
                    if (ui->TierscomboBox->currentText() == "CB")
                        Commission += " CB";
                    QString InsertComrequete = "INSERT INTO " TBL_LIGNESCOMPTES "(idCompte, idDep, idRec, LigneDate, LigneLibelle,  LigneMontant, LigneDebitCredit, LigneTypeOperation) VALUES ("
                            + idCompte + "," + max + "," + QString::number(m_idrecette) + ", '" + ui->dateEdit->date().toString("yyyy-MM-dd")
                            + "', '" + Commission + "'," + QString::number(QLocale().toDouble(ui->CommissionlineEdit->text())) + ",0,'Prélèvement')";
                    if (!db->StandardSQL(InsertComrequete))
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
            QString Updaterequete = "UPDATE " TBL_RECETTES " SET ";
             if (ui->ChequeradioButton->isChecked())
             {
                 updatelignerecettes = true;
                 Updaterequete += "TireurCheque = '" + Utils::correctquoteSQL(ui->TireurChequelineEdit->text()) + "', ";                   // Tireur chèque
                 Updaterequete += "BanqueCheque = '" + Utils::correctquoteSQL(ui->BanqueChequecomboBox->currentText()) + "'";              // BanqueCheque
             }
            QString NomTiers = ui->TierscomboBox->currentText();
            if (updatelignerecettes && NomTiers != "")
                Updaterequete += ", ";
            if (NomTiers != "")
            {
                Updaterequete += "NomTiers = '" + Utils::correctquoteSQL(NomTiers) + "'";
                updatelignerecettes = true;
            }
            if (updatelignerecettes)
            {
                Updaterequete += " WHERE idRecette = " + QString::number(m_idrecette);
                //        UpMessageBox::Watch(this,Updaterequete);
                if (!db->StandardSQL(Updaterequete,tr("Impossible de mettre à jour cette ligne de recette")))
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
                int ColonneMontantPaye = 8;
                UpLineEdit* Line = dynamic_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(i,ColonneMontantPaye));
                if (Line)
                    PayeAInserer = QString::number(QLocale().toDouble(Line->text()));
                //UpMessageBox::Watch(this,PayeAInserer);
                QString UpdatePmtrequete = "INSERT INTO " TBL_LIGNESPAIEMENTS " (idActe,Paye,idRecette) VALUES ("
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
    db->commit();
    Flags::I()->MAJFlagSalleDAttente();
    return OK;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir la DetailsTable en fonction du mode appelé -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::CompleteDetailsTable(QTableWidget *TableOrigine, int Rangee, bool Coche)
{
    QDoubleValidator        *val;
    QFontMetrics            fm(qApp->font());
    QString                 requete;

    switch (m_mode) {
    case EnregistrePaiementTiers:
    {
        QTableWidgetItem        *pItem1, *pItem2, *pItem3, *pItem4, *pItem5, *pItem6, *pItem7, *pItem8;
        if (Coche)
        {
            int i   = 0;//ui->DetailupTableWidget->rowCount();
            ui->DetailupTableWidget->insertRow(0);
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
            connect(CheckItem, &UpCheckBox::uptoggled, this, &dlg_paiementtiers::RenvoieRangee);
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
            LigneMontant->setColumn(8);
            connect (LigneMontant, &QLineEdit::textChanged, this, &dlg_paiementtiers::CalculTotalDetails);
            LigneMontant->installEventFilter(this);

            ui->DetailupTableWidget->setItem(i,0,pItem1);                  //idActe
            ui->DetailupTableWidget->setCellWidget(i,1,CheckItem);         //Check
            ui->DetailupTableWidget->setItem(i,2,pItem2);                  //Date
            ui->DetailupTableWidget->setItem(i,3,pItem3);                  //Nom Prenom
            ui->DetailupTableWidget->setItem(i,4,pItem4);                  //Cotation
            ui->DetailupTableWidget->setItem(i,5,pItem5);                  //Montant
            ui->DetailupTableWidget->setItem(i,6,pItem8);                  //Type tiers
            ui->DetailupTableWidget->setItem(i,7,pItem6);                  //Reste dû
            ui->DetailupTableWidget->setCellWidget(i,8,LigneMontant);      //Payé
            ui->DetailupTableWidget->setItem(i,9,pItem7);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique
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
                    connect(CheckItem, &UpCheckBox::uptoggled, this, &dlg_paiementtiers::RenvoieRangee);
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
                    LigneMontant->setColumn(8);
                    LigneMontant->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                           "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                    connect (LigneMontant, &QLineEdit::textChanged, this, &dlg_paiementtiers::CalculTotalDetails);
                    LigneMontant->installEventFilter(this);

                    ui->DetailupTableWidget->setItem(l,0,pItem1);                  //idActe
                    ui->DetailupTableWidget->setCellWidget(l,1,CheckItem);         //Check
                    ui->DetailupTableWidget->setItem(l,2,pItem2);                  //Date
                    ui->DetailupTableWidget->setItem(l,3,pItem3);                  //Nom Prenom
                    ui->DetailupTableWidget->setItem(l,4,pItem4);                  //Cotation
                    ui->DetailupTableWidget->setItem(l,5,pItem5);                  //Montant
                    ui->DetailupTableWidget->setItem(l,6,pItem6);                  //Type tiers
                    ui->DetailupTableWidget->setItem(l,7,pItem7);                  //Reste dû
                    ui->DetailupTableWidget->setCellWidget(l,8,LigneMontant);      //Payé
                    ui->DetailupTableWidget->setItem(l,9,pItem8);                  //Date format yyyy--MM-dd pour le tri par ordre chronologique
                }
            }
        }
        TrieListe(ui->DetailupTableWidget);
        break;
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
        QString TextidRecette   = TableOrigine->item(Rangee,0)->text();

        requete =   "SELECT act." CP_IDACTE_ACTES ", " CP_DATE_ACTES ", PatNom, PatPrenom, " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", Paye, TypePaiement, Tiers, TotalPaye\n"
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_LIGNESPAIEMENTS " lig, " TBL_TYPEPAIEMENTACTES " typ,\n"
                    " (SELECT lig.idActe, SUM(lig.paye) as TotalPaye FROM " TBL_LIGNESPAIEMENTS " lig,\n"
                    " (SELECT idActe FROM " TBL_LIGNESPAIEMENTS
                    " WHERE idRecette = " + TextidRecette + ") AS Result\n"
                    " WHERE lig.idActe = Result.idActe GROUP BY lig.idActe) AS calc\n"
                    " WHERE act." CP_IDACTE_ACTES " = lig.idActe\n"
                    " AND typ.idActe = act." CP_IDACTE_ACTES "\n"
                    " AND calc.idActe = act." CP_IDACTE_ACTES "\n"
                    " AND lig.idRecette = " + TextidRecette + "\n"
                    " AND act." CP_IDPAT_ACTES " = pat.idPat\n"
                    " ORDER BY " CP_DATE_ACTES " DESC, PatNom, PatPrenom";

        //UpMessageBox::Watch(this,requete);
        QList<QVariantList> detpmtlist = db->StandardSelectSQL(requete,m_ok);
        RemplirTableWidget(ui->DetailupTableWidget, Actes, detpmtlist, false, Qt::Unchecked);

        // Remplir les infos sur la recette concernée
        RecetteComptable* rec = db->loadRecetteComptablebyId(TextidRecette.toInt());
        if (rec == Q_NULLPTR)
            break;
        ui->dateEdit->setDate(rec->date());
        QRadioButton *RadioAClicker = Q_NULLPTR;
        QString mp = rec->modepaiement();
        if (mp == "V")
        {
            RadioAClicker = ui->VirementradioButton;
            QString Commission = QLocale().toString(rec->commission(),'f',2);
            ui->CommissionlineEdit->setText(Commission);
        }
        else if (mp == "E") RadioAClicker = ui->EspecesradioButton;
        else if (mp == "C") RadioAClicker = ui->ChequeradioButton;
        if (RadioAClicker != Q_NULLPTR)
            RadioAClicker->setChecked(true);
        ui->ComptesupComboBox->clearEditText();
        if (rec->compteid() > 0)
            ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(rec->compteid()));
        ui->TierscomboBox->setCurrentText(rec->payeur());
        if (mp == "C")
        {
            ui->TireurChequelineEdit->setText(rec->tireurcheque());
            ui->BanqueChequecomboBox->setCurrentText(rec->banquecheaque());
        }
        QString Montant = QLocale().toString(rec->montant(),'f',2);
        ui->MontantlineEdit->setText(Montant);
        delete rec;
        break;
    }
    default:
        break;
    } // fin switch
    CalculTotalDetails();
    RegleAffichageTypePaiementframe(false);
    RegleAffichageTypePaiementframe(true,false);
    ui->DetailupTableWidget->setAllRowHeight(int(fm.height()*1.1));
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Modifie un paiement par tiers ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::ModifPaiementTiers(int idRecetteAModifier)

{
    m_modifpaiementencours  = true;
    m_mode                  = EnregistrePaiementTiers;
    m_idrecette             = idRecetteAModifier;
    QString                 requete;
    m_listactesamodifier    .clear();
    m_montantactesamodifier .clear();

    // on fait la liste des actes et des montants payés de la recette à modifier
    QString Retrouverequete = "SELECT idActe, Paye FROM " TBL_LIGNESPAIEMENTS " WHERE idRecette = " + QString::number(m_idrecette);
    QList<QVariantList> actlist = db->StandardSelectSQL(Retrouverequete,m_ok);
    for (int i = 0; i < actlist.size(); i++)
    {
        m_listactesamodifier    << actlist.at(i).at(0).toInt();
        m_montantactesamodifier << actlist.at(i).at(1).toString();
    }

    // Remplir les infos sur la recette concernée
    RecetteComptable* rec = db->loadRecetteComptablebyId(m_idrecette);
    if (rec == Q_NULLPTR)
        return;
    ui->dateEdit    ->setDate(rec->date());
    QRadioButton *RadioAClicker = Q_NULLPTR;
    QString mp      = rec->modepaiement();
    if (mp == "V")
    {
        RadioAClicker       = ui->VirementradioButton;
        QString Commission  = QLocale().toString(rec->commission(),'f',2);
        ui->CommissionlineEdit->setText(Commission);
    }
    else if (mp == "E") RadioAClicker = ui->EspecesradioButton;
    else if (mp == "C") RadioAClicker = ui->ChequeradioButton;
    if (RadioAClicker != Q_NULLPTR)
        RadioAClicker   ->setChecked(true);
    ui->ComptesupComboBox->clearEditText();
    if (rec->compteid() > 0)
        ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(rec->compteid()));
    ui->TierscomboBox->setCurrentText(rec->payeur());
    if (mp == "C")
    {
        ui->TireurChequelineEdit    ->setText(rec->tireurcheque());
        ui->BanqueChequecomboBox    ->setCurrentText(rec->banquecheaque());
    }
    QString Montant = QLocale().toString(rec->montant(),'f',2);
    ui->MontantlineEdit->setText(Montant);

    /* Verifier si on peut modifier la recette - impossible si:
 . c'est un chèque et il a été déposé en banque
 . c'est un virement et il a été pointé sur le compte*/
    if (rec->modepaiement() == "C" && rec->idremisecheque() > 0)                                                //!> c'est un chèque et il a été déposé en banque
    {
        UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier les données comptables de ce paiement"),
                                tr("Le chèque a été déposé en banque!"));
        m_modiflignerecettepossible = false;
    }
    else
    {
        bool Consolide = false;
        requete = "SELECT idLigne, idCompte, idDep, idRec, idRecSpec,"                      // 0, 1, 2, 3, 4
                  " idRemCheq, LigneDate, LigneLibelle, LigneMontant, LigneDebitCredit,"    // 5, 6, 7, 8, 9
                  " LigneTypeOperation, LigneConsolide"                                     // 10, 11
                  " FROM " TBL_LIGNESCOMPTES
                  " WHERE idRec = " + QString::number(m_idrecette) + " and LigneDebitCredit = 1";
        QVariantList consdata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
        if (m_ok && consdata.size() > 0)
            Consolide = (consdata.at(11).toInt() == 1);
        if (!Consolide)
        {
            requete = "SELECT idLigne FROM " TBL_ARCHIVESBANQUE " WHERE idRec = " + QString::number(m_idrecette);
            QVariantList ligndata = db->getFirstRecordFromStandardSelectSQL(requete, m_ok);
            if (m_ok && ligndata.size() > 0)
                Consolide = true;
        }

        if (Consolide)
        {
            UpMessageBox::Watch(this,tr("Vous ne pourrez pas modifier les données comptables de ce paiement"),
                                     tr("Le virement a été enregistré en banque!"));
            m_modiflignerecettepossible = false;
        }
        else
        {
            // nettoyer LinesRecettes et le mettre en mémoire au cas où on devrait le restaurer
            m_lignerecetteamodifier.clear();
            m_lignerecetteamodifier << QString::number(m_idrecette);                                            //idRecette
            m_lignerecetteamodifier << QString::number(rec->iduser());                                        //idUser
            if (!rec->date().isValid())                                                                     //DatePaiement
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + rec->date().toString("yyyy-MM-dd") + "'";
            if (!rec->dateenregistrement().isValid())                                                       //DateEnregistrement
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + rec->dateenregistrement().toString("yyyy-MM-dd") + "'";
            m_lignerecetteamodifier << QString::number(rec->montant(),'f',2);                                 //Montant
            if (rec->modepaiement() == "")                                                                  //ModePaiement
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + rec->modepaiement() + "'";
            if (rec->tireurcheque() == "")                                                                  //TireurCheque
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + Utils::correctquoteSQL(rec->tireurcheque()) + "'";
            if (rec->compteid() == 0)                                                                       //CompteVirement
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << QString::number(rec->compteid()) ;
            if (rec->banquecheaque() == "")                                                                 //BanqueCheque
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + Utils::correctquoteSQL(rec->banquecheaque()) + "'";
            if (!rec->istierspayant())                                                                      //TiersPayant
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'O'";
            if (rec->payeur() == "")                                                                        //NomTiers
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + Utils::correctquoteSQL(rec->payeur()) + "'";
            if (rec->commission() == 0.0)                                                                   //Commission
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << QString::number(rec->commission(), 'f', 2);
            if (rec->monnaie() == "")                                                                       //Monnaie
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "'" + rec->monnaie() + "'";
            if (rec->idremisecheque() == 0)                                                                 //IdRemise
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << QString::number(rec->idremisecheque());
            if (!rec->ischeqenattente())                                                                    //EnAttente
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << "1";
            if (rec->iduserenregistreur() == 0)                                                             //EnregistrePar
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << QString::number(rec->iduserenregistreur());
            if (rec->typerecette() == 0)                                                                    //TypeRecette
                m_lignerecetteamodifier << "null";
            else
                m_lignerecetteamodifier << QString::number(rec->typerecette());
            requete = "DELETE FROM " TBL_RECETTES " WHERE idRecette = " + QString::number(m_idrecette);
            db->StandardSQL(requete);

            // nettoyer LignesComptes et le mettre en mémoire au cas où on devrait le restaurer
            m_lignecompteamodifier.clear();
            if (consdata.size() > 0)
            {
                m_lignecompteamodifier << consdata.at(0).toString();                                         //idLigne
                m_lignecompteamodifier << consdata.at(1).toString();                                         //idCompte
                if (consdata.at(2).isNull())                                                               //idDep
                    m_lignecompteamodifier << "null";
                else
                    m_lignecompteamodifier << consdata.at(2).toString();
                if (consdata.at(3).isNull())                                                               //idRec
                    m_lignecompteamodifier << "null";
                else
                    m_lignecompteamodifier << consdata.at(3).toString();
                if (consdata.at(4).isNull())                                                               //idRecSpec
                    m_lignecompteamodifier << "null";
                else
                    m_lignecompteamodifier << consdata.at(4).toString();
                if (consdata.at(5).isNull())                                                               //idRemCheq
                    m_lignecompteamodifier << "null";
                else
                    m_lignecompteamodifier << consdata.at(5).toString();
                m_lignecompteamodifier << "'" + consdata.at(6).toDate().toString("yyyy-MM-dd") + "'";        //LigneDate
                m_lignecompteamodifier << "'" + Utils::correctquoteSQL(consdata.at(7).toString()) + "'";    //Lignelibelle
                m_lignecompteamodifier << consdata.at(8).toString();                                         //LigneMontant
                m_lignecompteamodifier << consdata.at(9).toString();                                         //LigneDebiCredit
                m_lignecompteamodifier << "'" + Utils::correctquoteSQL(consdata.at(10).toString()) + "'";   //LigneTypeOperation
                if (consdata.at(11).toInt() == 0)                                                          //LigneConsolide
                    m_lignecompteamodifier << "null";
                else
                    m_lignecompteamodifier << consdata.at(11).toString();


                requete = "DELETE FROM " TBL_LIGNESCOMPTES " WHERE idRec = " + QString::number(m_idrecette) + " AND LigneDebitCredit = 1";
                db->StandardSQL(requete);

            }
            //Nettoyer Depenses  et comptes au besoin si une commission avait été enregistrée
            if (rec->commission() > 0)
            {
                  if (mp == "V")
                {
                    //Nettoyer et mettre en mémoire les commissions et les dépenses correspondantes éventuelles
                      m_lignecommissioncompteamodifier.clear();
                      QString CommCompterequete = "SELECT idLigne, idCompte, idDep, idRec, LigneDate,"
                                                  " LigneLibelle, LigneMontant, LigneDebitCredit, LigneTypeOperation, LigneConsolide"
                                                  " FROM " TBL_LIGNESCOMPTES
                                                  " WHERE idRec = " + QString::number(m_idrecette) + " and idDep > 0";
                      QVariantList commdata = db->getFirstRecordFromStandardSelectSQL(CommCompterequete, m_ok);
                      if (m_ok && commdata.size() > 0)
                      {
                          m_lignecommissioncompteamodifier << commdata.at(0).toString();                                      //idLigne
                          m_lignecommissioncompteamodifier << commdata.at(1).toString();                                      //idCompte
                          m_lignecommissioncompteamodifier << commdata.at(2).toString();                                      //idDep
                          m_lignecommissioncompteamodifier << commdata.at(3).toString();                                      //idRec
                          m_lignecommissioncompteamodifier << "null";                                                                         //idRecSpec
                          m_lignecommissioncompteamodifier << "null";                                                                         //idRemCheq
                          m_lignecommissioncompteamodifier << "'" + commdata.at(4).toDate().toString("yyyy-MM-dd") + "'";     //LigneDate
                          m_lignecommissioncompteamodifier << "'" + Utils::correctquoteSQL(commdata.at(5).toString()) + "'"; //Lignelibelle
                          m_lignecommissioncompteamodifier << commdata.at(6).toString();                                      //LigneMontant
                          m_lignecommissioncompteamodifier << commdata.at(7).toString();                                      //LigneDebiCredit
                          m_lignecommissioncompteamodifier << "'" + Utils::correctquoteSQL(commdata.at(8).toString()) + "'"; //LigneTypeOperation
                          if (commdata.at(9).toInt() == 0)
                              m_lignecommissioncompteamodifier << "null";
                          else
                              m_lignecommissioncompteamodifier << commdata.at(9).toString();                                  //LigneConsolide

                          requete = "DELETE FROM " TBL_LIGNESCOMPTES " WHERE idligne = " + commdata.at(0).toString();
                          db->StandardSQL(requete);
                      }

                      m_lignedepenseamodifier.clear();
                      QString Depenserequete = "select idDep, idUser, DateDep, Reffiscale, Objet,"
                                               " Montant, Famfiscale, Nooperation, Monnaie, idRec,"
                                               " ModePaiement, Compte, NoCheque, idFacture from " TBL_DEPENSES
                                               " where idDep = " + commdata.at(2).toString();

                      QVariantList depdata = db->getFirstRecordFromStandardSelectSQL(Depenserequete, m_ok);
                      if (m_ok && depdata.size() > 0)
                      {
                          m_lignedepenseamodifier << depdata.at(0).toString();                                            //idDep
                          m_lignedepenseamodifier << depdata.at(1).toString();                                            //idUser
                          if (depdata.at(2).toString().isEmpty())
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << "'" + depdata.at(2).toDate().toString("yyyy-MM-dd") + "'";       //DateDep
                          if (depdata.at(3).toString().isEmpty())
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << "'" + Utils::correctquoteSQL(depdata.at(3).toString()) + "'";    //RefFiscale
                          if (depdata.at(4).toString().isEmpty())
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << "'" + Utils::correctquoteSQL(depdata.at(4).toString()) + "'";    //Objet
                          m_lignedepenseamodifier << depdata.at(5).toString();                                            //Montant
                          if (depdata.at(6).toString().isEmpty())
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << "'" + Utils::correctquoteSQL(depdata.at(6).toString()) + "'";    //FamFiscale
                          if (depdata.at(7).toInt() == 0)
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << depdata.at(7).toString();                                        //Nooperation
                          if (depdata.at(8).toString().isEmpty())
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << "'" + Utils::correctquoteSQL(depdata.at(8).toString()) + "'";    //Monnaie
                          if (depdata.at(9).toInt() == 0)
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << depdata.at(9).toString();                                        //idRec
                          if (depdata.at(10).toString().isEmpty())
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << "'" + Utils::correctquoteSQL(depdata.at(10).toString()) + "'";   //ModePaiement
                          if (depdata.at(11).toInt() == 0)
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << depdata.at(11).toString();                                       //Compte
                          if (depdata.at(12).toInt() == 0)
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << depdata.at(12).toString();                                       //NoCheque
                          if (depdata.at(13).toInt() == 0)
                              m_lignedepenseamodifier << "null";
                          else
                              m_lignedepenseamodifier << depdata.at(13).toString();                                       //idFacture

                          requete = "DELETE FROM " TBL_DEPENSES " WHERE idDep = " + depdata.at(0).toString();
                          db->StandardSQL(requete);
                      }
                  }
            }
            ui->SupprimupPushButton->setVisible(true);
        }
    }

    // Nettoyer LignesPaiements
    requete = "DELETE FROM " TBL_LIGNESPAIEMENTS " WHERE idRecette = " + QString::number(m_idrecette);
    db->StandardSQL(requete);

    m_listidactes = m_listactesamodifier;
    RemplitLesTables();
    RegleAffichageFiche();
    ui->ComptesupComboBox->setCurrentIndex(ui->ComptesupComboBox->findData(rec->compteid()));
    ui->dateEdit->setDate(rec->date());
    delete rec;
    RegleAffichageTypePaiementframe(false);
    RegleAffichageTypePaiementframe(true,false);
    for (int i = 0 ; i < ui->DetailupTableWidget->rowCount();i++)
    {
        QList<QTableWidgetItem*> items;
        items = ui->DetailupTableWidget->findItems(QString::number(m_listactesamodifier.at(i)),Qt::MatchExactly);
        int ik = items.at(0)->row();
        QString B = QLocale().toString(m_montantactesamodifier.at(i).toDouble(),'f',2);
        UpLineEdit* Paye = dynamic_cast<UpLineEdit*>(ui->DetailupTableWidget->cellWidget(ik,ui->DetailupTableWidget->columnCount()-2));
        if (Paye)
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
    else
    {
        ui->TireurChequelineEdit->setEnabled(false);
        ui->Tireurlabel->setEnabled(false);
        ui->BanqueChequecomboBox->setEnabled(false);
        ui->Banquelabel->setEnabled(false);
    }
    //qDebug() << ui->ComptesupComboBox->currentText() + " - " + ui->ComptesupComboBox->currentData().toString();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Supprime de la liste des actes à afficher les actes verrouillés par d'autres utilisateur ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::NettoieVerrousListeActesAAfficher() //TODO pasfini
{
    if (m_listidactes.size() > 0)
    {
        for (int i=0; i < m_listidactes.size(); i++)
        {
            QString ChercheVerrou = "SELECT " CP_LOGIN_USR " FROM " TBL_VERROUCOMPTAACTES " ver, " TBL_UTILISATEURS " uti," TBL_ACTES " act"
                    " WHERE act." CP_IDACTE_ACTES " = "  + QString::number(m_listidactes.at(i)) +
                    " AND ver.idActe = act." CP_IDACTE_ACTES
                    " AND PosePar = uti." CP_ID_USR ;
            QVariantList usrdata = db->getFirstRecordFromStandardSelectSQL(ChercheVerrou, m_ok);
         }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Supprime les éventuels verrous laissés par erreur dans VerrousCompta ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::NettoieVerrousCompta()
{
    QString NettoieVerrousComptaActesRequete = "delete from " TBL_VERROUCOMPTAACTES " where PosePar = " + QString::number(m_userconnected->id()) + " or PosePar is null";
    db->StandardSQL(NettoieVerrousComptaActesRequete);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Pose un verrou sur un acte ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::PoseVerrouCompta(int ActeAVerrouiller)
{
    QString req = "select idActe from " TBL_VERROUCOMPTAACTES " where idActe = " + QString::number(ActeAVerrouiller);
    //UpMessageBox::Watch(this,verrourequete);
    QVariantList verroudata = db->getFirstRecordFromStandardSelectSQL(req,m_ok);
    if (m_ok && verroudata.size() == 0)
    {
        QString VerrouilleEnreg= "INSERT INTO " TBL_VERROUCOMPTAACTES
                " (idActe,DateTimeVerrou, PosePar)"
                " VALUES ("  + QString::number(ActeAVerrouiller) +
                ", NOW() ,"  + QString::number(m_userconnected->id()) + ")";
        db->StandardSQL(VerrouilleEnreg);
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Retire un verrou sur un acte ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RetireVerrouCompta(int ActeADeverrouiller)
{
    QString VerrouilleEnreg= "DELETE FROM " TBL_VERROUCOMPTAACTES
            " WHERE idActe = " + QString::number(ActeADeverrouiller);
    db->StandardSQL(VerrouilleEnreg);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Banques dans le combobox Banques --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::ReconstruitListeBanques()
{
    ui->BanqueChequecomboBox->clear();
    foreach (Banque* bq, *Datas::I()->banques->banques())
        ui->BanqueChequecomboBox->addItem(bq->nomabrege());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Banques dans le combobox Banques --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::ReconstruitListeTiers()
{
    ui->TierscomboBox->clear();
    foreach (Tiers *trs, *Datas::I()->tierspayants->tierspayants())
        ui->TierscomboBox->addItem(trs->nomtiers());
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Redessine les infos de paiement en fonction du mode de paiement choisi -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RegleAffichageTypePaiementframe(bool VerifierEmetteur, bool AppeleParClicK)
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
        ui->TiersWidget->setVisible(false);
        ui->BanqueWidget->setVisible(false);
        ui->TireurWidget->setVisible(false);
        ui->CommissionWidget->setVisible(false);
        ui->CompteWidget->setVisible(false);
        ui->dateEdit->setVisible(false);

        if (m_mode == Accueil)
        {
            ui->TypePaiementframe->setVisible(false);
            ui->MontantlineEdit->setText("0,00");
            ui->TierscomboBox->clearEditText();
            ui->BanqueChequecomboBox->clearEditText();
            ui->TireurChequelineEdit->clear();
            ui->ComptesupComboBox->clearEditText();
            ui->CommissionlineEdit->clear();
            ui->dateEdit->setDate(QDate::currentDate());
            ui->TotallineEdit->setText("0,00");
        }
        else
        {
            ui->TypePaiementframe->setVisible(true);
            ui->PaiementgroupBox->setVisible(true);
            ui->MontantWidget->setVisible(true);
            ui->dateEdit->setVisible(true);
            switch (m_mode) {
            case EnregistrePaiementTiers:
            {
                ui->TiersWidget->setVisible(true);
                ui->PaiementgroupBox->setFocusProxy(ui->VirementradioButton);
                break;
            }
            case VoirListePaiementsTiers:
            {
                ui->VirementradioButton->setVisible(true);
                break;
            }
            default:
                break;
            }
        }
    }
    else  // c'est le click sur un des radiobutton qui a appelé la méthode
    {
        if (Emetteur == ui->ChequeradioButton)
        {
            ui->TireurWidget        ->setVisible(true);
            ui->BanqueWidget        ->setVisible(true);
            ui->TiersWidget         ->setVisible(true);
            ui->MontantWidget     ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->CompteWidget        ->setVisible(false);
            // Si la Table Details n'est pas vide , on récupère le nom du premier de la liste pour remplir la case Tireur du chèque;
            if (m_mode == EnregistrePaiementTiers)
            {
                if (AppeleParClicK)
                    ui->TireurChequelineEdit->setText(ui->TierscomboBox->currentText());
            }
            ui->CommissionWidget    ->setVisible(false);

            if (m_mode == EnregistrePaiementTiers && AppeleParClicK)
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
        else if (Emetteur == ui->EspecesradioButton)
        {
            ui->TireurWidget        ->setVisible(false);
            ui->BanqueWidget        ->setVisible(false);
            ui->TiersWidget         ->setVisible(false);
            ui->MontantWidget       ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->CompteWidget        ->setVisible(false);
            ui->CommissionWidget    ->setVisible(false);
            if (m_mode == EnregistrePaiementTiers && AppeleParClicK)
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
        else if (Emetteur == ui->VirementradioButton)
        {
            ui->TireurWidget        ->setVisible(false);
            ui->BanqueWidget        ->setVisible(false);
            ui->TiersWidget         ->setVisible(true);
            ui->MontantWidget       ->setVisible(true);
            ui->Montantlabel        ->setEnabled(true);
            ui->MontantlineEdit     ->setEnabled(true);
            ui->CompteWidget        ->setVisible(true);
            ui->CommissionWidget    ->setVisible(true);
        }
        ui->OKupPushButton->setEnabled(true);
    }

    ui->TypePaiementframe   ->setEnabled(m_mode == EnregistrePaiementTiers);
    ui->PaiementgroupBox    ->setVisible(m_mode != Accueil);
    switch (m_mode) {
    case EnregistrePaiementTiers:
    {
        ui->OKupPushButton      ->setEnabled(QLocale().toDouble(ui->MontantlineEdit->text()) > 0 || (Emetteur !=Q_NULLPTR) || ui->DetailupTableWidget->rowCount() > 0);
        ui->PaiementgroupBox    ->setEnabled(m_modiflignerecettepossible);
        ui->MontantlineEdit     ->setEnabled(m_modiflignerecettepossible);
        ui->Montantlabel        ->setEnabled(m_modiflignerecettepossible);
        ui->CommissionlineEdit  ->setEnabled(m_modiflignerecettepossible);
        ui->Commissionlabel     ->setEnabled(m_modiflignerecettepossible);
        ui->dateEdit            ->setEnabled(m_modiflignerecettepossible);
        break;
    }
    case VoirListePaiementsTiers:
    {
        ui->OKupPushButton      ->setEnabled(ui->ListeupTableWidget->selectedRanges().size() > 0);
        break;
    }
    case Accueil:
    {
        ui->OKupPushButton->setEnabled(true);
        break;
    }
    }
    if (!ui->VirementradioButton->isChecked())
        ui->CommissionlineEdit->setText("0,00");
}


/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reinitialise tous les champs de saisie --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RemetToutAZero()
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
    m_listidactes.clear();
    RemplitLesTables();
    ui->OKupPushButton              ->setEnabled(false);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les TableWidget ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RemplirTableWidget(UpTableWidget *TableARemplir, TypeTable Type, QList<QVariantList> reclist, bool AvecUpcheckBox, Qt::CheckState CheckedOuPas)
{
    QTableWidgetItem    *pItem1, *pItem2, *pItem3, *pItem4, *pItem5, *pItem6, *pItem7;
    QString             A;
    QFontMetrics        fm(qApp->font());

    disconnect (TableARemplir, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);
    TableARemplir->clearContents();
    TableARemplir->verticalHeader()->hide();

    // cette ligne est nécessaire pour éviter un pb d'affichage des tables quand on les redéfinit
    TableARemplir->setRowCount(0);
    TableARemplir->setRowCount(reclist.size());

    if (Type == Actes)                                                               // Table affichant des actes
        for (int i = 0; i < reclist.size(); i++)
        {
            QTableWidgetItem    *pItem8, *pItem9;
            int col = 0;

            A = reclist.at(i).at(0).toString();                                                 // idACte
            pItem1 = new QTableWidgetItem() ;
            pItem1->setText(A);
            TableARemplir->setItem(i,col,pItem1);
            col++;

            if (AvecUpcheckBox)
            {
                UpCheckBox *CheckItem = new UpCheckBox();
                if (m_listidactes.contains(reclist.at(i).at(0).toInt()))
                    CheckItem->setCheckState(Qt::Checked);
                else
                    CheckItem->setCheckState(CheckedOuPas);
                CheckItem->setRowTable(i);
                CheckItem->setFocusPolicy(Qt::NoFocus);
                connect(CheckItem, &UpCheckBox::uptoggled, this, &dlg_paiementtiers::RenvoieRangee);
                CheckItem->installEventFilter(this);
                TableARemplir->setCellWidget(i,col,CheckItem);
            }
            if (AvecUpcheckBox)
                col++;

            A = reclist.at(i).at(1).toDate().toString(tr("dd-MM-yyyy"));                            // Date
            pItem2 = new QTableWidgetItem() ;
            pItem2->setText(A);
            TableARemplir->setItem(i,col,pItem2);
            col++;

            A = reclist.at(i).at(2).toString() + " " + reclist.at(i).at(3).toString();          // Nom Prenom
            pItem3 = new QTableWidgetItem() ;
            pItem3->setText(A);
            TableARemplir->setItem(i,col,pItem3);
            col++;

            A = reclist.at(i).at(4).toString();                                                 // Cotation
            pItem4 = new QTableWidgetItem() ;
            pItem4->setText(A);
            TableARemplir->setItem(i,col,pItem4);
            col++;

            QString Montant;
            if (reclist.at(i).at(6).toString() == "F")
                Montant = QLocale().toString(reclist.at(i).at(5).toDouble()/6.55957,'f',2);                         // Montant en F converti en euros
            else
                Montant = QLocale().toString(reclist.at(i).at(5).toDouble(),'f',2);                                 // Montant
            pItem5 = new QTableWidgetItem() ;
            pItem5->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
            pItem5->setText(Montant);
            TableARemplir->setItem(i,col,pItem5);
            col++;

            if (m_mode == EnregistrePaiementTiers)
            {
                A = reclist.at(i).at(8).toString();
                pItem6 = new QTableWidgetItem() ;
                pItem6->setTextAlignment(Qt::AlignCenter);
                pItem6->setText(A);
                // col = 6 / Type tiers
                TableARemplir->setItem(i,col,pItem6);                                           // Tiers
                col++;
                if (TableARemplir == ui->ListeupTableWidget)
                {
                    if (reclist.at(i).at(6).toString() == "F")
                        A = QLocale().toString((QLocale().toDouble(Montant) - reclist.at(i).at(7).toDouble())/6.55957,'f',2);   // Payé en F converti en euros
                    else
                        A = QLocale().toString(QLocale().toDouble(Montant) - reclist.at(i).at(7).toDouble(),'f',2);             // Payé
                    pItem7 = new QTableWidgetItem;
                    pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    pItem7->setText(A);
                    // col = 7 Reste du
                    TableARemplir->setItem(i,col,pItem7);
                    col++;

                    pItem8 = new QTableWidgetItem;
                    pItem8->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    if (reclist.at(i).at(6).toString() == "F")
                        A = QLocale().toString(reclist.at(i).at(7).toDouble()/6.55957,'f',2);
                    else
                        A = QLocale().toString(reclist.at(i).at(7).toDouble(),'f',2);
                    pItem8->setText(A);
                    // col = 8 Payé
                    TableARemplir->setItem(i,col,pItem8);                                           // Reste dû
                }
                else
                {
                    QDoubleValidator *val;
                    if (reclist.at(i).at(6).toString() == "F")
                        A = QLocale().toString(reclist.at(i).at(7).toDouble()/6.55957,'f',2);                     // Reste dû en F converti en euros
                    else
                        A = QLocale().toString(reclist.at(i).at(7).toDouble(),'f',2);                             // Reste dû
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
                    LignePaye->setRow(i);
                    LignePaye->setColumn(col);
                    LignePaye->setStyleSheet("UpLineEdit {background-color:white; border: 0px solid rgb(150,150,150);border-radius: 0px;}"
                                             "UpLineEdit:focus {border: 0px solid rgb(164, 205, 255);border-radius: 0px;}");
                    connect (LignePaye, &QLineEdit::textChanged, this, &dlg_paiementtiers::CalculTotalDetails);
                    LignePaye->installEventFilter(this);
                    TableARemplir->setCellWidget(i,col,LignePaye);
                }
                col++;
            }

            if (m_mode == VoirListePaiementsTiers)
            {
                pItem6 = new QTableWidgetItem() ;
                if (TableARemplir == ui->ListeupTableWidget)
                {
                    A = reclist.at(i).at(8).toString();
                    if (A == "T") A = reclist.at(i).at(9).toString();
                    if (A == "E") A = tr("Espèces");
                    if (A == "C") A = tr("Chèque");
                    if (A == "CB")  A = tr("Carte bancaire");
                    pItem6->setTextAlignment(Qt::AlignCenter);
                    pItem6->setText(A);
                    TableARemplir->setItem(i,col,pItem6);                                                       // Type paiement
                    col++;
                    A = QLocale().toString(reclist.at(i).at(7).toDouble(),'f',2);
                    pItem7 = new QTableWidgetItem;
                    pItem7->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    pItem7->setText(A);
                    TableARemplir->setItem(i,col,pItem7);                                                       // Payé
                    col++;
                }
                if (TableARemplir == ui->DetailupTableWidget)
                {
                    A = reclist.at(i).at(9).toString();
                    if (A == "CB")  A = tr("Carte bancaire");
                    pItem6->setTextAlignment(Qt::AlignCenter);
                    pItem6->setText(A);
                    TableARemplir->setItem(i,col,pItem6);                                                       // Type paiement
                    col++;
                    QString Paye;
                    if (reclist.at(i).at(6).toString() == "F")
                        Paye = QLocale().toString(reclist.at(i).at(7).toDouble()/6.55957,'f',2);                  // Paye en F converti en euros
                    else
                        Paye = QLocale().toString(reclist.at(i).at(7).toDouble(),'f',2);                          // Paye
                    QString TotalPaye;
                    if (reclist.at(i).at(6).toString() == "F")
                        TotalPaye = QLocale().toString(reclist.at(i).at(10).toDouble()/6.55957,'f',2);            // TotalPaye en F converti en euros
                    else
                        TotalPaye = QLocale().toString(reclist.at(i).at(10).toDouble(),'f',2);                    // TotalPaye
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

            A = reclist.at(i).at(1).toDate().toString("yyyy-MM-dd");                                        // ClassementparDate
            pItem9 = new QTableWidgetItem() ;
            pItem9->setText(A);
            TableARemplir->setItem(i,col,pItem9);
        }

    else if (Type == Paiements)                                                                   // Table affichant des paiements
        for (int i = 0; i < reclist.size(); i++)
        {
            int col = 0;

            A = reclist.at(i).at(0).toString();                                                 // idRecette
            pItem1 = new QTableWidgetItem() ;
            pItem1->setText(A);
            TableARemplir->setItem(i,col,pItem1);
            col++;

            if (AvecUpcheckBox)
            {
                UpCheckBox *CheckItem = new UpCheckBox();
                if (m_listidactes.contains(reclist.at(i).at(0).toInt()))
                    CheckItem->setCheckState(Qt::Checked);
                else
                    CheckItem->setCheckState(CheckedOuPas);
                CheckItem->setRowTable(i);
                CheckItem->setFocusPolicy(Qt::NoFocus);
                connect(CheckItem, &UpCheckBox::uptoggled, this, &dlg_paiementtiers::RenvoieRangee);
                CheckItem->installEventFilter(this);
                TableARemplir->setCellWidget(i,col,CheckItem);
                col++;
            }

            A = reclist.at(i).at(1).toDate().toString(tr("dd-MM-yyyy"));                            // Date
            pItem2 = new QTableWidgetItem() ;
            pItem2->setText(A);
            TableARemplir->setItem(i,col,pItem2);
            col++;

            QString mp = reclist.at(i).at(4).toString();
            if (mp == "V" && reclist.at(i).at(9).toString() == "CB")
                A = tr("Virement carte bancaire");
            else
                A = reclist.at(i).at(9).toString();                                                 // Payeur
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
                A = reclist.at(i).at(16).toDate().toString(tr("dd-MM-yyyy"));                        // Date validation
            else
                A = reclist.at(i).at(2).toDate().toString(tr("dd-MM-yyyy"));
            pItem5 = new QTableWidgetItem() ;
            pItem5->setText(A);
            TableARemplir->setItem(i,col,pItem5);
            col++;

            if (reclist.at(i).at(11).toString() == "F")
                A = QLocale().toString(reclist.at(i).at(17).toDouble()/6.55957,'f',2);            // Payé en F converti en euros
            else
                A = QLocale().toString(reclist.at(i).at(17).toDouble(),'f',2);                     // Payé
            pItem6 = new QTableWidgetItem() ;
            pItem6->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
            pItem6->setText(A);
            TableARemplir->setItem(i,col,pItem6);
            col++;

            A = reclist.at(i).at(1).toDate().toString("yyyy-MM-dd");                             // ClassementparDate
            pItem7 = new QTableWidgetItem() ;
            pItem7->setText(A);
            TableARemplir->setItem(i,col,pItem7);
        }
    TableARemplir->setAllRowHeight(int(fm.height()*1.1));
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplir les tables en fonction du mode appelé ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::RemplitLesTables()
{
    QString             requete;
    disconnect (ui->ListeupTableWidget, &QTableWidget::itemSelectionChanged, this, Q_NULLPTR);

    QString user =  " AND act.UserComptable = "  + QString::number(m_useracrediter->id()) + "\n";

    switch (m_mode)
    {

    /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Enregistrement d'un paiement par tiers --------------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    case EnregistrePaiementTiers:
    {
        /*
        1. Remplissage ui->ListeupTableWidget -- tous les paiemenst en attente en dehors de ceux de la salle d'attente
        càd ceux:
        . pour lesquels la somme des montants versés est inférieure au prix de l'acte
        . ou dont le type de paiement enregistré est "tiers"
        */
        DefinitArchitectureTableView(ui->ListeupTableWidget, Actes);
        requete =   "select * from (\n"
                    "SELECT act." CP_IDACTE_ACTES ", " CP_DATE_ACTES ", PatNom, PatPrenom, " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", " CP_MONTANT_ACTES " -SUM(Paye) as tot, Tiers\n"
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ, " TBL_LIGNESPAIEMENTS " lig\n"
                    " WHERE act." CP_IDACTE_ACTES " = typ.idActe\n"
                    " AND lig.idActe = act." CP_IDACTE_ACTES "\n"
                    " AND TypePaiement = 'T'\n"
                    " AND act." CP_IDPAT_ACTES " = pat.idPat\n";
        requete +=  user;
        requete +=  " group by act." CP_IDACTE_ACTES ") as mar\n"
                    " where tot > 0\n"
                    " union\n\n"                                    // tous les actes enregistrés en tiers pour lesquels le paiement est incomplet

                    " (SELECT act." CP_IDACTE_ACTES ", " CP_DATE_ACTES ", PatNom, PatPrenom, " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", " CP_MONTANT_ACTES " as tot, Tiers\n"
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ\n"
                    " WHERE act." CP_IDACTE_ACTES " = typ.idActe\n"
                    " AND act." CP_IDACTE_ACTES " not in (select idacte from " TBL_LIGNESPAIEMENTS ")\n"
                    " AND TypePaiement = 'T'\n"
                    " AND act." CP_IDPAT_ACTES " = pat.idPat\n"
                    " AND act." CP_IDUSERCOMPTABLE_ACTES " = "  + QString::number(m_useracrediter->id()) + "\n"
                    ")\n"
                    " order by " CP_DATE_ACTES " desc, PatNom, PatPrenom";

        //UpMessageBox::Watch(this,requete);

        QList<QVariantList> actlist = db->StandardSelectSQL(requete,m_ok);
        RemplirTableWidget(ui->ListeupTableWidget, Actes, actlist, true, Qt::Unchecked);
        ui->DetailupTableWidget->setRowCount(0);
        ui->DetailupTableWidget->setColumnCount(0);
        ui->DetailupTableWidget->horizontalHeader()->hide();

        //Remplissage de la table Details en cas de modification de paiement en cours
        DefinitArchitectureTableView(ui->DetailupTableWidget,Actes);
        if (m_listidactes.size() > 0 && m_modifpaiementencours)
        {
            NettoieVerrousListeActesAAfficher();
            QString CriteresRequete = " AND act.idActe IN (";
            for (int u = 0; u < m_listidactes.size(); u++)
            {
                PoseVerrouCompta(m_listidactes.at(u));
                if (u < m_listidactes.size()-1)
                    CriteresRequete += QString::number(m_listidactes.at(u)) + ", ";
                else
                    CriteresRequete += QString::number(m_listidactes.at(u));
             }
            requete =
                    "select * from (\n"
                    "SELECT act." CP_IDACTE_ACTES ", " CP_DATE_ACTES ", PatNom, PatPrenom, " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", " CP_MONTANT_ACTES " -SUM(Paye) as tot, Tiers\n"
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ, " TBL_LIGNESPAIEMENTS " lig\n"
                    " WHERE act." CP_IDACTE_ACTES " = typ.idActe\n"
                    " AND lig.idActe = act." CP_IDACTE_ACTES "\n"
                    " AND act.idPat = pat.idPat\n"
                    + CriteresRequete +
                    ")\n group by act." CP_IDACTE_ACTES ") as mar\n"

                    " union\n\n"

                    " (SELECT act." CP_IDACTE_ACTES ", " CP_DATE_ACTES ", PatNom, PatPrenom, " CP_COTATION_ACTES ", " CP_MONTANT_ACTES ", " CP_MONNAIE_ACTES ", " CP_MONTANT_ACTES " as tot, Tiers\n"
                    " FROM " TBL_ACTES " act, " TBL_PATIENTS " pat, " TBL_TYPEPAIEMENTACTES " typ\n"
                    " WHERE act." CP_IDACTE_ACTES " = typ.idActe\n"
                    " AND act." CP_IDACTE_ACTES " not in (select idacte from " TBL_LIGNESPAIEMENTS ")\n"
                    + CriteresRequete +
                    ")\n AND act." CP_IDPAT_ACTES " = pat.idPat)\n"
                    " order by " CP_DATE_ACTES " desc, PatNom, PatPrenom";

            //UpMessageBox::Watch(this,requete);
            QList<QVariantList> detlist = db->StandardSelectSQL(requete,m_ok);
            RemplirTableWidget(ui->DetailupTableWidget, Actes, detlist, true, Qt::Checked);
        }
        break;
    }

     /* -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Modification d'un paiement par tiers ------------------------------------------------------------------------------------------------------------------------------------------
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    case VoirListePaiementsTiers:      // on reconstruit la liste des tiers
    {
        // On sélectionne tous les paiements sans exception
        DefinitArchitectureTableView(ui->ListeupTableWidget,Paiements);
        requete =   "SELECT idRecette, DatePaiement, DateEnregistrement, Montant, ModePaiement, TireurCheque, CompteVirement, BanqueCheque, TiersPayant, NomTiers, Commission, Monnaie, idRemise, EnAttente, EnregistrePar, TypeRecette, RCDate, Montant as Paye FROM " TBL_RECETTES
                "\n LEFT OUTER JOIN (SELECT RCDate, idRemCheq FROM " TBL_REMISECHEQUES ") AS rc\n"
                " ON rc.idRemCheq = idRemise\n"
                " WHERE idUser = " + QString::number(m_useracrediter->id()) +
                "\n AND TiersPayant = 'O'\n"
                " ORDER BY DatePaiement DESC, NomTiers";

        //UpMessageBox::Watch(this,requete);
        QList<QVariantList> detpmtlist = db->StandardSelectSQL(requete,m_ok);
        RemplirTableWidget(ui->ListeupTableWidget, Paiements, detpmtlist, false, Qt::Unchecked);

        ui->DetailupTableWidget->setRowCount(0);
        ui->DetailupTableWidget->setColumnCount(0);
        ui->DetailupTableWidget->horizontalHeader()->hide();
        connect (ui->ListeupTableWidget, &QTableWidget::itemSelectionChanged, this, [=] {RenvoieRangee();});
        DefinitArchitectureTableView(ui->DetailupTableWidget,Actes);
        break;
     }
    default:
        break;
    }
    CalculTotalDetails();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Trie une table en remettant à jour les index des UpcheckBox et des UplineEdit ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::TrieListe(QTableWidget *TableATrier )
{
    int ncol = TableATrier->columnCount();
    int ColonneATrier;

    if (m_mode == EnregistrePaiementTiers)
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

    if (m_mode == EnregistrePaiementTiers)
    {
        int ColonneMontantPaye  = TableATrier->columnCount()- 2;
        int NombreRangees       = TableATrier->rowCount();
        for (int i= 0; i < NombreRangees; i++)
        {
            UpCheckBox* Check = dynamic_cast<UpCheckBox*>(TableATrier->cellWidget(i,1));
            //            QString nom = TableATrier->item(i,3)->text();
            if (Check)
            {
                disconnect(Check,       &UpCheckBox::uptoggled, this, &dlg_paiementtiers::RenvoieRangee);    // il faut faire cette manip sinon on n'arrive pas à modifier le checkstate
                Check->setRowTable(i);
                connect(Check,          &UpCheckBox::uptoggled, this, &dlg_paiementtiers::RenvoieRangee);
            }
            if (TableATrier == ui->DetailupTableWidget)
            {
                UpLineEdit* Line = dynamic_cast<UpLineEdit*>(TableATrier->cellWidget(i,ColonneMontantPaye));
                if (Line)
                {
                    disconnect (Line,   &QLineEdit::textChanged, this, &dlg_paiementtiers::CalculTotalDetails);         // même remarque
                    Line->setRow(i);
                    connect (Line,      &QLineEdit::textChanged, this, &dlg_paiementtiers::CalculTotalDetails);
                }
            }
        }
    }
    }

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Verifie la cohérence d'un paiement -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_paiementtiers::VerifCoherencePaiement()
{
    QString Msg = "";
    bool A          = true;
    bool AfficheMsg = true;
    bool Reponse    = true;

    while (Reponse) {
        // On n'a coché aucun mode de paiement
        if (m_modiflignerecettepossible &&
                !(ui->ChequeradioButton->isChecked()
                || ui->EspecesradioButton->isChecked()
                || ui->VirementradioButton->isChecked())
                && m_mode == EnregistrePaiementTiers)
        {
            Msg = tr("Vous avez oublié de cocher le mode de paiement!");
            A = false;
            break;
        }
        if (m_modiflignerecettepossible && (ui->MontantlineEdit->isVisible() && !(QLocale().toDouble(ui->MontantlineEdit->text()) > 0)))
        {
            Msg = tr("Vous avez oublié d'indiquer le montant!");
            ui->MontantlineEdit->setFocus();
            A = false;
            break;
        }
        // Le montant inscrit est supérieur au montant calculé
        if (ui->MontantWidget->isVisible() && (QLocale().toDouble(ui->MontantlineEdit->text()) > QLocale().toDouble(ui->TotallineEdit->text())))
        {
            if (ui->DetailupTableWidget->rowCount() > 0)
                Msg = tr("Le montant enregistré est supérieur au montant total calculé!\nEnregistrer quand même?");
            else
                Msg = tr("Il n'y a aucun acte enregistré!\nEnregistrer quand même?");
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
        if (ui->MontantWidget->isVisible() && (QLocale().toDouble(ui->MontantlineEdit->text()) < QLocale().toDouble(ui->TotallineEdit->text())))
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
                Dlg_Banq = new dlg_gestionbanques(this, Banq.toUpper());
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
        if (m_modiflignerecettepossible && (ui->TireurChequelineEdit->text() == "" && ui->ChequeradioButton->isChecked()))
        {
            Msg = tr("Il manque le nom du tireur du chèque!");
            ui->TireurChequelineEdit->setFocus();
            A = false;
            break;
        }
        if (m_modiflignerecettepossible
                && (
                    ui->TierscomboBox->currentText() == ""
                    && m_mode == EnregistrePaiementTiers
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
        if (m_modiflignerecettepossible && (ui->TierscomboBox->currentText() == "CB" && QLocale().toDouble(ui->CommissionlineEdit->text()) ==  0.0 && ui->VirementradioButton->isChecked()))
        {
            Msg = tr("Vous avez oublié de mentionner le montant de la comission bancaire pour ce paiement par carte de crédit!");
            ui->Commissionlabel->setFocus();
            A = false;
            break;
        }
        // Le montant de la commission dépasse le montant du paiement commission
        if (m_modiflignerecettepossible && (QLocale().toDouble(ui->CommissionlineEdit->text()) > QLocale().toDouble(ui->MontantlineEdit->text())))
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
bool dlg_paiementtiers::VerifVerrouCompta(QTableWidget *TableAVerifier, int Rangee)
{
    if (t_timerafficheacteverrouilleclignotant->isActive())
        return false;
    QString ChercheVerrou = "SELECT " CP_LOGIN_USR " FROM " TBL_VERROUCOMPTAACTES ", " TBL_UTILISATEURS
                     " WHERE idActe = "  + TableAVerifier->item(Rangee,0)->text() +
                     " AND PosePar = " CP_ID_USR ;
    //UpMessageBox::Watch(this,ChercheVerrou);
    QVariantList usrdata = db->getFirstRecordFromStandardSelectSQL(ChercheVerrou,m_ok);
    if (m_ok && usrdata.size() > 0)
    {
        ui->VerrouilleParlabel->setText(tr("Acte Verrouillé par ") + usrdata.at(0).toString());
        ui->VerrouilleParlabel->setStyleSheet("color: magenta");
        ui->VerrouilleParlabel->setVisible(true);
        t_timerafficheacteverrouilleclignotant->start(100);
        connect(t_timerafficheacteverrouilleclignotant, &QTimer::timeout, this, &dlg_paiementtiers::AfficheActeVerrouilleClignotant);
        t_timerafficheacteverrouille->start(2000);
        t_timerafficheacteverrouille->setSingleShot(true);
//        for (int i= 0; i != ui->ListeupTableWidget->rowCount(); i++)
//        {
//            UpCheckBox* Check = dynamic_cast<UpCheckBox*>(ui->ListeupTableWidget->cellWidget(i,1));
//            if (Check)
//                Check->setToggleable(false);
//        }
        QSound::play(NOM_ALARME);
        connect(t_timerafficheacteverrouille, &QTimer::timeout, this, &dlg_paiementtiers::AfficheActeVerrouille);
        return false;
    }
    return true;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Retirer de la table Details un élément décoché et le décocher dans la table Liste ou SalDat où il figure -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_paiementtiers::VideDetailsTable(int Rangee)
{
    bool                    idActeTrouve = false;

    // on récupère l'idActe de l'item concerné
    QString ActeAVirer = ui->DetailupTableWidget->item(Rangee,0)->text();
    //UpMessageBox::Watch(this,ui->DetailupTableWidget->item(Rangee,3)->text()+"\n"+ActeAVirer);
    //on décoche les items correspondants dans ui->ListeupTableWidget et TableSalDat
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
            connect(CheckItem, &UpCheckBox::uptoggled, this, &dlg_paiementtiers::RenvoieRangee);
            CheckItem->installEventFilter(this);
            ui->ListeupTableWidget->removeCellWidget(i,1);
            ui->ListeupTableWidget->setCellWidget(i,1,CheckItem);
            RetireVerrouCompta(ActeAVirer.toInt());
        }
    if (!idActeTrouve) RemplitLesTables();
    // on supprime la rangée de la ui->DetailupTableWidget et on reindexe les upcheckbox et les uplinetext
    ui->DetailupTableWidget->removeRow(Rangee);
    TrieListe(ui->DetailupTableWidget);
    CalculTotalDetails();
    RegleAffichageTypePaiementframe();
}

