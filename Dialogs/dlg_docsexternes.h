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

#ifndef DLG_DOCSEXTERNES_H
#define DLG_DOCSEXTERNES_H

#include "procedures.h"
#include "upswitch.h"
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include "playercontrols.h"

class dlg_docsexternes : public UpDialog
{
    Q_OBJECT
public:
    explicit dlg_docsexternes(DocsExternes* Docs, bool UtiliseTCP, QWidget *parent = Q_NULLPTR);
    ~dlg_docsexternes();
    Patient*                currentpatient() const { return m_docsexternes->patient(); } //!> renseigne le patient en cours d'affichage
    enum Mode               {Zoom, Normal};                                     Q_ENUM(Mode)
    enum Importance         {Min, Norm, Max};                                   Q_ENUM(Importance)
    enum ModeTri            {parDate, parType};                                 Q_ENUM(ModeTri)
    enum ModeFiltre         {FiltreSans, NormalFiltre, ImportantFiltre};        Q_ENUM(ModeFiltre)
    Mode                    mode() { return m_mode; };

private:
    DataBase                *db             = DataBase::I();
    Procedures              *proc           = Procedures::I();
    QFont                   m_font          = QApplication::font();
    User*                   currentuser() { return Datas::I()->users->userconnected(); }
    QDate                   m_currentdate = db->ServerDate();

    DocsExternes            *m_docsexternes;

    QGraphicsScene          *obj_graphicscene;
    QGraphicsVideoItem      *medobj_videoitem;
    QGraphicsView           *graphview_view;
    QImage                  img_image;
    QMediaPlayer            *medplay_player;

    QTreeView               *wdg_listdocstreewiew;
    UpCheckBox              *wdg_alldocsupcheckbox;
    UpCheckBox              *wdg_onlyimportantsdocsupcheckbox;
    UpSwitch                *wdg_upswitch;
    UpTableWidget           *wdg_scrolltablewidget;
    QLabel                  *wdg_inflabel;
    PlayerControls          *wdg_playctrl;

    QRectF                  m_rect;
    QList<QPixmap>          m_listpixmp;
    QPrinter                *m_printer;
    QStandardItemModel      *m_model = Q_NULLPTR;
    QStandardItemModel      *m_tripardatemodel = Q_NULLPTR;
    QStandardItemModel      *m_tripartypemodel = Q_NULLPTR;
    QPoint                  m_positionorigin;
    QSize                   m_sizeorigin;

    bool                    m_avecprevisu;
    double                  m_idealproportion;
    int                     m_hdelta , m_wdelta;
    int                     m_hdeltaframe, m_wdeltaframe;
    QString                 m_typedoc;
    Mode                    m_mode;
    ModeTri                 m_modetri;
    ModeFiltre              m_modefiltre;


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
    void                    Print(QPrinter* = Q_NULLPTR);

    bool                    ReImprimeDoc(DocExterne *docmt);
    void                    RemplirTreeView();
    void                    SupprimeDoc(DocExterne *docmt = Q_NULLPTR);
    void                    ZoomDoc();
};

#endif // DLG_DOCSEXTERNES_H
