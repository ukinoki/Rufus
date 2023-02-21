#include "cls_itemslist.h"

ItemsList::ItemsList(QObject *parent) : QObject(parent)
{

}

bool ItemsList::update(Item* item, QString field, QVariant newvalue)
{
    if (item == Q_NULLPTR)
        return false;
    QString table = "";
    QString clause;
    Depense *dep                = Q_NULLPTR;
    DocExterne *doc             = Q_NULLPTR;
    Acte *act                   = Q_NULLPTR;
    PatientEnCours *patcrs      = Q_NULLPTR;
    Patient *pat                = Q_NULLPTR;
    PosteConnecte *post         = Q_NULLPTR;
    Banque *bq                  = Q_NULLPTR;
    User *usr                   = Q_NULLPTR;
    Session *session            = Q_NULLPTR;
    SessionOperatoire *sessionop= Q_NULLPTR;
    Site *sit                   = Q_NULLPTR;
    Manufacturer *man           = Q_NULLPTR;
    TypeIntervention *typinterv = Q_NULLPTR;
    IOL *iol                    = Q_NULLPTR;
    Intervention *interv        = Q_NULLPTR;
    Tiers *tiers                = Q_NULLPTR;
    Commercial *com             = Q_NULLPTR;
    CommentLunet *comment       = Q_NULLPTR;
    MotCle *motcle              = Q_NULLPTR;
    Impression *impr            = Q_NULLPTR;
    DossierImpression *dossier  = Q_NULLPTR;
    Message *msg                = Q_NULLPTR;
    bool ok = false;
    bool loop = false;
    while (!loop)
    {
        doc = dynamic_cast<DocExterne*>(item);
        if (doc)
        {
            table = TBL_DOCSEXTERNES;
            ok = true;
            clause = CP_ID_DOCSEXTERNES " = " + QString::number(item->id());
            if (field == CP_SOUSTYPEDOC_DOCSEXTERNES )
            {
                doc->setsoustype(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IMPORTANCE_DOCSEXTERNES )
            {
                doc->setimportance(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_DATE_DOCSEXTERNES )
            {
                doc->setdate(newvalue.toDateTime());
                Utils::CalcDateTimeValueSQL(newvalue);
            }
            else if (field == CP_IDREFRACTION_DOCSEXTERNES)
            {
                doc->setidrefraction(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else
                ok = false;
            break;
        }
        act = dynamic_cast<Acte*>(item);
        if (act)
        {
            table = TBL_ACTES;
            ok = true;
            clause = CP_ID_ACTES " = " + QString::number(item->id());
            if (field == CP_MOTIF_ACTES)
            {
                act->setmotif(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_TEXTE_ACTES)
            {
                act->settexte(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_CONCLUSION_ACTES)
            {
                act->setconclusion(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_DATE_ACTES)
            {
                act->setdate(newvalue.toDate());
                Utils::CalcDateValueSQL(newvalue);
            }
            else if (field == CP_COURRIERAFAIRE_ACTES)
            {
                act->setcourrierafaire(newvalue.toString()== "T" || newvalue.toString()== "1");
                newvalue = ((newvalue.toString() == "T" || newvalue.toString() == "1")? "'T'" : "null");
            }
            else if (field == CP_IDUSER_ACTES)
            {
                act->setidsuperviseur(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);

            }
            else if (field == CP_IDUSERPARENT_ACTES)
            {
                act->setidparent(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);

            }
            else if (field == CP_IDUSERCOMPTABLE_ACTES)
            {
                act->setidcomptable(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_MONTANT_ACTES)
            {
                act->setmontant(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_COTATION_ACTES)
            {
                act->setcotation(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else
                ok = false;
            break;
        }
        patcrs = dynamic_cast<PatientEnCours*>(item);
        if (patcrs)
        {
            table = TBL_SALLEDATTENTE;
            ok = true;
            clause = CP_IDPAT_SALDAT " = " + QString::number(item->id());
            if (field == CP_IDPAT_SALDAT)
            {
                patcrs->setid(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);

            }
            else if (field == CP_IDUSER_SALDAT)
            {
                patcrs->setiduser(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);

            }
            else if (field == CP_STATUT_SALDAT)
            {
                patcrs->setstatut(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_HEURESTATUT_SALDAT)
            {
                patcrs->setheurestatut(newvalue.toTime());
                Utils::CalcTimeValueSQL(newvalue);
            }
            else if (field == CP_HEUREARRIVEE_SALDAT)
            {
                patcrs->setheurearrivee(newvalue.toTime());
                Utils::CalcTimeValueSQL(newvalue);

            }
            else if (field == CP_HEURERDV_SALDAT)
            {
                patcrs->setheurerdv(newvalue.toTime());
                Utils::CalcTimeValueSQL(newvalue);

            }
            else if (field == CP_MOTIF_SALDAT)
            {
                patcrs->setmotif(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MESSAGE_SALDAT)
            {
                patcrs->setmessage(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDACTEAPAYER_SALDAT)
            {
                patcrs->setidacteapayer(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);

            }
            else if (field == CP_POSTEEXAMEN_SALDAT)
            {
                patcrs->setposteexamen(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDUSERENCOURSEXAM_SALDAT)
            {
                patcrs->setiduserencoursexam(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else
                ok = false;
            break;
        }
        pat = dynamic_cast<Patient*>(item);
        if (pat)
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
            if (table == TBL_PATIENTS)
            {
                ok = true;
                clause = CP_IDPAT_PATIENTS " = " + QString::number(item->id());
                if (field == CP_NOM_PATIENTS)
                {
                    pat->setnom(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_PRENOM_PATIENTS)
                {
                    pat->setprenom(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_DDN_PATIENTS )
                {
                    pat->setdatedenaissance(newvalue.toDate());
                    Utils::CalcDateValueSQL(newvalue);
                }
                else if (field == CP_SEXE_PATIENTS)
                {
                    pat->setsexe(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_DATECREATION_PATIENTS )
                {
                    pat->setdatecreation(newvalue.toDate());
                    Utils::CalcDateValueSQL(newvalue);
                }
                else if (field == CP_IDCREATEUR_PATIENTS )
                {
                    pat->setidcreateur(newvalue.toInt());
                    Utils::CalcintValueSQL(newvalue);
                }
                else
                    ok = false;
            }
            else if (table == TBL_DONNEESSOCIALESPATIENTS)
            {
                ok = true;
                clause = CP_IDPAT_DSP " = " + QString::number(item->id());
                if (field == CP_ADRESSE1_DSP )
                {
                    pat->setadresse1(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_ADRESSE2_DSP )
                {
                    pat->setadresse2(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_ADRESSE3_DSP )
                {
                    pat->setadresse3(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_CODEPOSTAL_DSP )
                {
                    pat->setcodepostal(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_VILLE_DSP )
                {
                    pat->setville(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_TELEPHONE_DSP )
                {
                    pat->settelephone(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_PORTABLE_DSP )
                {
                    pat->setportable(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_MAIL_DSP )
                {
                    pat->setmail(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_NNI_DSP )
                {
                    pat->setNNI(newvalue.toLongLong());
                    newvalue = ((newvalue == QVariant() || newvalue.toLongLong() == 0)? "null" : "'" + newvalue.toString() + "'");
                }
                else if (field == CP_ALD_DSP )
                {
                    pat->setald(newvalue.toBool());
                    newvalue = (newvalue.toBool()? "1" : "null");
                }
                else if (field == CP_CMU_DSP )
                {
                    pat->setcmu(newvalue.toBool());
                    newvalue = (newvalue.toBool()? "1" : "null");
                }
                else if (field == CP_PROFESSION_DSP )
                {
                    pat->setprofession(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else
                    ok = false;
            }
            else if (table == TBL_RENSEIGNEMENTSMEDICAUXPATIENTS)
            {
                ok = true;
                clause = CP_IDPAT_RMP " = " + QString::number(item->id());
                if (field == CP_ATCDTSOPH_RMP )
                {
                    pat->setatcdtsoph(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_ATCDTSPERSOS_RMP )
                {
                    pat->setatcdtsgen(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_ATCDTSFAMLXS_RMP )
                {
                    pat->setatcdtsfam(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_TRAITMTOPH_RMP )
                {
                    pat->settraitemntsoph(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_TRAITMTGEN_RMP )
                {
                    pat->settraitemntsgen(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_AUTRESTOXIQUES_RMP )
                {
                    pat->setautrestoxiques(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_IMPORTANT_RMP )
                {
                    pat->setimportant(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_RESUME_RMP )
                {
                    pat->setresume(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else if (field == CP_IDMG_RMP )
                {
                    pat->setmg(newvalue.toInt());
                    Utils::CalcintValueSQL(newvalue);
                }
                else if (field == CP_IDSPE1_RMP )
                {
                    pat->setspe1(newvalue.toInt());
                    Utils::CalcintValueSQL(newvalue);
                }
                else if (field == CP_IDSPE2_RMP )
                {
                    pat->setspe2(newvalue.toInt());
                    Utils::CalcintValueSQL(newvalue);
                }
                else if (field == CP_TABAC_RMP )
                {
                    pat->settabac(newvalue.toString());
                    Utils::CalcStringValueSQL(newvalue);
                }
                else
                    ok = false;
            }
            break;
        }
        post = dynamic_cast<PosteConnecte*>(item);
        if (post)
        {
            table = TBL_USERSCONNECTES;
            ok = true;
            clause = CP_IDUSER_USRCONNECT " = " + QString::number(item->id()) + " and " CP_MACADRESS_USRCONNECT " = '" + post->macadress() + "'";
            if (field == CP_IDUSER_USRCONNECT )
            {
                post->setid(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_NOMPOSTE_USRCONNECT)
            {
                post->setnomposte(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MACADRESS_USRCONNECT)
            {
                post->setstringid(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IPADRESS_USRCONNECT)
            {
                post->setipadress(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_DISTANT_USRCONNECT )
            {
                post->setisdistant(newvalue.toBool());
                newvalue = (newvalue.toBool()? "1" : "null");
            }
            else if (field == CP_IDUSERSUPERVISEUR_USRCONNECT )
            {
                post->setidsuperviseur(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDUSERCOMPTABLE_USRCONNECT )
            {
                post->setidcomptable(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDUSERPARENT_USRCONNECT )
            {
                post->setidparent(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDLIEU_USRCONNECT )
            {
                post->setidlieu(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_HEUREDERNIERECONNECTION_USRCONNECT )
            {
                post->setdateheurederniereconnexion(newvalue.toDateTime());
                Utils::CalcDateTimeValueSQL(newvalue);
            }
            else if (field == CP_IDPATENCOURS_USRCONNECT )
            {
                post->setidpatencours(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            break;
        }
        bq = dynamic_cast<Banque*>(item);
        if (bq)
        {
            table = TBL_BANQUES;
            ok = true;
            clause = CP_ID_BANQUES " = " + QString::number(item->id());
            if (field == CP_NOMABREGE_BANQUES )
            {
                bq->setnomabrege(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_NOMBANQUE_BANQUES )
            {
                bq->setnom(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else
                ok = false;
            break;
        }
        dep =  dynamic_cast<Depense*>(item);
        if (dep)
        {
            table = TBL_DEPENSES;
            ok = true;
            clause = CP_ID_DEPENSES " = " + QString::number(item->id());
            if (field == CP_IDUSER_DEPENSES)
            {
                dep->setiduser(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_DATE_DEPENSES)
            {
                dep->setdate(newvalue.toDate());
                Utils::CalcDateValueSQL(newvalue);
            }
            else if (field == CP_REFFISCALE_DEPENSES)
            {
                dep->setrubriquefiscale(newvalue.toString());
                dep->setidrubriquefiscale(DataBase::I()->GetidRubriqueFiscale(dep));
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_OBJET_DEPENSES)
            {
                dep->setobjet(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MONTANT_DEPENSES)
            {
                dep->setmontant(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_FAMILLEFISCALE_DEPENSES)
            {
                dep->setfamillefiscale(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MONNAIE_DEPENSES)
            {
                dep->setmonnaie(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDRECETTE_DEPENSES)
            {
                dep->setidrecette(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_MODEPAIEMENT_DEPENSES)
            {
                dep->setmodepaiement(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_COMPTE_DEPENSES)
            {
                dep->setidcomptebancaire(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_NUMCHEQUE_DEPENSES)
            {
                dep->setnocheque(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDFACTURE_DEPENSES)
            {
                dep->setidfacture(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else
                ok = false;
            break;
        }
        usr = dynamic_cast<User*>(item);
        if (usr)
        {
            table = TBL_UTILISATEURS;
            ok = true;
            clause = CP_ID_USR " = " + QString::number(item->id());
            if (field == CP_ISDESACTIVE_USR )
            {
                usr->setdesactive(newvalue.toBool());
                newvalue = (newvalue.toBool()? "1" : "null");
            }
            if (field == CP_AFFICHEDOCSPUBLICS_USR )
            {
                usr->setaffichedocspublics(newvalue.toBool());
                newvalue = (newvalue.toBool()? "1" : "null");
            }
            if (field == CP_AFFICHECOMMENTSPUBLICS_USR )
            {
                usr->setaffichecommentslunettespublics(newvalue.toBool());
                newvalue = (newvalue.toBool()? "1" : "null");
            }
            else
                ok = false;
            break;
        }
        sessionop = dynamic_cast<SessionOperatoire*>(item);
        if (sessionop)
        {
            table = TBL_SESSIONSOPERATOIRES;
            ok = true;
            clause = CP_ID_SESSIONOPERATOIRE " = " + QString::number(item->id());
            if (field == CP_DATE_SESSIONOPERATOIRE )
            {
                sessionop->setdate(newvalue.toDate());
                Utils::CalcDateValueSQL(newvalue);
            }
            else if (field == CP_IDLIEU_SESSIONOPERATOIRE )
            {
                sessionop->setidlieu(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDUSER_SESSIONOPERATOIRE )
            {
                sessionop->setiduser(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDAIDE_SESSIONOPERATOIRE )
            {
                sessionop->setidaide(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_INCIDENT_SESSIONOPERATOIRE)
            {
                sessionop->setincident(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            break;
        }
        sit = dynamic_cast<Site*>(item);
        if (sit)
        {
            table = TBL_LIEUXEXERCICE;
            ok = true;
            clause = CP_ID_SITE " = " + QString::number(item->id());
            if (field == CP_NOM_SITE)
            {
                sit->setnom(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE1_SITE)
            {
                sit->setadresse1(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE2_SITE)
            {
                sit->setadresse2(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE3_SITE)
            {
                sit->setadresse3(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_CODEPOSTAL_SITE )
            {
                sit->setcodepostal(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_VILLE_SITE)
            {
                sit->setville(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_TELEPHONE_SITE)
            {
                sit->settelephone(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_FAX_SITE)
            {
                sit->setfax(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_COULEUR_SITE)
            {
                sit->setcouleur(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            break;
        }
        man = dynamic_cast<Manufacturer*>(item);
        if (man)
        {
            table = TBL_MANUFACTURERS;
            ok = true;
            clause = CP_ID_MANUFACTURER " = " + QString::number(item->id());
            if (field == CP_NOM_MANUFACTURER)
            {
                man->setnom(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE1_MANUFACTURER)
            {
                man->setadresse1(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE2_MANUFACTURER)
            {
                man->setadresse2(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE3_MANUFACTURER)
            {
                man->setadresse3(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_CODEPOSTAL_MANUFACTURER )
            {
                man->setcodepostal(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_VILLE_MANUFACTURER)
            {
                man->setville(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_TELEPHONE_MANUFACTURER)
            {
                man->settelephone(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_FAX_MANUFACTURER)
            {
                man->setfax(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_PORTABLE_MANUFACTURER)
            {
                man->setportable(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_WEBSITE_MANUFACTURER)
            {
                man->setwebsite(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MAIL_MANUFACTURER)
            {
                man->setmail(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            break;
        }
        typinterv = dynamic_cast<TypeIntervention*>(item);
        if (typinterv)
        {
            table = TBL_TYPESINTERVENTIONS;
            ok = true;
            clause = CP_ID_TYPINTERVENTION " = " + QString::number(item->id());
            if (field == CP_TYPEINTERVENTION_TYPINTERVENTION)
            {
                typinterv->settypeintervention(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_CODECCAM_TYPINTERVENTION)
            {
                typinterv->setcodeCCAM(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_DUREE_TYPINTERVENTION)
            {
                typinterv->setduree(newvalue.toTime());
                Utils::CalcTimeValueSQL(newvalue);
            }
            break;
        }
        iol = dynamic_cast<IOL*>(item);
        if (iol)
        {
            table = TBL_IOLS;
            ok = true;
            clause = CP_ID_IOLS " = " + QString::number(item->id());
            if (field == CP_IDMANUFACTURER_IOLS)
            {
                int a = newvalue.toInt();
                iol->setidmanufacturer(a);
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_MODELNAME_IOLS)
            {
                iol->setmodele(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_INACTIF_IOLS)
            {
                bool a = newvalue.toBool();
                iol->setactif(a);
                newvalue = (a? "null" : "1");
            }
            else if (field == CP_DIAOPT_IOLS)
            {
                iol->setDiaoptique(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_DIAALL_IOLS)
            {
                iol->setDiaall(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_ACD_IOLS)
            {
                iol->setAcd(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_MINPWR_IOLS)
            {
                iol->setPwrmin(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_MAXPWR_IOLS)
            {
                iol->setPwrmax(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_PWRSTEP_IOLS)
            {
                iol->setPwrstp(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_MINCYL_IOLS)
            {
                iol->setCylmin(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_MAXCYL_IOLS)
            {
                iol->setCylmax(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_CYLSTEP_IOLS)
            {
                iol->setCylstp(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_CSTEAOPT_IOLS)
            {
                iol->setCsteAopt(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_CSTEAECHO_IOLS)
            {
                iol->setCsteAEcho(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_HAIGISA0_IOLS)
            {
                iol->setHaigisa0(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_HAIGISA1_IOLS)
            {
                iol->setHaigisa1(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_HAIGISA2_IOLS)
            {
                iol->setHaigisa2(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_HOLL1_IOLS)
            {
                iol->setHolladay(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_DIAINJECTEUR_IOLS)
            {
                iol->setDiainjecteur(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_ARRAYIMG_IOLS)
            {
                iol->setArrayImgiol(newvalue.toByteArray());
                Utils::CalcBlobValueSQL(newvalue);
            }
            else if (field == CP_TYPIMG_IOLS)
            {
                iol->setTypeImage(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MATERIAU_IOLS)
            {
                iol->setMateriau(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_REMARQUE_IOLS)
            {
                iol->setRemarque(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_PRECHARGE_IOLS)
            {
                bool a = newvalue.toBool();
                iol->setprecharge(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_JAUNE_IOLS)
            {
                bool a = newvalue.toBool();
                iol->setjaune(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_TORIC_IOLS)
            {
                bool a = newvalue.toBool();
                iol->setToric(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_EDOF_IOLS)
            {
                bool a = newvalue.toBool();
                iol->setEdof(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_MULTIFOCAL_IOLS)
            {
                bool a = newvalue.toBool();
                iol->setMultifocal(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_TYP_IOLS)
            {
                iol->setType(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            break;
        }
        interv = dynamic_cast<Intervention*>(item);
        if (interv)
        {
            table = TBL_LIGNESPRGOPERATOIRES;
            ok = true;
            clause = CP_ID_LIGNPRGOPERATOIRE " = " + QString::number(item->id());
            if (field == CP_HEURE_LIGNPRGOPERATOIRE)
            {
                interv->setheure(newvalue.toTime());
                Utils::CalcTimeValueSQL(newvalue);
            }
            else if (field == CP_IDSESSION_LIGNPRGOPERATOIRE)
            {
                interv->setidsession(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDPATIENT_LIGNPRGOPERATOIRE)
            {
                interv->setidpatient(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_TYPEANESTH_LIGNPRGOPERATOIRE)
            {
                interv->setanesthesie(Intervention::ConvertModeAnesthesie(newvalue.toString()));
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDTYPEINTERVENTION_LIGNPRGOPERATOIRE)
            {
                interv->setidtypeintervention(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_COTE_LIGNPRGOPERATOIRE)
            {
                interv->setcote(Utils::ConvertCote(newvalue.toString()));
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDIOL_LIGNPRGOPERATOIRE)
            {
                interv->setidIOL(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_PWRIOL_LIGNPRGOPERATOIRE)
            {
                interv->setpuissanceIOL(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_CYLIOL_LIGNPRGOPERATOIRE)
            {
                interv->setcylindreIOL(newvalue.toDouble());
                Utils::CalcdoubleValueSQL(newvalue);
            }
            else if (field == CP_OBSERV_LIGNPRGOPERATOIRE)
            {
                interv->setobservation(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDACTE_LIGNPRGOPERATOIRE)
            {
                interv->setidacte(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_INCIDENT_LIGNPRGOPERATOIRE)
            {
                interv->setincident(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            break;
        }
        tiers = dynamic_cast<Tiers*>(item);
        if (tiers)
        {
            ok = true;
            table = TBL_TIERS;
            clause = CP_ID_TIERS " = " + QString::number(item->id());
            if (field == CP_NOM_TIERS)
            {
                tiers->setNom(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE1_TIERS)
            {
                tiers->setAdresse1(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE2_TIERS)
            {
                tiers->setAdresse2(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ADRESSE3_TIERS)
            {
                tiers->setAdresse3(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_CODEPOSTAL_TIERS)
            {
                tiers->setCodepostal(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_VILLE_TIERS)
            {
                tiers->setVille(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_TELEPHONE_TIERS)
            {
                tiers->setTelephone(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_FAX_TIERS)
            {
                tiers->setFax(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MAIL_TIERS)
            {
                tiers->setMail(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_WEB_TIERS)
            {
                tiers->setWebsite(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            break;
        }
        com = dynamic_cast<Commercial*>(item);
        if (com)
        {
            ok = true;
            table = TBL_COMMERCIALS;
            clause = CP_ID_COM " = " + QString::number(item->id());
            if (field == CP_NOM_COM)
            {
                com->setnom(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_PRENOM_COM)
            {
                com->setprenom(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_TELEPHONE_COM)
            {
                com->settelephone(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_STATUT_COM)
            {
                com->setstatut(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_MAIL_COM)
            {
                com->setmail(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDMANUFACTURER_COM)
            {
                com->setidmanufactureur(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            break;
        }
        comment = dynamic_cast<CommentLunet*>(item);
        if (comment)
        {
            ok = true;
            table = TBL_COMMENTAIRESLUNETTES;
            clause = CP_ID_COMLUN " = " + QString::number(item->id());
            if (field == CP_TEXT_COMLUN)
            {
                comment->settexte(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_RESUME_COMLUN)
            {
                comment->setresume(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDUSER_COMLUN)
            {
                comment->setiduser(newvalue.toInt());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_PARDEFAUT_COMLUN)
            {
                bool a = newvalue.toBool();
                comment->setdefaut(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_PUBLIC_COMLUN)
            {
                bool a = newvalue.toBool();
                comment->setpublic(a);
                newvalue = (a? "1" : "null");
            }
            break;
        }
        motcle = dynamic_cast<MotCle*>(item);
        if (motcle)
        {
            ok = true;
            table = TBL_MOTSCLES;
            clause = CP_ID_MOTCLE " = " + QString::number(item->id());
            if (field == CP_TEXT_MOTCLE)
            {
                motcle->setmotcle(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            break;
        }
        impr = dynamic_cast<Impression*>(item);
        if (impr)
        {
            ok = true;
            table = TBL_IMPRESSIONS;
            clause = CP_ID_IMPRESSIONS " = " + QString::number(item->id());
            if (field == CP_TEXTE_IMPRESSIONS)
            {
                impr->settext(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_RESUME_IMPRESSIONS)
            {
                impr->setresume(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_CONCLUSION_IMPRESSIONS)
            {
                impr->setconclusion(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDUSER_IMPRESSIONS)
            {
                impr->setiduser(newvalue.toInt());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_DOCPUBLIC_IMPRESSIONS)
            {
                bool a = newvalue.toBool();
                impr->setpublic(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_PRESCRIPTION_IMPRESSIONS)
            {
                bool a = newvalue.toBool();
                impr->setprescription(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_EDITABLE_IMPRESSIONS)
            {
                bool a = newvalue.toBool();
                impr->seteditable(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_MEDICAL_IMPRESSIONS)
            {
                bool a = newvalue.toBool();
                impr->setmedical(a);
                newvalue = (a? "1" : "null");
            }
            break;
        }
        dossier = dynamic_cast<DossierImpression*>(item);
        if (dossier)
        {
            ok = true;
            table = TBL_DOSSIERSIMPRESSIONS;
            clause = CP_ID_DOSSIERIMPRESSIONS " = " + QString::number(item->id());
            if (field == CP_RESUME_DOSSIERIMPRESSIONS)
            {
                dossier->setresume(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDUSER_DOSSIERIMPRESSIONS)
            {
                dossier->setiduser(newvalue.toInt());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_PUBLIC_DOSSIERIMPRESSIONS)
            {
                bool a = newvalue.toBool();
                dossier->setpublic(a);
                newvalue = (a? "1" : "null");
            }
            break;
        }
        msg = dynamic_cast<Message*>(item);
        if (msg)
        {
            ok = true;
            table = TBL_MESSAGES;
            clause = CP_ID_MSG " = " + QString::number(item->id());
            if (field == CP_IDEMETTEUR_MSG)
            {
                msg->setidemetteur(newvalue.toInt());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_TEXT_MSG)
            {
                msg->settexte(newvalue.toString());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_IDPATIENT_MSG)
            {
                msg->setidpatient(newvalue.toInt());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_TACHE_MSG)
            {
                bool a = newvalue.toBool();
                msg->settache(a);
                newvalue = (a? "1" : "null");
            }
            if (field == CP_DATELIMITE_MSG )
            {
                msg->setdatelimite(newvalue.toDate());
                Utils::CalcDateValueSQL(newvalue);
            }
            if (field == CP_DATECREATION_MSG )
            {
                msg->setdatecreation(newvalue.toDateTime());
                Utils::CalcDateTimeValueSQL(newvalue);
            }
            else if (field == CP_URGENT_MSG)
            {
                bool a = newvalue.toBool();
                msg->seturgent(a);
                newvalue = (a? "1" : "null");
            }
            else if (field == CP_ENREPONSEA_MSG)
            {
                msg->setidreponsea(newvalue.toInt());
                Utils::CalcStringValueSQL(newvalue);
            }
            else if (field == CP_ASUPPRIMER_MSG)
            {
                bool a = newvalue.toBool();
                msg->setasupprimer(a);
                newvalue = (a? "1" : "null");
            }
            break;
        }
        session = dynamic_cast<Session*>(item);
        if (session)
        {
            table = TBL_SESSIONS;
            ok = true;
            clause = CP_ID_SESSIONS " = " + QString::number(item->id());
            if (field == CP_DATEDEBUT_SESSIONS )
            {
                session->setdatedebut(newvalue.toDateTime());
                Utils::CalcDateTimeValueSQL(newvalue);
            }
            else if (field == CP_DATEFIN_SESSIONS )
            {
                session->setdatefin(newvalue.toDateTime());
                Utils::CalcDateTimeValueSQL(newvalue);
            }
            else if (field == CP_IDLIEU_SESSIONS )
            {
                session->setidlieu(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDUSER_SESSIONS )
            {
                session->setiduser(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDSUPERVISEUR_SESSIONS )
            {
                session->setidsuperviseur(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDPARENT_SESSIONS )
            {
                session->setidparent(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            else if (field == CP_IDCOMPTABLE_SESSIONS )
            {
                session->setidcomptable(newvalue.toInt());
                Utils::CalcintValueSQL(newvalue);
            }
            break;
        }
        return false;
    }

    if (ok)
    {
        QString req = "update " + table + " set " + field + " = " + newvalue.toString() + " where " + clause;
        //qDebug() << req;
        DataBase::I()->StandardSQL(req);
    }
    return ok;
}
