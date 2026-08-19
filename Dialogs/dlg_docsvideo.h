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

#ifndef DLG_DOCSVIDEO_H
#define DLG_DOCSVIDEO_H

#include <QDateEdit>
#include "upcombobox.h"
#include "uptoolbar.h"
#include "dlg_singleimageviewer.h"
#include "procedures.h"


class dlg_docsvideo : public QObject
{
    Q_OBJECT
public:
    dlg_docsvideo(Patient *pat, QWidget *parent = nullptr);
    void                NavigueVers(UpToolBar::Choice choix);

    dlg_singleimageviewer *dialog() const;
    bool                initOK() { return m_initOK; }

private:
    DataBase            *db                 = DataBase::I();
    Procedures          *proc               = Procedures::I();
    Patient             *m_currentpatient   = nullptr;
    QStringList         m_listtypesexamen   = QStringList();
    QString             m_docpath           = QString();
    QDate               m_currentdate       = db->ServerDate();
    QString             m_currentvideofile  = QString();

    UpLineEdit          *wdg_linetitre          = nullptr;
    QDateEdit           *wdg_editdate           = nullptr;
    UpComboBox          *wdg_typedoccombobx     = nullptr;
    UpToolBar           *wdg_toolbar            = nullptr;
    UpPushButton        *wdg_dirsearchbutton    = nullptr;
    dlg_singleimageviewer *dlg_imgviewer        = nullptr;
    bool                m_initOK                = false;
    QStringList         m_filters               = QStringList() << "*.mp4" << "*.mpg" << "*.m4v";

    void                addFullScreenMenu();
    void                ChangeFile();
    void                ValideFiche();
    bool                searchDir(bool &pathchanged);
    QMenu               *m_Menu                 = nullptr;
};

#endif // DLG_DOCSVIDEO_H
