#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include "procedures.h"
#include "upmediaplayer.h"



class ListImageWidget : public QGraphicsView {
    Q_OBJECT

private:
    QGraphicsScene                  *m_scene            = new QGraphicsScene;
    QList<QGraphicsPixmapItem*>     m_listgraphicsItem;
    QGraphicsVideoItem              *m_vidItem          = Q_NULLPTR;
    UpMediaPlayer                   *m_mediaPlayer      = Q_NULLPTR;
    qreal                           m_ScaleFactor;
    bool                            OKwheelzoom         = false;
    QString                         m_text              = "";
    void    fitImage(QSize size);
    void    fitVideo(QSize size);
    qreal   listImageScaleFactor(qreal w);
    qreal   videoScaleFactor(QSize finalsize, QSize originsize);
    qreal   sizeRatio(QSize size);
    void    setListPixmap(QList<QPixmap> listpixmap);
    void    scale(qreal s);
    void    zoomIn();
    void    zoomOut();
    void    checkSize();
    QPixmap itempPixmap(QGraphicsPixmapItem *itm)   { return itm->data(1).value<QPixmap>(); }
    QImage  itempImage(QGraphicsPixmapItem *itm)    { return itm->data(0).value<QImage>(); }
    QSize   setPixmapforItem(QGraphicsPixmapItem *itm, QSize size);              /*! set pixamp for item according to size */
    QSizeF  optimalSizeForVideo(QSize availablesize, qreal videoratio);

public:
    ListImageWidget(QWidget * parent = nullptr);
    void setOKwheelzoom(bool newOKwheelzoom);
    void setListimg(const QList<QImage> &newListimg, QSize size);
    void setVideo(const QString filename, QSize size);

    void setText(const QString &newText);

    UpMediaPlayer *mediaPlayer() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    void clicked();
};
