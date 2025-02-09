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

#ifndef DLG_IMAGEZOOM_H
#define DLG_IMAGEZOOM_H

#include <QVideoSink>
#include "database.h"
#include "playercontrols.h"
#include "upmediaplayer.h"
#include "uptablewidget.h"
#include "upvideowidget.h"
#include "cls_docexterne.h"
#include "cls_depense.h"

class dlg_imagezoom : public UpDialog
{
public:
    dlg_imagezoom(DocExterne *doc, int pagepdf = 0, QWidget *parent = Q_NULLPTR);
    dlg_imagezoom(QMap<QString,QVariant> doc, int pagepdf = 0, QWidget *parent = Q_NULLPTR);

    UpMediaPlayer*          player() const;
    void                    setPlayer(UpMediaPlayer *newPlayer);            //! set sizes to maximum for QDialog & displaywidg depending on display widget ratio & screen resolution ratio
    static void             setSizes(double ratio, UpDialog *dlg, QSize &sizeform, QSize &sizewidget, int correctionwidth = 0);

private:
    bool                    eventFilter(QObject *, QEvent *);
    void                    Display(QList<QImage> listimg, QString nomdoc = QString(), int pagepdf = 0);
    void                    PlayVideo(QString filepath);

    UpVideoWidget*          m_vdwdg             = Q_NULLPTR;
    UpLabel*                m_labwdg            = Q_NULLPTR;
    UpTableWidget*          m_tblwdg            = Q_NULLPTR;
    UpLabel*                m_labinfowdg        = Q_NULLPTR;
    QWidget*                m_parent            = Q_NULLPTR;
    PlayerControls*         m_controlplayer     = Q_NULLPTR;

    int                     m_pagepdf           = 0;

    bool                    m_resizeable        = true;
    QSize                   m_sizewidget        = QSize();
    QSize                   m_sizeform          = QSize();

    UpMediaPlayer           *m_player           = Q_NULLPTR;
};

#endif // DLG_IMAGEZOOM_H
