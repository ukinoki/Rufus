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

#ifndef DLG_DOCSSCANNER_H
#define DLG_DOCSSCANNER_H

#include "procedures.h"

class dlg_docsscanner : public UpDialog
{
    Q_OBJECT
public:
    enum Mode          {Document, Facture, Echeancier};    Q_ENUM(Mode)
    Mode                m_mode;
    explicit dlg_docsscanner(Item *item, Mode mode = Document, QString titre = "", QWidget *parent = Q_NULLPTR);
    ~dlg_docsscanner();
    bool                    initOK() const;
    void                    NavigueVers(QString);
    QMap<QString, QVariant> getdataFacture();

private:
    DataBase                *db = DataBase::I();
    Procedures              *proc = Procedures::I();
    bool                    m_accesdistant;
    QString                 m_pathdirstockageimagerie;

    int                     m_iditem;
    bool                    m_initok;
    QStringList             m_listtypesexamen;
    QString                 m_docpath;
    QString                 m_nomfichierimageencours;
    QMap<QString, QVariant> map_datafacture;
    QList<QImage>           m_listimages;

    UpLineEdit              *wdg_linetitre;
    QDateEdit               *wdg_editdate;
    UpComboBox              *wdg_typedoccombobx;
    UpTableWidget           *wdg_uptable;
    UpToolBar               *wdg_toolbar;
    UpPushButton            *wdg_dirsearchbutton;
    QLabel                  *wdg_inflabel;

    bool                    eventFilter(QObject *, QEvent *);
    void                    ValideFiche();
    void                    ChangeFile();
};

#endif // DLG_DOCSSCANNER_H
