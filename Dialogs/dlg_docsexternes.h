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

#ifndef DLG_DOCSEXTERNES_H
#define DLG_DOCSEXTERNES_H

#include <QPainter>
#include <QToolBar>
#include <QtXml>
#include <poppler-qt5.h>
#include "procedures.h"
#include "updialog.h"
#include "uplabel.h"
#include "upswitch.h"
#include "uptablewidget.h"
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QMediaPlayer>
#include "playercontrols.h"
#include "cls_docexterne.h"
#include "cls_docsexternes.h"

class dlg_docsexternes : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_docsexternes(DocsExternes* Docs, bool iscurrentpatient, bool UtiliseTCP, QWidget *parent = Q_NULLPTR);
    ~dlg_docsexternes();
    Patient*                getPatient() { return m_currentpatient; }   //!> renseigne sur le patient en cours d'affichage
    enum Mode               {Zoom, Normal};                                     Q_ENUM(Mode)
    enum Importance         {Min, Norm, Max};                                   Q_ENUM(Importance)
    enum ModeTri            {parDate, parType};                                 Q_ENUM(ModeTri)
    enum ModeFiltre         {FiltreSans, NormalFiltre, ImportantFiltre};        Q_ENUM(ModeFiltre)

private:
    Procedures              *proc;
    DataBase                *db;

    DocsExternes            *m_docsexternes;
    DocExterne              *docencours;
    Patient                 *m_currentpatient;
    User                    *m_currentuser;

    QGraphicsScene          *Scene;
    QGraphicsVideoItem      *videoItem;
    QGraphicsView           *GraphicView;
    QImage                  image;
    QLabel                  *inflabel;
    PlayerControls          *playctrl;
    QMediaPlayer            *player;
    QTreeView               *ListDocsTreeView;
    UpCheckBox              *AllDocsupCheckBox;
    UpCheckBox              *OnlyImportantDocsupCheckBox;
    UpSwitch                *sw;
    UpTableWidget           *ScrollTable;

    QRectF                  rect;
    QFont                   gFont;
    QList<QPixmap>          glistPix;
    QPrinter                *printer;
    QStandardItemModel      *gmodele, *gmodeleTriParDate, *gmodeleTriParType;
    QPoint                  PosOrigine;
    QSize                   SizeOrigine;

    bool                    AvecPrevisu;
    bool                    initOK;
    double                  idealproportion;
    int                     hdelta , wdelta;
    int                     hdeltaframe, wdeltaframe;
    QString                 gTypeDoc;
    Mode                    gMode;
    ModeTri                 gModeTri;
    ModeFiltre              gModeFiltre;


    bool                    eventFilter(QObject *, QEvent *);

    void                    ActualiseDocsExternes();
    void                    AfficheCustomMenu(DocExterne *docmt);
    void                    AfficheDoc(QModelIndex idx);
    void                    BasculeTriListe(int);

    QString                 CalcTitre(DocExterne *docmt);
    void                    CorrigeImportance(DocExterne *docmt, enum Importance imptce);
    void                    EnregistreImage(DocExterne* docmt);
    void                    EnregistreVideo();

    void                    FiltrerListe(UpCheckBox *chk);
    DocExterne*             getDocumentFromIndex(QModelIndex idx);
    QModelIndex             getIndexFromId(QStandardItemModel *model, int id);
    QStandardItem*          getItemFromDocument(QStandardItemModel *model, DocExterne* docmt);
    void                    ImprimeDoc();

    bool                    ModifieEtReImprimeDoc(DocExterne *docmt, bool modifiable, bool detruirealafin);
    void                    ModifierDate(QModelIndex idx);
    void                    ModifierItem(QModelIndex idx);
    void                    Print(QPrinter*);

    bool                    ReImprimeDoc(DocExterne *docmt);
    void                    RemplirTreeView();
    void                    SupprimeDoc(DocExterne *docmt = Q_NULLPTR);
    void                    ZoomDoc();
};

#endif // DLG_DOCSEXTERNES_H
