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

#ifndef DLG_LISTECOMMENTAIRES_H
#define DLG_LISTECOMMENTAIRES_H

#include "uptableview.h"
#include "updelegate.h"
#include "procedures.h"
#include "upstandarditemmodel.h"

class dlg_listecommentaires : public UpDialog
{
    Q_OBJECT
public:
    dlg_listecommentaires(QWidget *parent = Q_NULLPTR);
    ~dlg_listecommentaires();

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
    UpStandardItemModel         *m_model = Q_NULLPTR;
    CommentLunet                *m_currentcomment = Q_NULLPTR;
    QHash<QString, QVariant>    m_listbinds;
    QString                     m_commentaire = "";
    QString                     m_commentaireresume = "";

    bool                        eventFilter(QObject *, QEvent *);
    void                        keyPressEvent   (QKeyEvent * event );
    void                        Annulation();
    bool                        ChercheDoublon(QString str, int row);
    void                        ChoixButtonFrame();
    void                        ChoixMenuContextuel(QString);
    void                        ConfigMode(Mode mode, CommentLunet *com = Q_NULLPTR);
    void                        dblClicktextEdit();
    void                        DisableLines();
    void                        EnableButtons(CommentLunet* com = Q_NULLPTR);
    void                        EnableLines();
    void                        EnregistreCommentaire(CommentLunet *com);
    CommentLunet*               getCommentFromIndex(QModelIndex idx);
    void                        MenuContextuel();
    void                        RemplirTableView();
    void                        selectcurrentComment(CommentLunet *com, QAbstractItemView::ScrollHint hint = QAbstractItemView::EnsureVisible);
    void                        setCommentToRow(CommentLunet *com, int row);
    void                        SupprimmCommentaire(CommentLunet *com);
    void                        Validation();
};

#endif // DLG_LISTECOMMENTAIRES_H
