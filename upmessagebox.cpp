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

#include "icons.h"
#include "upmessagebox.h"
#include <QMovie>

UpMessageBox::UpMessageBox(QWidget *parent) : UpDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    lblIcon             = new UpLabel();
    Textedt             = new UpLabel();
    InfoTextedt         = new UpLabel();
    textlayout          = new QVBoxLayout();
    infolayout          = new QHBoxLayout();
    globallayout        = dynamic_cast<QVBoxLayout*>(layout());
    ReponsSmallButton   = 0;
    ReponsPushButton    = 0;
    Textedt             ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    InfoTextedt         ->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);

    textlayout      ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    textlayout      ->addSpacerItem(new QSpacerItem(350,0,QSizePolicy::Expanding, QSizePolicy::Expanding));
    infolayout      ->addLayout(textlayout);
    infolayout      ->setSpacing(30);
    textlayout      ->setSpacing(5);
    textlayout      ->setContentsMargins(0,0,0,0);
    globallayout    ->insertLayout(0,infolayout);
    globallayout    ->setSizeConstraint(QLayout::SetFixedSize);

    setModal(true);
}

UpMessageBox::~UpMessageBox()
{
}

void UpMessageBox::addButton(UpSmallButton *button, enum UpSmallButton::StyleBouton Style)
{
    button->setUpButtonStyle(Style);
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, [=] {Repons(button);});
}

void UpMessageBox::addButton(UpPushButton *button)
{
    AjouteWidgetLayButtons(button);
    connect(button, &QPushButton::clicked, [=] {Repons(button);});
}

void UpMessageBox::removeButton(UpSmallButton *button)
{
    for (int i=0; i<laybuttons->count();i++)
    {
        UpSmallButton *buttonARetirer =  dynamic_cast<UpSmallButton*>(laybuttons->itemAt(i)->widget());
        if (buttonARetirer!=NULL)
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
    if (but != NULL)
        ReponsSmallButton = but;
    else
        ReponsPushButton = static_cast<UpPushButton*>(button);
    accept();
}

UpSmallButton* UpMessageBox::clickedButton()
{
    return ReponsSmallButton;
}

UpPushButton* UpMessageBox::clickedpushbutton()
{
    return ReponsPushButton;
}

void UpMessageBox::setIcon(enum Icon icn)
{
    switch (icn) {
    case Warning:
        lblIcon     ->setPixmap(Icons::pxOups().scaled(80,80)); //WARNING : icon scaled : px 80,80
        break;
    case Quest:
        lblIcon     ->setPixmap(Icons::pxQuestion().scaled(80,80)); //WARNING : icon scaled : px 80,80
        break;
    case Info:
        lblIcon     ->setPixmap(Icons::pxInformation().scaled(80,80)); //WARNING : icon scaled : px 80,80
        break;
    case Critical:
        lblIcon     ->setPixmap(Icons::pxAnnuler().scaled(80,80)); //WARNING : icon scaled : px 80,80
        break;
    case Print:
        lblIcon     ->setPixmap(Icons::pxImprimer().scaled(80,80)); //WARNING : icon scaled : pxImprimer 80,80
        break;
    default:
        break;
    }
    lblIcon     ->setFixedSize(80,80);
    infolayout  ->insertWidget(0,lblIcon);
}

void UpMessageBox::setIconPixmap(QPixmap pix)
{
    lblIcon     ->setPixmap(pix);
    lblIcon     ->setFixedSize(pix.width(),pix.height());
    infolayout  ->insertWidget(0,lblIcon);
}

void UpMessageBox::UpMessageBox::setText(QString Text)
{
    if (Text == "")
        return;
    Textedt         ->setStyleSheet("border: 0px solid; background-color: rgb(200,200,200,0)");
    Textedt         ->setText("<b>" + Text + "</b>");
    Textedt         ->setWordWrap(true);
    Textedt         ->setFixedSize(CalcSize(Text));
    textlayout      ->insertWidget(1,Textedt);
}

void UpMessageBox::setInformativeText(QString Text)
{
    if (Text == "")
        return;
    InfoTextedt     ->setStyleSheet("border: 0px solid; background-color: rgb(200,200,200,0)");
    InfoTextedt     ->setText(Text);
    InfoTextedt     ->setWordWrap(true);
    int position = 1;
    if (dynamic_cast<QLabel*>(textlayout->itemAt(1)->widget()) != NULL)
        position += 1;
    InfoTextedt     ->setFixedSize(CalcSize(Text));
    textlayout      ->insertWidget(position,InfoTextedt);
    textlayout      ->setSizeConstraint(QLayout::SetFixedSize);
}

void UpMessageBox::setDefaultButton(QPushButton *butt)
{
    butt->setDefault(true);
    butt->setFocus();
}

QSize UpMessageBox::CalcSize(QString txt)
{
    QTextEdit txtedit;
    txtedit.setText(txt);
    txt = txtedit.toPlainText();
    QFont fm = qApp->font();
    QStringList lmsg            = txt.split("\n");
    int         w               = 0;
    double      hauteurligne    = QFontMetrics(fm).height()*1.2;
    int         nlignes         = lmsg.size();
    for (int k=0; k<nlignes; k++)
    {
        int x   = QFontMetrics(fm).width(lmsg.at(k))*1.2; //le 1.1 est là pour tenir compte des éventuels caractères gras
        w       = (x>w? x : w);
        //qDebug() << lmsg.at(k) + " - ligne = " + QString::number(k+1) + " - largeur = " + QString::number(w);
    }
    return QSize(w,hauteurligne*nlignes);
}

int UpMessageBox::Watch(QWidget *parent, QString Text, QString InfoText, Buttons Butts)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
//    QMovie movie(":// forbidden.gif");
//    msgbox->lblIcon->setMovie (&movie);
//    msgbox->lblIcon     ->setFixedSize(80,80);
//    msgbox->infolayout  ->insertWidget(0,msgbox->lblIcon);
//    movie.start ();
    msgbox  ->setText(Text);
    msgbox  ->setInformativeText(InfoText.toHtmlEscaped());
    msgbox  ->setIcon(UpMessageBox::Quest);
    msgbox  ->AjouteLayButtons(Butts);
    for (int i=0; i<msgbox->laybuttons->count();i++)
    {
        UpSmallButton *butt =  dynamic_cast<UpSmallButton*>(msgbox->laybuttons->itemAt(i)->widget());
        if (butt!=NULL)
        {
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, [=] {msgbox->Repons(butt);});
            if (butt->ButtonStyle() == UpSmallButton::STARTBUTTON)
                butt->setText("OK");
        }
    }
    dynamic_cast<QVBoxLayout*>(msgbox->layout())->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->laybuttons    ->setSpacing(50);
    msgbox  ->Textedt       ->setFixedSize(msgbox->CalcSize(Text));
    msgbox  ->InfoTextedt   ->setFixedSize(msgbox->CalcSize(InfoText));
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
    for (int i=0; i<msgbox->laybuttons->count();i++)
    {
        UpSmallButton *butt =  dynamic_cast<UpSmallButton*>(msgbox->laybuttons->itemAt(i)->widget());
        if (butt!=NULL)
        {
            if (textlist.size()>k)
                butt->setText(textlist.at(k));
            k++;
            if (butt->ButtonStyle() == UpSmallButton::CANCELBUTTON)
                msgbox->disconnect(butt);
            connect(butt, &QPushButton::clicked, [=] {msgbox->Repons(butt);});
        }
    }
    msgbox  ->Textedt       ->setFixedSize(msgbox->CalcSize(Text));
    msgbox  ->InfoTextedt   ->setFixedSize(msgbox->CalcSize(InfoText));
    dynamic_cast<QVBoxLayout*>(msgbox->layout())->setSizeConstraint(QLayout::SetFixedSize);
    msgbox  ->laybuttons->setSpacing(50);
    if (msgbox  ->exec()>0)
        return msgbox->clickedButton()->ButtonStyle();
    else return UpSmallButton::CANCELBUTTON;
}

void UpMessageBox::Information(QWidget *parent, QString Text, QString InfoText)
{
    UpMessageBox*msgbox     = new UpMessageBox(parent);
    msgbox->setText(Text);
    msgbox->setInformativeText(InfoText);
    msgbox->setIcon(UpMessageBox::Info);

    msgbox      ->AjouteLayButtons(UpDialog::ButtonOK);
    connect (msgbox->OKButton, &QPushButton::clicked, [=] {msgbox->accept();});
    msgbox  ->Textedt       ->setFixedSize(msgbox->CalcSize(Text));
    msgbox  ->InfoTextedt   ->setFixedSize(msgbox->CalcSize(InfoText));
    dynamic_cast<QVBoxLayout*>(msgbox->layout())->setSizeConstraint(QLayout::SetFixedSize);
    msgbox->exec();
}
