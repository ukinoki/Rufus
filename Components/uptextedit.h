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

#ifndef UPTEXTEDIT_H
#define UPTEXTEDIT_H

#include <QClipboard>
#include <QFontDialog>
#include <QMimeData>
#include <QTextEdit>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QMenu>
#include "icons.h"
#include "macros.h"
#include "utils.h"

class UpTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit    UpTextEdit(QWidget *parent = Q_NULLPTR);
    explicit    UpTextEdit(QString txt, QWidget *parent = Q_NULLPTR);
    void        setiD(int id);
    int         iD() const;
    void        setIdUser(int m_id);
    int         idUser() const;
    void        setvaleuravant(QString valprec);
    QString     valeuravant() const;
    void        setvaleurapres(QString valpost);
    QString     valeurapres() const;
    void        setchamp(QString champcorrespondant);
    QString     champ() const;
    void        setImmediateToolTip(QString);
    void        settable(QString tablecorrespondant);
    QString     table() const;
    void        setText(const QString &text);
    QString     appendHtml(QString appendtext,  bool supprimeLesLignesVidesDuMilieu = true, bool rajouteunelignealafin = true);

    bool        acceptimagemimedatas() const;
    void        setAcceptImageMimeDatas(bool acceptimagemimedatas);

private:
    bool        eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void        mouseDoubleClickEvent( QMouseEvent * event ) Q_DECL_OVERRIDE;
    void        insertFromMimeData( const QMimeData *source ) Q_DECL_OVERRIDE;
    bool        canInsertFromMimeData( const QMimeData *source ) const Q_DECL_OVERRIDE;
    QString     m_valeuravant, m_valeurapres, m_champ, m_table;
    QString     m_tooltipmsg;
    bool        m_acceptimagemimedatas = false;         //! Le textedit accepte de copier-coller des images
    int         m_id, m_iduser;
    void        ChoixMenuContextuel(QString choix);

    void        MenuContextuel();

signals:
    void        dblclick(int a);
};

#endif // UPTEXTEDIT_H
