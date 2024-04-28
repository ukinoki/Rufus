#include "cls_ville.h"

/*
 * Ville
*/
void Ville::setcodepostal(const QString &codePostal)
{
    m_codePostal = codePostal;
}

void Ville::setnom(const QString &nom)
{
    m_nom = nom;
}

Ville::Ville(QJsonObject data, QObject *parent) : Item(parent)
{
    setData( data );
}

QString Ville::codepostal() const { return m_codePostal; }
QString Ville::nom() const { return m_nom; }

void Ville::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
    setDataInt(data, CP_ID_VILLES, m_id);
    setDataString(data, CP_CP_VILLES, m_codePostal);
    setDataString(data, CP_NOM_VILLES, m_nom);
    m_data = data;
}


