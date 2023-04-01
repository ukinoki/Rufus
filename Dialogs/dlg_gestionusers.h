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

#ifndef DLG_GESTIONUSERS_H
#define DLG_GESTIONUSERS_H

#include "database.h"
#include "dlg_gestioncomptes.h"
#include "dlg_listelieux.h"
#include "icons.h"
#include "utils.h"
#include "upheaderview.h"

/* sert à gérer les comptes utilisateurs
 * QUASI-IDENTIQUE POUR RUFUS ET RUFUSADMIN*/

namespace Ui {
class dlg_gestionusers;
}

class dlg_gestionusers : public UpDialog
{
    Q_OBJECT
public:
    enum                    UserMode {PREMIERUSER, ADMIN, MODIFUSER}; Q_ENUM(UserMode)
    enum                    Mode {Creer, Modifier, PremierUsr};       Q_ENUM(Mode)
    /* correspond aux 3 façons dont la fiche a été appelée
     * PREMIERUSER -> la fiche est appelée au premier lancement du programme pour créer le premier utilisateur -> on peut tout modifier
     * MODIFUSER   -> appelé par l'utilisateur dans le premier onglet de la fiche dlg_param -> on ne peut modifier que les données d'identité, geographiques et bancaires
     * ADMIN       -> appelé par l'administrateur, on peut tout modidier, y compris le statut, de chaque utilisateur, sauf les données bancaires
    */
    explicit dlg_gestionusers(int idlieu, UserMode  mode, bool mdpverified=true, QWidget *parent = Q_NULLPTR);
    /*
     * la variable mdpverified est utilisée pour l'appel de la fiche dlg_gestionlieux
     * Cette fiche est parfois appelée alors que le mdp administrateur a déjà eté vérifié, parfois non
     */
    ~dlg_gestionusers();
    Ui::dlg_gestionusers    *ui;
    void                    setConfig(UserMode mode);
    bool                    isMDPverified() const;

private:
    DataBase                *db = DataBase::I();
    QBrush                  m_color = QBrush(QColor(Qt::magenta));
    Mode                    m_mode = Modifier;

    bool                    m_MDPverified;
    UserMode                m_usermode;
    bool                    m_ophtalmo;
    bool                    m_orthoptist;
    bool                    m_autresoignant;
    bool                    m_autrenonsoignant;
    bool                    m_neutre;
    bool                    m_medecin;
    bool                    m_soccomptable;
    bool                    m_assistant;
    bool                    m_liberal;
    bool                    m_liberalSEL;
    bool                    m_pasliberal;
    bool                    m_retrocession;
    bool                    m_cotation;
    bool                    m_soignant;
    bool                    m_responsable;
    bool                    m_respsalarie;
    bool                    m_respliberal;
    bool                    m_respliberalSEL;
    bool                    m_soignantnonremplacant;

    bool                    m_ok;

    UpDialog                *dlg_ask;
    int                     m_idlieu;
    User                    *m_userencours;
    QString                 gLoginupLineEdit, gMDPupLineEdit, gConfirmMDPupLineEdit;
    QString                 gLibActiv, gNoLibActiv;
    QString                 gNouvMDP, gAncMDP, gConfirmMDP;
    QStringList             gListBanques;
    UpDialog                *gAskMDP;
    bool                    AfficheParamUser(int idUser);
    void                    CalcListitemsCompteComptacomboBox(User *usr, bool m_soccomptable);
    void                    CalcListitemsEmployeurcomboBox(User* usr);
    void                    ChoixButtonFrame();
    void                    DefinitLesVariables();
    bool                    ExisteEmployeur(int iduser);
    void                    Inactifs();
    void                    RemplirTableWidget(int iduser);
    void                    ReconstruitListeLieuxExercice();
    bool                    VerifFiche();

    void                    setDataCurrentUser(int id);
    WidgetButtonFrame       *wdg_buttonframe;
    void                    Annulation();
    void                    CreerUser();
    void                    EnregistreNouvMDP();
    void                    EnregistreNouvUser();
    void                    EnregistreUser();
    void                    FermeFiche();
    void                    GestionComptes();
    void                    GestLieux();
    void                    ModifMDP();
    void                    ModifUser();
    void                    RegleAffichage();
    void                    SupprUser();
};

#endif // DLG_gestionusers_H
