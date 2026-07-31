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

#include "dlg_paramconnexion.h"
#include "ui_dlg_paramconnexion.h"
#include "mysqlinstaller.h"     //! motDePasseSQL() : mdp MySQL du cabinet (repli legacy MDP_SQL)
#include <QFileDialog>

dlg_paramconnexion::dlg_paramconnexion(QWidget *parent) :
    UpDialog(parent),
    ui(new Ui::dlg_paramconnexion)
{
    ui->setupUi(this);
    //! Look Rufus : on hérite de UpDialog (fenêtre stylée, sans la barre de titre « brute » de l'OS).
    //! Le .ui ayant un layout de premier niveau, on REMONTE ses cadres dans le layout de UpDialog
    //! (dlglayout) — chaque insertWidget(0,…) reparente le cadre, l'ordre final (haut→bas) étant :
    //! login, mode+IP, port, boutons. On garde les boutons du .ui (OK/Annuler) et leurs connexions ;
    //! on NE refixe PAS setWindowFlags (UpDialog gère sa propre fenêtre stylée).
    dlglayout()->insertWidget(0, ui->ButtonsFrame);
    dlglayout()->insertWidget(0, ui->prtFrame);
    dlglayout()->insertWidget(0, ui->MainFrame);
    dlglayout()->insertWidget(0, ui->frame);

    ui->PortcomboBox        ->addItems(QStringList() << "3306" << "3307");   
    ui->AccesgroupBox       ->setFocusProxy(ui->PosteradioButton);
    ui->OKuppushButton      ->setShortcut(QKeySequence("Meta+Return"));

    ui->LoginlineEdit   ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_15,this));
    ui->MDPlineEdit     ->setValidator(new QRegularExpressionValidator(Utils::rgx_AlphaNumeric_5_12,this));
    //! Seul contrôle conservé sur le champ IP : un validateur d'adresse IPv4. Toute
    //! l'ancienne mécanique (masque de saisie, normalisation à zéros, recalcul de l'IP
    //! au focus-out) est supprimée — c'est elle qui faisait resurgir l'IP du mode précédent.
    ui->IPlineEdit      ->setValidator(new QRegularExpressionValidator(Utils::rgx_IPV4_mask,this));

    connect(ui->AnnuluppushButton,          &QPushButton::clicked,          this,   &QDialog::reject);
    connect(ui->OKuppushButton,             &QPushButton::clicked,          this,   &dlg_paramconnexion::Verif);
    connect(ui->LocalradioButton,           &QRadioButton::clicked,         this,   [=, this] {RegleAffichage(ui->LocalradioButton);});
    connect(ui->PosteradioButton,           &QRadioButton::clicked,         this,   [=, this] {RegleAffichage(ui->PosteradioButton);});
    connect(ui->DistantradioButton,         &QRadioButton::clicked,         this,   [=, this] {RegleAffichage(ui->DistantradioButton);});
    connect(ui->ClesSSLuppushButton,        &QPushButton::clicked,          this,   &dlg_paramconnexion::DossierClesSSL);

    ui->ClesSSLLineEdit ->useselftextastooltip();
    QString dir = QDir::homePath();
    if (dir == "" || !QDir(dir).exists())
        ui->ClesSSLLineEdit ->clear();
    else ui->ClesSSLLineEdit->setText(dir);
    // État initial cohérent : IPFrame ET les widgets SSL masqués tant qu'aucun mode réseau
    // n'est choisi (RegleAffichage centralise cette logique). Combiné au sizeConstraint
    // SetFixedSize du layout (cf. .ui), la fiche s'ouvre à la bonne taille puis se
    // redimensionne automatiquement à chaque changement de mode.
    RegleAffichage(ui->PosteradioButton);
}

dlg_paramconnexion::~dlg_paramconnexion()
{
    delete ui;
}

void dlg_paramconnexion::DossierClesSSL()
{
    QString dir = ui->ClesSSLLineEdit->text();
    if (dir == "" || !QDir(dir).exists())
        ui->ClesSSLLineEdit ->clear();
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList()<<dir,false);
    if (url == QUrl())
        return;
    //! On renseigne le champ ; c'est VerifParamConnexion qui l'enregistrera avec le reste.
    ui->ClesSSLLineEdit->setText(url.path());
    ui->ClesSSLLineEdit ->setImmediateToolTip(ui->ClesSSLLineEdit->text());
}

void dlg_paramconnexion::RegleAffichage(QRadioButton *butt)
{
    ui->IPFrame             ->setVisible(butt!=ui->PosteradioButton);
    ui->ClesSSLuppushButton ->setVisible(butt == ui->DistantradioButton);
    ui->ClesSSLLabel        ->setVisible(butt == ui->DistantradioButton);
    ui->ClesSSLLineEdit     ->setVisible(butt == ui->DistantradioButton);
    if (butt == ui->DistantradioButton)
    {
        // Lien ACTIF et coloré : on passe l'URL en HTML (<a href>) ET en 5e paramètre « link »
        // de Watch, qui active alors l'ouverture externe (setOpenExternalLinks) et le clic.
        // Le href est retiré par convertPlainText au calcul de la taille → pas d'élargissement.
        const QString url = "https://www.rufusvision.org/configuration-pour-une-connexion-par-internet---le-cryptage-ssl.html";
        UpMessageBox::Watch(this, tr("Informations importantes sur l'accès par internet"),
                                tr("Pour des raisons de confidentialité, l'accès distant dans Rufus "
                                  "fonctionne obligatoirement avec un cryptage de données.") + "\n\n" +
                                tr("Si vous voulez utiliser l'accès distant sur ce poste, il faut que:") + "\n" +
                                tr("1. le serveur MySQL soit configuré pour le cryptage") + "\n" +
                                tr("2. que sur ce poste les clés clientes SSL soient installées.") + "\n\n" +
                                tr("Reportez-vous à la page internet :") + "\n" +
                                "<a href=\"" + url + "\">" + url + "</a>" + "\n" +
                                tr("pour savoir comment modifier la configuration du serveur et générer des clés de cryptage."),
                                UpDialog::ButtonOK, url);
    }
}

/*!
 * \brief dlg_paramconnexion::Verif
 * Bouton OK : la fiche se ferme dès que la saisie est complète, sans aucun test de connexion.
 */
void dlg_paramconnexion::Verif()
{
    if (VerifFiche())
        accept();
}

//! Base sécurisée sur un autre poste : propose de récupérer le mot de passe
//! aléatoire du cabinet, soit en important le fichier copié sur une clé USB depuis
//! un poste qui fonctionne, soit en le saisissant. En cas de succès, l'enregistre
//! (.dbkey + cache) et renvoie true pour signaler qu'un nouvel essai est possible.
dlg_paramconnexion::IssueMdp
dlg_paramconnexion::RecupererMotDePasseMySQL(QWidget *parent, const QString &titre,
                                             const QString &corps, bool monoposte)
{
    UpMessageBox msgbox(parent);
    msgbox.setText(!titre.isEmpty() ? titre : tr("Base de données sécurisée"));
    msgbox.setInformativeText(!corps.isEmpty() ? corps
                              : tr("Aucun mot de passe connu ne permet de se connecter à cette base : "
                                 "elle a été sécurisée sur un autre poste.\n\n"
                                 "Vous pouvez récupérer le mot de passe du cabinet copié sur une clé USB "
                                 "depuis un poste qui fonctionne, ou le saisir si vous le connaissez."));
    msgbox.setIcon(UpMessageBox::Warning);
    UpSmallButton *AnnulBouton  = new UpSmallButton();
    UpSmallButton *SaisirBouton = new UpSmallButton();
    UpSmallButton *USBBouton    = new UpSmallButton();
    AnnulBouton ->setText(tr("Annuler"));
    SaisirBouton->setText(tr("Saisir le mot de passe"));
    USBBouton   ->setText(tr("Importer depuis une clé USB"));
    msgbox.addButton(AnnulBouton,  UpSmallButton::CLOSEBUTTON);
    //! Deux issues de plus sur le poste hôte : récupérer la base par le mot de passe de secours, ou
    //! repartir d'une base neuve. Un client ne répare pas la base des autres.
    UpSmallButton *PasserBouton = nullptr;
    UpSmallButton *ReinitBouton = nullptr;
    if (monoposte)
    {
        PasserBouton = new UpSmallButton();
        PasserBouton->setText(tr("Je n'ai aucun mot de passe"));
        msgbox.addButton(PasserBouton, UpSmallButton::SKIPBUTTON);
        ReinitBouton = new UpSmallButton();
        ReinitBouton->setText(tr("Réinitialiser le programme"));
        msgbox.addButton(ReinitBouton, UpSmallButton::OUPSBUTTON);
    }
    msgbox.addButton(SaisirBouton, UpSmallButton::KEYBOARDBUTTON);
    msgbox.addButton(USBBouton,    UpSmallButton::RECORDBUTTON);
    msgbox.exec();

    if (PasserBouton && msgbox.clickedButton() == PasserBouton)
        return IssueMdp::Inconnu;
    if (ReinitBouton && msgbox.clickedButton() == ReinitBouton)
        return IssueMdp::Reinitialiser;

    QString mdp;
    if (msgbox.clickedButton() == USBBouton)
    {
        const QString fichier = QFileDialog::getOpenFileName(
                    parent, tr("Sélectionnez le fichier du mot de passe sur la clé USB"));
        if (fichier.isEmpty())
            return IssueMdp::EchecSaisie;
        QFile f(fichier);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            mdp = QString::fromUtf8(f.readAll()).trimmed();
            f.close();
        }
        //! GARDE-FOU : un fichier de mot de passe valide tient sur UNE seule ligne. Si on pointe par
        //! erreur sur un AUTRE fichier (typiquement un .dbkey « MONO=… / WAN=… », ou tout texte
        //! multi-lignes), on le REFUSE — on n'enregistre JAMAIS un contenu aberrant comme mot de passe
        //! (un mdp multi-lignes finissait stocké tel quel dans le .dbkey et provoquait des dégâts).
        if (mdp.contains('\n') || mdp.contains('\r')
            || mdp.startsWith("MONO=",   Qt::CaseInsensitive)
            || mdp.startsWith("LAN=",    Qt::CaseInsensitive)
            || mdp.startsWith("WAN=",    Qt::CaseInsensitive)
            || mdp.startsWith("MDPSQL=", Qt::CaseInsensitive))
            mdp.clear();
        if (mdp.isEmpty())
        {
            UpMessageBox::Watch(parent, tr("Fichier illisible"),
                                tr("Ce fichier ne contient pas un mot de passe valide."));
            return IssueMdp::EchecSaisie;
        }
    }
    else if (msgbox.clickedButton() == SaisirBouton)
    {
        UpDialog dlg(parent);
        dlg.setWindowTitle(tr("Mot de passe de la base"));
        UpLabel *lbl = new UpLabel();
        lbl->setText(tr("Entrez le mot de passe MySQL du cabinet :"));
        dlg.dlglayout()->insertWidget(0, lbl);
        UpLineEdit *champ = new UpLineEdit(&dlg);
        champ->setAlignment(Qt::AlignCenter);
        champ->setEchoMode(QLineEdit::Password);   //! mot de passe masqué à la saisie
        dlg.dlglayout()->insertWidget(1, champ);
        dlg.AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
        connect(dlg.OKButton, &QPushButton::clicked, &dlg, &QDialog::accept);
        champ->setFocus();
        if (dlg.exec() != QDialog::Accepted)
            return IssueMdp::EchecSaisie;
        mdp = champ->text().trimmed();
        if (mdp.isEmpty())
            return IssueMdp::EchecSaisie;
    }
    else
        return IssueMdp::Annule;

    //! Éprouvé avant d'écrire le .dbkey : sinon une saisie erronée effaçait l'ancien mot de passe,
    //! parfois l'unique copie du cabinet.
    const QString err = DataBase::I()->connectToDataBase(DB_RUFUS, LOGIN_SQL, mdp);
    if (!err.isEmpty())
    {
        UpMessageBox::Watch(parent, tr("Mot de passe incorrect"),
            tr("Ce mot de passe ne permet pas de se connecter à la base de données."));
        return IssueMdp::EchecSaisie;
    }
    MySQLInstaller::stockerMotDePasse(mdp);
    return IssueMdp::Obtenu;
}

/*!
 * \brief dlg_paramconnexion::VerifFiche
 * Saisie suffisante pour écrire un Rufus.ini valide ? Login et mot de passe, l'adresse du serveur hors
 * monoposte, le dossier des clés SSL en distant.
 */
bool dlg_paramconnexion::VerifFiche()
{
    if (ui->LoginlineEdit->text() == "")
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas précisé le login."));
        ui->LoginlineEdit->setFocus();
        return false;
    }
    if (ui->MDPlineEdit->text() == "")
    {
        UpMessageBox::Watch(this,tr( "Vous n'avez pas précisé le mot de passe."));
        ui->MDPlineEdit->setFocus();
        return false;
    }
    QList <QRadioButton*> listboutons = ui->AccesgroupBox->findChildren<QRadioButton*>();
    bool a = false;
    for (int i=0; i<listboutons.size(); i++)
    {
        if (listboutons.at(i)->isChecked())
        {
            a = true;
            break;
        }
    }
    if (!a)
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas précisé le mode d'accès."));
        return false;
    }
    if ((ui->LocalradioButton->isChecked() || ui->DistantradioButton->isChecked())
            && ui->IPlineEdit->text() == "")
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas précisé l'adresse du serveur."));
        return false;
    }
    if (ui->DistantradioButton->isChecked()
            && (ui->ClesSSLLineEdit->text() == "" || !QDir(ui->ClesSSLLineEdit->text()).exists()))
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas précisé d'adresse valide pour les clés SSL."));
        return false;
    }
    return true;
}
