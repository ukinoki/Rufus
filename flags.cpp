#include "flags.h"


Flags* Flags::instance =  Q_NULLPTR;
Flags* Flags::I()
{
    if( !instance )
        instance = new Flags();

    return instance;
}

Flags::Flags(QObject *parent) : QObject (parent)
{
    TCP = false;
    ok  = true;
}

Flags::~Flags()
{
}

void Flags::setTCP(bool tcp)
{
    TCP = tcp;
}

int Flags::flagCorrespondants()
{
    QVariantList flag = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagMG from " TBL_FLAGS, ok);
    if (ok && flag.size() > 0)
        return flag.at(0).toInt();
    return 0;
}

int Flags::flagMessages()
{
    QVariantList flag = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagMessages from " TBL_FLAGS, ok);
    if (ok && flag.size() > 0)
        return flag.at(0).toInt();
    return 0;
}

int Flags::flagSalleDAttente()
{
    QVariantList flag = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagSalDat from " TBL_FLAGS, ok);
    if (ok && flag.size() > 0)
        return flag.at(0).toInt();
    return 0;
}

void Flags::MAJflagCorrespondants()
{
    int a = 1;
    /* mise à jour du flag en cas de non utilisation du TCP ou pour les utilisateurs distants qui le surveillent et mettent ainsi à jour leur liste de correspondants  */
    if (!TCP)
    {
        QString MAJreq = "insert into " TBL_FLAGS " (MAJflagMG) VALUES (1)";
        QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagMG from " TBL_FLAGS, ok);
        if (ok && flagdata.size()>0) {
            a = flagdata.at(0).toInt() + 1;
            MAJreq = "update " TBL_FLAGS " set MAJflagMG = " + QString::number(a);
        }
        DataBase::I()->StandardSQL(MAJreq);
    }
    /* envoi du signal de MAJ de la liste des correspondants */
    emit UpdCorrespondants(a);
}

void Flags::MAJFlagSalleDAttente()
{
    int a = 1;
    /* mise à jour du flag en cas de non utilisation du TCP ou pour les utilisateurs distants qui le surveillent et mettent ainsi à jour leur salle d'attente  */
    if (!TCP)
    {
        QString MAJreq = "insert into " TBL_FLAGS " (MAJflagSalDat) VALUES (1)";
        QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagSalDat from " TBL_FLAGS, ok);
        if (ok && flagdata.size()>0) {
            a = flagdata.at(0).toInt() + 1;
            MAJreq = "update " TBL_FLAGS " set MAJflagSalDat = " + QString::number(a);
        }
        DataBase::I()->StandardSQL(MAJreq);
    }
    /* envoi du signal de MAJ de la salle d'attente */
    emit UpdSalleDAttente(a);
}

void Flags::MAJflagMessages()
{
    int a = 1;
    /* mise à jour du flag en cas de non utilisation du TCP ou pour les utilisateurs distants qui le surveillent et mettent ainsi à jour leur liste de messages  */
    if (!TCP)
    {
        QString MAJreq = "insert into " TBL_FLAGS " (MAJflagMessages) VALUES (1)";
        QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagMessages from " TBL_FLAGS, ok);
        if (ok && flagdata.size()>0) {
            a = flagdata.at(0).toInt() + 1;
            MAJreq = "update " TBL_FLAGS " set MAJflagMessages = " + QString::number(a);
        }
        DataBase::I()->StandardSQL(MAJreq);
    }
    /* envoi du signal de MAJ de la messagerie */
    emit UpdMessages(a);
}

