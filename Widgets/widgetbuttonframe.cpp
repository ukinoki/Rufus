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

#include "widgetbuttonframe.h"
#include "icons.h"

WidgetButtonFrame::WidgetButtonFrame(QAbstractItemView *proprio)
{
    wdg_proprio = proprio;
    widg_parent = new QWidget(dynamic_cast<QWidget*>(wdg_proprio->parent()));
    setStyleSheet("");
    setStyleSheet("border-style: none;");
}

void WidgetButtonFrame::AddButtons(Buttons Butt)
{
    wdg_plusBouton  = new UpSmallButton("", this);
    wdg_moinsBouton = new UpSmallButton("", this);
    wdg_modifBouton = new UpSmallButton("", this);
    wdg_plusBouton  ->setIcon(Icons::icAjouter());
    wdg_plusBouton  ->setIconSize(QSize(24,24));
    wdg_modifBouton ->setIcon(Icons::icEditer());
    wdg_modifBouton  ->setIconSize(QSize(24,24));
    wdg_moinsBouton ->setIcon(Icons::icRetirer());
    wdg_moinsBouton  ->setIconSize(QSize(24,24));
    wdg_plusBouton  ->setiD(1);
    wdg_modifBouton ->setiD(0);
    wdg_moinsBouton ->setiD(-1);
    int szicon  = 20;
    int geo     = szicon + 4;
    int larg    = geo + 7;
    for (int i=0; i<findChildren<UpSmallButton*>().size(); i++)
    {
        UpSmallButton *butt = findChildren<UpSmallButton*>().at(i);
        butt->setFlat(true);
        butt->setVisible(false);
        butt->setIconSize(QSize(szicon, szicon));
        butt->setFixedSize(geo,geo);
        connect(butt, &QPushButton::clicked, [=] {Choix(butt->iD());});
    }
    QHBoxLayout *ilay = new QHBoxLayout();
    ilay->setContentsMargins(0,0,0,0);
    ilay->setSpacing(0);
    setLayout(ilay);
    wdg_buttonwidglayout  = new QHBoxLayout();
    wdg_buttonwidglayout  ->setContentsMargins(0,0,0,0);
    wdg_buttonwidglayout  ->setSpacing(0);
    wdg_buttonwidglayout  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    wdg_buttonwidglayout  ->addWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout();
    vlay        ->setContentsMargins(0,0,0,0);
    vlay        ->setSpacing(0);
    vlay        ->addLayout(wdg_buttonwidglayout);
    widg_parent  ->resize(wdg_proprio->width(),wdg_proprio->height()+height());
    widg_parent  ->setFixedWidth(wdg_proprio->width());
    wdg_proprio  ->setParent(widg_parent);
    setParent(widg_parent);
    vlay        ->insertWidget(0, wdg_proprio);
    widg_parent  ->setLayout(vlay);

    int a = 0;
    if (Butt.testFlag(WidgetButtonFrame::PlusButton))
    {
        wdg_plusBouton->setVisible(true);
        ilay->addWidget(wdg_plusBouton);
        a++;
    }
    if (Butt.testFlag(WidgetButtonFrame::ModifButton))
    {
        wdg_modifBouton->setVisible(true);
        ilay->addWidget(wdg_modifBouton);
        a++;
    }
    if (Butt.testFlag(WidgetButtonFrame::MoinsButton))
    {
        wdg_moinsBouton->setVisible(true);
        ilay->addWidget(wdg_moinsBouton);
        a++;
    }
    setFixedSize(larg*a, larg);
}

void WidgetButtonFrame::replace()
{
    move(wdg_proprio->x()+wdg_proprio->width()-width(), wdg_proprio->y()+wdg_proprio->height()-1);
}

void WidgetButtonFrame::Choix(int id)
{
    switch (id) {
    case 1:     m_reponse = Plus;       break;
    case 0:     m_reponse = Modifier;   break;
    case -1:    m_reponse = Moins;      break;
    }
    emit choix();
}

WidgetButtonFrame::Bouton WidgetButtonFrame::Choix() const
{
    return m_reponse;
}

QWidget* WidgetButtonFrame::widgButtonParent() const
{
    return widg_parent;
}

QHBoxLayout* WidgetButtonFrame::layButtons() const
{
    return wdg_buttonwidglayout;
}
