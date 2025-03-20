/* (C) 2020 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include "procedures.h"
#include "upmediaplayer.h"
#include "cls_item.h"



class DisplayWidget : public QGraphicsView {
    Q_OBJECT

private:
    Item                            *m_rufusitem        = Q_NULLPTR;
    QGraphicsScene                  *m_scene            = new QGraphicsScene;
    QList<QGraphicsPixmapItem*>     m_listgraphicsItem;
    QGraphicsVideoItem              *m_vidItem          = Q_NULLPTR;
    UpMediaPlayer                   *m_mediaPlayer      = Q_NULLPTR;
    qreal                           m_ScaleFactor;
    bool                            OKwheelzoom         = false;
    QString                         m_text              = "";
    void    fitVideo(QSize size);
    qreal   listImageScaleFactor(qreal w);
    qreal   videoScaleFactor(QSize finalsize, QSize originsize);
    void    scale(qreal s);
    void    zoomIn();
    void    zoomOut();
    void    checkSize();

public:
    DisplayWidget(QWidget * parent = nullptr);
    QSizeF          optimalSizeForVideo(QSize availablesize, qreal videoratio);
    void            fitImage(QSize size);
    qreal           sizeRatio(QSize size);
    void            setOKwheelzoom(bool newOKwheelzoom);
    void            setListimg(const QList<QImage> &newListimg, QSize size);
    void            setVideo(const QString filename, QSize size);

    void            setText(const QString &newText);

    UpMediaPlayer*  mediaPlayer() const;
    QSize           setPixmapforItem(QGraphicsPixmapItem *itm, QSize size);              /*! set pixamp for item according to size */
    QPixmap         itemPixmap(QGraphicsPixmapItem *itm)   { return itm->data(1).value<QPixmap>(); }
    QImage          itemImage(QGraphicsPixmapItem *itm)    { return itm->data(0).value<QImage>(); }

    void            setrufusitem(Item *item) { m_rufusitem = item; }
    Item*           rufusitem() const        { return m_rufusitem; }
    bool            hasrufusitem() const     { return m_rufusitem != Q_NULLPTR; }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent * event) override;
    void keyPressEvent(QKeyEvent * event) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    void clicked();
};
