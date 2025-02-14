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

#include "uptablewidget.h"

UpTableWidget::UpTableWidget(QWidget *parent) : QTableWidget(parent)
{
    setStyleSheet("UpTableWidget {selection-color: rgb(255, 255, 255); selection-background-color: rgb(164, 205, 255); }");
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFont(qApp->font());
}

UpTableWidget::UpTableWidget(Item *rufusitem, QWidget *parent) : UpTableWidget(parent)
{
    m_rufusitem = rufusitem;
}

void UpTableWidget::dropEvent(QDropEvent *drop)
{
    m_encodedData = drop->mimeData()->data("application/x-qabstractitemmodeldatalist");
    drop->acceptProposedAction();
    emit dropsignal(m_encodedData);
}

/*!
 * \brief UpTableWidget::AfficheDoc(QMap<QString,QVariant> doc, bool aveczoom)
 * Affichage d'un document pdf ou jpg dans un QTableWidegt
 * argument QMap<QString,QVariant> doc contient 2 éléments
    . ba = le QByteArray contenant les données
    . type = jpg ou pdf
 * renvoie le Qlist<QImage> des images affichées dans la QTableWidget dans leurs tailles d'origine
 */

void UpTableWidget::AfficheDoc(QMap<QString,QVariant> doc, bool aveczoom)
{
    QList<QImage> listimg = QList<QImage>();
    QByteArray ba = doc.value(M_BA).toByteArray();

    if (doc.value(M_TYPE).toString() == PDF)
        listimg = Utils::calcImagefromPdf(ba);
    else if (doc.value(M_TYPE).toString() == JPG || doc.value(M_TYPE).toString() == PNG || doc.value(M_TYPE).toString() == JPEG)
    {
        QImage image;
        if (!image.loadFromData(ba))
            UpMessageBox::Watch(this,tr("Impossible de charger le document"));
        listimg << image;
    }
    setListimages(listimg, QSize (width(), height()));
    if (aveczoom)
        for (int i=0; i<labels().size(); i++)
            connect(labels().at(i), &UpLabel::clicked, this, &UpTableWidget::zoom);

}

QByteArray UpTableWidget::dropData()
{
    return m_encodedData;
}

QList<QImage> UpTableWidget::listimg() const
{
    return m_listimg;
}

QSize UpTableWidget::setListimages(const QList<QImage> &newListimg, QSize szavailable)
{
    for (int i=0; i<rowCount(); i++)
    {
        QWidget *lab = cellWidget(i,0);
        if (lab) delete lab;
    }
    clear();
    setColumnCount(1);
    setColumnWidth(0,width());
    setSelectionMode(QAbstractItemView::NoSelection);
    horizontalHeader()  ->setVisible(false);
    verticalHeader()    ->setVisible(false);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listimg = newListimg;
    setRowCount(m_listimg.size());

    //! create labels and adjust size of each label
    m_labels.clear();
    QSize szfinal = QSize();
    int maxw(0), maxh(0);
    int x(0), y(0);
    for (int i=0; i<m_listimg.size();++i)
    {
        QImage image    = m_listimg.at(i);
        QPixmap pix     = QPixmap::fromImage(image).scaledToWidth(szavailable.width(),
                                                       Qt::SmoothTransformation);
        x = pix.width();
        y = pix.height();
        UpLabel *lab            = new UpLabel(m_rufusitem, "", this);
        lab                     ->setPixmap(pix);
        lab                     ->setImage(image);
        lab                     ->setContextMenuPolicy(Qt::CustomContextMenu);
        lab                     ->setPagepdf(i);
        m_labels << lab;
        setRowHeight(i,pix.height());
        setCellWidget(i,0,lab);
        if (x>maxw) maxw = x;
        if (y>maxh) maxh = y;
    }
    if (maxw >0 && maxh >0)
        szfinal = QSize(maxw,maxh);
    return szfinal;
}

QList<UpLabel *> UpTableWidget::labels() const
{
    return m_labels;
}

QSize UpTableWidget::resizetofit(QSize sz)
{
    QSize szfinal = QSize();
    int maxw(0), maxh(0);
    for (int i=0; i <rowCount(); i++)
    {
        UpLabel *lbl = qobject_cast<UpLabel*>(cellWidget(i,0));
        if (lbl != Q_NULLPTR)
        {
            QImage img = listimg().at(i);
            QPixmap pix = QPixmap::fromImage(img).scaledToWidth(sz.width(), Qt::SmoothTransformation);
            lbl->setPixmap(pix);
            int x = pix.width();
            int y = pix.height();
            setRowHeight(i,y);
            setColumnWidth(i,x);
            if (x>maxw) maxw = x;
            if (y>maxh) maxh = y;
        }
    }
    if (maxw >0 && maxh >0)
    {
        szfinal = QSize(maxw,maxh);
        setColumnWidth(0,szfinal.width());
    }
    return szfinal;
}

void UpTableWidget::clearSelection()
{
    setRangeSelected(QTableWidgetSelectionRange(0,0,rowCount()-1,columnCount()-1),false);
}

void UpTableWidget::clearAllRowsExceptHeader()
{
    for (int i =0; i<rowCount(); i++)
        removeRow(i);
}

int UpTableWidget::FixLargeurTotale(int larg)
{
    for (int i=0; i < columnCount(); i++)
        if (!isColumnHidden(i))
        {
            larg += columnWidth(i);
            //qDebug() << i << columnWidth(i) << larg;
        }
//#ifdef Q_OS_WINDOWS
  setFixedWidth(larg + Utils::I()->correctedwidth(20));
//#else
    setFixedWidth(larg+2);
//#endif
    //qDebug() << width();
    return width();
}

int UpTableWidget::rowNoHiddenCount() const
{
    int nrow = 0;
    for (int i=0; i<rowCount(); i++)
        if (!isRowHidden(i))
            nrow += 1;
    return nrow;
}

int UpTableWidget::FirstRowNoHidden() const
{
    int row = -1;
    for (int i=0; i<rowCount(); i++)
        if (!isRowHidden(i))
        {
            row = i;
            break;
        }
    return row;
}

int UpTableWidget::LastRowNoHidden() const
{
    int row = -1;
    for (int i=rowCount()-1; i>-1; i--)
        if (!isRowHidden(i))
        {
            row = i;
            break;
        }
    return row;
}

void UpTableWidget::setAllRowHeight(int h)
{
    for (int i=0; i<rowCount(); i++)
        setRowHeight(i, h);
}

void UpTableWidget::selectRow(int row)
{
    if (selectionBehavior() == QAbstractItemView::SelectRows)
    {
        clearSelection();
        setRangeSelected(QTableWidgetSelectionRange(row,0,row,columnCount()-1),true);
    }
}
