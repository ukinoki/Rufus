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

#include "dlg_gestioncomptes.h"
#include "ui_dlg_gestioncomptes.h"
#include "icons.h"


dlg_gestioncomptes::dlg_gestioncomptes(User *user,
                                       bool societe,
                                       QWidget *parent)
                                       : UpDialog(parent),
                                        ui(new Ui::dlg_gestioncomptes)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    m_userencours            = user;

    m_societe                = societe;
    m_affichelesolde         = (m_userencours == Datas::I()->users->userconnected());

    m_comptencours           = Datas::I()->comptes->getById(m_userencours->idcomptepardefaut());

    t_timer                  = new QTimer(this);
    t_timer                  ->start(500);
    connect(t_timer, &QTimer::timeout, this, &dlg_gestioncomptes::Clign);
    ui->CompteFacticePushButton->setVisible(false);

    wdg_buttonframe             = new WidgetButtonFrame(ui->ComptesuptableWidget);
    wdg_buttonframe             ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_nouvbanqupsmallbutton    = new UpSmallButton();
    wdg_nouvbanqupsmallbutton    ->setText(tr("Gestion des organismes bancaires"));
    wdg_nouvbanqupsmallbutton    ->setIcon(Icons::icEuroCount());
    wdg_nouvbanqupsmallbutton    ->setIconSize(QSize(25,25));
    AjouteWidgetLayButtons(wdg_nouvbanqupsmallbutton);
    AjouteLayButtons(UpDialog::ButtonClose);
    CloseButton             ->setText(tr("Fermer"));
    //setStageCount(1);

    connect(CloseButton,                    &QPushButton::clicked,      this, &dlg_gestioncomptes::Fermer);
    connect(ui->OKModifupSmallButton,       &QPushButton::clicked,      this, &dlg_gestioncomptes::ValidCompte);
    connect(ui->AnnulModifupSmallButton,    &QPushButton::clicked,      this, &dlg_gestioncomptes::AnnulModif);
    connect(wdg_nouvbanqupsmallbutton,      &QPushButton::clicked,      this, &dlg_gestioncomptes::Banques);
    connect(wdg_buttonframe,                &WidgetButtonFrame::choix,  this, &dlg_gestioncomptes::ChoixButtonFrame);
    connect(ui->CompteFacticePushButton,    &QPushButton::clicked,      this, &dlg_gestioncomptes::CompteFactice);
    connect(ui->DesactiveComptecheckBox,    &QPushButton::clicked,      this, &dlg_gestioncomptes::DesactiveCompte);

    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->SoldeuplineEdit->setValidator(val);

    setWindowTitle(tr("Comptes bancaires de ") + m_userencours->login());

    ReconstruitComboBanques();

    RemplirTableView();
    ui->Compteframe             ->setEnabled(false);
    ui->OKModifupSmallButton    ->setVisible(false);
    ui->AnnulModifupSmallButton ->setVisible(false);
    ui->idCompteupLineEdit      ->setVisible(false);
    ui->idComptelabel           ->setVisible(false);
    ui->Soldelabel              ->setVisible(m_affichelesolde);
    ui->SoldeuplineEdit         ->setVisible(m_affichelesolde);

    QHBoxLayout *vlay = new QHBoxLayout();
    int marge   = 5;
    int space   = 5;
    vlay        ->setContentsMargins(marge,marge,marge,marge);
    vlay        ->setSpacing(space);
    vlay        ->addWidget(ui->Compteframe);
    vlay        ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding, QSizePolicy::Expanding));
    QHBoxLayout *hlay = new QHBoxLayout();
    hlay        ->setContentsMargins(marge,marge,marge,marge);
    hlay        ->setSpacing(space);
    hlay        ->addWidget(wdg_buttonframe->widgButtonParent());
    hlay        ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    hlay        ->addLayout(vlay);
    dlglayout() ->insertLayout(0,hlay);
    dlglayout() ->setSizeConstraint(QLayout::SetFixedSize);

    if (!m_comptencours)
      {
          m_mode = Nouv;
          NouvCompte();
      }
      else
          m_mode = Norm;
    ui->DesactiveComptecheckBox->setVisible(true);
}

dlg_gestioncomptes::~dlg_gestioncomptes()
{
    delete ui;
}

void dlg_gestioncomptes::closeEvent(QCloseEvent *event)
{
     if (ui->ComptesuptableWidget->rowCount() == 0)
    {
        UpMessageBox msgbox(this);
        UpSmallButton OKBouton(tr("Je confirme"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.setText(tr("Vous n'avez pas de compte bancaire enregistré!"));
        msgbox.setInformativeText(tr("Vous ne pourrez pas utiliser de comptabilité"));
        msgbox.setIcon(UpMessageBox::Warning);
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            event->ignore();
    }
}

void dlg_gestioncomptes::AfficheCompte(QTableWidgetItem *pitem, QTableWidgetItem *)
{
    int idCompte = ui->ComptesuptableWidget->item(pitem->row(),0)->text().toInt();
    m_comptencours = Datas::I()->comptes->getById(idCompte);
    if (!m_comptencours)
        return;
    ui->BanqueupcomboBox            ->setCurrentIndex(ui->BanqueupcomboBox->findData(m_comptencours->idBanque()));
    ui->IBANuplineEdit              ->setText(m_comptencours->iban());
    ui->IntituleCompteuplineEdit    ->setText(m_comptencours->intitulecompte());
    ui->NomCompteAbregeuplineEdit   ->setText(m_comptencours->nomabrege());
    ui->SoldeuplineEdit             ->setText(QLocale().toString(m_comptencours->solde(),'f',2));
    ui->idCompteupLineEdit          ->setText(QString::number(m_comptencours->id()));
    ui->DesactiveComptecheckBox     ->setChecked(m_comptencours->isDesactive());

    wdg_buttonframe->wdg_modifBouton    ->setEnabled(m_comptencours->idUser() == Datas::I()->users->userconnected()->id());
    ui->SoldeuplineEdit         ->setVisible(m_comptencours->idUser() == Datas::I()->users->userconnected()->id());
    ui->Soldelabel              ->setVisible(m_comptencours->idUser() == Datas::I()->users->userconnected()->id());

    /*On ne peut pas supprimer un compte s'il est utilisé ou s'il y a déjà eu des ecritures bancaires*/
    bool autorsupprimer = true;
    for (auto itusr = Datas::I()->users->actifs()->begin(); itusr != Datas::I()->users->actifs()->end();)
    {
        if (itusr.value()->idcomptepardefaut() == idCompte || itusr.value()->idcompteencaissementhonoraires()  == idCompte)
        {
            autorsupprimer = false;
            break;
        }
        ++ itusr;
    }
    if (autorsupprimer)
    {
        bool ok = true;
        QString req = "select " CP_IDCOMPTE_ARCHIVESCPT " from " TBL_ARCHIVESBANQUE
                " where " CP_IDCOMPTE_ARCHIVESCPT " = " + QString::number(idCompte) +
                " union"
                " select " CP_IDCOMPTE_LIGNCOMPTES " from " TBL_LIGNESCOMPTES
                " where " CP_IDCOMPTE_LIGNCOMPTES " = " + QString::number(idCompte) +
                " limit 1";
        autorsupprimer = (db->StandardSelectSQL(req, ok).size() == 0);
    }
    wdg_buttonframe->wdg_moinsBouton->setEnabled(autorsupprimer);
    QString ttip = "";
    if (!autorsupprimer)
        ttip = tr("Impossible de supprimer ce compte") + "\n" + tr("Des écritures  ont été enregistrées") + "\n" + tr("ou il est utilisé par un utilisateur");
    wdg_buttonframe->wdg_moinsBouton->setToolTip(ttip);
}

void dlg_gestioncomptes::AnnulModif()
{
    ui->Compteframe->setEnabled(false);
    m_mode = Norm;
    ui->DesactiveComptecheckBox->setVisible(true);
    if (ui->ComptesuptableWidget->rowCount() > 0)
        AfficheCompte(ui->ComptesuptableWidget->item(ui->ComptesuptableWidget->currentRow(),0),ui->ComptesuptableWidget->item(ui->ComptesuptableWidget->currentRow(),0));
    else
        ui->Compteframe->setVisible(false);
    ui->OKModifupSmallButton->setVisible(false);
    ui->AnnulModifupSmallButton->setVisible(false);
    ui->ComptesuptableWidget->setEnabled(true);
    wdg_buttonframe->setEnabled(true);
    ui->CompteFacticePushButton->setVisible(false);
    ui->ComptesuptableWidget->setFocus();
}

void dlg_gestioncomptes::Banques()
{
    dlg_gestionbanques *Dlg_Banq = new dlg_gestionbanques(this);
    if (Dlg_Banq->exec() == QDialog::Accepted)
        ReconstruitComboBanques();
    delete Dlg_Banq;
}

void dlg_gestioncomptes::DesactiveCompte()
{
    ui->BanqueupcomboBox            ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->IBANuplineEdit              ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->IntituleCompteuplineEdit    ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->NomCompteAbregeuplineEdit   ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->SoldeuplineEdit             ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    if (!ui->DesactiveComptecheckBox->isChecked())
    {
        /*On ne peut pas desactiver un compte si:
        * . il est le seul compte active pour cet utilisateur
        */
        bool ok = true;
        QList<QVariantList> listcomptes = db->SelectRecordsFromTable(QStringList() << CP_ID_COMPTES,
                                                                        TBL_COMPTES, ok,
                                                                        "where " CP_IDUSER_COMPTES " = " + QString::number(m_userencours->id()) + " and " CP_DESACTIVE_COMPTES " is null");
        ui->DesactiveComptecheckBox ->setEnabled(listcomptes.size()>1);
    }    
}

void dlg_gestioncomptes::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        NouvCompte();
        break;
    case WidgetButtonFrame::Modifier:
        ModifCompte();
        break;
    case WidgetButtonFrame::Moins:
        SupprCompte();
        break;
    }
}

void dlg_gestioncomptes::Clign()
{
    m_visible = !m_visible;
    if (m_visible)
        ui->CompteFacticePushButton->setIcon(Icons::icNull());
    else
        ui->CompteFacticePushButton->setIcon(Icons::icHelp());
}

void dlg_gestioncomptes::CompteFactice()
{
    UpMessageBox msgbox(this);
    UpSmallButton OKBouton(tr("Annuler"));
    UpSmallButton RemplirBouton(tr("Utiliser des coordonnées bancaires factices"));
    msgbox.setText(tr("Enregistrement des coordonnées bancaires"));
    msgbox.setInformativeText(tr("Si vous venez de créez ce compte utilisateur et que vous vous enregistrez en tant que libéral,"
                              "il faut indiquer vos coordonnées bancaires pour le bon fonctionnement de la comptabilité\n"
                              "notamment l'édition automatique des remises de chèques.\n"
                              "Si vous souhaitez seulement le tester, le logiciel "
                              "peut remplir les champs avec des coordonnées factices.\n"
                              "Voulez vous remplir automatiquement les champs avec des coordonnées factices?.\n"));
    msgbox.setIcon(UpMessageBox::Info);
    msgbox.addButton(&RemplirBouton, UpSmallButton::STARTBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::CANCELBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton()==&RemplirBouton)
    {
        Banque *newbq = Q_NULLPTR;
        for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
        {
            Banque *bq = const_cast<Banque*>(it.value());
            if (bq->nomabrege() == "PaPRS")
            {
                newbq = bq;
                break;
            }
        }
        if (newbq == Q_NULLPTR)
            newbq = Datas::I()->banques->CreationBanque("PaPRS",             //! idBanqueAbrege
                                                "Panama Papers");    //! NomBanque
        ReconstruitComboBanques();
        ui->BanqueupcomboBox->setCurrentIndex(ui->BanqueupcomboBox->findData(newbq->id()));
        QString intit;
        if (m_userencours->titre().size() )
            intit += m_userencours->titre() + " ";
        intit += m_userencours->prenom() + " " + m_userencours->nom();
        if (Utils::trim(intit) == "")
            intit = "DR EDWARD SNOWDEN";
        ui->IntituleCompteuplineEdit    ->setText(intit);

        QString iban = "FR";
        std::default_random_engine gen{std::random_device{}()};
        std::uniform_int_distribution<int> al{1, 100};
        QString dl = QString::number(al(gen));
        iban += dl + " ";

        std::uniform_int_distribution<int> bl{1, 10000};
        for(int i = 0; i < 5; ++i)
        {
            iban += QString::number(bl(gen)) + " ";
        }
        std::uniform_int_distribution<int> cl{1, 1000};
        iban += QString::number(cl(gen));

        ui->NomCompteAbregeuplineEdit   ->setText("PaPRS" + dl);
        ui->IBANuplineEdit              ->setText(iban);
        ui->SoldeuplineEdit             ->setText("0,00");
        ui->CompteFacticePushButton     ->setVisible(false);
    }
}

void dlg_gestioncomptes::ModifCompte()
{
    m_mode = Modif;
    ui->DesactiveComptecheckBox     ->setVisible(true);
    ui->Compteframe                 ->setEnabled(true);
    ui->OKModifupSmallButton        ->setVisible(true);
    ui->AnnulModifupSmallButton     ->setVisible(true);
    ui->ComptesuptableWidget        ->setEnabled(false);
    ui->BanqueupcomboBox            ->setFocus();
    wdg_buttonframe                     ->setEnabled(false);

    /*On ne peut pas desactiver un compte s'il est le seul compte activé pour cet utilisateur
    */
    if (!ui->DesactiveComptecheckBox->isChecked())
        ui->DesactiveComptecheckBox ->setEnabled(m_userencours->listecomptesbancaires().size()>1);
    else
        ui->DesactiveComptecheckBox ->setEnabled(true);
    ui->BanqueupcomboBox            ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->IBANuplineEdit              ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->IntituleCompteuplineEdit    ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->NomCompteAbregeuplineEdit   ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
    ui->SoldeuplineEdit             ->setEnabled(!ui->DesactiveComptecheckBox->isChecked());
}

void dlg_gestioncomptes::NouvCompte()
{
    m_mode = Nouv;
    ui->Compteframe                 ->setVisible(true);
    ui->Compteframe                 ->setEnabled(true);
    ui->OKModifupSmallButton        ->setVisible(true);
    ui->AnnulModifupSmallButton     ->setVisible(true);
    wdg_buttonframe                     ->setEnabled(false);

    ui->BanqueupcomboBox            ->setEnabled(true);
    ui->IBANuplineEdit              ->setEnabled(true);
    ui->IntituleCompteuplineEdit    ->setEnabled(true);
    ui->NomCompteAbregeuplineEdit   ->setEnabled(true);
    ui->SoldeuplineEdit             ->setEnabled(true);

    ui->BanqueupcomboBox            ->clearEditText();
    ui->IBANuplineEdit              ->clear();
    ui->IntituleCompteuplineEdit    ->clear();
    ui->NomCompteAbregeuplineEdit   ->clear();
    ui->SoldeuplineEdit             ->setText("0,00");
    ui->idCompteupLineEdit          ->clear();
    ui->ComptesuptableWidget        ->setEnabled(false);
    ui->BanqueupcomboBox            ->setFocus();
    ui->CompteFacticePushButton     ->setVisible(true);
    ui->DesactiveComptecheckBox     ->setChecked(false);
    ui->DesactiveComptecheckBox     ->setVisible(false);
}

void dlg_gestioncomptes::SupprCompte()
{
    /* si on est à ce point, c'est qu'aucune écriture n'a été saisie sur ce compte
     * si le compte n'est pas partagé, on le supprime
    */
    UpMessageBox msgbox(this);
    UpSmallButton OKBouton(tr("Supprimer le compte"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.setIcon(UpMessageBox::Warning);
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.setInformativeText(tr("Supprimer le compte ") + ui->BanqueupcomboBox->currentText() + " - " + ui->IntituleCompteuplineEdit->text() + "?");
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton)
        return;

    Datas::I()->comptes->SupprimeCompte(Datas::I()->comptes->getById(ui->idCompteupLineEdit->text().toInt()));
    m_userencours->setlistecomptesbancaires(Datas::I()->comptes->initListeComptesByIdUser(m_userencours->id()));
    RemplirTableView();
}

void dlg_gestioncomptes::Fermer()
{
    if (ui->OKModifupSmallButton->isVisible() && ui->OKModifupSmallButton->isEnabled())
    {
       if (UpMessageBox::Question(this, tr("Modifications non enregistrées"), tr("Voulez-vous enregistrer les modifications avant de fermer la fiche?")) == UpSmallButton::STARTBUTTON)
       {
           if (!VerifCompte())
               return;
           ValidCompte();
       }
    }
    close();
}

void dlg_gestioncomptes::ValidCompte()
{
    int idcompte = 0;
    if (!VerifCompte())
        return;
    ui->Compteframe->setEnabled(false);
    ui->OKModifupSmallButton->setVisible(false);
    int idbanque = 0;
    for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
    {
        Banque *bq = const_cast<Banque*>(it.value());
        if (bq->id() == ui->BanqueupcomboBox->currentData().toInt())
        {
            idbanque = bq->id();
            break;
        }
    }
    if (idbanque == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver la banque") + " " + ui->BanqueupcomboBox->currentText() + "!");
        return;
    }
    if (m_mode == Modif)
    {
        idcompte = ui->idCompteupLineEdit->text().toInt();
        QHash<QString, QVariant> listsets;
        listsets.insert("IBAN"                  , ui->IBANuplineEdit->text());
        listsets.insert("IntituleCompte"        , ui->IntituleCompteuplineEdit->text());
        listsets.insert("NomCompteABrege"       , ui->NomCompteAbregeuplineEdit->text());
        listsets.insert("SoldeSurDernierReleve" , QString::number(QLocale().toDouble(ui->SoldeuplineEdit->text()),'f',2));
        listsets.insert("idbanque"              , QString::number(idbanque));
        listsets.insert("partage"               , (m_societe? "1" : "null"));
        listsets.insert("desactive"             , (ui->DesactiveComptecheckBox->isChecked()? "1" : "null"));
        db->UpdateTable(TBL_COMPTES,
                        listsets,
                        "where " CP_ID_COMPTES "  = " + ui->idCompteupLineEdit->text());
        m_comptencours = Datas::I()->comptes->getById(idcompte,true);
    }
    else if (m_mode == Nouv)
    {
        m_comptencours = Datas::I()->comptes->CreationCompte(idbanque,                         //! idBanque
                                            m_userencours->id(),                               //! idUser
                                            ui->IBANuplineEdit->text(),                        //! IBAN
                                            ui->IntituleCompteuplineEdit->text(),              //! IntituleCompte
                                            ui->NomCompteAbregeuplineEdit->text(),             //! NomCompteAbrege
                                            QLocale().toDouble(ui->SoldeuplineEdit->text()),   //! SoldeSurDernierReleve
                                            m_societe,                                         //! Partage
                                            ui->DesactiveComptecheckBox->isChecked());         //! Desactive
    }
    m_userencours->setlistecomptesbancaires(Datas::I()->comptes->initListeComptesByIdUser(m_userencours->id()));

    RemplirTableView(idcompte);
    ui->OKModifupSmallButton->setVisible(false);
    ui->AnnulModifupSmallButton->setVisible(false);
    wdg_buttonframe->setEnabled(true);
    ui->ComptesuptableWidget->setEnabled(true);
    ui->ComptesuptableWidget->setFocus();
    if (ui->ComptesuptableWidget->findItems(QString::number(idcompte),Qt::MatchExactly).size() > 0)
        ui->ComptesuptableWidget->setCurrentItem(ui->ComptesuptableWidget->findItems(QString::number(idcompte),Qt::MatchExactly).at(0));
    m_mode = Norm;
    ui->DesactiveComptecheckBox->setVisible(true);
}

void dlg_gestioncomptes::ReconstruitComboBanques()
{
    ui->BanqueupcomboBox->clear();
    // toute la manip qui suit sert à remettre les banques par ordre alphabétique - si vous trouvez plus simple, ne vous génez pas
    QStandardItemModel *model = new QStandardItemModel();
    for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
    {
        Banque *bq = const_cast<Banque*>(it.value());
        QList<QStandardItem *> items;
        items << new QStandardItem(bq->nom()) << new QStandardItem(QString::number(bq->id()));
            model->appendRow(items);
    }
    model->sort(0);
    for(int i=0; i<model->rowCount(); i++)
        ui->BanqueupcomboBox->addItem(model->item(i)->text(), model->item(i,1)->text());
}

void dlg_gestioncomptes::RemplirTableView(int idcompte)
{
    ui->ComptesuptableWidget->disconnect();
    QTableWidgetItem    *pitem0, *pitem1;
    ui->ComptesuptableWidget->clearContents();
    ui->ComptesuptableWidget->setColumnCount(2);
    ui->ComptesuptableWidget->setColumnHidden(0,true);
    ui->ComptesuptableWidget->setColumnWidth(1,ui->ComptesuptableWidget->width());
    ui->ComptesuptableWidget->verticalHeader()->setVisible(false);
    ui->ComptesuptableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    ui->ComptesuptableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(Icons::icEuroCount(),"Comptes"));
    ui->ComptesuptableWidget->horizontalHeader()->setVisible(true);
    ui->ComptesuptableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    ui->ComptesuptableWidget->horizontalHeader()->setIconSize(QSize(25,25));
    ui->ComptesuptableWidget->setGridStyle(Qt::DotLine);

    if (m_userencours->listecomptesbancaires(true).size()>0)
    {
        ui->Compteframe->setVisible(true);
        ui->ComptesuptableWidget->setRowCount(m_userencours->listecomptesbancaires(true).size());
        int i=0;
        foreach (int idcpt, m_userencours->listecomptesbancaires(true))
        {
            Compte *cpt = Datas::I()->comptes->getById(idcpt);
            if (cpt != Q_NULLPTR)
            {
                pitem0 = new QTableWidgetItem;
                pitem1 = new QTableWidgetItem;
                pitem0->setText(QString::number(cpt->id()));
                pitem1->setText(cpt->nomabrege());
                ui->ComptesuptableWidget->setItem(i,0,pitem0);
                ui->ComptesuptableWidget->setItem(i,1,pitem1);
                ui->ComptesuptableWidget->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
                i++;
            }
        }
        connect(ui->ComptesuptableWidget, &QTableWidget::currentItemChanged, this, [=] {AfficheCompte(ui->ComptesuptableWidget->currentItem(),Q_NULLPTR);});
        if (idcompte > 0)
            ui->ComptesuptableWidget->setCurrentItem(ui->ComptesuptableWidget->findItems(QString::number(idcompte), Qt::MatchExactly).at(0));
        else if (m_userencours->idcomptepardefaut() > 0)
            ui->ComptesuptableWidget->setCurrentItem(ui->ComptesuptableWidget->findItems(QString::number(m_userencours->idcomptepardefaut()), Qt::MatchExactly).at(0));
        else
            ui->ComptesuptableWidget->setCurrentItem(ui->ComptesuptableWidget->item(0,1));
    }
    else
        ui->Compteframe->setVisible(false);
}

bool dlg_gestioncomptes::VerifCompte()
{
    QString msg = "";
    if (ui->BanqueupcomboBox->currentText() == "")
        msg = tr("la banque");
    else if (ui->IntituleCompteuplineEdit->text() == "")
        msg = tr("l'intitulé");
    else if (ui->IBANuplineEdit->text() == "")
        msg = tr("le code IBAN");
    else if (ui->NomCompteAbregeuplineEdit->text() == "")
        msg = tr("le nom abrégé du compte");
    else if (ui->SoldeuplineEdit->text() == "")
        msg = tr("le solde du compte");
    if (msg != "")
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas précisé ") + msg);
        return false;
    }

    int idbanque = 0;
    for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
    {
        Banque *bq = const_cast<Banque*>(it.value());
        if (bq->id() == ui->BanqueupcomboBox->currentData().toInt())
        {
            idbanque = bq->id();
            break;
        }
    }
    if (idbanque == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de retrouver la banque") + " " + ui->BanqueupcomboBox->currentText() + "!");
        return false;
    }
    QStringList ibanlist;
    if (m_mode == Nouv)
    {
        foreach (int idcpt, m_userencours->listecomptesbancaires(false))
        {
            Compte *cpt = Datas::I()->comptes->getById(idcpt);
            if (cpt != Q_NULLPTR)
            {
                if (cpt->idBanque() == idbanque)
                {
                    UpMessageBox::Watch(this,tr("Vous avez déjà un compte enregistré dans cet organisme bancaire!"));
                    return false;
                }
                if (cpt->nomabrege() == ui->NomCompteAbregeuplineEdit->text())
                {
                    UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré avec ce nom abrégé!"));
                    return false;
                }
            }
        }
        foreach (Compte *cpt, *Datas::I()->comptes->comptes())
            ibanlist << cpt->iban().replace(" ","").toUpper();
        if (ibanlist.contains(ui->IBANuplineEdit->text().replace(" ","").toUpper()))
        {
            UpMessageBox::Watch(this,("Un compte est déjà enregistré avec cet IBAN!"));
            return false;
        }
    }
    else if (m_mode == Modif)
    {
        foreach (int idcpt, m_userencours->listecomptesbancaires(false))
        {
            Compte *cpt = Datas::I()->comptes->getById(idcpt);
            if (cpt != Q_NULLPTR)
            {
                if (cpt->nomabrege() == ui->NomCompteAbregeuplineEdit->text() && cpt->id() != ui->idCompteupLineEdit->text().toInt())
                {
                    UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré avec ce nom abrégé!"));
                    return false;
                }
                if (ibanlist.contains(ui->IBANuplineEdit->text().replace(" ","").toUpper()))
                {
                    UpMessageBox::Watch(this,tr("Un compte est déjà enregistré avec cet IBAN!"));
                    return false;
                }
            }
        }
        foreach (Compte *cpt, *Datas::I()->comptes->comptes())
            if (cpt->id() != ui->ComptesuptableWidget->item(ui->ComptesuptableWidget->currentRow(),0)->text().toInt())
                ibanlist << cpt->iban().replace(" ","").toUpper();
        if (ibanlist.contains(ui->IBANuplineEdit->text().replace(" ","").toUpper()))
        {
            UpMessageBox::Watch(this,("Un compte est déjà enregistré avec cet IBAN!"));
            return false;
        }
    }
    return true;
}
