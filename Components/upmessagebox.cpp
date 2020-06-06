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

#include "upmessagebox.h"
#include <QMovie>

UpMessageBox::UpMessageBox(QWidget *parent) : UpDialog(parent)
{
    wdg_iconlbl             = new UpLabel();
    wdg_texteditlbl             = new UpLabel();
    wdg_infolbl           = new UpLabel();
    wdg_textlayout          = new QVBoxLayout();
    wdg_infolayout          = new QHBoxLayout();
    wdg_ReponsSmallButton   = Q_NULLPTR;
    wdg_ReponsPushButton    = Q_NULLPTR;
    wdg_texteditlbl             ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    wdg_infolbl           ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);

    wdg_textlayout      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    wdg_textlayout      ->addSpacerItem(new QSpacerItem(350,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    wdg_infolayout      ->addLayout(wdg_textlayout);
    wdg_infolayout      ->setSpacing(30);
    wdg_textlayout      ->setSpacing(5);
    wdg_textlayout      ->setContentsMargins(0,0,0,0);
    dlglayout()     ->insertLayout(0,wdg_infolayout);
    dlglayout()     ->setSizeConstraint(QLayout::SetFixedSize);

    setModal(true);
}

UpMessageBox::~UpMessageBox()
{
}

void UpMessageBox::addButton(UpSmallButton *button, enum UpSmallButton::StyleBouton Style)
{
    button->setUpButtonStyle(Style);
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, this, [=] {Repons(button);});
}

void UpMessageBox::addButton(UpPushButton *button)
{
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, this, [=] {Repons(button);});
}

void UpMessageBox::removeButton(UpSmallButton *button)
{
    for (int i=0; i<buttonslayout()->count();i++)
    {
        UpSmallButton *buttonARetirer =  dynamic_cast<UpSmallButton*>(buttonslayout()->itemAt(i)->widget());
        if (buttonARetirer!=Q_NULLPTR)
            if (buttonARetirer == button)
            {
                delete buttonARetirer;
                return;
            }
    }
}

void UpMessageBox::Repons(QPushButton *button)
{
    UpSmallButton *but = dynamic_cast<UpSmallButton*>(button);
    if (but != Q_NULLPTR)
        wdg_ReponsSmallButton = but;
    else
        wdg_ReponsPushButton = static_cast<UpPushButton*>(button);
    accept();
}

UpSmallButton* UpMessageBox::clickedButton() const
{
    return wdg_ReponsSmallButton;
}

UpPushButton* UpMessageBox::clickedpushbutton() const
{
    return wdg_ReponsPushButton;
}

void UpMessageBox::setIcon(enum Icon icn)
{
    switch (icn) {
    case Warning:
        wdg_iconlbl     ->setPixmap(QPixmap("://damn-icon.png").scaled(80,80));
        break;
    case Quest:
        wdg_iconlbl     ->setPixmap(QPixmap("://question.png").scaled(80,80));
        break;
    case Info:
        wdg_iconlbl     ->setPixmap(QPixmap("://information.png").scaled(80,80));
        break;
    case Critical:
        wdg_iconlbl     ->setPixmap(QPixmap("://cancel.png").scaled(80,80));
        break;
    case Print:
        wdg_iconlbl     ->setPixmap(QPixmap("://11865.png").scaled(80,80));
        break;
    }
    wdg_iconlbl     ->setFixedSize(80,80);
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
}

void UpMessageBox::setIconPixmap(QPixmap pix)
{
    wdg_iconlbl     ->setPixmap(pix);
    wdg_iconlbl     ->setFixedSize(pix.width(),pix.height());
    wdg_infolayout  ->insertWidget(0,wdg_iconlbl);
}

void UpMessageBox::UpMessageBox::setText(QString Text)
{
    if (Text == "")
        return;
    wdg_texteditlbl         ->setStyleSheet("border: 0px solid; background-color: rgb(200,200,200,0)");
    wdg_texteditlbl         ->setText("<b>" + Text + "</b>");
    wdg_texteditlbl         ->setWordWrap(true);
    wdg_texteditlbl         ->setFixedSize(Utils::CalcSize(Text));
    wdg_textlayout      ->insertWidget(1,wdg_texteditlbl);
}

void UpMessageBox::setInformativeText(QString Text)
{
    if (Text == "")
        return;
    wdg_infolbl     ->setStyleSheet("border: 0px solid; background-color: rgb(200,200,200,0)");
    wdg_infolbl     ->setText(Text);
    wdg_infolbl     ->setWordWrap(true);
    int position = 1;
    if (dynamic_cast<QLabel*>(wdg_textlayout->itemAt(1)->widget()) != Q_NULLPTR)
        position += 1;
    wdg_infolbl     ->setFixedSize(Utils::CalcSize(Text));
    wdg_textlayout      ->insertWidget(position,wdg_infolbl);
    wdg_textlayout      ->setSizeConstraint(QLayout::SetFixedSize);
}

void UpMessageBox::setDefaultButton(QPushButton *butt)
{
    butt->setDefault(true);
    butt->setFocus();
}

void UpMessageBox::Show(QWidget *parent, QString Text, QString InfoText)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
    msgbox  ->setAttribute(Qt::WA_DeleteOnClose);
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText.toHtmlEscaped());
    msgbox  ->setIcon(UpMessageBox::Quest);
    msgbox  ->AjouteLayButtons(UpDialog::ButtonOK);
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);
    msgbox  ->wdg_texteditlbl           ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl       ->setFixedSize(Utils::CalcSize(InfoText));

    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  dynamic_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
            connect(butt, &QPushButton::clicked, msgbox, &UpMessageBox::accept);
    }
    msgbox  ->exec();
}

int UpMessageBox::Watch(QWidget *parent, QString Text, QString InfoText, Buttons Butts)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
    msgbox  ->setAttribute(Qt::WA_DeleteOnClose);
//    QMovie movie("://forbidden.gif");
//    msgbox->lblIcon->setMovie (&movie);
//    msgbox->lblIcon     ->setFixedSize(80,80);
//    msgbox->infolayout  ->insertWidget(0,msgbox->lblIcon);
//    movie.start ();
    msgbox  ->setText(Text);
    UpTextEdit text(InfoText.replace("\n","<br>"));
    msgbox  ->setInformativeText(text.toHtml());
    msgbox  ->setIcon(UpMessageBox::Quest);
    msgbox  ->AjouteLayButtons(Butts);

    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  dynamic_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
        {
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, msgbox, [=] {msgbox->Repons(butt);});
            if (butt->ButtonStyle() == UpSmallButton::STARTBUTTON)
                butt->setText("OK");
        }
    }
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);
    msgbox  ->wdg_texteditlbl   ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl       ->setFixedSize(Utils::CalcSize(InfoText));
    if (msgbox  ->exec()>0)
        return msgbox->clickedButton()->ButtonStyle();
    else return UpSmallButton::CANCELBUTTON;
}

int UpMessageBox::Question(QWidget *parent, QString Text, QString InfoText, Buttons Butts, QStringList textlist)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText);
    msgbox  ->setIcon(UpMessageBox::Quest);
    msgbox  ->AjouteLayButtons(Butts);
    int k = 0;
    for (int i=0; i<msgbox->buttonslayout()->count();i++)
    {
        UpSmallButton *butt =  dynamic_cast<UpSmallButton*>(msgbox->buttonslayout()->itemAt(i)->widget());
        if (butt!=Q_NULLPTR)
        {
            if (textlist.size()>k)
                butt->setText(textlist.at(k));
            k++;
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, msgbox, [=] {msgbox->Repons(butt);});
        }
    }
    msgbox  ->wdg_texteditlbl           ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl         ->setFixedSize(Utils::CalcSize(InfoText));
    msgbox  ->dlglayout()       ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->buttonslayout()   ->setSpacing(50);
    int a = msgbox  ->exec();
    int repons = msgbox->clickedButton()->ButtonStyle();
    if (a==0)
        repons = UpSmallButton::CANCELBUTTON;
    delete msgbox;
    return repons;
}

void UpMessageBox::Information(QWidget *parent, QString Text, QString InfoText)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
    msgbox  ->setAttribute(Qt::WA_DeleteOnClose);
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText);
    msgbox  ->setIcon(UpMessageBox::Info);

    msgbox  ->AjouteLayButtons(UpDialog::ButtonOK);
    connect (msgbox->OKButton, &QPushButton::clicked, msgbox, [=] {msgbox->accept();});
    msgbox  ->wdg_texteditlbl       ->setFixedSize(Utils::CalcSize(Text));
    msgbox  ->wdg_infolbl   ->setFixedSize(Utils::CalcSize(InfoText));
    msgbox  ->dlglayout()   ->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->exec();
}
