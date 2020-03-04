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

class dlg_programmationinterventions : public UpDialog
{
    Q_OBJECT
public:
    explicit            dlg_programmationinterventions(Patient *pat, QWidget *parent = Q_NULLPTR);
    ~dlg_programmationinterventions();

private:
    Procedures          *proc = Procedures::I();
    QStandardItemModel  m_medecins, m_sessions, m_interventions, m_typeinterventions;

/*! les items */
    User                *m_currentchiruser      = Q_NULLPTR;                        //! le user dont on établit le programme opératoire
    Patient             *m_currentchirpatient   = Q_NULLPTR;                        //! le patient qu'on veut intégrer dans le programme
    SessionOperatoire   *m_currentsession       = Q_NULLPTR;                        //! la session en cours
    Intervention        *m_currentintervention  = Q_NULLPTR;                        //! l'intervention en cours

/*! les widgets */
    QMenu               *m_ctxtmenusessions;
    QMenu               *m_ctextmenuinterventions;
    QTreeView           *wdg_interventionstreeView  = new QTreeView();
    QTreeView           *wdg_sessionstreeView       = new QTreeView();
    QComboBox           *wdg_listmedecinscombo;
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
    void                CreerIntervention();
    void                EditIntervention();
    void                SupprimeIntervention();
    void                RemplirTreeInterventions(Intervention *intervention = Q_NULLPTR);
    void                MenuContextuelInterventionsions();

/*! les types d'interventions */
    void                ReconstruitListeTypeInterventions();
    void                CreerTypeIntervention();

/*! les IOLs */
    void                AfficheChoixIOL(int state);
};

#endif // DLG_PROGRAMMATIONINTERVENTIONS_H
