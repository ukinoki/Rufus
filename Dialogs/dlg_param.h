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

#ifndef DLG_PARAM_H
#define DLG_PARAM_H

#include <QToolTip>

#include "dlg_gestionbanques.h"
#include "ui_dlg_gestionbanques.h"
#include "dlg_gestioncotations.h"
#include "dlg_gestionusers.h"
#include "dlg_fontdialog.h"
#include "dlg_listevilles.h"
#include "dlg_motifs.h"

#include "villecpwidget.h"
#include "ui_villecpwidget.h"
#include "widgetbuttonframe.h"
#include "upheaderview.h"
#include "upstandarditemmodel.h"
#include "upstandarditem.h"
#include "cls_cotations.h"

namespace Ui {
class dlg_param;
}

/*!
 Refonte de la gestion des cotations (en cours) — pour que tout arrivant s'y retrouve.

 Cible : une SEULE table affichée, cotationsUpTableView (une UpTableView modèle/vue, modèle
 m_modelCotations), qui montre toutes les cotations du groupe d'utilisateurs + la NGAP. Chaque
 ligne porte sa Cotation (rufusitem d'un UpStandardItem) ; elle est cochée si l'utilisateur courant
 l'utilise (Cotation::isused()).
    * 4 types de cotation (Cotation::typcotation) : 1=CCAM, 2=NGAP, 3=horsNGAPnorCCAM, 4=association CCAM.
    * données : Cotations::loadCotations() charge la map de référence (table cotations) ; puis
      Cotations::loadUserCotations(user) marque used() et pose montant conventionnel/pratiqué en
      puisant les montants pratiqués dans les 4 tables de jointures (jointuresccam,
      jointuresassociations, jointurescotations, jointurescotationsautres), toutes clées sur idcotation.
    * getCotationFromIndex(idx) : rend la Cotation d'une ligne. AfficheToolTip(idx) : son tip.

 Boutons sous la table (wdg_cotationswdgbuttonframe) : Ajouter / Modifier / Supprimer la cotation
 en surbrillance.
    * Ajouter -> une fiche : soit capturer une cotation CCAM depuis la liste des actes CCAM, soit
      créer à la main une cotation d'association CCAM ou hors CCAM/NGAP.
    * supprimeCotation(cot) : retire la cotation de la table de jointure du user (= décocher) ; si
      plus personne ne l'utilise, la cotation disparaît de la table cotations. MAJCotation(cot) :
      mise à jour (à écrire). Les deux reçoivent la Cotation de l'appelant (getCotationFromIndex).
    * NGAP (type 2) : ni supprimer ni modifier (montant pratiqué non modulable) -> boutons grisés.

 En voie de disparition : ActesCCAMupTableWidget et HorsNomenclatureupTableWidget (et leurs boutons
 wdg_HN...) fusionnent dans ce mécanisme unique ; la liste des actes CCAM migrera dans la fiche
 d'ajout. Tant que le ménage n'est pas fini, du code référençant ces tables ne compile pas encore.
*/

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
    bool m_custombasevilles = Datas::I()->villes->iscustomizedbase();

    /*! table des cotations (modèle/vue) : les cotations de la map de référence, cochées si
        l'utilisateur courant les utilise (used) */
    Cotations               *m_cotations             = Q_NULLPTR;    //!< map de référence + used() du user courant
    UpStandardItemModel     *m_modelCotations        = Q_NULLPTR;    //!< modèle de cotationsUpTableView
    void                    remplitTableCotations();                //!< (re)remplit la table des cotations (used cochées)
    Cotation*               getCotationFromIndex(QModelIndex idx);  //!< la cotation portée par la ligne d'index idx

    bool                    m_modifposte            = false;
    bool                    m_alerteMDPencours      = false;   //! garde-fou de ré-entrance de l'alerte « mot de passe de connexion modifié »
    bool                    m_MDPadminverifie       = false;
    bool                    m_MDPuserverifie        = false;
    QString                 m_nouveauMDP            = "nouv";
    QString                 m_ancienMDP             = "anc";
    QString                 m_confirmeMDP           = "confirm";

    QStringList             m_listeappareils;

    QTimer                  t_timerverifimportdocs;

    VilleCPWidget           *wdg_villeCP;
    WidgetButtonFrame       *wdg_cotationswdgbuttonframe, *wdg_appareilswdgbuttonframe;
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
    void                enableCotations(bool enable = true);
    void                Remplir_TableActesCCAM(bool ophtaseul = true);
    void                Remplir_Tables();
    bool                Valide_Modifications();
    void                NouvCotation();
    void                ModifCotation();
    void                supprimeCotation(Cotation *cot);
    void                NouvAppareil();
    void                RecalcAvailablesPorts(bool fromSettings = false);
    void                RestaureBase();
    void                SupprAppareil();
    bool                VerifDirStockageImagerie();
    void                VerifPosteImportDocs();

    void                ConnectTimers(bool = true);

    void                AfficheToolTip(QModelIndex idx);        //!< infobulle de cotationsUpTableView (tip de la Cotation de la ligne)
    void                EnableSupprAppareilBouton();
    void                FermepushButtonClicked();
    void                scrollToCodeCCAM(QString);
    void                ChoixDossierEchangeAppareilImagerie(UpPushButton *butt);
    void                ChoixFontpushButtonClicked();
    void                ClearPortsComboBox(UpComboBox *box, int);
    void                DirLocalStockage();
    void                DirDistantStockage();
    void                PosteVideoDir();
    void                LocalVideoDir();
    void                DistantVideoDir();
    void                DossierClesSSL();
    void                ExporterClesSSLversUSB();        //!< (serveur) copie les clés CLIENT SSL conservées par ce poste vers une clé USB, pour les postes distants
    void                ExporterClesSSLDistantversUSB();  //!< (accès distant) copie vers une clé USB les clés d'accès au serveur distant (dossier Dossier_ClesSSL), pour les déployer sur un autre poste distant
    void                CreerClesSSL();                   //!< (Ce poste) régénère DE NOUVELLES clés SSL serveur (avertissement sévère : invalide les clés distribuées) puis relance Rufus
    void                EnableModif(QWidget *obj);
    void                EnableFrameServeur(QCheckBox *box, bool a);
    void                EnableComOrNetworkWidgetsAppareilRefraction(UpComboBox *combo, QString txtport);
    void                EnableOKModifPosteButton();
    void                EnregistreAppareil();
    void                EnregDossierStockageApp(UpLineEdit *line, QString dir);
    void                EnregistreEmplacementServeur(int);
    void                EnregistreNouvMDPAdmin();
    void                FiltreActesOphtaSeulmt(bool);
    void                GestionBanques();
    void                GestionDatasCurrentUser();
    void                GestionLieux();
    void                GestionUsers();
    void                MAJActesCCAM(QWidget *widg, QString txt = "");
    void                MAJCotation(Cotation *cot);
    void                ModifPathEchangeReglageRefracteur(Procedures::TypeAppareil appareil);           /*! indique l'emplacement réseau du fichier xml émis par Rufus destiné au préréglage du refracteur*/
    void                ModifPathDirEchangeMesure(Procedures::TypeAppareil appareil);                   /*! indique l'emplacement réseau du fichier d'échange d'un appareil de refraction*/
    void                ModifMDPAdmin();
    void                ParamMotifs();
    void                ConfigureChampMDPMySQL(UpLineEdit *champ, UpSmallButton *btnUSB);                    //! ajoute l'œil (afficher/masquer) et relie le bouton d'enregistrement du mdp sur clé USB
    void                ReglePortCOM_dlg(Procedures::TypeAppareil appareil);                                //! règle les aparamètres du port COM auquel est connecté l'appareil
    QString             ToolTipPortCOM(Procedures::TypeAppareil appareil);
    void                RegleAssocBoutons();
    void                ResetImprimante();
    void                ModifBDDVilles(Villes::TownsFrom from);

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
    void                fixAMnumberforSite(UpPushButton* AMnumberButton, QMap<Site*,qlonglong> mapsites);
};
#endif // DLG_PARAM_H
