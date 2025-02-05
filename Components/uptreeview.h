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

#ifndef UPTREEVIEW_H
#define UPTREEVIEW_H

#include <QTreeView>

class UpTreeView : public QTreeView
{
    Q_OBJECT
public:
    UpTreeView(QWidget *parent = Q_NULLPTR);

protected:
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
};

#endif // UPTREEVIEW_H
