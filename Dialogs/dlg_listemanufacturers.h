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

#ifndef dlg_listemanufacturers_H
#define dlg_listemanufacturers_H

#include "dlg_identificationmanufacturer.h"

class dlg_listemanufacturers : public UpDialog
{
    Q_OBJECT
public:
    explicit                dlg_listemanufacturers(QWidget *parent = Q_NULLPTR);
    ~dlg_listemanufacturers();
    bool                    listemanufacturersmodifiee() const;

private:
    bool                    m_listemodifiee = false;

    QStandardItemModel      *m_model = Q_NULLPTR;
    UpLabel                 *wdg_label;
    QTreeView               *wdg_manufacturerstree;
    UpLineEdit              *wdg_chercheuplineedit;
    WidgetButtonFrame       *wdg_buttonframe;
    QList<int>              m_listidmanufacturersutilises;

    void                    ChoixButtonFrame();
    void                    Enablebuttons();
    void                    EnregistreNouveauManufacturer();
    Manufacturer*           getmanufacturerFromIndex(QModelIndex idx);
    void                    ModifManufacturer(Manufacturer *man);
    void                    scrollToManufacturer(Manufacturer *man);
    void                    SupprManufacturer(Manufacturer *man);
    void                    ReconstruitTreeViewManufacturers(bool reconstruirelaliste = false, QString filtre = "");
};

#endif // dlg_listemanufacturers_H
