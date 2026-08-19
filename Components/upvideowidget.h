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

#ifndef UPVIDEOWIDGET_H
#define UPVIDEOWIDGET_H

#include "upmediaplayer.h"
#include <QMouseEvent>
#include <QVideoWidget>
#include "cls_item.h"

class UpVideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    UpVideoWidget(QWidget *parent = nullptr);

    QString         filename() const;
    void            setFilename(const QString &newFilename);

    UpMediaPlayer*  player() const;
    void            setPlayer(UpMediaPlayer *newPlayer);

    void            setrufusitem(Item* item)    { m_rufusitem = item; }
    Item*           rufusitem() const           { return m_rufusitem; }
    bool            hasrufusitem() const        { return m_rufusitem != nullptr; }
    int             id() const                  { return m_id; }
    void            setId(int id)               { m_id = id; }

private:
    bool            eventFilter(QObject *obj, QEvent *event)  ;
    void            keyPressEvent (QKeyEvent * keyEvent );
    QString         m_filename     = "";
    UpMediaPlayer   *m_player      = nullptr;
    Item            *m_rufusitem   = nullptr;
    int             m_id           = -1;

signals:
    void            clicked(int a);
    void            enter(int a);
    void            dblclick(int a);
};

#endif // UPVIDEOWIDGET_H
