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

#ifndef DLG_REMISECHEQUES_H
#define DLG_REMISECHEQUES_H

#include "procedures.h"
#include "uptablewidget.h"

namespace Ui {
class dlg_remisecheques;
}

class dlg_remisecheques : public QDialog
{
    Q_OBJECT

public:
    explicit dlg_remisecheques(QWidget *parent = Q_NULLPTR);
    ~dlg_remisecheques();
    Ui::dlg_remisecheques   *ui;
    bool                    initOK() const;
    enum Mode               {NouvelleRemise, RevoirRemisesPrecs, ReprendreRemise};   Q_ENUM(Mode)

private:
    DataBase                *db     = DataBase::I();
    Procedures              *proc   = Procedures::I();
    bool                    m_initok = true;
    Mode                    m_mode;
    QMap<int, User *>       *map_comptables;
    QMap<int, User *>       *map_comptablesavecchequesenattente;
    User                    *m_currentuser;
    QString                 m_valeuravantchangement;
    bool                    m_bloquecellchanged;  // ce flag sert à empêcher le déclenchement du signal cellchanged sur les tables quand on utilise certianes fonctions

    void                    closeEvent(QCloseEvent *event);
    bool                    eventFilter(QObject *obj, QEvent *event)  ;
    void                    keyPressEvent ( QKeyEvent * event );
    void                    reject();
    void                    ChangeCompte();
    void                    ChangeUser();
    void                    ImprimepushButton();
    bool                    ImprimerRemise(int);
    void                    AnnulupPushButton();
    void                    CorrigeRemise();
    void                    ItemChequeARemettreClicked(int A, int B);              //utilisé pour mettre en attente un chèque
    void                    ItemChequeEnAttenteClicked(int A, int B);              //utilisé pour mettre en encaissement un chèque auparavant en attente
    void                    MiseEnFormeMontant(int A, int B, int C, int D);
    void                    RecalculeMontant();
    void                    ReconstruitListeUsers();
    void                    RegleComptesComboBox(bool ActiveSeult = true);
    void                    RemplirRemisesPrecs(int);
    void                    ToolTip(int A, int B);
    void                    TrierListeCheques(int, int B);
    void                    TrierChequesEnAttente(int, int B);
    bool                    VoirNouvelleRemise();
    bool                    VoirRemisesPrecs();
};

#endif // DLG_REMISECHEQUES_H
