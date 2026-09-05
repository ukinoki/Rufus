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

#include <QGroupBox>
#include <QHeaderView>
#include <QRadioButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTimer>

#define LARGEURMINI_DESCRIPTIF  250     /*!< en deçà, le descriptif n'est plus lisible */

#include "gbl_datas.h"
#include "dlg_choixcotation.h"
#include "dlg_gestioncotations.h"
#include "database.h"
#include "upmessagebox.h"
#include "upstandarditem.h"

bool FiltreCotations::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex idxcot = sourceModel()->index(source_row, ColCotation, source_parent);
    if (m_ophtalmoseule && !idxcot.data(RoleOphtalmo).toBool())
        return false;
    if (m_typeseul && idxcot.data(RoleTypcotation).toInt() != m_typeseul)
        return false;
    const QRegularExpression rgx = filterRegularExpression();
    if (rgx.pattern().isEmpty())
        return true;
    for (int col : {ColCotation, ColDescriptif})
        if (sourceModel()->index(source_row, col, source_parent).data().toString().contains(rgx))
            return true;
    return false;
}

dlg_choixcotation::dlg_choixcotation(QWidget *parent) : UpDialog(parent)
{
    setWindowTitle(tr("Rechercher une cotation"));

    wdg_table = new QTableView(this);
    wdg_table   ->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    wdg_table   ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_table   ->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_table   ->setWordWrap(true);
    wdg_table   ->verticalHeader()->setVisible(false);
    wdg_table   ->setMinimumHeight(400);

    m_model = new QStandardItemModel(this);
    RemplitTable();

    m_proxy = new FiltreCotations(this);
    m_proxy     ->setSourceModel(m_model);
    m_proxy     ->setFilterCaseSensitivity(Qt::CaseInsensitive);
    wdg_table   ->setModel(m_proxy);

    wdg_table   ->setColumnWidth(ColCotation, 153);
    for (int col : {ColNonOptam, ColOptam, ColPratique})
        wdg_table   ->setColumnWidth(col, 100);
    /*! largeurs des colonnes, sur le modèle de UpDialog::setSaveGeometry : saveState les porte toutes */
    QSettings sets(PATH_FILE_INI, QSettings::IniFormat);
    const QByteArray etatcolonnes = sets.value(DialogColonnes Nom_fiche_ChoixCotation).toByteArray();
    if (!etatcolonnes.isEmpty())
        wdg_table   ->horizontalHeader()->restoreState(etatcolonnes);
    //! restoreState ne porte pas les modes : le descriptif s'étire, il se repose après
    wdg_table   ->horizontalHeader()->setSectionResizeMode(ColDescriptif, QHeaderView::Stretch);
    connect(this,   &QDialog::finished, this,   [=, this] {
        QSettings sets(PATH_FILE_INI, QSettings::IniFormat);
        sets.setValue(DialogColonnes Nom_fiche_ChoixCotation, wdg_table->horizontalHeader()->saveState());
    });
    /*! après restoreState, qui porte aussi la visibilité des colonnes */
    if (!DataBase::I()->parametres()->cotationsfrance())
    {
        wdg_table   ->setColumnHidden(ColNonOptam, true);
        wdg_table   ->setColumnHidden(ColOptam, true);
    }
    /*! la fiche ne peut pas rétrécir au point de rogner une colonne : seul le descriptif s'étire, sous
     *  réserve de rester lisible */
    auto majlargeurmini = [=, this] {
        int larg = LARGEURMINI_DESCRIPTIF + wdg_table->verticalScrollBar()->sizeHint().width()
                 + 2 * wdg_table->frameWidth();
        for (int col : {ColCotation, ColNonOptam, ColOptam, ColPratique})
            if (!wdg_table->isColumnHidden(col))
                larg += wdg_table->columnWidth(col);
        wdg_table   ->setMinimumWidth(larg);
    };
    /*! resizeRowsToContents mesure les 8000 lignes : à chaque pixel du redimensionnement, la fiche
     *  se fige. On attend l'arrêt du mouvement. */
    m_timerhauteurs = new QTimer(this);
    m_timerhauteurs ->setSingleShot(true);
    m_timerhauteurs ->setInterval(120);
    connect(m_timerhauteurs, &QTimer::timeout, this, [=, this] {wdg_table->resizeRowsToContents();});
    connect(wdg_table->horizontalHeader(), &QHeaderView::sectionResized, this, [=, this] (int col) {
        if (col == ColDescriptif)
            m_timerhauteurs ->start();      //! le texte se replie autrement, la hauteur suit
        else
            majlargeurmini();
    });
    majlargeurmini();

    wdg_buttonframe = new WidgetButtonFrame(wdg_table);
    wdg_buttonframe ->AddButtons(WidgetButtonFrame::Plus | WidgetButtonFrame::Modifier | WidgetButtonFrame::Moins);
    wdg_buttonframe ->addSearchLine();
    wdg_buttonframe ->layButtons()->setSpacing(10);   //! AddButtons colle les widgets les uns aux autres
    /*! le tri n'a de sens qu'avec la CCAM, absente de la version internationale */
    const bool ccamutilisee = DataBase::I()->parametres()->cotationsfrance();
    m_proxy         ->setOphtalmoSeule(ccamutilisee);
    if (ccamutilisee)
    {
        wdg_ophtalmo = new UpCheckBox(tr("Uniquement l'ophtalmologie"));
        wdg_ophtalmo    ->setChecked(true);
        wdg_buttonframe ->layButtons()->insertWidget(2, wdg_ophtalmo);
    }
    /*! AddButtons fige la largeur du conteneur, ce que veut une liste à largeur fixe : ici la table suit
     *  celle de la fiche */
    wdg_buttonframe ->widgButtonParent()->setMinimumWidth(0);
    wdg_buttonframe ->widgButtonParent()->setMaximumWidth(QWIDGETSIZE_MAX);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout() ->insertWidget(0, wdg_buttonframe->widgButtonParent());

    /*! filtre par type sous la table : sa largeur devient le minimum de la fiche, il reste visible */
    if (ccamutilisee)
    {
        const QList<QPair<int, QString>> types = {{0, tr("Toutes les cotations")}, {1, tr("CCAM")},
                                                 {3, tr("NGAP")}, {2, tr("Associations CCAM")},
                                                 {4, tr("Hors convention")}};
        QGroupBox *gbtypes    = new QGroupBox(this);
        QHBoxLayout *laytypes = new QHBoxLayout(gbtypes);
        for (const auto &typ : types)
        {
            QRadioButton *radio = new QRadioButton(typ.second);
            radio   ->setChecked(typ.first == 0);
            connect(radio, &QRadioButton::toggled, this, [=, this] (bool coche) {
                if (!coche)
                    return;
                m_proxy     ->setTypeSeul(typ.first);
                wdg_table   ->resizeRowsToContents();
                RegleOKButton();
            });
            laytypes->addWidget(radio);
        }
        laytypes    ->addStretch(1);
        dlglayout() ->insertWidget(1, gbtypes);
    }

    OKButton    ->setEnabled(false);
    connect(wdg_buttonframe->searchline(),  &QLineEdit::textEdited,     this,   [=, this] (QString txt) {
        m_proxy     ->setFilterFixedString(txt);
        wdg_table   ->resizeRowsToContents();
        RegleOKButton();
    });
    connect(wdg_table->selectionModel(),    &QItemSelectionModel::selectionChanged, this,   [=, this] {
        RegleOKButton();
        RegleCotationsBoutons();
    });
    connect(wdg_buttonframe,                &WidgetButtonFrame::choix,  this,   &dlg_choixcotation::ChoixButtonFrame);
    RegleCotationsBoutons();
    if (wdg_ophtalmo)
        connect(wdg_ophtalmo,               &QCheckBox::toggled,        this,   [=, this] (bool coche) {
            m_proxy     ->setOphtalmoSeule(coche);
            wdg_table   ->resizeRowsToContents();
            RegleOKButton();
        });
    /*! itemChanged part aussi sur un changement programmatique : on n'agit sur la case que si elle a
     *  réellement basculé, c.-à-d. si son état diffère de celui enregistré */
    connect(m_model, &QStandardItemModel::itemChanged, this, [=, this] (QStandardItem *itm) {
        UpStandardItem *upit = dynamic_cast<UpStandardItem*>(itm);
        if (upit == nullptr)
            return;
        Cotation *cot = qobject_cast<Cotation*>(upit->rufusitem());
        if (itm->column() == ColPratique)
        {
            const double montant = QLocale().toDouble(itm->text());
            MAJMontantPratique(cot, montant);
            m_model ->blockSignals(true);
            itm     ->setText(QLocale().toString(montant, 'f', 2));
            m_model ->blockSignals(false);
        }
        else if (itm->column() == ColCotation
                 && cot != nullptr && (itm->checkState() == Qt::Checked) != cot->isused())
            MAJCotation(itm);
    });
    connect(wdg_table,                      &QAbstractItemView::doubleClicked,      this,   [=, this] (QModelIndex idx) {
        if (idx.column() != ColPratique)    //! là, le double-clic ouvre la saisie du montant
            RetientCotation(idx);
    });
    connect(OKButton,                       &QPushButton::clicked,      this,   [=, this] {
        RetientCotation(wdg_table->currentIndex());
        accept();                       //! OK ferme aussi quand il n'ouvre que sur des modifications
    });

    wdg_buttonframe->searchline()->setFocus();
    setEnregPosition(true);
    setSaveGeometry(Nom_fiche_ChoixCotation);
}

void dlg_choixcotation::showEvent(QShowEvent *event)
{
    UpDialog::showEvent(event);
    //! la colonne étirée n'a sa largeur qu'une fois le layout appliqué, donc après ce show
    QTimer::singleShot(0, this, [=, this] {wdg_table->resizeRowsToContents();});
}

/*!
 * \brief dlg_choixcotation::RemplitTable
 * Une ligne par cotation, la cotation elle-même portée par la première colonne. Le montant pratiqué ne
 * vaut que pour celles que l'utilisateur pratique ; les codes seulement présents dans la CCAM n'en ont pas.
 */
void dlg_choixcotation::RemplitTable()
{
    /*! clear réinitialise le modèle : la vue perdrait largeurs et étirement du descriptif */
    const bool vueprete = wdg_table->model() != nullptr;
    const QByteArray etatcolonnes = vueprete? wdg_table->horizontalHeader()->saveState() : QByteArray();
    m_model     ->clear();
    m_model     ->setHorizontalHeaderLabels(QStringList()
                    << tr("Cotation") << tr("Descriptif") << tr("Non OPTAM") << tr("OPTAM") << tr("Pratiqué"));

    //! un remplaçant ne modifie pas les montants de celui qu'il remplace
    const bool sonparent = Datas::I()->users->userconnected()->idparent()
                        == Datas::I()->users->userconnected()->id();
    auto ajoute = [&] (Cotation *c, bool ophtalmo) {
        QList<QStandardItem*> ligne;
        ligne << new UpStandardItem(c->typeacte(), c);
        //! cochée : la cotation est dans celles de l'utilisateur courant
        ligne.at(ColCotation)->setCheckable(true);
        ligne.at(ColCotation)->setCheckState(c->isused()? Qt::Checked : Qt::Unchecked);
        if (!sonparent)     //! un remplaçant lit les habitudes de son parent, il ne les change pas
        {
            Qt::ItemFlags drapeaux = ligne.at(ColCotation)->flags();
            drapeaux.setFlag(Qt::ItemIsUserCheckable, false);
            ligne.at(ColCotation)->setFlags(drapeaux);
        }
        ligne.at(ColCotation)->setData(ophtalmo, RoleOphtalmo);
        ligne.at(ColCotation)->setData(c->typcotation(), RoleTypcotation);
        ligne << new QStandardItem(c->descriptif());
        ligne << new QStandardItem(QLocale().toString(c->montantnonoptam(), 'f', 2));
        ligne << new QStandardItem(QLocale().toString(c->montantoptam(), 'f', 2));
        //! le pratiqué d'un NGAP vaut toujours le conventionnel : il ne se saisit pas
        ligne << new UpStandardItem(c->isused() && !c->isNGAP()?
                                    QLocale().toString(c->montantpratique(), 'f', 2) : QString(), c);
        for (int col : {ColNonOptam, ColOptam, ColPratique})
            ligne.at(col)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ligne.at(ColCotation)->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
        for (QStandardItem *itm : ligne)
            itm ->setEditable(false);
        ligne.at(ColPratique)->setEditable(!c->isNGAP() && sonparent && c->isused());
        m_model ->appendRow(ligne);
    };

    QStringList codesvus;
    for (Cotation *c : *Datas::I()->cotations->cotations())
    {
        ajoute(c, true);                    //! les cotations du cabinet sont toutes retenues
        codesvus << c->typeacte();
    }
    Datas::I()->cotations->initListeCCAM();
    for (Cotation *c : *Datas::I()->cotations->ccam())
        if (!codesvus.contains(c->typeacte()))
            ajoute(c, c->typeacte().startsWith("B", Qt::CaseInsensitive)
                   || c->typeacte().startsWith("EBQF", Qt::CaseInsensitive));   //! les chapitres d'ophtalmologie de la CCAM
    if (vueprete)
    {
        wdg_table   ->horizontalHeader()->restoreState(etatcolonnes);
        wdg_table   ->horizontalHeader()->setSectionResizeMode(ColDescriptif, QHeaderView::Stretch);
    }
}

/*!
 * \brief dlg_choixcotation::MAJCotation
 * Coche/décoche : ajoute ou retire la jointure du user, et propose de supprimer de la table des
 * cotations celle que plus personne n'utilise. Repris de dlg_param::MAJCotation.
 * \param itcheck  l'item dont la case vient de basculer
 */
void dlg_choixcotation::MAJCotation(QStandardItem *itcheck)
{
    UpStandardItem *upit = dynamic_cast<UpStandardItem*>(itcheck);
    if (upit == nullptr)
        return;
    Cotation *cot = qobject_cast<Cotation*>(upit->rufusitem());
    if (cot == nullptr)
        return;

    const int typ       = cot->typcotation();
    const int iduser    = Datas::I()->users->userconnected()->id();
    const bool checked  = (itcheck->checkState() == Qt::Checked);
    QStandardItem *itprat = m_model->item(itcheck->row(), ColPratique);
    cot     ->setused(checked);
    m_cotationsmodifiees = true;
    RegleOKButton();

    /*! l'éditeur s'ouvre au tour suivant : appelé depuis le clic, celui-ci le refermerait aussitôt */
    auto ouvreEditionPratique = [&] (double v) {
        if (itprat == nullptr)
            return;
        m_model ->blockSignals(true);
        itprat  ->setText(QLocale().toString(v, 'f', 2));
        itprat  ->setEditable(true);
        m_model ->blockSignals(false);
        cot     ->setmontantpratique(v);
        const int row = itcheck->row();
        QTimer::singleShot(0, this, [this, row] {
            wdg_table->edit(m_proxy->mapFromSource(m_model->index(row, ColPratique)));});
    };

    if (!checked)
    {
        DataBase::I()->retireJointureCotation(typ, cot->id(), iduser);
        if (itprat != nullptr)
        {
            m_model ->blockSignals(true);
            itprat  ->setText("");
            itprat  ->setEditable(false);
            m_model ->blockSignals(false);
        }
        /*! le rechargement est différé : on ne détruit pas le modèle pendant qu'on traite son itemChanged */
        if (!cot->isNGAP() && !DataBase::I()->cotationUtiliseeParAutreUser(typ, cot->id(), iduser)
            && UpMessageBox::Question(this, tr("Cotation inutilisée"),
                   tr("Cette cotation n'est plus utilisée par personne.") + "\n"
                   + tr("Voulez-vous la supprimer de la liste des cotations ?"),
                   UpDialog::ButtonCancel | UpDialog::ButtonOK, QStringList() << tr("Non") << tr("Oui"))
               == UpSmallButton::STARTBUTTON)
        {
            DataBase::I()->SupprRecordFromTable(cot->id(), CP_ID_COTATIONS, TBL_COTATIONS);
            QTimer::singleShot(0, this, [=, this] {Datas::I()->cotations->initListe(); RemplitTable();});
        }
        return;
    }

    if (cot->isNGAP())                              //! jointure sans montant, pas d'édition du pratiqué
    {
        DataBase::I()->ajouteJointureCotation(typ, cot->id(), iduser, 0);
        return;
    }
    const bool optam = Datas::I()->users->userconnected()->isOPTAM();
    const double conv = cot->isAutre()? cot->montantoptam()
                                      : (optam? cot->montantoptam() : cot->montantnonoptam());
    DataBase::I()->ajouteJointureCotation(typ, cot->id(), iduser, conv, conv);
    ouvreEditionPratique(conv);
}

void dlg_choixcotation::MAJMontantPratique(Cotation *cot, double montant)
{
    if (cot == nullptr)
        return;
    DataBase::I()->majMontantPratiqueCotation(cot->typcotation(), cot->id(),
                                              Datas::I()->users->userconnected()->id(), montant);
    cot     ->setmontantpratique(montant);
    m_cotationsmodifiees = true;
    RegleOKButton();
}

void dlg_choixcotation::RetientCotation(QModelIndex idx)
{
    if (!idx.isValid())
        return;
    const QModelIndex source = m_proxy->mapToSource(idx.siblingAtColumn(ColCotation));
    UpStandardItem *item = dynamic_cast<UpStandardItem*>(m_model->itemFromIndex(source));
    if (item == nullptr || !item->hasrufusitem())
        return;
    m_cotation = qobject_cast<Cotation*>(item->rufusitem());
    accept();
}

void dlg_choixcotation::RegleOKButton()
{
    OKButton    ->setEnabled(wdg_table->selectionModel()->hasSelection() || m_cotationsmodifiees);
}

Cotation* dlg_choixcotation::cotationEnCours()
{
    const QModelIndex idx = wdg_table->currentIndex();
    if (!idx.isValid())
        return nullptr;
    UpStandardItem *itm = dynamic_cast<UpStandardItem*>(
                m_model->itemFromIndex(m_proxy->mapToSource(idx.siblingAtColumn(ColCotation))));
    return (itm && itm->hasrufusitem())? qobject_cast<Cotation*>(itm->rufusitem()) : nullptr;
}

/*!
 * \brief dlg_choixcotation::RegleCotationsBoutons
 * Repris de dlg_param : tout est réservé à l'utilisateur qui est son propre parent, la modification aux
 * seules cotations « autre », la suppression à celles que personne d'autre n'utilise.
 */
void dlg_choixcotation::RegleCotationsBoutons()
{
    const bool sonparent = Datas::I()->users->userconnected()->idparent()
                        == Datas::I()->users->userconnected()->id();
    Cotation *cot = cotationEnCours();
    wdg_buttonframe->wdg_plusBouton ->setEnabled(sonparent);
    const bool modifiable = cot != nullptr && (cot->isAutre() || cot->isAssocCCAM());
    wdg_buttonframe->wdg_modifBouton->setEnabled(sonparent && modifiable);
    wdg_buttonframe->wdg_moinsBouton->setEnabled(sonparent && modifiable
                                                 && !DataBase::I()->cotationUtiliseeParAutreUser(
                                                        cot->typcotation(), cot->id(),
                                                        Datas::I()->users->userconnected()->id()));
}

void dlg_choixcotation::ChoixButtonFrame()
{
    switch (wdg_buttonframe->Choix()) {
    case WidgetButtonFrame::Plus:
        NouvCotation();
        break;
    case WidgetButtonFrame::Modifier:
        ModifCotation();
        break;
    case WidgetButtonFrame::Moins:
        SupprimeCotation(cotationEnCours());
        break;
    }
}

void dlg_choixcotation::NouvCotation()
{
    dlg_gestioncotations dlg(dlg_gestioncotations::Creation, "", this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    Datas::I()->cotations->initListe();
    RemplitTable();
    m_cotationsmodifiees = true;
    RegleOKButton();
}

void dlg_choixcotation::ModifCotation()
{
    Cotation *cot = cotationEnCours();
    if (cot == nullptr)
        return;
    dlg_gestioncotations dlg(dlg_gestioncotations::Modification, cot->typeacte(), this);
    if (dlg.exec() <= 0)
        return;
    Datas::I()->cotations->initListe();
    RemplitTable();
    m_cotationsmodifiees = true;
    RegleOKButton();
}

/*!
 * \brief dlg_choixcotation::SupprimeCotation
 * Retire la cotation de la jointure du user, puis la supprime de la table si personne ne l'utilise plus
 * — sauf une NGAP, qui n'est stockée que là. Repris de dlg_param::supprimeCotation.
 * \param cot  la cotation à retirer
 */
void dlg_choixcotation::SupprimeCotation(Cotation *cot)
{
    if (cot == nullptr)
        return;
    if (UpMessageBox::Question(this, tr("Supprimer une cotation"),
            tr("Voulez-vous vraiment supprimer la cotation %1 ?").arg(cot->typeacte()),
            UpDialog::ButtonCancel | UpDialog::ButtonOK, QStringList() << tr("Annuler") << tr("Supprimer"))
        != UpSmallButton::STARTBUTTON)
        return;
    const int iduser = Datas::I()->users->userconnected()->id();
    DataBase::I()->retireJointureCotation(cot->typcotation(), cot->id(), iduser);
    if (!cot->isNGAP() && !DataBase::I()->cotationUtiliseeParAutreUser(cot->typcotation(), cot->id(), iduser))
        DataBase::I()->SupprRecordFromTable(cot->id(), CP_ID_COTATIONS, TBL_COTATIONS);
    Datas::I()->cotations->initListe();
    RemplitTable();
    m_cotationsmodifiees = true;
    RegleOKButton();
}
