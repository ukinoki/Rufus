#include "upheaderview.h"

UpHeaderView::UpHeaderView(QHeaderView *header) : QHeaderView(Qt::Horizontal, header), mainHeader(header)
{
    connect(static_cast<QTableWidget*>(mainHeader->parentWidget())->horizontalScrollBar(), &QScrollBar::valueChanged, this, [=] {updateOffset();});
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
