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

#ifndef DLG_LISTEIOLS_H
#define DLG_LISTEIOLS_H

#include "updialog.h"
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QTreeView>
#include "gbl_datas.h"
#include "updoublespinbox.h"
#include "widgetbuttonframe.h"
#include "dlg_identificationiol.h"

class dlg_listeiols : public UpDialog
{
    Q_OBJECT
public:
    explicit                dlg_listeiols(QWidget *parent = Q_NULLPTR);
    ~dlg_listeiols();
    bool                    listeIOLsmodifiee() const;

private:
    bool                    m_listemodifiee = false;
    QList<int>              m_listidiolsutilises;
    QStandardItemModel      *m_IOLsmodel = Q_NULLPTR;
    QStandardItemModel      *m_manufacturersmodel = Q_NULLPTR;
    UpComboBox              *wdg_manufacturerscbox;
    UpLabel                 *wdg_label;
    QTreeView               *wdg_iolstree;
    UpLineEdit              *wdg_chercheuplineedit;
    WidgetButtonFrame       *wdg_buttonframe;

    void                    ChoixButtonFrame();
    void                    ImportListeIOLS();
    void                    Enablebuttons(QModelIndex idx);
    void                    EnregistreNouveauIOL();
    IOL*                    getIOLFromIndex(QModelIndex idx);
    void                    ReconstruitListeManufacturers();                // la liste des fabricants
    void                    ModifIOL(IOL *iol);
    void                    resizeiolimage(int size = 8192);                               //! reprend toute la liste des images d'IOL pour les redimensionner au dessous du seuil
    void                    scrollToIOL(IOL *iol);
    void                    SupprIOL(IOL *iol);
    void                    ReconstruitTreeViewIOLs(bool reconstruirelaliste = false, QString filtre = "");
};

#endif // DLG_LISTEIOLS_H
