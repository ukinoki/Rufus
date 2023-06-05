#ifndef CLS_RENDEZVOUS_H
#define CLS_RENDEZVOUS_H

#include "cls_item.h"

class RendezVous : public Item
{
    Q_OBJECT
private:
    QString m_titreexamen;
    QString m_nomappareil;
    QString m_nomdossierechange;

public:
    explicit RendezVous(int idpatient = 0, QString motif = "", QDate ddnpatient = QDate(), QTime heurerdv = QTime(), QTime heurearrivee = QTime(), QString message = "", int idsuperviseur = 0, bool urgence = false)
    {
        m_idpatient = idpatient;
        m_motif = motif;
        m_ddnpatient = ddnpatient;
        m_heurerdv = heurerdv;
        m_heurearrivee = heurearrivee;
        m_message = message;
        m_idsuperviseur = idsuperviseur;
        m_urgence = urgence;
    }
    int m_idpatient = 0;                        //!< le pateint qui fait l'objet dur le message
    QString m_motif = "";                       //!< le motif du message
    QDate m_ddnpatient = QDate();               //!< la date de naissance du patient
    QTime m_heurerdv = QTime();                 //!> l'heure du rdv
    QTime m_heurearrivee = QTime();             //!> l'heure d'arrivée
    QString m_message = "";                     //!> message concernant le rdv
    int m_idsuperviseur = 0;                    //!> le soigant avec lequel la patient a rdv
    bool m_urgence = false;                     //! le rdv est une urgence

    int idpatient() const                           { return m_idpatient; }
    void setIdpatient(int idpatient)                { m_idpatient = idpatient; }
    QString motif() const                           { return m_motif; }
    void setMotif(const QString motif)              { m_motif = motif; }

    QDate ddnpatient() const                        { return m_ddnpatient; }
    void setDdnpatient(const QDate ddnpatient)      { m_ddnpatient = ddnpatient; }

    QTime heurerdv() const                          { return m_heurerdv; }
    void setHeurerdv(const QTime heurerdv)          { m_heurerdv = heurerdv; }

    QTime heurearrivee() const                      { return m_heurearrivee; }
    void setHeurearrivee(const QTime heurearrivee)  { m_heurearrivee = heurearrivee; }

    QString message() const                         { return m_message; }
    void setMessage(const QString message)          { m_message = message; }

    int idsuperviseur() const                       { return m_idsuperviseur; }
    void setIdsuperviseur(int idsuperviseur)        { m_idsuperviseur = idsuperviseur; }

    bool urgence() const                            { return m_urgence; }
    void setUrgence(bool urgence)                   { m_urgence = urgence; }
};

#endif // CLS_RENDEZVOUS_H











