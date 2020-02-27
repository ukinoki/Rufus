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

#ifndef DLG_PARAM_H
#define DLG_PARAM_H

#include <QToolTip>

#include "dlg_gestionbanques.h"
#include "ui_dlg_gestionbanques.h"
#include "dlg_gestioncotations.h"
#include "dlg_gestionusers.h"
#include "dlg_fontdialog.h"
#include "dlg_motifs.h"
#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "widgetbuttonframe.h"
#include "upheaderview.h"

namespace Ui {
class dlg_param;
}

class dlg_param : public QDialog
{
    Q_OBJECT
public:
    explicit dlg_param(QWidget *parent = Q_NULLPTR);
    ~dlg_param();
    Ui::dlg_param       *ui;
    bool                CotationsModifiees() const;
    bool                DataUserModifiees() const;
    QLineEdit           *wdg_CPDefautlineEdit, *wdg_VilleDefautlineEdit;

private:
    Procedures              *proc                   = Procedures::I();
    DataBase                *db                     = DataBase::I();
    bool                    m_cotationsmodifiees    = false;
    bool                    m_donneesusermodifiees  = false;
    User*                   currentuser() { return Datas::I()->users->userconnected(); }
    ParametresSysteme       *m_parametres           = db->parametres();

    bool                    m_modifposte            = false;
    bool                    m_MDPadminverifie       = false;
    bool                    m_MDPuserverifie        = false;
    QString                 m_nouveauMDP            = "nouv";
    QString                 m_ancienMDP             = "anc";
    QString                 m_confirmeMDP           = "confirm";

    dlg_gestionbanques      *Dlg_Banq;
    dlg_gestioncotations    *Dlg_CrrCot;
    dlg_gestionusers        *Dlg_GestUsr;
    dlg_fontdialog          *Dlg_Fonts;
    dlg_motifs              *Dlg_motifs;

    QStringList             m_listeappareils;

    QTimer                  t_timerverifimportdocs;

    VilleCPWidget           *wdg_villeCP;
    WidgetButtonFrame       *wdg_HNcotationswdgbuttonframe, *wdg_assocCCAMcotationswdgbuttonframe, *wdg_appareilswdgbuttonframe;
    QWidget                 *wdg_CCAM;
    UpDialog                *dlg_lieux;
    UpDialog                *dlg_askappareil, *dlg_askMDP;

    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                ChoixButtonFrame(WidgetButtonFrame *);
    void                ConnectSignals();
    void                AfficheParamUser();
    void                EnableWidgContent(QWidget *widg, bool = true);
    void                ModifScriptBackup();
    void                ReconstruitListeLieuxExerciceUser(User *user);
    void                ReconstruitListeLieuxExerciceAllusers();
    void                EnableActesCCAM(bool enable = true);
    void                EnableAssocCCAM(bool enable = true);
    void                EnableHorsNomenclature(bool enable = true);
    void                Remplir_TableActesCCAM(bool ophtaseul = true);
    void                Remplir_TableAssocCCAM();
    void                Remplir_TableHorsNomenclature();
    void                Remplir_Tables();
    bool                Valide_Modifications();
    void                NouvHorsNomenclature();
    void                ModifHorsNomenclature();
    void                SupprHorsNomenclature();
    void                NouvAssocCCAM();
    void                ModifAssocCCAM();
    void                SupprAssocCCAM();
    void                NouvAppareil();
    void                RestaureBase();
    void                SupprAppareil();
    bool                VerifDirStockageImagerie();
    void                VerifPosteImportDocs();

    void                ConnectTimers(bool = true);

    void                AfficheToolTip(QTableWidget* table, QTableWidgetItem* item);
    void                EnableSupprAppareilBouton();
    void                FermepushButtonClicked();
    void                ChercheCodeCCAM(QString);
    void                ChoixDossierStockageApp(UpPushButton *butt);
    void                EnregDossierStockageApp(UpLineEdit *line, QString dir);
    void                ChoixFontpushButtonClicked();
    void                ClearCom(UpComboBox *box, int);
    void                DirLocalStockage();
    void                DirDistantStockage();
    void                DirPosteStockage();
    void                DossierClesSSL();
    void                EnableModif(QWidget *obj);
    void                EnableFrameServeur(QCheckBox *box, bool a);
    void                EnableOKModifPosteButton();
    void                EnregistreAppareil();
    void                EnregistreEmplacementServeur(int);
    void                EnregistreNouvMDPAdmin();
    void                FiltreActesOphtaSeulmt(bool);
    void                GestionBanques();
    void                GestionDatasCurrentUser();
    void                GestionLieux();
    void                GestionUsers();
    void                MAJActesCCAM(QWidget *widg, QString txt = "");
    void                MAJAssocCCAM(QWidget *widg, QString txt = "");
    void                MAJHorsNomenclature(QWidget *widg, QString txt = "");
    void                ModifMDPAdmin();
    void                ParamMotifs();
    void                RegleAssocBoutons(QWidget *widg);
    void                ResetImprimante();

signals:
    void                click(QWidget *obj);

    /*! LA SAUVEGARDE DE LA BASE DE DONNEES
      le fonctionnement interne de la sauvegarde est expliqué dans procedures.h
     */
private:
    void                EffaceProgrammationDataBackup();
                        /*! efface les paramètres et la programmation de la sauvegarde automatique et réinitialise l'affichage */
    void                ModifHeureBackup();
                        /*! modifie l'heure du backup automatique et relance ParamAutoBackup() */
    void                ModifDateBackup();
                        /*! modifie la date du backup automatique et relance ParamAutoBackup() */
    void                ModifDirBackup();
                        /*! modifie le dossier de destination du backup automatique et relance ParamAutoBackup() */
    void                startImmediateBackup();

};
#endif // DLG_PARAM_H
