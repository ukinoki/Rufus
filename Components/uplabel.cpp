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

#include "uplabel.h"

UpLabel::UpLabel(QWidget *parent, QString txt) : QLabel(txt, parent)  //je ne sais plus pourquoi j'ai mis le parent avant le texte mais il y une raison à un moment donné du code...
{
    m_id          = -1;
    m_row         = -1;
    m_tooltipmsg = "";
    installEventFilter(this);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFont(qApp->font());
}

UpLabel::UpLabel(Item *item, QString txt, QWidget *parent) : UpLabel(parent, txt)
{
    m_rufusitem = item;
}

UpLabel::UpLabel(int id, QString txt, QWidget *parent) : UpLabel(parent, txt)
{
    m_id = id;
}

UpLabel::~UpLabel()
{

}

void UpLabel::AfficheToolTip()
{
    if (m_tooltipmsg != "" && isEnabled())
        QToolTip::showText(cursor().pos(),m_tooltipmsg);
}

bool UpLabel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        emit enter(id());
        AfficheToolTip();
    }
    else if (event->type() == QEvent::MouseButtonDblClick)
    {
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            emit dblclick(id());
    }
    else if (event->type() == QEvent::MouseButtonRelease)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            emit clicked(id());
   return QWidget::eventFilter(obj, event);
}

int UpLabel::pagepdf() const
{
    return m_pagepdf;
}

void UpLabel::setPagepdf(int newPagepdf)
{
    m_pagepdf = newPagepdf;
}

QImage UpLabel::image() const
{
    return m_image;
}

void UpLabel::setImage(const QImage &newImage)
{
    m_image = newImage;
}

void UpLabel::setiD(int idadef)
{
    m_id = idadef;
}

int UpLabel::id() const
{
    return m_id;
}

void UpLabel::setImmediateToolTip(QString Msg)
{
    m_tooltipmsg = Msg;
}

void UpLabel::setRow(int rowadef)
{
    m_row = rowadef;
}

int UpLabel::Row() const
{
    return m_row;
}
