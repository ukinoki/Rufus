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

#include "dlg_comptes.h"
#include "ui_dlg_comptes.h"

dlg_comptes::dlg_comptes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_comptes)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    restoreGeometry(proc->settings()->value("PositionsFiches/PositionComptes").toByteArray());
    setAttribute(Qt::WA_DeleteOnClose);

    // On reconstruit le combobox des comptes de l'utilisateur
    if (Datas::I()->users->userconnected()->listecomptesbancaires().isEmpty())
        proc->MAJComptesBancaires(Datas::I()->users->userconnected());
    m_comptesusr = Datas::I()->users->userconnected()->listecomptesbancaires(true);

    if (m_comptesusr.isEmpty())
    {
        UpMessageBox::Watch(this,tr("Vous n'avez pas de compte bancaire enregistré!"));
        m_initok = false;
        return;
    }
    else
    {
        m_compteencours = Q_NULLPTR;
        ui->BanquecomboBox->clear();
        int idcptprefer = -1;
        QListIterator<int> itcpt(m_comptesusr);
        while (itcpt.hasNext())
        {
            Compte *cpt = Datas::I()->comptes->getById(itcpt.next());
            if (cpt != Q_NULLPTR)
                ui->BanquecomboBox->addItem(cpt->nomabrege(),cpt->id());
        }
        if (Datas::I()->users->userconnected()->idcomptepardefaut() > 0)
            idcptprefer = Datas::I()->users->userconnected()->idcomptepardefaut();
        ui->BanquecomboBox->setCurrentIndex(ui->BanquecomboBox->findData(idcptprefer));
        m_idcompte = ui->BanquecomboBox->currentData().toInt();
        m_compteencours = Datas::I()->comptes->getById(m_idcompte);
        if (m_compteencours != Q_NULLPTR)
        {
            m_soldesurreleve = m_compteencours->solde();
            ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(m_soldesurreleve,'f',2) + " ");
            wdg_bigtable = ui->upTableWidget;
            wdg_bigtable->installEventFilter(this);

            DefinitArchitetureTable();
            RemplitLaTable(m_idcompte);
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
            m_initok = true;
        }
        else
        {
            UpMessageBox::Watch(this,tr("Vous n'avez pas de compte bancaire enregistré!"));
            m_initok = false;
        }
    }
}

dlg_comptes::~dlg_comptes()
{
    delete ui;
}

void dlg_comptes::AnnulArchive()
{
    bool ok;
    if (!db->createtransaction(QStringList() <<  TBL_ARCHIVESBANQUE << TBL_LIGNESCOMPTES << TBL_COMPTES))
        return;

    int max = db->selectMaxFromTable("idArchive", TBL_ARCHIVESBANQUE, ok);
    if (!ok || max == -1)
    {
        db->rollback();
        return;
    }

    if (!db->StandardSQL("insert into " TBL_LIGNESCOMPTES
                               " select * from"
                               "  (select idLigne, idCompte, idDep, idRec, idrecspec, idremcheq, LigneDate, LigneLibelle, LigneMontant,"
                               "LigneDebitCredit, LigneTypeOperation, 1 as ligneConsolide from " TBL_ARCHIVESBANQUE
                               " where idarchive = " + QString::number(max) + ")"
                               " as tet",
                               tr("Impossible d'ouvrir la table des archives bancaires")))
    {
        db->rollback();
        return;
    }

    // recalculer le solde
    double NouveauSolde = QLocale().toDouble(ui->MontantSoldeBrutlabel->text());
    QList<QVariantList> listsoldes = db->SelectRecordsFromTable(QStringList() << "LigneMontant" << "LigneDebitCredit",
                                                              TBL_LIGNESCOMPTES, ok,
                                                              " where idcompte = " + QString::number(m_idcompte));
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

    if (!db->SupprRecordFromTable(max, "idarchive", TBL_ARCHIVESBANQUE))
    {
        db->rollback();
        return;
    }


    if (!db->StandardSQL("update " TBL_COMPTES
                               " set SoldeSurDernierReleve = "
                               + QString::number(NouveauSolde,'f',2)
                               + " where idCompte = " + QString::number(m_idcompte)))
    {
        db->rollback();
        return;
    }

    db->commit();
    m_soldesurreleve = NouveauSolde;
    m_compteencours->setsolde(m_soldesurreleve);
    ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(m_soldesurreleve,'f',2) + " ");
    wdg_bigtable->clearContents();
    RemplitLaTable(m_idcompte);
}

void dlg_comptes::Archiver()
{
    QList<int> ListeActesAArchiver;
    for (int i = 0; i < wdg_bigtable->rowCount();i++)
    {
        QWidget* Wdg = dynamic_cast<QWidget*>(wdg_bigtable->cellWidget(i,6));
        if (Wdg)
        {
            QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
            for (int n = 0; n <  allCheck.size(); n++)
            {
                if (allCheck.at(n)->isChecked())
                {
                    QLabel *Lbl = dynamic_cast<QLabel*>(wdg_bigtable->cellWidget(i,0));
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
    bool ok;
    listlock << TBL_ARCHIVESBANQUE << TBL_LIGNESCOMPTES << TBL_COMPTES;
    if (!db->createtransaction(listlock))
        return;
    int max = db->selectMaxFromTable("idArchive", TBL_ARCHIVESBANQUE, ok);
    if ( !ok )
    {
        db->rollback();
        return;
    }

    QString Archiverequete = "insert into " TBL_ARCHIVESBANQUE " select * from  (select idLigne, idCompte, idDep, idRec, idrecspec, idremcheq, LigneDate, LigneLibelle, LigneMontant,"
            "LigneDebitCredit, LigneTypeOperation, date(now()) as LigneDateConsolidation, "
            + QString::number(max+1) + " as idArchive from " TBL_LIGNESCOMPTES
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
    if (!db->StandardSQL(" delete from " TBL_LIGNESCOMPTES " where " CP_IDLIGNE_LIGNCOMPTES " in " + reponse + ")"))
    {
        db->rollback();
        return;
    }
    if (!db->StandardSQL("update " TBL_COMPTES " set SoldeSurDernierReleve = "
                               + QString::number(QLocale().toDouble(ui->MontantSoldeConsolidelabel->text()),'f',2)
                               + " where idCompte = " + QString::number(m_idcompte)))
    {
        db->rollback();
        return;
    }

    db->commit();
    m_soldesurreleve = QLocale().toDouble(ui->MontantSoldeConsolidelabel->text());
    m_compteencours->setsolde(m_soldesurreleve);
    ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(m_soldesurreleve,'f',2) + " ");
    wdg_bigtable->clearContents();
    RemplitLaTable(m_idcompte);
}

void dlg_comptes::AnnulConsolidations()
{
    for (int i = 0; i < wdg_bigtable->rowCount(); i++)
    {
        QWidget* Wdg = dynamic_cast<QWidget*>(wdg_bigtable->cellWidget(i,6));
        if (Wdg)
        {
            QList<UpCheckBox *> allCheck = Wdg->findChildren<UpCheckBox *>();
            for (int n = 0; n <  allCheck.size(); n++)
                allCheck.at(n)->setCheckState(Qt::Unchecked);
        }
        LigneCompte *lign = getLigneCompteFromRow(i);
        if  (lign != Q_NULLPTR)
            lign->setconsolide(false);
    }
    db->StandardSQL("update " TBL_LIGNESCOMPTES " set " CP_CONSOLIDE_LIGNCOMPTES " = null where " CP_IDCOMPTE_LIGNCOMPTES " = " +  QString::number(m_idcompte));
    CalculeTotal();
}

void dlg_comptes::ContextMenuTableWidget(UpLabel *lbl)
{
    int row = lbl->Row();
    LigneCompte *lign = getLigneCompteFromRow(row);
    if  (lign != Q_NULLPTR)
    {
        QMenu *menuContextuel       = new QMenu(this);
        QString msg = tr("Supprimer l'écriture") + " - " + lign->libelle() + " du " + lign->date().toString("d MMM yyyy") + "?";
        QAction *pAction_SupprEcriture = menuContextuel->addAction(msg) ;
        connect (pAction_SupprEcriture, &QAction::triggered,    [=] { SupprimerEcriture(lign); });
        msg = tr("Modifer le montant de l'écriture") + " - " + lign->libelle() + " du " + lign->date().toString("d MMM yyyy") + "?";
        QAction *pAction_ModifEcriture = menuContextuel->addAction(msg) ;
        connect (pAction_ModifEcriture, &QAction::triggered,    [=] { ModifMontant(lign); });

        menuContextuel->exec(cursor().pos());
        delete menuContextuel;
        menuContextuel = Q_NULLPTR;
    }
}

void dlg_comptes::RenvoieRangee(UpCheckBox* Check)
{
    int R = Check->rowTable();
    LigneCompte *lign = getLigneCompteFromRow(R);
    if  (lign != Q_NULLPTR)
    {
        lign->setconsolide(Check->isChecked());
        QString requete = "update " TBL_LIGNESCOMPTES " set " CP_CONSOLIDE_LIGNCOMPTES " = ";
        requete += (Check->isChecked()? "1" : "null");
        requete += " where " CP_IDLIGNE_LIGNCOMPTES " = " + QString::number(lign->id());
        db->StandardSQL(requete);
        CalculeTotal();
    }
}

void dlg_comptes::RedessineFicheArchives()
{
    wdg_tablearchives->clear();
    int             ColCount = 6;
    if (m_modearchives == TOUT)
        ColCount ++;
    wdg_tablearchives     ->setColumnCount(ColCount);

    QStringList LabelARemplir;
    LabelARemplir << tr("NoLigne");
    LabelARemplir << tr("Date");
    LabelARemplir << tr("Type opération");
    LabelARemplir << tr("Libellé opération");
    LabelARemplir << tr("Crédit");
    LabelARemplir << tr("Débit");
    if (m_modearchives == TOUT)
        LabelARemplir << tr("consolidé le");

    wdg_tablearchives->setHorizontalHeaderLabels(LabelARemplir);
    wdg_tablearchives->horizontalHeader()->setVisible(true);
    wdg_tablearchives->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                   // Réglage de la largeur et du nombre des colonnes
    wdg_tablearchives->setColumnWidth(li,0);                                                // idLigne
    li++;
    wdg_tablearchives->setColumnWidth(li,100);                                              // Date affichage européen
    li++;
    wdg_tablearchives->setColumnWidth(li,210);                                              // Type d'opération (virement créditeur, débiteur, retrait, chèque, ...)
    li++;
    wdg_tablearchives->setColumnWidth(li,320);                                              // Libellé opération
    li++;
    wdg_tablearchives->setColumnWidth(li,85);                                               // Crédit
    li++;
    wdg_tablearchives->setColumnWidth(li,85);                                               // Débit
    if (m_modearchives == TOUT)
    {
        li++;
        wdg_tablearchives->setColumnWidth(li,95);                                           // Date consolidation
    }

    int larg = wdg_tablearchives->FixLargeurTotale();
    if (m_modearchives == PARARCHIVE)
    {
        wdg_loupbouton  ->setUpButtonStyle(UpSmallButton::LOUPEBUTTON);
        wdg_loupbouton  ->setImmediateToolTip(tr("Voir tout"));
    }
    else
    {
        wdg_loupbouton  ->setUpButtonStyle(UpSmallButton::CALENDARBUTTON);
        wdg_loupbouton  ->setImmediateToolTip(tr("Revoir par consolidation"));
    }

    wdg_listarchivescombo->setVisible(m_modearchives == PARARCHIVE);
    wdg_flechehtbouton->setVisible(m_modearchives == TOUT);
    dlg_archives->setFixedWidth(larg+20);

}

void dlg_comptes::RemplirTableArchives()
{
    QList<Archive*> listarchives;
    foreach (Archive * arc, *m_archivescptencours->archives())
    {
        if (m_modearchives == PARARCHIVE)
        {
            if (arc->idarchive() == wdg_listarchivescombo->currentData().toInt())
                listarchives << arc;
        }
        else listarchives << arc;
    }
    wdg_tablearchives->setRowCount(listarchives.size());
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
        wdg_tablearchives->setCellWidget(row,col,lbl0);
        col++;

        A = archive->lignedate().toString(tr("d MMM yyyy"));                                            // Date - col = 1
        lbl1->setText(A + " ");
        lbl1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl1->setFocusPolicy(Qt::NoFocus);
        wdg_tablearchives->setCellWidget(row,col,lbl1);
        col++;

        lbl2->setText(" " + archive->lignetypeoperation());                                             // Type opération - col = 2
        lbl2->setFocusPolicy(Qt::NoFocus);
        wdg_tablearchives->setCellWidget(row,col,lbl2);
        col++;

        lbl3->setText(" " + archive->lignelibelle());                                                   // Libellé opération - col = 3;
        lbl3->setFocusPolicy(Qt::NoFocus);
        wdg_tablearchives->setCellWidget(row,col,lbl3);
        col++;

        A = QLocale().toString(archive->montant(),'f',2);                                               // Crédit - col = 4
        if (archive->montant() > 0)
        {
            lbl4->setText(A + " ");
            lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl4->setFocusPolicy(Qt::NoFocus);
            wdg_tablearchives->setCellWidget(row,col,lbl4);
            col++;
            lbl5->setText("");
            lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl5->setFocusPolicy(Qt::NoFocus);
            wdg_tablearchives->setCellWidget(row,col,lbl5);
        }
        else                                                                                            // Débit - col = 5
        {
            lbl4->setText("");
            lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl4->setFocusPolicy(Qt::NoFocus);
            wdg_tablearchives->setCellWidget(row,col,lbl4);
            col++;
            lbl5->setText(A + " ");
            lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
            lbl5->setFocusPolicy(Qt::NoFocus);
            wdg_tablearchives->setCellWidget(row,col,lbl5);
        }
        if (m_modearchives == TOUT)
        {
            col++;
            UpLabel *  lbl6 = new UpLabel();
            lbl6->setContextMenuPolicy(Qt::NoContextMenu);
            lbl6->setText(" " + archive->lignedateconsolidation().toString("d MMM yyyy"));                   // Date consolidation col = 6;
            lbl6->setFocusPolicy(Qt::NoFocus);
            wdg_tablearchives->setCellWidget(row,col,lbl6);
        }
        wdg_tablearchives->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));
    }
}
void dlg_comptes::VoirArchives()
{
    dlg_archives            = new UpDialog(PATH_FILE_INI, "PositionsFiches/PositionArchives", this);
    wdg_tablearchives       = new UpTableWidget();
    wdg_listarchivescombo   = new QComboBox();
    wdg_lbltitre            = new UpLabel();
    QHBoxLayout *titreLay   = new QHBoxLayout();
    wdg_loupbouton          = new UpSmallButton();
    wdg_flechehtbouton      = new UpSmallButton();

    wdg_tablearchives      ->setFocusPolicy(Qt::NoFocus);
    wdg_tablearchives      ->setPalette(QPalette(Qt::white));
    wdg_tablearchives      ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_tablearchives      ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_tablearchives      ->verticalHeader()->setVisible(false);
    wdg_tablearchives      ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_tablearchives      ->setGridStyle(Qt::SolidLine);
    wdg_tablearchives      ->verticalHeader()->setVisible(false);
    wdg_tablearchives      ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical
    wdg_tablearchives      ->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    wdg_lbltitre           ->setText(tr("Liste des remises archivées sur le compte ") + m_compteencours->nomabrege() + " ");
    wdg_flechehtbouton     ->setIcon( Icons::icFlecheHaut() );
    wdg_flechehtbouton     ->setCursor(Qt::PointingHandCursor);
    wdg_flechehtbouton     ->setImmediateToolTip(tr("Voir les archives précédentes"));
    wdg_flechehtbouton     ->setIconSize(QSize(30,30));

    titreLay    ->addWidget(wdg_lbltitre);
    titreLay    ->addWidget(wdg_listarchivescombo);
    titreLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    titreLay    ->addWidget(wdg_flechehtbouton);
    titreLay    ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    titreLay    ->addWidget(wdg_loupbouton);
    titreLay    ->setSpacing(5);
    titreLay    ->setContentsMargins(0,0,0,0);

    /*Il y a 2 modes d'affichage
     * PARARCHIVE : on choisit dans un combobox l'archive qu'on veut afficher et on affiche les écritures archive par archive - c'est le mode de départ
     * TOUT : on affiche toutes les écritures datant de moins de 6 mois
     * L'intervalle de 6 mois se règle avec la variable intervalledate (dans ce cas 180)
     * On remonde dans le temps par pas de 6 mois en mode TOUT en cliquant sur la flèche haut mais c'est lent
     * */
    m_modearchives = PARARCHIVE;
    RedessineFicheArchives();

    dlg_archives->dlglayout()   ->insertWidget(0,wdg_tablearchives);
    dlg_archives->dlglayout()   ->insertLayout(0, titreLay);

    dlg_archives->AjouteLayButtons(UpDialog::ButtonOK);
    connect(dlg_archives->OKButton,     &QPushButton::clicked,              dlg_archives, [=] {dlg_archives->close();});
    dlg_archives->setModal(true);
    dlg_archives->dlglayout()->setStretch(0,1);
    dlg_archives->dlglayout()->setStretch(1,15);

    QList<Archive*> listarchives = db->loadArchiveByDate(m_dateencours, m_compteencours, m_intervalledate);
    m_dateencours = m_dateencours.addDays(-m_intervalledate);
    m_archivescptencours = new Archives();
    m_archivescptencours->addArchive(listarchives);
    if (listarchives.size()==0)
        UpMessageBox::Watch(this, tr("Aucune écriture archivée depuis ") + QString::number(m_intervalledate) + tr("jours"));
    // toute la manip qui suit sert à remetre les banques par ordre aplhabétique - si vous trouvez plus simple, ne vous génez pas
    QStandardItemModel *model = new QStandardItemModel(this);
    foreach (Archive * arc, *m_archivescptencours->archives())
    {
        QList<QStandardItem *> items;
        QString titre = tr("Consolidation") + " " + QString::number(arc->idarchive()) + " "
                + tr("du") + " " + arc->lignedateconsolidation().toString("d MMM yyyy");
        items << new QStandardItem(titre)
              << new QStandardItem(QString::number(arc->idarchive()));
        if (model->findItems(titre).size()==0)
            model->appendRow(items);
    }
    model->sort(1);
    wdg_listarchivescombo->clear();
    for(int i=0; i<model->rowCount(); i++)
        wdg_listarchivescombo->addItem(model->item(i,0)->text(), model->item(i,1)->text().toInt());

    connect(wdg_loupbouton,         &QPushButton::clicked,       this,  [=]
                {
                    m_modearchives = (m_modearchives == PARARCHIVE? TOUT : PARARCHIVE);
                    RedessineFicheArchives();
                    RemplirTableArchives();
                });
    connect(wdg_listarchivescombo,  QOverload<int>::of(&QComboBox::currentIndexChanged) ,this,  &dlg_comptes::RemplirTableArchives);
    connect(wdg_flechehtbouton,     &QPushButton::clicked ,this,   [=]
                {
                    QList<Archive*> listarchives = db->loadArchiveByDate(m_dateencours, m_compteencours, m_intervalledate);
                    m_dateencours = m_dateencours.addDays(-m_intervalledate);
                    m_archivescptencours->addArchive(listarchives);
                    RemplirTableArchives();
                });
    wdg_listarchivescombo->setMaxVisibleItems(20);
    wdg_listarchivescombo->setFocusPolicy(Qt::StrongFocus);
    wdg_listarchivescombo->setCurrentIndex(wdg_listarchivescombo->count()-1);
    /*! si la liste n'a qu'un élément, le fait de déclencehr setCurrentIndex() ne change pas le current index
     * et RemplirTableArchives() n'est pas lancé
     * d'où la suite */
    if (wdg_listarchivescombo->count() == 1)
        RemplirTableArchives();
    dlg_archives->exec();
    m_dateencours = QDate::currentDate();
    delete dlg_archives;

    m_archivescptencours->clearAll();
    delete m_archivescptencours;
    m_archivescptencours = Q_NULLPTR;
}

void dlg_comptes::ModifMontant(LigneCompte *lign)
{
    UpDialog *dlg_montant = new UpDialog(this);
    dlg_montant     ->setModal(true);
    dlg_montant     ->setWindowTitle(tr("Modification d'une écriture!"));
    QString txtlbl = (tr("Vous avez choisi de modifier le montant de l'écriture") + "\n" +
                      lign->libelle() + " du " + lign->date().toString("d MMM yyyy") + "\n\n" +
                      tr("Cette modification est définitive mais ne supprimera") + "\n" +
                      tr("pas l'opération de recette/dépense correspondante.") + "\n" +
                      tr("Modifier le montant d'une ligne du compte bancaire") + "\n" +
                      tr("sert en général à équilibrer le compte pour le rendre") + "\n" +
                      tr("conforme au relevé") + ".\n\n" +
                      tr("Entrez le nouveau montant") + ".\n");

    UpLineEdit *line = new UpLineEdit(dlg_montant);
    line->setMaxLength(10);
    line->setFixedWidth(100);
    line->setText(QString::number(lign->montant(), 'f', 2));
    line->setAlignment(Qt::AlignRight);
    QDoubleValidator *val = new QDoubleValidator(this);
    val->setDecimals(2);
    line->setValidator(val);
    QHBoxLayout *linelay = new QHBoxLayout();
    linelay     ->addSpacerItem(new QSpacerItem(5,5, QSizePolicy::Minimum, QSizePolicy::Expanding));
    linelay     ->addWidget(line);
    linelay     ->addSpacerItem(new QSpacerItem(5,5, QSizePolicy::Minimum, QSizePolicy::Expanding));
    linelay     ->setContentsMargins(0,0,0,0);
    dlg_montant->dlglayout()->insertLayout(0,linelay);
    UpLabel *lbl = new UpLabel();
    lbl->setText(txtlbl);
    dlg_montant->dlglayout()->insertWidget(0,lbl);
    line->setFocus();

    dlg_montant->AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    connect(dlg_montant->OKButton, &QPushButton::clicked, this, [=]
    {
        int row = getRowFromLigneCompte(lign);
        lign->setmontant(QLocale().toDouble(line->text()));
        SetLigneCompteToRow(lign, row);
        QString req = "update " TBL_LIGNESCOMPTES " set " CP_MONTANT_LIGNCOMPTES " = " + QString::number(lign->montant()) + " where " CP_IDLIGNE_LIGNCOMPTES " = " + QString::number(lign->id());
        DataBase::I()->StandardSQL(req);
        CalculeTotal();
        dlg_montant->accept();
    });
    connect(dlg_montant->CancelButton,  &QPushButton::clicked, dlg_montant, &QDialog::reject);
    dlg_montant->dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    dlg_montant->exec();
}

int dlg_comptes::getRowFromLigneCompte(LigneCompte *lign)
{
    int row = -1;
    for (int i=0; i<wdg_bigtable->rowCount(); ++i)
    {
        UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_bigtable->cellWidget(i,0));
        if (lbl == Q_NULLPTR)
            continue;
        if (lbl->iD() == lign->id())
        {
            row = i;
            break;
        }
    }
    return row;
}

void dlg_comptes::SupprimerEcriture(LigneCompte *lign)
{
    UpMessageBox msgbox;
    msgbox.setText(tr("Suppression d'une écriture!"));
    msgbox.setInformativeText(tr("Vous avez choisi de supprimer l'écriture") + "\n"
                              + lign->libelle() + " du " + lign->date().toString("d MMM yyyy") + "\n\n" +
                              tr("Cette suppression est définitive mais ne supprimera pas l'opération de recette/dépense correspondante.") + "\n" +
                              tr("Supprimer une écriture du compte bancaire sert en général à équilibrer le compte pour le rendre conforme au relevé") + ".\n" +
                              tr("Confirmez vous la suppression?") + "\n\n");
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton OKBouton;
    OKBouton.setText(tr("Supprimer"));
    UpSmallButton NoBouton;
    NoBouton.setText(tr("Annuler"));
    msgbox.addButton(&NoBouton, UpSmallButton::CANCELBUTTON);
    msgbox.addButton(&OKBouton, UpSmallButton::STARTBUTTON);
    msgbox.exec();
    if (msgbox.clickedButton() == &OKBouton)
    {
        Datas::I()->lignescomptes->SupprimeLigne(lign);
        RemplitLaTable(m_idcompte);
    }
}

void dlg_comptes::CalculeTotal()
{
    double Total = m_soldesurreleve;
    double TotalConsolide = m_soldesurreleve;
    foreach (LigneCompte *lign, Datas::I()->lignescomptes->lignescomptes()->values())
    {
        double montantligne = lign->montant();
        if (!lign->iscredit())
            montantligne = montantligne*-1;
        Total += montantligne;
        if (lign->isconsolide())
            TotalConsolide +=  montantligne;
    }
    ui->MontantSoldeBrutlabel->setText(QLocale().toString(Total,'f',2) + " ");
    ui->MontantSoldeConsolidelabel->setText(QLocale().toString(TotalConsolide,'f',2) + " ");
}

void dlg_comptes::ChangeCompte(int idcompte)
{
    m_idcompte = ui->BanquecomboBox->itemData(idcompte).toInt();
    m_compteencours = Datas::I()->comptes->getById(m_idcompte);
    if (m_compteencours != Q_NULLPTR)
    {
        m_soldesurreleve = m_compteencours->solde();
        m_compteencours->setsolde(m_soldesurreleve);  // à tout hasard
        ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(m_soldesurreleve,'f',2) + " ");
        RemplitLaTable(m_idcompte);
    }
    else
    {
        UpMessageBox::Watch(this,tr("Pas d'écriture sur ce compte!"));
        wdg_bigtable->clearContents();
        ui->MontantSoldeBrutlabel->setText("0,00 ");
        ui->MontantSoldeConsolidelabel->setText("0,00 ");
        ui->MontantSoldeSurRelevelabel->setText("0,00 ");
    }
}

void dlg_comptes::closeEvent(QCloseEvent *event)
{
    proc->settings()->setValue("PositionsFiches/PositionComptes",saveGeometry());
    event->accept();
}

// ------------------------------------------------------------------------------------------
// Interception des évènements internes
// ------------------------------------------------------------------------------------------
bool dlg_comptes::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == wdg_bigtable)
        if (event->type() == QEvent::KeyPress)            // l'apppui sur space fait changer d'état le checkbox
        {
            QKeyEvent *keyEvent= static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space)
            {
                QList<QTableWidgetSelectionRange>  RangeeSelectionne = wdg_bigtable->selectedRanges();
                if (RangeeSelectionne.size() > 0)
                {
                    int ab = RangeeSelectionne.at(0).topRow();
                    QWidget* Wdg = dynamic_cast<QWidget*>(wdg_bigtable->cellWidget(ab,6));
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

bool dlg_comptes::initOK() const
{
    return m_initok;
}

void dlg_comptes::DefinitArchitetureTable()
{
    int                 ColCount;
    
    wdg_bigtable->setFocusPolicy(Qt::NoFocus);
    ColCount = 9;
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
    LabelARemplir << tr("NoLigne");
    LabelARemplir << tr("Date");
    LabelARemplir << tr("Type opération");
    LabelARemplir << tr("Libellé opération");
    LabelARemplir << tr("Crédit");
    LabelARemplir << tr("Débit");
    LabelARemplir << tr("Consolidé");
    LabelARemplir << "idDep";
    LabelARemplir << "idRec";

    wdg_bigtable->setHorizontalHeaderLabels(LabelARemplir);
    wdg_bigtable->horizontalHeader()->setVisible(true);
    wdg_bigtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    int li = 0;                                                                     // Réglage de la largeur et du nombre des colonnes
    wdg_bigtable->setColumnWidth(li,0);                                                // idLigne
    li++;
    wdg_bigtable->setColumnWidth(li,100);                                              // Date affichage européen
    li++;
    wdg_bigtable->setColumnWidth(li,210);                                              // Type d'opération (virement créditeur, débiteur, retrait, chèque, ...)
    li++;
    wdg_bigtable->setColumnWidth(li,320);                                              // Libellé opération
    li++;
    wdg_bigtable->setColumnWidth(li,85);                                               // Crédit
    li++;
    wdg_bigtable->setColumnWidth(li,85);                                              // Débit
    li++;
    wdg_bigtable->setColumnWidth(li,78);                                               // Consolidé
    li++;
    wdg_bigtable->setColumnWidth(li,10);                                               // idDep
    li++;
    wdg_bigtable->setColumnWidth(li,10);                                               // idRec

    wdg_bigtable->setColumnHidden(7,true);
    wdg_bigtable->setColumnHidden(8,true);

    wdg_bigtable->setGridStyle(Qt::SolidLine);

}

void dlg_comptes::RemplitLaTable(int idcompte)
{
    Datas::I()->lignescomptes->initListe(idcompte);

    if (Datas::I()->lignescomptes->lignescomptes()->size() == 0)
    {
        UpMessageBox::Watch(this,tr("Pas d'écriture sur le compte"));
        wdg_bigtable->clearContents();
        ui->MontantSoldeSurRelevelabel->setText(QLocale().toString(m_soldesurreleve, 'f', 2) + " ");
        CalculeTotal();
    }
    wdg_bigtable->clearContents();

    wdg_bigtable->setRowCount(Datas::I()->lignescomptes->lignescomptes()->size());
    // toute la manip qui suit sert à remettre les lignes par ordre chronologique - si vous trouvez plus simple, ne vous génez pas

    QStandardItemModel *listlign = new QStandardItemModel();
    foreach(LigneCompte *lign, Datas::I()->lignescomptes->lignescomptes()->values())
    {
        UpStandardItem *item = new UpStandardItem(lign->date().toString("yyyyMMdd"), lign);
        listlign->appendRow(item);
    }
    listlign->sort(0);

    for(int i=0; i<listlign->rowCount(); i++)
    {
        UpStandardItem *itm = dynamic_cast<UpStandardItem*>(listlign->item(i));
        if (itm != Q_NULLPTR)
        {
            LigneCompte *lign = dynamic_cast<LigneCompte*>(itm->item());
            if (lign != Q_NULLPTR)
                SetLigneCompteToRow(lign, i);
            delete itm;
        }
    }
    delete listlign;
    CalculeTotal();
 }

LigneCompte* dlg_comptes::getLigneCompteFromRow(int row)
{
    int idcompte = 0;
    UpLabel *lbl = dynamic_cast<UpLabel*>(wdg_bigtable->cellWidget(row,0));
    if (lbl == Q_NULLPTR)
        return Q_NULLPTR;
    idcompte = lbl->iD();
    return Datas::I()->lignescomptes->getById(idcompte);
}

void dlg_comptes::SetLigneCompteToRow(LigneCompte *lign, int row)
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
    lbl0->setiD(lign->id());                      // idLigne
    lbl1->setiD(lign->id());                      // idLigne
    lbl2->setiD(lign->id());                      // idLigne
    lbl3->setiD(lign->id());                      // idLigne
    lbl4->setiD(lign->id());                      // idLigne
    lbl5->setiD(lign->id());                      // idLigne
    lbl7->setiD(lign->id());                      // idLigne
    lbl8->setiD(lign->id());                      // idLigne
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

    A = QString::number(lign->id());                                                             // idLigne - col = 0
    lbl0->setText(A + " ");
    lbl0->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    lbl0->setFocusPolicy(Qt::NoFocus);
    wdg_bigtable->setCellWidget(row,col,lbl0);
    col++;

    A = lign->date().toString(tr("d MMM yyyy"));                                        // Date - col = 1
    lbl1->setText(A + " ");
    lbl1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    lbl1->setFocusPolicy(Qt::NoFocus);
    wdg_bigtable->setCellWidget(row,col,lbl1);
    col++;

    lbl2->setText(" " + lign->typeoperation());                                                // Type opération - col = 2
    lbl2->setFocusPolicy(Qt::NoFocus);
    wdg_bigtable->setCellWidget(row,col,lbl2);
    col++;

    lbl3->setText(" " + lign->libelle());                                                // Libellé opération - col = 3;
    lbl3->setFocusPolicy(Qt::NoFocus);
    wdg_bigtable->setCellWidget(row,col,lbl3);
    col++;

    if (lign->iscredit())                                                                // Crédit - col = 4
    {
        A = QLocale().toString(lign->montant(),'f',2);
        lbl4->setText(A + " ");
        lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl4->setFocusPolicy(Qt::NoFocus);
        wdg_bigtable->setCellWidget(row,col,lbl4);
        col++;
        lbl5->setText("");
        lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl5->setFocusPolicy(Qt::NoFocus);
        wdg_bigtable->setCellWidget(row,col,lbl5);
    }
    else                                                                // Débit - col = 5
    {
        A = QLocale().toString(lign->montant()*-1,'f',2);
        lbl4->setText("");
        lbl4->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl4->setFocusPolicy(Qt::NoFocus);
        wdg_bigtable->setCellWidget(row,col,lbl4);
        col++;
        lbl5->setText(A + " ");
        lbl5->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        lbl5->setFocusPolicy(Qt::NoFocus);
        wdg_bigtable->setCellWidget(row,col,lbl5);
    }
    col++;

    wdg = new QWidget(this);
    Checkbx = new UpCheckBox(wdg);
    if (lign->isconsolide())
        Checkbx->setCheckState(Qt::Checked);
    else
        Checkbx->setCheckState(Qt::Unchecked);
    Checkbx->setRowTable(row);
    Checkbx->setFocusPolicy(Qt::NoFocus);

    connect(Checkbx,      &QCheckBox::clicked,  [=] {RenvoieRangee(Checkbx);});
    l = new QHBoxLayout(wdg);
    l->setContentsMargins(0,0,0,0);
    l->setAlignment( Qt::AlignCenter );
    l->addWidget(Checkbx);
    wdg->setLayout(l);
    wdg_bigtable->setCellWidget(row,col,wdg);
    col++;

    lbl7->setText(QString::number(lign->iddepense()));
    lbl7->setFocusPolicy(Qt::NoFocus);
    wdg_bigtable->setCellWidget(row,col,lbl7);
    col++;

    lbl8->setText(QString::number(lign->idrecette()));
    lbl8->setFocusPolicy(Qt::NoFocus);
    wdg_bigtable->setCellWidget(row,col,lbl8);

    wdg_bigtable->setRowHeight(row,int(QFontMetrics(qApp->font()).height()*1.3));

}
