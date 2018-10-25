#include "cls_depenses.h"

/*
 * GETTER
*/
QMap<int, Depense *> *Depenses::getDepenses() const
{
    return m_Depenses;
}


/*!
 * \brief Depenses::Depenses
 * Initialise la map Depenses
 */
Depenses::Depenses(QObject *parent) : QObject (parent)
{
    m_Depenses = new QMap<int, Depense*>();
}

/*!
 * \brief Depenses::addDepense
 * Cette fonction va ajouter le Depense passé en paramètre
 *
 * \param Depense le Depense que l'on veut ajouter
 * \return true si le Depense est ajouté
 * \return false si le paramètre Depense est un nullptr
 * \return false si le Depense est déjà présent
 */
bool Depenses::addDepense(Depense *Depense)
{
    if( Depense == Q_NULLPTR)
        return false;

    if( m_Depenses->contains(Depense->id()) )
        return false;

    m_Depenses->insert(Depense->id(), Depense);

    return true;
}

/*!
 * \brief Depenses::getDepenseById
 * \param id l'id du Depense recherché
 * \return nullptr si aucun Depense trouvé
 * \return Depense* le Depense Depense à l'id
 */
Depense* Depenses::getDepenseById(int id)
{
    QMap<int, Depense*>::const_iterator Depense = m_Depenses->find(id);
    if( Depense == m_Depenses->constEnd() )
        return Q_NULLPTR;
    return Depense.value();
}
