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

#include "fichevitale.h"

#include <uptableview.h>
#include <uplabel.h>
#include "database.h"
#include "utils.h"
#include "macros.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QApplication>
#include <QPixmap>
#include <QDate>
#include <QPushButton>

// ============================================================================================
//  Helpers de présentation
// ============================================================================================

namespace {

// "M"/"F"/"" à partir du 1er chiffre du NIR (1 = homme, 2 = femme). Sert à choisir la figurine.
QString sexeDepuisNIR(const QString &nir)
{
    if (nir.isEmpty())    return QString();
    if (nir.at(0) == '1') return "M";
    if (nir.at(0) == '2') return "F";
    return QString();
}

// Nom en MAJUSCULES + Prénom capitalisé, exactement comme la liste des patients
// (cf. Rufus::Remplir_ListePatients_TableView : nom.toUpper() ; prénom via trimcapitilize).
QString nomPrenom(const QString &nom, const QString &prenom)
{
    return nom.toUpper() + " " + Utils::trimcapitilize(prenom);
}

// La carte donne la date en jj/MM/aaaa ; la liste des patients l'affiche en jj-MM-aaaa. On aligne.
QString ddnAffichee(const QString &jjMMaaaa)
{
    const QDate d = QDate::fromString(jjMMaaaa, "dd/MM/yyyy");
    return d.isValid() ? d.toString("dd-MM-yyyy") : jjMMaaaa;
}

// En-têtes des 2 colonnes (Nom / Date de naissance), comme la liste des patients.
void poseEntetes(QStandardItemModel *model)
{
    QStandardItem *hNom = new QStandardItem(QObject::tr("Nom"));
    QStandardItem *hDdn = new QStandardItem(QObject::tr("Date de naissance"));
    hNom->setTextAlignment(Qt::AlignLeft);
    hDdn->setTextAlignment(Qt::AlignLeft);
    model->setHorizontalHeaderItem(0, hNom);
    model->setHorizontalHeaderItem(1, hDdn);
}

void ajouteLigne(QStandardItemModel *model, const QString &nomprenom, const QString &ddn)
{
    QStandardItem *c0 = new QStandardItem(nomprenom);
    QStandardItem *c1 = new QStandardItem(ddn);
    c0->setEditable(false);
    c1->setEditable(false);
    model->appendRow(QList<QStandardItem*>() << c0 << c1);
}

// Applique à une UpTableView le look EXACT de PatientsListeTableView (largeurs 230/122, lignes
// alternées, sélection par ligne, en-têtes fixes, viewport blanc…), et fige sa hauteur pour
// afficher lignesVisibles lignes.
void styleTableListe(UpTableView *tbl, int lignesVisibles, bool scrollVertical)
{
    tbl->setFrameShape(QFrame::Box);
    tbl->setFrameShadow(QFrame::Raised);
    tbl->setShowGrid(false);
    tbl->setAlternatingRowColors(true);
    tbl->setSelectionMode(QAbstractItemView::SingleSelection);
    tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tbl->setVerticalScrollBarPolicy(scrollVertical ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    tbl->setCursor(Qt::PointingHandCursor);
    tbl->setMouseTracking(true);

    const int h = int(QFontMetrics(qApp->font()).height() * 1.3);
    QHeaderView *hh = tbl->horizontalHeader();
    hh->setSectionResizeMode(QHeaderView::Fixed);
    hh->setFixedHeight(h);
    QHeaderView *vh = tbl->verticalHeader();
    vh->setVisible(false);
    vh->setSectionResizeMode(QHeaderView::Fixed);
    vh->setDefaultSectionSize(h);
    tbl->setColumnWidth(0, 230);
    tbl->setColumnWidth(1, 122);

    // Viewport blanc opaque (comme la liste des patients), les lignes alternées se peignant par-dessus.
    tbl->viewport()->setAutoFillBackground(true);
    QPalette pal = tbl->viewport()->palette();
    pal.setColor(tbl->viewport()->backgroundRole(), Qt::white);
    tbl->viewport()->setPalette(pal);

    tbl->setFixedHeight(h * (lignesVisibles + 1) + 2);   // + l'en-tête
    tbl->FixLargeurTotale();
}

} // namespace

// ============================================================================================
//  FicheVitale
// ============================================================================================

FicheVitale::FicheVitale(const QList<LecteurVitale::Porteur> &porteurs, QWidget *parent)
    : UpDialog(parent), m_porteurs(porteurs), m_db(DataBase::I())
{
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Carte Vitale"));

    const LecteurVitale::Porteur assure = m_porteurs.isEmpty() ? LecteurVitale::Porteur() : m_porteurs.first();

    // ---- En-tête : figurine (gauche) + NNI et table du porteur (droite) ----
    QHBoxLayout *entete = new QHBoxLayout;

    QDate ddn = QDate::fromString(assure.dateNaissance, "dd/MM/yyyy");
    QMap<QString,QVariant> age = Utils::CalculAge(ddn, QDate::currentDate(), sexeDepuisNIR(assure.nir));
    QString img = age.value("icone").toString();
    if (img.isEmpty())
        img = "silhouette";
    UpLabel *figurine = new UpLabel();
    figurine->setPixmap(QPixmap(":/" + img + ".png").scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    figurine->setFixedSize(100, 100);
    entete->addWidget(figurine, 0, Qt::AlignTop);

    QVBoxLayout *droite = new QVBoxLayout;
    UpLabel *nni = new UpLabel();
    nni->setText(tr("N° SS : ") + LecteurVitale::nirLisible(assure.nir));
    droite->addWidget(nni);

    // Table du porteur (1 ligne)
    QStandardItemModel *modPorteur = new QStandardItemModel(this);
    poseEntetes(modPorteur);
    ajouteLigne(modPorteur, nomPrenom(assure.nom, assure.prenom), ddnAffichee(assure.dateNaissance));
    m_tblPorteur = new UpTableView();
    m_tblPorteur->setModel(modPorteur);
    styleTableListe(m_tblPorteur, 1, false);
    droite->addWidget(m_tblPorteur);

    // Table des ayants droit, séparée par un petit espace
    droite->addSpacing(8);
    QStandardItemModel *modAyants = new QStandardItemModel(this);
    poseEntetes(modAyants);
    for (int i = 1; i < m_porteurs.size(); ++i)
        ajouteLigne(modAyants, nomPrenom(m_porteurs.at(i).nom, m_porteurs.at(i).prenom), ddnAffichee(m_porteurs.at(i).dateNaissance));
    m_tblAyants = new UpTableView();
    m_tblAyants->setModel(modAyants);
    styleTableListe(m_tblAyants, qMax(1, m_porteurs.size() - 1), false);
    droite->addWidget(m_tblAyants);
    droite->addStretch();

    entete->addLayout(droite);
    dlglayout()->addLayout(entete);

    // ---- Correspondances : 3ème table, décalée à gauche (pleine largeur) ----
    dlglayout()->addSpacing(10);
    UpLabel *lblCorresp = new UpLabel();
    lblCorresp->setText(tr("Dossiers correspondants"));
    dlglayout()->addWidget(lblCorresp, 0, Qt::AlignLeft);
    QStandardItemModel *modCorresp = new QStandardItemModel(this);
    poseEntetes(modCorresp);
    m_tblCorresp = new UpTableView();
    m_tblCorresp->setModel(modCorresp);
    styleTableListe(m_tblCorresp, 6, true);              // ~6 lignes visibles, ascenseur si besoin
    dlglayout()->addWidget(m_tblCorresp, 0, Qt::AlignLeft);

    AjouteLayButtons(UpDialog::ButtonOK);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    connect(OKButton, &QPushButton::clicked, this, &QDialog::accept);

    // ---- Surbrillance partagée + navigation clavier ----
    connect(m_tblPorteur->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FicheVitale::selectionPorteurChangee);
    connect(m_tblAyants->selectionModel(),  &QItemSelectionModel::selectionChanged, this, &FicheVitale::selectionAyantChangee);
    m_tblPorteur->installEventFilter(this);
    m_tblAyants->installEventFilter(this);

    // À l'ouverture : le porteur est en surbrillance (ce qui déclenche la 1ère recherche).
    selectionnePorteur();
}

//-----------------------------------------------------------------------------------------------------
// Navigation clavier : flèche bas depuis le porteur -> 1er ayant droit ; flèche haut depuis le 1er
// ayant droit -> porteur. Le reste (dans la table des ayants droit) suit le comportement standard.
//-----------------------------------------------------------------------------------------------------
bool FicheVitale::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress)
        {
        QKeyEvent *ke = static_cast<QKeyEvent*>(ev);
        if (obj == m_tblPorteur && ke->key() == Qt::Key_Down)
            {
            if (m_porteurs.size() > 1)
                { selectionneAyant(0); return true; }
            }
        else if (obj == m_tblAyants && ke->key() == Qt::Key_Up)
            {
            const QModelIndexList sel = m_tblAyants->selectionModel()->selectedRows();
            if (!sel.isEmpty() && sel.first().row() == 0)
                { selectionnePorteur(); return true; }
            }
        }
    return UpDialog::eventFilter(obj, ev);
}

//-----------------------------------------------------------------------------------------------------
// Une ligne du porteur devient surbrillante -> on retire la surbrillance des ayants droit, puis on
// recalcule les correspondances pour l'assuré.
//-----------------------------------------------------------------------------------------------------
void FicheVitale::selectionPorteurChangee()
{
    if (m_majSelection)
        return;
    if (m_tblPorteur->selectionModel()->selectedRows().isEmpty())
        return;
    m_majSelection = true;
    m_tblAyants->clearSelection();
    m_majSelection = false;
    if (!m_porteurs.isEmpty())
        surbrillanceChangee(m_porteurs.first());
}

void FicheVitale::selectionAyantChangee()
{
    if (m_majSelection)
        return;
    const QModelIndexList sel = m_tblAyants->selectionModel()->selectedRows();
    if (sel.isEmpty())
        return;
    m_majSelection = true;
    m_tblPorteur->clearSelection();
    m_majSelection = false;
    const int idx = sel.first().row() + 1;           // [0] = assuré, ayants droit à partir de 1
    if (idx < m_porteurs.size())
        surbrillanceChangee(m_porteurs.at(idx));
}

void FicheVitale::selectionnePorteur()
{
    m_tblPorteur->setFocus();
    m_tblPorteur->selectRow(0);                       // déclenche selectionPorteurChangee
}

void FicheVitale::selectionneAyant(int row)
{
    m_tblAyants->setFocus();
    m_tblAyants->selectRow(row);                      // déclenche selectionAyantChangee
}

//-----------------------------------------------------------------------------------------------------
// Recalcule les correspondances : cherche en base les dossiers dont nom + prénom + date de naissance
// collent à la personne surbrillée, et remplit la table du bas.
//-----------------------------------------------------------------------------------------------------
void FicheVitale::surbrillanceChangee(const LecteurVitale::Porteur &porteur)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(m_tblCorresp->model());
    if (model == nullptr)
        return;
    model->removeRows(0, model->rowCount());          // vide les lignes, garde les en-têtes

    const QDate ddn = QDate::fromString(porteur.dateNaissance, "dd/MM/yyyy");
    if (!ddn.isValid())
        return;

    // Échappement des apostrophes pour ne pas casser la requête (noms type O'BRIEN).
    QString nom    = porteur.nom.toUpper();    nom.replace("'", "''");
    QString prenom = porteur.prenom.toUpper(); prenom.replace("'", "''");

    const QString req =
            "SELECT PatNom, PatPrenom, PatDDN FROM " TBL_PATIENTS
            " WHERE UPPER(PatNom) LIKE '" + nom + "%'"
            " AND UPPER(PatPrenom) LIKE '" + prenom + "%'"
            " AND PatDDN = '" + ddn.toString("yyyy-MM-dd") + "'";

    bool ok = false;
    const QList<QVariantList> resultats = m_db->StandardSelectSQL(req, ok);
    if (!ok)
        return;
    for (const QVariantList &ligne : resultats)
        {
        const QString nomBase    = ligne.at(0).toString();
        const QString prenomBase = ligne.at(1).toString();
        const QDate   ddnBase    = ligne.at(2).toDate();
        const QString ddnAff     = ddnBase.isValid() ? ddnBase.toString("dd-MM-yyyy") : ligne.at(2).toString();
        ajouteLigne(model, nomPrenom(nomBase, prenomBase), ddnAff);
        }
}
