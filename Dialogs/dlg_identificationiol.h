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

#ifndef DLG_IDENTIFICATIONIOL_H
#define DLG_IDENTIFICATIONIOL_H

#include "procedures.h"
#include "updialog.h"
#include "cls_interventions.h"
#include "updoublevalidator.h"

class dlg_identificationIOL : public UpDialog
{
public:
    enum Mode   {Creation, Modification};    Q_ENUM(Mode)
    explicit dlg_identificationIOL(enum Mode mode, IOL *iol = Q_NULLPTR, Manufacturer *man = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    dlg_identificationIOL();
    void    setNomIOL(QString nom) {wdg_nomiolline->setText(nom);}
    IOL*    currentIOL() const      { return m_currentIOL; }
    QHash<QString, QVariant> Listbinds() const  { return m_listbinds;  }

private:
    QStandardItemModel  *m_manufacturersmodel   = Q_NULLPTR;
    QStandardItemModel  *m_IOLsmodel            = Q_NULLPTR;
    Mode                m_mode;
    Manufacturer        *m_currentmanufacturer  = Q_NULLPTR;
    IOL                 *m_currentIOL           = Q_NULLPTR;
    QHash<QString, QVariant>    m_listbinds;
    QComboBox           *wdg_manufacturercombo ;
    UpLineEdit          *wdg_nomiolline;
    UpLineEdit          *wdg_Aoptline;
    UpLineEdit          *wdg_Aecholine;
    UpLineEdit          *wdg_ACDline;
    UpLineEdit          *wdg_haigisaline;
    UpLineEdit          *wdg_haigisbline;
    UpLineEdit          *wdg_haigiscline;
    UpLabel             *wdg_imgIOL;
    UpLineEdit          *wdg_materiauline;
    UpTextEdit          *wdg_remarquetxt;

    void                AfficheDatasIOL();
    void                EnableOKpushButton();
    void                OKpushButtonClicked();
};

#endif // DLG_IDENTIFICATIONIOL_H
