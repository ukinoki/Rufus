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


dlg_gestioncomptes::dlg_gestioncomptes(User *DataUser, QSqlDatabase gdb,
                                       bool societe, bool AfficheLeSolde, QWidget *parent) :
    UpDialog(parent),
    ui(new Ui::dlg_gestioncomptes)
{
    ui->setupUi(this);
    db                      = gdb;
    gDataUser               = DataUser;

    gidUser                 = gDataUser->id();

    gidCompteParDefaut      = gDataUser->getIdCompteParDefaut();
    gSociete                = societe;
    gAfficheLeSolde         = AfficheLeSolde;

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
    QString req = "select idcompte, nombanque, IBAN, IntituleCompte, NomCompteAbrege, SoldeSurDernierReleve, Partage, desactive, cpt.idUser  from "  NOM_TABLE_COMPTES " as cpt"
                  " left outer join " NOM_TABLE_BANQUES " as bq on cpt.idbanque = bq.idbanque"
                  " where idcompte = " + QString::number(idCompte);
    //qDebug() << req;
    QSqlQuery quer(req,db);
    TraiteErreurRequete(quer,req);
    if (quer.size()>0)
    {
        quer.first();
        ui->BanqueupcomboBox            ->setCurrentText(quer.value(1).toString());
        ui->IBANuplineEdit              ->setText(quer.value(2).toString());
        ui->IntituleCompteuplineEdit    ->setText(quer.value(3).toString());
        ui->NomCompteAbregeuplineEdit   ->setText(quer.value(4).toString());
        ui->SoldeuplineEdit             ->setText(QLocale().toString(quer.value(5).toDouble(),'f',2));
        ui->idCompteupLineEdit          ->setText(quer.value(0).toString());
        ui->DesactiveComptecheckBox     ->setChecked(quer.value(7).toInt()==1);
    }
    createurducompte    =  (gidUser == quer.value(7).toInt());
    widgButtons->moinsBouton    ->setEnabled(true);

    /*On ne peut pas supprimer un compte si:
     * . il y a déjà eu des ecritures bancaires sur ce compte
    */
    bool autorsupprimer;
    autorsupprimer      = (QSqlQuery("select idcompte from " NOM_TABLE_LIGNESCOMPTES " where idcompte = " + quer.value(0).toString(),db).size()==0);  // il n'y a pas d'écritures en cours
    if (autorsupprimer)
        autorsupprimer  = (QSqlQuery("select idcompte from " NOM_TABLE_ARCHIVESBANQUE " where idcompte = " + quer.value(0).toString(),db).size()==0);  // il n'y a pas d'écritures en archives
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
    Dlg_Banq = new dlg_banque(db);
    if (Dlg_Banq->exec()>0)
        MetAJourListeBanques();
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
        QString req = "select idcompte from "  NOM_TABLE_COMPTES
                " where iduser = " + QString::number(gidUser) +
                " and desactive is null";
        QSqlQuery desquer(req,db);
        TraiteErreurRequete(desquer,req);
        ui->DesactiveComptecheckBox ->setEnabled(desquer.size()>1);
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
        QString req = "select idbanque, idbanqueabrege, nombanque from " NOM_TABLE_BANQUES " where idbanqueabrege = 'PaPRS'";
        QSqlQuery quer(req, db);
        if (quer.size()>0)
        {
            quer.first();
            idbanq = quer.value(0).toInt();
        }
        else
        {
            QSqlQuery ("insert into " NOM_TABLE_BANQUES " (idbanqueAbrege, Nombanque) values ('PaPRS','Panama Papers')", db);
            QSqlQuery quer2("select idbanque from " NOM_TABLE_BANQUES " where idbanqueabrege = 'PaPRS'", db);
            quer2.first();
            idbanq = quer2.value(0).toInt();
        }
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
    QString req = "select idcompte from "  NOM_TABLE_COMPTES
                  " where iduser = " + QString::number(gidUser) +
                  " and desactive is null";
    //qDebug() << req;
    QSqlQuery desquer(req,db);
    TraiteErreurRequete(desquer,req);
    if (!ui->DesactiveComptecheckBox->isChecked())
        ui->DesactiveComptecheckBox ->setEnabled(desquer.size()>1);
    else
        ui->DesactiveComptecheckBox->setEnabled(true);
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
    QString rq = "delete from " NOM_TABLE_COMPTES " where idCompte = " + ui->idCompteupLineEdit->text();
    TraiteErreurRequete(QSqlQuery(rq,db),rq);
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
    req = "select idbanque from " NOM_TABLE_BANQUES " where nomBanque = '" + CorrigeApostrophe(ui->BanqueupcomboBox->currentText()) + "'";
    QSqlQuery quer(req,db);
    quer.first();
    int idbanque = quer.value(0).toInt();
    if (gMode == Modif)
    {
        idcompte = ui->idCompteupLineEdit->text().toInt();
        req = "update " NOM_TABLE_COMPTES " set "
              " IBAN = '"                   + CorrigeApostrophe(ui->IBANuplineEdit->text())                             + "', " +
              " IntituleCompte = '"         + CorrigeApostrophe(ui->IntituleCompteuplineEdit->text())                   + "', " +
              " NomCompteABrege = '"        + CorrigeApostrophe(ui->NomCompteAbregeuplineEdit->text())                  + "', " +
              " SoldeSurDernierReleve = "   + QString::number(QLocale().toDouble(ui->SoldeuplineEdit->text()),'f',2)    + ", " +
              " idbanque = "                + QString::number(idbanque)                                                 + ", " +
              " partage = "                 + (gSociete? "1" : "null")                                                  + ", " +
              " desactive = "               + (ui->DesactiveComptecheckBox->isChecked()? "1" : "null") +
              " where idCompte = "          + ui->idCompteupLineEdit->text();
        QSqlQuery quer0(req,db);
        TraiteErreurRequete(quer0,req);
    }
    else if (gMode == Nouv)
    {
        req = "insert into " NOM_TABLE_COMPTES " (iduser, idbanque, IBAN, IntituleCompte, NomCompteAbrege, SoldeSurDernierReleve, Partage, desactive)\n"
               " values (" +
              QString::number(gidUser)                                              + ", "  +
              QString::number(idbanque)                                             + ", "  +
              "'" + CorrigeApostrophe(ui->IBANuplineEdit->text())             + "', " +
              "'" + CorrigeApostrophe(ui->IntituleCompteuplineEdit->text())   + "', " +
              "'" + CorrigeApostrophe(ui->NomCompteAbregeuplineEdit->text())  + "', " +
              QString::number(QLocale().toDouble(ui->SoldeuplineEdit->text()),'f',2) + ", " +
              (gSociete? "1" : "null") + ", " +
              (ui->DesactiveComptecheckBox->isChecked()? "1" : "null") + ")";
        QSqlQuery (req,db);
        req = "select max(idcompte) from " NOM_TABLE_COMPTES ";";
        QSqlQuery quer1(req,db);
        TraiteErreurRequete(quer1,req);
        quer1.first();
        idcompte = quer1.value(0).toInt();
        if (!gAfficheLeSolde)
            UpMessageBox::Watch(this, tr("Le compte ") + ui->IntituleCompteuplineEdit->text() + tr(" a été enregistré."),
                                      tr("le solde a été fixé à O,OO euros et devra être corrigé par le propriétaire du compte"));
    }
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
    gListBanques.clear();
    QString req = "select nombanque, idBanque from " NOM_TABLE_BANQUES;
    QSqlQuery quer(req,db);
    for (int j=0; j<ui->BanqueupcomboBox->count(); j++)
        ui->BanqueupcomboBox->removeItem(j);
    for (int i=0; i<quer.size(); i++)
    {
        quer.seek(i);
        ui->BanqueupcomboBox->insertItem(0, quer.value(0).toString(), quer.value(1).toInt());
    }
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
    QString req = "select idcompte, NomCompteAbrege  from " NOM_TABLE_COMPTES
                  " where iduser = " + QString::number(gidUser) +
                  " order by nomcompteabrege";
    //qDebug() << req;
    QSqlQuery quer(req,db);
    TraiteErreurRequete(quer,req);
    if (quer.size() > 0)
    {
        ui->Compteframe->setVisible(true);
        ui->ComptesuptableWidget->setRowCount(quer.size());
        quer.first();
        for (int i=0; i<quer.size(); i++)
        {
            pitem0 = new QTableWidgetItem;
            pitem1 = new QTableWidgetItem;
            pitem0->setText(quer.value(0).toString());
            pitem1->setText(quer.value(1).toString());
            ui->ComptesuptableWidget->setItem(i,0,pitem0);
            ui->ComptesuptableWidget->setItem(i,1,pitem1);
            ui->ComptesuptableWidget->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
            quer.next();
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

/*-----------------------------------------------------------------------------------------------------------------
    -- Traite et affiche le signal d'erreur d'une requete -------------------------------------------------------------
    -----------------------------------------------------------------------------------------------------------------*/
bool dlg_gestioncomptes::TraiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage)
{
    if (query.lastError().type() != QSqlError::NoError)
    {
        UpMessageBox::Watch(Q_NULLPTR, ErrorMessage, tr("\nErreur\n") + query.lastError().text() +  tr("\nrequete = ") + requete);
        return true;
    }
    else return false;
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
    req = "select idbanque from " NOM_TABLE_BANQUES " where nomBanque = '" + CorrigeApostrophe(ui->BanqueupcomboBox->currentText()) + "'";
    QSqlQuery quer(req,db);
    quer.first();
    idbanque = quer.value(0).toInt();

    if (gMode == Nouv)
    {
        req = "select idbanque from " NOM_TABLE_COMPTES
              " where idUser = " + QString::number(gidUser) +
              " and idbanque = " + QString::number(idbanque);
        QSqlQuery quer0(req,db);
        TraiteErreurRequete(quer0,req);
        if (quer0.size()>0)
        {
            UpMessageBox::Watch(this,tr("Vous avez déjà un compte enregistré dans cet organisme bancaire!"));
            return false;
        }
        req = "select nomcompteabrege from " NOM_TABLE_COMPTES
              " where idUser = " + QString::number(gidUser) +
              " and nomcompteabrege = '" + ui->NomCompteAbregeuplineEdit->text() + "'";
        QSqlQuery quer1(req,db);
        TraiteErreurRequete(quer1,req);
        if (quer1.size()>0)
        {
            UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré avec ce nom abrégé!"));
            return false;
        }
        req = "select IBAN from " NOM_TABLE_COMPTES;
        QSqlQuery ibanquer(req,db);
        TraiteErreurRequete(ibanquer,req);
        if (ibanquer.size()>0)
        {
            QStringList ibanlist;
            ibanquer.first();
            for (int i=0; i<ibanquer.size(); i++)
            {
                ibanlist << ibanquer.value(0).toString().replace(" ","").toUpper();
                ibanquer.next();
            }
            if (ibanlist.contains(ui->IBANuplineEdit->text().replace(" ","").toUpper()))
            {
                UpMessageBox::Watch(this,("Un compte est déjà enregistré avec cet IBAN!"));
                return false;
            }
        }
    }
    else if (gMode == Modif)
    {
        req = "select idbanque from " NOM_TABLE_COMPTES
              " where idUser = " + QString::number(gidUser) +
              " and idbanque = " + QString::number(idbanque) +
              " and idcompte <> " + ui->idCompteupLineEdit->text();
        QSqlQuery quer0(req,db);
        TraiteErreurRequete(quer0,req);
        if (quer0.size()>0)
        {
            UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré dans cet organisme bancaire!"));
            return false;
        }
        req = "select nomcompteabrege from " NOM_TABLE_COMPTES
                " where idUser = " + QString::number(gidUser) +
                " and nomcompteabrege = '" + ui->NomCompteAbregeuplineEdit->text() + "'" +
                " and idcompte <> " + ui->idCompteupLineEdit->text();
        QSqlQuery quer1(req,db);
        TraiteErreurRequete(quer1,req);
        if (quer1.size()>0)
        {
            UpMessageBox::Watch(this,tr(" Vous avez déjà un compte enregistré avec ce nom abrégé!"));
            return false;
        }
        req = "select IBAN from " NOM_TABLE_COMPTES " where idcompte <> " + ui->ComptesuptableWidget->item(ui->ComptesuptableWidget->currentRow(),0)->text();
        QSqlQuery ibanquer(req,db);
        TraiteErreurRequete(ibanquer,req);
        if (ibanquer.size()>0)
        {
            QStringList ibanlist;
            ibanquer.first();
            for (int i=0; i<ibanquer.size(); i++)
            {
                ibanlist << ibanquer.value(0).toString().replace(" ","").toUpper();
                ibanquer.next();
            }
            if (ibanlist.contains(ui->IBANuplineEdit->text().replace(" ","").toUpper()))
            {
                UpMessageBox::Watch(this,tr("Un compte est déjà enregistré avec cet IBAN!"));
                return false;
            }
        }
    }
    return true;
}
