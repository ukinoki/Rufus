/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include <QMediaPlayer>
#include <QLabel>
#include <QWidget>
#include <QBoxLayout>
#include <QSlider>
#include <QStyle>
#include <QTime>
#include <QToolButton>
#include <QComboBox>
#include <QAudio>


QT_BEGIN_NAMESPACE
class QAbstractButton;
class QAbstractSlider;
QT_END_NAMESPACE

class PlayerControls : public QWidget
{
    Q_OBJECT

public:
    PlayerControls(QMediaPlayer *plyr, QWidget *parent = 0);
    void        setPlayer(QMediaPlayer*);

signals:
    void ctrl(int);
    void recfile();

private slots:
    void playClicked();
    void stopClicked();
    void playSeek(int);
    void recvideo();
    void positionChanged(qint64 progress);
    void updateDurationInfo(qint64 currentInfo);

private:
    QMediaPlayer    *player;
    QAbstractButton *playButton;
    QAbstractButton *stopButton;
    QAbstractButton *recfileButton;
    QSlider         *slider;
    QLabel          *labelDuration;
    qint64          duration;
};

#endif // PLAYERCONTROLS_H
