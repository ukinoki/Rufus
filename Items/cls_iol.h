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
    explicit IOL(QJsonObject data = {}, QObject *parent = nullptr);
    void setData(QJsonObject data = QJsonObject{}) override;

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
    QImage m_nullimage      = QImage("://IOL.png");
    QImage m_currentimage   = m_nullimage;

    //! material
    QString m_opticmaterial = "";               //! le matériau de l'IOL
        //!new
    QString m_hydrofily     = "";               //! IOL hydrofily
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

    double m_csteAEcho_nominal  = 0;            //! constante A echographique
    double m_csteAopt_nominal   = 0;            //! constante A optique nominal
    double m_haigisa0_nominal   = 0;            //! constante Haigis a0 nominal
    double m_haigisa1_nominal   = 0;            //! constante Haigis a1 nominal
    double m_haigisa2_nominal   = 0;            //! constante Haigis a2 nominal
    double m_holladay1_nominal  = 0;            //! constante Holladay1 nominal
    double m_hofferq_nominal    = 0;            //! constante HofferQ nominal
    double m_barrettdf_nominal   = 0;            //! constante BarrettDF nominal
    double m_barrettlf_nominal   = 0;            //! constante BarrettLF nominal
    double m_olsen_nominal      = 0;            //! constante Olsen nominal

    int m_ulibresults        = 0;               //! nbre de resultats ulib
    double m_csteAEcho_ulib  = 0;               //! constante A echographique ulib
    double m_csteAopt_ulib   = 0;               //! constante A optique ulib
    double m_haigisa0_ulib   = 0;               //! constante Haigis a0 ulib
    double m_haigisa1_ulib   = 0;               //! constante Haigis a1 ulib
    double m_haigisa2_ulib   = 0;               //! constante Haigis a2 ulib
    double m_holladay1_ulib  = 0;               //! constante Holladay1 ulib
    double m_hofferq_ulib    = 0;               //! constante HofferQ ulib
    double m_barrettdf_ulib   = 0;               //! constante BarrettDF ulib
    double m_barrettlf_ulib   = 0;               //! constante BarrettLF ulib
    double m_olsen_ulib      = 0;               //! constante Olsen ulib

    int m_optimizedresults        = 0;          //! nbre de resultats optimized
    double m_csteAopt_optimized   = 0;          //! constante A optique optimized
    double m_haigisa0_optimized   = 0;          //! constante Haigis a0 optimized
    double m_haigisa1_optimized   = 0;          //! constante Haigis a1 optimized
    double m_haigisa2_optimized   = 0;          //! constante Haigis a2 optimized
    double m_holladay1_optimized  = 0;          //! constante Holladay1 optimized
    double m_hofferq_optimized    = 0;          //! constante HofferQ optimized

    double m_acd            = 0;                //! ACD

    //! type IOL
    int m_typeInt           = 0;                //! le type de l'IOL : CP, CA, addon, support irien, refractif ca, autre exprimé en database



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

    double csteAEcho_nominal() const            { return m_csteAEcho_nominal; }
    double csteAopt_nominal() const             { return m_csteAopt_nominal; }
    double haigisa0_nominal() const             { return m_haigisa0_nominal; }
    double haigisa1_nominal() const             { return m_haigisa1_nominal; }
    double haigisa2_nominal() const             { return m_haigisa2_nominal; }
    double holladay1_nominal() const            { return m_holladay1_nominal; }
    double hofferQ_nominal() const              { return m_hofferq_nominal; }
    double barrettLF_nominal() const            { return m_barrettlf_nominal; }
    double barrettDF_nominal() const            { return m_barrettdf_nominal; }
    double olsen_nominal() const                { return m_olsen_nominal; }

    int results_ulib() const                    { return m_ulibresults; }
    double csteAEcho_ulib() const               { return m_csteAEcho_ulib; }
    double csteAopt_ulib() const                { return m_csteAopt_ulib; }
    double haigisa0_ulib() const                { return m_haigisa0_ulib; }
    double haigisa1_ulib() const                { return m_haigisa1_ulib; }
    double haigisa2_ulib() const                { return m_haigisa2_ulib; }
    double holladay1_ulib() const               { return m_holladay1_ulib; }
    double hofferQ_ulib() const                 { return m_hofferq_ulib; }
    double barrettLF_ulib() const               { return m_barrettlf_ulib; }
    double barrettDF_ulib() const               { return m_barrettdf_ulib; }
    double olsen_ulib() const                   { return m_olsen_ulib; }

    int results_optimized() const               { return m_optimizedresults; }
    double csteAopt_optimized() const           { return m_csteAopt_optimized; }
    double haigisa0_optimized() const           { return m_haigisa0_optimized; }
    double haigisa1_optimized() const           { return m_haigisa1_optimized; }
    double haigisa2_optimized() const           { return m_haigisa2_optimized; }
    double holladay1_optimized() const          { return m_holladay1_optimized; }
    double hofferQ_optimized() const            { return m_hofferq_optimized; }

    double acd() const                          { return m_acd; }
    double diainjecteur() const                 { return m_diainjecteur; }
    double diaall() const                       { return m_diaall; }
    double opticdiameter() const                { return m_opticaldiameter; }
    QByteArray arrayimgiol() const              { return m_arrayimgiol; }
    QString imageformat() const                 { return m_imageformat; }
    QString opticalmaterial() const             { return m_opticmaterial; }
    QString hydrofily() const                   { return m_hydrofily; }
    QString hapticalmaterial() const            { return m_hapticmaterial; }
    bool issinglepiece() const                  { return m_singlepiece; }
    QString remarque() const                    { return m_remarque; }
    bool isactif() const                        { return !m_inactif; }
    bool ispreloaded() const                    { return m_preloaded; }
    bool isyellow() const                       { return m_yellow; }
    bool ismultifocal() const                   { return m_multifocal; }
    bool isedof() const                         { return m_edof; }
    bool istoric() const                        { return m_toric; }
    QString typeString() const
    {
        if (m_typeInt == 1)        return IOL_CP;
        else if (m_typeInt == 2)   return IOL_CA;
        else if (m_typeInt == 3)   return IOL_ADDON;
        else if (m_typeInt == 4)   return IOL_IRIEN;
        else if (m_typeInt == 5)   return IOL_CAREFRACTIF;
        else if (m_typeInt == 6)   return IOL_AUTRE;
        else                       return "";
    }
    int type() const                            { return m_typeInt; }

    QString typeStringtotr() { return typeStringtotr(typeString()); }
    static QString typeStringtotr(QString type)
    {
        if (type == IOL_CP)                 return tr("Chambre postérieure");
        else if (type == IOL_CA)            return tr("Chambre antérieure");
        else if (type == IOL_ADDON)         return tr("Add-on");
        else if (type == IOL_IRIEN)         return tr("Support irien");
        else if (type == IOL_CAREFRACTIF)   return tr("Refractif CA");
        else if (type == IOL_AUTRE)         return tr("Autre");
        else return "type";
    }
    static QString typeStringfromtr(QString type)
    {
        if (type == tr("Chambre postérieure"))      return IOL_CP;
        else if (type == tr("Chambre antérieure"))  return IOL_CA;
        else if (type == tr("Add-on"))              return IOL_ADDON;
        else if (type == tr("Support irien"))       return IOL_IRIEN;
        else if (type == tr("Refractif CA"))        return IOL_CAREFRACTIF;
        else if (type == tr("Autre"))               return IOL_AUTRE;
        else return "type";
    }

    QString opticalMaterialtotr() const
    {
        if (m_opticmaterial == "acrylique")            return tr("acrylique");
        else if (m_opticmaterial == "PMMA")            return tr("PMMA");
        else if (m_opticmaterial == "silicone")        return tr("silicone");
        else if (m_opticmaterial == "copolymère")      return tr("copolymère");
        else return m_opticmaterial;
    }
    static QString opticalMaterialfromtr(QString material)
    {
        if (material == tr("acrylique"))        return "acrylique";
        else if (material == tr("PMMA"))        return "PMMA";
        else if (material == tr("silicone"))    return "silicone";
        else if (material == tr("copolymère"))  return "copolymère";
        else return material;
    }

    QString hydrofilytotr() const
    {
        if (m_hydrofily == "hydrophile")           return tr("hydrophile");
        else if (m_hydrofily == "hydrophobe")      return tr("hydrophobe");
        else return m_hydrofily;
    }
    static QString hydrofilyfromtr(QString Hydrofily)
    {
        if (Hydrofily == tr("hydrophile"))          return "hydrophile";
        else if (Hydrofily == tr("hydrophile"))     return "hydrophile";
        else return Hydrofily;
    }

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

    void setCsteAEcho_nominal(double csteAEcho)         { m_csteAEcho_nominal = csteAEcho;      m_data[CP_CSTEAECHO_IOLS] = csteAEcho; }
    void setCsteAopt_nominal(double csteAopt)           { m_csteAopt_nominal = csteAopt;        m_data[CP_CSTEAOPT_IOLS] = csteAopt; }
    void setHaigisa0_nominal(double haigisa0)           { m_haigisa0_nominal = haigisa0;        m_data[CP_HAIGISA0_IOLS] = haigisa0; }
    void setHaigisa1_nominal(double haigisa1)           { m_haigisa1_nominal = haigisa1;        m_data[CP_HAIGISA1_IOLS] = haigisa1; }
    void setHaigisa2_nominal(double haigisa2)           { m_haigisa2_nominal = haigisa2;        m_data[CP_HAIGISA2_IOLS] = haigisa2; }
    void setHolladay1_nominal(double holladay1)         { m_holladay1_nominal = holladay1;      m_data[CP_HOLL1_IOLS] = holladay1; }
    void setHofferQ_nominal(double hofferq)             { m_hofferq_nominal = hofferq;          m_data[CP_HOFFERQ_IOLS] = hofferq; }
    void setBarrettLF_nominal(double barrettlf)         { m_barrettlf_nominal = barrettlf;      m_data[CP_BARRETTLF_IOLS] = barrettlf; }
    void setBarrettDF_nominal(double barrettdf)         { m_barrettdf_nominal = barrettdf;      m_data[CP_BARRETTDF_IOLS] = barrettdf; }
    void setOlsen_nominal(double olsen)                 { m_olsen_nominal = olsen;              m_data[CP_OLSEN_IOLS] = olsen; }

    void setresults_ulib(int results)                   { m_ulibresults = results;              m_data[CP_RESULTSU_IOLS] = results; }
    void setCsteAEcho_ulib(double csteAEcho)            { m_csteAEcho_ulib = csteAEcho;         m_data[CP_CSTEAECHOU_IOLS] = csteAEcho; }
    void setCsteAopt_ulib(double csteAopt)              { m_csteAopt_ulib = csteAopt;           m_data[CP_CSTEAOPTU_IOLS] = csteAopt; }
    void setHaigisa0_ulib(double haigisa0)              { m_haigisa0_ulib = haigisa0;           m_data[CP_HAIGISA0U_IOLS] = haigisa0; }
    void setHaigisa1_ulib(double haigisa1)              { m_haigisa1_ulib = haigisa1;           m_data[CP_HAIGISA1U_IOLS] = haigisa1; }
    void setHaigisa2_ulib(double haigisa2)              { m_haigisa2_ulib = haigisa2;           m_data[CP_HAIGISA2U_IOLS] = haigisa2; }
    void setHolladay1_ulib(double holladay1)            { m_holladay1_ulib = holladay1;         m_data[CP_HOLL1U_IOLS] = holladay1; }
    void setHofferQ_ulib(double hofferq)                { m_hofferq_ulib = hofferq;             m_data[CP_HOFFERQU_IOLS] = hofferq; }
    void setBarrettLF_ulib(double barrettlf)            { m_barrettlf_ulib = barrettlf;         m_data[CP_BARRETTLFU_IOLS] = barrettlf; }
    void setBarrettDF_ulib(double barrettdf)            { m_barrettdf_ulib = barrettdf;         m_data[CP_BARRETTDFU_IOLS] = barrettdf; }
    void setOlsen_ulib(double olsen)                    { m_olsen_ulib = olsen;                 m_data[CP_OLSENU_IOLS] = olsen; }

    void setresults_optimized(int results)              { m_optimizedresults = results;         m_data[CP_RESULTSO_IOLS] = results; }
    void setCsteAopt_optimized(double csteAopt)         { m_csteAopt_optimized = csteAopt;      m_data[CP_CSTEAOPTO_IOLS] = csteAopt; }
    void setHaigisa0_optimized(double haigisa0)         { m_haigisa0_optimized = haigisa0;      m_data[CP_HAIGISA0O_IOLS] = haigisa0; }
    void setHaigisa1_optimized(double haigisa1)         { m_haigisa1_optimized = haigisa1;      m_data[CP_HAIGISA1O_IOLS] = haigisa1; }
    void setHaigisa2_optimized(double haigisa2)         { m_haigisa2_optimized = haigisa2;      m_data[CP_HAIGISA2O_IOLS] = haigisa2; }
    void setHolladay1_optimized(double holladay1)       { m_holladay1_optimized = holladay1;    m_data[CP_HOLL1O_IOLS] = holladay1; }
    void setHofferQ_optimized(double hofferq)           { m_hofferq_optimized = hofferq;        m_data[CP_HOFFERQO_IOLS] = hofferq; }


    void setAcd(double acd)                             { m_acd = acd;                          m_data[CP_ACD_IOLS] = acd; }
    void setDiainjecteur(double diainjecteur)           { m_diainjecteur = diainjecteur;        m_data[CP_DIAINJECTEUR_IOLS] = diainjecteur; }
    void setDiaall(double diaall)                       { m_diaall = diaall;                    m_data[CP_DIAALL_IOLS] = diaall; }
    void setOpticalDiameter(double opticaldiameter)     { m_opticaldiameter = opticaldiameter;  m_data[CP_DIAOPT_IOLS] = opticaldiameter; }
    void setArrayImgiol(const QByteArray &imgiol)       { m_arrayimgiol = imgiol;               m_data[CP_ARRAYIMG_IOLS] = QLatin1String(imgiol.toBase64()); }
    void setimageformat(const QString &typeimage)       { m_imageformat = typeimage;            m_data[CP_TYPIMG_IOLS] = typeimage; }
    void setOpticalMaterial(const QString &material)    { m_opticmaterial = material;           m_data[CP_OPTICMATERIAU_IOLS] = material; }
    void setHydrofily(const QString &hydrofily)         { m_hydrofily = hydrofily;              m_data[CP_HYDROFILY_IOLS] = hydrofily; }
    void setHapticalMaterial(const QString &material)   { m_hapticmaterial = material;          m_data[CP_HAPTICMATERIAU_IOLS] = material; }
    void setsinglepiece(bool &singlepiece)              { m_singlepiece = singlepiece;          m_data[CP_SINGLEPIECE_IOLS] = singlepiece; }
    void setRemarque(const QString &remarque)           { m_remarque = remarque;                m_data[CP_REMARQUE_IOLS] = remarque; }
    void setpreloaded(bool &preloaded)                  { m_preloaded = preloaded;              m_data[CP_PRECHARGE_IOLS] = preloaded; }
    void setyellow(bool &yellow)                        { m_yellow = yellow;                    m_data[CP_JAUNE_IOLS] = yellow; }
    void setMultifocal(bool &multifocal)                { m_multifocal = multifocal;            m_data[CP_MULTIFOCAL_IOLS] = multifocal; }
    void setToric(bool &toric)                          { m_toric = toric;                      m_data[CP_TORIC_IOLS] = toric; }
    void setEdof(bool &edof)                            { m_edof = edof;                        m_data[CP_EDOF_IOLS] = edof; }
    void setTypeInt(int type)                           { m_typeInt = type;                     m_data[CP_TYP_IOLS] = type; }
    void setType(const QString &type)
    {
        if (type ==IOL_CP)                  { setTypeInt(1); }
        else if (type ==IOL_CA)             { setTypeInt(2); }
        else if (type ==IOL_ADDON)          { setTypeInt(3); }
        else if (type ==IOL_IRIEN)          { setTypeInt(4); }
        else if (type ==IOL_CAREFRACTIF)    { setTypeInt(5); }
        else if (type ==IOL_AUTRE)          { setTypeInt(6); }
        else                                { setTypeInt(0); }
    }

    QImage image() const                        { return m_currentimage; }
    //! Un setter conforme les données à ce que l'objet attend. Ici : on stocke SYSTÉMATIQUEMENT
    //! l'image bitmap en JPEG comprimé sous le seuil (SIZEMAXIMGIOL) — une seule fois dans la vie de
    //! l'image (relue depuis la base, elle est déjà sous le seuil → le test de taille passe, pas de
    //! recompression). Le champ type (m_imageformat) tranche le cas PDF : un cliché PDF n'est pas
    //! recomprimé (m_arrayimgiol garde les octets du PDF), setimage n'en fournit que l'affichable.
    void setimage(QImage img = QImage())
    {
        if (img.isNull())
        {
            m_currentimage = m_nullimage;
            m_arrayimgiol  = QByteArray();
            m_imageformat  = "";
        }
        else if (m_imageformat == PDF)
            m_currentimage = img;
        else
        {
            m_currentimage = img;
            m_arrayimgiol  = Utils::CompressImageToJPG(img, SIZEMAXIMGIOL);
            m_imageformat  = JPG;
        }
        m_data[CP_ARRAYIMG_IOLS] = QLatin1String(m_arrayimgiol.toBase64());
        m_data[CP_TYPIMG_IOLS]   = m_imageformat;
    }
    void resetdatas();
    bool isnull() const                         { return m_id == 0; }
    bool hasnoregisteredcsteA()                 { return (m_csteAEcho_nominal == 0
                                                        && m_csteAopt_nominal == 0
                                                        && m_csteAopt_ulib == 0
                                                        && m_csteAEcho_ulib == 0
                                                        && m_csteAopt_optimized == 0);
                                                }
};
#endif // IOL_H
