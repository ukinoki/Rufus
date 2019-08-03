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

#include "gbl_datas.h"
#include "dlg_refraction.h"
#include "icons.h"
#include "ui_dlg_refraction.h"

dlg_refraction::dlg_refraction(Acte *acte, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_refraction)
{
    ui->setupUi(this);
    proc            = Procedures::I();
    db              = DataBase::I();
    m_currentacte   = acte;

    setWindowTitle("Refraction - " + Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom());
    setWindowIcon(Icons::icLunettes());
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    InitDivers();
    Init_variables();
    Connect_Slots();

    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionRefraction").toByteArray());
    // Recherche si Mesure en cours et affichage.
    if (proc->TypeMesureRefraction() != Procedures::None)
    {
        if (proc->TypeMesureRefraction() == Procedures::Subjectif || proc->TypeMesureRefraction() == Procedures::Final)
        {
            AfficheMesureRefracteur();
            proc->setTypeMesureRefraction(Procedures::None);
        }
        else if (proc->TypeMesureRefraction() == Procedures::Fronto)
        {
            AfficheMesureFronto();
            proc->setTypeMesureRefraction(Procedures::None);
        }
        else if (proc->TypeMesureRefraction() == Procedures::Autoref)
        {
            AfficheMesureAutoref();
            proc->setTypeMesureRefraction(Procedures::None);
        }
    }
    else
        RechercheMesureEnCours();

    ui->SphereOD->setFocus();
    ui->SphereOD->selectAll();
    ui->OupsPushButton->setIcon(Icons::icOups());
    InitEventFilters();
}

dlg_refraction::~dlg_refraction()
{
    delete ui;
}

void dlg_refraction::closeEvent(QCloseEvent *)
{
    if (!EscapeFlag)
    {
        if (FermeComment)
            accept();
        else
            reject();
        proc->gsettingsIni->setValue("PositionsFiches/PositionRefraction", saveGeometry());
    }
}

//----------------------------------------------------------------------------------
//  Connection des actions associees a chaque objet du formulaire et aux menus
//----------------------------------------------------------------------------------
void dlg_refraction::Connect_Slots()
{
    foreach (UpDoubleSpinBox* spinbox, findChildren<UpDoubleSpinBox *>())
        connect (spinbox,                           SIGNAL(valueChanged(double)),               this,     SLOT (Slot_Refraction_ValueChanged()));
    connect (ui->PorteRadioButton,                  SIGNAL(clicked()),                          this,     SLOT (Slot_PorteRadioButton_Clicked()) );
    connect (ui->AutorefRadioButton,                SIGNAL(clicked()),                          this,     SLOT (Slot_AutorefRadioButton_Clicked()) );
    connect (ui->ConvODPushButton,                  SIGNAL(clicked()),                          this,     SLOT (Slot_ConvODPushButton_Clicked()) );
    connect (ui->ConvOGPushButton,                  SIGNAL(clicked()),                          this,     SLOT (Slot_ConvOGPushButton_Clicked()) );
    connect (ui->CycloplegieCheckBox,               SIGNAL(clicked()),                          this,     SLOT (Slot_CycloplegieCheckBox_Clicked()) );

    connect (ui->DepoliODCheckBox,                  SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_DepoliCheckBox_Clicked(int)) );
    connect (ui->DepoliOGCheckBox,                  SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_DepoliCheckBox_Clicked(int)) );
    connect (ui->DeuxMonturesPrescritRadioButton,   SIGNAL(clicked()),                          this,     SLOT (Slot_DeuxMonturesPrescritradioButton_Clicked()) );
    connect (ui->ODCheckBox,                        SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_ODGCheckBox_Changed(int)) );
    connect (ui->OGCheckBox,                        SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_ODGCheckBox_Changed(int)) );

    connect (ui->ODPrescritCheckBox,                SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_PrescritCheckBox_Changed(int)));
    connect (ui->OGPrescritCheckBox,                SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_PrescritCheckBox_Changed(int)));
    connect (ui->PlanODCheckBox,                    SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_PlanCheckBox_Changed(int)));
    connect (ui->PlanOGCheckBox,                    SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_PlanCheckBox_Changed(int)));
    connect (ui->PrescriptionRadioButton,           SIGNAL(clicked()),                          this,     SLOT (Slot_PrescriptionRadionButton_clicked()));

    connect (ui->RefractionRadioButton,             SIGNAL(clicked()),                          this,     SLOT (Slot_RefractionRadioButton_Clicked()) );
    connect (ui->RyserODCheckBox,                   SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_RyserCheckBox_Clicked(int)) );
    connect (ui->RyserOGCheckBox,                   SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_RyserCheckBox_Clicked(int)) );
    connect (ui->UneMonturePrescritRadioButton,     SIGNAL(clicked()),                          this,     SLOT (Slot_UneMonturePrescritRadioButton_Clicked()) );
    connect (ui->V2RadioButton,                     SIGNAL(clicked()),                          this,     SLOT (Slot_QuelleDistance_Clicked()) );

    connect (ui->V2PrescritRadioButton,             SIGNAL(clicked()),                          this,     SLOT (Slot_VPrescritRadioButton_Clicked()) );
    connect (ui->VerresTeintesCheckBox,             SIGNAL(stateChanged(int)),                  this,     SLOT (Slot_VerresTeintesCheckBox_Changed(int)) );
    connect (ui->VLRadioButton,                     SIGNAL(clicked()),                          this,     SLOT (Slot_QuelleDistance_Clicked()) );
    connect (ui->VLPrescritRadioButton,             SIGNAL(clicked()),                          this,     SLOT (Slot_VPrescritRadioButton_Clicked()) );
    connect (ui->VPRadioButton,                     SIGNAL(clicked()),                          this,     SLOT (Slot_QuelleDistance_Clicked()) );
    connect (ui->VPPrescritRadioButton,             SIGNAL(clicked()),                          this,     SLOT (Slot_VPrescritRadioButton_Clicked()) );


    connect (ui->AxeCylindreOD,                     SIGNAL(valueChanged(int)),                  this,     SLOT (Slot_Refraction_ValueChanged()) );
    connect (ui->AxeCylindreOG,                     SIGNAL(valueChanged(int)),                  this,     SLOT (Slot_Refraction_ValueChanged()) );
    connect (ui->BasePrismeOD,                      SIGNAL(valueChanged(int)),                  this,     SLOT (Slot_BasePrisme_ValueChanged()) );
    connect (ui->BasePrismeOG,                      SIGNAL(valueChanged(int)),                  this,     SLOT (Slot_BasePrisme_ValueChanged()) );

    connect (ui->BasePrismeTextODComboBox,          SIGNAL(currentIndexChanged(int)),           this,     SLOT (Slot_BasePrismeTextODComboBox_Changed(int)) );
    connect (ui->BasePrismeTextOGComboBox,          SIGNAL(currentIndexChanged(int)),           this,     SLOT (Slot_BasePrismeTextOGComboBox_Changed(int)) );
    connect (ui->CommentairePrescriptionTextEdit,   SIGNAL(textChanged()),                      this,     SLOT (Slot_CommentairePrescriptionTextEdit_Changed() ));  // 01.07.2014

    connect (ui->K1OD,                              SIGNAL(editingFinished()),                  this,     SLOT (Slot_Controle_K()) );
    connect (ui->K1OG,                              SIGNAL(editingFinished()),                  this,     SLOT (Slot_Controle_K()) );
    connect (ui->K2OD,                              SIGNAL(editingFinished()),                  this,     SLOT (Slot_Controle_K()) );

    connect (ui->K2OG,                              SIGNAL(editingFinished()),                  this,     SLOT (Slot_Controle_K()) );
    connect (ui->PressonODCheckBox,                 SIGNAL(clicked(bool)),                      this,     SLOT (Slot_PressonCheckBox_Changed()) );
    connect (ui->PressonOGCheckBox,                 SIGNAL(clicked(bool)),                      this,     SLOT (Slot_PressonCheckBox_Changed()) );

    connect (ui->RyserSpinBox,                      SIGNAL(valueChanged(int)),                  this,     SLOT (Slot_Refraction_ValueChanged()) );		// 28.06.2014

    connect (ui->AnnulPushButton,                   SIGNAL(clicked()),                          this,     SLOT (Slot_AnnulPushButton_Clicked()));
    connect (ui->AppelCommentPushButton,            SIGNAL(clicked()),                          this,     SLOT (Slot_Commentaires()));
    connect (ui->DetailsPushButton,                 SIGNAL(clicked()),                          this,     SLOT (Slot_Detail_Clicked()) );

    connect (ui->OupsPushButton,                    SIGNAL(clicked()),                          this,     SLOT (Slot_OupsButtonClicked()));
    connect (ui->ResumePushButton,                  SIGNAL(clicked()),                          this,     SLOT (Slot_ResumePushButton_Clicked()));
    connect (ui->OKPushButton,                      SIGNAL(clicked()),                          this,     SLOT (Slot_OKPushButton_Clicked()));
    connect (ui->ReprendrePushButton,               SIGNAL(clicked()),                          this,     SLOT (Slot_ReprendreButtonClicked()));

    if (proc->PortFronto()!=Q_NULLPTR || proc->PortAutoref()!=Q_NULLPTR || proc->PortRefracteur()!=Q_NULLPTR)
        connect (proc,                              SIGNAL(NouvMesureRefraction()),             this,     SLOT(Slot_NouvMesureRefraction()));
}

//--------------------------------------------------------------------------------
// Tous les Slots
//--------------------------------------------------------------------------------

//1. Les RadioButton, checkBox, combo...etc...--------------------------------------------------------------------------------
void dlg_refraction::Slot_AutorefRadioButton_Clicked()
{
    gMode = Refraction::Autoref;
    RegleAffichageFiche();
}
void dlg_refraction::Slot_CycloplegieCheckBox_Clicked()
{
    if (ui->RefractionRadioButton->isChecked())
        RegleAffichageFiche();
}
void dlg_refraction::Slot_PorteRadioButton_Clicked()
{
    gMode = Refraction::Fronto;
    RegleAffichageFiche();
}
void dlg_refraction::Slot_PressonCheckBox_Changed()
{
    if (gMode == Refraction::Prescription) ResumePrescription();
}

void dlg_refraction::Slot_QuelleDistance_Clicked()
{
    RegleAffichageFiche();
}

void dlg_refraction::Slot_RefractionRadioButton_Clicked()
{
    gMode = Refraction::Acuite;
    if(!ui->CycloplegieCheckBox->isChecked())   ui->V2RadioButton->setChecked(true);
    RegleAffichageFiche();
}

void dlg_refraction::Slot_BasePrismeTextODComboBox_Changed(int index)
{
    switch (index) {
        case 0 :    ui->BasePrismeOD->setValue(0);      break;
        case 1 :    ui->BasePrismeOD->setValue(90);     break;
        case 2 :    ui->BasePrismeOD->setValue(180);    break;
        case 3 :    ui->BasePrismeOD->setValue(270);    break;
        default:    break;
        }
}
void dlg_refraction::Slot_BasePrismeTextOGComboBox_Changed(int index)
{
    switch (index) {
        case 0 :    ui->BasePrismeOG->setValue(180);     break;
        case 1 :    ui->BasePrismeOG->setValue(90);      break;
        case 2 :    ui->BasePrismeOG->setValue(0);       break;
        case 3 :    ui->BasePrismeOG->setValue(270);     break;
        default:    break;
        }
}

void dlg_refraction::Slot_DepoliCheckBox_Clicked(int etat)
{
    QCheckBox* check = qobject_cast<QCheckBox *>(sender());
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
    if (gMode == Refraction::Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
    check = Q_NULLPTR;
    delete check;
}

void dlg_refraction::Slot_DeuxMonturesPrescritradioButton_Clicked()
{
    if (ui->UneMonturePrescritRadioButton->isChecked() && ui->DeuxMonturesPrescritRadioButton->isChecked())
        ui->UneMonturePrescritRadioButton->setChecked(false);
    ResumePrescription();
}

void dlg_refraction::Slot_ODGCheckBox_Changed(int etat)
{
    QCheckBox* check = qobject_cast<QCheckBox *>(sender());
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
    check = Q_NULLPTR;
    delete check;
}

void dlg_refraction::Slot_PrescritCheckBox_Changed(int etat)
{   // pas 2 yeux decoches en meme temps
    QCheckBox* check = qobject_cast<QCheckBox *>(sender());
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
    check = Q_NULLPTR;
    delete check;
}

void dlg_refraction::Slot_PlanCheckBox_Changed(int etat)
{
    if (sender() == ui->PlanODCheckBox)
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
    if (sender() == ui->PlanOGCheckBox)
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
    if (gMode == Refraction::Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
}

void dlg_refraction::Slot_RyserCheckBox_Clicked(int etat)
{
    QCheckBox* check = qobject_cast<QCheckBox *>(sender());
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

    if (gMode == Refraction::Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
    check = Q_NULLPTR;
    delete check;
}

void dlg_refraction::Slot_UneMonturePrescritRadioButton_Clicked()
{
    if (ui->UneMonturePrescritRadioButton->isChecked() && ui->DeuxMonturesPrescritRadioButton->isChecked())
        ui->DeuxMonturesPrescritRadioButton->setChecked(false);
    ResumePrescription();
}

void dlg_refraction::Slot_VerresTeintesCheckBox_Changed(int )
{
    ResumePrescription();
}

void dlg_refraction::Slot_VPrescritRadioButton_Clicked()
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
void dlg_refraction::Slot_BasePrisme_ValueChanged()
{
    QSpinBox *box = static_cast<QSpinBox *>(sender());
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
    if (gMode == Refraction::Prescription) ResumePrescription();
    box = Q_NULLPTR;
    delete box;
}

void dlg_refraction::Slot_CommentairePrescriptionTextEdit_Changed()    // 01.07.2014
{
    ResumePrescription();
}

void dlg_refraction::Slot_Controle_K()
{
    static_cast<QLineEdit *>(sender())->setText(QLocale().toString(QLocale().toDouble(static_cast<QLineEdit *>(sender())->text())));
}

void dlg_refraction::Slot_Refraction_ValueChanged()
{
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
    if (gMode == Refraction::Prescription) ResumePrescription();
}

//3. Les pushButton ----------------------------------------------------------------------------------------------
void dlg_refraction::Slot_AnnulPushButton_Clicked()
{
    FermeFiche(Annul);
}

void dlg_refraction::Slot_Commentaires()
{
    Dlg_Comments    = new dlg_commentaires();
    if (Dlg_Comments->exec() > 0)
    {
        m_commentaire =  Dlg_Comments->Commentaire();
        ResumePrescription();
        m_commentaireresume = Dlg_Comments->CommentaireResume();
    }
    Dlg_Comments->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_Comments;
}

void dlg_refraction::Slot_ConvODPushButton_Clicked()
{
    if ((ui->SphereOD->value() + ui->CylindreOD->value()) > 20)
        {UpMessageBox::Watch(this,tr("Réfraction"), tr("Conversion refusée !"));
         return;
        }
    Init_Value_DoubleSpin( ui->SphereOD, ui->SphereOD->value() + ui->CylindreOD->value());
    Init_Value_DoubleSpin( ui->CylindreOD, ui->CylindreOD->value() * -1);
    if (ui->AxeCylindreOD->value() >= 90)
         ui->AxeCylindreOD->setValue(ui->AxeCylindreOD->value() - 90);
    else
        ui->AxeCylindreOD->setValue(ui->AxeCylindreOD->value() + 90);
}

void dlg_refraction::Slot_ConvOGPushButton_Clicked()
{
    if ((ui->SphereOG->value() + ui->CylindreOG->value()) > 20)
        {UpMessageBox::Watch(this,tr("Réfraction"), tr("Conversion refusée !"));
         return;
        }
    Init_Value_DoubleSpin(ui->SphereOG, ui->SphereOG->value() + ui->CylindreOG->value());
    Init_Value_DoubleSpin(ui->CylindreOG, ui->CylindreOG->value() * -1);
    if (ui->AxeCylindreOG->value() >= 90)
        ui->AxeCylindreOG->setValue(ui->AxeCylindreOG->value() - 90);
    else
        ui->AxeCylindreOG->setValue(ui->AxeCylindreOG->value() + 90);
}

void dlg_refraction::Slot_Detail_Clicked()
{
    if (gAfficheDetail &&
        (ui->PrismeOD->value() != 0.0       || ui->PrismeOG->value() != 0.0     ||
        ui->RyserODCheckBox->isChecked()    || ui->RyserOGCheckBox->isChecked() ||
        ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()  ||
        ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked()))
        return;
    else
        gAfficheDetail = !gAfficheDetail;
    AfficherDetail(gAfficheDetail);
    if (gMode == Refraction::Fronto)
    {
        if (gAfficheDetail)
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    }
    if (gMode == Refraction::Prescription)
    {
        if (gAfficheDetail)
            setFixedSize(width(), HAUTEUR_AVEC_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_AVEC_ORDONNANCE_SANS_DETAIL);
    }
}

//----------------------------------------------------------------------------------
// OKPushButton
//----------------------------------------------------------------------------------
void dlg_refraction::Slot_OKPushButton_Clicked()
{
    focusNextChild();
    int IdRefract;
    gFlagBugValidEnter = 0;

    UpDoubleSpinBox *dblSpin = dynamic_cast<UpDoubleSpinBox *>(focusWidget());
    if (dblSpin)
    {
        int a = int(dblSpin->value()/dblSpin->singleStep());
        int b = int(dblSpin->value()*10);
        if (a != int(dblSpin->value()/dblSpin->singleStep()))
            if (((dblSpin->singleStep() == 0.25) && ((abs(b)%10 != 2 && abs(b)%10 != 7) || b != int(dblSpin->value()*10)))
               || dblSpin->singleStep() == 0.50)
            {
                QSound::play(NOM_ALARME);
                dblSpin->setFocus();
                dblSpin->selectAll();
                return;
            }
    }
    dblSpin = Q_NULLPTR;
    delete dblSpin;

    if (!ControleCoherence())        return;

    if (gMode == Refraction::Fronto)
    {
        // On vérifie dans Refractions s'il existe un enregistrement identique au meme jour pour ne pas surcharger la table
        IdRefract = (LectureMesure(Aujourdhui, Refraction::Fronto, NoDilatation, 0, false, CalculFormule_OD(), CalculFormule_OG()));
        if (IdRefract == 0) // il n'y en a pas - on suit la procédure normale
            InscriptRefraction();
        FermeFiche(OK);
    }

    else if (gMode == Refraction::Autoref || gMode == Refraction::Acuite)
    {
        // On vérifie dans Refractions s'il existe un enregistrement identique et si oui, on l'écrase
        Cycloplegie dilat = (ui->CycloplegieCheckBox->isChecked()? Dilatation : NoDilatation);
        int IDMesure = LectureMesure(Aujourdhui, ConvertMesure(QuelleMesure()), dilat, 0, false);
        if (IDMesure > 0)
            // suppression de la mesure dans table Refraction
            DetruireLaMesure(Datas::I()->refractions->getById(IDMesure));
        InscriptRefraction();
        FermeFiche(OK);
    }
    else if (gMode == Refraction::Prescription)
        FermeFiche(Imprime);
}

void dlg_refraction::Slot_OupsButtonClicked()
{
    OuvrirListeMesures("SUPPR");
}

void dlg_refraction::Slot_PrescriptionRadionButton_clicked()
{
    gMode = Refraction::Prescription;
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
    RegleAffichageFiche();
}

void dlg_refraction::Slot_NouvMesureRefraction()
{
    if (proc->TypeMesureRefraction() == Procedures::Subjectif || proc->TypeMesureRefraction() == Procedures::Final)
        AfficheMesureRefracteur();
    if (proc->TypeMesureRefraction() == Procedures::Fronto)
        AfficheMesureFronto();
    if (proc->TypeMesureRefraction() == Procedures::Autoref)
        AfficheMesureAutoref();
    proc->setTypeMesureRefraction(Procedures::None);
}

void dlg_refraction::Slot_ReprendreButtonClicked()
{
    OuvrirListeMesures("RECUP");
}
void dlg_refraction::Slot_ResumePushButton_Clicked()
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
        gFlagBugValidEnter = 0;
        if (keyEvent->key() == Qt::Key_Escape)
        {
            if (obj->inherits("UpLineEdit"))
            {
                UpLineEdit* objUpLine = static_cast<UpLineEdit*>(obj);
                objUpLine->setText(objUpLine->getValeurAvant());
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
                Slot_OKPushButton_Clicked();
                return true;
            }
            gFlagBugValidEnter = 1; // on évite ainsi le second appel à ValidVerrres qui va être émis pas la touche flèche simulée
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
                if (gMode == Refraction::Fronto && ui->PrismeGroupBox->isVisible())                                   return DeplaceVers(ui->PrismeGroupBox,"Fin");
                if (gMode == Refraction::Fronto || (gMode == Refraction::Acuite && ui->QuelleDistanceGroupBox->isVisible()))
                                                                                                                      return DeplaceVers(ui->QuelleDistanceGroupBox);
                if (gMode == Refraction::Acuite && !ui->QuelleDistanceGroupBox->isVisible())                          return DeplaceVers(ui->QuelleMesureGroupBox);
                if (gMode == Refraction::Autoref)                                                                     return DeplaceVers(ui->KeratometrieGroupBox,"Fin");
                if (gMode == Refraction::Prescription)                                                                return DeplaceVers(ui->QuelleMontureGroupBox);
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
                    if (gMode == Refraction::Fronto) return DeplaceVers(ui->QuelleDistanceGroupBox);
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
                if (gMode == Refraction::Fronto || (gMode == Refraction:: Acuite && ui->QuelleDistanceGroupBox->isVisible()))  return DeplaceVers(ui->QuelleDistanceGroupBox);
                if (gMode == Refraction::Acuite && !ui->QuelleDistanceGroupBox->isVisible())                                   return DeplaceVers(ui->MesureGroupBox,"Debut");
                if (gMode == Refraction::Autoref)                                                                              return DeplaceVers(ui->KeratometrieGroupBox,"Debut");
                if (gMode == Refraction::Prescription)
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
        Slot_AnnulPushButton_Clicked();
        break;
    }
    case Qt::Key_Escape:
    {
        EscapeFlag = false;
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
    gMode                   = Refraction::Fronto;
    gAfficheDetail          = false;

    ui->DateDateEdit        ->setDate(QDate::currentDate());
    m_commentaire           = CommentaireObligatoire();
    m_commentaireresume     = "";
    EscapeFlag              = true;
    gDioptrAstOD            = 0;
    gDioptrAstOG            = 0;
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
    PrefixePlus(ui->SphereOG);
    PrefixePlus(ui->CylindreOG);
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
    if (gMode == Refraction::Fronto || gMode == Refraction::Prescription)
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
    if (gMode == Refraction::Prescription && ui->VPPrescritRadioButton->isChecked())
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
    if (gMode == Refraction::Acuite)
    {
        if (ui->VLRadioButton->isChecked())
            ResultatOG = ResultatOGVL + " " + AVLOG->text() + "" ;
        else
            ResultatOG = ResultatOGVL + " " + AVLOG->text() + " P" + AVPOG->text() + " " + ResultatOGVP ;
        return ResultatOG ;
    }
    if (gMode == Refraction::Prescription)
    {
        if (ui->VLPrescritRadioButton->isChecked())
            return ResultatOGVL + " " + OGPrisme ;
        return ResultatOG + " " + OGPrisme ;
    }
    if (gMode == Refraction::Autoref)
        return ResultatOGVL;
    if (gMode == Refraction::Fronto)
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
    PrefixePlus(ui->SphereOD);
    PrefixePlus(ui->CylindreOD);
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
    if (gMode == Refraction::Fronto || gMode == Refraction::Prescription)
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
    if (gMode == Refraction::Prescription && ui->VPPrescritRadioButton->isChecked())
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
    if (gMode == Refraction::Acuite)
    {
        if (ui->VLRadioButton->isChecked())
            ResultatOD = ResultatODVL + " " + AVLOD->text() + "" ;
        else
            ResultatOD = ResultatODVL + " " + AVLOD->text() + " P" + AVPOD->text() + " " + ResultatODVP ;
        return ResultatOD ;
    }
    if (gMode == Refraction::Prescription)
    {
        if (ui->VLPrescritRadioButton->isChecked())
            return ResultatODVL + " " + ODPrisme ;
        return ResultatOD + " " + ODPrisme ;
    }
    if (gMode == Refraction::Autoref)
        return ResultatODVL;
    if (gMode == Refraction::Fronto)
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

//------------------------------------------------------------------------------------------
// Controle et reformatage des champs saisis
//------------------------------------------------------------------------------------------
double dlg_refraction::ConvDouble(QString textdouble)
{    QString ntextdouble = textdouble.replace(",",".");
     return (ntextdouble.toDouble());
}

Refraction::Mesure dlg_refraction::ConvertMesure(QString Mesure)
{
    if (Mesure == "P") return Refraction::Fronto;
    if (Mesure == "A") return Refraction::Autoref;
    if (Mesure == "O") return Refraction::Prescription;
    if (Mesure == "R") return Refraction::Acuite;
    return  Refraction::NoMesure;
}

QString dlg_refraction::ConvertMesure( Refraction::Mesure Mesure)
{
    switch (Mesure) {
    case Refraction::Fronto:       return "P";
    case Refraction::Autoref:      return "A";
    case Refraction::Prescription: return "0";
    case Refraction::Acuite:       return "R";
    default: return "";
    }
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
        Slot_QuelleDistance_Clicked();
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
                else if (AVPOG->isVisible())    AVPOG->setFocus();
                else if (AVLOG->isVisible())    AVLOG->setFocus();
                else                                ui->AxeCylindreOG->setFocus();
                return true;
            }
            else
            {
                if (ui->AddVPOD->isVisible())       ui->AddVPOD->setFocus();
                else if (AVPOD->isVisible())    AVPOD->setFocus();
                else if (AVLOD->isVisible())    AVLOD->setFocus();
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
    EscapeFlag = false;
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
        disconnect (ui->OKPushButton,   SIGNAL(clicked()),  this,   SLOT (Slot_OKPushButton_Clicked()));
        if (Imprimer_Ordonnance())
        {
            ResumeObservation();
            if (LectureMesure(Aujourdhui, Refraction::Prescription, NoDilatation, 0, false, CalculFormule_OD(), CalculFormule_OG()) == 0)
                InscriptRefraction();
        }
        else
        {
            connect (ui->OKPushButton,  SIGNAL(clicked()),  this,   SLOT (Slot_OKPushButton_Clicked()));
            return;
        }
    }
    QList<QDialog *> ListDialog = this->findChildren<QDialog *>();
    for (int n = 0; n <  ListDialog.size(); n++)
        ListDialog.at(n)->close();
    FermeComment = (mode!=Annul);
    close();
}

int     dlg_refraction::getidRefraction()
{
    return gidRefraction;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Imprimer une ordonnance ------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------*/
bool    dlg_refraction::Imprimer_Ordonnance()
{
    QString Corps, Entete, Pied;
    bool AvecDupli   = (proc->gsettingsIni->value("Param_Imprimante/OrdoAvecDupli").toString() == "YES");
    bool AvecPrevisu = proc->ApercuAvantImpression();
    bool AvecNumPage = false;

    //création de l'entête
    User *userEntete = Datas::I()->users->getById(Datas::I()->users->userconnected()->idsuperviseur(), Item::LoadDetails);
    Entete = proc->ImpressionEntete(ui->DateDateEdit->date(), userEntete).value("Norm");
    if (Entete == "") return false;
    Entete.replace("{{TITRE1}}"            , "");
    Entete.replace("{{TITRE}}"             , "");
    Entete.replace("{{PRENOM PATIENT}}"    , Datas::I()->patients->currentpatient()->prenom());
    Entete.replace("{{NOM PATIENT}}"       , Datas::I()->patients->currentpatient()->nom().toUpper());
    Entete.replace("{{DDN}}"               , "");

    // création du pied
    Pied = proc->ImpressionPied(userEntete, true);
    if (Pied == "") return false;

    // creation du corps de l'ordonnance
    Corps = proc->ImpressionCorps(ui->ResumePrescriptionTextEdit->toPlainText());
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
        listbinds[CP_IDUSER_IMPRESSIONS] =           Datas::I()->users->userconnected()->id();
        listbinds[CP_IDPAT_IMPRESSIONS] =            Datas::I()->patients->currentpatient()->id();
        listbinds[CP_TYPEDOC_IMPRESSIONS] =          PRESCRIPTION;
        listbinds[CP_SOUSTYPEDOC_IMPRESSIONS] =      CORRECTION;
        listbinds[CP_TITRE_IMPRESSIONS] =            "Prescription correction";
        listbinds[CP_TEXTENTETE_IMPRESSIONS] =       Entete;
        listbinds[CP_TEXTCORPS_IMPRESSIONS] =        Corps;
        listbinds[CP_TEXTORIGINE_IMPRESSIONS] =      ui->ResumePrescriptionTextEdit->toPlainText();
        listbinds[CP_TEXTPIED_IMPRESSIONS] =         Pied;
        listbinds[CP_DATE_IMPRESSIONS] =             ui->DateDateEdit->date().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss");
        listbinds[CP_IDEMETTEUR_IMPRESSIONS] =       Datas::I()->users->userconnected()->id();
        listbinds[CP_ALD_IMPRESSIONS] =              QVariant(QVariant::String);
        listbinds[CP_EMISORRECU_IMPRESSIONS] =       "0";
        listbinds[CP_FORMATDOC_IMPRESSIONS] =        PRESCRIPTIONLUNETTES;
        listbinds[CP_IDLIEU_IMPRESSIONS] =           Datas::I()->users->userconnected()->idsitedetravail();
        DocExterne * doc = DocsExternes::CreationDocumentExterne(listbinds);
        if (doc != Q_NULLPTR)
            delete doc;
    }
    delete Etat_textEdit;
    userEntete = Q_NULLPTR;
    delete userEntete;
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
    AVPOD = new UpLineEdit(ui->AVPODupComboBox);
    AVPOG = new UpLineEdit(ui->AVPOGupComboBox);
    AVLOD = new UpLineEdit(ui->AVLODupComboBox);
    AVLOG = new UpLineEdit(ui->AVLOGupComboBox);
    QString styl =
    "UpLineEdit {background-color:white; border-style: none;}"
    "UpLineEdit:focus {border-style:none;}";
    AVPOD->setStyleSheet(styl);
    AVPOG->setStyleSheet(styl);
    AVLOD->setStyleSheet(styl);
    AVLOG->setStyleSheet(styl);
    ui->AVPODupComboBox->setStyleSheet("border-left: 0px;");
    ui->AVPOGupComboBox->setStyleSheet("border-left: 0px;");
    ui->label_POD->setStyleSheet("border-left: 1px solid #adadad; border-top: 1px solid #adadad; border-bottom: 1px solid #adadad; border-right: 0px;  background-color: #fff;");
    ui->label_POG->setStyleSheet("border-left: 1px solid #adadad; border-top: 1px solid #adadad; border-bottom: 1px solid #adadad; border-right: 0px;  background-color: #fff;");

    QRegExp reg = QRegExp("(1\\.5|1,5|2|3|4|5|6|8|10|14|28|<28)");
    AVPOD->setValidator(new QRegExpValidator(reg,this));
    AVPOG->setValidator(new QRegExpValidator(reg,this));

    reg = QRegExp(tr("(zeroPL|PLnonO|PLO|VBLM|CLD 50cm|CLD 1m|0,5/10|0\\.5/10|1/10|1,6/10|1\\.6/10|2/10|2\\.5/10|2,5/10|3/10|4/10|5/10|6/10|6\\.3/10|6,3/10|7/10|8/10|9/10|10/10|12/10|16/10)"));
    AVLOD->setValidator(new QRegExpValidator(reg,this));
    AVLOG->setValidator(new QRegExpValidator(reg,this));

    ui->AVPODupComboBox->setLineEdit(AVPOD);
    ui->AVPOGupComboBox->setLineEdit(AVPOG);
    ui->AVLODupComboBox->setLineEdit(AVLOD);
    ui->AVLOGupComboBox->setLineEdit(AVLOG);

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

    ui->AVPODupComboBox->setCurrentIndex(ui->AVPODupComboBox->getIndexParDefaut());
    ui->AVPOGupComboBox->setCurrentIndex(ui->AVPOGupComboBox->getIndexParDefaut());
    ui->AVLODupComboBox->setCurrentIndex(ui->AVLODupComboBox->getIndexParDefaut());
    ui->AVLOGupComboBox->setCurrentIndex(ui->AVLOGupComboBox->getIndexParDefaut());

    ui->PorteRadioButton->setChecked(true);
    ui->DateDateEdit->setDate(QDate::currentDate());

    upDoubleValidator *val = new upDoubleValidator(6,9.50,2,this);
    val->setNotation(QDoubleValidator::StandardNotation);
    ui->K1OD->setValidator(val);
    ui->K1OG->setValidator(val);
    ui->K2OD->setValidator(val);
    ui->K2OG->setValidator(val);

    ui->AxeKOD->setValidator(new QIntValidator(0,180, this));
    ui->AxeKOG->setValidator(new QIntValidator(0,180, this));

    ui->AxeCylindreOD->setSuffix("°");
    ui->AxeCylindreOG->setSuffix("°");
    ui->frame_Detail->setVisible(false);
    ui->frame_Prescription->setVisible(false);

    PrefixePlus(ui->SphereOD);
    PrefixePlus(ui->SphereOG);
    PrefixePlus(ui->CylindreOD);
    PrefixePlus(ui->CylindreOG);
    PrefixePlus(ui->AddVPOD);
    PrefixePlus(ui->AddVPOG);
}

//-----------------------------------------------------------------------------------------
// Intialise la valeur du SpinBox et ajoute le préfixe + ou -
//-----------------------------------------------------------------------------------------
void dlg_refraction::Init_Value_DoubleSpin(QDoubleSpinBox *DoubleSpinBox, double ValeurDouble)
{
    DoubleSpinBox->setValue(ValeurDouble);
    PrefixePlus(DoubleSpinBox);
}

//---------------------------------------------------------------------------------
// Creation des mesure en fonction de type de mesure ??
//---------------------------------------------------------------------------------
void dlg_refraction::InscriptRefraction()
{
    bool a = InsertRefraction();
    QString req = "select max(idrefraction) from " TBL_REFRACTIONS " where idpat = " + QString::number(Datas::I()->patients->currentpatient()->id());
    QVariantList refractdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
    if (ok && refractdata.size()>0)
        gidRefraction = refractdata.at(0).toInt();
    if ((gMode == Refraction::Autoref || gMode == Refraction::Acuite) && a)
        MajDonneesOphtaPatient();
    if (gMode == Refraction::Prescription && a)
    {
        bool ok;
        req = "select max(idimpression) from " TBL_DOCSEXTERNES " where idpat = " + QString::number(Datas::I()->patients->currentpatient()->id());
        QVariantList imprdata = db->getFirstRecordFromStandardSelectSQL(req, ok);
        if (ok && imprdata.size()>0)
        {
            int idimp = imprdata.at(0).toInt();
            db->StandardSQL("update " TBL_DOCSEXTERNES " set idRefraction = " + QString::number(gidRefraction) + " where idimpression = " + QString::number(idimp));
        }
    }
}

QString dlg_refraction::CommentaireObligatoire()
{
    QString rep ("");
    bool ok;
    QString req = "SELECT TextComment"
                  " FROM "  TBL_COMMENTAIRESLUNETTES
                  " WHERE idUser = " + QString::number(Datas::I()->users->userconnected()->id()) +
                  " and ParDefautComment = 1"
                  " ORDER BY ResumeComment";
    QList<QVariantList> commentlist = db->StandardSelectSQL(req,ok);
    if (ok && commentlist.size()>0)
        foreach (const QVariant &comment, commentlist)
        {
            rep += comment.toString();
            if (comment != commentlist.last())
                rep += "\n";
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
   listbinds["QuelleMesure"]    = QuelleMesure();
   listbinds["QuelleDistance"]  = QuelleDistance();
   if ((ConvDouble(ui->K1OD->text()) > 0 || ConvDouble(ui->K2OD->text()) > 0) && ui->ODCheckBox->isChecked()) // 16-07-2014
   {
       listbinds["K1OD"]        = QLocale().toDouble(ui->K1OD->text());
       listbinds["K1OD"]        = QLocale().toDouble(ui->K2OD->text());
       listbinds["AxeKOD"]      = ui->AxeKOD->text();
   }
   if ((ConvDouble(ui->K1OG->text()) > 0 || ConvDouble(ui->K2OG->text()) > 0) && ui->OGCheckBox->isChecked())  // 16-07-2014
   {
       listbinds["K1OG"]        = QLocale().toDouble(ui->K1OG->text());
       listbinds["K1OG"]        = QLocale().toDouble(ui->K2OG->text());
       listbinds["AxeKOG"]      = ui->AxeKOG->text();
   }
   if (ConvDouble(ui->K1OD->text()) > 0 || ConvDouble(ui->K2OD->text()) > 0 || // 16-07-2014
       ConvDouble(ui->K1OG->text()) > 0 || ConvDouble(ui->K2OG->text()) > 0)   // 16-07-2014
   {
       listbinds["OrigineK"]     = QuelleMesure();
       listbinds["DateK"]        = ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
   }
   if (ui->ODCheckBox->isChecked())
   {
       listbinds["SphereOD"]     = ui->SphereOD->value();
       if (ui->CylindreOD->value() != 0.0)
       {
           listbinds["CylindreOD"]      = ui->CylindreOD->value();
           listbinds["AxeCylindreOD"]   = ui->AxeCylindreOD->value();
       }
       if (gMode == Refraction::Acuite)
           listbinds["AVLOD"]   = AVLOD->text();
       if (ui->AddVPOD->value() > 0  && gMode == Refraction::Acuite)
           listbinds["AddVPOD"] = ui->AddVPOD->value();
       if (gMode == Refraction::Acuite  && !ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
           listbinds["AVPOD"]   = AVPOD->text();
       listbinds["DateRefOD"]   = ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
   }
   if (ui->OGCheckBox->isChecked())
   {
       listbinds["SphereOG"]     = ui->SphereOG->value();
       if (ui->CylindreOG->value() != 0.0)
       {
           listbinds["CylindreOG"]      = ui->CylindreOG->value();
           listbinds["AxeCylindreOG"]   = ui->AxeCylindreOG->value();
       }
       if (gMode == Refraction::Acuite)
           listbinds["AVLOG"]   = AVLOG->text();
       if (ui->AddVPOG->value() > 0  && gMode == Refraction::Acuite)
           listbinds["AddVPOG"] = ui->AddVPOG->value();
       if (gMode == Refraction::Acuite  && !ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
           listbinds["AVPOG"]   = AVPOG->text();
       listbinds["DateRefOG"]   = ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
   }
   db->InsertSQLByBinds(TBL_DONNEES_OPHTA_PATIENTS, listbinds, tr("Erreur de MAJ dans ")+ TBL_DONNEES_OPHTA_PATIENTS);
}

//---------------------------------------------------------------------------------
// Enregistre la mesure qui vient d'être validée dans la table Refractions
//---------------------------------------------------------------------------------
bool dlg_refraction::InsertRefraction()
{
    QHash<QString, QVariant> listbinds;
    listbinds["idPat"]              = Datas::I()->patients->currentpatient()->id();
    listbinds["idActe"]             = m_currentacte->id();
    listbinds["DateRefraction"]     = ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    listbinds["QuelleMesure"]       = QuelleMesure();
    if(QuelleMesure() != "A")
        listbinds["QuelleDistance"]  = QuelleDistance();
    if(QuelleMesure() == "A" || QuelleMesure() == "R")
        listbinds["Cycloplegie"]    = ui->CycloplegieCheckBox->isChecked()? 1 : 0;

    listbinds["ODcoche"]            = ui->ODCheckBox->isChecked()? 1 : 0;
    if(ui->ODCheckBox->isChecked())
    {
        listbinds["SphereOD"]       = ui->SphereOD->value();
        if (ui->CylindreOD->value() != 0.0)
        {
            listbinds["CylindreOD"]      = ui->CylindreOD->value();
            listbinds["AxeCylindreOD"]   = ui->AxeCylindreOD->value();
        }
    }
    if(AVLOD->isVisible())
        listbinds["AVLOD"]   = AVLOD->text();
    if(ui->AddVPOD->isVisible())
        listbinds["AddVPOD"] = ui->AddVPOD->value();
    if(AVPOD->isVisible())
        listbinds["AVPOD"]   = AVPOD->text();
    if(ui->PrismeOD->isVisible() && ui->PrismeOD->text().toDouble() > 0)
    {
        listbinds["PrismeOD"]           = ui->PrismeOD->value();
        listbinds["BasePrismeOD"]       = ui->BasePrismeOD->value();
        listbinds["BasePrismeTextOD"]   = ui->BasePrismeTextODComboBox->currentText();
        listbinds["PressOnOD"]          = ui->PressonODCheckBox->isChecked()? 1 : 0;
    }
    listbinds["DepoliOD"]               = ui->DepoliODCheckBox->isChecked()? 1 : 0;
    listbinds["PlanOD"]                 = ui->PlanODCheckBox->isChecked()? 1 : 0;
    if(ui->RyserODCheckBox->isChecked())
        listbinds["RyserOD"]            = ui->RyserSpinBox->value();
    listbinds["FormuleOD"]              = CalculFormule_OD();

    listbinds["OGcoche"]            = ui->OGCheckBox->isChecked()? 1 : 0;
    if(ui->OGCheckBox->isChecked())
    {
        listbinds["SphereOG"]       = ui->SphereOG->value();
        if (ui->CylindreOG->value() != 0.0)
        {
            listbinds["CylindreOG"]      = ui->CylindreOG->value();
            listbinds["AxeCylindreOG"]   = ui->AxeCylindreOG->value();
        }
    }
    if(AVLOG->isVisible())
        listbinds["AVLOG"]   = AVLOG->text();
    if(ui->AddVPOG->isVisible())
        listbinds["AddVPOG"] = ui->AddVPOG->value();
    if(AVPOG->isVisible())
        listbinds["AVPOG"]   = AVPOG->text();
    if(ui->PrismeOG->isVisible() && ui->PrismeOG->text().toDouble() > 0)
    {
        listbinds["PrismeOG"]           = ui->PrismeOG->value();
        listbinds["BasePrismeOG"]       = ui->BasePrismeOG->value();
        listbinds["BasePrismeTextOG"]   = ui->BasePrismeTextOGComboBox->currentText();
        listbinds["PressOnOG"]          = ui->PressonOGCheckBox->isChecked()? 1 : 0;
    }
    listbinds["DepoliOG"]               = ui->DepoliOGCheckBox->isChecked()? 1 : 0;
    listbinds["PlanOG"]                 = ui->PlanOGCheckBox->isChecked()? 1 : 0;
    if(ui->RyserOGCheckBox->isChecked())
        listbinds["RyserOG"]            = ui->RyserSpinBox->value();
    listbinds["FormuleOG"]              = CalculFormule_OG();

    if(QuelleMesure() == "O")
    {
        listbinds["CommentaireOrdoLunettes"]    = CalculCommentaire();
        listbinds["QuelsVerres"]                = QuelsVerres();
        listbinds["QuelOeil"]                   = QuelsYeux();
        listbinds["Monture"]                    = QuelleMonture();
        listbinds["VerreTeinte"]                = ui->VerresTeintesCheckBox->isChecked()? 1 : 0;
    }
    bool a = db->InsertSQLByBinds(TBL_REFRACTIONS, listbinds, tr("Erreur de création dans ") + TBL_REFRACTIONS);
    return a;
}

/*! ---------------------------------------------------------------------------------
Lecture d'une mesure en base
\param DateMesure               -> mesure faite ce jour ou mesures antérieures
\param TypeMesureMesure         -> MFronto, MAutoref, MRefraction ou Mprescription
\param Cycloplegie dilatation   -> Dilatation = que les mesures avec dilatation  - NoDilatation = toutes les mesures, dilatées ou pas
\param int idrefraction         -> idrefraction à lire
\param bool Affichage           -> remplir ou pas le formulaire avec la mesure trouvee
\param OeilCoche = Pour un oeil particulier
---------------------------------------------------------------------------------*/
int dlg_refraction::LectureMesure(DateMesure Quand, Refraction::Mesure Mesure, Cycloplegie dilatation, int idrefraction, bool Affichage, QString FormuleOD, QString FormuleOG)
{
    bool ok;
    QString a;
//    QString Dilatation ("");
//    switch (Cyclo) {
//    case Refraction::Cycloplegie: Dilatation = "1";
//    case Refraction::NoCycloplegie: Dilatation = "0";
//    default: break;
//    }
    QString requete = "SELECT  idRefraction, idPat, DateRefraction, QuelleMesure, QuelleDistance, "           // 0-1-2-3-4
            " Cycloplegie, ODcoche, SphereOD, CylindreOD, AxeCylindreOD, AVLOD, "                   // 5-6-7-8-9-10
            " AddVPOD, AVPOD, PrismeOD, BasePrismeOD, BasePrismeTextOD, PressOnOD,"                 // 11-12-13-14-15-16
            " DepoliOD, PlanOD, RyserOD, FormuleOD, OGcoche, SphereOG, CylindreOG,"                 // 17-18-19-20-21-22-23
            " AxeCylindreOG, AVLOG, AddVPOG, AVPOG, PrismeOG, BasePrismeOG, "                       // 24-25-26-27-28-29
            " BasePrismeTextOG, PressOnOG, DepoliOG, PlanOG, RyserOG, FormuleOG, "                  // 30-31-32-34-35
            " CommentaireOrdoLunettes, QuelsVerres, QuelOeil, Monture, VerreTeinte"                 // 36-37-38-39-40
            " FROM " TBL_REFRACTIONS ;

    // On relit la mesure après selection dans la liste mesure (reprendre)
    if (idrefraction > 0)
        requete += " WHERE idRefraction = "    + QString::number(idrefraction) ;
    else
        // fabrication des criteres de recherche selon le cas de lecture
    {
        requete += " WHERE  IdPat = " + QString::number(Datas::I()->patients->currentpatient()->id()) ;
        if (Quand == Aujourdhui)
            requete += " AND DateRefraction = '" + QDate::currentDate().toString("yyyy-MM-dd") + "'";
        else if (Quand == Avant)
            requete += " AND DateRefraction < '" + QDate::currentDate().toString("yyyy-MM-dd") + "'";
        if (Mesure != Refraction::NoMesure)
            requete += " AND QuelleMesure = '"   + ConvertMesure(Mesure) + "'";
        if (dilatation == Dilatation)
            requete += " AND Cycloplegie =  1";
        if (FormuleOD.length() > 0)                                  // 10-07-2014
            requete += " AND FormuleOD =  '"    + FormuleOD + "'";
        if (FormuleOG.length() > 0)                                  // 10-07-2014
            requete += " AND FormuleOG =  '"    + FormuleOG + "'";
    }
    requete += " ORDER BY DateRefraction, idRefraction";

    QList<QVariantList> mesureslist = db->StandardSelectSQL(requete, ok, tr("Impossible d'accéder à la liste table des mesures!"));
    if (!ok || mesureslist.size()==0)
        return 0;

    if (Affichage)
    {
        // Remplissage des champs Oeil Droit
        if (mesureslist.last().at(6).toInt() == 1)
            ui->ODCheckBox->setChecked(true);                                                               // ODcoche
        if (ui->ODCheckBox->isChecked())
        {
            Init_Value_DoubleSpin(ui->SphereOD,  mesureslist.last().at(7).toDouble());                       // SphereOD
            Init_Value_DoubleSpin(ui->CylindreOD,mesureslist.last().at(8).toDouble());                       // CylindreOD
            Init_Value_DoubleSpin(ui->AddVPOD,   mesureslist.last().at(11).toDouble());                      // AddVPOD
            ui->AxeCylindreOD->setValue(mesureslist.last().at(9).toInt());                                // AxeCylindreOD
            if (mesureslist.last().at(10).toString() != "")
                AVLOD->setText(mesureslist.last().at(10).toString());                                    // AVLOD
            if (mesureslist.last().at(12).toString() != "")
                AVPOD->setText(mesureslist.last().at(12).toString());                                    // AVPOG
            ui->PrismeOD->setValue(mesureslist.last().at(13).toDouble());                                    // PrismeOD
            ui->BasePrismeOD->setValue(mesureslist.last().at(14).toInt());                                // BasePrismeOD
            ui->PressonODCheckBox->setChecked(mesureslist.last().at(16).toBool());                           // PressOnOD
            ui->DepoliODCheckBox->setChecked(mesureslist.last().at(17).toBool());                            // DepoliOD
            ui->PlanODCheckBox->setChecked(mesureslist.last().at(18).toBool());                              // PlanOD
            ui->RyserODCheckBox->setChecked(false);
            if (mesureslist.last().at(19).toInt() > 0)
            {
                ui->RyserODCheckBox->setChecked(true);
                ui->RyserSpinBox->setValue(mesureslist.last().at(19).toInt());                               // RyserOD
            }
        } // fin Oeil droit coche

        // Remplissage des champs Oeil Gauche
        if (mesureslist.last().at(21).toInt() == 1)
            ui->OGCheckBox->setChecked(true);                                                               // ODcoche
        if (ui->OGCheckBox->isChecked())
        {
            Init_Value_DoubleSpin(ui->SphereOG,   mesureslist.last().at(22).toDouble());                     // SphereOG
            Init_Value_DoubleSpin(ui->CylindreOG, mesureslist.last().at(23).toDouble());                     // CylindreOG
            Init_Value_DoubleSpin(ui->AddVPOG,    mesureslist.last().at(26).toDouble());                     // AddVPOG
            ui->AxeCylindreOG->setValue(mesureslist.last().at(24).toInt());                               // AxeCylindreOG
            if (mesureslist.last().at(25).toString() != "")
                AVLOG->setText(mesureslist.last().at(25).toString());                                    // AVLOG
            if (mesureslist.last().at(27).toString() != "")
                AVPOG->setText(mesureslist.last().at(27).toString());                                    // AVPOG
            ui->PrismeOG->setValue(mesureslist.last().at(28).toDouble());                                    // PrismeOG
            ui->BasePrismeOG->setValue(mesureslist.last().at(29).toInt());                                // BasePrismeOG
            ui->PressonOGCheckBox->setChecked(mesureslist.last().at(31).toBool());                           // PressOnOG
            ui->DepoliOGCheckBox->setChecked(mesureslist.last().at(32).toBool());                            // DepoliOG
            ui->PlanOGCheckBox->setChecked(mesureslist.last().at(33).toBool());                              // PlanOG
            ui->RyserOGCheckBox->setChecked(false);
            if (mesureslist.last().at(34).toInt() > 0)
            {
                ui->RyserOGCheckBox->setChecked(true);
                ui->RyserSpinBox->setValue(mesureslist.last().at(34).toInt());
            }                                                                                               // RyserOG

        } // fin Oeil gauche coche
    }
    return mesureslist.last().at(0).toInt();              // retourne idRefraction
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
              " WHERE   (idPat = " + QString::number(Datas::I()->patients->currentpatient()->id()) +
              " AND QuelleMesure = '" + QuelleMesure() + "')";
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
if (ui->DepoliODCheckBox->isChecked())
    ui->RyserODCheckBox->setVisible(false);
else
    {ui->RyserODCheckBox->setVisible(true);
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
void dlg_refraction::OuvrirListeMesures(QString SupOuRecup)
{
    int RetourListe = 0;
    int idrefraction;

    // Creation du formulaire Dlg Liste Mesures
    Dlg_ListeMes    = new dlg_listemesures(SupOuRecup);
    Dlg_ListeMes->setWindowTitle(tr("Liste des mesures : ") + Datas::I()->patients->currentpatient()->nom() + " " + Datas::I()->patients->currentpatient()->prenom() );

    RetourListe = Dlg_ListeMes->exec();

    // relecture et affichage de la mesure selectionnee
    if (RetourListe > 0 && SupOuRecup == "RECUP")
    {
        idrefraction = Dlg_ListeMes->idRefractionAOuvrir();
        if (idrefraction > 0)
        {
            LectureMesure(NoDate,  Refraction::NoMesure, NoDilatation, idrefraction, true);
            RegleAffichageFiche();
        }
    }
    if (RetourListe > 0 && SupOuRecup == "SUPP")
        RechercheMesureEnCours();
    Dlg_ListeMes->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_ListeMes;
    RegleAffichageFiche();
}

//---------------------------------------------------------------------------------------------------------
// Traitement du prefixe + ou - devant les doubles.
//---------------------------------------------------------------------------------------------------------
void dlg_refraction::PrefixePlus(QDoubleSpinBox *leDouble)
{
    leDouble->setPrefix("");
    if (leDouble->value() >= 0)    leDouble->setPrefix("+");
}

// -------------------------------------------------------------------------------------
// Retourne les valeurs des CheckBox
//--------------------------------------------------------------------------------------
QString dlg_refraction::QuelleMesure()
{
    if (gMode == Refraction::Fronto)          return "P";
    if (gMode == Refraction::Autoref)        return "A";
    if (gMode == Refraction::Acuite)     return "R";
    if (gMode == Refraction::Prescription)   return "O";
    return "";
}
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
    if (gMode != Refraction::Prescription)
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
        if (ui->AddVPOG->value() != ui->AddVPOG->getValeurAvant())
            if (ui->AddVPOD->value() == 0.0 && ui->AddVPOD->isVisible())
            {
                ui->AddVPOD->setValue(ui->AddVPOG->value());
                ui->AddVPOD->CorrigeDioptrie(UpDoubleSpinBox::Near);
                PrefixePlus(ui->AddVPOD);
            }
    }
    if (obj == ui->AddVPOD)
    {
        ui->AddVPOD->setValue(ui->AddVPOD->valueFromText(ui->AddVPOD->text()));
        if (ui->AddVPOD->value() != ui->AddVPOD->getValeurAvant())
            if (ui->AddVPOG->value() == 0.0 && ui->AddVPOG->isVisible())
            {
                ui->AddVPOG->setValue(ui->AddVPOD->value());
                ui->AddVPOG->CorrigeDioptrie(UpDoubleSpinBox::Near);
                PrefixePlus(ui->AddVPOG);
            }
    }
}

//---------------------------------------------------------------------------------
// Recherche d'une ou plusieurs mesure en cours pour ce patient et affichage.
//---------------------------------------------------------------------------------
void dlg_refraction::RechercheMesureEnCours()
{
    bool ok;
    QString Reponse ="";

    if (Datas::I()->refractions->refractions()->isEmpty())
    {
        gMode = Refraction::Fronto;
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
    bool mesuretrouvee = false;
    QString requete = "SELECT idActe, QuelleMesure FROM " TBL_REFRACTIONS   // recherche d'une mesure pour le jour en cours
              " WHERE DateRefraction = '" + QDate::currentDate().toString("yyyy-MM-dd") +
              "' AND   IdPat = " + QString::number(Datas::I()->patients->currentpatient()->id()) ;
    QList<QVariantList> listmesure = db->StandardSelectSQL(requete, ok);
    if (!ok || listmesure.size() == 0)
        mesuretrouvee = true;
    foreach (const QVariant &mesure, listmesure)
    {
        if (mesure.toString() == "O")
        {
            Reponse = "O";
            mesuretrouvee = true;
            break;
        }
    }
    if (!mesuretrouvee)
        foreach (const QVariant &mesure, listmesure)
        {
            if (mesure.toString() == "R")
            {
                Reponse = "R";
                mesuretrouvee = true;
                break;
            }
        }
    if (!mesuretrouvee)
        foreach (const QVariant &mesure, listmesure)
        {
            if (mesure.toString() == "A")
            {
                Reponse = "A";
                mesuretrouvee = true;
                break;
            }
        }
    if (!mesuretrouvee)
        foreach (const QVariant &mesure, listmesure)
        {
            if (mesure.toString() == "P")
            {
                Reponse = "P";
                mesuretrouvee = true;
                break;
            }
        }
    if (Reponse != "")
        if (LectureMesure(Aujourdhui, ConvertMesure(Reponse), NoDilatation, 0, true) > 0)            // on affiche la mesure du jour trouvée
        {
            if (gMode == Refraction::Acuite || gMode == Refraction::Prescription)    Slot_PrescriptionRadionButton_clicked();
            if (gMode == Refraction::Autoref)       gMode = Refraction::Acuite;
            if (gMode == Refraction::Fronto)        gMode = Refraction::Autoref;
            RegleAffichageFiche();
            return ;
        }

    // On n'a rien trouvé pour le jour >> on cherche la dernière mesure de réfraction
    if (LectureMesure(Avant, Refraction::Acuite, NoDilatation, 0, true) > 0)
    {
        RegleAffichageFiche();
        return ;
    }

    // on n'a rien trouvé en réfraction - on cherche la dernière prescription
    if (LectureMesure(Avant, Refraction::Prescription, NoDilatation, 0, true) > 0)
    {
        RegleAffichageFiche();
        return ;
    }
    // on n'a rien trouvé en prescription - on cherche la dernière mesure Autoref
    if (LectureMesure(Avant, Refraction::Autoref, NoDilatation, 0, true) > 0)
    {
        RegleAffichageFiche();
        return ;
    }
    // on n'a rien trouvé en autoref - on cherche la dernière mesure de fronto
    if (LectureMesure(Avant, Refraction::Fronto, NoDilatation, 0, true) > 0)
    {
        RegleAffichageFiche();
        return ;
    }
}

//------------------------------------------------------------------------------------------------------
//  Calcul de la variable Resultat pour resume refraction.
//------------------------------------------------------------------------------------------------------
QString dlg_refraction::RechercheResultat(QString Mesure, QString Cycloplegie, QString TypLun)
 {
    bool ok;
    QString     Resultat    = "";
    QString     ResultatOD  = "";
    QString     ResultatOG  = "";
    QString     Separateur  = "";
    QString     DateOD      = "";
    QString     DateOG      = "";
    QString     Dilate      = "";


    if (Mesure == "A")  Separateur = " / ";
    if (Mesure == "R")  Separateur = "\n\t\t";

    QString requeteBase, zdate;
    requeteBase =   "SELECT ODcoche, OGcoche, DateRefraction, FormuleOD, FormuleOG "     // 0-1-2-3-4
                    " FROM " TBL_REFRACTIONS
                    " WHERE  idPat        =  " + QString::number(Datas::I()->patients->currentpatient()->id()) +
                    " AND    QuelleMesure = '" + Mesure     + "'"
                    " AND    Cycloplegie  =  " + Cycloplegie   ;
    QString requete =  requeteBase;
    if (TypLun > "")
        requete +=  " AND QuelleDistance = '" + TypLun + "'";
    requete += " ORDER BY DateRefraction ASC ";

    QList<QVariantList> resultlist = db->StandardSelectSQL(requete,ok);
    if (!ok || resultlist.size() == 0)
        return "";
    zdate = resultlist.last().at(2).toDate().toString(tr("dd-MM-yyyy"));                        // date YYYY-MM-DD
    if (resultlist.last().at(0).toBool() && resultlist.last().at(1).toBool())      // OD coche et OG coche
        Resultat = tr("dilaté") + ")" "\n\t" + zdate + "\t" + resultlist.last().at(3).toString() + Separateur + resultlist.last().at(4).toString();
    else
    {
        // OD coche = true
        if (resultlist.last().at(0).toBool())
        {
            DateOD      = zdate;
            ResultatOD  =  resultlist.last().at(3).toString();
            requete     = requeteBase + " AND OGcoche =  true  ";
            requete     += " ORDER BY DateRefraction DESC ";
            QVariantList resultdata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
            if (ok && resultdata.size()>0)
            {
                zdate  = resultdata.at(2).toDate().toString(tr("dd-MM-yyyy"));
                ResultatOD  =  resultdata.at(3).toString();
            }
        } // fin OD coche
        else
        {
            // OG coche = true
            if (resultlist.last().at(1).toBool())
            {
                DateOG      = zdate;
                ResultatOG  = resultlist.last().at(4).toString();
                requete     = requeteBase + " AND ODcoche =  true  ";
                requete     += " ORDER BY DateRefraction DESC ";
                QVariantList resultdata = db->getFirstRecordFromStandardSelectSQL(requete, ok);
                if (ok && resultdata.size()>0)
                {
                    zdate  = resultdata.at(2).toDate().toString(tr("dd-MM-yyyy"));
                    ResultatOD  =  resultdata.at(3).toString();
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
    if (Cycloplegie == "1")
        Dilate = "        (";
    else
        Dilate = "        (" + tr("non") + " ";
    Resultat = "\n" + Dilate + Resultat;

    return Resultat;
}

//------------------------------------------------------------------------------------------------------
//  Calcul de la variable ResultatVerres
//------------------------------------------------------------------------------------------------------
QString dlg_refraction::RechercheVerres()
{
    bool ok;
    QString     ResultatVerres = "";
    QString     zdate, Formule, TypeMesure;

    QString requete     =   "SELECT ODcoche, OGcoche, DateRefraction, FormuleOD, FormuleOG, QuelleMesure "     // 0-1-2-3-4-5
                    " FROM " TBL_REFRACTIONS
                    " WHERE  idPat        =  "+ QString::number(Datas::I()->patients->currentpatient()->id()) +
                    " AND (QuelleMesure = 'P' OR QuelleMesure = 'O') "
                    " ORDER  BY DateRefraction DESC ";
    QList<QVariantList> verreslist = db->StandardSelectSQL(requete,ok);
    if (!ok || verreslist.size() == 0)
        return "";
    for (int i = 0; i < verreslist.size(); i++)
        {
        if (verreslist.at(i).at(5).toString() == "O")
            TypeMesure =  tr("Prescription");
        else
            TypeMesure =  tr("Verres portés");
        zdate = verreslist.at(i).at(2).toString();                         // date YYYY-MM-DD
        zdate = zdate.mid(8,2) + "-" + zdate.mid(5,2) + "-" + zdate.left(4);
        // calcul Formule
        Formule = verreslist.at(i).at(3).toString();            // Formule OD
        if (Formule.length() > 0)
            Formule += " / ";
        Formule += verreslist.at(i).at(4).toString();           // Formule OG
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
    QString DelimiterDebut  = "<a name=\"debut" + QString::number(gidRefraction) + "\"></a>";
    QString DelimiterFin    = "<a name=\"fin" + QString::number(gidRefraction) + "\"></a>";
    // QString TagAncre, numIDref; // 07-07-2014 08-08-2014

    DistanceMesure Distance = Les2;

    // Génération du code html pour TAG Ancre avec ID refraction            // 07-07-2014 // 08-08-2014
    //numIDref = QString::number(gListeRefractionID.at(gListeRefractionID.size()-1));
    //TagAncre = "<a name=\"" + numIDref + "\"></a>" "<span ><a href=\""+ numIDref + "\" style=\"text-decoration:none\" style=\"color:#000000\"> ";
    if (ui->SphereOD->hasFocus())       PrefixePlus(ui->SphereOD);
    if (ui->SphereOG->hasFocus())       PrefixePlus(ui->SphereOG);
    if (ui->CylindreOD->hasFocus())     PrefixePlus(ui->CylindreOD);
    if (ui->CylindreOG->hasFocus())     PrefixePlus(ui->CylindreOG);
    if (ui->AddVPOD->hasFocus())        PrefixePlus(ui->AddVPOD);
    if (ui->AddVPOG->hasFocus())        PrefixePlus(ui->AddVPOG);

    if (gMode == Refraction::Prescription)
    {
        if (ui->V2PrescritRadioButton->isChecked()) Distance = Les2;
        if (ui->VLPrescritRadioButton->isChecked()) Distance = Loin;
        if (ui->VPPrescritRadioButton->isChecked()) Distance = Pres;
    }
    else
    {
        if (ui->V2RadioButton->isChecked()) Distance = Les2;
        if (ui->VLRadioButton->isChecked()) Distance = Loin;
        if (ui->VPRadioButton->isChecked()) Distance = Pres;
        if (gMode == Refraction::Acuite && ui->CycloplegieCheckBox->isChecked())        Distance = Loin;
    }

    if (gMode == Refraction::Fronto || gMode == Refraction::Prescription)
        //EN MODE PORTE  ou Prescription --  détermination de gResultatPO ou gResultatPR  ---------------------------------------------------------------------------
    {
        // - 1 - détermination des verres
        // On se débarasse des dépoli et des plan----------------------------------------------------
        VerreSpecialOD = "non";
        VerreSpecialOG = "non";
        if (gAfficheDetail)
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
            if (((!ui->ODCheckBox->isChecked() && (gMode != Refraction::Prescription)) && VerreSpecialOD == "non") || (!ui->ODPrescritCheckBox->isChecked() && (gMode == Refraction::Prescription)))
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
            if (((!ui->OGCheckBox->isChecked() && (gMode != Refraction::Prescription)) && VerreSpecialOG == "non") || (!ui->OGPrescritCheckBox->isChecked() && (gMode == Refraction::Prescription)))
                ResultatOG = "Rien";
        }

        gResultatP = "";

        // Détermination de gResultatP
        switch (Distance)
        {
        case Les2: // Mesure ou prescription de verres multifocaux
            if (ResultatOD == ResultatOG)
            {
                if (ResultatVPOD != tr("plan") || VerreSpecialOD != "non")
                    gResultatP = ResultatOD + tr(" ODG");
                else
                    gResultatP = ResultatVLOD + tr(" VL ODG");
            }
            else
            {
                if (VerreSpecialOD != "non")
                {
                    if (VerreSpecialOG != "non")
                        gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;
                    else
                    {
                        if (ResultatOG == "Rien")
                            gResultatP = VerreSpecialOD + tr(" OD");
                        else
                            gResultatP = VerreSpecialOD + " / " + ResultatOG + (ui->AddVPOG->value() == 0.0? tr(" VL") : "");
                    }
                }
                else if (ResultatOD != "Rien")
                {
                    if (ui->AddVPOD->value() == 0.0)
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = ResultatVLOD + " / " + VerreSpecialOG + tr(" VL");
                        else if (ResultatOG == "Rien")
                            gResultatP = ResultatVLOD + tr(" OD VL");
                        else if (ui->AddVPOG->value() == 0.0)
                            gResultatP = ResultatVLOD + " / " + ResultatVLOG + tr(" VL");
                        else
                            gResultatP = ResultatVLOD + " / " + ResultatOG;
                    }
                    else
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = ResultatOD + " / " + VerreSpecialOG;
                        else if (ResultatOG == "Rien")
                            gResultatP = ResultatOD + tr(" OD");
                        else
                        {
                            if (ui->AddVPOG->value() == 0.0)
                                gResultatP = ResultatOD + " / " + ResultatVLOG;
                            else
                            {
                                if (ResultatVPOD == ResultatVPOG)
                                    gResultatP = ResultatVLOD + " / " + ResultatOG + tr(" ODG");
                                else
                                    gResultatP = ResultatOD + " / " + ResultatOG;
                            }
                        }
                    }
                }
                else if (ResultatOD == "Rien")
                {
                    if (VerreSpecialOG != "non")
                         gResultatP = VerreSpecialOG + tr(" OG");
                    else if (ResultatOG != "Rien")
                    {
                     if (ui->AddVPOG->value() == 0.0)
                         gResultatP = ResultatVLOG + tr(" OG VL");
                     else
                         gResultatP = ResultatOG + tr(" OG");
                    }
                }

            }
            break;
        case Loin: // Mesure ou prescription de verres de loin
            if (ResultatOD == ResultatOG)
            {
                if (VerreSpecialOD != "non")
                    gResultatP = VerreSpecialOD + tr(" ODG");
                else
                    gResultatP = ResultatVLOD + tr(" VL ODG");
            }
            else
            {
                if (VerreSpecialOD != "non")
                {
                    if (VerreSpecialOG != "non")
                        gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;
                    else if (ResultatOG == "Rien")
                        gResultatP = VerreSpecialOD + tr(" OD");
                    else
                        gResultatP = VerreSpecialOD + " / " + ResultatVLOG + tr(" VL");
                }
                else if (ResultatOD != "Rien")
                {
                    if (VerreSpecialOG != "non")
                        gResultatP = ResultatVLOD + " / " + VerreSpecialOG + tr(" VL");
                    else if (ResultatOG == "Rien")
                        gResultatP = ResultatVLOD + tr(" OD VL");
                    else
                        gResultatP = ResultatVLOD + " / " + ResultatVLOG + tr(" VL");
                }
                else
                {
                    if (VerreSpecialOG != "non")
                        gResultatP = VerreSpecialOG + tr(" VL");
                    else if (ResultatOG != "Rien")
                        gResultatP = ResultatVLOG + tr(" OG VL");
                }
            }
            break;
        case Pres: // Mesure ou prescription de verres de près
            if (gMode == Refraction::Fronto)                                     // Calcul des verres de près en mode porte
            {
                if (ResultatOD == ResultatOG)
                {
                    if (VerreSpecialOD != "non")
                        gResultatP = VerreSpecialOD + tr(" ODG");
                    else
                        gResultatP = ResultatVLOD +  tr(" VP ODG");
                }
                else
                {
                    if (VerreSpecialOD != "non")
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;
                        else if (ResultatOG == "Rien")
                            gResultatP = VerreSpecialOD +  tr(" OD");
                        else
                            gResultatP = VerreSpecialOD + " / " + ResultatVLOG +  tr(" VP");
                    }
                    else  if (ResultatOD != "Rien")
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = ResultatVLOD + " / " + VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            gResultatP = ResultatVLOD +  tr(" OD VP");
                        else
                            gResultatP = ResultatVLOD + " / " + ResultatVLOG +  tr(" VP");
                    }
                    else
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            gResultatP = ResultatVLOG +  tr(" OG VP");
                        else
                            gResultatP = ResultatVLOG +  tr(" OG VP");
                    }
                }
            }
            else if (gMode == Refraction::Prescription) // Calcul des verres de près en mode impression
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
                        gResultatP = VerreSpecialOD + tr(" ODG");
                    else
                        gResultatP = ResultatVLOD + tr(" VP ODG");
                }
                else
                {
                    if (VerreSpecialOD != "non")
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;
                        else if (ResultatOG == "Rien")
                            gResultatP = VerreSpecialOD + tr(" OD");
                        else
                            gResultatP = VerreSpecialOD + " / " + ResultatVLOG + tr(" VP");
                    }
                    else  if (ResultatOD != "Rien")
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = ResultatVLOD + " / " + VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            gResultatP = ResultatVLOD +  tr(" OD VP");
                        else
                            gResultatP = ResultatVLOD + " / " + ResultatVLOG +  tr(" VP");
                    }
                    else
                    {
                        if (VerreSpecialOG != "non")
                            gResultatP = VerreSpecialOG +  tr(" VP");
                        else if (ResultatOG == "Rien")
                            gResultatP = ResultatVLOG +  tr(" OG VP");
                        else
                            gResultatP = ResultatVLOG +  tr(" OG VP");
                    }
                }
            }
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
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + " / " + ResultatPrismeOG + "</td>";
            if (ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" OD") + "</td>"
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + tr(" OD") + "</td>"
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + " / " + ResultatPrismeOG + "</td>";
        }
        if (ResultatPrismeOD != "" &&  ResultatPrismeOG != "" && ResultatPrismeOD == ResultatPrismeOG)
        {
            if (ui->PressonODCheckBox->isChecked() && ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" ODG") + "</td>";
            if (ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" OD") + "</td>"
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + tr(" OD") + "</td>"
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOG + tr(" OG") + "</td>";
            if (!ui->PressonODCheckBox->isChecked() && !ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prismes ") + ResultatPrismeOD + tr(" ODG") + "</td>";
        }
        if (ResultatPrismeOD != "" &&  ResultatPrismeOG == "")
        {
            if (ui->PressonODCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOD + tr(" OD") + "</td>";
            else
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOD + tr(" OD") + "</td>";
        }
        if (ResultatPrismeOD == "" &&  ResultatPrismeOG != "")
        {
            if (ui->PressonOGCheckBox->isChecked())
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Press-On " + ResultatPrismeOG + tr(" OG") + "</td>";
            else
                ResultatPrisme =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>" + tr("Prisme ") + ResultatPrismeOG + tr(" OG") + "</td>";
        }

        //3 - incorporation des valeurs de Ryser
        ResultatRyser = "";
        if (ui->RyserODCheckBox->isChecked())
            ResultatRyser =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Ryser " + ui->RyserSpinBox->text() + tr(" OD") + "</td>";
        if (ui->RyserOGCheckBox->isChecked())
            ResultatRyser =
                        "<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td>Ryser " + ui->RyserSpinBox->text() + tr(" OG") + "</td>";

        // 4 - détermination du resultat final
        if (gMode == Refraction::Fronto)
        {
            gResultatPO =  "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>Porte:</b></font></td><td>" + gResultatP + "</td>" + ResultatPrisme + ResultatRyser;
            gResultatPO.insert(gResultatPO.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
        else if (gMode == Refraction::Prescription)
        {
            gResultatPR =  "<td width=\"30\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>VP:</b></font></td><td>" + gResultatP + " " + ui->CommentairePrescriptionTextEdit->toPlainText() + "</td>" + ResultatPrisme + ResultatRyser;
            gResultatPR.insert(gResultatPR.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
    }

    if (gMode == Refraction::Autoref)
        //EN MODE Autoref --  détermination de gResultatA  ---------------------------------------------------------------------------
    {
        gResultatA = "";
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
        if (gResultatA == "" && ResultatOD == "Rien" && ResultatOG != "Rien")
            gResultatA = ResultatVLOG + tr(" OG");

        if (gResultatA == "" && ResultatOG == "Rien" && ResultatOD != "Rien")
            gResultatA = ResultatVLOD + tr(" OD");

        if (gResultatA == "" && ResultatOD != "Rien" && ResultatOG != "Rien")
            gResultatA = ResultatVLOD + " / " + ResultatVLOG;

        QString kerato = "";
        if (QLocale().toDouble(ui->K1OD->text())>0)
        {
            if (gDioptrAstOD!=0.0)
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OD->text() + "/" + ui->K2OD->text() + " Km = " + QString::number((QLocale().toDouble(ui->K1OD->text()) + QLocale().toDouble(ui->K2OD->text()))/2,'f',2)
                        + "</td><td width=\"120\">" + QString::number(gDioptrAstOD,'f',2) +  tr(" à ") + ui->AxeKOD->text() + "°</td>";
            else
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOD") + ":</b></font></td><td width=\"240\">"
                        + ui->K1OD->text() + tr(" à ") + ui->AxeKOD->text() + "°/" + ui->K2OD->text() + tr(" Km = ") + QString::number((QLocale().toDouble(ui->K1OD->text()) + QLocale().toDouble(ui->K2OD->text()))/2,'f',2) + "</td>";
        }
        if (QLocale().toDouble(ui->K1OG->text())>0)
        {
            if (gDioptrAstOG!=0.0)
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OG->text() + "/" + ui->K2OG->text() + " Km = " + QString::number((QLocale().toDouble(ui->K1OG->text()) + QLocale().toDouble(ui->K2OG->text()))/2,'f',2)
                        + "</td><td width=\"120\">" + QString::number(gDioptrAstOG,'f',2) +  tr(" à ") + ui->AxeKOG->text() + "°</td>";
            else
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " COULEUR_TITRES "><b>" + tr("KOG") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OG->text() +  tr(" à ") + ui->AxeKOG->text() + "°/" + ui->K2OG->text() + tr(" Km = ") + QString::number((QLocale().toDouble(ui->K1OG->text()) + QLocale().toDouble(ui->K2OG->text()))/2,'f',2) + "</td>";
        }
        if (ui->CycloplegieCheckBox->isChecked())
        {
            gResultatAdil    = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>Autoref:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatA + "</td><td><font color = \"red\">" + tr("(dilaté)") + "</font></td>" + kerato;
            gResultatAdil.insert(gResultatAdil.lastIndexOf("</font></td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
        else
        {
            gResultatAnondil = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>Autoref:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatA + "</td><td>" + tr("(non dilaté)") + "</td>" + kerato;
            gResultatAnondil.insert(gResultatAnondil.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
    }

    if (gMode == Refraction::Acuite)
        //EN MODE Refraction --  détermination de gResultatR  ---------------------------------------------------------------------------
    {
        // - 1 - détermination des verres
        gResultatR = "";

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
        if (AVLOD->text().contains("/10"))
            av = AVLOD->text().replace("/10","").toDouble();
        if (av < 6)
            colorVLOD =  "\"red\"";
        if (av > 5 && av < 9)
            colorVLOD =  "\"orange\"";
        av = 0;
        if (AVLOG->text().contains("/10"))
            av = AVLOG->text().replace("/10","").toDouble();
        if (av < 6)
            colorVLOG =  "\"red\"";
        if (av >5 && av < 9)
            colorVLOG =  "\"orange\"";
        if (AVPOD->text().replace(",",".").toInt() > 4 || AVPOD->text().contains("<"))
            colorVPOD =  "\"red\"";
        if (AVPOD->text().replace(",",".").toInt() > 2 && AVPOD->text().replace(",",".").toInt() < 5)
            colorVPOD =  "\"orange\"";
        if (AVPOG->text().replace(",",".").toInt() > 4 || AVPOG->text().contains("<"))
            colorVPOG =  "\"red\"";
        if (AVPOG->text().replace(",",".").toInt() > 2 && AVPOG->text().replace(",",".").toInt() < 5)
            colorVPOG =  "\"orange\"";


        // Détermination de gResultatR
        switch (Distance)
        {
        case Les2: // Refraction de loin et de près
            if (ResultatOD != "Rien")
            {
                if (ui->AddVPOD->value() == 0.0)
                {
                    if (ResultatOG == "Rien")
                        gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD");
                    else
                    {
                        if (ui->AddVPOG->value() == 0.0)
                            gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD") + "</td></p>"
                                    +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG") + "</td>";
                        else
                            gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD") + "</td></p>"
                                    +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG") + "</td>";
                    }
                }
                else
                {
                    if (ResultatOG == "Rien")
                        gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD");
                    else
                    {
                        if (ui->AddVPOG->value() == 0.0)
                            gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD") + "</td></p>"
                                    +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG") + "</td>";
                        else
                            gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD") + "</td></p>"
                                    +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                                    + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG") + "</td>";
                    }
                }
            }
            else if (ResultatOG != "Rien")
            {
                if (ui->AddVPOG->value() == 0.0)
                    gResultatR = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG");
                else
                    gResultatR = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG");
            }
            break;
        case Loin: // Réfraction de loin ou sous cycloplégie
            if (ResultatOD != "Rien")
            {
                if (ResultatOG == "Rien")
                    gResultatR = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + AVLOD->text() + "</b></font> " + tr("OD");
                else
                    gResultatR = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + AVLOD->text() + "</b></font> " + tr("OD") + "</td></p>"
                            +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                            + ResultatVLOG + " " + "<font color = " + colorVLOG + "><b>" + AVLOG->text() + "</b></font> " + tr("OG") + "</td>";
            }
            else if (ResultatOG != "Rien")
                gResultatR = ResultatVLOG + "<font color = " + colorVLOG + "><b>" + AVLOG->text() + "</b></font> " + tr("OG");
            break;
        default:
            break;
        }
        if (ui->CycloplegieCheckBox->isChecked())
        {
            gResultatRdil = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatR + "</td><td width=\"60\"><font color = \"red\">" + tr("(dilaté)") + "</font></td><td>" + Datas::I()->users->userconnected()->login() + "</td>";
            gResultatRdil.insert(gResultatRdil.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
        else
        {
            gResultatRnondil = "<td width=\"60\">" + DelimiterDebut + "<font color = " COULEUR_TITRES "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatR + "</td><td width=\"70\">" + tr("(non dilaté)") + "</td><td>" + Datas::I()->users->userconnected()->login() + "</td>";
            gResultatRnondil.insert(gResultatRnondil.lastIndexOf("</td>")-1, DelimiterFin);       // on met le dernier caractère en ancre
        }
    }

    // Consolidation de tous les résultats dans un même QString

    gResultatObservation = gResultatPO + gResultatAnondil + gResultatAdil + gResultatRnondil + gResultatRdil;
    gResultatPR = gResultatPR + m_commentaireresume;
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
                    break;
                }
                if (ui->AddVPOD->value() != 0.0 && ui->AddVPOG->value() != 0.0)
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
                    break;
                }
                if (ui->AddVPOD->value() != 0.0 && ui->AddVPOG->value() == 0.0)
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
                    break;
                }
                if (ui->AddVPOD->value() == 0.0 && ui->AddVPOG->value() != 0.0)
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
                    break;
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
            Resultat = Resultat + "\n" + ODPrisme + "\n" + OGPrisme;;
    }

    //4-5 La monture
    if (ui->UneMonturePrescritRadioButton->isChecked())
        Resultat = Resultat + "\n" + tr("Monture");
    if (ui->DeuxMonturesPrescritRadioButton->isChecked())
        Resultat = Resultat + "\n" + tr("2 montures");

    //4-6 Les commentaires
    Resultat = Resultat + "\n" + m_commentaire;
    if (ui->CommentairePrescriptionTextEdit->document()->toPlainText() > "")
        Resultat = Resultat + "\n" + ui->CommentairePrescriptionTextEdit->document()->toPlainText();

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
    ResultatAutorefDil = RechercheResultat("A", "1", "");

    // 2-2- Mesure non dilatee
    ResultatAutorefNonDil = RechercheResultat("A", "0", "");

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
    ResultatRefractionDil = RechercheResultat("R", "1", "");

    // 3-2 Mesure non dilatee
    ResultatRefractionV2 = RechercheResultat("R", "0", "2");
    ResultatRefractionVL = RechercheResultat("R", "0", "L");

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
    if (gMode == Refraction::Fronto)      // mode Porte
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
        gAfficheDetail =
            (ui->PrismeOD->value() != 0.0         || ui->PrismeOG->value() != 0.0          ||
            ui->RyserODCheckBox->isChecked()      || ui->RyserOGCheckBox->isChecked()       ||
            ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()      ||
            ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked());
        if (gAfficheDetail && ui->DetailsPushButton->text() ==  tr("- de détails"))
            ui->DetailsPushButton->setEnabled(false);
        if (ui->DetailsPushButton->text() ==  tr("- de détails")) gAfficheDetail = true;
        AfficherDetail(gAfficheDetail);
        if(gAfficheDetail)
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
        if (gAfficheDetail)
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    } // fin mode Porte

    if (gMode == Refraction::Autoref)    // mode AutoRef
    {
        ui->AnnulPushButton->setVisible(true);
        ui->OKPushButton->setIcon(Icons::icOK());
        ui->OKPushButton->setText( tr("Enregistrer\net fermer"));
        ui->ODCheckBox->setVisible(true);
        ui->OGCheckBox->setVisible(true);
        ui->KeratometrieGroupBox->setVisible(true);
        ui->frame_Prescription->setVisible(false);
        ui->AutorefRadioButton->setChecked(true);
        ui->DetailsPushButton->setVisible(false);
        AfficherDetail(false);
        ui->KeratometrieGroupBox->setVisible(true);
        Afficher_AVL_AVP(false);
        Afficher_AddVP(false);
        ui->QuelleDistanceGroupBox->setVisible(false);
        setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    }

    if (gMode == Refraction::Acuite) // mode Refraction
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

    if (gMode == Refraction::Prescription)   // mode Prescription
    {
        Afficher_AVL_AVP(false);
        ui->OKPushButton->setIcon(Icons::icImprimer());
        ui->OKPushButton->setText( tr("Imprimer"));
        ui->CycloplegieCheckBox->setVisible(false);
        ui->KeratometrieGroupBox->setVisible(false);
        ui->ODCheckBox->setVisible(false);
        ui->OGCheckBox->setVisible(false);
        gAfficheDetail =
            (ui->PrismeOD->value() != 0.0          || ui->PrismeOG->value() != 0.0      ||
            ui->RyserODCheckBox->isChecked()      || ui->RyserOGCheckBox->isChecked()   ||
            ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()  ||
            ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked());
        if (gAfficheDetail && ui->DetailsPushButton->text() ==  tr("- de détails"))
            ui->DetailsPushButton->setEnabled(false);
        if (ui->DetailsPushButton->text() ==  tr("- de détails")) gAfficheDetail = true;
        AfficherDetail(gAfficheDetail);
        if(gAfficheDetail)
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

        if (gAfficheDetail)
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

QString dlg_refraction::ResultatPrescription()
{
    return gResultatPR;
}
QString dlg_refraction::ResultatObservation()
{
    return gResultatObservation;
}
//---------------------------------------------------------------------------------
// Maj d'un enregistrement dans DonneesOphtaPatient
//---------------------------------------------------------------------------------
void dlg_refraction::UpdateDonneesOphtaPatient()
{
    QString UpdateDOPrequete = "UPDATE  " TBL_DONNEES_OPHTA_PATIENTS
                " SET QuelleMesure = '" + QuelleMesure() + "'";
    if (QuelleMesure()=="A")
    {
        if ((ConvDouble(ui->K1OD->text()) > 0 || ConvDouble(ui->K2OD->text()) > 0) && ui->ODCheckBox->isChecked())       // 16-07-2014
        {
            UpdateDOPrequete +=
                    " , K1OD = " + QString::number(QLocale().toDouble(ui->K1OD->text()),'f',2) +
                    " , K2OD = " + QString::number(QLocale().toDouble(ui->K2OD->text()),'f',2) +
                    " , AxeKOD = " + ui->AxeKOD->text();
        }
        if ((ConvDouble(ui->K1OG->text()) > 0 || ConvDouble(ui->K2OG->text()) > 0) && ui->OGCheckBox->isChecked())       // 16-07-2014
        {
            UpdateDOPrequete +=
                    ", K1OG = " + QString::number(QLocale().toDouble(ui->K1OG->text()),'f',2) +
                    ", K2OG = " + QString::number(QLocale().toDouble(ui->K2OG->text()),'f',2) +
                    ", AxeKOG = " + ui->AxeKOG->text();
        }
        if (ConvDouble(ui->K1OD->text()) > 0 || ConvDouble(ui->K2OD->text()) > 0 || // 16-07-2014
                ConvDouble(ui->K1OG->text()) > 0 || ConvDouble(ui->K2OG->text()) > 0)   // 16-07-2014
        {
            UpdateDOPrequete +=
                    ", OrigineK = '" + QuelleMesure() + "'" +
                    ", DateK =  '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss") + "'";
        }
    }
    if (ui->ODCheckBox->isChecked())
    {
        if (ui->SphereOD->value() == 0.0)
            UpdateDOPrequete += ", SphereOD = null";
        else
            UpdateDOPrequete += ", SphereOD = " + QString::number(ui->SphereOD->value());
        if (ui->CylindreOD->value() == 0.0)
            UpdateDOPrequete +=
                ", CylindreOD = null"
                ", AxeCylindreOD = null";
        else
            UpdateDOPrequete +=
                ", CylindreOD = " + QString::number(ui->CylindreOD->value()) +
                ", AxeCylindreOD = " + QString::number(ui->AxeCylindreOD->value());
        if (gMode == Refraction::Acuite)
            UpdateDOPrequete += ", AVLOD = '" + AVLOD->text() + "'";
        if (ui->AddVPOD->value() > 0 && ui->V2RadioButton->isChecked()  && gMode == Refraction::Acuite)
            UpdateDOPrequete += ", AddVPOD = " + QString::number(ui->AddVPOD->value());
        else
            UpdateDOPrequete += ",AddVPOD = null";
        if (gMode == Refraction::Acuite && ui->V2RadioButton->isChecked())
            UpdateDOPrequete += ", AVPOD = '" + AVPOD->text() + "'";
        else
            UpdateDOPrequete += ", AVPOD = null";
        UpdateDOPrequete += ", DateRefOD = '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss") + "'";
    }
    if (ui->OGCheckBox->isChecked())
    {
        if (ui->SphereOG->value() == 0.0)
            UpdateDOPrequete += ", SphereOG = null";
        else
            UpdateDOPrequete += ", SphereOG = " + QString::number(ui->SphereOG->value());
        if (ui->CylindreOG->value() == 0.0)
            UpdateDOPrequete +=
                ", CylindreOG = null"
                ", AxeCylindreOG = null";
        else
            UpdateDOPrequete +=
                ", CylindreOG = " + QString::number(ui->CylindreOG->value()) +
                ", AxeCylindreOG = " + QString::number(ui->AxeCylindreOG->value());
        if (gMode == Refraction::Acuite)
            UpdateDOPrequete += ", AVLOG = '" + AVLOG->text() + "'";
        if (ui->AddVPOG->value() > 0 && ui->V2RadioButton->isChecked() && gMode == Refraction::Acuite)
            UpdateDOPrequete += ", AddVPOG = " + QString::number(ui->AddVPOG->value());
        else
            UpdateDOPrequete += ",AddVPOG = null";
        if (gMode == Refraction::Acuite && ui->V2RadioButton->isChecked())
            UpdateDOPrequete += ", AVPOG = '" + AVPOG->text() + "'";
        else
            UpdateDOPrequete += ", AVPOG = null";
        UpdateDOPrequete += ", DateRefOG = '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss") + "'";
    }
    UpdateDOPrequete +=  " WHERE idPat = " + QString::number(Datas::I()->patients->currentpatient()->id()) + " AND QuelleMesure = '" + QuelleMesure() + "'";
    db->StandardSQL(UpdateDOPrequete, tr("Erreur de MAJ dans ")+ TBL_DONNEES_OPHTA_PATIENTS);
}

//---------------------------------------------------------------------------------------------------------
// Traitement du prefixe + ou - devant les valeurs numériques.
//---------------------------------------------------------------------------------------------------------
QString dlg_refraction::Valeur(QString StringValeur)
{
    if (StringValeur.toDouble() > 0 && StringValeur.left(1) != "+")
        return("+" + StringValeur);
    else
        return(StringValeur);
}

//-----------------------------------------------------------------------------------------
// Affichage des données lues sur le port série du Fronto
//-----------------------------------------------------------------------------------------
void dlg_refraction::AfficheMesureFronto()
{
    Slot_PorteRadioButton_Clicked();
    if (proc->DonneesFronto().isEmpty())
    {
        UpMessageBox::Watch(this, tr("pas de données reçues du frontofocomètre"));
        return;
    }
    QMap<QString,QVariant>  MesuresFronto = proc->DonneesFronto();

    //A - AFFICHER LA MESURE --------------------------------------------------------------------------------------------------------------------------------------------------------
    if (MesuresFronto["AddOD"].toDouble()>0 || MesuresFronto["AddOG"].toDouble()>0)
    {
        if (!ui->V2RadioButton->isChecked())
        {
            ui->V2RadioButton->setChecked(true);
            ui->VLRadioButton->setChecked(false);
            ui->VPRadioButton->setChecked(false);
            Slot_QuelleDistance_Clicked();
        }
    }
    else
    {
        if (ui->V2RadioButton->isChecked())
        {
            ui->V2RadioButton->setChecked(false);
            ui->VLRadioButton->setChecked(true);
            ui->VPRadioButton->setChecked(false);
            Slot_QuelleDistance_Clicked();
        }
    }
    // OEIL DROIT -----------------------------------------------------------------------------
    Init_Value_DoubleSpin(ui->SphereOD, MesuresFronto["SphereOD"].toDouble());
    Init_Value_DoubleSpin(ui->CylindreOD, MesuresFronto["CylOD"].toDouble());
    ui->AxeCylindreOD   ->setValue(MesuresFronto["AxeOD"].toInt());
    ui->AddVPOD         ->setValue(MesuresFronto["AddOD"].toDouble());
    // OEIL GAUCHE ---------------------------------------------------------------------------
    Init_Value_DoubleSpin(ui->SphereOG, MesuresFronto["SphereOG"].toDouble());
    Init_Value_DoubleSpin(ui->CylindreOG, MesuresFronto["CylOG"].toDouble());
    ui->AxeCylindreOG   ->setValue(MesuresFronto["AxeOG"].toInt());
    ui->AddVPOG         ->setValue(MesuresFronto["AddOG"].toDouble());
}

//-----------------------------------------------------------------------------------------
// Affichage des données lues sur le port série de l'Autoref
//-----------------------------------------------------------------------------------------
void dlg_refraction::AfficheMesureAutoref()
{
    QMap<QString,QVariant>  MesureAutoref = proc->DonneesAutoref();
    QMap<QString,QVariant>  MesureKerato  = proc->DonneesKerato();
    if (MesureAutoref.size()==0 && MesureKerato.size()==0)
    {
        UpMessageBox::Watch(this, tr("pas de données reçues de l'autorefractomètre"));
        return;
    }
    QString mSphereOD   = MesureAutoref["SphereOD"].toString();
    QString mCylOD      = MesureAutoref["CylOD"].toString();
    QString mAxeOD      = MesureAutoref["AxeOD"].toString();
    QString mSphereOG   = MesureAutoref["SphereOG"].toString();
    QString mCylOG      = MesureAutoref["CylOG"].toString();
    QString mAxeOG      = MesureAutoref["AxeOG"].toString();
    QString mK1OD       = MesureKerato["K1OD"].toString();
    QString mK2OD       = MesureKerato["K2OD"].toString();
    int     mAxeKOD     = MesureKerato["AxeKOD"].toInt();
    QString mDioptrKOD  = MesureKerato["DioptrKOD"].toString();
    QString mK1OG       = MesureKerato["K1OG"].toString();
    QString mK2OG       = MesureKerato["K2OG"].toString();
    int     mAxeKOG     = MesureKerato["AxeKOG"].toInt();
    QString mDioptrKOG  = MesureKerato["DioptrKOG"].toString();
    gDioptrAstOD        = 0;
    gDioptrAstOG        = 0;

    Slot_AutorefRadioButton_Clicked();

    // OEIL DROIT -----------------------------------------------------------------------------
    Init_Value_DoubleSpin(ui->SphereOD, mSphereOD.toDouble());
    Init_Value_DoubleSpin(ui->CylindreOD, mCylOD.toDouble());
    ui->AxeCylindreOD       ->setValue(mAxeOD.toInt());
    // OEIL GAUCHE ---------------------------------------------------------------------------
    Init_Value_DoubleSpin(ui->SphereOG, mSphereOG.toDouble());
    Init_Value_DoubleSpin(ui->CylindreOG, mCylOG.toDouble());
    ui->AxeCylindreOG       ->setValue(mAxeOG.toInt());
    // OEIL DROIT -----------------------------------------------------------------------------
    if (mK1OD != "")
    {
        ui->K1OD            ->setText(QLocale().toString(mK1OD.toDouble(),'f',2 ));
        ui->K2OD            ->setText(QLocale().toString(mK2OD.toDouble(),'f',2 ));
        ui->AxeKOD          ->setText(QString::number(mAxeKOD));
        gDioptrAstOD        = QLocale().toDouble(mDioptrKOD);
    }
   // OEIL GAUCHE ---------------------------------------------------------------------------
    if (mK1OG != "")
    {
        ui->K1OG            ->setText(QLocale().toString(mK1OG.toDouble(),'f',2 ));
        ui->K2OG            ->setText(QLocale().toString(mK2OG.toDouble(),'f',2 ));
        ui->AxeKOG          ->setText(QString::number(mAxeKOG));
        gDioptrAstOG        = QLocale().toDouble(mDioptrKOG);
    }
}

//-----------------------------------------------------------------------------------------
// Affichage des données lues sur le port série du refracteur
//-----------------------------------------------------------------------------------------
void dlg_refraction::AfficheMesureRefracteur()
{
    QMap<QString,QVariant>  MesuresRefracteur;
    if ( proc->DonneesRefracteurFin().isEmpty())
    {
        Slot_RefractionRadioButton_Clicked();
        gMode = Refraction::Acuite;
        MesuresRefracteur = proc->DonneesRefracteurSubj();
    }
    else
    {
        Slot_PrescriptionRadionButton_clicked();
        gMode = Refraction::Prescription;
        MesuresRefracteur = proc->DonneesRefracteurFin();
    }
    if (MesuresRefracteur.isEmpty())
    {
        UpMessageBox::Watch(this, tr("pas de données reçues du refracteur"));
        return;
    }

    Init_Value_DoubleSpin(ui->SphereOD, MesuresRefracteur["SphereOD"].toDouble());
    Init_Value_DoubleSpin(ui->CylindreOD, MesuresRefracteur["CylOD"].toDouble());
    ui->AxeCylindreOD   ->setValue(MesuresRefracteur["AxeOD"].toInt());
    ui->AddVPOD->setValue(MesuresRefracteur["AddOD"].toDouble());
    // OEIL GAUCHE ---------------------------------------------------------------------------
    Init_Value_DoubleSpin(ui->SphereOG, MesuresRefracteur["SphereOG"].toDouble());
    Init_Value_DoubleSpin(ui->CylindreOG, MesuresRefracteur["CylOG"].toDouble());
    ui->AxeCylindreOG   ->setValue(MesuresRefracteur["AxeOG"].toInt());
    ui->AddVPOG->setValue(MesuresRefracteur["AddOG"].toDouble());
    QString AVLOD = QString::number(MesuresRefracteur["AVLOD"].toDouble()*10) + "/10";
    QString AVLOG = QString::number(MesuresRefracteur["AVLOG"].toDouble()*10) + "/10";

    switch (gMode) {
    case Refraction::Acuite:
    {
       ui->AVLODupComboBox->setCurrentText(AVLOD);
        ui->AVPODupComboBox->setCurrentText(MesuresRefracteur["AVPOD"].toString());
        ui->AVLOGupComboBox->setCurrentText(AVLOG);
        ui->AVPOGupComboBox->setCurrentText(MesuresRefracteur["AVPOG"].toString());
        ui->V2RadioButton->setChecked(MesuresRefracteur["AddOD"].toDouble()>0 || MesuresRefracteur["AddOG"].toDouble()>0);
        ui->VPRadioButton->setChecked(false);
        ui->VLRadioButton->setChecked(MesuresRefracteur["AddOD"].toDouble()==0.0 && MesuresRefracteur["AddOG"].toDouble()==0.0);
        Slot_QuelleDistance_Clicked();
        break;
    }
    case Refraction::Prescription:
        ui->V2PrescritRadioButton->setChecked(MesuresRefracteur["AddOD"].toDouble()>0 || MesuresRefracteur["AddOG"].toDouble()>0);
        Slot_VPrescritRadioButton_Clicked();
        break;
    default:
        break;
    }
    // qDebug() << "AVLOD = " + AVLOD << "AVPOD = " + MesuresRefracteur["AVPOD"].toString() << "AVLOG = " + AVLOG << "AVPOG = " + MesuresRefracteur["AVPOG"].toString();
}


/***
          http://www.progtown.com/topic344628-the-delegate-for-editing-and-qtextedit.html
 ***/
