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

#ifndef DLG_COMMENTAIRES_H
#define DLG_COMMENTAIRES_H

#include "uptableview.h"
#include "updelegate.h"
#include "procedures.h"

namespace Ui {
class dlg_commentaires;
}

class dlg_commentaires : public UpDialog
{
    Q_OBJECT
public:
    dlg_commentaires(QWidget *parent = Q_NULLPTR);
    ~dlg_commentaires();
    Ui::dlg_commentaires                *ui;
    QString Commentaire() const         { return m_commentaire; }
    QString CommentaireResume() const   { return m_commentaireresume; }
    enum Mode                           {Selection, Modification, Creation};     Q_ENUM(Mode)

private:
    DataBase                    *db              = DataBase::I();
    User*                       currentuser() { return Datas::I()->users->userconnected(); }

    Mode                        m_mode;
    UpTableView                 *wdg_tblview;
    UpTextEdit                  *wdg_comtxt;
    WidgetButtonFrame           *wdg_buttonframe;
    QStandardItemModel          *m_model;
    CommentLunet                *m_currentcomment = Q_NULLPTR;
    QHash<QString, QVariant>    m_listbinds;
    QString                     m_commentaire = "";
    QString                     m_commentaireresume = "";

    void                        changeEvent(QEvent *e);
    bool                        eventFilter(QObject *, QEvent *);
    void                        keyPressEvent   (QKeyEvent * event );
    void                        Annulation();
    bool                        ChercheDoublon(QString str, int row);
    void                        ChoixButtonFrame();
    void                        ChoixMenuContextuel(QString);
    void                        ConfigMode(Mode mode, CommentLunet *com = Q_NULLPTR);
    void                        dblClicktextEdit();
    void                        DisableLines();
    void                        Enablebuttons(QModelIndex idx);
    void                        EnableLines();
    void                        EnregistreCommentaire(CommentLunet *com);
    CommentLunet*               getCommentFromIndex(QModelIndex idx);
    int                         getRowFromComment(CommentLunet *com);
    void                        MenuContextuel();
    void                        RemplirTableView();
    void                        selectcurrentComment(CommentLunet *com);
    void                        setCommentToRow(CommentLunet *com, int row);
    void                        SupprimmCommentaire(CommentLunet *com);
    void                        Validation();
};

#endif // DLG_COMMENTAIRES_H
