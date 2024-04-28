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

#include "cls_mesurerefraction.h"

MesureRefraction::MesureRefraction() {}


void MesureRefraction::setdatas(Refraction *ref, bool round)
{
    if (!ref->isODmesure() && !ref->isOGmesure())
    {
        cleandatas();
        return;
    }
    m_isnullOD = !ref->isODmesure();
    if (m_isnullOD)
        cleandatas(Item::Droit);
    else
    {
        m_sphereOD          = ref->sphereOD();
        m_cylindreOD        = ref->cylindreOD();
        m_axecylindreOD     = (round? Utils::roundToNearestFive(ref->axecylindreOD()) : ref->axecylindreOD());
        m_avlOD             = ref->avlOD();
        m_addVPOD           = ref->addVPOD();
        m_avpPOD            = ref->avpOD();
        m_prismeOD          = ref->prismeOD();
        m_baseprismeOD      = (round? Utils::roundToNearestFive(ref->baseprismeOD()) : ref->baseprismeOD());
        m_formuleOD         = ref->formuleOD();
    }
    m_isnullOG = !ref->isOGmesure();
    if (m_isnullOG)
        cleandatas(Item::Gauche);
    else
    {
        m_sphereOG          = ref->sphereOG();
        m_cylindreOG        = ref->cylindreOG();
        m_axecylindreOG     = (round? Utils::roundToNearestFive(ref->axecylindreOG()) : ref->axecylindreOG());
        m_avlOG             = ref->avlOG();
        m_addVPOG           = ref->addVPOG();
        m_avpPOG            = ref->avpOG();
        m_prismeOG          = ref->prismeOG();
        m_baseprismeOG      = (round? Utils::roundToNearestFive(ref->baseprismeOG()) : ref->baseprismeOG());
        m_formuleOG         = ref->formuleOG();
    }
    m_ecartIP           = ref->ecartIP();
    m_typemesure        = ref->typemesure();
    m_cleandatas        = false;
}

void MesureRefraction::setdatas(MesureRefraction *mesure)
{
    if (mesure->isdataclean())
    {
        cleandatas();
        return;
    }
    m_isnullOD = mesure->isnullLOD();
    if (m_isnullOD)
        cleandatas(Item::Droit);
    else
    {
        m_sphereOD          = mesure->sphereOD();
        m_cylindreOD        = mesure->cylindreOD();
        m_axecylindreOD     = mesure->axecylindreOD();
        m_avlOD             = mesure->avlOD();
        m_addVPOD           = mesure->addVPOD();
        m_avpPOD            = mesure->avpOD();
        m_prismeOD          = mesure->prismeOD();
        m_baseprismeOD      = mesure->baseprismeOD();
        m_formuleOD         = mesure->formuleOD();
    }
    m_isnullOG = mesure->isnullLOG();
    if (m_isnullOG)
        cleandatas(Item::Gauche);
    else
    {
        m_sphereOG          = mesure->sphereOG();
        m_cylindreOG        = mesure->cylindreOG();
        m_axecylindreOG     = mesure->axecylindreOG();
        m_avlOG             = mesure->avlOG();
        m_addVPOG           = mesure->addVPOG();
        m_avpPOG            = mesure->avpOG();
        m_prismeOG          = mesure->prismeOG();
        m_baseprismeOG      = mesure->baseprismeOG();
        m_formuleOG         = mesure->formuleOG();
    }
    m_ecartIP           = mesure->ecartIP();
    m_typemesure        = mesure->typemesure();
    m_cleandatas        = false;
}
