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

#include "upsmallbutton.h"

UpSmallButton::UpSmallButton(QString text, QWidget *parent) : QPushButton(parent)
{
    setAutoDefault(true);
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFixedHeight(35);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);
    setText(text);

    StyleBouton = NOBUTTON;
    giconBack           .addFile(QStringLiteral("://back.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconCancel         .addFile(QStringLiteral("://cancel.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconCancelpushed   .addFile(QStringLiteral("://cancelpushed.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconClose          .addFile(QStringLiteral("://shutdown.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconClosepushed    .addFile(QStringLiteral("://shutdowndown.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconCopy           .addFile(QStringLiteral("://copy_v2.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconDoNotRecord    .addFile(QStringLiteral("://donotrecord.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconEdit           .addFile(QStringLiteral("://edit.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOK             .addFile(QStringLiteral("://start.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOKpushed       .addFile(QStringLiteral("://startpushed.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOKpushed       .addFile(QStringLiteral("://startpushed.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOups           .addFile(QStringLiteral("://damn-icon.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconPrint          .addFile(QStringLiteral("://Imprimer.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconReception      .addFile(QStringLiteral("://reception_icon.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconRecord         .addFile(QStringLiteral("://save.png"), QSize(30,30), QIcon::Normal, QIcon::Off);
    giconSuppr          .addFile(QStringLiteral("://trash.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    gLuggage            = -1;
    gToolTipMsg         = "";
    setStyleSheet("UpSmallButton {border-style: none; margin: 3px 3px 3px 3px;}"
        "UpSmallButton:pressed {background-color: rgb(205, 205, 205);}"
        "UpSmallButton:focus {color : #000000; border: 1px solid rgb(164, 205, 255); border-radius: 5px;}");
}

UpSmallButton::UpSmallButton(QWidget *parent) : QPushButton(parent)
{
    setAutoDefault(true);
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFixedHeight(35);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);

    StyleBouton = NOBUTTON;
    giconBack           .addFile(QStringLiteral("://back.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconCancel         .addFile(QStringLiteral("://cancel.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconCancelpushed   .addFile(QStringLiteral("://cancelpushed.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconClose          .addFile(QStringLiteral("://shutdown.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconClosepushed    .addFile(QStringLiteral("://shutdowndown.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconCopy           .addFile(QStringLiteral("://copy_v2.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconDoNotRecord    .addFile(QStringLiteral("://donotrecord.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconEdit           .addFile(QStringLiteral("://edit.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOK             .addFile(QStringLiteral("://start.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOKpushed       .addFile(QStringLiteral("://startpushed.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOKpushed       .addFile(QStringLiteral("://startpushed.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconOups           .addFile(QStringLiteral("://damn-icon.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconPrint          .addFile(QStringLiteral("://Imprimer.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconReception      .addFile(QStringLiteral("://reception_icon.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    giconRecord         .addFile(QStringLiteral("://save.png"), QSize(30,30), QIcon::Normal, QIcon::Off);
    giconSuppr          .addFile(QStringLiteral("://trash.png"), QSize(35,35), QIcon::Normal, QIcon::Off);
    gLuggage            = -1;
    gToolTipMsg         = "";
    setStyleSheet("UpSmallButton {border-style: none; margin: 3px 3px 3px 3px;}"
        "UpSmallButton:pressed {background-color: rgb(205, 205, 205);}"
        "UpSmallButton:focus {color : #000000; border: 1px solid rgb(164, 205, 255); border-radius: 5px;}");
}

UpSmallButton::~UpSmallButton()
{
}

void UpSmallButton::AfficheToolTip()
{
    if (gToolTipMsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),gToolTipMsg);
}

void UpSmallButton::setImmediateToolTip(QString Msg)
{
    gToolTipMsg = Msg;
}

void UpSmallButton::setUpButtonStyle(enum StyleBouton Style)
{
    switch (Style) {
    case BACKBUTTON:            setIcon(giconBack);         break;
    case CANCELBUTTON:          setIcon(giconCancel);       break;
    case CLOSEBUTTON:           setIcon(giconClose);        break;
    case COPYBUTTON:            setIcon(giconCopy);         break;
    case DONOTRECORDBUTTON:     setIcon(giconDoNotRecord);  break;
    case EDITBUTTON:            setIcon(giconEdit);         break;
    case OUPSBUTTON:            setIcon(giconOups);         break;
    case PRINTBUTTON:           setIcon(giconPrint);        break;
    case RECEPTIONBUTTON:       setIcon(giconReception);    break;
    case RECORDBUTTON:          setIcon(giconRecord);       break;
    case STARTBUTTON:           setIcon(giconOK);           break;
    case SUPPRBUTTON:           setIcon(giconSuppr);        break;
    default:                                                break;
    }
    StyleBouton = Style;
    setCursor(Qt::PointingHandCursor);
    setIconSize(QSize(30,30));
    if (Style==CANCELBUTTON)
        setIconSize(QSize(25,25));
    if (Style==STARTBUTTON)
        setIconSize(QSize(35,35));
}

void UpSmallButton::setText(QString txt)
{
    QPushButton::setText(txt);
    if (txt == "")
        return;             /*Cette commande est nécessaire parce qu'apparemment,
                            la commande setText(QString txt) est toujours appelée à la création du bouton
                            même si txt = "", ce qui entraîne alors obligatoirement le changement de style du bouton*/
    setFocusPolicy(Qt::StrongFocus);
    setFixedHeight(45);
    setStyleSheet("UpSmallButton {border: 1px none black; padding-left: 8px; padding-top: 3px; padding-right: 8px; padding-bottom: 3px;}"
                  "UpSmallButton:pressed {background-color: rgb(205, 205, 205);}"
                  "UpSmallButton:focus {color : #000000; border: 1px solid rgb(164, 205, 255); border-radius: 5px;}");
}

void UpSmallButton::setLuggage(QVariant var)
{
    gLuggage = var;
}

QVariant UpSmallButton::Luggage()
{
    return gLuggage;
}

void UpSmallButton::setId(int idbut)
{
    id  = idbut;
}

int UpSmallButton::getId()
{
    return id;
}

int UpSmallButton::ButtonStyle()
{
    return StyleBouton;
}

bool UpSmallButton::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        AfficheToolTip();
        return true;
    }
    if(event->type() == QEvent::MouseButtonPress)
    {
        if (ButtonStyle() == STARTBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(giconOKpushed);
        }
        if (ButtonStyle() == CANCELBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(giconCancelpushed);
        }
        if (ButtonStyle() == CLOSEBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(giconClosepushed);
        }
    }

    if(event->type() == QEvent::MouseMove)
    {
        if (ButtonStyle() == STARTBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->pos(),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon(giconOKpushed);
            else
                Button->setIcon(giconOK);
        }
        if (ButtonStyle() == CANCELBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->pos(),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon(giconCancelpushed);
            else
                Button->setIcon(giconCancel);
        }
        if (ButtonStyle() == CLOSEBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->pos(),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon(giconClosepushed);
            else
                Button->setIcon(giconClose);
        }
    }

    if(event->type() == QEvent::MouseButtonRelease)
    {
        if (ButtonStyle() == STARTBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(giconOK);
        }
        if (ButtonStyle() == CANCELBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(giconCancel);
        }
        if (ButtonStyle() == CLOSEBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon(giconClose);
        }if (isEnabled())
        {
            if (ButtonStyle() == PRINTBUTTON)
                emit clicked(Luggage());
            else
                emit clicked(getId());
        }
    }
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key()==Qt::Key_Return  && keyEvent->modifiers() == Qt::NoModifier) || keyEvent->key() == Qt::Key_Enter)
        {
            emit clicked(true);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

