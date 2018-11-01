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
    void                    RemplirTreeView();
    QTreeView               *ListDocsTreeView;
    void                    AfficheDoc(QModelIndex idx);
    bool                    InitOK();

private:
    Procedures              *proc;
    DocsExternes            m_ListDocs;
    DocExterne              *docencours;
    DataBase                *db;
    QAction                 *fin, *debut, *prec, *suiv;
    QFont                   gFont;
    QImage                  image;
    QList<QPixmap>          glistPix;
    QPrinter                *printer;
    UpTableWidget           *ScrollTable;
    QGraphicsPixmapItem     *jpgPixmap;
    QGraphicsScene          *Scene;
    QGraphicsView           *GraphicView;
    QGraphicsVideoItem      *videoItem;
    QGraphicsPixmapItem     *jpgItem;
    PlayerControls          *playctrl;
    QMediaPlayer            *player;
    QStandardItemModel      *gmodele, *gmodeleTriParDate, *gmodeleTriParType;
    QTimer                  *MAJTreeViewTimer;
    QLabel                  *inflabel;
    QRectF                  rect;
    QPoint                  PosOrigine;
    QSize                   SizeOrigine;
    UpCheckBox              *AllDocsupCheckBox;
    UpCheckBox              *OnlyImportantDocsupCheckBox;
    UpSwitch                *sw;
    int                     gidPatient;
    int                     hdelta , wdelta;
    int                     hdeltaframe, wdeltaframe;
    int                     nbredocs;
    int                     gMode;
    enum gMode              {Zoom, Normal};
    int                     Importance;
    enum Importance         {Min, Norm, Max};
    double                  idealproportion;
    QString                 gTypeDoc;
    int                     gModeTri;
    enum gModeTri           {parDate, parType};
    bool                    eventFilter(QObject *, QEvent *);
    bool                    AvecPrevisu;
    bool                    initOK;
    void                    AfficheCustomMenu();
    int                     CompteNbreDocs();
    bool                    EcritDansUnFichier(QString NomFichier, QByteArray TexteFichier);
    QMap<QString,QVariant>  CalcImage(int idimpression, bool imagerie, bool afficher = true);

    void                    BasculeTriListe(int);
    QString                 CalcTitre(DocExterne *docmt);
    void                    CorrigeImportance(DocExterne *docmt, enum Importance imptce);
    void                    EnregistreVideo();
    void                    FiltrerListe(UpCheckBox *chk);
    DocExterne*             getDocumentFromIndex(QModelIndex idx);
    QModelIndex             getIndexFromId(int id);
    QStandardItem*          getItemFromDocument(DocExterne* docmt);
    void                    ImprimeDoc();
    int                     initListDocs();
    void                    ModifierItem(QModelIndex idx);
    void                    PlayerCtrl(int);
    void                    SupprimeDoc();
    void                    ZoomDoc();
    void                    Print(QPrinter*);

private slots:
    void                    Slot_CompteNbreDocs();
};

#endif // DLG_DOCSEXTERNES_H
