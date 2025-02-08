#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPinchGesture>
#include <QtWidgets>
#include <QMenu>
#include <QAction>
#include <QClipboard>


class ImageWidget : public QGraphicsView {
  Q_OBJECT
  
  QGraphicsScene m_scene;
  QGraphicsPixmapItem m_graphicsItem;
  qreal m_minScaleFactor;
  QPixmap m_pixmap;


public:
  ImageWidget(QWidget * parent = nullptr);
  void fitImage( qreal w, qreal h);
  void calcScaleFactor(qreal w, qreal h);
  void setPixmap(QPixmap pixmap, qreal w, qreal h);
  void scale(qreal s);
  void zoomIn();
  void zoomOut();
  void checkSize();

protected:
  void wheelEvent(QWheelEvent * event) override;
  void keyPressEvent(QKeyEvent * event) override;
  void resizeEvent(QResizeEvent* event) override;
};
