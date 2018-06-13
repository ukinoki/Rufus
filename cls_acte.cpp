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

Acte::Acte(int idActe, int nbActe, int noActe, QObject *parent) : Item(parent), m_id(idActe), m_nbActes(nbActe), m_noActe(noActe)
{

}

void Acte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, "id");
    setDataInt(data, "nbActes");
    setDataInt(data, "noActe");
    setDataInt(data, "idActeMin");
    setDataInt(data, "idActeMax");
    setDataInt(data, "idCreatedBy");
    setDataInt(data, "idPatient");

    setDataDouble(data, "montant");

    setDataString(data, "motif");
    setDataString(data, "texte");
    setDataString(data, "conclusion");
    setDataString(data, "courrierStatus");
    setDataString(data, "cotation");
    setDataString(data, "monnaie");
    setDataString(data, "paiementType");
    setDataString(data, "paiementTiers");

    setDataDateTime(data, "date");
    setDataDateTime(data, "agePatient");
}


bool Acte::isValid() { return m_nbActes > 0; }
bool Acte::courrierAFaire() { return m_courrierStatus == "T" || m_courrierStatus == "1"; }
bool Acte::isPayeEnFranc() const { return m_monnaie == "F"; }
