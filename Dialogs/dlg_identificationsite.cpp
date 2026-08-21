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

#include "dlg_identificationsite.h"

dlg_identificationsite::dlg_identificationsite(Site *sit, Mode mode, QWidget *parent)
    : UpDialog(parent), m_site(sit), m_mode(mode)
{
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    setWindowModality(Qt::WindowModal);
    setWindowTitle(tr("Lieu d'exercice"));

    QVBoxLayout *laylbl     = new QVBoxLayout();
    QVBoxLayout *layledit   = new QVBoxLayout();
    QHBoxLayout *laycom     = new QHBoxLayout();
    QVBoxLayout *lay        = qobject_cast<QVBoxLayout*>(layout());

    UpLabel *lblnom         = new UpLabel(this, tr("Nom de la structure"));
    UpLabel *lbladr1        = new UpLabel(this, tr("Adresse1"));
    UpLabel *lbladr2        = new UpLabel(this, tr("Adresse2"));
    UpLabel *lbladr3        = new UpLabel(this, tr("Adresse3"));
    UpLabel *lblcp          = new UpLabel(this, tr("Code postal"));
    UpLabel *lblville       = new UpLabel(this, tr("Ville"));
    UpLabel *lbltel         = new UpLabel(this, tr("Telephone"));
    UpLabel *lblfax         = new UpLabel(this, tr("Fax"));
    UpLabel *lblmail        = new UpLabel(this, tr("Mail"));
    UpLabel *lblsmtpserveur = new UpLabel(this, tr("Serveur d'envoi"));
    UpLabel *lblsmtpport    = new UpLabel(this, tr("Port"));
    UpLabel *lblsmtplogin   = new UpLabel(this, tr("Login du compte mail"));

    int h = 22;
    foreach (UpLabel *lbl, findChildren<UpLabel*>())
        lbl                 ->setFixedHeight(h);

    laylbl                  ->addWidget(lblnom);
    laylbl                  ->addWidget(lbladr1);
    laylbl                  ->addWidget(lbladr2);
    laylbl                  ->addWidget(lbladr3);
    laylbl                  ->addWidget(lblcp);
    laylbl                  ->addWidget(lblville);
    laylbl                  ->addWidget(lbltel);
    laylbl                  ->addWidget(lblfax);
    laylbl                  ->addWidget(lblmail);
    laylbl                  ->addWidget(lblsmtpserveur);
    laylbl                  ->addWidget(lblsmtpport);
    laylbl                  ->addWidget(lblsmtplogin);
    laylbl                  ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));

    wdg_nomlineedit         = new UpLineEdit(this);
    wdg_adress1lineedit     = new UpLineEdit(this);
    wdg_adress2lineedit     = new UpLineEdit(this);
    wdg_adress3lineedit     = new UpLineEdit(this);
    wdg_CPlineedit          = new UpLineEdit(this);
    wdg_villelineedit       = new UpLineEdit(this);
    wdg_tellineedit         = new UpLineEdit(this);
    wdg_faxlineedit         = new UpLineEdit(this);
    wdg_maillineedit        = new UpLineEdit(this);
    wdg_smtpserveurlineedit = new UpLineEdit(this);
    wdg_smtpportlineedit    = new UpLineEdit(this);
    wdg_smtploginlineedit   = new UpLineEdit(this);

    wdg_nomlineedit         ->setFixedWidth(240);
    wdg_adress1lineedit     ->setFixedWidth(240);
    wdg_adress2lineedit     ->setFixedWidth(240);
    wdg_adress3lineedit     ->setFixedWidth(240);
    wdg_CPlineedit          ->setFixedWidth(90);
    wdg_villelineedit       ->setFixedWidth(240);
    wdg_tellineedit         ->setFixedWidth(120);
    wdg_faxlineedit         ->setFixedWidth(120);
    wdg_maillineedit        ->setFixedWidth(240);
    wdg_smtpserveurlineedit ->setFixedWidth(240);
    wdg_smtpportlineedit    ->setFixedWidth(90);
    wdg_smtploginlineedit   ->setFixedWidth(240);

    wdg_nomlineedit         ->setMaxLength(80);
    wdg_adress1lineedit     ->setMaxLength(45);
    wdg_adress2lineedit     ->setMaxLength(45);
    wdg_adress3lineedit     ->setMaxLength(45);
    wdg_CPlineedit          ->setMaxLength(9);
    wdg_villelineedit       ->setMaxLength(45);
    wdg_tellineedit         ->setMaxLength(17);
    wdg_faxlineedit         ->setMaxLength(17);
    wdg_maillineedit        ->setMaxLength(100);
    wdg_smtpserveurlineedit ->setMaxLength(100);
    wdg_smtpportlineedit    ->setMaxLength(5);
    wdg_smtploginlineedit   ->setMaxLength(100);

    wdg_nomlineedit         ->setValidator(new QRegularExpressionValidator(Utils::rgx_ville));
    wdg_adress1lineedit     ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse));
    wdg_adress2lineedit     ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse));
    wdg_adress3lineedit     ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresse));
    wdg_CPlineedit          ->setValidator(new QRegularExpressionValidator(Utils::rgx_CP));
    wdg_villelineedit       ->setValidator(new QRegularExpressionValidator(Utils::rgx_ville));
    wdg_tellineedit         ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone));
    wdg_faxlineedit         ->setValidator(new QRegularExpressionValidator(Utils::rgx_telephone));
    wdg_maillineedit        ->setValidator(new QRegularExpressionValidator(Utils::rgx_mail));
    wdg_smtpserveurlineedit ->setValidator(new QRegularExpressionValidator(Utils::rgx_adresseserveur));
    wdg_smtpportlineedit    ->setValidator(new QIntValidator(1, 65535));

    wdg_maillineedit        ->setImmediateToolTip(tr("L'adresse d'où partiront les documents envoyés par mail."));
    wdg_smtpserveurlineedit ->setImmediateToolTip(tr("Le serveur d'envoi de votre fournisseur de mail, en général smtp. suivi de son nom:")
                                           + "<br>smtp.sfr.fr, smtp.orange.fr, smtp.free.fr, ssl0.ovh.net, smtp.gmail.com<br>"
                                           + tr("Il figure dans les réglages de votre webmail, à la rubrique POP/IMAP."));
    wdg_smtpportlineedit    ->setImmediateToolTip(tr("465 si la liaison est chiffrée dès la connexion, 587 sinon.")
                                           + "<br>" + tr("En cas de doute, essayez 465."));
    wdg_smtploginlineedit   ->setImmediateToolTip(tr("Ce que vous tapez pour vous connecter à votre messagerie, en général l'adresse mail complète.")
                                           + "<br>" + tr("Chez Gmail et Outlook, le mot de passe habituel est refusé: il faut créer un mot de passe d'application dans les réglages du compte."));

    AjouteWidgetLayButtons(Utils::BoutonAide(
        tr("Les quatre derniers champs servent à envoyer les documents par mail depuis ce lieu.") + "<br><br>"
        + "<b>" + tr("Mail") + "</b><br>"
        + tr("L'adresse d'où partiront les documents. C'est elle que verra le destinataire, et c'est à elle qu'il répondra.") + "<br><br>"
        + "<b>" + tr("Serveur d'envoi") + "</b><br>"
        + tr("Le serveur par lequel votre fournisseur de mail accepte d'expédier. Son nom commence presque toujours par smtp:") + "<br>"
        + tr("SFR") + " smtp.sfr.fr &nbsp;&nbsp; " + tr("Orange") + " smtp.orange.fr &nbsp;&nbsp; " + tr("Free") + " smtp.free.fr<br>"
        + tr("OVH") + " ssl0.ovh.net &nbsp;&nbsp; " + tr("Gmail") + " smtp.gmail.com &nbsp;&nbsp; " + tr("Outlook") + " smtp-mail.outlook.com<br>"
        + tr("Vous le trouverez aussi dans les réglages de votre webmail, à la rubrique POP/IMAP.") + "<br><br>"
        + "<b>" + tr("Port") + "</b><br>"
        + tr("Le numéro par lequel on entre sur ce serveur. 465 quand la liaison est chiffrée dès la connexion, 587 quand elle l'est juste après.") + "<br>"
        + tr("En cas de doute, essayez 465 puis 587.") + "<br><br>"
        + "<b>" + tr("Login du compte mail") + "</b><br>"
        + tr("Ce que vous tapez pour vous connecter à votre messagerie. Chez la plupart des fournisseurs, c'est l'adresse mail complète et non la partie qui précède l'arobase.") + "<br><br>"
        + tr("Le mot de passe n'est pas ici: il n'est jamais enregistré dans la base, il reste sur l'ordinateur qui envoie et vous est demandé au premier envoi.") + "<br>"
        + tr("Chez Gmail et Outlook, le mot de passe habituel est refusé: il faut créer un mot de passe d'application dans les réglages du compte."),
        tr("À quoi servent ces champs ?")), false);

    layledit                ->addWidget(wdg_nomlineedit);
    layledit                ->addWidget(wdg_adress1lineedit);
    layledit                ->addWidget(wdg_adress2lineedit);
    layledit                ->addWidget(wdg_adress3lineedit);
    layledit                ->addWidget(wdg_CPlineedit);
    layledit                ->addWidget(wdg_villelineedit);
    layledit                ->addWidget(wdg_tellineedit);
    layledit                ->addWidget(wdg_faxlineedit);
    layledit                ->addWidget(wdg_maillineedit);
    layledit                ->addWidget(wdg_smtpserveurlineedit);
    layledit                ->addWidget(wdg_smtpportlineedit);
    layledit                ->addWidget(wdg_smtploginlineedit);
    layledit                ->addSpacerItem(new QSpacerItem(5,5,QSizePolicy::Expanding,QSizePolicy::Expanding));

    wdg_couleurpushbutt     = new UpPushButton();
    wdg_couleurpushbutt     ->setFixedHeight(35);
    wdg_couleurpushbutt     ->setText(m_mode == Nouv? tr("choisir la couleur du texte") : tr("modifier la couleur du texte"));

    laycom                  ->addLayout(laylbl);
    laylbl                  ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Fixed,QSizePolicy::Fixed));
    laycom                  ->addLayout(layledit);

    lay                     ->insertLayout(0, laycom);
    lay                     ->insertWidget(1, wdg_couleurpushbutt);
    lay                     ->setSizeConstraint(QLayout::SetFixedSize);

    if (m_site != nullptr)
    {
        wdg_nomlineedit         ->setText(m_site->nom());
        wdg_adress1lineedit     ->setText(m_site->adresse1());
        wdg_adress2lineedit     ->setText(m_site->adresse2());
        wdg_adress3lineedit     ->setText(m_site->adresse3());
        wdg_CPlineedit          ->setText(m_site->codepostal());
        wdg_villelineedit       ->setText(m_site->ville());
        wdg_tellineedit         ->setText(m_site->telephone());
        wdg_faxlineedit         ->setText(m_site->fax());
        wdg_maillineedit        ->setText(m_site->mail());
        wdg_smtpserveurlineedit ->setText(m_site->smtpserveur());
        wdg_smtpportlineedit    ->setText(QString::number(m_site->smtpport() == 0? 587 : m_site->smtpport()));
        wdg_smtploginlineedit   ->setText(m_site->smtplogin());
        str_nouvcolor           = m_site->couleur();
    }
    else
        wdg_smtpportlineedit    ->setText("587");   /*!< 465 = connexion chiffrée d'emblée, sinon chiffrement après connexion */

    if (m_mode == Complement)   /*! on ne vient que compléter les coordonnées d'envoi */
    {
        wdg_nomlineedit         ->setEnabled(false);
        wdg_adress1lineedit     ->setEnabled(false);
        wdg_adress2lineedit     ->setEnabled(false);
        wdg_adress3lineedit     ->setEnabled(false);
        wdg_CPlineedit          ->setEnabled(false);
        wdg_villelineedit       ->setEnabled(false);
        wdg_tellineedit         ->setEnabled(false);
        wdg_faxlineedit         ->setEnabled(false);
        wdg_couleurpushbutt     ->setEnabled(false);
        wdg_maillineedit        ->setFocus();
    }
    else
        connect(wdg_couleurpushbutt,    &QPushButton::clicked,  this,   &dlg_identificationsite::ModifCouleur);

    foreach (UpLineEdit *led, findChildren<UpLineEdit*>())
        connect(led,            &QLineEdit::textEdited,         this,   [=, this] { OKButton->setEnabled(true); });
    OKButton                ->setEnabled(false);
    connect(OKButton,           &QPushButton::clicked,          this,   &dlg_identificationsite::Valide);
}

void dlg_identificationsite::ModifCouleur()
{
    QColor colordep = QColor("#FF" + str_nouvcolor);
    QColor colorfin = Utils::SelectCouleur(colordep, this);
    if (!colorfin.isValid())
        return;
    str_nouvcolor = colorfin.name().replace("#","");
    OKButton                ->setEnabled(true);
}

/*!
 * \brief dlg_identificationsite::Valide
 * Contrôle les champs obligatoires, remplit listbinds et ferme la fiche.
 */
void dlg_identificationsite::Valide()
{
    QString Msg = tr("Vous n'avez pas spécifié ");
    if (wdg_nomlineedit->text() == "" || wdg_nomlineedit->text() == tr("non défini"))
    {
        wdg_nomlineedit     ->setFocus();
        UpMessageBox::Watch(this, Msg + tr("le nom de la structure de soins"));
        return;
    }
    if (wdg_CPlineedit->text().toInt() == 0)
    {
        wdg_CPlineedit      ->setFocus();
        UpMessageBox::Watch(this, Msg + tr("le code postal"));
        return;
    }
    if (wdg_villelineedit->text() == "")
    {
        wdg_villelineedit   ->setFocus();
        UpMessageBox::Watch(this, Msg + tr("la ville"));
        return;
    }
    m_listbinds[CP_NOM_SITE]            = wdg_nomlineedit->text();
    m_listbinds[CP_ADRESSE1_SITE]       = Utils::trimcapitilize(wdg_adress1lineedit->text().toUpper(), true);
    m_listbinds[CP_ADRESSE2_SITE]       = Utils::trimcapitilize(wdg_adress2lineedit->text(), true);
    m_listbinds[CP_ADRESSE3_SITE]       = Utils::trimcapitilize(wdg_adress3lineedit->text(), true);
    m_listbinds[CP_CODEPOSTAL_SITE]     = Utils::trim(wdg_CPlineedit->text());
    m_listbinds[CP_VILLE_SITE]          = Utils::trimcapitilize(wdg_villelineedit->text(), true);
    m_listbinds[CP_TELEPHONE_SITE]      = Utils::trim(wdg_tellineedit->text());
    m_listbinds[CP_FAX_SITE]            = Utils::trim(wdg_faxlineedit->text());
    m_listbinds[CP_COULEUR_SITE]        = str_nouvcolor;
    m_listbinds[CP_MAIL_SITE]           = Utils::trim(wdg_maillineedit->text());
    m_listbinds[CP_SMTPSERVEUR_SITE]    = Utils::trim(wdg_smtpserveurlineedit->text());
    m_listbinds[CP_SMTPPORT_SITE]       = wdg_smtpportlineedit->text().toInt();
    m_listbinds[CP_SMTPLOGIN_SITE]      = Utils::trim(wdg_smtploginlineedit->text());
    accept();
}
