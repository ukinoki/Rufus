#include "imagewidget.h"


ListImageWidget::ListImageWidget(QWidget *parent) : QGraphicsView(parent) {
    setScene(m_scene);
    //setDragMode(QGraphicsView::RubberBandDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setContextMenuPolicy(Qt::CustomContextMenu);
    viewport()->installEventFilter(this);
}

void ListImageWidget::setOKwheelzoom(bool newOKwheelzoom)
{
    OKwheelzoom = newOKwheelzoom;
}

void ListImageWidget::calcScaleFactor(qreal w)
{
    foreach(QGraphicsPixmapItem *itm, m_listgraphicsItem)
    {
        qreal pw = itempPixmap(itm).width();
        qreal scaleFactorWidth = w/pw;
        m_ScaleFactor = qMax(scaleFactorWidth, m_ScaleFactor);
    }
}

void ListImageWidget::setListimg(const QList<QImage> &newListimg, QSize size)
{
    if (m_scene->items().size() >0)
        m_scene->clear();             /*! QGraphicsScene::clear() -> "Removes and deletes all items from the scene..." */
    if (m_listgraphicsItem.size() >0)
        m_listgraphicsItem.clear();
    int h=0;
    foreach (QImage img, newListimg)
    {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
        QPixmap pix = QPixmap::fromImage(img).scaledToWidth(size.width(), Qt::SmoothTransformation);
        item->setPixmap(pix);
        item->setPos(0,h);
        item->setData(0, QVariant(img));
        item->setData(1, pix);
        m_scene->addItem(item);
        m_listgraphicsItem << item;
        h += pix.height();
    }
    calcScaleFactor(size.width());
}

void ListImageWidget::fitImage(){
    resetTransform();
    scale(m_ScaleFactor);
    int h = 0;
    foreach(QGraphicsPixmapItem *itm, m_listgraphicsItem)
        h+= itempPixmap(itm).height();
    m_scene->setSceneRect(0, 0, itempPixmap(m_listgraphicsItem.at(0)).width(), h);
}

void ListImageWidget::scale(qreal s) {
    QGraphicsView::scale(s, s);
}

void ListImageWidget::zoomIn() {
    scale(1.10);
}

void ListImageWidget::zoomOut() {
    scale(0.91);
    checkSize();
}

void ListImageWidget::checkSize() {
    QTransform trMatrix = QGraphicsView::transform();
    qreal dx = trMatrix.m11();
    if( dx < m_ScaleFactor ) {
        resetTransform();
        scale(m_ScaleFactor);
    }
}

void ListImageWidget::wheelEvent(QWheelEvent *event) {
    if (OKwheelzoom)
    {
        QPoint scrollAmount = event->angleDelta();
        scrollAmount.y() > 0 ? zoomIn() : zoomOut();
    }
    else
        QGraphicsView::wheelEvent(event);
}


void ListImageWidget::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    }
    if (parentWidget()) {
        QApplication::sendEvent(parentWidget(), event);
    }
}

void ListImageWidget::resizeEvent(QResizeEvent* event) {
    int h = 0;
    for (int i = 0; i < m_listgraphicsItem.size(); i++) {
        QImage img  = m_listgraphicsItem.at(i)->data(0).value<QImage>();
        QPixmap pix = QPixmap::fromImage(img).scaledToWidth(event->size().width(), Qt::SmoothTransformation);
        m_listgraphicsItem.at(i)->resetTransform();
        m_listgraphicsItem.at(i)->setPixmap(pix);
        m_listgraphicsItem.at(i)->setPos(0,h);
        m_listgraphicsItem.at(i)->setData(1,pix);
        h += pix.height();
    }
    calcScaleFactor(event->size().width());
    fitImage();
    checkSize();
}

bool ListImageWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            emit clicked();
    return QWidget::eventFilter(obj, event);
}

