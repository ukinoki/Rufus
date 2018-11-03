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
    explicit dlg_docsexternes(Procedures *ProcAPasser, int idpat, QWidget *parent = Q_NULLPTR);
    ~dlg_docsexternes();
    bool                    InitOK();

private:
    Procedures              *proc;
    DocsExternes            m_ListDocs;
    DocExterne              *docencours;
    DataBase                *db;
    Patient                 *patient;

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
    int                     gidPatient;
    int                     hdelta , wdelta;
    int                     hdeltaframe, wdeltaframe;
    int                     gMode;
    enum gMode              {Zoom, Normal};
    int                     Importance;
    enum Importance         {Min, Norm, Max};
    int                     gModeTri;
    enum gModeTri           {parDate, parType};
    int                     gModeFiltre;
    enum gModeFiltre        {FiltreSans, NormalFiltre, ImportantFiltre};
    double                  idealproportion;
    QString                 gTypeDoc;

    bool                    eventFilter(QObject *, QEvent *);

    void                    ActualiseDocsExternes();
    void                    AfficheCustomMenu(DocExterne *docmt);
    void                    AfficheDoc(QModelIndex idx);
    void                    BasculeTriListe(int);
    QMap<QString,QVariant>  CalcImage(int idimpression, bool imagerie, bool afficher = true);

    QString                 CalcTitre(DocExterne *docmt);
    int                     CompteNbreDocs();
    void                    CorrigeImportance(DocExterne *docmt, enum Importance imptce);
    bool                    EcritDansUnFichier(QString NomFichier, QByteArray TexteFichier);
    void                    EnregistreVideo();

    void                    FiltrerListe(UpCheckBox *chk);
    DocExterne*             getDocumentFromIndex(QModelIndex idx);
    QModelIndex             getIndexFromId(QStandardItemModel *model, int id);
    QStandardItem*          getItemFromDocument(QStandardItemModel *model, DocExterne* docmt);
    void                    ImprimeDoc();

    int                     initListDocs();
    bool                    ModifieEtReImprimeDoc(DocExterne *docmt, bool modifiable, bool detruirealafin);
    void                    ModifierItem(QModelIndex idx);
    void                    PlayerCtrl(int);
    void                    Print(QPrinter*);

    bool                    ReImprimeDoc(DocExterne *docmt);
    void                    RemplirTreeView();
    void                    SupprimeDoc(DocExterne *docmt = Q_NULLPTR);
    void                    ZoomDoc();
};

#endif // DLG_DOCSEXTERNES_H
