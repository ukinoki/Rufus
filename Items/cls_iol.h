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
    //!<m_id = Id de l'ntervention en base
    int m_idmanufacturer    = 0;                //! id du fabricant
    int m_iddistributeur    = 0;                //! id du distributeur du fabricant - non stocké en base parce que redondant
    QString m_modele        = "";               //! modèle de l'implant
    bool m_inactif          = false;
    double m_pwrmax         = 0;                //! puissance maximale de l'implant
    double m_pwrmin         = 0;                //! puissance minimale de l'implant
    double m_pwrstp         = 0;                //! pas de variation de la puissance l'implant
    double m_cylmax         = 0;                //! cylindre maximale de l'implant
    double m_cylmin         = 0;                //! cylinde minimale de l'implant
    double m_cylstp         = 0;                //! pas de variation du cylindre l'implant
    double m_csteAopt       = 0;                //! constante A optique
    double m_csteAEcho      = 0;                //! constante A echographique
    double m_haigisa0       = 0;                //! constante Haigis a0
    double m_haigisa1       = 0;                //! constante Haigis a1
    double m_haigisa2       = 0;                //! constante Haigis a2
    double m_holladay       = 0;                //! constante Holladay
    double m_acd            = 0;                //! ACD
    double m_diainjecteur   = 0;                //! diamètre minimal de l'injecteur
    double m_diaall         = 0;                //! diamètre hors tout
    double m_diaoptique     = 0;                //! diamètre optique
    QByteArray m_arrayimgiol= QByteArray();     //! le cliché de l'IOL
    QString m_imageformat   = "";               //! le type de cliché, jpg ou pdf
    QString m_materiau      = "";               //! le matériau de l'IOL
    QString m_remarque      = "";               //! remarque à propos de l'IOL
    bool m_precharge        = false;            //! préchargé
    bool m_jaune            = false;            //! jaune ou clair
    bool m_multifocal       = false;            //! multifocal
    bool m_edof             = false;            //! edof
    bool m_toric            = false;            //! toric
    QString m_type          = "";               //! le type de l'IOL : CP, CA, addon, support irien, refractif ca
    QImage m_nullimage      = QImage("://IOL.png");
    QImage m_currentimage   = m_nullimage;


public:
    int idmanufacturer() const                  { return m_idmanufacturer; }
    int iddistributeur() const                  { return m_iddistributeur; }
    QString modele() const                      { return m_modele; }
    double pwrmax() const                       { return m_pwrmax; }
    double pwrmin() const                       { return m_pwrmin; }
    double pwrstp() const                       { return m_pwrstp; }
    double cylmax() const                       { return m_cylmax; }
    double cylmin() const                       { return m_cylmin; }
    double cylstp() const                       { return m_cylstp; }
    double csteAopt() const                     { return m_csteAopt; }
    double csteAEcho() const                    { return m_csteAEcho; }
    double haigisa0() const                     { return m_haigisa0; }
    double haigisa1() const                     { return m_haigisa1; }
    double haigisa2() const                     { return m_haigisa2; }
    double holladay() const                     { return m_holladay; }
    double acd() const                          { return m_acd; }
    double diainjecteur() const                 { return m_diainjecteur; }
    double diaall() const                       { return m_diaall; }
    double diaoptique() const                   { return m_diaoptique; }
    QByteArray arrayimgiol() const              { return m_arrayimgiol; }
    QString imageformat() const                 { return m_imageformat; }
    QString materiau() const                    { return m_materiau; }
    QString remarque() const                    { return m_remarque; }
    bool isactif() const                        { return !m_inactif; }
    bool isprecharge() const                    { return m_precharge; }
    bool isjaune() const                        { return m_jaune; }
    bool ismultifocal() const                   { return m_multifocal; }
    bool isedof() const                         { return m_edof; }
    bool istoric() const                        { return m_toric; }
    QString type() const                        { return m_type; }
    QString tooltip(bool avecimage = false) const;


    void setidmanufacturer(int &id)             { m_idmanufacturer = id;            m_data[CP_IDMANUFACTURER_IOLS] = id; }
    void setidistributeur(int id)               { m_iddistributeur = id;}
    void setmodele(const QString &txt)          { m_modele = txt;                   m_data[CP_MODELNAME_IOLS] = txt; }
    void setactif(bool &actif)                  { m_inactif = !actif;               m_data[CP_INACTIF_IOLS] = !actif; }
    void setPwrmax(double pwrmax)               { m_pwrmax = pwrmax;                m_data[CP_MAXPWR_IOLS] = pwrmax; }
    void setPwrmin(double pwrmin)               { m_pwrmin = pwrmin;                m_data[CP_MINPWR_IOLS] = pwrmin; }
    void setPwrstp(double pwrstp)               { m_pwrstp = pwrstp;                m_data[CP_PWRSTEP_IOLS] = pwrstp; }
    void setCylmax(double cylmax)               { m_cylmax = cylmax;                m_data[CP_MAXCYL_IOLS] = cylmax; }
    void setCylmin(double cylmin)               { m_cylmin = cylmin;                m_data[CP_MINCYL_IOLS] = cylmin; }
    void setCylstp(double cylstp)               { m_cylstp = cylstp;                m_data[CP_CYLSTEP_IOLS] = cylstp; }
    void setCsteAopt(double csteAopt)           { m_csteAopt = csteAopt;            m_data[CP_CSTEAOPT_IOLS] = csteAopt; }
    void setCsteAEcho(double csteAEcho)         { m_csteAEcho = csteAEcho;          m_data[CP_CSTEAOPT_IOLS] = csteAEcho; }
    void setHaigisa0(double haigisa0)           { m_haigisa0 = haigisa0;            m_data[CP_HAIGISA0_IOLS] = haigisa0; }
    void setHaigisa1(double haigisa1)           { m_haigisa1 = haigisa1;            m_data[CP_HAIGISA1_IOLS] = haigisa1; }
    void setHaigisa2(double haigisa2)           { m_haigisa2 = haigisa2;            m_data[CP_HAIGISA2_IOLS] = haigisa2; }
    void setHolladay(double holladay)           { m_holladay = holladay;            m_data[CP_HOLL1_IOLS] = holladay; }
    void setAcd(double acd)                     { m_acd = acd;                      m_data[CP_ACD_IOLS] = acd; }
    void setDiainjecteur(double diainjecteur)   { m_diainjecteur = diainjecteur;    m_data[CP_DIAINJECTEUR_IOLS] = diainjecteur; }
    void setDiaall(double diaall)               { m_diaall = diaall;                m_data[CP_DIAALL_IOLS] = diaall; }
    void setDiaoptique(double diaoptique)       { m_diaoptique = diaoptique;        m_data[CP_DIAOPT_IOLS] = diaoptique; }
    void setArrayImgiol(const QByteArray &imgiol){ m_arrayimgiol = imgiol;          m_data[CP_ARRAYIMG_IOLS] = QLatin1String(imgiol.toBase64()); }
    void setimageformat(const QString &typeimage) { m_imageformat = typeimage;        m_data[CP_TYPIMG_IOLS] = typeimage; }
    void setMateriau(const QString &materiau)   { m_materiau = materiau;            m_data[CP_MATERIAU_IOLS] = materiau; }
    void setRemarque(const QString &remarque)   { m_remarque = remarque;            m_data[CP_REMARQUE_IOLS] = remarque; }
    void setprecharge(bool &precharge)          { m_precharge = precharge;          m_data[CP_PRECHARGE_IOLS] = precharge; }
    void setjaune(bool &jaune)                  { m_jaune = jaune;                  m_data[CP_JAUNE_IOLS] = jaune; }
    void setMultifocal(bool &multifocal)        { m_multifocal = multifocal;        m_data[CP_MULTIFOCAL_IOLS] = multifocal; }
    void setToric(bool &toric)                  { m_toric = toric;                  m_data[CP_TORIC_IOLS] = toric; }
    void setEdof(bool &edof)                    { m_edof = edof;                    m_data[CP_EDOF_IOLS] = edof; }
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
