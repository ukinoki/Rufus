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

    stopButton = new QToolButton(this);
    stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    stopButton->setEnabled(false);

    slider      = new QSlider(Qt::Horizontal,this);
    slider      ->setFixedWidth(250);
    slider      ->setEnabled(true);
    slider      ->setRange(0, Utils::MaxInt());
    labelDuration    = new QLabel(this);
    labelDuration   ->setAlignment(Qt::AlignRight);

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(stopButton);
    layout->addWidget(playButton);
    layout->addWidget(slider);
    layout->addWidget(labelDuration);
    setLayout(layout);

    player = Q_NULLPTR;

    connect(playButton, &QAbstractButton::clicked, this,    &PlayerControls::playClicked);
    connect(stopButton, &QAbstractButton::clicked, this,    &PlayerControls::stopClicked);
    setFixedWidth(450);
}

PlayerControls::~PlayerControls()
{
    delete player;
}

QString PlayerControls::format(QMediaPlayer *plyr)
{
    return (plyr->duration()/1000 > 3600? "HH:mm:ss" : "mm:ss");
}

void PlayerControls::setPlayer(QMediaPlayer *md)
{
    if (md == Q_NULLPTR)
        return;
    player->disconnect();
    slider->disconnect();
    player = md;
    connect(player, &QMediaPlayer::positionChanged, this, [=] (qint64 a) { positionChanged(a); });
    connect(slider, &QSlider::sliderMoved,          this, [=] (int a) { playSeek(a);});
    labelDuration->setFixedSize(Utils::CalcSize(QTime(0,0,0).toString(format(player)) + " / " + QTime(0,0,0).toString(format(player))));
}

void PlayerControls::startplay()
{
    emit ctrl(Play);
}

void PlayerControls::playClicked()
{
    stopButton->setEnabled(true);
    if (player->state() == QMediaPlayer::PlayingState)
    {
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        emit ctrl(Pause);
    }
    else
    {
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        emit ctrl(Play);
    }
}

void PlayerControls::stopClicked()
{
    if (player->state() != QMediaPlayer::StoppedState)
    {
        stopButton->setEnabled(false);
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        player->setPosition(0);
        emit ctrl(Stop);
    }
}

void PlayerControls::playSeek(int progress)
{
    double position = (progress < 1? 0 : (double(progress)/Utils::MaxInt())*player->duration());
    if (position > player->duration())
        position = player->duration();
    player->setPosition(qint64(position));
}

void PlayerControls::positionChanged(qint64 progress)
{
    if (!slider->isSliderDown())
    {
        double position = progress*(double(Utils::MaxInt())/player->duration());
        if (position > Utils::MaxInt())
            position = Utils::MaxInt();
        slider->setValue(int(position));
    }
    updateDurationInfo(progress);
    if (progress == player->duration())
        stopClicked();
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
        tStr = currentTime.toString(format(player)) + " / " + totalTime.toString(format(player));
    }
    labelDuration->setText(tStr);
}
