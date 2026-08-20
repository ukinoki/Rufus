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

#ifndef DLG_IDENTIFICATIONSITE_H
#define DLG_IDENTIFICATIONSITE_H

#include "database.h"
#include "gbl_datas.h"
#include "uplineedit.h"
#include "uppushbutton.h"

/*! La fiche dlg_identificationsite saisit les coordonnées d'un lieu d'exercice
    * elle ne touche pas à la base : elle valide les champs et expose le résultat dans listbinds(), l'appelant enregistre
    * trois modes d'ouverture
        * Nouv        -> tous les champs vides et saisissables, choix de la couleur
        * Modif       -> tous les champs préremplis et saisissables
        * Complement  -> les coordonnées du lieu préremplies et grisées, seuls les champs d'envoi de mail restent saisissables
*/

class dlg_identificationsite : public UpDialog
{
    Q_OBJECT
public:
    enum Mode                       {Nouv, Modif, Complement}; Q_ENUM(Mode)
    explicit                        dlg_identificationsite(Site *sit = nullptr, Mode mode = Modif, QWidget *parent = nullptr);
    QHash<QString, QVariant>        listbinds() const           { return m_listbinds; }

private:
    /*! les widgets */
    UpLineEdit                      *wdg_nomlineedit;
    UpLineEdit                      *wdg_adress1lineedit;
    UpLineEdit                      *wdg_adress2lineedit;
    UpLineEdit                      *wdg_adress3lineedit;
    UpLineEdit                      *wdg_CPlineedit;
    UpLineEdit                      *wdg_villelineedit;
    UpLineEdit                      *wdg_tellineedit;
    UpLineEdit                      *wdg_faxlineedit;
    UpLineEdit                      *wdg_maillineedit;
    UpLineEdit                      *wdg_smtpserveurlineedit;
    UpLineEdit                      *wdg_smtpportlineedit;
    UpLineEdit                      *wdg_smtploginlineedit;
    UpPushButton                    *wdg_couleurpushbutt;

    /*! les données */
    Site                            *m_site;
    Mode                            m_mode;
    QString                         str_nouvcolor = "";
    QHash<QString, QVariant>        m_listbinds;

    void                            ModifCouleur();
    void                            Valide();                   /*!< contrôle la fiche, remplit listbinds et ferme */
};

#endif // DLG_IDENTIFICATIONSITE_H
