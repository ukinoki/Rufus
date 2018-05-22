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

#include "dlg_autresmesures.h"
#include <QMessageBox>

dlg_autresmesures::dlg_autresmesures(int *idPatAPasser, Procedures *procAPasser, enum mode mod,  QWidget *parent) :
    UpDialog(QDir::homePath() + NOMFIC_INI, "PositionsFiches/PositionTono", parent)
{
    proc        = procAPasser;
    gidPatient  = *idPatAPasser;
    db          = proc->getDataBase();
    mode        = mod;
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dynamic_cast<QVBoxLayout*>(layout())->setSizeConstraint(QLayout::SetFixedSize);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    connect (OKButton,   &QPushButton::clicked,   [=] {OKButtonClicked();});
    if (mode == TONO)
    {
        widgto      = new WidgTono(this);
        widgto      ->setFixedSize(275,95);
        dynamic_cast<QVBoxLayout*>(layout())->insertWidget(0,widgto);
        if (MODE_MESURE_TONOMETRIE_PAR_DEFAUT == "Air")          widgto->ui->AirRadioButton->setChecked(true);
        if (MODE_MESURE_TONOMETRIE_PAR_DEFAUT == "Aplanation")   widgto->ui->AplanationRadioButton->setChecked(true);
        if (MODE_MESURE_TONOMETRIE_PAR_DEFAUT == "Autre")        widgto->ui->AutreRadioButton->setChecked(true);
        widgto->ui->TOODSpinBox->installEventFilter(this);
        widgto->ui->TOOGSpinBox->installEventFilter(this);
    }
    CancelButton->installEventFilter(this);
    OKButton->installEventFilter(this);
 }

dlg_autresmesures::~dlg_autresmesures()
{
}

/*-----------------------------------------------------------------------------------------------------------------
-- Traitement des SLOTS / actions associees a chaque objet du formulaire et aux menus -----------------------------
-----------------------------------------------------------------------------------------------------------------*/
void    dlg_autresmesures::OKButtonClicked()
{
    switch (mode) {
    case TONO:
        EnregistreTono();
        break;
    default:
        break;
    }
}

//-------------------------------------------------------------------------------------
// Gestion des évènements
//-------------------------------------------------------------------------------------

bool dlg_autresmesures::eventFilter(QObject *obj, QEvent *event) // A REVOIR
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        // Fleche Gauche - -----------------------------------------
        if(keyEvent->key()==Qt::Key_Left)
            if (!obj->inherits("QPushButton")) return QWidget::focusPreviousChild();

        // Fleche Droite - -----------------------------------------
        if(keyEvent->key()==Qt::Key_Right)
            if (!obj->inherits("QPushButton")) return QWidget::focusNextChild();
        // Return - Idem Flèche Droite sauf sur les pshButton ---------------------------
        if(keyEvent->key()==Qt::Key_Return || keyEvent->key()==Qt::Key_Enter)
            if (!obj->inherits("QPushButton"))
                return QWidget::focusNextChild();
    }
    return QWidget::eventFilter(obj, event);
}

//-------------------------------------------------------------------------------------
// Enregistre la Tonométrie dans la table
//-------------------------------------------------------------------------------------
void dlg_autresmesures::EnregistreTono()
{
    QString TOD, TOG, Methode;
    TOD = widgto->ui->TOODSpinBox->text();
    TOG = widgto->ui->TOOGSpinBox->text();
    if (widgto->ui->AirRadioButton->isChecked())        Methode = "Air";
    if (widgto->ui->AutreRadioButton->isChecked())      Methode = "Autre";
    if (widgto->ui->AplanationRadioButton->isChecked()) Methode = "Aplanation";

    QString req = "INSERT INTO " NOM_TABLE_TONOMETRIE " (idPat, TOOD, TOOG, TODate, TOType) VALUES  (" + QString::number(gidPatient) + "," + TOD + "," + TOG + ", now(),'" + Methode + "')";

    QSqlQuery EnregTOQuery (req,db);
    proc->TraiteErreurRequete(EnregTOQuery,req,tr("Impossible de sauvegarder la mesure!"));

    accept();
}

