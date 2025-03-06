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

void ListImageWidget::calcVideoScaleFactor(QSize finalsize, QSize originsize)
{
    qreal finalsizeratio    = sizeRatio(finalsize);
    qreal originsizeratio   = sizeRatio(originsize);
    qreal finalsizeheight   = finalsize.height();
    qreal originsizeheight  = originsize.height();
    qreal finalsizewidth    = finalsize.width();
    qreal originsizewidth   = originsize.width();
    if (finalsizeratio > originsizeratio)
        m_ScaleFactor = finalsizeheight/ originsizeheight;
    else
        m_ScaleFactor = finalsizewidth/ originsizewidth;
}


qreal ListImageWidget::sizeRatio(QSize size)
{
    qreal w = size.width();
    qreal h = size.height();
    return w/h;
}

void ListImageWidget::setListimg(const QList<QImage> &newListimg, QSize size)
{
    if (m_scene->items().size() >0)
        m_scene->clear();             /*! QGraphicsScene::clear() -> "Removes and deletes all items from the scene..." */
    if (m_listgraphicsItem.size() >0)
        m_listgraphicsItem.clear();
    if (m_mediaPlayer != Q_NULLPTR)
    {
        delete m_mediaPlayer;
        m_mediaPlayer = Q_NULLPTR;
    }
    int h=0;
    foreach (QImage img, newListimg)
    {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
        item->setData(0, img);
        QSize sz = setPixmapforItem(item, size);
        item->setPos(0,h);
        m_scene->addItem(item);
        m_listgraphicsItem << item;
        h += sz.height();
    }
}

void ListImageWidget::setVideo(const QString filename, QSize size)
{
    if (m_scene->items().size() >0)
        m_scene->clear();             /*! QGraphicsScene::clear() -> "Removes and deletes all items from the scene..." */
    if (m_listgraphicsItem.size() >0)
        m_listgraphicsItem.clear();
    if (m_mediaPlayer == Q_NULLPTR)
        m_mediaPlayer           = new UpMediaPlayer(filename, this);
    m_vidItem                   = new QGraphicsVideoItem();
    m_scene                     ->addItem(m_vidItem);
    qreal availablesizeratio    = sizeRatio(size);
    QSizeF optimalsize          = optimalSizeForVideo(m_mediaPlayer->videosize(), availablesizeratio);
    m_vidItem                   ->setSize(optimalsize);
    m_mediaPlayer             ->setVideoOutput(m_vidItem );
    m_mediaPlayer             ->play();
    resize(QSize(optimalsize.width(), optimalsize.height()));
    qDebug() << "m_vidItem->size()" << m_vidItem->size() << "optimalsize" << QSize(optimalsize.width(), optimalsize.height());
}

QSizeF ListImageWidget::optimalSizeForVideo(QSize availablesize, qreal videoratio)
{
    /*! available resolution */
    qreal          wscroll  = availablesize.width();
    qreal          hscroll  = availablesize.height();
    qreal          availableratio = wscroll/hscroll;

    /*! if availableratio >= videoratio  => availableratio height is used for video height */
    qreal finalh (0), finalw(0);
    if (availableratio >= videoratio)
    {
        finalh = hscroll;
        finalw = finalh * videoratio;
    }
    /*! if availableratio < videoratio  => screenwidth is used for max width */
    else
    {
        finalw = wscroll;
        finalh = finalw / videoratio;
    }
    return QSizeF(finalw, finalh);
}

UpMediaPlayer *ListImageWidget::mediaPlayer() const
{
    return m_mediaPlayer;
}

void ListImageWidget::fitImage(QSize size){
    foreach(QGraphicsPixmapItem *itm, m_listgraphicsItem)
    {
        qreal pw = itempPixmap(itm).width();
        qreal scaleFactorWidth = size.width()/pw;
        m_ScaleFactor = qMax(scaleFactorWidth, m_ScaleFactor);
    }
    qDebug() << "size" << size << "m_ScaleFactor" << m_ScaleFactor;
    resetTransform();
    scale(m_ScaleFactor);
    int h = 0;
    foreach(QGraphicsPixmapItem *itm, m_listgraphicsItem)
        h+= itempPixmap(itm).height();
    m_scene->setSceneRect(0, 0, itempPixmap(m_listgraphicsItem.at(0)).width(), h);
}

void ListImageWidget::fitVideo(QSize size, QGraphicsVideoItem *itm){
    calcVideoScaleFactor(size, QSize(itm->size().width(), itm->size().height()));
    qDebug() << "size" << size << "m_ScaleFactor" << m_ScaleFactor;
    resetTransform();
    scale(m_ScaleFactor);
    m_scene->setSceneRect(0, 0, m_vidItem->size().width(), m_vidItem->size().height());
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

QSize ListImageWidget::setPixmapforItem(QGraphicsPixmapItem *itm, QSize size)
{
    QSize szpix = QSize();
    if(itm->data(0).value<QImage>() != QImage())
    {
        QPixmap pix = QPixmap::fromImage(itm->data(0).value<QImage>()).scaledToWidth(size.width(), Qt::SmoothTransformation);
        itm->resetTransform();
        itm->setPixmap(pix);
        itm->setData(1,pix);
        szpix = pix.size();
    }
    return szpix;
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
    QGraphicsPixmapItem *itm = dynamic_cast<QGraphicsPixmapItem *>(m_scene->items().at(0));
    if (itm)
    {
        int h = 0;
        for (int i = 0; i < m_listgraphicsItem.size(); i++) {
            QSize sz = setPixmapforItem(m_listgraphicsItem.at(i),event->size());
            m_listgraphicsItem.at(i)->setPos(0,h);
            h += sz.height();
        }
        fitImage(event->size());
        checkSize();
    }
    else
    {
        QGraphicsVideoItem *itm = dynamic_cast<QGraphicsVideoItem *>(m_scene->items().at(0));
        if (itm)
            fitVideo(event->size(),itm);
    }
}

bool ListImageWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            emit clicked();
    return QWidget::eventFilter(obj, event);
}
