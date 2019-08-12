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

#ifndef DLG_REFRACTION_H
#define DLG_REFRACTION_H

#include <QTimer>
#include "updoublespinbox.h"
#include "updoublevalidator.h"
#include "upgroupbox.h"
#include "upspinbox.h"

#include "dlg_commentaires.h"
#include "ui_dlg_commentaires.h"
#include "dlg_refractionlistemesures.h"


namespace Ui {
    class dlg_refraction;
}

class dlg_refraction : public QDialog
{
    Q_OBJECT

public:
    dlg_refraction(Acte *acte, QWidget *parent = Q_NULLPTR);
    ~dlg_refraction();
    Ui::dlg_refraction      *ui;
    dlg_listemesures        *Dlg_ListeMes;
    dlg_commentaires        *Dlg_Comments;

    QString                 ResultatPrescription(), ResultatObservation();
    int                     getidRefraction();

private slots:

    // les CheckBox, RadioButton,...etc...
    void                    Slot_AutorefRadioButton_Clicked();
    void                    Slot_CycloplegieCheckBox_Clicked();
    void                    Slot_DepoliCheckBox_Clicked(int etat);
    void                    Slot_DeuxMonturesPrescritradioButton_Clicked();
    void                    Slot_ODGCheckBox_Changed(int etat);
    void                    Slot_PrescritCheckBox_Changed(int etat);
    void                    Slot_PorteRadioButton_Clicked();
    void                    Slot_PressonCheckBox_Changed();
    void                    Slot_QuelleDistance_Clicked();
    void                    Slot_PlanCheckBox_Changed(int etat);
    void                    Slot_PrescriptionRadionButton_clicked();
    void                    Slot_RefractionRadioButton_Clicked();
    void                    Slot_RyserCheckBox_Clicked(int);
    void                    Slot_UneMonturePrescritRadioButton_Clicked();
    void                    Slot_VerresTeintesCheckBox_Changed(int etat);
    void                    Slot_VPrescritRadioButton_Clicked();

    //Les pushButton
    void                    Slot_AnnulPushButton_Clicked();
    void                    Slot_Commentaires();
    void                    Slot_ConvOGPushButton_Clicked();
    void                    Slot_ConvODPushButton_Clicked();
    void                    Slot_Detail_Clicked();
    void                    Slot_OKPushButton_Clicked();
    void                    Slot_OupsButtonClicked();
    void                    Slot_ResumePushButton_Clicked();

    // les autres zones de saisie
    void                    Slot_Controle_K();
    void                    Slot_BasePrismeTextODComboBox_Changed(int);
    void                    Slot_BasePrismeTextOGComboBox_Changed(int);
    void                    Slot_BasePrisme_ValueChanged();
    void                    Slot_Refraction_ValueChanged();
    void                    Slot_ReprendreButtonClicked();
    void                    Slot_CommentairePrescriptionTextEdit_Changed() ; //01.07.2014

private:
    Procedures              *proc;
    DataBase                *db;
    Patient                 *m_currentpatient;
    Acte                    *m_currentacte;
    int                     gidRefraction;
    QStringList             gstringListe1, gstringListe2;
    Refraction::Mesure      gMode;
    enum ModeSortie         {Annul, Imprime, OK};
    enum DateMesure         {Aujourdhui, Avant, NoDate};
    enum DistanceMesure     {Loin, Pres, Les2};
    enum Cycloplegie        {Dilatation, NoDilatation};
    bool                    ok;
    bool                    gAfficheDetail;
    bool                    EscapeFlag;
    bool                    FermeComment;
    double                  gSphereOD;
    double                  gCylindreOD;
    double                  gAxeCylindreOD;
    double                  gAddVPOD;
    double                  gSphereOG;
    double                  gCylindreOG;
    double                  gAxeCylindreOG;
    double                  gAddVPOG;
    int                     gFlagBugValidEnter; // Modif 17/04
    QString                 gResultatP, gResultatPO, gResultatAnondil, gResultatA, gResultatAdil, gResultatR, gResultatRdil, gResultatRnondil, m_commentaire, m_commentaireresume;
    QString                 gResultatPR, gResultatObservation;
    QMenu                   *qQmenuSup;
    UpLineEdit              *AVPOD, *AVLOD;
    UpLineEdit              *AVPOG, *AVLOG;

    bool                    Imprimer_Ordonnance();

    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    void                    closeEvent(QCloseEvent *);
    void                    keyPressEvent ( QKeyEvent * event );
    void                    Afficher_AVL_AVP(bool TrueFalse);
    void                    AfficherDetail(bool typ);
    void                    Afficher_AddVP(bool TrueFalse);
    void                    Afficher_AVP(bool);
    void                    RegleAffichageFiche();
    void                    Afficher_Oeil_Droit(bool TrueFalse);
    void                    Afficher_Oeil_Gauche(bool TrueFalse);
    QString                 CalculFormule_OD();
    QString                 CalculFormule_OG();
    QString                 CalculCommentaire();
    void                    Connect_Slots();

    QString                 CommentaireObligatoire();
    bool                    ControleCoherence();
    double                  ConvDouble(QString textdouble);
    Refraction::Mesure      ConvertMesure(QString Mesure);
    QString                 ConvertMesure(Refraction::Mesure Mesure);
    bool                    DeplaceVers(QWidget *widget, QString FinOuDebut = "");
    void                    DetruireLaMesure(Refraction* ref);
    void                    FermeFiche(enum ModeSortie);
    void                    InitDivers();
    void                    InitEventFilters();
    void                    Init_Value_DoubleSpin(QDoubleSpinBox *DoubleSpinBox, double ValeurDouble);
    void                    Init_variables();
    void                    InscriptRefraction();
    void                    InsertDonneesOphtaPatient();
    bool                    InsertRefraction();
    int                     LectureMesure(DateMesure Quand, Refraction::Mesure Mesure, Cycloplegie dilatation, int idrefraction, bool Affichage, QString FormuleOD = "", QString FormuleOG = "");
    void                    OuvrirListeMesures(QString SupOuRecup);
    void                    MajDonneesOphtaPatient();
    void                    MasquerObjetsOeilDecoche();
    QString                 QuelleDistance();
    QString                 QuelleMonture();
    QString                 QuelsYeux();
    QString                 QuelsVerres();
    void                    QuitteAddVP(UpDoubleSpinBox *obj);
    void                    PrefixePlus(QDoubleSpinBox *DoubleSpinBox);
    void                    RechercheMesureEnCours();
    QString                 RechercheResultat(QString Mesure, QString Cyclople, QString TYpLun);
    QString                 RechercheVerres();
    void                    ResumeObservation();
    void                    ResumePrescription();
    void                    ResumeRefraction();
    void                    UpdateDonneesOphtaPatient();
    QString                 Valeur(QString StringlValeur);


// les connexions aux appareils de mesure
private slots:
    void                    Slot_NouvMesureRefraction();
private:
    double                  gDioptrAstOD, gDioptrAstOG;
    QString                 mSphereOD, mCylOD, mAxeOD, mAddOD, mSphereOG, mCylOG, mAxeOG, mAddOG;

public:
    void                    AfficheMesureFronto();
    void                    AfficheMesureAutoref();
    void                    AfficheMesureRefracteur();
};

#endif // DLG_REFRACTION_H
