#include "cls_site.h"

Site::Site(QJsonObject data, QObject *parent) : Item(parent)
{
    setData( data );
}

void Site::setData(QJsonObject data)
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


int Site::getId() const { return m_id; }
QString Site::getNom() const { return m_nom; }
QString Site::getAdresse1() const { return m_adresse1; }
QString Site::getAdresse2() const { return m_adresse2; }
QString Site::getAdresse3() const { return m_adresse3; }
int Site::getCodePostal() const { return m_codePostal; }
QString Site::getVille() const { return m_ville; }
QString Site::getTelephone() const { return m_telephone; }
