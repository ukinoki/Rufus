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

#ifndef WIDGPACHY_H
#define WIDGPACHY_H

#include <QWidget>

namespace Ui {
class WidgPachy;
}

class WidgPachy : public QWidget
{
    Q_OBJECT

public:
    explicit WidgPachy(QWidget *parent = nullptr);
    ~WidgPachy();
    Ui::WidgPachy *ui;

private:
};

#endif // WIDGPACHY_H
