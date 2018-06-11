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

#include "dlg_fontdialog.h"
#include "icons.h"

dlg_fontdialog::dlg_fontdialog(QString nomSettings, QString Position, QWidget *parent) :
    UpDialog(nomSettings, Position, parent)
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setFixedWidth(280);
    setMaximumSize(280,1200);
    QFontDatabase database;
    TreeWidget  = new QTreeWidget(this);
    frame       = new QFrame(this);
    button      = new UpPushButton(frame);
    lbl         = new UpLabel(this);
    lbl         ->setAlignment(Qt::AlignCenter);
    lbl         ->setMinimumHeight(31);
    frame       ->setMinimumHeight(60);
    frame       ->setFrameShape(QFrame::Panel);
    frame       ->setFrameShadow(QFrame::Sunken);
    button      ->setUpButtonStyle(UpPushButton::OKBUTTON, UpPushButton::Large);
    button      ->setText(tr("Chercher avec la\ndate de naissance"));
    button      ->setFixedSize(170,52);
    QHBoxLayout *box = new QHBoxLayout();
    box         ->addWidget(button);
    frame       ->setLayout(box);
    button      ->setIcon(Icons::icBoy());
    TreeWidget  ->setColumnCount(1);
    TreeWidget  ->setHeaderLabels(QStringList() << "Nom de la police");
    TreeWidget  ->setStyleSheet("QTreeWidget {selection-color: rgb(0,0,0); selection-background-color: rgb(164, 205, 255);}");
    QStringList listmodifs;
    listmodifs << "Italic" << "Light" << "Regular" << "Normal" << "Bold" << "SemiBold" << "Black" << "Bold Italic";

    foreach (const QString &family, database.families(QFontDatabase::Latin)) {
        QTreeWidgetItem *familyItem = new QTreeWidgetItem(TreeWidget);
        familyItem->setText(0, family);

        foreach (const QString &style, database.styles(family)) {
            if (listmodifs.contains(style,Qt::CaseInsensitive))
            {
                QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
                styleItem->setText(0, style);
                //qDebug() << family + " - " + style;
            }
        }
        //qDebug() << family;
    }
    AjouteLayButtons();

    connect (TreeWidget,            SIGNAL(itemClicked(QTreeWidgetItem*,int)),                      this,   SLOT(Slot_Redessinelabel(QTreeWidgetItem*)));
    connect (TreeWidget,            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),  this,   SLOT(Slot_Redessinelabel(QTreeWidgetItem*)));
    connect (OKButton,              SIGNAL(clicked(bool)),                                          this,   SLOT(Slot_FermeFiche()));

    QList<QTreeWidgetItem*> listitems = TreeWidget->findItems(qApp->font().family(),Qt::MatchExactly,0);
    if (listitems.size()>0)
    {
        QTreeWidgetItem *item = listitems.at(0);
        if (item->childCount() == 0)
            TreeWidget->setCurrentItem(item);
        QString FontAttribut = database.styleString(qApp->font());
        bool itemtrouve = false;
        for (int i=0;i<item->childCount();i++)
        {
            if (item->child(i)->text(0) == FontAttribut)
            {
                TreeWidget->setCurrentItem(item->child(i));
                itemtrouve = true;
                break;
            }
        }
        if (!itemtrouve)
            TreeWidget->setCurrentItem(item->child(0));
    }
    TreeWidget->expandAll();
    QVBoxLayout *globallay  = dynamic_cast<QVBoxLayout*>(layout());
    globallay->insertWidget(0,frame);
    globallay->insertWidget(0,lbl);
    globallay->insertWidget(0,TreeWidget);
    globallay->setSpacing(5);
}

dlg_fontdialog::~dlg_fontdialog()
{
}

void dlg_fontdialog::SlotChangeItem(QTreeWidgetItem *arrivee)
{
    if (arrivee!=NULL)
    Slot_Redessinelabel(arrivee);
}

void dlg_fontdialog::Slot_FermeFiche()
{
    UpMessageBox::Watch(this,tr("Vous devrez relancer le programme pour appliquer le changement de police"));
    accept();
}

void dlg_fontdialog::Slot_Redessinelabel(QTreeWidgetItem *item)
{
    QFont fontlabel;
    QString child = item->text(0);
    fontlabel.setStyle(QFont::StyleNormal);
    fontlabel.setWeight(QFont::Normal);
    if (item->parent()!=NULL)
    {
        fontlabel.setFamily(item->parent()->text(0));
        if (!child.contains("Regular",Qt::CaseInsensitive))
        {
            if (child.contains("Italic",Qt::CaseInsensitive))
                fontlabel.setStyle(QFont::StyleItalic);
            if (child.contains("Light",Qt::CaseInsensitive))
                fontlabel.setWeight(QFont::Light);
            if (child.contains("Normal",Qt::CaseInsensitive))
                fontlabel.setWeight(QFont::Normal);
            if (child.contains("SemiBold",Qt::CaseInsensitive))
                fontlabel.setWeight(QFont::DemiBold);
            if (child.contains("Bold",Qt::CaseInsensitive))
                fontlabel.setWeight(QFont::Bold);
            if (child.contains("Black",Qt::CaseInsensitive))
                fontlabel.setWeight(QFont::Black);
        }
        gFontAttribut = child;
        child = item->parent()->text(0) + " " + child;
    }
    else
    {
        fontlabel.setFamily(child);
        gFontAttribut = "";
    }
    for (int i = 5; i < 30; i++)
    {
        fontlabel.setPointSize(i);
        QFontMetrics fm(fontlabel);
        int Htaille = fm.width("date de naissance");
        if (Htaille > 108 || fm.height()*1.1 > 20)
        {
            fontlabel.setPointSize(i-1);
            i=30;
        }
    }
    gFontDialog = fontlabel;
    button->setFont(fontlabel);
    QString a = child + " " + QString::number(fontlabel.pointSize()) + " pt";
    lbl->setText(a);
    lbl->setFont(fontlabel);
}

QFont   dlg_fontdialog::getFont() const
{
    return gFontDialog;
}

void    dlg_fontdialog::setFont(QFont font)
{
    gFontDialog = font;
}

QString   dlg_fontdialog::getFontAttribut() const
{
    return gFontAttribut;
}

void    dlg_fontdialog::setFontAttribut(QString fontAttribut)
{
    gFontAttribut = fontAttribut;
}
