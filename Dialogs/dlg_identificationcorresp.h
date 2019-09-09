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

#ifndef DLG_IDENTIFICATIONCORRESP_H
#define DLG_IDENTIFICATIONCORRESP_H

#include "database.h"
#include "uplabel.h"
#include "uptablewidget.h"
#include "procedures.h"
#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "flags.h"

namespace Ui {
class dlg_identificationcorresp;
}

class dlg_identificationcorresp : public UpDialog
{
    Q_OBJECT

public:
    enum Mode   {Creation, Modification};    Q_ENUM(Mode)
    explicit dlg_identificationcorresp(enum Mode mode, bool quelesmedecins, Correspondant *cor = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    Ui::dlg_identificationcorresp *ui;
    ~dlg_identificationcorresp();
    bool                identcorrespondantmodifiee() const;
    Correspondant*      correspondantrenvoye() const;

private:
    DataBase            *db                 = DataBase::I();
    Correspondant       *m_correspondant    = Q_NULLPTR;

    int                 m_idcorrespondant;
    bool                m_onlydoctors;
    bool                m_modifdatascor;
    QString             m_nomcor, m_prenomcor, m_sexecor;
    Mode                m_mode;
    QLineEdit           *wdg_CPlineedit, *wdg_villelineedit;
    VilleCPWidget       *wdg_villeCP;


    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                AfficheDossierAlOuverture();
    void                EnableOKpushButton();
    void                OKpushButtonClicked();
    void                ReconstruitListeSpecialites();
    void                RegleAffichage();

private slots:
    void                Slot_Majuscule();
};

#endif // DLG_IDENTIFICATIONCORRESP_H
