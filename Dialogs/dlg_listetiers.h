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

#ifndef DLG_LISTETIERS_H
#define DLG_LISTETIERS_H

#include "dlg_identificationtiers.h"

class dlg_listetiers : public UpDialog
{
    Q_OBJECT
public:
    explicit                dlg_listetiers(QWidget *parent = Q_NULLPTR);
    ~dlg_listetiers();
    bool                    listetiersmodifiee() const;

private:
    bool                    m_listemodifiee = false;

    QStandardItemModel      *m_model = Q_NULLPTR;
    UpLabel                 *wdg_label;
    QTreeView               *wdg_tierstree;
    UpLineEdit              *wdg_chercheuplineedit;
    WidgetButtonFrame       *wdg_buttonframe;
    QList<int>              m_listidtiersutilises;

    void                    ChoixButtonFrame();
    void                    Enablebuttons(QModelIndex idx);
    void                    EnregistreNouveauTiers();
    Tiers*                  getTiersFromIndex(QModelIndex idx);
    void                    ModifTiers(Tiers *man);
    void                    scrollToTiers(Tiers *man);
    void                    SupprTiers(Tiers *man);
    void                    ReconstruitTreeViewtiers(bool reconstruirelaliste = false, QString filtre = "");
};

#endif // DLG_LISTETIERS_H
