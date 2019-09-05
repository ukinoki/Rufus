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

    QString                 ResultatPrescription() const;
    QString                 ResultatObservation() const;
    int                     idrefraction() const;

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
    Procedures              *proc   = Procedures::I();
    DataBase                *db     = DataBase::I();
    Patient                 *m_currentpatient;
    Acte                    *m_currentacte;
    int                     m_idrefraction;
    QStringList             m_stringliste1, m_stringliste2;
    Refraction::Mesure      m_mode;
    enum ModeSortie         {Annul, Imprime, OK};
    enum DateMesure         {Aujourdhui, Avant, NoDate};
    enum DistanceMesure     {Loin, Pres, Les2};
    enum Cycloplegie        {Dilatation, NoDilatation};

    bool                    m_ok;
    bool                    m_affichedetail;
    bool                    m_escapeflag;
    bool                    m_fermecommentaire;
    double                  m_sphereOD;
    double                  m_cylindreOD;
    double                  m_axecylindreOD;
    double                  m_additionVPOD;
    double                  m_sphereOG;
    double                  m_cylindreOg;
    double                  m_axecylindreOG;
    double                  m_additionVPOG;
    int                     m_flagbugvalidenter; // Modif 17/04
    QString                 m_resultFronto, m_resultPubliFronto;
    QString                 m_resultAutorefnonDilate, m_resultPubliAutoref, m_resultAutorefDilate;
    QString                 m_resultPubliRefraction,m_resultRefractionDilate, m_resultRefractionnonDilate;
    QString                 m_commentaire, m_commentaireresume;
    QString                 m_resultPrescription;
    QString                 m_resultObservation;
    UpLineEdit              *wdg_AVPOD, *wdg_AVLOD;
    UpLineEdit              *wdg_AVPOG, *wdg_AVLOG;

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
    Refraction*             InsertRefraction();
    Refraction*             LectureMesure(DateMesure Quand, Refraction::Mesure Mesure, Cycloplegie dilatation, QString FormuleOD = "", QString FormuleOG = "");
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
    void                    RemplitChamps(Refraction *ref);
    void                    ResumeObservation();
    void                    ResumePrescription();
    void                    ResumeRefraction();
    void                    UpdateDonneesOphtaPatient();
    QString                 Valeur(QString StringlValeur);


// les connexions aux appareils de mesure
private slots:
    void                    Slot_NouvMesureRefraction();
private:
    double                  m_mesureDioptrAstigmOD, m_mesureDioptrAstigmOG;
public:
    void                    AfficheMesureFronto();
    void                    AfficheMesureAutoref();
    void                    AfficheMesureRefracteur();
};

#endif // DLG_REFRACTION_H
