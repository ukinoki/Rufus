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

#include "cls_cotations.h"



Cotations::Cotations(QObject *parent) : ItemsList(parent)
{
    map_cotations     = new QMap<int, Cotation*>();
    map_usercotations = new QMap<int, Cotation*>();
}

QMap<int, Cotation *> *Cotations::cotations() const
{
    return map_cotations;
}

QMap<int, Cotation *> *Cotations::usercotations() const
{
    return map_usercotations;
}

/*!
 * \brief Cotations::initListeByUser
 * Charge l'ensemble des cotations pour le user
 * et les ajoute à la classe Correspondants
 */
void Cotations::initListeByUser(User *usr)
{
    if (usr == Q_NULLPTR)
         return;
     QList<Cotation*> listcotations = DataBase::I()->loadCotationsByUser(usr);
     epurelist(map_cotations, &listcotations);
     addList(map_cotations, &listcotations);
}

/*!
 * \brief Cotations::loadCotations
 * Charge l'ensemble des cotations de la table cotations (avec leur type), sans idUser ni montant
 * pratiqué (désormais portés par les tables de jointures par utilisateur).
 */
void Cotations::loadCotations()
{
    QList<Cotation*> listcotations = DataBase::I()->loadCotations();
    epurelist(map_cotations, &listcotations);
    addList(map_cotations, &listcotations);
}

/*!
 * \brief Cotations::loadUserCotations
 * Réunit dans map_usercotations toutes les cotations du user extraites des 4 tables de jointures
 * (montant conventionnel selon son statut OPTAM, montant pratiqué propre au user).
 */
void Cotations::loadUserCotations(User *usr)
{
    if (usr == Q_NULLPTR)
        return;
    //! reconstruction complète : la clé est l'idsynth (unique), le vrai id reste dans la cotation.
    //! addList clé par id() (le vrai idcotation/idccam, qui peut entrer en collision entre tables),
    //! on insère donc à la main par idsynth.
    clearAll(map_usercotations);
    QList<Cotation*> listcotations = DataBase::I()->loadUserCotations(usr);
    for (Cotation *c : listcotations)
        map_usercotations->insert(c->idsynth(), c);
}

