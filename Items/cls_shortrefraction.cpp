#include "cls_shortrefraction.h"

ShortRefraction::ShortRefraction()
{

}
ShortRefraction::ShortRefraction(Refraction *ref)
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
}
