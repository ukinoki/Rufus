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

#include "uptablewidget.h"
#include <qdebug.h>

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

QByteArray UpTableWidget::dropData()
{
    return encodedData;
}

void UpTableWidget::clearSelection()
{
    setRangeSelected(QTableWidgetSelectionRange(0,0,rowCount()-1,columnCount()-1),false);
}

void UpTableWidget::FixLargeurTotale()
{
    int larg = 0;
    for (int i=0; i < columnCount(); i++)
        if (!isColumnHidden(i))
            larg += columnWidth(i);
    setFixedWidth(larg+2);
}

int UpTableWidget::rowNoHiddenCount()
{
    int nrow = 0;
    for (int i=0; i<rowCount(); i++)
        if (!isRowHidden(i))
            nrow += 1;
    return nrow;
}

void UpTableWidget::setAttribut(QString attrib)
{
    Attrib = attrib;
}

QString UpTableWidget::Attribut()
{
    return Attrib;
}

