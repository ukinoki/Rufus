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

#include <algorithm>
#include <QLocale>
#include <QPair>
#include "dlg_initbase.h"

//! territoires où s'appliquent les cotations françaises (CCAM, NGAP)
static const QStringList TERRITOIRES_FRANCE = {"FR", "GF", "PF", "NC", "GP", "MQ", "RE", "YT"};

//! Drapeau émoji d'un code ISO à 2 lettres (2 « indicateurs régionaux ») : les .ico du dépôt ne couvrent que les 7 langues
static QString Drapeau(QString code)
{
    QString drapeau;
    const QString maj = code.toUpper();
    for (const QChar &c : maj)
    {
        const char32_t ucs = char32_t(0x1F1E6 + c.unicode() - u'A');
        drapeau += QString::fromUcs4(&ucs, 1);
    }
    return drapeau;
}

dlg_initbase::dlg_initbase(QString langue, QWidget *parent) :
    UpDialog(parent)
{
    setWindowTitle(tr("Premier paramétrage de Rufus"));
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    /*! contenu inséré au-dessus de la rangée de boutons (cf. Utils::SaisirNouvelUtilisateur) */
    QVBoxLayout *lay = new QVBoxLayout();
    lay                 ->setContentsMargins(5, 5, 5, 5);
    lay                 ->setSpacing(5);

    UpLabel *soustitre  = new UpLabel();
    soustitre           ->setText(tr("La base patients est créée. Ces réglages valent pour tout le cabinet ; "
                                     "vous pourrez les changer dans Edition/Paramètres."));
    soustitre           ->setWordWrap(true);
    soustitre           ->setMaximumWidth(400);
    lay                 ->addWidget(soustitre);

    //! --- comptabilité : libellés repris de l'onglet Paramètres, pour ne pas dire deux choses ---
    wdg_comptanormalecheck  = new UpCheckBox(tr("Enregistrer une comptabilité"));
    wdg_comptareduitecheck  = new UpCheckBox(tr("Enregistrer une comptabilité simplifiée"));
    wdg_sanscomptacheck     = new UpCheckBox(tr("Ne pas enregistrer de comptabilité"));
    wdg_comptanormalecheck  ->setImmediateToolTip(tr("Actes, recettes, dépenses et comptes bancaires"));
    wdg_comptareduitecheck  ->setImmediateToolTip(tr("Les recettes des actes, sans dépenses ni banque"));
    wdg_sanscomptacheck     ->setImmediateToolTip(tr("Aucune écriture comptable"));
    wdg_comptanormalecheck  ->setChecked(true);
    lay                     ->addWidget(GroupeExclusif(tr("Comptabilité"),
                                                       {wdg_comptanormalecheck, wdg_comptareduitecheck, wdg_sanscomptacheck}));

    //! --- cotation des actes ---
    wdg_cotationsfrancecheck        = new UpCheckBox(tr("Utiliser les cotations d'actes françaises"));
    wdg_cotationsgeneriquescheck    = new UpCheckBox(tr("Utiliser des cotations génériques"));
    wdg_cotationsfrancecheck        ->setImmediateToolTip(tr("CCAM, NGAP, ALD, secteur conventionnel, OPTAM"));
    wdg_cotationsgeneriquescheck    ->setImmediateToolTip(tr("Libellés et montants libres, sans nomenclature"));
    wdg_cotationsfrancecheck        ->setChecked(true);
    wdg_cotationsgroup              = GroupeExclusif(tr("Cotation des actes"),
                                                     {wdg_cotationsfrancecheck, wdg_cotationsgeneriquescheck});
    lay                             ->addWidget(wdg_cotationsgroup);

    //! --- langue de l'interface et territoire d'exercice ---
    wdg_languecombo     = new QComboBox();
    wdg_territoirecombo = new QComboBox();
    RemplitComboLangues(wdg_languecombo, langue);
    RemplitTerritoires();
    wdg_languecombo     ->setToolTip(tr("Langue de l'interface"));
    wdg_territoirecombo ->setToolTip(tr("Pays d'exercice : villes et codes postaux"));
    UpGroupBox *wdg_languegroup = new UpGroupBox();
    wdg_languegroup     ->setTitle(tr("Langue et territoire"));
    QVBoxLayout *languelay = new QVBoxLayout;
    languelay           ->setContentsMargins(10, 28, 10, 10);
    languelay           ->setSpacing(2);
    languelay           ->addWidget(LigneCombo(tr("Langue"),     wdg_languecombo));
    languelay           ->addWidget(LigneCombo(tr("Territoire"), wdg_territoirecombo));
    wdg_languegroup     ->setLayout(languelay);
    lay                 ->addWidget(wdg_languegroup);

    //! --- utilisateur factice : information seule, la case est verrouillée ---
    const QString msgsnow = tr("Un utilisateur factice est créé pour démarrer ; vous pourrez le modifier "
                               "ou le supprimer dans Edition/Paramètres");
    UpCheckBox *wdg_snowcheck = new UpCheckBox(tr("Créer l'utilisateur") + " " + tr("Docteur") + " Snow");
    wdg_snowcheck       ->setChecked(true);
    wdg_snowcheck       ->setEnabled(false);
    wdg_snowcheck       ->setImmediateToolTip(msgsnow, true);
    wdg_snowcheck       ->setToolTip(msgsnow);      //! filet : la case est désactivée, l'infobulle standard passe quand même
    QFrame *wdg_snowframe = new QFrame();
    wdg_snowframe       ->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *snowlay = new QVBoxLayout;
    snowlay             ->addWidget(wdg_snowcheck);
    wdg_snowframe       ->setLayout(snowlay);
    lay                 ->addWidget(wdg_snowframe);

    //! la cotation suit le territoire, et n'a plus de sens sans comptabilité
    auto SuitTerritoire = [=, this] {
        if (TERRITOIRES_FRANCE.contains(territoire()))
            wdg_cotationsfrancecheck    ->setChecked(true);
        else
            wdg_cotationsgeneriquescheck->setChecked(true);
    };
    connect(wdg_territoirecombo, &QComboBox::currentIndexChanged, this, SuitTerritoire);
    SuitTerritoire();
    connect(wdg_sanscomptacheck, &QCheckBox::toggled, this, [=, this] (bool sanscompta) {
        wdg_cotationsgroup->setEnabled(!sanscompta);
    });

    AjouteLayButtons(UpDialog::ButtonOK);
    dlglayout()         ->insertLayout(0, lay);
    dlglayout()         ->setSizeConstraint(QLayout::SetFixedSize);
    connect(OKButton, &QPushButton::clicked, this, [=, this] {accept();});
}

/*!
 * \brief dlg_initbase::GroupeExclusif
 * Construit un QGroupBox de cases à cocher dont une seule peut être cochée.
 * \param titre  titre du groupe
 * \param cases  les cases, dans l'ordre d'affichage
 */
UpGroupBox* dlg_initbase::GroupeExclusif(QString titre, QList<UpCheckBox*> cases)
{
    UpGroupBox *groupe  = new UpGroupBox();
    groupe              ->setTitle(titre);
    QButtonGroup *grp   = new QButtonGroup(this);
    grp                 ->setExclusive(true);
    QVBoxLayout *lay    = new QVBoxLayout;
    lay                 ->setContentsMargins(10, 28, 10, 10);   //! 20 en haut : le stylesheet UpGroupBox ne réserve pas la place du titre
    for (UpCheckBox *cas : cases)
    {
        grp             ->addButton(cas);
        lay             ->addWidget(cas);
    }
    groupe              ->setLayout(lay);
    return groupe;
}

/*!
 * \brief dlg_initbase::LigneCombo
 * Assemble un libellé et sa liste déroulante sur une ligne.
 * \param texte  libellé affiché à gauche
 * \param combo  la liste déroulante
 */
QWidget* dlg_initbase::LigneCombo(QString texte, QComboBox *combo)
{
    UpLabel *label      = new UpLabel();
    label               ->setText(texte);
    label               ->setMinimumWidth(90);
    combo               ->setFixedWidth(280);
    QWidget *widg       = new QWidget();
    QHBoxLayout *lay    = new QHBoxLayout;
    lay                 ->insertWidget(0, label);
    lay                 ->insertSpacerItem(1, new QSpacerItem(10,5));
    lay                 ->insertWidget(2, combo);
    lay                 ->setContentsMargins(0,0,0,0);
    widg                ->setLayout(lay);
    return widg;
}

/*!
 * \brief dlg_initbase::RemplitComboLangues
 * Remplit une liste déroulante des 7 langues traduites, à la présentation du sélecteur de dlg_param.
 * \param combo   la liste à remplir
 * \param langue  la langue à présenter, repli sur le premier item si elle est inconnue
 */
void dlg_initbase::RemplitComboLangues(QComboBox *combo, QString langue)
{
    combo   ->addItem(QIcon(QPixmap("://France.ico")),          "Français",     QStringLiteral("FR"));
    combo   ->addItem(QIcon(QPixmap("://United-kingdom.ico")),  "English",      QStringLiteral("EN"));
    combo   ->addItem(QIcon(QPixmap("://Spain.ico")),           "Español",      QStringLiteral("ES"));
    combo   ->addItem(QIcon(QPixmap("://Brazil.ico")),          "Brasileiro",   QStringLiteral("BR"));
    combo   ->addItem(QIcon(QPixmap("://Portugal.ico")),        "Português",    QStringLiteral("PT"));
    combo   ->addItem(QIcon(QPixmap("://Italy.ico")),           "Italiano",     QStringLiteral("IT"));
    combo   ->addItem(QIcon(QPixmap("://Romania.ico")),         "Română",       QStringLiteral("RO"));
    combo   ->setCurrentIndex(qMax(0, combo->findData(langue)));
}

/*!
 * \brief dlg_initbase::RemplitTerritoires
 * Remplit la liste des territoires : tous ceux que connaît QLocale, nom dans la langue du pays,
 * code ISO à 2 lettres entre parenthèses et drapeau devant.
 */
void dlg_initbase::RemplitTerritoires()
{
    QMap<QString, QString> nomparcode;    /*!< code ISO -> nom du territoire, doublons éliminés */
    const QList<QLocale> locales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyTerritory);
    for (const QLocale &loc : locales)
    {
        const QString code = QLocale::territoryToCode(loc.territory());
        if (code.size() != 2)                                   //! 2 lettres : écarte les regroupements (Monde, Europe…)
            continue;
        //! nom anglais : nativeTerritoryName() donnait la langue de la 1re locale trouvée (France -> « Frañs », du breton)
        nomparcode.insert(code, QLocale::territoryToString(loc.territory()));
    }

    QList<QPair<QString, QString>> liste;    /*!< nom, code - trié sur le nom affiché */
    for (auto it = nomparcode.cbegin(); it != nomparcode.cend(); ++it)
        liste << qMakePair(it.value(), it.key());
    std::sort(liste.begin(), liste.end(), [] (const QPair<QString, QString> &a, const QPair<QString, QString> &b)
                                             { return QString::localeAwareCompare(a.first, b.first) < 0; });

    for (const QPair<QString, QString> &terr : liste)
        wdg_territoirecombo->addItem(Drapeau(terr.second) + "  " + terr.first + " (" + terr.second + ")", terr.second);

    //! le pays du poste, à défaut la France
    int idx = wdg_territoirecombo->findData(QLocale::territoryToCode(QLocale::system().territory()));
    if (idx < 0)
        idx = wdg_territoirecombo->findData(QStringLiteral("FR"));
    wdg_territoirecombo ->setCurrentIndex(idx);
}

