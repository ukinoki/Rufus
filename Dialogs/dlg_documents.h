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

namespace Ui {
class dlg_documents;
}

class dlg_documents : public QDialog
{
    Q_OBJECT

public:
    explicit                    dlg_documents(int idPatAPasser, QString NomPatient, QString PrenomPatient,
                                              Procedures *procAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_documents();
    Ui::dlg_documents           *ui;
    QString                     gReponseResumeOrdo, gReponseResumePrescription;
    QStringList                 TextDocumentsAImprimerList, TitreDocumentAImprimerList, PrescriptionAImprimerList, DupliAImprimerList, AdministratifAImprimerList;
    QStringList                 glistid;
    int                         gidUserEntete;

protected:
    void                        changeEvent(QEvent *e);
    bool                        event(QEvent *event);

private:
    void                        closeEvent      (QCloseEvent *event);
    bool                        eventFilter     (QObject *obj, QEvent *event);
    void                        keyPressEvent   (QKeyEvent * event );
    WidgetButtonFrame           *widgButtonsDocs, *widgButtonsDossiers;
    double                      gOpacity;
    int                         gMode;
    enum gMode                  {Selection,CreationDOC,ModificationDOC,CreationDOSS,ModificationDOSS,SuppressionDOSS};
    int                         gidUser, gidPatient, gidUserSuperviseur;
    Procedures                  *proc;
    QGraphicsOpacityEffect      *gOp;
    QList<int>                  glistidCor;
    QMap<QString,QString>       gChampsMap;
    QStringList                 glisttxt;
    UpDialog                    *gAsk;
    UpDialog                    *gAskDialog;
    UpDialog                    *gAskCorresp;
    QMenu                       *gmenuContextuel;
    QSqlDatabase                db;
    QString                     gNomPat, gPrenomPat;
    QTimer                      *gTimerEfface;


    void                        Annulation();
    void                        ChoixButtonFrame(int, WidgetButtonFrame *);
    void                        ChoixMenuContextuel(QString);
    int                         AskDialog(QString titre);
    void                        CheckPublicEditablAdmin(QCheckBox *check);
    bool                        ChercheDoublon(QString, int row);
    void                        ChoixCorrespondant(QSqlQuery);
    void                        CocheLesDocs(int iddoss, bool A);
    void                        ConfigMode(int mode, int row = 0);
    void                        dblClicktextEdit();
    void                        DisableLines();
    void                        DocCellDblClick(UpLineEdit *line);
    void                        DocCellEnter(UpLineEdit *line);
    void                        EffaceWidget(QWidget* widg, bool AvecOuSansPause = true);
    void                        EnableLines();
    void                        EnableOKPushButton(UpCheckBox *Check = Q_NULLPTR);
    void                        FiltreListe(QString);
    void                        InsertDocument(int row);
    void                        InsertDossier(int row);
    void                        LineSelect(UpTableWidget *table, int row);
    void                        ListidCor();
    void                        MenuContextuel(QWidget *widg);
    void                        MetAJour(QString texte, bool pourVisu = true);
    void                        OrdoAvecDupli(bool);
    void                        Remplir_TableWidget();
    void                        SupprimmDocument(int row);
    void                        SupprimmDossier(int row);
    void                        TriDocupTableWidget();
    void                        UpdateDocument(int row);
    void                        UpdateDossier(int row);
    void                        Validation();
    void                        VerifCoherencegAsk();
    bool                        VerifDocumentPublic(int row, bool msg = true);
    void                        VerifDossiers();
    bool                        VerifDossierPublic(int row, bool msg = true);

private:
    QString TITRUSER            =tr("TITREUSER");
    QString NOMPAT              =tr("NOM PATIENT");
    QString DATEDOC             =tr("DATE");
    QString DDNPAT              =tr("DDN");
    QString TITREPAT            =tr("TITRE PATIENT");
    QString AGEPAT              =tr("AGE PATIENT");
    QString PRENOMPAT           =tr("PRENOM PATIENT");
    QString MGPAT               =tr("MEDECIN PATIENT");
    QString POLITESSEMG         =tr("FORMULE POLITESSE MEDECIN");
    QString PRENOMMG            =tr("PRENOM MEDECIN");
    QString NOMMG               =tr("NOM MEDECIN");
    QString REFRACT             =tr("REFRACTION");
    QString KERATO              =tr("KERATOMETRIE");
    QString POLITESSECOR        =tr("FORMULE POLITESSE CORRESPONDANT");
    QString CORPAT              =tr("CORRESPONDANT PATIENT");
    QString PRENOMCOR           =tr("PRENOM CORRESPONDANT");
    QString NOMCOR              =tr("NOM CORRESPONDANT");

};

#endif // DLG_DOCUMENTS_H