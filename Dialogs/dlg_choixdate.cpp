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

#include "dlg_choixdate.h"
#include "ui_dlg_choixdate.h"

dlg_choixdate::dlg_choixdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_choixdate)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    map_date = QMap<Utils::Period, QDate>();
    ui->JourradioButton             ->setChecked(true);
    ui->DepuisdateEdit              ->setDate(QDate::currentDate());
    ui->Depuislabel                 ->setVisible(false);
    ui->JusquAulabel                ->setVisible(false);
    ui->JusquAdateEdit              ->setDate(QDate::currentDate());
    ui->JusquAdateEdit              ->setVisible(false);
    ui->PlusDateFinpushButton       ->setVisible(false);
    ui->MoinsDateFinpushButton      ->setVisible(false);
    ui->PlusDebutPeriodepushButton  ->setVisible(false);
    ui->MoinsFinPeriodepushButton   ->setVisible(false);

    connect(ui->OKupPushButton,                 &QPushButton::clicked,  this,    [=] {Fermefiche();});
    connect(ui->AnnulupPushButton,              &QPushButton::clicked,  this,    [=] {reject();});
    connect(ui->JourradioButton,                &QPushButton::clicked,  this,    [=] {AfficheDates(ui->JourradioButton);});
    connect(ui->SemaineradioButton,             &QPushButton::clicked,  this,    [=] {AfficheDates(ui->SemaineradioButton);});
    connect(ui->MoisradioButton,                &QPushButton::clicked,  this,    [=] {AfficheDates(ui->MoisradioButton);});
    connect(ui->AnneeradioButton,               &QPushButton::clicked,  this,    [=] {AfficheDates(ui->AnneeradioButton);});

    connect(ui->PlusDateDebutpushButton,        &QPushButton::clicked,  this,    [=] {ModifDate(ui->PlusDateDebutpushButton);});
    connect(ui->MoinsDateDebutpushButton,       &QPushButton::clicked,  this,    [=] {ModifDate(ui->MoinsDateDebutpushButton);});
    connect(ui->PlusDateFinpushButton,          &QPushButton::clicked,  this,    [=] {ModifDate(ui->PlusDateFinpushButton);});
    connect(ui->MoinsDateFinpushButton,         &QPushButton::clicked,  this,    [=] {ModifDate(ui->MoinsDateFinpushButton);});
    connect(ui->PlusDebutPeriodepushButton,     &QPushButton::clicked,  this,    [=] {ModifDate(ui->PlusDebutPeriodepushButton);});
    connect(ui->MoinsFinPeriodepushButton,      &QPushButton::clicked,  this,    [=] {ModifDate(ui->MoinsFinPeriodepushButton);});

    ui->AnnulupPushButton           ->setFixedSize(100,46);
    ui->OKupPushButton              ->setFixedSize(105,46);
    installEventFilter(this);
}

dlg_choixdate::~dlg_choixdate()
{
    delete ui;
}

void    dlg_choixdate::AfficheDates(QWidget *widg)
{
    bool a = (widg != ui->JourradioButton);
    ui->JusquAulabel                ->setVisible(a);
    ui->JusquAdateEdit              ->setVisible(a);
    ui->PlusDateFinpushButton       ->setVisible(a);
    ui->MoinsDateFinpushButton      ->setVisible(a);
    ui->PlusDebutPeriodepushButton  ->setVisible(a);
    ui->MoinsFinPeriodepushButton   ->setVisible(a);

    if (widg == ui->JourradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate::currentDate());
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

    else if (widg == ui->SemaineradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek()));
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

    else if (widg == ui->MoisradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(),1));
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

    else if (widg == ui->AnneeradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate(QDate::currentDate().year(),1,1));
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

}
void dlg_choixdate::ModifDate(QWidget *widg)
{
    if (widg == ui->PlusDateDebutpushButton && ui->DepuisdateEdit->date() < ui->JusquAdateEdit->date() && ui->DepuisdateEdit->date() < QDate::currentDate())
        ui->DepuisdateEdit->setDate(ui->DepuisdateEdit->date().addDays(1));
    if (widg == ui->MoinsDateDebutpushButton)
        ui->DepuisdateEdit->setDate(ui->DepuisdateEdit->date().addDays(-1));
    if (widg == ui->PlusDateFinpushButton && ui->JusquAdateEdit->date() < QDate::currentDate())
        ui->JusquAdateEdit->setDate(ui->JusquAdateEdit->date().addDays(1));
    if (widg == ui->MoinsDateFinpushButton && ui->DepuisdateEdit->date() < ui->JusquAdateEdit->date())
        ui->JusquAdateEdit->setDate(ui->JusquAdateEdit->date().addDays(-1));
    if (widg == ui->PlusDebutPeriodepushButton)
    {
        if (ui->SemaineradioButton->isChecked())
        {
            int JourDeLaSemaine= ui->DepuisdateEdit->date().dayOfWeek();
            QDate DateDebut;
            DateDebut = ui->DepuisdateEdit->date().addDays(8 - JourDeLaSemaine);
            if (DateDebut > QDate::currentDate()) return;
            ui->DepuisdateEdit->setDate(DateDebut);
            ui->JusquAdateEdit->setDate(QDate::currentDate());
        }
        if (ui->MoisradioButton->isChecked())
        {
            int Mois  = ui->DepuisdateEdit->date().month();
            int Annee = ui->DepuisdateEdit->date().year();
            if (Mois == 12)
            {
                Annee += 1;
                Mois  = 1;
            }
            else Mois += 1;
            if (QDate(Annee,Mois,1) > QDate::currentDate()) return;
            int Jour = QDate(Annee,Mois,1).daysInMonth();
            ui->DepuisdateEdit->setDate(QDate(Annee,Mois,1));
            ui->JusquAdateEdit->setDate(QDate(Annee,Mois,Jour));
        }
        if (ui->AnneeradioButton->isChecked())
        {
            int Annee = ui->DepuisdateEdit->date().year() + 1;
            if (QDate(Annee,1,1) > QDate::currentDate()) return;
            ui->DepuisdateEdit->setDate(QDate(Annee,1,1));
            ui->JusquAdateEdit->setDate(QDate(Annee,12,31));
        }
    }
    else if (widg == ui->MoinsFinPeriodepushButton)
    {
        if (ui->SemaineradioButton->isChecked())
        {
            int JourDeLaSemaine = ui->DepuisdateEdit->date().dayOfWeek();
            QDate DateDebut;
            DateDebut = ui->DepuisdateEdit->date().addDays(-6 - JourDeLaSemaine);
            ui->DepuisdateEdit->setDate(DateDebut);
            ui->JusquAdateEdit->setDate(DateDebut.addDays(6));
        }
        if (ui->MoisradioButton->isChecked())
        {
            int Mois  = ui->DepuisdateEdit->date().month();
            int Annee = ui->DepuisdateEdit->date().year();
            if (Mois == 1)
            {
                Annee -= 1;
                Mois  = 12;
            }
            else Mois -= 1;
            int Jour = QDate(Annee,Mois,1).daysInMonth();
            ui->DepuisdateEdit->setDate(QDate(Annee,Mois,1));
            ui->JusquAdateEdit->setDate(QDate(Annee,Mois,Jour));
        }
        if (ui->AnneeradioButton->isChecked())
        {
            int Annee = ui->DepuisdateEdit->date().year() - 1;
            ui->DepuisdateEdit->setDate(QDate(Annee,1,1));
            ui->JusquAdateEdit->setDate(QDate(Annee,12,31));
        }
    }
}
bool dlg_choixdate::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key()==Qt::Key_Enter)
            if (keyEvent->modifiers() == Qt::MetaModifier)
                Fermefiche();
        if (keyEvent->key() == Qt::Key_F12)
                reject();
    }
    return QWidget::eventFilter(obj, event);
}

void dlg_choixdate::Fermefiche()
{
        map_date[Utils::Debut] = ui->DepuisdateEdit->date();
        map_date[Utils::Fin] = (ui->JourradioButton->isChecked()? map_date[Utils::Debut] : ui->JusquAdateEdit->date());
        accept();
}
