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

#include <QGraphicsVideoItem>
#include <QGraphicsView>

#include "procedures.h"
#include "upswitch.h"
#include "playercontrols.h"
#include "textprinter.h"
#include <uppushbutton.h>
#include <upcheckbox.h>
#include "dlg_imageviewer.h"
#include <QGraphicsOpacityEffect>

class PlayerControls;
class UpPushButton;
class UpCheckBox;
class TextPrinter;
class UpSwitch;


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
    Mode                    mode()   {return m_mode;}
    DocsExternes*           docsexternes() { return m_docsexternes;}

private:
    DataBase                *db             = DataBase::I();
    Procedures              *proc           = Procedures::I();
    QFont                   m_font          = QApplication::font();
    User*                   currentuser()   { return Datas::I()->users->userconnected(); }
    QDate                   m_currentdate   = db->ServerDate();

    DocsExternes            *m_docsexternes;    //! tous les docsexternes
    DocsExternes            *m_docsimagery;     //! les docsexternes qui correspondent à des documenst d'imagerie
    DocExterne              *m_currentdocument = Q_NULLPTR;

/*! la classe dlg_docsexternes affiche les documents pdf, jpg ou video dans une fiche Updialog
 *  Les intitulés des documents sont affichés à gauche dans un QTreeView *wdg_listdocstreewiew
 *  et les documents eux-mêmes sont affichés à droite dans
 *      . un UpTablWidget *wdg_scrolltablewidget pour les pdf ou un QGraphicsVieuw wdg_graphview pour les videos et les jpg
 */
    QTreeView               *wdg_listdocstreewiew       = Q_NULLPTR;
    UpTableWidget           *wdg_scrolltablewidget      = Q_NULLPTR;
    QHBoxLayout *m_lay    = new QHBoxLayout();

    /*! les video et les jpg sont affichés via une QGraphicsScene *obj_graphicscene dans un QGraphicsView *wdg_graphview    */
    QGraphicsView           *wdg_graphview              = Q_NULLPTR;
    QGraphicsScene          *obj_graphicscene           = Q_NULLPTR;
        /*! Le QGraphicsScene *obj_graphicscene affiche
         *     . les video dans un QGraphicsVideoItem  *obj_videoitem
         *     . les jpg dans un QPixmap créé à la volée
        */
        QGraphicsVideoItem      *obj_videoitem          = Q_NULLPTR;

            /*! le QGraphicsVideoItem *obj_videoitem est alimenté par un QMediaPlayer *medplay_player
             *  contrôlé parcontrôlé par le PlayerControls *wdg_playctrl
            */
                QMediaPlayer            *medplay_player = Q_NULLPTR;
                PlayerControls          *wdg_playctrl   = Q_NULLPTR;

    /*! les pdf sont affichés via des QLabel dans la UpTableWidget *wdg_scrolltablewidget.
     *  Le pdf est transformé en liste d'images QList<QImage> m_imagelist via la fonction pagelist() de cls_docexterne.h
     *  chaque image est transformée en QPixmap puis insérée dans un QLabel, inséré lui-même dans une cellule du scrolltablewidget
     */
    QList<QImage>           m_imagelist;

    /*! pour l'affichage des jpg comme des pdf, une listde QPixmap est créée QList<QPixmap> m_listpixmp;
     *  pour gérer le redimensionnement en mode zoom en redimensionnement
     *  cette liste est effacée à chaque changement de document.
     *  Pour les jpg, elle ne contient qu'un seul Pixmap
     */

    QList<QPixmap>          m_listpixmp;

    UpCheckBox              *wdg_alldocsupcheckbox    = Q_NULLPTR;
    UpCheckBox              *wdg_onlyimportantsdocsupcheckbox    = Q_NULLPTR;
    UpSwitch                *wdg_upswitch       = Q_NULLPTR;
    QLabel                  *wdg_inflabel       = Q_NULLPTR;

    QStandardItemModel      *m_model            = Q_NULLPTR;
    QStandardItemModel      *m_tripardatemodel  = Q_NULLPTR;
    QStandardItemModel      *m_tripartypemodel  = Q_NULLPTR;
    QPoint                  m_positionorigin;
    QSize                   m_sizeorigin;

    double                  m_idealproportion;
    int                     m_hdelta , m_wdelta;
    int                     m_hdeltaframe, m_wdeltaframe;
    Mode                    m_mode;
    ModeTri                 m_modetri;
    ModeFiltre              m_modefiltre;


    bool                    eventFilter(QObject *, QEvent *);

    void                    ActualiseDocsExternes();
    void                    AfficheCustomMenu(DocExterne *docmt);
    void                    AfficheDoc(QModelIndex idx);
    void                    BasculeTriListe(dlg_docsexternes::ModeTri mode);

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

    void                    OpenMultiImageViewer();

    void                    ReImprimeDoc(DocExterne *docmt);
    void                    RemplirTreeView();
    void                    SupprimeDoc(DocExterne *docmt = Q_NULLPTR);
    void                    ZoomDoc();
};

#endif // DLG_DOCSEXTERNES_H
