/* (C) 2018 LAINE SERGE
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

dlg_paramconnexion::dlg_paramconnexion(bool connectavecLoginSQL, bool OKAccesDistant, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_paramconnexion)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    m_connectavecloginSQL = connectavecLoginSQL;

    ui->PortcomboBox        ->addItems(QStringList() << "3306" << "3307");
    ui->IPlabel             ->setVisible(false);
    ui->IPlineEdit          ->setVisible(false);
    ui->DistantradioButton  ->setEnabled(OKAccesDistant);
    ui->HelpupPushButton    ->setIconSize(QSize(50,50));
    ui->AccesgroupBox       ->setFocusProxy(ui->PosteradioButton);
    ui->OKuppushButton      ->setShortcut(QKeySequence("Meta+Return"));
    QTimer t_timer;
    t_timer                  .start(500);
    ui->LoginlineEdit   ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_15,this));
    ui->MDPlineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12,this));

    connect(ui->AnnuluppushButton,      &QPushButton::clicked,          this,   &QDialog::reject);
    connect(ui->HelpupPushButton,       &QPushButton::clicked,          this,   &dlg_paramconnexion::HelpMsg);
    connect(ui->OKuppushButton,         &QPushButton::clicked,          this,   &dlg_paramconnexion::Verif);
    connect(ui->TestuppushButton,       &QPushButton::clicked,          this,   &dlg_paramconnexion::Test);
    connect(ui->LocalradioButton,       &QRadioButton::clicked,         this,   [=] {RegleAffichage(ui->LocalradioButton);});
    connect(ui->PosteradioButton,       &QRadioButton::clicked,         this,   [=] {RegleAffichage(ui->PosteradioButton);});
    connect(ui->DistantradioButton,     &QRadioButton::clicked,         this,   [=] {RegleAffichage(ui->DistantradioButton);});
    connect(ui->IPlineEdit,             &QLineEdit::textEdited,         this,   &dlg_paramconnexion::CalcIP);
    connect(ui->IPlineEdit,             &QLineEdit::editingFinished,    this,   &dlg_paramconnexion::MAJIP);

    connect(&t_timer,                   &QTimer::timeout,               this,   &dlg_paramconnexion::Clign);
}

dlg_paramconnexion::~dlg_paramconnexion()
{
    delete ui;
}

void dlg_paramconnexion::CalcIP(QString IP)
{
    if (ui->PosteradioButton->isChecked())
        m_adresseserveur    = "localhost";
    else if (ui->DistantradioButton->isChecked())
    {
        if (!Utils::rgx_IPV4.exactMatch(IP))
            m_adresseserveur  = ui->IPlineEdit->text();
    }
    else if (ui->LocalradioButton->isChecked()
         || (ui->DistantradioButton->isChecked() && Utils::rgx_IPV4.exactMatch(IP)))
    {
        QStringList listIP = IP.split(".");
        m_IPaveczero        = "";
        QString m_IPsanszero        = "";
        m_adresseserveur           = "";
        for (int i=0;i<listIP.size();i++)
        {
            m_IPsanszero += QString::number(listIP.at(i).toInt());
            QString AvecZero;
            AvecZero += QString::number(listIP.at(i).toInt());
            if (listIP.at(i).toInt()<100)
                AvecZero = "0" + AvecZero;
            if (listIP.at(i).toInt()<10)
                AvecZero = "0" + AvecZero;
            m_IPaveczero += AvecZero;
            if (i<listIP.size()-1)
            {
                m_IPaveczero += ".";
                m_IPsanszero += ".";
            }
            m_adresseserveur = m_IPsanszero;
        }
    }
}

void dlg_paramconnexion::Clign()
{
    m_visible = !m_visible;
    if (m_visible)
        ui->HelpupPushButton->setIcon(Icons::icHelp());
    else
        ui->HelpupPushButton->setIcon(Icons::icNull());
}

void dlg_paramconnexion::HelpMsg()
{
    QMessageBox msgbox;
    UpSmallButton OKBouton("OK");
    msgbox.setText("Paramètrage de MySQL");
    msgbox.setInformativeText(tr("Si vous venez d'installer MySQL sur ce poste et que vous voulez vous connecter,\n\n"
                              "Utilisez\n"
                              "1. le login de connexion que vous avez créé en paramètrant MySQL dans la fenêtre login,\n"
                              "2. le mot de passe que vous avez créé en paramètrant MySQL"
                              "dans la fenêtre mot de passe,\n"
                              "3. choisissez \"Sur ce poste\" dans la boîte \"emplacement du serveur\",\n"
                              "4. et \"3306\" dans la liste des ports.\n\n"
                              "Cliquez sur Tester pour tester la validité de ces valeurs.\n"
                              "Si cela ne marche pas, essayez avec le port 3307\n\n"
                              "Si cela ne marche toujours pas, revoyez la configuration du serveur.\n"));
    msgbox.setIcon(QMessageBox::Information);
    msgbox.addButton(&OKBouton, QMessageBox::AcceptRole);
    msgbox.exec();
}

void dlg_paramconnexion::MAJIP()
{
    CalcIP(ui->IPlineEdit->text());
    if (ui->LocalradioButton->isChecked()
        || (ui->DistantradioButton->isChecked() && Utils::rgx_IPV4.exactMatch(m_IPaveczero)))
        ui->IPlineEdit->setText(m_IPaveczero);
    //UpMessageBox::Watch(this, "AvecZero = " + gIPAvecZero + "\nSansZero = " + gIPSansZero + "\nClient = " + gClient);
}

void dlg_paramconnexion::RegleAffichage(QRadioButton *butt)
{
    if (butt == ui->PosteradioButton)
    {
        ui->IPlabel->setVisible(false);
        ui->IPlineEdit->setVisible(false);
    }
    else if (butt == ui->LocalradioButton)
    {
        ui->IPlabel->setVisible(true);
        ui->IPlabel->setText(tr("adresse IP"));
        QString AdressIP, MasqueReseauLocal;
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
                 AdressIP = address.toString();
        }
        QStringList listIP = AdressIP.split(".");
        for (int i=0;i<listIP.size()-1;i++)
        {
            if (listIP.at(i).toInt()<100)
                MasqueReseauLocal += "0";
            if (listIP.at(i).toInt()<10)
                MasqueReseauLocal += "0";
            MasqueReseauLocal += listIP.at(i) + ".";
        }
        MasqueReseauLocal += "000";
        ui->IPlineEdit->setText(MasqueReseauLocal);
        ui->IPlineEdit->setInputMask("000.000.000.000");
        ui->IPlineEdit->setVisible(true);
        ui->IPlineEdit->setFocus();
    }
    else if (butt == ui->DistantradioButton)
    {
        UpMessageBox::Watch(this, tr("Informations importantes sur l'accès par internet"),
                                tr("Pour d'évidentes raisons de confidentialité, l'accès distant dans Rufus "
                                  "fonctionne obligatoirement avec un cryptage de données openSSL sur 2048 bits.\n\n"
                                  "Si vous voulez utiliser l'accès distant sur ce poste, il faut que:\n"
                                  "1. le serveur MySQL soit configuré pour le cryptage (modification du fichier /etc/my.cnf "
                                  "et présence des clés serveur dans le répertoire /etc/mysql/)\n"
                                  "2. et que sur ce poste les clés clientes SSL soient installées dans le répertoire /etc/mysql/.\n\n"
                                  "Sinon, ça ne marchera pas\n\n"
                                  "Reportez vous à la page internet\n"
                                  "http://www.rufusvision.org/configuration-pour-un-accegraves-agrave-distance---le-cryptage-de-donneacutees-ssl.html\n"
                                  "pour savoir comment modifier la configuration du serveur et générer des clés de cryptage\n"));
        ui->IPlabel->setVisible(true);
        ui->IPlabel->setText(tr("adresse IP ou DNS"));
        ui->IPlineEdit->setInputMask("");
        ui->IPlineEdit->setText("");
        ui->IPlineEdit->setVisible(true);
        ui->IPlineEdit->setFocus();
    }
}

void dlg_paramconnexion::Test()
{
    if (TestConnexion(false))
        {
            UpMessageBox::Watch(this,tr("Paramètres OK!"));
            //QSqlDatabase::removeDatabase("Rufus");
        }
}

void dlg_paramconnexion::Verif()
{
    if (TestConnexion(true))
        accept();
}

bool dlg_paramconnexion::TestConnexion(bool avecverifbase)
{
    if (!VerifFiche())
        return false;

    MAJIP();
    Utils::ModeAcces mode = Utils::Poste;
    QString server;
    if (ui->PosteradioButton->isChecked())           mode = Utils::Poste;
    else if (ui->LocalradioButton->isChecked())      mode = Utils::ReseauLocal;
    else if (ui->DistantradioButton->isChecked())    mode = Utils::Distant;
    DataBase::I()->setModeacces(mode);
    DataBase::I()->initParametresConnexionSQL(m_adresseserveur, ui->PortcomboBox->currentText().toInt());

    QString Login = ui->LoginlineEdit->text();
    QString Password = ui->MDPlineEdit->text();
    if ( Login.isEmpty() )    {UpMessageBox::Watch(this,tr("Vous n'avez pas précisé votre identifiant!"));    ui->LoginlineEdit->setFocus(); return 0;}
    if ( Password.isEmpty() ) {UpMessageBox::Watch(this,tr("Vous n'avez pas précisé votre mot de passe!"));   ui->MDPlineEdit->setFocus();    return 0;}
    if (m_connectavecloginSQL)
    {
        Login = LOGIN_SQL;
        Password = MDP_SQL;
    }
    QString req;
    QString error = "";
    error = DataBase::I()->connectToDataBase(DB_CONSULTS, Login, Password);

    if( error.size() )
    {
        UpMessageBox::Watch(this, tr("Erreur sur le serveur MySQL"),
                            tr("Impossible de se connecter au serveur avec le login ") + Login
                            + tr(" et ce mot de passe") + "\n"
                            + tr("Revoyez le réglage des paramètres de connexion dans le fichier rufus.ini.") + "\n"
                            + error);
        return false;
    }
    /*! la connection à la base fonctionne -> on vérifie la version de la base */
    if (avecverifbase)
        emit verifbase();

    if (m_connectavecloginSQL)
    {
        DataBase::QueryResult rep = DataBase::I()->verifExistUser(ui->LoginlineEdit->text(), ui->MDPlineEdit->text());
        if (rep == DataBase::Error)
        {
            UpMessageBox::Watch(this, tr("Erreur sur la base patients"),
                                tr("Impossible d'ouvrir la table Utilisateurs"));
            return false;
        }
        if( rep == DataBase::Empty )
        {
            UpMessageBox::Watch(this, tr("Erreur sur le compte utilisateur"),
                                tr("Identifiant ou mot de passe incorrect") );
            return false;
        }
    }

    return true;
}

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
        ui->IPlineEdit->setFocus();
        return false;
    }
    return true;
}
