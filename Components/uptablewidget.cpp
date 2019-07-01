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

#include "uptablewidget.h"

UpTableWidget::UpTableWidget(QWidget *parent) : QTableWidget(parent)
{
    setStyleSheet("UpTableWidget {selection-color: rgb(255, 255, 255); selection-background-color: rgb(164, 205, 255); }");
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setContextMenuPolicy(Qt::NoContextMenu);
    Attrib  = "";
}

void UpTableWidget::dropEvent(QDropEvent *drop)
{
    encodedData = drop->mimeData()->data("application/x-qabstractitemmodeldatalist");
//    for (int j=0; j<drop->mimeData()->formats().size(); j++)
//        qDebug() << drop->mimeData()->formats().at(j) + "\n";

//    QDataStream stream(&encodedData, QIODevice::ReadOnly);
//    while (!stream.atEnd() && a)
//    {
//        int                 row, col;
//        QMap<int,QVariant>  roleDataMap;
//        stream >> row >> col >> roleDataMap;
//        QString ab = "col = " + QString::number(col) + " row = " + QString::number(row);
//        for (int i=0;i<roleDataMap.size();i++)
//            ab += " roledatamap[" + QString::number(i) + "]= " + roleDataMap[i].toString();
//        qDebug() << ab;
//    }
    drop->acceptProposedAction();
    emit dropsignal(encodedData);
}

/*!
 * \brief UpTableWidget::AfficheDoc(QMap<QString,QVariant> doc, bool aveczoom)
 * Affichage d'un document pdf ou jpg dans un QTableWidegt
 * argument QMap<QString,QVariant> doc contient 2 éléments
    . ba = le QByteArray contenant les données
    . type = jpg ou pdf
 * renvoie le Qlist<QImage> des images affichées dans la QTableWidget dans leurs tailles d'origine
 */

QList<QImage> UpTableWidget::AfficheDoc(QMap<QString,QVariant> doc, bool aveczoom)
{
    QList<QImage> listimage = QList<QImage>();
    QPixmap     pix;
    QByteArray ba = doc.value("ba").toByteArray();

    clear();
    setColumnCount(1);
    setColumnWidth(0,width()-2);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // sinon on n'a pas de scrollbar vertical vu qu'il n'y a qu'une seule ligne affichée
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QString suffixe;
    if (doc.value("type").toString() == PDF)
    {
        Poppler::Document* document = Poppler::Document::loadFromData(ba);
        if (!document || document->isLocked()) {
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de charger le document"));
            delete document;
            return listimage;
        }
        if (document == Q_NULLPTR) {
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de charger le document"));
            delete document;
            return listimage;
        }

        document->setRenderHint(Poppler::Document::TextAntialiasing);
        int numpages = document->numPages();
        setRowCount(numpages);
        for (int i=0; i<numpages ;i++)
        {
            Poppler::Page* pdfPage = document->page(i);  // Document starts at page 0
            if (pdfPage == Q_NULLPTR) {
                UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
                delete document;
                return listimage;
            }
            QImage image = pdfPage->renderToImage(150,150);
            if (image.isNull()) {
                UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de retrouver les pages du document"));
                delete document;
                return listimage;
            }
            // ... use image ...
            pix = QPixmap::fromImage(image).scaled(width()-2,height()-2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
            listimage << image;
            setRowHeight(i,pix.height());
            UpLabel *lab = new UpLabel(this);
            lab->resize(pix.width(),pix.height());
            lab->setPixmap(pix);
            if (aveczoom)
                connect(lab, &UpLabel::clicked, this, &UpTableWidget::zoom);
            delete pdfPage;
            setCellWidget(i,0,lab);
        }
        delete document;
    }
    else if (doc.value("type").toString() == JPG)
    {
        QImage image;
        if (!image.loadFromData(ba))
            UpMessageBox::Watch(Q_NULLPTR,tr("Impossible de charger le document"));
        pix = QPixmap::fromImage(image).scaled(width()-2,height()-2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
        listimage << image;
        UpLabel* lab     = new UpLabel(this);
        lab->setPixmap(pix);
        if (aveczoom)
            connect(lab, &UpLabel::clicked, this, &UpTableWidget::zoom);
        setRowCount(1);
        setRowHeight(0,pix.height());
        setCellWidget(0,0,lab);
    }
    return listimage;
}

QByteArray UpTableWidget::dropData()
{
    return encodedData;
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

int UpTableWidget::FixLargeurTotale()
{
    int larg = 0;
    for (int i=0; i < columnCount(); i++)
        if (!isColumnHidden(i))
        {
            larg += columnWidth(i);
            //qDebug() << columnWidth(i) << larg;
        }
    setFixedWidth(larg+2);
    return larg+2;
}

int UpTableWidget::rowNoHiddenCount()
{
    int nrow = 0;
    for (int i=0; i<rowCount(); i++)
        if (!isRowHidden(i))
            nrow += 1;
    return nrow;
}

int UpTableWidget::FirstRowNoHidden()
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

int UpTableWidget::LastRowNoHidden()
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

void UpTableWidget::setAttribut(QString attrib)
{
    Attrib = attrib;
}

QString UpTableWidget::Attribut()
{
    return Attrib;
}

