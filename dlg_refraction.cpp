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

#include "dlg_refraction.h"
#include "icons.h"
#include "ui_dlg_refraction.h"

dlg_refraction::dlg_refraction(int *idPatAPasser, QString *NomPatient, QString *PrenomPatient, int *idActeAPasser, int *AgeAPasser,
                               Procedures *procAPasser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_refraction)
{
    ui->setupUi(this);
    proc            = procAPasser;
    gidPatient      = *idPatAPasser;
    gNomPatient     = *NomPatient;
    gPrenomPatient  = *PrenomPatient;
    gidUser         = proc->getDataUser()["idUser"].toInt();
    gidActe         = *idActeAPasser;
    gAgePatient     = *AgeAPasser;

    db = proc->getDataBase();
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    InitDivers();
    //  Connection des actions associees a chaque objet du formulaire et aux menus
    Init_variables();
    Connect_Slots();

    restoreGeometry(proc->gsettingsIni->value("PositionsFiches/PositionRefraction").toByteArray());
    // Recherche si Mesure en cours et affichage.
    if (proc->TypeMesureRefraction() != "")
    {
        if (proc->TypeMesureRefraction() == "Refracteur")
        {
            AfficheMesureRefracteur();
            proc->setTypeMesureRefraction("");
        }
        else if (proc->TypeMesureRefraction() == "Fronto")
        {
            AfficheMesureFronto();
            proc->setTypeMesureRefraction("");
        }
        else if (proc->TypeMesureRefraction() == "Autoref")
        {
            AfficheMesureAutoref();
            proc->setTypeMesureRefraction("");
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
    QList<UpDoubleSpinBox *> listdblSpin = findChildren<UpDoubleSpinBox *>();
    for (int i=0; i<listdblSpin.size(); i++){
        connect (listdblSpin.at(i),                 SIGNAL(valueChanged(double)),               this,     SLOT (Slot_Refraction_ValueChanged()));
    }
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

    if (proc->PortFronto()!=NULL || proc->PortAutoref()!=NULL || proc->PortRefracteur()!=NULL)
        connect (proc,                              SIGNAL(NouvMesureRefraction()),             this,     SLOT(Slot_NouvMesureRefraction()));
}

//--------------------------------------------------------------------------------
// Tous les Slots
//--------------------------------------------------------------------------------

//1. Les RadioButton, checkBox, combo...etc...--------------------------------------------------------------------------------
void dlg_refraction::Slot_AutorefRadioButton_Clicked()
{
    gMode = Autoref;
    AfficherLaMesure();
}
void dlg_refraction::Slot_CycloplegieCheckBox_Clicked()
{
    if (ui->RefractionRadioButton->isChecked())
        AfficherLaMesure();
}
void dlg_refraction::Slot_PorteRadioButton_Clicked()
{
    gMode = Porte;
    AfficherLaMesure();
}
void dlg_refraction::Slot_PressonCheckBox_Changed()
{
    if (gMode == Prescription) ResumePrescription();
}

void dlg_refraction::Slot_QuelleDistance_Clicked()
{
    AfficherLaMesure();
   // TabulationVersOK(); // Modif 17/04
}
void dlg_refraction::Slot_RefractionRadioButton_Clicked()
{
    gMode = Refraction;
    if(!ui->CycloplegieCheckBox->isChecked())   ui->V2RadioButton->setChecked(true);
    AfficherLaMesure();
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
    default:
        break;
    }
    // si depoli on masque les mesures
    AfficherLaMesure();
    if (gMode == Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
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
    default:
        break;
    }
    AfficherLaMesure();
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
    default:
        break;
    }
    ResumePrescription();
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
        default:
            break;
        }
    }

    AfficherLaMesure();
    if (gMode == Prescription) ResumePrescription();
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
    default:
        break;
    }
    ui->RyserSpinBox->setVisible(ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked());

    if (gMode == Prescription) ResumePrescription();
    if (ui->PrismeOD->value()>0 || ui->PrismeOG->value()>0
        || ui->DepoliODCheckBox->isChecked() || ui->DepoliOGCheckBox->isChecked()
        || ui->PlanODCheckBox->isChecked() || ui->PlanOGCheckBox->isChecked()
        || ui->RyserODCheckBox->isChecked() || ui->RyserOGCheckBox->isChecked())
        ui->DetailsPushButton->setEnabled(false);
    else
        ui->DetailsPushButton->setEnabled(true);
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
    if (gMode == Prescription) ResumePrescription();
}
void dlg_refraction::Slot_CommentairePrescriptionTextEdit_Changed()    // 01.07.2014
{
    ResumePrescription();
}
void dlg_refraction::Slot_Controle_K()
{
    QLineEdit *K = static_cast<QLineEdit *>(sender());
    K->setText(QLocale().toString(QLocale().toDouble(K->text())));
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
    if (gMode == Prescription) ResumePrescription();
}

//3. Les pushButton ----------------------------------------------------------------------------------------------
void dlg_refraction::Slot_AnnulPushButton_Clicked()
{
    FermeFiche("Annul");
}
void dlg_refraction::Slot_Commentaires()
{
    Dlg_Comments    = new dlg_commentaires(proc, this);
    if (Dlg_Comments->exec() > 0)
    {
        gResultatCommPreDef =  Dlg_Comments->gReponseResumePrescription;
        ResumePrescription();
        gResultatCommResumOrdo = Dlg_Comments->gReponseResumeDossier;
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
        (ui->PrismeOD->value() != 0          || ui->PrismeOG->value() != 0      ||
        ui->RyserODCheckBox->isChecked()      || ui->RyserOGCheckBox->isChecked()   ||
        ui->PlanODCheckBox->isChecked()     || ui->PlanOGCheckBox->isChecked()  ||
        ui->DepoliODCheckBox->isChecked()   || ui->DepoliOGCheckBox->isChecked()))
        return;
    else
        gAfficheDetail = !gAfficheDetail;
    AfficherDetail(gAfficheDetail);
    if (gMode == Porte)
    {
        if (gAfficheDetail)
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_AVEC_DETAIL);
        else
            setFixedSize(width(), HAUTEUR_SANS_ORDONNANCE_MINI);
    }
    if (gMode == Prescription)
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
        int a = dblSpin->value()/dblSpin->singleStep();
        int b = dblSpin->value()*10;
        if (a != dblSpin->value()/dblSpin->singleStep())
            if (((dblSpin->singleStep() == 0.25) && ((abs(b)%10 != 2 && abs(b)%10 != 7) || b != dblSpin->value()*10))
               || dblSpin->singleStep() == 0.50)
            {
                QSound::play(NOM_ALARME);
                dblSpin->setFocus();
                dblSpin->selectAll();
                return;
            }
    }

    if (!ControleCoherence())        return;

    if (gMode == Porte)
    {
        // On vérifie dans Refractions s'il existe un enregistrement identique au meme jour pour ne pas surcharger la table
        IdRefract = (LectureMesure("JOUR","P","","","","","",CalculFormule_OD(),CalculFormule_OG()));
        if (IdRefract == 0) // il n'y en a pas - on suit la procédure normale
            InscriptRefraction();
        FermeFiche("OK");
    }

    if (gMode == Autoref || gMode == Refraction)
    {
        // On vérifie dans Refractions s'il existe un enregistrement identique et si oui, on l'écrase
        QString Cycloplegie = "0";
        if (ui->CycloplegieCheckBox->isChecked()) Cycloplegie = "1";
        int IDMesure = LectureMesure("JOUR",QuelleMesure(),"",Cycloplegie,"","","","","");
        if (IDMesure > 0)
            // suppression de la mesure dans table Refraction
            DetruireLaMesure(IDMesure);
        InscriptRefraction();
        FermeFiche("OK");
    }
    if (gMode == Prescription)
        FermeFiche("Imprime");
}

void dlg_refraction::Slot_OupsButtonClicked()
{
    OuvrirListeMesures("SUPPR");
}
void dlg_refraction::Slot_PrescriptionRadionButton_clicked()
{
    gMode = Prescription;
    ui->ODPrescritCheckBox->setChecked(ui->ODCheckBox->isChecked());
    ui->OGPrescritCheckBox->setChecked(ui->OGCheckBox->isChecked());
    ui->VLPrescritRadioButton->setChecked(ui->VLRadioButton->isChecked());
    ui->VPPrescritRadioButton->setChecked(ui->VPRadioButton->isChecked());
    if (ui->V2RadioButton->isChecked())
    {
        if (!ui->ODCheckBox->isChecked() && ui->AddVPOG->value() == 0) ui->VLPrescritRadioButton->setChecked(true);
        else if (!ui->OGCheckBox->isChecked() && ui->AddVPOD->value() == 0) ui->VLPrescritRadioButton->setChecked(true);
        else if (ui->ODCheckBox->isChecked() && ui->OGCheckBox->isChecked() && ui->AddVPOD->value() == 0 && ui->AddVPOG->value() == 0)
            ui->VLPrescritRadioButton->setChecked(true);
        else ui->V2PrescritRadioButton->setChecked(true);
    }
    AfficherLaMesure();
}

void dlg_refraction::Slot_NouvMesureRefraction()
{
    //proc->Edit(proc->TypeMesureRefraction().split("//").at(0) + "\n" + proc->TypeMesureRefraction().split("//").at(1));
    if (proc->TypeMesureRefraction() == "Refracteur")
        AfficheMesureRefracteur();
    if (proc->TypeMesureRefraction() == "Fronto")
        AfficheMesureFronto();
    if (proc->TypeMesureRefraction().split("//").at(0) == "Autoref")
        AfficheMesureAutoref();
    proc->setTypeMesureRefraction("");
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
        if (obj->inherits("UpLineEdit"))        {
            UpLineEdit* objUpLine = static_cast<UpLineEdit*>(obj);
            objUpLine->selectAll();
            objUpLine = 0;
        }
        if (obj->inherits("UpDoubleSpinBox"))   {
            UpDoubleSpinBox* objUpdSpin = static_cast<UpDoubleSpinBox*>(obj);
            objUpdSpin->setPrefix("");
            objUpdSpin->selectAll();
            objUpdSpin = 0;
        }
        if (obj->inherits("UpComboBox"))   {
            UpComboBox* objUpCombo = static_cast<UpComboBox*>(obj);
            objUpCombo->setCurrentIndex(objUpCombo->findText(objUpCombo->currentText()));
            objUpCombo = 0;
        }
        if (obj->inherits("UpSpinBox"))   {
            UpSpinBox* objUpSpin = static_cast<UpSpinBox*>(obj);
            objUpSpin->selectAll();
            objUpSpin = 0;
        }
        QWidget *widg = dynamic_cast<QWidget *>(obj);
        if (widg!=NULL)   {
            QList<QGroupBox *> listbox = findChildren<QGroupBox *>();
            for (int i=0; i<listbox.size(); i++)    {
                if (listbox.at(i)->isAncestorOf(widg) && listbox.at(i) != ui->CommentaireGroupBox) {
                    listbox.at(i)->setStyleSheet("QGroupBox {border: 2px solid rgb(164, 205, 255); border-radius: 10px;"
                                                 "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF);}");
                    i = listbox.size();
                }
            }
        }
    }

    if (event->type() == QEvent::FocusOut )
    {
        if (obj == ui->CylindreOD)          if (ui->CylindreOD->value() == 0)   ui->AxeCylindreOD->setValue(0);
        if (obj == ui->CylindreOG)          if (ui->CylindreOG->value() == 0)   ui->AxeCylindreOG->setValue(0);
        if (obj == ui->AddVPOG) QuitteAddVP(ui->AddVPOG);
        if (obj == ui->AddVPOD) QuitteAddVP(ui->AddVPOD);
        if (obj == ui->K1OD || obj == ui->K2OD || obj == ui->K1OG || obj == ui->K2OG)
            return QWidget::eventFilter(obj, event); //traité par la classe uplineedit.h

        QWidget *widg = dynamic_cast<QWidget *>(obj);
        if (widg!=NULL)
        {
            QList<QGroupBox *> listbox = this->findChildren<QGroupBox *>();
            for (int i=0; i<listbox.size(); i++)
            {
                if (listbox.at(i)->isAncestorOf(widg) && listbox.at(i) != ui->CommentaireGroupBox  && !listbox.at(i)->isAncestorOf(focusWidget()))
                {
                    listbox.at(i)->setStyleSheet("");
                    i = listbox.size();
                }
            }
        }
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
                objUpLine = 0;
            }
        }

        if (keyEvent->key() == Qt::Key_Left  && !ui->CommentaireGroupBox->isAncestorOf(widg))
        {        // quand le focus entre sur un qGroupBox rempli de radioButton, il doit se porter sur le radiobutton qui est coché
            QGroupBox *boxdep;
            boxdep = 0;
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
            if (radio!=NULL)
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
            boxdep = 0;
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
            if (radio!=NULL)
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
                if (gMode == Porte && ui->PrismeGroupBox->isVisible())                                    return DeplaceVers(ui->PrismeGroupBox,"Fin");
                if (gMode == Porte || (gMode == Refraction && ui->QuelleDistanceGroupBox->isVisible()))   return DeplaceVers(ui->QuelleDistanceGroupBox);
                if (gMode == Refraction && !ui->QuelleDistanceGroupBox->isVisible())                      return DeplaceVers(ui->QuelleMesureGroupBox);
                if (gMode == Autoref)                                                                     return DeplaceVers(ui->KeratometrieGroupBox,"Fin");
                if (gMode == Prescription)                                                                return DeplaceVers(ui->QuelleMontureGroupBox);
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
                    if (gMode == Porte) return DeplaceVers(ui->QuelleDistanceGroupBox);
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
                if (gMode == Porte || (gMode == Refraction && ui->QuelleDistanceGroupBox->isVisible()))    return DeplaceVers(ui->QuelleDistanceGroupBox);
                if (gMode == Refraction && !ui->QuelleDistanceGroupBox->isVisible())                       return DeplaceVers(ui->MesureGroupBox,"Debut");
                if (gMode == Autoref)                                                                      return DeplaceVers(ui->KeratometrieGroupBox,"Debut");
                if (gMode == Prescription)
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
        FermeFiche("Annul");
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

    AVLOD->installEventFilter(this);
    AVPOD->installEventFilter(this);
    ui->AVPODupComboBox->installEventFilter(this);
    ui->AVLODupComboBox->installEventFilter(this);
    AVLOG->installEventFilter(this);
    AVPOG->installEventFilter(this);
    ui->AVPOGupComboBox->installEventFilter(this);
    ui->AVLOGupComboBox->installEventFilter(this);

    ui->V2RadioButton->installEventFilter(this);
    ui->VLRadioButton->installEventFilter(this);
    ui->VPRadioButton->installEventFilter(this);

    //ui->QuelleDistanceGroupBox->installEventFilter(this);
    ui->CommentairePrescriptionTextEdit->installEventFilter(this);
    ui->VPPrescritRadioButton->installEventFilter(this);
    ui->VLPrescritRadioButton->installEventFilter(this);
    ui->V2PrescritRadioButton->installEventFilter(this);
    ui->VerresTeintesCheckBox->installEventFilter(this);
    ui->UneMonturePrescritRadioButton->installEventFilter(this);
    ui->DeuxMonturesPrescritRadioButton->installEventFilter(this);

    ui->PorteRadioButton->installEventFilter(this);
    ui->AutorefRadioButton->installEventFilter(this);
    ui->PrescriptionRadioButton->installEventFilter(this);
    ui->RefractionRadioButton->installEventFilter(this);

    //ui->QuelleMesureGroupBox->installEventFilter(this); // ????
    ui->OKPushButton->installEventFilter(this); // ????
    ui->AnnulPushButton->installEventFilter(this); // ????
    ui->AppelCommentPushButton->installEventFilter(this); // ????

    ui->ODPrescritCheckBox->installEventFilter(this);
    ui->OGPrescritCheckBox->installEventFilter(this);
    ui->DateDateEdit->installEventFilter(this);
    ui->BasePrismeTextODComboBox->installEventFilter(this);
    ui->PressonODCheckBox->installEventFilter(this);
    ui->AxeKOD->installEventFilter(this);
    ui->K1OD->installEventFilter(this);
    ui->K2OD->installEventFilter(this);
    ui->BasePrismeTextOGComboBox->installEventFilter(this);
    ui->PressonOGCheckBox->installEventFilter(this);
    ui->AxeKOG->installEventFilter(this);
    ui->K1OG->installEventFilter(this);
    ui->K2OG->installEventFilter(this);
}

//----------------------------------------------------------------------------------
// Initialisation des variables
//----------------------------------------------------------------------------------
void dlg_refraction::Init_variables()
{
    gMode                   = Porte;
    gAfficheDetail          = false;

    ui->DateDateEdit        ->setDate(QDate::currentDate());
    gResultatCommPreDef     = InsertCommentaireObligatoire();
    gResultatCommResumOrdo  = "";
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
                (ui->PrismeOD->value() != 0          || ui->PrismeOG->value() != 0      ||
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

//---------------------------------------------------------------------------------
// Masque ou pas les champs en fonction de type de mesure
//---------------------------------------------------------------------------------
void dlg_refraction::AfficherLaMesure()
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
    if (gMode == Porte)      // mode Porte
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
            (ui->PrismeOD->value() != 0          || ui->PrismeOG->value() != 0          ||
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

    if (gMode == Autoref)    // mode AutoRef
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

    if (gMode == Refraction) // mode Refraction
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

    if (gMode == Prescription)   // mode Prescription
    {
        Afficher_AVL_AVP(false);
        ui->OKPushButton->setIcon(Icons::icImprimer());
        ui->OKPushButton->setText( tr("Imprimer"));
        ui->CycloplegieCheckBox->setVisible(false);
        ui->KeratometrieGroupBox->setVisible(false);
        ui->ODCheckBox->setVisible(false);
        ui->OGCheckBox->setVisible(false);
        gAfficheDetail =
            (ui->PrismeOD->value() != 0          || ui->PrismeOG->value() != 0      ||
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
    QString LocalRequete = "SELECT  idRefraction FROM " NOM_TABLE_REFRACTION " WHERE  idPat = " + QString::number(gidPatient) ;
    QSqlQuery CompteRefractionsQuery (LocalRequete,db);
    proc->TraiteErreurRequete(CompteRefractionsQuery,LocalRequete,"");
    if (CompteRefractionsQuery.size() == 0)
    {
        ui->OupsPushButton->setEnabled(false);
        ui->ReprendrePushButton->setEnabled(false);
        ui->ResumePushButton->setEnabled(false);
    }
    else
    {
        ui->OupsPushButton->setEnabled(true);
        ui->ReprendrePushButton->setEnabled(true);
        ui->ResumePushButton->setEnabled(true);
    }
    MasquerObjetsOeilDecoche();
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
    if (gMode == Porte || gMode == Prescription)
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
    if (gMode == Prescription && ui->VPPrescritRadioButton->isChecked())
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
    if (gMode == Refraction)
    {
        if (ui->VLRadioButton->isChecked())
            ResultatOG = ResultatOGVL + " " + AVLOG->text() + "" ;
        else
            ResultatOG = ResultatOGVL + " " + AVLOG->text() + " P" + AVPOG->text() + " " + ResultatOGVP ;
        return ResultatOG ;
    }
    if (gMode == Prescription)
    {
        if (ui->VLPrescritRadioButton->isChecked())
            return ResultatOGVL + " " + OGPrisme ;
        return ResultatOG + " " + OGPrisme ;
    }
    if (gMode == Autoref)
        return ResultatOGVL;
    if (gMode == Porte)
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
    if (gMode == Porte || gMode == Prescription)
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
    if (gMode == Prescription && ui->VPPrescritRadioButton->isChecked())
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
    if (gMode == Refraction)
    {
        if (ui->VLRadioButton->isChecked())
            ResultatOD = ResultatODVL + " " + AVLOD->text() + "" ;
        else
            ResultatOD = ResultatODVL + " " + AVLOD->text() + " P" + AVPOD->text() + " " + ResultatODVP ;
        return ResultatOD ;
    }
    if (gMode == Prescription)
    {
        if (ui->VLPrescritRadioButton->isChecked())
            return ResultatODVL + " " + ODPrisme ;
        return ResultatOD + " " + ODPrisme ;
    }
    if (gMode == Autoref)
        return ResultatODVL;
    if (gMode == Porte)
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

//---------------------------------------------------------------------------------
// Deplacement du curseur sur un des GroupBox
//---------------------------------------------------------------------------------
bool dlg_refraction::DeplaceVers(QWidget *widg, QString FinOuDebut)
{
    if (widg == ui->QuelleDistanceGroupBox)
    {
        if (ui->VLRadioButton->isChecked() == true)     {ui->VLRadioButton->setFocus();     return true;}
        if (ui->VPRadioButton->isChecked() == true)     {ui->VPRadioButton->setFocus();     return true;}
        if (ui->V2RadioButton->isChecked() == true)     {ui->V2RadioButton->setFocus();     return true;}
        ui->V2RadioButton->setFocus();
        ui->V2RadioButton->setChecked(true);
        Slot_QuelleDistance_Clicked();
        return true;
    }
    if (widg == ui->KeratometrieGroupBox)
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
    if (widg == ui->PrismeGroupBox)
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
    if (widg == ui->MesureGroupBox)
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
    if (widg == ui->QuelleMesureGroupBox)
    {
        if (ui->PorteRadioButton->isChecked() == true)          {ui->PorteRadioButton->setFocus();          return true;}
        if (ui->AutorefRadioButton->isChecked() == true)        {ui->AutorefRadioButton->setFocus();        return true;}
        if (ui->RefractionRadioButton->isChecked() == true)     {ui->RefractionRadioButton->setFocus();     return true;}
        if (ui->PrescriptionRadioButton->isChecked() == true)   {ui->PrescriptionRadioButton->setFocus();   return true;}
        return true;
    }
    if (widg == ui->QuelleMontureGroupBox)
    {
        if (ui->UneMonturePrescritRadioButton->isChecked() == true)          {ui->UneMonturePrescritRadioButton->setFocus();          return true;}
        if (ui->DeuxMonturesPrescritRadioButton->isChecked() == true)        {ui->DeuxMonturesPrescritRadioButton->setFocus();          return true;}
        return true;
    }
    if (widg == ui->QuelOeilGroupBox)
    {
        if (ui->ODPrescritCheckBox->isEnabled())
            ui->ODPrescritCheckBox->setFocus();
        else
            ui->OGPrescritCheckBox->setFocus();
        return true;
    }
    if (widg == ui->QuelsVerresGroupBox)
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
int dlg_refraction::DetruireLaMesure(int IdRefract)
{
//1. On supprime la mesure de la table
    QString requete = "DELETE  FROM " NOM_TABLE_REFRACTION " WHERE  idRefraction = " + QString::number(IdRefract);
    QSqlQuery DetruireMesureQuery (requete,db);
    proc->TraiteErreurRequete(DetruireMesureQuery,requete, "Impossible de supprimer la mesure");

//2. s'il n'y a plus de mesures pour le patient => on cache les boutons Oups, Resume et Reprendre
    QString LocalRequete = "SELECT  idRefraction"
              " FROM " NOM_TABLE_REFRACTION
              " WHERE  idPat = " + QString::number(gidPatient) ;
    QSqlQuery   CompteRefractionsQuery (LocalRequete,db);
    proc->TraiteErreurRequete(CompteRefractionsQuery,LocalRequete,"");
    if (CompteRefractionsQuery.size() == 0)
    {
        ui->OupsPushButton->setEnabled(false);
        ui->ReprendrePushButton->setEnabled(false);
        ui->ResumePushButton->setEnabled(false);
    }
    else
    {
        ui->OupsPushButton->setEnabled(true);
        ui->ReprendrePushButton->setEnabled(true);
        ui->ResumePushButton->setEnabled(true);
    }
    return 0;
}

//--------------------------------------------------------------------------------
// Click sur OK ou ANNULER >> Fermer la fiche refraction
//--------------------------------------------------------------------------------
void dlg_refraction::FermeFiche(QString ModeSortie)
{
    EscapeFlag = false;
    if (ModeSortie == "Annul")
    {
        FermeComment = false;
        QList<UpDoubleSpinBox *> dblSpinList = findChildren<UpDoubleSpinBox *>();
        for (int i=1; i<dblSpinList.size(); i++)
            dblSpinList.at(i)->setAutorCorrigeDioptr(false);
        QList<QDialog *> ListDialog = findChildren<QDialog *>();
        for (int n = 0; n <  ListDialog.size(); n++)
            ListDialog.at(n)->close();
        close();
        return;
    }
    if (ModeSortie == "OK")  ResumeObservation();
    if (ModeSortie == "Imprime")
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
            if (LectureMesure("JOUR","O","","","","","",CalculFormule_OD(),CalculFormule_OG()) == 0)
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
    FermeComment = true;
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
    proc->setDataOtherUser(proc->getDataUser()["UserSuperviseur"].toInt());
    Entete = proc->ImpressionEntete(ui->DateDateEdit->date()).value("Norm");
    if (Entete == "") return false;
    Entete.replace("{{TITRE1}}"            , "");
    Entete.replace("{{TITRE}}"             , "");
    Entete.replace("{{PRENOM PATIENT}}"    , gPrenomPatient);
    Entete.replace("{{NOM PATIENT}}"       , gNomPatient.toUpper());
    Entete.replace("{{DDN}}"               , "");

    // création du pied
    Pied = proc->ImpressionPied(true);
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
        QSqlQuery query = QSqlQuery(db);
        // on doit passer par les bindvalue pour incorporer le bytearray dans la requête
        query.prepare("insert into " NOM_TABLE_IMPRESSIONS " (idUser, idpat, TypeDoc, SousTypeDoc, Titre, TextEntete, TextCorps, TextOrigine, TextPied, Dateimpression, UserEmetteur, ALD, EmisRecu, FormatDoc, idLieu)"
                                                           " values(:iduser, :idpat, :typeDoc, :soustypedoc, :titre, :textEntete, :textCorps, :textorigine, :textPied, :dateimpression, :useremetteur, :ald, :emisrecu, :formatdoc, :idlieu)");
        query.bindValue(":iduser", QString::number(gidUser));
        query.bindValue(":idpat", QString::number(gidPatient));
        query.bindValue(":typeDoc", "Prescription");
        query.bindValue(":soustypedoc", "Correction");
        query.bindValue(":titre", "Prescription correction");
        query.bindValue(":textEntete", Entete);
        query.bindValue(":textCorps", Corps);
        query.bindValue(":textorigine", ui->ResumePrescriptionTextEdit->toPlainText());
        query.bindValue(":textPied", Pied);
        query.bindValue(":dateimpression", ui->DateDateEdit->date().toString("yyyy-MM-dd") + " " + QTime::currentTime().toString("HH:mm:ss"));
        query.bindValue(":useremetteur", QString::number(gidUser));
        query.bindValue(":ald", QVariant(QVariant::String));
        query.bindValue(":emisrecu", "0");
        query.bindValue(":formatdoc", PRESCRIPTIONLUNETTES);
        query.bindValue(":idlieu", proc->getDataUser()["idLieu"].toString());

        if(!query.exec())
            UpMessageBox::Watch(this, tr("Impossible d'enregistrer ce document dans la base!"));
    }
    delete Etat_textEdit;
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

    reg = QRegExp(tr("(zeroPL|PLnonO|PLO|VBLM|CLD 50cm|CLD 1m|0,5/10|0\\.5/10|1/10|1,6/10|1\\.6/10|2/10|2\\.5/10|2,5/10|3/10|4/10|5/10|6/10|7/10|8/10|9/10|10/10|12/10)"));
    AVLOD->setValidator(new QRegExpValidator(reg,this));
    AVLOG->setValidator(new QRegExpValidator(reg,this));

    ui->AVPODupComboBox->setLineEdit(AVPOD);
    ui->AVPOGupComboBox->setLineEdit(AVPOG);
    ui->AVLODupComboBox->setLineEdit(AVLOD);
    ui->AVLOGupComboBox->setLineEdit(AVLOG);

    QStringList listVP, listVL;
    listVP << "1.5" << "2" << "3" << "4" << "5" << "6" << "8" << "10" << "14" << "28" << "<28";
    listVL << "12/10" << "10/10" << "9/10" << "8/10" << "7/10" << "6/10" << "5/10" << "4/10" << "3/10" << "2.5/10" << "2/10" << "1.6/10" << "1/10" << "0.5/10" << tr("CLD 1m") << tr("CLD 50cm") << tr("VBLM") << tr("PLO") << tr("PLnonO") << tr("zeroPL");
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

    PrefixePlus(ui->SphereOD);               // 11-07-2014
    PrefixePlus(ui->SphereOG);               // 11-07-2014
    PrefixePlus(ui->CylindreOD);             // 11-07-2014
    PrefixePlus(ui->CylindreOG);             // 11-07-2014
    PrefixePlus(ui->AddVPOD);                // 11-07-2014
    PrefixePlus(ui->AddVPOG);                // 11-07-2014
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
    if ((gMode == Autoref || gMode == Refraction) && a)
        MajDonneesOphtaPatient();
    if (gMode == Prescription && a)
    {
        QString req = "select max(idrefraction) from " NOM_TABLE_REFRACTION " where idpat = " + QString::number(gidPatient);
        QSqlQuery quer(req,db);
        if (quer.size()>0)
        {
            quer.first();
            gidRefraction = quer.value(0).toInt();
            req = "select max(idimpression) from " NOM_TABLE_IMPRESSIONS " where idpat = " + QString::number(gidPatient);
            QSqlQuery quer1(req,db);
            if (quer1.size()>0)
            {
                quer1.first();
                int idimp = quer1.value(0).toInt();
                QSqlQuery ("update " NOM_TABLE_IMPRESSIONS " set idRefraction = " + QString::number(gidRefraction) + " where idimpression = " + QString::number(idimp),db);
            }
        }
    }
}

QString dlg_refraction::InsertCommentaireObligatoire()
{
    QString rep ("");
    QString req = "SELECT TextComment"
                  " FROM "  NOM_TABLE_COMMENTAIRESLUNETTES
                  " WHERE idUser = " + QString::number(gidUser) +
                  " and ParDefautComment = 1"
                  " ORDER BY ResumeComment";
    QSqlQuery quer(req,db);
    if (quer.size()>0)
        for (int i=0; i<quer.size(); i++)
        {
            quer.seek(i);
            rep += quer.value(0).toString();
            if (i<quer.size()-1)
                rep += "\n";
        }
    return rep;
}

//---------------------------------------------------------------------------------
// Creation d'un nouvel enregistrement dans DonneesOphtaPatient
//---------------------------------------------------------------------------------
void dlg_refraction::InsertDonneesOphtaPatient()
{
   QString requete = "INSERT INTO " NOM_TABLE_DONNEES_OPHTA_PATIENTS
              " (idPat, QuelleMesure, QuelleDistance, K1OD, K2OD, AxeKOD, K1OG, K2OG, AxeKOG, OrigineK, DateK, "
              " SphereOD, CylindreOD, AxeCylindreOD, AVLOD, AddVPOD, AVPOD, DateRefOD, "
              " SphereOG, CylindreOG, AxeCylindreOG, AVLOG, AddVPOG, AVPOG, DateRefOG) "
              " VALUES "
              " (:idPat, :QuelleMesure, :QuelleDistance, :K1OD, :K2OD, :AxeKOD, :K1OG, :K2OG, :AxeKOG, :OrigineK, :DateK, "
              " :SphereOD, :CylindreOD, :AxeCylindreOD, :AVLOD, :AddVPOD, :AVPOD, :DateRefOD, "
              " :SphereOG, :CylindreOG, :AxeCylindreOG, :AVLOG, :AddVPOG, :AVPOG, :DateRefOG) ";


   gstringListe1.clear();
   gstringListe2.clear();

   gstringListe1 << ":idPat";
   gstringListe2 << QString::number(gidPatient);
   gstringListe1 << ":QuelleMesure";
   gstringListe2 << QuelleMesure();
   gstringListe1 << ":QuelleDistance";
   gstringListe2 << QuelleDistance();
   if ((ConvDouble(ui->K1OD->text()) > 0 || ConvDouble(ui->K2OD->text()) > 0) && ui->ODCheckBox->isChecked()) // 16-07-2014
   {
       gstringListe1 << ":K1OD";
       gstringListe2 << QString::number(QLocale().toDouble(ui->K1OD->text()),'f',2);
       gstringListe1 << ":K2OD";
       gstringListe2 << QString::number(QLocale().toDouble(ui->K2OD->text()),'f',2);
       gstringListe1 << ":AxeKOD";
       gstringListe2 << ui->AxeKOD->text();
   }
   if ((ConvDouble(ui->K1OG->text()) > 0 || ConvDouble(ui->K2OG->text()) > 0) && ui->OGCheckBox->isChecked())  // 16-07-2014
   {
       gstringListe1 << ":K1OG";
       gstringListe2 << QString::number(QLocale().toDouble(ui->K1OG->text()),'f',2);
       gstringListe1 << ":K2OG";
       gstringListe2 << QString::number(QLocale().toDouble(ui->K2OG->text()),'f',2);
       gstringListe1 << ":AxeKOG";
       gstringListe2 << ui->AxeKOG->text();
   }
   if (ConvDouble(ui->K1OD->text()) > 0 || ConvDouble(ui->K2OD->text()) > 0 || // 16-07-2014
       ConvDouble(ui->K1OG->text()) > 0 || ConvDouble(ui->K2OG->text()) > 0)   // 16-07-2014
   {
       gstringListe1 << ":OrigineK";
       gstringListe2 << QuelleMesure();
       gstringListe1 << ":DateK";
       gstringListe2 << ui->DateDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss");
   }

   if (ui->ODCheckBox->isChecked())
   {
       gstringListe1 << ":SphereOD";
       gstringListe2 << QString::number(ui->SphereOD->value());
       if (ui->CylindreOD->value() != 0)
       {
           gstringListe1 << ":CylindreOD";
           gstringListe2 << QString::number(ui->CylindreOD->value());
           gstringListe1 << ":AxeCylindreOD";
           gstringListe2 << QString::number(ui->AxeCylindreOD->value());
       }
       if (gMode == Refraction)
       {
           gstringListe1 << ":AVLOD";
           gstringListe2 << AVLOD->text();
       }
       if (ui->AddVPOD->value() > 0  && gMode == Refraction)
       {
           gstringListe1 << ":AddVPOD";
           gstringListe2 << QString::number(ui->AddVPOD->value());
       }
       if (gMode == Refraction  && !ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
       {
           gstringListe1 << ":AVPOD";
           gstringListe2 << AVPOD->text();
       }
       gstringListe1 << ":DateRefOD";
       gstringListe2 << ui->DateDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss");
   }
   if (ui->OGCheckBox->isChecked())
   {
       gstringListe1 << ":SphereOG";
       gstringListe2 << QString::number(ui->SphereOG->value());
       if (ui->CylindreOG->value() != 0)
       {
           gstringListe1 << ":CylindreOG";
           gstringListe2 << QString::number(ui->CylindreOG->value());
           gstringListe1 << ":AxeCylindreOG";
           gstringListe2 << QString::number(ui->AxeCylindreOG->value());
       }
       if (gMode == Refraction)
       {
           gstringListe1 << ":AVLOG";
           gstringListe2 << AVLOG->text();
       }
       if (ui->AddVPOG->value() > 0 && gMode == Refraction)
       {
           gstringListe1 << ":AddVPOG";
           gstringListe2 << QString::number(ui->AddVPOG->value());
       }
       if (gMode == Refraction  && !ui->CycloplegieCheckBox->isChecked() && ui->V2RadioButton->isChecked())
       {
           gstringListe1 << ":AVPOG";
           gstringListe2 << AVPOG->text();
       }
       gstringListe1 << ":DateRefOG";
       gstringListe2 << ui->DateDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss");
   }


     QSqlQuery InsertDonneesOphtaQuery (db);
   InsertDonneesOphtaQuery.prepare(requete);
   for (int i = 0; i< gstringListe1.size(); i++)
   {
       InsertDonneesOphtaQuery.bindValue(gstringListe1.at(i), gstringListe2.at(i));
   }
   InsertDonneesOphtaQuery.exec();
   if (proc->TraiteErreurRequete(InsertDonneesOphtaQuery,requete, tr("Erreur de MAJ dans ")+ NOM_TABLE_DONNEES_OPHTA_PATIENTS))
       return;
}

//---------------------------------------------------------------------------------
// Enregistre la mesure qui vient d'être validée dans la table Refractions
//---------------------------------------------------------------------------------
bool dlg_refraction::InsertRefraction()
{
    QString requete = "INSERT INTO " NOM_TABLE_REFRACTION
              " (idPat, idActe, DateRefraction, QuelleMesure, QuelleDistance, Cycloplegie, ODcoche,"
              " SphereOD, CylindreOD, AxeCylindreOD, AVLOD, AddVPOD, AVPOD, PrismeOD, BasePrismeOD,"
              " BasePrismeTextOD, PressOnOD, DepoliOD, PlanOD, RyserOD, FormuleOD,"
              " OGcoche, SphereOG, CylindreOG, AxeCylindreOG, AVLOG, AddVPOG, AVPOG, PrismeOG, BasePrismeOG, "
              " BasePrismeTextOG, PressOnOG, DepoliOG, PlanOG, RyserOG, FormuleOG, "
              " CommentaireOrdoLunettes, QuelsVerres, QuelOeil, Monture, VerreTeinte, PrimKeyDocMed) "
              " VALUES "
              " (:idPat, :idActe, :DateRefraction, :QuelleMesure, :QuelleDistance, :Cycloplegie,:ODcoche,"
              " :SphereOD, :CylindreOD, :AxeCylindreOD, :AVLOD, :AddVPOD, :AVPOD, :PrismeOD, :BasePrismeOD,"
              " :BasePrismeTextOD, :PressOnOD, :DepoliOD, :PlanOD, :RyserOD, :FormuleOD, "
              " :OGcoche, :SphereOG, :CylindreOG, :AxeCylindreOG, :AVLOG, :AddVPOG, :AVPOG, :PrismeOG, :BasePrismeOG, "
              " :BasePrismeTextOG, :PressOnOG, :DepoliOG, :PlanOG, :RyserOG, :FormuleOG, "
              " :CommentaireOrdoLunettes, :QuelsVerres, :QuelOeil, :Monture, :VerreTeinte, :PrimKeyDocMed) ";

    gstringListe1.clear();
    gstringListe2.clear();

    gstringListe1 << ":idPat";
    gstringListe2 << QString::number(gidPatient);
    gstringListe1 << ":idActe";
    gstringListe2 << QString::number(gidActe);
    gstringListe1 << ":DateRefraction";
    gstringListe2 << ui->DateDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    gstringListe1 << ":QuelleMesure";
    gstringListe2 << QuelleMesure();
    if(QuelleMesure() != "A")
    {
        gstringListe1 << ":QuelleDistance";
        gstringListe2 << QuelleDistance();
    }
    if(QuelleMesure() == "A" || QuelleMesure() == "R")
    {
        gstringListe1 << ":Cycloplegie";
        if (ui->CycloplegieCheckBox->isChecked())
            gstringListe2 << "1";
        else
            gstringListe2 << "0";
    }
    gstringListe1 << ":ODcoche";
    if (ui->ODCheckBox->isChecked())
        gstringListe2 << "1";
    else
        gstringListe2 << "0";
    if(ui->ODCheckBox->isChecked())
    {
        gstringListe1 << ":SphereOD";
        gstringListe2 << QString::number(ui->SphereOD->value());
        if (ui->CylindreOD->value() != 0)
        {
            gstringListe1 << ":CylindreOD";
            gstringListe2 << QString::number(ui->CylindreOD->value());
            gstringListe1 << ":AxeCylindreOD";
            gstringListe2 << QString::number(ui->AxeCylindreOD->value());
        }
    }
    if(AVLOD->isVisible())
    {
        gstringListe1 << ":AVLOD";
        gstringListe2 << AVLOD->text();
    }
    if(ui->AddVPOD->isVisible())
    {
        gstringListe1 << ":AddVPOD";
        gstringListe2 << QString::number(ui->AddVPOD->value());
    }
    if(AVPOD->isVisible())
    {
        gstringListe1 << ":AVPOD";
        gstringListe2 << AVPOD->text().replace(",",".");
    }
    if(ui->PrismeOD->isVisible() && ui->PrismeOD->text().toDouble() > 0)
    {
        gstringListe1 << ":PrismeOD";
        gstringListe2 << QString::number(ui->PrismeOD->value());
        gstringListe1 << ":BasePrismeOD";
        gstringListe2 << QString::number(ui->BasePrismeOD->value());
        gstringListe1 << ":BasePrismeTextOD";
        gstringListe2 << ui->BasePrismeTextODComboBox->currentText();
        gstringListe1 << ":PressOnOD";
        if (ui->PressonODCheckBox->isChecked())
            gstringListe2 << "1";
        else
            gstringListe2 << "0";
    }
    gstringListe1 << ":DepoliOD";
    if (ui->DepoliODCheckBox->isChecked())
        gstringListe2 << "1";
    else
        gstringListe2 << "0";
    gstringListe1 << ":PlanOD";
    if (ui->PlanODCheckBox->isChecked())
        gstringListe2 << "1";
    else
        gstringListe2 << "0";
    if(ui->RyserODCheckBox->isChecked())
    {
        gstringListe1 << ":RyserOD";
        gstringListe2 << QString::number(ui->RyserSpinBox->value());
    }
    gstringListe1 << ":FormuleOD";
    gstringListe2 << CalculFormule_OD();

    gstringListe1 << ":OGcoche";
    if (ui->OGCheckBox->isChecked())
        gstringListe2 << "1";
    else
        gstringListe2 << "0";
    if(ui->OGCheckBox->isChecked())
    {
        gstringListe1 << ":SphereOG";
        gstringListe2 << QString::number(ui->SphereOG->value());
        if (ui->CylindreOG->value() != 0)
        {
            gstringListe1 << ":CylindreOG";
            gstringListe2 << QString::number(ui->CylindreOG->value());
            gstringListe1 << ":AxeCylindreOG";
            gstringListe2 << QString::number(ui->AxeCylindreOG->value());
        }
    }
    if(AVLOG->isVisible())
    {
        gstringListe1 << ":AVLOG";
        gstringListe2 << AVLOG->text();
    }
    if(ui->AddVPOG->isVisible())
    {
        gstringListe1 << ":AddVPOG";
        gstringListe2 << QString::number(ui->AddVPOG->value());
    }
    if(AVPOG->isVisible())
    {
        gstringListe1 << ":AVPOG";
        gstringListe2 << AVPOG->text().replace(",",".");
    }
    if(ui->PrismeOG->isVisible() && ui->PrismeOG->text().toDouble() > 0)
    {
        gstringListe1 << ":PrismeOG";
        gstringListe2 << QString::number(ui->PrismeOG->value());
        gstringListe1 << ":BasePrismeOG";
        gstringListe2 << QString::number(ui->BasePrismeOG->value());
        gstringListe1 << ":BasePrismeTextOG";
        gstringListe2 << ui->BasePrismeTextOGComboBox->currentText();
        gstringListe1 << ":PressOnOG";
        if (ui->PressonOGCheckBox->isChecked())
            gstringListe2 << "1";
        else
            gstringListe2 << "0";
    }
    gstringListe1 << ":DepoliOG";
    if (ui->DepoliOGCheckBox->isChecked())
        gstringListe2 << "1";
    else
        gstringListe2 << "0";
    gstringListe1 << ":PlanOG";
    if (ui->PlanOGCheckBox->isChecked())
        gstringListe2 << "1";
    else
        gstringListe2 << "0";
    if(ui->RyserOGCheckBox->isChecked())
    {
        gstringListe1 << ":RyserOG";
        gstringListe2 << QString::number(ui->RyserSpinBox->value());
    }
    gstringListe1 << ":FormuleOG";
    gstringListe2 << CalculFormule_OG();
   if(QuelleMesure() == "O")
   {
       gstringListe1 << ":CommentaireOrdoLunettes";
       gstringListe2 << CalculCommentaire();
       gstringListe1 << ":QuelsVerres";
       gstringListe2 << QuelsVerres();
       gstringListe1 << ":QuelOeil";
       gstringListe2 << QuelsYeux();
       gstringListe1 << ":Monture";
       gstringListe2 << QuelleMonture();
       gstringListe1 << ":VerreTeinte";
       if (ui->VerresTeintesCheckBox->isChecked())
           gstringListe2 << "1";
       else
           gstringListe2 << "0";
    }
    gstringListe1 << ":PrimKeyDocMed";
    gstringListe2 << QString::number(gidPatient);

    QSqlQuery InsertRefractionQuery (db);
    InsertRefractionQuery.prepare(requete);
    for (int i = 0; i< gstringListe1.size(); i++)
        InsertRefractionQuery.bindValue(gstringListe1.at(i), gstringListe2.at(i));
    InsertRefractionQuery.exec();
    return !proc->TraiteErreurRequete(InsertRefractionQuery,requete, tr("Erreur de création dans ") + NOM_TABLE_REFRACTION);
}

//---------------------------------------------------------------------------------
// Lecture d'une mesure en base
// Quand = JOUR - AVANT
// Mesure = P - A - R - O
// TypLun = L - P - 2
// Cycloplegie = true - false
// IdRefraction = N° Enregistrement a lire
// Affichage = QM remplir ou pas le formulaire avec la mesure trouvee
// OeilCoche = Pour un oeil particulier
//---------------------------------------------------------------------------------
int dlg_refraction::LectureMesure(QString Quand, QString Mesure, QString TypLun, QString Cycloplegie, QString IdRefraction, QString Affichage, QString OeilCoche, QString FormuleOD, QString FormuleOG)
{
    QString a;
    QString requete = "SELECT  idRefraction, idPat, DateRefraction, QuelleMesure, QuelleDistance, "           // 0-1-2-3-4
            " Cycloplegie, ODcoche, SphereOD, CylindreOD, AxeCylindreOD, AVLOD, "                   // 5-6-7-8-9-10
            " AddVPOD, AVPOD, PrismeOD, BasePrismeOD, BasePrismeTextOD, PressOnOD,"                 // 11-12-13-14-15-16
            " DepoliOD, PlanOD, RyserOD, FormuleOD, OGcoche, SphereOG, CylindreOG,"                 // 17-18-19-20-21-22-23
            " AxeCylindreOG, AVLOG, AddVPOG, AVPOG, PrismeOG, BasePrismeOG, "                       // 24-25-26-27-28-29
            " BasePrismeTextOG, PressOnOG, DepoliOG, PlanOG, RyserOG, FormuleOG, "                  // 30-31-32-34-35
            " CommentaireOrdoLunettes, QuelsVerres, QuelOeil, Monture, VerreTeinte, PrimKeyDocMed"  // 36-37-38-39-40-41
            " FROM " NOM_TABLE_REFRACTION ;

    // On relit la mesure après selection dans la liste mesure (reprendre)
    if (IdRefraction.length() > 0)
        requete += " WHERE idRefraction = "    + IdRefraction ;
    else
        // fabrication des criteres de recherche selon le cas de lecture
    {
        requete += " WHERE  IdPat = " + QString::number(gidPatient) ;
        if (Quand == "JOUR")
            requete += " AND DateRefraction = '" + QDate::currentDate().toString("yyyy-MM-dd") + "'";
        if (Quand == "AVANT")
            requete += " AND DateRefraction < '" + QDate::currentDate().toString("yyyy-MM-dd") + "'";
        if (Mesure.length() > 0)
            requete += " AND QuelleMesure = '"   + Mesure + "'";
        if (TypLun.length() > 0)
            requete += " AND QuelleDistance = '" + TypLun + "'";
        if (Cycloplegie.length() > 0)
            requete += " AND Cycloplegie =  "    + Cycloplegie ;
        if (OeilCoche.length() > 0)
            requete += OeilCoche;
        if (FormuleOD.length() > 0)                                  // 10-07-2014
            requete += " AND FormuleOD =  '"    + FormuleOD + "'";
        if (FormuleOG.length() > 0)                                  // 10-07-2014
            requete += " AND FormuleOG =  '"    + FormuleOG + "'";
    }

    // on ajoute un tri sur la date, du plus rescent au plus ancien.
    requete += " ORDER BY DateRefraction, idRefraction";

    QSqlQuery LectureMesureQuery (requete,db);
    if (proc->TraiteErreurRequete(LectureMesureQuery,requete,"Impossible d'accéder à la liste table des mesures!"))
        return 0;
    if (LectureMesureQuery.size() == 0) return 0;
    LectureMesureQuery.last();

    if (Affichage != "")
    {
        if (Affichage != "QUNOEIL")
        {
            if (LectureMesureQuery.value(3).toString() == "P")  gMode = Porte;                                              // QuelleMesure
            if (LectureMesureQuery.value(3).toString() == "A")  gMode = Autoref;                                            // QuelleMesure
            if (LectureMesureQuery.value(3).toString() == "R")  gMode = Refraction;                                         // QuelleMesure
            if (LectureMesureQuery.value(3).toString() == "O")  gMode = Prescription;                                       // QuelleMesure
            if (LectureMesureQuery.value(4).toString() == "L")   ui->VLRadioButton->setChecked(true);                       // QuelleDistance
            if (LectureMesureQuery.value(4).toString() == "P")   ui->VPRadioButton->setChecked(true);                       // QuelleDistance
            if (LectureMesureQuery.value(4).toString() == "2")   ui->V2RadioButton->setChecked(true);                       // QuelleDistance
            ui->CycloplegieCheckBox->setChecked(LectureMesureQuery.value(5).toBool());                                      // Cycloplegie

            ui->CommentairePrescriptionTextEdit->setPlainText(LectureMesureQuery.value(36).toString());                     // CommentaireOrdoLunettes
            if (LectureMesureQuery.value(37).toString() == "L")   ui->VLPrescritRadioButton->setChecked(true);              // QuelsVerres
            if (LectureMesureQuery.value(37).toString() == "P")   ui->VPPrescritRadioButton->setChecked(true);              // QuelsVerres
            if (LectureMesureQuery.value(37).toString() == "2")   ui->V2PrescritRadioButton->setChecked(true);              // QuelsVerres
            if (LectureMesureQuery.value(38).toString() == "D")   ui->OGPrescritCheckBox->setChecked(false);                // QuelsYeux
            if (LectureMesureQuery.value(38).toString() == "G")   ui->ODPrescritCheckBox->setChecked(false);                // QuelsYeux
            if (LectureMesureQuery.value(39).toString() == "2")   ui->DeuxMonturesPrescritRadioButton->setChecked(true);    // Monture
            if (LectureMesureQuery.value(39).toString() == "1")   ui->UneMonturePrescritRadioButton->setChecked(true);      // Monture
            ui->VerresTeintesCheckBox->setChecked(LectureMesureQuery.value(40).toBool());                                   // VerreTeinte
        } // fin affiche non partiel

        // Remplissage des champs Oeil Droit
        if (LectureMesureQuery.value(6).toInt() == 1)
            ui->ODCheckBox->setChecked(true);                                                               // ODcoche
        if (ui->ODCheckBox->isChecked())
        {
            Init_Value_DoubleSpin(ui->SphereOD,  LectureMesureQuery.value(7).toDouble());                       // SphereOD
            Init_Value_DoubleSpin(ui->CylindreOD,LectureMesureQuery.value(8).toDouble());                       // CylindreOD
            Init_Value_DoubleSpin(ui->AddVPOD,   LectureMesureQuery.value(11).toDouble());                      // AddVPOD
            ui->AxeCylindreOD->setValue(LectureMesureQuery.value(9).toDouble());                                // AxeCylindreOD
            if (LectureMesureQuery.value(10).toString() != "")
                AVLOD->setText(LectureMesureQuery.value(10).toString());                                    // AVLOD
            if (LectureMesureQuery.value(12).toString() != "")
                AVPOD->setText(LectureMesureQuery.value(12).toString());                                    // AVPOG
            ui->PrismeOD->setValue(LectureMesureQuery.value(13).toDouble());                                    // PrismeOD
            ui->BasePrismeOD->setValue(LectureMesureQuery.value(14).toDouble());                                // BasePrismeOD
            ui->PressonODCheckBox->setChecked(LectureMesureQuery.value(16).toBool());                           // PressOnOD
            ui->DepoliODCheckBox->setChecked(LectureMesureQuery.value(17).toBool());                            // DepoliOD
            ui->PlanODCheckBox->setChecked(LectureMesureQuery.value(18).toBool());                              // PlanOD
            ui->RyserODCheckBox->setChecked(false);
            if (LectureMesureQuery.value(19).toInt() > 0)
            {
                ui->RyserODCheckBox->setChecked(true);
                ui->RyserSpinBox->setValue(LectureMesureQuery.value(19).toInt());                               // RyserOD
            }
        } // fin Oeil droit coche

        // Remplissage des champs Oeil Gauche
        if (LectureMesureQuery.value(21).toInt() == 1)
            ui->OGCheckBox->setChecked(true);                                                               // ODcoche
        if (ui->OGCheckBox->isChecked())
        {
            Init_Value_DoubleSpin(ui->SphereOG,   LectureMesureQuery.value(22).toDouble());                     // SphereOG
            Init_Value_DoubleSpin(ui->CylindreOG, LectureMesureQuery.value(23).toDouble());                     // CylindreOG
            Init_Value_DoubleSpin(ui->AddVPOG,    LectureMesureQuery.value(26).toDouble());                     // AddVPOG
            ui->AxeCylindreOG->setValue(LectureMesureQuery.value(24).toDouble());                               // AxeCylindreOG
            if (LectureMesureQuery.value(25).toString() != "")
                AVLOG->setText(LectureMesureQuery.value(25).toString());                                    // AVLOG
            if (LectureMesureQuery.value(27).toString() != "")
                AVPOG->setText(LectureMesureQuery.value(27).toString());                                    // AVPOG
            ui->PrismeOG->setValue(LectureMesureQuery.value(28).toDouble());                                    // PrismeOG
            ui->BasePrismeOG->setValue(LectureMesureQuery.value(29).toDouble());                                // BasePrismeOG
            ui->PressonOGCheckBox->setChecked(LectureMesureQuery.value(31).toBool());                           // PressOnOG
            ui->DepoliOGCheckBox->setChecked(LectureMesureQuery.value(32).toBool());                            // DepoliOG
            ui->PlanOGCheckBox->setChecked(LectureMesureQuery.value(33).toBool());                              // PlanOG
            ui->RyserOGCheckBox->setChecked(false);
            if (LectureMesureQuery.value(34).toInt() > 0)
            {
                ui->RyserOGCheckBox->setChecked(true);
                ui->RyserSpinBox->setValue(LectureMesureQuery.value(34).toInt());
            }                                                                                               // RyserOG

        } // fin Oeil gauche coche
    }
    int w = LectureMesureQuery.value(0).toInt();              // retourne idRefraction
    return w;
}

//---------------------------------------------------------------------------------
// Mise a jour DonneesOphtaPatient
// Après l'enregistrement des mesures dans la table refraction, on met à jour la table donneesOphtaPatients qui fait un ra mise à jour de la table refraction
//---------------------------------------------------------------------------------
void dlg_refraction::MajDonneesOphtaPatient()
{
    // Recherche d'un enregistrement existant.
    QString MAJrequete = "SELECT   idPat FROM " NOM_TABLE_DONNEES_OPHTA_PATIENTS
              " WHERE   (idPat = " + QString::number(gidPatient) +
              " AND QuelleMesure = '" + QuelleMesure() + "')";
    QSqlQuery MAJDonnesOphtaQuery (MAJrequete,db);
    if(proc->TraiteErreurRequete(MAJDonnesOphtaQuery,MAJrequete,"Impossible de se connecter à la table des Donnees biométriques!"))
        return;
    else
    {
        if (MAJDonnesOphtaQuery.size() > 0)
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
    QString nuRefraction = "";

    // Creation du formulaire Dlg Liste Mesures
    Dlg_ListeMes    = new dlg_listemesures(&gidPatient, SupOuRecup, proc);
    Dlg_ListeMes->setWindowTitle(tr("Liste des mesures : ") + gNomPatient + " " + gPrenomPatient );

    RetourListe = Dlg_ListeMes->exec();

    // relecture et affichage de la mesure selectionnee
    if (RetourListe > 0 && SupOuRecup == "RECUP")
    {
        nuRefraction = Dlg_ListeMes->IdRefractAOuvrir();
        if (nuRefraction.toInt() > 0)
        {
            LectureMesure("","","","",nuRefraction,"QM","","","");
            AfficherLaMesure();
        }
    }
    if (RetourListe > 0 && SupOuRecup == "SUPP")
        RechercheMesureEnCours();
    Dlg_ListeMes->close(); // nécessaire pour enregistrer la géométrie
    delete Dlg_ListeMes;
    AfficherLaMesure();
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
    if (gMode == Porte)          return "P";
    if (gMode == Autoref)        return "A";
    if (gMode == Refraction)     return "R";
    if (gMode == Prescription)   return "O";
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
    if (gMode != Prescription)
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
            if (ui->AddVPOD->value() == 0 && ui->AddVPOD->isVisible())
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
            if (ui->AddVPOG->value() == 0  && ui->AddVPOG->isVisible())
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
    int i = 0;
    QString Reponse ="";

    // On cherche si le patient est enregistré dans la table réfractions - sinon, on sort de la procédure
    QString selrequete = "SELECT idActe FROM " NOM_TABLE_REFRACTION
              " WHERE IdPat = " + QString::number(gidPatient) + " and quellemesure <> 'null'" ;
    //proc->Edit(selrequete);
    QSqlQuery RechercheMesureQuery1 (selrequete,db);
    proc->TraiteErreurRequete(RechercheMesureQuery1,selrequete,"");
    if (RechercheMesureQuery1.size() == 0)
    {
        gMode = Porte;
        ui->ReprendrePushButton->setEnabled(false);
        ui->OupsPushButton->setEnabled(false);
        ui->ResumePushButton->setEnabled(false);
        if (gAgePatient < 45)
            ui->VLRadioButton->setChecked(true);
        else
            ui->V2RadioButton->setChecked(true);
        AfficherLaMesure();
        return;
    }

    // recherche d'une mesure du jour
    while (i == 0)
    {
        selrequete = "SELECT idActe, QuelleMesure FROM " NOM_TABLE_REFRACTION   // recherche d'une mesure pour le jour en cours
                  " WHERE DateRefraction = '" + QDate::currentDate().toString("yyyy-MM-dd") +
                  "' AND   IdPat = " + QString::number(gidPatient) ;
        QSqlQuery RechercheMesureQuery2 (selrequete,db);
        proc->TraiteErreurRequete(RechercheMesureQuery2,selrequete,"");
        if (RechercheMesureQuery2.size() == 0)  break;
        RechercheMesureQuery2.first();
        for (int i = 0; i<RechercheMesureQuery2.size();i++)
        {
            if (RechercheMesureQuery2.value(1).toString() == "O")
            {
                Reponse = "O";
                i = RechercheMesureQuery2.size();
            }
            RechercheMesureQuery2.next();
        }
        if (Reponse == "O")  break;
        RechercheMesureQuery2.first();
        for (int i = 0; i<RechercheMesureQuery2.size();i++)
        {
            if (RechercheMesureQuery2.value(1).toString() == "R")
            {
                Reponse = "R";
                i = RechercheMesureQuery2.size();
            }
            RechercheMesureQuery2.next();
        }
        if (Reponse == "R") break;
        RechercheMesureQuery2.first();
        for (int i = 0; i<RechercheMesureQuery2.size();i++)
        {
            if (RechercheMesureQuery2.value(1).toString() == "A")
            {
                Reponse = "A";
                i = RechercheMesureQuery2.size();
            }
            RechercheMesureQuery2.next();
        }
        if (Reponse == "A") break;
        RechercheMesureQuery2.first();
        for (int i = 0; i<RechercheMesureQuery2.size();i++)
        {
            if (RechercheMesureQuery2.value(1).toString() == "P")
            {
                Reponse = "P";
                i = RechercheMesureQuery2.size();
            }
            RechercheMesureQuery2.next();
        }
        if (Reponse == "P") break;
        i = 1;
    }
    if (Reponse != "")
        if (LectureMesure("JOUR",Reponse,"","","","QM","","","") > 0)            // on affiche la mesure du jour trouvée
        {
            if (gMode == Refraction || gMode == Prescription)    Slot_PrescriptionRadionButton_clicked();
            if (gMode == Autoref)       gMode = Refraction;
            if (gMode == Porte)         gMode = Autoref;
            AfficherLaMesure();
            return ;
        }

    // On n'a rien trouvé pour le jour >> on cherche la dernière mesure de réfraction
    if (LectureMesure("AVANT","R","","","","QM","","","") > 0)
    {
        AfficherLaMesure();
        return ;
    }

    // on n'a rien trouvé en réfraction - on cherche la dernière mesure de prescription
    if (LectureMesure("AVANT","O","","","","QM","","","") > 0)
    {
        AfficherLaMesure();
        return ;
    }
    // on n'a rien trouvé en réfraction - on cherche la dernière mesure Autoref
    if (LectureMesure("AVANT","A","","","","QM","","","") > 0)
    {
        AfficherLaMesure();
        return ;
    }
    // on n'a rien trouvé en réfraction - on cherche la dernière mesure de fronto
    if (LectureMesure("AVANT","P","","","","QM","","","") > 0)
    {
        AfficherLaMesure();
        return ;
    }
}

//------------------------------------------------------------------------------------------------------
//  Calcul de la variable Resultat pour resume refraction.
//------------------------------------------------------------------------------------------------------
QString dlg_refraction::RechercheResultat(QString Mesure, QString Cycloplegie, QString TypLun)
 {
    QString     Resultat    ="";
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
                    " FROM " NOM_TABLE_REFRACTION
                    " WHERE  idPat        =  " + QString::number(gidPatient) +
                    " AND    QuelleMesure = '" + Mesure     + "'"
                    " AND    Cycloplegie  =  " + Cycloplegie   ;
    QString requete =  requeteBase;
    if (TypLun > "")
        requete +=  " AND QuelleDistance = '" + TypLun + "'";
    requete += " ORDER BY DateRefraction ASC ";

    QSqlQuery RechercheResultatQuery(requete,db);
    proc->TraiteErreurRequete(RechercheResultatQuery,requete,"");
    if (RechercheResultatQuery.size() == 0) return "";                                        // Aucune mesure trouvee pour ces criteres
    RechercheResultatQuery.last();
    zdate = RechercheResultatQuery.value(2).toDate().toString(tr("dd-MM-yyyy"));                        // date YYYY-MM-DD
    if (RechercheResultatQuery.value(0).toBool() && RechercheResultatQuery.value(1).toBool())      // OD coche et OG coche
        Resultat = tr("dilaté") + ")" "\n\t" + zdate + "\t" + RechercheResultatQuery.value(3).toString() + Separateur + RechercheResultatQuery.value(4).toString();
    else
    {
        // OD coche = true
        if (RechercheResultatQuery.value(0).toBool())
        {
            DateOD      = zdate;
            ResultatOD  =  RechercheResultatQuery.value(3).toString();
            requete     = requeteBase + " AND OGcoche =  true  ";
            requete     += " ORDER BY DateRefraction DESC ";
            QSqlQuery RechercheResultatODQuery(requete,db);
            proc->TraiteErreurRequete(RechercheResultatODQuery,requete,"");
            if (RechercheResultatODQuery.isActive() && RechercheResultatODQuery.next())
            {
                zdate      = RechercheResultatODQuery.value(2).toDate().toString(tr("dd-MM-yyyy"));
                ResultatOG  =  RechercheResultatODQuery.value(4).toString();
            }
        } // fin OD coche
        else
        {
            // OG coche = true
            if (RechercheResultatQuery.value(1).toBool())
            {
                DateOG      = zdate;
                ResultatOG  = RechercheResultatQuery.value(4).toString();
                requete     = requeteBase + " AND ODcoche =  true  ";
                requete     += " ORDER BY DateRefraction DESC ";
                QSqlQuery RechercheResultatOGQuery(requete,db);
                proc->TraiteErreurRequete(RechercheResultatOGQuery,requete,"");
                if (RechercheResultatOGQuery.isActive() && RechercheResultatOGQuery.next())
                {
                    zdate  = RechercheResultatOGQuery.value(2).toDate().toString(tr("dd-MM-yyyy"));
                    ResultatOD  =  RechercheResultatOGQuery.value(3).toString();
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
    QString     ResultatVerres = "";
    QString     zdate, Formule, TypeMesure;

    QString requete     =   "SELECT ODcoche, OGcoche, DateRefraction, FormuleOD, FormuleOG, QuelleMesure "     // 0-1-2-3-4-5
                    " FROM " NOM_TABLE_REFRACTION
                    " WHERE  idPat        =  "+ QString::number(gidPatient) +
                    " AND (QuelleMesure = 'P' OR QuelleMesure = 'O') "
                    " ORDER  BY DateRefraction DESC ";

    QSqlQuery RechercheVerresQuery(requete,db);
    proc->TraiteErreurRequete(RechercheVerresQuery,requete,"");
    RechercheVerresQuery.first();
    for (int i = 0; i < RechercheVerresQuery.size(); i++)
        {
        if (RechercheVerresQuery.value(5).toString() == "O")
            TypeMesure =  tr("Prescription");
        else
            TypeMesure =  tr("Verres portés");
        zdate = RechercheVerresQuery.value(2).toString();                         // date YYYY-MM-DD
        zdate = zdate.mid(8,2) + "-" + zdate.mid(5,2) + "-" + zdate.left(4);
        // calcul Formule
        Formule = RechercheVerresQuery.value(3).toString();            // Formule OD
        if (Formule.length() > 0)
            Formule += " / ";
        Formule += RechercheVerresQuery.value(4).toString();           // Formule OG
        if (ResultatVerres != "")
            ResultatVerres += "\n\n";
        ResultatVerres += "        " + zdate + " - " + TypeMesure + "\n\t" + Formule;
        RechercheVerresQuery.next();
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
    // QString TagAncre, numIDref; // 07-07-2014 08-08-2014

    int IDistance = 0;
    int IMesure   = 0;

    // Génération du code html pour TAG Ancre avec ID refraction            // 07-07-2014 // 08-08-2014
    //numIDref = QString::number(gListeRefractionID.at(gListeRefractionID.size()-1));
    //TagAncre = "<a name=\"" + numIDref + "\"></a>" "<span ><a href=\""+ numIDref + "\" style=\"text-decoration:none\" style=\"color:#000000\"> ";
    if (ui->SphereOD->hasFocus())       PrefixePlus(ui->SphereOD);
    if (ui->SphereOG->hasFocus())       PrefixePlus(ui->SphereOG);
    if (ui->CylindreOD->hasFocus())     PrefixePlus(ui->CylindreOD);
    if (ui->CylindreOG->hasFocus())     PrefixePlus(ui->CylindreOG);
    if (ui->AddVPOD->hasFocus())        PrefixePlus(ui->AddVPOD);
    if (ui->AddVPOG->hasFocus())        PrefixePlus(ui->AddVPOG);

    if (gMode == Porte)                                                     IMesure = 1;
    if (gMode == Autoref && !ui->CycloplegieCheckBox->isChecked())          IMesure = 2;
    if (gMode == Autoref && ui->CycloplegieCheckBox->isChecked())           IMesure = 3;
    if (gMode == Refraction && !ui->CycloplegieCheckBox->isChecked())       IMesure = 4;
    if (gMode == Refraction && ui->CycloplegieCheckBox->isChecked())        IMesure = 5;
    if (gMode == Prescription)                                              IMesure = 6;

    if (IMesure != 6)
    {
        if (ui->V2RadioButton->isChecked()) IDistance = 1;
        if (ui->VLRadioButton->isChecked()) IDistance = 2;
        if (ui->VPRadioButton->isChecked()) IDistance = 3;
        if (gMode == Refraction && ui->CycloplegieCheckBox->isChecked())        IDistance = 2;
    }
    else
    {
        if (ui->V2PrescritRadioButton->isChecked()) IDistance = 1;
        if (ui->VLPrescritRadioButton->isChecked()) IDistance = 2;
        if (ui->VPPrescritRadioButton->isChecked()) IDistance = 3;
    }

    if (IMesure == 1 || IMesure == 6)
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

        gResultatP = "";

        {
            // détermination OD
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() != 0)
                ResultatVLOD = Valeur(ui->SphereOD->text()) + " (" + Valeur(ui->CylindreOD->text()) + " à " + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() != 0)
                ResultatVLOD = Valeur(ui->SphereOD->text());
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() == 0)
                ResultatVLOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() == 0)
                ResultatVLOD = tr("plan");

            if (ui->AddVPOD->value() > 0)
                ResultatVPOD = Valeur(ui->AddVPOD->text());
            else
            {
                ResultatVPOD = tr("plan");
                if (ResultatVLOD == tr("plan")) VerreSpecialOD = tr("plan");
            }

            ResultatOD = ResultatVLOD + " add." + ResultatVPOD + tr(" VP");
            if (((!ui->ODCheckBox->isChecked() && (gMode != Prescription)) && VerreSpecialOD == "non") || (!ui->ODPrescritCheckBox->isChecked() && (gMode == Prescription)))
                ResultatOD = "Rien";
            if (VerreSpecialOD !="non") ResultatOD = VerreSpecialOD;

            // détermination OG
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() != 0)
                ResultatVLOG = Valeur(ui->SphereOG->text()) + " (" + Valeur(ui->CylindreOG->text()) + " à " + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() != 0)
                ResultatVLOG = Valeur(ui->SphereOG->text());
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() == 0)
                ResultatVLOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() == 0)
                ResultatVLOG = tr("plan");

            if (ui->AddVPOG->value() > 0)
                ResultatVPOG = Valeur(ui->AddVPOG->text());
            else
            {ResultatVPOG = tr("plan");
                if (ResultatVLOG == tr("plan")) VerreSpecialOG = tr("plan");}
            ResultatOG = ResultatVLOG + " add." + ResultatVPOG + tr(" VP");
            if (((!ui->OGCheckBox->isChecked() && (gMode != Prescription)) && VerreSpecialOG == "non") || (!ui->OGPrescritCheckBox->isChecked() && (gMode == Prescription)))
                ResultatOG = "Rien";
            if (VerreSpecialOG !="non") ResultatOG = VerreSpecialOG;
        }

        // Détermination de gResultatP
        switch (IDistance)
        {   case 1: //Verres bifocaux

            if (ResultatOD == ResultatOG)
            {
                if (ResultatVPOD != tr("plan") || VerreSpecialOD != "non")
                    gResultatP = ResultatOD + tr(" ODG");
                else
                    gResultatP = ResultatVLOD + tr(" VL ODG");
            }

            if (gResultatP == "" && (VerreSpecialOD != "non" && VerreSpecialOG != "non"))
                gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;

            if (gResultatP == "" && (VerreSpecialOD != "non" && ResultatOG == "Rien"))
                gResultatP = VerreSpecialOD + tr(" OD");

            if (gResultatP == "" && (VerreSpecialOD != "non" && ui->AddVPOG->value() == 0) && ResultatOG != "Rien")
                gResultatP = VerreSpecialOD + " / " + ResultatVLOG + tr(" VL");

            if (gResultatP == "" && (VerreSpecialOD != "non" && ui->AddVPOG->value() > 0) && ResultatOG != "Rien")
                gResultatP = VerreSpecialOD + " / " + ResultatOG;

            if (gResultatP == "" && ResultatOD != "Rien" && ui->AddVPOD->value() == 0 && VerreSpecialOG != "non")
                gResultatP = ResultatVLOD + " / " + VerreSpecialOG + tr(" VL");

            if (gResultatP == "" && ResultatOD != "Rien" && ui->AddVPOD->value() == 0  && ResultatOG == "Rien")
                gResultatP = ResultatVLOD + tr(" OD VL");

            if (gResultatP == "" && ResultatOD != "Rien" && (ui->AddVPOD->value() == 0 && ui->AddVPOG->value() == 0) && ResultatOG != "Rien")
                gResultatP = ResultatVLOD + " / " + ResultatVLOG + tr(" VL");

            if (gResultatP == "" && ResultatOD != "Rien" && (ui->AddVPOD->value() == 0 && ui->AddVPOG->value() > 0) && ResultatOG != "Rien")
                gResultatP = ResultatVLOD + " / " + ResultatOG;

            if (gResultatP == "" && ResultatOD != "Rien" && ui->AddVPOD->value() > 0 && VerreSpecialOG != "non")
                gResultatP = ResultatOD + " / " + VerreSpecialOG;

            if (gResultatP == "" && ResultatOD != "Rien" && ui->AddVPOD->value() > 0  && ResultatOG == "Rien")
                gResultatP = ResultatOD + tr(" OD");

            if (gResultatP == "" && ResultatOD != "Rien" && ui->AddVPOD->value() > 0 && ui->AddVPOG->value() == 0 && ResultatOG != "Rien")
                gResultatP = ResultatOD + " / " + ResultatVLOG;

            if (gResultatP == "" && ResultatOD != "Rien" && ui->AddVPOD->value() > 0 && ui->AddVPOG->value() > 0 && ResultatOG != "Rien")
            {if (ResultatVPOD == ResultatVPOG)
                    gResultatP = ResultatVLOD + " / " + ResultatOG + tr(" ODG");
                else
                    gResultatP = ResultatOD + " / " + ResultatOG;}

            if (gResultatP == "" && (ResultatOD == "Rien" && VerreSpecialOG != "non"))
                gResultatP = VerreSpecialOG + tr(" OG");

            if (gResultatP == "" && (ResultatOD == "Rien" &&  ui->AddVPOG->value() == 0) && ResultatOG != "Rien")
                gResultatP = ResultatVLOG + tr(" OG VL");

            if (gResultatP == "" && (ResultatOD == "Rien" &&  ui->AddVPOG->value() > 0) && ResultatOG != "Rien")
                gResultatP = ResultatOG + tr(" OG");

            break;

        case 2: //Verres de loin
            if (ResultatOD == ResultatOG)
            {
                if (VerreSpecialOD != "non")
                    gResultatP = VerreSpecialOD + tr(" ODG");
                else
                    gResultatP = ResultatVLOD + tr(" VL ODG");
            }
            if (gResultatP == "" && VerreSpecialOD != "non" && VerreSpecialOG != "non")
                gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;

            if (gResultatP == "" && VerreSpecialOD != "non" && ResultatOG == "Rien")
                gResultatP = VerreSpecialOD + tr(" OD");

            if (gResultatP == "" && VerreSpecialOD != "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                gResultatP = VerreSpecialOD + " / " + ResultatVLOG + tr(" VL");

            if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && VerreSpecialOG != "non")
                gResultatP = ResultatVLOD + " / " + VerreSpecialOG + tr(" VL");

            if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && ResultatOG == "Rien")
                gResultatP = ResultatVLOD + tr(" OD VL");

            if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                gResultatP = ResultatVLOD + " / " + ResultatVLOG + tr(" VL");

            if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && VerreSpecialOG != "non")
                gResultatP = VerreSpecialOG + tr(" VL");

            if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && ResultatOG == "Rien")
                gResultatP = ResultatVLOG + tr(" OG VL");

            if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                gResultatP = ResultatVLOG + tr(" OG VL");

            break;

        case 3: // Verres de près
            switch (IMesure) {
            case 1:                                     // Calcul des verres de près en mode porte
                if (ResultatOD == ResultatOG)
                {if (VerreSpecialOD != "non")
                        gResultatP = VerreSpecialOD + tr(" ODG");
                    else
                        gResultatP = ResultatVLOD +  tr(" VP ODG");}

                if (gResultatP == "" && VerreSpecialOD != "non" && VerreSpecialOG != "non")
                    gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;

                if (gResultatP == "" && VerreSpecialOD != "non" && ResultatOG == "Rien")
                    gResultatP = VerreSpecialOD +  tr(" OD");

                if (gResultatP == "" && VerreSpecialOD != "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                    gResultatP = VerreSpecialOD + " / " + ResultatVLOG +  tr(" VP");

                if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && VerreSpecialOG != "non")
                    gResultatP = ResultatVLOD + " / " + VerreSpecialOG +  tr(" VP");

                if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && ResultatOG == "Rien")
                    gResultatP = ResultatVLOD +  tr(" OD VP");

                if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                    gResultatP = ResultatVLOD + " / " + ResultatVLOG +  tr(" VP");

                if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && VerreSpecialOG != "non")
                    gResultatP = VerreSpecialOG +  tr(" VP");

                if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && ResultatOG == "Rien")
                    gResultatP = ResultatVLOG +  tr(" OG VP");

                if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                    gResultatP = ResultatVLOG +  tr(" OG VP");
                break;
            case 6:                                     // Calcul des verres de près en mode impression
                ResultatGlobalSphereOD = QString::number((ui->SphereOD->value() + ui->AddVPOD->value()) ,'f',2).toDouble();
                ResultatGlobalSphereOG = QString::number((ui->SphereOG->value() + ui->AddVPOG->value()) ,'f',2).toDouble();
                ResultatVLOD = Valeur(QString::number(ResultatGlobalSphereOD,'f',2));
                ResultatVLOG = Valeur(QString::number(ResultatGlobalSphereOG,'f',2));

                if (ui->CylindreOD->value() != 0 && ResultatGlobalSphereOD != 0)
                    ResultatVLOD = ResultatVLOD + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
                if (ui->CylindreOD->value() != 0 && ResultatGlobalSphereOD == 0)
                    ResultatVLOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
                if (ui->CylindreOD->value() == 0 && ResultatGlobalSphereOD == 0)
                    ResultatVLOD =  tr("plan");
                ResultatOD = ResultatVLOD;
                if (!ui->ODPrescritCheckBox->isChecked())
                    ResultatOD = "Rien";
                if (VerreSpecialOD !="non") ResultatOD = VerreSpecialOD;

                if (ui->CylindreOG->value() != 0 && ResultatGlobalSphereOG != 0)
                    ResultatVLOG = ResultatVLOG + " (" + Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() + ")";
                if (ui->CylindreOG->value() != 0 && ResultatGlobalSphereOG == 0)
                    ResultatVLOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
                if (ui->CylindreOG->value() == 0 && ResultatGlobalSphereOG == 0)
                    ResultatVLOG = tr("plan");
                ResultatOG = ResultatVLOG;
                if (!ui->OGPrescritCheckBox->isChecked())
                    ResultatOG = "Rien";
                if (VerreSpecialOG !="non") ResultatOG = VerreSpecialOG;

                if (ResultatOD == ResultatOG)
                {if (VerreSpecialOD != "non")
                        gResultatP = VerreSpecialOD + tr(" ODG");
                    else
                        gResultatP = ResultatVLOD + tr(" VP ODG");}

                if (gResultatP == "" && VerreSpecialOD != "non" && VerreSpecialOG != "non")
                    gResultatP = VerreSpecialOD + " / " + VerreSpecialOG;

                if (gResultatP == "" && VerreSpecialOD != "non" && ResultatOG == "Rien")
                    gResultatP = VerreSpecialOD + tr(" OD");

                if (gResultatP == "" && VerreSpecialOD != "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                    gResultatP = VerreSpecialOD + " / " + ResultatVLOG + tr(" VP");

                if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && VerreSpecialOG != "non")
                    gResultatP = ResultatVLOD + " / " + VerreSpecialOG +  tr(" VP");

                if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && ResultatOG == "Rien")
                    gResultatP = ResultatVLOD +  tr(" OD VP");

                if (gResultatP == "" && ResultatOD != "Rien" && VerreSpecialOD == "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                    gResultatP = ResultatVLOD + " / " + ResultatVLOG +  tr(" VP");

                if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && VerreSpecialOG != "non")
                    gResultatP = VerreSpecialOG +  tr(" VP");

                if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && ResultatOG == "Rien")
                    gResultatP = ResultatVLOG +  tr(" OG VP");

                if (gResultatP == "" && ResultatOD == "Rien" && VerreSpecialOD == "non" && ResultatOG != "Rien" && VerreSpecialOG == "non")
                    gResultatP = ResultatVLOG +  tr(" OG VP");
                break;
            default:
                break;
            }

            break;
        default:
            break;
        }


        // 2- incorporation des valeurs des prismes eventuels

        ResultatPrismeOD = "";
        if (ui->PrismeOD->value() != 0 && ui->BasePrismeTextODComboBox->currentText() != "" && !ui->DepoliODCheckBox->isChecked())
            ResultatPrismeOD = ui->PrismeOD->text() + " base " + ui->BasePrismeTextODComboBox->currentText();
        if (ui->PrismeOD->value() != 0 && ui->BasePrismeTextODComboBox->currentText() == "" && !ui->DepoliODCheckBox->isChecked())
            ResultatPrismeOD = ui->PrismeOD->text() + " base " + ui->BasePrismeOD->text();

        ResultatPrismeOG = "";
        if (ui->PrismeOG->value() != 0 && ui->BasePrismeTextOGComboBox->currentText() != "" && !ui->DepoliOGCheckBox->isChecked())
            ResultatPrismeOG = ui->PrismeOG->text() + " base " + ui->BasePrismeTextOGComboBox->currentText();
        if (ui->PrismeOG->value() != 0 && ui->BasePrismeTextOGComboBox->currentText() == "" && !ui->DepoliOGCheckBox->isChecked())
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
        switch (IMesure)
        {
        case 1:
            gResultatPO =  "<td width=\"60\"><font color = " + proc->CouleurTitres + "><b>Porte:</b></font></td><td>" + gResultatP + "</td>" + ResultatPrisme + ResultatRyser;
            break;
        case 6:
            gResultatPR =  "<td width=\"30\"><font color = " + proc->CouleurTitres + "><b>VP:</b></font></td><td>" + gResultatP + " " + ui->CommentairePrescriptionTextEdit->toPlainText() + "</td>" + ResultatPrisme + ResultatRyser;
            break;
        default:
            break;
        }
    }

    if (IMesure == 2 || IMesure == 3)
        //EN MODE Autoref --  détermination de gResultatA  ---------------------------------------------------------------------------
    {
        gResultatA = "";
        {
            // détermination OD
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() != 0)
                ResultatVLOD = Valeur(ui->SphereOD->text()) + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() != 0)
                ResultatVLOD = Valeur(ui->SphereOD->text());
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() == 0)
                ResultatVLOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() == 0)
                ResultatVLOD = tr("plan");
            if (!ui->ODCheckBox->isChecked()) ResultatOD = "Rien";

            // détermination OG
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() != 0)
                ResultatVLOG = Valeur(ui->SphereOG->text()) + " (" + Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() != 0)
                ResultatVLOG = Valeur(ui->SphereOG->text());
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() == 0)
                ResultatVLOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() == 0)
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
            if (gDioptrAstOD!=0)
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + proc->CouleurTitres + "><b>" + tr("KOD") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OD->text() + "/" + ui->K2OD->text() + " Km = " + QString::number((QLocale().toDouble(ui->K1OD->text()) + QLocale().toDouble(ui->K2OD->text()))/2,'f',2)
                        + "</td><td width=\"120\">" + QString::number(gDioptrAstOD,'f',2) +  tr(" à ") + ui->AxeKOD->text() + "°</td>";
            else
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + proc->CouleurTitres + "><b>" + tr("KOD") + ":</b></font></td><td width=\"240\">"
                        + ui->K1OD->text() + tr(" à ") + ui->AxeKOD->text() + "°/" + ui->K2OD->text() + tr(" Km = ") + QString::number((QLocale().toDouble(ui->K1OD->text()) + QLocale().toDouble(ui->K2OD->text()))/2,'f',2) + "</td>";
        }
        if (QLocale().toDouble(ui->K1OG->text())>0)
        {
            if (gDioptrAstOG!=0)
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + proc->CouleurTitres + "><b>" + tr("KOG") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OG->text() + "/" + ui->K2OG->text() + " Km = " + QString::number((QLocale().toDouble(ui->K1OG->text()) + QLocale().toDouble(ui->K2OG->text()))/2,'f',2)
                        + "</td><td width=\"120\">" + QString::number(gDioptrAstOG,'f',2) +  tr(" à ") + ui->AxeKOG->text() + "°</td>";
            else
                kerato += "</p><p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"><font color = " + proc->CouleurTitres + "><b>" + tr("KOG") + ":</b></font></td><td width=\"180\">"
                        + ui->K1OG->text() +  tr(" à ") + ui->AxeKOG->text() + "°/" + ui->K2OG->text() + tr(" Km = ") + QString::number((QLocale().toDouble(ui->K1OG->text()) + QLocale().toDouble(ui->K2OG->text()))/2,'f',2) + "</td>";
        }
        switch (IMesure)
        {
        case 2:
            gResultatAnondil = "<td width=\"60\"><font color = " + proc->CouleurTitres + "><b>Autoref:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatA + "</td><td>(non dilaté)</td>" + kerato;
            break;
        case 3:
            gResultatAdil    = "<td width=\"60\"><font color = " + proc->CouleurTitres + "><b>Autoref:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatA + "</td><td><font color = \"red\">(dilaté)</font></td>" + kerato;
            break;
        default:
            break;
        }
    }

    if (IMesure == 4 || IMesure == 5)
        //EN MODE Refraction --  détermination de gResultatR  ---------------------------------------------------------------------------
    {
        // - 1 - détermination des verres
        gResultatR = "";

        // détermination OD
        if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() != 0)
            ResultatVLOD = Valeur(ui->SphereOD->text()) + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
        if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() != 0)
            ResultatVLOD = Valeur(ui->SphereOD->text());
        if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() == 0)
            ResultatVLOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
        if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() == 0)
            ResultatVLOD = tr("plan");

        if (ui->AddVPOD->value() > 0)
            ResultatVPOD = Valeur(ui->AddVPOD->text());
        else
            ResultatVPOD = tr("plan");

        ResultatOD = ResultatVLOD + " add." + ResultatVPOD + tr(" VP");
        if (!ui->ODCheckBox->isChecked())
            ResultatOD = "Rien";

        // détermination OG
        if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() != 0)
            ResultatVLOG = Valeur(ui->SphereOG->text()) + " (" + Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() + ")";
        if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() != 0)
            ResultatVLOG = Valeur(ui->SphereOG->text());
        if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() == 0)
            ResultatVLOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
        if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() == 0)
            ResultatVLOG = tr("plan");

        if (ui->AddVPOG->value() > 0)
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
        double av = 0;
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
        switch (IDistance)
        {
        case 1: //Vision de loin et de près
            if (gResultatR == "" && ResultatOD != "Rien" && ui->AddVPOD->value() == 0  && ResultatOG == "Rien")
                gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD");

            if (gResultatR == "" && ResultatOD != "Rien" && (ui->AddVPOD->value() == 0 && ui->AddVPOG->value() == 0) && ResultatOG != "Rien")
                gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG") + "</td>";

            if (gResultatR == "" && ResultatOD != "Rien" && (ui->AddVPOD->value() == 0 && ui->AddVPOG->value() > 0) && ResultatOG != "Rien")
                gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + tr(" OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG") + "</td>";

            if (gResultatR == "" && ResultatOD != "Rien" && ui->AddVPOD->value() > 0  && ResultatOG == "Rien")
                gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD");

            if (gResultatR == "" && ResultatOD != "Rien" && ui->AddVPOD->value() > 0 && ui->AddVPOG->value() == 0 && ResultatOG != "Rien")
                gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG") + "</td>";

            if (gResultatR == "" && ResultatOD != "Rien" && ui->AddVPOD->value() > 0 && ui->AddVPOG->value() > 0 && ResultatOG != "Rien")
                gResultatR = ResultatVLOD + " " + "<b><font color = " + colorVLOD + "><b>" + AVLOD->text() + "</font><font color = " + colorVPOD + "> P" + AVPOD->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOD->text()) + tr("VP OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG") + "</td>";

            if (gResultatR == "" && (ResultatOD == "Rien" &&  ui->AddVPOG->value() == 0) && ResultatOG != "Rien")
                gResultatR = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + tr(" OG");

            if (gResultatR == "" && (ResultatOD == "Rien" &&  ui->AddVPOG->value() > 0) && ResultatOG != "Rien")
                gResultatR = ResultatVLOG + " " + "<b><font color = " + colorVLOG + "><b>" + AVLOG->text() + "</font><font color = " + colorVPOG + "> P" + AVPOG->text().replace("<","&lt;") + "</font></b>" + " add." + Valeur(ui->AddVPOG->text()) + tr("VP OG");
            break;

        case 2: //Verres de loin ou cycloplégie
            if (gResultatR == "" && ResultatOD != "Rien" && ResultatOG == "Rien")
                gResultatR = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + AVLOD->text() + "</b></font> " + tr("OD");

            if (gResultatR == "" && ResultatOD != "Rien" && ResultatOG != "Rien")
                gResultatR = ResultatVLOD + " " + "<font color = " + colorVLOD + "><b>" + AVLOD->text() + "</b></font> " + tr("OD") + "</td></p>"
                        +"<p style = \"margin-top:0px; margin-bottom:0px;margin-left: 0px;\"><td width=\"60\"></td><td width=\"" LARGEUR_FORMULE "\">"
                        + ResultatVLOG + " " + "<font color = " + colorVLOG + "><b>" + AVLOG->text() + "</b></font> " + tr("OG") + "</td>";

            if (gResultatR == "" && ResultatOD == "Rien" && ResultatOG != "Rien")
                gResultatR = ResultatVLOG + "<font color = " + colorVLOG + "><b>" + AVLOG->text() + "</b></font> " + tr("OG");
            break;

        default:
            break;
        }
        switch (IMesure)
        {
            case 4:
                gResultatRnondil = "<td width=\"60\"><font color = " + proc->CouleurTitres + "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatR + "</td><td width=\"70\">" + tr("(non dilaté)") + "</td><td>" + proc->getDataUser()["UserLogin"].toString() + "</td>";
                break;
            case 5:
                gResultatRdil = "<td width=\"60\"><font color = " + proc->CouleurTitres + "><b>AV:</b></font></td><td width=\"" LARGEUR_FORMULE "\">" + gResultatR + "</td><td width=\"70\"><font color = \"red\">" + tr("(dilaté)") + "</font></td><td>" + proc->getDataUser()["UserLogin"].toString() + "</td>";
                break;
            default:
            break;
        }
    }

    // Consolidation de tous les résultats dans un même QString

    gResultatObservation = gResultatPO + gResultatAnondil + gResultatAdil + gResultatRnondil + gResultatRdil;
    gResultatPR = gResultatPR + gResultatCommResumOrdo;
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
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() != 0)
                ResultatOD = SphOD + " (" + CylOD + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() != 0)
                ResultatOD = SphOD;
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() == 0)
                ResultatOD = CylOD + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() == 0)
            {ResultatOD = tr("plan");   VerreSpecialOD = "oui";}
        }

        // Determination de ResultatOG en VL
        if (VerreSpecialOG == "non")
        {
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() != 0)
                ResultatOG = SphOG + " (" + CylOG + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() != 0)
                ResultatOG = SphOG;
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() == 0)
                ResultatOG = CylOG + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() == 0)
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
                if (ui->AddVPOD->value() == 0 && ui->AddVPOG->value() == 0)
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
                if (ui->AddVPOD->value() != 0 && ui->AddVPOG->value() != 0)
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
                if (ui->AddVPOD->value() != 0 && ui->AddVPOG->value() == 0)
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
                if (ui->AddVPOD->value() == 0 && ui->AddVPOG->value() != 0)
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
            if (ui->SphereOD->value() + ui->AddVPOD->value() == 0)
            {if (ui->CylindreOD->value() == 0)
                {VerreSpecialOD = "oui"; ResultatOD = tr("plan");}
                else
                    ResultatOD = Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() ;
            }
            if (ui->SphereOD->value() + ui->AddVPOD->value() != 0)
            {
                VerreSpecialOD = "non";
                ResultatOD = Valeur(QString::number((ui->SphereOD->value() + ui->AddVPOD->value()) ,'f',2));
                if (ui->CylindreOD->value() != 0)
                    ResultatOD = ResultatOD + " (" + Valeur(ui->CylindreOD->text()) + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            }
            // Oeil Gauche
            if (ui->SphereOG->value() + ui->AddVPOG->value() == 0)
            {if (ui->CylindreOG->value() == 0)
                {VerreSpecialOG = "oui"; ResultatOG = tr("plan");}
                else
                    ResultatOG = Valeur(ui->CylindreOG->text()) + tr(" à ") + ui->AxeCylindreOG->text() ;
            }
            if (ui->SphereOG->value() + ui->AddVPOG->value() != 0)
            {
                VerreSpecialOG = "non";
                ResultatOG = Valeur(QString::number((ui->SphereOG->value() + ui->AddVPOG->value()) ,'f',2));
                if (ui->CylindreOG->value() != 0)
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
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() != 0)
                ResultatOD = SphOD + " (" + CylOD + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() != 0)
                ResultatOD = SphOD;
            if (ui->CylindreOD->value() != 0 && ui->SphereOD->value() == 0)
                ResultatOD = CylOD + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0 && ui->SphereOD->value() == 0)
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
            if (ui->AddVPOD->value() == 0)
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
            if (ui->CylindreOD->value() != 0 && SphVPOD != "0.00")   ResultatOD = SphVPOD + " (" + CylOD + tr(" à ") + ui->AxeCylindreOD->text() + ")";
            if (ui->CylindreOD->value() == 0 && SphVPOD != "0.00")   ResultatOD = SphVPOD;
            if (ui->CylindreOD->value() != 0 && SphVPOD == "0.00")   ResultatOD = CylOD + tr(" à ") + ui->AxeCylindreOD->text() ;
            if (ui->CylindreOD->value() == 0 && SphVPOD == "0.00")   ResultatOD = tr("plan");
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
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() != 0)
                ResultatOG = SphOG + " (" + CylOG + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() != 0)
                ResultatOG = SphOG;
            if (ui->CylindreOG->value() != 0 && ui->SphereOG->value() == 0)
                ResultatOG = CylOG + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0 && ui->SphereOG->value() == 0)
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
            if (ui->AddVPOG->value() == 0)
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
            if (ui->CylindreOG->value() != 0 && SphVPOG != "0.00")   ResultatOG = SphVPOG + " (" + CylOG + tr(" à ") + ui->AxeCylindreOG->text() + ")";
            if (ui->CylindreOG->value() == 0 && SphVPOG != "0.00")   ResultatOG = SphVPOG;
            if (ui->CylindreOG->value() != 0 && SphVPOG == "0.00")   ResultatOG = CylOG + tr(" à ") + ui->AxeCylindreOG->text() ;
            if (ui->CylindreOG->value() == 0 && SphVPOG == "0.00")   ResultatOG = tr("plan");
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
        if (ui->PrismeOD->value() != 0 && !ui->DepoliODCheckBox->isChecked())
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
        if (ui->PrismeOG->value() != 0 && !ui->DepoliOGCheckBox->isChecked())
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
    Resultat = Resultat + "\n" + gResultatCommPreDef;
    if (ui->CommentairePrescriptionTextEdit->document()->toPlainText() > "")
        Resultat = Resultat + "\n" + ui->CommentairePrescriptionTextEdit->document()->toPlainText();


    ui->ResumePrescriptionTextEdit->setText(Resultat);
}

//---------------------------------------------------------------------------------
// Resume de la refraction - fait apparaître un résumé des réfractions précédentes dans une fenêtre resumeRefraction
//---------------------------------------------------------------------------------
void dlg_refraction::ResumeRefraction()
{
    QList<dlg_resumerefraction *> listres = findChildren<dlg_resumerefraction *>();
    if (listres.size()> 0)
    {
        if (listres.at(0)->isVisible())
            listres.at(0)->raise();
        else
            listres.at(0)->show();
        return;
    }
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

    Dlg_ResumeRef   = new dlg_resumerefraction(proc,this);
    Dlg_ResumeRef->txtedit->setText(ResultatRefraction);
    Dlg_ResumeRef->setWindowTitle(tr("Historique réfractions ") + gPrenomPatient + " " + gNomPatient);
    Dlg_ResumeRef->show();
    Dlg_ResumeRef->setWindowIcon(Icons::icLunettes());
}

//---------------------------------------------------------------------------------
// Maj d'un enregistrement dans DonneesOphtaPatient
//---------------------------------------------------------------------------------
void dlg_refraction::UpdateDonneesOphtaPatient()
{
    QString UpdateDOPrequete = "UPDATE  " NOM_TABLE_DONNEES_OPHTA_PATIENTS
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
                    ", DateK =  '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss") + "'";
        }
    }
    if (ui->ODCheckBox->isChecked())
    {
        if (ui->SphereOD->value() == 0)
            UpdateDOPrequete += ", SphereOD = null";
        else
            UpdateDOPrequete += ", SphereOD = " + QString::number(ui->SphereOD->value());
        if (ui->CylindreOD->value() == 0)
            UpdateDOPrequete +=
                ", CylindreOD = null"
                ", AxeCylindreOD = null";
        else
            UpdateDOPrequete +=
                ", CylindreOD = " + QString::number(ui->CylindreOD->value()) +
                ", AxeCylindreOD = " + QString::number(ui->AxeCylindreOD->value());
        if (gMode == Refraction)
            UpdateDOPrequete += ", AVLOD = '" + AVLOD->text() + "'";
        if (ui->AddVPOD->value() > 0 && ui->V2RadioButton->isChecked()  && gMode == Refraction)
            UpdateDOPrequete += ", AddVPOD = " + QString::number(ui->AddVPOD->value());
        else
            UpdateDOPrequete += ",AddVPOD = null";
        if (gMode == Refraction && ui->V2RadioButton->isChecked())
            UpdateDOPrequete += ", AVPOD = '" + AVPOD->text() + "'";
        else
            UpdateDOPrequete += ", AVPOD = null";
        UpdateDOPrequete += ", DateRefOD = '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss") + "'";
    }
    if (ui->OGCheckBox->isChecked())
    {
        if (ui->SphereOG->value() == 0)
            UpdateDOPrequete += ", SphereOG = null";
        else
            UpdateDOPrequete += ", SphereOG = " + QString::number(ui->SphereOG->value());
        if (ui->CylindreOG->value() == 0)
            UpdateDOPrequete +=
                ", CylindreOG = null"
                ", AxeCylindreOG = null";
        else
            UpdateDOPrequete +=
                ", CylindreOG = " + QString::number(ui->CylindreOG->value()) +
                ", AxeCylindreOG = " + QString::number(ui->AxeCylindreOG->value());
        if (gMode == Refraction)
            UpdateDOPrequete += ", AVLOG = '" + AVLOG->text() + "'";
        if (ui->AddVPOG->value() > 0 && ui->V2RadioButton->isChecked() && gMode == Refraction)
            UpdateDOPrequete += ", AddVPOG = " + QString::number(ui->AddVPOG->value());
        else
            UpdateDOPrequete += ",AddVPOG = null";
        if (gMode == Refraction && ui->V2RadioButton->isChecked())
            UpdateDOPrequete += ", AVPOG = '" + AVPOG->text() + "'";
        else
            UpdateDOPrequete += ", AVPOG = null";
        UpdateDOPrequete += ", DateRefOG = '" + ui->DateDateEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss") + "'";
    }
    UpdateDOPrequete +=  " WHERE idPat = " + QString::number(gidPatient) + " AND QuelleMesure = '" + QuelleMesure() + "'";
    QSqlQuery MAJDonneesOphtaQuery (db);
    MAJDonneesOphtaQuery.prepare(UpdateDOPrequete);

    MAJDonneesOphtaQuery.exec();
    proc->TraiteErreurRequete(MAJDonneesOphtaQuery,UpdateDOPrequete, tr("Erreur de MAJ dans ")+ NOM_TABLE_DONNEES_OPHTA_PATIENTS);
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
// Envoi de données vers le refracteur
//-----------------------------------------------------------------------------------------
void dlg_refraction::EnvoiMesureRefracteur()
{
    proc->PortRefracteur()->clear();
    //proc->ab(1);
    /*
    *-SOH                      1
    *-DLM                      2
    *-STX                      3
    *- R- 2.25- 0.75180        4->20
    * ETB                      21
    *- L+ 4.75- 0.75 90        22->38
    *-ETB                      39
    *-AR+ 2.75                 40->45
    *-ETB                      46
    *-AL+ 2.50                 47->52
    * EOT                      53
    */
    QByteArray wbuff;
    wbuff.append(QByteArray::fromHex("1"));            //SOH -> start of header
    wbuff.append("DRM");                                //                          -> DLM correspont au fronto
    wbuff.append(QByteArray::fromHex("2"));            //STX -> start of text
    wbuff.append("OR- 2.25- 0.75180");                  //                          -> Sphere, cylindre, axe pour l'OD -2.25(-0.75 à 180°)
    wbuff.append(QByteArray::fromHex("17"));            //ETB -> end of text block  -> fin SCA OD
    wbuff.append("OL+ 4.75- 0.75 90");                  //                          -> Sphere, cylindre, axe pour l'OG +4.75(-0.75 à 90°)
    wbuff.append(QByteArray::fromHex("17"));            //ETB -> end of text block  -> fin SCA OG
    wbuff.append("PD63");                               //                          -> ecart pupillaire
    wbuff.append(QByteArray::fromHex("17"));            //ETB -> end of text block  -> fin ecart pupillaire
    wbuff.append(QByteArray::fromHex("4"));            //EOT -> end of transmission
    /*
        QByteArray wbuff;
        QString valeur = "";
        wbuff.append(QByteArray::fromHex("1"));            //SOH -> start of header
        wbuff.append("DLM");                                //                          -> DLM correspont au fronto
        wbuff.append(QByteArray::fromHex("2"));            //STX -> start of text
        if (SphereOD!="null")
        {
            valeur += " R";
            if (SphereOD.toDouble()>=0)
                valeur += "+";
            else
                valeur += "-";
            if ((SphereOD.toDouble()*SphereOD.toDouble())<100)
                valeur += " ";
            if (SphereOD.toDouble()<0)
                SphereOD = QString::number(SphereOD.toDouble()*-1,'f',2);
            else
                SphereOD = QString::number(SphereOD.toDouble(),'f',2);
            valeur += SphereOD;
            if (CylOD.toDouble()>=0)
                valeur += "+";
            else
                valeur += "-";
            if ((CylOD.toDouble()*CylOD.toDouble())<100)
                valeur += " ";
            if (CylOD.toDouble()<0)
                CylOD = QString::number(CylOD.toDouble()*-1,'f',2);
            else
                CylOD = QString::number(CylOD.toDouble(),'f',2);
            valeur += CylOD;
            if (AxeOD.toDouble()<100)
                valeur += " ";
            valeur += AxeOD;
            wbuff.append(valeur);
            wbuff.append(QByteArray::fromHex("17"));            //ETB -> end of text block  -> fin SCA OD
        }
        valeur = "";
        if (SphereOG!="null")
        {
            valeur += " L";
            if (SphereOG.toDouble()>=0)
                valeur += "+";
            else
                valeur += "-";
            if ((SphereOG.toDouble()*SphereOG.toDouble())<100)
                valeur += " ";
            if (SphereOG.toDouble()<0)
                SphereOG = QString::number(SphereOG.toDouble()*-1,'f',2);
            else
                SphereOG = QString::number(SphereOG.toDouble(),'f',2);
            valeur += SphereOG;
            if (CylOG.toDouble()>=0)
                valeur += "+";
            else
                valeur += "-";
            if ((CylOG.toDouble()*CylOG.toDouble())<100)
                valeur += " ";
            if (CylOG.toDouble()<0)
                CylOG = QString::number(CylOG.toDouble()*-1,'f',2);
            else
                CylOG = QString::number(CylOG.toDouble(),'f',2);
            valeur += CylOG;
            if (AxeOG.toDouble()<100)
                valeur += " ";
            valeur += AxeOG;
            wbuff.append(valeur);
            wbuff.append(QByteArray::fromHex("17"));            //ETB -> end of text block  -> fin SCA OG
        }
        valeur = "";
        if (AddOD!="null")
        {
            valeur += "AR+";
            if (AddOD.toDouble()<10)
                valeur += " ";
            valeur += QString::number(AddOD.toDouble(),'f',2);
            wbuff.append(valeur);
            wbuff.append(QByteArray::fromHex("17"));            //ETB -> end of text block  -> fin add OD
        }
        valeur = "";
        if (AddOG!="null")
        {
            valeur += "AL+";
            if (AddOG.toDouble()<10)
                valeur += " ";
            valeur += QString::number(AddOG.toDouble(),'f',2);
            wbuff.append(valeur);
            wbuff.append(QByteArray::fromHex("17"));            //ETB -> end of text block  -> fin add OG
        }
        wbuff.append(QByteArray::fromHex("04"));            //EOT -> end of transmission
        */

    QString txt = wbuff;
    //proc->Edit(txt);                                    //->DLM R- 2.25- 0.75180 L+ 4.75- 0.75 90AR+ 2.75AL+ 2.50<-
    //proc->PortRefracteur()->setRequestToSend(true);
    proc->PortRefracteur()->clear();
    proc->PortRefracteur()->write(wbuff);
    //proc->PortRefracteur()->flush();
    //        proc->PortRefracteur()->waitForBytesWritten(1000);
    //        proc->PortRefracteur()->waitForReadyRead(1000);
    mSphereOD   = "";
    mCylOD      = "";
    mAxeOD      = "";
    mAddOD      = "";
    mSphereOG   = "";
    mCylOG      = "";
    mAxeOG      = "";
    mAddOG      = "";
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
        gMode = Refraction;
        MesuresRefracteur = proc->DonneesRefracteurSubj();
    }
    else
    {
        Slot_PrescriptionRadionButton_clicked();
        gMode = Prescription;
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
    case Refraction:
    {
       ui->AVLODupComboBox->setCurrentText(AVLOD);
        ui->AVPODupComboBox->setCurrentText(MesuresRefracteur["AVPOD"].toString());
        ui->AVLOGupComboBox->setCurrentText(AVLOG);
        ui->AVPOGupComboBox->setCurrentText(MesuresRefracteur["AVPOG"].toString());
        ui->V2RadioButton->setChecked(MesuresRefracteur["AddOD"].toDouble()>0 || MesuresRefracteur["AddOG"].toDouble()>0);
        ui->VPRadioButton->setChecked(false);
        ui->VLRadioButton->setChecked(MesuresRefracteur["AddOD"].toDouble()==0 && MesuresRefracteur["AddOG"].toDouble()==0);
        Slot_QuelleDistance_Clicked();
        break;
    }
    case Prescription:
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
