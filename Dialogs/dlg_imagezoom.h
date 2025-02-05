#ifndef DLG_IMAGEZOOM_H
#define DLG_IMAGEZOOM_H

#include <QVideoSink>
#include "playercontrols.h"
#include "upvideowidget.h"

class ImageZoom : public UpDialog
{
public:
    ImageZoom(QWidget * wdg, QWidget *parent = Q_NULLPTR);

    QMediaPlayer *player() const;
    void setPlayer(QMediaPlayer *newPlayer);

private:
    int m_hdelta = 0;
    int m_wdelta = 0;
    PlayerControls *wdg_playctrl    = Q_NULLPTR;
    QMediaPlayer *m_player          = Q_NULLPTR;
};

#endif // DLG_IMAGEZOOM_H
