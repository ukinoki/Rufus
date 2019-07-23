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

#ifndef DLG_DOCSSCANNER_H
#define DLG_DOCSSCANNER_H

#include "updialog.h"
#include "procedures.h"
#include "upcombobox.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "uptablewidget.h"
#include "uptoolbar.h"
#include "dlg_message.h"

class dlg_docsscanner : public UpDialog
{
    Q_OBJECT
public:
    enum Mode          {Document, Facture, Echeancier};    Q_ENUM(Mode)
    Mode                gMode;
    explicit dlg_docsscanner(Item *item, Mode mode = Document, QString titre = "", QWidget *parent = Q_NULLPTR);
    ~dlg_docsscanner();
    bool                    getinitOK();
    void                    NavigueVers(QString);
    QMap<QString, QVariant> getdataFacture();

private:
    Procedures          *proc;
    DataBase            *db;
    int                 iditem;
    QMap<QString, QVariant> datafacture;
    UpLineEdit          *linetitre;
    QDateEdit           *editdate;
    UpComboBox          *typeDocCombo;
    UpTableWidget       *uptable;
    UpToolBar           *toolbar;
    UpPushButton        *dirsearchbutton;
    bool                initOK;
    QLabel              *inflabel;
    QList<QImage>       glistImg;
    QStringList         ListTypeExams;
    QString             docpath;
    QString             NomDirStockageImagerie;
    QString             fichierimageencours;
    bool                eventFilter(QObject *, QEvent *);
    void                ValideFiche();
    void                ChangeFile();
    bool                AccesDistant;
};

#endif // DLG_DOCSSCANNER_H
