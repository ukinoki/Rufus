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

#ifndef DLG_DOCSVIDEO_H
#define DLG_DOCSVIDEO_H

#include "updialog.h"
#include "procedures.h"
#include "upcombobox.h"
#include "uplabel.h"
#include "uplineedit.h"
#include "uptablewidget.h"
#include "uptoolbar.h"
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QMediaRecorder>


class dlg_docsvideo : public UpDialog
{
    Q_OBJECT
public:
    dlg_docsvideo(Patient *pat, QWidget *parent = Q_NULLPTR);
    void                NavigueVers(QString);

private:
    DataBase            *db = DataBase::I();
    Procedures          *proc = Procedures::I();
    Patient             *m_currentpatient;
    QList<QPixmap>      m_listpixmaps;
    QStringList         m_listtypesexamen;
    QString             m_docpath;

    UpLabel             *wdg_inflabel;
    UpLineEdit          *wdg_linetitre;
    QDateEdit           *wdg_editdate;
    UpComboBox          *wdg_typedoccombobx;
    QVideoWidget        *wdg_visuvideowdg;
    UpToolBar           *wdg_toolbar;
    UpPushButton        *wdg_dirsearchbutton;

    void                AfficheVideo(QString filebut);
    void                ChangeFile();
    void                ValideFiche();
};

#endif // DLG_DOCSVIDEO_H
