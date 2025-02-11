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

#include "dlg_imageviewer.h"
#include "upmediaplayer.h"
#include "upvideowidget.h"
#include "imagewidget.h"

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
    QList<int>              m_listiddocsimagery = QList<int>();    //! les id des docsexternes qui correspondent à des documenst d'imagerie
    DocExterne              *m_currentdocument = Q_NULLPTR;

/*! la classe dlg_docsexternes affiche les documents pdf, jpg ou video dans une fiche Updialog
 *  Les intitulés des documents sont affichés à gauche dans un QTreeView *wdg_listdocstreewiew
 *  et les documents eux-mêmes sont affichés à droite dans
 *      . un UpTablWidget *wdg_pdftbl pour les pdf,
 *      . un QVideoWidget wdg_video pour les videos
 *      . ou un UpLabel wdg_jpglbl pour les jpg */

    QScrollArea             *mainscroll = new QScrollArea();

    QTreeView               *wdg_listdocstreewiew   = new QTreeView();
    int                     m_treeviewwidth         = 185;
    int                     m_spacing               = 10;
    QMargins                m_margins               = QMargins (0,0,0,0);
    QHBoxLayout             *m_lay                  = new QHBoxLayout();

    /*! les pdf sont affichées via un UpTableWidget *wdg_pdftbl    */
    UpTableWidget           *wdg_pdftbl             = Q_NULLPTR;

    /*! les video sont affichées via un QVideoWidget *wdg_viedo    */
    UpVideoWidget           *wdg_video              = Q_NULLPTR;
    PlayerControls          *wdg_playctrl           = new PlayerControls;

    /*! lesjpg sont affichés via un UpLabel *wdg_jpglbl    */
    UpLabel                 *wdg_jpglbl             = Q_NULLPTR;
    ImageWidget             *wdg_imgwdg             = Q_NULLPTR;



    UpCheckBox              *wdg_alldocsupcheckbox    = Q_NULLPTR;
    UpCheckBox              *wdg_onlyimportantsdocsupcheckbox    = Q_NULLPTR;
    UpComboBox              *wdg_updatetypebox  = Q_NULLPTR;
    QLabel                  *wdg_inflabel       = Q_NULLPTR;

    QStandardItemModel      *m_model            = Q_NULLPTR;
    QStandardItemModel      *m_tripardatemodel  = Q_NULLPTR;
    QStandardItemModel      *m_tripartypemodel  = Q_NULLPTR;
    QPoint                  m_positionorigin;
    QSize                   m_sizeorigin;

    double                  m_vidorimgratio = 1;
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

    void                    OpenMultiImageViewer(int iddoc = 0);

    void                    ReImprimeDoc(DocExterne *docmt);
    void                    RemplirTreeView();
    void                    SupprimeDoc(DocExterne *docmt = Q_NULLPTR);
    void                    ZoomDoc(bool changemode = true);
    QSize                   sizeforunit() {
        int width = sizefordisplay().width()
                    - m_treeviewwidth
                    - m_margins.left()
                    - m_margins.right()
                    - m_spacing;      //!estimated width for one pic in a row
        int height = sizefordisplay().height();
        return QSize(width,height);
    }
};

#endif // DLG_DOCSEXTERNES_H
