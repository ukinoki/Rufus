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

#include "dlg_fontdialog.h"
#include "icons.h"

dlg_fontdialog::dlg_fontdialog(QString nomSettings, QString Position, QWidget *parent) :
    UpDialog(nomSettings, Position, parent)
{
    //setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowModality(Qt::WindowModal);

    setFixedWidth(280);
    setMaximumSize(280,1200);
    QFontDatabase database;
    wdg_treewidget  = new QTreeWidget(this);
    wdg_frame       = new QFrame(this);
    wdg_uppushbutton      = new UpPushButton(wdg_frame);
    wdg_lbl         = new UpLabel(this);
    wdg_lbl         ->setAlignment(Qt::AlignCenter);
    wdg_lbl         ->setMinimumHeight(31);
    wdg_frame       ->setMinimumHeight(60);
    wdg_frame       ->setFrameShape(QFrame::Panel);
    wdg_frame       ->setFrameShadow(QFrame::Sunken);
    wdg_uppushbutton->setUpButtonStyle(UpPushButton::OKBUTTON, UpPushButton::Large);
    wdg_uppushbutton->setText(tr("Chercher avec la\ndate de naissance"));
    wdg_uppushbutton->setFixedSize(170,52);
    QHBoxLayout *box = new QHBoxLayout();
    box             ->addWidget(wdg_uppushbutton);
    wdg_frame       ->setLayout(box);
    wdg_uppushbutton->setIcon(Icons::icBoy());
    wdg_treewidget  ->setColumnCount(1);
    wdg_treewidget  ->setHeaderLabels(QStringList() << "Nom de la police");
    wdg_treewidget  ->setStyleSheet("QTreeWidget {selection-color: rgb(0,0,0); selection-background-color: rgb(164, 205, 255);}");
    QStringList listmodifs;
    listmodifs << "Italic" << "Light" << "Regular" << "Normal" << "Bold" << "SemiBold" << "Black" << "Bold Italic";

    foreach (const QString &family, database.families(QFontDatabase::Latin)) {
        QTreeWidgetItem *familyItem = new QTreeWidgetItem(wdg_treewidget);
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

    connect (wdg_treewidget,    &QTreeWidget::itemClicked,         this,   [=] (QTreeWidgetItem *item) {Redessinelabel(item);});
    connect (wdg_treewidget,    &QTreeWidget::currentItemChanged,  this,   [=] (QTreeWidgetItem *item) {Redessinelabel(item);});
    connect (OKButton,          &QPushButton::clicked,             this,   &dlg_fontdialog::FermeFiche);

    QList<QTreeWidgetItem*> listitems = wdg_treewidget->findItems(qApp->font().family(),Qt::MatchExactly,0);
    if (listitems.size()>0)
    {
        QTreeWidgetItem *item = listitems.at(0);
        if (item->childCount() == 0)
            wdg_treewidget->setCurrentItem(item);
        QString FontAttribut = database.styleString(qApp->font());
        bool itemtrouve = false;
        for (int i=0;i<item->childCount();i++)
        {
            if (item->child(i)->text(0) == FontAttribut)
            {
                wdg_treewidget->setCurrentItem(item->child(i));
                itemtrouve = true;
                break;
            }
        }
        if (!itemtrouve)
            wdg_treewidget->setCurrentItem(item->child(0));
    }
    wdg_treewidget->expandAll();
    dlglayout()->insertWidget(0,wdg_frame);
    dlglayout()->insertWidget(0,wdg_lbl);
    dlglayout()->insertWidget(0,wdg_treewidget);
    dlglayout()->setSpacing(5);
}

dlg_fontdialog::~dlg_fontdialog()
{
}

void dlg_fontdialog::FermeFiche()
{
    UpMessageBox::Watch(this,tr("Vous devrez relancer le programme pour appliquer le changement de police"));
    accept();
}

void dlg_fontdialog::Redessinelabel(QTreeWidgetItem *item)
{
    QFont fontlabel;
    QString child = item->text(0);
    fontlabel.setStyle(QFont::StyleNormal);
    fontlabel.setWeight(QFont::Normal);
    if (item->parent()!=Q_NULLPTR)
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
        m_fontattribut = child;
        child = item->parent()->text(0) + " " + child;
    }
    else
    {
        fontlabel.setFamily(child);
        m_fontattribut = "";
    }
    Utils::CalcFontSize(fontlabel);
    m_font = fontlabel;
    wdg_uppushbutton->setFont(fontlabel);
    QString a = child + " " + QString::number(fontlabel.pointSize()) + " pt";
    wdg_lbl->setText(a);
    wdg_lbl->setFont(fontlabel);
}

QFont dlg_fontdialog::font()
{
    return m_font;
}

void    dlg_fontdialog::setFont(QFont font)
{
    m_font = font;
}

QString   dlg_fontdialog::fontAttribut() const
{
    return m_fontattribut;
}

void    dlg_fontdialog::setFontAttribut(QString fontAttribut)
{
    m_fontattribut = fontAttribut;
}
