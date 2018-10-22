#include "gbl_datas.h"

Datas* Datas::instance = 0;
Datas* Datas::I()
{
    if( !instance )
        instance = new Datas();

    return instance;
}
Datas::Datas()
{
    userConnected = nullptr;

    users = new Users();
    patients = new Patients();
    correspondants = new Correspondants();
}


