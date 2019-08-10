/* (C) 2018 LAINE SERGE
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

#ifndef UPCHECKBOX_H
#define UPCHECKBOX_H

#include <QCheckBox>
#include <QToolTip>
#include <QMouseEvent>

class UpCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit UpCheckBox(QWidget *parent = Q_NULLPTR);
    explicit UpCheckBox(QString text, QWidget *parent = Q_NULLPTR);
    ~UpCheckBox();

    void    setRowTable(int val);
    int     rowTable() const;
    void    setiD(int val);
    int     iD() const;
    void    setToggleable(bool val);
            // cette propriété permet de bloquer la modification d'état du checkBox
            // la propriété checkable n'est pas utilisable car elle ne permet pas de bloquer le checkbox en position checked
    bool    Toggleable() const;
    void    setImmediateToolTip(QString);

signals:
    void    uptoggled(bool check);
            // ce signal ne sera émis que si la propriété Toggleabble est true et permet de se départir des comportements implicites du signal toggled()

private:
    int     m_rowtable, m_id;
    bool    m_toggleable;
    QString m_tooltipmsg;
    bool    eventFilter(QObject *obj, QEvent *event)  ;
    void    AfficheToolTip();

signals:
    void    enter();
};

#endif // UPCHECKBOX_H
