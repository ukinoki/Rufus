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

#include "upsmallbutton.h"
#include "icons.h"
#include "styles.h"


UpSmallButton::UpSmallButton(QWidget *parent) : QPushButton(parent)
{
    setAutoDefault(true);
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFixedHeight(35);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);

    StyleBouton         = NOBUTTON;
    gLuggage            = -1;
    gToolTipMsg         = "";
    setStyleSheet(STYLE_UPSMALLBUTTON);
}
UpSmallButton::UpSmallButton(QString text, QWidget *parent) : UpSmallButton(parent)
{
    setText(text);
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
    case BACKBUTTON:            setIcon( Icons::icBack() );         break;
    case CALENDARBUTTON:        setIcon( Icons::icDate() );         break;
    case CANCELBUTTON:          setIcon( Icons::icAnnuler() );      break;
    case CLOSEBUTTON:           setIcon( Icons::icFerme() );        break;
    case COPYBUTTON:            setIcon( Icons::icRecopier() );     break;
    case DONOTRECORDBUTTON:     setIcon( Icons::icDoNotRecord() );  break;
    case EDITBUTTON:            setIcon( Icons::icEditer() );       break;
    case HOMEBUTTON:            setIcon( Icons::icPageHome() );     break;
    case LOUPEBUTTON:           setIcon( Icons::icLoupe() );        break;
    case OUPSBUTTON:            setIcon( Icons::icOups() );         break;
    case PRINTBUTTON:           setIcon( Icons::icImprimer() );     break;
    case QWANTBUTTON:           setIcon( Icons::icQwant() );        break;
    case RECEPTIONBUTTON:       setIcon( Icons::icReception() );    break;
    case RECORDBUTTON:          setIcon( Icons::icSauvegarder() );  break;
    case STARTBUTTON:           setIcon( Icons::icOK() );           break;
    case SUPPRBUTTON:           setIcon( Icons::icPoubelle() );     break;
    default:                                                        break;
    }
    StyleBouton = Style;
    setCursor(Qt::PointingHandCursor);
    if (Style==CANCELBUTTON)
        setIconSize(QSize(25,25));
    else if (Style==STARTBUTTON)
        setIconSize(QSize(35,35));
    else
        setIconSize(QSize(30,30));
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
    setStyleSheet(STYLE_UPSMALLBUTTON);
}

void UpSmallButton::setData(QVariant var)
{
    gLuggage = var;
}

QVariant UpSmallButton::Data()
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
            Button->setIcon( Icons::icOKpushed() );
        }
        if (ButtonStyle() == CANCELBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon( Icons::icAnnulerAppuye() );
        }
        if (ButtonStyle() == CLOSEBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon( Icons::icFermeAppuye() );
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
                Button->setIcon( Icons::icOKpushed() );
            else
                Button->setIcon( Icons::icOK() );
        }
        if (ButtonStyle() == CANCELBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->pos(),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon( Icons::icAnnulerAppuye() );
            else
                Button->setIcon( Icons::icAnnuler() );
        }
        if (ButtonStyle() == CLOSEBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            QRect rect = QRect(Button->pos(),Button->size());
            QPoint pos = mapFromParent(cursor().pos());
            if (rect.contains(pos))
                Button->setIcon( Icons::icFermeAppuye() );
            else
                Button->setIcon( Icons::icFerme() );
        }
    }

    if(event->type() == QEvent::MouseButtonRelease)
    {
        if (ButtonStyle() == STARTBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon( Icons::icOK() );
        }
        if (ButtonStyle() == CANCELBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon( Icons::icAnnuler() );
        }
        if (ButtonStyle() == CLOSEBUTTON)
        {
            QPushButton* Button = static_cast<QPushButton*>(obj);
            Button->setIcon( Icons::icFerme() );
        }
        if (isEnabled())
        {
            if (ButtonStyle() == PRINTBUTTON)
                emit clicked(Data());
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

