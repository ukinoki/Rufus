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

#include "displaywidget.h"
#include <QPainter>


DisplayWidget::DisplayWidget(QWidget *parent) : QGraphicsView(parent) {
    setScene(m_scene);
    //! avec les pixmaps natifs (m_fitByTransform), c'est la transform de vue qui met l'image à
    //! l'échelle : on demande un lissage pour conserver la qualité au sous-échantillonnage.
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    //setDragMode(QGraphicsView::RubberBandDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // ScrollBarAlwaysOn (au lieu de AsNeeded) : avec AsNeeded, la barre verticale apparaît/disparaît
    // selon la hauteur du contenu ; sous Windows (barre de ~17 px, NON overlay contrairement à macOS)
    // chaque apparition CHANGE la largeur du viewport → re-resize → re-rasterisation → la barre se
    // re-évalue… boucle de rétroaction (image qui tremble, gel ~30 s, crash). En la gardant TOUJOURS
    // affichée, la largeur du viewport est CONSTANTE → la boucle ne peut plus s'amorcer.
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setContextMenuPolicy(Qt::CustomContextMenu);
    viewport()->installEventFilter(this);
 }

void DisplayWidget::setOKwheelzoom(bool newOKwheelzoom)
{
    OKwheelzoom = newOKwheelzoom;
}

qreal DisplayWidget::listImageScaleFactor(qreal w)
{
    qreal scale = 0;
    foreach(QGraphicsPixmapItem *itm, m_listgraphicsItem)
    {
        qreal pw = itemPixmap(itm).width();
        qreal scaleFactorWidth = w/pw;
        scale = qMax(scaleFactorWidth, scale);
    }
    return scale;
}

qreal DisplayWidget::videoScaleFactor(QSize finalsize, QSize originsize)
{
    qreal finalsizeratio    = sizeRatio(finalsize);
    qreal originsizeratio   = sizeRatio(originsize);
    qreal finalsizeheight   = finalsize.height();
    qreal originsizeheight  = originsize.height();
    qreal finalsizewidth    = finalsize.width();
    qreal originsizewidth   = originsize.width();
    if (finalsizeratio > originsizeratio)
        return finalsizewidth/ originsizewidth;
    else
        return finalsizeheight/ originsizeheight;
}


qreal DisplayWidget::sizeRatio(QSize size)
{
    qreal w = size.width();
    qreal h = size.height();
    return w/h;
}

void DisplayWidget::setListimg(const QList<QImage> &newListimg, QSize size)
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
        item->setData(SourceImage, img);
        QSize sz;
        if (m_fitByTransform)
        {
            //! pixmap natif posé une fois pour toutes ; la mise à l'échelle se fera par la
            //! transform de vue (fitImage), jamais en re-rastérisant.
            QPixmap pix = QPixmap::fromImage(img);
            item->setPixmap(pix);
            item->setData(RenderedPixmap, pix);
            sz = pix.size();
        }
        else
            sz = setPixmapforItem(item, size);
        item->setPos(0,h);
        m_scene->addItem(item);
        //QGraphicsRectItem *rect = m_scene->addRect(QRectF(10, h + 10, 10, 10));

        m_listgraphicsItem << item;
        h += sz.height();
    }
    if (m_fitByTransform)
        fitImage(size);             //! transform de fit initiale (avant le premier resizeEvent)
}

void DisplayWidget::setVideo(const QString filename, QSize size)
{
    if (m_scene->items().size() >0)
        m_scene->clear();             /*! QGraphicsScene::clear() -> "Removes and deletes all items from the scene..." */
    if (m_listgraphicsItem.size() >0)
        m_listgraphicsItem.clear();
    if (m_mediaPlayer == Q_NULLPTR)
        m_mediaPlayer           = new UpMediaPlayer(filename, this);
    else
        m_mediaPlayer           ->init(filename);
    m_vidItem                   = new QGraphicsVideoItem();
    m_scene                     ->addItem(m_vidItem);
    qreal videosizeratio        = sizeRatio(m_mediaPlayer->videosize());
    QSizeF optimalsize          = optimalSizeForVideo(size, videosizeratio);
    m_vidItem                   ->setSize(optimalsize);
    fitVideo(size);
    m_mediaPlayer               ->setVideoOutput(m_vidItem);
}

void DisplayWidget::setText(const QString &newText)
{
    m_text = newText;
    QFont font = qApp->font();
    font.setBold(true);
    QGraphicsTextItem *text = m_scene->addText(m_text);
    text->setDefaultTextColor(Qt::magenta);
    text->setPos(10, 10);
    text->setVisible(true);
    QPen pen(Qt::red, 0.8);
    pen.setWidth(2);
    QGraphicsLineItem *line = m_scene->addLine(10,10,100,5,pen);
    line->setVisible(true);
}

QSizeF DisplayWidget::optimalSizeForVideo(QSize availablesize, qreal videoratio)
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

UpMediaPlayer *DisplayWidget::mediaPlayer() const
{
    return m_mediaPlayer;
}

void DisplayWidget::fitImage(QSize size)
{
    if (m_listgraphicsItem.size() > 0)
    {
        m_ScaleFactor = listImageScaleFactor(size.width());
        resetTransform();
        scale(m_ScaleFactor);
        int h = 0;
        foreach(QGraphicsPixmapItem *itm, m_listgraphicsItem)
            if (itm)
                h += itemPixmap(itm).height();
        if (m_listgraphicsItem.at(0))
            m_scene->setSceneRect(0, 0, itemPixmap(m_listgraphicsItem.at(0)).width(), h);
    }
}

void DisplayWidget::fitVideo(QSize size)
{
    if (m_vidItem != Q_NULLPTR)
    {
        m_ScaleFactor = videoScaleFactor(size, QSize(m_vidItem->size().width(), m_vidItem->size().height()));
        resetTransform();
        scale(m_ScaleFactor);
        m_scene->setSceneRect(0, 0, m_vidItem->size().width(), m_vidItem->size().height());
    }
}

void DisplayWidget::scale(qreal s) {
    QGraphicsView::scale(s, s);
}

void DisplayWidget::zoomIn() {
    scale(1.10);
}

void DisplayWidget::zoomOut() {
    scale(0.91);
    checkSize();
}

void DisplayWidget::checkSize() {
    QTransform trMatrix = QGraphicsView::transform();
    qreal dx = trMatrix.m11();
    if( dx < m_ScaleFactor ) {
        resetTransform();
        scale(m_ScaleFactor);
    }
}

QSize DisplayWidget::setPixmapforItem(QGraphicsPixmapItem *itm, QSize size)
{
    QSize szpix = QSize();
    if(itm->data(SourceImage).value<QImage>() != QImage())
    {
        QPixmap pix = QPixmap::fromImage(itm->data(SourceImage).value<QImage>()).scaledToWidth(size.width(), Qt::SmoothTransformation);
        itm->resetTransform();
        itm->setPixmap(pix);
        itm->setData(RenderedPixmap,pix);
        szpix = pix.size();
    }
    return szpix;
}

void DisplayWidget::wheelEvent(QWheelEvent *event) {
    if (OKwheelzoom)
    {
        QPoint scrollAmount = event->angleDelta();
        scrollAmount.y() > 0 ? zoomIn() : zoomOut();
    }
    else
        QGraphicsView::wheelEvent(event);
}


void DisplayWidget::keyPressEvent(QKeyEvent *event) {
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

void DisplayWidget::resizeEvent(QResizeEvent* event) {
    if (m_scene->items().size() >0)
    {
        QGraphicsPixmapItem *itm = dynamic_cast<QGraphicsPixmapItem *>(m_scene->items().at(0));
        if (itm)
        {
            if (!m_fitByTransform)
            {
                //! ancien chemin : re-rastérise chaque pixmap à la largeur du viewport.
                int h = 0;
                for (int i = 0; i < m_listgraphicsItem.size(); i++) {
                    QSize sz = setPixmapforItem(m_listgraphicsItem.at(i),event->size());
                    m_listgraphicsItem.at(i)->setPos(0,h);
                    h += sz.height();
                }
            }
            //! m_fitByTransform : pixmaps natifs déjà posés, on ne re-rastérise plus ; le resize ne
            //! fait que recalculer la transform de vue (idempotent, peu coûteux) -> pas de boucle.
            fitImage(event->size());
            checkSize();
        }
        else
        {
            QGraphicsVideoItem *itm = dynamic_cast<QGraphicsVideoItem *>(m_scene->items().at(0));
            if (itm)
                fitVideo(event->size());
        }
    }
}

bool DisplayWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        if (dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            emit clicked();
    return QWidget::eventFilter(obj, event);
}

