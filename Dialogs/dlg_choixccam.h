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

#ifndef DLG_CHOIXCCAM_H
#define DLG_CHOIXCCAM_H

#include <QCheckBox>
#include "procedures.h"
#include "uptablewidget.h"
#include "uptextedit.h"

/*!
 Fiche de choix d'un acte dans la table CCAM (appelée par dlg_gestioncotations, mode CCAM).
 Reprend, en orientation portrait, la présentation de la zone CCAM de dlg_param :
    * en tête, le titre « Actes CCAM » et la case « uniquement l'ophtalmologie » (cochée par
      défaut -> filtre les codes en B..) ; la décocher recharge toute la CCAM.
    * la table des actes (code / OPTAM / non OPTAM), une seule ligne sélectionnable.
    * sous la table, un UpTextEdit qui affiche le libellé de l'acte sélectionné.
 Le bouton OK est désactivé tant qu'aucune ligne n'est choisie. À l'acceptation, codechoisi()
 rend le code CCAM retenu (vide si annulation).
*/

class dlg_choixccam : public UpDialog
{
    Q_OBJECT
public:
    explicit    dlg_choixccam(QWidget *parent = Q_NULLPTR);
    QString     codechoisi() const              { return m_codechoisi; }    /*!< code CCAM retenu (vide si aucun) */

private:
    DataBase    *db = DataBase::I();
    QString     m_codechoisi;

    QCheckBox   *wdg_ophtaseul = Q_NULLPTR;      //!< « uniquement l'ophtalmologie » (coché par défaut)
    UpTableWidget *wdg_table = Q_NULLPTR;        //!< actes CCAM : col 0 code, 1 OPTAM, 2 non OPTAM, 3 libellé (masquée)
    UpTextEdit  *wdg_libelle = Q_NULLPTR;        //!< libellé de l'acte sélectionné

    void        remplitTable(bool ophtaseul);   //!< (re)remplit la table (ophtaseul -> codes en B.. seulement)
    void        selectionChangee();             //!< met à jour libellé + code retenu + état du bouton OK
    void        chercheEtSelectionne(const QString &code); //!< sélectionne/défile vers la 1re ligne dont le code commence par...
};

#endif // DLG_CHOIXCCAM_H
