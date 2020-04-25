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

#ifndef DLG_IDENTIFICATIONMANUFACTURER_H
#define DLG_IDENTIFICATIONMANUFACTURER_H

#include "procedures.h"
#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "flags.h"
#include <QDesktopServices>
#include "dlg_identificationcommercial.h"

namespace Ui {
class dlg_identificationmanufacturer;
}

class dlg_identificationmanufacturer : public UpDialog
{
    Q_OBJECT
public:
    enum Mode   {Creation, Modification};    Q_ENUM(Mode)
    explicit dlg_identificationmanufacturer(enum Mode mode, Manufacturer *man = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    Ui::dlg_identificationmanufacturer *ui;
    ~dlg_identificationmanufacturer();
    int idcurrentmanufacturer() const { return (m_currentmanufacturer? m_currentmanufacturer->id() : 0); }

private:
    DataBase                    *db                = DataBase::I();
    Manufacturer                *m_currentmanufacturer    = Q_NULLPTR;

    QRegExp                     rgx_nom             = QRegExp("[a-zA-Z0-9 .+-]*");
    QString                     m_nommanufacturer;
    Mode                        m_mode;
    QLineEdit                   *wdg_CPlineedit, *wdg_villelineedit;
    VilleCPWidget               *wdg_villeCP;
    QHash<QString, QVariant>    m_listbinds;
    QStandardItemModel          *m_commodel = Q_NULLPTR;
    WidgetButtonFrame           *wdg_buttonframe;

    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                AfficheDatasManufacturer();
    void                ChoixButtonFrame();
    void                Enablebuttons(QModelIndex idx);
    void                EnableOKpushButton();
    Commercial*         getCommercialFromIndex(QModelIndex idx);
    void                Majuscule(QLineEdit *ledit);
    void                OKpushButtonClicked();
    void                reconstruitCommercialsModel();
    void                EnregistreNouveauCommercial();
    void                ModifieCommercial(Commercial *com);
    void                SupprimeCommercial(Commercial *com);
};
#endif // DLG_IDENTIFICATIONMANUFACTURER_H
