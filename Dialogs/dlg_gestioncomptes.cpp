/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_gestioncomptes.h"
#include "ui_dlg_gestioncomptes.h"
#include "icons.h"


dlg_gestioncomptes::dlg_gestioncomptes(User *DataUser,
                                       bool societe,
                                       bool AfficheLeSolde,
                                       QWidget *parent)
                                       : UpDialog(parent),
                                        ui(new Ui::dlg_gestioncomptes)
{
    ui->setupUi(this);
    db                      = DataBase::getInstance();
    gDataUser               = DataUser;

    gidUser                 = gDataUser->id();

    gidCompteParDefaut      = gDataUser->getIdCompteParDefaut();
    gSociete                = societe;
    gAfficheLeSolde         = AfficheLeSolde;

    comptesusr              = new Comptes();
    comptesusr->addCompte( db->loadComptesByUser(gDataUser->id()) );
    CompteEnCours = new (Compte);
    CompteEnCours = comptesusr->getCompteById(gDataUser->getIdCompteParDefaut());

    gVisible                = true;
    gTimer                  = new QTimer(this);
    gTimer                  ->start(500);
    connect(gTimer, &QTimer::timeout, [=] {Clign();});
    ui->CompteFacticePushButton->setVisible(false);

    setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());

    widgButtons             = new WidgetButtonFrame(ui->ComptesuptableWidget);
    widgButtons             ->AddButtons(WidgetButtonFrame::PlusButton | WidgetButtonFrame::ModifButton | WidgetButtonFrame::MoinsButton);
    NouvBanqupPushButton    = new UpSmallButton();
    NouvBanqupPushButton    ->setText(tr("Gestion des organismes bancaires"));
    NouvBanqupPushButton    ->setIcon(Icons::icEuroCount());
    NouvBanqupPushButton    ->setIconSize(QSize(25,25));
    AjouteWidgetLayButtons(NouvBanqupPushButton);
    AjouteLayButtons(UpDialog::ButtonClose);
    CloseButton             ->setText(tr("Fermer"));

    connect(CloseButton,                    &QPushButton::clicked,      [=] {Fermer();});
    connect(ui->OKModifupSmallButton,       &QPushButton::clicked,      [=] {ValidCompte();});
    connect(ui->AnnulModifupSmallButton,    &QPushButton::clicked,      [=] {AnnulModif();});
    connect(NouvBanqupPushButton,           &QPushButton::clicked,      [=] {Banques();});
    connect(widgButtons,                    &WidgetButtonFrame::choix,  [=] {ChoixButtonFrame(widgButtons->Reponse());});
    connect(ui->CompteFacticePushButton,    &QPushButton::clicked,      [=] {CompteFactice();});
    connect(ui->DesactiveComptecheckBox,    &QPushButton::clicked,      [=] {DesactiveCompte();});

    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    ui->SoldeuplineEdit->setValidator(val);

    gUserLogin          = gDataUser->getLogin();
    setWindowTitle(tr("Comptes bancaires de ") + gUserLogin);

    MetAJourListeBanques();

    RemplirTableView(gidCompteParDefaut);
    ui->Compteframe             ->setEnabled(false);
    ui->OKModifupSmallButton    ->setVisible(false);
    ui->AnnulModifupSmallButton ->setVisible(false);
    ui->idCompteupLineEdit      ->setVisible(false);
    ui->idComptelabel           ->setVisible(false);
    ui->Soldelabel              ->setVisible(gAfficheLeSolde || gSociete);
    ui->SoldeuplineEdit         ->setVisible(gAfficheLeSolde || gSociete);
    ui->CompteSocietecheckBox   ->setEnabled(false);
    ui->CompteSocietecheckBox   ->setChecked(gSociete);

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
    hlay        ->addWidget(widgButtons->widgButtonParent());
    hlay        ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    hlay        ->addLayout(vlay);
    globallay   ->insertLayout(0,hlay);
    globallay   ->setSizeConstraint(QLayout::SetFixedSize);

    gMode = Norm;
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
        UpMessageBox msgbox;
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
    CompteEnCours = comptesusr->getCompteById(idCompte);
        ui->BanqueupcomboBox            ->setCurrentText(CompteEnCours->nombanque());
        ui->IBANuplineEdit              ->setText(CompteEnCours->iban());
        ui->IntituleCompteuplineEdit    ->setText(CompteEnCours->intitulecompte());
        ui->NomCompteAbregeuplineEdit   ->setText(CompteEnCours->nom());
        ui->SoldeuplineEdit             ->setText(QLocale().toString(CompteEnCours->solde(),'f',2));
        ui->idCompteupLineEdit          ->setText(QString::number(CompteEnCours->id()));
        ui->DesactiveComptecheckBox     ->setChecked(CompteEnCours->isDesactive());
    createurducompte    =  (gidUser == CompteEnCours->idUser());
    widgButtons->moinsBouton    ->setEnabled(true);

    /*On ne peut pas supprimer un compte si:
     * . il y a déjà eu des ecritures bancaires sur ce compte
    */
    bool autorsupprimer;
    bool ok = true;
    QList<QList<QVariant>> listlignescomptes = db->SelectRecordsFromTable(QStringList() << "idcompte",
                                                                          NOM_TABLE_LIGNESCOMPTES, ok,
                                                                          "where idcompte = " + QString::number(CompteEnCours->id()));
    autorsupprimer = (listlignescomptes.size()==0);// il n'y a pas d'écritures en cours
    if (autorsupprimer)
    {
        listlignescomptes = db->SelectRecordsFromTable(QStringList() << "idcompte",
                                                       NOM_TABLE_ARCHIVESBANQUE, ok,
                                                       "where idcompte = " + QString::number(CompteEnCours->id()));
        autorsupprimer = (listlignescomptes.size()==0);// il n'y a pas d'écritures en cours
    }
    widgButtons->moinsBouton->setEnabled(autorsupprimer);
}

void dlg_gestioncomptes::AnnulModif()
{
    ui->Compteframe->setEnabled(false);
    gMode = Norm;
    ui->DesactiveComptecheckBox->setVisible(true);
    if (ui->ComptesuptableWidget->rowCount() > 0)
        AfficheCompte(ui->ComptesuptableWidget->item(ui->ComptesuptableWidget->currentRow(),0),ui->ComptesuptableWidget->item(ui->ComptesuptableWidget->currentRow(),0));
    else
        ui->Compteframe->setVisible(false);
    ui->OKModifupSmallButton->setVisible(false);
    ui->AnnulModifupSmallButton->setVisible(false);
    ui->ComptesuptableWidget->setEnabled(true);
    widgButtons->setEnabled(true);
    ui->CompteFacticePushButton->setVisible(false);
    ui->ComptesuptableWidget->setFocus();
}

void dlg_gestioncomptes::Banques()
{
    Dlg_Banq = new dlg_banque();
    if (Dlg_Banq->exec()>0)
        MetAJourListeBanques();
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
        QList<QList<QVariant>> listcomptes = db->SelectRecordsFromTable(QStringList() << "idcompte",
                                                                        NOM_TABLE_LIGNESCOMPTES, ok,
                                                                        "where iduser = " + QString::number(gidUser) + " and desactive is null");
        ui->DesactiveComptecheckBox ->setEnabled(listcomptes.size()>1);
    }
}

void dlg_gestioncomptes::ChoixButtonFrame(int i)
{
    switch (i) {
    case 1:
        NouvCompte();
        break;
    case 0:
        ModifCompte();
        break;
    case -1:
        SupprCompte();
        break;
    default:
        break;
    }
}

void dlg_gestioncomptes::Clign()
{
    gVisible = !gVisible;
    if (gVisible)
        ui->CompteFacticePushButton->setIcon(Icons::icNull());
    else
        ui->CompteFacticePushButton->setIcon(Icons::icHelp());
}

void dlg_gestioncomptes::CompteFactice()
{
    UpMessageBox msgbox;
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
        int idbanq = 0;
        bool ok = true;
        QList<QList<QVariant>> listPaPRS = db->SelectRecordsFromTable(QStringList() << "idbanque",
                                                                      NOM_TABLE_BANQUES, ok,
                                                                      "where idbanqueabrege = 'PaPRS'");
        if (listPaPRS.size()==0)
        {
            db->StandardSQL("insert into " NOM_TABLE_BANQUES " (idbanqueAbrege, Nombanque) values ('PaPRS','Panama Papers')");
            listPaPRS = db->SelectRecordsFromTable(QStringList() << "idbanque",
                                                   NOM_TABLE_BANQUES, ok,
                                                   "where idbanqueabrege = 'PaPRS'");
        }
        idbanq = listPaPRS.at(0).at(0).toInt();
        MetAJourListeBanques();
        ui->BanqueupcomboBox->setCurrentIndex(ui->BanqueupcomboBox->findData(idbanq));
        QString intit;
        if (gDataUser->getTitre().size() )
            intit += gDataUser->getTitre() + " ";
        intit += gDataUser->getPrenom() + " " + gDataUser->getNom();
        if (intit.remove(" ") == "") intit = "DOCTEUR EDWARD SNOWDEN";
        ui->IntituleCompteuplineEdit    ->setText(intit);
        int al = 0;
        QString iban = "FR";
        srand(static_cast<uint>(time(Q_NULLPTR)));
        al = rand() % 100;
        while (al<10)
            al = rand() % 100;
        iban += QString::number(al) + " ";
        for(int i=0; i<5; i++)
        {
            al = rand() % 10000;
            while (al<1000)
                al = rand() % 10000;
            iban += QString::number(al) + " ";
        }
        al = rand() % 1000;
        while (al<100)
            al = rand() % 1000;
        iban += QString::number(al);
        ui->NomCompteAbregeuplineEdit   ->setText("PaPRS"+QString::number(al));
        ui->IBANuplineEdit              ->setText(iban);
        ui->SoldeuplineEdit             ->setText("0,00");
        ui->CompteFacticePushButton     ->setVisible(false);
    }
}

void dlg_gestioncomptes::ModifCompte()
{
    gMode = Modif;
    ui->DesactiveComptecheckBox     ->setVisible(true);
    ui->Compteframe                 ->setEnabled(true);
    ui->OKModifupSmallButton        ->setVisible(true);
    ui->AnnulModifupSmallButton     ->setVisible(true);
    ui->ComptesuptableWidget        ->setEnabled(false);
    ui->BanqueupcomboBox            ->setFocus();
    widgButtons                     ->setEnabled(false);

    /*On ne peut pas desactiver un compte si:
     * . il est le seul compte active pour cet utilisateur
    */
    bool ok = true;
    QList<QList<QVariant>> listcomptes = db->SelectRecordsFromTable(QStringList() << "idcompte",
                                                                    NOM_TABLE_LIGNESCOMPTES, ok,
                                                                    "where iduser = " + QString::number(gidUser) + " and desactive is null");
    if (!ui->DesactiveComptecheckBox->isChecked())
        ui->DesactiveComptecheckBox ->setEnabled(listcomptes.size()>1);
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
    gMode = Nouv;
    ui->Compteframe                 ->setVisible(true);
    ui->Compteframe                 ->setEnabled(true);
    ui->OKModifupSmallButton        ->setVisible(true);
    ui->AnnulModifupSmallButton     ->setVisible(true);
    widgButtons                     ->setEnabled(false);

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
     * si le compte n'est pas partagé, on le supprime et on supprime son lien dans comptesJiointures
    */
    UpMessageBox msgbox;
    UpSmallButton OKBouton(tr("Supprimer le compte"));
    UpSmallButton NoBouton(tr("Annuler"));
    msgbox.setIcon(UpMessageBox::Warning);
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::SUPPRBUTTON);
    msgbox.setInformativeText(tr("Supprimer le compte ") + ui->BanqueupcomboBox->currentText() + " - " + ui->IntituleCompteuplineEdit->text() + "?");
    msgbox.exec();
    if (msgbox.clickedButton() != &OKBouton)
        return;
    db->StandardSQL("delete from " NOM_TABLE_COMPTES " where idCompte = " + ui->idCompteupLineEdit->text());
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
    int idcompte=0;
    QString req;
    if (!VerifCompte())
        return;
    ui->Compteframe->setEnabled(false);
    ui->OKModifupSmallButton->setVisible(false);
    bool ok = true;
    QList<QList<QVariant>> listbanq = db->SelectRecordsFromTable(QStringList() << "idbanque",
                                                                  NOM_TABLE_BANQUES, ok,
                                                                  "where nomBanque = '" + Utils::CorrigeApostrophe(ui->BanqueupcomboBox->currentText()) + "'");
    int idbanque = listbanq.at(0).at(0).toInt();
    if (gMode == Modif)
    {
        idcompte = ui->idCompteupLineEdit->text().toInt();
        QHash<QString, QString> listsets;
        listsets.insert(" IBAN = '"                   , ui->IBANuplineEdit->text());
        listsets.insert(" IntituleCompte = '"         , ui->IntituleCompteuplineEdit->text());
        listsets.insert(" NomCompteABrege = '"        , ui->NomCompteAbregeuplineEdit->text());
        listsets.insert(" SoldeSurDernierReleve = "   , QString::number(QLocale().toDouble(ui->SoldeuplineEdit->text()),'f',2));
        listsets.insert(" idbanque = "                , QString::number(idbanque));
        listsets.insert(" partage = "                 , (gSociete? "1" : "null"));
        listsets.insert(" desactive = "               , (ui->DesactiveComptecheckBox->isChecked()? "1" : "null"));
        db->UpdateTable(NOM_TABLE_COMPTES,
                        listsets,
                        "where idCompte = "          + ui->idCompteupLineEdit->text());
    }
    else if (gMode == Nouv)
    {
        QHash<QString, QString> listsets;
        listsets.insert("iduser"                      , QString::number(gidUser));
        listsets.insert("idbanque"                    , QString::number(idbanque));
        listsets.insert(" IBAN = '"                   , ui->IBANuplineEdit->text());
        listsets.insert(" IntituleCompte = '"         , ui->IntituleCompteuplineEdit->text());
        listsets.insert(" NomCompteABrege = '"        , ui->NomCompteAbregeuplineEdit->text());
        listsets.insert(" SoldeSurDernierReleve = "   , QString::number(QLocale().toDouble(ui->SoldeuplineEdit->text()),'f',2));
        listsets.insert(" partage = "                 , (gSociete? "1" : "null"));
        listsets.insert(" desactive = "               , (ui->DesactiveComptecheckBox->isChecked()? "1" : "null"));
        db->InsertIntoTable(NOM_TABLE_BANQUES, listsets);
        idcompte = db->selectMaxFromTable("idcompte",NOM_TABLE_COMPTES);
        if (!gAfficheLeSolde)
            UpMessageBox::Watch(this, tr("Le compte ") + ui->IntituleCompteuplineEdit->text() + tr(" a été enregistré."),
                                      tr("le solde a été fixé à O,OO euros et devra être corrigé par le propriétaire du compte"));
    }
    comptesusr->removeCompte(CompteEnCours);
    comptesusr->addCompte(db->loadComptesByUser(idcompte));
    CompteEnCours = comptesusr->getCompteById(idcompte);

    RemplirTableView();
    ui->OKModifupSmallButton->setVisible(false);
    ui->AnnulModifupSmallButton->setVisible(false);
    widgButtons->setEnabled(true);
    ui->ComptesuptableWidget->setEnabled(true);
    ui->ComptesuptableWidget->setFocus();
    ui->ComptesuptableWidget->setCurrentCell(ui->ComptesuptableWidget->findItems(QString::number(idcompte),Qt::MatchExactly).at(0)->row(),1);
    gMode = Norm;
    ui->DesactiveComptecheckBox->setVisible(true);
}

QString dlg_gestioncomptes::CorrigeApostrophe(QString RechAp)
{
    return RechAp.replace("'","\\'");
}

void dlg_gestioncomptes::MetAJourListeBanques()
{
    bool ok = true;
    QList<QList<QVariant>> listbanques = db->SelectRecordsFromTable(QStringList() << "nombanque" << "idbanque",
                                                                  NOM_TABLE_BANQUES, ok);
    ui->BanqueupcomboBox->clear();
    for (int i=0; i<listbanques.size(); i++)
        ui->BanqueupcomboBox->insertItem(0, listbanques.at(i).at(0).toString(), listbanques.at(i).at(1).toInt());
}

void dlg_gestioncomptes::RemplirTableView(int idcompte)
{
    ui->ComptesuptableWidget->disconnect();
    QTableWidgetItem    *pitem0, *pitem1;
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

    QList<Compte*> listcomptes;
    for(QMultiMap<int, Compte*>::const_iterator itcpt = comptesusr->comptesAll().constBegin(); itcpt != comptesusr->comptesAll().constEnd(); ++itcpt)
    {
        Compte *cpt = const_cast<Compte*>(itcpt.value());
        listcomptes << cpt;
    }
    if (listcomptes.size()>0)
    {
        ui->Compteframe->setVisible(true);
        ui->ComptesuptableWidget->setRowCount(listcomptes.size());
        for (int i=0; i<listcomptes.size(); i++)
        {
            pitem0 = new QTableWidgetItem;
            pitem1 = new QTableWidgetItem;
            pitem0->setText(QString::number(listcomptes.at(i)->id()));
            pitem1->setText(listcomptes.at(i)->nom());
            ui->ComptesuptableWidget->setItem(i,0,pitem0);
            ui->ComptesuptableWidget->setItem(i,1,pitem1);
            ui->ComptesuptableWidget->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
        }
        connect(ui->ComptesuptableWidget, &QTableWidget::currentItemChanged, [=] {AfficheCompte(ui->ComptesuptableWidget->currentItem(),Q_NULLPTR);});
        if (idcompte<1)
            ui->ComptesuptableWidget->setCurrentItem(ui->ComptesuptableWidget->item(0,1));
        else
            ui->ComptesuptableWidget->setCurrentItem(ui->ComptesuptableWidget->findItems(QString::number(idcompte), Qt::MatchExactly).at(0));
    }
    else
        ui->Compteframe->setVisible(false);
}

bool dlg_gestioncomptes::VerifCompte()
{
    QString msg = "";
    QString req;
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

    int idbanque = -1;
    bool ok = true;
    QList<QList<QVariant>> listbanq = db->SelectRecordsFromTable(QStringList() << "idbanque",
                                                                  NOM_TABLE_BANQUES, ok,
                                                                  "where nomBanque = '" + Utils::CorrigeApostrophe(ui->BanqueupcomboBox->currentText()) + "'");
    idbanque = listbanq.at(0).at(0).toInt();

    if (gMode == Nouv)
    {
        QList<QList<QVariant>> listcpt = db->SelectRecordsFromTable(QStringList() << "idbanque",
                                                                      NOM_TABLE_COMPTES, ok,
                                                                      "where idUser = " + QString::number(gidUser) + " and idbanque = " + QString::number(idbanque));
        if (listcpt.size()>0)
        {
            UpMessageBox::Watch(this,tr("Vous avez déjà un compte enregistré dans cet organisme bancaire!"));
            return false;
        }
        QList<QList<QVariant>> listnomcpt = db->SelectRecordsFromTable(QStringList() << "nomcompteabrege",
                                                                      NOM_TABLE_COMPTES, ok,
                                                                      "where idUser = " + QString::number(gidUser) + " and nomcompteabrege = '" + Utils::CorrigeApostrophe(ui->NomCompteAbregeuplineEdit->text()) + "'");
        if (listnomcpt.size()>0)
        {
            UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré avec ce nom abrégé!"));
            return false;
        }
        QList<QList<QVariant>> listiban = db->SelectRecordsFromTable(QStringList() << "IBAN", NOM_TABLE_COMPTES, ok);
        if (listiban.size()>0)
        {
            QStringList ibanlist;
            for (int i=0; i<listiban.size(); i++)
                ibanlist << listiban.at(0).at(0).toString().replace(" ","").toUpper();
            if (ibanlist.contains(ui->IBANuplineEdit->text().replace(" ","").toUpper()))
            {
                UpMessageBox::Watch(this,("Un compte est déjà enregistré avec cet IBAN!"));
                return false;
            }
        }
    }
    else if (gMode == Modif)
    {
        QList<QList<QVariant>> listcpt = db->SelectRecordsFromTable(QStringList() << "idbanque",
                                                                    NOM_TABLE_COMPTES, ok,
                                                                    "where idUser = " + QString::number(gidUser) +
                                                                    " and idbanque = " + QString::number(idbanque) +
                                                                    " and idcompte <> " + ui->idCompteupLineEdit->text());
        if (listcpt.size()>0)
        {
            UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré dans cet organisme bancaire!"));
            return false;
        }
        QList<QList<QVariant>> listnomabrg = db->SelectRecordsFromTable(QStringList() << "nomcompteabrege",
                                                                    NOM_TABLE_COMPTES, ok,
                                                                    "where idUser = " + QString::number(gidUser) +
                                                                    " and nomcompteabrege = '" + ui->NomCompteAbregeuplineEdit->text() + "'" +
                                                                    " and idcompte <> " + ui->idCompteupLineEdit->text());
        if (listnomabrg.size()>0)
        {
            UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré avec ce nom abrégé!"));
            return false;
        }
        QList<QList<QVariant>> listiban = db->SelectRecordsFromTable(QStringList() << "IBAN",
                                                                    NOM_TABLE_COMPTES, ok,
                                                                    "where idcompte <> " + ui->ComptesuptableWidget->item(ui->ComptesuptableWidget->currentRow(),0)->text());
        if (listiban.size()>0)
        {
            QStringList ibanlist;
            for (int i=0; i<listiban.size(); i++)
                ibanlist << listiban.at(i).at(0).toString().replace(" ","").toUpper();
            if (ibanlist.contains(ui->IBANuplineEdit->text().replace(" ","").toUpper()))
            {
                UpMessageBox::Watch(this,tr("Un compte est déjà enregistré avec cet IBAN!"));
                return false;
            }
        }
    }
    return true;
}
