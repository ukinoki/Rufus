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

#ifndef UPLINEEDIT_H
#define UPLINEEDIT_H

#include <QLineEdit>
#include <QDoubleValidator>
#include <QEvent>
#include <QSound>
#include <QTimer>
#include <QToolTip>

class UpLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit    UpLineEdit(QWidget *parent = Q_NULLPTR);
    ~UpLineEdit();
    void        setRow(int val);
    int         Row() const;
    void        setiD(int m_id);
    int         iD() const;
    void        setColumn(int val);
    int         Column() const;
    void        setdatas(QVariant data);
    QVariant    datas() const;
    void        setvaleuravant(QString valprec);
    QString     valeuravant() const;
    void        setvaleurapres(QString valpost);
    QString     valeurapres() const;
    void        setchamp(QString champcorrespondant);
    QString     champ() const;
    void        settable(QString tablecorrespondant);
    QString     table() const;
    void        setImmediateToolTip(QString Msg);
    void        selectAll();

private:
    QString     m_tooltipmsg;
    bool        eventFilter(QObject *obj, QEvent *event)  ;
    int         m_row, m_col, m_id;
    QString     m_valeuravant, m_valeurapres, m_champ, m_table;
    QVariant    m_datas;
    void        AfficheToolTip();

signals:
    void        TextModified(QString);
    void        mouseEnter(int row);
    void        mouseRelease(int row);
    void        mouseDoubleClick(int row);

protected:
    void        enterEvent(QEvent *e);
    void        mouseReleaseEvent(QMouseEvent *e);
    void        mouseDoubleClickEvent(QMouseEvent *e);

};

#endif // UPLINEEDIT_H
