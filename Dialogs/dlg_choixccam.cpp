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

#include <QToolTip>
#include "dlg_choixccam.h"

dlg_choixccam::dlg_choixccam(QWidget *parent) :
    UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowTitle(tr("Actes CCAM"));

    //! --- en-tête : titre « Actes CCAM » + case « uniquement l'ophtalmologie » (MargeTitreCCAMLayout) ---
    UpLabel *titrelabel = new UpLabel();    titrelabel->setText(tr("Actes CCAM"));
    wdg_ophtaseul   = new QCheckBox(tr("uniquement l'ophtalmologie"));
    wdg_ophtaseul   ->setChecked(true);          //! par défaut on ne montre que l'ophtalmologie (codes en B..)
    QWidget *titrewidg = new QWidget();
    QHBoxLayout *titrelay = new QHBoxLayout;
    titrelay        ->insertWidget(0, titrelabel);
    titrelay        ->addStretch();
    titrelay        ->addWidget(wdg_ophtaseul);
    titrelay        ->setContentsMargins(0,0,0,0);
    titrewidg       ->setLayout(titrelay);
    connect(wdg_ophtaseul,  &QCheckBox::clicked, this, [=] (bool b) {filtreOphta(b);});
    dlglayout()     ->addWidget(titrewidg);

    //! --- table des actes CCAM, précédée du petit repère de gauche (horizontalLayout_13) ---
    UpLabel *reperelabel = new UpLabel();        //! équivalent de ShowCCAMlabel : simple marge à gauche
    reperelabel     ->setFixedSize(15,15);
    wdg_table       = new UpTableWidget();
    wdg_table       ->setSelectionMode(QAbstractItemView::SingleSelection);
    wdg_table       ->setSelectionBehavior(QAbstractItemView::SelectRows);
    wdg_table       ->setEditTriggers(QAbstractItemView::NoEditTriggers);
    wdg_table       ->setMouseTracking(true);
    wdg_table       ->setFixedHeight(380);
    QWidget *tablewidg = new QWidget();
    QHBoxLayout *tablelay = new QHBoxLayout;
    tablelay        ->insertWidget(0, reperelabel);
    tablelay        ->insertWidget(1, wdg_table);
    tablelay        ->setContentsMargins(0,0,0,0);
    tablewidg       ->setLayout(tablelay);
    connect(wdg_table, &QTableWidget::itemSelectionChanged, this, [=] {selectionChangee();});
    //! survol souris : infobulle immédiate avec le libellé de l'acte (colonne masquée), comme la table
    //! des cotations de dlg_param (mouseTracking + signal entered)
    connect(wdg_table, &QAbstractItemView::entered, this, [=] (QModelIndex idx) {
        QTableWidgetItem *lib = wdg_table->item(idx.row(), 3);
        if (lib)
            QToolTip::showText(QCursor::pos(), lib->text(), wdg_table, QRect(QCursor::pos(), QSize(10,10)), 2000);
    });
    dlglayout()     ->addWidget(tablewidg);

    //! --- libellé de l'acte sélectionné ---
    wdg_libelle     = new UpTextEdit();
    wdg_libelle     ->setReadOnly(true);
    wdg_libelle     ->setFixedHeight(70);
    dlglayout()     ->addWidget(wdg_libelle);

    int marge = 5;
    dlglayout()     ->setSpacing(marge);
    dlglayout()     ->setContentsMargins(marge, marge, marge, marge);

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    //! loupe + champ de recherche dans la rangée de boutons : vu le nombre d'actes CCAM, on tape le
    //! code et la table saute sur la 1re ligne qui commence par la saisie (complétion sur les codes).
    addSearchLine();
    dlglayout()     ->addWidget(widgetbuttons());
    OKButton        ->setEnabled(false);         //! actif seulement quand une ligne est sélectionnée
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    connect(searchline(),   &QLineEdit::textChanged, this, [=] (QString t) {chercheEtSelectionne(t.trimmed());});
    connect(OKButton,       &QPushButton::clicked, this, [=] {accept();});
    connect(CancelButton,   &QPushButton::clicked, this, [=] {reject();});

    //! seule la ligne de recherche prend le focus (clavier) ; les autres widgets restent cliquables à la
    //! souris mais ne le captent pas. Elle a le focus dès l'ouverture -> on peut taper tout de suite.
    wdg_table   ->setFocusPolicy(Qt::NoFocus);
    wdg_libelle ->setFocusPolicy(Qt::NoFocus);
    wdg_ophtaseul->setFocusPolicy(Qt::NoFocus);
    OKButton    ->setFocusPolicy(Qt::NoFocus);
    CancelButton->setFocusPolicy(Qt::NoFocus);
    searchline()->setFocus();

    remplitTable();
}

/*!
 * \brief dlg_choixccam::remplitTable
 * remplit la table avec TOUS les actes de la table ccam (une seule fois, repris de
 * dlg_param::Remplir_TableActesCCAM, allégé : ni case à cocher ni colonne tarif). Le libellé va dans
 * une colonne masquée, lu à la sélection. Le filtrage ophtalmologie ne rejoue pas la requête : il ne
 * masque/affiche que des rangées (filtreOphta).
 */
void dlg_choixccam::remplitTable()
{
    wdg_table   ->setPalette(QPalette(Qt::white));
    wdg_table   ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wdg_table   ->verticalHeader()->setVisible(false);
    wdg_table   ->setColumnCount(4);
    wdg_table   ->setColumnWidth(0,90);              //! code CCAM
    wdg_table   ->setColumnWidth(1,80);              //! OPTAM
    wdg_table   ->setColumnWidth(2,90);              //! non OPTAM
    wdg_table   ->setColumnHidden(3,true);           //! libellé (affiché dans le UpTextEdit)
    wdg_table   ->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Cotation")));
    wdg_table   ->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("OPTAM")));
    wdg_table   ->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("non OPTAM")));
    wdg_table   ->setHorizontalHeaderItem(3, new QTableWidgetItem(""));
    wdg_table   ->horizontalHeader()->setVisible(true);
    wdg_table   ->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);
    wdg_table   ->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);
    wdg_table   ->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    wdg_table   ->FixLargeurTotale();
    wdg_table   ->horizontalHeader()->setFixedHeight(int(QFontMetrics(qApp->font()).height()*2.3));

    bool ok;
    QString req = "SELECT " CP_NOM_CCAM ", " CP_CODECCAM_CCAM ", " CP_MONTANTOPTAM_CCAM ", " CP_MONTANTNONOPTAM_CCAM
                  " from " TBL_CCAM " order by " CP_CODECCAM_CCAM;
    QList<QVariantList> acteslist = db->StandardSelectSQL(req, ok);
    if (!ok)
        return;
    wdg_table   ->clearContents();
    wdg_table   ->setRowCount(acteslist.size());
    for (int i=0; i<acteslist.size(); i++)
    {
        QTableWidgetItem *pcode     = new QTableWidgetItem(acteslist.at(i).at(1).toString());
        QTableWidgetItem *poptam    = new QTableWidgetItem(QLocale().toString(acteslist.at(i).at(2).toDouble(),'f',2));
        QTableWidgetItem *pnooptam  = new QTableWidgetItem(QLocale().toString(acteslist.at(i).at(3).toDouble(),'f',2));
        QTableWidgetItem *plibelle  = new QTableWidgetItem(acteslist.at(i).at(0).toString());
        poptam      ->setTextAlignment(Qt::AlignRight);
        pnooptam    ->setTextAlignment(Qt::AlignRight);
        wdg_table   ->setItem(i,0,pcode);
        wdg_table   ->setItem(i,1,poptam);
        wdg_table   ->setItem(i,2,pnooptam);
        wdg_table   ->setItem(i,3,plibelle);
        wdg_table   ->setRowHeight(i, int(QFontMetrics(qApp->font()).height()*1.1));
    }
    //! application du filtre ophtalmologie initial (visibilité des rangées + complétion + reset)
    filtreOphta(wdg_ophtaseul->isChecked());
}

/*!
 * \brief dlg_choixccam::filtreOphta
 * masque/affiche les rangées selon la case « ophtalmologie » (visible = code en B..), sans réinterroger
 * la base (tous les actes sont déjà chargés). Reconstruit la complétion sur les seuls codes visibles et
 * remet la table sans sélection.
 * \param ophtaseul  vrai -> ne montre que les actes d'ophtalmologie (codes en B..)
 */
void dlg_choixccam::filtreOphta(bool ophtaseul)
{
    QStringList codesVisibles;
    for (int row=0; row<wdg_table->rowCount(); row++)
    {
        QTableWidgetItem *it = wdg_table->item(row,0);
        const bool ophta = it && it->text().startsWith("B", Qt::CaseInsensitive);
        const bool cache = ophtaseul && !ophta;
        wdg_table   ->setRowHidden(row, cache);
        if (!cache && it)
            codesVisibles << it->text();
    }
    //! complétion de la recherche sur les seuls codes visibles
    if (searchline())
    {
        QCompleter *ancien    = searchline()->completer();
        QCompleter *completer = new QCompleter(codesVisibles, this);
        completer   ->setCaseSensitivity(Qt::CaseInsensitive);
        completer   ->setFilterMode(Qt::MatchStartsWith);
        searchline()->setCompleter(completer);
        if (ancien)
            ancien  ->deleteLater();             //! évite l'accumulation à chaque filtrage
    }
    //! la table repart sans sélection : rien de choisi, OK désactivé, libellé vide
    wdg_table   ->clearSelection();
    m_codechoisi = "";
    wdg_libelle ->clear();
    OKButton    ->setEnabled(false);
}

/*!
 * \brief dlg_choixccam::selectionChangee
 * à chaque changement de ligne sélectionnée : affiche le libellé, retient le code et active OK.
 */
void dlg_choixccam::selectionChangee()
{
    int row = wdg_table->currentRow();
    if (row < 0 || wdg_table->item(row,0) == Q_NULLPTR)
    {
        m_codechoisi = "";
        wdg_libelle ->clear();
        OKButton    ->setEnabled(false);
        return;
    }
    m_codechoisi = wdg_table->item(row,0)->text();
    wdg_libelle ->setPlainText(wdg_table->item(row,3) ? wdg_table->item(row,3)->text() : "");
    OKButton    ->setEnabled(true);
}

/*!
 * \brief dlg_choixccam::chercheEtSelectionne
 * sélectionne et fait défiler la table jusqu'à la 1re ligne dont le code commence par le texte saisi
 * (la sélection déclenche selectionChangee -> libellé + code retenu + OK actif).
 * \param code  début de code CCAM tapé dans la ligne de recherche
 */
void dlg_choixccam::chercheEtSelectionne(const QString &code)
{
    if (code.isEmpty())
        return;
    for (int row=0; row<wdg_table->rowCount(); row++)
    {
        if (wdg_table->isRowHidden(row))
            continue;                            //! on ne saute que sur les rangées visibles (filtre ophta)
        QTableWidgetItem *it = wdg_table->item(row,0);
        if (it && it->text().startsWith(code, Qt::CaseInsensitive))
        {
            //! setCurrentCell (et non selectRow) : règle AUSSI la cellule courante, dont dépend
            //! selectionChangee (currentRow()) -> le code est bien retenu même sans clic souris
            wdg_table->setCurrentCell(row, 0);
            wdg_table->scrollToItem(it, QAbstractItemView::PositionAtCenter);
            return;
        }
    }
}
