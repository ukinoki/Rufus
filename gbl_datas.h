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
#include "cls_banque.h"
#include "cls_compte.h"
#include <cls_correspondants.h>
#include "cls_cotation.h"
#include <cls_depenses.h>
#include "cls_docsexternes.h"
#include "cls_motif.h"
#include "cls_paiementsdirects.h"
#include <cls_patients.h>
#include "cls_tiers.h"
#include <cls_users.h>
#include "cls_document.h"

class Datas : public QObject
{
    Q_OBJECT
private:
    Datas();
    static Datas *instance;

public:
    static Datas *I();

    User *userConnected;
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
    PaiementsDirects *paiementsdirects; //!< tous les paiements en attente d'enregistrement
    DocsExternes *docsexternes;         //!< Les documents externes
    Documents *documents;               //!< les documents émis (ordonnances, certificats, docs administratifs...etc...)
    MetaDocuments *metadocuments;       //!< les dossiers de documents émis (ordonnances, certificats, docs administratifs...etc...)

signals:


public slots:
};

#endif // GBL_DATAS_H
