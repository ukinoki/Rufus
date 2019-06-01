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

#include "cls_correspondants.h"

/*
 * GETTER
*/
QMap<int, Correspondant *> *Correspondants::correspondants() const
{
    return m_correspondants;
}

/*!
 * \brief Correspondants::Correspondants
 * Initialise la map Correspondants
 */
Correspondants::Correspondants(QObject *parent) : ItemsList(parent)
{
    m_correspondants = new QMap<int, Correspondant*>();
}

void Correspondants::clearAll()
{
    for( QMap<int, Correspondant*>::const_iterator itcor = m_correspondants->constBegin(); itcor != m_correspondants->constEnd(); ++itcor)
        delete itcor.value();
    m_correspondants->clear();
}

void Correspondants::remove(Correspondant *cor)
{
    if (cor == Q_NULLPTR)
        return;
    m_correspondants->remove(cor->id());
    delete cor;
}

/*!
 * \brief Correspondants::add
 * ajoute le correspondant passé en paramètre
 * à la liste des correspondants
 * \param Correspondant le correspondant que l'on veut ajouter
 * \return true si le correspondant est ajouté
 * \return false si le paramètre correspondant est un Q_NULLPTR
 * \return false si le correspondant est déjà présent
 */
bool Correspondants::add(Correspondant *cor)
{
    if( cor == Q_NULLPTR)
        return false;
    if( m_correspondants->contains(cor->id()) )
        return false;
    m_correspondants->insert(cor->id(), cor);
    return true;
}

void Correspondants::addList(QList<Correspondant*> listcor)
{
    QList<Correspondant*>::const_iterator it;
    for( it = listcor.constBegin(); it != listcor.constEnd(); ++it )
        add( *it );
}

/*!
 * \brief Correspondants::getById
 * \param id l'id du correspondant recherché
 * \param loadDetails   -> charge les détails si ce n'est pas déjà fait
 * \param addToList     -> ajoute à la liste des correspondants s'il ne s'y trouve pas encore
 * \return Q_NULLPTR si aucun correspondant trouvé
 * \return Correspondant* le correspondant correspondant à l'id
 */
Correspondant* Correspondants::getById(int id, Item::LOADDETAILS loaddetails, bool addToList)
{
    QMap<int, Correspondant*>::const_iterator itcor = m_correspondants->find(id);
    Correspondant *result;
    if( itcor == m_correspondants->constEnd() )
        result = new Correspondant();
    else
    {
        result = itcor.value();
        if(loaddetails == Item::NoLoadDetails)
            return result;
        addToList = false;
    }

    if( !result->isAllLoaded() )
    {
        QJsonObject jsonCorrespondant = DataBase::I()->loadCorrespondantData(id);
        if( jsonCorrespondant.isEmpty() )
        {
            delete result;
            return Q_NULLPTR;
        }
        else
            result->setData(jsonCorrespondant);
    }
    if( addToList )
        add( result );
    return result;
}


/*!
 * \brief Correspondants::initListe correspondants
 * Charge l'ensemble des correspondants
 * et les ajoute à la classe Correspondants
 */
void Correspondants::initListe(bool all)
{
    clearAll();
    QList<Correspondant*> listcorrespondants;
    if (all)
        listcorrespondants = DataBase::I()->loadCorrespondantsALL();
    else
        listcorrespondants = DataBase::I()->loadCorrespondants();
    addList(listcorrespondants);
}
