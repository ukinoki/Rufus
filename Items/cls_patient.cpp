#include "cls_patient.h"

//GETTER | SETTER
bool Patient::isAllLoaded() const
{
    return m_isAllLoaded;
}

int Patient::id() const
{
    return m_id;
}
QString Patient::nom() const
{
    return m_nom;
}
QString Patient::prenom() const
{
    return m_prenom;
}
QString Patient::sexe() const
{
    return m_sexe;
}


QMap<int, Acte *> *Patient::actes() const
{
    return m_actes;
}
void Patient::setActes(QMap<int, Acte *> *actes)
{
    m_actes = actes;
}




Patient::Patient(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Patient::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataBool(data, "isAllLoaded", m_isAllLoaded);

    setDataInt(data, "id", m_id);

    setDataString(data, "nom", m_nom);
    setDataString(data, "prenom", m_prenom);
    setDataString(data, "sexe", m_sexe);

    setDataDateTime(data, "dateDeNaissance", m_dateDeNaissance);
}

/*!
 * \brief Patient::addActe
 * ajout un acte au patient
 * \param acte
*/
void Patient::addActe(Acte *acte)
{
    if( m_actes->contains(acte->id()) )
        return;
    m_actes->insert(acte->id(), acte);
}
