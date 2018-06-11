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

#include "dlg_identificationuser.h"
#include "icons.h"
#include "ui_dlg_identificationuser.h"

/*!
*
* Ecran de connexion de l'utilisateur
*
*/

dlg_identificationuser::dlg_identificationuser(bool ChgUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_identificationuser)
{
    ui->setupUi(this);
    setWindowTitle(tr("Rufus - Identification de l'utilisateur"));
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    ui->LoginlineEdit   ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric,this));
    ui->MDPlineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric,this));
    ui->MDPlineEdit     ->setEchoMode(QLineEdit::Password);

    connect (ui->OKpushButton,      &QPushButton::clicked,  this,   &dlg_identificationuser::Slot_RetourOK);
    connect (ui->AnnulpushButton,   &QPushButton::clicked, this,   &dlg_identificationuser::Slot_RetourAnnul);

    ui->OKpushButton    ->setShortcut(QKeySequence("Meta+Return"));
    ui->AnnulpushButton ->setShortcut(QKeySequence("F12"));

    ui->OKpushButton    ->installEventFilter(this);
    ui->AnnulpushButton ->installEventFilter(this);
    ui->LoginlineEdit   ->installEventFilter(this);
    ui->MDPlineEdit     ->installEventFilter(this);

    gChgUsr             = ChgUser;
    gTimerControl       = new QTimer(this);
    ui->LoginlineEdit   ->setFocus();
}

dlg_identificationuser::~dlg_identificationuser()
{
    delete ui;
}

QMap<QString,QString> dlg_identificationuser::getParamConnexion()
{
    return gmap;
}

void dlg_identificationuser::Slot_RetourAnnul()
{
    reject();
}

void dlg_identificationuser::Slot_RetourOK()
{
    //On desactive le button OK pour eviter de recliquer dessus.
    ui->OKpushButton->setEnabled(false);


    QString AdminDocs = NOM_ADMINISTRATEURDOCS; //TODO : !!! Test en dur
    if (ui->LoginlineEdit->text().toUpper() == AdminDocs.toUpper())
    {
        UpMessageBox::Watch(this, tr("Vous ne pouvez pas utiliser ce Login pour vous connecter"));
        return;
    }

    int a = ControleDonnees();
    if (a == -3 || a == -4 || a > 0)
        //l'erreur -1 se produit quand le couple mot de passe-Login est erroné
        //l'erreur -2 quand les droits utilisateurs sur le serveur sont incomplets
        //l'erreur -5 quand l'utilisateur ne fait pas partie de la liste des utilisateurs référencés dans la base Rufus
        //l'erreur -6 quand l'utilisateur est déjà connecté sur un autre poste
        // ces 4 erreurs peuvent correspondre à une faute de frappe et ne ferment donc pas la fiche
    {
        done (a);
        return;
    }
    db.close();

    //on le reactive
    ui->OKpushButton->setEnabled(true);
}

//-------------------------------------------------------------------------------------
// Gestion des évènements
//-------------------------------------------------------------------------------------

bool dlg_identificationuser::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Fleche Gauche - -----------------------------------------
        if(keyEvent->key()==Qt::Key_Left)
            if (!obj->inherits("QPushButton")) return QWidget::focusPreviousChild();

        // Fleche Droite - -----------------------------------------
        if(keyEvent->key()==Qt::Key_Right)
            if (!obj->inherits("QPushButton")) return QWidget::focusNextChild();

        // Return - Idem Flèche Droite sauf sur les pushButton ---------------------------
        if(keyEvent->key()==Qt::Key_Return || keyEvent->key()==Qt::Key_Enter)
        {
            if (obj == ui->MDPlineEdit) Slot_RetourOK();
            if (!obj->inherits("QPushButton"))
                return QWidget::focusNextChild();
        }
    }
    return QWidget::eventFilter(obj, event);
}

/*--------------------------------------------------------------------------------------------------------------
-- Recherche de l'utilisateur ----------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
int dlg_identificationuser::ControleDonnees()
{
    //TODO : SQL
    QString req;
    if (ui->LoginlineEdit->text() == "")    {UpMessageBox::Watch(this,tr("Vous n'avez pas précisé votre identifiant!"));    ui->LoginlineEdit->setFocus(); return 0;}
    if (ui->MDPlineEdit->text() == "")      {UpMessageBox::Watch(this,tr("Vous n'avez pas précisé votre mot de passe!"));   ui->MDPlineEdit->setFocus();    return 0;}

    QString Login = ui->LoginlineEdit->text();
    QString Client;
    if (gBase == "BDD_DISTANT")
            Client = "%";
    else if (gBase == "BDD_LOCAL" && Utils::rgx_IPV4.exactMatch(gServeur))
    {
        QStringList listIP = gServeur.split(".");
        for (int i=0;i<listIP.size()-1;i++)
        {
            Client += QString::number(listIP.at(i).toInt()) + ".";
            if (i==listIP.size()-2)
                Client += "%";
        }
    }
    else
        Client = gServeur;

    if (!gChgUsr)
    {

//TODO : SQL Mettre en place un compte generique pour l'accès à la base de données.
        QString error = DataBase::getInstance()->connectToDataBase(NOM_BASE_CONSULTS, Login, ui->MDPlineEdit->text());
        if( error )
        {
            ui->IconServerOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(200);
            UpMessageBox::Watch(this, tr("Erreur sur le serveur MySQL"),
                            tr("Impossible de se connecter au serveur avec le login ") + ui->LoginlineEdit->text()
                            + tr(" et ce mot de passe") + "\n"
                            + tr("Revoyez le réglage des paramètres de connexion dans le fichier rufus.ini.") + "\n"
                            + error);
            return -1;
        }
        req = "show grants for '" + Login + "'@'" + Client + "'";
        //UpMessageBox::Watch(this,req);
        QSqlQuery grantsquery(req,db);
        if (grantsquery.size()==0)
        {
            ui->IconServerOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            UpMessageBox::Watch(this, tr("Erreur sur le serveur MySQL"),
                            tr("Impossible de retrouver les droits de l'utilisateur ") + ui->LoginlineEdit->text() + "\n" +
                            tr("Revoyez la configuration du serveur MySQL pour corriger le problème.") + "\n");
            return -2;
        }
        grantsquery.first();
        QString reponse = grantsquery.value(0).toString();
        if (reponse.left(9) != "GRANT ALL")
        {
            ui->IconServerOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            UpMessageBox::Watch(this, tr("Erreur sur le serveur MySQL"),
                            tr("L'utilisateur ") + ui->LoginlineEdit->text()
                            + tr(" existe mais ne dispose pas de toutes les autorisations pour modifier/créer des données sur le serveur.")
                            + "\n" + tr("Revoyez la configuration du serveur MySQL pour corriger le problème.") + "\n");
            return -2;
        }


        ui->IconServerOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(300);
        req = "SHOW TABLES FROM " NOM_BASE_CONSULTS " LIKE '%tilisateurs%'";
        //qDebug() << req;
        QSqlQuery VerifBaseQuery(req,db);
        //UpMessageBox::Watch(this,req + "\n" + QString::number(VerifBaseQuery.size()));
        if (VerifBaseQuery.size()<2)
        {
            ui->IconBaseOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            return -3;
        }
//END

        QJsonObject rep = DataBase::getInstance()->login(ui->LoginlineEdit->text(), ui->MDPlineEdit->text());
        if (rep["code"] == -3)
        {
            ui->IconBaseOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);

            UpMessageBox::Watch(this, tr("Erreur sur la base patients"),
                            tr("Impossible d'ouvrir la table Utilisateurs")
                            + " \n" + tr("requete = ") + rep["request"] + "\n"); //a mettre plutot dans un fichier de log
            return -3;
        }

        ui->IconBaseOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(300);
        if( rep["code"] == -4 )
        {
            ui->IconUserOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            UpMessageBox::Watch(this, tr("Erreur sur le compte utilisateur"),
                            tr("Identifiant ou mot de passe incorrect") );
            return -4;
        }
        //Faille de sécurité
        /*
        if (UserQuery.size() == 0)
        {
            req =   "SELECT UserLogin FROM " + gTblUser;
            QSqlQuery listusrquery (req,db);
            if (listusrquery.size() == 0)
            {
                ui->IconBaseOKupLabel->setPixmap(Icons::pxError());
                Utils::Pause(600);
                return -4;
            }
            ui->IconBaseOKupLabel->setPixmap(Icons::pxCheck());
            Utils::Pause(300);
            ui->IconUserOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            listusrquery.first();
            QString listusr;
            bool ExistLogin = false;
            for (int i=0; i< listusrquery.size(); i++)
            {
                if (listusrquery.value(0).toString() == ui->LoginlineEdit->text())
                    ExistLogin = true;
                else
                    listusr += "\n\t" + listusrquery.value(0).toString();
                listusrquery.next();
            }
            if (ExistLogin)
            {
                UpMessageBox::Watch(this, tr("Erreur sur le compte utilisateur"),
                                tr("Le Login") + "\n\t" + ui->LoginlineEdit->text() + "\n"
                                + tr("existe bien dans la base de données mais pas avec le mot de passe que vous avez utilisé.")
                                + tr("Modifiez le mot de passe MySQL de ") + ui->LoginlineEdit->text()
                                + tr(" pourqu'il soit identique à celui de la base de données Rufus") + "\n"
                                + tr("ou connectez vous avec un autre des logins référencés dans la base Rufus.") + "\n"
                                + listusr + "\n"
                                + tr("Impossible de continuer.") + "\n");
             }
            else
                return -4;
            return -5;
        }
        */

        ui->IconBaseOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(300);
        if (rep["code"] == -1)
        {
            ui->IconUserOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            UpMessageBox::Watch(this, tr("Utilisateur déjà connecté"),
                            tr("Impossible de vous connecter sur ce poste!") + "\n"
                            + tr("Vous semblez être déjà connecté sur le poste") + "\n" + rep["poste"] + "\n"
                            + tr("Si ce n'est pas le cas, il faut retirer votre identifiant de la table des utilisateurs connectés.") + "\n"
                            + tr("Pour cela, redémarrez Rufus sur le poste ") + rep["poste"]
                            + tr(" puis quittez Rufus.") + "\n"
                            + tr("Cela purgera la table des utilisateurs connectés.") + "/n");

            return -1;
        }

        ui->IconUserOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(600);
        return DataBase::getInstance()->getUser()->id();
    }
    else if (gChgUsr)
    {
        ui->IconServerOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(300);
        ui->IconBaseOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(300);
        req =   "SELECT idUser FROM " + gTblUser + " WHERE UserLogin = '" +
                ui->LoginlineEdit->text() +
                "' AND UserMDP = '" + ui->MDPlineEdit->text() + "'" ;
        QSqlQuery UserQuery (req,db);
        if (UserQuery.size() == 0)
        {
            ui->IconUserOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            UpMessageBox::Watch(this, tr("Utilisateur inconnu"),
                            tr("L'utilisateur ") + ui->LoginlineEdit->text()
                            + tr(" n'existe pas dans la base avec ce mot de passe") + "\n");
            return -6;
        }
        else
        {
            UserQuery.first();
            gidUser = UserQuery.value(0).toInt();
            req = "select NomPosteconnecte from Rufus.utilisateurs where iduser = " + QString::number(gidUser);
            QSqlQuery Userquer(req,db);
            if (Userquer.size() > 0)
            {
                Userquer.first();
                if (Userquer.value(0).toString() != QHostInfo::localHostName().left(60))
                {
                    ui->IconUserOKupLabel->setPixmap(Icons::pxError());
                    Utils::Pause(600);
                    UpMessageBox::Watch(this, tr("Utilisateur déjà connecté"),
                                    tr("Impossible de vous connecter sur ce poste!") + "\n"
                                    + tr("Vous semblez être déjà connecté sur le poste\n") + Userquer.value(0).toString() + "\n"
                                    + tr("Si ce n'est pas le cas, il faut retirer votre identifiant de la table des utilisateurs connectés.") + "\n"
                                    + tr("Pour cela, redémarrez Rufus sur le poste ") + Userquer.value(0).toString() + tr(" puis quittez Rufus.") + "\n"
                                    + tr("Cela purgera la table des utilisateurs connectés.") + "\n");
                    return -6;
                }
            }
        }
        ui->IconUserOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(600);
        return gidUser;
    }
    return -7;
}

