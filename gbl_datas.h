/* (C) 2018 LAINE SERGE
This file is part of RufusAdmin or Rufus.

RufusAdmin and Rufus are free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License,
or any later version.

RufusAdmin and Rufus are distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RufusAdmin and Rufus.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GBL_DATAS_H
#define GBL_DATAS_H

#include <QObject>
#include "cls_actes.h"
#include "cls_banques.h"
#include "cls_comptes.h"
#include "cls_correspondants.h"
#include "cls_cotations.h"
#include "cls_depenses.h"
#include "cls_docsexternes.h"
#include "cls_motifs.h"
#include "cls_paiementstiers.h"
#include "cls_patients.h"
#include "cls_typestiers.h"
#include "cls_tierspayants.h"
#include "cls_users.h"
#include "cls_documents.h"
#include "cls_villes.h"
#include "cls_sites.h"
#include "cls_recettes.h"

class Datas : public QObject
{
    Q_OBJECT
private:
    Datas();
    static Datas *instance;

public:
    static Datas *I();

    User *userConnected;
    Site *sitedetravail;
    Actes *actes;                       //!< la liste d'actes pour un patient
    Users *users;                       //!< Les users
    Patients *patients;                 //!< Les patients
    Correspondants *correspondants;     //!< Les correspondants
    Cotations *cotations;               //!< Les cotations
    Depenses *depenses;                 //!< les depenses pour un user et une  année donnée
    Comptes *comptes;                   //!< tous les comptes bancaires d'un utilisateur
    Banques *banques;                   //!< toutes les banques
    Motifs *motifs;                     //!< tous les motifs d'actes
    TiersPayants *tiers;                //!< tous les tiers payants
    TypesTiers *typestiers;             //!< tous les types tiers payants
    PaiementsTiers *paiementstiers;     //!< tous les paiements par tiers payants
    DocsExternes *docsexternes;         //!< Les documents externes
    Sites *sites;                       //!< Les lieux de travail
    Documents *documents;               //!< les documents émis (ordonnances, certificats, docs administratifs...etc...)
    MetaDocuments *metadocuments;       //!< les dossiers de documents émis (ordonnances, certificats, docs administratifs...etc...)
    Recettes *recettes;                 //!> toutes les recettes sur une période donnée
    Villes *villes;                     //!< toutes les villes
};

#endif // GBL_DATAS_H
