#ifndef GBL_DATAS_H
#define GBL_DATAS_H

#include <QObject>
#include <cls_users.h>
#include <cls_correspondants.h>
#include <cls_patients.h>
#include <cls_depenses.h>

class Datas : public QObject
{
    Q_OBJECT
private:
    Datas();
    static Datas *instance;

public:
    static Datas *I();

    User *userConnected;
    Users *users;                       //!< Les users
    Patients *patients;                 //!< Les patients
    Correspondants *correspondants;     //!< Les correspondants
    Depenses *depenses;               //!< les depenses pour un user et une  année donnée


signals:


public slots:
};

#endif // GBL_DATAS_H
