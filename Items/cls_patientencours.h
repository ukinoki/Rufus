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

#ifndef CLS_PATIENTENCOURS_H
#define CLS_PATIENTENCOURS_H

#include "cls_item.h"


/*!
 * \brief Patientencours class
 * l'ensemble des patients présents dans le centre à l'instant: salle d'attente, en cours d'examen, ou en cours de paperasserie post-acte médical à l'accueil
 */

class PatientEnCours : public Item
{
    Q_OBJECT
private:
    int m_idusersuperviseur     = 0;            //!< le user surperviseur pour lequel le patient consulte
    QString m_statut            = "";
    QTime m_heurestatut         = QTime();
    QTime m_heurerdv            = QTime();
    QTime m_heurearrivee        = QTime();
    QString m_motif             = "";
    QString m_message           = "";
    int m_idacteapayer          = 0;
    QString m_posteexamen       = "";
    int m_iduserencoursexam     = 0;             //!< le user en train d'examiner le patient
    int m_idsaldat              = 0;

public:
    explicit PatientEnCours(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int idusersuperviseur() const       { return m_idusersuperviseur; }
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

    void setid(int id)                  { m_id = id;
                                          m_data[CP_IDPAT_SALDAT] = id; }
    void setidusersuperviseur(int id)   { m_idusersuperviseur = id;
                                          m_data[CP_IDUSERSUPERVISEUR_SALDAT] = id; }
    void setstatut(QString txt)         { m_statut = txt;
                                          m_data[CP_STATUT_SALDAT] = txt; }
    void setheurestatut(QTime time)     { m_heurestatut = time;
                                          m_data[CP_HEURESTATUT_SALDAT] = time.toString("HH:mm:ss"); }
    void setheurerdv(QTime time)        { m_heurerdv = time;
                                          m_data[CP_HEURERDV_SALDAT] = time.toString("HH:mm:ss"); }
    void setheurearrivee(QTime time)    { m_heurearrivee = time;
                                          m_data[CP_HEUREARRIVEE_SALDAT] = time.toString("HH:mm:ss"); }
    void setmotif(QString txt)          { m_motif = txt;
                                          m_data[CP_MOTIF_SALDAT] = txt; }
    void setmessage(QString txt)        { m_message = txt;
                                          m_data[CP_MESSAGE_SALDAT] = txt; }
    void setidacteapayer(int id)        { m_idacteapayer = id;
                                          m_data[CP_IDACTEAPAYER_SALDAT] = id; }
    void setposteexamen(QString txt)    { m_posteexamen = txt;
                                          m_data[CP_POSTEEXAMEN_SALDAT] = txt; }
    void setiduserencoursexam(int id)   { m_iduserencoursexam = id;
                                          m_data[CP_IDUSERENCOURSEXAM_SALDAT] = id; }
    void setidsaldat(int id)            { m_idsaldat = id;
                                          m_data[CP_IDSALDAT_SALDAT] = id; }
};

#endif // CLS_PATIENTENCOURS_H
