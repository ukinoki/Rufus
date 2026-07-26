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

#ifndef CLS_COTATION_H
#define CLS_COTATION_H

#include "cls_item.h"

/*!
 Une cotation = une ligne de la table cotations (partagée par tous les utilisateurs). Son type
 (m_typcotation : 1=CCAM, 2=association CCAM, 3=NGAP, 4=autre) est porté par la colonne « ccam » et
 déduit en 4 booléens is* par settypcotation().
    * montantoptam / montantnonoptam : le tarif conventionnel selon le secteur ;
    * montantconventionnel() : le montantconventionnel qui sera utilisé - oeut varier si l'utilisateur est optam ou non
    * montantpratique : le tarif propre à l'utilisateur, renseigné par Cotations::loadUserCotations
      depuis sa table de jointure ;
    * isused() : la cotation est dans les cotations de l'utilisateur courant.
*/

class Cotation : public Item
{
    Q_OBJECT
    friend class Cotations;                 //!< seule Cotations accède à montantoptam()/montantnonoptam()
private:
    int     m_frequence;
    bool    m_userused = false;             //!< la cotation est utilisée par l'utilisateur courant (bâtit map_usercotations)
    QString m_typeacte, m_descriptif;       //!< m_typeacte = la cotation ; m_descriptif = le descriptif de l'acte
    int     m_typcotation = 1;              //!< 1=CCAM, 2=association CCAM, 3=NGAP, 4=horsNGAPnorCCAM
    bool    m_isCCAM = false;               //!< déduits de m_typcotation par settypcotation()
    bool    m_isNGAP = false;
    bool    m_isAssocCCAM = false;
    bool    m_isnorNGAPnorCCAM = false;
    double  m_montantoptam = 0, m_montantnonoptam = 0, m_montantpratique = 0, m_montantconventionnel = 0;
public:
    explicit Cotation(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void    setData(QJsonObject data);

    QString typeacte() const                { return m_typeacte; }
    double  montantoptam() const            { return m_montantoptam; }
    double  montantnonoptam() const         { return m_montantnonoptam; }
    double  montantconventionnel() const    { return m_montantconventionnel; }
    double  montantpratique() const         { return m_montantpratique; }
    int     frequence() const               { return m_frequence; }
    QString descriptif() const              { return m_descriptif; }
    bool    isused() const                  { return m_userused; }
    void    setused(bool u)                 { m_userused = u; }
    void    setmontantpratique(double m)      { m_montantpratique = m; }
    void    setmontantconventionnel(double m) { m_montantconventionnel = m; }//!< écrit m_montantoptam (montantconventionnel() le renvoie)
    void    setdescriptif(QString d)          { m_descriptif = d; }          //!< Tip de la cotation (pour un CCAM, son libellé rapatrié de ccam)
    int     typcotation() const             { return m_typcotation; }
    void    settypcotation(int typ) { m_typcotation = typ;                   //!< 1=CCAM 2=association CCAM 3=NGAP 4=autre
                                      m_isCCAM           = (typ == 1);
                                      m_isAssocCCAM      = (typ == 2);
                                      m_isNGAP           = (typ == 3);
                                      m_isnorNGAPnorCCAM = (typ == 4); }
    bool    isCCAM() const                  { return m_isCCAM; }
    bool    isNGAP() const                  { return m_isNGAP; }
    bool    isAssocCCAM() const             { return m_isAssocCCAM; }
    bool    isnorNGAPnorCCAM() const        { return m_isnorNGAPnorCCAM; }
};

#endif // CLS_COTATION_H
