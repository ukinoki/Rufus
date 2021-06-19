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

#ifndef DLG_PROGRAMMATIONINTERVENTIONS_H
#define DLG_PROGRAMMATIONINTERVENTIONS_H

#include <updialog.h>
#include <procedures.h>
#include "updoublespinbox.h"
#include <uptableview.h>
#include "widgetbuttonframe.h"
#include "dlg_impressions.h"
#include "ui_dlg_impressions.h"
#include "dlg_identificationmanufacturer.h"
#include "ui_dlg_identificationmanufacturer.h"
#include "dlg_identificationiol.h"
#include "dlg_listeiols.h"
#include "dlg_listemanufacturers.h"
#include "updoublevalidator.h"

class dlg_programmationinterventions : public UpDialog
{
    Q_OBJECT
public:
    explicit            dlg_programmationinterventions(Patient *pat, Acte *act = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    ~dlg_programmationinterventions();
    bool                docimprime() const      { return m_docimprime; }

private:
    Procedures                      *proc = Procedures::I();
    QStandardItemModel              *m_medecinsmodel            = new QStandardItemModel(this);
    QStandardItemModel              *m_sessionsmodel            = new QStandardItemModel(this);
    QStandardItemModel              *m_interventionsmodel       = new QStandardItemModel(this);
    QStandardItemModel              *m_typeinterventionsmodel   = new QStandardItemModel(this);
    QStandardItemModel              *m_manufacturersmodel       = new QStandardItemModel(this);
    QStandardItemModel              *m_IOLsmodel                = new QStandardItemModel(this);
    QFont                           m_font = QApplication::font();

    bool                eventFilter(QObject *obj, QEvent *event);
    void                EnregistreIncident(Item *itm);
    void                ModifStatutActeCorrespondant(int idacte);                       /*! va rechercher s'il existe un acte correspondant à cette intervention (même patient, même date)
                                                                                        * dans la liste des actes en cours et va modifier sa propriété m_intervention
                                                                                        */
/*! les items */
    Acte                *m_currentchiracte          = Q_NULLPTR;                        //! l'acte concerné par le programme opératoire
    User                *m_currentchiruser          = Q_NULLPTR;                        //! le user dont on établit le programme opératoire
    Patient             *m_currentchirpatient       = Q_NULLPTR;                        //! le patient qu'on veut intégrer dans le programme

    //! l'intervention en cours
    Intervention*       currentintervention() const                     { return Datas::I()->interventions->currentintervention(); }
    void                setcurrentintervention(Intervention *interv)    { Datas::I()->interventions->setcurrentintervention(interv); }

    //! la session en cours
    SessionOperatoire*  currentsession() const                          { return Datas::I()->sessionsoperatoires->currentsession(); }
    void                setcurrentsession(SessionOperatoire *session)   { Datas::I()->sessionsoperatoires->setcurrentsession(session); }

    TypeIntervention    *m_currenttypeintervention  = Q_NULLPTR;                        //! le type d'intervention en cours
    Manufacturer        *m_currentmanufacturer      = Q_NULLPTR;                        //! le fabricant en cours
    IOL                 *m_currentIOL               = Q_NULLPTR;                        //! l'IOL en cours

/*! les widgets */
    QMenu               *m_ctxtmenusessions;
    QMenu               *m_ctxtmenuinterventions;
    QStringList          m_IOLcompleterlist;
    QStringList          m_manufacturercompleterlist;
    QTreeView           *wdg_interventionstreeView  = new QTreeView();
    QTreeView           *wdg_sessionstreeView       = new QTreeView();
    QCheckBox           *wdg_IOLchk;
    QComboBox           *wdg_listmedecinscombo;
    QComboBox           *wdg_manufacturercombo;
    QComboBox           *wdg_IOLcombo;
    UpLabel             *wdg_lblinterventions;
    UpPushButton        *wdg_incidentbutt;
    UpPushButton        *wdg_commandeIOLbutt;
    UpPushButton        *wdg_manufacturerbutt;
    UpPushButton        *wdg_IOLbutt;
    UpPushButton        *wdg_choixIOLbutt;
    WidgetButtonFrame   *wdg_buttonsessionsframe;
    WidgetButtonFrame   *wdg_buttoninterventionframe;
    UpDoubleSpinBox     *wdg_pwrIOLspinbox;
    UpDoubleSpinBox     *wdg_cylIOLspinbox;
    QWidget             *wdg_choixcylwdg;

/*! les médecins */
    void                ChoixMedecin(int iduser);

/*! les sessions */
    void                ChoixSessionFrame();
    void                AfficheInterventionsSession(QModelIndex idx);
    void                EnregistreIncidentSession()             {EnregistreIncident(currentsession());}
    void                RemplirTreeSessions();
    void                MenuContextuelSessions();
    void                FicheSession(SessionOperatoire *session = Q_NULLPTR);                                           //! crée la fiche qui permet de modifier ou d'enregistrer une session
    void                ModifSession();
    void                SupprimeSession();

/*! les interventions */
    void                ChoixIntervention(QModelIndex idx);
    void                ChoixInterventionFrame();
    void                EnregistreIncidentIntervention()        {EnregistreIncident(currentintervention());}
    void                FicheIntervention(Intervention *interv = Q_NULLPTR);                                            //! crée la fiche qui permet de modifier ou d'enregistrer une intervention
    void                SupprimeIntervention();
    void                RemplirTreeInterventions(Intervention *intervention = Q_NULLPTR);
    void                PositionneTreeInterventionsSurIntervention(Intervention* interv);
    void                MenuContextuelInterventionsions();
    void                VerifExistIntervention(bool &ok, QComboBox *box);
    void                VerifFicheIntervention(bool &ok, QTimeEdit *timeedit, QComboBox *box, Patient *pat);
private slots:
    void                CreerFicheIntervention();
    void                ModifIntervention();

/*! les types d'interventions */
private:
    void                ReconstruitListeTypeInterventions();
    void                FicheTypeIntervention(QString txt);

/*! les fabricants */
    void                ChoixManufacturer(int idx);
    void                FicheListeManufacturers();
    void                ReconstruitListeManufacturers(int idmanufacturer = 0);

/*! les IOLs */
    void                CalcRangeBox(IOL* iol = Q_NULLPTR);
    void                FicheListeIOLs();
    void                ReconstruitListeIOLs(int idmanufacturer, int idiol = 0);                                        //! recalcule la liste des IOLs pour un fabricant

/*! les impressions */
    void                ImprimeSession();                                                                               //! imprime le programme pératoire
    void                ImprimeRapportIncident();                                                                       //! imprime le rapport des incidents d'une session
    void                ImprimeListeIOLsSession();                                                                      //! imprime la liste des IOLS nécessaires pour une session, classés par fabricant
    void                FicheImpressions(Patient *pat, Intervention *interv);                                           //! ouvre la fiche dlg_impressions et prépare la liste de documents à imprimer
    bool                m_docimprime = false;

signals:
    void                updateHtml(Patient *pat);

};

#endif // DLG_PROGRAMMATIONINTERVENTIONS_H
