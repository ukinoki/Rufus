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
#include "dlg_paramconnexion.h"
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

    /*! dossiers partagés par le serveur : le poste du réseau local y lit l'imagerie et les vidéos */
    wdg_imagerielabel   = new UpLabel();
    wdg_imagerielabel   ->setText(tr("Dossier d'imagerie du serveur"));
    wdg_videoslabel     = new UpLabel();
    wdg_videoslabel     ->setText(tr("Dossier des vidéos du serveur"));
    wdg_imagerielineedit = new UpLineEdit();
    wdg_videoslineedit  = new UpLineEdit();
    wdg_imageriebouton  = new QPushButton("...");
    wdg_videosbouton    = new QPushButton("...");
    QHBoxLayout *imagerielay = new QHBoxLayout;
    QHBoxLayout *videoslay   = new QHBoxLayout;
    for (UpLineEdit *champ : {wdg_imagerielineedit, wdg_videoslineedit})
    {
        champ           ->setAlignment(Qt::AlignCenter);
        champ           ->useselftextastooltip();
    }
    for (QPushButton *butt : {wdg_imageriebouton, wdg_videosbouton})
    {
        butt            ->setFixedSize(54, 32);
        butt            ->setContextMenuPolicy(Qt::NoContextMenu);
    }
    imagerielay         ->setContentsMargins(0, 0, 0, 0);
    imagerielay         ->addWidget(wdg_imagerielineedit, 1);
    imagerielay         ->addWidget(wdg_imageriebouton, 0);
    videoslay           ->setContentsMargins(0, 0, 0, 0);
    videoslay           ->addWidget(wdg_videoslineedit, 1);
    videoslay           ->addWidget(wdg_videosbouton, 0);

    wdg_ipframe         = new QFrame();
    wdg_ipframe         ->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *iplay  = new QVBoxLayout;
    iplay               ->addWidget(iplabel);
    iplay               ->addWidget(wdg_iplineedit);
    iplay               ->addWidget(wdg_clesSSLlabel);
    iplay               ->addLayout(clesSSLlay);
    iplay               ->addWidget(wdg_imagerielabel);
    iplay               ->addLayout(imagerielay);
    iplay               ->addWidget(wdg_videoslabel);
    iplay               ->addLayout(videoslay);
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
    connect(wdg_imageriebouton, &QPushButton::clicked,  this, [=, this] {DossierPartage(wdg_imagerielineedit);});
    connect(wdg_videosbouton,   &QPushButton::clicked,  this, [=, this] {DossierPartage(wdg_videoslineedit);});
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
 * \brief dlg_paramconnexion::DossierPartage
 * Fait désigner un dossier partagé par le serveur et le renseigne dans le champ.
 * \param champ  champ à renseigner
 */
void dlg_paramconnexion::DossierPartage(UpLineEdit *champ)
{
    const QString dir = champ->text();
    QUrl url = Utils::getExistingDirectoryUrl(this, "", QUrl::fromLocalFile(QDir(dir).exists()? dir : QDir::homePath()), QStringList());
    if (url == QUrl())
        return;
    champ   ->setText(url.path());
}

void dlg_paramconnexion::RegleAffichage(QRadioButton *butt)
{
    wdg_ipframe         ->setVisible(butt != wdg_posteradio);
    wdg_clesSSLbouton   ->setVisible(butt == wdg_distantradio);
    wdg_clesSSLlabel    ->setVisible(butt == wdg_distantradio);
    wdg_clesSSLlineedit ->setVisible(butt == wdg_distantradio);
    wdg_imagerielabel   ->setVisible(butt == wdg_localradio);
    wdg_imagerielineedit->setVisible(butt == wdg_localradio);
    wdg_imageriebouton  ->setVisible(butt == wdg_localradio);
    wdg_videoslabel     ->setVisible(butt == wdg_localradio);
    wdg_videoslineedit  ->setVisible(butt == wdg_localradio);
    wdg_videosbouton    ->setVisible(butt == wdg_localradio);
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
