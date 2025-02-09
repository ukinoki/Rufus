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
    dlg_imagezoom(QWidget * wdg, QWidget *parent = Q_NULLPTR);
    dlg_imagezoom(DocExterne *doc, int pagepdf = 0, QWidget *parent = Q_NULLPTR);
    dlg_imagezoom(QMap<QString,QVariant> doc, int pagepdf = 0, QWidget *parent = Q_NULLPTR);

    QMediaPlayer            *player() const;
    void                    setPlayer(QMediaPlayer *newPlayer);
    static void             setSizeAvailable(QSize size, UpDialog *dlg, QSize &sizeform, QSize &sizewidget);

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
