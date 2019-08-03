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

#include "gbl_datas.h"

Datas* Datas::instance =  Q_NULLPTR;
Datas* Datas::I()
{
    if( !instance )
        instance = new Datas();

    return instance;
}
Datas::Datas()
{
    userConnected = Q_NULLPTR;

    actes           = new Actes();
    banques         = new Banques();
    comptes         = new Comptes();
    correspondants  = new Correspondants();
    cotations       = new Cotations();
    depenses        = new Depenses();
    docsexternes    = new DocsExternes();
    documents       = new Impressions();
    lignespaiements = new LignesPaiements();
    metadocuments   = new DossiersImpressions();
    motifs          = new Motifs();
    paiementstiers  = new PaiementsTiers();
    patients        = new Patients();
    patientsencours = new PatientsEnCours();
    recettes        = new Recettes();
    refractions     = new Refractions();
    sites           = new Sites();
    tierspayants    = new TiersPayants();
    typestiers      = new TypesTiers();
    users           = new Users();
    postesconnectes = new PostesConnectes();
    villes          = new Villes();
}


