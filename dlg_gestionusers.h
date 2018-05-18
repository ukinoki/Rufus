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

#ifndef DLG_GESTIONUSERS_H
#define DLG_GESTIONUSERS_H

#include <QDesktopWidget>
#include <QInputDialog>
#include <QNetworkInterface>
#include <QTableWidgetItem>
#include <QtSql>
#include <QSqlDatabase>

#include "dlg_gestioncomptes.h"
#include "uplineedit.h"
#include "uptextedit.h"
#include "widgetbuttonframe.h"
#include "functordatauser.h"
#include "functormajpremierelettre.h"
#include "macros.h"

namespace Ui {
class dlg_gestionusers;
}

class dlg_gestionusers : public UpDialog
{
    Q_OBJECT

public:
    explicit dlg_gestionusers(int idUser, int idlieu, QSqlDatabase db, QMap<QString,QIcon> Icons, QWidget *parent = Q_NULLPTR);
    ~dlg_gestionusers();
    Ui::dlg_gestionusers        *ui;
    int                     Mode;
    enum                    Mode {PREMIERUSER, ADMIN, MODIFUSER};
    void                    setConfig(enum Mode);

private:
    bool                    ophtalmo;
    bool                    orthoptist;
    bool                    autresoignant;
    bool                    medecin;
    bool                    soccomptable;
    bool                    assistant;
    bool                    liberal;
    bool                    pasliberal;
    bool                    retrocession;
    bool                    cotation;
    bool                    soignant;
    bool                    responsable;
    bool                    respsalarie;
    bool                    respliberal;
    bool                    soigntnonrplct;

    FunctorDataUser         fdatauser;
    FunctorMAJPremiereLettre fMAJPremiereLettre;
    dlg_gestioncomptes      *Dlg_GestComptes;
    UpDialog                *gAsk;
    QSqlDatabase            db;
    QBrush                  gcolor;
    QMap<QString,QIcon>     gmapIcons;
    int                     gMode;
        enum gMode          {Creer, Modifier, PremierUsr};
    int                     gidUserDepart;
    int                     gidLieu;
    QMap<QString,QVariant>  OtherUser;
    QString                 gLoginupLineEdit, gMDPupLineEdit, gConfirmMDPupLineEdit;
    QString                 gLibActiv, gNoLibActiv;
    QString                 gNouvMDP, gAncMDP, gConfirmMDP;
    QStringList             gListBanques;
    QRegExp                 rx, rxMail, rxTel;
    UpDialog                *gAskMDP;
    void                    ActualiseRsgnmtBanque(bool soccomptable);
    bool                    AfficheParamUser(int idUser);
    void                    CalcListitemsCompteActescomboBox(int iduser);
    void                    CalcListitemsCompteComptacomboBox(int iduser, bool soccomptable);
    void                    CalcListitemsEmployeurcomboBox(int iduser);
    QString                 CorrigeApostrophe(QString RechAp);
    void                    DefinitLesVariables();
    QString                 Edit(QString txt, QString titre = "");
    bool                    ExisteEmployeur(int iduser);
    void                    RemplirTableWidget(int iduser);
    bool                    TraiteErreurRequete(QSqlQuery query, QString requete, QString ErrorMessage = "");
    bool                    VerifFiche();

    bool                    setDataUser(int id);
    QMap<QString,QVariant>  DataUser();
    WidgetButtonFrame       *widgButtons;
    void                    CreerUser();
    void                    ModifUser();
    void                    SupprUser();

private slots:
    void                    Slot_Annulation();
    void                    Slot_CompleteRenseignements(QTableWidgetItem*,QTableWidgetItem*);
    void                    Slot_EnableOKpushButton();
    void                    Slot_EnregistreNouvMDP();
    void                    Slot_EnregistreUser();
    void                    Slot_FermeFiche();
    void                    Slot_GestionComptes();
    void                    Slot_ModifMDP();
    void                    Slot_RegleAffichage();
    void                    Slot_EnregistreNouvUser();
    void                    Slot_ChoixButtonFrame(int);
};

#endif // DLG_gestionusers_H
