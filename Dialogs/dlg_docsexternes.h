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

#include "dlg_multiimageviewer.h"
#include "upmediaplayer.h"
#include "upvideowidget.h"

class dlg_docsexternes : public dlg_singleimageviewer
{
    Q_OBJECT
public:
    explicit dlg_docsexternes(DocsExternes* Docs, bool UtiliseTCP, QWidget *parent = Q_NULLPTR);
    ~dlg_docsexternes();
    Patient*                currentpatient() const { return m_docsexternes->patient(); } //!> renseigne le patient en cours d'affichage
    enum Importance         {Min, Norm, Max};                                   Q_ENUM(Importance)
    enum ModeTri            {parDate, parType};                                 Q_ENUM(ModeTri)
    enum ModeFiltre         {FiltreSans, NormalFiltre, ImportantFiltre};        Q_ENUM(ModeFiltre)
    DocsExternes*           docsexternes() { return m_docsexternes;}

private:
    DataBase                *db                 = DataBase::I();
    Procedures              *proc               = Procedures::I();
    QFont                   m_font              = QApplication::font();
    User*                   currentuser()       { return Datas::I()->users->userconnected(); }
    QDate                   m_currentdate       = db->ServerDate();

    DocsExternes            *m_docsexternes;    //! tous les docsexternes
    QList<int>              m_listiddocsimagery = QList<int>();    //! les id des docsexternes qui correspondent à des documenst d'imagerie
    DocExterne              *m_currentdocument  = Q_NULLPTR;

    UpSmallButton           *m_ZoomInButton     = new UpSmallButton();
    UpSmallButton           *m_ZoomOutButton    = new UpSmallButton();

/*! la classe dlg_docsexternes affiche les documents pdf, jpg ou video dans une fiche Updialog
 *  Les intitulés des documents sont affichés à gauche dans un QTreeView *wdg_listdocstreewiew
 *  et les documents eux-mêmes sont affichés à droite dans
 *      . un UpTablWidget *wdg_pdftbl pour les pdf,
 *      . un QVideoWidget wdg_video pour les videos
 *      . ou un UpLabel wdg_jpglbl pour les jpg */

    QTreeView               *wdg_listdocstreewiew   = new QTreeView();

    UpCheckBox              *wdg_alldocsupcheckbox  = Q_NULLPTR;
    UpCheckBox              *wdg_onlyimportantsdocsupcheckbox    = Q_NULLPTR;
    UpComboBox              *wdg_updatetypebox      = Q_NULLPTR;

    QStandardItemModel      *m_model                = Q_NULLPTR;
    QStandardItemModel      *m_tripardatemodel      = Q_NULLPTR;
    QStandardItemModel      *m_tripartypemodel      = Q_NULLPTR;
    QPoint                  m_positionorigin;
    QSize                   m_sizeorigin;
    int                     m_treeviewwidth = 180;

    ModeTri                 m_modetri;
    ModeFiltre              m_modefiltre;


    void                    ActualiseDocsExternes();
    void                    AfficheCustomMenu(DocExterne *docmt);
    void                    AfficheDoc(QModelIndex idx);
    void                    BasculeTriListe(dlg_docsexternes::ModeTri mode);
    QMap<QString, QString>  CalcNomFilePdf();

    void                    CorrigeImportance(DocExterne *docmt, enum Importance imptce);

    void                    FiltrerListe(UpCheckBox *chk);
    DocExterne*             getDocumentFromIndex(QModelIndex idx);
    QModelIndex             getIndexFromId(QStandardItemModel *model, int id);
    QStandardItem*          getItemFromDocument(QStandardItemModel *model, DocExterne* docmt);
    void                    ImprimeDoc();

    bool                    ModifieEtReImprimeDoc(DocExterne *docmt, bool modifiable, bool detruirealafin);
    void                    ModifierDate(QModelIndex idx);
    void                    ModifierItem(QModelIndex idx);

    void                    OpenMultiImageViewer(int iddoc = 0);

    void                    RemplirTreeView();
    void                    SupprimeDoc(DocExterne *docmt = Q_NULLPTR);
    void                    ZoomDoc(bool changemode = true);

signals:
    void                    Actualize();
};

#endif // DLG_DOCSEXTERNES_H
