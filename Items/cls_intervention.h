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

#ifndef INTERVENTION_H
#define INTERVENTION_H

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
    QDate m_date            = QDate();          //!< Date de l'intervention
    int m_iduser            = 0;                //! id du chirurgien
    int m_idpatient         = 0;                //! id du patient
    int m_idlieu            = 0;                //! id du lieu de l'intervention
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

    QDate date() const                      { return m_date; }
    int iduser() const                      { return m_iduser; }
    int idpatient() const                   { return m_idpatient; }
    int idlieu() const                      { return m_idlieu; }
    ModeAnesthesie anesthesie() const       { return m_anesth; }
    int idtypeintervention() const          { return m_typeintervention; }
    Utils::Cote cote() const                { return m_cote; }
    int idIOL() const                       { return m_idIOL; }
    double puissanceIOL() const             { return m_pwrIOL; }
    double cylindreIOL() const              { return m_cylIOL; }
    QString observation() const             { return m_observation; }

    void setdate(QDate date)                { m_date = date; }
    void setiduser(int id)                  { m_iduser = id; }
    void setidpatient(int id)               { m_idpatient = id; }
    void setidlieu(int id)                  { m_idlieu = id; }
    void anesthesie(ModeAnesthesie mode)    { m_anesth = mode; }
    void setidtypeintervention(int id)      { m_typeintervention = id; }
    void cote(Utils::Cote cote)             { m_cote = cote; }
    void setidIOL(int id)                   { m_idIOL = id; }
    void puissanceIOL(double pwr)           { m_pwrIOL = pwr; }
    void cylindreIOL(double cyl)            { m_cylIOL = cyl; }
    void observation(QString txt)           { m_observation = txt; }

    void resetdatas();
    bool isnull() const                     { return m_id == 0; }

 };

#endif // INTERVENTION_H
