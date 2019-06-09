#include "cls_itemslist.h"

ItemsList::ItemsList(QObject *parent) : QObject(parent)
{

}

bool ItemsList::update(Item* item, QString field, QVariant newvalue)
{
    if (item == Q_NULLPTR)
        return false;
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
            if (field == CP_NOM_PATIENTS
                    || field == CP_PRENOM_PATIENTS
                    || field == CP_DDN_PATIENTS
                    || field == CP_SEXE_PATIENTS
                    || field == CP_DATECREATION_PATIENTS
                    || field == CP_IDCREATEUR_PATIENTS
                    || field == CP_IDLIEU_PATIENTS
                    || field == CP_COMMENTAIRE_PATIENTS)
                table = TBL_PATIENTS;
            else if(field == CP_ADRESSE1_DSP
                    || field == CP_ADRESSE2_DSP
                    || field == CP_ADRESSE3_DSP
                    || field == CP_CODEPOSTAL_DSP
                    || field == CP_VILLE_DSP
                    || field == CP_TELEPHONE_DSP
                    || field == CP_PORTABLE_DSP
                    || field == CP_MAIL_DSP
                    || field == CP_NNI_DSP
                    || field == CP_ALD_DSP
                    || field == CP_CMU_DSP
                    || field == CP_PROFESSION_DSP)
                table = TBL_DONNEESSOCIALESPATIENTS;
            else if (field == CP_ATCDTSOPH_RMP
                     || field == CP_TRAITMTOPH_RMP
                     || field == CP_IMPORTANT_RMP
                     || field == CP_RESUME_RMP
                     || field == CP_ATCDTSPERSOS_RMP
                     || field == CP_ATCDTSFAMLXS_RMP
                     || field == CP_TRAITMTGEN_RMP
                     || field == CP_AUTRESTOXIQUES_RMP
                     || field == CP_IDMG_RMP
                     || field == CP_IDSPE1_RMP
                     || field == CP_IDSPE2_RMP
                     || field == CP_TABAC_RMP)
                table = TBL_RENSEIGNEMENTSMEDICAUXPATIENTS;
            loop = false;
            break;;
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
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IMPORTANCE_IMPRESSIONS )
        {
            doc->setimportance(newvalue.toInt());
            value = (newvalue == QVariant()? "null" : newvalue.toString());
        }
        else if (field == CP_DATE_IMPRESSIONS )
        {
            doc->setDate(newvalue.toDateTime());
            value = ((newvalue == QVariant() || !newvalue.toDate().isValid())? "null" : "'" + newvalue.toDateTime().toString("yyyy-MM-dd HH:mm:ss") + "'");
        }
        else
            ok = false;
    }
    else if (table == TBL_ACTES)
    {
        ok = true;
        nomfieldid = CP_IDACTE_ACTES;
        if (field == CP_MOTIF_ACTES)
        {
            act->setmotif(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_TEXTE_ACTES)
        {
            act->settexte(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_CONCLUSION_ACTES)
        {
            act->setconclusion(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_DATE_ACTES)
        {
            act->setdate(newvalue.toDate());
            value = ((newvalue == QVariant() || !newvalue.toDate().isValid())? "null" : "'" + newvalue.toDate().toString("yyyy-MM-dd") + "'");
        }
        else if (field == CP_COURRIERAFAIRE_ACTES)
        {
            act->setcourrierafaire(newvalue.toString()== "T" || newvalue.toString()== "1");
            value = ((newvalue.toString() == "T" || newvalue.toString()== "1")? "'T'" : "null");
        }
        else if (field == CP_IDUSER_ACTES)
        {
            act->setiduser(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());

        }
        else if (field == CP_IDUSERPARENT_ACTES)
        {
            act->setiduserparent(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());

        }
        else if (field == CP_IDUSERCOMPTABLE_ACTES)
        {
            act->setidusercomptable(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());
        }
        else
            ok = false;
    }
    else if (table == TBL_SALLEDATTENTE)
    {
        ok = true;
        nomfieldid = CP_IDPAT_SALDAT;
        if (field == CP_IDPAT_SALDAT)
        {
            patcrs->setid(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());

        }
        else if (field == CP_IDUSER_SALDAT)
        {
            patcrs->setiduser(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());

        }
        else if (field == CP_STATUT_SALDAT)
        {
            patcrs->setstatut(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_HEURESTATUT_SALDAT)
        {
            patcrs->setheurestatut(newvalue.toTime());
            value = ((newvalue == QVariant() || !newvalue.toTime().isValid())? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");
        }
        else if (field == CP_HEUREARRIVEE_SALDAT)
        {
            patcrs->setheurearrivee(newvalue.toTime());
            value = ((newvalue == QVariant() || !newvalue.toTime().isValid())? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");

        }
        else if (field == CP_HEURERDV_SALDAT)
        {
            patcrs->setheurerdv(newvalue.toTime());
            value = ((newvalue == QVariant() || !newvalue.toTime().isValid())? "null" : "'" + newvalue.toTime().toString("HH:mm:ss") + "'");

        }
        else if (field == CP_MOTIF_SALDAT)
        {
            patcrs->setmotif(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_MESSAGE_SALDAT)
        {
            patcrs->setmessage(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IDACTEAPAYER_SALDAT)
        {
            patcrs->setidacteapayer(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());

        }
        else if (field == CP_POSTEEXAMEN_SALDAT)
        {
            patcrs->setposteexamen(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IDUSERENCOURSEXAM_SALDAT)
        {
            patcrs->setiduserencoursexam(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());
        }
        else
            ok = false;
    }
    else if (table == TBL_PATIENTS)
    {
        ok = true;
        nomfieldid = CP_IDPAT_PATIENTS;
        if (field == CP_NOM_PATIENTS)
        {
            pat->setnom(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        if (field == CP_PRENOM_PATIENTS)
        {
            pat->setprenom(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_DDN_PATIENTS )
        {
            pat->setdatedenaissance(newvalue.toDate());
            value = ((newvalue == QVariant() || !newvalue.toDate().isValid())? "null" : "'" + newvalue.toDateTime().toString("yyyy-MM-dd") + "'");
        }
        if (field == CP_SEXE_PATIENTS)
        {
            pat->setsexe(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_DATECREATION_PATIENTS )
        {
            pat->setdatecreation(newvalue.toDate());
            value = ((newvalue == QVariant() || !newvalue.toDate().isValid())? "null" : "'" + newvalue.toDateTime().toString("yyyy-MM-dd") + "'");
        }
        else if (field == CP_IDCREATEUR_PATIENTS )
        {
            pat->setidcreateur(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());
        }
        else
            ok = false;
    }
    else if (table == TBL_DONNEESSOCIALESPATIENTS)
    {
        ok = true;
        nomfieldid = CP_IDPAT_DSP;
        if (field == CP_ADRESSE1_DSP )
        {
            pat->setadresse1(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_ADRESSE2_DSP )
        {
            pat->setadresse2(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_ADRESSE3_DSP )
        {
            pat->setadresse3(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_CODEPOSTAL_DSP )
        {
            pat->setcodepostal(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + newvalue.toString() + "'");
        }
        else if (field == CP_VILLE_DSP )
        {
            pat->setville(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_TELEPHONE_DSP )
        {
            pat->settelephone(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + newvalue.toString() + "'");
        }
        else if (field == CP_PORTABLE_DSP )
        {
            pat->setportable(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + newvalue.toString() + "'");
        }
        else if (field == CP_MAIL_DSP )
        {
            pat->setmail(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + newvalue.toString() + "'");
        }
        else if (field == CP_NNI_DSP )
        {
            pat->setNNI(newvalue.toLongLong());
            value = ((newvalue == QVariant() || newvalue.toLongLong() == 0)? "null" : "'" + newvalue.toString() + "'");
        }
        else if (field == CP_ALD_DSP )
        {
            pat->setald(newvalue.toBool());
            value = (newvalue.toBool()? "1" : "null");
        }
        else if (field == CP_CMU_DSP )
        {
            pat->setcmu(newvalue.toBool());
            value = (newvalue.toBool()? "1" : "null");
        }
        else if (field == CP_PROFESSION_DSP )
        {
            pat->setprofession(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else
            ok = false;
    }
    else if (table == TBL_RENSEIGNEMENTSMEDICAUXPATIENTS)
    {
        ok = true;
        nomfieldid = CP_IDPAT_RMP;
        if (field == CP_ATCDTSOPH_RMP )
        {
            pat->setatcdtsoph(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_ATCDTSPERSOS_RMP )
        {
            pat->setatcdtsgen(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_ATCDTSFAMLXS_RMP )
        {
            pat->setatcdtsfam(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_TRAITMTOPH_RMP )
        {
            pat->settraitemntsoph(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_TRAITMTGEN_RMP )
        {
            pat->settraitemntsgen(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_AUTRESTOXIQUES_RMP )
        {
            pat->setautrestoxiques(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IMPORTANT_RMP )
        {
            pat->setimportant(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_RESUME_RMP )
        {
            pat->setresume(newvalue.toString());
            value = ((newvalue == QVariant() || newvalue.toString() == "")? "null" : "'" + Utils::correctquoteSQL(newvalue.toString()) + "'");
        }
        else if (field == CP_IDMG_RMP )
        {
            pat->setmg(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());
        }
        else if (field == CP_IDSPE1_RMP )
        {
            pat->setspe1(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());
        }
        else if (field == CP_IDSPE2_RMP )
        {
            pat->setspe2(newvalue.toInt());
            value = ((newvalue == QVariant() || newvalue.toInt() == 0)? "null" : newvalue.toString());
        }
        else if (field == CP_TABAC_RMP )
        {
            pat->settabac(newvalue.toString());
            value = (newvalue == QVariant()? "null" : newvalue.toString());
        }
        else
            ok = false;
    }

    if (ok)
    {
        QString req = "update " + table + " set " + field + " = " + value + " where " + nomfieldid + " = " + QString::number(item->id());
        //qDebug() << req;
        DataBase::I()->StandardSQL(req);
    }
    return ok;
}
