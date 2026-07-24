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
#include "dlg_gestioncotations.h"
#include "dlg_choixccam.h"

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
    connect(wdg_chkCCAM,    &QCheckBox::toggled, this, [=] (bool c) {if (c) {m_typecotation = 1; appliqueMode();}});
    connect(wdg_chkAssoc,   &QCheckBox::toggled, this, [=] (bool c) {if (c) {m_typecotation = 2; appliqueMode();}});
    connect(wdg_chkAutre,   &QCheckBox::toggled, this, [=] (bool c) {if (c) {m_typecotation = 4; appliqueMode();}});

    //! --- code (1er) ---
    wdg_codeline    = new UpLineEdit();
    wdg_codeline    ->setFixedWidth(150);
    wdg_codeline    ->setAlignment(Qt::AlignRight);
    wdg_codeline    ->setMaxLength(15);
    wdg_codeline    ->setText(CodeActe);
    UpLabel *codelabel = new UpLabel();     codelabel->setText("Code");
    wdg_boutonCCAM1 = new QPushButton("...");  //! bouton « parcourir » (façon PosteVideoDirupPushButton)
    wdg_boutonCCAM1 ->setFixedSize(54,32);
    wdg_boutonCCAM1 ->setToolTip(tr("Choisir un code CCAM)"));
    wdg_boutonCCAM1 ->setFocusPolicy(Qt::NoFocus);
    wdg_codewidg    = new QWidget();
    QHBoxLayout *codelay = new QHBoxLayout;
    codelay         ->insertWidget(0, codelabel);
    codelay         ->insertSpacerItem(1, new QSpacerItem(10,5));
    codelay         ->insertWidget(2, wdg_codeline);
    codelay         ->insertWidget(3, wdg_boutonCCAM1);
    codelay         ->setContentsMargins(0,0,0,0);
    wdg_codewidg    ->setLayout(codelay);
    connect(wdg_codeline,   &QLineEdit::textEdited,      this,   [=] {regleOK();});
    connect(wdg_codeline,   &QLineEdit::editingFinished, this,   [=] {remplitDepuisCCAM();});
    connect(wdg_boutonCCAM1, &QPushButton::clicked,      this,   [=] {appelleTableCCAM(wdg_codeline);});
    dlglayout()     ->addWidget(wdg_codewidg);

    //! --- code (2e, association) ---
    wdg_codeline2   = new UpLineEdit();
    wdg_codeline2   ->setFixedWidth(150);
    wdg_codeline2   ->setAlignment(Qt::AlignRight);
    wdg_codeline2   ->setMaxLength(15);
    UpLabel *code2label = new UpLabel();    code2label->setText(tr("2e code"));
    wdg_boutonCCAM2 = new QPushButton("...");  //! bouton « parcourir » (façon PosteVideoDirupPushButton)
    wdg_boutonCCAM2 ->setFixedSize(54,32);
    wdg_boutonCCAM2 ->setToolTip(tr("Choisir un code CCAM)"));
    wdg_boutonCCAM2 ->setFocusPolicy(Qt::NoFocus);
    wdg_code2widg   = new QWidget();
    QHBoxLayout *code2lay = new QHBoxLayout;
    code2lay        ->insertWidget(0, code2label);
    code2lay        ->insertSpacerItem(1, new QSpacerItem(10,5));
    code2lay        ->insertWidget(2, wdg_codeline2);
    code2lay        ->insertWidget(3, wdg_boutonCCAM2);
    code2lay        ->setContentsMargins(0,0,0,0);
    wdg_code2widg   ->setLayout(code2lay);
    connect(wdg_codeline2,  &QLineEdit::textEdited,      this,   [=] {regleOK();});
    connect(wdg_codeline2,  &QLineEdit::editingFinished, this,   [=] {remplitDepuisCCAM();});
    connect(wdg_boutonCCAM2, &QPushButton::clicked,      this,   [=] {appelleTableCCAM(wdg_codeline2);});
    dlglayout()     ->addWidget(wdg_code2widg);

    //! --- montant OPTAM / conventionnel ---
    wdg_tarifoptamline  = new UpLineEdit();
    wdg_tarifoptamline  ->setFixedWidth(100);
    wdg_tarifoptamline  ->setAlignment(Qt::AlignRight);
    wdg_tarifoptamline  ->setValidator(new QRegularExpressionValidator(rgxMontant, this));
    wdg_optamlabel      = new UpLabel();
    wdg_tarifoptamwidg  = new QWidget();
    QHBoxLayout *optamlay = new QHBoxLayout;
    optamlay            ->insertWidget(0, wdg_optamlabel);
    optamlay            ->insertSpacerItem(1, new QSpacerItem(10,5));
    optamlay            ->insertWidget(2, wdg_tarifoptamline);
    optamlay            ->setContentsMargins(0,0,0,0);
    wdg_tarifoptamwidg  ->setLayout(optamlay);
    connect(wdg_tarifoptamline, &QLineEdit::textEdited, this,   [=] {regleOK();});
    dlglayout()     ->addWidget(wdg_tarifoptamwidg);

    //! --- montant non OPTAM (masqué en mode 4) ---
    wdg_tarifnooptamline = new UpLineEdit();
    wdg_tarifnooptamline ->setFixedWidth(100);
    wdg_tarifnooptamline ->setAlignment(Qt::AlignRight);
    wdg_tarifnooptamline ->setValidator(new QRegularExpressionValidator(rgxMontant, this));
    UpLabel *nooptamlabel = new UpLabel();  nooptamlabel->setText(tr("Tarif conventionnel non OPTAM"));
    wdg_tarifnooptamwidg = new QWidget();
    QHBoxLayout *nooptamlay = new QHBoxLayout;
    nooptamlay          ->insertWidget(0, nooptamlabel);
    nooptamlay          ->insertSpacerItem(1, new QSpacerItem(10,5));
    nooptamlay          ->insertWidget(2, wdg_tarifnooptamline);
    nooptamlay          ->setContentsMargins(0,0,0,0);
    wdg_tarifnooptamwidg ->setLayout(nooptamlay);
    connect(wdg_tarifnooptamline, &QLineEdit::textEdited, this, [=] {regleOK();});
    dlglayout()     ->addWidget(wdg_tarifnooptamwidg);

    //! --- montant pratiqué ---
    wdg_tarifpratiqueline = new UpLineEdit();
    wdg_tarifpratiqueline ->setFixedWidth(100);
    wdg_tarifpratiqueline ->setAlignment(Qt::AlignRight);
    wdg_tarifpratiqueline ->setValidator(new QRegularExpressionValidator(rgxMontant, this));
    UpLabel *pratiquelabel = new UpLabel(); pratiquelabel->setText(tr("Tarif pratiqué"));
    wdg_tarifpratiquewidg = new QWidget();
    QHBoxLayout *pratiquelay = new QHBoxLayout;
    pratiquelay         ->insertWidget(0, pratiquelabel);
    pratiquelay         ->insertSpacerItem(1, new QSpacerItem(10,5));
    pratiquelay         ->insertWidget(2, wdg_tarifpratiqueline);
    pratiquelay         ->setContentsMargins(0,0,0,0);
    wdg_tarifpratiquewidg ->setLayout(pratiquelay);
    connect(wdg_tarifpratiqueline, &QLineEdit::textEdited, this, [=] {regleOK();});
    dlglayout()     ->addWidget(wdg_tarifpratiquewidg);

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
    connect(wdg_tipline, &QTextEdit::textChanged, this, [=] {regleOK();});
    dlglayout()     ->addWidget(wdg_tipwidg);

    int marge = 5;
    dlglayout()     ->setSpacing(marge);
    dlglayout()     ->setContentsMargins(marge, marge, marge, marge);

    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout()     ->addWidget(widgetbuttons());
    OKButton        ->setEnabled(false);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);
    connect(OKButton,       &QPushButton::clicked,  this,   [=] {if (VerifFiche()) accept();});
    connect(CancelButton,   &QPushButton::clicked,  this,   [=] {reject();});

    if (m_mode == Modification)
    {
        //! pas de choix de mode en modification : le type est celui de la cotation (2 ou 4)
        wdg_groupmode->setVisible(false);
        QVariantList t = db->getFirstRecordFromStandardSelectSQL(
            "select " CP_TYPECOTATION_COTATIONS ", " CP_MONTANTOPTAM_COTATIONS ", " CP_MONTANTNONOPTAM_COTATIONS ", "
            CP_MONTANTPRATIQUE_COTATIONS ", " CP_TIP_COTATIONS " from " TBL_COTATIONS
            " where " CP_IDUSER_COTATIONS " = " + QString::number(Datas::I()->users->userconnected()->id())
            + " and " CP_TYPEACTE_COTATIONS " = '" + m_codeacte + "'", ok);
        if (ok && t.size() >= 5)
        {
            m_typecotation = t.at(0).toInt();
            wdg_tarifoptamline   ->setText(QLocale().toString(t.at(1).toDouble(), 'f', 2));
            wdg_tarifnooptamline ->setText(QLocale().toString(t.at(2).toDouble(), 'f', 2));
            wdg_tarifpratiqueline->setText(QLocale().toString(t.at(3).toDouble(), 'f', 2));
            wdg_tipline          ->setPlainText(t.at(4).toString());
        }
        //! association : le code stocké réunit 2 codes CCAM séparés diversement selon le créateur
        //! (« + », «  + », «  »…). On les ré-extrait par le motif d'un code CCAM plutôt que de deviner
        //! le séparateur ; le 1er code, déjà mis à la construction, est écrasé par le vrai 1er code.
        if (m_typecotation == 2)
        {
            QStringList codes;
            QRegularExpressionMatchIterator it = Utils::rgx_cotationCCAM.globalMatch(m_codeacte);
            while (it.hasNext())
                codes << it.next().captured(0);
            if (codes.size() >= 1) wdg_codeline ->setText(codes.at(0));
            if (codes.size() >= 2) wdg_codeline2->setText(codes.at(1));
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
    if (comp1) comp1->setCaseSensitivity(Qt::CaseInsensitive);
    wdg_codeline    ->setCompleter(comp1);
    QCompleter *comp2 = assoc ? new QCompleter(m_modelCCAM, this) : Q_NULLPTR;
    if (comp2) comp2->setCaseSensitivity(Qt::CaseInsensitive);
    wdg_codeline2   ->setCompleter(comp2);

    //! le montant du haut est retitré « Montant conventionnel » en mode autre
    wdg_optamlabel  ->setText(autre ? tr("Montant conventionnel")
                                    : (m_cotationsfrance ? tr("Tarif conventionnel OPTAM") : tr("Tarif remboursé")));

    //! éditabilité : conventionnel saisi seulement en mode autre ; non-OPTAM jamais (repris/calculé) ;
    //! pratiqué figé en association (calculé), éditable en CCAM et autre ; tip non éditable en CCAM (repris)
    wdg_tarifoptamline   ->setReadOnly(!autre);
    wdg_tarifnooptamline ->setReadOnly(true);
    wdg_tarifpratiqueline->setReadOnly(assoc);
    wdg_tipline          ->setReadOnly(ccam);

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

    //! petit lecteur d'un acte CCAM : rend optam/nonoptam/libellé ; false si code vide ou inconnu
    auto litCCAM = [this] (const QString &code, double &optam, double &nonoptam, QString &nom) -> bool {
        if (code.isEmpty())
            return false;
        bool ok = false;
        QVariantList r = db->getFirstRecordFromStandardSelectSQL(
            "select " CP_MONTANTOPTAM_CCAM ", " CP_MONTANTNONOPTAM_CCAM ", " CP_NOM_CCAM
            " from " TBL_CCAM " where " CP_CODECCAM_CCAM " = '" + code + "'", ok);
        if (!ok || r.size() < 3)
            return false;
        optam    = r.at(0).toDouble();
        nonoptam = r.at(1).toDouble();
        nom      = r.at(2).toString();
        return true;
    };

    double optam = 0, nonoptam = 0;
    QString nom1;
    if (!litCCAM(wdg_codeline->text().trimmed(), optam, nonoptam, nom1))
        return;                             //! 1er code absent/inconnu : on ne remplit rien
    QStringList noms;   noms << nom1;

    //! association : le 2e acte compte pour MOITIÉ (optam1 + optam2/2, idem non-OPTAM)
    if (m_typecotation == 2)
    {
        double optam2 = 0, nonoptam2 = 0;
        QString nom2;
        if (!litCCAM(wdg_codeline2->text().trimmed(), optam2, nonoptam2, nom2))
            return;                         //! tant que le 2e code n'est pas saisi/valide, on n'écrit rien
        optam    += optam2 / 2.0;
        nonoptam += nonoptam2 / 2.0;
        noms     << nom2;
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
    const QString optamSQL    = QString::number(QLocale().toDouble(wdg_tarifoptamline->text()));
    const QString nonoptamSQL = (m_typecotation == 4) ? "null" : QString::number(QLocale().toDouble(wdg_tarifnooptamline->text()));
    const QString pratiqueSQL = QString::number(QLocale().toDouble(wdg_tarifpratiqueline->text()));
    const QString tipSQL      = Utils::correctquoteSQL(wdg_tipline->toPlainText());
    const QString codeSQL     = Utils::correctquoteSQL(code);
    const QString iduserSQL   = QString::number(iduser);
    bool ok;

    //! --- MODIFICATION : mise à jour de la cotation par son ancien code ---
    if (m_mode == Modification)
    {
        db->StandardSQL("update " TBL_COTATIONS " set "
              CP_TYPEACTE_COTATIONS " = '"       + codeSQL + "', "
              CP_MONTANTOPTAM_COTATIONS " = "    + optamSQL + ", "
              CP_MONTANTNONOPTAM_COTATIONS " = " + nonoptamSQL + ", "
              CP_MONTANTPRATIQUE_COTATIONS " = " + pratiqueSQL + ", "
              CP_TIP_COTATIONS " = '"            + tipSQL + "'"
              " where " CP_IDUSER_COTATIONS " = " + iduserSQL
              + " and " CP_TYPEACTE_COTATIONS " = '" + Utils::correctquoteSQL(m_codeacte) + "'");
        m_codeenregistre = code;
        return true;
    }

    //! --- CRÉATION, mode « autre » (type 4) : non partagé -> refus des doublons puis insertion ---
    if (m_typecotation != 1 && m_typecotation != 2)
    {
        QVariantList d = db->getFirstRecordFromStandardSelectSQL(
            "select " CP_ID_COTATIONS " from " TBL_COTATIONS " where " CP_TYPEACTE_COTATIONS " = '" + codeSQL + "' and "
            CP_IDUSER_COTATIONS " = " + iduserSQL, ok);
        if (ok && d.size() > 0)
        {
            UpMessageBox::Watch(this, msg, tr("Cet acte est déjà enregistré"));
            return false;
        }
        db->StandardSQL("insert into " TBL_COTATIONS " (" CP_TYPEACTE_COTATIONS ", " CP_MONTANTOPTAM_COTATIONS ", "
              CP_MONTANTNONOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS ", " CP_TYPECOTATION_COTATIONS ", "
              CP_IDUSER_COTATIONS ", " CP_TIP_COTATIONS ") VALUES ('"
              + codeSQL + "', " + optamSQL + ", " + nonoptamSQL + ", " + pratiqueSQL + ", "
              + QString::number(m_typecotation) + ", " + iduserSQL + ", '" + tipSQL + "')");
        m_codeenregistre = code;
        return true;
    }

    //! --- CRÉATION, modes CCAM (1) / association (2) : la table cotations est PARTAGÉE ---
    //! l'acte existe déjà -> on ne le duplique pas, on rafraîchit seulement ses montants conventionnels ;
    //! sinon on l'insère. Puis on crée (ou met à jour) la jointure du user = son montant pratiqué.
    QVariantList ex = db->getFirstRecordFromStandardSelectSQL(
        "select min(" CP_ID_COTATIONS ") from " TBL_COTATIONS " where " CP_TYPEACTE_COTATIONS " = '" + codeSQL + "'", ok);
    int idcot = (ok && ex.size() > 0 && !ex.at(0).isNull()) ? ex.at(0).toInt() : 0;
    if (idcot > 0)
        db->StandardSQL("update " TBL_COTATIONS " set "
                        CP_MONTANTOPTAM_COTATIONS " = "    + optamSQL + ", "
                        CP_MONTANTNONOPTAM_COTATIONS " = " + nonoptamSQL
                        + " where " CP_ID_COTATIONS " = " + QString::number(idcot));
    else
    {
        db->StandardSQL("insert into " TBL_COTATIONS " (" CP_TYPEACTE_COTATIONS ", " CP_MONTANTOPTAM_COTATIONS ", "
              CP_MONTANTNONOPTAM_COTATIONS ", " CP_MONTANTPRATIQUE_COTATIONS ", " CP_TYPECOTATION_COTATIONS ", "
              CP_IDUSER_COTATIONS ", " CP_TIP_COTATIONS ") VALUES ('"
              + codeSQL + "', " + optamSQL + ", " + nonoptamSQL + ", " + pratiqueSQL + ", "
              + QString::number(m_typecotation) + ", " + iduserSQL + ", '" + tipSQL + "')");
        QVariantList n = db->getFirstRecordFromStandardSelectSQL(
            "select min(" CP_ID_COTATIONS ") from " TBL_COTATIONS " where " CP_TYPEACTE_COTATIONS " = '" + codeSQL + "'", ok);
        idcot = (ok && n.size() > 0 && !n.at(0).isNull()) ? n.at(0).toInt() : 0;
    }

    //! jointure du user pour cette cotation : créée si absente, sinon son montant pratiqué mis à jour
    if (idcot > 0)
    {
        const QString jointure    = (m_typecotation == 1) ? TBL_JOINTURESCCAM            : TBL_JOINTURESASSOCIATIONS;
        const QString chpIdcot    = (m_typecotation == 1) ? CP_IDCOTATION_JOINTCCAM      : CP_IDCOTATION_JOINTASSOCIATIONS;
        const QString chpIduser   = (m_typecotation == 1) ? CP_IDUSER_JOINTCCAM          : CP_IDUSER_JOINTASSOCIATIONS;
        const QString chpPratique = (m_typecotation == 1) ? CP_MONTANTPRATIQUE_JOINTCCAM : CP_MONTANTPRATIQUE_JOINTASSOCIATIONS;
        const QString wId = " where " + chpIdcot + " = " + QString::number(idcot) + " and " + chpIduser + " = " + iduserSQL;
        QVariantList j = db->getFirstRecordFromStandardSelectSQL("select " + chpPratique + " from " + jointure + wId, ok);
        if (ok && j.size() > 0)
            db->StandardSQL("update " + jointure + " set " + chpPratique + " = " + pratiqueSQL + wId);
        else
            db->StandardSQL("insert into " + jointure + " (" + chpIdcot + ", " + chpIduser + ", " + chpPratique + ") values ("
                            + QString::number(idcot) + ", " + iduserSQL + ", " + pratiqueSQL + ")");
    }

    m_codeenregistre = code;
    return true;
}
