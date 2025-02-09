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

#ifndef UPMEDIAPLAYER_H
#define UPMEDIAPLAYER_H

#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QEventLoop>
#include <QObject>
#include <QSize>

class UpMediaPlayer : public QMediaPlayer
{
public:
    explicit UpMediaPlayer(QString filename, QObject *parent = nullptr);
    QSize videoresolution() const;

private:
    QString                 m_filename = QString();
    QSize                   m_videoresolution = QSize();
};

#endif // UPMEDIAPLAYER_H
