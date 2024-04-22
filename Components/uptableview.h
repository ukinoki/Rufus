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

#ifndef UPTABLEVIEW_H
#define UPTABLEVIEW_H

#include <QTableView>
#include "utils.h"
#include "macros.h"
#include "uptextedit.h"

class UpTextEdit;
class Utils;
class UpTableView : public QTableView
{
    Q_OBJECT
public:
    explicit UpTableView(QWidget *parent = Q_NULLPTR);
    void    FixLargeurTotale();
    int     rowNoHiddenCount() const;
};

#endif // UPTABLEVIEW_H
