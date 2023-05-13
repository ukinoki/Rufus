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

#ifndef UPPROGRESDIALOG_H
#define UPPROGRESDIALOG_H

#include <QProgressDialog>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLabel>

class UpProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    explicit    UpProgressDialog(int n, int t, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::Popup);
    ~UpProgressDialog();
    void show() {forceShow();};

private:
    QLabel *label = new QLabel();
    QProgressBar *bar = new QProgressBar();
};

#endif // UPPROGRESDIALOG_H
