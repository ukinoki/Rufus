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

#include "gbl_datas.h"
#include "dlg_refraction.h"
#include "icons.h"
#include "ui_dlg_refraction.h"

dlg_refraction::dlg_refraction(ModeOuverture modeouverture, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_refraction)
{
    ui->setupUi(this);
    m_modeouverture = modeouverture;

    setWindowTitle("Refraction - " + Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom());
    setWindowIcon(Icons::icLunettes());
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    InitDivers();
    Init_variables();
    ConnectSignals();

    restoreGeometry(proc->settings()->value(Position_Fiche "Refraction").toByteArray());
    //!> Recherche si Mesure en cours et affichage - la fiche a été appelée par un instrument de mesure
    if (m_modeouverture == Auto)
        AfficheMesureRefracteur();
    else
        RechercheMesureEnCours();
    AfficheKerato();

    ui->SphereOD->setFocus();
    ui->SphereOD->selectAll();
    ui->OupsPushButton->setIcon(Icons::icOups());
    InitEventFilters();
}

dlg_refraction::~dlg_refraction()
{
}

void dlg_refraction::closeEvent(QCloseEvent *)
{
    if (!m_escapeflag)
    {
        if (m_fermecommentaire)
            accept();
        else
            reject();
        proc->settings()->setValue(Position_Fiche "Refraction", saveGeometry());
    }
}

//----------------------------------------------------------------------------------
//  Connection des actions associees a chaque objet du formulaire et aux menus
//----------------------------------------------------------------------------------
void dlg_refraction::ConnectSignals()
{
    foreach (UpDoubleSpinBox* spinbox, findChildren<UpDoubleSpinBox *>())
        connect (spinbox,                           QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                                                                                                this,     &dlg_refraction::Refraction_ValueChanged);
    connect (ui->PorteRadioButton,                  &QRadioButton::clicked,                     this,     &dlg_refraction::RadioButtonFronto_Clicked);
    connect (ui->AutorefRadioButton,                &QRadioButton::clicked,                     this,     &dlg_refraction::RadioButtonAutoref_Clicked);
    connect (ui->ConvODPushButton,                  &QPushButton::clicked,                      this,     &dlg_refraction::ConvODPushButton_Clicked);
    connect (ui->ConvOGPushButton,                  &QPushButton::clicked,                      this,     &dlg_refraction::ConvOGPushButton_Clicked);
    connect (ui->CycloplegieCheckBox,               &QCheckBox::clicked,                        this,     &dlg_refraction::CycloplegieCheckBox_Clicked);

    connect (ui->DepoliODCheckBox,                  &QCheckBox::stateChanged,                   this,     [=] (int a) {DepoliCheckBox_Clicked(ui->DepoliODCheckBox, a);});
    connect (ui->DepoliOGCheckBox,                  &QCheckBox::stateChanged,                   this,     [=] (int a) {DepoliCheckBox_Clicked(ui->DepoliOGCheckBox, a);});
    connect (ui->DeuxMonturesPrescritRadioButton,   &QRadioButton::clicked,                     this,     &dlg_refraction::DeuxMonturesPrescritradioButton_Clicked);
    connect (ui->ODCheckBox,                        &QCheckBox::stateChanged,                   this,     [=] (int a) {ODGCheckBox_Changed(ui->ODCheckBox, a);});
    connect (ui->OGCheckBox,                        &QCheckBox::stateChanged,                   this,     [=] (int a) {ODGCheckBox_Changed(ui->OGCheckBox, a);});

    connect (ui->ODPrescritCheckBox,                &QCheckBox::stateChanged,                   this,     [=] (int a) {PrescritCheckBox_Changed(ui->ODPrescritCheckBox, a);});
    connect (ui->OGPrescritCheckBox,                &QCheckBox::stateChanged,                   this,     [=] (int a) {PrescritCheckBox_Changed(ui->OGPrescritCheckBox, a);});
    connect (ui->PlanODCheckBox,                    &QCheckBox::stateChanged,                   this,     [=] (int a) {PlanCheckBox_Changed(ui->PlanODCheckBox, a);});
    connect (ui->PlanOGCheckBox,                    &QCheckBox::stateChanged,                   this,     [=] (int a) {PlanCheckBox_Changed(ui->PlanOGCheckBox, a);});
    connect (ui->PrescriptionRadioButton,           &QRadioButton::clicked,                     this,     &dlg_refraction::RadioButtonPrescription_clicked);

    connect (ui->RefractionRadioButton,             &QRadioButton::clicked,                     this,     &dlg_refraction::RadioButtonRefraction_Clicked);
    connect (ui->RyserODCheckBox,                   &QCheckBox::stateChanged,                   this,     [=] (int a) {RyserCheckBox_Clicked(ui->RyserODCheckBox, a);});
    connect (ui->RyserOGCheckBox,                   &QCheckBox::stateChanged,                   this,     [=] (int a) {RyserCheckBox_Clicked(ui->RyserOGCheckBox, a);});
    connect (ui->UneMonturePrescritRadioButton,     &QRadioButton::clicked,                     this,     &dlg_refraction::UneMonturePrescritRadioButton_Clicked);
    connect (ui->V2RadioButton,                     &QRadioButton::clicked,                     this,     &dlg_refraction::RegleAffichageFiche);

    connect (ui->V2PrescritRadioButton,             &QRadioButton::clicked,                     this,     &dlg_refraction::VPrescritRadioButton_Clicked);
    connect (ui->VerresTeintesCheckBox,             &QCheckBox::stateChanged,                   this,     &dlg_refraction::VerresTeintesCheckBox_Changed);
    connect (ui->VLRadioButton,                     &QRadioButton::clicked,                     this,     &dlg_refraction::RegleAffichageFiche);
    connect (ui->VLPrescritRadioButton,             &QRadioButton::clicked,                     this,     &dlg_refraction::VPrescritRadioButton_Clicked);
    connect (ui->VPRadioButton,                     &QRadioButton::clicked,                     this,     &dlg_refraction::RegleAffichageFiche);
    connect (ui->VPPrescritRadioButton,             &QRadioButton::clicked,                     this,     &dlg_refraction::VPrescritRadioButton_Clicked);


    connect (ui->AxeCylindreOD,                     QOverload<int>::of(&QSpinBox::valueChanged),this,     &dlg_refraction::Refraction_ValueChanged);
    connect (ui->AxeCylindreOG,                     QOverload<int>::of(&QSpinBox::valueChanged),this,     &dlg_refraction::Refraction_ValueChanged);
    connect (ui->BasePrismeOD,                      QOverload<int>::of(&QSpinBox::valueChanged),
                                                                                                this,     [=] {BasePrisme_ValueChanged(ui->BasePrismeOD);});
    connect (ui->BasePrismeOG,                      QOverload<int>::of(&QSpinBox::valueChanged),
                                                                                                this,     [=] {BasePrisme_ValueChanged(ui->BasePrismeOG);});
    connect (ui->BasePrismeTextODComboBox,          QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                                this,     &dlg_refraction::BasePrismeTextODComboBox_Changed);
    connect (ui->BasePrismeTextOGComboBox,          QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                                                this,     &dlg_refraction::BasePrismeTextOGComboBox_Changed);
    connect (ui->CommentairePrescriptionTextEdit,   &QTextEdit::textChanged,                    this,     &dlg_refraction::CommentairePrescriptionTextEdit_Changed);

    connect (ui->PressonODCheckBox,                 &QCheckBox::clicked,                        this,     &dlg_refraction::PressonCheckBox_Changed);
    connect (ui->PressonOGCheckBox,                 &QCheckBox::clicked,                        this,     &dlg_refraction::PressonCheckBox_Changed);

    connect (ui->RyserSpinBox,                      QOverload<int>::of(&QSpinBox::valueChanged),this,     &dlg_refraction::Refraction_ValueChanged);

    connect (ui->AnnulPushButton,                   &QPushButton::clicked,                      this,     &dlg_refraction::AnnulPushButton_Clicked);
    connect (ui->AppelCommentPushButton,            &QPushButton::clicked,                      this,     &dlg_refraction::Commentaires);
    connect (ui->DetailsPushButton,                 &QPushButton::clicked,                      this,     &dlg_refraction::Detail_Clicked);

    connect (ui->OupsPushButton,                    &QPushButton::clicked,                      this,     &dlg_refraction::OupsButtonClicked);
    connect (ui->ResumePushButton,                  &QPushButton::clicked,                      this,     &dlg_refraction::ResumePushButton_Clicked);
    connect (ui->OKPushButton,                      &QPushButton::clicked,                      this,     &dlg_refraction::OKPushButton_Clicked);
    connect (ui->ReprendrePushButton,               &QPushButton::clicked,                      this,     &dlg_refraction::ReprendreButtonClicked);

    if (proc->PortFronto()!=Q_NULLPTR || proc->PortAutoref()!=Q_NULLPTR || proc->PortRefracteur()!=Q_NULLPTR)
        connect (proc,                              &Procedures::NouvMesure,                    this,     &dlg_refraction::NouvMesureRefraction);
}

//1. Les RadioButton, checkBox, combo...etc...--------------------------------------------------------------------------------
void dlg_refraction::RadioButtonAutoref_Clicked()
{
    m_mode = Refraction::Autoref;
    RegleAffichageFiche();
}
void dlg_refraction::RadioButtonFronto_Clicked()
{
    m_mode = Refraction::Fronto;
    RegleAffichageFiche();
}
void dlg_refraction::RadioButtonRefraction_Clicked()
{
    m_mode = Refraction::Acuite;
    if(!ui->CycloplegieCheckBox->isChecked())   ui->V2RadioButton->setChecked(true);
    RegleAffichageFiche();
}
void dlg_refraction::RadioButtonPrescription_clicked()
{
    m_mode = Refraction::Prescription;
    ui->ODPrescritCheckBox->setChecked(ui->ODCheckBox->isChecked());
    ui->OGPrescritCheckBox->setChecked(ui->OGCheckBox->isChecked());
    ui->VLPrescritRadioButton->setChecked(ui->VLRadioButton->isChecked());
    ui->VPPrescritRadioButton->setChecked(ui->VPRadioButton->isChecked());
    if (ui->V2RadioButton->isChecked())
    {
        if (!ui->ODCheckBox->isChecked() && ui->AddVPOG->value() == 0.0) ui->VLPrescritRadioButton->setChecked(true);
        else if (!ui->OGCheckBox->isChecked() && ui->AddVPOD->value() == 0.0) ui->VLPrescritRadioButton->setChecked(true);
        else if (ui->ODCheckBox->isChecked() && ui->OGCheckBox->isChecked() && ui->AddVPOD->value() == 0.0 && ui->AddVPOG->value() == 0.0)
            ui->VLPrescritRadioButton->setChecked(true);
        else ui->V2PrescritRadioButton->setChecked(true);
    }
    Datas::I()->commentslunets->initListe();
    m_commentaire           = CommentaireObligatoire();
    RegleAffichageFiche();
}

void dlg_refraction::CycloplegieCheckBox_Clicked()
{
    if (ui->RefractionRadioButton->isChecked())
        RegleAffichageFiche();
}
void dlg_refraction::PressonCheckBox_Changed()
{
    if (m_mode == Refraction::Prescription) ResumePrescription();
}

void dlg_refraction::BasePrismeTextODComboBox_Changed(int index)
{
    switch (index) {
        case 0 :    ui->BasePrismeOD->setValue(0);      break;
        case 1 :    ui->BasePrismeOD->setValue(90);     break;
        case 2 :    ui->BasePrismeOD->setValue(180);    break;
        case 3 :    ui->BasePrismeOD->setValue(270);    break;
        default:    break;
        }
}
void dlg_refraction::BasePrismeTextOGComboBox_Changed(int index)
{
    switch (index) {
        case 0 :    ui->BasePrismeOG->setValue(180);     break;
        case 1 :    ui->BasePrismeOG->setValue(90);      break;
        case 2 :    ui->BasePrismeOG->setValue(0);       break;
        case 3 :    ui->BasePrismeOG->setValue(270);     break;
        default:    break;
        }
}

void dlg_refraction::DepoliCheckBox_Clicked(QCheckBox *check, int etat)
{
    switch (etat) {
    case Qt::Checked:
        // pas de depoli pour les 2 yeux
        if (check == ui->DepoliODCheckBox)    ui->DepoliOGCheckBox->setEnabled(false);
        if (check == ui->DepoliOGCheckBox)    ui->DepoliODCheckBox->setEnabled(false);
        if (check == ui->DepoliOGCheckBox)
        {
            if (focusWidget() == ui->SphereOG)          ui->SphereOD->setFocus();
            if (focusWidget() == ui->CylindreOG)        ui->CylindreOD->setFocus();
            if (focusWidget() == ui->AxeCylindreOG)     ui->AxeCylindreOD->setFocus();
            if (focusWidget() == ui->AVLOGupComboBox)   ui->AVLODupComboBox->setFocus();
            if (focusWidget() == ui->AVPOGupComboBox)   ui->AVPODupComboBox->setFocus();
            if (focusWidget() == ui->AddVPOG)           ui->AddVPOD->setFocus();
            if (focusWidget() == ui->K1OG)              ui->K1OD->setFocus();
            if (focusWidget() == ui->K2OG)              ui->K2OD->setFocus();
            if (focusWidget() == ui->AxeKOG)            ui->AxeKOD->setFocus();
            // un seul type autorise
            ui->PlanOGCheckBox->setChecked(false);
        }
        if (check == ui->DepoliODCheckBox)
        {
            if (focusWidget() == ui->SphereOD)          ui->SphereOG->setFocus();
            if (focusWidget() == ui->CylindreOD)        ui->CylindreOG->setFocus();
            if (focusWidget() == ui->AxeCylindreOD)     ui->AxeCylindreOG->setFocus();
            if (focusWidget() == ui->AVLODupComboBox)   ui->AVLOGupComboBox->setFocus();
            if (focusWidget() == ui->AVPODupComboBox)   ui->AVPOGupComboBox->setFocus();
            if (focusWidget() == ui->AddVPOD)           ui->AddVPOG->setFocus();
            if (focusWidget() == ui->K1OD)              ui->K1OG->setFocus();
            if (focusWidget() == ui->K2OD)              ui->K2OG->setFocus();
            if (focusWidget() == ui->AxeKOD)            ui->AxeKOG->setFocus();
            // un seul type autorise
            ui->PlanODCheckBox->setChecked(false);
        }
        break;
    case Qt::Unchecked:
        ui->DepoliODCheckBox->setEnabled(true);
        ui->DepoliOGCheckBox->setEnabled(true);
        break;
    default:
        break;
    }
    // si depoli on masque les mesures
    RegleAffichageFiche();
    if (m_mode == Refraction::Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
}

void dlg_refraction::DeuxMonturesPrescritradioButton_Clicked()
{
    if (ui->UneMonturePrescritRadioButton->isChecked() && ui->DeuxMonturesPrescritRadioButton->isChecked())
        ui->UneMonturePrescritRadioButton->setChecked(false);
    ResumePrescription();
}

void dlg_refraction::ODGCheckBox_Changed(QCheckBox* check, int etat)
{
    switch (etat) {
    case Qt::Unchecked:
        if (check == ui->ODCheckBox)
        {
            if (focusWidget() == ui->SphereOD)          ui->SphereOG->setFocus();
            if (focusWidget() == ui->CylindreOD)        ui->CylindreOG->setFocus();
            if (focusWidget() == ui->AxeCylindreOD)     ui->AxeCylindreOG->setFocus();
            if (focusWidget() == ui->AVLODupComboBox)   ui->AVLOGupComboBox->setFocus();
            if (focusWidget() == ui->AVPODupComboBox)   ui->AVPOGupComboBox->setFocus();
            if (focusWidget() == ui->AddVPOD)           ui->AddVPOG->setFocus();
            if (focusWidget() == ui->K1OD)              ui->K1OG->setFocus();
            if (focusWidget() == ui->K2OD)              ui->K2OG->setFocus();
            if (focusWidget() == ui->AxeKOD)            ui->AxeKOG->setFocus();
            if (!ui->OGCheckBox->isChecked())           ui->OGCheckBox->setChecked(true);
        }
        if (check == ui->OGCheckBox)
        {
            if (focusWidget() == ui->SphereOG)          ui->SphereOD->setFocus();
            if (focusWidget() == ui->CylindreOG)        ui->CylindreOD->setFocus();
            if (focusWidget() == ui->AxeCylindreOG)     ui->AxeCylindreOD->setFocus();
            if (focusWidget() == ui->AVLOGupComboBox)   ui->AVLODupComboBox->setFocus();
            if (focusWidget() == ui->AVPOGupComboBox)   ui->AVPODupComboBox->setFocus();
            if (focusWidget() == ui->AddVPOG)           ui->AddVPOD->setFocus();
            if (focusWidget() == ui->K1OG)              ui->K1OD->setFocus();
            if (focusWidget() == ui->K2OG)              ui->K2OD->setFocus();
            if (focusWidget() == ui->AxeKOG)            ui->AxeKOD->setFocus();
            if (!ui->ODCheckBox->isChecked())           ui->ODCheckBox->setChecked(true);
        }
        break;
    default:
        break;
    }
    RegleAffichageFiche();
}

void dlg_refraction::PrescritCheckBox_Changed(QCheckBox* check, int etat)
{   // pas 2 yeux decoches en meme temps
    switch (etat) {
    case Qt::Checked:
        ui->ODPrescritCheckBox->setEnabled(true);
        ui->OGPrescritCheckBox->setEnabled(true);
        break;
    case Qt::Unchecked:
        if (check == ui->ODPrescritCheckBox)    ui->OGPrescritCheckBox->setEnabled(false);
        if (check == ui->OGPrescritCheckBox)    ui->ODPrescritCheckBox->setEnabled(false);
        break;
    default:
        break;
    }
    ResumePrescription();
}

void dlg_refraction::PlanCheckBox_Changed(QCheckBox* check, int etat)
{
    if (check == ui->PlanODCheckBox)
    {
        switch (etat) {
        case 2:
            if (focusWidget() == ui->SphereOD)              ui->SphereOG->setFocus();
            else if (focusWidget() == ui->CylindreOD)       ui->CylindreOG->setFocus();
            else if (focusWidget() == ui->AxeCylindreOD)    ui->AxeCylindreOG->setFocus();
            else if (focusWidget() == ui->AddVPOD)          ui->AddVPOG->setFocus();
            // un seul type autorise
            if (ui->DepoliODCheckBox->isChecked())          ui->DepoliODCheckBox->setChecked(false);
            break;
        case 0:
            ui->SphereOD->setFocus();
            break;
        default:
            break;
        }
    }
    if (check == ui->PlanOGCheckBox)
    {
        switch (etat) {
        case 2:
            if (focusWidget() == ui->SphereOG)              ui->SphereOD->setFocus();
            else if (focusWidget() == ui->CylindreOG)       ui->CylindreOD->setFocus();
            else if (focusWidget() == ui->AxeCylindreOG)    ui->AxeCylindreOD->setFocus();
            else if (focusWidget() == ui->AddVPOG)          ui->AddVPOD->setFocus();
            // un seul type autorise
            if (ui->DepoliOGCheckBox->isChecked())          ui->DepoliOGCheckBox->setChecked(false);
            break;
        case 0:
            ui->SphereOG->setFocus();
            break;
        default:
            break;
        }
    }

    RegleAffichageFiche();
    if (m_mode == Refraction::Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
}

void dlg_refraction::RyserCheckBox_Clicked(QCheckBox* check, int etat)
{
    switch (etat) {
    case Qt::Checked:
        // pas de Ryser pour les 2 yeux
        if (check == ui->RyserODCheckBox)    ui->RyserOGCheckBox->setEnabled(false);
        if (check == ui->RyserOGCheckBox)    ui->RyserODCheckBox->setEnabled(false);
        break;
    case Qt::Unchecked:
        ui->RyserODCheckBox->setEnabled(true);
        ui->RyserOGCheckBox->setEnabled(true);
        break;
    default:
        break;
    }
    ui->RyserSpinBox->setVisible(ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked());

    if (m_mode == Refraction::Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
}

void dlg_refraction::UneMonturePrescritRadioButton_Clicked()
{
    if (ui->UneMonturePrescritRadioButton->isChecked() && ui->DeuxMonturesPrescritRadioButton->isChecked())
        ui->DeuxMonturesPrescritRadioButton->setChecked(false);
    ResumePrescription();
}

void dlg_refraction::VerresTeintesCheckBox_Changed()
{
    ResumePrescription();
}

void dlg_refraction::VPrescritRadioButton_Clicked()
{
    if (ui->V2PrescritRadioButton->isChecked())
    {
        ui->DeuxMonturesPrescritRadioButton->setVisible(true);
    }
    else
    {
        ui->DeuxMonturesPrescritRadioButton->setChecked(false);
        ui->DeuxMonturesPrescritRadioButton->setVisible(false);
    }
    ResumePrescription();
}


//2. Les Line Edit et TextEdit --------------------------------------------------------------------------------------------------
void dlg_refraction::BasePrisme_ValueChanged(QSpinBox *box)
{
    int a = box->value();
    if (box == ui->BasePrismeOD)
        switch (a) {
        case 0:     ui->BasePrismeTextODComboBox->setCurrentIndex(0);   break;
        case 90:    ui->BasePrismeTextODComboBox->setCurrentIndex(1);   break;
        case 180:   ui->BasePrismeTextODComboBox->setCurrentIndex(2);   break;
        case 270:   ui->BasePrismeTextODComboBox->setCurrentIndex(3);   break;
        default:    ui->BasePrismeTextODComboBox->setCurrentIndex(-1);  break;}
    else if (box == ui->BasePrismeOG)
        switch (a) {
        case 180:   ui->BasePrismeTextOGComboBox->setCurrentIndex(0);   break;
        case 90:    ui->BasePrismeTextOGComboBox->setCurrentIndex(1);   break;
        case 0:     ui->BasePrismeTextOGComboBox->setCurrentIndex(2);   break;
        case 270:   ui->BasePrismeTextOGComboBox->setCurrentIndex(3);   break;
        default:    ui->BasePrismeTextOGComboBox->setCurrentIndex(-1);  break;}
    if (m_mode == Refraction::Prescription) ResumePrescription();
}

void dlg_refraction::CommentairePrescriptionTextEdit_Changed()    // 01.07.2014
{
    ResumePrescription();
}

void dlg_refraction::Refraction_ValueChanged()
{
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
    if (m_mode == Refraction::Prescription) ResumePrescription();
}

//3. Les pushButton ----------------------------------------------------------------------------------------------
void dlg_refraction::AnnulPushButton_Clicked()
{
    FermeFiche(Annul);
}

void dlg_refraction::Commentaires()
{
    dlg_listecommentaires *Dlg_Comments    = new dlg_listecommentaires(m_listcommentaires, this);
    if (Dlg_Comments->exec() > 0)
    {
        Dlg_Comments->close(); // nécessaire pour enregistrer la géométrie
        m_commentaire = "";
        m_commentaireresume = "";
        m_listcommentaires = Dlg_Comments->ListeCommentaires();
        for (int i=0; i<m_listcommentaires.size(); ++i) {
            CommentLunet *com = m_listcommentaires.at(i);
            if (com)
            {
                m_commentaire += (m_commentaire!= ""? "\n" + com->texte() : com->texte());
                if ((com->isdefaut() && com->iduser() != currentuser()->id()) || !com->isdefaut())
                    m_commentaireresume += (m_commentaireresume!= ""? " - " + com->resume() : com->resume());
            }
        }
    }
    ResumePrescription();
    delete Dlg_Comments;
}

void dlg_refraction::ConvODPushButton_Clicked()
{
    if ((ui->SphereOD->value() + ui->CylindreOD->value()) > 20)
        {UpMessageBox::Watch(this,tr("Réfraction"), tr("Conversion refusée !"));
         return;
        }
    ui->SphereOD->setValuewithPrefix(ui->SphereOD->value() + ui->CylindreOD->value());
    ui->CylindreOD->setValuewithPrefix(ui->CylindreOD->value() * -1);
    if (ui->AxeCylindreOD->value() >= 90)
         ui->AxeCylindreOD->setValue(ui->AxeCylindreOD->value() - 90);
    else
        ui->AxeCylindreOD->setValue(ui->AxeCylindreOD->value() + 90);
}

void dlg_refraction::ConvOGPushButton_Clicked()
{
    if ((ui->SphereOG->value() + ui->CylindreOG->value()) > 20)
        {UpMessageBox::Watch(this,tr("Réfraction"), tr("Conversion refusée !"));
         return;
        }
    ui->SphereOG->setValuewithPrefix(ui->SphereOG->value() + ui->CylindreOG->value());
    ui->CylindreOG->setValuewithPrefix(ui->CylindreOG->value() * -1);
    if (ui->AxeCylindreOG->value() >= 90)
        ui->AxeCylindreOG->setValue(ui->AxeCylindreOG->value() - 90);
    else
        ui->AxeCylindreOG->setValue(ui->AxeCylindreOG->value() + 90);
}

void dlg_refraction::Detail_Clicked()
{
    if (m_affichedetail &&
        (ui->PrismeOD->value() != 0.0       || ui->PrismeOG->value() != 0.0     ||
        ui->RyserODCheckBox->isChecked()    || ui->RyserOGCheckBox->isChecked() ||
        ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()  ||
        ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked()))
        return;
    else
        m_affichedetail = !m_affichedetail;
    AfficherDetail(m_affichedetail);
    if (m_mode == Refraction::Fronto)
    {
        if (m_affichedetail)
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    }
    if (m_mode == Refraction::Prescription)
    {
        if (m_affichedetail)
            setFixedSize(width(), HAUTEUR_AVEC_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_AVEC_ORDONNANCE_SANS_DETAIL);
    }
}

//----------------------------------------------------------------------------------
// OKPushButton
//----------------------------------------------------------------------------------
void dlg_refraction::OKPushButton_Clicked()
{
    focusNextChild();
    m_flagbugvalidenter = 0;

    UpDoubleSpinBox *dblSpin = dynamic_cast<UpDoubleSpinBox *>(focusWidget());
    if (dblSpin)
    {
        int a = int(dblSpin->value()/dblSpin->singleStep());
        int b = int(dblSpin->value()*10);
        if (a != int(dblSpin->value()/dblSpin->singleStep()))
            if (((dblSpin->singleStep() == 0.25) && ((abs(b)%10 != 2 && abs(b)%10 != 7) || b != int(dblSpin->value()*10)))
               || dblSpin->singleStep() == 0.50)
            {
                Utils::playAlarm();
                dblSpin->setFocus();
                dblSpin->selectAll();
                return;
            }
    }
    dblSpin = Q_NULLPTR;
    delete dblSpin;

    if (!ControleCoherence())        return;

    if (m_mode == Refraction::Fronto)
    {
        // On vérifie s'il existe un enregistrement identique au meme jour pour ne pas surcharger la table
        if (LectureMesure(Aujourdhui, Refraction::Fronto, Refraction::NoDilatation, CalculFormule_OD(), CalculFormule_OG()) == Q_NULLPTR) // il n'y en a pas - on suit la procédure normale
        {
            Refraction *ref = InsertRefraction();
            if (ref == Q_NULLPTR)
                return;
            m_idrefraction = ref->id();
            MajDonneesOphtaPatient();
        }
        FermeFiche(OK);
    }

    else if (m_mode == Refraction::Autoref || m_mode == Refraction::Acuite)
    {
        // On vérifie s'il existe un enregistrement identique et si oui, on l'écrase
        Refraction::Cycloplegie dilat = (ui->CycloplegieCheckBox->isChecked()? Refraction::Dilatation : Refraction::NoDilatation);
        Refraction * ref = LectureMesure(Aujourdhui, m_mode, dilat);
        if (ref != Q_NULLPTR)
            DetruireLaMesure(ref);
        ref = InsertRefraction();
        if (ref == Q_NULLPTR)
            return;
        m_idrefraction = ref->id();
        MajDonneesOphtaPatient();
        FermeFiche(OK);
    }
    else if (m_mode == Refraction::Prescription)
        FermeFiche(Imprime);
}

void dlg_refraction::OupsButtonClicked()
{
    OuvrirListeMesures(dlg_refractionlistemesures::Supprimer);
}

void dlg_refraction::NouvMesureRefraction(Procedures::TypeMesure TypeMesure)
{
    if (TypeMesure == Procedures::MesureRefracteur)
        AfficheMesureRefracteur();
    else if (TypeMesure == Procedures::MesureFronto)       //! c'est le fronto et pas le refracteur (cas où le fronto est branché sur la box) qui a transmis une mesure d'autoref, on l'affiche
        AfficheMesureFronto();
    else if (TypeMesure == Procedures::MesureAutoref)     //! c'est l'autoref et pas le refracteur (cas où l'autoref est branché sur la box) qui a transmis une mesure de fronto, on l'affiche
        AfficheMesureAutoref();
}

void dlg_refraction::ReprendreButtonClicked()
{
    OuvrirListeMesures(dlg_refractionlistemesures::Recuperer);
}
void dlg_refraction::ResumePushButton_Clicked()
{
    ResumeRefraction();
}

// ------------------------------------------------------------------------------------------
// Traitement des touches avant prise en compte par QT
// pour navigation non stnadard
// ------------------------------------------------------------------------------------------
 // A REVOIR les Fleches Gauches et droites sur les radio boutton de mesure ??????

bool dlg_refraction::eventFilter(QObject *obj, QEvent *event) // A REVOIR
{
    if (event->type() == QEvent::FocusIn )
    {
        UpLineEdit* objUpLine = dynamic_cast<UpLineEdit*>(obj);
        if (objUpLine != Q_NULLPTR)        {
            objUpLine->selectAll();
            return false;
        }
        UpDoubleSpinBox* objUpdSpin = dynamic_cast<UpDoubleSpinBox*>(obj);
        if (objUpdSpin != Q_NULLPTR)   {
            objUpdSpin->setPrefix("");
            objUpdSpin->selectAll();
            return false;
        }
        UpComboBox* objUpCombo = dynamic_cast<UpComboBox*>(obj);
        if (objUpCombo != Q_NULLPTR)   {
            objUpCombo->setCurrentIndex(objUpCombo->findText(objUpCombo->currentText()));
            return false;
        }
        UpSpinBox* objUpSpin = dynamic_cast<UpSpinBox*>(obj);
        if (objUpSpin != Q_NULLPTR)   {
            objUpSpin->selectAll();
            return false;
        }
//        UpGroupBox* box = dynamic_cast<UpGroupBox*>(obj->parent());
//        if (box != Q_NULLPTR){
//            box->setStyleSheet(STYLE_UPGROUBOXACTIVE);
//            return false;
//        }
    }

    if (event->type() == QEvent::FocusOut )
    {
//        UpGroupBox* box = dynamic_cast<UpGroupBox*>(obj->parent());
//        if (box!=Q_NULLPTR)
//            box->setStyleSheet(STYLE_UPGROUBOXINACTIVE);
        if (obj == ui->CylindreOD)          if (ui->CylindreOD->value() == 0.0)   ui->AxeCylindreOD->setValue(0);
        if (obj == ui->CylindreOG)          if (ui->CylindreOG->value() == 0.0)   ui->AxeCylindreOG->setValue(0);
        if (obj == ui->AddVPOG) QuitteAddVP(ui->AddVPOG);
        if (obj == ui->AddVPOD) QuitteAddVP(ui->AddVPOD);
        if (obj == ui->K1OD || obj == ui->K2OD || obj == ui->K1OG || obj == ui->K2OG)
            return QWidget::eventFilter(obj, event); //traité par la classe uplineedit.h
    }

    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QWidget *widg = static_cast<QWidget *>(obj);
        m_flagbugvalidenter = 0;
        if (keyEvent->key() == Qt::Key_Escape)
        {
            if (obj->inherits("UpLineEdit"))
            {
                UpLineEdit* objUpLine = static_cast<UpLineEdit*>(obj);
                objUpLine->setText(objUpLine->valeuravant());
                objUpLine = Q_NULLPTR;
            }
        }

        if (keyEvent->key() == Qt::Key_Left  && !ui->CommentaireGroupBox->isAncestorOf(widg))
        {        // quand le focus entre sur un qGroupBox rempli de radioButton, il doit se porter sur le radiobutton qui est coché
            QGroupBox *boxdep;
            boxdep = Q_NULLPTR ;
            QList<QGroupBox *> listbox = findChildren<QGroupBox *>();
            for (int i=0; i<listbox.size(); i++)
            {
                if (listbox.at(i)->isAncestorOf(widg))
                {
                    boxdep = listbox.at(i);
                    i = listbox.size();
                }
            }
            if (obj == ui->AddVPOG) QuitteAddVP(ui->AddVPOG);
            if (obj == ui->AddVPOD) QuitteAddVP(ui->AddVPOD);
            if (!(obj == ui->DateDateEdit
                  && ui->DateDateEdit->currentSection() != QDateTimeEdit::DaySection))
                focusPreviousChild();
            QRadioButton *radio = dynamic_cast<QRadioButton *>(focusWidget());
            if (radio != Q_NULLPTR)
                if (!radio->isChecked())
                {
                    for (int i=0; i<listbox.size(); i++)
                        if (listbox.at(i)->isAncestorOf(radio)){
                            if (listbox.at(i) != boxdep){
                                QList<QRadioButton *> listradio = listbox.at(i)->findChildren<QRadioButton *>();
                                for (int k=0; k < listradio.size(); k++)
                                    if (listradio.at(k)->isChecked())   {listradio.at(k)->setFocus();   k = listradio.size();}
                            }
                            i = listbox.size();
                        }
                }
        }

        if (keyEvent->key() == Qt::Key_Right  && !ui->CommentaireGroupBox->isAncestorOf(widg))
        {        // quand le focus entre sur un qGroupBox rempli de radioButton, il doit se porter sur le radiobutton qui est coché
            QGroupBox *boxdep;
            boxdep = Q_NULLPTR;
            QList<QGroupBox *> listbox = findChildren<QGroupBox *>();
            for (int i=0; i<listbox.size(); i++)
                if (listbox.at(i)->isAncestorOf(widg))
                {
                    boxdep = listbox.at(i);
                    i = listbox.size();
                }
            if (obj == ui->AddVPOG) QuitteAddVP(ui->AddVPOG);
            if (obj == ui->AddVPOD) QuitteAddVP(ui->AddVPOD);
            if (!(obj == ui->DateDateEdit
                  && ui->DateDateEdit->currentSection() != QDateTimeEdit::YearSection))
                focusNextChild();
            QRadioButton *radio = dynamic_cast<QRadioButton *>(focusWidget());
            if (radio != Q_NULLPTR )
                if (!radio->isChecked())
                {
                    for (int i=0; i<listbox.size(); i++)
                        if (listbox.at(i)->isAncestorOf(radio)){
                            if (listbox.at(i) != boxdep)
                            {
                                QList<QRadioButton *> listradio = listbox.at(i)->findChildren<QRadioButton *>();
                                for (int k=0; k < listradio.size(); k++)
                                    if (listradio.at(k)->isChecked())   {listradio.at(k)->setFocus();   k = listradio.size();}
                            }
                            i = listbox.size();
                        }
                }
        }

        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key()==Qt::Key_Enter)  && !ui->CommentaireGroupBox->isAncestorOf(widg))            // Return - Idem Flèche Droite - On boucle dans la box en cours ---------------------------
        {
            if (keyEvent->modifiers() == Qt::MetaModifier)
            {
                OKPushButton_Clicked();
                return true;
            }
            m_flagbugvalidenter = 1; // on évite ainsi le second appel à ValidVerrres qui va être émis pas la touche flèche simulée
            QKeyEvent *newevent = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Right , Qt::NoModifier);
            QCoreApplication::postEvent (obj, newevent);
            return QWidget::eventFilter(obj, newevent);
        }

        if (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() == Qt::ShiftModifier)            // SHIFT TAB - On boucle sur les box ------------------------------------------------------------
        {
            if (obj == ui->AddVPOG) QuitteAddVP(ui->AddVPOG);
            if (obj == ui->AddVPOD) QuitteAddVP(ui->AddVPOD);
            if (ui->MesureGroupBox->isAncestorOf(widg))
            {
                if (m_mode == Refraction::Fronto && ui->PrismeGroupBox->isVisible())                                   return DeplaceVers(ui->PrismeGroupBox,"Fin");
                if (m_mode == Refraction::Fronto || (m_mode == Refraction::Acuite && ui->QuelleDistanceGroupBox->isVisible()))
                                                                                                                       return DeplaceVers(ui->QuelleDistanceGroupBox);
                if (m_mode == Refraction::Acuite && !ui->QuelleDistanceGroupBox->isVisible())                          return DeplaceVers(ui->QuelleMesureGroupBox);
                if (m_mode == Refraction::Autoref)                                                                     return DeplaceVers(ui->KeratometrieGroupBox,"Fin");
                if (m_mode == Refraction::Prescription)                                                                return DeplaceVers(ui->QuelleMontureGroupBox);
                return true;
            }
            if (ui->QuelleMesureGroupBox->isAncestorOf(widg))               return DeplaceVers(ui->MesureGroupBox,"Fin");
            if (ui->QuelleDistanceGroupBox->isAncestorOf(widg))             return DeplaceVers(ui->QuelleMesureGroupBox);
            if (ui->KeratometrieGroupBox->isAncestorOf(widg))               return DeplaceVers(ui->QuelleMesureGroupBox);
            if (ui->PrismeGroupBox->isAncestorOf(widg))
            {
                if (ui->QuelleDistanceGroupBox->isEnabled())
                     return DeplaceVers(ui->QuelleDistanceGroupBox);
                else return DeplaceVers(ui->QuelleMesureGroupBox);
            }
            if (ui->QuelsVerresGroupBox->isAncestorOf(widg))
            {
                if (ui->PrismeGroupBox->isVisible())
                    DeplaceVers(ui->PrismeGroupBox,"Fin");
                else
                {
                    if (m_mode == Refraction::Fronto) return DeplaceVers(ui->QuelleDistanceGroupBox);
                    else return DeplaceVers(ui->QuelleMesureGroupBox);
                }
                return true;
            }
            if (ui->QuelOeilGroupBox->isAncestorOf(widg))                   return DeplaceVers(ui->QuelsVerresGroupBox);
            if (ui->QuelleMontureGroupBox->isAncestorOf(widg))              return DeplaceVers(ui->QuelOeilGroupBox);
            if (obj == ui->DateDateEdit)                                    return DeplaceVers(ui->QuelleMontureGroupBox);
        }

        if (keyEvent->key()==Qt::Key_Tab && keyEvent->modifiers() == Qt::NoModifier)            // TAB - On boucle sur les box ------------------------------------------------------------
        {
            if (obj == ui->AddVPOG) QuitteAddVP(ui->AddVPOG);
            if (obj == ui->AddVPOD) QuitteAddVP(ui->AddVPOD);
            if (ui->MesureGroupBox->isAncestorOf(widg))                     return DeplaceVers(ui->QuelleMesureGroupBox);
            if (ui->QuelleMesureGroupBox->isAncestorOf(widg))
            {
                if (m_mode == Refraction::Fronto || (m_mode == Refraction:: Acuite && ui->QuelleDistanceGroupBox->isVisible()))  return DeplaceVers(ui->QuelleDistanceGroupBox);
                if (m_mode == Refraction::Acuite && !ui->QuelleDistanceGroupBox->isVisible())                                   return DeplaceVers(ui->MesureGroupBox,"Debut");
                if (m_mode == Refraction::Autoref)                                                                              return DeplaceVers(ui->KeratometrieGroupBox,"Debut");
                if (m_mode == Refraction::Prescription)
                {
                    if (ui->PrismeGroupBox->isVisible())
                        return DeplaceVers(ui->PrismeGroupBox,"Debut");
                    return DeplaceVers(ui->QuelsVerresGroupBox);
                }
                return true;
            }
            if (ui->QuelleDistanceGroupBox->isAncestorOf(widg))
            {
                if (ui->PrismeGroupBox->isVisible())
                    return DeplaceVers(ui->PrismeGroupBox,"Debut");
                else return DeplaceVers(ui->MesureGroupBox,"Debut");
            }
            if (ui->KeratometrieGroupBox->isAncestorOf(widg))               return DeplaceVers(ui->MesureGroupBox,"Debut");
            if (ui->PrismeGroupBox->isAncestorOf(widg) || ui->CommentaireGroupBox->isAncestorOf(widg))
            {
                if (ui->QuelsVerresGroupBox->isVisible())
                    return DeplaceVers(ui->QuelsVerresGroupBox);
                else return DeplaceVers(ui->MesureGroupBox,"Debut");
            }
            if (ui->QuelsVerresGroupBox->isAncestorOf(widg))                return DeplaceVers(ui->QuelOeilGroupBox);
            if (ui->QuelOeilGroupBox->isAncestorOf(widg))                   return DeplaceVers(ui->QuelleMontureGroupBox);
            if (ui->QuelleMontureGroupBox->isAncestorOf(widg))              return DeplaceVers(ui->MesureGroupBox,"Debut");
            if (obj == ui->DateDateEdit)                                    return DeplaceVers(ui->MesureGroupBox,"Debut");
        }

        if(keyEvent->key()==Qt::Key_Up)
        {
            if (obj == ui->K1OG)            {if (ui->K1OD->isVisible())     ui->K1OD->setFocus();           return true;}
            if (obj == ui->K2OG)            {if (ui->K2OD->isVisible())     ui->K2OD->setFocus();           return true;}
            if (obj == ui->AxeKOG)          {if (ui->AxeKOD->isVisible())   ui->AxeKOD->setFocus();         return true;}
            if (obj == ui->OGPrescritCheckBox)              {ui->ODPrescritCheckBox->setFocus();            return true;}
            if (obj == ui->DeuxMonturesPrescritRadioButton) {ui->UneMonturePrescritRadioButton->setFocus(); return true;}
        }
        if(keyEvent->key()==Qt::Key_Down)
        {
            if (obj == ui->K1OD)            {if (ui->K1OG->isVisible())     ui->K1OG->setFocus();           return true;}
            if (obj == ui->K2OD)            {if (ui->K2OG->isVisible())     ui->K2OG->setFocus();           return true;}
            if (obj == ui->AxeKOD)          {if (ui->AxeKOG->isVisible())   ui->AxeKOG->setFocus();         return true;}
            if (obj == ui->ODPrescritCheckBox)                  {ui->OGPrescritCheckBox->setFocus();        return true;}
            if (obj == ui->UneMonturePrescritRadioButton)
            {
                if (ui->DeuxMonturesPrescritRadioButton->isVisible())
                    ui->DeuxMonturesPrescritRadioButton->setFocus();
                return true;
            }
            if (obj == ui->PressonODCheckBox)
            {
                if (ui->PressonOGCheckBox->isVisible())
                    ui->PressonOGCheckBox->setFocus();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

//-------------------------------------------------------------------------------------
// Gestion des touches fonctions
//-------------------------------------------------------------------------------------
void dlg_refraction::keyPressEvent ( QKeyEvent * event )
{
   // qDebug() << "keyPressEvent  : " + QString::number(event->key());

    switch (event->key()) {
    case Qt::Key_F12:
    {
        AnnulPushButton_Clicked();
        break;
    }
    case Qt::Key_Escape:
    {
        m_escapeflag = false;
        FermeFiche(Annul);
        break;
    }
    default:
        break;
    }
}

//----------------------------------------------------------------------------------------------------
// Initialisation des filtres sur les objets du formulaire.
//----------------------------------------------------------------------------------------------------
void dlg_refraction::InitEventFilters()
{
    QList<UpDoubleSpinBox *> listdspin = this->findChildren<UpDoubleSpinBox*>();
    for (int i=0; i<listdspin.size(); i++)
        listdspin.at(i)->installEventFilter(this);
    QList<UpSpinBox *> listspin = this->findChildren<UpSpinBox*>();
    for (int i=0; i<listspin.size(); i++)
        listspin.at(i)->installEventFilter(this);
    QList<UpGroupBox *> listbox = this->findChildren<UpGroupBox*>();
    for (int i=0; i<listbox.size(); i++)
        foreach (QWidget* widg, listbox.at(i)->findChildren<QWidget*>())
            widg->installEventFilter(this);

    ui->CommentairePrescriptionTextEdit->installEventFilter(this);
    ui->VerresTeintesCheckBox->installEventFilter(this);

    ui->OKPushButton->installEventFilter(this); // ????
    ui->AnnulPushButton->installEventFilter(this); // ????
    ui->AppelCommentPushButton->installEventFilter(this); // ????

    ui->DateDateEdit->installEventFilter(this);
}

//----------------------------------------------------------------------------------
// Initialisation des variables
//----------------------------------------------------------------------------------
void dlg_refraction::Init_variables()
{
    m_mode                   = Refraction::Fronto;
    m_affichedetail          = false;

    ui->DateDateEdit        ->setDate(QDate::currentDate());
    m_commentaireresume     = "";
    m_escapeflag              = true;
}

//--------------------------------------------------------------------------------------------
// Afficher ou Masquer les objets Detail (Prisme ...)
//--------------------------------------------------------------------------------------------
void dlg_refraction::AfficherDetail(bool iDetail)
{
    if (iDetail == true)
    {           // Afficher le Detail
        ui->DetailsPushButton->setText(tr("- de détails"));
        ui->frame_Detail->setVisible(true);
        if (!ui->RyserODCheckBox->isChecked() && !ui->RyserOGCheckBox->isChecked()) ui->RyserSpinBox->setVisible(false);
        if (ui->BasePrismeOD->value() == 0)   ui->BasePrismeTextODComboBox->setCurrentIndex(0);
        else
            if (ui->BasePrismeOD->value() == 90)   ui->BasePrismeTextODComboBox->setCurrentIndex(1);
            else
                if (ui->BasePrismeOD->value() == 180)   ui->BasePrismeTextODComboBox->setCurrentIndex(2);
                else
                    if (ui->BasePrismeOD->value() == 270)   ui->BasePrismeTextODComboBox->setCurrentIndex(3);
                    else
                        ui->BasePrismeTextODComboBox->clearEditText();
        if (ui->BasePrismeOG->value() == 180)   ui->BasePrismeTextOGComboBox->setCurrentIndex(0);
        else
            if (ui->BasePrismeOG->value() == 90)   ui->BasePrismeTextOGComboBox->setCurrentIndex(1);
            else
                if (ui->BasePrismeOG->value() == 0)   ui->BasePrismeTextOGComboBox->setCurrentIndex(2);
                else
                    if (ui->BasePrismeOG->value() == 270)   ui->BasePrismeTextOGComboBox->setCurrentIndex(3);
                    else
                        ui->BasePrismeTextOGComboBox->clearEditText();
    }
    else
    {           // Masquer le Detail
        if ((ui->PorteRadioButton->isChecked() || ui->PrescriptionRadioButton->isChecked()) &&
                (ui->PrismeOD->value() != 0.0          || ui->PrismeOG->value() != 0.0      ||
                 ui->RyserODCheckBox->isChecked()      || ui->RyserOGCheckBox->isChecked()   ||
                 ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()  ||
                 ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked()))
            return;
        ui->DetailsPushButton->setText(tr("+ de détails"));
        ui->frame_Detail->setVisible(false);
    }
}

void dlg_refraction::Afficher_AVL_AVP(bool TrueFalse)
{
    ui->AVLODupComboBox->setVisible(TrueFalse);
    ui->AVLOGupComboBox->setVisible(TrueFalse);
    ui->AVPODupComboBox ->setVisible(TrueFalse);
    ui->AVPOGupComboBox->setVisible(TrueFalse);
    ui->label_POD->setVisible(TrueFalse);
    ui->label_POG->setVisible(TrueFalse);
    ui->labelAVL->setVisible(TrueFalse);
    ui->labelAVP->setVisible(TrueFalse);
}

// ----------------------------------------------------------------------------
//  Afficher en fonction de la distance
// ----------------------------------------------------------------------------

void dlg_refraction::Afficher_AddVP(bool TrueFalse)
{
    ui->labelAddVP->setVisible(TrueFalse);
    ui->AddVPOD->setVisible(TrueFalse);
    ui->AddVPOG->setVisible(TrueFalse);
}
void dlg_refraction::Afficher_AVP(bool TrueFalse)
{
    ui->AVPODupComboBox->setVisible(TrueFalse);
    ui->AVPOGupComboBox->setVisible(TrueFalse);
    ui->label_POD->setVisible(TrueFalse);
    ui->label_POG->setVisible(TrueFalse);
    ui->labelAVP->setVisible(TrueFalse);
}

// ----------------------------------------------------------------------------
//  Afficher ou masquer tous les objets liés a un oeil
// ----------------------------------------------------------------------------
void dlg_refraction::Afficher_Oeil_Droit(bool TrueFalse)
{
    ui->SphereOD->setVisible(TrueFalse);
    ui->CylindreOD->setVisible(TrueFalse);
    ui->AxeCylindreOD->setVisible(TrueFalse);
    ui->AVLODupComboBox->setVisible(TrueFalse);
    ui->label_POD->setVisible(TrueFalse);
    ui->AVPODupComboBox->setVisible(TrueFalse);
    ui->AddVPOD->setVisible(TrueFalse);
    ui->ConvODPushButton->setVisible(TrueFalse);

    ui->PrismeOD->setVisible(TrueFalse);
    ui->BasePrismeTextODComboBox->setVisible(TrueFalse);
    ui->BasePrismeOD->setVisible(TrueFalse);
    ui->PressonODCheckBox->setVisible(TrueFalse);
    ui->K1OD->setVisible(TrueFalse);
    ui->K2OD->setVisible(TrueFalse);
    ui->AxeKOD->setVisible(TrueFalse);
}

// ----------------------------------------------------------------------------
//  Afficher ou masquer tous les objets liés a un oeil
// ----------------------------------------------------------------------------
void dlg_refraction::Afficher_Oeil_Gauche(bool TrueFalse)
{
    ui->SphereOG->setVisible(TrueFalse);
    ui->CylindreOG->setVisible(TrueFalse);
    ui->AxeCylindreOG->setVisible(TrueFalse);
    ui->AVLOGupComboBox->setVisible(TrueFalse);
    ui->label_POG->setVisible(TrueFalse);
    ui->AVPOGupComboBox->setVisible(TrueFalse);
    ui->AddVPOG->setVisible(TrueFalse);
    ui->ConvOGPushButton->setVisible(TrueFalse);

    ui->PrismeOG->setVisible(TrueFalse);
    ui->BasePrismeTextOGComboBox->setVisible(TrueFalse);
    ui->BasePrismeOG->setVisible(TrueFalse);
    ui->PressonOGCheckBox->setVisible(TrueFalse);
    ui->K1OG->setVisible(TrueFalse);
    ui->K2OG->setVisible(TrueFalse);
    ui->AxeKOG->setVisible(TrueFalse);
}

//---------------------------------------------------------------------------------
// Mise en forme du commentaire lunettes
//---------------------------------------------------------------------------------
QString dlg_refraction::CalculCommentaire()
{
    QString TousLesCom = "";
    if (ui->CommentairePrescriptionTextEdit->document()->toPlainText() > "")
        TousLesCom = TousLesCom + "\n" + ui->CommentairePrescriptionTextEdit->document()->toPlainText();

    return TousLesCom;
}

//---------------------------------------------------------------------------------
// Mise en forme de la Formule Oeil Gauche
//---------------------------------------------------------------------------------
QString dlg_refraction::CalculFormule_OG()
{
    ui->SphereOG->PrefixePlus();
    ui->CylindreOG->PrefixePlus();
    QString OGPrisme    = "";
    QString ResultatOGVL  = "";
    QString ResultatOGVP  = "";
    QString ResultatOG  = "";
    if(!ui->OGCheckBox->isChecked())
    {
        if (ui->DepoliOGCheckBox->isChecked())
            ResultatOG =  tr("dépoli");
        else
            ResultatOG = "" ;
        return ResultatOG;
    }
    if (m_mode == Refraction::Fronto || m_mode == Refraction::Prescription)
        if (ui->PrismeOG->value() != 0.00)
        {
            OGPrisme = ui->PrismeOG->text();
            if (ui->BasePrismeTextOGComboBox->currentText() != "")
                OGPrisme +=  tr(" dioptries base ") +  ui->BasePrismeTextOGComboBox->currentText();
            else
                OGPrisme +=  tr(" dioptries base ") + ui->BasePrismeOG->text() + "°";
            if (ui->PressonOGCheckBox->isChecked() && OGPrisme != "")
                OGPrisme = "Press-ON " + OGPrisme;
            else
                ResultatOG +=  tr("Prisme ") + OGPrisme;
        }
    if (m_mode == Refraction::Prescription && ui->VPPrescritRadioButton->isChecked())
    {
        if (ui->SphereOG->value() + ui->AddVPOG->value() != 0.00)
        {
            ResultatOG = QString::number(ui->SphereOG->value() + ui->AddVPOG->value(),'f',2);   // 26.06.2014
            ResultatOG = Valeur(ResultatOG) + " (" + ui->CylindreOG->text() + tr(" à ") + ui->AxeCylindreOG->text() + ")";  // 26.06.2014
        }
        else
            ResultatOG = ui->CylindreOG->text() + tr(" à ") + ui->AxeCylindreOG->text();
        return ResultatOG ;
    }
    else
    {
        if (ui->CylindreOG->value() != 0.00 && ui->SphereOG->value() != 0.00)
            ResultatOGVL = ui->SphereOG->text() + " (" + ui->CylindreOG->text() + tr(" à ") + ui->AxeCylindreOG->text() + ")" ;
        if (ui->CylindreOG->value() == 0.00 && ui->SphereOG->value() != 0.00)
            ResultatOGVL = ui->SphereOG->text() ;
        if (ui->CylindreOG->value() != 0.00 && ui->SphereOG->value() == 0.00)
            ResultatOGVL = ui->CylindreOG->text() + tr(" à ") + ui->AxeCylindreOG->text();
        if (ui->CylindreOG->value() == 0.00 && ui->SphereOG->value() == 0.00)
            ResultatOGVL =  tr("plan");
        if (ui->AddVPOG->isVisible() && ui->AddVPOG->value() > 0.00)
            ResultatOGVP = " add." + ui->AddVPOG->text() +  tr(" VP") ;
        ResultatOG = ResultatOGVL + " " + ResultatOGVP;
    }
    if (m_mode == Refraction::Acuite)
    {
        if (ui->VLRadioButton->isChecked())
            ResultatOG = ResultatOGVL + " " + wdg_AVLOG->text() + "" ;
        else
            ResultatOG = ResultatOGVL + " " + wdg_AVLOG->text() + " P" + wdg_AVPOG->text() + " " + ResultatOGVP ;
        return ResultatOG ;
    }
    if (m_mode == Refraction::Prescription)
    {
        if (ui->VLPrescritRadioButton->isChecked())
            return ResultatOGVL + " " + OGPrisme ;
        return ResultatOG + " " + OGPrisme ;
    }
    if (m_mode == Refraction::Autoref)
        return ResultatOGVL;
    if (m_mode == Refraction::Fronto)
    {
        if (ui->V2RadioButton->isChecked())
            return ResultatOGVL + " " + ResultatOGVP + " " + OGPrisme ;
        else
            return ResultatOGVL + " " + OGPrisme ;
    }
    return ResultatOG;
}

//---------------------------------------------------------------------------------
// Mise en forme de la Formule Oeil Droit
//---------------------------------------------------------------------------------
QString dlg_refraction::CalculFormule_OD()
{
    ui->SphereOD->PrefixePlus();
    ui->CylindreOD->PrefixePlus();
    QString ODPrisme    = "";
    QString ResultatODVL  = "";
    QString ResultatODVP  = "";
    QString ResultatOD  = "";
    if(!ui->ODCheckBox->isChecked())
    {
        if (ui->DepoliODCheckBox->isChecked())
            ResultatOD =  tr("dépoli");
        else
            ResultatOD = "" ;
        return ResultatOD;
    }
    if (m_mode == Refraction::Fronto || m_mode == Refraction::Prescription)
        if (ui->PrismeOD->value() != 0.00)
        {
            ODPrisme = ui->PrismeOD->text();
            if (ui->BasePrismeTextODComboBox->currentText() != "")
                ODPrisme +=  tr(" dioptries base ") +  ui->BasePrismeTextODComboBox->currentText();
            else
                ODPrisme +=  tr(" dioptries base ") + ui->BasePrismeOD->text() + "°";
            if (ui->PressonODCheckBox->isChecked() && ODPrisme != "")
                ODPrisme = "Press-ON " + ODPrisme;
            else
                ResultatOD +=  tr("Prisme ") + ODPrisme;
        }
    if (m_mode == Refraction::Prescription && ui->VPPrescritRadioButton->isChecked())
    {
        if (ui->SphereOD->value() + ui->AddVPOD->value() != 0.00)
        {
            ResultatOD = QString::number((ui->SphereOD->value() + ui->AddVPOD->value()),'f',2);
            ResultatOD = Valeur(ResultatOD) + " (" + ui->CylindreOD->text() + tr(" à ") + ui->AxeCylindreOD->text() + ")";
        }
        else
            ResultatOD = ui->CylindreOD->text() + tr(" à ") + ui->AxeCylindreOD->text();
        return ResultatOD ;
    }
    else
    {
        if (ui->CylindreOD->value() != 0.00 && ui->SphereOD->value() != 0.00)
            ResultatODVL = ui->SphereOD->text() + " (" + ui->CylindreOD->text() + tr(" à ") + ui->AxeCylindreOD->text() + ")" ;
        if (ui->CylindreOD->value() == 0.00 && ui->SphereOD->value() != 0.00)
            ResultatODVL = ui->SphereOD->text() ;
        if (ui->CylindreOD->value() != 0.00 && ui->SphereOD->value() == 0.00)
            ResultatODVL = ui->CylindreOD->text() + tr(" à ") + ui->AxeCylindreOD->text();
        if (ui->CylindreOD->value() == 0.00 && ui->SphereOD->value() == 0.00)
            ResultatODVL =  tr("plan");
        if (ui->AddVPOD->isVisible() && ui->AddVPOD->value() > 0.00)
            ResultatODVP = " add." + ui->AddVPOD->text() +  tr(" VP") ;
        ResultatOD = ResultatODVL + " " + ResultatODVP;
    }
    if (m_mode == Refraction::Acuite)
    {
        if (ui->VLRadioButton->isChecked())
            ResultatOD = ResultatODVL + " " + wdg_AVLOD->text() + "" ;
        else
            ResultatOD = ResultatODVL + " " + wdg_AVLOD->text() + " P" + wdg_AVPOD->text() + " " + ResultatODVP ;
        return ResultatOD ;
    }
    if (m_mode == Refraction::Prescription)
    {
        if (ui->VLPrescritRadioButton->isChecked())
            return ResultatODVL + " " + ODPrisme ;
        return ResultatOD + " " + ODPrisme ;
    }
    if (m_mode == Refraction::Autoref)
        return ResultatODVL;
    if (m_mode == Refraction::Fronto)
    {
        if (ui->V2RadioButton->isChecked())
            return ResultatODVL + " " + ResultatODVP + " " + ODPrisme ;
        else
            return ResultatODVL + " " + ODPrisme ;
    }
    return ResultatOD;
}

//---------------------------------------------------------------------------------
// Controle coherence - absence d'oubli
//---------------------------------------------------------------------------------
bool dlg_refraction::ControleCoherence()
{
    if (
           (ui->PorteRadioButton->isChecked() && ui->V2RadioButton->isChecked())
         &&
           ((ui->AddVPOD->value() == 0.00 && ui->AddVPOG->value() == 0.00)
           ||
           (ui->AddVPOD->isVisible() == false && ui->AddVPOG->value() == 0.00)
           ||
           (ui->AddVPOG->isVisible() == false && ui->AddVPOD->value() == 0.00))
       )
        {
        if (ui->AddVPOD->isVisible() == true && ui->AddVPOD->value() == 0.00)
            ui->AddVPOD->setFocus();
        else
            ui->AddVPOG->setFocus();
        UpMessageBox::Watch(this, tr("Contrôle de cohérence"), tr("Vous avez oublié de renseigner la correction de près!"));
        return false;
        }
    return true;
}

//---------------------------------------------------------------------------------
// Deplacement du curseur sur un des GroupBox
//---------------------------------------------------------------------------------
bool dlg_refraction::DeplaceVers(QWidget *widget, QString FinOuDebut)
{
    if (widget == ui->QuelleDistanceGroupBox)
    {
        if (ui->VLRadioButton->isChecked() == true)     {ui->VLRadioButton->setFocus();     return true;}
        if (ui->VPRadioButton->isChecked() == true)     {ui->VPRadioButton->setFocus();     return true;}
        ui->V2RadioButton->setFocus();
        ui->V2RadioButton->setChecked(true);
        RegleAffichageFiche();
        return true;
    }
    if (widget == ui->KeratometrieGroupBox)
    {
        if (FinOuDebut == "Debut")
        {
            if (ui->K1OD->isVisible())      {ui->K1OD->setFocus(); return true;}
            if (ui->K1OG->isVisible())      {ui->K1OG->setFocus(); return true;}
        }
        if (FinOuDebut == "Fin")
        {
            if (ui->AxeKOG->isVisible())    {ui->AxeKOG->setFocus(); return true;}
            if (ui->AxeKOD->isVisible())    {ui->AxeKOD->setFocus(); return true;}
        }
        return true;
    }
    if (widget == ui->PrismeGroupBox)
    {
        if (FinOuDebut == "Debut")
        {
            if (ui->PrismeOD->isVisible())
                ui->PrismeOD->setFocus();
            else
                ui->PrismeOG->setFocus();
        }
        else
        {
            if (ui->PressonOGCheckBox->isVisible())
                ui->PressonOGCheckBox->setFocus();
            else
                ui->PressonODCheckBox->setFocus();
        }
        return true;
    }
    if (widget == ui->MesureGroupBox)
    {
        if (FinOuDebut == "Debut")
        {
            if (ui->SphereOD->isVisible())
                ui->SphereOD->setFocus();
            else
                ui->SphereOG->setFocus();
            return true;
        }
        if (FinOuDebut == "Fin")
        {
            if (ui->SphereOG->isVisible())
            {
                if (ui->AddVPOG->isVisible())       ui->AddVPOG->setFocus();
                else if (wdg_AVPOG->isVisible())    wdg_AVPOG->setFocus();
                else if (wdg_AVLOG->isVisible())    wdg_AVLOG->setFocus();
                else                                ui->AxeCylindreOG->setFocus();
                return true;
            }
            else
            {
                if (ui->AddVPOD->isVisible())       ui->AddVPOD->setFocus();
                else if (wdg_AVPOD->isVisible())    wdg_AVPOD->setFocus();
                else if (wdg_AVLOD->isVisible())    wdg_AVLOD->setFocus();
                else                                ui->AxeCylindreOD->setFocus();
                return true;
            }
        }
        return true;
    }
    if (widget == ui->QuelleMesureGroupBox)
    {
        if (ui->PorteRadioButton->isChecked() == true)          {ui->PorteRadioButton->setFocus();          return true;}
        if (ui->AutorefRadioButton->isChecked() == true)        {ui->AutorefRadioButton->setFocus();        return true;}
        if (ui->RefractionRadioButton->isChecked() == true)     {ui->RefractionRadioButton->setFocus();     return true;}
        if (ui->PrescriptionRadioButton->isChecked() == true)   {ui->PrescriptionRadioButton->setFocus();   return true;}
        return true;
    }
    if (widget == ui->QuelleMontureGroupBox)
    {
        if (ui->UneMonturePrescritRadioButton->isChecked() == true)          {ui->UneMonturePrescritRadioButton->setFocus();          return true;}
        if (ui->DeuxMonturesPrescritRadioButton->isChecked() == true)        {ui->DeuxMonturesPrescritRadioButton->setFocus();          return true;}
        return true;
    }
    if (widget == ui->QuelOeilGroupBox)
    {
        if (ui->ODPrescritCheckBox->isEnabled())
            ui->ODPrescritCheckBox->setFocus();
        else
            ui->OGPrescritCheckBox->setFocus();
        return true;
    }
    if (widget == ui->QuelsVerresGroupBox)
    {
        if (ui->V2PrescritRadioButton->isChecked() == true)     {ui->V2PrescritRadioButton->setFocus();     return true;}
        if (ui->VLPrescritRadioButton->isChecked() == true)     {ui->VLPrescritRadioButton->setFocus();     return true;}
        if (ui->VPPrescritRadioButton->isChecked() == true)     {ui->VPPrescritRadioButton->setFocus();     return true;}
        return true;
    }
    return true;
}

//---------------------------------------------------------------------------------
// Suppression d'une mesure en base
//---------------------------------------------------------------------------------
void dlg_refraction::DetruireLaMesure(Refraction* ref)
{
    if (ref == Q_NULLPTR)
        return;
//1. On supprime la mesure de la table
    Datas::I()->refractions->SupprimeRefraction(ref);

//2. s'il n'y a plus de mesures pour le patient => on cache les boutons Oups, Resume et Reprendre
    int n = Datas::I()->refractions->refractions()->size();
    ui->OupsPushButton->setEnabled(n > 0);
    ui->ReprendrePushButton->setEnabled(n > 0);
    ui->ResumePushButton->setEnabled(n > 0);
}

//--------------------------------------------------------------------------------
// Click sur OK ou ANNULER >> Fermer la fiche refraction
//--------------------------------------------------------------------------------
void dlg_refraction::FermeFiche(dlg_refraction::ModeSortie mode)
{
    m_escapeflag = false;
    if (mode == Annul )
    {
        QList<UpDoubleSpinBox *> dblSpinList = findChildren<UpDoubleSpinBox *>();
        for (int i=1; i<dblSpinList.size(); i++)
            dblSpinList.at(i)->setAutorCorrigeDioptr(false);
    }
    else if (mode == OK)
        ResumeObservation();
    else if (mode == Imprime)
    {
        if    (
              (ui->V2PrescritRadioButton->isChecked())
           &&
              ((ui->AddVPOD->value() == 0.00 && ui->AddVPOG->value() == 0.00)
              ||
              (ui->AddVPOD->isVisible() == false && ui->AddVPOG->value() == 0.00)
              ||
              (ui->AddVPOG->isVisible() == false && ui->AddVPOD->value() == 0.00))
              )
        {
            if (ui->AddVPOD->isVisible() == true && ui->AddVPOD->value() == 0.00)
                ui->AddVPOD->setFocus();
            else
                ui->AddVPOG->setFocus();
        UpMessageBox::Watch(this, tr("Contrôle de cohérence"), tr("Vous avez oublié de renseigner la correction de près!"));
            return;
        }
        ResumePrescription();
        // on vérifie dans Refraction s'il existe un enregistrement identique pour ne pas surcharger la table avec
        disconnect (ui->OKPushButton,   &QPushButton::clicked,  this,     &dlg_refraction::OKPushButton_Clicked);
        if (LectureMesure(Aujourdhui, Refraction::Prescription, Refraction::NoDilatation, CalculFormule_OD(), CalculFormule_OG()) == Q_NULLPTR)
        {
            ResumeObservation();
            Refraction *ref = InsertRefraction();
            if (ref)
                if (!Imprimer_Ordonnance(ref))
                {
                    Datas::I()->refractions->SupprimeRefraction(ref);
                    m_resultPrescription = "";
                }

        }
        else
        {
            connect (ui->OKPushButton,  &QPushButton::clicked,  this,     &dlg_refraction::OKPushButton_Clicked);
            return;
        }
    }
    QList<QDialog *> ListDialog = this->findChildren<QDialog *>();
    for (int n = 0; n <  ListDialog.size(); n++)
        ListDialog.at(n)->close();
    m_fermecommentaire = (mode!=Annul);
    close();
}

int     dlg_refraction::idrefraction() const
{
    return m_idrefraction;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Imprimer une ordonnance ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool    dlg_refraction::Imprimer_Ordonnance(Refraction *ref)
{
    QString Corps, Entete, Pied;
    bool AvecDupli   = (proc->settings()->value(Imprimante_OrdoAvecDupli).toString() == "YES");
    bool AvecPrevisu = proc->ApercuAvantImpression();
    bool AvecNumPage = false;

    //création de l'entête
    User *userEntete = Q_NULLPTR;
    User *userconnected = Datas::I()->users->userconnected();
    if (userconnected)
    {
        int idsuperviseur = userconnected->idsuperviseur();
        userEntete = Datas::I()->users->getById(idsuperviseur);
    }
    if (userEntete == Q_NULLPTR)
        return false;
    Entete = proc->CalcEnteteImpression(ui->DateDateEdit->date(), userEntete).value("Norm");
    if (Entete == "") return false;
    Entete.replace("{{TITRE1}}"            , "");
    Entete.replace("{{TITRE}}"             , "");
    Entete.replace("{{PRENOM PATIENT}}"    , Datas::I()->patients->currentpatient()->prenom());
    Entete.replace("{{NOM PATIENT}}"       , Datas::I()->patients->currentpatient()->nom().toUpper());
    Entete.replace("{{DDN}}"               , "");

    // création du pied
    Pied = proc->CalcPiedImpression(userEntete, true);
    if (Pied == "") return false;

    // creation du corps de l'ordonnance
    Corps = proc->CalcCorpsImpression(ui->ResumePrescriptionTextEdit->toHtml());
    if (Corps == "") return false;

    QTextEdit *Etat_textEdit = new QTextEdit;
    Etat_textEdit->setHtml(Corps);
    bool a = proc->Imprime_Etat(Etat_textEdit, Entete, Pied,
                       proc->TaillePieddePageOrdoLunettes(), proc->TailleEnTete(), proc->TailleTopMarge(),
                       AvecDupli, AvecPrevisu, AvecNumPage);
    // stockage de l'ordonnance dans la base de donnees - table impressions
    if (a)
    {
        QHash<QString, QVariant> listbinds;
        listbinds[CP_IDUSER_DOCSEXTERNES] =           Datas::I()->users->userconnected()->id();
        listbinds[CP_IDPAT_DOCSEXTERNES] =            Datas::I()->patients->currentpatient()->id();
        listbinds[CP_TYPEDOC_DOCSEXTERNES] =          PRESCRIPTION;
        listbinds[CP_SOUSTYPEDOC_DOCSEXTERNES] =      CORRECTION;
        listbinds[CP_TITRE_DOCSEXTERNES] =            "Prescription correction";
        listbinds[CP_TEXTENTETE_DOCSEXTERNES] =       Entete;
        listbinds[CP_TEXTCORPS_DOCSEXTERNES] =        Corps;
        listbinds[CP_TEXTORIGINE_DOCSEXTERNES] =      ui->ResumePrescriptionTextEdit->toPlainText();
        listbinds[CP_TEXTPIED_DOCSEXTERNES] =         Pied;
        listbinds[CP_DATE_DOCSEXTERNES] =             ui->DateDateEdit->date().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds[CP_IDEMETTEUR_DOCSEXTERNES] =       Datas::I()->users->userconnected()->id();
        listbinds[CP_ALD_DOCSEXTERNES] =              QVariant(QVariant::String);
        listbinds[CP_EMISORRECU_DOCSEXTERNES] =       "0";
        listbinds[CP_FORMATDOC_DOCSEXTERNES] =        PRESCRIPTIONLUNETTES;
        listbinds[CP_IDLIEU_DOCSEXTERNES] =           Datas::I()->sites->idcurrentsite();
        listbinds[CP_IDREFRACTION_DOCSEXTERNES] =     ref->id();
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if (doc != Q_NULLPTR)
            delete doc;
    }
    delete Etat_textEdit;
    userEntete = Q_NULLPTR;
    return a;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Initialisation divers (Validator, geometry...etc...) -----------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void dlg_refraction::InitDivers()
{
    ui->OKPushButton->setShortcut(QKeySequence("Meta+Return"));

    QFont font = qApp->font();
    font.setItalic(true);
    font.setBold(true);
    font.setPointSize(font.pointSize()-1);
    ui->ODQLabel->setFont(font);
    ui->OGQLabel->setFont(font);
    ui->ODQLabel_2->setFont(font);
    ui->OGQLabel_2->setFont(font);
    ui->ODQLabel->setText("<font color='magenta'>" + tr("Œil droit") + "</font>");
    ui->ODQLabel_2->setText("<font color='magenta'>" + tr("Œil droit") + "</font>");
    ui->OGQLabel->setText("<font color='magenta'>" + tr("Œil gauche") + "</font>");
    ui->OGQLabel_2->setText("<font color='magenta'>" + tr("Œil gauche") + "</font>");

    // Initialisation des objets du formulaire
    wdg_AVPOD = new UpLineEdit(ui->AVPODupComboBox);
    wdg_AVPOG = new UpLineEdit(ui->AVPOGupComboBox);
    wdg_AVLOD = new UpLineEdit(ui->AVLODupComboBox);
    wdg_AVLOG = new UpLineEdit(ui->AVLOGupComboBox);
    QString styl =
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}";
    wdg_AVPOD->setStyleSheet(styl);
    wdg_AVPOG->setStyleSheet(styl);
    wdg_AVLOD->setStyleSheet(styl);
    wdg_AVLOG->setStyleSheet(styl);
    ui->AVPODupComboBox->setStyleSheet("border-left: 0px;");
    ui->AVPOGupComboBox->setStyleSheet("border-left: 0px;");
    ui->label_POD->setStyleSheet("border-left: 1px solid #adadad; border-top: 1px solid #adadad; border-bottom: 1px solid #adadad; border-right: 0px;  background-color: #fff;");
    ui->label_POG->setStyleSheet("border-left: 1px solid #adadad; border-top: 1px solid #adadad; border-bottom: 1px solid #adadad; border-right: 0px;  background-color: #fff;");

    QRegularExpression reg = QRegularExpression("(1\\.5|1,5|2|3|4|5|6|8|10|14|28|<28)");
    wdg_AVPOD->setValidator(new QRegularExpressionValidator(reg,this));
    wdg_AVPOG->setValidator(new QRegularExpressionValidator(reg,this));

    reg = QRegularExpression(tr("(zeroPL|PLnonO|PLO|VBLM|CLD 50cm|CLD 1m|0,5/10|0\\.5/10|1/10|1,6/10|1\\.6/10|2/10|2\\.5/10|2,5/10|3/10|4/10|5/10|6/10|6\\.3/10|6,3/10|7/10|8/10|9/10|10/10|12/10|16/10)"));
    wdg_AVLOD->setValidator(new QRegularExpressionValidator(reg,this));
    wdg_AVLOG->setValidator(new QRegularExpressionValidator(reg,this));

    ui->AVPODupComboBox->setLineEdit(wdg_AVPOD);
    ui->AVPOGupComboBox->setLineEdit(wdg_AVPOG);
    ui->AVLODupComboBox->setLineEdit(wdg_AVLOD);
    ui->AVLOGupComboBox->setLineEdit(wdg_AVLOG);

    QStringList listVP, listVL;
    listVP << "1.5" << "2" << "3" << "4" << "5" << "6" << "8" << "10" << "14" << "28" << "<28";
    listVL << "16/10" << "12/10" << "10/10" << "9/10" << "8/10" << "7/10" << "6.3/10"  << "6/10" << "5/10" << "4/10" << "3/10" << "2.5/10" << "2/10" << "1.6/10" << "1/10" << "0.5/10" << tr("CLD 1m") << tr("CLD 50cm") << tr("VBLM") << tr("PLO") << tr("PLnonO") << tr("zeroPL");
    ui->AVPODupComboBox->insertItems(0,listVP);
    ui->AVPOGupComboBox->insertItems(0,listVP);
    ui->AVLODupComboBox->insertItems(0,listVL);
    ui->AVLOGupComboBox->insertItems(0,listVL);

    ui->AVLODupComboBox->setIndexParDefaut(1);
    ui->AVLOGupComboBox->setIndexParDefaut(1);
    ui->AVPODupComboBox->setIndexParDefaut(1);
    ui->AVPOGupComboBox->setIndexParDefaut(1);

    ui->AVPODupComboBox->setCurrentIndex(ui->AVPODupComboBox->IndexParDefaut());
    ui->AVPOGupComboBox->setCurrentIndex(ui->AVPOGupComboBox->IndexParDefaut());
    ui->AVLODupComboBox->setCurrentIndex(ui->AVLODupComboBox->IndexParDefaut());
    ui->AVLOGupComboBox->setCurrentIndex(ui->AVLOGupComboBox->IndexParDefaut());

    ui->PorteRadioButton->setChecked(true);
    ui->DateDateEdit->setDate(QDate::currentDate());

    m_val = new upDoubleValidator(MinK, MaxK , 2, this);
    ui->K1OD->setValidator(m_val);
    ui->K1OG->setValidator(m_val);
    ui->K2OD->setValidator(m_val);
    ui->K2OG->setValidator(m_val);

    ui->AxeKOD->setValidator(new QIntValidator(0,180, this));
    ui->AxeKOG->setValidator(new QIntValidator(0,180, this));

    ui->K1OD->setAlignment(Qt::AlignRight);
    ui->K2OD->setAlignment(Qt::AlignRight);
    ui->K1OG->setAlignment(Qt::AlignRight);
    ui->K2OG->setAlignment(Qt::AlignRight);
    ui->AxeKOD->setAlignment(Qt::AlignRight);
    ui->AxeKOG->setAlignment(Qt::AlignRight);

    ui->AxeCylindreOD->setSuffix("°");
    ui->AxeCylindreOG->setSuffix("°");
    ui->frame_Detail->setVisible(false);
    ui->frame_Prescription->setVisible(false);

    ui->SphereOD->PrefixePlus();
    ui->SphereOG->PrefixePlus();
    ui->CylindreOD->PrefixePlus();
    ui->CylindreOG->PrefixePlus();
    ui->AddVPOD->PrefixePlus();
    ui->AddVPOG->PrefixePlus();
}

MesureRefraction* dlg_refraction::CalcMesureRefraction()
{
    MesureRefraction* ref = Q_NULLPTR;
    switch (m_mode) {
    case Refraction::Fronto:
        ref = Datas::I()->mesurefronto;
        break;
    case Refraction::Autoref:
        ref = Datas::I()->mesureautoref;
        break;
    case Refraction::Acuite:
        ref = Datas::I()->mesureacuite;
        break;
    case Refraction::Prescription:
        ref = Datas::I()->mesurefinal;
        break;
    default:
        return ref;
    }
    if (ref ==  Datas::I()->mesurefronto) {

        if (ui->OGCheckBox->isChecked())
        {
            ref->cleandatas(Utils::Gauche);
            ref->setsphereOG(ui->SphereOG->value());
            ref->setcylindreOG(ui->CylindreOG->value());
            ref->setaxecylindreOG(ui->AxeCylindreOG->value());
            if (!ui->V2RadioButton->isChecked())
                ref->setaddVPOD(ui->AddVPOD->value());
            if (m_affichedetail)
            {
                ref->setprismeOG(ui->PrismeOG->value());
                ref->setbaseprismeOG(ui->BasePrismeOG->value());
            }
        }
        if (ui->ODCheckBox->isChecked())
        {
            ref->cleandatas(Utils::Droit);
            ref->setsphereOD(ui->SphereOD->value());
            ref->setcylindreOD(ui->CylindreOD->value());
            ref->setaxecylindreOD(ui->AxeCylindreOD->value());
            if (!ui->V2RadioButton->isChecked())
                ref->setaddVPOD(ui->AddVPOD->value());
            if (m_affichedetail)
            {
                ref->setprismeOD(ui->PrismeOD->value());
                ref->setbaseprismeOD(ui->BasePrismeOD->value());
            }
        }
    }
    else if (ref == Datas::I()->mesureacuite)
    {
        if (ui->OGCheckBox->isChecked())
            ref->cleandatas(Utils::Gauche);
        if (ui->ODCheckBox->isChecked())
            ref->cleandatas(Utils::Droit);
    }
    else if (ref == Datas::I()->mesureacuite)
    {
        if (ui->OGCheckBox->isChecked())
            ref->cleandatas(Utils::Gauche);
        if (ui->ODCheckBox->isChecked())
            ref->cleandatas(Utils::Droit);
    }
    else if (ref == Datas::I()->mesureacuite)
    {
        if (ui->OGPrescritCheckBox->isChecked())
            ref->cleandatas(Utils::Gauche);
        if (ui->ODPrescritCheckBox->isChecked())
            ref->cleandatas(Utils::Droit);
    }
    return ref;
 }

QString dlg_refraction::CommentaireObligatoire()
{
    QString rep ("");
    for (auto itcom = Datas::I()->commentslunets->commentaires()->constBegin(); itcom != Datas::I()->commentslunets->commentaires()->constEnd(); itcom ++)
    {
        CommentLunet *com = itcom.value();
        if (com->isdefaut())
            if (com->iduser() == currentuser()->id())
            {
                rep += (rep != ""? "\n" + com->texte() : com->texte());
                m_listcommentaires << com;
            }
    }
    return rep;
}

//---------------------------------------------------------------------------------
// Creation d'un nouvel enregistrement dans DonneesOphtaPatient
//---------------------------------------------------------------------------------
void dlg_refraction::InsertDonneesOphtaPatient()
{
   QHash<QString, QVariant> listbinds;
   listbinds["idPat"]           = Datas::I()->patients->currentpatient()->id();
   listbinds["QuelleMesure"]    = Refraction::ConvertMesure(m_mode);
   listbinds["QuelleDistance"]  = QuelleDistance();
   if (m_mode == Refraction::Autoref && m_modeouverture == Manuel)
   {
       double K1OD = QLocale().toDouble(ui->K1OD->text());
       double K2OD = QLocale().toDouble(ui->K2OD->text());
       bool okOD = (K1OD >= m_val->bottom() && K1OD <= m_val->top() && K2OD >= m_val->bottom() && K2OD <= m_val->top());
       if (okOD && ui->ODCheckBox->isChecked()) // 16-07-2014
       {
           listbinds["K1OD"]        = K1OD;
           listbinds["K2OD"]        = K2OD;
           listbinds["AxeKOD"]      = ui->AxeKOD->text();
           Datas::I()->mesurekerato->setK1OD(K1OD);
           Datas::I()->mesurekerato->setK2OD(K2OD);
           Datas::I()->mesurekerato->setaxeKOD(ui->AxeKOD->text().toInt());
       }
       double K1OG = QLocale().toDouble(ui->K1OG->text());
       double K2OG = QLocale().toDouble(ui->K2OG->text());
       bool okOG = (K1OG >= m_val->bottom() && K1OG <= m_val->top() && K2OG >= m_val->bottom() && K2OG <= m_val->top());
       if (okOG && ui->OGCheckBox->isChecked())  // 16-07-2014
       {
           listbinds["K1OG"]        = K1OG;
           listbinds["K2OG"]        = K2OG;
           listbinds["AxeKOG"]      = ui->AxeKOG->text();
           Datas::I()->mesurekerato->setK1OG(K1OG);
           Datas::I()->mesurekerato->setK2OG(K2OG);
           Datas::I()->mesurekerato->setaxeKOG(ui->AxeKOG->text().toInt());
       }
       if (okOD || okOG)
       {
           listbinds["OrigineK"]     = Refraction::ConvertMesure(m_mode);
           listbinds["DateK"]        = ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
       }
   }
   if (ui->ODCheckBox->isChecked())
   {
       listbinds["SphereOD"]        = ui->SphereOD->value();
       listbinds["CylindreOD"]      = ui->CylindreOD->value();
       listbinds["AxeCylindreOD"]   = ui->AxeCylindreOD->value();
       if (m_mode == Refraction::Acuite)
       {
           listbinds["AVLOD"]       = wdg_AVLOD->text();
           if (!ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
           {
               listbinds["AddVPOD"] = ui->AddVPOD->value();
               listbinds["AVPOD"]   = wdg_AVPOD->text();
           }
       }
       listbinds["DateRefOD"]   = ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
   }
   if (ui->OGCheckBox->isChecked())
   {
       listbinds["SphereOG"]        = ui->SphereOG->value();
       listbinds["CylindreOG"]      = ui->CylindreOG->value();
       listbinds["AxeCylindreOG"]   = ui->AxeCylindreOG->value();
       if (m_mode == Refraction::Acuite)
       {
           listbinds["AVLOG"]       = wdg_AVLOG->text();
           if (!ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
           {
               listbinds["AddVPOG"] = ui->AddVPOG->value();
               listbinds["AVPOG"]   = wdg_AVPOG->text();
           }
       }
       listbinds["DateRefOG"]   = ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
   }
   if (ui->EIPLabel->text().toInt() > 0 && ui->ODCheckBox->isChecked() && ui->OGCheckBox->isChecked())
       listbinds[CP_PD_REFRACTIONS] = ui->EIPLabel->text();
   db->InsertSQLByBinds(TBL_DONNEES_OPHTA_PATIENTS, listbinds, tr("Erreur d'écriture dans ") + TBL_DONNEES_OPHTA_PATIENTS);
   Datas::I()->patients->actualiseDonneesOphtaCurrentPatient();
}

//---------------------------------------------------------------------------------
// Enregistre la mesure qui vient d'être validée dans la table Refractions
//---------------------------------------------------------------------------------
Refraction* dlg_refraction::InsertRefraction()
{
    QHash<QString, QVariant> listbinds;
    listbinds[CP_IDPAT_REFRACTIONS]                 = Datas::I()->patients->currentpatient()->id();
    listbinds[CP_IDACTE_REFRACTIONS]                = Datas::I()->actes->currentacte()->id();
    listbinds[CP_DATE_REFRACTIONS]                  = ui->DateDateEdit->date().toString("yyyy-MM-dd");
    listbinds[CP_TYPEMESURE_REFRACTIONS]            = Refraction::ConvertMesure(m_mode);
    if(m_mode != Refraction::Autoref)
        listbinds[CP_DISTANCEMESURE_REFRACTIONS]    = QuelleDistance();
    if(m_mode == Refraction::Autoref || m_mode == Refraction::Acuite)
        listbinds[CP_CYCLOPLEGIE_REFRACTIONS]       = ui->CycloplegieCheckBox->isChecked()? 1 : 0;

    listbinds[CP_ODMESURE_REFRACTIONS]              = ui->ODCheckBox->isChecked()? 1 : 0;
    if(ui->ODCheckBox->isChecked())
    {
        listbinds[CP_SPHEREOD_REFRACTIONS]          = ui->SphereOD->value();
        if (ui->CylindreOD->value() != 0.0)
        {
            listbinds[CP_CYLINDREOD_REFRACTIONS]    = ui->CylindreOD->value();
            listbinds[CP_AXECYLOD_REFRACTIONS]      = ui->AxeCylindreOD->value();
        }
    }
    if(wdg_AVLOD->isVisible())
        listbinds[CP_AVLOD_REFRACTIONS]             = wdg_AVLOD->text();
    if(ui->AddVPOD->isVisible())
        listbinds[CP_ADDVPOD_REFRACTIONS]           = ui->AddVPOD->value();
    if(wdg_AVPOD->isVisible())
        listbinds[CP_AVPOD_REFRACTIONS]             = wdg_AVPOD->text();
    if(ui->PrismeOD->isVisible() && ui->PrismeOD->text().toDouble() > 0)
    {
        listbinds[CP_PRISMEOD_REFRACTIONS]          = ui->PrismeOD->value();
        listbinds[CP_BASEPRISMEOD_REFRACTIONS]      = ui->BasePrismeOD->value();
        listbinds[CP_BASEPRISMETEXTOD_REFRACTIONS]  = ui->BasePrismeTextODComboBox->currentText();
        listbinds[CP_PRESSONOD_REFRACTIONS]         = ui->PressonODCheckBox->isChecked()? 1 : 0;
    }
    listbinds[CP_DEPOLIOD_REFRACTIONS]              = ui->DepoliODCheckBox->isChecked()? 1 : 0;
    listbinds[CP_PLANOD_REFRACTIONS]                = ui->PlanODCheckBox->isChecked()? 1 : 0;
    if(ui->RyserODCheckBox->isChecked())
        listbinds[CP_RYSEROD_REFRACTIONS]           = ui->RyserSpinBox->value();
    listbinds[CP_FORMULEOD_REFRACTIONS]             = CalculFormule_OD();

    listbinds[CP_OGMESURE_REFRACTIONS]              = ui->OGCheckBox->isChecked()? 1 : 0;
    if(ui->OGCheckBox->isChecked())
    {
        listbinds[CP_SPHEREOG_REFRACTIONS]          = ui->SphereOG->value();
        if (ui->CylindreOG->value() != 0.0)
        {
            listbinds[CP_CYLINDREOG_REFRACTIONS]    = ui->CylindreOG->value();
            listbinds[CP_AXECYLOG_REFRACTIONS]      = ui->AxeCylindreOG->value();
        }
    }
    if(wdg_AVLOG->isVisible())
        listbinds[CP_AVLOG_REFRACTIONS]             = wdg_AVLOG->text();
    if(ui->AddVPOG->isVisible())
        listbinds[CP_ADDVPOG_REFRACTIONS]           = ui->AddVPOG->value();
    if(wdg_AVPOG->isVisible())
        listbinds[CP_AVPOG_REFRACTIONS]             = wdg_AVPOG->text();
    if(ui->PrismeOG->isVisible() && ui->PrismeOG->text().toDouble() > 0)
    {
        listbinds[CP_PRISMEOG_REFRACTIONS]          = ui->PrismeOG->value();
        listbinds[CP_BASEPRISMEOG_REFRACTIONS]      = ui->BasePrismeOG->value();
        listbinds[CP_BASEPRISMETEXTOG_REFRACTIONS]  = ui->BasePrismeTextOGComboBox->currentText();
        listbinds[CP_PRESSONOG_REFRACTIONS]         = ui->PressonOGCheckBox->isChecked()? 1 : 0;
    }
    listbinds[CP_DEPOLIOG_REFRACTIONS]              = ui->DepoliOGCheckBox->isChecked()? 1 : 0;
    listbinds[CP_PLANOG_REFRACTIONS]                = ui->PlanOGCheckBox->isChecked()? 1 : 0;
    if(ui->RyserOGCheckBox->isChecked())
        listbinds[CP_RYSEROG_REFRACTIONS]           = ui->RyserSpinBox->value();
    listbinds[CP_FORMULEOG_REFRACTIONS]             = CalculFormule_OG();

    if(m_mode == Refraction::Prescription)
    {
        listbinds[CP_COMMENTAIREORDO_REFRACTIONS]   = CalculCommentaire();
        listbinds[CP_TYPEVERRES_REFRACTIONS]        = QuelsVerres();
        listbinds[CP_OEIL_REFRACTIONS]              = QuelsYeux();
        listbinds[CP_MONTURE_REFRACTIONS]           = QuelleMonture();
        listbinds[CP_VERRETEINTE_REFRACTIONS]       = ui->VerresTeintesCheckBox->isChecked()? 1 : 0;
    }
    return Datas::I()->refractions->CreationRefraction(listbinds);
}

/*! ---------------------------------------------------------------------------------
Recherche d'une mesure
\param DateMesure               -> mesure faite ce jour ou mesures antérieures
\param TypeMesureMesure         -> Fronto, Autoref, Refraction ou prescription
\param Cycloplegie dilatation   -> Dilatation = que les mesures avec dilatation  - NoDilatation = toutes les mesures, dilatées ou pas
\param FormuleOD                -> pour cette formule de l'OD
\param FormuleOG                -> pour cette formule de l'OG
---------------------------------------------------------------------------------*/
Refraction* dlg_refraction::LectureMesure(DateMesure Quand, Refraction::Mesure Mesure, Refraction::Cycloplegie dilatation, QString FormuleOD, QString FormuleOG)
{
    if (Datas::I()->refractions->refractions()->size() == 0)
        return Q_NULLPTR;
    bool cejour = (Quand == Aujourdhui);
    bool dilat  = (dilatation == Refraction::Dilatation);
    QMap<int, Refraction*> mapref;
    for (auto it = Datas::I()->refractions->refractions()->constBegin(); it != Datas::I()->refractions->refractions()->constEnd(); ++it)
    {
        Refraction* ref = const_cast<Refraction*>(it.value());
        if (((ref->daterefraction() == QDate::currentDate()) == cejour)
                && ref->typemesure() == Mesure
                && ref->isdilate() == dilat)
        {
            if (FormuleOD != "" && ref->formuleOD() != FormuleOD)
                continue;
            if (FormuleOG != "" && ref->formuleOG() != FormuleOG)
                continue;
            mapref.insert(ref->id(), ref);
        }
    }
    if (mapref.size() == 0)
        return Q_NULLPTR;
    return mapref.last();
}

void dlg_refraction::RemplitChamps(Refraction *ref)
{
    if (ref == Q_NULLPTR)
        return;
    ref->ConversionCylindreNeg();
    // Remplissage des champs Oeil Droit
    ui->ODCheckBox->setChecked(ref->isODmesure());
    if (ref->isODmesure())
    {
        ui->SphereOD->setValuewithPrefix(ref->sphereOD());
        ui->CylindreOD->setValuewithPrefix(ref->cylindreOD());
        ui->AddVPOD->setValuewithPrefix(ref->addVPOD());
        ui->AxeCylindreOD->setValue(            ref->axecylindreOD());
        if (ref->avlOD() != "")
            wdg_AVLOD->setText(                 ref->avlOD());
        if (ref->avpOD() != "")
            wdg_AVPOD->setText(                 ref->avpOD());
        ui->PrismeOD->setValue(                 ref->prismeOD());
        ui->BasePrismeOD->setValue(             ref->baseprismeOD());
        ui->PressonODCheckBox->setChecked(      ref->haspressonOD());
        ui->DepoliODCheckBox->setChecked(       ref->hasdepoliOD());
        ui->PlanODCheckBox->setChecked(         ref->hasplanOD());
        ui->RyserODCheckBox->setChecked(false);
        if (ref->ryserOD() > 0)
        {
            ui->RyserODCheckBox->setChecked(true);
            ui->RyserSpinBox->setValue(         ref->ryserOD());
        }
    } // fin Oeil droit coche

    // Remplissage des champs Oeil Gauche
    ui->OGCheckBox->setChecked(ref->isOGmesure());
    if (ref->isOGmesure())
    {
        ui->SphereOG->setValuewithPrefix(ref->sphereOG());
        ui->CylindreOG->setValuewithPrefix(ref->cylindreOG());
        ui->AddVPOG->setValuewithPrefix( ref->addVPOG());
        ui->AxeCylindreOG->setValue(            ref->axecylindreOG());
        if (ref->avlOG() != "")
            wdg_AVLOG->setText(                 ref->avlOG());
        if (ref->avpOG() != "")
            wdg_AVPOG->setText(                 ref->avpOG());
        ui->PrismeOG->setValue(                 ref->prismeOG());
        ui->BasePrismeOG->setValue(             ref->baseprismeOG());
        ui->PressonOGCheckBox->setChecked(      ref->haspressonOG());
        ui->DepoliOGCheckBox->setChecked(       ref->hasdepoliOG());
        ui->PlanOGCheckBox->setChecked(         ref->hasplanOG());
        ui->RyserOGCheckBox->setChecked(false);
        if (ref->ryserOG() > 0)
        {
            ui->RyserOGCheckBox->setChecked(true);
            ui->RyserSpinBox->setValue(         ref->ryserOG());
        }
    } // fin Oeil gauche coche
    ui->EIPLabel->setText(tr("Ecart interpupillaire") + " " + (ref->ecartIP() > 0? QString::number(ref->ecartIP()) +"mm" : tr("inconnu")));
}


//---------------------------------------------------------------------------------
// Mise a jour DonneesOphtaPatient
// Après l'enregistrement des mesures dans la table refraction, on met à jour la table donneesOphtaPatients qui fait un ra mise à jour de la table refraction
//---------------------------------------------------------------------------------
void dlg_refraction::MajDonneesOphtaPatient()
{
    // Recherche d'un enregistrement existant
    bool ok;
    QString MAJrequete = "SELECT   idPat FROM " TBL_DONNEES_OPHTA_PATIENTS
              " WHERE idPat = " + QString::number(Datas::I()->patients->currentpatient()->id()) +
              " AND QuelleMesure = '" + Refraction::ConvertMesure(m_mode) + "'";
    QList<QVariantList> MAJDonneesOphtalist = db->StandardSelectSQL(MAJrequete, ok, tr("Impossible de se connecter à la table des Donnees biométriques!"));
    if (!ok)
        return;
    else
    {
        if (MAJDonneesOphtalist.size() > 0)
            UpdateDonneesOphtaPatient();    // Un enregistrement existe : On le met a jour
        else
            InsertDonneesOphtaPatient();    // On cree un entregistement pour ce patient
    }
}

//---------------------------------------------------------------------------------
// Masquer les objets liés à un oeil qui est décoché.
//---------------------------------------------------------------------------------
void dlg_refraction::MasquerObjetsOeilDecoche()
{
    if (!ui->ODCheckBox->isChecked())
        Afficher_Oeil_Droit(false);
    if (!ui->OGCheckBox->isChecked())
        Afficher_Oeil_Gauche(false);
    ui->EIPLabel        ->setVisible(ui->OGCheckBox->isChecked() && ui->ODCheckBox->isChecked());
    if (ui->DepoliODCheckBox->isChecked())
        ui->RyserODCheckBox->setVisible(false);
    else
    {
        ui->RyserODCheckBox->setVisible(true);
        ui->PlanODCheckBox->setVisible(true);
    }
    if (ui->DepoliOGCheckBox->isChecked())
        ui->RyserOGCheckBox->setVisible(false);
    else
    {ui->RyserOGCheckBox->setVisible(true);
        ui->PlanOGCheckBox->setVisible(true);
    }
}

//---------------------------------------------------------------------------------
// Ouverture formulaire de la liste des mesures pour suppression ou Recuperation
//---------------------------------------------------------------------------------
void dlg_refraction::OuvrirListeMesures(dlg_refractionlistemesures::Mode mode)
{
    int RetourListe = 0;
    dlg_refractionlistemesures *Dlg_ListeMes    = new dlg_refractionlistemesures(mode, this);
    Dlg_ListeMes->setWindowTitle(tr("Liste des mesures : ") + Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom() );

    RetourListe = Dlg_ListeMes->exec();

    // relecture et affichage de la mesure selectionnee
    if (RetourListe > 0 && mode == dlg_refractionlistemesures::Recuperer)
    {
        Refraction *ref = Dlg_ListeMes->RefractionAOuvrir();
        if (ref != Q_NULLPTR)
        {
            RemplitChamps(ref);
            m_mode = ref->typemesure();
            if (m_mode == Refraction::Prescription)
                RadioButtonPrescription_clicked();
            else
                RegleAffichageFiche();
        }
    }
    if (RetourListe > 0 && mode == dlg_refractionlistemesures::Supprimer)
        RechercheMesureEnCours();
    Dlg_ListeMes->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_ListeMes;
    RegleAffichageFiche();
}

// -------------------------------------------------------------------------------------
// Retourne les valeurs des CheckBox
//--------------------------------------------------------------------------------------
QString dlg_refraction::QuelleDistance()
{
    if (ui->VLRadioButton->isChecked()) return "L";
    if (ui->VPRadioButton->isChecked()) return "P";
    if (ui->V2RadioButton->isChecked()) return "2";
    return "";
}
QString dlg_refraction::QuelsVerres()
{
    if (ui->VLPrescritRadioButton->isChecked()) return "L";
    if (ui->VPPrescritRadioButton->isChecked()) return "P";
    if (ui->V2PrescritRadioButton->isChecked()) return "2";
    return "";
}
QString dlg_refraction::QuelsYeux()
{
    if (m_mode != Refraction::Prescription)
        {
        if (ui->ODCheckBox->isChecked() && ui->OGCheckBox->isChecked())     return "2";
        if (ui->OGCheckBox->isChecked())                                    return "G";
        if (ui->ODCheckBox->isChecked())                                    return "D";
        }
        else
        {
        if (ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked())     return "2";
        if (ui->OGPrescritCheckBox->isChecked())     return "G";
        if (ui->ODPrescritCheckBox->isChecked())    return "D";
        }
    return "";
}
QString dlg_refraction::QuelleMonture()
{
    if (ui->DeuxMonturesPrescritRadioButton->isChecked())   return "2";
    if (ui->UneMonturePrescritRadioButton->isChecked())     return "1";
    return "";
}

/*-----------------------------------------------------------------------------------------------------------------
-- Modifie AddVPOD en fonction d'AddVPOG et vice-versa ------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    dlg_refraction::QuitteAddVP(UpDoubleSpinBox *obj)
{
    if (obj == ui->AddVPOG)
    {
        ui->AddVPOG->setValue(ui->AddVPOG->valueFromText(ui->AddVPOG->text()));
        if (ui->AddVPOG->value() != ui->AddVPOG->valeuravant())
            if (ui->AddVPOD->value() == 0.0 && ui->AddVPOD->isVisible())
            {
                ui->AddVPOD->setValue(ui->AddVPOG->value());
                ui->AddVPOD->CorrigeDioptrie(UpDoubleSpinBox::Near);
                ui->AddVPOD->PrefixePlus();
            }
    }
    if (obj == ui->AddVPOD)
    {
        ui->AddVPOD->setValue(ui->AddVPOD->valueFromText(ui->AddVPOD->text()));
        if (ui->AddVPOD->value() != ui->AddVPOD->valeuravant())
            if (ui->AddVPOG->value() == 0.0 && ui->AddVPOG->isVisible())
            {
                ui->AddVPOG->setValue(ui->AddVPOD->value());
                ui->AddVPOG->CorrigeDioptrie(UpDoubleSpinBox::Near);
                ui->AddVPOG->PrefixePlus();
            }
    }
}

//---------------------------------------------------------------------------------
// Recherche d'une ou plusieurs mesure en cours pour ce patient et affichage.
//---------------------------------------------------------------------------------
void dlg_refraction::RechercheMesureEnCours()
{
    Refraction::Mesure Reponse = Refraction::NoMesure;
    if (Datas::I()->refractions->refractions()->isEmpty())
    {
        m_mode = Refraction::Fronto;
        ui->ReprendrePushButton->setEnabled(false);
        ui->OupsPushButton->setEnabled(false);
        ui->ResumePushButton->setEnabled(false);
        QMap<QString,QVariant>  Age = Utils::CalculAge(Datas::I()->patients->currentpatient()->datedenaissance());
        if (Age["annee"].toInt() < 45)
            ui->VLRadioButton->setChecked(true);
        else
            ui->V2RadioButton->setChecked(true);
        RegleAffichageFiche();
        return;
    }

    // recherche d'une mesure du jour
    foreach (Refraction *ref, *Datas::I()->refractions->refractions())
        if (ref->daterefraction() == QDate::currentDate() && ref->typemesure() == Refraction::Prescription)
        {
            Reponse = ref->typemesure();
            break;
        }
    if (Reponse == Refraction::NoMesure)
        foreach (Refraction *ref, *Datas::I()->refractions->refractions())
            if (ref->daterefraction() == QDate::currentDate() && ref->typemesure() == Refraction::Acuite)
            {
                Reponse = ref->typemesure();
                break;
            }
    if (Reponse == Refraction::NoMesure)
        foreach (Refraction *ref, *Datas::I()->refractions->refractions())
            if (ref->daterefraction() == QDate::currentDate() && ref->typemesure() == Refraction::Autoref)
            {
                Reponse = ref->typemesure();
                break;
            }
    if (Reponse == Refraction::NoMesure)
        foreach (Refraction *ref, *Datas::I()->refractions->refractions())
        {
            if (ref->daterefraction() == QDate::currentDate() && ref->typemesure() == Refraction::Fronto)
            {
                Reponse = ref->typemesure();
                break;
            }
        }

    if (Reponse != Refraction::NoMesure)
    {
        Refraction *ref = LectureMesure(Aujourdhui, Reponse, Refraction::NoDilatation);            // on affiche la mesure du jour trouvée
        if (ref == Q_NULLPTR)
            ref = LectureMesure(Aujourdhui, Reponse, Refraction::Dilatation);
        if (ref != Q_NULLPTR)            // on affiche la mesure du jour trouvée
        {
            RemplitChamps(ref);
            AfficheKerato();
            // on passe au mode de mesure suivant
            if (Reponse == Refraction::Acuite || Reponse == Refraction::Prescription)
                RadioButtonPrescription_clicked();
            else if (Reponse == Refraction::Fronto)
            {
                m_mode = Refraction::Autoref;
                RegleAffichageFiche();
            }
            else if (Reponse == Refraction::Autoref)
            {
                m_mode = Refraction::Acuite;
                RegleAffichageFiche();
            }
            return ;
        }
    }

    // On n'a rien trouvé pour le jour >> on cherche la dernière mesure de réfraction
    Refraction* ref = LectureMesure(Avant, Refraction::Acuite, Refraction::NoDilatation);
    if (ref == Q_NULLPTR)
        ref = LectureMesure(Avant, Refraction::Prescription, Refraction::NoDilatation); // on n'a rien trouvé en réfraction - on cherche la dernière prescription
    if (ref == Q_NULLPTR)
        ref = LectureMesure(Avant, Refraction::Autoref, Refraction::NoDilatation);      // on n'a rien trouvé en prescription - on cherche la dernière mesure Autoref
    if (ref == Q_NULLPTR)
        ref = LectureMesure(Avant, Refraction::Fronto, Refraction::NoDilatation);       // on n'a rien trouvé en autoref - on cherche la dernière mesure de fronto
    if (ref != Q_NULLPTR)
    {
        RemplitChamps(ref);
        AfficheKerato();
        m_mode = ref->typemesure();
        RegleAffichageFiche();
    }

}

//------------------------------------------------------------------------------------------------------
//  Calcul de la variable Resultat pour resume refraction.
//------------------------------------------------------------------------------------------------------
QString dlg_refraction::RechercheResultat(Refraction::Mesure mesure, Refraction::Cycloplegie dilatation, Refraction::Distance typlun)
 {
    QString     Resultat    = "";
    QString     ResultatOD  = "";
    QString     ResultatOG  = "";
    QString     Separateur  = "";
    QString     DateOD      = "";
    QString     DateOG      = "";
    QString     Dilate      = "";

    if (mesure == Refraction::Autoref)  Separateur = " / ";
    if (mesure == Refraction::Acuite)  Separateur = "\n\t\t";
    QString zdate;
    QMap<int, Refraction*> mapref;
    bool dilat = (dilatation == Refraction::Dilatation);
    for (auto it = Datas::I()->refractions->refractions()->constBegin(); it != Datas::I()->refractions->refractions()->constEnd(); ++it)
    {
        Refraction* ref = const_cast<Refraction*>(it.value());
        if (ref->typemesure() == mesure
                && ref->isdilate() == dilat)
        {
            if (typlun != Refraction::Inconnu)
                mapref.insert(ref->id(), ref);
            else
                mapref.insert(ref->id(), ref);
        }
    }
    if (mapref.size() == 0)
        return "";
    Refraction *ref = mapref.last();
    zdate = ref->daterefraction().toString(tr("dd-MM-yyyy"));                      // date YYYY-MM-DD
    if (ref->isODmesure() && ref->isOGmesure())      // OD coche et OG coche
        Resultat = tr("dilaté") + ")" "\n\t" + zdate + "\t" + ref->formuleOD() + Separateur + ref->formuleOG();
    else
    {
        if (ref->isODmesure())
        {
            DateOD      = zdate;
            ResultatOD  =  ref->formuleOD();
            QMap<int, Refraction*> maprefOG;
            for (auto it = Datas::I()->refractions->refractions()->constBegin(); it != Datas::I()->refractions->refractions()->constEnd(); ++it)
            {
                Refraction* ref = const_cast<Refraction*>(it.value());
                if (ref->typemesure() == mesure
                        && ref->isdilate() == dilat
                        && ref->isOGmesure())
                         maprefOG.insert(ref->id(), ref);
            }
            if (maprefOG.size() > 0)
            {
                zdate  = maprefOG.last()->daterefraction().toString(tr("dd-MM-yyyy"));
                ResultatOD  =  maprefOG.last()->formuleOD();
            }
        }
        else
        {
            if (ref->isOGmesure())
            {
                DateOG      = zdate;
                ResultatOG  =  ref->formuleOG();
                QMap<int, Refraction*> maprefOD;
                for (auto it = Datas::I()->refractions->refractions()->constBegin(); it != Datas::I()->refractions->refractions()->constEnd(); ++it)
                {
                    Refraction* ref = const_cast<Refraction*>(it.value());
                    if (ref->typemesure() == mesure
                            && ref->isdilate() == dilat
                            && ref->isODmesure())
                             maprefOD.insert(ref->id(), ref);
                }
                if (maprefOD.size() > 0)
                {
                    zdate  = maprefOD.last()->daterefraction().toString(tr("dd-MM-yyyy"));
                    ResultatOG  =  maprefOD.last()->formuleOG();
                }
            }
        }
    }
    if (Resultat == "")
    {
        if (ResultatOD != "" && ResultatOG == "")
            Resultat = tr("dilaté") + ")"  "\n\t" + DateOD + "\tOD: " + ResultatOD;
        if (ResultatOD == "" && ResultatOG != "")
            Resultat = tr("dilaté") + ")"  "\n\t" + DateOG + "\tOG: " + ResultatOG;
        if (ResultatOD != "" && ResultatOG != "")
            Resultat = tr("dilaté") + ")"   "\n\t" + DateOD + "\t: " + ResultatOD +
                                          "\n\t" + DateOG + "\t: " + ResultatOG;
    }
    if (dilatation == Refraction::Dilatation)
        Dilate = "        (";
    else
        Dilate = "        (" + tr("non") + " ";
    return  "\n" + Dilate + Resultat;
}

//------------------------------------------------------------------------------------------------------
//  Calcul de la variable ResultatVerres
//------------------------------------------------------------------------------------------------------
QString dlg_refraction::RechercheVerres()
{
    QString     ResultatVerres = "";
    QString     zdate, Formule, TypeMesure;
    QMap<int, Refraction*> mapref;
    for (auto it = Datas::I()->refractions->refractions()->constBegin(); it != Datas::I()->refractions->refractions()->constEnd(); ++it)
    {
        Refraction* ref = const_cast<Refraction*>(it.value());
        if (ref->typemesure() == Refraction::Autoref
                || ref->typemesure() == Refraction::Prescription)
                mapref.insert(ref->id(), ref);
    }
    if (mapref.size() == 0)
        return "";
    for (auto it = mapref.constBegin(); it != mapref.constEnd(); ++it)
    {
        Refraction* ref = const_cast<Refraction*>(it.value());
        if (ref->typemesure() == Refraction::Prescription)
            TypeMesure =  tr("Prescription");
        else
            TypeMesure =  tr("Verres portés");
        zdate = ref->daterefraction().toString("dd-MM-yyyy");
        // calcul Formule
        Formule = ref->formuleOD();
        if (Formule.length() > 0)
            Formule += " / ";
        Formule += ref->formuleOG();
        if (ResultatVerres != "")
            ResultatVerres += "\n\n";
        ResultatVerres += "        " + zdate + " - " + TypeMesure + "\n\t" + Formule;
    }
    return ResultatVerres;
}

// -------------------------------------------------------------------------------------
// Generation du resumé de l'observation pour inscription dans le champ correspondant de la fiche principale au moment de la fermeture de la fiche
//--------------------------------------------------------------------------------------
void dlg_refraction::ResumeObservation()
{
    QString VerreSpecialOD, VerreSpecialOG;
    QString ResultatOD, ResultatVLOD, ResultatVPOD, ResultatPrismeOD;
    QString ResultatOG, ResultatVLOG, ResultatVPOG, ResultatPrismeOG;
    QString ResultatPrisme, ResultatRyser;
    double  ResultatGlobalSphereOD,  ResultatGlobalSphereOG;
    QString DelimiterDebut  = "<a name=\"debut" + QString::number(m_idrefraction) + "\"></a>";
    QString DelimiterFin    = "<a name=\"fin" + QString::number(m_idrefraction) + "\"></a>";
    // QString TagAncre, numIDref; // 07-07-2014 08-08-2014

    Refraction::Distance Distance = Refraction::AllDistance;

    // Génération du code html pour TAG Ancre avec ID refraction            // 07-07-2014 // 08-08-2014
    //numIDref = QString::number(gListeRefractionID.at(gListeRefractionID.size()-1));
    //TagAncre = "<a name=\"" + numIDref + "\"></a>" "<span ><a href=\""+ numIDref + "\" style=\"text-decoration:none\" style=\"color:#000000\"> ";
    if (ui->SphereOD->hasFocus())       ui->SphereOD->PrefixePlus();
    if (ui->SphereOG->hasFocus())       ui->SphereOG->PrefixePlus();
    if (ui->CylindreOD->hasFocus())     ui->CylindreOD->PrefixePlus();
    if (ui->CylindreOG->hasFocus())     ui->CylindreOG->PrefixePlus();
    if (ui->AddVPOD->hasFocus())        ui->AddVPOD->PrefixePlus();
    if (ui->AddVPOG->hasFocus())        ui->AddVPOG->PrefixePlus();

    if (m_mode == Refraction::Prescription)
    {
        if (ui->V2PrescritRadioButton->isChecked()) Distance = Refraction::AllDistance;
        if (ui->VLPrescritRadioButton->isChecked()) Distance = Refraction::Loin;
        if (ui->VPPrescritRadioButton->isChecked()) Distance = Refraction::Pres;
    }
    else
    {
        if (ui->V2RadioButton->isChecked()) Distance = Refraction::AllDistance;
        if (ui->VLRadioButton->isChecked()) Distance = Refraction::Loin;
        if (ui->VPRadioButton->isChecked()) Distance = Refraction::Pres;
        if (m_mode == Refraction::Acuite && ui->CycloplegieCheckBox->isChecked())        Distance = Refraction::Loin;
    }

    if (m_mode == Refraction::Fronto || m_mode == Refraction::Prescription)
        //EN MODE PORTE  ou Prescription --  détermination de gResultatPO ou gResultatPR  ---------------------------------------------------------------------------
    {
        // - 1 - détermination des verres
        // On se débarasse des dépoli et des plan----------------------------------------------------
        VerreSpecialOD = "non";
        VerreSpecialOG = "non";
        if (m_affichedetail)
        {
            if (ui->PlanODCheckBox->isChecked())
                VerreSpecialOD =  tr("plan");
            if (ui->PlanOGCheckBox->isChecked())
                VerreSpecialOG =  tr("plan");
            if (ui->DepoliODCheckBox->isChecked())
                VerreSpecialOD = tr("dépoli");
            if (ui->DepoliOGCheckBox->isChecked())
                VerreSpecialOG = tr("dépoli");
        }

        // détermination OD
        if (VerreSpecialOD !="non")
            ResultatOD = VerreSpecialOD;
        else
        {
            if (ui->SphereOD->value() != 0.0)
                ResultatVLOD = Valeur(ui->SphereOD->text())
                             + (ui->CylindreOD->value() != 0.0? " (" + Valeur(ui->CylindreOD->text()) + " à " + ui->AxeCylindreOD->text() + ")" : "");
            else
                ResultatVLOD = (ui->CylindreOD->value() != 0.0? Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() : tr("plan"));
            ResultatOD = ResultatVLOD;
            if (ui->AddVPOD->value() > 0)
            {
                ResultatVPOD = Valeur(ui->AddVPOD->text());
                ResultatOD += " add." + ResultatVPOD + tr(" VP");
            }
            else
            {
                ResultatVPOD = tr("plan");
                if (ResultatVLOD == tr("plan"))
                {
                    VerreSpecialOD = tr("plan");
                    ResultatOD = tr("plan");
                }
            }
            if (((!ui->ODCheckBox->isChecked() && (m_mode != Refraction::Prescription)) && VerreSpecialOD == "non") || (!ui->ODPrescritCheckBox->isChecked() && (m_mode == Refraction::Prescription)))
                ResultatOD = "Rien";
        }

        // détermination OG
        if (VerreSpecialOG !="non")
            ResultatOG = VerreSpecialOG;
        else
        {
            if (ui->SphereOG->value() != 0.0)
                ResultatVLOG = Valeur(ui->SphereOG->text())
                             + (ui->CylindreOG->value() != 0.0? " (" + Valeur(ui->CylindreOG->text()) + " à " + ui->AxeCylindreOG->text() + ")" : "");
            else
                ResultatVLOG = (ui->CylindreOG->value() != 0.0? Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() : tr("plan"));
            ResultatOG = ResultatVLOG;
            if (ui->AddVPOG->value() > 0)
            {
                ResultatVPOG = Valeur(ui->AddVPOG->text());
                ResultatOG += " add." + ResultatVPOG + tr(" VP");
            }
            else
            {
                ResultatVPOG = tr("plan");
                if (ResultatVLOG == tr("plan"))
                {
                    VerreSpecialOG = tr("plan");
                    ResultatOG = tr("plan");
                }
            }
            if (((!ui->OGCheckBox->isChecked() && (m_mode != Refraction::Prescription)) && VerreSpecialOG == "non") || (!ui->OGPrescritCheckBox->isChecked() && (m_mode == Refraction::Prescription)))
                ResultatOG = "Rien";
        }

        m_resultFronto = "";

        // Détermination de gResultatP
        switch (Distance)
        {
        case Refraction::AllDistance: // Mesure ou prescription de verres multifocaux
            if (ResultatOD == ResultatOG)
            {
                if (ResultatVPOD != tr("plan") || VerreSpecialOD != "non")
                    m_resultFronto = ResultatOD + tr(" ODG");
                else
                    m_resultFronto = ResultatVLOD + tr(" VL ODG");
            }
            else
            {
                if (VerreSpecialOD != "non")
                {
                    if (VerreSpecialOG != "non")
                        m_resultFronto = VerreSpecialOD + " / " + VerreSpecialOG;
                    else
                    {
                        if (ResultatOG == "Rien")
                            m_resultFronto = VerreSpecialOD + tr(" OD");
                        else
                            m_resultFronto = VerreSpecialOD + " / " + ResultatOG + (ui->AddVPOG->value() == 0.0? tr(" VL") : "");
                    }
                }
                else if (ResultatOD != "Rien")
                {
                    if (ui->AddVPOD->value() == 0.0)
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = ResultatVLOD + " / " + VerreSpecialOG + tr(" VL");
                        else if (ResultatOG == "Rien")
                            m_resultFronto = ResultatVLOD + tr(" OD VL");
                        else if (ui->AddVPOG->value() == 0.0)
                            m_resultFronto = ResultatVLOD + " / " + ResultatVLOG + tr(" VL");
                        else
                            m_resultFronto = ResultatVLOD + " / " + ResultatOG;
                    }
                    else
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = ResultatOD + " / " + VerreSpecialOG;
                        else if (ResultatOG == "Rien")
                            m_resultFronto = ResultatOD + tr(" OD");
                        else
                        {
                            if (ui->AddVPOG->value() == 0.0)
                                m_resultFronto = ResultatOD + " / " + ResultatVLOG;
                            else
                            {
                                if (ResultatVPOD == ResultatVPOG)
                                    m_resultFronto = ResultatVLOD + " / " + ResultatOG + tr(" ODG");
                                else
                                    m_resultFronto = ResultatOD + " / " + ResultatOG;
                            }
                        }
                    }
                }
                else if (ResultatOD == "Rien")
                {
                    if (VerreSpecialOG != "non")
                         m_resultFronto = VerreSpecialOG + tr(" OG");
                    else if (ResultatOG != "Rien")
                    {
                     if (ui->AddVPOG->value() == 0.0)
                         m_resultFronto = ResultatVLOG + tr(" OG VL");
                     else
                         m_resultFronto = ResultatOG + tr(" OG");
                    }
                }

            }
            break;
        case Refraction::Loin: // Mesure ou prescription de verres de loin
            if (ResultatOD == ResultatOG)
            {
                if (VerreSpecialOD != "non")
                    m_resultFronto = VerreSpecialOD + tr(" ODG");
                else
                    m_resultFronto = ResultatVLOD + tr(" VL ODG");
            }
            else
            {
                if (VerreSpecialOD != "non")
                {
                    if (VerreSpecialOG != "non")
                        m_resultFronto = VerreSpecialOD + " / " + VerreSpecialOG;
                    else if (ResultatOG == "Rien")
                        m_resultFronto = VerreSpecialOD + tr(" OD");
                    else
                        m_resultFronto = VerreSpecialOD + " / " + ResultatVLOG + tr(" VL");
                }
                else if (ResultatOD != "Rien")
                {
                    if (VerreSpecialOG != "non")
                        m_resultFronto = ResultatVLOD + " / " + VerreSpecialOG + tr(" VL");
                    else if (ResultatOG == "Rien")
                        m_resultFronto = ResultatVLOD + tr(" OD VL");
                    else
                        m_resultFronto = ResultatVLOD + " / " + ResultatVLOG + tr(" VL");
                }
                else
                {
                    if (VerreSpecialOG != "non")
                        m_resultFronto = VerreSpecialOG + tr(" VL");
                    else if (ResultatOG != "Rien")
                        m_resultFronto = ResultatVLOG + tr(" OG VL");
                }
            }
            break;
        case Refraction::Pres: // Mesure ou prescription de verres de près
            if (m_mode == Refraction::Fronto)                                     // Calcul des verres de près en mode porte
            {
                if (ResultatOD == ResultatOG)
                {
                    if (VerreSpecialOD != "non")
                        m_resultFronto = VerreSpecialOD + tr(" ODG");
                    else
                        m_resultFronto = ResultatVLOD +  tr(" VP ODG");
                }
                else
                {
                    if (VerreSpecialOD != "non")
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = VerreSpecialOD + " / " + VerreSpecialOG;
                        else if (ResultatOG == "Rien")
                            m_resultFronto = VerreSpecialOD +  tr(" OD");
                        else
                            m_resultFronto = VerreSpecialOD + " / " + ResultatVLOG +  tr(" VP");
                    }
                    else  if (ResultatOD != "Rien")
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = ResultatVLOD + " / " + VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            m_resultFronto = ResultatVLOD +  tr(" OD VP");
                        else
                            m_resultFronto = ResultatVLOD + " / " + ResultatVLOG +  tr(" VP");
                    }
                    else
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            m_resultFronto = ResultatVLOG +  tr(" OG VP");
                        else
                            m_resultFronto = ResultatVLOG +  tr(" OG VP");
                    }
                }
            }
            else if (m_mode == Refraction::Prescription) // Calcul des verres de près en mode impression
            {
                ResultatGlobalSphereOD = QString::number((ui->SphereOD->value() + ui->AddVPOD->value()) ,'f',2).toDouble();
                ResultatGlobalSphereOG = QString::number((ui->SphereOG->value() + ui->AddVPOG->value()) ,'f',2).toDouble();
                ResultatVLOD = Valeur(QString::number(ResultatGlobalSphereOD,'f',2));
                ResultatVLOG = Valeur(QString::number(ResultatGlobalSphereOG,'f',2));

                if (ui->CylindreOD->value() != 0.0 && ResultatGlobalSphereOD != 0.0)
                    ResultatVLOD = ResultatVLOD + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
                if (ui->CylindreOD->value() != 0.0 && ResultatGlobalSphereOD == 0.0)
                    ResultatVLOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
                if (ui->CylindreOD->value() == 0.0 && ResultatGlobalSphereOD == 0.0)
                    ResultatVLOD =  tr("plan");
                ResultatOD = ResultatVLOD;
                if (!ui->ODPrescritCheckBox->isChecked())
                    ResultatOD = "Rien";
                if (VerreSpecialOD !="non") ResultatOD = VerreSpecialOD;

                if (ui->CylindreOG->value() != 0.0 && ResultatGlobalSphereOG != 0.0)
                    ResultatVLOG = ResultatVLOG + " (" + Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() + ")";
                if (ui->CylindreOG->value() != 0.0 && ResultatGlobalSphereOG == 0.0)
                    ResultatVLOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
                if (ui->CylindreOG->value() == 0.0 && ResultatGlobalSphereOG == 0.0)
                    ResultatVLOG = tr("plan");
                ResultatOG = ResultatVLOG;
                if (!ui->OGPrescritCheckBox->isChecked())
                    ResultatOG = "Rien";
                if (VerreSpecialOG !="non") ResultatOG = VerreSpecialOG;

                if (ResultatOD == ResultatOG)
                {
                    if (VerreSpecialOD != "non")
                        m_resultFronto = VerreSpecialOD + tr(" ODG");
                    else
                        m_resultFronto = ResultatVLOD + tr(" VP ODG");
                }
                else
                {
                    if (VerreSpecialOD != "non")
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = VerreSpecialOD + " / " + VerreSpecialOG;
                        else if (ResultatOG == "Rien")
                            m_resultFronto = VerreSpecialOD + tr(" OD");
                        else
                            m_resultFronto = VerreSpecialOD + " / " + ResultatVLOG + tr(" VP");
                    }
                    else  if (ResultatOD != "Rien")
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = ResultatVLOD + " / " + VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            m_resultFronto = ResultatVLOD +  tr(" OD VP");
                        else
                            m_resultFronto = ResultatVLOD + " / " + ResultatVLOG +  tr(" VP");
                    }
                    else
                    {
                        if (VerreSpecialOG != "non")
                            m_resultFronto = VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            m_resultFronto = ResultatVLOG +  tr(" OG VP");
                        else
                            m_resultFronto = ResultatVLOG +  tr(" OG VP");
                    }
                }
            }
            break;
        default:
            break;
        }


        // 2- incorporation des valeurs des prismes eventuels

        ResultatPrismeOD = "";
        if (ui->PrismeOD->value() != 0.0 && ui->BasePrismeTextODComboBox->currentText() != "" && !ui->DepoliODCheckBox->isChecked())
            ResultatPrismeOD = ui->PrismeOD->text() + " base " + ui->BasePrismeTextODComboBox->currentText();
        if (ui->PrismeOD->value() != 0.0 && ui->BasePrismeTextODComboBox->currentText() == "" && !ui->DepoliODCheckBox->isChecked())
            ResultatPrismeOD = ui->PrismeOD->text() + " base " + ui->BasePrismeOD->text();

        ResultatPrismeOG = "";
        if (ui->PrismeOG->value() != 0.0 && ui->BasePrismeTextOGComboBox->currentText() != "" && !ui->DepoliOGCheckBox->isChecked())
            ResultatPrismeOG = ui->PrismeOG->text() + " base " + ui->BasePrismeTextOGComboBox->currentText();
        if (ui->PrismeOG->value() != 0.0 && ui->BasePrismeTextOGComboBox->currentText() == "" && !ui->DepoliOGCheckBox->isChecked())
            ResultatPrismeOG = ui->PrismeOG->text() + " base " + ui->BasePrismeOG->text();

        if (ResultatPrismeOD != "" &&  ResultatPrismeOG != "" && ResultatPrismeOD != ResultatPrismeOG)
        {
            if (ui->PressonODCheckBox->isChecked() && ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + " / " + ResultatPrismeOG + "</td>";
            if (ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" OD") + "</td>"
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + tr(" OD") + "</td>"
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + " / " + ResultatPrismeOG + "</td>";
        }
        if (ResultatPrismeOD != "" &&  ResultatPrismeOG != "" && ResultatPrismeOD == ResultatPrismeOG)
        {
            if (ui->PressonODCheckBox->isChecked() && ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" ODG") + "</td>";
            if (ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" OD") + "</td>"
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + tr(" OD") + "</td>"
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prismes ") + ResultatPrismeOD + tr(" ODG") + "</td>";
        }
        if (ResultatPrismeOD != "" &&  ResultatPrismeOG == "")
        {
            if (ui->PressonODCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" OD") + "</td>";
            else
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + tr(" OD") + "</td>";
        }
        if (ResultatPrismeOD == "" &&  ResultatPrismeOG != "")
        {
            if (ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Press-On " + ResultatPrismeOG + tr(" OG") + "</td>";
            else
                ResultatPrisme =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOG + tr(" OG") + "</td>";
        }

        //3 - incorporation des valeurs de Ryser
        ResultatRyser = "";
        if (ui->RyserODCheckBox->isChecked())
            ResultatRyser =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Ryser " + ui->RyserSpinBox->text() + tr(" OD") + "</td>";
        if (ui->RyserOGCheckBox->isChecked())
            ResultatRyser =
                        HTML_RETOURLIGNE "<td width=\"60\"></td><td>Ryser " + ui->RyserSpinBox->text() + tr(" OG") + "</td>";

        // 4 - détermination du resultat final
        if (m_mode == Refraction::Fronto)
        {
            m_resultPubliFronto =  "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>Porte:</b></font></td><td>" + m_resultFronto + "</td>" + ResultatPrisme + ResultatRyser;
            m_resultPubliFronto.insert(m_resultPubliFronto.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
        else if (m_mode == Refraction::Prescription)
        {
            m_resultPrescription =  "<td width=\"30\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>VP:</b></font></td><td>" + m_resultFronto + " " + ui->CommentairePrescriptionTextEdit->toPlainText() + "</td>" + ResultatPrisme + ResultatRyser;
            m_resultPrescription.insert(m_resultPrescription.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
    }

    if (m_mode == Refraction::Autoref)
        //EN MODE Autoref --  détermination de gResultatA  ---------------------------------------------------------------------------
    {
        m_resultPubliAutoref = "";
        {
            // détermination OD
            if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() != 0.0)
                ResultatVLOD = Valeur(ui->SphereOD->text()) + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() != 0.0)
                ResultatVLOD = Valeur(ui->SphereOD->text());
            if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() == 0.0)
                ResultatVLOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() == 0.0)
                ResultatVLOD = tr("plan");
            if (!ui->ODCheckBox->isChecked()) ResultatOD = "Rien";

            // détermination OG
            if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() != 0.0)
                ResultatVLOG = Valeur(ui->SphereOG->text()) + " (" + Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() != 0.0)
                ResultatVLOG = Valeur(ui->SphereOG->text());
            if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() == 0.0)
                ResultatVLOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() == 0.0)
                ResultatVLOG = tr("plan");
            if (!ui->OGCheckBox->isChecked()) ResultatOG = "Rien";

            // Détermination de gResultatA
        }
        if (m_resultPubliAutoref == "" && ResultatOD == "Rien" && ResultatOG != "Rien")
            m_resultPubliAutoref = ResultatVLOG + tr(" OG");

        if (m_resultPubliAutoref == "" && ResultatOG == "Rien" && ResultatOD != "Rien")
            m_resultPubliAutoref = ResultatVLOD + tr(" OD");

        if (m_resultPubliAutoref == "" && ResultatOD != "Rien" && ResultatOG != "Rien")
            m_resultPubliAutoref = ResultatVLOD + " / " + ResultatVLOG;

        QString kerato = "";
        if (QLocale().toDouble(ui->K1OD->text())>0 && m_modeouverture == Manuel)
        {
            if (DataBase::I()->donneesOphtaPatient()->dioptriesKOD()!=0.0)
                kerato += "</p> " HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OD->text() + "/" + ui->K2OD->text() + " Km = " + QString::number((QLocale().toDouble(ui->K1OD->text()) + QLocale().toDouble(ui->K2OD->text()))/2,'f',2)
                        + "</td><td width=\"120\">" + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesKOD(),'f',2) +  tr(" à ") + ui->AxeKOD->text() + "°</td>";
            else
                kerato += "</p> " HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td><td width=\"240\">"
                        + ui->K1OD->text() + tr(" à ") + ui->AxeKOD->text() + "°/" + ui->K2OD->text() + tr(" Km = ") + QString::number((QLocale().toDouble(ui->K1OD->text()) + QLocale().toDouble(ui->K2OD->text()))/2,'f',2) + "</td>";
        }
        if (QLocale().toDouble(ui->K1OG->text())>0 && m_modeouverture == Manuel)
        {
            if (DataBase::I()->donneesOphtaPatient()->dioptriesKOG()!=0.0)
                kerato += "</p> " HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OG->text() + "/" + ui->K2OG->text() + " Km = " + QString::number((QLocale().toDouble(ui->K1OG->text()) + QLocale().toDouble(ui->K2OG->text()))/2,'f',2)
                        + "</td><td width=\"120\">" + QString::number(DataBase::I()->donneesOphtaPatient()->dioptriesKOG(),'f',2) +  tr(" à ") + ui->AxeKOG->text() + "°</td>";
            else
                kerato += "</p> " HTML_RETOURLIGNE "<td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OG->text() +  tr(" à ") + ui->AxeKOG->text() + "°/" + ui->K2OG->text() + tr(" Km = ") + QString::number((QLocale().toDouble(ui->K1OG->text()) + QLocale().toDouble(ui->K2OG->text()))/2,'f',2) + "</td>";
        }
        if (ui->CycloplegieCheckBox->isChecked())
        {
            m_resultAutorefDilate    = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>Autoref:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + m_resultPubliAutoref + "</td><td><font color = \"red\">" + tr("(dilaté)") + "</font></td>" + kerato;
            m_resultAutorefDilate.insert(m_resultAutorefDilate.lastIndexOf("</font></td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
        else
        {
            m_resultAutorefnonDilate = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>Autoref:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + m_resultPubliAutoref + "</td><td>" + tr("(non dilaté)") + "</td>" + kerato;
            m_resultAutorefnonDilate.insert(m_resultAutorefnonDilate.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
    }

    if (m_mode == Refraction::Acuite)
        //EN MODE Refraction --  détermination de gResultatR  ---------------------------------------------------------------------------
    {
        // - 1 - détermination des verres
        m_resultPubliRefraction = "";

        // détermination OD
        if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() != 0.0)
            ResultatVLOD = Valeur(ui->SphereOD->text()) + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
        if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() != 0.0)
            ResultatVLOD = Valeur(ui->SphereOD->text());
        if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() == 0.0)
            ResultatVLOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
        if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() == 0.0)
            ResultatVLOD = tr("plan");

        if (ui->AddVPOD->value() > 0.0)
            ResultatVPOD = Valeur(ui->AddVPOD->text());
        else
            ResultatVPOD = tr("plan");

        ResultatOD = ResultatVLOD + " add." + ResultatVPOD + tr(" VP");
        if (!ui->ODCheckBox->isChecked())
            ResultatOD = "Rien";

        // détermination OG
        if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() != 0.0)
            ResultatVLOG = Valeur(ui->SphereOG->text()) + " (" + Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() + ")";
        if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() != 0.0)
            ResultatVLOG = Valeur(ui->SphereOG->text());
        if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() == 0.0)
            ResultatVLOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
        if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() == 0.0)
            ResultatVLOG = tr("plan");

        if (ui->AddVPOG->value() > 0.0)
            ResultatVPOG = Valeur(ui->AddVPOG->text());
        else
            ResultatVPOG = tr("plan");

        ResultatOG = ResultatVLOG + " add." + ResultatVPOG + tr(" VP");
        if (!ui->OGCheckBox->isChecked())
            ResultatOG = "Rien";

        QString colorVLOD, colorVPOD, colorVLOG, colorVPOG;
        colorVPOG = "\"blue\"";
        colorVPOD = "\"blue\"";
        colorVLOG = "\"blue\"";
        colorVLOD = "\"blue\"";
        double av = 0.0;
        if (wdg_AVLOD->text().contains("/10"))
            av = wdg_AVLOD->text().replace("/10","").toDouble();
        if (av < 6)
            colorVLOD =  "\"red\"";
        if (av > 5 && av < 9)
            colorVLOD =  "\"orange\"";
        av = 0;
        if (wdg_AVLOG->text().contains("/10"))
            av = wdg_AVLOG->text().replace("/10","").toDouble();
        if (av < 6)
            colorVLOG =  "\"red\"";
        if (av >5 && av < 9)
            colorVLOG =  "\"orange\"";
        if (wdg_AVPOD->text().replace(",",".").toInt() > 4 || wdg_AVPOD->text().contains("<"))
            colorVPOD =  "\"red\"";
        if (wdg_AVPOD->text().replace(",",".").toInt() > 2 && wdg_AVPOD->text().replace(",",".").toInt() < 5)
            colorVPOD =  "\"orange\"";
        if (wdg_AVPOG->text().replace(",",".").toInt() > 4 || wdg_AVPOG->text().contains("<"))
            colorVPOG =  "\"red\"";
        if (wdg_AVPOG->text().replace(",",".").toInt() > 2 && wdg_AVPOG->text().replace(",",".").toInt() < 5)
            colorVPOG =  "\"orange\"";


        // Détermination de gResultatR
        switch (Distance)
        {
        case Refraction::AllDistance: // Refraction de loin et de près
            if (ResultatOD != "Rien")
            {
                if (ui->AddVPOD->value() == 0.0)
                {
                    if (ResultatOG == "Rien")
                        m_resultPubliRefraction = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + wdg_AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD");
                    else
                    {
                        if (ui->AddVPOG->value() == 0.0)
                            m_resultPubliRefraction = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + wdg_AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD") + "</td></p>"
                                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + wdg_AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG") + "</td>";
                        else
                            m_resultPubliRefraction = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + wdg_AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD") + "</td></p>"
                                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + wdg_AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG") + "</td>";
                    }
                }
                else
                {
                    if (ResultatOG == "Rien")
                        m_resultPubliRefraction = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + wdg_AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD");
                    else
                    {
                        if (ui->AddVPOG->value() == 0.0)
                            m_resultPubliRefraction = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + wdg_AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD") + "</td></p>"
                                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + wdg_AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG") + "</td>";
                        else
                            m_resultPubliRefraction = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + wdg_AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD") + "</td></p>"
                                    HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + wdg_AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG") + "</td>";
                    }
                }
            }
            else if (ResultatOG != "Rien")
            {
                if (ui->AddVPOG->value() == 0.0)
                    m_resultPubliRefraction = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + wdg_AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG");
                else
                    m_resultPubliRefraction = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + wdg_AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG");
            }
            break;
        case Refraction::Loin: // Réfraction de loin ou sous cycloplégie
            if (ResultatOD != "Rien")
            {
                if (ResultatOG == "Rien")
                    m_resultPubliRefraction = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</b></font> " + tr("OD");
                else
                    m_resultPubliRefraction = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + wdg_AVLOD->text() + "</b></font> " + tr("OD") + "</td></p>"
                            HTML_RETOURLIGNE "<td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                            + ResultatVLOG + " " + "<font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</b></font> " + tr("OG") + "</td>";
            }
            else if (ResultatOG != "Rien")
                m_resultPubliRefraction = ResultatVLOG + "<font color = " + colorVLOG + "><b>" + wdg_AVLOG->text() + "</b></font> " + tr("OG");
            break;
        default:
            break;
        }
        if (ui->CycloplegieCheckBox->isChecked())
        {
            m_resultRefractionDilate = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + m_resultPubliRefraction + "</td><td width=\"60\"><font color = \"red\">" + tr("(dilaté)") + "</font></td><td>" + Datas::I()->users->userconnected()->login() + "</td>";
            m_resultRefractionDilate.insert(m_resultRefractionDilate.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
        else
        {
            m_resultRefractionnonDilate = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + m_resultPubliRefraction + "</td><td width=\"70\">" + tr("(non dilaté)") + "</td><td>" + Datas::I()->users->userconnected()->login() + "</td>";
            m_resultRefractionnonDilate.insert(m_resultRefractionnonDilate.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
    }

    // Consolidation de tous les résultats dans un même QString

    m_resultObservation = m_resultPubliFronto + m_resultAutorefnonDilate + m_resultAutorefDilate + m_resultRefractionnonDilate + m_resultRefractionDilate;
    m_resultPrescription = m_resultPrescription + m_commentaireresume;
}

// -------------------------------------------------------------------------------------
// ResumePrescription - génère l'aperçu de la prescription qui apparaît dans la fenêtre Aperçu
//--------------------------------------------------------------------------------------
void dlg_refraction::ResumePrescription()
{
    QString QuelOeil, VerreSpecialOD, VerreSpecialOG, Resultat, ResultatOD, ResultatOG;

    if (ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked()) QuelOeil = "2";
    else if (ui->OGPrescritCheckBox->isChecked())                                   QuelOeil = "G";
    else if (ui->ODPrescritCheckBox->isChecked())                                   QuelOeil = "D";

    // a - Determination de la puissance du verre

    VerreSpecialOD = "non";
    VerreSpecialOG = "non";
    if (ui->frame_Detail->isVisible())
    {
        if (ui->PlanODCheckBox->isChecked())
        {
            ResultatOD = tr("plan");
            VerreSpecialOD = "oui";
        }
        if (ui->PlanOGCheckBox->isChecked())
        {
            ResultatOG = tr("plan");
            VerreSpecialOG = "oui";
        }
        if (ui->DepoliODCheckBox->isChecked())
        {
            ResultatOD = tr("dépoli");
            VerreSpecialOD = "oui";
        }
        if (ui->DepoliOGCheckBox->isChecked())
        {
            ResultatOG = tr("dépoli");
            VerreSpecialOG = "oui";
        }
    }

    if (VerreSpecialOG == "oui" && VerreSpecialOD == "oui")
    {
        QuelOeil ="";
        if (ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked())     Resultat = "\tOD " + ResultatOD + "\n\tOG " + ResultatOG;
        else if (ui->OGPrescritCheckBox->isChecked())                                       Resultat = "\tOG " + ResultatOG;
        else if (ui->ODPrescritCheckBox->isChecked())                                       Resultat = "\tOD " + ResultatOD;
    }

    QString SphOD   = Valeur(QString::number(ui->SphereOD   ->value(),'f',2));                          // la méthode Valeur(ui->SphereOD->text()) oublie le + s'il n'est pas marqué... alors on utilise ce détour
    QString SphVPOD = Valeur(QString::number(ui->SphereOD   ->value()
                                             + ui->AddVPOD  ->value(),'f',2));                          // id
    QString CylOD   = Valeur(QString::number(ui->CylindreOD ->value(),'f',2));                          // id
    QString SphOG   = Valeur(QString::number(ui->SphereOG   ->value(),'f',2));                          // id
    QString SphVPOG = Valeur(QString::number(ui->SphereOG   ->value()
                                             + ui->AddVPOG  ->value(),'f',2));                          // id
    QString CylOG   = Valeur(QString::number(ui->CylindreOG ->value(),'f',2));                          // id
    QString AddVPOD = Valeur(QString::number(ui->AddVPOD    ->value(),'f',2));                          // id
    QString AddVPOG = Valeur(QString::number(ui->AddVPOG    ->value(),'f',2));                          // id

    // 1- Quand  QuelOeil = 2 -------------------------------------------------------------------
    if (QuelOeil == "2")
    {
        // Determination de ResultatOD en VL
        if (VerreSpecialOD == "non")
        {
            if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() != 0.0)
                ResultatOD = SphOD + " (" + CylOD + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() != 0.0)
                ResultatOD = SphOD;
            if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() == 0.0)
                ResultatOD = CylOD + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() == 0.0)
            {ResultatOD = tr("plan");   VerreSpecialOD = "oui";}
        }

        // Determination de ResultatOG en VL
        if (VerreSpecialOG == "non")
        {
            if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() != 0.0)
                ResultatOG = SphOG + " (" + CylOG + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() != 0.0)
                ResultatOG = SphOG;
            if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() == 0.0)
                ResultatOG = CylOG + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() == 0.0)
            {ResultatOG = tr("plan");   VerreSpecialOG = "oui";}
        }

        if (ui->VLPrescritRadioButton->isChecked())
        {
            ui->DeuxMonturesPrescritRadioButton->setVisible(false);
            Resultat = tr("Vision de loin") + "\n\t" + tr("OD ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
            if (VerreSpecialOD == "oui" && VerreSpecialOG == "non")
                Resultat = "\t" + tr("OD ") + ResultatOD +  "\n\t" + tr("OG Vision de loin ") + ResultatOG;
            if (VerreSpecialOD == "non" && VerreSpecialOG == "oui")
                Resultat = "\t" + tr("OD Vision de loin ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
            if (VerreSpecialOD == "oui" && VerreSpecialOG == "oui")
                Resultat = "\t" + tr("OD ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
        }

        if (ui->V2PrescritRadioButton->isChecked())
        {
            bool OKPourResultat = false;
            while (!OKPourResultat)
            {
                if (ui->AddVPOD->value() == 0.0 && ui->AddVPOG->value() == 0.0)
                {
                    ui->DeuxMonturesPrescritRadioButton->setVisible(false);
                    if (ui->DeuxMonturesPrescritRadioButton->isChecked())
                    {
                        ui->UneMonturePrescritRadioButton->setChecked(true);
                        ui->DeuxMonturesPrescritRadioButton->setChecked(false);
                    }
                    Resultat = tr("Vision de loin") + "\n\t" + tr("OD") + " "+  ResultatOD + "\n\t" + tr("OG") + " " + ResultatOG;
                    if (VerreSpecialOD == "oui" && VerreSpecialOG == "non")
                        Resultat = "\t" + tr("OD") + " " + ResultatOD +  "\n\t" + tr("OG Vision de loin") + " " + ResultatOG;
                    if (VerreSpecialOD == "non" && VerreSpecialOG == "oui")
                        Resultat = "\t" + tr("OD Vision de loin") + " " + ResultatOD + "\n\t" + tr("OG") + " " + ResultatOG;
                    if (VerreSpecialOD == "oui" && VerreSpecialOG == "oui")
                        Resultat = "\t" + tr("OD") + " " + ResultatOD + "\n\t" + tr("OG") + " " + ResultatOG;
                    OKPourResultat = true;
                }
                else if (ui->AddVPOD->value() != 0.0 && ui->AddVPOG->value() != 0.0)
                {
                    ui->DeuxMonturesPrescritRadioButton->setVisible(true);
                    Resultat = tr("Vision de loin") + "\n\t" + tr("OD ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
                    if (ResultatOD != tr("dépoli") && ResultatOG != tr("dépoli") && !ui->PlanODCheckBox->isChecked() && !ui->PlanOGCheckBox->isChecked())
                        Resultat += "\n"+ tr("Vision de près") + "\n\t" "add. " + AddVPOD + tr(" VP OD") + "\n\t" "add. " + AddVPOG + tr(" VP OG");
                    else if (ResultatOD == tr("dépoli"))
                        Resultat += "\n"+ tr("Vision de près") + "\n\t" "add. " + AddVPOG + tr(" VP OG");
                    else if (ResultatOG == tr("dépoli"))
                        Resultat += "\n" + tr("Vision de près") + "\n\t" "add. " + AddVPOD + tr(" VP OD");
                    else if (ui->PlanODCheckBox->isChecked())
                        Resultat += "\n" + tr("Vision de près") + "\n\t" "add. " + AddVPOG + tr(" VP OG");
                    else if (ui->PlanOGCheckBox->isChecked())
                        Resultat += "\n" + tr("Vision de près") + "\n\t" "add. " + AddVPOD + tr(" VP OD");
                    OKPourResultat = true;
                }
                else if (ui->AddVPOD->value() != 0.0 && ui->AddVPOG->value() == 0.0)
                {
                    ui->DeuxMonturesPrescritRadioButton->setVisible(!(ResultatOD == tr("dépoli") || ui->PlanODCheckBox->isChecked()));
                    Resultat = tr("Vision de loin") + "\n\t" + tr("OD ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
                    if (ResultatOD != tr("dépoli") && !ui->PlanODCheckBox->isChecked())
                        Resultat += "\n" + tr("Vision de près") + "\n\tadd. " + AddVPOD + tr(" VP OD");
                    else
                    {
                        ui->DeuxMonturesPrescritRadioButton->setVisible(false);
                        if (ui->DeuxMonturesPrescritRadioButton->isChecked())
                        {
                            ui->UneMonturePrescritRadioButton->setChecked(true);
                            ui->DeuxMonturesPrescritRadioButton->setChecked(false);
                        }
                    }
                    OKPourResultat = true;
                }
                else if (ui->AddVPOD->value() == 0.0 && ui->AddVPOG->value() != 0.0)
                {
                    ui->DeuxMonturesPrescritRadioButton->setVisible(!(ResultatOG == tr("dépoli") || ui->PlanOGCheckBox->isChecked()));
                    Resultat = tr("Vision de loin") + "\n\t" + tr("OD ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
                    if (ResultatOG != tr("dépoli") && !ui->PlanOGCheckBox->isChecked())
                        Resultat += "\n" + tr("Vision de près") + "\n\t" "add. " + AddVPOG + tr(" VP OG");
                    else
                    {
                        ui->DeuxMonturesPrescritRadioButton->setVisible(false);
                        if (ui->DeuxMonturesPrescritRadioButton->isChecked())
                        {
                            ui->UneMonturePrescritRadioButton->setChecked(true);
                            ui->DeuxMonturesPrescritRadioButton->setChecked(false);
                        }
                    }
                    OKPourResultat = true;
                }
            }

        } // fin V2 coche
        if (ui->VPPrescritRadioButton->isChecked())
        {
            ui->DeuxMonturesPrescritRadioButton->setVisible(false);
            // Oeil Droit
            if (ui->SphereOD->value() + ui->AddVPOD->value() == 0.0)
            {if (ui->CylindreOD->value() == 0.0)
                {VerreSpecialOD = "oui"; ResultatOD = tr("plan");}
                else
                    ResultatOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
            }
            if (ui->SphereOD->value() + ui->AddVPOD->value() != 0.0)
            {
                VerreSpecialOD = "non";
                ResultatOD = Valeur(QString::number((ui->SphereOD->value() + ui->AddVPOD->value()) ,'f',2));
                if (ui->CylindreOD->value() != 0.0)
                    ResultatOD = ResultatOD + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            }
            // Oeil Gauche
            if (ui->SphereOG->value() + ui->AddVPOG->value() == 0.0)
            {if (ui->CylindreOG->value() == 0.0)
                {VerreSpecialOG = "oui"; ResultatOG = tr("plan");}
                else
                    ResultatOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
            }
            if (ui->SphereOG->value() + ui->AddVPOG->value() != 0.0)
            {
                VerreSpecialOG = "non";
                ResultatOG = Valeur(QString::number((ui->SphereOG->value() + ui->AddVPOG->value()) ,'f',2));
                if (ui->CylindreOG->value() != 0.0)
                    ResultatOG = ResultatOG + " (" + Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            }
            Resultat = tr("Vision de près") + "\n\t" + tr("OD ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
            if (VerreSpecialOD == "oui" && VerreSpecialOG == "non")
                Resultat = "\t" + tr("OD ") + ResultatOD +  "\n\t" + tr("OG Vision de près ") + ResultatOG;
            if (VerreSpecialOD == "non" && VerreSpecialOG == "oui")
                Resultat = "\t" + tr("OD Vision de près ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
            if (VerreSpecialOD == "oui" && VerreSpecialOG == "oui")
                Resultat = "\t" + tr("OD ") + ResultatOD + "\n\t" + tr("OG ") + ResultatOG;
        } // fin VP coche

    }// fin QuelOeil = 2


    // 2- Quand  QuelOeil = D --------------------------------------------------------------------
    if (QuelOeil == "D")
    {
        // Determination de ResultatOD en VL
        if (VerreSpecialOD == "non")
        {
            if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() != 0.0)
                ResultatOD = SphOD + " (" + CylOD + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() != 0.0)
                ResultatOD = SphOD;
            if (ui->CylindreOD->value() != 0.0 && ui->SphereOD->value() == 0.0)
                ResultatOD = CylOD + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0.0 && ui->SphereOD->value() == 0.0)
            {ResultatOD = tr("plan");   VerreSpecialOD = "oui";}
        }

        if (ui->VLPrescritRadioButton->isChecked())
        {
            ui->DeuxMonturesPrescritRadioButton->setVisible(false);
            if (VerreSpecialOD == "non")
                Resultat = "\t" + tr("OD") + "\n\t   " + tr("Vision de loin ") + ResultatOD ;
            else
                Resultat = "\t" + tr("OD ") + ResultatOD ;
        }

        if (ui->V2PrescritRadioButton->isChecked())
        {
            if (ui->AddVPOD->value() == 0.0)
            {
                ui->DeuxMonturesPrescritRadioButton->setVisible(false);
                if (ui->DeuxMonturesPrescritRadioButton->isChecked())
                {
                    ui->UneMonturePrescritRadioButton->setChecked(true);
                    ui->DeuxMonturesPrescritRadioButton->setChecked(false);
                }
                if (VerreSpecialOD == "non")
                    Resultat = "\t" + tr("OD") + "\n\t   " + tr("Vision de loin ") + ResultatOD ;
                else
                    Resultat = "\tOD " + ResultatOD ;
            }
            else
            {
                ui->DeuxMonturesPrescritRadioButton->setVisible(true);
                if (ResultatOD == tr("dépoli") || ui->PlanODCheckBox->isChecked())
                    Resultat   = "\t" + tr("OD ") + ResultatOD;
                else
                    Resultat   = "\t" + tr("OD") + "\n\t   " + tr("Vision de loin ") + ResultatOD + "\n\t   " + tr("Vision de près") + " add. " + AddVPOD;
            }
        }

        if (ui->VPPrescritRadioButton->isChecked())
        {
            ui->DeuxMonturesPrescritRadioButton->setVisible(false);
            if (ui->CylindreOD->value() != 0.0 && SphVPOD != "0.00")   ResultatOD = SphVPOD + " (" + CylOD + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0.0 && SphVPOD != "0.00")   ResultatOD = SphVPOD;
            if (ui->CylindreOD->value() != 0.0 && SphVPOD == "0.00")   ResultatOD = CylOD + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0.0 && SphVPOD == "0.00")   ResultatOD = tr("plan");
            if (ui->PlanODCheckBox->isChecked()) ResultatOD = tr("plan");
            if (ResultatOD == tr("dépoli")|| ResultatOD == tr("plan"))
                Resultat   = "\t" + tr("OD ") + ResultatOD;
            else
                Resultat = "\t" + tr("OD") + "\n\t   " + tr("Vision de près ") + ResultatOD;
        }
    } // fin QuelOeil = D

    // 3- Quand  QuelOeil = G ------------------------------------------------------------------------
    if (QuelOeil == "G")
    {
        // Determination de ResultatOG en VL
        if (VerreSpecialOG == "non")
        {
            if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() != 0.0)
                ResultatOG = SphOG + " (" + CylOG + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() != 0.0)
                ResultatOG = SphOG;
            if (ui->CylindreOG->value() != 0.0 && ui->SphereOG->value() == 0.0)
                ResultatOG = CylOG + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0.0 && ui->SphereOG->value() == 0.0)
            {ResultatOG = tr("plan");   VerreSpecialOG = "oui";}
        }

        if (ui->VLPrescritRadioButton->isChecked())
        {
            ui->DeuxMonturesPrescritRadioButton->setVisible(false);
            if (VerreSpecialOG == "non")
                Resultat = "\t" + tr("OG") + "\n\t   " + tr("Vision de loin ") + ResultatOG ;
            else
                Resultat = "\t"+ tr("OG ") + ResultatOG ;
        }

        if (ui->V2PrescritRadioButton->isChecked())
        {
            if (ui->AddVPOG->value() == 0.0)
            {
                ui->DeuxMonturesPrescritRadioButton->setVisible(false);
                if (ui->DeuxMonturesPrescritRadioButton->isChecked())
                {
                    ui->UneMonturePrescritRadioButton->setChecked(true);
                    ui->DeuxMonturesPrescritRadioButton->setChecked(false);
                }
                if (VerreSpecialOG == "non")
                    Resultat = "\t" + tr("OG") + "\n\t   " + tr("Vision de loin ") + ResultatOG ;
                else
                    Resultat = "\t" + tr("OG ") + ResultatOG ;
            }
            else
            {
                ui->DeuxMonturesPrescritRadioButton->setVisible(true);
                if (ResultatOG == tr("dépoli") || ui->PlanOGCheckBox->isChecked())
                    Resultat   = "\t" + tr("OG ") + ResultatOG;
                else
                    Resultat   = "\t" + tr("OG") + "\n\t   " + tr("Vision de loin ") + ResultatOG + "\n\t   " + tr("Vision de près") + " add. " + AddVPOG;
            }
        }

        if (ui->VPPrescritRadioButton->isChecked())
        {
            ui->DeuxMonturesPrescritRadioButton->setVisible(false);
            if (ui->CylindreOG->value() != 0.0 && SphVPOG != "0.00")   ResultatOG = SphVPOG + " (" + CylOG + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0.0 && SphVPOG != "0.00")   ResultatOG = SphVPOG;
            if (ui->CylindreOG->value() != 0.0 && SphVPOG == "0.00")   ResultatOG = CylOG + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0.0 && SphVPOG == "0.00")   ResultatOG = tr("plan");
            if (ui->PlanOGCheckBox->isChecked()) ResultatOG = tr("plan");
            if (ResultatOG == tr("dépoli") || ui->PlanOGCheckBox->isChecked() || ResultatOG == tr("plan"))
                Resultat   = "\t" + tr("OG ") + ResultatOG;
            else
                Resultat = "\t" + tr("OG") + "\n\t   " + tr("Vision de près ") + ResultatOG;
        }

    } // fin QuelOeil = G

     // 4 - Les autres parametres ----------------------------------------------------------
    // 4-1 Les verres teintes
    QString A = "";
    if (ui->VerresTeintesCheckBox->isChecked() && (ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked()))
        A = tr("verres correcteurs teintés");
    if (ui->VerresTeintesCheckBox->isChecked() && (!ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked()))
        {
        A = tr("verre correcteur");
        if (VerreSpecialOG != tr("OG dépoli"))
        A = A + " " + tr("teinté");
        }
    if (ui->VerresTeintesCheckBox->isChecked() && (ui->ODPrescritCheckBox->isChecked() && !ui->OGPrescritCheckBox->isChecked()))
        {
        A = tr("verre correcteur");
        if (VerreSpecialOD != tr("OD dépoli"))
        A = A + " " + tr("teinté");
        }
    if (!ui->VerresTeintesCheckBox->isChecked() && (ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked()))
        A = tr("verres correcteurs");
    if (!ui->VerresTeintesCheckBox->isChecked() && !(ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked()))
        A = tr("verre correcteur");

     // 4-2 un verre ou deux
     if ((ui->ODPrescritCheckBox->isChecked() && ui->OGPrescritCheckBox->isChecked())
             && (ui->UneMonturePrescritRadioButton->isChecked() || ui->DeuxMonturesPrescritRadioButton->isChecked()))
         Resultat = A + "\n\n" + Resultat;
     else
         Resultat = tr("Remplacement") + " " + A + "\n\n" + Resultat;

    // 4-3 Les Ryser

    QString Ryser = "";
    if (ui->RyserODCheckBox->isChecked())
        Ryser = "Ryser " + ui->RyserSpinBox->text() + tr(" OD");
    if (ui->RyserOGCheckBox->isChecked())
        Ryser = "Ryser " + ui->RyserSpinBox->text() + tr(" OG");
    if (Ryser != "")
        Resultat = Resultat + "\n" + Ryser;

    // 4-4 Les prismes
    if (ui->PrismeGroupBox->isVisible())
    {
        QString ODPrisme = "";
        QString OGPrisme = "";
        if (ui->PrismeOD->value() != 0.0 && !ui->DepoliODCheckBox->isChecked())
            {ODPrisme = Valeur(QString::number(ui->PrismeOD->value(),'f',2));
            if (ui->BasePrismeTextODComboBox->currentText() != "")
                ODPrisme = ODPrisme + tr(" dioptrie base ") + ui->BasePrismeTextODComboBox->currentText() +tr(" OD");
            else
                ODPrisme = ODPrisme + tr(" dioptrie base ") + ui->BasePrismeOD->text() +tr(" OD");
            if (ui->PressonODCheckBox->isChecked())
                    ODPrisme = "Press-On " + ODPrisme;
                    else
                    ODPrisme = tr("Prisme ") + ODPrisme;
            }
        if (ui->PrismeOG->value() != 0.0 && !ui->DepoliOGCheckBox->isChecked())
            {OGPrisme = Valeur(QString::number(ui->PrismeOG->value(),'f',2));
            if (ui->BasePrismeTextOGComboBox->currentText() != "")
                OGPrisme = OGPrisme + tr(" dioptrie base ") + ui->BasePrismeTextOGComboBox->currentText() +tr(" OG");
            else
                OGPrisme = OGPrisme + tr(" dioptrie base ") + ui->BasePrismeOG->text() +tr(" OG");
            if (ui->PressonOGCheckBox->isChecked())
                    OGPrisme = "Press-On " + OGPrisme;
                    else
                    OGPrisme = tr("Prisme ") + OGPrisme;
            }

        if (ODPrisme != "" && OGPrisme == "")
            Resultat = Resultat + "\n" + ODPrisme;
        if (ODPrisme == "" && OGPrisme != "")
            Resultat = Resultat + "\n" + OGPrisme;
        if (ODPrisme != "" && OGPrisme != "")
            Resultat = Resultat + "\n" + ODPrisme + "\n" + OGPrisme;
    }

    //4-5 La monture
    if (ui->UneMonturePrescritRadioButton->isChecked())
        Resultat = Resultat + "\n" + tr("Monture");
    if (ui->DeuxMonturesPrescritRadioButton->isChecked())
        Resultat = Resultat + "\n" + tr("2 montures");

    //4-6 Les commentaires
    QTextEdit txtedit;
    txtedit.setText(Resultat);
    Resultat = txtedit.toHtml();
    if (ui->CommentairePrescriptionTextEdit->toPlainText() != "")
        Resultat += HTML_RETOURLIGNE + ui->CommentairePrescriptionTextEdit->toHtml();
    Resultat += HTML_RETOURLIGNE "<td width=\"60\">""</td>";
    txtedit.setText(m_commentaire);
    m_commentaire = txtedit.toHtml();
    Resultat += HTML_RETOURLIGNE "<td width=\"500\"><span style=\"font-size:10pt;color:blue\"><b>" + m_commentaire.toUpper() + "</b></span></td>";

    ui->ResumePrescriptionTextEdit->setText(Resultat);
}

//---------------------------------------------------------------------------------
// Resume de la refraction - fait apparaître un résumé des réfractions précédentes dans une fenêtre resumeRefraction
//---------------------------------------------------------------------------------
void dlg_refraction::ResumeRefraction()
{
    // construction du texte Resume Refraction a partir de la table Refraction.
    // 2 - La partie Autoref
    QString ResultatAutorefDil      = "";
    QString ResultatAutorefNonDil   = "";
    QString ResultatAutoref         = "" ;


    // 2-1 - Mesure dilatee
    ResultatAutorefDil = RechercheResultat(Refraction::Autoref, Refraction::Dilatation);

    // 2-2- Mesure non dilatee
    ResultatAutorefNonDil = RechercheResultat(Refraction::Autoref, Refraction::NoDilatation);

    // 2-3- Determination de ResultatAutoref
    if (ResultatAutorefDil == "" && ResultatAutorefNonDil != "")
        ResultatAutoref = tr("AUTOREFRACTOMÈTRIE") + ResultatAutorefNonDil;
    if (ResultatAutorefDil != "" && ResultatAutorefNonDil == "")
        ResultatAutoref = tr("AUTOREFRACTOMÈTRIE") + ResultatAutorefDil;
    if (ResultatAutorefDil != "" && ResultatAutorefNonDil != "")
        ResultatAutoref = tr("AUTOREFRACTOMÈTRIE") +  ResultatAutorefDil + ResultatAutorefNonDil;

    // 3 - la partie Refraction
    QString ResultatRefractionDil   = "";
    QString ResultatRefractionV2    = "";
    QString ResultatRefractionVL    = "";
    QString ResultatRefractionNonDil= "";
    QString ResultatRefraction      = "";

    // 3-1 Mesure dilatee
    ResultatRefractionDil = RechercheResultat(Refraction::Acuite, Refraction::Dilatation);

    // 3-2 Mesure non dilatee
    ResultatRefractionV2 = RechercheResultat(Refraction::Acuite, Refraction::NoDilatation, Refraction::AllDistance);
    ResultatRefractionVL = RechercheResultat(Refraction::Acuite, Refraction::NoDilatation, Refraction::Loin);

    // calcul formule avec dilation
    if (ResultatRefractionV2 != "" && ResultatRefractionVL != "")
        ResultatRefractionNonDil = ResultatRefractionV2 + "\n" + ResultatRefractionVL;
    if (ResultatRefractionV2 == "" && ResultatRefractionVL != "")
        ResultatRefractionNonDil =  ResultatRefractionVL;
    if (ResultatRefractionV2 != "" && ResultatRefractionVL == "")
        ResultatRefractionNonDil =  ResultatRefractionV2;

     // 3-3 calcul de la formule de refraction finale
    if (ResultatRefractionDil != "" && ResultatRefractionNonDil != "")
        ResultatRefraction = ResultatRefractionDil + ResultatRefractionNonDil;
    if (ResultatRefractionDil == "" && ResultatRefractionNonDil != "")
        ResultatRefraction =  ResultatRefractionNonDil;
    if (ResultatRefractionDil != "" && ResultatRefractionNonDil == "")
        ResultatRefraction =  ResultatRefractionDil;
    if (ResultatRefraction != "")
        ResultatRefraction = tr("DERNIÈRES MESURES D'ACUITÉ VISUELLE") + ResultatRefraction ;
    if (ResultatAutoref != "")
        ResultatRefraction = ResultatAutoref + "\n\n" + ResultatRefraction;

    // 4 - La partie Verres portes ou prescrits
    QString ResultatVerres  = "";
    ResultatVerres = RechercheVerres();
    if (ResultatVerres != "")
        {
        ResultatVerres = tr("VERRES PRESCRITS OU MESURÉS") + "\n\n" + ResultatVerres;
        ResultatRefraction += "\n\n" + ResultatVerres;
        }
    proc->Edit(ResultatRefraction, tr("Historique réfractions ") + Datas::I()->patients->currentpatient()->prenom() + " " + Datas::I()->patients->currentpatient()->nom(), false);
}

//---------------------------------------------------------------------------------
// Masque ou pas les champs en fonction de type de mesure
//---------------------------------------------------------------------------------
void dlg_refraction::RegleAffichageFiche()
{

    // on affiche tous les widgets
    Afficher_AVL_AVP(true);
    Afficher_AddVP(true);
    ui->CycloplegieCheckBox->setVisible(true);
    ui->QuelleDistanceGroupBox->setVisible(true);
    ui->VPRadioButton->setVisible(true);
    ui->VLRadioButton->setChecked(false);
    ui->V2RadioButton->setVisible(true);
    ui->DetailsPushButton->setVisible(true);
    ui->DetailsPushButton->setEnabled(true);
    Afficher_Oeil_Droit(true);
    Afficher_Oeil_Gauche(true);

    // on masque les objets inutiles selon les cas
    if (m_mode == Refraction::Fronto)
    {
        ui->AnnulPushButton->setVisible(true);
        ui->OKPushButton->setIcon(Icons::icOK());
        ui->OKPushButton->setText( tr("Enregistrer\net fermer"));
        ui->ODCheckBox->setVisible(true);
        ui->OGCheckBox->setVisible(true);
        ui->KeratometrieGroupBox->setVisible(false);
        ui->frame_Prescription->setVisible(false);
        ui->V2RadioButton->setEnabled(true);
        ui->VPRadioButton->setEnabled(true);
        ui->VLRadioButton->setEnabled(true);
        ui->PorteRadioButton->setChecked(true);
        ui->QuelleDistanceGroupBox->setGeometry(735,10,120,95);
        ui->QuelleDistanceGroupBox->setVisible(true);
        Afficher_AVL_AVP(false);
        if (ui->VLRadioButton->isChecked() || ui->VPRadioButton->isChecked())
            Afficher_AddVP(false);
        ui->CycloplegieCheckBox->setVisible(false);
        m_affichedetail =
            (ui->PrismeOD->value() != 0.0         || ui->PrismeOG->value() != 0.0          ||
            ui->RyserODCheckBox->isChecked()      || ui->RyserOGCheckBox->isChecked()       ||
            ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()      ||
            ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked());
        if (m_affichedetail && ui->DetailsPushButton->text() ==  tr("- de détails"))
            ui->DetailsPushButton->setEnabled(false);
        if (ui->DetailsPushButton->text() ==  tr("- de détails")) m_affichedetail = true;
        AfficherDetail(m_affichedetail);
        if(m_affichedetail)
        {
            ui->PrismeGroupBox->setVisible(true);
            ui->VerresSpeciauxGroupBox->setVisible(true);
            ui->RyserGroupBox->setVisible(true);
            if (ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
                ui->RyserSpinBox->setVisible(true);
            else
                ui->RyserSpinBox->setVisible(false);
            if (ui->DepoliODCheckBox->isChecked())                 // si depoli on masque les mesures
            {
                Afficher_Oeil_Droit(false);
                if (ui->RyserODCheckBox->isChecked())
                    ui->RyserODCheckBox->setChecked(false);
                if (ui->RyserOGCheckBox->isChecked())
                    ui->RyserOGCheckBox->setChecked(false);
                ui->RyserGroupBox->setVisible(false);
            }
            if (ui->DepoliOGCheckBox->isChecked())                 // si depoli on masque les mesures
            {
                Afficher_Oeil_Gauche(false);
                if (ui->RyserODCheckBox->isChecked())
                    ui->RyserODCheckBox->setChecked(false);
                if (ui->RyserOGCheckBox->isChecked())
                    ui->RyserOGCheckBox->setChecked(false);
                ui->RyserGroupBox->setVisible(false);
            }
            if (ui->PlanODCheckBox->isChecked())
            {
                ui->SphereOD->setVisible(false);
                ui->CylindreOD->setVisible(false);
                ui->AxeCylindreOD->setVisible(false);
                ui->label_POD->setVisible(false);
                ui->AddVPOD->setVisible(false);
                ui->ConvODPushButton->setVisible(false);
            }
            if (ui->PlanOGCheckBox->isChecked())
            {
                ui->SphereOG->setVisible(false);
                ui->CylindreOG->setVisible(false);
                ui->AxeCylindreOG->setVisible(false);
                ui->label_POG->setVisible(false);
                ui->AddVPOG->setVisible(false);
                ui->ConvOGPushButton->setVisible(false);
            }
        }
        if (m_affichedetail)
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    } // fin mode Porte

    if (m_mode == Refraction::Autoref)
    {
        ui->AnnulPushButton->setVisible(true);
        ui->OKPushButton->setIcon(Icons::icOK());
        ui->OKPushButton->setText( tr("Enregistrer\net fermer"));
        ui->ODCheckBox->setVisible(true);
        ui->OGCheckBox->setVisible(true);
        ui->KeratometrieGroupBox->setVisible(true);
        ui->KeratometrieGroupBox->setEnabled(m_modeouverture == Manuel);
        ui->frame_Prescription->setVisible(false);
        ui->AutorefRadioButton->setChecked(true);
        ui->DetailsPushButton->setVisible(false);
        AfficherDetail(false);
        Afficher_AVL_AVP(false);
        Afficher_AddVP(false);
        ui->QuelleDistanceGroupBox->setVisible(false);
        setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    }

    if (m_mode == Refraction::Acuite)
    {
        ui->AnnulPushButton->setVisible(true);
        ui->OKPushButton->setIcon(Icons::icOK());
        ui->OKPushButton->setText( tr("Enregistrer\net fermer"));
        ui->ODCheckBox->setVisible(true);
        ui->OGCheckBox->setVisible(true);
        ui->KeratometrieGroupBox->setVisible(false);
        ui->frame_Prescription->setVisible(false);
        ui->V2RadioButton->setEnabled(true);
        ui->VLRadioButton->setEnabled(true);
        ui->RefractionRadioButton->setChecked(true);
        ui->VPRadioButton->setVisible(false);
        ui->QuelleDistanceGroupBox->setGeometry(735,10,120,65);
        if (ui->CycloplegieCheckBox->isChecked())
        {
            Afficher_AVP(false);
            Afficher_AddVP(false);
            ui->QuelleDistanceGroupBox->setVisible(false);
        }
        ui->DetailsPushButton->setVisible(false);
        if (ui->VLRadioButton->isChecked())
        {
            Afficher_AVP(false);
            Afficher_AddVP(false);
        }
        if (ui->VPRadioButton->isChecked())
        {
            ui->V2RadioButton->setChecked(true);
            Afficher_AVP(true);
            Afficher_AddVP(true);
        }
        AfficherDetail(false);
        setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    }

    if (m_mode == Refraction::Prescription)
    {
        Afficher_AVL_AVP(false);
        ui->OKPushButton->setIcon(Icons::icImprimer());
        ui->OKPushButton->setText( tr("Imprimer"));
        ui->CycloplegieCheckBox->setVisible(false);
        ui->KeratometrieGroupBox->setVisible(false);
        ui->ODCheckBox->setVisible(false);
        ui->OGCheckBox->setVisible(false);
        m_affichedetail =
            (ui->PrismeOD->value() != 0.0          || ui->PrismeOG->value() != 0.0      ||
            ui->RyserODCheckBox->isChecked()      || ui->RyserOGCheckBox->isChecked()   ||
            ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()  ||
            ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked());
        if (m_affichedetail && ui->DetailsPushButton->text() ==  tr("- de détails"))
            ui->DetailsPushButton->setEnabled(false);
        if (ui->DetailsPushButton->text() ==  tr("- de détails")) m_affichedetail = true;
        AfficherDetail(m_affichedetail);
        if(m_affichedetail)
        {
            ui->PrismeGroupBox->setVisible(true);
            ui->VerresSpeciauxGroupBox->setVisible(true);
            ui->KeratometrieGroupBox->setVisible(false);
            ui->RyserGroupBox->setVisible(true);
            if (ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
                ui->RyserSpinBox->setVisible(true);
            else
                ui->RyserSpinBox->setVisible(false);
            if (ui->DepoliODCheckBox->isChecked())                 // si depoli on masque les mesures
            {
                Afficher_Oeil_Droit(false);
                if (ui->RyserODCheckBox->isChecked())
                    ui->RyserODCheckBox->setChecked(false);
                if (ui->RyserOGCheckBox->isChecked())
                    ui->RyserOGCheckBox->setChecked(false);
                ui->RyserGroupBox->setVisible(false);
            }
            if (ui->DepoliOGCheckBox->isChecked())                 // si depoli on masque les mesures
            {
                Afficher_Oeil_Gauche(false);
                if (ui->RyserODCheckBox->isChecked())
                    ui->RyserODCheckBox->setChecked(false);
                if (ui->RyserOGCheckBox->isChecked())
                    ui->RyserOGCheckBox->setChecked(false);
                ui->RyserGroupBox->setVisible(false);
            }
            if (ui->PlanODCheckBox->isChecked())
            {
                ui->SphereOD->setVisible(false);
                ui->CylindreOD->setVisible(false);
                ui->AxeCylindreOD->setVisible(false);
                ui->label_POD->setVisible(false);
                ui->AddVPOD->setVisible(false);
                ui->ConvODPushButton->setVisible(false);
            }
            if (ui->PlanOGCheckBox->isChecked())
            {
                ui->SphereOG->setVisible(false);
                ui->CylindreOG->setVisible(false);
                ui->AxeCylindreOG->setVisible(false);
                ui->label_POG->setVisible(false);
                ui->AddVPOG->setVisible(false);
                ui->ConvOGPushButton->setVisible(false);
            }
        }

        ui->frame_Prescription->setVisible(true);
        if (!ui->ODCheckBox->isChecked()) ui->ODPrescritCheckBox->setChecked(false);
        ui->ODPrescritCheckBox->setVisible(ui->ODCheckBox->isChecked());
        if (!ui->OGCheckBox->isChecked()) ui->OGPrescritCheckBox->setChecked(false);
        ui->OGPrescritCheckBox->setVisible(ui->OGCheckBox->isChecked());

        ui->V2RadioButton->setEnabled(false);
        ui->VPRadioButton->setEnabled(false);
        ui->VLRadioButton->setEnabled(false);
        ui->PrescriptionRadioButton->setChecked(true);
        ui->OupsPushButton->setEnabled(false);
        ui->ReprendrePushButton->setEnabled(true);

        if (m_affichedetail)
            setFixedSize(width(), HAUTEUR_AVEC_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_AVEC_ORDONNANCE_SANS_DETAIL);
        ResumePrescription();
    }
    ui->OupsPushButton->setEnabled(Datas::I()->refractions->refractions()->size() > 0);
    ui->ReprendrePushButton->setEnabled(Datas::I()->refractions->refractions()->size() > 0);
    ui->ResumePushButton->setEnabled(Datas::I()->refractions->refractions()->size() > 0);
    MasquerObjetsOeilDecoche();
}

QString dlg_refraction::ResultatPrescription() const
{
    return m_resultPrescription;
}
QString dlg_refraction::ResultatObservation() const
{
    return m_resultObservation;
}
//---------------------------------------------------------------------------------
// Maj d'un enregistrement dans DonneesOphtaPatient
//---------------------------------------------------------------------------------
void dlg_refraction::UpdateDonneesOphtaPatient()
{
    QString UpdateDOPrequete = "UPDATE  " TBL_DONNEES_OPHTA_PATIENTS
                " SET QuelleMesure = '" + Refraction::ConvertMesure(m_mode) + "'";
    if (m_mode == Refraction::Autoref && m_modeouverture == Manuel)
    {
        double K1OD = QLocale().toDouble(ui->K1OD->text());
        double K2OD = QLocale().toDouble(ui->K2OD->text());
        bool okOD = (K1OD >= m_val->bottom() && K1OD <= m_val->top() && K2OD >= m_val->bottom() && K2OD <= m_val->top());
        if (okOD && ui->ODCheckBox->isChecked())
        {
            UpdateDOPrequete +=
                    " , K1OD = " + QString::number(QLocale().toDouble(ui->K1OD->text()),'f',2) +
                    " , K2OD = " + QString::number(QLocale().toDouble(ui->K2OD->text()),'f',2) +
                    " , AxeKOD = " + ui->AxeKOD->text();
            Datas::I()->mesurekerato->setK1OD(K1OD);
            Datas::I()->mesurekerato->setK2OD(K2OD);
            Datas::I()->mesurekerato->setaxeKOD(ui->AxeKOD->text().toInt());
        }
        double K1OG = QLocale().toDouble(ui->K1OG->text());
        double K2OG = QLocale().toDouble(ui->K2OG->text());
        bool okOG = (K1OG >= m_val->bottom() && K1OG <= m_val->top() && K2OG >= m_val->bottom() && K2OG <= m_val->top());
        if (okOG && ui->OGCheckBox->isChecked())
        {
            UpdateDOPrequete +=
                    ", K1OG = " + QString::number(QLocale().toDouble(ui->K1OG->text()),'f',2) +
                    ", K2OG = " + QString::number(QLocale().toDouble(ui->K2OG->text()),'f',2) +
                    ", AxeKOG = " + ui->AxeKOG->text();
            Datas::I()->mesurekerato->setK1OG(K1OG);
            Datas::I()->mesurekerato->setK2OG(K2OG);
            Datas::I()->mesurekerato->setaxeKOG(ui->AxeKOG->text().toInt());
        }
        if (okOD || okOG)
        {
            UpdateDOPrequete +=
                    ", OrigineK = 'M'"
                    ", DateK =  '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss") + "'";
        }
    }
    if (ui->ODCheckBox->isChecked())
    {
        UpdateDOPrequete += ", SphereOD = " + QString::number(ui->SphereOD->value());
        if (ui->CylindreOD->value() == 0.0)
            UpdateDOPrequete +=
                ", CylindreOD = 0"
                ", AxeCylindreOD = 0";
        else
            UpdateDOPrequete +=
                ", CylindreOD = " + QString::number(ui->CylindreOD->value()) +
                ", AxeCylindreOD = " + QString::number(ui->AxeCylindreOD->value());
        if (m_mode == Refraction::Acuite)
        {
            UpdateDOPrequete += ", AVLOD = '" + wdg_AVLOD->text() + "'";
            if (!ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
            {
                UpdateDOPrequete += ", AddVPOD = " + QString::number(ui->AddVPOD->value());
                UpdateDOPrequete += ", AVPOD = '" + wdg_AVPOD->text() + "'";
            }
            else
            UpdateDOPrequete += ",AddVPOD = null, AVPOD = null";
        }
        else
            UpdateDOPrequete += ", AVLOD = null, AddVPOD = null, AVPOD = null";
        UpdateDOPrequete += ", DateRefOD = '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss") + "'";
    }
    if (ui->OGCheckBox->isChecked())
    {
        UpdateDOPrequete += ", SphereOG = " + QString::number(ui->SphereOG->value());
        if (ui->CylindreOG->value() == 0.0)
            UpdateDOPrequete +=
                ", CylindreOG = 0"
                ", AxeCylindreOG = 0";
        else
            UpdateDOPrequete +=
                ", CylindreOG = " + QString::number(ui->CylindreOG->value()) +
                ", AxeCylindreOG = " + QString::number(ui->AxeCylindreOG->value());
        if (m_mode == Refraction::Acuite)
        {
            UpdateDOPrequete += ", AVLOG = '" + wdg_AVLOG->text() + "'";
            if (!ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
            {
                UpdateDOPrequete += ", AddVPOG = " + QString::number(ui->AddVPOG->value());
                UpdateDOPrequete += ", AVPOG = '" + wdg_AVPOG->text() + "'";
            }
            else
            UpdateDOPrequete += ",AddVPOG = null, AVPOG = null";
        }
        else
            UpdateDOPrequete += ", AVLOG = null, AddVPOG = null, AVPOG = null";
        UpdateDOPrequete += ", DateRefOG = '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss") + "'";
    }
    if (ui->EIPLabel->text().toInt() > 0 && ui->ODCheckBox->isChecked() && ui->OGCheckBox->isChecked())
        UpdateDOPrequete += ", PD = '" + ui->EIPLabel->text() + "'";
    UpdateDOPrequete +=  " WHERE idPat = " + QString::number(Datas::I()->patients->currentpatient()->id()) + " AND QuelleMesure = '" + Refraction::ConvertMesure(m_mode) + "'";
    //proc->Edit(UpdateDOPrequete);
    db->StandardSQL(UpdateDOPrequete, tr("Erreur de MAJ dans ")+ TBL_DONNEES_OPHTA_PATIENTS);
    Datas::I()->patients->actualiseDonneesOphtaCurrentPatient();
}

//---------------------------------------------------------------------------------------------------------
// Traitement du prefixe + ou - devant les valeurs numériques.
//---------------------------------------------------------------------------------------------------------
QString dlg_refraction::Valeur(QString StringValeur)
{
    if (StringValeur == "")
        return QLocale().toString(StringValeur.toDouble());
    if (StringValeur.toDouble() > 0 && StringValeur.at(0) != QString("+"))
        return("+" + StringValeur);
    else
        return(StringValeur);
}

//-----------------------------------------------------------------------------------------
// Affichage des données lues sur le port série du Fronto
//-----------------------------------------------------------------------------------------
void dlg_refraction::AfficheMesureFronto()
{
    RadioButtonFronto_Clicked();
    if (Datas::I()->mesurefronto == Q_NULLPTR)
    {
        UpMessageBox::Watch(this, tr("pas de données reçues du frontofocomètre"));
        return;
    }
    //A - AFFICHER LA MESURE --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (Datas::I()->mesurefronto->addVPOD() >  0 || Datas::I()->mesurefronto->addVPOG() > 0)
    {
        if (!ui->V2RadioButton->isChecked())
        {
            ui->V2RadioButton->setChecked(true);
            ui->VLRadioButton->setChecked(false);
            ui->VPRadioButton->setChecked(false);
            RegleAffichageFiche();
        }
    }
    else
    {
        if (ui->V2RadioButton->isChecked())
        {
            ui->V2RadioButton->setChecked(false);
            ui->VLRadioButton->setChecked(true);
            ui->VPRadioButton->setChecked(false);
            RegleAffichageFiche();
        }
    }
    // OEIL DROIT -----------------------------------------------------------------------------
    ui->SphereOD->setValuewithPrefix(Datas::I()->mesurefronto->sphereOD());
    ui->CylindreOD->setValuewithPrefix(Datas::I()->mesurefronto->cylindreOD());
    ui->AxeCylindreOD   ->setValue(Datas::I()->mesurefronto->axecylindreOD());
    ui->AddVPOD         ->setValue(Datas::I()->mesurefronto->addVPOD());
    // OEIL GAUCHE ---------------------------------------------------------------------------
    ui->SphereOG->setValuewithPrefix(Datas::I()->mesurefronto->sphereOG());
    ui->CylindreOG->setValuewithPrefix(Datas::I()->mesurefronto->cylindreOG());
    ui->AxeCylindreOG   ->setValue(Datas::I()->mesurefronto->axecylindreOG());
    ui->AddVPOG         ->setValue(Datas::I()->mesurefronto->addVPOG());
    ui->EIPLabel->setText(tr("Ecart interpupillaire") + " " + (Datas::I()->mesurefronto->ecartIP() > 0? QString::number(Datas::I()->mesurefronto->ecartIP()) +"mm" : tr("inconnu")));
}

//-----------------------------------------------------------------------------------------
// Affichage des données lues sur le port série de l'Autoref
//-----------------------------------------------------------------------------------------
void dlg_refraction::AfficheMesureAutoref()
{
    if (Datas::I()->mesureautoref->isdataclean() && Datas::I()->mesurekerato->isdataclean())
    {
        UpMessageBox::Watch(this, tr("pas de données reçues de l'autorefractomètre"));
        return;
    }
    RadioButtonAutoref_Clicked();

    // OEIL DROIT -----------------------------------------------------------------------------
    ui->SphereOD->setValuewithPrefix(Datas::I()->mesureautoref->sphereOD());
    ui->CylindreOD->setValuewithPrefix(Datas::I()->mesureautoref->cylindreOD());
    ui->AxeCylindreOD       ->setValue(Datas::I()->mesureautoref->axecylindreOD());
    // OEIL GAUCHE ---------------------------------------------------------------------------
    ui->SphereOG->setValuewithPrefix(Datas::I()->mesureautoref->sphereOG());
    ui->CylindreOG->setValuewithPrefix(Datas::I()->mesureautoref->cylindreOG());
    ui->AxeCylindreOG       ->setValue(Datas::I()->mesureautoref->axecylindreOG());
    ui->EIPLabel->setText(tr("Ecart interpupillaire") + " " + (Datas::I()->mesureautoref->ecartIP() > 0? QString::number(Datas::I()->mesureautoref->ecartIP()) +"mm" : tr("inconnu")));
    AfficheKerato();
}


//-----------------------------------------------------------------------------------------
// Affichage des données lues sur le port série de l'Autoref
//-----------------------------------------------------------------------------------------
void dlg_refraction::AfficheKerato()
{
    if (DataBase::I()->donneesOphtaPatient()->ismesurekerato())
    {
        // OEIL DROIT -----------------------------------------------------------------------------
        if (DataBase::I()->donneesOphtaPatient()->K1OD() >0)
        {
            ui->K1OD            ->setText(QLocale().toString(DataBase::I()->donneesOphtaPatient()->K1OD(),'f',2 ));
            ui->K2OD            ->setText(QLocale().toString(DataBase::I()->donneesOphtaPatient()->K2OD(),'f',2 ));
            ui->AxeKOD          ->setText(QString::number(DataBase::I()->donneesOphtaPatient()->axeKOD()));
        }
        else
        {
            ui->K1OD            ->clear();
            ui->K2OD            ->clear();
            ui->AxeKOD          ->clear();
        }
        // OEIL GAUCHE ---------------------------------------------------------------------------
        if (DataBase::I()->donneesOphtaPatient()->K1OG() >0)
        {
            ui->K1OG            ->setText(QLocale().toString(DataBase::I()->donneesOphtaPatient()->K1OG(),'f',2 ));
            ui->K2OG            ->setText(QLocale().toString(DataBase::I()->donneesOphtaPatient()->K2OG(),'f',2 ));
            ui->AxeKOG          ->setText(QString::number(DataBase::I()->donneesOphtaPatient()->axeKOG()));
        }
        else
        {
            ui->K1OG            ->clear();
            ui->K2OG            ->clear();
            ui->AxeKOG          ->clear();
        }
    }
}

//-----------------------------------------------------------------------------------------
// Affichage des données lues sur le port série du refracteur
//-----------------------------------------------------------------------------------------
void dlg_refraction::AfficheMesureRefracteur()
{
    MesureRefraction*  MesureRefracteur = Q_NULLPTR;
    if ( Datas::I()->mesurefinal->isdataclean() )
    {
        RadioButtonRefraction_Clicked();
        m_mode = Refraction::Acuite;
        MesureRefracteur = Datas::I()->mesureacuite;
    }
    else
    {
        RadioButtonPrescription_clicked();
        m_mode = Refraction::Prescription;
        MesureRefracteur = Datas::I()->mesurefinal;
    }
    if (MesureRefracteur == Q_NULLPTR)
    {
        UpMessageBox::Watch(this, tr("pas de données reçues du refracteur"));
        return;
    }
    QString AVLOD (""), AVLOG ("");
    // OEIL DROIT ---------------------------------------------------------------------------
    if (!MesureRefracteur->isnullLOD())
    {
        ui->SphereOD->setValuewithPrefix(MesureRefracteur->sphereOD());
        ui->CylindreOD->setValuewithPrefix(MesureRefracteur->cylindreOD());
        ui->AxeCylindreOD   ->setValue(MesureRefracteur->axecylindreOD());
        ui->AddVPOD->setValue(MesureRefracteur->addVPOD());
        AVLOD = QString::number(MesureRefracteur->avlOD().toDouble()*10) + "/10";
    }
    // OEIL GAUCHE ---------------------------------------------------------------------------
    if (!MesureRefracteur->isnullLOG())
    {
        ui->SphereOG->setValuewithPrefix(MesureRefracteur->sphereOG());
        ui->CylindreOG->setValuewithPrefix(MesureRefracteur->cylindreOG());
        ui->AxeCylindreOG   ->setValue(MesureRefracteur->axecylindreOG());
        ui->AddVPOG->setValue(MesureRefracteur->addVPOG());
        AVLOG = QString::number(MesureRefracteur->avlOG().toDouble()*10) + "/10";
    }
    AfficheKerato();
    ui->ODCheckBox->setChecked(!MesureRefracteur->isnullLOD());
    ui->OGCheckBox->setChecked(!MesureRefracteur->isnullLOG());
    switch (m_mode) {
    case Refraction::Acuite:
    {
        if (!MesureRefracteur->isnullLOD())
        {
            ui->AVLODupComboBox->setCurrentText(AVLOD);
            ui->AVPODupComboBox->setCurrentText(MesureRefracteur->avpOD());
        }
        if (!MesureRefracteur->isnullLOG())
        {
            ui->AVLOGupComboBox->setCurrentText(AVLOG);
            ui->AVPOGupComboBox->setCurrentText(MesureRefracteur->avpOG());
        }
        ui->V2RadioButton->setChecked(MesureRefracteur->addVPOD() > 0 || MesureRefracteur->addVPOG() > 0);
        ui->VPRadioButton->setChecked(false);
        ui->VLRadioButton->setChecked(MesureRefracteur->addVPOD() == 0.0 && MesureRefracteur->addVPOG() == 0.0);
        RegleAffichageFiche();
        break;
    }
    case Refraction::Prescription:
        ui->V2PrescritRadioButton->setChecked(MesureRefracteur->addVPOD() > 0 || MesureRefracteur->addVPOG() > 0);
        VPrescritRadioButton_Clicked();
        break;
    default:
        break;
    }
    ui->EIPLabel->setText(tr("Ecart interpupillaire") + " " + (MesureRefracteur->ecartIP() > 0? QString::number(MesureRefracteur->ecartIP()) +"mm" : tr("inconnu")));
    // qDebug() << "AVLOD = " + AVLOD << "AVPOD = " + MesuresRefracteur["AVPOD"].toString() << "AVLOG = " + AVLOG << "AVPOG = " + MesuresRefracteur["AVPOG"].toString();
    MesureRefracteur = Q_NULLPTR;
}
