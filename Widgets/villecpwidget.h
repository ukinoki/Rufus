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

#ifndef VILLECPWIDGET_H
#define VILLECPWIDGET_H

#include "updelegate.h"
#include <QCompleter>
#include <QSound>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QListView>
#include "upmessagebox.h"
#include "database.h"
#include "gbl_datas.h"
#include "utils.h"
#include "database.h"
#include "cls_villes.h"

namespace Ui {
    class VilleCPWidget;
}

class VilleCPWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VilleCPWidget(Villes *villes, QWidget *parent = Q_NULLPTR);
    ~VilleCPWidget();
    Ui::VilleCPWidget       *ui;

    Villes          *villes() const;
    bool            isValid(bool avecmsgbox = false);
    bool            RechercheCP() {return m_rechercheCP;}

private:
    Villes          *m_villes;
    bool            m_rechercheCP = true;

    void            StartChercheVille();
    void            ChercheVille(bool confirmerleCP = true);
    void            StartChercheCodePostal();
    QString         ConfirmeVille(QString ville);
    void            ChercheCodePostal(bool confirmerlaville = true);

    QString         dialogList(QList<Ville*> &listvilles, VilleListModel::FieldName fieldName, QString headerName);
    void            Repons(QListView *lv, UpDialog *ud, QString &newValue);
    QWidget         *wdg_parent         = Q_NULLPTR;
    QCompleter      *complListVilles    = new QCompleter(this);
    QCompleter      *complListCP        = new QCompleter(this);
    QObject         *m_villeobject      = Q_NULLPTR;
    QObject         *m_CPobject         = Q_NULLPTR;
    QObject         *m_completerobject  = Q_NULLPTR;

    void connectrecherche();
    void disconnectrecherche();

    void            ChercheVilleBaseIndividual(QString nomville);
    void            ChercheCPBaseIndividual(QString nomville);

signals:
    void            villecpmodified();
};

#endif // VILLECPWIDGET_H
