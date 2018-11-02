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

    users           = new Users();
    patients        = new Patients();
    correspondants  = new Correspondants();
    depenses        = new Depenses();
    comptesallusers = new Comptes();
}


