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

#ifndef DLG_GESTIONCOTATIONS_H
#define DLG_GESTIONCOTATIONS_H

#include <QGroupBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QStandardItemModel>
#include "procedures.h"

/*!
 Fiche de création/modification d'une cotation (table cotations, une seule).

 EN CRÉATION, un QGroupBox « Nouvelle cotation » propose 3 modes EXCLUSIFS (Cotation::typcotation) :
    * 1 = CCAM         : on capture un acte de la table CCAM. code + montants NON éditables (repris
                         de la table ccam), libellé repris (non éditable) ; le pratiqué est éditable
                         et part du conventionnel (optam si isOPTAM, sinon nonoptam). Un QCompleter
                         CCAM contraint le code à un acte CCAM existant. Un bouton ouvre la table CCAM.
    * 2 = association  : DEUX codes CCAM (chacun avec QCompleter CCAM, seuls des codes CCAM acceptés) ;
                         les montants sont CALCULÉS (non éditables).
    * 4 = autre        : tout éditable, pas de CCAM, pas de completer.
 EN MODIFICATION, pas de choix : le type est celui de la cotation (2 ou 4 — CCAM/NGAP ne se modifient
 pas), le sélecteur est masqué et les champs sont pré-remplis.

 Les champs sont UNIQUES et montrés/masqués/retitrés selon le mode par appliqueMode() :
    * codeLine (+ codeLine2 en association), 3 montants (optam/conventionnel, nonoptam, pratiqué),
      le tip, et le bouton d'appel de la table CCAM (mode 1 seulement).

 TODO : la 2e fiche (table des actes CCAM, appelée par le bouton) et le stockage propre d'une
 association à 2 codes restent à faire (aujourd'hui l'association est stockée « code1+code2 »).
*/

class dlg_gestioncotations : public UpDialog
{
    Q_OBJECT
public:
    enum Mode {Creation, Modification};    Q_ENUM(Mode)
    explicit        dlg_gestioncotations(Mode mode, QString CodeActe = "", QWidget *parent = Q_NULLPTR);
    ~dlg_gestioncotations();
    QString         codeenregistre() const      { return m_codeenregistre; }    /*!< code réellement écrit (pour scroller dessus dans la table appelante) */

private:
    DataBase        *db = DataBase::I();
    Mode            m_mode;
    bool            m_cotationsfrance = db->parametres()->cotationsfrance();
    QString         m_codeacte;
    QString         m_codeenregistre;                           //!< code écrit à la validation (création/modif), lu par l'appelant
    int             m_idcotation = 0;                           //!< id de la cotation en modification (clé de l'update)
    int             m_typecotation = 1;                         //!< mode courant : 1=CCAM, 2=association, 4=autre
    QStringList     m_listeCCAM;                                //!< codes de la table ccam (pour valider « c'est bien un code CCAM »)
    QStandardItemModel *m_modelCCAM = Q_NULLPTR;                //!< code CCAM (DisplayRole) + libellé (ToolTipRole) : source des QCompleter avec infobulle

    /*! sélecteur de mode (création seulement) */
    QGroupBox       *wdg_groupmode = Q_NULLPTR;
    QCheckBox       *wdg_chkCCAM = Q_NULLPTR, *wdg_chkAssoc = Q_NULLPTR, *wdg_chkAutre = Q_NULLPTR;

    /*! les champs (uniques, montrés/masqués selon le mode) */
    UpLineEdit      *wdg_codeline = Q_NULLPTR, *wdg_codeline2 = Q_NULLPTR;
    UpLineEdit      *wdg_tarifoptamline = Q_NULLPTR, *wdg_tarifnooptamline = Q_NULLPTR, *wdg_tarifpratiqueline = Q_NULLPTR;
    UpLabel         *wdg_optamlabel = Q_NULLPTR;                //!< retitré « Conventionnel » en mode 4
    UpTextEdit      *wdg_tipline = Q_NULLPTR;
    QPushButton     *wdg_boutonCCAM1 = Q_NULLPTR,               //!< « ... » à droite du 1er code : ouvre la table CCAM
                    *wdg_boutonCCAM2 = Q_NULLPTR;               //!< « ... » à droite du 2e code (association)
    QWidget         *wdg_codewidg = Q_NULLPTR, *wdg_code2widg = Q_NULLPTR, *wdg_tarifoptamwidg = Q_NULLPTR,
                    *wdg_tarifnooptamwidg = Q_NULLPTR, *wdg_tarifpratiquewidg = Q_NULLPTR, *wdg_tipwidg = Q_NULLPTR;

    void            appliqueMode();                             //!< montre/masque/retitre les champs et pose les completers selon m_typecotation
    void            remplitDepuisCCAM();                        //!< modes 1/2 : remplit montants + libellé à partir du/des code(s) CCAM
    void            appelleTableCCAM(UpLineEdit *cible);        //!< ouvre la table CCAM ; le code choisi remplit cible puis remplitDepuisCCAM()
    void            regleOK();                                  //!< active OK selon le mode (tous champs visibles remplis ; « autre » : code + pratiqué)
    bool            VerifFiche();
};

#endif // DLG_GESTIONCOTATIONS_H
