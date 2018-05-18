/* (C) 2016 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UPTEXTEDIT_H
#define UPTEXTEDIT_H

#include <QTextEdit>
#include <QMouseEvent>
#include <QKeyEvent>

class UpTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit    UpTextEdit(QWidget *parent = Q_NULLPTR);
    explicit    UpTextEdit(QString txt, QWidget *parent = Q_NULLPTR);
                ~UpTextEdit();
    void        setId(int id);
    int         getId() const;
    void        setIdUser(int id);
    int         getIdUser() const;
    void        setValeurAvant(QString valprec);
    QString     getValeurAvant() const;
    void        setValeurApres(QString valpost);
    QString     getValeurApres() const;
    void        setChampCorrespondant(QString champcorrespondant);
    QString     getChampCorrespondant() const;
    void        setTableCorrespondant(QString tablecorrespondant);
    QString     getTableCorrespondant() const;

private:
    void        mouseDoubleClickEvent( QMouseEvent * event ) Q_DECL_OVERRIDE;
    bool        eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    QString     ValeurAvant, ValeurApres, Champ, Table;
    int         id, idUser;

signals:
    void        dblclick(int a);
};

#endif // UPTEXTEDIT_H
