#include "gbl_datas.h"

Datas* Datas::instance =  Q_NULLPTR;
Datas* Datas::I()
{
    if( !instance )
        instance = new Datas();

    return instance;
}
Datas::Datas()
{
    userConnected = Q_NULLPTR;

    banques         = new Banques();
    comptes         = new Comptes();
    correspondants  = new Correspondants();
    cotations       = new Cotations();
    depenses        = new Depenses();
    motifs          = new Motifs();
    patients        = new Patients();
    tiers           = new TiersPayants();
    typestiers      = new TypesTiers();
    users           = new Users();
}


