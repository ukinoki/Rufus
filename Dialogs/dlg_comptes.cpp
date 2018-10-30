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
    db = DataBase::getInstance();
    intervalledate = 180;
    dateencours = QDate::currentDate();
    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionComptes").toByteArray());
    setAttribute(Qt::WA_DeleteOnClose);

    // On reconstruit le combobox des comptes de l'utilisateur
    comptesusr = new Comptes();
    comptesusr->addCompte( db->loadComptesByUser(proc->getUserConnected()->id()) );
    proc->getUserConnected()->setComptes(comptesusr);


    if (comptesusr->comptesAll().size() == 0)
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas de compte bancaire enregistré!"));
        InitOK = false;
        return;
    }
    else
    {
        CompteEnCours = new (Compte);
        ui->BanquecomboBox->clear();
        int idcptprefer = -1;
        QMultiMap<int, Compte*>::const_iterator itcpt;
        for (itcpt = comptesusr->comptes().constBegin(); itcpt != comptesusr->comptes().constEnd(); ++itcpt)
        {
            Compte *cpt = const_cast<Compte*>(itcpt.value());
            ui->BanquecomboBox->addItem(cpt->nom(),cpt->id());
            if (cpt->isPrefere())
                idcptprefer = cpt->id();
        }
        ui->BanquecomboBox->setCurrentIndex(ui->BanquecomboBox->findData(idcptprefer));
        idCompte = ui->BanquecomboBox->currentData().toInt();
        CompteEnCours = comptesusr->getCompteById(idCompte);
        if (CompteEnCours != Q_NULLPTR)
        {
            SoldeSurReleve = CompteEnCours->solde();
            ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(SoldeSurReleve,'f',2) + " ");
            gBigTable = ui->upTableWidget;
            gBigTable->installEventFilter(this);

            DefinitArchitetureTable();
            RemplitLaTable(idCompte);
            connect(ui->AnnulArchivepushButton,             &QPushButton::clicked,                              this,   [=] {AnnulArchive();});
            connect(ui->ArchiverpushButton,                 &QPushButton::clicked,                              this,   [=] {Archiver();});
            connect(ui->AnnulerConsolidationspushButton,    &QPushButton::clicked,                              this,   [=] {AnnulConsolidations();});
            connect(ui->OKpushButton,                       &QPushButton::clicked,                              this,   [=] {accept();});
            connect(ui->BanquecomboBox,                     QOverload<int>::of(&QComboBox::currentIndexChanged),this,   [=](int){ChangeCompte(ui->BanquecomboBox->currentIndex());});
            connect(ui->VoirArchivespushButton,             &QPushButton::clicked,                              this,   &dlg_comptes::VoirArchives);

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
    if (!db->locktables(QStringList() <<  NOM_TABLE_ARCHIVESBANQUE << NOM_TABLE_LIGNESCOMPTES << NOM_TABLE_COMPTES))
        return;

    int max = db->selectMaxFromTable("idArchive", NOM_TABLE_ARCHIVESBANQUE);
    if (max==-1)
    {
        db->rollback();
        return;
    }

    if (!db->StandardSQL("insert into " NOM_TABLE_LIGNESCOMPTES
                               " select * from"
                               "  (select idLigne, idCompte, idDep, idRec, idrecspec, idremcheq, LigneDate, LigneLibelle, LigneMontant,"
                               "LigneDebitCredit, LigneTypeOperation, 1 as ligneConsolide from " NOM_TABLE_ARCHIVESBANQUE
                               " where idarchive = " + QString::number(max) + ")"
                               " as tet",
                               tr("Impossible d'ouvrir la table des archives bancaires")))
    {
        db->rollback();
        return;
    }

    // recalculer le solde
    double NouveauSolde = QLocale().toDouble(ui->MontantSoldeBrutlabel->text());
    bool ok = true;
    QList<QList<QVariant>> listsoldes = db->SelectRecordsFromTable(QStringList() << "LigneMontant" << "LigneDebitCredit",
                                                              NOM_TABLE_LIGNESCOMPTES, ok,
                                                              " where idcompte = " + QString::number(idCompte));
    if (listsoldes.size() == 0)
    {
        UpMessageBox::Watch(this, tr("Il n'y a pas d'acte à désarchiver!"));
        return;
    }
    for (int i = 0; i < listsoldes.size(); i++)
    {
        if (listsoldes.at(i).at(1).toInt() == 1)
            NouveauSolde -= listsoldes.at(i).at(0).toDouble();
        else
            NouveauSolde += listsoldes.at(i).at(0).toDouble();
    }

    if (!db->SupprRecordFromTable(max, "idarchive", NOM_TABLE_ARCHIVESBANQUE))
    {
        db->rollback();
        return;
    }


    if (!db->StandardSQL("update " NOM_TABLE_COMPTES
                               " set SoldeSurDernierReleve = "
                               + QString::number(NouveauSolde,'f',2)
                               + " where idCompte = " + QString::number(idCompte)))
    {
        db->rollback();
        return;
    }

    db->commit();
    SoldeSurReleve = NouveauSolde;
    CompteEnCours->setSolde(SoldeSurReleve);
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

    QStringList listlock;
    listlock << NOM_TABLE_ARCHIVESBANQUE << NOM_TABLE_LIGNESCOMPTES << NOM_TABLE_COMPTES;
    if (!db->locktables(listlock))
        return;
    int max = db->selectMaxFromTable("idArchive", NOM_TABLE_ARCHIVESBANQUE);
    if (max==-1)
    {
        db->rollback();
        return;
    }

    QString Archiverequete = "insert into " NOM_TABLE_ARCHIVESBANQUE " select * from  (select idLigne, idCompte, idDep, idRec, idrecspec, idremcheq, LigneDate, LigneLibelle, LigneMontant,"
            "LigneDebitCredit, LigneTypeOperation, date(now()) as LigneDateConsolidation, "
            + QString::number(max+1) + " as idArchive from " NOM_TABLE_LIGNESCOMPTES
            " where idLigne in ";
    QString reponse = "(" + QString::number(ListeActesAArchiver.at(0));
    for (int i = 1; i < ListeActesAArchiver.size();i++)
      reponse += "," + QString::number(ListeActesAArchiver.at(i));
    Archiverequete += reponse + ")) as tet";

    if (!db->StandardSQL(Archiverequete))
    {
        db->rollback();
        return;
    }
    if (!db->StandardSQL(" delete from " NOM_TABLE_LIGNESCOMPTES " where idligne in " + reponse + ")"))
    {
        db->rollback();
        return;
    }
    if (!db->StandardSQL("update " NOM_TABLE_COMPTES " set SoldeSurDernierReleve = "
                               + QString::number(QLocale().toDouble(ui->MontantSoldeConsolidelabel->text()),'f',2)
                               + " where idCompte = " + QString::number(idCompte)))
    {
        db->rollback();
        return;
    }

    db->commit();
    SoldeSurReleve = QLocale().toDouble(ui->MontantSoldeConsolidelabel->text());
    CompteEnCours->setSolde(SoldeSurReleve);
    ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(SoldeSurReleve,'f',2) + " ");
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
    db->StandardSQL("update " NOM_TABLE_LIGNESCOMPTES " set Ligneconsolide = null");
    CalculeTotal();
}

void dlg_comptes::ContextMenuTableWidget(UpLabel *lbl)
{
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
    QString requete = "update " NOM_TABLE_LIGNESCOMPTES " set Ligneconsolide = ";
    requete += (Coche? "1" : "null");
    requete += " where idligne = " + lbl->text();
    db->StandardSQL(requete);
    CalculeTotal();
}

void dlg_comptes::RedessineFicheArchives()
{
    gTableArchives->clear();
    int             ColCount = 6;
    if (gModeArchives == TOUT)
        ColCount ++;
    gTableArchives     ->setColumnCount(ColCount);

    QStringList LabelARemplir;
    LabelARemplir << tr("NoLigne");
    LabelARemplir << tr("Date");
    LabelARemplir << tr("Type opération");
    LabelARemplir << tr("Libellé opération");
    LabelARemplir << tr("Crédit");
    LabelARemplir << tr("Débit");
    if (gModeArchives == TOUT)
        LabelARemplir << tr("consolidé le");

    gTableArchives->setHorizontalHeaderLabels(LabelARemplir);
    gTableArchives->horizontalHeader()->setVisible(true);
    gTableArchives->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                   // Réglage de la largeur et du nombre des colonnes
    gTableArchives->setColumnWidth(li,0);                                                // idLigne
    li++;
    gTableArchives->setColumnWidth(li,100);                                              // Date affichage européen
    li++;
    gTableArchives->setColumnWidth(li,210);                                              // Type d'opération (virement créditeur, débiteur, retrait, chèque, ...)
    li++;
    gTableArchives->setColumnWidth(li,320);                                              // Libellé opération
    li++;
    gTableArchives->setColumnWidth(li,85);                                               // Crédit
    li++;
    gTableArchives->setColumnWidth(li,85);                                               // Débit
    if (gModeArchives == TOUT)
    {
        li++;
        gTableArchives->setColumnWidth(li,95);                                           // Date consolidation
    }

    int larg = gTableArchives->FixLargeurTotale();
    if (gModeArchives == PARARCHIVE)
    {
        gloupButton  ->setUpButtonStyle(UpSmallButton::LOUPEBUTTON);
        gloupButton  ->setImmediateToolTip(tr("Voir tout"));
    }
    else
    {
        gloupButton  ->setUpButtonStyle(UpSmallButton::CALENDARBUTTON);
        gloupButton  ->setImmediateToolTip(tr("Revoir par consolidation"));
    }

    glistArchCombo->setVisible(gModeArchives == PARARCHIVE);
    gFlecheHtButton->setVisible(gModeArchives == TOUT);
    gArchives->setFixedWidth(larg+20);

}

void dlg_comptes::RemplirTableArchives()
{
    QList<Archive*> listarchives;
    for( QMap<int, Archive*>::const_iterator itarc = archivescptencours->archives().constBegin(); itarc != archivescptencours->archives().constEnd(); ++itarc )
    {
        Archive *arc = const_cast<Archive*>(itarc.value());
        if (gModeArchives == PARARCHIVE)
        {
            if (arc->idarchive() == glistArchCombo->currentData().toInt())
                listarchives << arc;
        }
        else listarchives << arc;
    }
    gTableArchives->setRowCount(listarchives.size());
    // remplissage de la table
    for (int row=0; row<listarchives.size(); row++)
    {
        Archive* archive = listarchives.at(row);
        UpLabel *      lbl0 = new UpLabel();
        UpLabel *      lbl1 = new UpLabel();
        UpLabel *      lbl2 = new UpLabel();
        UpLabel *      lbl3 = new UpLabel();
        UpLabel *      lbl4 = new UpLabel();
        UpLabel *      lbl5 = new UpLabel();

        QString     A;

        lbl0->setContextMenuPolicy(Qt::NoContextMenu);
        lbl1->setContextMenuPolicy(Qt::NoContextMenu);
        lbl2->setContextMenuPolicy(Qt::NoContextMenu);
        lbl3->setContextMenuPolicy(Qt::NoContextMenu);
        lbl4->setContextMenuPolicy(Qt::NoContextMenu);
        lbl5->setContextMenuPolicy(Qt::NoContextMenu);

        int col = 0;

        A = QString::number(archive->id());                                                             // idLigne - col = 0
        lbl0->setText(A + " ");
        lbl0->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl0->setFocusPolicy(Qt::NoFocus);
        gTableArchives->setCellWidget(row,col,lbl0);
        col++;

        A = archive->lignedate().toString(tr("d MMM yyyy"));                                            // Date - col = 1
        lbl1->setText(A + " ");
        lbl1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl1->setFocusPolicy(Qt::NoFocus);
        gTableArchives->setCellWidget(row,col,lbl1);
        col++;

        lbl2->setText(" " + archive->lignetypeoperation());                                             // Type opération - col = 2
        lbl2->setFocusPolicy(Qt::NoFocus);
        gTableArchives->setCellWidget(row,col,lbl2);
        col++;

        lbl3->setText(" " + archive->lignelibelle());                                                   // Libellé opération - col = 3;
        lbl3->setFocusPolicy(Qt::NoFocus);
        gTableArchives->setCellWidget(row,col,lbl3);
        col++;

        A = QLocale().toString(archive->montant(),'f',2);                                               // Crédit - col = 4
        if (archive->montant() > 0)
        {
            lbl4->setText(A + " ");
            lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl4->setFocusPolicy(Qt::NoFocus);
            gTableArchives->setCellWidget(row,col,lbl4);
            col++;
            lbl5->setText("");
            lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl5->setFocusPolicy(Qt::NoFocus);
            gTableArchives->setCellWidget(row,col,lbl5);
        }
        else                                                                                            // Débit - col = 5
        {
            lbl4->setText("");
            lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl4->setFocusPolicy(Qt::NoFocus);
            gTableArchives->setCellWidget(row,col,lbl4);
            col++;
            lbl5->setText(A + " ");
            lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl5->setFocusPolicy(Qt::NoFocus);
            gTableArchives->setCellWidget(row,col,lbl5);
        }
        if (gModeArchives == TOUT)
        {
            col++;
            UpLabel *  lbl6 = new UpLabel();
            lbl6->setContextMenuPolicy(Qt::NoContextMenu);
            lbl6->setText(" " + archive->lignedateconsolidation().toString("d MMM yyyy"));                   // Date consolidation col = 6;
            lbl6->setFocusPolicy(Qt::NoFocus);
            gTableArchives->setCellWidget(row,col,lbl6);
        }
        gTableArchives->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));
    }
}
void dlg_comptes::VoirArchives()
{
    gArchives       = new UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionArchives", this);
    gTableArchives  = new UpTableWidget();
    glistArchCombo  = new QComboBox();
    glbltitre       = new UpLabel();
    QVBoxLayout     *globallay      = dynamic_cast<QVBoxLayout*>(gArchives->layout());
    QHBoxLayout     *titreLay       = new QHBoxLayout();
    gloupButton     = new UpSmallButton();
    gFlecheHtButton = new UpSmallButton();

    gTableArchives      ->setFocusPolicy(Qt::NoFocus);
    gTableArchives      ->setPalette(QPalette(Qt::white));
    gTableArchives      ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    gTableArchives      ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    gTableArchives      ->verticalHeader()->setVisible(false);
    gTableArchives      ->setSelectionMode(QAbstractItemView::SingleSelection);
    gTableArchives      ->setGridStyle(Qt::SolidLine);
    gTableArchives      ->verticalHeader()->setVisible(false);
    gTableArchives      ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    gTableArchives      ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    glbltitre           ->setText(tr("Liste des remises archivées sur le compte ") + CompteEnCours->nom() + " ");
    gFlecheHtButton     ->setIcon( Icons::icFlecheHaut() );
    gFlecheHtButton     ->setCursor(Qt::PointingHandCursor);
    gFlecheHtButton     ->setImmediateToolTip(tr("Voir les archives précédentes"));
    gFlecheHtButton     ->setIconSize(QSize(30,30));

    titreLay    ->addWidget(glbltitre);
    titreLay    ->addWidget(glistArchCombo);
    titreLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    titreLay    ->addWidget(gFlecheHtButton);
    titreLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    titreLay    ->addWidget(gloupButton);
    titreLay    ->setSpacing(5);
    titreLay    ->setContentsMargins(0,0,0,0);

    /*Il y a 2 modes d'affichage
     * PARARCHIVE : on choisit dans un combobox l'archive qu'on veut afficher et on affiche les écritures archive par archive - c'est le mode de départ
     * TOUT : on affiche toutes les écritures datant de moins de 6 mois
     * L'intervalle de 6 mois se règle avec la variable intervalledate (dans ce cas 180)
     * On remonde dans le temps par pas de 6 mois en mode TOUT en cliquant sur la flèche haut mais c'est lent
     * */
    gModeArchives = PARARCHIVE;
    RedessineFicheArchives();

    globallay   ->insertWidget(0,gTableArchives);
    globallay   ->insertLayout(0, titreLay);

    gArchives->AjouteLayButtons(UpDialog::ButtonOK);
    connect(gArchives->OKButton,     &QPushButton::clicked,              gArchives, [=] {gArchives->close();});
    gArchives->setModal(true);
    globallay->setStretch(0,1);
    globallay->setStretch(1,15);

    QList<Archive*> listarchives = db->loadArchiveByDate(dateencours, CompteEnCours, intervalledate);
    dateencours = dateencours.addDays(-intervalledate);
    archivescptencours = new Archives();
    archivescptencours->addArchive(listarchives);
    for (QMap<int, Archive*>::const_iterator itarc = archivescptencours->archives().constBegin(); itarc != archivescptencours->archives().constEnd(); ++itarc)
    {
        Archive* arc = const_cast<Archive*>(itarc.value());
        if (glistArchCombo->findData(arc->idarchive()) == -1)
            glistArchCombo->addItem(tr("Consolidation") + " " + QString::number(arc->idarchive()) + " "
                                  + tr("du") + " " + arc->lignedateconsolidation().toString("d MMM yyyy"), arc->idarchive());
    }
    connect(gloupButton,             &QPushButton::clicked,       this,  [=]
    {
        if (gModeArchives == PARARCHIVE)    gModeArchives = TOUT;
        else                                gModeArchives = PARARCHIVE;
        RedessineFicheArchives();
        RemplirTableArchives();
    });
    connect(glistArchCombo,          QOverload<int>::of(&QComboBox::currentIndexChanged) ,this,  &dlg_comptes::RemplirTableArchives);
    connect(gFlecheHtButton,         &QPushButton::clicked ,this,   [=]
    {
        QList<Archive*> listarchives = db->loadArchiveByDate(dateencours, CompteEnCours, intervalledate);
        dateencours = dateencours.addDays(-intervalledate);
        archivescptencours->addArchive(listarchives);
        RemplirTableArchives();
    });
    glistArchCombo->setMaxVisibleItems(20);
    glistArchCombo->setFocusPolicy(Qt::StrongFocus);
    glistArchCombo->setCurrentIndex(glistArchCombo->count()-1);
    gArchives->exec();
    dateencours = QDate::currentDate();
    delete archivescptencours;
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
    UpSmallButton *OKBouton = new UpSmallButton(tr("Supprimer"));
    UpSmallButton *NoBouton = new UpSmallButton(tr("Annuler"));
    msgbox->addButton(NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox->addButton(OKBouton, UpSmallButton::STARTBUTTON);
    msgbox->exec();
    if (msgbox->clickedButton() == OKBouton)
    {
        db->StandardSQL("delete from " NOM_TABLE_LIGNESCOMPTES " where idligne = " + QString::number(gidLigneASupprimer));
        RemplitLaTable(idCompte);
    }
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
                Total += QLocale().toDouble(Lbl2->text());
                QWidget *Wdg = dynamic_cast<QWidget*>(gBigTable->cellWidget(k,6));
                if (Wdg)
                {
                    QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
                    for (int n = 0; n <  allCheck.size(); n++)
                        if (allCheck.at(n)->isChecked())
                            TotalConsolide += QLocale().toDouble(Lbl2->text());
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
    CompteEnCours = comptesusr->getCompteById(idCompte);
    // on doit refaire la requête parce que le sole s'il est null est passé en 0 par loadcomptesbyUser()
    bool ok = true;
    QList<QList<QVariant>> listsoldes = db->SelectRecordsFromTable(QStringList() << "SoldeSurDernierReleve",
                                                                   NOM_TABLE_COMPTES, ok,
                                                                   "where idcompte = " + QString::number(idCompte));
    if (listsoldes.size() > 0)
    {
        SoldeSurReleve = listsoldes.at(0).at(0).toDouble();
        CompteEnCours->setSolde(SoldeSurReleve);  // à tout hasard
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
    bool ok = true;
    QList<QList<QVariant>> listfamfiscale = db->SelectRecordsFromTable(QStringList() << "idLigne" << "idCompte" << "idDep" << "idRec" << "LigneDate" << "LigneLibelle"
                                                                       << "LigneMontant" << "LigneDebitCredit" << "LigneTypeOperation" << "LigneConsolide",
                                                                       NOM_TABLE_LIGNESCOMPTES, ok,
                                                                       "where idCompte = " + QString::number(idCompteAVoir),
                                                                       "order by LigneDate, lignelibelle, ligneMontant");

    if (listfamfiscale.size()==0)
    {
        UpMessageBox::Watch(this,tr("Pas d'écriture sur le compte"));
        gBigTable->clearContents();
        ui->MontantSoldeBrutlabel->setText("0,00 ");
        ui->MontantSoldeConsolidelabel->setText("0,00 ");
        ui->MontantSoldeSurRelevelabel->setText("0,00 ");
    }
    gBigTable->clearContents();

    gBigTable->setRowCount(listfamfiscale.size());


    for (int i = 0; i < listfamfiscale.size(); i++)
    {
        InsertLigneSurLaTable(listfamfiscale.at(i),i);
    }
    CalculeTotal();
 }

void dlg_comptes::InsertLigneSurLaTable(QList<QVariant> ligne, int row)
{
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
    lbl0->setId(ligne.at(0).toInt());                      // idLigne
    lbl1->setId(ligne.at(0).toInt());                      // idLigne
    lbl2->setId(ligne.at(0).toInt());                      // idLigne
    lbl3->setId(ligne.at(0).toInt());                      // idLigne
    lbl4->setId(ligne.at(0).toInt());                      // idLigne
    lbl5->setId(ligne.at(0).toInt());                      // idLigne
    lbl7->setId(ligne.at(0).toInt());                      // idLigne
    lbl8->setId(ligne.at(0).toInt());                      // idLigne
    lbl0->setRow(row);
    lbl1->setRow(row);
    lbl2->setRow(row);
    lbl3->setRow(row);
    lbl4->setRow(row);
    lbl5->setRow(row);
    lbl7->setRow(row);
    lbl8->setRow(row);

    connect (lbl0,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl0);});
    connect (lbl1,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl1);});
    connect (lbl2,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl2);});
    connect (lbl3,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl3);});
    connect (lbl4,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl4);});
    connect (lbl5,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl5);});
    connect (lbl7,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl7);});
    connect (lbl8,        &QWidget::customContextMenuRequested, [=] {ContextMenuTableWidget(lbl8);});

    int col = 0;

    A = ligne.at(0).toString();                                                             // idLigne - col = 0
    lbl0->setText(A + " ");
    lbl0->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    lbl0->setFocusPolicy(Qt::NoFocus);
    gBigTable->setCellWidget(row,col,lbl0);
    col++;

    A = ligne.at(4).toDate().toString(tr("d MMM yyyy"));                                        // Date - col = 1
    lbl1->setText(A + " ");
    lbl1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    lbl1->setFocusPolicy(Qt::NoFocus);
    gBigTable->setCellWidget(row,col,lbl1);
    col++;

    lbl2->setText(" " + ligne.at(8).toString());                                                // Type opération - col = 2
    lbl2->setFocusPolicy(Qt::NoFocus);
    gBigTable->setCellWidget(row,col,lbl2);
    col++;

    lbl3->setText(" " + ligne.at(5).toString());                                                // Libellé opération - col = 3;
    lbl3->setFocusPolicy(Qt::NoFocus);
    gBigTable->setCellWidget(row,col,lbl3);
    col++;

    if (ligne.at(7).toInt() > 0)                                                                // Crédit - col = 4
    {
        A = QLocale().toString(ligne.at(6).toDouble(),'f',2);
        lbl4->setText(A + " ");
        lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl4->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(row,col,lbl4);
        col++;
        lbl5->setText("");
        lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl5->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(row,col,lbl5);
    }
    if (ligne.at(7).toInt() < 1)                                                                // Débit - col = 5
    {
        A = QLocale().toString(ligne.at(6).toDouble()*-1,'f',2);
        lbl4->setText("");
        lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl4->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(row,col,lbl4);
        col++;
        lbl5->setText(A + " ");
        lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl5->setFocusPolicy(Qt::NoFocus);
        gBigTable->setCellWidget(row,col,lbl5);
    }
    col++;

    int b = ligne.at(9).toInt();                                                                // Consolidé - col = 6
    wdg = new QWidget(this);
    Checkbx = new UpCheckBox(wdg);
    if (b == 1)
        Checkbx->setCheckState(Qt::Checked);
    else
        Checkbx->setCheckState(Qt::Unchecked);
    Checkbx->setRowTable(row);
    Checkbx->setFocusPolicy(Qt::NoFocus);

    connect(Checkbx,      &QCheckBox::clicked,  [=] {RenvoieRangee(Checkbx->isChecked(), Checkbx);});
    l = new QHBoxLayout(wdg);
    l->setContentsMargins(0,0,0,0);
    l->setAlignment( Qt::AlignCenter );
    l->addWidget(Checkbx);
    wdg->setLayout(l);
    gBigTable->setCellWidget(row,col,wdg);
    col++;

    lbl7->setText(ligne.at(2).toString());
    lbl7->setFocusPolicy(Qt::NoFocus);
    gBigTable->setCellWidget(row,col,lbl7);
    col++;

    lbl8->setText(ligne.at(3).toString());
    lbl8->setFocusPolicy(Qt::NoFocus);
    gBigTable->setCellWidget(row,col,lbl8);

    gBigTable->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));
}
