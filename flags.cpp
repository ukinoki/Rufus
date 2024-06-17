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
    m_ok  = true;
}

Flags::~Flags()
{
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

void Flags::videFlags()
{
    DataBase::I()->StandardSQL("delete from " TBL_FLAGS);
}

void Flags::cleanFlags()
{
    DataBase::I()->locktable(TBL_FLAGS);
    QList<QVariantList> nbrecordslist = DataBase::I()->StandardSelectSQL("Select count(*) from " TBL_FLAGS, m_ok);
    if (m_ok)
        if (nbrecordslist.size()>0)
        {
            int n = nbrecordslist.at(0).at(0).toInt();
            if (n>1)
            {
                int maxsal  = DataBase::I()->selectMaxFromTable(CP_MAJFLAGSALDAT_FLAGS, TBL_FLAGS, m_ok);
                int maxmsg  = DataBase::I()->selectMaxFromTable(CP_MAJFLAGMESSAGES_FLAGS, TBL_FLAGS, m_ok);
                int maxcor  = DataBase::I()->selectMaxFromTable(CP_MAJFLAGCORRESPONDANTS_FLAGS, TBL_FLAGS, m_ok);
                int maxusr  = DataBase::I()->selectMaxFromTable(CP_MAJFLAGUSERDISTANT_FLAGS, TBL_FLAGS, m_ok);
                int maximp  = DataBase::I()->selectMaxFromTable(CP_MAJFLAGIMPRESSIONS_FLAGS, TBL_FLAGS, m_ok);
                QHash<QString, QString> listsets;
                listsets.insert(CP_MAJFLAGSALDAT_FLAGS,         QString::number(maxsal));
                listsets.insert(CP_MAJFLAGMESSAGES_FLAGS,       QString::number(maxmsg));
                listsets.insert(CP_MAJFLAGCORRESPONDANTS_FLAGS, QString::number(maxcor));
                listsets.insert(CP_MAJFLAGUSERDISTANT_FLAGS,    QString::number(maxusr));
                listsets.insert(CP_MAJFLAGIMPRESSIONS_FLAGS,    QString::number(maximp));
                DataBase::I()->StandardSQL("delete from " TBL_FLAGS);
                DataBase::I()->InsertIntoTable(TBL_FLAGS, listsets);
            }
        }
    DataBase::I()->unlocktables();
}

void Flags::MajFlag(Flag flag)
{
    QString field = "";
    switch (flag) {
    case Correspondants:
        field = CP_MAJFLAGCORRESPONDANTS_FLAGS;
        break;
    case SalleDattente:
        field = CP_MAJFLAGSALDAT_FLAGS;
        break;
    case Messages:
        field = CP_MAJFLAGMESSAGES_FLAGS;
        break;
    case UserDistant:
        field = CP_MAJFLAGUSERDISTANT_FLAGS;
        break;
    }
    int a = 1;
    /* mise à jour du flag en cas de non utilisation du TCP ou pour les utilisateurs distants qui le surveillent et mettent ainsi à jour*/
    QString MAJreq = "insert into " TBL_FLAGS " (" + field + ") VALUES (1)";
    DataBase::I()->locktable(TBL_FLAGS);
    QVariantList flagdata = DataBase::I()->getFirstRecordFromStandardSelectSQL("select " + field + " from " TBL_FLAGS, m_ok);
    if (m_ok && flagdata.size()>0) {
        a = flagdata.at(0).toInt() + 1;
        MAJreq = "update " TBL_FLAGS " set " + field + " = " + QString::number(a);
    }
    DataBase::I()->StandardSQL(MAJreq);
    DataBase::I()->unlocktables();
    /* envoi du signal de MAJ */
    switch (flag) {
    case Correspondants:
        emit UpdCorrespondants(a);
        break;
    case SalleDattente:
        emit UpdSalleDAttente(a);
        break;
    case Messages:
        emit UpdMessages(a);
        break;
    case UserDistant:
        break;
    }
}

void Flags::MAJflagCorrespondants()
{
    MajFlag(Correspondants);
}

void Flags::MAJFlagSalleDAttente()
{
    MajFlag(SalleDattente);
}

void Flags::MAJflagMessages()
{
    MajFlag(Messages);
}

void Flags::MAJflagUserDistant()
{
    MajFlag(UserDistant);
}

void Flags::remiseAZeroFlags()
{
    DataBase::I()->StandardSQL("update " TBL_FLAGS " set " CP_MAJFLAGUSERDISTANT_FLAGS " = 0, "  CP_MAJFLAGMESSAGES_FLAGS " = 0, "  CP_MAJFLAGCORRESPONDANTS_FLAGS " = 0, "  CP_MAJFLAGSALDAT_FLAGS " = 0");
}
