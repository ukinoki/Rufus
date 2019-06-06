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
    explicit dlg_param(int idUser, QWidget *parent = Q_NULLPTR);
    ~dlg_param();
    Ui::dlg_param       *ui;
    bool                CotationsModifiees();
    bool                DataUserModifiees();
    QLineEdit           *CPDefautlineEdit, *VilleDefautlineEdit;

private:
    dlg_gestionbanques      *Dlg_Banq;
    dlg_gestioncotations    *Dlg_CrrCot;
    dlg_gestionusers        *Dlg_GestUsr;
    dlg_fontdialog          *Dlg_Fonts;
    dlg_motifs              *Dlg_motifs;
    VilleCPWidget           *VilleCPDefautWidg;
    UpDialog                *gLieuxDialog;
    int                     gidUser;
    bool                    gModifPoste;
    bool                    gCotationsModifiees;
    bool                    DonneesUserModifiees;
    bool                    MDPVerifiedAdmin, MDPVerifiedUser;
    Procedures              *proc;
    UpDialog                *gAskAppareil, *gAskMDP;
    User                    *gDataUser;
    ParametresSysteme       *m_parametres;
    DataBase                *db;
    QString                 gNouvMDP, gAncMDP, gConfirmMDP;
    QStringList             glistAppareils;
    QTimer                  gTimerVerifPosteImportDocs, gTimerVerifTCP;
    WidgetButtonFrame       *widgHN, *widgAssocCCAM, *widgAppareils;
    QWidget*                widgCCAM;

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
    void                VerifTCP();

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

signals:
    void                click(QWidget *obj);

    /*LA SAUVEGARDE DE LA BASE DE DONNEES

      La sauvegarde de la BDD peut-être planifiée dans le Qframe ui->Sauvegardeframe qui est en haut de l'onglet ui->GeneralParamtab.
      On peut planifier l'emplacement du fichier de sauvegarde, l'heure de la sauvegarde, et les jours de la sauvegarde.
      La sauvegarde ne peut se programmer que sur le serveur et pas ailleurs. Il faut donc installer une instance de Rufus sur le serveur même si elle ne sert qu'à ça.
      Les éléments du cadre ui->Sauvegardeframe sont donc désactivés si on n'est pas en mode Poste, autrement dit, sur le serveur.

      La sauvegarde se fait par un script qui lance le prg mysqldump de sauvegarde des données.
      Ce script définit l'emplacement de la sauvegarde, le nom de la sauvegarde et détruit les sauvegardes datant de plus de 14 jours
      . C'est le script RufusBackupScript.sh situé dans /Users/nomdutlisateur/Documents/Rufus
      Sous MacOS, la programmation de l'éxécution de ce script se fait par un fichier xml qui va déterminer les jours de la semaine et l'heure de la sauvegarde.
      . c'est le fichier xml rufus.bup.plist situé dans /Users/nomutilisateur/Library/LaunchAgents.
      . Ce fichier est chargé au démarrage de la machine par le launchd Apple.
      . Il est donc éxécuté même quand Rufus ne tourne pas
      Sous Linux, c'est un système de timer qui lance la sauvegarde et le programme dooit donc tourner pour que la sauvegare se fasse (pas trouvé le moyen de modifier la crontab depuis Qt - pas trop cherché non plus)

      Au chargement de la classe dlg_param, les données de programmation sont récupérées à partir de la table ParametresSysteme pour régler l'affichage des données dans  ui->Sauvegardeframe.

      Une modification de l'emplacement de sauvegarde se fait par un clic sur le bouton ui->DirBackuppushButton qui va lancer le slot Slot_ModifDirBachup()
      Un changement d'heure ou de jour lance le slot Slot_ModifScriptList().
      Le bouton ui->EffacePrgSauvupPushButton réinitialise la programmation en déclenchant le slot Slot_EffacePrgSauvegarde()
          Ce slot réinitialise l'affichage dans ui->Sauvegardeframe,
            et appelle la fonction Procedures::EffaceAutoBackup() qui va`
                supprimer le script de sauvegarde RufusBackupScript.sh
                sur Mac, supprimer le script de programmation rufus.bup.plist et le décharger du launchd
                sur Linux, stoppe le timer de sauvegarde
      En cas de modification des parametres de sauvegarde, si ces paramètres sont complets, la fonction ModifParamAutoBackup():
            * vérifie que la paramètrage de la sauvegarde est complet
            * appelle la fonction Procedures::ParamAutoBackup() qui va
                * créer le fichier RufusScriptBackup.sh (fonction Procedures::DefinitScriptBackup()
                * sur Mac, modifier le fichier xml rufus.bup.plist et recharger ce fichier.
                * sur Linux, lance le timer de sauvegarde
      Une sauvegarde immédiate est effectuée par un clic sur le bouton ui->ImmediatBackupupPushButton qui lance la fonction Slot_ImmediateBackup()
      Après vérification de l'absence d'autres utilisateurs connectés, cette fonction lance la fonction Procedures::ImmediateBackup()
      Cette fonction redéfinit un script de sauvegarde temporaire après une boîte de dialogue de sélection des éléments à sauvegarder
      puis elle rétablit le script original s'il y en avait un.
     */
private slots:
    void                    Slot_ModifDirBackup();
    void                    Slot_ModifDateBackup();
    void                    Slot_EffacePrgSauvegarde();
private:
    void                    ModifParamAutoBackup();
    bool                    gModifBackup;
};
#endif // DLG_PARAM_H
