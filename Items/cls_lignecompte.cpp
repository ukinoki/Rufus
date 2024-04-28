#include "cls_lignecompte.h"

LigneCompte::LigneCompte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void LigneCompte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    setDataInt(data, CP_ID_LIGNCOMPTES, m_id);
    setDataInt(data, CP_IDCOMPTE_LIGNCOMPTES, m_idcompte);
    setDataInt(data, CP_IDDEP_LIGNCOMPTES , m_iddep);
    setDataInt(data, CP_IDREC_LIGNCOMPTES , m_idrecette);
    setDataInt(data, CP_IDRECSPEC_LIGNCOMPTES , m_idrecettespeciale);
    setDataInt(data, CP_IDREMCHEQ_LIGNCOMPTES , m_idremisecheque);
    setDataDate(data, CP_DATE_LIGNCOMPTES , m_date);
    setDataString(data, CP_LIBELLE_LIGNCOMPTES, m_libelle);
    setDataString(data, CP_TYPEOPERATION_LIGNCOMPTES, m_typeoperation);
    setDataBool(data, CP_DEBITCREDIT_LIGNCOMPTES, m_iscredit);
    setDataBool(data, CP_CONSOLIDE_LIGNCOMPTES, m_isconsolide);
    setDataDouble(data, CP_MONTANT_LIGNCOMPTES, m_montant);
    m_data = data;
}
