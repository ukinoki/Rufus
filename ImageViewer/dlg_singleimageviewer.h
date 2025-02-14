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

#include "database.h"
#include "playercontrols.h"
#include <QAudioOutput>
#include <QTreeView>
#include "uptablewidget.h"
#include "upvideowidget.h"
#include "cls_docexterne.h"
#include "cls_depense.h"

class dlg_singleimageviewer : public UpDialog
{
public:
    dlg_singleimageviewer(QWidget *parent = Q_NULLPTR, QString positionfiche = "");
    enum Mode               {Zoom, Normal};                                     Q_ENUM(Mode)
    enum TypeDoc            {Image, Video};                                     Q_ENUM(TypeDoc)

    UpMediaPlayer*          player() const;
    void                    setPlayer(UpMediaPlayer *newPlayer);            //! set sizes to maximum for QDialog & displaywidg depending on display widget ratio & screen resolution ratio

    QMap<QString,QVariant>  mapimg() const;
    void                    setDocument (DocExterne *doc);
    void                    setDepense (Depense *dep);
    void                    setMapimg(const QMap<QString,QVariant> &newMapimg);
    void                    setVideofile (QString filename);

    UpVideoWidget*          videoWidget() const;
    UpTableWidget*          tableWidget() const;
    int                     correctionwidth() {
        if (m_correctionwidget!= Q_NULLPTR)
            return m_correctionwidget->width();
        return 0;
    }

    dlg_singleimageviewer::Mode     mode() const;
    void                    setMode(Mode newMode);
    void                    changeMode(Mode newMode);
    QSize                   normalWidgetsize()  {   int wdelta  = originalgeometry().size().width()
                                                                - sizefordisplay()  .width();
                                                    int hdelta  = originalgeometry().size().height()
                                                                - sizefordisplay()  .height();
                                                    if (m_correctionwidget!= Q_NULLPTR)
                                                        wdelta += m_correctionwidget->width();
                                                    return QSize(originalgeometry().size().width()-wdelta, originalgeometry().size().height()-hdelta);
                                                }

    QWidget                 *currentwidget() const;
    UpLabel                 *labinfowidget() const;
    QHBoxLayout             *mainlayout() const;
    QList<UpLabel *>        labels() const { return m_labels; }

    void                    setCorrectionwidget(QWidget *newCorrectionwidget);

private:
    bool                    eventFilter(QObject *, QEvent *);
    void                    calcWidgetRatio(QList<QImage> listimg);
    void                    DisplayImage(QList<QImage> listimg, QString nomdoc = QString());
    void                    DisplayVideo(QString filepath);
    void                    InitDisplay(TypeDoc typ);
    Mode                    m_mode;

    UpVideoWidget*          m_vdwdg             = Q_NULLPTR;
    UpLabel*                m_labwdg            = Q_NULLPTR;
    UpTableWidget*          m_tblwdg            = Q_NULLPTR;
    UpLabel*                m_labinfowdg        = new UpLabel();
    QWidget*                m_parent            = Q_NULLPTR;
    QWidget*                m_currentwidget     = Q_NULLPTR;
    QWidget*                m_correctionwidget  = Q_NULLPTR;
    QHBoxLayout*            m_mainlayout        = new QHBoxLayout();
    PlayerControls*         m_controlplayer     = Q_NULLPTR;

    QMap<QString,QVariant>  m_mapimg            = QMap<QString,QVariant>();
    QList<UpLabel*>         m_labels            = QList<UpLabel*>();
    QString                 m_videofile         = QString();

    bool                    m_resizeable        = true;
    QSize                   m_sizewidget        = QSize();
    QSize                   m_sizeform          = QSize();
    int                     m_spacing           = 0;
    int                     m_marge             = 0;
    QMargins                m_marges            = QMargins(m_marge,m_marge,m_marge,m_marge);
    QSize                   m_normalwidgetsize  = QSize();

    double                  m_wdgratio          = 0;

};

#endif // DLG_SINGLEIMAGEVIEWER_H
