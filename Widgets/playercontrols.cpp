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
    , wdg_playButton(Q_NULLPTR)
    , wdg_stopButton(Q_NULLPTR)
{
    wdg_playButton = new QToolButton(this);
    wdg_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    wdg_stopButton = new QToolButton(this);
    wdg_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    wdg_stopButton->setEnabled(false);

    wdg_slider      = new QSlider(Qt::Horizontal,this);
    wdg_slider      ->setFixedWidth(250);
    wdg_slider      ->setEnabled(true);
    wdg_slider      ->setRange(0, Utils::MaxInt());
    wdg_labelDuration    = new QLabel(this);
    wdg_labelDuration   ->setAlignment(Qt::AlignRight);

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(wdg_stopButton);
    layout->addWidget(wdg_playButton);
    layout->addWidget(wdg_slider);
    layout->addWidget(wdg_labelDuration);
    setLayout(layout);

    m_player = Q_NULLPTR;

    connect(wdg_playButton, &QAbstractButton::clicked, this,    &PlayerControls::playClicked);
    connect(wdg_stopButton, &QAbstractButton::clicked, this,    &PlayerControls::stopClicked);
    setFixedWidth(450);
}

PlayerControls::~PlayerControls()
{
    delete m_player;
}

QString PlayerControls::format(QMediaPlayer *plyr)
{
    return (plyr->duration()/1000 > 3600? "HH:mm:ss" : "mm:ss");
}

void PlayerControls::setPlayer(QMediaPlayer *md)
{
    if (md == Q_NULLPTR)
        return;
    m_player->disconnect();
    wdg_slider->disconnect();
    m_player = md;
    connect(m_player, &QMediaPlayer::positionChanged, this, [=] (qint64 a) { positionChanged(a); });
    connect(wdg_slider, &QSlider::sliderMoved,          this, [=] (int a) { playSeek(a);});
    wdg_labelDuration->setFixedSize(Utils::CalcSize(QTime(0,0,0).toString(format(m_player)) + " / " + QTime(0,0,0).toString(format(m_player))));
}

void PlayerControls::startplay()
{
    emit ctrl(Play);
}

void PlayerControls::playClicked()
{
    wdg_stopButton->setEnabled(true);
    if (m_player->state() == QMediaPlayer::PlayingState)
    {
        wdg_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        emit ctrl(Pause);
    }
    else
    {
        wdg_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        emit ctrl(Play);
    }
}

void PlayerControls::stopClicked()
{
    if (m_player->state() != QMediaPlayer::StoppedState)
    {
        wdg_stopButton->setEnabled(false);
        wdg_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_player->setPosition(0);
        emit ctrl(Stop);
    }
}

void PlayerControls::playSeek(int progress)
{
    double position = (progress < 1? 0 : (double(progress)/Utils::MaxInt())*m_player->duration());
    if (position > m_player->duration())
        position = m_player->duration();
    m_player->setPosition(qint64(position));
}

void PlayerControls::positionChanged(qint64 progress)
{
    if (!wdg_slider->isSliderDown())
    {
        double position = progress*(double(Utils::MaxInt())/m_player->duration());
        if (position > Utils::MaxInt())
            position = Utils::MaxInt();
        wdg_slider->setValue(int(position));
    }
    updateDurationInfo(progress);
    if (progress == m_player->duration())
        stopClicked();
}

void PlayerControls::updateDurationInfo(qint64 progress)
{
    qint64 duration = m_player->duration();
    QString tStr;
    if (progress>-1) {
        progress = progress/1000;
        duration = duration/1000;
        QTime currentTime(  (progress / 3600) % 60,  (progress / 60) % 60,    progress % 60);
        QTime totalTime(    (duration / 3600) % 60,  (duration / 60) % 60,    duration % 60);
        tStr = currentTime.toString(format(m_player)) + " / " + totalTime.toString(format(m_player));
    }
    wdg_labelDuration->setText(tStr);
}
