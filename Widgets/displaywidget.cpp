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
    //! avec les pixmaps natifs, c'est la transform de vue qui met l'image à l'échelle : on demande
    //! un lissage pour conserver la qualité au sous-échantillonnage.
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    //setDragMode(QGraphicsView::RubberBandDrag);
    //! Barres de défilement TOUJOURS masquées, dans les deux sens. Raison : une barre qui réserve de
    //! la place (cas non-overlay : Windows, ou macOS sous certaines versions de Qt comme 6.11.1 où les
    //! barres redeviennent non-transientes) rétrécit le viewport quand elle apparaît → resizeEvent →
    //! re-fit → la barre se réévalue… boucle de rétroaction (tremblement, gouttière + liseré blanc,
    //! voire crash). En les masquant, la largeur du viewport est CONSTANTE et la boucle ne peut pas
    //! s'amorcer. Le défilement/pan d'une image rognée reste possible à la molette / au pavé tactile
    //! (qui fonctionnent même barres masquées).
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    //! Garde : la reconstruction vide puis reremplit la scène ; un resizeEvent délivré pendant ce
    //! laps (ex. delete m_mediaPlayer peut traiter des événements) lirait une scène incohérente.
    m_rebuilding = true;
    //! On LÂCHE nos pointeurs AVANT de détruire les items : sinon m_listgraphicsItem pointe un court
    //! instant sur des QGraphicsPixmapItem déjà supprimés par m_scene->clear() (pointeurs pendants).
    m_listgraphicsItem.clear();
    if (m_scene->items().size() >0)
        m_scene->clear();             /*! QGraphicsScene::clear() -> "Removes and deletes all items from the scene..." */
    m_vidItem = nullptr;            //! détruit par m_scene->clear() (item de la scène) : ne pas garder un pointeur mort (lu par resizeEvent)
    if (m_mediaPlayer != nullptr)
    {
        delete m_mediaPlayer;
        m_mediaPlayer = nullptr;
    }
    int h=0;
    foreach (QImage img, newListimg)
    {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
        item->setData(SourceImage, img);
        //! pixmap natif posé une fois pour toutes ; la mise à l'échelle se fait par la transform
        //! de vue (fitImage), jamais en re-rastérisant.
        QPixmap pix = QPixmap::fromImage(img);
        item->setPixmap(pix);
        item->setData(RenderedPixmap, pix);
        item->setPos(0,h);
        m_scene->addItem(item);
        m_listgraphicsItem << item;
        h += pix.height();
    }
    m_rebuilding = false;
    fitImage(size);                 //! transform de fit initiale (avant le premier resizeEvent)
}

void DisplayWidget::setVideo(const QString filename, QSize size)
{
    m_rebuilding = true;
    m_listgraphicsItem.clear();       //! nos pointeurs d'abord (cf. setListimg), puis la scène
    if (m_scene->items().size() >0)
        m_scene->clear();             /*! QGraphicsScene::clear() -> "Removes and deletes all items from the scene..." */
    m_vidItem = nullptr;            //! l'ancien item vidéo vient d'être détruit par clear() : pas de pointeur mort
    if (m_mediaPlayer == nullptr)
        m_mediaPlayer           = new UpMediaPlayer(filename, this);
    else
        m_mediaPlayer           ->init(filename);
    m_vidItem                   = new QGraphicsVideoItem();
    m_scene                     ->addItem(m_vidItem);
    qreal videosizeratio        = sizeRatio(m_mediaPlayer->videosize());
    QSizeF optimalsize          = optimalSizeForVideo(size, videosizeratio);
    m_vidItem                   ->setSize(optimalsize);
    m_rebuilding = false;
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
    if (m_listgraphicsItem.size() == 0 || !m_listgraphicsItem.at(0))
        return;
    //! dimensions natives : largeur = 1re page, hauteur = somme des pages empilées.
    int h = 0;
    foreach(QGraphicsPixmapItem *itm, m_listgraphicsItem)
        if (itm)
            h += itemPixmap(itm).height();
    qreal w = itemPixmap(m_listgraphicsItem.at(0)).width();

    resetTransform();
    //! NB : on ne touche PAS à la politique des barres ici (fitImage est appelé pendant le resize) :
    //! le faire pouvait déclencher une ré-entrée du resizeEvent -> récursion -> crash (vu sous
    //! Qt 6.11.1). Les barres sont réglées une fois pour toutes (masquées) dans le constructeur.
    if (m_listgraphicsItem.size() == 1)
    {
        //! Image unique : "cover". On remplit TOUT le viewport (facteur = le PLUS GRAND des deux,
        //! largeur/hauteur), quitte à rogner l'excédent -> aucune zone blanche, même en étirant la
        //! fiche dans un sens. La scène reste l'image ENTIÈRE : on peut faire défiler à la molette /
        //! au pavé tactile pour voir les bords rognés (sinon il faudrait agrandir la fiche).
        m_ScaleFactor = qMax(qreal(size.width()) / w, qreal(size.height()) / h);
    }
    else
        //! Multi-pages : ajustées à la largeur, défilement vertical (molette) pour parcourir les pages.
        m_ScaleFactor = listImageScaleFactor(size.width());
    scale(m_ScaleFactor);
    m_scene->setSceneRect(0, 0, w, h);
}

void DisplayWidget::fitVideo(QSize size)
{
    if (m_vidItem != nullptr)
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

void DisplayWidget::wheelEvent(QWheelEvent *event) {
    if (OKwheelzoom)
    {
        QPoint scrollAmount = event->angleDelta();
        scrollAmount.y() > 0 ? zoomIn() : zoomOut();
    }
    else
        QGraphicsView::wheelEvent(event);       //! défilement (image rognée à voir en entier, ou multi-pages)
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
    //! Garde anti-récursion : un ajustement peut, sur certaines versions de Qt, ré-émettre un
    //! resizeEvent (barre qui apparaît/disparaît, relayout) -> sans ce garde, récursion -> crash.
    //! m_rebuilding : ne pas mesurer une scène en cours de reconstruction (setListimg/setVideo).
    if (m_inResize || m_rebuilding)
        return;
    m_inResize = true;
    QGraphicsView::resizeEvent(event);          //! laisse la base poser le viewport AVANT de mesurer
    //! On se fie à NOTRE état (images vs vidéo), jamais à items().at(0) : l'ordre d'empilement et le
    //! type de l'item de tête ne sont pas garantis, et un cast raté n'ajustait alors rien.
    if (!m_listgraphicsItem.isEmpty())
    {
        //! On ajuste à la taille du VIEWPORT (zone réellement visible) et NON à celle du widget :
        //! pixmaps natifs, pas de re-rasterisation, juste la transform de vue.
        fitImage(viewport()->size());
        checkSize();
    }
    else if (m_vidItem != nullptr)
        fitVideo(viewport()->size());
    m_inResize = false;
}

bool DisplayWidget::eventFilter(QObject *obj, QEvent *event)
{
    //! type() puis static_cast : sûr et rapide. dynamic_cast sur un QEvent* est fragile (RTTI, réputé
    //! capricieux à travers les frontières de DLL sous Windows) et déréférencer son résultat sans garde
    //! était exposé à un plantage.
    if (event->type() == QEvent::MouseButtonRelease)
        if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
            emit clicked();
    return QWidget::eventFilter(obj, event);
}

