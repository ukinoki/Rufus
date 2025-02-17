#include "imagewidget.h"

ImageWidget::ImageWidget(QWidget *parent) : QGraphicsView(parent) {
    setScene(&m_scene);
    m_scene.addItem(&m_graphicsItem);
    //setDragMode(QGraphicsView::ScrollHandDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setContextMenuPolicy(Qt::CustomContextMenu);
    viewport()->installEventFilter(this);
}

ImageWidget::ImageWidget(QList<QImage> listimg, QWidget *parent) : QGraphicsView(parent) {
    setScene(&m_scene);
    foreach (QImage img, listimg)
    {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
        QPixmap pix = QPixmap::fromImage(img);
        item->setPixmap(pix);
        m_scene.addItem(item);
    }
    //setDragMode(QGraphicsView::ScrollHandDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setContextMenuPolicy(Qt::CustomContextMenu);
    //viewport()->installEventFilter(this);
}

void ImageWidget::setImage(const QImage &newImage, QSize size)
{
    m_image     = newImage;
    QPixmap pix = QPixmap::fromImage(m_image);
    setPixmap(pix, size.width(), size.height());
    fitImage();
}

void ImageWidget::setOKwheelzoom(bool newOKwheelzoom)
{
    OKwheelzoom = newOKwheelzoom;
}

void ImageWidget::calcScaleFactor(qreal w, qreal h){
    qreal pw = m_pixmap.width();
    qreal ph = m_pixmap.height();
    qreal scaleFactorWidth = w/pw;
    qreal scaleFactorHeight = h/ph;
    m_ScaleFactor = qMax(scaleFactorWidth, scaleFactorHeight);
}

QList<QImage> ImageWidget::listimg() const
{
    return m_listimg;
}

void ImageWidget::setListimg(const QList<QImage> &newListimg, QSize size)
{
    m_scene.clear();
    m_listimg = newListimg;
    int h=0;
    foreach (QImage img, m_listimg)
    {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
        QPixmap pix = QPixmap::fromImage(img).scaledToWidth(size.width(), Qt::SmoothTransformation);
        item->setPixmap(pix);
        item->setPos(0,h);
        m_scene.addItem(item);
        qDebug() << h;
        h += pix.height();
    }
    m_scene.setSceneRect(0,0, size.width(),h);
}

void ImageWidget::fitImage(){
    resetTransform();
    scale(m_ScaleFactor);
    m_scene.setSceneRect(0, 0, m_pixmap.width(), m_pixmap.height());
}

void ImageWidget::setPixmap(QPixmap pixmap, qreal w, qreal h) {
    m_pixmap = pixmap;
    m_graphicsItem.resetTransform();
    m_graphicsItem.setPixmap(pixmap);
    calcScaleFactor(w, h);
}

void ImageWidget::scale(qreal s) {
    QGraphicsView::scale(s, s);
}

void ImageWidget::zoomIn() {
    scale(1.10);
}

void ImageWidget::zoomOut() {
    scale(0.91);
    checkSize();
}

void ImageWidget::checkSize() {
    QTransform trMatrix = QGraphicsView::transform();
    qreal dx = trMatrix.m11();
    if( dx < m_ScaleFactor ) {
        resetTransform();
        scale(m_ScaleFactor);
    }
}

void ImageWidget::wheelEvent(QWheelEvent *event) {
    if (OKwheelzoom)
    {
        QPoint scrollAmount = event->angleDelta();
        scrollAmount.y() > 0 ? zoomIn() : zoomOut();
    }
}


void ImageWidget::keyPressEvent(QKeyEvent *event) {
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

void ImageWidget::resizeEvent(QResizeEvent* event) {
    QPixmap pix     = QPixmap::fromImage(m_image).scaledToWidth(event->size().width(), Qt::SmoothTransformation);
    setPixmap(pix, event->size().width(), event->size().height());
    fitImage();
    checkSize();
}

bool ImageWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            emit clicked();
    return QWidget::eventFilter(obj, event);
}

