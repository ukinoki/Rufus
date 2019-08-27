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

#include "uplineedit.h"
#include "dlg_gestionbanques.h"
#include "ui_dlg_gestionbanques.h"
#include "dlg_gestioncotations.h"
#include "dlg_gestionusers.h"
#include "dlg_fontdialog.h"
#include "dlg_gestionlieux.h"
#include "dlg_motifs.h"
#include "upradiobutton.h"
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
    Procedures              *proc;
    DataBase                *db;

    dlg_gestionbanques      *Dlg_Banq;
    dlg_gestioncotations    *Dlg_CrrCot;
    dlg_gestionusers        *Dlg_GestUsr;
    dlg_fontdialog          *Dlg_Fonts;
    dlg_motifs              *Dlg_motifs;

    User                    *m_currentuser;
    ParametresSysteme       *m_parametres;

    bool                    m_modifposte;
    bool                    m_cotationsmodifiees;
    bool                    m_donneesusermodifiees;
    bool                    m_MDPadminverifie, m_MDPuserverifie;
    QString                 m_nouveauMDP, m_ancienMDP, m_confirmeMDP;
    QStringList             m_listeappareils;

    QTimer                  t_timerverifimportdocs;

    VilleCPWidget           *wdg_villeCP;
    WidgetButtonFrame       *wdg_HNcotationswdgbuttonframe, *wdg_assocCCAMcotationswdgbuttonframe, *wdg_appareilswdgbuttonframe;
    QWidget                 *wdg_CCAM;
    UpDialog                *dlg_lieux;
    UpDialog                *dlg_askappareil, *dlg_askMDP;

    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                ConnectSlots();
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
    void                SupprAppareil();
    bool                VerifDirStockageImagerie();
    void                VerifPosteImportDocs();

    void                ConnectTimers(bool = true);

private slots:
    void                Slot_AfficheToolTip(QTableWidgetItem*);
    void                Slot_EnableAppBoutons();
    void                Slot_FermepushButtonClicked();
    void                Slot_ChercheCCAM(QString);
    void                Slot_ChoixDossierStockageApp();
    void                Slot_ChoixButtonFrame(int);
    void                Slot_EnregDossierStockageApp(QString dir);
    void                Slot_ChoixFontpushButtonClicked();
    void                Slot_ClearCom(int);
    void                Slot_DirLocalStockage();
    void                Slot_DirDistantStockage();
    void                Slot_DirPosteStockage();
    void                Slot_EnableModif(QWidget *obj);
    void                Slot_EnableFrameServeur(bool a);
    void                Slot_EnableOKModifPosteButton();
    void                Slot_EnregistreNouvMDPAdmin();
    void                Slot_FiltreActesOphtaSeulmt(bool);
    void                Slot_GestionBanques();
    void                Slot_GestionDatasCurrentUser();
    void                Slot_GestLieux();
    void                Slot_GestionUsers();
    void                Slot_MAJActesCCAM(QString txt = "");
    void                Slot_MAJAssocCCAM(QString txt = "");
    void                Slot_MAJHorsNomenclature(QString txt = "");
    void                Slot_ModifMDPAdmin();
    void                Slot_ParamMotifs();
    void                Slot_RegleAssocBoutons();
    void                Slot_ResetImprimante();
    void                Slot_EnregistreAppareil();
    void                Slot_EnregistreEmplacementServeur(int);

    void                RestaureBase();

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
