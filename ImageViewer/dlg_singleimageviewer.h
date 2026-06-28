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
#include "displaywidget.h"
#include "upvideowidget.h"


class dlg_singleimageviewer : public UpDialog
{
    Q_OBJECT
public:
    dlg_singleimageviewer(QWidget *parent = Q_NULLPTR);
    enum Mode               {Zoom, Normal};                                     Q_ENUM(Mode)

    UpMediaPlayer*          player() const;
    void                    setPlayer(UpMediaPlayer *newPlayer);            //! set sizes to maximum for QDialog & displaywidg depending on display widget ratio & screen resolution ratio

    QMap<QString,QVariant>  mapimg() const;
    void                    setListDocuments (QList<DocExterne *> listdocs, DocExterne *doc = Q_NULLPTR);
    void                    setDepense (Depense *dep);
    void                    DisplayVideo(QString filepath);

    dlg_singleimageviewer::Mode     mode() const;
    void                    setMode(Mode newMode);
    void                    changeMode(Mode newMode);

    UpLabel                 *labinfowidget() const;
    QHBoxLayout             *mainlayout() const;

    void                    setCorrectionwidget(QWidget *newCorrectionwidget);

    DisplayWidget*          imagewidget() const;

    QList<DocExterne *> ListDocs() const;

protected:
    bool                    eventFilter(QObject *, QEvent *) override;

private:
    qreal                   widgetRatio(QList<QImage> listimg);
    QRect                   optimalGeometryForZoom(double ratioimgorigine) const;   //! géométrie du dialogue en mode Zoom (image à sa taille max non tronquée)
    void                    centerForZoom();                                        //! centre la fiche en mode Zoom (sur l'axe qui a de la place libre)
    void                    DisplayImage(QList<QImage> listimg, QString nomdoc = QString());
    void                    goTo(UpToolBar::Choice choice);
    void                    displaycurrentDocument();

    Mode                    m_mode              = Normal;

    QList<DocExterne*>      m_ListDocs          = QList<DocExterne*>();
    DocExterne*             m_currentDoc       = Q_NULLPTR;
    UpLabel*                m_labwdg            = Q_NULLPTR;
    UpLabel*                m_labinfowdg        = new UpLabel();
    DisplayWidget         *m_imgwdg           = Q_NULLPTR;
    QWidget*                m_parent            = Q_NULLPTR;
    QHBoxLayout*            m_mainlayout        = new QHBoxLayout();
    PlayerControls*         m_controlplayer     = Q_NULLPTR;
    UpToolBar*              wdg_toolbar         = Q_NULLPTR;

    QMap<QString,QVariant>  m_mapimg            = QMap<QString,QVariant>();

    int                     m_spacing           = 0;
    int                     m_marge             = 0;
    QMargins                m_marges            = QMargins(m_marge,m_marge,m_marge,m_marge);

    qreal                   m_wdgratio          = 0;
};

#endif // DLG_SINGLEIMAGEVIEWER_H
