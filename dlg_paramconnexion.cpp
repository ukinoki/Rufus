/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_paramconnexion.h"
#include "icons.h"
#include "ui_dlg_paramconnexion.h"

dlg_paramconnexion::dlg_paramconnexion(bool OKAccesDistant, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_paramconnexion)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    QStringList ports;
    ports << "3306" << "3307";
    ui->PortcomboBox        ->addItems(ports);
    ui->IPlabel             ->setVisible(false);
    ui->IPlineEdit          ->setVisible(false);
    ui->DistantradioButton  ->setEnabled(OKAccesDistant);
    ui->HelpupPushButton    ->setIconSize(QSize(50,50));
    ui->AccesgroupBox       ->setFocusProxy(ui->PosteradioButton);
    ui->OKuppushButton      ->setShortcut(QKeySequence("Meta+Return"));
    rxIP                    = QRegExp("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");
    gVisible                = true;
    gIPAvecZero             = "";
    gIPSansZero             = "";
    gClient                 = "";
    gServeur                = "";
    gTimer                  = new QTimer(this);
    gTimer                  ->start(500);

    connect(ui->AnnuluppushButton,      SIGNAL(clicked(bool)),          this,   SLOT(reject()));
    connect(ui->HelpupPushButton,       SIGNAL(clicked(bool)),          this,   SLOT(Slot_HelpMsg()));
    connect(ui->OKuppushButton,         SIGNAL(clicked(bool)),          this,   SLOT(Slot_Verif()));
    connect(ui->TestuppushButton,       SIGNAL(clicked(bool)),          this,   SLOT(Slot_Test()));
    connect(ui->LocalradioButton,       SIGNAL(clicked(bool)),          this,   SLOT(Slot_RegleAffichage()));
    connect(ui->PosteradioButton,       SIGNAL(clicked(bool)),          this,   SLOT(Slot_RegleAffichage()));
    connect(ui->DistantradioButton,     SIGNAL(clicked(bool)),          this,   SLOT(Slot_RegleAffichage()));
    connect(ui->IPlineEdit,             SIGNAL(textEdited(QString)),    this,   SLOT(Slot_CalcIP(QString)));
    connect(ui->IPlineEdit,             SIGNAL(editingFinished()),      this,   SLOT(Slot_MAJIP()));

    connect(gTimer,                     SIGNAL(timeout()),      this,   SLOT(Slot_Clign()));
}

dlg_paramconnexion::~dlg_paramconnexion()
{
    delete ui;
}

void dlg_paramconnexion::Slot_CalcIP(QString IP)
{
    if (ui->PosteradioButton->isChecked())
    {
        gClient     = "localhost";
        gServeur    = "localhost";
    }
    if (ui->DistantradioButton->isChecked())
    {
        gClient   = "%";
        if (!rxIP.exactMatch(IP))
            gServeur  = ui->IPlineEdit->text();
    }
    if (ui->LocalradioButton->isChecked()
        || (ui->DistantradioButton->isChecked() && rxIP.exactMatch(IP)))
    {
        QStringList listIP = IP.split(".");
        gIPAvecZero        = "";
        gIPSansZero        = "";
        gClient            = "";
        gServeur           = "";
        for (int i=0;i<listIP.size();i++)
        {
            gIPSansZero += QString::number(listIP.at(i).toInt());
            QString AvecZero;
            AvecZero += QString::number(listIP.at(i).toInt());
            if (listIP.at(i).toInt()<100)
                AvecZero = "0" + AvecZero;
            if (listIP.at(i).toInt()<10)
                AvecZero = "0" + AvecZero;
            gIPAvecZero += AvecZero;
            if (i<listIP.size()-1)
            {
                gIPAvecZero += ".";
                gIPSansZero += ".";
            }
            if (i==listIP.size()-2 && ui->LocalradioButton->isChecked())
                gClient = gIPSansZero + "%";
            gServeur = gIPSansZero;
        }
    }
}

void dlg_paramconnexion::Slot_Clign()
{
    gVisible = !gVisible;
    if (gVisible)
        ui->HelpupPushButton->setIcon(Icons::icHelp());
    else
        ui->HelpupPushButton->setIcon(Icons::icNull());
}

void dlg_paramconnexion::Slot_HelpMsg()
{
    QMessageBox msgbox;
    UpPushButton *OKBouton = new UpPushButton;
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
    OKBouton->setText("OK");
    msgbox.addButton(OKBouton, QMessageBox::AcceptRole);
    msgbox.exec();
}

void dlg_paramconnexion::Slot_MAJIP()
{
    Slot_CalcIP(ui->IPlineEdit->text());
    if (ui->LocalradioButton->isChecked()
        || (ui->DistantradioButton->isChecked() && rxIP.exactMatch(gIPAvecZero)))
        ui->IPlineEdit->setText(gIPAvecZero);
    //UpMessageBox::Watch(this, "AvecZero = " + gIPAvecZero + "\nSansZero = " + gIPSansZero + "\nClient = " + gClient);
}

void dlg_paramconnexion::Slot_RegleAffichage()
{
    if (sender() == ui->PosteradioButton)
    {
        ui->IPlabel->setVisible(false);
        ui->IPlineEdit->setVisible(false);
    }
    else if (sender() == ui->LocalradioButton)
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
    else if (sender() == ui->DistantradioButton)
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

void dlg_paramconnexion::Slot_Test()
{
    if (TestConnexion())
        {
            UpMessageBox::Watch(this,tr("Paramètres OK!"));
            //QSqlDatabase::removeDatabase("Rufus");
            db.close();
        }
}

void dlg_paramconnexion::Slot_Verif()
{
    if (TestConnexion())
        accept();
}

QSqlDatabase dlg_paramconnexion::getdatabase()
{
    return db;
}

bool dlg_paramconnexion::TestConnexion()
{
    if (!VerifFiche())
        return false;

    Slot_MAJIP();
    QString login = ui->LoginlineEdit->text();
    if (ui->DistantradioButton->isChecked())
        login += "SSL";

    db = QSqlDatabase::addDatabase("QMYSQL","Rufus");
    db.setHostName(gServeur);
    db.setUserName(login);
    db.setPassword(ui->MDPlineEdit->text());
    db.setPort(ui->PortcomboBox->currentText().toInt());
    QString  ConnectOptions = (ui->DistantradioButton->isChecked()?
                              "SSL_KEY=/etc/mysql/client-key.pem;"
                              "SSL_CERT=/etc/mysql/client-cert.pem;"
                              "SSL_CA=/etc/mysql/ca-cert.pem;"
                              "MYSQL_OPT_RECONNECT=1"
                                 :
                              "MYSQL_OPT_RECONNECT=1");
    db.setConnectOptions(ConnectOptions);


    if (!db.open())
    {
        UpMessageBox::Watch(this,tr("Paramètres non reconnus!") + "<br />"
                                + tr("Serveur") + "\n\t-> " + gServeur + "<br />"
                                //+ tr("Login") + "\n\t-> " NOM_ADMINISTRATEURDOCS "<br />"
                                + tr("Login") + "\n\t-> " + login + "<br />"
                                + tr("MDP") + "\n\t-> " + ui->MDPlineEdit->text() + "<br />"
                                + tr("Port") + "\n\t-> " + ui->PortcomboBox->currentText() + "<br />"
                                + tr("Connexion impossible.") + "<br />"
                                + db.lastError().text());
        return false;
    }

    QSqlQuery("set global sql_mode = 'NO_ENGINE_SUBSTITUTION,STRICT_TRANS_TABLES';", db);
    QSqlQuery("SET GLOBAL event_scheduler = 1", db);
    QSqlQuery("SET GLOBAL max_allowed_packet=" MAX_ALLOWED_PACKET "*1024*1024 ;", db);
    QString req = "show grants for '" + login + "'@'" + gClient + "'";
    QSqlQuery grantsquery(req, db);
    if (grantsquery.size()==0)
    {
        UpMessageBox::Watch(this,tr("Erreur sur le serveur"),
                            tr("Impossible de retrouver les droits de l'utilisateur ") + ui->LoginlineEdit->text());
        return false;
    }
    grantsquery.first();
    QString reponse = grantsquery.value(0).toString();
    if (reponse.left(9) != "GRANT ALL")
    {
        UpMessageBox::Watch(this,tr("Erreur sur le serveur"),
                            tr("L'utilisateur ") + ui->LoginlineEdit->text() + tr(" existe mais ne dispose pas "
                             "de toutes les autorisations pour modifier ou créer des données sur le serveur.\n"
                             "Choisissez un autre utilisateur ou modifiez les droits de cet utilisateur au niveau du serveur.\n"));
        return false;
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
