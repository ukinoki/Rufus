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

#ifndef UPCHECKBOX_H
#define UPCHECKBOX_H

#include <QCheckBox>
#include <QToolTip>
#include <QMouseEvent>
#include <QApplication>

class UpCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit UpCheckBox(QWidget *parent = nullptr);
    explicit UpCheckBox(const QString &text, QWidget *parent = nullptr);
    ~UpCheckBox() override = default;

    void    setRowTable(int val);
    int     rowTable() const;
    void    setiD(int val);
    int     iD() const;
    void    setToggleable(bool val);
            // Cette propriété permet de bloquer la modification d'état du checkBox.
            // La propriété checkable n'est pas utilisable car elle ne permet pas
            // de bloquer le checkbox en position checked.
    bool    Toggleable() const;
    void    setImmediateToolTip(const QString &msg);

    int     columntable() const;
    void    setColumntable(int newColumntable);

signals:
    void    uptoggled(bool check);
            // Émis uniquement si Toggleable est true ; permet de s'affranchir
            // des comportements implicites du signal toggled().
    void    enter();

private:
    int     m_rowtable    = -1;
    int     m_id          = -1;
    int     m_columntable = -1;
    bool    m_toggleable  = true;
    QString m_tooltipmsg;

    bool    eventFilter(QObject *obj, QEvent *event) override;
    void    afficheToolTip();
};

#endif // UPCHECKBOX_H
