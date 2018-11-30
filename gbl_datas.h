#ifndef GBL_DATAS_H
#define GBL_DATAS_H

#include <QObject>
#include <cls_users.h>
#include <cls_correspondants.h>
#include <cls_patients.h>
#include <cls_depenses.h>
#include "cls_banque.h"
#include "cls_compte.h"
#include "cls_paiementsdirects.h"
#include "cls_tiers.h"

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
    Depenses *depenses;                 //!< les depenses pour un user et une  année donnée
    Comptes *comptesallusers;           //!< tous les comptes bancaires de tous les users
    Banques *banques;                   //!< toutes les banques
    TiersPayants *tiers;                //!< tous les tiers payants
    TypesTiers *typestiers;             //!< tous les types tiers payants
    PaiementsDirects *paiementsdirects; //!< tous les paiements en attente d'enregistrement

signals:


public slots:
};

#endif // GBL_DATAS_H
