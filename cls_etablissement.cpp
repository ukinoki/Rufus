#include "cls_etablissement.h"

Etablissement::Etablissement(QJsonObject data, QObject *parent) : Item(parent)
{
    setData( data );
}

void Etablissement::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;


    setDataInt(data, "idLieu", m_id);
    setDataString(data, "nomLieu", m_nom);
    setDataString(data, "adresse1", m_adresse1);
    setDataString(data, "adresse2", m_adresse2);
    setDataString(data, "adresse3", m_adresse3);
    setDataInt(data, "codePostal", m_codePostal);
    setDataString(data, "ville", m_ville);
    setDataString(data, "telephone", m_telephone);
    setDataString(data, "fax", m_fax);
}


int Etablissement::getId() const { return m_id; }
QString Etablissement::getNom() const { return m_nom; }
QString Etablissement::getAdresse1() const { return m_adresse1; }
QString Etablissement::getAdresse2() const { return m_adresse2; }
QString Etablissement::getAdresse3() const { return m_adresse3; }
int Etablissement::getCodePostal() const { return m_codePostal; }
QString Etablissement::getVille() const { return m_ville; }
QString Etablissement::getTelephone() const { return m_telephone; }
