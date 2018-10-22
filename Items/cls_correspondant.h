#ifndef CLS_CORRESPONDANT_H
#define CLS_CORRESPONDANT_H

#include "cls_item.h"

#include <QObject>

/*!
 * \brief The correspondant class
 * l'ensemble des informations concernant un correspondant
 */

class Correspondant : public Item
{
    Q_OBJECT

private: //Données du correspondant
    int m_id;                       //!< Id du correspondant en base
    QString m_nom;                  //!< Nom du correspondant
    QString m_prenom;               //!< Prénom du correspondant
    QString m_sexe;                 //!< Sexe du correspondant
    QString m_metier;               //!< metier du correspondant
    QString m_adresse1;             //!< 1ere ligne de l'adresse du correspondant
    QString m_adresse2;             //!< 2ème ligne de l'adresse du correspondant
    QString m_adresse3;             //!< 3ème ligne de l'adresse du correspondant
    QString m_codepostal;           //!< code postal du correspondant
    QString m_ville;                //!< ville du correspondant
    QString m_telephone;            //!< telephone du correspondant
    bool    m_generaliste;          //!< generaliste?

private:

public:
    //GETTER | SETTER
    bool isMG() const;
    int id() const;
    QString nom() const;
    QString prenom() const;
    QString nomprenom() const;
    QString sexe() const;
    QString adresse1() const;
    QString adresse2() const;
    QString adresse3() const;
    QString adresseComplete() const;
    QString codepostal() const;
    QString ville() const;
    QString telephone() const;
    QString metier() const;

    explicit Correspondant(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);
};

#endif // CLS_CORRESPONDANT_H
