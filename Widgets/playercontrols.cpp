/* (C) 2018 LAINE SERGE
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

#include "playercontrols.h"

PlayerControls::PlayerControls(QWidget *parent)
    : QWidget(parent)
    , playButton(Q_NULLPTR)
    , stopButton(Q_NULLPTR)
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
    slider      ->setFixedWidth(250);
    slider      ->setEnabled(true);
    slider      ->setRange(0, Utils::MaxInt());
    labelDuration    = new QLabel(this);

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(stopButton);
    layout->addWidget(playButton);
    layout->addWidget(recfileButton);
    layout->addWidget(slider);
    layout->addWidget(labelDuration);
    setLayout(layout);

    player = Q_NULLPTR;

    connect(playButton,     SIGNAL(clicked()),                  this, SLOT(playClicked()));
    connect(stopButton,     SIGNAL(clicked()),                  this, SLOT(stopClicked()));
    connect(recfileButton,  SIGNAL(clicked(bool)),              this, SLOT(recvideo()));
}

PlayerControls::~PlayerControls()
{
    delete player;
}

void PlayerControls::setPlayer(QMediaPlayer *md)
{
    if (md == Q_NULLPTR)
        return;
    player->disconnect();
    slider->disconnect();
    player = md;
    connect(player,         SIGNAL(positionChanged(qint64)),    this, SLOT(positionChanged(qint64)));
    connect(slider,         SIGNAL(sliderMoved(int)),           this, SLOT(playSeek(int)));
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

void PlayerControls::playSeek(int progress)
{
    player->setPosition(qint64((double(progress)/Utils::MaxInt())*player->duration()));
}

void PlayerControls::positionChanged(qint64 progress)
{
    if (!slider->isSliderDown())
        slider->setValue(int(progress*(double(Utils::MaxInt())/player->duration())));
    updateDurationInfo(progress);
}

void PlayerControls::recvideo()
{
    emit recfile();
}

void PlayerControls::updateDurationInfo(qint64 progress)
{
    qint64 duration = player->duration();
    QString tStr;
    if (progress>-1) {
        progress = progress/1000;
        duration = duration/1000;
        QTime currentTime(  (progress / 3600) % 60,  (progress / 60) % 60,    progress % 60);
        QTime totalTime(    (duration / 3600) % 60,  (duration / 60) % 60,    duration % 60);
        QString format = (duration > 3600? "hh:mm:ss" : "mm:ss");
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    labelDuration->setText(tStr);
}
