#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QSplitter>
#include <QMouseEvent>
#include <QWheelEvent>
#include "procedures.h"



class ListImageWidget : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene                  *m_scene         = new QGraphicsScene;
    QList<QGraphicsPixmapItem*>     m_listgraphicsItem;
    qreal                           m_ScaleFactor;
    bool                            OKwheelzoom = false;
    void    fitImage();
    void    calcScaleFactor(qreal w);
    void    setListPixmap(QList<QPixmap> listpixmap);
    void    scale(qreal s);
    void    zoomIn();
    void    zoomOut();
    void    checkSize();
    QPixmap itempPixmap(QGraphicsPixmapItem *itm)   { return itm->data(1).value<QPixmap>(); }
    QImage  itempImage(QGraphicsPixmapItem *itm)    { return itm->data(0).value<QImage>(); }

public:
    ListImageWidget(QWidget * parent = nullptr);
    void setOKwheelzoom(bool newOKwheelzoom);
    void setListimg(const QList<QImage> &newListimg, QSize size);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void wheelEvent(QWheelEvent * event);
    void keyPressEvent(QKeyEvent * event);
    void resizeEvent(QResizeEvent* event);

signals:
    void clicked();
};
