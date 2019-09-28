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
    m_useTCP = false;
    m_ok  = true;
}

Flags::~Flags()
{
}

void Flags::setTCP(bool tcp)
{
    m_useTCP = tcp;
}

int Flags::flagCorrespondants() const
{
    bool ok;
    QVariantList flag = DataBase::I()->getFirstRecordFromStandardSelectSQL("select " CP_MAJFLAGCORRESPONDANTS_FLAGS " from " TBL_FLAGS, ok);
    if (m_ok && flag.size() > 0)
        return flag.at(0).toInt();
    return 0;
}

int Flags::flagMessages() const
{
    bool ok;
    QVariantList flag = DataBase::I()->getFirstRecordFromStandardSelectSQL("select " CP_MAJFLAGMESSAGES_FLAGS " from " TBL_FLAGS, ok);
    if (m_ok && flag.size() > 0)
        return flag.at(0).toInt();
    return 0;
}

int Flags::flagSalleDAttente() const
{
    bool ok;
    QVariantList flag = DataBase::I()->getFirstRecordFromStandardSelectSQL("select " CP_MAJFLAGSALDAT_FLAGS " from " TBL_FLAGS, ok);
    if (m_ok && flag.size() > 0)
        return flag.at(0).toInt();
    return 0;
}

int Flags::flagUserDistant() const
{
    bool ok;
    QVariantList flag = DataBase::I()->getFirstRecordFromStandardSelectSQL("select " CP_MAJFLAGUSERDISTANT_FLAGS " from " TBL_FLAGS, ok);
    if (m_ok && flag.size() > 0)
        return flag.at(0).toInt();
    return 0;
}

void Flags::MAJflagCorrespondants()
{
    int a = 1;
    /* mise à jour du flag en cas de non utilisation du TCP ou pour les utilisateurs distants qui le surveillent et mettent ainsi à jour leur liste de correspondants  */
    QString MAJreq = "insert into " TBL_FLAGS " (" CP_MAJFLAGCORRESPONDANTS_FLAGS ") VALUES (1)";
    QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagMG from " TBL_FLAGS, m_ok);
    if (m_ok && flagdata.size()>0) {
        a = flagdata.at(0).toInt() + 1;
        MAJreq = "update " TBL_FLAGS " set " CP_MAJFLAGCORRESPONDANTS_FLAGS " = " + QString::number(a);
    }
    DataBase::I()->StandardSQL(MAJreq);
    /* envoi du signal de MAJ de la liste des correspondants */
    emit UpdCorrespondants(a);
}

void Flags::MAJFlagSalleDAttente()
{
    int a = 1;
    /* mise à jour du flag en cas de non utilisation du TCP ou pour les utilisateurs distants qui le surveillent et mettent ainsi à jour leur salle d'attente  */
    QString MAJreq = "insert into " TBL_FLAGS " (" CP_MAJFLAGSALDAT_FLAGS ") VALUES (1)";
    QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagSalDat from " TBL_FLAGS, m_ok);
    if (m_ok && flagdata.size()>0) {
        a = flagdata.at(0).toInt() + 1;
        MAJreq = "update " TBL_FLAGS " set " CP_MAJFLAGSALDAT_FLAGS " = " + QString::number(a);
    }
    DataBase::I()->StandardSQL(MAJreq);
    /* envoi du signal de MAJ de la salle d'attente */
    emit UpdSalleDAttente(a);
}

void Flags::MAJflagMessages()
{
    int a = 1;
    /* mise à jour du flag en cas de non utilisation du TCP ou pour les utilisateurs distants qui le surveillent et mettent ainsi à jour leur liste de messages  */
    QString MAJreq = "insert into " TBL_FLAGS " (" CP_MAJFLAGMESSAGES_FLAGS ") VALUES (1)";
    QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagMessages from " TBL_FLAGS, m_ok);
    if (m_ok && flagdata.size()>0) {
        a = flagdata.at(0).toInt() + 1;
        MAJreq = "update " TBL_FLAGS " set " CP_MAJFLAGMESSAGES_FLAGS " = " + QString::number(a);
    }
    DataBase::I()->StandardSQL(MAJreq);
    /* envoi du signal de MAJ de la messagerie */
    emit UpdMessages(a);
}

void Flags::MAJflagUserDistant()
{
    int a = 1;
    /* mise à jour du flag en cas de connection d'un utilisateur distant -  surveillé par RufusAdmin  */
    QString MAJreq = "insert into " TBL_FLAGS " (" CP_MAJFLAGUSERDISTANT_FLAGS ") VALUES (1)";
    QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select MAJflagMessages from " TBL_FLAGS, m_ok);
    if (m_ok && flagdata.size()>0) {
        a = flagdata.at(0).toInt() + 1;
        MAJreq = "update " TBL_FLAGS " set " CP_MAJFLAGUSERDISTANT_FLAGS " = " + QString::number(a);
    }
    DataBase::I()->StandardSQL(MAJreq);
}
