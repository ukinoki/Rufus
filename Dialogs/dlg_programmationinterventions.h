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

class dlg_programmationinterventions : public UpDialog
{
    Q_OBJECT
public:
    explicit            dlg_programmationinterventions(Patient *pat, QWidget *parent = Q_NULLPTR);
    ~dlg_programmationinterventions();
    bool                docimprime() const      { return m_docimprime; }

private:
    Procedures                      *proc = Procedures::I();
    QStandardItemModel              m_medecinsmodel, m_sessionsmodel, m_interventionsmodel, m_typeinterventionsmodel, m_manufacturersmodel, m_IOLsmodel;
    QFont                           m_font = QApplication::font();
    dlg_impressions                 *Dlg_Imprs;
    dlg_identificationmanufacturer  *Dlg_IdentManufacturer;
    bool                            Imprimer_Document(Patient *pat, User *user, QString titre, QString Entete, QString text, QDate date, QString nom, QString prenom,
                                          bool Prescription, bool ALD, bool AvecPrintDialog, bool AvecDupli = false, bool AvecChoixImprimante = false, bool Administratif = true);
    bool                            m_docimprime = false;

/*! les items */
    User                *m_currentchiruser          = Q_NULLPTR;                        //! le user dont on établit le programme opératoire
    Patient             *m_currentchirpatient       = Q_NULLPTR;                        //! le patient qu'on veut intégrer dans le programme
    SessionOperatoire   *m_currentsession           = Q_NULLPTR;                        //! la session en cours
    Intervention        *m_currentintervention      = Q_NULLPTR;                        //! l'intervention en cours
    TypeIntervention    *m_currenttypeintervention  = Q_NULLPTR;                        //! le type d'intervention en cours
    Manufacturer        *m_currentmanufacturer      = Q_NULLPTR;                        //! le fabricant en cours
    IOL                 *m_currentIOL               = Q_NULLPTR;                        //! l'IOL en cours

/*! les widgets */
    QMenu               *m_ctxtmenusessions;
    QMenu               *m_ctxtmenuinterventions;
    QTreeView           *wdg_interventionstreeView  = new QTreeView();
    QTreeView           *wdg_sessionstreeView       = new QTreeView();
    QComboBox           *wdg_listmedecinscombo;
    QComboBox           *wdg_manufacturercombo;
    WidgetButtonFrame   *wdg_buttonsessionsframe;
    WidgetButtonFrame   *wdg_buttoninterventionframe;

/*! les médecins */
    void                ChoixMedecin(int iduser);

/*! les sessions */
    void                ChoixSessionFrame();
    void                AfficheInterventionsSession(QModelIndex idx);
    void                RemplirTreeSessions(SessionOperatoire* session = Q_NULLPTR);
    void                MenuContextuelSessions();
    void                CreerSession();
    void                EditSession();
    void                SupprimeSession();

/*! les interventions */
    void                ChoixIntervention(QModelIndex idx);
    void                ChoixInterventionFrame();
    void                FicheIntervention();                                       // crée la fiche qui permet de modifier ou d'enregistrer une intervention
    void                ImprimeDoc(Patient *pat, Intervention *interv);
    void                SupprimeIntervention();
    void                RemplirTreeInterventions(Intervention *intervention = Q_NULLPTR);
    void                MenuContextuelInterventionsions();
    void                VerifExistIntervention(bool &ok, QComboBox *box);
    void                VerifFicheIntervention(bool &ok, QTimeEdit *timeedit, QComboBox *box, Patient *pat);
private slots:
    void                CreerFicheIntervention();
    void                ModifFicheIntervention();

/*! les types d'interventions */
private:
    void                ReconstruitListeTypeInterventions();
    void                CreerTypeIntervention(QString txt);

/*! les fabricants */
    void                ChoixManufacturer(int idx);
    void                ReconstruitListeManufacturers();
    void                VerifExistManufacturer(bool &ok);
/*! les IOLs */
    void                AfficheChoixIOL(int state);
    void                ChoixIOL(QModelIndex idx);
    void                CreerIOL(QString nomiol);
    void                ReconstruitListeIOLs(int idmanufacturer);                                        //! recalcule la liste des IOLs pour un fabricant
    void                VerifExistIOL(bool &ok, QComboBox *box);
};

#endif // DLG_PROGRAMMATIONINTERVENTIONS_H
