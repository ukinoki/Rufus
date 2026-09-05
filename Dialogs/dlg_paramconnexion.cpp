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

#include <QButtonGroup>
#include <QSettings>
#include "dlg_paramconnexion.h"
#include "mysqlinstaller.h"
#include "upgroupbox.h"

dlg_paramconnexion::dlg_paramconnexion(QWidget *parent) :
    UpDialog(parent)
{
    setWindowTitle(tr("Paramètrage de connexion à la base"));

    QVBoxLayout *lay    = new QVBoxLayout();
    lay                 ->setContentsMargins(5, 5, 5, 5);
    lay                 ->setSpacing(5);

    /*! --- emplacement du serveur : trois modes exclusifs --- */
    wdg_posteradio      = new QRadioButton(tr("Sur ce poste"));
    wdg_localradio      = new QRadioButton(tr("Réseau local"));
    wdg_distantradio    = new QRadioButton(tr("Accès distant"));
    UpGroupBox *groupe  = new UpGroupBox();
    groupe              ->setTitle(tr("Emplacement du serveur"));
    groupe              ->setFocusProxy(wdg_posteradio);
    QVBoxLayout *acceslay = new QVBoxLayout;
    acceslay            ->setContentsMargins(10, 28, 10, 10);   /*!< 28 en haut : le stylesheet UpGroupBox ne réserve pas la place du titre */
    for (QRadioButton *radio : {wdg_posteradio, wdg_localradio, wdg_distantradio})
        acceslay        ->addWidget(radio);
    groupe              ->setLayout(acceslay);
    groupe              ->setMinimumWidth(350);   /*!< seul bloc toujours visible : il tient la largeur de la fiche, titre compris */
    lay                 ->addWidget(groupe);

    /*! --- adresse du serveur et clés SSL : visibles selon le mode --- */
    UpLabel *iplabel    = new UpLabel();
    iplabel             ->setText(tr("Adresse IP du serveur"));
    wdg_iplineedit      = new UpLineEdit();
    wdg_iplineedit      ->setAlignment(Qt::AlignCenter);
    /*! Seul contrôle sur le champ IP : un validateur IPv4. L'ancienne mécanique (masque, normalisation
     *  à zéros, recalcul au focus-out) faisait resurgir l'IP du mode précédent. */
    wdg_iplineedit      ->setValidator(new QRegularExpressionValidator(Utils::rgx_IPV4_mask, this));

    wdg_clesSSLlabel    = new UpLabel();
    wdg_clesSSLlabel    ->setText(tr("Emplacement clés SSL"));
    wdg_clesSSLlineedit = new UpLineEdit();
    wdg_clesSSLlineedit ->setAlignment(Qt::AlignCenter);
    wdg_clesSSLlineedit ->useselftextastooltip();
    wdg_clesSSLbouton   = new QPushButton("...");
    wdg_clesSSLbouton   ->setFixedSize(54, 32);
    wdg_clesSSLbouton   ->setContextMenuPolicy(Qt::NoContextMenu);
    QHBoxLayout *clesSSLlay = new QHBoxLayout;
    clesSSLlay          ->setContentsMargins(0, 0, 0, 0);
    clesSSLlay          ->addWidget(wdg_clesSSLlineedit, 1);
    clesSSLlay          ->addWidget(wdg_clesSSLbouton, 0);

    wdg_importbouton    = new UpPushButton(tr("Importer les données de connexion"));
    wdg_importbouton    ->setImmediateToolTip(Utils::tipImportDonneesConnexion());

    wdg_ipframe         = new QFrame();
    wdg_ipframe         ->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *iplay  = new QVBoxLayout;
    iplay               ->addWidget(iplabel);
    iplay               ->addWidget(wdg_iplineedit);
    iplay               ->addWidget(wdg_clesSSLlabel);
    iplay               ->addLayout(clesSSLlay);
    iplay               ->addWidget(wdg_importbouton);
    wdg_ipframe         ->setLayout(iplay);
    lay                 ->addWidget(wdg_ipframe);

    /*! --- port --- */
    UpLabel *portlabel  = new UpLabel();
    portlabel           ->setText(tr("Port"));
    wdg_portcombo       = new QComboBox();
    wdg_portcombo       ->addItems(QStringList() << "3306" << "3307");
    wdg_portcombo       ->setFixedWidth(110);
    QFrame *portframe   = new QFrame();
    portframe           ->setFrameShape(QFrame::StyledPanel);
    QHBoxLayout *portlay = new QHBoxLayout;
    portlay             ->addWidget(portlabel);
    portlay             ->addWidget(wdg_portcombo);
    portlay             ->addStretch(1);
    portframe           ->setLayout(portlay);
    lay                 ->addWidget(portframe);

    AjouteLayButtons(UpDialog::ButtonOK | UpDialog::ButtonCancel);
    dlglayout()         ->insertLayout(0, lay);
    dlglayout()         ->setSizeConstraint(QLayout::SetFixedSize);

    connect(OKButton,           &QPushButton::clicked,  this, &dlg_paramconnexion::Verif);
    connect(CancelButton,       &QPushButton::clicked,  this, &QDialog::reject);
    connect(wdg_clesSSLbouton,  &QPushButton::clicked,  this, &dlg_paramconnexion::DossierClesSSL);
    connect(wdg_importbouton,   &QPushButton::clicked,  this, &dlg_paramconnexion::ImporterDonneesConnexion);
    for (QRadioButton *radio : {wdg_posteradio, wdg_localradio, wdg_distantradio})
        connect(radio, &QRadioButton::clicked, this, [=, this] {RegleAffichage(radio);});

    const QString dir = QDir::homePath();
    if (dir != "" && QDir(dir).exists())
        wdg_clesSSLlineedit ->setText(dir);
    wdg_posteradio      ->setChecked(true);
    RegleAffichage(wdg_posteradio);
}

void dlg_paramconnexion::DossierClesSSL()
{
    QString dir = wdg_clesSSLlineedit->text();
    if (dir == "" || !QDir(dir).exists())
        wdg_clesSSLlineedit ->clear();
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(dir), QStringList()<<dir);
    if (url == QUrl())
        return;
    /*! On renseigne le champ ; c'est VerifParamConnexion qui l'enregistrera avec le reste. */
    wdg_clesSSLlineedit ->setText(url.path());
    wdg_clesSSLlineedit ->setImmediateToolTip(wdg_clesSSLlineedit->text());
}

/*!
 * \brief dlg_paramconnexion::ImporterDonneesConnexion
 * Reprend le dossier RufusConnexion exporté par le serveur : adresse, port et mot de passe du mode choisi,
 * clés SSL recopiées chez Rufus pour l'accès distant.
 */
void dlg_paramconnexion::ImporterDonneesConnexion()
{
    const bool distant = (modeacces() == Utils::Distant);

    QUrl url = Utils::getExistingDirectoryUrl(this, tr("Sélectionnez le dossier %1 sur la clé USB").arg(QString(NOM_DIR_CONNEXION)),
                                              QUrl::fromLocalFile(QDir::homePath()), QStringList());
    if (url == QUrl())
        return;

    QString source = url.path();
    if (QDir(source + "/" NOM_DIR_CONNEXION).exists())
        source += "/" NOM_DIR_CONNEXION;            //! la clé elle-même a été désignée, pas le dossier
    if (!QFile::exists(source + "/" NOM_FILE_CONNEXION) || (distant && !QDir(source + "/" NOM_DIR_CLESSSL).exists()))
    {
        UpMessageBox::Watch(this, tr("Dossier incomplet"),
                            tr("Ce dossier ne contient pas les données de connexion exportées par le serveur."));
        return;
    }

    //! les clés sont recopiées chez Rufus : le support est amovible et ne sera plus là au démarrage suivant
    const QString destcles = QString(PATH_DIR_RUFUS) + "/" NOM_DIR_CLESSSL;
    if (distant)
    {
        if (!QDir().mkpath(destcles))
        {
            UpMessageBox::Watch(this, tr("Dossier inaccessible"),
                                tr("Impossible de créer le dossier des clés SSL :") + "\n" + destcles);
            return;
        }
        QStringList echecs;
        const QStringList cles = QDir(source + "/" NOM_DIR_CLESSSL).entryList(QStringList() << "*.pem", QDir::Files);
        for (const QString &f : cles)
        {
            const QString cible = destcles + "/" + f;
            QFile::remove(cible);                   //! QFile::copy échoue si la cible existe déjà
            if (!QFile::copy(source + "/" NOM_DIR_CLESSSL "/" + f, cible))
                echecs << f;
        }
        if (!echecs.isEmpty())
        {
            UpMessageBox::Watch(this, tr("Import incomplet"),
                                tr("Certaines clés SSL n'ont pas pu être copiées :") + "\n" + echecs.join(", "));
            return;
        }
        MySQLInstaller(this).corrigerDroitsClesSSL(destcles);
        wdg_clesSSLlineedit ->setText(destcles);
    }

    QSettings connexion(source + "/" NOM_FILE_CONNEXION, QSettings::IniFormat);
    const QString mdp = connexion.value(CLE_CONNEXION_MDP).toString();
    wdg_iplineedit  ->setText(connexion.value(distant? CLE_CONNEXION_SERVEUR : CLE_CONNEXION_LOCAL).toString());
    wdg_portcombo   ->setCurrentText(connexion.value(CLE_CONNEXION_PORT).toString());
    if (!mdp.isEmpty())
        MySQLInstaller::stockerMotDePassePourMode(modeacces(), mdp);   //! sans lui, la connexion échouerait juste après
}

void dlg_paramconnexion::RegleAffichage(QRadioButton *butt)
{
    wdg_ipframe         ->setVisible(butt != wdg_posteradio);
    wdg_clesSSLbouton   ->setVisible(butt == wdg_distantradio);
    wdg_clesSSLlabel    ->setVisible(butt == wdg_distantradio);
    wdg_clesSSLlineedit ->setVisible(butt == wdg_distantradio);
    if (butt == wdg_distantradio)
    {
        /*! Lien ACTIF : l'URL passe en HTML (<a href>) ET en 5e paramètre « link » de Watch, qui active
         *  alors l'ouverture externe. Le href est retiré par convertPlainText au calcul de la taille. */
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

/*!
 * \brief dlg_paramconnexion::VerifFiche
 * Saisie suffisante pour écrire un Rufus.ini valide ? L'adresse du serveur hors monoposte, le dossier
 * des clés SSL en distant.
 */
bool dlg_paramconnexion::VerifFiche()
{
    if (modeacces() != Utils::Poste && ip() == "")
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas précisé l'adresse du serveur."));
        return false;
    }
    if (modeacces() == Utils::Distant
            && (dossierclesSSL() == "" || !QDir(dossierclesSSL()).exists()))
    {
        UpMessageBox::Watch(this, tr("Vous n'avez pas précisé d'adresse valide pour les clés SSL."));
        return false;
    }
    return true;
}
