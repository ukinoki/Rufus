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

#include "dlg_salledattente.h"
#include "ui_dlg_salledattente.h"

dlg_salledattente::dlg_salledattente(int *idPatAPasser, int *idActeAPasser, QString *Titre, Procedures *procAPAsser, QWidget *parent):
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionSalDat", parent),
    ui(new Ui::dlg_salledattente)
{
    ui->setupUi(this);
    proc                = procAPAsser;
    gidPatient          = *idPatAPasser;
    gidUser             = proc->getDataUser()["idUser"].toInt();
    gidUserSuperviseur  = proc->getDataUser()["UserSuperviseur"].toInt();
    gTitre              = *Titre;
    gidActe             = *idActeAPasser;
    db                  = proc->getDataBase();
    QVBoxLayout *globallay = dynamic_cast<QVBoxLayout*>(layout());
    ui->MessagetextEdit->setText(gTitre);
    ui->MessagetextEdit->document()->setTextWidth(width()-globallay->contentsMargins().left()*2-2);
    ui->MessagetextEdit->setFixedSize(width()-globallay->contentsMargins().left()*2, ui->MessagetextEdit->document()->size().height()+2);
    globallay->insertWidget(0,ui->MiseEnSalleDAttentegroupBox);
    globallay->insertWidget(0,ui->MessagetextEdit);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    globallay->setSizeConstraint(QLayout::SetFixedSize);

    connect (OKButton,                                  SIGNAL(clicked()),                  this,            SLOT (Slot_OKButtonClicked()));
    connect (CancelButton,                              SIGNAL(clicked()),                  this,            SLOT (reject()));
    connect (ui->ExamEnCoursradioButton,                SIGNAL(clicked(bool)),              this,            SLOT (Slot_EnableOKButton()));
    connect (ui->ExamenEnAttenteAutreAvisradioButton,   SIGNAL(clicked(bool)),              this,            SLOT (Slot_EnableOKButton()));
    connect (ui->RetourAccueilradioButton,              SIGNAL(clicked(bool)),              this,            SLOT (Slot_EnableOKButton()));


    QStringList ListUser;

    ui->UsercomboBox->clear();
    QString Usersrequete = "SELECT UserLogin FROM " NOM_TABLE_UTILISATEURS " as usr, " NOM_TABLE_USERSCONNECTES " as connectusr"
            " WHERE (UserFonction = '" + tr("Médecin") + "' OR UserFonction = '" + tr("Orthoptiste") + "' OR UserFonction = '" + tr("Assistant") + "')"
            " AND usr.idUser <> '" + QString::number(gidUser) + "'"
            " AND usr.idUser = connectusr.idUser";
    //proc->Edit( Usersrequete);
    QSqlQuery ListUserQuery(Usersrequete,db);
    proc->TraiteErreurRequete(ListUserQuery,Usersrequete,"");

    if (ListUserQuery.size() == 0)
    {
        ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(false);
        ui->UsercomboBox->setEnabled(false);
    }
    else if (ListUserQuery.size() == 1)
    {
        ListUserQuery.first();
        ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(true);
        ui->ExamenEnAttenteAutreAvisradioButton->setText(tr("Examen terminé, en attente d'être vu par ") + ListUserQuery.value(0).toString());
        ListUser << ListUserQuery.value(0).toString();
        ui->UsercomboBox->setEditText(ListUserQuery.value(0).toString());
        ui->UsercomboBox->insertItems(0,ListUser);
        ui->UsercomboBox->setVisible(false);
    }
    else
    {
        for (int i = 0; i < ListUserQuery.size(); i++)
        {
            ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(true);
            ui->UsercomboBox->setVisible(true);
            ListUserQuery.seek(i);
            ListUser << ListUserQuery.value(0).toString();
            if (i == 0) ui->UsercomboBox->setEditText(ListUserQuery.value(0).toString());
        }
        ui->UsercomboBox->insertItems(0,ListUser);
    }
    OKButton->setEnabled(false);
    ui->MsgtextEdit->setEnabled(false);
    ui->Msglabel->setEnabled(false);
    ui->RetourAccueilradioButton->setText(tr("Examen terminé, en attente des informations de paiement"));
}

dlg_salledattente::~dlg_salledattente()
{
}

/*-----------------------------------------------------------------------------------------------------------------
-- Traitement des SLOTS / actions associees a chaque objet du formulaire et aux menus -----------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    dlg_salledattente::Slot_OKButtonClicked()
{
    QString Statut;

    if (!ui->ExamEnCoursradioButton->isChecked() && !ui->ExamenEnAttenteAutreAvisradioButton->isChecked() && !ui->RetourAccueilradioButton->isChecked())
    {
        UpMessageBox::Watch(this,tr("Vous devez choisir un statut avant de mettre le patient en salle d'attente!"));
        return;
    }

    QString ActeSal ="null";
    QString Msg("");
    if (ui->ExamEnCoursradioButton->isChecked())                Statut = ENCOURS;
    if (ui->ExamenEnAttenteAutreAvisradioButton->isChecked())   Statut = ENATTENTENOUVELEXAMEN + ui->UsercomboBox->currentText();
    if (ui->RetourAccueilradioButton->isChecked())
    {
        ActeSal = QString::number(gidActe);
        QString req = "select idacte, iduser from " NOM_TABLE_ACTES " where idacte = " + ActeSal;
        QSqlQuery actquer(req,db);
        actquer.first();
        if (actquer.value(1).toInt() == -1)
            QSqlQuery ("update " NOM_TABLE_ACTES " set idUser = " + QString::number(proc->UserSuperviseur()) + ", UserComptable = " + QString::number(proc->UserComptable()) + ", UserParent = " + QString::number(proc->UserParent()) + " where idacte = " + actquer.value(0).toString(), db);
        Statut  = RETOURACCUEIL;
        Msg     = ui->MsgtextEdit->toPlainText();
    }

    QString saldatrequete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE " WHERE idPat = " + QString::number(gidPatient);
    QSqlQuery SalDatQuery(saldatrequete,db);
    proc->TraiteErreurRequete(SalDatQuery,saldatrequete,tr("Impossible de trouver le dossier dans la salle d'attente!"));
    QString MsgErreur;
    if (SalDatQuery.size() == 0)
    {

        saldatrequete =     "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                            " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, idActeAPayer, PosteExamen, Message)"
                            " VALUES ('" + QString::number(gidPatient) + "','" + QString::number(gidUserSuperviseur) + "','" + Statut + "','"
                            + QTime::currentTime().toString("hh:mm") +"', null," + ActeSal + ",'" + proc->CorrigeApostrophe(Msg) + "',null)";
        MsgErreur           = tr("Impossible de mettre ce dossier en salle d'attente");
    }
    else
    {
        saldatrequete =     "UPDATE " NOM_TABLE_SALLEDATTENTE " SET"
                            " Statut = '" + Statut +
                             "', HeureStatut = '" + QTime::currentTime().toString("hh:mm") +
                            "', idUserEnCoursExam = null"
                            ", PosteExamen = null";
        if (ActeSal != "null")
            saldatrequete   += ", idActeAPayer = " + ActeSal;
        saldatrequete       += ", Message = '" + proc->CorrigeApostrophe(Msg) + "'";
        saldatrequete       += " WHERE idPat = '" + QString::number(gidPatient) + "'";
        MsgErreur           = tr("Impossible de modifier les statuts du dossier en salle d'attente!");
    }
    //proc->Edit(saldatrequete);
    QSqlQuery ModifSalDatQuery(saldatrequete,db);
    proc->TraiteErreurRequete(ModifSalDatQuery,saldatrequete,MsgErreur);
    proc->UpdVerrouSalDat();
    accept();
}

void    dlg_salledattente::Slot_EnableOKButton()
{
    OKButton            ->setEnabled(true);
    QRadioButton *btu   = dynamic_cast<QRadioButton*>(sender());
    if (btu != NULL)
    {
        ui->MsgtextEdit ->setEnabled(btu==ui->RetourAccueilradioButton);
        ui->Msglabel    ->setEnabled(btu==ui->RetourAccueilradioButton);
    }
}

