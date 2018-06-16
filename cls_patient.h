#ifndef CLS_PATIENT_H
#define CLS_PATIENT_H

#include "cls_item.h"

#include <QObject>

/*!
 * \brief The Patient class
 * l'ensemble des informations concernant une patient
 */

class Patient : public Item
{
    Q_OBJECT

private:
    int m_id;                   //!< Id du patient en base


public:
    explicit Patient(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);
};

#endif // CLS_PATIENT_H
