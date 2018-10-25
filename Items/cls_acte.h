#ifndef CLS_ACTE_H
#define CLS_ACTE_H

#include <QDate>
#include <QJsonObject>
#include "cls_item.h"


/*!
 * \brief The Acte class
 * l'ensemble des informations concernant un acte
 */

class Acte : public Item
{
    Q_OBJECT

private:
    int m_id;                   //!< Id de l'acte en base
    int m_nbActes;              //!< nombres d'acte total
    int m_noActe;               //!< numero de l'acte
    int m_idActeMin;            //!< id du premier acte
    int m_idActeMax;            //!< id du dernier acte
    int m_idCreatedBy;          //!< id du User qui a créé l'acte
    int m_idPatient;            //!< id du Patient correspondant à l'acte
    int m_idUser;               //!< id du User
    //TODO : ??? : différence idUser et idCreatedBy
    //-> reponse = le user est le responsable médical de l'acte - le idcreatedby est le user qui a créé l'acte et c'est parfois différent en cas de travail avec un assistant

    double m_montant;           //!< montant total

    QString m_motif;            //!< motif
    QString m_texte;            //!< détail
    QString m_conclusion;       //!< conclusion
    QString m_courrierStatus;   //!< status du courrier //TODO : à compléter
    QString m_cotation;         //!< //TODO : à compléter : je ne sais pas
    QString m_monnaie;          //!< en franc "F" ou en euro
    QString m_paiementType;     //!< moyen de paiement
    QString m_paiementTiers;    //!< //TODO : à compléter : je ne sais pas

    QDateTime m_date;           //!< date de la création de l'acte
    QDateTime m_agePatient;     //!< date de naissance du Patient //TODO : MOVE to Patient



public:
    Acte(QObject *parent = nullptr);
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
    int idUser() const;
};

#endif // CLS_ACTE_H
