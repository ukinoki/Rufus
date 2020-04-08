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

#ifndef DLG_IDENTIFICATIONTIERS_H
#define DLG_IDENTIFICATIONTIERS_H

#include <QDialog>

#include "procedures.h"
#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "flags.h"

namespace Ui {
class dlg_identificationtiers;
}

class dlg_identificationtiers : public UpDialog
{
    Q_OBJECT
public:
    enum Mode   {Creation, Modification};    Q_ENUM(Mode)
    explicit dlg_identificationtiers(enum Mode mode, Tiers *trs = Q_NULLPTR, QWidget *parent = Q_NULLPTR);
    Ui::dlg_identificationtiers *ui;
    ~dlg_identificationtiers();
    Tiers* currentTiers() const { return m_currenttiers; }
    void    setnomtiers(QString nom) const;

private:
    DataBase                    *db                = DataBase::I();
    Tiers                       *m_currenttiers    = Q_NULLPTR;

    Mode                        m_mode;
    QLineEdit                   *wdg_CPlineedit, *wdg_villelineedit;
    VilleCPWidget               *wdg_villeCP;
    QHash<QString, QVariant>    m_listbinds;


    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                AfficheDatasTiers();
    void                EnableOKpushButton();
    void                Majuscule(QLineEdit *ledit);
    void                OKpushButtonClicked();
};

#endif // DLG_IDENTIFICATIONTIERS_H
