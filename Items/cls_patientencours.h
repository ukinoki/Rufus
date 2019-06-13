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

#ifndef CLS_PATIENTENCOURS_H
#define CLS_PATIENTENCOURS_H

#include "cls_item.h"


/*!
 * \brief Patientencours class
 * l'ensemble des patients présents dans le centre à l'instant: salle d'attente, en cours d'exame, ou en cours de paperasserie post-acte médical à l'accueil
 */

class PatientEnCours : public Item
{

private:
    int m_iduser;
    QString m_statut;
    QTime m_heurestatut;
    QTime m_heurerdv;
    QTime m_heurearrivee;
    QString m_motif;
    QString m_message;
    int m_idacteapayer;
    QString m_posteexamen;
    int m_iduserencoursexam;
    int m_idsaldat;

public:
    explicit PatientEnCours(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int iduser() const                  { return m_iduser; }
    QString statut() const              { return m_statut; }
    QTime heurestatut() const           { return m_heurestatut; }
    QTime heurerdv() const              { return m_heurerdv; }
    QTime heurerarrivee() const         { return m_heurearrivee; }
    QString motif() const               { return m_motif; }
    QString message() const             { return m_message; }
    int idacteapayer() const            { return m_idacteapayer; }
    QString posteexamen() const         { return m_posteexamen; }
    int iduserencoursexam() const       { return m_iduserencoursexam; }
    int idsaldat() const                { return m_idsaldat; }

    void setiduser(int id)              { m_iduser = id; }
    void setstatut(QString txt)         { m_statut = txt; }
    void setheurestatut(QTime time)     { m_heurestatut = time; }
    void setheurerdv(QTime time)        { m_heurerdv = time; }
    void setheurearrivee(QTime time)    { m_heurearrivee = time; }
    void setmotif(QString txt)          { m_motif = txt; }
    void setmessage(QString txt)        { m_message = txt; }
    void setidacteapayer(int id)        { m_idacteapayer = id; }
    void setposteexamen(QString txt)    { m_posteexamen = txt; }
    void setiduserencoursexam(int id)   { m_iduserencoursexam = id; }
    void setidsaldat(int id)            { m_idsaldat = id; }
};

#endif // CLS_PATIENTENCOURS_H
