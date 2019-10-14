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

#ifndef DLG_AUTRESMESURES_H
#define DLG_AUTRESMESURES_H

#include "procedures.h"
#include "widgtono.h"
#include "ui_widgtono.h"
#include "widgpachy.h"
#include "ui_widgpachy.h"

namespace Ui {
class dlg_autresmesures;
}

class dlg_autresmesures : public UpDialog
{
    Q_OBJECT

public:
    ~dlg_autresmesures();
    enum Mode {TONO, PACHY};    Q_ENUM(Mode)
    explicit            dlg_autresmesures(enum Mode mod = TONO, QWidget *parent = Q_NULLPTR);
    QWidget*            Widget() const;

private:
    Mode                m_mode;
    WidgTono            *widgto;
    WidgPachy           *widgpa;
    QWidget             *m_widget;

    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                EnregistreTono();
    void                EnregistrePachy();
    void                OKButtonClicked();
    void                setWidget(QWidget *widget);
};

#endif // DLG_AUTRESMESURES_H
