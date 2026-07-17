/* (C) 2026 LAINE SERGE
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

#include "recherchedossier.h"

#include <uptableview.h>
#include <upcheckbox.h>
#include <upgroupbox.h>
#include <uplabel.h>
#include <upstandarditem.h>
#include "database.h"
#include "gbl_datas.h"
#include "cls_patient.h"
#include "cls_patients.h"
#include "cls_users.h"
#include "cls_user.h"
#include "utils.h"
#include "macros.h"

#include <QLineEdit>
#include <QDateEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QFontMetrics>
#include <QApplication>
#include <QPushButton>
#include <QMenu>
#include <QToolTip>
#include <QCursor>

// Look de la table, calqué sur PatientsListeTableView (largeurs 230/122, lignes alternées, en-têtes).
static void styleTable(UpTableView *tbl)
{
    tbl->setFrameShape(QFrame::Box);
    tbl->setFrameShadow(QFrame::Raised);
    tbl->setShowGrid(false);
    tbl->setAlternatingRowColors(true);
    tbl->setSelectionMode(QAbstractItemView::SingleSelection);
    tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tbl->setCursor(Qt::PointingHandCursor);
    tbl->setMouseTracking(true);                       // pour le signal entered (info-bulle)
    tbl->setContextMenuPolicy(Qt::CustomContextMenu);
    tbl->verticalHeader()->setVisible(false);

    const int h = int(QFontMetrics(qApp->font()).height() * 1.3);
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tbl->horizontalHeader()->setFixedHeight(h);
    tbl->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tbl->verticalHeader()->setDefaultSectionSize(h);
    tbl->setColumnWidth(0, 230);
    tbl->setColumnWidth(1, 122);

    tbl->viewport()->setAutoFillBackground(true);
    QPalette pal = tbl->viewport()->palette();
    pal.setColor(tbl->viewport()->backgroundRole(), Qt::white);
    tbl->viewport()->setPalette(pal);

    tbl->setFixedHeight(h * 11 + 4);        // ~10 lignes visibles + en-tête, ascenseur au-delà
    tbl->FixLargeurTotale();
}

RechercheDossier::RechercheDossier(const QString &nom, const QString &prenom, const QDate &ddn, QWidget *parent)
    : UpDialog(parent), m_db(DataBase::I())
{
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Recherche d'un dossier"));

    // ---- Haut : champs (gauche) + choix des critères (droite) ----
    QVBoxLayout *champs = new QVBoxLayout;
    lNom        = new UpLabel();
    lNom        ->setText(tr("Nom"));
    lNom        ->setAlignment(Qt::AlignCenter);
    m_nom       = new QLineEdit();
    m_nom       ->setMaxLength(45);
    m_nom       ->setFixedWidth(150);
    lPre        = new UpLabel();
    lPre        ->setText(tr("Prénom"));
    lPre        ->setAlignment(Qt::AlignCenter);
    m_prenom    = new QLineEdit();
    m_prenom    ->setMaxLength(45);
    m_prenom    ->setFixedWidth(150);
    lDdn        = new UpLabel();
    lDdn        ->setText(tr("Date de naissance"));
    lDdn        ->setAlignment(Qt::AlignCenter);
    m_ddn       = new QDateEdit();
    m_ddn       ->setCalendarPopup(true);
    m_ddn       ->setDisplayFormat(tr("dd/MM/yyyy"));
    m_ddn       ->setMinimumDate(QDate(1900, 1, 1));
    m_ddn       ->setFixedWidth(150);
    m_ddn       ->setDate(m_datepardefaut);

    champs->addWidget(lNom);
    champs->addWidget(m_nom);
    champs->addWidget(lPre);
    champs->addWidget(m_prenom);
    champs->addWidget(lDdn);
    champs->addWidget(m_ddn);
    champs->setSpacing(5);
    champs->addSpacerItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    champs->addStretch();
    QWidget *widg_crit  = new QWidget();
    widg_crit           ->setFixedSize(180,120);
    widg_crit           ->setLayout(champs);

    UpGroupBox *critere     = new UpGroupBox();
    QVBoxLayout *critlay    = new QVBoxLayout(critere);
    m_parNom                = new UpCheckBox(tr("par nom / prénom"));
    m_parDdn                = new UpCheckBox(tr("par date de naissance"));
    m_parNom                ->setAutoExclusive(true);
    m_parDdn                ->setAutoExclusive(true);
    critlay                 ->addWidget(m_parNom);
    critlay                 ->addWidget(m_parDdn);
    critlay                 ->addStretch();

    QHBoxLayout *haut       = new QHBoxLayout;
    haut                    ->addWidget(widg_crit);
    haut                    ->addSpacing(10);
    haut                    ->addWidget(critere, 0, Qt::AlignTop);
    haut                    ->addStretch();

    // ---- Table des patients (comme rufus.cpp) ----
    m_model             = new QStandardItemModel(this);
    QStandardItem *hNom = new QStandardItem(tr("Nom"));
    QStandardItem *hDdn = new QStandardItem(tr("Date de naissance"));
    hNom                ->setTextAlignment(Qt::AlignLeft);
    hDdn                ->setTextAlignment(Qt::AlignLeft);
    m_model             ->setHorizontalHeaderItem(0, hNom);
    m_model             ->setHorizontalHeaderItem(1, hDdn);
    m_table             = new UpTableView();
    m_table             ->setModel(m_model);
    styleTable(m_table);

    m_total             = new UpLabel();

    // ---- Assemblage, au-dessus de la barre de boutons ----
    QVBoxLayout *corps  = new QVBoxLayout;
    corps               ->addLayout(haut);
    corps               ->addWidget(m_table, 0, Qt::AlignLeft);
    corps               ->addWidget(m_total, 0, Qt::AlignCenter);
    dlglayout()         ->insertLayout(0, corps);

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    // OK : on mémorise le dossier sélectionné puis on ferme.
    connect(OKButton, &QPushButton::clicked, this, [this] {
        const QModelIndexList sel = m_table->selectionModel()->selectedRows();
        if (!sel.isEmpty()) {
            m_idChoisi = idDeLigne(sel.first());
            User *u = Datas::I()->users->userconnected();
            m_ouvrir = (u != nullptr && u->isSoignant());   // action par défaut selon le rôle
        }
        accept();
    });

    // Recherche relancée à chaque changement de critère.
    connect(m_nom,    &QLineEdit::textChanged, this, &RechercheDossier::rechercher);
    connect(m_prenom, &QLineEdit::textChanged, this, &RechercheDossier::rechercher);
    connect(m_ddn,    &QDateEdit::dateChanged, this, &RechercheDossier::rechercher);
    connect(m_parNom, &QCheckBox::toggled,     this, &RechercheDossier::RegleAffichage);
    connect(m_parDdn, &QCheckBox::toggled,     this, &RechercheDossier::RegleAffichage);

    // Sur la table : double-clic (retenir + fermer), clic droit (menu), survol (info-bulle).
    connect(m_table, &QTableView::doubleClicked,              this, &RechercheDossier::activer);
    connect(m_table, &QTableView::customContextMenuRequested, this, &RechercheDossier::menuContextuel);
    connect(m_table, &QTableView::entered,                    this, &RechercheDossier::afficheBulle);

    m_parNom    ->setChecked(true);

}

/*-----------------------------------------------------------------------------------------------------------------
-- Basculer le mode de recherche du dossier (DDN ou Nom/Prenom) -------------
-----------------------------------------------------------------------------------------------------------------*/
void    RechercheDossier::RegleAffichage()
{
    m_ddn->setDate(m_datepardefaut);
    m_mode      = (m_parNom->isChecked()? RechercheParNom : RechercheParDDN);
    bool a      = (m_mode == RechercheParNom);
    m_nom       ->setVisible(a);
    lNom        ->setVisible(a);
    m_prenom    ->setVisible(a);
    lPre        ->setVisible(a);
    m_ddn       ->setVisible(!a);
    lDdn        ->setVisible(!a);
    if (a)
        m_nom   ->setFocus();
    else
        m_ddn   ->setFocus();
    rechercher();
}

int RechercheDossier::idDeLigne(const QModelIndex &index) const
{
    if (!index.isValid() || m_model == nullptr)
        return 0;
    UpStandardItem *it = static_cast<UpStandardItem*>(m_model->item(index.row(), 0));
    return it ? it->listids().value(0) : 0;
}

void RechercheDossier::activer(const QModelIndex &index)
{
    const int id = idDeLigne(index);
    if (id <= 0)
        return;
    User *u = Datas::I()->users->userconnected();
    m_idChoisi = id;
    m_ouvrir   = (u != nullptr && u->isSoignant());   // action par défaut selon le rôle
    accept();
}

void RechercheDossier::menuContextuel(const QPoint &pos)
{
    const int id = idDeLigne(m_table->indexAt(pos));
    if (id <= 0)
        return;
    User *u = Datas::I()->users->userconnected();
    const bool soignant = (u != nullptr && u->isSoignant());
    QMenu menu;
    QAction *actOuvrir = soignant ? menu.addAction(tr("Ouvrir le dossier")) : nullptr;
    menu.addAction(tr("Inscrire en salle d'attente"));
    QAction *choisi = menu.exec(m_table->viewport()->mapToGlobal(pos));
    if (choisi == nullptr)
        return;
    m_idChoisi = id;
    m_ouvrir   = (choisi == actOuvrir);
    accept();
}

void RechercheDossier::afficheBulle(const QModelIndex &index)
{
    const int id = idDeLigne(index);
    if (id <= 0)
        return;
    Patient *pat = Datas::I()->patients->getById(id, Item::LoadDetails);
    if (pat != nullptr)
        QToolTip::showText(QCursor::pos(), pat->texteInfoBulle(QDate::currentDate()));
}

void RechercheDossier::rechercher()
{
    m_model->removeRows(0, m_model->rowCount());

    // Requête DANS DataBase ; liste LOCALE : on ne touche pas à la table partagée des patients
    // (celle de la fenêtre principale). On libère la liste juste après l'avoir affichée.
    QList<Patient*> resultats = (m_mode == RechercheParDDN)
            ? DataBase::I()->loadPatientsByDDN(m_ddn->date())
            : DataBase::I()->loadPatientsAll(m_nom->text(), m_prenom->text(), true);

    for (Patient *pat : resultats)
        {
        UpStandardItem *c0 = new UpStandardItem(pat->nom().toUpper() + " " + Utils::trimcapitilize(pat->prenom()));
        c0->setListids(QList<int>{ pat->id() });                                       // idPat porté par la ligne
        UpStandardItem *c1 = new UpStandardItem(pat->datedenaissance().toString(tr("dd-MM-yyyy")));
        c0->setEditable(false);
        c1->setEditable(false);
        m_model->appendRow(QList<QStandardItem*>() << c0 << c1);
        }

    int total = resultats.size();
    qDeleteAll(resultats);            // liste locale non partagée -> on la libère
    m_model->sort(0);                 // tri par nom (colonne 0)

    // loadPatientsAll plafonne à 1000 lignes ; au-delà, on demande le vrai total (mode Nom seulement).
    if (m_mode == RechercheParNom && total == 1000)
        total = DataBase::I()->countPatientsAll(m_nom->text(), m_prenom->text());

    if (total == 0)
        m_total->setText(tr("aucun dossier pour ces critères"));
    else if (total == 1)
        m_total->setText(tr("1 dossier"));
    else
        m_total->setText(QString::number(total) + " " + tr("dossiers"));
}
