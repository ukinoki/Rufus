/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "dlg_choixdate.h"
#include "ui_dlg_choixdate.h"

dlg_choixdate::dlg_choixdate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_choixdate)
{
    ui->setupUi(this);

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

    connect(ui->OKupPushButton,                 SIGNAL(clicked()),      this,       SLOT(accept()));
    connect(ui->AnnulupPushButton,              SIGNAL(clicked()),      this,       SLOT(reject()));
    connect(ui->JourradioButton,                SIGNAL(clicked()),      this,       SLOT(Slot_AfficheDates()));
    connect(ui->SemaineradioButton,             SIGNAL(clicked()),      this,       SLOT(Slot_AfficheDates()));
    connect(ui->MoisradioButton,                SIGNAL(clicked()),      this,       SLOT(Slot_AfficheDates()));
    connect(ui->AnneeradioButton,               SIGNAL(clicked()),      this,       SLOT(Slot_AfficheDates()));

    connect(ui->PlusDateDebutpushButton,        SIGNAL(clicked()),      this,       SLOT(Slot_ModifDate()));
    connect(ui->MoinsDateDebutpushButton,       SIGNAL(clicked()),      this,       SLOT(Slot_ModifDate()));
    connect(ui->PlusDateFinpushButton,          SIGNAL(clicked()),      this,       SLOT(Slot_ModifDate()));
    connect(ui->MoinsDateFinpushButton,         SIGNAL(clicked()),      this,       SLOT(Slot_ModifDate()));
    connect(ui->PlusDebutPeriodepushButton,     SIGNAL(clicked()),      this,       SLOT(Slot_ModifDate()));
    connect(ui->MoinsFinPeriodepushButton,      SIGNAL(clicked()),      this,       SLOT(Slot_ModifDate()));

    ui->AnnulupPushButton           ->setFixedSize(100,46);
    ui->OKupPushButton              ->setFixedSize(105,46);
    installEventFilter(this);
}

dlg_choixdate::~dlg_choixdate()
{
    delete ui;
}

void    dlg_choixdate::Slot_AfficheDates()
{
    bool a = (sender() != ui->JourradioButton);
    ui->JusquAulabel                ->setVisible(a);
    ui->JusquAdateEdit              ->setVisible(a);
    ui->PlusDateFinpushButton       ->setVisible(a);
    ui->MoinsDateFinpushButton      ->setVisible(a);
    ui->PlusDebutPeriodepushButton  ->setVisible(a);
    ui->MoinsFinPeriodepushButton   ->setVisible(a);

    if (sender() == ui->JourradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate::currentDate());
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

    if (sender() == ui->SemaineradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek()));
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

    if (sender() == ui->MoisradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(),1));
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

    if (sender() == ui->AnneeradioButton)
    {
        ui->DepuisdateEdit->setDate(QDate(QDate::currentDate().year(),1,1));
        ui->JusquAdateEdit->setDate(QDate::currentDate());
    }

}
void dlg_choixdate::Slot_ModifDate()
{
    if (sender() == ui->PlusDateDebutpushButton && ui->DepuisdateEdit->date() < ui->JusquAdateEdit->date() && ui->DepuisdateEdit->date() < QDate::currentDate())
        ui->DepuisdateEdit->setDate(ui->DepuisdateEdit->date().addDays(1));
    if (sender() == ui->MoinsDateDebutpushButton)
        ui->DepuisdateEdit->setDate(ui->DepuisdateEdit->date().addDays(-1));
    if (sender() == ui->PlusDateFinpushButton && ui->JusquAdateEdit->date() < QDate::currentDate())
        ui->JusquAdateEdit->setDate(ui->JusquAdateEdit->date().addDays(1));
    if (sender() == ui->MoinsDateFinpushButton && ui->DepuisdateEdit->date() < ui->JusquAdateEdit->date())
        ui->JusquAdateEdit->setDate(ui->JusquAdateEdit->date().addDays(-1));
    if (sender() == ui->PlusDebutPeriodepushButton)
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
    if (sender() == ui->MoinsFinPeriodepushButton)
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
                accept();
        if (keyEvent->key() == Qt::Key_F12)
                reject();
    }
    return QWidget::eventFilter(obj, event);
}
