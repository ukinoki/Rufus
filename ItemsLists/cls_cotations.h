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

#ifndef CLS_COTATIONS_H
#define CLS_COTATIONS_H

#include "cls_cotation.h"
#include "cls_itemslist.h"
#include "cls_user.h"

/*!
 Liste des cotations.
    * map_cotations : la map de RÉFÉRENCE (toutes les cotations de la table, clées par idcotation),
      chargée par initListe().
    * map_usercotations : une VUE des cotations d'un utilisateur, réunies depuis les 4 tables de
      jointures par loadUserCotations() — mêmes pointeurs que map_cotations, marqués used() et
      renseignés de leur montant pratiqué/conventionnel (d'où clear() et non clearAll()).
 completeTipsManquants() renseigne, une seule fois au lancement, les Tip vides (CCAM depuis la table
 ccam, NGAP depuis le fichier xml).
*/

class Cotations : public ItemsList
{
    Q_OBJECT
private:
    QMap<int, Cotation*> *map_cotations;        //!< référence : toutes les cotations (clé idcotation)
    QMap<int, Cotation*> *map_usercotations;    //!< vue : les cotations d'un user (réunies des 4 jointures)
    QMap<QString, Cotation*> *map_ccam;         //!< la nomenclature CCAM entière, clée par son code (vide hors France)
    User* m_userparent = nullptr;               //!< le user dont on a chargé les cotations

public:
    explicit Cotations(QObject *parent = nullptr);
    QMap<int, Cotation *> *cotations() const     { return map_cotations; }
    QMap<int, Cotation *> *usercotations() const { return map_usercotations; }
    QMap<QString, Cotation *> *ccam() const      { return map_ccam; }
    Cotation* ccamByCode(const QString &code) const { return map_ccam->value(code, nullptr); }
    void  initListe();                  //!< charge toutes les cotations de la table (avec leur type), sans idUser ni montant pratiqué
    void  initListeCCAM();              //!< charge la nomenclature CCAM en mémoire — UNE FOIS au lancement
    void  completeTipsManquants();      //!< renseigne les Tip vides (CCAM depuis ccam, NGAP depuis le xml) — UNE FOIS au lancement, pas à chaque initListe
    void  loadUserCotations(User *usr); //!< réunit dans map_usercotations les cotations du user depuis les 4 jointures
    User* userparent()      { return m_userparent; }
};

#endif // CLS_COTATIONS_H
