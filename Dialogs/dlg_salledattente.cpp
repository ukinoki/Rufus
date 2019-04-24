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

#include "dlg_salledattente.h"
#include "ui_dlg_salledattente.h"

dlg_salledattente::dlg_salledattente(Patient* pat, Acte* act, QString Titre, QWidget *parent):
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionSalDat", parent),
    ui(new Ui::dlg_salledattente)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    gPatientEnCours     = pat;
    gActeEnCours        = act;

    ui->MessagetextEdit->setText(gPatientEnCours->nom() + " " + gPatientEnCours->prenom() + "\n" + Titre);

    ui->MessagetextEdit->document()->setTextWidth(width()-dlglayout()->contentsMargins().left()*2-2);
    ui->MessagetextEdit->setFixedSize(int(width()-dlglayout()->contentsMargins().left()*2), int(ui->MessagetextEdit->document()->size().height()+2));
    dlglayout()->insertWidget(0,ui->MiseEnSalleDAttentegroupBox);
    dlglayout()->insertWidget(0,ui->MessagetextEdit);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    connect (OKButton,                                  SIGNAL(clicked()),                  this,            SLOT (Slot_OKButtonClicked()));
    connect (CancelButton,                              SIGNAL(clicked()),                  this,            SLOT (reject()));
    connect (ui->ExamEnCoursradioButton,                SIGNAL(clicked(bool)),              this,            SLOT (Slot_EnableOKButton()));
    connect (ui->ExamenEnAttenteAutreAvisradioButton,   SIGNAL(clicked(bool)),              this,            SLOT (Slot_EnableOKButton()));
    connect (ui->RetourAccueilradioButton,              SIGNAL(clicked(bool)),              this,            SLOT (Slot_EnableOKButton()));


    QStringList ListUser;

    ui->UsercomboBox->clear();
    QString Usersrequete = "SELECT UserLogin FROM " NOM_TABLE_UTILISATEURS " as usr, " NOM_TABLE_USERSCONNECTES " as connectusr"
            " WHERE (UserFonction = '" + tr("Médecin") + "' OR UserFonction = '" + tr("Orthoptiste") + "' OR UserFonction = '" + tr("Assistant") + "')"
            " AND usr.idUser <> '" + QString::number(db->getUserConnected()->id()) + "'"
            " AND usr.idUser = connectusr.idUser";
    //proc->Edit( Usersrequete);
    bool ok;
    QList<QVariantList> ListUsers = db->StandardSelectSQL(Usersrequete,ok);
    if (!ok)
        return;
    if (ListUsers.size() == 0)
    {
        ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(false);
        ui->UsercomboBox->setEnabled(false);
    }
    else if (ListUsers.size() == 1)
    {
        ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(true);
        ui->ExamenEnAttenteAutreAvisradioButton->setText(tr("Examen terminé, en attente d'être vu par ") + ListUsers.at(0).at(0).toString());
        ListUser << ListUsers.at(0).at(0).toString();
        ui->UsercomboBox->setEditText(ListUsers.at(0).at(0).toString());
        ui->UsercomboBox->insertItems(0,ListUser);
        ui->UsercomboBox->setVisible(false);
    }
    else
    {
        for (int i = 0; i < ListUsers.size(); i++)
        {
            ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(true);
            ui->UsercomboBox->setVisible(true);
            ListUser << ListUsers.at(i).at(0).toString();
            if (i == 0) ui->UsercomboBox->setEditText(ListUsers.at(i).at(0).toString());
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
    bool ok;
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
        ActeSal = QString::number(gActeEnCours->id());
        QString req = "select idacte, iduser from " NOM_TABLE_ACTES " where idacte = " + ActeSal;
        QList<QVariantList> ListActes = db->StandardSelectSQL(req,ok);
        if (ListActes.at(0).at(1).toInt() == -1)
            db->StandardSQL("update " NOM_TABLE_ACTES " set idUser = " + QString::number(db->getUserConnected()->getIdUserActeSuperviseur()) +
                       ", UserComptable = " + QString::number(db->getUserConnected()->getIdUserComptable()) +
                       ", UserParent = " + QString::number(db->getUserConnected()->getIdUserParent()) + " where idacte = " + ListActes.at(0).at(0).toString());
        Statut  = RETOURACCUEIL;
        Msg     = ui->MsgtextEdit->toPlainText();
    }

    QString saldatrequete =   "SELECT idPat FROM " NOM_TABLE_SALLEDATTENTE " WHERE idPat = " + QString::number(gPatientEnCours->id());
    QList<QVariantList> ListPatients = db->StandardSelectSQL(saldatrequete,ok,tr("Impossible de trouver le dossier dans la salle d'attente!"));
    QString MsgErreur;
    if (ListPatients.size() == 0)
    {

        saldatrequete =     "INSERT INTO " NOM_TABLE_SALLEDATTENTE
                            " (idPat, idUser, Statut, HeureStatut, idUserEnCoursExam, idActeAPayer, PosteExamen, Message)"
                            " VALUES ('" + QString::number(gPatientEnCours->id()) + "','" + QString::number(db->getUserConnected()->getIdUserActeSuperviseur()) + "','" + Statut + "','"
                            + QTime::currentTime().toString("hh:mm") +"', null," + ActeSal + ",'" + Utils::correctquoteSQL(Msg) + "',null)";
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
        saldatrequete       += ", Message = '" + Utils::correctquoteSQL(Msg) + "'";
        saldatrequete       += " WHERE idPat = '" + QString::number(gPatientEnCours->id()) + "'";
        MsgErreur           = tr("Impossible de modifier les statuts du dossier en salle d'attente!");
    }
    //proc->Edit(saldatrequete);
    db->StandardSQL(saldatrequete,MsgErreur);
    Procedures::I()->MAJTcpMsgEtFlagSalDat();
    accept();
}

void    dlg_salledattente::Slot_EnableOKButton()
{
    OKButton            ->setEnabled(true);
    QRadioButton *btu   = dynamic_cast<QRadioButton*>(sender());
    if (btu != Q_NULLPTR)
    {
        ui->MsgtextEdit ->setEnabled(btu==ui->RetourAccueilradioButton);
        ui->Msglabel    ->setEnabled(btu==ui->RetourAccueilradioButton);
    }
}

