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

#ifndef VILLECPWIDGET_H
#define VILLECPWIDGET_H

#include "qlabeldelegate.h"
#include <QCompleter>
#include <QSound>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include <QtSql>
#include "upmessagebox.h"
#include "database.h"
#include "gbl_datas.h"
#include "utils.h"

namespace Ui {
    class VilleCPWidget;
}

class VilleCPWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VilleCPWidget(Villes *villes, QWidget *parent = Q_NULLPTR, QString Son = NOM_ALARME);
    ~VilleCPWidget();
    Ui::VilleCPWidget       *ui;

    Villes          *villes() const;

private:
    QString         m_alarme;
    Villes          *m_villes;

    void            Slot_ChercheVille();
    void            ChercheVille(bool confirmerleCP = true);
    void            Slot_ChercheCodePostal();
    QString         ConfirmeVille(QString ville);
    void            ChercheCodePostal(bool confirmerlaville = true);

    QString         dialogList(QList<Ville*> &listData, QString fieldName, QString headerName);
    void            Repons(QListView *lv, UpDialog *ud, QString &newValue);

signals:
    void            villecpmodified();
};

#endif // VILLECPWIDGET_H
