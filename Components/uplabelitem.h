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

#ifndef UPLABELITEM_H
#define UPLABELITEM_H

#include <QLabel>
#include <QMouseEvent>
#include <QToolTip>

#include "cls_item.h"

/*! cette classe est un doublon de la classe UpLabel qui incorpore la référence vers la classe Item.
 *  Il est impossible d'incorporer la classe Item dans UpLabel
 *  sans créer un problème de référence circulaire que je ne suis pas arrivé à résoudre */

class Item;

class UpLabelItem : public QLabel
{
    Q_OBJECT
public:
    explicit                UpLabelItem(QWidget *parent = Q_NULLPTR, QString txt = QString());
    explicit                UpLabelItem(Item* item = Q_NULLPTR, QString txt = QString(), QWidget *parent = Q_NULLPTR);
    ~UpLabelItem();
    void                    setiD(int m_id);
    int                     iD() const;
    void                    setdatas(QMap<QString, QVariant> datas);
    QMap<QString, QVariant> datas() const;
    void                    setRow(int m_id);
    void                    setImmediateToolTip(QString);
    int                     Row() const;
    void                    setitem(Item* item) { m_item = item; }
    Item*                   item() const        { return m_item; }
    bool                    hasitem() const     { return m_item != Q_NULLPTR; }

private:
    Item*                   m_item          = Q_NULLPTR;
    int                     m_id            = -1;
    int                     m_row           = -1;
    QString                 m_tooltipmsg    = "";
    QMap<QString, QVariant> m_datas         = QMap<QString, QVariant>();
    void                    AfficheToolTip();
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
signals:
    void                    clicked(int a);
    void                    enter(int a);
    void                    dblclick(int a);
};

#endif // UPLABELITEM_H
