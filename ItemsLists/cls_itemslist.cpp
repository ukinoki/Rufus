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
    PatientEnCours* pat = Q_NULLPTR;

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
        pat = dynamic_cast<PatientEnCours*>(item);
        if (pat != Q_NULLPTR)
        {
            table = TBL_SALLEDATTENTE;
            loop = true;
            break;
        }
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
            pat->setid(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_IDUSER_SALDAT)
        {
            pat->setiduser(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_STATUT_SALDAT)
        {
            pat->setstatut(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_HEURESTATUT_SALDAT)
        {
            pat->setheurestatut(newvalue.toTime());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");
        }
        else if (field == CP_HEUREARRIVEE_SALDAT)
        {
            pat->setheurerarrivee(newvalue.toTime());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");

        }
        else if (field == CP_HEURERDV_SALDAT)
        {
            pat->setheurerdv(newvalue.toTime());
            value = (newvalue == QVariant()? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");

        }
        else if (field == CP_MOTIF_SALDAT)
        {
            pat->setmotif(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_MESSAGE_SALDAT)
        {
            pat->setmessage(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IDACTEAPAYER_SALDAT)
        {
            pat->setidacteapayer(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());

        }
        else if (field == CP_POSTEEXAMEN_SALDAT)
        {
            pat->setposteexamen(newvalue.toString());
            value = (newvalue == QVariant()? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IDUSERENCOURSEXAM_SALDAT)
        {
            pat->setiduserencoursexam(newvalue.toInt());
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
