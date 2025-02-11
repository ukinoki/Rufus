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

#include "upvideowidget.h"

UpVideoWidget::UpVideoWidget(QString filename, QWidget *parent) : QVideoWidget(parent)
{
    m_filename  = filename;
    m_player    = new UpMediaPlayer(m_filename, this);
    m_player    ->setVideoOutput(this);
}

QString UpVideoWidget::filename() const
{
    return m_filename;
}

void UpVideoWidget::setFilename(const QString &newFilename)
{
    m_filename = newFilename;
}

UpMediaPlayer *UpVideoWidget::player() const
{
    return m_player;
}

void UpVideoWidget::setPlayer(UpMediaPlayer *newPlayer)
{
    m_player    = newPlayer;
    m_player    ->setParent(this);
}
