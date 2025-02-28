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
    QList<QPixmap>                  m_listpixmap;
    QList<QImage>                   m_listimg   = QList<QImage>();
    bool                            OKwheelzoom = false;
    void fitImage();
    void calcScaleFactor(qreal w);
    void setListPixmap(QList<QPixmap> listpixmap);
    void scale(qreal s);
    void zoomIn();
    void zoomOut();
    void checkSize();

public:
    ListImageWidget(QWidget * parent = nullptr);
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
