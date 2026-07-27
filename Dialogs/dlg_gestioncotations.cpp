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

#include <QRegularExpressionValidator>
#include <QToolTip>
#include "dlg_gestioncotations.h"

dlg_gestioncotations::dlg_gestioncotations(Mode mode, QString CodeActe, QWidget *parent) :
    UpDialog(parent)
{
    m_mode     = mode;
    m_codeacte = CodeActe;
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    //! saisie numérique imposée dans les champs montant : chiffres + séparateur décimal (. ou ,) optionnel
    const QRegularExpression rgxMontant("[0-9]{0,6}([.,][0-9]{0,2})?");

    //! table ccam : codes (pour valider) + modèle code/libellé (source des QCompleter avec infobulle).
    //! Chaque item porte le code en DisplayRole et le libellé de l'acte en ToolTipRole -> le popup du
    //! completer affiche la description au survol.
    m_modelCCAM = new QStandardItemModel(this);
    bool ok = false;
    QList<QVariantList> lccam = db->StandardSelectSQL("select " CP_CODECCAM_CCAM ", " CP_NOM_CCAM " from " TBL_CCAM
                                                      " order by " CP_CODECCAM_CCAM, ok);
    if (ok)
        for (const QVariantList &r : lccam)
        {
            const QString code = r.at(0).toString();
            m_listeCCAM << code;
            QStandardItem *it = new QStandardItem(code);
            it->setToolTip(r.at(1).toString());        //! libellé de l'acte, affiché en infobulle dans le completer
            m_modelCCAM->appendRow(it);
        }

    //! --- sélecteur de mode : QGroupBox « Nouvelle cotation » + 3 cases exclusives ---
    wdg_groupmode   = new QGroupBox(tr("Nouvelle cotation"));
    wdg_chkCCAM     = new QCheckBox(tr("Ajouter une cotation CCAM"));
    wdg_chkAssoc    = new QCheckBox(tr("Créer une association CCAM"));
    wdg_chkAutre    = new QCheckBox(tr("Créer une cotation"));
    QButtonGroup *grp = new QButtonGroup(this);
    grp             ->setExclusive(true);
    grp             ->addButton(wdg_chkCCAM);
    grp             ->addButton(wdg_chkAssoc);
    grp             ->addButton(wdg_chkAutre);
    QVBoxLayout *grouplay = new QVBoxLayout;
    grouplay        ->addWidget(wdg_chkCCAM);
    grouplay        ->addWidget(wdg_chkAssoc);
    grouplay        ->addWidget(wdg_chkAutre);
    wdg_groupmode   ->setLayout(grouplay);
    dlglayout()     ->replaceWidget(widgetbuttons(), wdg_groupmode);
    //! changer de mode réapplique la présentation ET recalcule les montants (remplitDepuisCCAM fait
    //! return en mode « autre », donc inoffensif là)
    connect(wdg_chkCCAM,    &QCheckBox::toggled, this, [=, this] (bool c) {if (c) {m_typecotation = 1; appliqueMode(); remplitDepuisCCAM();}});
    connect(wdg_chkAssoc,   &QCheckBox::toggled, this, [=, this] (bool c) {if (c) {m_typecotation = 2; appliqueMode(); remplitDepuisCCAM();}});
    connect(wdg_chkAutre,   &QCheckBox::toggled, this, [=, this] (bool c) {if (c) {m_typecotation = 4; appliqueMode(); remplitDepuisCCAM();}});

    //! fabriques de lignes de saisie : même présentation d'une ligne à l'autre, sans la répéter.
    //! ligneCode  : label + champ code + bouton « ... » (appel de la table CCAM) ;
    //! ligneMontant : label + champ montant (validateur numérique). Elles renseignent les pointeurs
    //! membres passés par référence et ajoutent la ligne au layout.
    auto ligneCode = [&] (UpLineEdit *&edit, QPushButton *&bouton, QWidget *&widg, const QString &texteLabel) {
        edit    = new UpLineEdit();
        edit    ->setFixedWidth(150);
        edit    ->setAlignment(Qt::AlignRight);
        edit    ->setMaxLength(15);
        UpLabel *label = new UpLabel();     label->setText(texteLabel);
        bouton  = new QPushButton("...");   //! bouton « parcourir » (façon PosteVideoDirupPushButton)
        bouton  ->setFixedSize(54,32);
        bouton  ->setToolTip(tr("Choisir un code CCAM"));
        bouton  ->setFocusPolicy(Qt::NoFocus);
        widg    = new QWidget();
        QHBoxLayout *lay = new QHBoxLayout;
        lay     ->insertWidget(0, label);
        lay     ->insertSpacerItem(1, new QSpacerItem(10,5));
        lay     ->insertWidget(2, edit);
        lay     ->insertWidget(3, bouton);
        lay     ->setContentsMargins(0,0,0,0);
        widg    ->setLayout(lay);
        connect(edit,   &QLineEdit::textEdited,      this, [=, this] {regleOK();});
        connect(edit,   &QLineEdit::editingFinished, this, [=, this] {remplitDepuisCCAM();});
        connect(bouton, &QPushButton::clicked,       this, [=, this] {appelleTableCCAM(edit);});
        dlglayout()->addWidget(widg);
    };
    auto ligneMontant = [&] (UpLineEdit *&edit, QWidget *&widg, UpLabel *label) {
        edit    = new UpLineEdit();
        edit    ->setFixedWidth(100);
        edit    ->setAlignment(Qt::AlignRight);
        edit    ->setValidator(new QRegularExpressionValidator(rgxMontant, this));
        widg    = new QWidget();
        QHBoxLayout *lay = new QHBoxLayout;
        lay     ->insertWidget(0, label);
        lay     ->insertSpacerItem(1, new QSpacerItem(10,5));
        lay     ->insertWidget(2, edit);
        lay     ->setContentsMargins(0,0,0,0);
        widg    ->setLayout(lay);
        connect(edit, &QLineEdit::textEdited, this, [=, this] {regleOK();});
        dlglayout()->addWidget(widg);
    };

    //! --- code (1er), puis 2e code (association) ---
    ligneCode(wdg_codeline,  wdg_boutonCCAM1, wdg_codewidg,  "Code");
    wdg_codeline ->setText(CodeActe);
    ligneCode(wdg_codeline2, wdg_boutonCCAM2, wdg_code2widg, tr("2e code"));

    //! --- montants : OPTAM/conventionnel (label retitré par appliqueMode), non OPTAM (masqué en mode 4), pratiqué ---
    wdg_optamlabel = new UpLabel();     //! son texte est posé par appliqueMode selon le mode
    ligneMontant(wdg_tarifoptamline,    wdg_tarifoptamwidg,   wdg_optamlabel);
    UpLabel *nooptamlabel  = new UpLabel();  nooptamlabel ->setText(tr("Tarif conventionnel non OPTAM"));
    ligneMontant(wdg_tarifnooptamline,  wdg_tarifnooptamwidg, nooptamlabel);
    UpLabel *pratiquelabel = new UpLabel();  pratiquelabel->setText(tr("Tarif pratiqué"));
    ligneMontant(wdg_tarifpratiqueline, wdg_tarifpratiquewidg, pratiquelabel);

    //! --- tip (libellé / description) ---
    wdg_tipline     = new UpTextEdit();
    wdg_tipline     ->setFixedWidth(300);
    wdg_tipline     ->setFixedHeight(90);       /*!< ~4-5 lignes : le Tip est multiligne */
    UpLabel *tiplabel = new UpLabel();          tiplabel->setText(TIP);
    wdg_tipwidg     = new QWidget();
    QHBoxLayout *tiplay = new QHBoxLayout;
    tiplay          ->insertWidget(0, tiplabel);
    tiplay          ->insertSpacerItem(1, new QSpacerItem(10,5));
    tiplay          ->insertWidget(2, wdg_tipline);
    tiplay          ->setContentsMargins(0,0,0,0);
    wdg_tipwidg     ->setLayout(tiplay);
    connect(wdg_tipline, &QTextEdit::textChanged, this, [=, this] {regleOK();});
    wdg_tipline     ->installEventFilter(this);      //! focus -> curseur en fin de texte (cf. eventFilter)
    dlglayout()     ->addWidget(wdg_tipwidg);

    int marge = 5;
    dlglayout()     ->setSpacing(marge);
    dlglayout()     ->setContentsMargins(marge, marge, marge, marge);

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout()     ->addWidget(widgetbuttons());
    OKButton        ->setEnabled(false);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    connect(OKButton,       &QPushButton::clicked,  this,   [=, this] {if (VerifFiche()) accept();});
    connect(CancelButton,   &QPushButton::clicked,  this,   [=, this] {reject();});

    if (m_mode == Modification)
    {
        //! modification réservée aux « autres » (type 4) : la case « autre » est cochée, les 2 cases
        //! CCAM sont désactivées — on ne modifie pas un acte CCAM, ses montants CCAM étant figés.
        wdg_groupmode->setTitle(tr("Modifier la cotation"));
        wdg_chkCCAM ->setEnabled(false);
        wdg_chkAssoc->setEnabled(false);
        wdg_chkAutre->setChecked(true);             //! -> m_typecotation = 4 (via le toggled)

        const int iduser = Datas::I()->users->userconnected()->id();
        //! id de la cotation (clé de l'update) + libellé, depuis la table partagée
        QVariantList c = db->getFirstRecordFromStandardSelectSQL(
            "select " CP_ID_COTATIONS ", " CP_TIP_COTATIONS " from " TBL_COTATIONS
            " where " CP_TYPEACTE_COTATIONS " = '" + m_codeacte + "'", ok);
        if (ok && c.size() >= 2)
        {
            m_idcotation = c.at(0).toInt();
            wdg_tipline->setPlainText(c.at(1).toString());
        }
        //! montants propres à l'utilisateur (conventionnel + pratiqué) depuis SA jointure
        double conv = 0, prat = 0;
        if (db->lisMontantsJointureAutre(m_idcotation, iduser, conv, prat))
        {
            wdg_tarifoptamline   ->setText(QLocale().toString(conv, 'f', 2));
            wdg_tarifpratiqueline->setText(QLocale().toString(prat, 'f', 2));
        }
    }
    else
        wdg_chkCCAM->setChecked(true);          //! création : mode CCAM par défaut

    appliqueMode();
}

dlg_gestioncotations::~dlg_gestioncotations()
{
}

/*!
 * \brief dlg_gestioncotations::appliqueMode
 * montre/masque/retitre les champs, règle leur éditabilité et pose les QCompleter selon le mode
 * courant (m_typecotation : 1 CCAM, 2 association, 4 autre).
 */
void dlg_gestioncotations::appliqueMode()
{
    const bool ccam  = (m_typecotation == 1);
    const bool assoc = (m_typecotation == 2);
    const bool autre = (m_typecotation == 4);

    //! 2e code : association seulement (son bouton « ... » suit avec lui)
    wdg_code2widg   ->setVisible(assoc);
    //! bouton « ... » du 1er code : présent dès qu'on saisit un code CCAM (modes CCAM et association)
    wdg_boutonCCAM1 ->setVisible(ccam || assoc);
    //! non-OPTAM : masqué en mode autre
    wdg_tarifnooptamwidg->setVisible(!autre);

    //! QCompleter CCAM sur les codes en modes 1 et 2 (on ne peut choisir qu'un code CCAM) ; aucun en mode
    //! autre. Le completer s'appuie sur m_modelCCAM -> infobulle (libellé de l'acte) dans le popup.
    QCompleter *comp1 = (ccam || assoc) ? new QCompleter(m_modelCCAM, this) : Q_NULLPTR;
    if (comp1) comp1    ->setCaseSensitivity(Qt::CaseInsensitive);
    wdg_codeline        ->setCompleter(comp1);
    QCompleter *comp2 = assoc ? new QCompleter(m_modelCCAM, this) : Q_NULLPTR;
    if (comp2) comp2    ->setCaseSensitivity(Qt::CaseInsensitive);
    wdg_codeline2       ->setCompleter(comp2);

    //! le montant du haut est retitré « Montant conventionnel » en mode autre
    wdg_optamlabel  ->setText(autre ? tr("Montant conventionnel")
                                    : (m_cotationsfrance ? tr("Tarif conventionnel OPTAM") : tr("Tarif remboursé")));

    //! éditabilité : conventionnel saisi seulement en mode autre ; non-OPTAM jamais (repris/calculé) ;
    //! pratiqué TOUJOURS éditable (pré-rempli au conventionnel calculé, l'utilisateur peut le dépasser,
    //! y compris en association) ; tip non éditable en CCAM (repris)
    wdg_tarifnooptamline    ->setFocusPolicy(Qt::NoFocus);
    wdg_tarifoptamline      ->setFocusPolicy(autre? Qt::StrongFocus : Qt::NoFocus);

    wdg_tipline   ->setFocusPolicy(ccam ? Qt::NoFocus : Qt::StrongFocus);

    setWindowTitle(ccam ? tr("Cotation CCAM") : assoc ? tr("Association CCAM") : tr("Cotation"));

    regleOK();      //! le changement de mode change les champs requis -> réévalue l'état du bouton OK
}

/*!
 * \brief dlg_gestioncotations::remplitDepuisCCAM
 * modes 1 et 2 : à partir du (des) code(s) CCAM saisi(s), remplit les montants (association : 1er acte
 * plein + 2e acte pour moitié), le libellé (repris de ccam) et le pratiqué (conventionnel selon le
 * secteur OPTAM du parent).
 */
void dlg_gestioncotations::remplitDepuisCCAM()
{
    if (m_typecotation != 1 && m_typecotation != 2)
        return;

    //! lecteur d'un acte CCAM : rend ses montants (0 si code vide ou inconnu) et son libellé.
    //! On ne s'interrompt jamais : un code absent/invalide contribue simplement 0 -> on recalcule
    //! toujours à partir de ce qui est présent (pas de cas particuliers à traiter un par un).
    auto litCCAM = [this] (const QString &code, double &optam, double &nonoptam, QString &nom) {
        optam = 0; nonoptam = 0; nom.clear();
        if (code.isEmpty())
            return;
        bool ok = false;
        QVariantList r = db->getFirstRecordFromStandardSelectSQL(
            "select " CP_MONTANTOPTAM_CCAM ", " CP_MONTANTNONOPTAM_CCAM ", " CP_NOM_CCAM
            " from " TBL_CCAM " where " CP_CODECCAM_CCAM " = '" + code + "'", ok);
        if (ok && r.size() >= 3)
        {
            optam    = r.at(0).toDouble();
            nonoptam = r.at(1).toDouble();
            nom      = r.at(2).toString();
        }
    };

    //! 1er acte : toujours pris en compte (modes 1 et 2)
    double optam = 0, nonoptam = 0;
    QString nom1;
    litCCAM(wdg_codeline->text().trimmed(), optam, nonoptam, nom1);
    QStringList noms;
    if (!nom1.isEmpty())    noms << nom1;

    //! association : le 2e acte compte pour MOITIÉ (optam1 + optam2/2, idem non-OPTAM)
    if (m_typecotation == 2)
    {
        double optam2 = 0, nonoptam2 = 0;
        QString nom2;
        litCCAM(wdg_codeline2->text().trimmed(), optam2, nonoptam2, nom2);
        optam    += optam2 / 2.0;
        nonoptam += nonoptam2 / 2.0;
        if (!nom2.isEmpty())    noms << nom2;
    }

    wdg_tarifoptamline  ->setText(QLocale().toString(optam, 'f', 2));
    wdg_tarifnooptamline->setText(QLocale().toString(nonoptam, 'f', 2));
    wdg_tipline         ->setPlainText(noms.join(" + "));

    //! pratiqué par défaut = conventionnel selon le secteur du PARENT (titulaire) : OPTAM -> optam, sinon nonoptam
    User *u      = Datas::I()->users->userconnected();
    User *parent = Datas::I()->users->getById(u->idparent());
    const bool optamRef = parent ? parent->isOPTAM() : u->isOPTAM();
    wdg_tarifpratiqueline->setText(QLocale().toString(optamRef ? optam : nonoptam, 'f', 2));
    regleOK();
}

/*!
 * \brief dlg_gestioncotations::regleOK
 * active OK selon le mode : CCAM (1) et association (2) exigent TOUS les lineedit visibles remplis ;
 * « autre » (4) n'exige que le code et le tarif pratiqué (le conventionnel est facultatif).
 */
void dlg_gestioncotations::regleOK()
{
    auto rempli = [] (UpLineEdit *l) { return !l->text().trimmed().isEmpty(); };
    bool complet;
    if (m_typecotation == 1)                //! CCAM
        complet = rempli(wdg_codeline) && rempli(wdg_tarifoptamline)
                  && rempli(wdg_tarifnooptamline) && rempli(wdg_tarifpratiqueline);
    else if (m_typecotation == 2)           //! association CCAM
        complet = rempli(wdg_codeline) && rempli(wdg_codeline2) && rempli(wdg_tarifoptamline)
                  && rempli(wdg_tarifnooptamline) && rempli(wdg_tarifpratiqueline);
    else                                    //! autre : code + pratiqué (conventionnel facultatif)
        complet = rempli(wdg_codeline) && rempli(wdg_tarifpratiqueline);
    OKButton->setEnabled(complet);
}

/*!
 * \brief dlg_gestioncotations::eventFilter
 * quand le tip (UpTextEdit) prend le focus, place le curseur en fin de texte (prêt à compléter).
 */
bool dlg_gestioncotations::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == wdg_tipline && event->type() == QEvent::FocusIn)
        wdg_tipline->moveCursor(QTextCursor::End);
    return UpDialog::eventFilter(obj, event);
}

/*!
 * \brief dlg_gestioncotations::appelleTableCCAM
 * ouvre la fiche de choix d'un acte CCAM ; le code retenu remplit le champ cible (1er ou 2e code) puis
 * remplitDepuisCCAM() renseigne les montants OPTAM / non OPTAM et le libellé.
 * \param cible  ligne de code à remplir (wdg_codeline ou wdg_codeline2)
 */
void dlg_gestioncotations::appelleTableCCAM(UpLineEdit *cible)
{
    dlg_choixccam dlg(this);
    if (dlg.exec() == QDialog::Accepted && !dlg.codechoisi().isEmpty())
    {
        cible->setText(dlg.codechoisi());
        remplitDepuisCCAM();
    }
}

/*!
 * \brief dlg_gestioncotations::VerifFiche
 * contrôle la saisie selon le mode puis enregistre (insert en création, update en modification).
 */
bool dlg_gestioncotations::VerifFiche()
{
    const QString msg    = tr("Renseignements incomplets");
    const int     iduser = Datas::I()->users->userconnected()->id();

    //! code final (association = « code1+code2 » — stockage provisoire, cf. TODO du .h)
    const QString code1 = wdg_codeline->text().trimmed();
    const QString code2 = wdg_codeline2->text().trimmed();
    QString code = (m_typecotation == 2) ? code1 + "+" + code2 : code1;
    if (code1.isEmpty() || (m_typecotation == 2 && code2.isEmpty()))
    {
        UpMessageBox::Watch(this, msg, tr("Vous avez oublié le code de l'acte"));
        return false;
    }
    //! modes CCAM / association : les codes doivent exister dans la table ccam
    if (m_typecotation == 1 || m_typecotation == 2)
    {
        QStringList codes;  codes << code1;
        if (m_typecotation == 2) codes << code2;
        for (const QString &c : codes)
            if (!m_listeCCAM.contains(c))
            {
                UpMessageBox::Watch(this, msg, tr("« %1 » n'est pas un code CCAM").arg(c));
                return false;
            }
    }
    //! tarif pratiqué obligatoire
    if (QLocale().toDouble(wdg_tarifpratiqueline->text()) == 0.0)
    {
        UpMessageBox::Watch(this, msg, tr("Vous avez oublié le tarif pratiqué"));
        wdg_tarifpratiqueline->setFocus();
        return false;
    }
    //! montants (non-OPTAM null en mode autre, où le champ est masqué)
    const double  optamVal    = QLocale().toDouble(wdg_tarifoptamline->text());       //! en mode « autre », c'est le conventionnel
    const double  pratiqueVal = QLocale().toDouble(wdg_tarifpratiqueline->text());
    const QString optamSQL    = QString::number(optamVal);
    const QString nonoptamSQL = (m_typecotation == 4) ? "null" : QString::number(QLocale().toDouble(wdg_tarifnooptamline->text()));
    const QString pratiqueSQL = QString::number(pratiqueVal);
    const QString tipSQL      = Utils::correctquoteSQL(wdg_tipline->toPlainText());
    const QString codeSQL     = Utils::correctquoteSQL(code);
    const QString iduserSQL   = QString::number(iduser);
    bool ok;

    //! --- MODIFICATION (réservée au type 4) : update par idcotation ---
    if (m_mode == Modification)
    {
        //! cotation partagée : code, conventionnel (dans MontantOptam) et libellé, clé = idcotation
        db->StandardSQL("update " TBL_COTATIONS " set "
              CP_TYPEACTE_COTATIONS " = '"    + codeSQL + "', "
              CP_MONTANTOPTAM_COTATIONS " = " + optamSQL + ", "
              CP_TIP_COTATIONS " = '"         + tipSQL + "'"
              " where " CP_ID_COTATIONS " = " + QString::number(m_idcotation));
        //! montants propres à l'utilisateur (conventionnel + pratiqué) dans SA jointure
        db->ajouteJointureCotation(m_typecotation, m_idcotation, iduser, pratiqueVal, optamVal);
        m_codeenregistre = code;
        return true;
    }

    //! --- CRÉATION : la table cotations est PARTAGÉE -> on évite les doublons ---
    //! Dédup restreinte au même type : CCAM (1) / association (2) par code exact (codes toujours en
    //! majuscules) ; « autre » (4) en comparant les codes MIS EN MAJUSCULES (codes libres, à ne pas
    //! dupliquer sur une simple différence de casse). L'acte déjà présent n'est pas dupliqué : on
    //! rafraîchit ses montants ; sinon on l'insère. Puis on crée (ou met à jour) la jointure du user.
    const bool autre = (m_typecotation == 4);
    const QString whereActe =
        (autre ? "UPPER(" CP_TYPEACTE_COTATIONS ") = UPPER('" + codeSQL + "')"
               : QString(CP_TYPEACTE_COTATIONS " = '") + codeSQL + "'")
        + " and " CP_TYPECOTATION_COTATIONS " = " + QString::number(m_typecotation);

    QVariantList ex = db->getFirstRecordFromStandardSelectSQL(
        "select min(" CP_ID_COTATIONS ") from " TBL_COTATIONS " where " + whereActe, ok);
    int idcot = (ok && ex.size() > 0 && !ex.at(0).isNull()) ? ex.at(0).toInt() : 0;
    if (idcot > 0)
    {
        //! montants rafraîchis : « autre » -> conventionnel (dans MontantOptam) + pratiqué ;
        //! CCAM/assoc -> OPTAM + non-OPTAM (le pratiqué reste dans la jointure)
        const QString set = autre
            ? QString(CP_MONTANTOPTAM_COTATIONS " = ") + optamSQL + ", " CP_MONTANTPRATIQUE_COTATIONS " = " + pratiqueSQL
            : QString(CP_MONTANTOPTAM_COTATIONS " = ") + optamSQL + ", " CP_MONTANTNONOPTAM_COTATIONS " = " + nonoptamSQL;
        db->StandardSQL("update " TBL_COTATIONS " set " + set + " where " CP_ID_COTATIONS " = " + QString::number(idcot));
    }
    else
    {
        db->StandardSQL("insert into " TBL_COTATIONS " (" CP_TYPEACTE_COTATIONS ", " CP_MONTANTOPTAM_COTATIONS ", "
              CP_MONTANTNONOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS ", " CP_TYPECOTATION_COTATIONS ", "
              CP_IDUSER_COTATIONS ", " CP_TIP_COTATIONS ") VALUES ('"
              + codeSQL + "', " + optamSQL + ", " + nonoptamSQL + ", " + pratiqueSQL + ", "
              + QString::number(m_typecotation) + ", " + iduserSQL + ", '" + tipSQL + "')");
        QVariantList n = db->getFirstRecordFromStandardSelectSQL(
            "select min(" CP_ID_COTATIONS ") from " TBL_COTATIONS " where " + whereActe, ok);
        idcot = (ok && n.size() > 0 && !n.at(0).isNull()) ? n.at(0).toInt() : 0;
    }

    //! jointure du user : (re)créée avec les montants. « autre » -> conventionnel + pratiqué ;
    //! CCAM/association -> pratiqué seul (le conventionnel passé est alors ignoré).
    if (idcot > 0)
        db->ajouteJointureCotation(m_typecotation, idcot, iduser, pratiqueVal, optamVal);

    m_codeenregistre = code;
    return true;
}


/* ============================================================================================
 *  dlg_choixccam — fiche de choix d'un acte CCAM (déclarée dans dlg_gestioncotations.h car elle
 *  ne sert qu'ici). Appelée par dlg_gestioncotations::appelleTableCCAM().
 * ============================================================================================ */

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
    connect(wdg_ophtaseul,  &QCheckBox::clicked, this, [=, this] (bool b) {filtreOphta(b);});
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
    connect(wdg_table, &QTableWidget::itemSelectionChanged, this, [=, this] {selectionChangee();});
    //! double-clic sur une ligne : sélectionne (via la cellule courante) et valide comme OK
    connect(wdg_table, &QAbstractItemView::doubleClicked, this, [=, this] (QModelIndex idx) {
        wdg_table->setCurrentCell(idx.row(), 0);
        if (!m_codechoisi.isEmpty())
            accept();
    });
    //! survol souris : infobulle immédiate avec le libellé de l'acte (colonne masquée), comme la table
    //! des cotations de dlg_param (mouseTracking + signal entered)
    connect(wdg_table, &QAbstractItemView::entered, this, [=, this] (QModelIndex idx) {
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
    connect(searchline(),   &QLineEdit::textChanged, this, [=, this] (QString t) {chercheEtSelectionne(t.trimmed());});
    connect(OKButton,       &QPushButton::clicked, this, [=, this] {accept();});
    connect(CancelButton,   &QPushButton::clicked, this, [=, this] {reject();});

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
