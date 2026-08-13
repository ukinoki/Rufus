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

#ifndef DLG_INITBASE_H
#define DLG_INITBASE_H

#include <QButtonGroup>
#include <QComboBox>
#include <QFrame>
#include "database.h"
#include "updialog.h"
#include "upcheckbox.h"
#include "upgroupbox.h"

/*!
Fiche unique des choix d'installation, affichée juste après la création d'une base vierge.
Elle remplace les trois messagebox qui défilaient auparavant (premier utilisateur, cotations, codes postaux).

* quatre blocs, tous préréglés sur la configuration française
	* comptabilité : 3 cases exclusives (normale, simplifiée, aucune) -> compta()
	* cotation des actes : 2 cases exclusives (françaises, génériques) -> cotationsfrance()
	* langue de l'interface et territoire d'exercice, drapeau devant chaque item -> langue(), territoire()
	* rappel non modifiable de la création de l'utilisateur factice Docteur Snow
* la fiche n'écrit rien : l'appelant lit les choix par les accesseurs après exec()
*/

class dlg_initbase : public UpDialog
{
    Q_OBJECT
public:
    explicit            dlg_initbase(QWidget *parent = Q_NULLPTR);
    DataBase::compta    compta() const           { return wdg_comptareduitecheck->isChecked()? DataBase::Reduite
                                                        : wdg_sanscomptacheck->isChecked()?    DataBase::NoCompta
                                                                                             : DataBase::Normal; }
    bool                cotationsfrance() const  { return wdg_cotationsfrancecheck->isChecked(); }
    QString             langue() const           { return wdg_languecombo->currentData().toString(); }
    QString             territoire() const       { return wdg_territoirecombo->currentData().toString(); }

private:
    /*! les cases à cocher */
    UpCheckBox      *wdg_comptanormalecheck, *wdg_comptareduitecheck, *wdg_sanscomptacheck;
    UpCheckBox      *wdg_cotationsfrancecheck, *wdg_cotationsgeneriquescheck;
    /*! les listes déroulantes - QComboBox nu, comme le sélecteur de langue de dlg_param */
    QComboBox       *wdg_languecombo, *wdg_territoirecombo;
    /*! le groupe des cotations, grisé quand on ne tient pas de comptabilité */
    UpGroupBox      *wdg_cotationsgroup;

    UpGroupBox*     GroupeExclusif(QString titre, QList<UpCheckBox*> cases);    //!< groupe où une seule case peut être cochée
    QWidget*        LigneCombo(QString texte, QComboBox *combo);                //!< libellé + liste déroulante sur une ligne
    void            RemplitLangues();                                          //!< les 7 langues traduites, avec leur drapeau
    void            RemplitTerritoires();                                      //!< territoires de QLocale, nom natif + code ISO à 2 lettres
};

#endif // DLG_INITBASE_H
