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
    dlg_docsvideo(int idPat, QWidget *parent = Q_NULLPTR);
    void                NavigueVers(QString);

private:
    Procedures          *proc;
    DataBase            *db;
    int                 idpat;
    UpLabel             *inflabel;
    UpLineEdit          *linetitre;
    QDateEdit           *editdate;
    UpComboBox          *typeDocCombo;
    QVideoWidget        *upvisu;
    UpToolBar           *toolbar;
    UpPushButton        *dirsearchbutton;
    QList<QPixmap>      glistPix;
    QStringList         ListTypeExams;
    QString             docpath;
    void                AfficheVideo(QString filebut);
    void                ChangeFile();
    void                ValideFiche();
};

#endif // DLG_DOCSVIDEO_H
