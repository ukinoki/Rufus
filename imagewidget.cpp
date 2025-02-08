#include "imagewidget.h"

ImageWidget::ImageWidget(QWidget *parent) : QGraphicsView(parent) {
  setScene(&m_scene);
  m_scene.addItem(&m_graphicsItem);
  setDragMode(QGraphicsView::ScrollHandDrag);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

void ImageWidget::calcScaleFactor(qreal w, qreal h){
  qreal pw = m_pixmap.width();
  qreal ph = m_pixmap.height();
  qreal scaleFactorWidth = w/pw;
  qreal scaleFactorHeight = h/ph;
  m_minScaleFactor = qMin(scaleFactorWidth, scaleFactorHeight);
}


void ImageWidget::fitImage(qreal w, qreal h){
  resetTransform();
  scale(m_minScaleFactor);

  QPointF offset = -QRectF(m_pixmap.rect()).center();
  m_graphicsItem.setOffset(offset);
  m_scene.setSceneRect(-m_pixmap.width() / 2.0, -m_pixmap.height() / 2.0, m_pixmap.width(), m_pixmap.height());
}

void ImageWidget::setPixmap(QPixmap pixmap, qreal w, qreal h) {
  m_pixmap = pixmap;
  m_graphicsItem.resetTransform();
  m_graphicsItem.setPixmap(pixmap);
  m_graphicsItem.setTransformationMode(Qt::SmoothTransformation);
  calcScaleFactor(w, h);
  fitImage(w, h);
}


void ImageWidget::scale(qreal s) {
  QGraphicsView::scale(s, s);
}

void ImageWidget::zoomIn() {
  scale(1.25);
}

void ImageWidget::zoomOut() {
  scale(0.8);
  checkSize();
}

void ImageWidget::checkSize() {
  QTransform trMatrix = QGraphicsView::transform();
  qreal dx = trMatrix.m11();
  if( dx < m_minScaleFactor ) {
    resetTransform();
    scale(m_minScaleFactor);
  }
}

void ImageWidget::wheelEvent(QWheelEvent *event) {
  QPoint scrollAmount = event->angleDelta();
  scrollAmount.y() > 0 ? zoomIn() : zoomOut();
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
  calcScaleFactor(event->size().width(), event->size().height());
  //fitImage(event->size().width(), event->size().height());
  checkSize();
}
