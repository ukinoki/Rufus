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
    UpLabel *lNom = new UpLabel();    lNom->setText(tr("Nom"));
    m_nom = new QLineEdit();          m_nom->setMaxLength(45);    m_nom->setFixedWidth(150);   m_nom->setText(nom);
    UpLabel *lPre = new UpLabel();    lPre->setText(tr("Prénom"));
    m_prenom = new QLineEdit();       m_prenom->setMaxLength(45); m_prenom->setFixedWidth(150); m_prenom->setText(prenom);
    UpLabel *lDdn = new UpLabel();    lDdn->setText(tr("Date de naissance"));
    m_ddn = new QDateEdit();          m_ddn->setCalendarPopup(true); m_ddn->setDisplayFormat("dd/MM/yyyy");
    m_ddn->setMinimumDate(QDate(1900, 1, 1)); m_ddn->setFixedWidth(150);
    m_ddn->setDate(ddn.isValid() ? ddn : QDate(1999, 12, 31));
    champs->addWidget(lNom);   champs->addWidget(m_nom);
    champs->addWidget(lPre);   champs->addWidget(m_prenom);
    champs->addWidget(lDdn);   champs->addWidget(m_ddn);
    champs->addStretch();

    UpGroupBox *critere = new UpGroupBox();
    critere->setTitle(tr("Rechercher"));
    QVBoxLayout *critlay = new QVBoxLayout(critere);
    m_parNom = new UpCheckBox(tr("par nom / prénom"));
    m_parDdn = new UpCheckBox(tr("par date de naissance"));
    m_parNom->setChecked(true);
    m_parDdn->setChecked(ddn.isValid());
    critlay->addWidget(m_parNom);
    critlay->addWidget(m_parDdn);
    critlay->addStretch();

    QHBoxLayout *haut = new QHBoxLayout;
    haut->addLayout(champs);
    haut->addSpacing(20);
    haut->addWidget(critere, 0, Qt::AlignTop);
    haut->addStretch();

    // ---- Table des patients (comme rufus.cpp) ----
    m_model = new QStandardItemModel(this);
    QStandardItem *hNom = new QStandardItem(tr("Nom"));
    QStandardItem *hDdn = new QStandardItem(tr("Date de naissance"));
    hNom->setTextAlignment(Qt::AlignLeft);
    hDdn->setTextAlignment(Qt::AlignLeft);
    m_model->setHorizontalHeaderItem(0, hNom);
    m_model->setHorizontalHeaderItem(1, hDdn);
    m_table = new UpTableView();
    m_table->setModel(m_model);
    styleTable(m_table);

    m_total = new UpLabel();

    // ---- Assemblage, au-dessus de la barre de boutons ----
    QVBoxLayout *corps = new QVBoxLayout;
    corps->addLayout(haut);
    corps->addWidget(m_table, 0, Qt::AlignLeft);
    corps->addWidget(m_total, 0, Qt::AlignLeft);
    dlglayout()->insertLayout(0, corps);

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
    connect(m_parNom, &QCheckBox::toggled,     this, &RechercheDossier::rechercher);
    connect(m_parDdn, &QCheckBox::toggled,     this, &RechercheDossier::rechercher);

    // Sur la table : double-clic (retenir + fermer), clic droit (menu), survol (info-bulle).
    connect(m_table, &QTableView::doubleClicked,              this, &RechercheDossier::activer);
    connect(m_table, &QTableView::customContextMenuRequested, this, &RechercheDossier::menuContextuel);
    connect(m_table, &QTableView::entered,                    this, &RechercheDossier::afficheBulle);

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

    // Construction des critères selon les cases cochées.
    QStringList conds;
    if (m_parNom->isChecked()) {
        QString nom = m_nom->text().trimmed().toUpper();
        QString pre = m_prenom->text().trimmed().toUpper();
        if (!nom.isEmpty()) { nom.replace("'", "''"); conds << "UPPER(PatNom) LIKE '" + nom + "%'"; }
        if (!pre.isEmpty()) { pre.replace("'", "''"); conds << "UPPER(PatPrenom) LIKE '" + pre + "%'"; }
    }
    if (m_parDdn->isChecked())
        conds << "PatDDN = '" + m_ddn->date().toString("yyyy-MM-dd") + "'";

    if (conds.isEmpty()) {
        m_total->setText(tr("Cochez un critère et saisissez une valeur."));
        return;
    }
    const QString where = conds.join(" AND ");

    bool ok = false;
    const QVariantList cnt = m_db->getFirstRecordFromStandardSelectSQL(
                "SELECT COUNT(*) FROM " TBL_PATIENTS " WHERE " + where, ok);
    const int total = (ok && !cnt.isEmpty()) ? cnt.at(0).toInt() : 0;

    const QList<QVariantList> lignes = m_db->StandardSelectSQL(
                "SELECT idPat, PatNom, PatPrenom, PatDDN FROM " TBL_PATIENTS " WHERE " + where
                + " ORDER BY PatNom, PatPrenom LIMIT 50", ok);
    for (const QVariantList &l : lignes) {
        UpStandardItem *c0 = new UpStandardItem(l.at(1).toString().toUpper() + " " + Utils::trimcapitilize(l.at(2).toString()));
        c0->setListids(QList<int>{ l.at(0).toInt() });
        const QDate d = l.at(3).toDate();
        UpStandardItem *c1 = new UpStandardItem(d.isValid() ? d.toString("dd-MM-yyyy") : l.at(3).toString());
        c0->setEditable(false);
        c1->setEditable(false);
        m_model->appendRow(QList<QStandardItem*>() << c0 << c1);
    }

    m_total->setText(total > 50 ? tr("%1 dossiers (50 affichés)").arg(total)
                                : tr("%1 dossier(s)").arg(total));
}
