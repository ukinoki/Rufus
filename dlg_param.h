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

#ifndef DLG_PARAM_H
#define DLG_PARAM_H

#include <QToolTip>

#include "uplineedit.h"
#include "dlg_banque.h"
#include "ui_dlg_banque.h"
#include "dlg_creeracte.h"
#include "dlg_gestionusers.h"
#include "dlg_fontdialog.h"
#include "dlg_gestionlieux.h"
#include "dlg_motifs.h"
#include "upqsqltablemodel.h"
#include "upradiobutton.h"
#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "widgetbuttonframe.h"

namespace Ui {
class dlg_param;
}

class dlg_param : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_param(int idUser, Procedures *procAPasser, QWidget *parent = Q_NULLPTR);
    ~dlg_param();
    Ui::dlg_param       *ui;
    bool                CotationsModifiees();
    bool                DataUserModifiees();
    QLineEdit           *CPDefautlineEdit, *VilleDefautlineEdit;

private:
    dlg_banque              *Dlg_Banq;
    dlg_creeracte           *Dlg_CrrAct;
    dlg_gestionusers        *Dlg_GestUsr;
    dlg_fontdialog          *Dlg_Fonts;
    dlg_motifs              *Dlg_motifs;
    VilleCPWidget           *VilleCPDefautWidg;
    UpDialog                *gLieuxDialog;
    int                     gidUser;
    bool                    gModifPoste;
    bool                    gCotationsModifiees;
    bool                    DonneesUserModifiees;
    Procedures              *proc;
    UpDialog                *gAskAppareil, *gAskMDP;
    QMap<QString,QVariant>  gDataUser;
    QSqlDatabase            db;
    QString                 gNouvMDP, gAncMDP, gConfirmMDP;
    QStringList             glistAppareils;
    QTimer                  *gTimerVerifPosteImportDocs;
    UpQSqlTableModel        *gActestbl;
    WidgetButtonFrame       *widgHN, *widgAssocCCAM, *widgAppareils;
    QWidget*                widgCCAM;

    bool                eventFilter(QObject *obj, QEvent *event)  ;
    void                ConnectSlots();
    void                AfficheParamUser();
    void                AskAppareil();
    void                EnableWidgContent(QWidget *widg, bool = true);
    void                ModifScriptBackup();
    void                ReconstruitListeLieuxExercice();
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


private slots:
    void                Slot_AfficheToolTip(QTableWidgetItem*);
    void                Slot_EnableAppBoutons();
    void                Slot_FermepushButtonClicked();
    void                Slot_ChercheCCAM(QString);
    void                Slot_ChoixDossierStockageApp();
    void                Slot_ChoixButtonFrame(int);
    void                Slot_ConnectTimers(bool = true);
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
    void                Slot_FileDialog();
    void                Slot_FiltreActesOphtaSeulmt(bool);
    void                Slot_GestionBanques();
    void                Slot_GestDataPersoUser();
    void                Slot_GestLieux();
    void                Slot_GestUser();
    void                Slot_ImmediateBackup();
    void                Slot_MAJActesCCAM(QString txt = "");
    void                Slot_MAJAssocCCAM(QString txt = "");
    void                Slot_MAJHorsNomenclature(QString txt = "");
    void                Slot_ModifMDPAdmin();
    void                Slot_ParamMotifs();
    void                Slot_RegleAssocBoutons();
    void                Slot_ResetImprimante();
    void                Slot_EnregistreAppareil();
    void                Slot_EnregistreEmplacementServeur(int);

    void                Slot_RestaureBase();
    void                Slot_ReinitBase();
    void                Slot_VerifPosteImportDocs();

signals:
    void                click(QWidget *obj);

    /*LA SAUVEGARDE DE LA BASE DE DONNEES

      La sauvegarde de la BDD peut-être planifiée dans le Qframe ui->Sauvegardeframe qui est en haut de l'onglet ui->GeneralParamtab.
      On peut planifier l'emplacement du fichier de sauvegarde, l'heure de la sauvegarde, et les jours de la sauvegarde.
      La sauvegarde ne peut se programmer que sur le serveur et pas ailleurs. Il faut donc installer une instance de Rufus sur le serveur même si elle ne sert qu'à ça.
      Les éléments du cadre ui->Sauvegardeframe sont donc désactivés si on n'est pas en mode Poste, autrement dit, sur le serveur.

      Les paramètres de programmation de la sauvegarde sont sauvegardés dans
      . Rufus.ini qui ne sert en l'occurence qu'à gérer l'affichage des paramètres de sauvegarde

      La sauvegarde se fait par un script qui lance le prg mysqldump de sauvegarde des données.
      Ce script définit l'emplacement de la sauvegarde, le nom de la sauvegarde et détruit les sauvegardes datant de plus de 14 jours
      . pour Mac c'est le script RufusBackupScript.sh situé dans /Users/nomdutlisateur/Documents/Rufus
      La programmation de la sauvegarde se fait par un autre script qui va déterminer les jours de la semaine et l'heure de la sauvegarde.
      . Pour Mac, c'est le fichier xml rufus.bup.plist situé dans /Users/nomutilisateur/Library/LaunchAgents. Ce fichier est chargé au démarrage par le launchd Apple.

      Au chargement de la classe dlg_param, les données de Rufus.ini sont récupérées pour régler l'affichage des données dans  ui->Sauvegardeframe.

      Une modification de l'emplacement de sauvegarde se fait par un clic sur le bouton ui->DirBackuppushButton qui va lancer le slot Slot_ModifDirBachup()
      Un changement d'heure ou de jour lance le slot Slot_ModifScriptList().
      Le bouton ui->EffacePrgSauvupPushButton réinitialise la programmation en déclenchant le slot Slot_EffacePrgSauvegarde()
          Ce slot annule les données de programmation dans rufus.ini,`
            réinitialise l'affichage dans ui->Sauvegardeframe,`
            supprime le script de sauvegarde RufusBackupScript.sh
            et le script de programmation rufus.bup.plist
            et, sur Mac, décharge ce fichier du launchd
      En cas de modification des parametres de sauvegarde, si ces paramètres sont complets, la fonction ModifParamBackup():
            * vérifie que la paramètrage de la sauvegarde est complet
            * va créer le fichier RufusScriptBackup.sh (fonction proc->DefinitScriptBackup(NomDirStockageImagerie)) et enregistrer l'emplacement de sauvegarde dans rufus.ini
            * va  modifier le fichier xml rufus.bup.plist, recharger ce fichier dans le launchd et enregistrer les données de programmation dans le rufus.ini.
     */
private slots:
    void                    Slot_ModifDirBackup();
    void                    Slot_ModifDateBackup();
    void                    Slot_EffacePrgSauvegarde();
private:
    void                    ModifParamBackup();
    bool                    gModifBackup;
};
#endif // DLG_PARAM_H
