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

#include "playercontrols.h"

PlayerControls::PlayerControls(QWidget *parent) : QWidget(parent)
{
    wdg_playButton  = new QToolButton(this);
    wdg_playButton  ->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    wdg_stopButton  = new QToolButton(this);
    wdg_stopButton  ->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    wdg_stopButton  ->setEnabled(false);

    wdg_slider      = new QSlider(Qt::Horizontal,this);
    wdg_slider      ->setMaximumWidth(250);
    wdg_slider      ->setEnabled(true);
    wdg_slider      ->setRange(0, Utils::MaxInt());
    wdg_slider      ->setStyleSheet("QSlider::sub-page:Horizontal { background-color: #9F2425; }"
                             "QSlider::add-page:Horizontal { background-color: #333333; }"
                             "QSlider::groove:Horizontal { background: transparent; height:0px; }"
                             "QSlider::handle:Horizontal { width:8px; border-radius:4px; background:#9F2425; margin: -5px 0px -5px 0px; }");
    wdg_labelDuration    = new QLabel(this);
    wdg_labelDuration   ->setAlignment(Qt::AlignRight);

    int marge = 0;
    m_layout->setContentsMargins(marge,marge,marge,marge);
    m_layout->setSpacing(5);
    m_layout->addWidget(wdg_stopButton);
    m_layout->addWidget(wdg_playButton);
    m_layout->addWidget(wdg_slider);
    m_layout->addWidget(wdg_labelDuration);
    setLayout(m_layout);

    connect(wdg_playButton,     &QAbstractButton::clicked, this,    &PlayerControls::playClicked);
    connect(wdg_stopButton,     &QAbstractButton::clicked, this,    &PlayerControls::stopClicked);
    setFixedWidth(250);
}

PlayerControls::~PlayerControls()
{
}

QString PlayerControls::format(QMediaPlayer *plyr)
{
    return (plyr->duration()/1000 > 3600? "HH:mm:ss" : "mm:ss");
}

void PlayerControls::setPlayer(QMediaPlayer *md)
{
    if (md == Q_NULLPTR)
        return;
    m_ctrlplayer    = md;
    m_ctrlplayer    ->disconnect();
    wdg_slider  ->disconnect();
    connect(m_ctrlplayer,   &QMediaPlayer::positionChanged,     this, &PlayerControls::positionChanged);
    connect(wdg_slider,     &QSlider::sliderMoved,              this, &PlayerControls::playSeek);
    connect (this,          &PlayerControls::ctrl,              this, [=, this] (PlayerControls::State  state)
            {
                switch (state){
                case PlayerControls::Stop:  m_ctrlplayer->stop();     break;
                case PlayerControls::Pause: m_ctrlplayer->pause();    break;
                case PlayerControls::Play:  m_ctrlplayer->play();
                }
            });
    wdg_labelDuration->setFixedSize(Utils::CalcSize(QTime(0,0,0).toString(format(m_ctrlplayer)) + " / " + QTime(0,0,0).toString(format(m_ctrlplayer))));
    if (m_ctrlplayer->hasAudio())
    {
        QDial           *dial           = new QDial();
        m_layout        ->addWidget(dial);
        QAudioOutput    *audioOutput    = new QAudioOutput();
        connect(dial,   &QDial::valueChanged,   this, [=, this] (int val) {
            float vol = val;
            audioOutput->setVolume(vol/100);
        });
        dial            ->setMinimum(0);
        dial            ->setMaximum(100);
        m_ctrlplayer    ->setAudioOutput(audioOutput);
        dial            ->setValue(50);
        //audioOutput->setVolume(50);
    }
}

void PlayerControls::startplay()
{
    emit ctrl(Play);
    wdg_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    wdg_stopButton->setEnabled(true);
}

void PlayerControls::playClicked()
{
    wdg_stopButton->setEnabled(true);
    if (m_ctrlplayer->playbackState() == QMediaPlayer::PlayingState)
    {
        wdg_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        emit ctrl(Pause);
    }
    else if (m_ctrlplayer->playbackState() == QMediaPlayer::PausedState || m_ctrlplayer->playbackState() == QMediaPlayer::StoppedState)
    {
        wdg_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        emit ctrl(Play);
    }
}

void PlayerControls::stopClicked()
{
    if (m_ctrlplayer->playbackState() != QMediaPlayer::StoppedState)
    {
        wdg_stopButton  ->setEnabled(false);
        wdg_playButton  ->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_ctrlplayer        ->setPosition(0);
        emit ctrl(Stop);
    }
}

void PlayerControls::playSeek(int progress)
{
    double position = (progress < 1?
                           0 :
                           (double(progress)/Utils::MaxInt())*m_ctrlplayer->duration());
    if (position > m_ctrlplayer->duration())
        position = m_ctrlplayer->duration();
    m_ctrlplayer->setPosition(qint64(position));
}

void PlayerControls::setposition(qint64 progress)
{
    double position = (progress < 1?
                           0 :
                           (double(progress)/Utils::MaxInt())*m_ctrlplayer->duration());
    if (position > m_ctrlplayer->duration())
        position = m_ctrlplayer->duration();
    wdg_slider->setValue(int(position));
}

void PlayerControls::positionChanged(qint64 progress)
{
    if (!wdg_slider->isSliderDown())
    {
        double position = progress*(double(Utils::MaxInt())/m_ctrlplayer->duration());
        if (position > Utils::MaxInt())
            position = Utils::MaxInt();
        wdg_slider->setValue(int(position));
    }
    updateDurationInfo(progress);
    if (progress == m_ctrlplayer->duration())
        stopClicked();
}

void PlayerControls::updateDurationInfo(qint64 progress)
{
    qint64 duration = m_ctrlplayer->duration();
    QString tStr;
    if (progress>-1) {
        progress = progress/1000;
        duration = duration/1000;
        QTime currentTime(  (progress / 3600) % 60,  (progress / 60) % 60,    progress % 60);
        QTime totalTime(    (duration / 3600) % 60,  (duration / 60) % 60,    duration % 60);
        tStr = currentTime.toString(format(m_ctrlplayer)) + " / " + totalTime.toString(format(m_ctrlplayer));
    }
    wdg_labelDuration->setText(tStr);
}
