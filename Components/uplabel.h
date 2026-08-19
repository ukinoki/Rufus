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

#ifndef UPLABEL_H
#define UPLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QToolTip>
#include <QApplication>
#include "cls_item.h"

class UpLabel : public QLabel
{
    Q_OBJECT
public:
    explicit                UpLabel(QWidget *parent = nullptr, QString txt = QString());
    explicit                UpLabel(Item* itm, QString txt = QString(), QWidget* parent = nullptr);
    explicit                UpLabel(int id, QString txt = QString(), QWidget* parent = nullptr);
    ~UpLabel();
    void                    setiD(int m_id);
    int                     id() const;
    void                    setRow(int m_id);
    void                    setImmediateToolTip(QString);
    int                     Row() const;
    void                    setrufusitem(Item *item) { m_rufusitem = item; }
    Item*                   rufusitem() const        { return m_rufusitem; }
    bool                    hasrufusitem() const     { return m_rufusitem != nullptr; }

    QImage                  image() const;                                  //! Qimage used by label
    void                    setImage(const QImage &newImage);

    int                     pagepdf() const;                                //! page number used when Uplanel are used as CellWidget inside UptableWidget
    void                    setPagepdf(int newPagepdf);

private:
    bool                    eventFilter(QObject *obj, QEvent *event)  ;

    int                     m_id            = 0;
    int                     m_pagepdf       = 0;
    int                     m_row           = -1;
    QString                 m_tooltipmsg    = "";
    QImage                  m_image         = QImage();
    Item                    *m_rufusitem    = nullptr;

    void                    AfficheToolTip();

signals:
    void                    clicked(int a);
    void                    enter(int a);
    void                    dblclick(int a);
};
#endif // UPLABEL_H
