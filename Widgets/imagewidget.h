#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QSplitter>
#include <QMouseEvent>
#include <QWheelEvent>


class ImageWidget : public QGraphicsView {
    Q_OBJECT

    QGraphicsScene m_scene;
    QGraphicsPixmapItem m_graphicsItem;
    qreal m_ScaleFactor;
    QPixmap m_pixmap;

private:
    QList<QImage>       m_listimg   = QList<QImage>();
    QImage              m_image     = QImage();
    bool                OKwheelzoom = false;
    void fitImage();
    void calcScaleFactor(qreal w, qreal h);
    void setPixmap(QPixmap pixmap, qreal w, qreal h);
    void scale(qreal s);
    void zoomIn();
    void zoomOut();
    void checkSize();

public:
    ImageWidget(QWidget * parent = nullptr);
    ImageWidget(QList<QImage> listimg, QWidget * parent = nullptr);
    //ImageWidget(QWidget * parent = nullptr);
    void setImage(const QImage &newImage, QSize size);
    void setOKwheelzoom(bool newOKwheelzoom);

    QList<QImage> listimg() const;
    void setListimg(const QList<QImage> &newListimg, QSize size);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override ;
    void wheelEvent(QWheelEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    void clicked();
};
