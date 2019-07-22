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

#ifndef DLG_DOCUMENTS_H
#define DLG_DOCUMENTS_H

#include "procedures.h"
#include "upcheckbox.h"
#include "upcombobox.h"
#include "updialog.h"
#include "uplineedit.h"
#include "uptablewidget.h"
#include <QGraphicsOpacityEffect>
#include "widgetbuttonframe.h"
#include "gbl_datas.h"
#include "cls_user.h"
#include "cls_site.h"
#include "icons.h"

namespace Ui {
class dlg_documents;
}

class dlg_documents : public QDialog
{
    Q_OBJECT

public:
    explicit                    dlg_documents(Patient *pat, QWidget *parent = Q_NULLPTR);
    ~dlg_documents();
    Ui::dlg_documents           *ui;
    QString                     gReponseResumeOrdo, gReponseResumePrescription;
    QStringList                 TextDocumentsAImprimerList, TitreDocumentAImprimerList, PrescriptionAImprimerList, DupliAImprimerList, AdministratifAImprimerList;
    QStringList                 glistid;
    User*                       getUserEntete();

protected:
    void                        changeEvent(QEvent *e);
    bool                        event(QEvent *event);

private:
    DataBase                    *db             = DataBase::I();
    User                        *m_currentuser   = Datas::I()->users->userconnected();
    Procedures                  *proc           = Procedures::I();
    Patient                     *m_currentpatient;
    User                        *gUserEntete;
    bool                        ok;
    void                        closeEvent      (QCloseEvent *event);
    bool                        eventFilter     (QObject *obj, QEvent *event);
    void                        keyPressEvent   (QKeyEvent * event );
    WidgetButtonFrame           *widgButtonsDocs, *widgButtonsDossiers;
    double                      gOpacity;
    int                         gMode;
    enum gMode                  {Selection,CreationDOC,ModificationDOC,CreationDOSS,ModificationDOSS,SuppressionDOSS};
    QGraphicsOpacityEffect      *gOp;
    QList<Correspondant*>       m_listedestinataires;
    QMap<QString,QString>       gChampsMap;
    QStringList                 glisttxt;
    UpDialog                    *gAsk;
    UpDialog                    *gAskDialog;
    UpDialog                    *gAskCorresp;
    QMenu                       *gmenuContextuel;
    QTimer                      *gTimerEfface;


    void                        Annulation();
    void                        ChoixButtonFrame(int, WidgetButtonFrame *);
    void                        ChoixMenuContextuel(QString);
    int                         AskDialog(QString titre);
    void                        CheckPublicEditablAdmin(QCheckBox *check);
    bool                        ChercheDoublon(QString, int row);
    void                        ChoixCorrespondant(QList<Correspondant*> listcor);
    void                        CocheLesDocs(int iddoss, bool A);
    void                        ConfigMode(int mode, int row = 0);
    void                        dblClicktextEdit();
    void                        DisableLines();
    void                        DocCellDblClick(UpLineEdit *line);
    void                        DocCellEnter(UpLineEdit *line);
    void                        EffaceWidget(QWidget* widg, bool AvecOuSansPause = true);
    void                        EnableLines();
    void                        EnableOKPushButton(UpCheckBox *Check = Q_NULLPTR);
    void                        FiltreListe();
    Document*                   getDocumentFromRow(int row);
    MetaDocument*               getMetaDocumentFromRow(int row);
    void                        InsertDocument(int row);
    void                        InsertDossier(int row);
    void                        LineSelect(UpTableWidget *table, int row);
    void                        ListidCor();
    void                        MenuContextuel(QWidget *widg);
    void                        MetAJour(QString texte, bool pourVisu = true);
    void                        OrdoAvecDupli(bool);
    void                        Remplir_TableWidget();
    void                        SetDocumentToRow(Document *doc, int row);
    void                        SetMetaDocumentToRow(MetaDocument *dossier, int row);
    void                        SupprimmDocument(int row);
    void                        SupprimmDossier(int row);
    void                        TriDocupTableWidget();
    void                        TriDossiersupTableWidget();
    void                        UpdateDocument(int row);
    void                        UpdateDossier(int row);
    void                        Validation();
    void                        VerifCoherencegAsk();
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
