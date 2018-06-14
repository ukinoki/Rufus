#ifndef CLS_PATIENT_H
#define CLS_PATIENT_H

#include "cls_item.h"

#include <QObject>

class Patient : public Item
{
    Q_OBJECT

public:
    explicit Patient(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);
};

#endif // CLS_PATIENT_H
