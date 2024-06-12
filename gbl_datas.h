/* (C) 2020 LAINE SERGE
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
#include "cls_commentslunets.h"
#include "cls_commercials.h"
#include "cls_comptes.h"
#include "cls_correspondants.h"
#include "cls_cotations.h"
#include "cls_depenses.h"
#include "cls_lignespaiements.h"
#include "cls_docsexternes.h"
#include "cls_interventions.h"
#include "cls_interventions.h"
#include "cls_iols.h"
#include "cls_listrendezvous.h"
#include "cls_manufacturers.h"
#include "cls_messages.h"
#include "cls_motscles.h"
#include "cls_motifs.h"
#include "cls_paiementstiers.h"
#include "cls_patients.h"
#include "cls_patientsencours.h"
#include "cls_typestiers.h"
#include "cls_tierspayants.h"
#include "cls_users.h"
#include "cls_postesconnectes.h"
#include "cls_impressions.h"
#include "cls_refractions.h"
#include "cls_villes.h"
#include "cls_sites.h"
#include "cls_recettes.h"
#include "cls_mesurerefraction.h"
#include "cls_mesurekerato.h"
#include "cls_mesuredivers.h"
#include "cls_lignescomptes.h"
#include "cls_sessions.h"

class Datas : public QObject
{
    Q_OBJECT
private:
    Datas();
    static Datas *instance;

public:
    static Datas *I();

    Actes *actes;                           //!< la liste d'actes pour un patient
    Banques *banques;                       //!< toutes les banques
    Impressions *impressions;               //!< les documents émis (ordonnances, certificats, docs administratifs...etc...)
    Comptes *comptes;                       //!< tous les comptes bancaires d'un utilisateur
    CommentsLunets *commentslunets;         //! tous les commentaires lunettes
    Commercials *commercials;               //!< tous les commerciaux
    Correspondants *correspondants;         //!< Les correspondants
    Depenses *depenses;                     //!< les depenses pour un user et une  année donnée
    DocsExternes *docsexternes;             //!< Les documents externes
    Interventions *interventions;           //!< les interventions d'une session
    IOLs *iols;                             //!< Les iimplants
    LignesComptes *lignescomptes;           //!< Les lignes non rapprochées d'un compte bancaire
    LignesPaiements *lignespaiements;       //!< Les lignes de paiement des actes d'un patient
    ListRendezVous *listrendezvous;         //!< Les rendezvous en cours
    DossiersImpressions *metadocuments;     //!< les dossiers de documents émis (ordonnances, certificats, docs administratifs...etc...)
    Messages *messages;                     //!< les messages
    Manufacturers *manufacturers;           //!< tous les fabricants
    Motifs *motifs;                         //!< tous les motifs d'actes
    MotsCles *motscles;                     //!< tous les mots clés
    PaiementsTiers *paiementstiers;         //!< tous les paiements par tiers payants
    Patients *patients;                     //!> tous les patients en mémoire
    PatientsEnCours *patientsencours;       //!> les patients en cours : patients dont les dossiers sont ouverts (en cours d'examen), ou présents en salle d'attente ou à l'accueil
    Recettes *recettes;                     //!> toutes les recettes sur une période donnée
    Refractions *refractions;               //!> toutes les refractions d'un patient
    SessionsOperatoires *sessionsoperatoires; //!> les sessionsoperatoires d'un user
    Sessions *sessions;                     //!> les sessions
    Sites *sites;                           //!< Les lieux de travail
    TiersPayants *tierspayants;             //!< tous les tiers payants
    TypesInterventions *typesinterventions; //!< tous les types d'interventions
    TypesTiers *typestiers;                 //!< tous les types tiers payants
    Users *users;                           //!< Les users
    PostesConnectes *postesconnectes;       //!< Les postesconnectes
    Villes *villes;                         //!< toutes les villes
    QMap<int, Cotations*> *listecotations;  //!< la liste des cotations par iduser
    MesureRefraction*   mesurefronto;
    MesureRefraction*   mesureautoref;
    MesureRefraction*   mesureacuite;
    MesureRefraction*   mesurefinal;
    Keratometrie*       mesurekerato;
    Pachymetrie*        mesurepachy;
    Tonometrie*         mesuretono;

};

#endif // GBL_DATAS_H
