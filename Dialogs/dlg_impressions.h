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

#ifndef DLG_IMPRESSIONS_H
#define DLG_IMPRESSIONS_H

#include "procedures.h"
#include "uptablewidget.h"
#include <QGraphicsOpacityEffect>
#include "gbl_datas.h"
#include "icons.h"

namespace Ui {
class dlg_impressions;
}

class dlg_impressions : public QDialog
{
    Q_OBJECT
public:
    explicit                                    dlg_impressions(Patient *pat, QWidget *parent = Q_NULLPTR);
    ~dlg_impressions();
    Ui::dlg_impressions                         *ui;
    enum DATASAIMPRIMER                         {Texte, Titre, Prescription, Dupli, Administratif};                                     Q_ENUM(DATASAIMPRIMER)
    enum Mode                                   {Selection,CreationDOC,ModificationDOC,CreationDOSS,ModificationDOSS,SuppressionDOSS};  Q_ENUM(Mode)
    User*                                       userentete() const;
    QMap<int, QMap<DATASAIMPRIMER, QString> >   mapdocsaimprimer() const;

private:
    DataBase                                    *db             = DataBase::I();
    User*                                       currentuser() { return Datas::I()->users->userconnected(); }
    Procedures                                  *proc           = Procedures::I();
    Patient                                     *m_currentpatient;
    User                                        *m_userentete;
    QList<Correspondant*>                       m_listedestinataires;
    QMap<int, QMap<DATASAIMPRIMER, QString> >   map_docsaimprimer;

    Mode                        m_mode;
    double                      m_opacity;
    QMap<QString,QString>       map_champs;
    QStringList                 m_listid;
    QStringList                 m_listtexts;   /*! pour un document donné, il peut y avoir plusieurs impressions s'il y a plusieurs destinataires
                                                -> le texte varie légèrement d'un destinataire à l'autre en ce qui concerne le nom du correspondant
                                                -> la variable correspond à la liste des textes */
    QGraphicsOpacityEffect      *m_opacityeffect;
    QMenu                       *m_menucontextuel;
    QTimer                      *t_timerefface;

    UpDialog                    *dlg_ask;
    UpDialog                    *dlg_askdialog;
    UpDialog                    *dlg_askcorrespondant;
    WidgetButtonFrame           *wdg_docsbuttonframe, *wdg_dossiersbuttonframe;


    bool                        event(QEvent *event);
    void                        closeEvent      (QCloseEvent *event);
    bool                        eventFilter     (QObject *obj, QEvent *event);
    void                        keyPressEvent   (QKeyEvent * event );
    void                        Annulation();
    void                        ChoixButtonFrame(WidgetButtonFrame *);
    void                        ChoixMenuContextuel(QString);
    int                         AskDialog(QString titre);
    void                        CheckPublicEditablAdmin(QCheckBox *check);
    bool                        ChercheDoublon(QString, int row);
    void                        ChoixCorrespondant(QList<Correspondant*> listcor);
    void                        CocheLesDocs(int iddoss, bool A);
    void                        ConfigMode(Mode mode, int row = 0);
    void                        dblClicktextEdit();
    void                        DisableLines();
    void                        DocCellDblClick(UpLineEdit *line);
    void                        DocCellEnter(UpLineEdit *line);
    void                        EffaceWidget(QWidget* widg, bool AvecOuSansPause = true);
    void                        EnableLines();
    void                        EnableOKPushButton(UpCheckBox *Check = Q_NULLPTR);
    void                        FiltreListe();
    Impression*                 getDocumentFromRow(int row);
    DossierImpression*          getMetaDocumentFromRow(int row);
    void                        InsertDocument(int row);
    void                        InsertDossier(int row);
    void                        LineSelect(UpTableWidget *table, int row);
    void                        ListidCor();
    void                        MenuContextuel(QWidget *widg);
    void                        MetAJour(QString texte, bool pourVisu = true);
    void                        OrdoAvecDupli(bool);
    void                        Remplir_TableWidget();
    void                        SetDocumentToRow(Impression *doc, int row);
    void                        SetMetaDocumentToRow(DossierImpression *dossier, int row);
    void                        SupprimmDocument(int row);
    void                        SupprimmDossier(int row);
    void                        TriDocupTableWidget();
    void                        TriDossiersupTableWidget();
    void                        UpdateDocument(int row);
    void                        UpdateDossier(int row);
    void                        Validation();
    void                        VerifCoherencedlg_ask();
    bool                        VerifDocumentPublic(int row, bool msg = true);
    void                        VerifDossiers();
    bool                        VerifDossierPublic(int row, bool msg = true);

private:
    QString TITRUSER            = tr("TITREUSER");
    QString NOMPAT              = tr("NOM PATIENT");
    QString DATEDOC             = tr("DATE");
    QString DDNPAT              = tr("DDN");
    QString TITREPAT            = tr("TITRE PATIENT");
    QString AGEPAT              = tr("AGE PATIENT");
    QString PRENOMPAT           = tr("PRENOM PATIENT");
    QString SEXEPAT             = tr("SEXE PATIENT");
    QString MGPAT               = tr("MEDECIN PATIENT");
    QString MGPATTITRE          = tr("TITRE MEDECIN PATIENT");
    QString POLITESSEMG         = tr("FORMULE POLITESSE MEDECIN");
    QString PRENOMMG            = tr("PRENOM MEDECIN");
    QString NOMMG               = tr("NOM MEDECIN");
    QString REFRACT             = tr("REFRACTION");
    QString KERATO              = tr("KERATOMETRIE");
    QString POLITESSECOR        = tr("FORMULE POLITESSE CORRESPONDANT");
    QString CORPAT              = tr("CORRESPONDANT PATIENT");
    QString PRENOMCOR           = tr("PRENOM CORRESPONDANT");
    QString NOMCOR              = tr("NOM CORRESPONDANT");
    QString PRENOMUSER          = tr("PRENOM RESPONSABLE");
    QString NOMUSER             = tr("NOM RESPONSABLE");
    QString TELEPHONE           = tr("TELEPHONE PATIENT");

    QString TYPEANESTHESIE      = tr("TYPEANESTHESIE");
    QString PROVENANCE          = tr("PROVENANCE");
    QString TYPESEJOUR          = tr("SEJOUR");
    QString COTE                = tr("COTE");
    QString SITE                = tr("SITE");

    QString NOCOR               = tr("PAS DE CORRESPONDANT RÉFÉRENCÉ POUR CE PATIENT");

};

#endif // DLG_DOCUMENTS_H
