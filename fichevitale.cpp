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
#include <upstandarditem.h>
#include <upsmallbutton.h>
#include <uppushbutton.h>
#include "icons.h"
#include "recherchedossier.h"
#include "database.h"
#include "gbl_datas.h"
#include "cls_patient.h"
#include "cls_patients.h"
#include "cls_parametressysteme.h"
#include "cls_users.h"
#include "cls_user.h"
#include "utils.h"
#include "macros.h"

#include <QMenu>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QApplication>
#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <QColor>
#include <QDate>
#include <QPushButton>
#include <QToolTip>
#include <QCursor>
#include <QModelIndex>
#include <QRegularExpression>
#include <QVector>
#include <QPair>
#include <algorithm>

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

// =========================== Rapprochement approximatif (record linkage) =====================
// On ne cherche PAS à filtrer sur le nom (un nom change au mariage, se tape mal…). On récupère
// large sur la DDN, puis on classe par similarité. Pas d'IA : de simples distances de chaînes,
// transparentes et réglables, font mieux et se lisent.

// Normalise pour la COMPARAISON (pas pour l'affichage) : majuscules + suppression des accents
// (é→e, ç→c…). La plupart des dossiers sont saisis sans accents ; on compare donc à égalité.
QString normalise(const QString &s)
{
    const QString d = s.toUpper().normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(d.size());
    for (const QChar &c : d)
        if (c.category() != QChar::Mark_NonSpacing)     // retire les accents décomposés
            out.append(c);
    return out.trimmed();
}

// Distance de Jaro (0..1) : proportion de caractères communs, bien placés.
double jaro(const QString &s1, const QString &s2)
{
    if (s1.isEmpty() && s2.isEmpty()) return 1.0;
    if (s1.isEmpty() || s2.isEmpty()) return 0.0;
    const int len1 = s1.size(), len2 = s2.size();
    int fenetre = qMax(len1, len2) / 2 - 1;
    if (fenetre < 0) fenetre = 0;
    QVector<bool> m1(len1, false), m2(len2, false);
    int matches = 0;
    for (int i = 0; i < len1; ++i) {
        const int deb = qMax(0, i - fenetre);
        const int fin = qMin(i + fenetre + 1, len2);
        for (int j = deb; j < fin; ++j) {
            if (m2[j] || s1.at(i) != s2.at(j)) continue;
            m1[i] = m2[j] = true; ++matches; break;
        }
    }
    if (matches == 0) return 0.0;
    double transpositions = 0; int k = 0;
    for (int i = 0; i < len1; ++i) {
        if (!m1[i]) continue;
        while (!m2[k]) ++k;
        if (s1.at(i) != s2.at(k)) transpositions += 0.5;
        ++k;
    }
    const double m = matches;
    return (m / len1 + m / len2 + (m - transpositions) / m) / 3.0;
}

// Jaro-Winkler : Jaro + bonus de préfixe commun (idéal pour les noms : schmit/schmidt ~0,95).
double jaroWinkler(const QString &a, const QString &b)
{
    const double j = jaro(a, b);
    int prefixe = 0;
    const int maxp = qMin(4, int(qMin(a.size(), b.size())));
    for (int i = 0; i < maxp && a.at(i) == b.at(i); ++i) ++prefixe;
    return j + prefixe * 0.1 * (1.0 - j);
}

// Similarité de noms tenant compte des noms composés (Dumont / Dumont D'Urville) : max entre la
// similarité globale et une similarité par jetons (chaque jeton du plus court retrouvé dans l'autre).
double simNoms(const QString &a, const QString &b)
{
    const QString A = normalise(a), B = normalise(b);
    const double global = jaroWinkler(A, B);
    static const QRegularExpression sep("[\\s'\\-]+");
    const QStringList ta = A.split(sep, Qt::SkipEmptyParts);
    const QStringList tb = B.split(sep, Qt::SkipEmptyParts);
    if (ta.isEmpty() || tb.isEmpty()) return global;
    const QStringList &court  = (ta.size() <= tb.size()) ? ta : tb;
    const QStringList &longue = (ta.size() <= tb.size()) ? tb : ta;
    double somme = 0;
    for (const QString &t : court) {
        double best = 0;
        for (const QString &u : longue) best = qMax(best, jaroWinkler(t, u));
        somme += best;
    }
    return qMax(global, somme / court.size());
}

// Score de rapprochement (0..1) entre la personne lue sur la CV et un dossier candidat.
// IMPORTANT : la DDN a DÉJÀ servi de filtre (tous les candidats la partagent, ou ont le même NNI) ;
// on ne la RECOMPTE PAS dans le score, sinon tout le monde démarrerait artificiellement gonflé (un
// simple hasard de même date suffirait à faire passer un inconnu à l'orange). Le score = ressemblance
// nom + prénom, pondérée selon le sexe (cf. ci-dessous).
double scoreCandidat(const LecteurVitale::Porteur &cv, const QDate &ddnCV, Patient *pat, bool france)
{
    const double simPrenom = jaroWinkler(normalise(cv.prenom), normalise(pat->prenom()));
    const double simNom    = simNoms(cv.nom, pat->nom());
    const QString sexeCV   = sexeDepuisNIR(cv.nir);        // "M"/"F"/"" d'après le 1er chiffre du NIR
    const bool   memeDDN   = (pat->datedenaissance() == ddnCV);

    // Poids nom/prénom selon le SEXE : une FEMME change souvent de nom (mariage) -> le nom pèse peu et
    // le prénom (stable) domine, pour rester tolérant à un nom différent ; un HOMME (ou sexe inconnu,
    // par prudence) -> le nom pèse davantage, car un nom différent y est nettement plus suspect.
    const double wPrenom = (sexeCV == "F") ? 0.70 : 0.45;
    const double wNom    = (sexeCV == "F") ? 0.30 : 0.55;
    const double simIdentite = wPrenom * simPrenom + wNom * simNom;

    // Roue de secours (France seulement) : un NNI identique vaut quasi-certitude — SAUF si NI la date
    // NI le nom/prénom ne corroborent (dossier manifestement incohérent, ou NNI erroné) : on refuse
    // alors le faux positif. Un vrai NNI est corroboré soit par la DDN, soit par une identité proche
    // (cas « DDN mal saisie mais bonne personne », qui est justement le rôle de la roue de secours).
    // Testé AVANT la pénalité de sexe : un NNI qui colle prime (un sexe erroné dans le dossier sera
    // d'ailleurs proposé à la correction ensuite).
    if (france && !cv.nir.isEmpty() && pat->NNI() > 0)
        {
        bool num = false;
        const qlonglong nni = cv.nir.toLongLong(&num);
        if (num && nni == pat->NNI() && (memeDDN || simIdentite >= 0.60))
            return 1.0;
        }

    // Sexe DIFFÉRENT : une erreur de sexe est rarissime, donc c'est un fort signal « pas la même
    // personne » (typiquement des jumeaux : même nom, même date, sexe opposé) -> on minore de moitié.
    // Uniquement si le sexe est connu des deux côtés (les ayants droit, sans NIR, ne sont pas pénalisés).
    if (!sexeCV.isEmpty() && !pat->sexe().isEmpty() && pat->sexe().toUpper() != sexeCV)
        return simIdentite * 0.5;

    return simIdentite;
}

// Pastille de confiance : vert (≥ 0,85), orange (≥ 0,65), gris en dessous.
QIcon pastille(double score)
{
    const QColor c = score >= 0.85 ? QColor( 40, 180,  70)
                   : score >= 0.65 ? QColor(240, 170,  40)
                                   : QColor(170, 170, 170);
    QPixmap px(14, 14);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawEllipse(2, 2, 10, 10);
    p.end();
    return QIcon(px);
}

// Ajoute une ligne (Nom Prénom | Naissance). On accroche à la 1ère cellule l'idPat du dossier via
// setListids() — le dossier complet est chargé à la demande (survol, action) pour ne pas garder de
// pointeur susceptible d'être détruit ; icone = pastille de confiance (vide pour porteur/ayants droit).
void ajouteLigne(QStandardItemModel *model, const QString &nomprenom, const QString &ddn,
                 int idPat = 0, const QIcon &icone = QIcon())
{
    UpStandardItem *c0 = new UpStandardItem(icone, nomprenom);
    if (idPat > 0)
        c0->setListids(QList<int>{ idPat });
    UpStandardItem *c1 = new UpStandardItem(ddn);
    c0->setEditable(false);
    c1->setEditable(false);
    model->appendRow(QList<QStandardItem*>() << c0 << c1);
}

// Applique à une UpTableView le look de PatientsListeTableView (largeurs 230/122, lignes alternées,
// sélection par ligne, viewport blanc), SANS en-têtes, et fige la hauteur pour lignesVisibles lignes.
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
    tbl->horizontalHeader()->setVisible(false);     // pas d'en-têtes de colonnes
    tbl->verticalHeader()->setVisible(false);

    const int h = int(QFontMetrics(qApp->font()).height() * 1.3);
    tbl->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tbl->verticalHeader()->setDefaultSectionSize(h);
    tbl->setColumnWidth(0, 230);
    tbl->setColumnWidth(1, 122);

    // Viewport blanc opaque (comme la liste des patients), les lignes alternées se peignant par-dessus.
    tbl->viewport()->setAutoFillBackground(true);
    QPalette pal = tbl->viewport()->palette();
    pal.setColor(tbl->viewport()->backgroundRole(), Qt::white);
    tbl->viewport()->setPalette(pal);

    tbl->setFixedHeight(h * lignesVisibles + 4);
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

    // Tout le contenu va dans un conteneur inséré AU-DESSUS de la barre de boutons (que le
    // constructeur d'UpDialog place en haut du layout).
    QVBoxLayout *corps = new QVBoxLayout;

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
    modPorteur->setColumnCount(2);
    ajouteLigne(modPorteur, nomPrenom(assure.nom, assure.prenom), ddnAffichee(assure.dateNaissance));
    m_tblPorteur = new UpTableView();
    m_tblPorteur->setModel(modPorteur);
    styleTableListe(m_tblPorteur, 1, false);
    droite->addWidget(m_tblPorteur);

    // Table des ayants droit, séparée par un petit espace
    droite->addSpacing(8);
    QStandardItemModel *modAyants = new QStandardItemModel(this);
    modAyants->setColumnCount(2);
    for (int i = 1; i < m_porteurs.size(); ++i)
        ajouteLigne(modAyants, nomPrenom(m_porteurs.at(i).nom, m_porteurs.at(i).prenom), ddnAffichee(m_porteurs.at(i).dateNaissance));
    m_tblAyants = new UpTableView();
    m_tblAyants->setModel(modAyants);
    styleTableListe(m_tblAyants, qMax(1, m_porteurs.size() - 1), false);
    droite->addWidget(m_tblAyants);
    droite->addStretch();

    entete->addLayout(droite);
    corps->addLayout(entete);

    // ---- Correspondances : 3ème table, décalée à gauche (pleine largeur) ----
    corps->addSpacing(10);
    UpLabel *lblCorresp = new UpLabel();
    lblCorresp->setText(tr("Dossiers correspondants"));
    corps->addWidget(lblCorresp, 0, Qt::AlignLeft);
    QStandardItemModel *modCorresp = new QStandardItemModel(this);
    modCorresp->setColumnCount(2);
    m_tblCorresp = new UpTableView();
    m_tblCorresp->setModel(modCorresp);
    styleTableListe(m_tblCorresp, 6, true);              // ~6 lignes visibles, ascenseur si besoin

    // Table + colonne de droite (bouton de recherche manuelle « archives » puis légende des pastilles).
    QHBoxLayout *ligneCorresp = new QHBoxLayout;
    ligneCorresp->addWidget(m_tblCorresp, 0, Qt::AlignTop);

    QVBoxLayout *colDroite = new QVBoxLayout;
    UpPushButton *btnRecherche = new UpPushButton(tr("Rechercher dans la base"));
    btnRecherche->setIcon(Icons::icArchive());
    btnRecherche->setToolTip(tr("Rechercher manuellement un dossier"));
    connect(btnRecherche, &QPushButton::clicked, this, &FicheVitale::rechercheManuelle);
    colDroite->addWidget(btnRecherche);

    // Légende des pastilles : mêmes couleurs que pastille() (vert / orange / gris).
    colDroite->addSpacing(10);
    UpLabel *legende = new UpLabel();
    legende->setTextFormat(Qt::RichText);
    legende->setText(tr(
        "<span style=\"color:#28B446;font-size:16px\">&#9679;</span>&nbsp; correspondance forte<br>"
        "<span style=\"color:#F0AA28;font-size:16px\">&#9679;</span>&nbsp; correspondance moyenne<br>"
        "<span style=\"color:#AAAAAA;font-size:16px\">&#9679;</span>&nbsp; correspondance faible"));
    colDroite->addWidget(legende);
    colDroite->addStretch();

    ligneCorresp->addLayout(colDroite);
    ligneCorresp->addStretch();
    corps->addLayout(ligneCorresp);

    dlglayout()->insertLayout(0, corps);                 // AU-DESSUS de la barre de boutons

    // ---- 4 boutons en bas : Ouvrir (soignant seulement) / Salle d'attente / Créer / Annuler ----
    User *u = Datas::I()->users->userconnected();
    if (u != nullptr && u->isSoignant())
        {
        UpPushButton *btnOuvrir = new UpPushButton(tr("Ouvrir le dossier"));
        btnOuvrir->setIcon(Icons::icSortirDossier());
        AjouteWidgetLayButtons(btnOuvrir);
        connect(btnOuvrir, &QPushButton::clicked, this, [this] {
            const int id = idCorrespChoisi();
            if (id > 0) { m_action = Ouvrir; m_idDossierActive = id; accept(); } });
        }
    UpPushButton *btnSalle = new UpPushButton(tr("Inscrire en salle d'attente"));
    btnSalle->setIcon(Icons::icAttente());
    AjouteWidgetLayButtons(btnSalle);
    connect(btnSalle, &QPushButton::clicked, this, [this] {
        const int id = idCorrespChoisi();
        if (id > 0) { m_action = SalleAttente; m_idDossierActive = id; accept(); } });

    UpPushButton *btnCreer = new UpPushButton(tr("Créer le dossier"));
    btnCreer->setIcon(Icons::icCreer());
    AjouteWidgetLayButtons(btnCreer);
    connect(btnCreer, &QPushButton::clicked, this, [this] { m_action = Creer; accept(); });

    UpPushButton *btnAnnuler = new UpPushButton(tr("Annuler"));
    btnAnnuler->setIcon(Icons::icAnnuler());
    AjouteWidgetLayButtons(btnAnnuler);
    connect(btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);

    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    // ---- Surbrillance partagée + navigation clavier + info-bulle ----
    connect(m_tblPorteur->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FicheVitale::selectionPorteurChangee);
    connect(m_tblAyants->selectionModel(),  &QItemSelectionModel::selectionChanged, this, &FicheVitale::selectionAyantChangee);
    connect(m_tblCorresp, &QTableView::entered, this, &FicheVitale::afficheBulleCorresp);
    m_tblCorresp->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tblCorresp, &QTableView::doubleClicked,              this, &FicheVitale::activerDepuisCorresp);
    connect(m_tblCorresp, &QTableView::customContextMenuRequested, this, &FicheVitale::menuCorresp);
    m_tblPorteur->installEventFilter(this);
    m_tblAyants->installEventFilter(this);

    // À l'ouverture : le porteur est en surbrillance (ce qui déclenche la 1ère recherche).
    selectionnePorteur();
}

// La fiche possède les candidats chargés (loadPatientsCV) et les libère à sa fermeture.
FicheVitale::~FicheVitale()
{
    for (const QList<Patient*> &liste : m_cacheParDDN.values())
        qDeleteAll(liste);
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
// Info-bulle du dossier survolé dans la table des correspondances : on retrouve l'idPat porté par la
// cellule, on charge le dossier complet à la demande et on réutilise Patient::texteInfoBulle (comme
// la liste des patients). On ne charge le détail qu'au survol — les candidats ne portent que l'identité.
//-----------------------------------------------------------------------------------------------------
// idPat accroché à une ligne de la table des correspondances, ou 0.
int FicheVitale::idCorresp(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(m_tblCorresp->model());
    if (model == nullptr)
        return 0;
    UpStandardItem *it = static_cast<UpStandardItem*>(model->item(index.row(), 0));
    return it ? it->listids().value(0) : 0;
}

void FicheVitale::afficheBulleCorresp(const QModelIndex &index)
{
    const int id = idCorresp(index);
    if (id <= 0)
        return;
    Patient *pat = Datas::I()->patients->getById(id, Item::LoadDetails);
    if (pat != nullptr)
        QToolTip::showText(QCursor::pos(), pat->texteInfoBulle(QDate::currentDate()));
}

// Double-clic : action par défaut selon le rôle (soignant → ouvrir ; sinon → salle d'attente).
void FicheVitale::activerDepuisCorresp(const QModelIndex &index)
{
    const int id = idCorresp(index);
    if (id <= 0)
        return;
    User *u = Datas::I()->users->userconnected();
    m_idDossierActive = id;
    m_action = (u != nullptr && u->isSoignant()) ? Ouvrir : SalleAttente;
    accept();
}

// Clic droit : le soignant a 2 choix (ouvrir / salle d'attente), le non-soignant seulement salle d'attente.
void FicheVitale::menuCorresp(const QPoint &pos)
{
    const int id = idCorresp(m_tblCorresp->indexAt(pos));
    if (id <= 0)
        return;
    User *u = Datas::I()->users->userconnected();
    const bool soignant = (u != nullptr && u->isSoignant());
    QMenu menu;
    QAction *actOuvrir = soignant ? menu.addAction(tr("Ouvrir le dossier")) : nullptr;
    menu.addAction(tr("Inscrire en salle d'attente"));
    QAction *choisi = menu.exec(m_tblCorresp->viewport()->mapToGlobal(pos));
    if (choisi == nullptr)
        return;
    m_idDossierActive = id;
    m_action = (choisi == actOuvrir) ? Ouvrir : SalleAttente;
    accept();
}

// Correspondance sur laquelle agissent les boutons Ouvrir / Salle d'attente : la ligne sélectionnée,
// ou, à défaut de sélection, la première (la plus probable).
int FicheVitale::idCorrespChoisi() const
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(m_tblCorresp->model());
    if (model == nullptr || model->rowCount() == 0)
        return 0;
    const QModelIndexList sel = m_tblCorresp->selectionModel()->selectedRows();
    const int row = sel.isEmpty() ? 0 : sel.first().row();
    UpStandardItem *it = static_cast<UpStandardItem*>(model->item(row, 0));
    return it ? it->listids().value(0) : 0;
}

//-----------------------------------------------------------------------------------------------------
// Recherche manuelle d'un dossier (bouton « archives »), pour rattraper un dossier que le
// rapprochement automatique n'aurait pas fait remonter. Comportement à définir.
//-----------------------------------------------------------------------------------------------------
void FicheVitale::rechercheManuelle()
{
    // Fiche de recherche, pré-remplie avec le porteur courant.
    RechercheDossier dlg(m_porteurCourant.nom, m_porteurCourant.prenom,
                         QDate::fromString(m_porteurCourant.dateNaissance, "dd/MM/yyyy"), this);
    if (dlg.exec() == QDialog::Accepted && dlg.idChoisi() > 0)
        {
        // Le dossier trouvé manuellement est activé comme une correspondance, avec l'action choisie.
        m_idDossierActive = dlg.idChoisi();
        m_action = dlg.ouvrir() ? Ouvrir : SalleAttente;
        accept();
        }
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
    m_porteurCourant = porteur;                       // mémorisé pour la recherche manuelle
    model->removeRows(0, model->rowCount());          // vide les lignes

    const QDate ddnCV = QDate::fromString(porteur.dateNaissance, "dd/MM/yyyy");
    if (!ddnCV.isValid())
        return;

    const bool france = m_db->parametres()->cotationsfrance();

    // NNI : roue de secours France seulement — beaucoup de dossiers saisis à la main n'en ont pas.
    qlonglong nni = 0;
    if (france && !porteur.nir.isEmpty())
        {
        bool num = false;
        const qlonglong v = porteur.nir.toLongLong(&num);
        if (num && v > 0)
            nni = v;
        }

    // 1) Filet large chargé UNE FOIS par (DDN, NNI) et gardé en mémoire : rebasculer d'un porteur à
    //    l'autre ne réinterroge pas le serveur (accès internet lents). Requête légère (identité + NNI).
    const QString cle = ddnCV.toString("yyyy-MM-dd") + "/" + QString::number(nni);
    if (!m_cacheParDDN.contains(cle))
        m_cacheParDDN.insert(cle, m_db->loadPatientsCV(ddnCV, nni));
    const QList<Patient*> &candidats = m_cacheParDDN.value(cle);

    // 2) Score de chaque candidat, puis tri par probabilité décroissante.
    QList<QPair<double, Patient*>> classes;
    for (Patient *pat : candidats)
        classes.append(qMakePair(scoreCandidat(porteur, ddnCV, pat, france), pat));
    std::sort(classes.begin(), classes.end(),
              [](const QPair<double, Patient*> &a, const QPair<double, Patient*> &b) { return a.first > b.first; });

    // 3) Remplissage : le plus probable en haut, avec sa pastille de confiance.
    for (const QPair<double, Patient*> &c : classes)
        {
        Patient *pat = c.second;
        const QString ddnAff = pat->datedenaissance().isValid()
                                   ? pat->datedenaissance().toString("dd-MM-yyyy")
                                   : QString();
        ajouteLigne(model, nomPrenom(pat->nom(), pat->prenom()), ddnAff, pat->id(), pastille(c.first));
        }
}
