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

#ifndef VILLECPWIDGET_H
#define VILLECPWIDGET_H

#include "qlabeldelegate.h"
#include <QCompleter>
#include <QSound>
#include <QSqlDatabase>
#include <QStandardItemModel>
#include <QtSql>
#include "upmessagebox.h"
#include "functormajpremierelettre.h"

namespace Ui {
class VilleCPWidget;
}

class VilleCPWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VilleCPWidget(QSqlDatabase initdb, QString NomtableVilles, QWidget *parent = Q_NULLPTR, QStringList listVilles = QStringList(), QStringList listCP = QStringList(), QString Son = "");
    ~VilleCPWidget();
    QString                 CP();
    QString                 Ville();
    Ui::VilleCPWidget       *ui;
    QStringList             listeCP;

private:
    FunctorMAJPremiereLettre fMAJPremiereLettre;
    UpDialog                *gAskCP, *gAskVille;
    QSqlDatabase            db;
    bool                    VilleAConfirmer, CPAConfirmer;
    QString                 NouveauCP,NouvVille, TableVilles;
    QString                 Alarme;

    bool                    eventFilter(QObject *obj, QEvent *event);

    void                    ChercheCPdepuisQLine();
    void                    ChercheVilledepuisQLine();
    void                    ChercheVille(bool confirmerleCP = true);
    QString                 ConfirmeVille(QString Ville);
    void                    ChercheCodePostal(bool confirmerlaville = true);
    bool                    VerifCoherence();
    QString                 CorrigeApostrophe(QString);
    bool                    TraiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage = "");

private slots:
    void                    Slot_ConfirmerLaVille();
    void                    Slot_ConfirmerLeCP();
    void                    Slot_ChercheCPdepuisCompleter();
    void                    Slot_ChercheVilledepuisCompleter();
    void                    Slot_ReponsCodePostal();
    void                    Slot_ReponsVille();
    void                    Slot_CPEnableOKbutton();
    void                    Slot_VilleEnableOKbutton();
    void                    Slot_EnableOKpushButton();

signals:
    void                    villecpmodified();
};

#endif // VILLECPWIDGET_H
