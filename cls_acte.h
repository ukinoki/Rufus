#ifndef CLS_ACTE_H
#define CLS_ACTE_H

#include <QDate>
#include <QJsonObject>
#include "cls_item.h"



class Acte : public Item
{
    Q_OBJECT

private:
    int m_id;
    int m_nbActes;
    int m_noActe;
    int m_idActeMin;
    int m_idActeMax;
    int m_idCreatedBy;
    int m_idPatient;

    double m_montant;

    QString m_motif;
    QString m_texte;
    QString m_conclusion;
    QString m_courrierStatus;
    QString m_cotation;
    QString m_monnaie;
    QString m_paiementType;
    QString m_paiementTiers;

    QDateTime m_date;
    QDateTime m_agePatient; //TODO : MOVE to user



public:
    Acte(int idActe, int nbActe, int noActe, QObject *parent = nullptr);
    void setData(QJsonObject data);

    bool isValid();
    bool courrierAFaire();
    bool isPayeEnFranc() const;

    /* GETTER / SETTER*/
    int id() const;
    int nbActes() const;
    int noActe() const;
    QDate date() const;
    QString motif() const;
    QString texte() const;
    QString conclusion() const;
    QString courrierStatus() const;
    int idCreatedBy() const;
    int idPatient() const;
    QDate agePatient() const;
    QString cotation() const;
    double montant() const;
    QString paiementType() const;
    QString paiementTiers() const;
};

#endif // CLS_ACTE_H
