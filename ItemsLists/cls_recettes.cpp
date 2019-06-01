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

#include "cls_recettes.h"

Recettes::Recettes(QObject *parent) : ItemsList(parent)
{
    m_recettes = new QMap<int, Recette*>();
}

QMap<int, Recette *> *Recettes::recettes() const
{
    return m_recettes;
}

/*!
 * \brief Recettes::initListeRecettes
 * Charge l'ensemble des Recettes
 * et les ajoute à la classe Recettes
 */
void Recettes::initListe(QMap<QString, QDate> DateMap)
{
    clearAll();
    addList(DataBase::I()->loadRecettesByDate(DateMap["DateDebut"], DateMap["DateFin"]));
}

void Recettes::add(Recette *Recette)
{
    if( m_recettes->contains(Recette->id()) )
        return;
    m_recettes->insert(Recette->id(), Recette);
}

void Recettes::addList(QList<Recette*> listRecettes)
{
    QList<Recette*>::const_iterator it;
    for( it = listRecettes.constBegin(); it != listRecettes.constEnd(); ++it )
        add( *it );
}

void Recettes::clearAll()
{
    for( QMap<int, Recette*>::const_iterator itrec = m_recettes->constBegin(); itrec != m_recettes->constEnd(); ++itrec)
        delete itrec.value();
    m_recettes->clear();
}

void Recettes::remove(Recette *recette)
{
    if (recette == Q_NULLPTR)
        return;
    m_recettes->remove(recette->id());
    delete recette;
}

Recette* Recettes::getById(int id)
{
    QMap<int, Recette*>::const_iterator itcpt = m_recettes->find(id);
    if( itcpt == m_recettes->constEnd() )
        return Q_NULLPTR;
    return itcpt.value();
}
