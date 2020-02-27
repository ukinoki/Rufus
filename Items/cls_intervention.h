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

#ifndef CLS_INTERVENTION_H
#define CLS_INTERVENTION_H

#include "cls_item.h"
#include "utils.h"

/*!
 * \brief classe Intervention
 * l'ensemble des informations concernant une intervention
 */

class Intervention : public Item
{

public:
    enum ModeAnesthesie {Locale, LocoRegionale, Generale, NoLoSo};      Q_ENUM(ModeAnesthesie)
    explicit Intervention(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data = QJsonObject{});

private: //Données de l'intervention
    //!<m_id = Id de l'ntervention en base
    QTime m_heure           = QTime();          //! l'heure de l'intervention
    int m_idsession         = 0;                //! id de la session
    int m_idpatient         = 0;                //! id du patient
    ModeAnesthesie m_anesth = NoLoSo;           //! le type d'anesthésie
    int m_typeintervention  = 0;                //! id du type d'intervention
    Utils::Cote m_cote      = Utils::NoLoSo;    //! côté de l'intervention
    int m_idIOL             = 0;                //! id de l'IOL
    double m_pwrIOL         = 0;                //! puissance de l'implant
    double m_cylIOL         = 0;                //! cylindre de l'implant
    QString m_observation   = "";               //! observation
    ModeAnesthesie          ConvertModeAnesthesie(QString mode);
    QString                 ConvertModeAnesthesie(ModeAnesthesie mode);

public:
    QTime heure() const                     { return m_heure; }
    int idpatient() const                   { return m_idpatient; }
    int idsession() const                   { return m_idsession; }
    ModeAnesthesie anesthesie() const       { return m_anesth; }
    int idtypeintervention() const          { return m_typeintervention; }
    Utils::Cote cote() const                { return m_cote; }
    int idIOL() const                       { return m_idIOL; }
    double puissanceIOL() const             { return m_pwrIOL; }
    double cylindreIOL() const              { return m_cylIOL; }
    QString observation() const             { return m_observation; }

    void setheure(QTime time)               { m_heure = time;
                                              m_data[CP_HEURE_LIGNPRGOPERATOIRE] = time.toString("HH:mm::ss"); }
    void setidpatient(int id)               { m_idpatient = id;
                                              m_data[CP_IDPATIENT_LIGNPRGOPERATOIRE] = id; }
    void setidsession(int id)               { m_idsession = id;
                                              m_data[CP_IDSESSION_LIGNPRGOPERATOIRE] = id; }
    void anesthesie(ModeAnesthesie mode)    { m_anesth = mode;
                                              m_data[CP_TYPEANESTH_LIGNPRGOPERATOIRE] = ConvertModeAnesthesie(mode); }
    void setidtypeintervention(int id)      { m_typeintervention = id;
                                              m_data[CP_TYPEINTERVENTION_LIGNPRGOPERATOIRE] = id; }
    void cote(Utils::Cote cote)             { m_cote = cote;
                                              m_data[CP_COTE_LIGNPRGOPERATOIRE] = Utils::ConvertCote(cote); }
    void setidIOL(int id)                   { m_idIOL = id;
                                              m_data[CP_IDIOL_LIGNPRGOPERATOIRE] = id; }
    void puissanceIOL(double pwr)           { m_pwrIOL = pwr;
                                              m_data[CP_PWRIOL_LIGNPRGOPERATOIRE] = pwr; }
    void cylindreIOL(double cyl)            { m_cylIOL = cyl;
                                              m_data[CP_CYLIOL_LIGNPRGOPERATOIRE] = cyl; }
    void observation(QString txt)           { m_observation = txt;
                                              m_data[CP_OBSERV_LIGNPRGOPERATOIRE] = txt; }

    void resetdatas();
    bool isnull() const                     { return m_id == 0; }
 };

class SessionOperatoire : public Item
{

public:
    explicit SessionOperatoire(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data = QJsonObject{})
    {
        if( data.isEmpty() )
            return;
        Utils::setDataInt(data, CP_ID_SESSIONOPERATOIRE, m_id);
        Utils::setDataInt(data, CP_IDLIEU_SESSIONOPERATOIRE, m_idlieu);
        Utils::setDataInt(data, CP_IDUSER_SESSIONOPERATOIRE, m_iduser);
        Utils::setDataInt(data, CP_IDAIDE_SESSIONOPERATOIRE, m_idaide);
        m_data = data;
    }


private: //Données de la session
    //!<m_id = Id de l'ntervention en base
    QDate m_date            = QDate();          //!< Date de l'intervention
    int m_idlieu            = 0;                //! id du lieu de l'intervention
    int m_iduser            = 0;                //! id du chirurgien
    int m_idaide            = 0;                //! id de l'aide opératoire

public:
    QDate date() const                      { return m_date; }
    int idlieu() const                      { return m_idlieu; }
    int iduser() const                      { return m_iduser; }
    int idaide() const                      { return m_idaide; }

    void setdate(QDate date)                { m_date = date;
                                              m_data[CP_DATE_SESSIONOPERATOIRE] = date.toString("yyyy-MM-dd"); }
    void setidlieu(int id)                  { m_idlieu = id;
                                              m_data[CP_IDLIEU_SESSIONOPERATOIRE] = id; }
    void setiduser(int id)                  { m_iduser = id;
                                              m_data[CP_IDUSER_SESSIONOPERATOIRE] = id; }
    void setidaide(int id)                  { m_iduser = id;
                                              m_data[CP_IDAIDE_SESSIONOPERATOIRE] = id; }
    void resetdatas()
    {
        QJsonObject data;
        data[CP_ID_SESSIONOPERATOIRE]           = 0;
        data[CP_IDLIEU_SESSIONOPERATOIRE]       = 0;
        data[CP_IDUSER_SESSIONOPERATOIRE]       = 0;
        data[CP_IDAIDE_SESSIONOPERATOIRE]       = 0;
        setData(data);
    }
    bool isnull() const                     { return m_id == 0; }
 };


/*!
 * \brief classe IOL
 * l'ensemble des informations concernant un IOL
 */

class IOL : public Item
{

public:
    explicit IOL(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data = QJsonObject{});

private: //Données de l'intervention
    //!<m_id = Id de l'ntervention en base
    int m_idmanufacturer    = 0;                //! id du frabricant
    QString m_modele        = "";               //! modèle de l'implant

public:

    int idmanufacturer() const              { return m_idmanufacturer; }
    QString modele() const                  { return m_modele; }

    void setidmanufacturer(int id)          { m_idmanufacturer = id;
                                              m_data[CP_IDMANUFACTURER_IOLS] = id; }
    void setmodele(QString txt)             { m_modele = txt;
                                              m_data[CP_MODELNAME_IOLS] = txt; }

    void resetdatas();
    bool isnull() const                     { return m_id == 0; }

 };

/*!
 * \brief classe TypeIntervention
 * l'ensemble des informations concernant un IOL
 */

class TypeIntervention : public Item
{

public:
    explicit TypeIntervention(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data = QJsonObject{});

private: //Données de l'intervention
    //!<m_id = Id de l'ntervention en base
    QString m_typeintervention  = "";                //! le type
    QString m_codeCCAM          = "";               //! code CCAM

public:

    QString typeintervention() const        { return m_typeintervention; }
    QString codeCCAM() const                { return m_codeCCAM; }

    void settypeintervention(QString txt)   { m_typeintervention = txt;
                                              m_data[CP_TYPEINTERVENTION_TYPINTERVENTION] = txt; }
    void setcodeCCAM(QString txt)           { m_codeCCAM = txt;
                                              m_data[CP_CODECCAM_TYPINTERVENTION] = txt; }

    void resetdatas();
    bool isnull() const                     { return m_id == 0; }

 };

#endif // CLS_INTERVENTION_H
