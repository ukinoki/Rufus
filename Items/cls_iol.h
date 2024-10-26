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

#ifndef IOL_H
#define IOL_H

#include "cls_item.h"
#include "utils.h"

/*!
 * \brief classe IOL
 * l'ensemble des informations concernant un IOL
 */

class IOL : public Item
{
    Q_OBJECT
public:
    explicit IOL(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data = QJsonObject{});

private: //Données de l'intervention
    //!<m_id = Id de l'iol en base
    //! m_stringid = manufacturer
    int m_idmanufacturer    = 0;                //! id du fabricant
    int m_iddistributeur    = 0;                //! id du distributeur du fabricant - non stocké en base parce que redondant
    int m_idiolcon          = 0;                //! id iol in iolcon base
    QString m_modele        = "";               //! modèle de l'implant
    bool m_inactif          = false;

    //! diamètres
    double m_diainjecteur   = 0;                //! diamètre minimal de l'injecteur
    double m_diaall         = 0;                //! diamètre hors tout
    double m_opticaldiameter= 0;                //! diamètre optique

    //! image
    QByteArray m_arrayimgiol= QByteArray();     //! le cliché de l'IOL
    QString m_imageformat   = "";               //! le type de cliché, jpg ou pdf

    //! material
    QString m_opticmaterial = "";               //! le matériau de l'IOL
            //!new
    QString m_hapticmaterial= "";               //! le matériau des haptiques
            //!new
    bool m_singlepiece      = true;

    QString m_remarque      = "";               //! remarque à propos de l'IOL

    bool m_preloaded        = false;            //! préchargé
    bool m_yellow           = false;            //! jaune ou clair
    bool m_multifocal       = false;            //! multifocal
    bool m_edof             = false;            //! edof
    bool m_toric            = false;            //! toric

    //! range power
    double m_pwrmax         = 0;                //! puissance maximale de l'implant
    double m_pwrmin         = 0;                //! puissance minimale de l'implant
    double m_pwrstp         = 0;                //! pas de variation de la puissance l'implant
    double m_cylmax         = 0;                //! cylindre maximal de l'implant
    double m_cylmin         = 0;                //! cylinde minimal de l'implant
    double m_cylstp         = 0;                //! pas de variation du cylindre l'implant
        //!new
    double m_addintermediate= 0;                //! addition in intermediate vision
        //!new
    double m_addnear        = 0;                //! addition in near vision

    //! constants
    double m_csteAEcho      = 0;                //! constante A echographique

    double m_csteAopt_nominal   = 0;            //! constante A optique nominal
    double m_haigisa0_nominal   = 0;            //! constante Haigis a0 nominal
    double m_haigisa1_nominal   = 0;            //! constante Haigis a1 nominal
    double m_haigisa2_nominal   = 0;            //! constante Haigis a2 nominal
    double m_holladay1_nominal  = 0;            //! constante Holladay1 nominal
    double m_hofferq_nominal    = 0;            //! constante HofferQ nominal
    double m_barettdf_nominal   = 0;            //! constante BarettDF nominal
    double m_barettlf_nominal   = 0;            //! constante BarettLF nominal
    double m_olsen_nominal      = 0;            //! constante Olsen nominal

    int m_ulibresults        = 0;               //! nbre de resultats ulib
    double m_csteAopt_ulib   = 0;               //! constante A optique ulib
    double m_haigisa0_ulib   = 0;               //! constante Haigis a0 ulib
    double m_haigisa1_ulib   = 0;               //! constante Haigis a1 ulib
    double m_haigisa2_ulib   = 0;               //! constante Haigis a2 ulib
    double m_holladay1_ulib  = 0;               //! constante Holladay1 ulib
    double m_hofferq_ulib    = 0;               //! constante HofferQ ulib
    double m_barettdf_ulib   = 0;               //! constante BarettDF ulib
    double m_barettlf_ulib   = 0;               //! constante BarettLF ulib
    double m_olsen_ulib      = 0;               //! constante Olsen ulib

    int m_optimizedresults        = 0;          //! nbre de resultats optimized
    double m_csteAopt_optimized   = 0;          //! constante A optique optimized
    double m_haigisa0_optimized   = 0;          //! constante Haigis a0 optimized
    double m_haigisa1_optimized   = 0;          //! constante Haigis a1 optimized
    double m_haigisa2_optimized   = 0;          //! constante Haigis a2 optimized
    double m_holladay1_optimized  = 0;          //! constante Holladay1 optimized
    double m_hofferq_optimized    = 0;          //! constante HofferQ optimized
    double m_barettdf_optimized   = 0;          //! constante BarettDF optimized
    double m_barettlf_optimized   = 0;          //! constante BarettLF optimized
    double m_olsen_optimized      = 0;          //! constante Olsen ulib

    double m_acd            = 0;                //! ACD

    //! type IOL
    QString m_type          = "";               //! le type de l'IOL : CP, CA, addon, support irien, refractif ca

    QImage m_nullimage      = QImage("://IOL.png");
    QImage m_currentimage   = m_nullimage;


public:
    int idmanufacturer() const                  { return m_idmanufacturer; }
    int iddistributeur() const                  { return m_iddistributeur; }
    int iddiolcon() const                       { return m_idiolcon; }
    QString modele() const                      { return m_modele; }
    double pwrmax() const                       { return m_pwrmax; }
    double pwrmin() const                       { return m_pwrmin; }
    double pwrstp() const                       { return m_pwrstp; }
    double cylmax() const                       { return m_cylmax; }
    double cylmin() const                       { return m_cylmin; }
    double cylstp() const                       { return m_cylstp; }
    double addnear() const                      { return m_addnear; }
    double addintermediate() const              { return m_addintermediate; }

    double csteAEcho() const                    { return m_csteAEcho; }

    double csteAopt_nominal() const             { return m_csteAopt_nominal; }
    double haigisa0_nominal() const             { return m_haigisa0_nominal; }
    double haigisa1_nominal() const             { return m_haigisa1_nominal; }
    double haigisa2_nominal() const             { return m_haigisa2_nominal; }
    double holladay1_nominal() const            { return m_holladay1_nominal; }
    double hofferQ_nominal() const              { return m_hofferq_nominal; }
    double barettLF_nominal() const             { return m_barettlf_nominal; }
    double barettDF_nominal() const             { return m_barettdf_nominal; }
    double olsen_nominal() const                { return m_olsen_nominal; }

    int results_ulib() const                    { return m_ulibresults; }
    double csteAopt_ulib() const                { return m_csteAopt_ulib; }
    double haigisa0_ulib() const                { return m_haigisa0_ulib; }
    double haigisa1_ulib() const                { return m_haigisa1_ulib; }
    double haigisa2_ulib() const                { return m_haigisa2_ulib; }
    double holladay1_ulib() const               { return m_holladay1_ulib; }
    double hofferQ_ulib() const                 { return m_hofferq_ulib; }
    double barettLF_ulib() const                { return m_barettlf_ulib; }
    double barettDF_ulib() const                { return m_barettdf_ulib; }
    double olsen_ulib() const                   { return m_olsen_ulib; }

    int results_optimized() const               { return m_optimizedresults; }
    double csteAopt_optimized() const           { return m_csteAopt_optimized; }
    double haigisa0_optimized() const           { return m_haigisa0_optimized; }
    double haigisa1_optimized() const           { return m_haigisa1_optimized; }
    double haigisa2_optimized() const           { return m_haigisa2_optimized; }
    double holladay1_optimized() const          { return m_holladay1_optimized; }
    double hofferQ_optimized() const            { return m_hofferq_optimized; }
    double barettLF_optimized() const           { return m_barettlf_optimized; }
    double barettDF_optimized() const           { return m_barettdf_optimized; }
    double olsen_optimized() const              { return m_olsen_optimized; }

    double acd() const                          { return m_acd; }
    double diainjecteur() const                 { return m_diainjecteur; }
    double diaall() const                       { return m_diaall; }
    double opticdiameter() const                { return m_opticaldiameter; }
    QByteArray arrayimgiol() const              { return m_arrayimgiol; }
    QString imageformat() const                 { return m_imageformat; }
    QString opticalmaterial() const             { return m_opticmaterial; }
    QString hapticalmaterial() const            { return m_hapticmaterial; }
    bool issinglepiece() const                  { return m_singlepiece; }
    QString remarque() const                    { return m_remarque; }
    bool isactif() const                        { return !m_inactif; }
    bool ispreloaded() const                    { return m_preloaded; }
    bool isyellow() const                       { return m_yellow; }
    bool ismultifocal() const                   { return m_multifocal; }
    bool isedof() const                         { return m_edof; }
    bool istoric() const                        { return m_toric; }
    QString type() const                        { return m_type; }
    int typetoint() const                       {
        if (m_type ==IOL_CP) return 1;
        else if (m_type ==IOL_CA) return 2;
        else if (m_type ==IOL_ADDON) return 3;
        else if (m_type ==IOL_IRIEN) return 4;
        else if (m_type ==IOL_CAREFRACTIF) return 5;
        else if (m_type ==IOL_AUTRE) return 6;
        else return 0;
; }
    QString tooltip(bool avecimage = false) const;


    void setidmanufacturer(int &id)                     { m_idmanufacturer = id;                m_data[CP_IDMANUFACTURER_IOLS] = id; }
    void setidistributeur(int id)                       { m_iddistributeur = id;}
    void setidiolcon(int id)                            { m_idiolcon = id;                      m_data[CP_IDIOLCON_IOLS] = id; }
    void setmodele(const QString &txt)                  { m_modele = txt;                       m_data[CP_MODELNAME_IOLS] = txt; }
    void setactif(bool &actif)                          { m_inactif = !actif;                   m_data[CP_INACTIF_IOLS] = !actif; }
    void setPwrmax(double pwrmax)                       { m_pwrmax = pwrmax;                    m_data[CP_MAXPWR_IOLS] = pwrmax; }
    void setPwrmin(double pwrmin)                       { m_pwrmin = pwrmin;                    m_data[CP_MINPWR_IOLS] = pwrmin; }
    void setPwrstp(double pwrstp)                       { m_pwrstp = pwrstp;                    m_data[CP_PWRSTEP_IOLS] = pwrstp; }
    void setCylmax(double cylmax)                       { m_cylmax = cylmax;                    m_data[CP_MAXCYL_IOLS] = cylmax; }
    void setCylmin(double cylmin)                       { m_cylmin = cylmin;                    m_data[CP_MINCYL_IOLS] = cylmin; }
    void setCylstp(double cylstp)                       { m_cylstp = cylstp;                    m_data[CP_CYLSTEP_IOLS] = cylstp; }
    void setAddnear(double addnear)                     { m_addnear = addnear;                  m_data[CP_ADDNEAR_IOLS] = addnear; }
    void setAddintermediate(double addintermediate)     { m_addintermediate = addintermediate;  m_data[CP_ADDINTERMEDIATE_IOLS] = addintermediate; }

    void setCsteAEcho(double csteAEcho)                 { m_csteAEcho = csteAEcho;              m_data[CP_CSTEAOPT_IOLS] = csteAEcho; }

    void setCsteAopt_nominal(double csteAopt)           { m_csteAopt_nominal = csteAopt;        m_data[CP_CSTEAOPT_IOLS] = csteAopt; }
    void setHaigisa0_nominal(double haigisa0)           { m_haigisa0_nominal = haigisa0;        m_data[CP_HAIGISA0_IOLS] = haigisa0; }
    void setHaigisa1_nominal(double haigisa1)           { m_haigisa1_nominal = haigisa1;        m_data[CP_HAIGISA1_IOLS] = haigisa1; }
    void setHaigisa2_nominal(double haigisa2)           { m_haigisa2_nominal = haigisa2;        m_data[CP_HAIGISA2_IOLS] = haigisa2; }
    void setHolladay1_nominal(double holladay1)         { m_holladay1_nominal = holladay1;      m_data[CP_HOLL1_IOLS] = holladay1; }
    void setHofferQ_nominal(double hofferq)             { m_haigisa0_nominal = hofferq;         m_data[CP_HOFFERQ_IOLS] = hofferq; }
    void setBarettLF_nominal(double barettlf)           { m_haigisa1_nominal = barettlf;        m_data[CP_BARETTLF_IOLS] = barettlf; }
    void setBarettDF_nominal(double barettdf)           { m_haigisa2_nominal = barettdf;        m_data[CP_BARETTDF_IOLS] = barettdf; }
    void setOlsen_nominal(double olsen)                 { m_olsen_nominal = olsen;              m_data[CP_OLSEN_IOLS] = olsen; }

    void setresults_ulib(int results)                   { m_ulibresults = results;              m_data[CP_RESULTSU_IOLS] = results; }
    void setCsteAopt_ulib(double csteAopt)              { m_csteAopt_ulib = csteAopt;           m_data[CP_CSTEAOPTU_IOLS] = csteAopt; }
    void setHaigisa0_ulib(double haigisa0)              { m_haigisa0_ulib = haigisa0;           m_data[CP_HAIGISA0U_IOLS] = haigisa0; }
    void setHaigisa1_ulib(double haigisa1)              { m_haigisa1_ulib = haigisa1;           m_data[CP_HAIGISA1U_IOLS] = haigisa1; }
    void setHaigisa2_ulib(double haigisa2)              { m_haigisa2_ulib = haigisa2;           m_data[CP_HAIGISA2U_IOLS] = haigisa2; }
    void setHolladay1_ulib(double holladay1)            { m_holladay1_ulib = holladay1;         m_data[CP_HOLL1U_IOLS] = holladay1; }
    void setHofferQ_ulib(double hofferq)                { m_haigisa0_ulib = hofferq;            m_data[CP_HOFFERQU_IOLS] = hofferq; }
    void setBarettLF_ulib(double barettlf)              { m_haigisa1_ulib = barettlf;           m_data[CP_BARETTLFU_IOLS] = barettlf; }
    void setBarettDF_ulib(double barettdf)              { m_haigisa2_ulib = barettdf;           m_data[CP_BARETTDFU_IOLS] = barettdf; }
    void setOlsen_ulib(double olsen)                    { m_olsen_ulib = olsen;                 m_data[CP_OLSENU_IOLS] = olsen; }

    void setresults_optimized(int results)              { m_optimizedresults = results;         m_data[CP_RESULTSO_IOLS] = results; }
    void setCsteAopt_optimized(double csteAopt)         { m_csteAopt_optimized = csteAopt;      m_data[CP_CSTEAOPTO_IOLS] = csteAopt; }
    void setHaigisa0_optimized(double haigisa0)         { m_haigisa0_optimized = haigisa0;      m_data[CP_HAIGISA0O_IOLS] = haigisa0; }
    void setHaigisa1_optimized(double haigisa1)         { m_haigisa1_optimized = haigisa1;      m_data[CP_HAIGISA1O_IOLS] = haigisa1; }
    void setHaigisa2_optimized(double haigisa2)         { m_haigisa2_optimized = haigisa2;      m_data[CP_HAIGISA2O_IOLS] = haigisa2; }
    void setHolladay1_optimized(double holladay1)       { m_holladay1_optimized = holladay1;    m_data[CP_HOLL1O_IOLS] = holladay1; }
    void setHofferQ_optimized(double hofferq)           { m_haigisa0_optimized = hofferq;       m_data[CP_HOFFERQO_IOLS] = hofferq; }
    void setBarettLF_optimized(double barettlf)         { m_haigisa1_optimized = barettlf;      m_data[CP_BARETTLFO_IOLS] = barettlf; }
    void setBarettDF_optimized(double barettdf)         { m_haigisa2_optimized = barettdf;      m_data[CP_BARETTDFO_IOLS] = barettdf; }
    void setOlsen_optimized(double olsen)               { m_olsen_optimized = olsen;            m_data[CP_OLSENO_IOLS] = olsen; }


    void setAcd(double acd)                             { m_acd = acd;                          m_data[CP_ACD_IOLS] = acd; }
    void setDiainjecteur(double diainjecteur)           { m_diainjecteur = diainjecteur;        m_data[CP_DIAINJECTEUR_IOLS] = diainjecteur; }
    void setDiaall(double diaall)                       { m_diaall = diaall;                    m_data[CP_DIAALL_IOLS] = diaall; }
    void setOpticalDiameter(double opticaldiameter)     { m_opticaldiameter = opticaldiameter;  m_data[CP_DIAOPT_IOLS] = opticaldiameter; }
    void setArrayImgiol(const QByteArray &imgiol)       { m_arrayimgiol = imgiol;               m_data[CP_ARRAYIMG_IOLS] = QLatin1String(imgiol.toBase64()); }
    void setimageformat(const QString &typeimage)       { m_imageformat = typeimage;            m_data[CP_TYPIMG_IOLS] = typeimage; }
    void setOpticalMaterial(const QString &material)    { m_opticmaterial = material;           m_data[CP_OPTICMATERIAU_IOLS] = material; }
    void setHapticalMaterial(const QString &material)   { m_hapticmaterial = material;          m_data[CP_HAPTICMATERIAU_IOLS] = material; }
    void setsinglepiece(bool &singlepiece)              { m_singlepiece = singlepiece;          m_data[CP_SINGLEPIECE_IOLS] = singlepiece; }
    void setRemarque(const QString &remarque)           { m_remarque = remarque;                m_data[CP_REMARQUE_IOLS] = remarque; }
    void setpreloaded(bool &preloaded)                  { m_preloaded = preloaded;              m_data[CP_PRECHARGE_IOLS] = preloaded; }
    void setyellow(bool &yellow)                        { m_yellow = yellow;                    m_data[CP_JAUNE_IOLS] = yellow; }
    void setMultifocal(bool &multifocal)                { m_multifocal = multifocal;            m_data[CP_MULTIFOCAL_IOLS] = multifocal; }
    void setToric(bool &toric)                          { m_toric = toric;                      m_data[CP_TORIC_IOLS] = toric; }
    void setEdof(bool &edof)                            { m_edof = edof;                        m_data[CP_EDOF_IOLS] = edof; }
    void setType(const QString &type)
    {
        m_type = type;
        if (type ==IOL_CP) m_data[CP_TYP_IOLS] = 1;
        else if (type ==IOL_CA) m_data[CP_TYP_IOLS] = 2;
        else if (type ==IOL_ADDON) m_data[CP_TYP_IOLS] = 3;
        else if (type ==IOL_IRIEN) m_data[CP_TYP_IOLS] = 4;
        else if (type ==IOL_CAREFRACTIF) m_data[CP_TYP_IOLS] = 5;
        else if (type ==IOL_AUTRE) m_data[CP_TYP_IOLS] = 6;
        else m_data[CP_TYP_IOLS] = 0;
    }

    QImage image() const                        { return m_currentimage; }
    void setimage(QImage &img)
    {
        if (img.isNull())
            m_nullimage.swap(m_currentimage);
        else
            img.swap(m_currentimage);
        m_data[CP_ARRAYIMG_IOLS] = Utils::jsonValFromImage(m_currentimage);
    }
    void resetdatas();
    bool isnull() const                         { return m_id == 0; }

};
#endif // IOL_H
