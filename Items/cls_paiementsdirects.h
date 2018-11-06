#ifndef CLS_PAIEMENTSDIRECTS_H
#define CLS_PAIEMENTSDIRECTS_H

#include <QObject>
#include "cls_paiementdirect.h"

/*!
 * \brief Cette classe gére la liste des paiements directs
 *  ( = actes dont le paiement est en attente d'être enregistré )
 * = tous les actes effectués pour lequel il faut encore enregistré le moyen de paiement:
 *  . paiement direct proprement dit
 *  . ou paiement par tiers
 */

class PaiementsDirects
{
public:
    explicit PaiementsDirects();
};

#endif // CLS_PAIEMENTSDIRECTS_H
