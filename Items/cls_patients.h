#ifndef CLS_PATIENTS_H
#define CLS_PATIENTS_H

#include <QObject>
#include <cls_patient.h>

class Patients
{
    QMap<int, Patient*> *m_patients; //!<Collection de tous les Patient

public:
    //GETTER
    QMap<int, Patient *> *getPatients() const;


    Patients();

    bool addPatient(Patient *patient);
    Patient* getPatientById(int id);
};

#endif // CLS_PATIENTS_H
