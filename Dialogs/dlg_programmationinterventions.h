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
    WidgetButtonFrame   *wdg_buttondateframe;
    WidgetButtonFrame   *wdg_buttoninterventionframe;
    QComboBox           *wdg_listmedecinscombo;
    User                *m_currentuser;
    Patient             *m_currentpatient;
    QDate               m_currentdate = QDate::currentDate();
    User*               currentuser() const { return m_currentuser; }
    void                ChoixDateFrame();
    void                ChoixInterventionFrame();
    void                ChoixMedecin(int idx);
    void                CreerIntervention();
};

#endif // DLG_PROGRAMMATIONINTERVENTIONS_H
