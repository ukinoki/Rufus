/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UPTABLEWIDGET_H
#define UPTABLEWIDGET_H

#include <QTableWidget>
#include <QHeaderView>
#include <QMouseEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>


class UpTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit UpTableWidget(QWidget *parent = Q_NULLPTR);
    ~UpTableWidget();

    void        clearSelection();
    void        FixLargeurTotale();
    QString     Attribut();
    void        setAttribut(QString attrib);
    int         rowNoHiddenCount();
    QByteArray  dropData();

private:
    QString     Attrib;
    QByteArray  encodedData;

protected:
    void        dropEvent(QDropEvent *) Q_DECL_OVERRIDE;

signals:
    void        dropsignal(QByteArray);

};

#endif // UPTABLEWIDGET_H
