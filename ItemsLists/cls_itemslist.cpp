#include "cls_itemslist.h"

ItemsList::ItemsList(QObject *parent) : QObject(parent)
{

}

bool ItemsList::update(Item* item, QString field, QVariant newvalue)
{
    QString table;
    QString value;
    QString nomfieldid;
    DocExterne * doc = Q_NULLPTR;
    Acte* act = Q_NULLPTR;
    PatientEnCours* patcrs = Q_NULLPTR;
    Patient* pat = Q_NULLPTR;

    bool ok = false;
    bool loop = false;
    while (!loop)
    {
        doc = dynamic_cast<DocExterne*>(item);
        if (doc != Q_NULLPTR)
        {
            table = TBL_IMPRESSIONS;
            loop = true;
            break;
        }
        act = dynamic_cast<Acte*>(item);
        if (act != Q_NULLPTR)
        {
            table = TBL_ACTES;
            loop = true;
            break;
        }
        patcrs = dynamic_cast<PatientEnCours*>(item);
        if (patcrs != Q_NULLPTR)
        {
            table = TBL_SALLEDATTENTE;
            loop = true;
            break;
        }
        pat = dynamic_cast<Patient*>(item);
        if (pat != Q_NULLPTR)
        {
            table = TBL_SALLEDATTENTE;
            loop = true;
            return false;
        }
        return false;
    }

    if (table == TBL_IMPRESSIONS)
    {
        ok = true;
        nomfieldid = CP_IDIMPRESSION_IMPRESSIONS;
        if (field == CP_SOUSTYPEDOC_IMPRESSIONS )
        {
            doc->setsoustype(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IMPORTANCE_IMPRESSIONS )
        {
            doc->setimportance(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());
        }
        else if (field == CP_DATE_IMPRESSIONS )
        {
            doc->setDate(newvalue.toDateTime());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toDateTime().toString("yyyy-MM-dd HH:mm:ss") + "'");
        }
    }
    else if (table == TBL_ACTES)
    {
        ok = true;
        nomfieldid = CP_IDACTE_ACTES;
        if (field == CP_MOTIF_ACTES)
        {
            act->setmotif(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_TEXTE_ACTES)
        {
            act->settexte(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_CONCLUSION_ACTES)
        {
            act->setconclusion(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_ACTEDATE_ACTES)
        {
            act->setdate(newvalue.toDate());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toDate().toString("yyyy-MM-dd") + "'");
        }
        else if (field == CP_COURRIERAFAIRE_ACTES)
        {
            act->setcourrierafaire(newvalue.toString()== "T" || newvalue.toString()== "1");
            value = ((newvalue.toString() == "T" || newvalue.toString()== "1")? "'T'" : "null");
        }
        else if (field == CP_IDUSER_ACTES)
        {
            act->setiduser(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_IDUSERPARENT_ACTES)
        {
            act->setiduserparent(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_IDUSERCOMPTABLE_ACTES)
        {
            act->setidusercomptable(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());
        }
    }
    else if (table == TBL_SALLEDATTENTE)
    {
        ok = true;
        nomfieldid = CP_IDPAT_SALDAT;
        if (field == CP_IDPAT_SALDAT)
        {
            patcrs->setid(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_IDUSER_SALDAT)
        {
            patcrs->setiduser(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_STATUT_SALDAT)
        {
            patcrs->setstatut(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_HEURESTATUT_SALDAT)
        {
            patcrs->setheurestatut(newvalue.toTime());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");
        }
        else if (field == CP_HEUREARRIVEE_SALDAT)
        {
            patcrs->setheurerarrivee(newvalue.toTime());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");

        }
        else if (field == CP_HEURERDV_SALDAT)
        {
            patcrs->setheurerdv(newvalue.toTime());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");

        }
        else if (field == CP_MOTIF_SALDAT)
        {
            patcrs->setmotif(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_MESSAGE_SALDAT)
        {
            patcrs->setmessage(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IDACTEAPAYER_SALDAT)
        {
            patcrs->setidacteapayer(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_POSTEEXAMEN_SALDAT)
        {
            patcrs->setposteexamen(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IDUSERENCOURSEXAM_SALDAT)
        {
            patcrs->setiduserencoursexam(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());
        }
    }
    if (ok)
    {
        QString req = "update " + table + " set " + field + " = " + value + " where " + nomfieldid + " = " + QString::number(item->id());
        //qDebug() << req;
        DataBase::I()->StandardSQL(req);
    }
    return ok;
}
