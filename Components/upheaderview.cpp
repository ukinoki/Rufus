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

#include "upheaderview.h"

UpHeaderView::UpHeaderView(QHeaderView *header) : QHeaderView(Qt::Horizontal, header), mainHeader(header)
{
    connect(qobject_cast<QTableWidget*>(mainHeader->parentWidget())->horizontalScrollBar(), &QScrollBar::valueChanged, this, &UpHeaderView::updateOffset);
    setGeometry(0, 0, header->width(), header->height());
    updateOffset();
    mainHeader->installEventFilter(this);
}

void UpHeaderView::reDim(int section, int deb, int fin)
{
    resizeSection(section, getSectionSizes(deb, fin));
}

void UpHeaderView::updateOffset()
{
    setOffset(mainHeader->offset());
}

bool UpHeaderView::eventFilter(QObject *o, QEvent *e)
{
    if (o == mainHeader)
    {
        if (e->type() == QEvent::Resize)
        {
            setOffset(mainHeader->offset());
            setGeometry(0, 0, mainHeader->width(), mainHeader->height());
        }
        return false;
    }
    return QHeaderView::eventFilter(o, e);
}

int UpHeaderView::getSectionSizes(int first, int second)
{
    int size = 0;
    for (int a=first;a<=second;++a)
        size += mainHeader->sectionSize(a);
    return size;
}
