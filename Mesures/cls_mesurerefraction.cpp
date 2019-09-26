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

#include "cls_mesurerefraction.h"

MesureRefraction::MesureRefraction() {}

MesureRefraction::MesureRefraction(Refraction::Mesure typemesure)
{
    setmesure(typemesure);
}

void MesureRefraction::setdata(Refraction *ref)
{
    m_sphereOD          = ref->sphereOD();
    m_cylindreOD        = ref->cylindreOD();
    m_axecylindreOD     = ref->axecylindreOD();
    m_avlOD             = ref->avlOD();
    m_addVPOD           = ref->addVPOD();
    m_avpPOD            = ref->avpOD();
    m_prismeOD          = ref->prismeOD();
    m_baseprismeOD      = ref->baseprismeOD();
    m_formuleOD         = ref->formuleOD();
    m_sphereOG          = ref->sphereOG();
    m_cylindreOG        = ref->cylindreOG();
    m_axecylindreOG     = ref->axecylindreOG();
    m_avlOG             = ref->avlOG();
    m_addVPOG           = ref->addVPOG();
    m_avpPOG            = ref->avpOG();
    m_prismeOG          = ref->prismeOG();
    m_baseprismeOG      = ref->baseprismeOG();
    m_formuleOG         = ref->formuleOG();
    m_ecartIP           = ref->ecartIP();
    m_typemesure        = ref->typemesure();
    m_cleandatas        = false;
}

void MesureRefraction::setdata(MesureRefraction *mesure)
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
    m_sphereOG          = mesure->sphereOG();
    m_cylindreOG        = mesure->cylindreOG();
    m_axecylindreOG     = mesure->axecylindreOG();
    m_avlOG             = mesure->avlOG();
    m_addVPOG           = mesure->addVPOG();
    m_avpPOG            = mesure->avpOG();
    m_prismeOG          = mesure->prismeOG();
    m_baseprismeOG      = mesure->baseprismeOG();
    m_formuleOG         = mesure->formuleOG();
    m_ecartIP           = mesure->ecartIP();
    m_typemesure        = mesure->typemesure();
    m_cleandatas        = false;
}
