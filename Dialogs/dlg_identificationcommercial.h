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

#ifndef DLG_IDENTIFICATIONCOMMERCIAL_H
#define DLG_IDENTIFICATIONCOMMERCIAL_H

#include "procedures.h"
#include "updialog.h"
#include "cls_commercials.h"
#include "updoublevalidator.h"
#include "updoublespinbox.h"
#include "uptoolbar.h"
#include "uppushbutton.h"


class dlg_identificationcommercial : public UpDialog
{
public:
    enum                        Mode {Creation, Modification};    Q_ENUM(Mode)
    explicit                    dlg_identificationcommercial(enum Mode mode, Item *itm, QWidget *parent = Q_NULLPTR);
    dlg_identificationcommercial();
    void                        setNomcommercial(QString nom)   {wdg_nomcommercialline->setText(nom);}
    int                         idcurrentcommercial() const     { return (m_currentcommercial? m_currentcommercial->id() : 0); }

private:
    Mode                m_mode;
    Manufacturer        *m_currentmanufacturer  = Q_NULLPTR;
    QStandardItemModel  *m_manufacturersmodel;
    Commercial          *m_currentcommercial    = Q_NULLPTR;
    QHash<QString, QVariant>
                        m_listbinds;
    QComboBox           *wdg_manufacturercombo ;
    UpLineEdit          *wdg_nomcommercialline;
    UpLineEdit          *wdg_prenomcommercialline;
    UpLineEdit          *wdg_statutcommercialline;
    UpLineEdit          *wdg_mailcommercialline;
    UpLineEdit          *wdg_telephonecommercialline;

    bool                eventFilter(QObject *obj, QEvent *event);
    void                AfficheDatascommercial(Commercial *commercial);
    void                connectSignals();
    void                disconnectSignals();
    void                EnableOKpushButton();
    void                EnableWidget(bool a);
    void                OKpushButtonClicked();
};


#endif // DLG_IDENTIFICATIONCOMMERCIAL_H
