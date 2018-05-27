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

#include "dlg_comptes.h"
#include "ui_dlg_comptes.h"

dlg_comptes::dlg_comptes(Procedures *procAPasser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_comptes)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    proc = procAPasser;
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionComptes").toByteArray());
    setAttribute(Qt::WA_DeleteOnClose);

    // On reconstruit le combobox des comptes de l'utilisateur
    QString ChercheComptesrequete = "SELECT NomCompteAbrege, idcompte from " NOM_TABLE_COMPTES " where idUser = " + QString::number(proc->getDataUser()["idUser"].toInt());
    QSqlQuery ChercheComptesQuery (ChercheComptesrequete,proc->getDataBase());
    if (ChercheComptesQuery.size() == 0)
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas de compte bancaire enregistré!"));
        InitOK = false;
        return;
    }
    else
    {
        ChercheComptesQuery.first();
        ui->BanquecomboBox->clear();
        for (int i = 0; i < ChercheComptesQuery.size(); i++)
        {
            ChercheComptesQuery.seek(i);
            ui->BanquecomboBox->addItem(ChercheComptesQuery.value(0).toString(),ChercheComptesQuery.value(1));
        }
        QString chercheComptePrefereRequete = " select nomcompteabrege from " NOM_TABLE_COMPTES
                " where idcompte in (select idcomptepardefaut from " NOM_TABLE_UTILISATEURS " where iduser = " + QString::number(proc->getDataUser()["idUser"].toInt()) + ")";
        QSqlQuery chercheComptePreferQuery (chercheComptePrefereRequete,proc->getDataBase());
        if (chercheComptePreferQuery.size() > 0)
        {
            chercheComptePreferQuery.first();
            ui->BanquecomboBox->setCurrentText(chercheComptePreferQuery.value(0).toString());
        }
        idCompte = ui->BanquecomboBox->currentData().toInt();
        QString SoldeComptereq = " select SoldeSurDernierReleve from " NOM_TABLE_COMPTES " where idCompte = " + QString::number(idCompte);
        QSqlQuery SoldeCompteQuery (SoldeComptereq,proc->getDataBase());
        if (SoldeCompteQuery.size() > 0)
        {
            SoldeCompteQuery.first();
            SoldeSurReleve = SoldeCompteQuery.value(0).toDouble();
            ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(SoldeSurReleve,'f',2) + " ");
            gBigTable = ui->upTableWidget;
            gBigTable->installEventFilter(this);

            DefinitArchitetureTable();
            RemplitLaTable(idCompte);
            connect(ui->AnnulArchivepushButton,             &QPushButton::clicked,                                  [=] {AnnulArchive();});
            connect(ui->ArchiverpushButton,                 &QPushButton::clicked,                                  [=] {Archiver();});
            connect(ui->AnnulerConsolidationspushButton,    &QPushButton::clicked,                                  [=] {AnnulConsolidations();});
            connect(ui->OKpushButton,                       &QPushButton::clicked,                                  [=] {accept();});
            connect(ui->BanquecomboBox,                     QOverload<int>::of(&QComboBox::currentIndexChanged),    [=](int){ChangeCompte(ui->BanquecomboBox->currentIndex());});

            setWindowTitle(tr("Gestion des comptes bancaires"));
            QList<UpPushButton *> allUpButtons = this->findChildren<UpPushButton *>();
            for (int n = 0; n <  allUpButtons.size(); n++)
                allUpButtons.at(n)->setUpButtonStyle(UpPushButton::NORMALBUTTON, UpPushButton::Large);

            ui->OKpushButton->setShortcut(QKeySequence("Meta+Return"));
            InitOK = true;
        }
        else
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas de compte bancaire enregistré!"));
            InitOK = false;
        }
    }
}

dlg_comptes::~dlg_comptes()
{
    delete ui;
}

void dlg_comptes::AnnulArchive()
{
    QSqlQuery ("SET AUTOCOMMIT = 0;", proc->getDataBase());
    QString lockrequete = "LOCK TABLES " NOM_TABLE_ARCHIVESBANQUE " WRITE, " NOM_TABLE_LIGNESCOMPTES " WRITE, " NOM_TABLE_COMPTES " WRITE;";
    QSqlQuery lockquery (lockrequete, proc->getDataBase());
    if (proc->TraiteErreurRequete(lockquery,lockrequete, tr("Impossible de verrouiller ") +  NOM_TABLE_ARCHIVESBANQUE ", " NOM_TABLE_COMPTES + tr(" et ") + NOM_TABLE_LIGNESCOMPTES))
        return;
    QString CalcidArchiverequete = "select max(idArchive) from " NOM_TABLE_ARCHIVESBANQUE;
    QSqlQuery query1(CalcidArchiverequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(query1,CalcidArchiverequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }
    query1.first();
    int max = query1.value(0).toInt();

    QString RestaureArchiverequete = "insert into " NOM_TABLE_LIGNESCOMPTES " select * from  (select idLigne, idCompte, idDep, idRec, LigneDate, LigneLibelle, LigneMontant,"
            "LigneDebitCredit, LigneTypeOperation, 1 as ligneConsolide from " NOM_TABLE_ARCHIVESBANQUE " where idarchive = " + QString::number(max) + ") as tet";
    QSqlQuery query3(RestaureArchiverequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(query3,RestaureArchiverequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }

    // recalculer le solde
    double NouveauSolde = QLocale().toDouble(ui->MontantSoldeBrutlabel->text());
    QString CalcSolderequete = "select LigneMontant, LigneDebitCredit from " NOM_TABLE_LIGNESCOMPTES " where idcompte = " + QString::number(idCompte);
    QSqlQuery query2(CalcSolderequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(query2,CalcSolderequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }

    if (query2.size() == 0)
    {
        UpMessageBox::Watch(this, tr("Il n'y a pas d'acte à désarchiver!"));
        return;
    }
    query2.first();
    for (int i = 0; i < query2.size(); i++)
    {
        if (query2.value(1).toInt() == 1)
            NouveauSolde -= query2.value(0).toDouble();
        else
            NouveauSolde += query2.value(0).toDouble();
        query2.next();
    }

    QString SupprimArchiverequete = " delete from " NOM_TABLE_ARCHIVESBANQUE " where idarchive = " + QString::number(max);
    QSqlQuery query4(SupprimArchiverequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(query4,SupprimArchiverequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }


    QString UpdateSolderequete = "update " NOM_TABLE_COMPTES " set SoldeSurDernierReleve = " + QString::number(NouveauSolde,'f',2)
                                    + " where idCompte = " + QString::number(idCompte);
    QSqlQuery query5(UpdateSolderequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(query5,UpdateSolderequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }

    proc->commit(proc->getDataBase());
    SoldeSurReleve = NouveauSolde;
    ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(SoldeSurReleve,'f',2) + " ");
    gBigTable->clearContents();
    RemplitLaTable(idCompte);
}

void dlg_comptes::Archiver()
{
    QList<int> ListeActesAArchiver;
    for (int i = 0; i < gBigTable->rowCount();i++)
    {
        QWidget* Wdg = dynamic_cast<QWidget*>(gBigTable->cellWidget(i,6));
        if (Wdg)
        {
            QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
            for (int n = 0; n <  allCheck.size(); n++)
            {
                if (allCheck.at(n)->isChecked())
                {
                    QLabel *Lbl = dynamic_cast<QLabel*>(gBigTable->cellWidget(i,0));
                    if (Lbl)
                        ListeActesAArchiver << Lbl->text().toInt();
                }
            }
        }
    }

    if (ListeActesAArchiver.size() == 0)
    {
        UpMessageBox::Watch(this,tr("Il n'y a pas d'écriture à archiver!"));
        return;
    }
    QSqlQuery ("SET AUTOCOMMIT = 0;", proc->getDataBase());
    QString lockrequete = "LOCK TABLES " NOM_TABLE_ARCHIVESBANQUE " WRITE, " NOM_TABLE_LIGNESCOMPTES " WRITE, " NOM_TABLE_COMPTES " WRITE;";
    QSqlQuery lockquery (lockrequete, proc->getDataBase());
    if (proc->TraiteErreurRequete(lockquery,lockrequete, tr("Impossible de verrouiller ") + NOM_TABLE_ARCHIVESBANQUE ", "  NOM_TABLE_COMPTES + tr(" et ") + NOM_TABLE_LIGNESCOMPTES))
        return;

    QString CalcidArchiverequete = "select max(idArchive) from " NOM_TABLE_ARCHIVESBANQUE;
    QSqlQuery query1(CalcidArchiverequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(query1,CalcidArchiverequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }
    query1.first();
    int max = query1.value(0).toInt();

    QString Archiverequete = "insert into " NOM_TABLE_ARCHIVESBANQUE " select * from  (select idLigne, idCompte, idDep, idRec, LigneDate, LigneLibelle, LigneMontant,"
            "LigneDebitCredit, LigneTypeOperation, date(now()) as LigneDateConsolidation, "
            + QString::number(max+1) + " as idArchive from " NOM_TABLE_LIGNESCOMPTES
            " where idLigne in ";

    QString reponse = "(" + QString::number(ListeActesAArchiver.at(0));
    for (int i = 1; i < ListeActesAArchiver.size();i++)
      reponse += "," + QString::number(ListeActesAArchiver.at(i));
    Archiverequete += reponse + ")) as tet";

    QSqlQuery query2 (Archiverequete, proc->getDataBase());
    if (proc->TraiteErreurRequete(query2,Archiverequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }

    QString ArchiveCompterequete = " delete from " NOM_TABLE_LIGNESCOMPTES " where idligne in " + reponse + ")";
    QSqlQuery query3(ArchiveCompterequete, proc->getDataBase());
    if (proc->TraiteErreurRequete(query3,ArchiveCompterequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }

    SoldeSurReleve = QLocale().toDouble(ui->MontantSoldeConsolidelabel->text());
    ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(SoldeSurReleve,'f',2) + " ");

    QString UpdateSolderequete = "update " NOM_TABLE_COMPTES " set SoldeSurDernierReleve = " + QString::number(SoldeSurReleve,'f',2)
                                    + " where idCompte = " + QString::number(idCompte);
    QSqlQuery query4(UpdateSolderequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(query4,UpdateSolderequete,""))
    {
        proc->rollback(proc->getDataBase());
        return;
    }

    proc->commit(proc->getDataBase());
    gBigTable->clearContents();
    RemplitLaTable(idCompte);
}

void dlg_comptes::AnnulConsolidations()
{
    for (int i = 0; i < gBigTable->rowCount(); i++)
    {
        QWidget* Wdg = dynamic_cast<QWidget*>(gBigTable->cellWidget(i,6));
        if (Wdg)
        {
            QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
            for (int n = 0; n <  allCheck.size(); n++)
                allCheck.at(n)->setCheckState(Qt::Unchecked);
        }
    }
    QString MetAJourConsoliderequete = "update " NOM_TABLE_LIGNESCOMPTES " set Ligneconsolide = null";
    QSqlQuery (MetAJourConsoliderequete,proc->getDataBase());
    CalculeTotal();
}

void dlg_comptes::Slot_ContextMenuTableWidget()
{
    UpLabel *lbl = dynamic_cast<UpLabel*>(sender());
    if (!lbl)
            return;
    int row = lbl->getRow();
    gidLigneASupprimer = lbl->getId();
    QString msg = static_cast<UpLabel*>(ui->upTableWidget->cellWidget(row,3))->text()
            + " - du " + static_cast<UpLabel*>(ui->upTableWidget->cellWidget(row,1))->text();
    QString ecriture = tr("Supprimer l'écriture") + " -" + msg + "?";

    QMenu *menuContextuel       = new QMenu(this);
    QAction *pAction_SupprEcriture = menuContextuel->addAction(ecriture) ;
    connect (pAction_SupprEcriture, &QAction::triggered,    [=] {SupprimerEcriture(msg);});

    // ouvrir le menu
    menuContextuel->exec(cursor().pos());
    delete menuContextuel;
}

void dlg_comptes::RenvoieRangee(bool Coche, UpCheckBox* Check)
{
    int R = Check->getRowTable();
    QLabel* lbl = dynamic_cast<QLabel*>(gBigTable->cellWidget(R,0));
    QString idLigne = lbl->text();
    QString MetAJourConsoliderequete = "update " NOM_TABLE_LIGNESCOMPTES " set Ligneconsolide = ";
    if (Coche)
        MetAJourConsoliderequete += "1";
    else
        MetAJourConsoliderequete += "null";
    MetAJourConsoliderequete += " where idligne = " + idLigne;
    QSqlQuery (MetAJourConsoliderequete,proc->getDataBase());
    CalculeTotal();
}

void dlg_comptes::SupprimerEcriture(QString msg)
{
    UpMessageBox *msgbox = new UpMessageBox(this);
    msgbox->setText(tr("Suppression d'une écriture!"));
    msgbox->setInformativeText(tr("Vous avez choisi de supprimer l'écriture") + "\n"
                              + msg + "\n\n" +
                              tr("Cette suppression est définitive mais ne supprimera pas l'opération de recette/dépense correspondante.") + "\n" +
                              tr("Supprimer une écriture du compte bancaire sert en général à équilibrer le compte pour le rendre conforme au relevé") + ".\n" +
                              tr("Confirmez vous la suppression?") + "\n\n");
    msgbox->setIcon(UpMessageBox::Warning);
    UpSmallButton *OKBouton = new UpSmallButton();
    OKBouton->setText(tr("Supprimer"));
    UpSmallButton *NoBouton = new UpSmallButton();
    NoBouton->setText(tr("Annuler"));
    msgbox->addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox->addButton(OKBouton, UpSmallButton::STARTBUTTON);
    msgbox->exec();
    if (msgbox->clickedButton() == OKBouton)
    {
        QString req = "delete from " NOM_TABLE_LIGNESCOMPTES " where idligne = " + QString::number(gidLigneASupprimer);
        QSqlQuery(req,proc->getDataBase());
        RemplitLaTable(idCompte);
    }
    delete OKBouton;
    delete NoBouton;
}

void dlg_comptes::CalculeTotal()
{
    double Total = SoldeSurReleve;
    double TotalConsolide = SoldeSurReleve;
    if (gBigTable->rowCount() > 0)
    {
        for (int k = 0; k < gBigTable->rowCount(); k++)
        {
            QLabel *Lbl = dynamic_cast<QLabel*>(gBigTable->cellWidget(k,4));
            if (Lbl)
            {
                Total += QLocale().toDouble(Lbl->text());
                QWidget *Wdg = dynamic_cast<QWidget*>(gBigTable->cellWidget(k,6));
                if (Wdg)
                {
                    QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
                    for (int n = 0; n <  allCheck.size(); n++)
                        if (allCheck.at(n)->isChecked())
                            TotalConsolide += QLocale().toDouble(Lbl->text());
                }
            }
            QLabel *Lbl2 = dynamic_cast<QLabel*>(gBigTable->cellWidget(k,5));
            if (Lbl2)
            {
                Total -= QLocale().toDouble(Lbl2->text());
                QWidget *Wdg = dynamic_cast<QWidget*>(gBigTable->cellWidget(k,6));
                if (Wdg)
                {
                    QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
                    for (int n = 0; n <  allCheck.size(); n++)
                        if (allCheck.at(n)->isChecked())
                            TotalConsolide -= QLocale().toDouble(Lbl2->text());
                }
            }
        }
    ui->MontantSoldeBrutlabel->setText(QLocale().toString(Total,'f',2) + " ");
    ui->MontantSoldeConsolidelabel->setText(QLocale().toString(TotalConsolide,'f',2) + " ");
    }
}

void dlg_comptes::ChangeCompte(int idx)
{
    idCompte = ui->BanquecomboBox->itemData(idx).toInt();
    QString SoldeComptereq = " select SoldeSurDernierReleve from " NOM_TABLE_COMPTES " where idcompte = " + QString::number(idCompte);
    QSqlQuery SoldeCompteQuery (SoldeComptereq,proc->getDataBase());
    if (SoldeCompteQuery.size() > 0)
    {
        SoldeCompteQuery.first();
        SoldeSurReleve = SoldeCompteQuery.value(0).toDouble();
        ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(SoldeSurReleve,'f',2) + " ");
        RemplitLaTable(idCompte);
    }
    else
    {
        UpMessageBox::Watch(this,tr("Pas d'écriture sur ce compte!"));
        gBigTable->clearContents();
        ui->MontantSoldeBrutlabel->setText("0,00 ");
        ui->MontantSoldeConsolidelabel->setText("0,00 ");
        ui->MontantSoldeSurRelevelabel->setText("0,00 ");
    }
}

void dlg_comptes::closeEvent(QCloseEvent *event)
{
    proc->gsettingsIni->setValue("PositionsFiches/PositionComptes",saveGeometry());
    event->accept();
}

// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------
bool dlg_comptes::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == gBigTable)
        if (event->type() == QEvent::KeyPress)            // l'apppui sur space fait changer d'état le checkbox
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space)
            {
                QList<QTableWidgetSelectionRange>  RangeeSelectionne = gBigTable->selectedRanges();
                if (RangeeSelectionne.size() > 0)
                {
                    int ab = RangeeSelectionne.at(0).topRow();
                    QWidget* Wdg = dynamic_cast<QWidget*>(gBigTable->cellWidget(ab,6));
                    if (Wdg)
                    {
                        QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
                        for (int n = 0; n <  allCheck.size(); n++)
                        {
                            allCheck.at(n)->toggle();
                            CalculeTotal();
                        }
                    }
                }
            }
        }
    return QWidget::eventFilter(obj, event);
}

bool dlg_comptes::getInitOK()
{
    return InitOK;
}

void dlg_comptes::setInitOK(bool init)
{
    InitOK = init;
}

void dlg_comptes::DefinitArchitetureTable()
{
    int                 ColCount;

    gBigTable->setFocusPolicy(Qt::NoFocus);
    ColCount = 9;
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
    LabelARemplir << tr("NoLigne");
    LabelARemplir << tr("Date");
    LabelARemplir << tr("Type opération");
    LabelARemplir << tr("Libellé opération");
    LabelARemplir << tr("Crédit");
    LabelARemplir << tr("Débit");
    LabelARemplir << tr("Consolidé");
    LabelARemplir << "idDep";
    LabelARemplir << "idRec";

    gBigTable->setHorizontalHeaderLabels(LabelARemplir);
    gBigTable->horizontalHeader()->setVisible(true);
    gBigTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    gBigTable->setColumnWidth(li,0);                                                // idLigne
    li++;
    gBigTable->setColumnWidth(li,100);                                              // Date affichage européen
    li++;
    gBigTable->setColumnWidth(li,210);                                              // Type d'opération (virement créditeur, débiteur, retrait, chèque, ...)
    li++;
    gBigTable->setColumnWidth(li,320);                                              // Libellé opération
    li++;
    gBigTable->setColumnWidth(li,85);                                               // Crédit
    li++;
    gBigTable->setColumnWidth(li,85);                                              // Débit
    li++;
    gBigTable->setColumnWidth(li,78);                                               // Consolidé
    li++;
    gBigTable->setColumnWidth(li,10);                                               // idDep
    li++;
    gBigTable->setColumnWidth(li,10);                                               // idRec

    gBigTable->setColumnHidden(7,true);
    gBigTable->setColumnHidden(8,true);

    gBigTable->setGridStyle(Qt::SolidLine);

}

void dlg_comptes::RemplitLaTable(int idCompteAVoir)
{
    QString LignesComptesrequete = "select idLigne, idCompte, idDep, idRec, LigneDate, LigneLibelle, LigneMontant, LigneDebitCredit, LigneTypeOperation, LigneConsolide from " NOM_TABLE_LIGNESCOMPTES
            " where idCompte = " + QString::number(idCompteAVoir) + " order by LigneDate, lignelibelle, ligneMontant";

    UpLabel *      lbl0;
    UpLabel *      lbl1;
    UpLabel *      lbl2;
    UpLabel *      lbl3;
    UpLabel *      lbl4;
    UpLabel *      lbl5;
    UpLabel *      lbl7;
    UpLabel *      lbl8;

    QPointer<QWidget>     wdg;
    QPointer<UpCheckBox>  Checkbx;
    QPointer<QHBoxLayout> l;
    QString     A;

    QSqlQuery LignesComptesQuery (LignesComptesrequete,proc->getDataBase());
    if (proc->TraiteErreurRequete(LignesComptesQuery,LignesComptesrequete, tr("Impossible de construire la table des comptes")))
        reject();

    if (LignesComptesQuery.size() == 0)
    {
        UpMessageBox::Watch(this,tr("Pas d'écriture sur le compte"));
        gBigTable->clearContents();
        ui->MontantSoldeBrutlabel->setText("0,00 ");
        ui->MontantSoldeConsolidelabel->setText("0,00 ");
        ui->MontantSoldeSurRelevelabel->setText("0,00 ");
    }
    gBigTable->clearContents();

    gBigTable->setRowCount(LignesComptesQuery.size());

    LignesComptesQuery.first();
    for (int i = 0; i < LignesComptesQuery.size(); i++)
    {
        lbl0 = new UpLabel;
        lbl1 = new UpLabel;
        lbl2 = new UpLabel;
        lbl3 = new UpLabel;
        lbl4 = new UpLabel;
        lbl5 = new UpLabel;
        lbl7 = new UpLabel;
        lbl8 = new UpLabel;
        lbl0->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl1->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl2->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl3->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl4->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl5->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl7->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl8->setContextMenuPolicy(Qt::CustomContextMenu);
        lbl0->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl1->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl2->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl3->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl4->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl5->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl7->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl8->setId(LignesComptesQuery.value(0).toInt());                      // idLigne
        lbl0->setRow(i);
        lbl1->setRow(i);
        lbl2->setRow(i);
        lbl3->setRow(i);
        lbl4->setRow(i);
        lbl5->setRow(i);
        lbl7->setRow(i);
        lbl8->setRow(i);

        connect (lbl0,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        connect (lbl1,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        connect (lbl2,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        connect (lbl3,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        connect (lbl4,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        connect (lbl5,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        connect (lbl7,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        connect (lbl8,        SIGNAL(customContextMenuRequested(QPoint)),         this,       SLOT (Slot_ContextMenuTableWidget()));
        /*
        connect (lbl0,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        connect (lbl1,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        connect (lbl2,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        connect (lbl3,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        connect (lbl4,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        connect (lbl5,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        connect (lbl7,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        connect (lbl8,        &QWidget::customContextMenuRequested, [=] {Slot_ContextMenuTableWidget(cursor().pos());});
        NE MARCHE PAS ET JE NE SAIS PAS POURQUOI........
        */
        int col = 0;

        A = LignesComptesQuery.value(0).toString();                                                             // idLigne - col = 0
        lbl0->setText(A + " ");
        lbl0->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl0->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(i,col,lbl0);
        col++;

        A = LignesComptesQuery.value(4).toDate().toString(tr("d MMM yyyy"));                                        // Date - col = 1
        lbl1->setText(A + " ");
        lbl1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl1->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(i,col,lbl1);
        col++;

        lbl2->setText(" " + LignesComptesQuery.value(8).toString());                                                // Type opération - col = 2
        lbl2->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(i,col,lbl2);
        col++;

        lbl3->setText(" " + LignesComptesQuery.value(5).toString());                                                // Libellé opération - col = 3;
        lbl3->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(i,col,lbl3);
        col++;

        if (LignesComptesQuery.value(7).toInt() > 0)
        {
            A = QLocale().toString(LignesComptesQuery.value(6).toDouble(),'f',2);                                 // Crédit - col = 4
            lbl4->setText(A + " ");
            lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl4->setFocusPolicy(Qt::NoFocus);
            gBigTable->setCellWidget(i,col,lbl4);
            col++;
            lbl5->setText("");
            lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl5->setFocusPolicy(Qt::NoFocus);
            gBigTable->setCellWidget(i,col,lbl5);
        }
        if (LignesComptesQuery.value(7).toInt() < 1)
        {
            lbl4->setText("");
            lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl4->setFocusPolicy(Qt::NoFocus);
            gBigTable->setCellWidget(i,col,lbl4);
            col++;
            A = QLocale().toString(LignesComptesQuery.value(6).toDouble(),'f',2);                                 // Dédit - col = 5
            lbl5->setText(A + " ");
            lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl5->setFocusPolicy(Qt::NoFocus);
            gBigTable->setCellWidget(i,col,lbl5);
        }
        col++;

        int b = LignesComptesQuery.value(9).toInt();                                                            // Consolidé - col = 6
        wdg = new QWidget(this);
        Checkbx = new UpCheckBox(wdg);
        if (b == 1)
            Checkbx->setCheckState(Qt::Checked);
        else
            Checkbx->setCheckState(Qt::Unchecked);
        Checkbx->setRowTable(i);
        Checkbx->setFocusPolicy(Qt::NoFocus);

        connect(Checkbx,      &QCheckBox::clicked,  [=] {RenvoieRangee(Checkbx->isChecked(), Checkbx);});
        l = new QHBoxLayout(wdg);
        l->setContentsMargins(0,0,0,0);
        l->setAlignment( Qt::AlignCenter );
        l->addWidget(Checkbx);
        wdg->setLayout(l);
        gBigTable->setCellWidget(i,col,wdg);
        col++;

        lbl7->setText(LignesComptesQuery.value(2).toString());
        lbl7->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(i,col,lbl7);
        col++;

        lbl8->setText(LignesComptesQuery.value(3).toString());
        lbl8->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(i,col,lbl8);

        gBigTable->setRowHeight(i,QFontMetrics(qApp->font()).height()*1.3);

        LignesComptesQuery.next();
    }
    CalculeTotal();
 }
