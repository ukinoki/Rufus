/* (C) 2025 LAINE SERGE
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

#ifndef DLG_SINGLEIMAGEVIEWER_H
#define DLG_SINGLEIMAGEVIEWER_H


#include <QAudioOutput>
#include <QTreeView>
#include "playercontrols.h"
#include "imagewidget.h"
#include "upvideowidget.h"


class dlg_singleimageviewer : public UpDialog
{
public:
    dlg_singleimageviewer(QWidget *parent = Q_NULLPTR);
    enum Mode               {Zoom, Normal};                                     Q_ENUM(Mode)

    UpMediaPlayer*          player() const;
    void                    setPlayer(UpMediaPlayer *newPlayer);            //! set sizes to maximum for QDialog & displaywidg depending on display widget ratio & screen resolution ratio

    QMap<QString,QVariant>  mapimg() const;
    void                    setDocument (DocExterne *doc);
    void                    setDepense (Depense *dep);
    void                    setMapimg(const QMap<QString,QVariant> &newMapimg);
    void                    setVideofile (QString filepath);

    dlg_singleimageviewer::Mode     mode() const;
    void                    setMode(Mode newMode);
    void                    changeMode(Mode newMode);

    UpLabel                 *labinfowidget() const;
    QHBoxLayout             *mainlayout() const;

    void                    setCorrectionwidget(QWidget *newCorrectionwidget);

    ListImageWidget*        imagewidget() const;

private:
    bool                    eventFilter(QObject *, QEvent *);
    qreal                   widgetRatio(QList<QImage> listimg);
    void                    DisplayImage(QList<QImage> listimg, QString nomdoc = QString());
    void                    DisplayVideo(QString filepath);

    Mode                    m_mode              = Normal;

    UpLabel*                m_labwdg            = Q_NULLPTR;
    UpLabel*                m_labinfowdg        = new UpLabel();
    QGraphicsScene          *m_scene            = Q_NULLPTR;
    ListImageWidget         *m_imgwdg           = Q_NULLPTR;
    QWidget*                m_parent            = Q_NULLPTR;
    QWidget*                m_correctionwidget  = Q_NULLPTR;
    QHBoxLayout*            m_mainlayout        = new QHBoxLayout();
    PlayerControls*         m_controlplayer     = Q_NULLPTR;

    QMap<QString,QVariant>  m_mapimg            = QMap<QString,QVariant>();
    QString                 m_videofile         = QString();

    bool                    m_resizeable        = true;
    int                     m_spacing           = 0;
    int                     m_marge             = 0;
    QMargins                m_marges            = QMargins(m_marge,m_marge,m_marge,m_marge);
    QSize                   m_normalwidgetsize  = QSize();

    qreal                   m_wdgratio          = 0;

    /*void ListImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);

    QPen pen(Qt::red, 0.8);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect(0,0,size().width()/2, size().height()/10);

    QTextEdit txtedit;
    txtedit.setText(m_text);
    QString header = txtedit.toHtml();
    QTextDocument doc;
    doc.setUseDesignMetrics(true);
    doc.setHtml(header);
    doc.documentLayout()->setPaintDevice(painter.device());
    doc.setPageSize(rect.size());

    QRectF clip(10, 10, rect.width(), rect.height());
    doc.drawContents(&painter, clip);
    painter.restore();
    QGraphicsView::paintEvent(event);
}*/

};

#endif // DLG_SINGLEIMAGEVIEWER_H
