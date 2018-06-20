#include "cls_acte.h"

int Acte::id() const
{
    return m_id;
}

int Acte::nbActes() const
{
    return m_nbActes;
}
int Acte::noActe() const
{
    return m_noActe;
}
QDate Acte::date() const
{
    return m_date.date();
}
QString Acte::motif() const
{
    return m_motif;
}
QString Acte::texte() const
{
    return m_texte;
}
QString Acte::conclusion() const
{
    return m_conclusion;
}
QString Acte::courrierStatus() const
{
    return m_courrierStatus;
}
int Acte::idCreatedBy() const
{
    return m_idCreatedBy;
}
int Acte::idPatient() const
{
    return m_idPatient;
}
QDate Acte::agePatient() const
{
    return m_agePatient.date();
}
QString Acte::cotation() const
{
    return m_cotation;
}
double Acte::montant() const
{
    if( isPayeEnFranc() )
        return m_montant / 6.55957;
    return m_montant;
}
QString Acte::paiementType() const
{
    return m_paiementType;
}
QString Acte::paiementTiers() const
{
    return m_paiementTiers;
}
int Acte::idUser() const
{
    return m_idUser;
}


Acte::Acte(QObject *parent) : Item(parent)
{

}
Acte::Acte(int idActe, int nbActe, int noActe, QObject *parent) : Item(parent), m_id(idActe), m_nbActes(nbActe), m_noActe(noActe)
{

}

void Acte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id", m_id);
    setDataInt(data, "nbActes", m_nbActes);
    setDataInt(data, "noActe", m_noActe);
    setDataInt(data, "idActeMin", m_idActeMin);
    setDataInt(data, "idActeMax", m_idActeMax);
    setDataInt(data, "idCreatedBy", m_idCreatedBy);
    setDataInt(data, "idPatient", m_idPatient);
    setDataInt(data, "idUser", m_idUser);

    setDataDouble(data, "montant", m_montant);

    setDataString(data, "motif", m_motif, true);
    setDataString(data, "texte", m_texte, true);
    setDataString(data, "conclusion", m_conclusion, true);
    setDataString(data, "courrierStatus", m_courrierStatus);
    setDataString(data, "cotation", m_cotation);
    setDataString(data, "monnaie", m_monnaie);
    setDataString(data, "paiementType", m_paiementType);
    setDataString(data, "paiementTiers", m_paiementTiers);

    setDataDateTime(data, "date", m_date);
    setDataDateTime(data, "agePatient", m_agePatient);
}


bool Acte::isValid() { return m_nbActes > 0; }
bool Acte::courrierAFaire() { return m_courrierStatus == "T" || m_courrierStatus == "1"; }
bool Acte::isPayeEnFranc() const { return m_monnaie == "F"; }
