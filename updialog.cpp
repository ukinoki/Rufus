/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "updialog.h"
#include <QDebug>

UpDialog::UpDialog(QString NomSettings, QString NomPosition, QWidget *parent) : QDialog(parent)
{
    EnregPosition   = true;
    Position        = NomPosition;
    NomFichIni      = NomSettings;
    SettingsIni     = new QSettings(NomFichIni, QSettings::IniFormat);
    restoreGeometry(SettingsIni->value(Position).toByteArray());
    AjouteLay();
    gMode           = "";
}

UpDialog::UpDialog(QWidget *parent) : QDialog(parent)
{
    AjouteLay();
    EnregPosition   = false;
}

void UpDialog::AjouteLay()
{
    laybuttons      = new QHBoxLayout();
    laybuttons      ->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Expanding));
    laybuttons      ->setSpacing(20);
    QVBoxLayout *globallay = new QVBoxLayout(this);
    globallay       ->addLayout(laybuttons);
    globallay       ->setContentsMargins(10,10,10,10);
    globallay       ->setSpacing(5);
}
void UpDialog::AjouteLayButtons(Buttons Button)
{
    // le Button Cancel est toujours le plus à gauche
    // Close le plus à droite et OK juste avant Close
    if (Button.testFlag(UpDialog::ButtonCancel))
    {
        CancelButton    = new UpSmallButton();
        CancelButton    ->setShortcut(QKeySequence("F12"));
        CancelButton    ->setUpButtonStyle(UpSmallButton::CANCELBUTTON);
        laybuttons      ->addWidget(CancelButton);
        connect(CancelButton,   &QPushButton::clicked, [=] {reject();});
    }
    if (Button.testFlag(UpDialog::ButtonPrint))
    {
        PrintButton     = new UpSmallButton();
        PrintButton     ->setUpButtonStyle(UpSmallButton::PRINTBUTTON);
        PrintButton     ->setShortcut(QKeySequence("Meta+P"));
        laybuttons      ->addWidget(PrintButton);
    }
    if (Button.testFlag(UpDialog::ButtonSuppr))
    {
        SupprButton     = new UpSmallButton();
        SupprButton     ->setUpButtonStyle(UpSmallButton::SUPPRBUTTON);
        laybuttons      ->addWidget(SupprButton);
    }
    if (Button.testFlag(UpDialog::ButtonEdit))
    {
        EditButton     = new UpSmallButton();
        EditButton     ->setUpButtonStyle(UpSmallButton::EDITBUTTON);
        laybuttons     ->addWidget(EditButton);
    }
    if (Button.testFlag(UpDialog::ButtonOK))
    {
        OKButton        = new UpSmallButton();
        OKButton        ->setUpButtonStyle(UpSmallButton::STARTBUTTON);
        OKButton        ->setShortcut(QKeySequence("Meta+Return"));
        laybuttons      ->addWidget(OKButton);
    }
    if (Button.testFlag(UpDialog::ButtonClose))
    {
        CloseButton     = new UpSmallButton();
        CloseButton     ->setUpButtonStyle(UpSmallButton::CLOSEBUTTON);
        CloseButton     ->setShortcut(QKeySequence("Meta+P"));
        laybuttons      ->addWidget(CloseButton);
    }
    UpdateTabOrder();
}

void UpDialog::UpdateTabOrder()
{
    if (laybuttons->findChildren<UpSmallButton*>().size()>1)
        for (int i=0; i<laybuttons->findChildren<UpSmallButton*>().size()-1; i++)
            setTabOrder(laybuttons->findChildren<UpSmallButton*>().at(i),laybuttons->findChildren<UpSmallButton*>().at(i+1));
}

void UpDialog::AjouteWidgetLayButtons(QWidget *widg, bool ALaFin)
{
    if (ALaFin)
        laybuttons->addWidget(widg);
    else
        laybuttons->insertWidget(0,widg);
    UpSmallButton *but = dynamic_cast<UpSmallButton*>(widg);
    if (but != NULL)
    {
        if (but->ButtonStyle() == UpSmallButton::CANCELBUTTON)
            but    ->setShortcut(QKeySequence("F12"));
        else if (but->ButtonStyle() == UpSmallButton::STARTBUTTON)
            but    ->setShortcut(QKeySequence("Meta+Return"));
        else if (but->ButtonStyle() == UpSmallButton::CLOSEBUTTON)
            but    ->setShortcut(QKeySequence("Meta+Return"));
        UpdateTabOrder();
    }
}

void UpDialog::setMode(QString mode)
{
     gMode = mode;
}

QString UpDialog::mode()
{
    return gMode;
}

void UpDialog::closeEvent(QCloseEvent *event)
{
    if (EnregPosition)
        SettingsIni->setValue(Position, saveGeometry());
    event->accept();
}

void UpDialog::setEnregPosition(bool a)
{
    EnregPosition = a;
}

void UpDialog::TuneSize()
{
    int larg    = 0;
    int haut    = 0;
    int r,t,l,b;
    layout()               ->getContentsMargins(&r,&t,&l,&b);
    larg        = larg + r + l;
    haut        = haut + t + b;
    for (int i=0; i< layout()->count(); i++)
    {
        if (layout()->itemAt(i)->sizeHint().width() > larg)
            larg += layout()->itemAt(i)->sizeHint().width();
        haut += layout()->itemAt(i)->sizeHint().height();
    }
    haut += (layout()->count()-1) * layout()->spacing();
    resize(larg, haut);
}

