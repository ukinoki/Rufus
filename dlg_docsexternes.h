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

class dlg_docsexternes : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_docsexternes(Procedures *ProcAPasser, int idpat, QWidget *parent = Q_NULLPTR);
    ~dlg_docsexternes();
    void                    RemplirTreeView(bool recalcul = true);
    QTreeView               *ListDocsTreeView;
private:
    Procedures              *proc;
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
    QMap<QString,QVariant>  gDataUser;
    QStandardItemModel      *gmodele, *gmodeleTriParDate, *gmodeleTriParType;
    QSqlQuery               TreeQuery;
    QSortFilterProxyModel   *gproxymodele;
    QTimer                  *MAJTreeViewTimer;
    QLabel                  *inflabel;
    QRectF                  rect;
    QPoint                  PosOrigine;
    QSize                   SizeOrigine;
    UpSwitch                *sw;
    int                     gidPatient;
    int                     hdelta , wdelta;
    int                     hdeltaframe, wdeltaframe;
    int                     nbredocs;
    int                     gMode;
    enum gMode              {Zoom, Normal};
    double                  idealproportion;
    QString                 gTypeDoc;
    int                     gModeTri;
    enum gModeTri           {parDate, parType};
    bool                    eventFilter(QObject *, QEvent *);
    bool                    AvecPrevisu;
    bool                    initOK;
    int                     CompteNbreDocs();
    bool                    EcritDansUnFichier(QString NomFichier, QByteArray TexteFichier);
    void                    ImprimeDoc(QString idimpr);
    QMap<QString,QVariant>  CalcImage(int idimpression, bool imagerie, bool afficher = true);

    void                    BasculeTriListe(int);
public slots:
    void                    Slot_AfficheDoc(QModelIndex idx);
    bool                    InitOK();
private slots:
    void                    Slot_CompteNbreDocs();
    void                    Slot_EditSousTitre(QStandardItem *item);
    void                    Slot_EnregistreVideo();
    void                    Slot_ImprimeDoc();
    void                    Slot_PlayerCtrl(int);
    void                    Slot_Print(QPrinter*);
    void                    Slot_SupprimeDoc();
    void                    Slot_ZoomDoc();
};

#endif // DLG_DOCSEXTERNES_H
