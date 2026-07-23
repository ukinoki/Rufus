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
 * \brief Cotations::loadCotations
 * Charge l'ensemble des cotations de la table cotations (avec leur type), sans idUser ni montant
 * pratiqué (désormais portés par les tables de jointures par utilisateur).
 */
void Cotations::loadCotations()
{
    //! map de référence clée par idcotation (unique dans la table)
    QList<Cotation*> listcotations = DataBase::I()->loadCotations();
    epurelist(map_cotations, &listcotations);
    addList(map_cotations, &listcotations);

    //! actes CCAM sans descriptif (Tip vide) : on rapatrie le libellé depuis la table ccam et on le
    //! persiste (ItemsList::update -> base + objet en mémoire). Ne coûte qu'au premier chargement,
    //! ensuite le Tip est renseigné et la condition ci-dessous n'est plus vraie.
    for (Cotation *c : *map_cotations)
    {
        if (!c->isCCAM() || !c->descriptif().isEmpty())
            continue;
        bool ok = false;
        QVariantList rec = DataBase::I()->getFirstRecordFromStandardSelectSQL(
                    "select " CP_NOM_CCAM " from " TBL_CCAM
                    " where " CP_CODECCAM_CCAM " = '" + c->typeacte() + "'", ok);
        if (ok && !rec.isEmpty() && !rec.at(0).toString().isEmpty())
            ItemsList::update(c, CP_TIP_COTATIONS, rec.at(0).toString());
    }

    //! même chose pour les NGAP (AMY...) : le libellé vient du fichier de cotations (xml). On ne le
    //! charge (et ne le parse) qu'une fois, et seulement s'il y a au moins un NGAP à compléter.
    QMap<QString,QString> nomsNGAP;
    bool ngapcharge = false;
    for (Cotation *c : *map_cotations)
    {
        if (!c->isNGAP() || !c->descriptif().isEmpty())
            continue;
        if (!ngapcharge)
        {
            nomsNGAP   = DataBase::I()->nomsNGAPFromXml();
            ngapcharge = true;
        }
        const QString nom = nomsNGAP.value(c->typeacte());
        if (!nom.isEmpty())
            ItemsList::update(c, CP_TIP_COTATIONS, nom);
    }
}

/*!
 * \brief Cotations::loadUserCotations
 * Bâtit map_usercotations SANS créer de nouveaux pointeurs : on récupère les idcotation utilisés
 * par le user (montant pratiqué) dans les 4 jointures, on retrouve chaque cotation dans
 * map_cotations, on la marque used() et on lui affecte son montant pratiqué et son montant
 * conventionnel (selon OPTAM). map_usercotations n'est qu'une vue : les pointeurs restent la
 * propriété de map_cotations (d'où clear() et non clearAll()).
 */
void Cotations::loadUserCotations(User *usr)
{
    if (usr == nullptr)
        return;
    const bool optam = usr->isOPTAM();
    map_usercotations->clear();
    for (Cotation *c : *map_cotations)
        c->setused(false);
    QMap<int, double> montants = DataBase::I()->loadMontantsPratiquesByUser(usr);
    for (auto it = montants.constBegin(); it != montants.constEnd(); ++it)
    {
        Cotation *c = map_cotations->value(it.key(), nullptr);        //! retrouvée par idcotation
        if (c == nullptr)
            continue;
        c->setused(true);
        c->setmontantpratique(it.value());
        //! montant conventionnel : nonoptam seulement si non OPTAM et CCAM (1)/assoc (2), sinon optam
        const double conv = (!optam && (c->isCCAM() || c->isAssocCCAM())) ? c->montantnonoptam() : c->montantoptam();
        c->setmontantconventionnel(conv);
        map_usercotations->insert(it.key(), c);
    }
    m_userparent = usr;
}

