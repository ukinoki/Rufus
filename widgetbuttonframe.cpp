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

#include "widgetbuttonframe.h"

WidgetButtonFrame::WidgetButtonFrame(QWidget *proprio)
{
    gProprio = proprio;
    widgParent = new QWidget(dynamic_cast<QWidget*>(gProprio->parent()));
    setStyleSheet("");
    setStyleSheet("border-style: none;");
    giconModif          .addFile(QStringLiteral("://edit.png"), QSize(24,24), QIcon::Normal, QIcon::Off);
    giconMoins          .addFile(QStringLiteral("://ComMoins.png"), QSize(24,24), QIcon::Normal, QIcon::Off);
    giconPlus           .addFile(QStringLiteral("://ComPlus.png"), QSize(24,24), QIcon::Normal, QIcon::Off);
}

void WidgetButtonFrame::AddButtons(Buttons Butt)
{
    plusBouton  = new UpSmallButton("", this);
    moinsBouton = new UpSmallButton("", this);
    modifBouton = new UpSmallButton("", this);
    plusBouton  ->setIcon(giconPlus);
    modifBouton ->setIcon(giconModif);
    moinsBouton ->setIcon(giconMoins);
    plusBouton  ->setId(1);
    modifBouton ->setId(0);
    moinsBouton ->setId(-1);
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
        connect(butt, &QPushButton::clicked, [=] {Reponse(butt->getId());});
    }
    QHBoxLayout *ilay = new QHBoxLayout();
    ilay->setContentsMargins(0,0,0,0);
    ilay->setSpacing(0);
    setLayout(ilay);
    glayButtonWidg  = new QHBoxLayout();
    glayButtonWidg  ->setContentsMargins(0,0,0,0);
    glayButtonWidg  ->setSpacing(0);
    glayButtonWidg  ->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    glayButtonWidg  ->addWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout();
    vlay        ->setContentsMargins(0,0,0,0);
    vlay        ->setSpacing(0);
    vlay        ->addLayout(glayButtonWidg);
    widgParent  ->resize(gProprio->width(),gProprio->height()+height());
    widgParent  ->setFixedWidth(gProprio->width());
    gProprio    ->setParent(widgParent);
    setParent(widgParent);
    vlay        ->insertWidget(0, gProprio);
    widgParent  ->setLayout(vlay);

    int a = 0;
    if (Butt.testFlag(WidgetButtonFrame::PlusButton))
    {
        plusBouton->setVisible(true);
        ilay->addWidget(plusBouton);
        a++;
    }
    if (Butt.testFlag(WidgetButtonFrame::ModifButton))
    {
        modifBouton->setVisible(true);
        ilay->addWidget(modifBouton);
        a++;
    }
    if (Butt.testFlag(WidgetButtonFrame::MoinsButton))
    {
        moinsBouton->setVisible(true);
        ilay->addWidget(moinsBouton);
        a++;
    }
    setFixedSize(larg*a, larg);
}

void WidgetButtonFrame::replace()
{
    move(gProprio->x()+gProprio->width()-width(), gProprio->y()+gProprio->height()-1);
}

void WidgetButtonFrame::Reponse(int id)
{
    gReponse = id;
    emit choix(gReponse);
}

int WidgetButtonFrame::Reponse()
{
    return gReponse;
}

QWidget* WidgetButtonFrame::widgButtonParent()
{
    return widgParent;
}

QHBoxLayout* WidgetButtonFrame::layButtons()
{
    return glayButtonWidg;
}
