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

#include "cls_villes.h"
#include "database.h"

#include <QException>

/*
 * VilleListModel
*/
VilleListModel::VilleListModel(const QList<Ville*> &villes, FieldName fieldName, QObject *parent) : QAbstractListModel(parent)
{
    m_villes = villes;
    setFieldName(fieldName);
}

void VilleListModel::setFieldName(FieldName fieldName)
{
    m_fieldName = fieldName;

    if( m_fieldName == NOM )
        std::sort( m_villes.begin(), m_villes.end(), VilleListModel::sortByName );

    if( m_fieldName == CODEPOSTAL )
        std::sort( m_villes.begin(), m_villes.end(), VilleListModel::sortByCodePostal );
}
int VilleListModel::rowCount(const QModelIndex& parent) const { Q_UNUSED(parent) return m_villes.size(); }

QVariant VilleListModel::data(const QModelIndex& index, int role) const
{
    // Check that the index is valid and within the correct range first:
    if( !index.isValid()
            || index.row() >= m_villes.size()
            || role != Qt::DisplayRole )
        return QVariant();

    if( m_fieldName == NOM )
        return QString(m_villes.at(index.row())->nom());

    if( m_fieldName == CODEPOSTAL )
        return QString(m_villes.at(index.row())->codePostal());

    return QVariant();
}


/*
 * Villes
*/
Villes::Villes(QObject *parent) : ItemsList(parent)
{
}

void Villes::initListe()
{
    addList(DataBase::I()->loadVilles());
}

bool Villes::add(Ville *ville)
{
    if (ville == Q_NULLPTR)
        return false;
    map_villes.insert(ville->nom(), ville);
    map_codespostaux.insert(ville->codePostal(), ville);
    return true;
}

void Villes::addList(QList<Ville*> listvilles)
{
    QList<Ville*>::const_iterator it;
    for( it = listvilles.constBegin(); it != listvilles.constEnd(); ++it )
    {
        Ville* ville = const_cast<Ville*>(*it);
        add( ville );
    }
}



QStringList Villes::ListeVilles()
{
   if( m_listeNomVilles.isEmpty() )
        m_listeNomVilles = QStringList(map_villes.uniqueKeys());

    return m_listeNomVilles;
}
QStringList Villes::ListeCodesPostaux()
{
    if( m_listeCodePostal.isEmpty() )
        m_listeCodePostal = QStringList(map_codespostaux.uniqueKeys());

    return m_listeCodePostal;
}

QList<Ville *> Villes::getVilleByCodePostal(QString codePostal, bool testIntegrite)
{
    QMultiMap<QString, Ville*>::const_iterator it = map_codespostaux.constFind( codePostal );
    QJsonObject error{};
    if( testIntegrite && (it == map_codespostaux.constEnd()) )
    {
        error["errorCode"] = 1;
        error["errorMessage"] = QObject::tr("Code postal inconnu");
        throw error;
    }

    QList<Ville *> listV;
    while( it != map_codespostaux.cend() && it.key() == codePostal)
    {
        listV << it.value();
        ++it;
    }

    if( listV.isEmpty() )
    {
        error["errorCode"] = 2;
        error["errorMessage"] = QObject::tr("Impossible de trouver la ville correspondant au code postal ") + codePostal;
        throw error;
    }

    return listV;
}
QList<Ville *> Villes::getVilleByName(QString name, bool distinct)
{
    QList<QString> listVName; //Permet de tester si le nom d'une ville est déjà présente.
    QList<Ville *> listV;
    QList<Ville *> listVStartWith;
    QMultiMap<QString, Ville*>::const_iterator it = map_villes.constBegin();
    while( it != map_villes.constEnd() )
    {
        if( it.value()->nom() == name )
        {
            if( !distinct || (distinct && !listVName.contains(it.value()->nom())) )
            {
                listVName << it.value()->nom();
                listV << it.value();
            }
        }

        if( it.value()->nom().startsWith(name))
        {
            if( !distinct || (distinct && !listVName.contains(it.value()->nom())) )
            {
                listVName << it.value()->nom();
                listVStartWith << it.value();
            }
        }
        //ajouter test dans le cas ou : accents, espace, trait union
        ++it;
    }
    if( listV.isEmpty() && listVStartWith.size() )
        return listVStartWith;

    return listV;
}
QList<Ville *> Villes::getVilleByCodePostalEtNom(QString codePostal, QString name)
{
    QList<Ville *> listV;
    QJsonObject error{};
    /*
    if( testIntegrite && !m_listeCodePostal.contains(QString::number(codePostal)) )
    {
        error["errorCode"] = 1;
        error["errorMessage"] = QObject::tr("Code postal inconnu");
        throw error;
    }
    */

    QMultiMap<QString, Ville*>::const_iterator it = map_codespostaux.constFind( codePostal );
    while( it != map_codespostaux.cend() && it.key() == codePostal)
    {
        if( it.value()->nom() == name )
            listV << it.value();
        ++it;
    }

    if( listV.isEmpty() )
    {
        error["errorCode"] = 2;
        error["errorMessage"] = QObject::tr("Impossible de trouver la ville correspondant au nom ") + name;
        throw error;
    }

    return listV;
}


