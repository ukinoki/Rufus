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

#ifndef DLG_REFRACTION_H
#define DLG_REFRACTION_H

#include "updoublespinbox.h"
#include "updoublevalidator.h"

#include "dlg_listecommentaires.h"
#include "dlg_refractionlistemesures.h"


namespace Ui {
    class dlg_refraction;
}

class dlg_refraction : public QDialog
{
    Q_OBJECT
public:
    enum ModeOuverture {
                Auto,           //!> la fiche est lancée par l'envoi de données depuis le refracteur
                Manuel          //!> la fiche est lancée par l'utilisateur
                };  Q_ENUM(ModeOuverture)
    dlg_refraction(ModeOuverture modeouverture, QWidget *parent = Q_NULLPTR);
    ~dlg_refraction();
    Ui::dlg_refraction          *ui;

    QString                 ResultatPrescription() const;
    QString                 ResultatObservation() const;
    int                     idrefraction() const;

private:

    Procedures              *proc   = Procedures::I();
    DataBase                *db     = DataBase::I();
    int                     m_idrefraction;
    QStringList             m_stringliste1, m_stringliste2;
    Refraction::Mesure      m_mode;
    ModeOuverture           m_modeouverture;
    enum ModeSortie         {Annul, Imprime, OK};
    enum DateMesure         {Aujourdhui, Avant, NoDate};
    upDoubleValidator       *m_val;

    bool                    m_ok;
    bool                    m_affichedetail;
    bool                    m_escapeflag;
    bool                    m_fermecommentaire;
    int                     m_flagbugvalidenter; // Modif 17/04
    QString                 m_resultFronto, m_resultPubliFronto;
    QString                 m_resultAutorefnonDilate, m_resultPubliAutoref, m_resultAutorefDilate;
    QString                 m_resultPubliRefraction,m_resultRefractionDilate, m_resultRefractionnonDilate;
    QString                 m_commentaire, m_commentaireresume;
    QString                 m_resultPrescription;
    QString                 m_resultObservation;
    UpLineEdit              *wdg_AVPOD, *wdg_AVLOD;
    UpLineEdit              *wdg_AVPOG, *wdg_AVLOG;

    // les CheckBox, RadioButton,...etc...
    void                    CycloplegieCheckBox_Clicked();
    void                    DepoliCheckBox_Clicked(QCheckBox* check, int etat);
    void                    ODGCheckBox_Changed(QCheckBox* check, int etat);
    void                    PrescritCheckBox_Changed(QCheckBox* check,int etat);
    void                    RadioButtonFronto_Clicked();
    void                    PressonCheckBox_Changed();
    void                    PlanCheckBox_Changed(QCheckBox* check, int etat);
    void                    RyserCheckBox_Clicked(QCheckBox* check, int etat);
    void                    VerresTeintesCheckBox_Changed();

    //Les pushButton
    void                    AnnulPushButton_Clicked();
    void                    Commentaires();
    void                    ConvOGPushButton_Clicked();
    void                    ConvODPushButton_Clicked();
    void                    Detail_Clicked();
    void                    OKPushButton_Clicked();
    void                    OupsButtonClicked();
    void                    ResumePushButton_Clicked();

    // les autres zones de saisie
    void                    BasePrismeTextODComboBox_Changed(int);
    void                    BasePrismeTextOGComboBox_Changed(int);
    void                    BasePrisme_ValueChanged(QSpinBox *box);
    void                    Refraction_ValueChanged();
    void                    CommentairePrescriptionTextEdit_Changed() ; //01.07.2014

    bool                    Imprimer_Ordonnance();

    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    void                    closeEvent(QCloseEvent *);
    void                    keyPressEvent ( QKeyEvent * event );
    // les CheckBox, RadioButton,...etc...
    void                    RadioButtonAutoref_Clicked();
    void                    DeuxMonturesPrescritradioButton_Clicked();
    void                    RadionButtonPrescription_clicked();
    void                    RadioButtonRefraction_Clicked();
    void                    UneMonturePrescritRadioButton_Clicked();
    void                    VPrescritRadioButton_Clicked();
    void                    ReprendreButtonClicked();

    void                    AfficheKerato();
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
    void                    ConnectSignals();

    MesureRefraction*       CalcMesureRefraction();
    QString                 CommentaireObligatoire();
    bool                    ControleCoherence();
    bool                    DeplaceVers(QWidget *widget, QString FinOuDebut = "");
    void                    DetruireLaMesure(Refraction* ref);
    void                    FermeFiche(enum ModeSortie);
    void                    InitDivers();
    void                    InitEventFilters();
    void                    Init_variables();
    void                    InscriptRefraction();
    void                    InsertDonneesOphtaPatient();
    Refraction*             InsertRefraction();
    Refraction*             LectureMesure(DateMesure Quand, Refraction::Mesure Mesure, Refraction::Cycloplegie dilatation, QString FormuleOD = "", QString FormuleOG = "");
    void                    OuvrirListeMesures(dlg_refractionlistemesures::Mode mode);
    void                    MajDonneesOphtaPatient();
    void                    MasquerObjetsOeilDecoche();
    QString                 QuelleDistance();
    QString                 QuelleMonture();
    QString                 QuelsYeux();
    QString                 QuelsVerres();
    void                    QuitteAddVP(UpDoubleSpinBox *obj);
    void                    RechercheMesureEnCours();
    QString                 RechercheResultat(Refraction::Mesure mesure, Refraction::Cycloplegie dilatation, Refraction::Distance typlun = Refraction::Inconnu);
    QString                 RechercheVerres();
    void                    RemplitChamps(Refraction *ref);
    void                    ResumeObservation();
    void                    ResumePrescription();
    void                    ResumeRefraction();
    void                    UpdateDonneesOphtaPatient();
    QString                 Valeur(QString StringlValeur);


// les connexions aux appareils de mesure
private:
    void                    NouvMesureRefraction(Procedures::TypeMesure TypeMesure);
public:
    void                    AfficheMesureFronto();
    void                    AfficheMesureAutoref();
    void                    AfficheMesureRefracteur();
};

#endif // DLG_REFRACTION_H
