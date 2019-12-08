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

    ui->LoginlineEdit   ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_15,this));
    ui->MDPlineEdit     ->setValidator(new QRegExpValidator(Utils::rgx_AlphaNumeric_5_12,this));
    ui->MDPlineEdit     ->setEchoMode(QLineEdit::Password);

    connect (ui->OKpushButton,      &QPushButton::clicked,  this,  &dlg_identificationuser::Validation);
    connect (ui->AnnulpushButton,   &QPushButton::clicked, this,   &dlg_identificationuser::reject);

    ui->OKpushButton    ->setShortcut(QKeySequence("Meta+Return"));
    ui->AnnulpushButton ->setShortcut(QKeySequence("F12"));

    ui->OKpushButton    ->installEventFilter(this);
    ui->AnnulpushButton ->installEventFilter(this);
    ui->LoginlineEdit   ->installEventFilter(this);
    ui->MDPlineEdit     ->installEventFilter(this);

    m_chgmtuser         = ChgUser;
    ui->LoginlineEdit   ->setFocus();
}

dlg_identificationuser::~dlg_identificationuser()
{
    delete ui;
}

void dlg_identificationuser::Validation()
{
    //On desactive le button OK pour neutraliser le double clic.
    ui->OKpushButton->setEnabled(false);


    QString AdminDocs = NOM_ADMINISTRATEURDOCS; //FIXME : !!! Test en dur
    if (ui->LoginlineEdit->text().toUpper() == AdminDocs.toUpper())
    {
        UpMessageBox::Watch(this, tr("Vous ne pouvez pas utiliser ce Login pour vous connecter"));
        ui->OKpushButton->setEnabled(true);
        return;
    }

    LoginResult a = ControleDonnees();
    if (a == OK)
        accept();
    else if (a != NoUser && a != NoConnexion)
        done(-1);
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
            if (obj == ui->MDPlineEdit) Validation();
            if (!obj->inherits("QPushButton"))
                return QWidget::focusNextChild();
        }
    }
    return QWidget::eventFilter(obj, event);
}

/*--------------------------------------------------------------------------------------------------------------
-- Recherche de l'utilisateur ----------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------*/
dlg_identificationuser::LoginResult dlg_identificationuser::ControleDonnees()
{
    bool ok;
    //TODO : SQL
    QString req;
    QString Login = ui->LoginlineEdit->text();
    QString Password = ui->MDPlineEdit->text();

    if ( Login.isEmpty() )    {UpMessageBox::Watch(this,tr("Vous n'avez pas précisé votre identifiant!"));    ui->LoginlineEdit->setFocus();    return NoUser;}
    if ( Password.isEmpty() ) {UpMessageBox::Watch(this,tr("Vous n'avez pas précisé votre mot de passe!"));   ui->MDPlineEdit->setFocus();      return NoUser;}

    if (!m_chgmtuser)
    {
//TODO : SQL Mettre en place un compte generique pour l'accès à la base de données.
        QString error = "";
        error = db->connectToDataBase(DB_CONSULTS, Login, Password);

        if( error.size() )
        {
            ui->IconServerOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(100);
            UpMessageBox::Watch(this, tr("Erreur sur le serveur MySQL"),
                            tr("Impossible de se connecter au serveur avec le login ") + Login
                            + tr(" et ce mot de passe") + "\n"
                            + tr("Revoyez le réglage des paramètres de connexion dans le fichier rufus.ini.") + "\n"
                            + error);
            return NoConnexion;
        }

        QString Client;
        if (db->getMode() == Utils::Distant)
                Client = "%";
        else if (db->getMode() == Utils::ReseauLocal && Utils::rgx_IPV4.exactMatch(db->getServer()))
        {
            QStringList listIP = db->getServer().split(".");
            for (int i=0;i<listIP.size()-1;i++)
            {
                Client += QString::number(listIP.at(i).toInt()) + ".";
                if (i==listIP.size()-2)
                    Client += "%";
            }
        }
        else
            Client = db->getServer();
        req = "show grants for '" + Login + (db->getMode() == Utils::Distant? "SSL" : "")  + "'@'" + Client + "'";
        //qDebug() << req;

        QVariantList grantsdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (!ok || grantsdata.size()==0)
        {
            ui->IconServerOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(150);
            UpMessageBox::Watch(this, tr("Erreur sur le serveur MySQL"),
                            tr("Impossible de retrouver les droits de l'utilisateur ") + Login + "\n" +
                            tr("Revoyez la configuration du serveur MySQL pour corriger le problème.") + "\n");
            return UnDefinedRights;
        }
        QString reponse = grantsdata.at(0).toString();
        if (reponse.left(9) != "GRANT ALL")
        {
            ui->IconServerOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(150);
            UpMessageBox::Watch(this, tr("Erreur sur le serveur MySQL"),
                            tr("L'utilisateur ") + Login
                            + tr(" existe mais ne dispose pas de toutes les autorisations pour modifier/créer des données sur le serveur.")
                            + "\n" + tr("Revoyez la configuration du serveur MySQL pour corriger le problème.") + "\n");
            return UnCorrectRights;
        }


        ui->IconServerOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(150);
        req = "SHOW TABLES FROM " DB_CONSULTS " LIKE '%tilisateurs%'";
        QList<QVariantList> tablist = db->StandardSelectSQL(req, ok);
        if (tablist.size()<2)
        {
            ui->IconBaseOKupLabel->setPixmap(Icons::pxError());
            Utils::Pause(600);
            //TODO : Erreur non géré
            return CorruptedBase;
        }
//END
    }
    else
    {
        ui->IconServerOKupLabel->setPixmap(Icons::pxCheck());
        Utils::Pause(150);
    }

    DataBase::QueryResult rep = db->calcidUserConnected(Login, Password);
    if (rep == DataBase::Error)
    {
        ui->IconBaseOKupLabel->setPixmap(Icons::pxError());
        Utils::Pause(150);

        UpMessageBox::Watch(this, tr("Erreur sur la base patients"),
                        tr("Impossible d'ouvrir la table Utilisateurs"));
        return CorruptedBase;
    }

    ui->IconBaseOKupLabel->setPixmap(Icons::pxCheck());
    Utils::Pause(150);
    if( rep == DataBase::Empty )
    {
        ui->IconUserOKupLabel->setPixmap(Icons::pxError());
        Utils::Pause(150);
        UpMessageBox::Watch(this, tr("Erreur sur le compte utilisateur"),
                        tr("Identifiant ou mot de passe incorrect") );
        return CorruptedUser;
    }

    ui->IconBaseOKupLabel->setPixmap(Icons::pxCheck());
    Utils::Pause(150);

    ui->IconUserOKupLabel->setPixmap(Icons::pxCheck());
    Utils::Pause(150);
    return OK;
}

