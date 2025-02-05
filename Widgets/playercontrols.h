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

#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <QMediaPlayer>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include "utils.h"

class PlayerControls : public QWidget
{
    Q_OBJECT

public:
    PlayerControls(QWidget *parent = Q_NULLPTR);
    ~PlayerControls();
    void        setPlayer(QMediaPlayer*);
    void        startplay();
    void        setposition(int pos);
    enum State {Pause, Stop, Play}; Q_ENUM(State)
    void        setposition(qint64 progres);

signals:
    void ctrl(PlayerControls::State);

private:
    QMediaPlayer    *m_player           = Q_NULLPTR;
    QAbstractButton *wdg_playButton     = Q_NULLPTR;
    QAbstractButton *wdg_stopButton     = Q_NULLPTR;
    QSlider         *wdg_slider         = Q_NULLPTR;
    QLabel          *wdg_labelDuration  = Q_NULLPTR;
    QString format(QMediaPlayer *plyr);
    void    playClicked();
    void    playSeek(int);
    void    positionChanged(qint64 progress);
    void    stopClicked();
    void    updateDurationInfo(qint64 progress);
};

#endif // PLAYERCONTROLS_H
