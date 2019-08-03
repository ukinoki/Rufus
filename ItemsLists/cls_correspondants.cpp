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

/*!
 * \brief Correspondants::getById
 * \param id l'id du correspondant recherché
 * \param loadDetails   -> charge les détails si ce n'est pas déjà fait
 * \param addToList     -> ajoute à la liste des correspondants s'il ne s'y trouve pas encore
 * \return Q_NULLPTR si aucun correspondant trouvé
 * \return Correspondant* le correspondant correspondant à l'id
 */
Correspondant* Correspondants::getById(int id, Item::LOADDETAILS loaddetails, ADDTOLIST addToList)
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
        addToList = ItemsList::NoAddToList;
    }

    if( !result->isallloaded())
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
    if( addToList == ItemsList::AddToList)
        add(m_correspondants, result );
    return result;
}

void Correspondants::loadAll(Correspondant *cor, Item::UPDATE upd)
{
    if (cor == Q_NULLPTR)
        return;
    if (!cor->isallloaded() || upd == Item::ForceUpdate)
    {
        QJsonObject jsoncor = DataBase::I()->loadCorrespondantData(cor->id());
        if( !jsoncor.isEmpty() )
            cor->setData(jsoncor);
    }
    if (m_correspondants->find(cor->id()) == m_correspondants->cend())
        add (m_correspondants, cor);
}


/*!
 * \brief Correspondants::initListe correspondants
 * Charge l'ensemble des correspondants
 * et les ajoute à la classe Correspondants
 */
void Correspondants::initListe(bool all)
{
    QList<Correspondant*> listcorrespondants;
    if (all)
        listcorrespondants = DataBase::I()->loadCorrespondantsALL();
    else
        listcorrespondants = DataBase::I()->loadCorrespondants();
    epurelist(m_correspondants, &listcorrespondants);
    addList(m_correspondants, &listcorrespondants);
}

QStringList Correspondants::autresprofessions()
{
    QStringList listprof = QStringList();
    foreach  (const Correspondant *cor, m_correspondants->values())
    {
        if (!cor->ismedecin())
            listprof << cor->metier();
    }
    return listprof;
}

void Correspondants::SupprimeCorrespondant(Correspondant *cor)
{
    if (cor == Q_NULLPTR)
        return;
    QString id = QString::number(cor->id());
    Supprime(m_correspondants, cor);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedmg  = null where idcormedmg  = " + id);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe1 = null where idcormedspe1 = " + id);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe2 = null where idcormedspe2 = " + id);
    DataBase::I()->StandardSQL("update " TBL_RENSEIGNEMENTSMEDICAUXPATIENTS " set idcormedspe3 = null where idcormedspe3 = " + id);
}

