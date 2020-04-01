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

#ifndef dlg_listecorrespondants_H
#define dlg_listecorrespondants_H

#include "dlg_identificationcorresp.h"

class dlg_listecorrespondants : public UpDialog
{
    Q_OBJECT
public:
    explicit                dlg_listecorrespondants(QWidget *parent = Q_NULLPTR);
    ~dlg_listecorrespondants();
    bool                    listecorrespondantsmodifiee() const;

private:
    bool                    m_listemodifiee = false;

    QStandardItemModel      *m_model = Q_NULLPTR;
    UpLabel                 *wdg_label;
    QTreeView               *wdg_correspstree;
    UpLineEdit              *wdg_chercheuplineedit;
    WidgetButtonFrame       *wdg_buttonframe;

    void                    ChoixButtonFrame();
    void                    Enablebuttons();
    void                    EnregistreNouveauCorresp();
    Correspondant*          getCorrespondantFromIndex(QModelIndex idx);
    QList<UpStandardItem *> ListeMetiers();                // la liste des metiers
    void                    ModifCorresp(Correspondant *cor);
    void                    scrollToCorresp(Correspondant *cor);
    void                    SupprCorresp(Correspondant *cor);
    void                    ReconstruitTreeViewCorrespondants(bool reconstruirelaliste = false, QString filtre = "");
};

#endif // dlg_listecorrespondants_H
