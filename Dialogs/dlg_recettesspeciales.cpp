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

#include "dlg_recettesspeciales.h"
#include "gbl_datas.h"
#include "icons.h"
#include "ui_dlg_recettesspeciales.h"
#include "cls_compte.h"

dlg_recettesspeciales::dlg_recettesspeciales(QWidget *parent) :
    QDialog(parent), ui(new Ui::dlg_recettesspeciales)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Liste des recettes spéciales"));

    ui->Userlabel->setText(tr("Recettes spéciales de ") + Datas::I()->users->userconnected()->login());

    restoreGeometry(proc->settings()->value(Position_Fiche Nom_fiche_Depenses).toByteArray());

    m_initok = initializeUserSelected();
    if( !m_initok )
        return;

    setMaximumSize(1642,800);

    wdg_bigtable       = new UpTableWidget(this);
    ui->horizontalLayout_3->addWidget(wdg_bigtable);
    wdg_enreguppushbutton = new UpPushButton();
    ui->Buttonlayout->insertWidget(2, wdg_enreguppushbutton);
    wdg_enreguppushbutton->setFixedSize(160,46);
    wdg_enreguppushbutton->setText("Valider");
    wdg_enreguppushbutton->setIcon(Icons::icOK());
    wdg_enreguppushbutton->setIconSize(QSize(30,30));
    wdg_enreguppushbutton->setVisible(true);

    wdg_annuluppushbutton = new UpPushButton();
    ui->Buttonlayout->insertWidget(1,wdg_annuluppushbutton);
    wdg_annuluppushbutton->setFixedSize(160,46);
    wdg_annuluppushbutton->setText("Annuler");
    wdg_annuluppushbutton->setIcon(Icons::icAnnuler());
    wdg_annuluppushbutton->setIconSize(QSize(30,30));
    wdg_annuluppushbutton->setVisible(true);

    ui->frame->setStyleSheet("QFrame#frame{border: 1px solid gray; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 #f6f7fa, stop: 1 rgba(200, 210, 210, 50));}");

    QStandardItemModel *model = new QStandardItemModel(this);
    for (auto it = Datas::I()->banques->banques()->constBegin(); it != Datas::I()->banques->banques()->constEnd(); ++it)
    {
        Banque* bq = const_cast<Banque*>(it.value());
        QList<QStandardItem *> items;
        items << new QStandardItem(bq->nomabrege()) << new QStandardItem(QString::number(bq->id()));
            model->appendRow(items);
    }
    model->sort(0);
    for(int i=0; i<model->rowCount(); i++)
        ui->BanqChequpComboBox->addItem(model->item(i)->text(), model->item(i,1)->text());

    QStringList ListeRubriques;
    ListeRubriques << tr("Apport praticien") << tr("Divers et autres recettes");
    ui->RefFiscalecomboBox->insertItems(0,ListeRubriques);
    ui->RefFiscalecomboBox->setCurrentText(ListeRubriques.at(0));

    m_listemoyensdepaiement << CHEQUE;
    m_listemoyensdepaiement << ESPECES;
    m_listemoyensdepaiement << VIREMENT;
    ui->PaiementcomboBox->insertItems(0,m_listemoyensdepaiement );
    ui->PaiementcomboBox->setCurrentText(m_listemoyensdepaiement.at(0));

    QDoubleValidator *val= new QDoubleValidator(this);
    val->setDecimals(2);
    ui->MontantlineEdit->setValidator(val);

    QList<UpPushButton *> allUpButtons = this->findChildren<UpPushButton *>();
    for (int n = 0; n <  allUpButtons.size(); n++)
        allUpButtons.at(n)->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Mid);
    ui->GestionComptesupPushButton->resize(290,48);

    DefinitArchitectureBigTable();

    ReconstruitListeAnnees();

    connect (ui->GestionComptesupPushButton,    &QPushButton::clicked, this,          [=] {GestionComptes();});
    connect (ui->NouvelleRecetteupPushButton,   &QPushButton::clicked, this,          [=] {GererRecette(ui->NouvelleRecetteupPushButton);});
    connect (ui->ModifierupPushButton,          &QPushButton::clicked, this,          [=] {GererRecette(ui->ModifierupPushButton);});
    connect (ui->OKupPushButton,                &QPushButton::clicked, this,          [=] {accept();});
    connect (ui->MontantlineEdit,               &QLineEdit::editingFinished, this,    [=] {ConvertitDoubleMontant();});
    connect (ui->PaiementcomboBox,              QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                             this,    [=](int) {ChoixPaiement();});
    connect (ui->ObjetlineEdit,                 &QLineEdit::textEdited, this,         [=] {EnableModifiepushButton();});
    connect (ui->MontantlineEdit,               &QLineEdit::textEdited, this,         [=] {EnableModifiepushButton();});
    connect (ui->DateRecdateEdit,               &QDateEdit::dateChanged, this,        [=] {EnableModifiepushButton();});
    connect (ui->PaiementcomboBox,              &QComboBox::currentTextChanged, this, [=] {EnableModifiepushButton();});
    connect (ui->RefFiscalecomboBox,            &QComboBox::currentTextChanged, this, [=] {EnableModifiepushButton();});
    connect (ui->SupprimerupPushButton,         &QPushButton::clicked, this,          [=] {SupprimerRecette();});

    connect (wdg_enreguppushbutton,             &QPushButton::clicked, this,          [=] {m_mode == Enregistrer? EnregistreRecette() : ModifierRecette();});
    connect (wdg_annuluppushbutton,             &QPushButton::clicked, this,          [=] {AnnulEnreg();});

    QString year = m_currentdate.toString("yyyy");
    int idx = ui->AnneecomboBox->findText(year);
    ui->AnneecomboBox->setCurrentIndex(idx==-1? ui->AnneecomboBox->count()-1 : idx);
    connect (ui->AnneecomboBox,                 QOverload<int>::of(&QComboBox::currentIndexChanged), this,    [=](int) {RedessineBigTable();});
    RedessineBigTable();

    wdg_bigtable->setFocus();

    RegleComptesComboBox(false);

    setFixedWidth(wdg_bigtable->width() + ui->frame->width() + layout()->contentsMargins().left()*3);
    m_initok = true;
}

void    dlg_recettesspeciales::RegleAffichageFiche(Mode mode, bool majfiche)
{
    m_mode = mode;
    ui->DateRecdateEdit     ->setVisible(m_mode != TableVide);
    ui->ObjetlineEdit       ->setVisible(m_mode != TableVide);
    ui->MontantlineEdit     ->setVisible(m_mode != TableVide);
    ui->PaiementcomboBox    ->setVisible(m_mode != TableVide);

    ui->BanqueChequewidget  ->setVisible(m_mode != TableVide);
    ui->Tireurwidget        ->setVisible(m_mode != TableVide);
    ui->CompteWidget        ->setVisible(m_mode != TableVide);
    ui->RefFiscalecomboBox  ->setVisible(m_mode != TableVide);
    ui->DateRecettelabel    ->setVisible(m_mode != TableVide);
    ui->Objetlabel          ->setVisible(m_mode != TableVide);
    ui->Montantlabel        ->setVisible(m_mode != TableVide);
    ui->Paiementlabel       ->setVisible(m_mode != TableVide);
    ui->RefFiscalelabel     ->setVisible(m_mode != TableVide);

    ui->DateRecdateEdit     ->setEnabled(m_mode != Lire);
    ui->ObjetlineEdit       ->setEnabled(m_mode != Lire);
    ui->MontantlineEdit     ->setEnabled(m_mode != Lire);
    ui->Tireurlabel         ->setEnabled(m_mode != Lire);
    ui->BanqueChequelabel   ->setEnabled(m_mode != Lire);
    ui->TireurlineEdit      ->setEnabled(m_mode != Lire);
    ui->PaiementcomboBox    ->setEnabled(m_mode != Lire);
    ui->BanqChequpComboBox  ->setEnabled(m_mode != Lire);
    ui->ComptesupComboBox   ->setEnabled(m_mode != Lire);
    ui->Comptelabel         ->setEnabled(m_mode != Lire);
    ui->RefFiscalecomboBox  ->setEnabled(m_mode != Lire);
    ui->DateRecettelabel    ->setEnabled(m_mode != Lire);
    ui->Objetlabel          ->setEnabled(m_mode != Lire);
    ui->Montantlabel        ->setEnabled(m_mode != Lire);
    ui->Paiementlabel       ->setEnabled(m_mode != Lire);
    ui->RefFiscalelabel     ->setEnabled(m_mode != Lire);
    ui->OKupPushButton      ->setEnabled(m_mode == Lire || m_mode == TableVide);
    ui->GestionComptesupPushButton  ->setEnabled(m_mode == Lire || m_mode == TableVide);
    ui->SupprimerupPushButton       ->setVisible(m_mode == Lire);
    ui->SupprimerupPushButton       ->setIcon(Icons::icPoubelle());

    ui->ModifierupPushButton        ->setVisible(m_mode == Lire);
    int sz = currentuser()->listecomptesbancaires().size();
    ui->NouvelleRecetteupPushButton ->setEnabled((m_mode == Lire || m_mode == TableVide) && sz>0);
    ui->NouvelleRecetteupPushButton ->setToolTip((m_mode == Lire || m_mode == TableVide) && sz>0? "" : tr("Vous ne pouvez pas enregistrer de recettes.\nAucun compte bancaire n'est enregistré."));
    wdg_enreguppushbutton           ->setVisible(!(m_mode == Lire || m_mode == TableVide));
    wdg_annuluppushbutton           ->setVisible(!(m_mode == Lire || m_mode == TableVide));
    wdg_bigtable                    ->setEnabled(m_mode == Lire);
    if (wdg_bigtable->rowCount()==0 && m_mode== Enregistrer)
    {
        ui->Tireurwidget        ->setVisible(false);
        ui->BanqueChequewidget  ->setVisible(false);
        ui->PaiementcomboBox    ->setCurrentText(VIREMENT);
    }
    RegleComptesComboBox(m_mode==Enregistrer);
    if (majfiche)
        MetAJourFiche();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Clic sur AnnulupPushButton ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::AnnulEnreg()
{
    RegleAffichageFiche(wdg_bigtable->rowCount()>0? Lire : TableVide);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Change l'utilisateur courant ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
bool dlg_recettesspeciales::initializeUserSelected()
{
    if( currentuser()->listecomptesbancaires(true).size() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible de continuer!"), tr("Pas de compte bancaire enregistré pour ") + currentuser()->login());
        return false;
    }
    if (currentuser()->idcomptepardefaut() == 0)
    {
        UpMessageBox::Watch(this,tr("Impossible d'ouvrir la fiche recettes spéciales!"), tr("Pas de compte bancaire enregistré pour ")
                                     + currentuser()->login());
        return false;
    }

    return true;
}

void dlg_recettesspeciales::ChoixPaiement()
{
    ui->BanqueChequewidget  ->setVisible(ui->PaiementcomboBox->currentText() == CHEQUE);
    ui->Tireurwidget        ->setVisible(ui->PaiementcomboBox->currentText() == CHEQUE);
    ui->CompteWidget        ->setVisible(ui->PaiementcomboBox->currentText() == VIREMENT);
}

void dlg_recettesspeciales::ConvertitDoubleMontant()
{
    QString b = ui->MontantlineEdit->text();
    b = QLocale().toString(QLocale().toDouble(b),'f',2);
    ui->MontantlineEdit->setText(b);
}

void dlg_recettesspeciales::EnableModifiepushButton()
{
    ui->ModifierupPushButton->setEnabled(true);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Enregistrer une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::EnregistreRecette()
{
    bool OnSauteLaQuestionSuivante = false;
    QString pb = "";
    QString Paiement;
    Paiement = ui->PaiementcomboBox->currentText();

    if (!VerifSaisie())
        return;

    // vérifier que cette recette n'a pas été déjà saisie
    bool OK = true;
    QList<QVariantList> listrec =
            db->StandardSelectSQL("select " CP_DATE_AUTRESRECETTES " from " TBL_RECETTESSPECIALES
                                  " where " CP_DATE_AUTRESRECETTES " = '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") + "'"
                                  " and " CP_LIBELLE_AUTRESRECETTES " = '"  + Utils::correctquoteSQL(ui->ObjetlineEdit->text())  + "'"
                                  " and " CP_MONTANT_AUTRESRECETTES " = "   + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                                  " and " CP_IDUSER_AUTRESRECETTES " = "    + QString::number(currentuser()->id()),OK);
    if (listrec.size() > 0)
    {
        pb = tr("Elle a déjà été saisie");
        OnSauteLaQuestionSuivante = true;
    }

    if (!OnSauteLaQuestionSuivante)
    {
        if (m_currentdate > ui->DateRecdateEdit->date().addDays(90))
            pb = tr("Elle date de plus de 3 mois");
        bool OK = true;
        listrec = db->StandardSelectSQL(
                "select " CP_DATE_AUTRESRECETTES
                " from " TBL_RECETTESSPECIALES
                " where " CP_DATE_AUTRESRECETTES " > '" + ui->DateRecdateEdit->date().addDays(-180).toString("yyyy-MM-dd") + "' "
                " and " CP_LIBELLE_AUTRESRECETTES " = '" + Utils::correctquoteSQL(ui->ObjetlineEdit->text()) + "' "
                " and " CP_MONTANT_AUTRESRECETTES " = " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
                " and " CP_IDUSER_AUTRESRECETTES " = " + QString::number(currentuser()->id()),OK);
        if (listrec.size() > 0)
        {
            if (pb != "")
                pb += "\n";
            pb += tr("Une recette semblable a été saisie le ") + QLocale::system().toString(listrec.last().at(0).toDate(),"dd MMM yyyy");
        }
    }

    if (pb != "")
    {
        UpMessageBox msgbox(this);
        msgbox.setText(tr("Il y a un problème avec cette recette!" ));
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

    // Insertion de l'écriture dans la table autresrecettes
    QString idRec, m;
    if (Paiement == ESPECES)              m = ESP;
    else if (Paiement == VIREMENT)        m = VRMT;
    else if (Paiement == CHEQUE)          m = CHQ;

    QStringList listtables;
    listtables << TBL_RECETTESSPECIALES << TBL_ARCHIVESBANQUE << TBL_LIGNESCOMPTES;
    if (!db->createtransaction(listtables))
        return;
    if (!db->StandardSQL("insert into " TBL_RECETTESSPECIALES " ("
                         CP_DATE_AUTRESRECETTES ", "
                         CP_IDUSER_AUTRESRECETTES ", "
                         CP_LIBELLE_AUTRESRECETTES ", "
                         CP_MONTANT_AUTRESRECETTES ", "
                         CP_TYPERECETTE_AUTRESRECETTES ", "
                         CP_PAIEMENT_AUTRESRECETTES ", "
                         CP_IDCPTEVIREMENT_AUTRESRECETTES ", "
                         CP_TIREURCHEQUE_AUTRESRECETTES ", "
                         CP_BANQUECHEQUE_AUTRESRECETTES
                         ")"
            " VALUES ('" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") +
            "', " + QString::number(currentuser()->id()) +
            ", '" + Utils::correctquoteSQL(ui->ObjetlineEdit->text()) +
            "', " + QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) +
            ", '" + Utils::correctquoteSQL(ui->RefFiscalecomboBox->currentText()) +
            "', '" + m +
            "', " + (m==VRMT||m==ESP? ui->ComptesupComboBox->currentData().toString()    : "null") +
            ", "  + (m==CHQ? "'" + ui->TireurlineEdit->text() + "'"  : "null") +
            ", "  + (m==CHQ? "'" + ui->BanqChequpComboBox->currentText() + "'"  : "null") + ")",tr("Impossible d'enregister cete recette!")))

    {
        db->rollback();
        return;
    }
    bool ok;
    idRec       = QString::number(db->selectMaxFromTable(CP_ID_AUTRESRECETTES, TBL_RECETTESSPECIALES, ok));

    // insertion de l'écriture dans la table lignescomptes quand il s'agit d'un virement ou d'un dépôt d'espèces
    if (Paiement == VIREMENT)
    {
        Paiement = tr("Virement crébiteur");
        int a = db->getIdMaxTableComptesTableArchives();
        if (!db->StandardSQL("insert into " TBL_LIGNESCOMPTES "("
                             CP_ID_LIGNCOMPTES ", "
                             CP_IDCOMPTE_LIGNCOMPTES ", "
                             CP_IDRECSPEC_LIGNCOMPTES ", "
                             CP_DATE_LIGNCOMPTES ", "
                             CP_LIBELLE_LIGNCOMPTES ", "
                             CP_MONTANT_LIGNCOMPTES ", "
                             CP_DEBITCREDIT_LIGNCOMPTES ", "
                             CP_TYPEOPERATION_LIGNCOMPTES
                             ") VALUES (" +
                            QString::number(a) + "," +
                            ui->ComptesupComboBox->currentData().toString() + ", " +
                            idRec + ", '" +
                            ui->DateRecdateEdit->date().toString("yyyy-MM-dd") + "', '" +
                            Utils::correctquoteSQL(ui->ObjetlineEdit->text()) + "', "  +
                            QString::number(QLocale().toDouble(ui->MontantlineEdit->text())) + ", "
                            "1, '" +
                            Paiement + "'"
                            ")"))
        {
            db->rollback();
            return;
        }
    }
    db->commit();

    wdg_bigtable->setEnabled(true);
    ui->SupprimerupPushButton->setVisible(true);
    ui->ModifierupPushButton->setVisible(true);
    ui->NouvelleRecetteupPushButton->setEnabled(true);

    int annee =  ui->DateRecdateEdit->date().year();
    if (ui->AnneecomboBox->currentText() != QString::number(annee))
    {
        if (ui->AnneecomboBox->findText(QString::number(annee)) == -1)
            ReconstruitListeAnnees();
        ui->AnneecomboBox->disconnect();
        ui->AnneecomboBox->setCurrentText(QString::number(annee));
        connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged), this,  [=](int) {RedessineBigTable();});
    }
    RedessineBigTable();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Gerer une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::GererRecette(QPushButton *widgsender)
{
    if (widgsender == ui->ModifierupPushButton)
    {
        if (m_mode == Lire)
            RegleAffichageFiche(Modifier);
        else
            RegleAffichageFiche(wdg_bigtable->rowCount()>0? Lire : TableVide);
    }
    else
        RegleAffichageFiche(Enregistrer);
}

void dlg_recettesspeciales::MenuContextuel(int id)
{
    QMenu *menu;
    menu = new QMenu(this);

    QAction *pAction_RecopieDep = menu->addAction(tr("Effectuer une copie de cette recette à la date d'aujourd'hui"));
    connect (pAction_RecopieDep, &QAction::triggered, this,   [=] {ChoixMenu(id);});
    QAction *pAction_ChercheVal = menu->addAction(tr("Rechercher une valeur"));
    connect (pAction_ChercheVal, &QAction::triggered, this,   [=] {ChoixMenu();});

    // ouvrir le menu
    menu->exec(cursor().pos());
}

void dlg_recettesspeciales::ChoixMenu(int id)
{
    if (id == -1)
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
        Utils::EnChantier();
    }
    else
    {
        MetAJourFiche();
        m_mode = Enregistrer;
        ui->DateRecdateEdit             ->setEnabled(true);
        ui->ObjetlineEdit               ->setEnabled(true);
        ui->MontantlineEdit             ->setEnabled(true);
        ui->PaiementcomboBox            ->setEnabled(true);
        ui->BanqChequpComboBox          ->setEnabled(true);
        ui->BanqueChequelabel           ->setEnabled(true);
        ui->Comptelabel                 ->setEnabled(true);
        ui->ComptesupComboBox           ->setEnabled(true);
        ui->RefFiscalecomboBox          ->setEnabled(true);
        ui->DateRecettelabel            ->setEnabled(true);
        ui->Objetlabel                  ->setEnabled(true);
        ui->Montantlabel                ->setEnabled(true);
        ui->Paiementlabel               ->setEnabled(true);
        ui->RefFiscalelabel             ->setEnabled(true);
        ui->OKupPushButton              ->setEnabled(false);
        ui->GestionComptesupPushButton  ->setEnabled(false);
        ui->SupprimerupPushButton       ->setVisible(false);
        ui->ModifierupPushButton        ->setVisible(false);
        ui->NouvelleRecetteupPushButton ->setEnabled(false);
        wdg_enreguppushbutton           ->setVisible(true);
        wdg_annuluppushbutton           ->setVisible(true);
        wdg_bigtable                    ->setEnabled(false);
        wdg_bigtable                    ->disconnect();
        ui->DateRecdateEdit             ->setDate(m_currentdate);
        wdg_enreguppushbutton           ->setText("Enregistrer");
        ui->OKupPushButton              ->setShortcut(QKeySequence());
        ui->ModifierupPushButton        ->setShortcut(QKeySequence());
        wdg_enreguppushbutton           ->setShortcut(QKeySequence("Meta+Return"));
    }
}

void dlg_recettesspeciales::RegleComptesComboBox(bool ActiveSeult)
{
    ui->ComptesupComboBox->clear();
    foreach (int idcpt, currentuser()->listecomptesbancaires() )
    {
        Compte *cpt = Datas::I()->comptes->getById(idcpt);
        if (cpt)
        {
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

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Supprimer une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::SupprimerRecette()
{
    if (wdg_bigtable->selectedRanges().size() == 0) return;
    // s'il s'agit d'une dépense par transaction bancaire, on vérifie qu'elle n'a pas été enregistrée sur le compte
    bool OK = true;
    QList<QVariantList> listidrecspec = db->StandardSelectSQL(" select " CP_IDRECSPEC_ARCHIVESCPT " from " TBL_ARCHIVESBANQUE " where " CP_IDRECSPEC_ARCHIVESCPT " = " + QString::number(m_idrecetteencours),OK);
    if (listidrecspec.size() > 0)
    {
        UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
        return;
    }

    QList<QVariantList> listremises = db->StandardSelectSQL(" select " CP_IDREMISECHQ_AUTRESRECETTES " from " TBL_RECETTESSPECIALES " where " CP_ID_AUTRESRECETTES " = " + QString::number(m_idrecetteencours),OK);
    if (listremises.size() > 0)
        if (listremises.at(0).at(0).toInt()>0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
            return;
        }

    QDate   Dateop;
    QString Libelle;
    double  montant;
    QList<QVariantList> listrecettes = db->StandardSelectSQL("select " CP_DATE_AUTRESRECETTES ", " CP_LIBELLE_AUTRESRECETTES ", " CP_MONTANT_AUTRESRECETTES " from " TBL_RECETTESSPECIALES " where " CP_ID_AUTRESRECETTES " = " + QString::number(m_idrecetteencours),OK);
    if (listrecettes.size() > 0)
    {
        Dateop  = listrecettes.at(0).at(0).toDate();
        Libelle = listrecettes.at(0).at(1).toString();
        montant = listrecettes.at(0).at(2).toDouble();
        bool ok = true;
        QList<QVariantList> listlignes = db->StandardSelectSQL("select " CP_ID_ARCHIVESCPT " from " TBL_ARCHIVESBANQUE " where " CP_DATE_ARCHIVESCPT " = '" + Dateop.toString("yyyy-MM-dd")
                + "' and LigneLibelle = '" + Utils::correctquoteSQL(Libelle) + "' and LigneMontant = " + QString::number(montant), ok);
        if (listlignes.size()> 0)
        {
            UpMessageBox::Watch(this,tr("Vous ne pouvez pas supprimer cette écriture"), tr("Elle a déjà été enregistrée sur le compte bancaire"));
            return;
        }
        QString daterec = QLocale::system().toString(Dateop,"dd MMM yyyy");
        UpMessageBox msgbox(this);
        msgbox.setText(tr("Supprimer une recette!"));
        msgbox.setInformativeText(tr("Confirmer la suppression de\n") + daterec + "\n" + Libelle + "\n" + QLocale().toString(montant,'f',2) + "?");
        msgbox.setIcon(UpMessageBox::Warning);
        UpSmallButton OKBouton(tr("Supprimer"));
        UpSmallButton NoBouton(tr("Annuler"));
        msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
        msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
        msgbox.exec();
        if (msgbox.clickedButton() != &OKBouton)
            return;

        //On supprime l'écriture
        db->SupprRecordFromTable(m_idrecetteencours, CP_IDRECSPEC_LIGNCOMPTES, TBL_LIGNESCOMPTES);
        db->SupprRecordFromTable(m_idrecetteencours, CP_ID_AUTRESRECETTES, TBL_RECETTESSPECIALES);
        if (wdg_bigtable->rowCount() == 1)
        {
            ui->AnneecomboBox->disconnect();
            RedessineBigTable();
            ReconstruitListeAnnees();
            QString year = m_currentdate.toString("yyyy");
            int idx = ui->AnneecomboBox->findText(year);
            ui->AnneecomboBox->setCurrentIndex(idx==-1? ui->AnneecomboBox->count()-1 : idx);
            connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged), this,   [=](int) {RedessineBigTable();});
        }
        else
        {
            for (int i = 0; i< wdg_bigtable->rowCount(); i++)
            {
                UpLabel *iddeplbl = static_cast<UpLabel *>(wdg_bigtable->cellWidget(i,0));
                if (iddeplbl->text() == QString::number(m_idrecetteencours))
                {
                    wdg_bigtable->removeRow(i);
                    if (i < wdg_bigtable->rowCount() - 1)
                        wdg_bigtable->setCurrentCell(i,1);
                    else
                        wdg_bigtable->setCurrentCell(wdg_bigtable->rowCount()-1,1);
                    i = wdg_bigtable->rowCount();
                }
            }
        }
        CalculTotalRecettes();
        RegleAffichageFiche(wdg_bigtable->rowCount()>0? Lire : TableVide);
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Recalcule le total des recettes -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::CalculTotalRecettes()
{
    double Total = 0;
    if (wdg_bigtable->rowCount() > 0)
    {
        for (int k = 0; k < wdg_bigtable->rowCount(); k++)
        {
            UpLabel* Line = dynamic_cast<UpLabel*>(wdg_bigtable->cellWidget(k,3));
            if (Line)
                Total += QLocale().toDouble(Line->text());
        }
    }
    QString TotalRecettes = QLocale().toString(Total,'f',2);
    ui->TotallineEdit->setText(tr("Total ")  + ui->AnneecomboBox->currentText() + " -> " + TotalRecettes);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Ouvre la gestion des comptes -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::GestionComptes()
{
    dlg_comptes *Dlg_Cmpt          = new dlg_comptes(this);
    Dlg_Cmpt->setWindowModality(Qt::WindowModal);
    if (Dlg_Cmpt->initOK())
        Dlg_Cmpt->exec();
    delete Dlg_Cmpt;
    wdg_bigtable->setCurrentCell(wdg_bigtable->rowCount()-1,1);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Met à jour la fiche --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::MetAJourFiche()
{
    if (m_mode == Lire)
    {
        ui->ModifierupPushButton        ->setEnabled(m_currentdate < ui->DateRecdateEdit->date().addDays(365));
        ui->SupprimerupPushButton       ->setEnabled(m_currentdate < ui->DateRecdateEdit->date().addDays(365));
    }
    if (wdg_bigtable->rowCount() > 0)
    {
        ui->DateRecdateEdit     ->disconnect();
        ui->RefFiscalecomboBox  ->disconnect();
        ui->PaiementcomboBox    ->disconnect();

        UpLabel* lbl = static_cast<UpLabel*>(wdg_bigtable->cellWidget(wdg_bigtable->currentRow(),0));
        if (lbl == Q_NULLPTR)
            return;
        m_idrecetteencours = lbl->text().toInt();

        //TODO : SQL
        bool OK = true;
        QList<QVariantList> listrecettes =
                db->StandardSelectSQL("select "
                                      CP_DATE_AUTRESRECETTES ", "
                                      CP_LIBELLE_AUTRESRECETTES ", "
                                      CP_MONTANT_AUTRESRECETTES ", "
                                      CP_PAIEMENT_AUTRESRECETTES ", "
                                      CP_IDREMISECHQ_AUTRESRECETTES ", "
                                      CP_TYPERECETTE_AUTRESRECETTES ", "
                                      CP_IDCPTEVIREMENT_AUTRESRECETTES ", "
                                      CP_BANQUECHEQUE_AUTRESRECETTES ", "
                                      CP_TIREURCHEQUE_AUTRESRECETTES
                                      " from " TBL_RECETTESSPECIALES " where " CP_ID_AUTRESRECETTES " = " + QString::number(m_idrecetteencours),OK);
        QVariantList recette = listrecettes.at(0);

        ui->ObjetlineEdit->setText(recette.at(1).toString());
        ui->DateRecdateEdit->setDate(recette.at(0).toDate());
        ui->MontantlineEdit->setText(QLocale().toString(recette.at(2).toDouble(),'f',2));
        QString A = recette.at(3).toString();                                                         // Mode de paiement - col = 4
        QString B = "";
        ui->CompteWidget        ->setVisible(false);
        ui->BanqueChequewidget  ->setVisible(false);
        ui->Tireurwidget        ->setVisible(false);
        ui->ComptesupComboBox   ->setCurrentIndex(-1);
        ui->BanqChequpComboBox  ->setCurrentIndex(-1);

        if (A == ESP)
            A = ESPECES;
        else if (A == VRMT)
        {
            if (recette.at(6).toInt()>0)
            {
                int idx = currentuser()->listecomptesbancaires(true).indexOf(recette.at(6).toInt());
                if( idx > -1 )
                {
                    Compte *cpt = Datas::I()->comptes->getById(currentuser()->listecomptesbancaires(true).at(idx));
                    if (cpt)
                        B = cpt->nomabrege();
                    ui->CompteWidget        ->setVisible(true);
                    ui->ComptesupComboBox   ->setCurrentIndex(ui->ComptesupComboBox->findData(recette.at(6).toInt()));
                }
            }
            A = VIREMENT;
        }
        else if (A == CHQ)
        {
            if (recette.at(7).toString() != "")
            {
                B = recette.at(7).toString();
                ui->BanqueChequewidget  ->setVisible(true);
                ui->BanqChequpComboBox  ->setCurrentIndex(ui->BanqChequpComboBox->findText(B));
            }
            if (recette.at(8).toString() != "")
            {
                B = recette.at(8).toString();
                ui->Tireurwidget    ->setVisible(true);
                ui->TireurlineEdit  ->setText(B);
            }
            A = CHEQUE;
        }
        ui->PaiementcomboBox    ->setCurrentIndex(ui->PaiementcomboBox->findText(A));
        ui->RefFiscalecomboBox  ->setCurrentText(recette.at(5).toString());

        connect (ui->DateRecdateEdit,       &QDateEdit::dateChanged,                             this,   [=] {EnableModifiepushButton();});
        connect (ui->PaiementcomboBox,      &QComboBox::currentTextChanged,                      this,   [=] {EnableModifiepushButton();});
        connect (ui->PaiementcomboBox,      QOverload<int>::of(&QComboBox::currentIndexChanged), this,   [=](int) {ChoixPaiement();});
        connect (ui->RefFiscalecomboBox,    &QComboBox::currentTextChanged,                      this,   [=] {EnableModifiepushButton();});

        QString Paiement = ui->PaiementcomboBox->currentText();
        bool modifiable = false;

        switch (m_mode) {
        case Enregistrer:
            ui->ObjetlineEdit->clear();
            ui->MontantlineEdit->setText(QLocale().toString(0.00,'f',2));
            ui->DateRecdateEdit->setDate(m_currentdate);
            ui->CompteWidget        ->setVisible(true);
            ui->ComptesupComboBox   ->setCurrentIndex(ui->ComptesupComboBox->findData(currentuser()->idcomptepardefaut()));
            ui->PaiementcomboBox    ->setCurrentText(VIREMENT);
            break;
        case Modifier:
            if (Paiement == VIREMENT)
                // on recherche si l'écriture existe dans archivesbanques et si c'est le cas, on ne peut pas modifier le montant
            {
                bool ok = true;
                QList<QVariantList> listlignes = db->StandardSelectSQL("select " CP_ID_ARCHIVESCPT " from " TBL_ARCHIVESBANQUE " where " CP_IDRECSPEC_ARCHIVESCPT " = " + QString::number(m_idrecetteencours),ok);
                if (ok)
                    modifiable = (listlignes.size() == 0);
                ui->MontantlineEdit->setEnabled(modifiable);
                ui->PaiementcomboBox->setEnabled(modifiable);
                ui->CompteWidget    ->setVisible(modifiable);
                ui->Comptelabel->setEnabled(modifiable);
                ui->ComptesupComboBox->setEnabled(modifiable);
            }
            else if (Paiement == CHEQUE)
                // on recherche si le chéque a été déposé et si c'est le cas, on ne peut pas modifier le montant
            {
                bool ok = true;
                QList<QVariantList> listlignes = db->StandardSelectSQL("select " CP_IDREMISECHQ_AUTRESRECETTES " from " TBL_RECETTESSPECIALES " where " CP_ID_AUTRESRECETTES " = " + QString::number(m_idrecetteencours),ok);
                if (ok)
                    modifiable = !(listlignes.at(0).at(0).toInt()>0);
                ui->MontantlineEdit     ->setEnabled(modifiable);
                ui->PaiementcomboBox    ->setEnabled(modifiable);
                ui->BanqChequpComboBox  ->setEnabled(modifiable);
                ui->BanqueChequelabel   ->setEnabled(modifiable);
                ui->Tireurlabel         ->setEnabled(modifiable);
                ui->TireurlineEdit      ->setEnabled(modifiable);
                ui->Tireurwidget        ->setVisible(modifiable);
                ui->BanqueChequewidget  ->setVisible(modifiable);
            }
            break;
        case Lire:
            if (Paiement != ESPECES)
                // on recherche si l'écriture existe dans archivesbanques et si c'est le cas, on ne peut pas modifier le montant
            {
                bool ok = true;
                QList<QVariantList> listlignes = db->StandardSelectSQL("select " CP_ID_ARCHIVESCPT " from " TBL_ARCHIVESBANQUE " where " CP_IDRECSPEC_ARCHIVESCPT " = " + QString::number(m_idrecetteencours),ok);
                if (ok)
                    modifiable = (listlignes.size() == 0);
                ui->SupprimerupPushButton   ->setEnabled(modifiable);
            }

        default:
            break;
        }
    }
    else
    {
        ui->ModifierupPushButton        ->setVisible(false);
        ui->SupprimerupPushButton       ->setVisible(false);
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Modifier une recette ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::ModifierRecette()
{
    QString idRec       = QString::number(m_idrecetteencours);

    if (!VerifSaisie())
        return;
    // on reconstruit les renseignements de la recette à modifier
    bool ok = true;
    QList<QVariantList> listpaiements = db->StandardSelectSQL("select "
                                                              CP_PAIEMENT_AUTRESRECETTES ", "
                                                              CP_IDCPTEVIREMENT_AUTRESRECETTES ", "
                                                              CP_BANQUECHEQUE_AUTRESRECETTES ", "
                                                              CP_IDREMISECHQ_AUTRESRECETTES
                                                              " from " TBL_RECETTESSPECIALES " where " CP_ID_AUTRESRECETTES " = " + idRec,ok);
    if (listpaiements.size()==0)
    {
        EnregistreRecette();
        return;
    }
    QString ancpaiement = listpaiements.at(0).at(0).toString();

    if (ancpaiement == ESP)
    {
        db->SupprRecordFromTable(m_idrecetteencours, CP_ID_AUTRESRECETTES, TBL_RECETTESSPECIALES);
        EnregistreRecette();
    }
    else if (ancpaiement == CHQ)
    {
        if (listpaiements.at(0).at(3).toInt()>0)                // le cheque a été remis en banque, on se contente de mettre à jour la date, la rubrique fiscale et l'intitulé dans autresrecette
            db->StandardSQL("update " TBL_RECETTESSPECIALES " set "
                  CP_DATE_AUTRESRECETTES " = '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") + "', "
                  CP_LIBELLE_AUTRESRECETTES " = '" + Utils::correctquoteSQL(ui->ObjetlineEdit->text()) +"', "
                  CP_TYPERECETTE_AUTRESRECETTES " = '" + ui->RefFiscalecomboBox->currentText() + "'"
                  " where  " CP_ID_AUTRESRECETTES " = " + idRec);
        else                                                    // le cheque n'a pas été remis en banque, on remet tout à jour
        {
            db->SupprRecordFromTable(m_idrecetteencours, CP_ID_AUTRESRECETTES, TBL_RECETTESSPECIALES);
            QList<QVariantList> listlignescomptes = db->SelectRecordsFromTable(QStringList() << CP_ID_LIGNCOMPTES,
                                                                                  TBL_LIGNESCOMPTES, ok,
                                                                                  "where " CP_IDRECSPEC_LIGNCOMPTES " = " + idRec);
            if (listlignescomptes.size() > 0)                // l'écriture existe et on la supprime
                db->SupprRecordFromTable(listlignescomptes.at(0).at(0).toInt(), CP_ID_LIGNCOMPTES, TBL_LIGNESCOMPTES);
            EnregistreRecette();
        }
    }
    else if (ancpaiement == VRMT)
    {
        QString         Paiement = ui->PaiementcomboBox->currentText();
        bool ok = true;
        QList<QVariantList> listlignesarchives = db->SelectRecordsFromTable(QStringList() << CP_ID_ARCHIVESCPT,
                                                                               TBL_ARCHIVESBANQUE, ok,
                                                                               "where " CP_IDRECSPEC_ARCHIVESCPT " = " + idRec);
        if (listlignesarchives.size() > 0)                // l'écriture existe et on la modifie
        {
            QHash<QString, QVariant> listsets;
            listsets.insert(CP_DATE_ARCHIVESCPT,            ui->DateRecdateEdit->date().toString("yyyy-MM-dd"));
            listsets.insert(CP_LIBELLE_ARCHIVESCPT,         ui->ObjetlineEdit->text());
            listsets.insert(CP_DEBITCREDIT_ARCHIVESCPT,     "1");
            listsets.insert(CP_TYPEOPERATION_ARCHIVESCPT,   Paiement);
            DataBase:: I()->UpdateTable(TBL_ARCHIVESBANQUE,
                                        listsets,
                                        " where " CP_IDRECSPEC_ARCHIVESCPT " = " + idRec);
            db->StandardSQL("update " TBL_RECETTESSPECIALES " set "
                  CP_DATE_AUTRESRECETTES " = '" + ui->DateRecdateEdit->date().toString("yyyy-MM-dd") + "', "
                  CP_LIBELLE_AUTRESRECETTES " = '" + Utils::correctquoteSQL(ui->ObjetlineEdit->text()) +"', "
                  CP_TYPERECETTE_AUTRESRECETTES " = '" + ui->RefFiscalecomboBox->currentText() + "'"
                  " where  " CP_ID_AUTRESRECETTES " = " + idRec);
        }
        else
        {
            db->SupprRecordFromTable(m_idrecetteencours, CP_ID_AUTRESRECETTES, TBL_RECETTESSPECIALES);
            QList<QVariantList> listlignescomptes = db->SelectRecordsFromTable(QStringList() << CP_ID_LIGNCOMPTES,
                                                                               TBL_LIGNESCOMPTES, ok,
                                                                               "where " CP_IDRECSPEC_LIGNCOMPTES " = " + idRec);
            if (listlignescomptes.size() > 0)                // l'écriture existe et on la modifie
                db->SupprRecordFromTable(listlignescomptes.at(0).at(0).toInt(), CP_ID_LIGNCOMPTES, TBL_LIGNESCOMPTES);
            EnregistreRecette();
        }
    }
    RedessineBigTable(idRec.toInt());
    RegleAffichageFiche(wdg_bigtable->rowCount()>0? Lire : TableVide);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged),  this,  [=](int) {RedessineBigTable();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::RedessineBigTable(int idRec)
{
    RemplitBigTable();
    CalculTotalRecettes();
    if (wdg_bigtable->rowCount() > 0)
    {
        if (idRec != -1)
        {
            bool trouve = false;
            for (int row=0; row< wdg_bigtable->rowCount(); row++)
            {
                UpLabel* idReclbl = static_cast<UpLabel*>(wdg_bigtable->cellWidget(row,0));
                if (idReclbl->text() == QString::number(idRec))
                {
                    wdg_bigtable->setCurrentCell(row,1);
                    wdg_bigtable->scrollTo(wdg_bigtable->model()->index(row,1));
                    trouve = true;
                }
            }
            if (!trouve)
            {
                wdg_bigtable->setCurrentCell(wdg_bigtable->rowCount()-1,1);
                wdg_bigtable->scrollTo(wdg_bigtable->model()->index(wdg_bigtable->model()->rowCount()-1,1));
            }
            m_mode = Lire;
        }
        else
        {
            wdg_bigtable->setCurrentCell(wdg_bigtable->rowCount()-1,1);
            wdg_bigtable->scrollTo(wdg_bigtable->model()->index(wdg_bigtable->model()->rowCount()-1,1));
        }
    }
    else
        RegleAffichageFiche(TableVide);
}

void dlg_recettesspeciales::closeEvent(QCloseEvent *event)
{
    proc->settings()->setValue(Position_Fiche Nom_fiche_Depenses,saveGeometry());
    event->accept();
}

//-------------------------------------------------------------------------------------
// Interception des évènements clavier
//-------------------------------------------------------------------------------------
void dlg_recettesspeciales::keyPressEvent ( QKeyEvent * event )
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

bool dlg_recettesspeciales::initOK() const
{
    return m_initok;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Definit les colonnes de BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::DefinitArchitectureBigTable()
{
    int                 ColCount;

    ColCount = 7;
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
    LabelARemplir << tr("Recette");
    LabelARemplir << tr("Montant");
    LabelARemplir << tr("Mode de paiement");
    LabelARemplir << tr("Rubrique fiscale");
    LabelARemplir << tr("Classement par date");

    wdg_bigtable->setHorizontalHeaderLabels(LabelARemplir);
    wdg_bigtable->horizontalHeader()->setVisible(true);
    wdg_bigtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    wdg_bigtable->setColumnWidth(li,25);                                               // idRecette
    li++;
    wdg_bigtable->setColumnWidth(li,100);                                              // DateRecette affichage européen
    li++;
    wdg_bigtable->setColumnWidth(li,400);                                              // Libelle
    li++;
    wdg_bigtable->setColumnWidth(li,100);                                              // Montant
    li++;
    wdg_bigtable->setColumnWidth(li,160);                                              // ModePaiement
    li++;
    wdg_bigtable->setColumnWidth(li,300);                                              // Rubrique fiscqale
    li++;
    wdg_bigtable->setColumnWidth(li,0);                                                // DepDate

    wdg_bigtable->setColumnHidden(0,true);
    wdg_bigtable->setColumnHidden(ColCount-1,true);

    wdg_bigtable->setGridStyle(Qt::SolidLine);
    wdg_bigtable->FixLargeurTotale();
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -- Reconstruit la liste des Annees dans le combobox Annees --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::ReconstruitListeAnnees()
{
    bool ok = true;
    QList<QVariantList> listannees =
            db->StandardSelectSQL("SELECT distinct Annee from (SELECT year(" CP_DATE_AUTRESRECETTES ") as Annee FROM " TBL_RECETTESSPECIALES
                                  " WHERE " CP_IDUSER_AUTRESRECETTES " = " + QString::number(currentuser()->id()) + ") as ghf order by Annee",ok);
    QStringList ListeAnnees;
    for (int i = 0; i < listannees.size(); i++)
    {
        //qDebug() << listannees.at(i).at(0).toString();
        ListeAnnees << listannees.at(i).at(0).toString();
    }
    if (listannees.size()==0)
        ListeAnnees << m_currentdate.toString("yyyy");
    ui->AnneecomboBox->disconnect();
    ui->AnneecomboBox->clear();
    ui->AnneecomboBox->insertItems(0,ListeAnnees);
    connect (ui->AnneecomboBox,     QOverload<int>::of(&QComboBox::currentIndexChanged), this,  [=](int) {RedessineBigTable();});
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Remplit BigTableWidget --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void dlg_recettesspeciales::RemplitBigTable()
{
    //TODO : SQL
    QTableWidgetItem    *pItem8;
    QString             A;
    UpLabel *label0, *label1, *label2, *label3, *label4, *label5, *label6;
    wdg_bigtable->disconnect();
    wdg_bigtable->clearContents();
    QString Recrequete = "SELECT " CP_ID_AUTRESRECETTES ", " CP_IDUSER_AUTRESRECETTES ", year(" CP_DATE_AUTRESRECETTES ") as Annee, " CP_DATE_AUTRESRECETTES ", " CP_LIBELLE_AUTRESRECETTES ", " CP_MONTANT_AUTRESRECETTES ", "
                         CP_TYPERECETTE_AUTRESRECETTES ", " CP_NOOPERATION_AUTRESRECETTES ", " CP_MONNAIE_AUTRESRECETTES ", " CP_PAIEMENT_AUTRESRECETTES ", " CP_IDCPTEVIREMENT_AUTRESRECETTES ", " CP_BANQUECHEQUE_AUTRESRECETTES
                         " FROM " TBL_RECETTESSPECIALES
                         " WHERE " CP_IDUSER_AUTRESRECETTES " = " + QString::number(currentuser()->id());
    if (ui->AnneecomboBox->currentText() != "")
        Recrequete += " AND year(DateRecette) = " + ui->AnneecomboBox->currentText();
    Recrequete += " ORDER BY DateRecette, Libelle";
    //qDebug() << Recrequete;
    bool ok = true;
    QList<QVariantList> listrecettes = db->StandardSelectSQL(Recrequete,ok);

    if (listrecettes.size()==0)
        return;
    if (listrecettes.size()==1 && listrecettes.at(0).at(0)==QVariant())
        return;
    wdg_bigtable->setRowCount(listrecettes.size());

    for (int i = 0; i < listrecettes.size(); i++)
        {
            int col = 0;
            QVariantList recette = listrecettes.at(i);
            int id = recette.at(0).toInt();

            label0 = new UpLabel;
            label1 = new UpLabel;
            label2 = new UpLabel;
            label3 = new UpLabel;
            label4 = new UpLabel;
            label5 = new UpLabel;
            label6 = new UpLabel;

            label0->setiD(id);                      // idRecette
            label1->setiD(id);                      // idRecette
            label2->setiD(id);                      // idRecette
            label3->setiD(id);                      // idRecette
            label4->setiD(id);                      // idRecette
            label5->setiD(id);                      // idRecette
            label6->setiD(id);                      // idRecette

            label0->setContextMenuPolicy(Qt::CustomContextMenu);
            label1->setContextMenuPolicy(Qt::CustomContextMenu);
            label2->setContextMenuPolicy(Qt::CustomContextMenu);
            label3->setContextMenuPolicy(Qt::CustomContextMenu);
            label4->setContextMenuPolicy(Qt::CustomContextMenu);
            label5->setContextMenuPolicy(Qt::CustomContextMenu);
            label6->setContextMenuPolicy(Qt::CustomContextMenu);

            connect(label0,  &QWidget::customContextMenuRequested, this,  [=] {MenuContextuel(id);});
            connect(label1,  &QWidget::customContextMenuRequested, this,  [=] {MenuContextuel(id);});
            connect(label2,  &QWidget::customContextMenuRequested, this,  [=] {MenuContextuel(id);});
            connect(label3,  &QWidget::customContextMenuRequested, this,  [=] {MenuContextuel(id);});
            connect(label4,  &QWidget::customContextMenuRequested, this,  [=] {MenuContextuel(id);});
            connect(label5,  &QWidget::customContextMenuRequested, this,  [=] {MenuContextuel(id);});
            connect(label6,  &QWidget::customContextMenuRequested, this,  [=] {MenuContextuel(id);});

            A = recette.at(0).toString();                                                                  // idRecette - col = 0
            label0->setText(A);
            wdg_bigtable->setCellWidget(i,col,label0);
            col++;

            A = QLocale::system().toString(recette.at(3).toDate(),tr("d MMM yyyy"));                       // DateRecette col = 1
            label1->setText(A + " ");
            label1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            wdg_bigtable->setCellWidget(i,col,label1);
            col++;

            label2->setText(" " + recette.at(4).toString());                                               // Libelle - col = 2
            wdg_bigtable->setCellWidget(i,col,label2);
            col++;

            if (recette.at(8).toString() == "F")
                A = QLocale().toString(recette.at(5).toDouble()/6.55957,'f',2);                            // Montant en F converti en euros
            else
                A = QLocale().toString(recette.at(5).toDouble(),'f',2);                                    // Montant - col = 3
            label3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            label3->setText(A + " ");                                                                                   // Montant - col = 3
            wdg_bigtable->setCellWidget(i,col,label3);
            col++;

            A = recette.at(9).toString();                                                                  // Paiement - col = 4
            QString B = "";
            if (A == ESP)
                A = ESPECES;
            else if (A == VRMT)
            {
                if (recette.at(10).toInt() > 0)
                {
                    int idx = currentuser()->listecomptesbancaires(true).indexOf(recette.at(10).toInt());
                    if( idx > -1 )
                    {
                        Compte *cpt = Datas::I()->comptes->getById(currentuser()->listecomptesbancaires(true).at(idx));
                        if (cpt)
                            B = cpt->nomabrege();
                    }
                }
                A = VIREMENT + (B==""? "" : " " + B);
            }
            else if (A == CHQ)
            {
                B = recette.at(11).toString();
                A = CHEQUE + (B==""? "" : " " + B);
            }
            label4->setText(" " + A);
            wdg_bigtable->setCellWidget(i,col,label4);
            col++;

            A = recette.at(6).toString();                                                                  // Famille fiscale - col = 5
            label6->setText(" " + A);
            wdg_bigtable->setCellWidget(i,col,label6);
            col++;

            A = recette.at(3).toDate().toString("yyyy-MM-dd");                                             // ClassementparDate - col = 6
            pItem8 = new QTableWidgetItem() ;
            pItem8->setText(A);
            wdg_bigtable->setItem(i,col,pItem8);

            wdg_bigtable->setRowHeight(i,int(QFontMetrics(qApp->font()).height()*1.3));
        }
    ui->SupprimerupPushButton->setEnabled(false);
    connect (wdg_bigtable,     &QTableWidget::itemSelectionChanged, this, [=] {RegleAffichageFiche(Lire);});
}

//  Vérifer que la saisie est complète et cohérente
bool dlg_recettesspeciales::VerifSaisie()
{
    QString Paiement = ui->PaiementcomboBox->currentText();

    QString Erreur = "";
    if (ui->ObjetlineEdit->text() == "")
        Erreur = tr("l'objet de la recette");
    else if (!(QLocale().toDouble(ui->MontantlineEdit->text()) > 0))
        Erreur = tr("le montant");
    else if (ui->PaiementcomboBox->currentText() == "")
        Erreur = tr("le mode de paiement");
    else if (ui->RefFiscalecomboBox->currentText() == "")
        Erreur = tr("la rubrique fiscale");
    else if (Paiement == VIREMENT)
    {
        if (ui->ComptesupComboBox->currentText()=="")
            Erreur = tr("le compte crédité par le virement");
    }
    else if (Paiement == ESPECES)
    {
        if (ui->ComptesupComboBox->currentText()=="")
            Erreur = tr("le compte crédité par ce versement d'espèces");
    }
    else if (Paiement == CHEQUE)
    {
        if (ui->BanqChequpComboBox->currentText()=="")
            Erreur = tr("la banque émettrice du chèque");
        if (ui->TireurlineEdit->text()=="")
            Erreur = tr("l'a banque émettrice'émetteur du chèque");
    }

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
        else if (Erreur == tr("le compte crédité par le virement"))
            ui->ComptesupComboBox->setFocus();
        else if (Erreur == tr("la banque émettrice du chèque"))
            ui->BanqChequpComboBox->setFocus();
        else if (Erreur == tr("l'a banque émettrice'émetteur du chèque"))
            ui->TireurlineEdit->setFocus();
        return false;
    }
    return true;
}

