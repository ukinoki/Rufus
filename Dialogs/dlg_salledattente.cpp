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

#include "dlg_salledattente.h"
#include "ui_dlg_salledattente.h"

dlg_salledattente::dlg_salledattente(Acte* act, QString Titre, QWidget *parent):
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionSalDat", parent),
    ui(new Ui::dlg_salledattente)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    m_currentact        = act;

    auto it = Datas::I()->patientsencours->patientsencours()->constFind(Datas::I()->patients->currentpatient()->id());
    if ( it != Datas::I()->patientsencours->patientsencours()->constEnd())
    {
        m_patcrs = const_cast<PatientEnCours*>(it.value());
    }

    ui->MessagetextEdit->setText(Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom() + "\n" + Titre);

    ui->MessagetextEdit->document()->setTextWidth(width()-dlglayout()->contentsMargins().left()*2-2);
    ui->MessagetextEdit->setFixedSize(int(width()-dlglayout()->contentsMargins().left()*2), int(ui->MessagetextEdit->document()->size().height()+2));
    dlglayout()->insertWidget(0,ui->MiseEnSalleDAttentegroupBox);
    dlglayout()->insertWidget(0,ui->MessagetextEdit);
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);

    connect (OKButton,                                  &QPushButton::clicked,  this,   &dlg_salledattente::OKButtonClicked);
    connect (CancelButton,                              &QPushButton::clicked,  this,   &dlg_salledattente::reject);
    connect (ui->ExamEnCoursradioButton,                &QRadioButton::clicked, this,   &dlg_salledattente::EnableOKButton);
    connect (ui->ExamenEnAttenteAutreAvisradioButton,   &QRadioButton::clicked, this,   &dlg_salledattente::EnableOKButton);
    connect (ui->RetourAccueilradioButton,              &QRadioButton::clicked, this,   &dlg_salledattente::EnableOKButton);

    QStringList ListUser;

    ui->UsercomboBox->clear();
    QList<User*> listusrs;
    Datas::I()->postesconnectes->initListe();
    foreach (PosteConnecte* post, *Datas::I()->postesconnectes->postesconnectes())
    {
        if (post->iduser() != Datas::I()->users->userconnected()->id())
        {
            User *usr = Datas::I()->users->getById(post->iduser());
            if (usr)
                if (usr->isSoignant())
                    listusrs << usr;
        }
    }
    if (listusrs.size() == 0)
    {
        ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(false);
        ui->UsercomboBox->setEnabled(false);
    }
    else if (listusrs.size() == 1)
    {
        ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(true);
        ui->ExamenEnAttenteAutreAvisradioButton->setText(tr("Examen terminé, en attente d'être vu par ") + listusrs.at(0)->login());
        ListUser << listusrs.at(0)->login();
        ui->UsercomboBox->setEditText(listusrs.at(0)->login());
        ui->UsercomboBox->insertItems(0,ListUser);
        ui->UsercomboBox->setVisible(false);
    }
    else
    {
        ui->ExamenEnAttenteAutreAvisradioButton->setEnabled(true);
        ui->UsercomboBox->setVisible(true);
        for (int i = 0; i < listusrs.size(); i++)
        {
            ListUser << listusrs.at(i)->login();
            if (i == 0) ui->UsercomboBox->setEditText(listusrs.at(i)->login());
        }
        ui->UsercomboBox->insertItems(0,ListUser);
    }
    OKButton->setEnabled(false);
    ui->MsgtextEdit->setVisible(false);
    if (m_patcrs != Q_NULLPTR)
        ui->MsgtextEdit->setText(m_patcrs->messageretour());
    ui->Msglabel->setVisible(false);
    ui->RetourAccueilradioButton->setText(tr("Examen terminé, en attente des informations de paiement"));
}

dlg_salledattente::~dlg_salledattente()
{
}

void dlg_salledattente::OKButtonClicked()
{
    QString Statut;

    if (!ui->ExamEnCoursradioButton->isChecked() && !ui->ExamenEnAttenteAutreAvisradioButton->isChecked() && !ui->RetourAccueilradioButton->isChecked())
    {
        UpMessageBox::Watch(this,tr("Vous devez choisir un statut avant de mettre le patient en salle d'attente!"));
        return;
    }

    QString ActeSal ="null";
    QString Msg = (m_patcrs? m_patcrs->message():"");
    if (ui->ExamEnCoursradioButton->isChecked())                Statut = ENCOURS;
    if (ui->ExamenEnAttenteAutreAvisradioButton->isChecked())   Statut = ENATTENTENOUVELEXAMEN + ui->UsercomboBox->currentText();
    if (ui->RetourAccueilradioButton->isChecked())
    {
        ActeSal = QString::number(m_currentact->id());
        if (m_currentact->idUserSuperviseur() < 1)
        {
            ItemsList::update(m_currentact, CP_IDUSER_ACTES,         Datas::I()->users->userconnected()->idsuperviseur());
            ItemsList::update(m_currentact, CP_IDUSERPARENT_ACTES,   Datas::I()->users->userconnected()->idparent());
            ItemsList::update(m_currentact, CP_IDUSERCOMPTABLE_ACTES,Datas::I()->users->userconnected()->idcomptableactes());
        }
        Statut  = RETOURACCUEIL;
        Msg     = ui->MsgtextEdit->toPlainText();
    }
    if (m_patcrs == Q_NULLPTR)
        Datas::I()->patientsencours->CreationPatient(Datas::I()->patients->currentpatient()->id(),                                  //! idPat
                                                 Datas::I()->users->getById(Datas::I()->users->userconnected()->idsuperviseur()),   //! User
                                                 Statut,                                                                            //! Statut
                                                 QTime(0,0,0,0),                                                                    //! heureStatut
                                                 QTime(),                                                                           //! heureRDV
                                                 QTime(),                                                                           //! heureArrivee
                                                 "",                                                                                //! Motif
                                                 Msg,                                                                               //! Message
                                                 ActeSal.toInt(),                                                                   //! idActeAPayer
                                                 "",                                                                                //! PosteExamen
                                                 0,                                                                                 //! idUserEnCoursExamen
                                                 0);                                                                                //! idSalDat
    else
    {
        ItemsList::update(m_patcrs, CP_STATUT_SALDAT, Statut);
        ItemsList::update(m_patcrs, CP_IDACTEAPAYER_SALDAT, ActeSal.toInt());
        ItemsList::update(m_patcrs, CP_MESSAGERETOUR_SALDAT, Msg);
        ItemsList::update(m_patcrs, CP_MESSAGE_SALDAT, Msg);       // pour garder la comm_patcrsibilité avec la version 77 de la base
        ItemsList::update(m_patcrs, CP_HEURESTATUT_SALDAT, QTime::currentTime());
        ItemsList::update(m_patcrs, CP_IDUSERENCOURSEXAM_SALDAT);
        ItemsList::update(m_patcrs, CP_POSTEEXAMEN_SALDAT);
    }
    Flags::I()->MAJFlagSalleDAttente();
    accept();
}

void dlg_salledattente::EnableOKButton()
{
    OKButton            ->setEnabled(true);
    QRadioButton *btu   = qobject_cast<QRadioButton*>(sender());
    if (btu != Q_NULLPTR)
    {
        ui->MsgtextEdit ->setVisible(btu==ui->RetourAccueilradioButton);
        ui->Msglabel    ->setVisible(btu==ui->RetourAccueilradioButton);
    }
}

