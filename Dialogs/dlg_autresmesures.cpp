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

#include "dlg_autresmesures.h"

dlg_autresmesures::dlg_autresmesures(Mode mod,  QWidget *parent) :
    UpDialog(PATH_FILE_INI, "PositionsFiches/PositionTono", parent)
{
    m_mode            = mod;
    AjouteLayButtons(UpDialog::ButtonCancel | UpDialog::ButtonOK);
    dlglayout()->setSizeConstraint(QLayout::SetFixedSize);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    connect (OKButton,   &QPushButton::clicked,   this, [=] {OKButtonClicked();});
    if (m_mode == TONO)
    {
        widgto      = new WidgTono(this);
        widgto      ->setFixedSize(275,95);
        dlglayout()->insertWidget(0,widgto);
        widgto->ui->AirRadioButton->setChecked(true);
        widgto->ui->TOODSpinBox->installEventFilter(this);
        widgto->ui->TOOGSpinBox->installEventFilter(this);
        setWidget(widgto);
    }
    else if (m_mode == PACHY)
    {
        widgpa      = new WidgPachy(this);
        widgpa      ->setFixedSize(275,95);
        dlglayout()->insertWidget(0,widgpa);
        widgpa->ui->EchoRadioButton->setChecked(true);
        widgpa->ui->PachyODSpinBox->installEventFilter(this);
        widgpa->ui->PachyOGSpinBox->installEventFilter(this);
        setWidget(widgpa);
    }
    CancelButton->installEventFilter(this);
    OKButton->installEventFilter(this);
 }

dlg_autresmesures::~dlg_autresmesures()
{
}

void    dlg_autresmesures::OKButtonClicked()
{
    switch (m_mode) {
    case TONO:
        EnregistreTono();
        break;
    case PACHY:
        EnregistrePachy();
    }
}

//-------------------------------------------------------------------------------------
// Gestion des évènements
//-------------------------------------------------------------------------------------

bool dlg_autresmesures::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (qobject_cast<QPushButton *>(obj) == Q_NULLPTR)
        {
            // Fleche Gauche - -----------------------------------------
            if(keyEvent->key()==Qt::Key_Left)
                return QWidget::focusPreviousChild();
            // Fleche Droite - -----------------------------------------
            if(keyEvent->key()==Qt::Key_Right)
                return QWidget::focusNextChild();
            // Return - Idem Flèche Droite sauf sur les pshButton ---------------------------
            if(keyEvent->key()==Qt::Key_Return || keyEvent->key()==Qt::Key_Enter)
                return QWidget::focusNextChild();
        }
    }
    return QWidget::eventFilter(obj, event);
}

//-------------------------------------------------------------------------------------
// Enregistre la Tonométrie
//-------------------------------------------------------------------------------------
void dlg_autresmesures::EnregistreTono()
{
    Datas::I()->mesuretono->cleandatas();
    Datas::I()->mesuretono->setTOD(widgto->ui->TOODSpinBox->text().toInt());
    Datas::I()->mesuretono->setTOG(widgto->ui->TOOGSpinBox->text().toInt());
    QString Methode;
    if (widgto->ui->AirRadioButton->isChecked())        Methode = AIR_TO;
    if (widgto->ui->AutreRadioButton->isChecked())      Methode = AUTRE_TO;
    if (widgto->ui->AplanationRadioButton->isChecked()) Methode = APLANATION_TO;
    Datas::I()->mesuretono->setmodemesure(Tonometrie::ConvertMesure(Methode));
    accept();
}

//-------------------------------------------------------------------------------------
// Enregistre la pachymétrie
//-------------------------------------------------------------------------------------
void dlg_autresmesures::EnregistrePachy()
{
    Datas::I()->mesurepachy->cleandatas();
    Datas::I()->mesurepachy->setpachyOD(widgpa->ui->PachyODSpinBox->text().replace("µ","").toInt());
    Datas::I()->mesurepachy->setpachyOG(widgpa->ui->PachyOGSpinBox->text().replace("µ","").toInt());
    QString Methode;
    if (widgpa->ui->OptiqueRadioButton->isChecked())    Methode = OPTIQUE_PACHY;
    if (widgpa->ui->EchoRadioButton->isChecked())       Methode = ECHO_PACHY;
    if (widgpa->ui->OCTRadioButton->isChecked())        Methode = OCT_PACHY;
    Datas::I()->mesurepachy->setmodemesure(Pachymetrie::ConvertMesure(Methode));
    accept();
}

QWidget *dlg_autresmesures::Widget() const
{
    return  m_widget;
}

void dlg_autresmesures::setWidget(QWidget* widget)
{
    m_widget = widget;
}



