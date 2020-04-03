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

#ifndef DLG_IDENTIFICATIONPATIENT_H
#define DLG_IDENTIFICATIONPATIENT_H

#include "dlg_identificationcorresp.h"
#include "ui_dlg_identificationcorresp.h"
#include "flags.h"

namespace Ui {
    class dlg_identificationpatient;
}

/*!
 * \brief The dlg_identificationpatient class
 * Affiche les informations de profil d'un patient
 */
class dlg_identificationpatient : public UpDialog
{
    Q_OBJECT
public:
    enum Mode                       {Copie, Creation, Modification};    Q_ENUM(Mode)
    explicit                        dlg_identificationpatient(enum Mode mode, Patient *pat, QWidget *parent = Q_NULLPTR);
    ~dlg_identificationpatient();
    Ui::dlg_identificationpatient   *ui;
    Patient*                        currentpatient() const;

private:
    DataBase                        *db = DataBase::I();
    Patient                         *m_currentpatient;
    bool                            m_ok;
    int                             m_flagcorrespondants;
    Mode                            m_mode;
    QMenu                           *m_menucontextuel;
    QTimer                          *t_timer;
    UpSmallButton                   *widg_vitalebutton;
    VilleCPWidget                   *wdg_villeCP;
    QLineEdit                       *wdg_CPlineedit, *wdg_villelineedit;

    bool                            eventFilter(QObject *obj, QEvent *event)  ;
    void                            AfficheDossierAlOuverture();
    void                            AnnulpushButtonClicked();
    void                            ChoixMG();
    void                            EnableOKpushButton();
    int                             EnregistreNouveauCorresp();
    void                            MAJMG();
    void                            Majuscule(QLineEdit *ledit);
    void                            MenuContextuelMedecin();
    void                            ModifCorrespondant();
    void                            ModifDDN();
    void                            OKpushButtonClicked();
    void                            VerifMGFlag();
};

#endif // DLG_IDENTIFICATIONPATIENT_H
