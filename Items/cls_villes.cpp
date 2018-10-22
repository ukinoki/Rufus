#include "cls_villes.h"

#include <QException>

/*
 * Ville
*/
Ville::Ville(QJsonObject data, QObject *parent) : Item(parent)
{
    setData( data );
}

int Ville::id() const { return m_id; }
QString Ville::codePostal() const { return m_codePostal; }
QString Ville::nom() const { return m_nom; }

void Ville::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "ville_id", m_id);
    setDataString(data, "codePostal", m_codePostal);
    setDataString(data, "ville", m_nom);
}


/*
 * VilleListModel
*/
VilleListModel::VilleListModel(const QList<Ville*> &villes, QString fieldName, QObject *parent) : QAbstractListModel(parent)
{
    m_villes = villes;
    setFieldName(fieldName);
}

void VilleListModel::setFieldName(QString fieldName)
{
    m_fieldName = fieldName;

    if( m_fieldName.toLower() == "nom" )
        std::sort( m_villes.begin(), m_villes.end(), VilleListModel::sortByName );

    if( m_fieldName.toLower() == "codepostal" )
        std::sort( m_villes.begin(), m_villes.end(), VilleListModel::sortByCodePostal );
}
int VilleListModel::rowCount(const QModelIndex& parent) const { Q_UNUSED(parent); return m_villes.size(); }
QVariant VilleListModel::data(const QModelIndex& index, int role) const
{
    // Check that the index is valid and within the correct range first:
    if( !index.isValid()
            || index.row() >= m_villes.size()
            || role != Qt::DisplayRole )
        return QVariant();

    if( m_fieldName.toLower() == "nom" )
        return QString(m_villes.at(index.row())->nom());

    if( m_fieldName.toLower() == "codepostal" )
        return QString(m_villes.at(index.row())->codePostal());

    return QVariant();
}


/*
 * Villes
*/
Villes::Villes()
{}

void Villes::addVille(Ville *ville)
{
    /*
    if( m_villes.contains(ville->id()) )
        return;
    */
    m_villes.insert(ville->nom(), ville);
    m_codePostal.insert(ville->codePostal(), ville);
}

QStringList Villes::getListVilles()
{
   if( m_listeNomVilles.isEmpty() )
        m_listeNomVilles = QStringList(m_villes.uniqueKeys());

    return m_listeNomVilles;
}
QStringList Villes::getListCodePostal()
{
    if( m_listeCodePostal.isEmpty() )
        m_listeCodePostal = QStringList(m_codePostal.uniqueKeys());

    return m_listeCodePostal;
}

QList<Ville *> Villes::getVilleByCodePostal(QString codePostal, bool testIntegrite)
{
    QMap<QString, Ville*>::const_iterator it = m_codePostal.find( codePostal );
    QJsonObject error{};
    if( testIntegrite && (it == m_codePostal.constEnd()) )
    {
        error["errorCode"] = 1;
        error["errorMessage"] = QObject::tr("Code postal inconnu");
        throw error;
    }

    QList<Ville *> listV;
    while( it != m_codePostal.end() && it.key() == codePostal)
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
    QMap<QString, Ville*>::const_iterator it = m_villes.constBegin();
    while( it != m_villes.constEnd() )
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

    QMap<QString, Ville*>::const_iterator it = m_codePostal.find( codePostal );
    while( it != m_codePostal.end() && it.key() == codePostal)
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


