#include "cls_lignecompte.h"

LigneCompte::LigneCompte(QJsonObject data, QObject *parent) : Item(parent)
{
    setData(data);
}

void LigneCompte::setData(QJsonObject data)
{
    if( data.isEmpty() )
        return;

    Utils::setDataInt(data, CP_IDLIGNE_LIGNCOMPTES, m_id);
    Utils::setDataInt(data, CP_IDCOMPTE_LIGNCOMPTES, m_idcompte);
    Utils::setDataInt(data, CP_IDDEP_LIGNCOMPTES , m_iddep);
    Utils::setDataInt(data, CP_IDREC_LIGNCOMPTES , m_idrecette);
    Utils::setDataInt(data, CP_IDRECSPEC_LIGNCOMPTES , m_idrecettespeciale);
    Utils::setDataInt(data, CP_IDREMCHEQ_LIGNCOMPTES , m_idremisecheque);
    Utils::setDataDate(data, CP_DATE_LIGNCOMPTES , m_date);
    Utils::setDataString(data, CP_LIBELLE_LIGNCOMPTES, m_libelle);
    Utils::setDataString(data, CP_TYPEOPERATION_LIGNCOMPTES, m_typeoperation);
    Utils::setDataBool(data, CP_DEBITCREDIT_LIGNCOMPTES, m_iscredit);
    Utils::setDataBool(data, CP_CONSOLIDE_LIGNCOMPTES, m_isconsolide);
    Utils::setDataDouble(data, CP_MONTANT_LIGNCOMPTES, m_montant);
    m_data = data;
}
