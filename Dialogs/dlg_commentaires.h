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

#ifndef DLG_COMMENTAIRES_H
#define DLG_COMMENTAIRES_H

#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QToolTip>

#include "procedures.h"
#include "upcheckbox.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "uptablewidget.h"
#include "widgetbuttonframe.h"

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
    User                        *m_currentuser   = Datas::I()->users->userconnected();
    QTimer                      *m_timerefface    = new QTimer(this);

    Mode                        m_mode;
    WidgetButtonFrame           *wdg_buttonframe;

    QGraphicsOpacityEffect      m_opacityeffect;
    QString                     m_commentaire, m_commentaireresume;

    void                        changeEvent(QEvent *e);
    bool                        eventFilter(QObject *, QEvent *);
    void                        keyPressEvent   (QKeyEvent * event );
    void                        ChoixMenuContextuel(QString);
    void                        Del_Com();
    void                        Modif_Com();
    QString                     CalcToolTip(QString ab);
    bool                        ChercheDoublon(QString str, int row);
    void                        ConfigMode(Mode mode, int row = 0);
    void                        EffaceWidget(QWidget* widg, bool AvecOuSansPause = true);
    void                        DisableLines();
    void                        EnableLines();
    void                        InsertCommentaire(int row);
    void                        LineSelect(int row);
    void                        UpdateCommentaire(int row);
    void                        SupprimmCommentaire(int row);
    void                        Remplir_TableView();

    void                        Annulation();
    void                        ChoixButtonFrame();
    void                        MenuContextuel(UpLineEdit *line);
    void                        dblClicktextEdit();
    void                        EnableOKPushbutton();
    void                        Validation();
};

#endif // DLG_COMMENTAIRES_H
