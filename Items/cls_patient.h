#ifndef CLS_PATIENT_H
#define CLS_PATIENT_H

#include "cls_item.h"
#include "cls_acte.h"

#include <QObject>

/*!
 * \brief The Patient class
 * l'ensemble des informations concernant une patient
 */

class Patient : public Item
{
    Q_OBJECT

private: //Données du patient
    int m_id;                       //!< Id du patient en base
    QString m_nom;                  //!< Nom du patient
    QString m_prenom;               //!< Prénom du patient
    QString m_sexe;                 //!< Sexe du patient

    QDateTime m_dateDeNaissance;    //!< Date de naissance du patient



private:
    bool m_isAllLoaded = false;

    QMap<int, Acte*> *m_actes;      //!< ensemble des actes du patient

public:
    //GETTER | SETTER
    bool isAllLoaded() const;

    int id() const;
    QString nom() const;
    QString prenom() const;
    QString sexe() const;

    void setActes(QMap<int, Acte *> *actes);
    QMap<int, Acte *> *actes() const;



    explicit Patient(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);
    void addActe(Acte *acte);

};

#endif // CLS_PATIENT_H
