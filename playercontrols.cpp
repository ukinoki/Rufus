/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#include "playercontrols.h"

PlayerControls::PlayerControls(QMediaPlayer *plyr, QWidget *parent)
    : QWidget(parent)
    , playButton(0)
    , stopButton(0)
{
    playButton = new QToolButton(this);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    state = 2;

    stopButton = new QToolButton(this);
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton->setEnabled(false);

    recfileButton = new QToolButton(this);
    recfileButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));

    slider      = new QSlider(Qt::Horizontal,this);
    slider      ->setEnabled(true);
    labelDuration    = new QLabel(this);

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(stopButton);
    layout->addWidget(playButton);
    layout->addWidget(slider);
    layout->addWidget(labelDuration);
    layout->addWidget(recfileButton);
    setLayout(layout);

    setPlayer(plyr);
    connect(slider,         SIGNAL(sliderMoved(int)),           this, SLOT(playSeek(int)));
    connect(player,         SIGNAL(positionChanged(qint64)),    this, SLOT(positionChanged(qint64)));
    connect(playButton,     SIGNAL(clicked()),                  this, SLOT(playClicked()));
    connect(stopButton,     SIGNAL(clicked()),                  this, SLOT(stopClicked()));
    connect(recfileButton,  SIGNAL(clicked(bool)),              this, SLOT(recvideo()));
}

void PlayerControls::setPlayer(QMediaPlayer *md)
{
    player = md;
}

void PlayerControls::playClicked()
{
    stopButton->setEnabled(true);
    if (player->state() == QMediaPlayer::PlayingState)
    {
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        state = 1;
    }
    else
    {
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        state = 2;
    }
    emit ctrl();
}

int PlayerControls::State()
{
    return state;
}

void PlayerControls::stopClicked()
{
    if (player->state() != QMediaPlayer::StoppedState)
    {
        stopButton->setEnabled(false);
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        state = 0;
        emit ctrl();
    }
}

void PlayerControls::playSeek(int seconds)
{
    player->setPosition(seconds);
}

void PlayerControls::positionChanged(qint64 progress)
{
    if (progress > player->duration()-10)
    {
        progress = 0;
        player->setPosition(progress);
    }
    if (!slider->isSliderDown())
        slider->setValue(progress);
    updateDurationInfo(progress / 1000);
}

void PlayerControls::recvideo()
{
    emit recfile();
}

void PlayerControls::updateDurationInfo(qint64 currentInfo)
{
    duration = player->duration() / 1000;
    slider->setRange(0, player->duration());
    QString tStr;
    if (currentInfo) {
        QTime currentTime(  (currentInfo / 3600) % 60,  (currentInfo / 60) % 60,    currentInfo % 60,   (currentInfo * 1000) % 1000);
        QTime totalTime(    (duration / 3600) % 60,     (duration / 60) % 60,       duration % 60,      (duration * 1000) % 1000);
        QString format = (duration > 3600? "hh:mm:ss" : "mm:ss");
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    labelDuration->setText(tStr);
}
