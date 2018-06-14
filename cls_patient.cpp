#include "cls_patient.h"

Patient::Patient(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void Patient::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;
}
