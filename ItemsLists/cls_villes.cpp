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

void Villes::initListe(TownsFrom from)
{
    clearAll(m_mapvilles);
    map_villes.clear();
    map_codespostaux.clear();
    m_listeNomVilles.clear();
    m_listeCodePostal.clear();
    QList<Ville*> list = QList<Ville*>();
    switch (from) {
    case DATABASE:      list  = DataBase::I()->loadVilles();        break;
    case CUSTOM:    list  = DataBase::I()->loadAutresVilles();  break;
    }
    addList(list);
}

bool Villes::add(Ville *ville)
{
    if (ville == Q_NULLPTR)
        return false;
    map_villes.insert(ville->nom(), ville);
    map_codespostaux.insert(ville->codePostal(), ville);
    ItemsList::add(m_mapvilles, ville);
    return true;
}

void Villes::addList(QList<Ville*> listvilles)
{
    foreach (Ville* ville, listvilles)
        if (ville != Q_NULLPTR)
            add( ville );
}

void Villes::enregistreNouvelleVille(QString CP, QString nomville)
{
    int id;
    DataBase::I()->EnregistreAutreVille(CP, nomville, id);
    QJsonObject jEtab{};
    jEtab[CP_ID_VILLES] = id;
    jEtab[CP_CP_VILLES] = CP;
    jEtab[CP_NOM_VILLES] = nomville;
    Ville *newville = new Ville(jEtab);
    add(newville);
    m_listeCodePostal = QStringList(map_codespostaux.uniqueKeys());
    m_listeNomVilles = QStringList(map_villes.uniqueKeys());
}

QStringList Villes::ListeNomsVilles()
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
    QMap<QString, Ville*>::const_iterator it = map_codespostaux.constFind( codePostal );
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
    QMap<QString, Ville*>::const_iterator it = map_villes.constBegin();
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

    QMap<QString, Ville*>::const_iterator it = map_codespostaux.constFind( codePostal );
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


