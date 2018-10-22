/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DLG_GESTIONLIEUX_H
#define DLG_GESTIONLIEUX_H

#include <QApplication>
#include <QDir>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTableView>
#include <QSqlQuery>
#include "dlg_message.h"
#include "functormajpremierelettre.h"
#include "macros.h"
#include "updialog.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "upmessagebox.h"
#include "widgetbuttonframe.h"

/* sert à gérer les lieux d'exercice
 * IDENTIQUE POUR RUFUS ET RUFUSADMIN*/

class dlg_GestionLieux : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_GestionLieux(QSqlDatabase db, QWidget *parent = Q_NULLPTR);
    ~dlg_GestionLieux();
private slots:
    void                    Slot_EnregLieux();
    void                    Slot_AfficheDetails(QModelIndex, QModelIndex);
    void                    Slot_ChoixButtonFrame(int i);
    void                    Slot_EnableOKButton();
    void                    Slot_EnregNouvLieu();
    void                    Slot_ModifLieu();

private:
    FunctorMAJPremiereLettre fmaj;
    QSqlDatabase            db;
    QTableView              *tabLM;
    UpDialog                *gLieuDialog;
    UpLabel                 *Adressuplbl;
    UpLineEdit              *leditnom;
    UpLineEdit              *leditadr1;
    UpLineEdit              *leditadr2;
    UpLineEdit              *leditadr3;
    UpLineEdit              *leditcp;
    UpLineEdit              *leditville;
    UpLineEdit              *ledittel;
    UpLineEdit              *leditfax;
    QStandardItemModel      *tabModel;
    WidgetButtonFrame       *widg;
    int                     idLieuAModifier;
    QString                 CorrigeApostrophe(QString RechAp);
    void                    CreerLieu();
    void                    ModifLieu();
    void                    SupprLieu();
    void                    ModifLieuxDialog();
    void                    ReconstruitModel();
    bool                    ValidationFiche();
};

#endif // DLG_GESTIONLIEUX_H
