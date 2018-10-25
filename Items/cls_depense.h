#ifndef CLS_DEPENSE_H
#define CLS_DEPENSE_H

#include "cls_item.h"

#include <QObject>

/*!
 * \brief The depense class
 * l'ensemble des informations concernant une dépense
 */

class Depense : public Item
{
    Q_OBJECT

private: //Données de la dépense
    int m_iddepense;            //!< id de la dépense
    int m_iduser;               //!< id du user qui a fait la dépense (ce user est comptable)
    QDate m_datedepepense;      //!< date le la dépense
    QString m_referencefiscale; //!< la référence fiscale 2035 de la dépense
    QString m_objetdepense;     //!< l'objet de la dépense
    double m_montant;           //!> le montant de la dépense
    QString m_famillefiscale;   //!< la famille de la référence fiscale de la dépense
    QString m_monnaie;          //!< monnaie utilisée (F ou €)
    int m_idRec;                //!< Ce champ sert à stocker l’idRec de LignesRecettes d’une commission sur la recette IdRec. Si on veut supprimer la recette idRec, on retrouve ainsi sans pb la commission Depensee.
    QString m_modepaiement;     //!< mode de paiement: V virement, C chèque, P prélèvement, T TIP, B carte de crédit, E espèces
    int m_compte;               //!< le compte bancaire concerné
    int m_nocheque;             //!< le no du chèque
    int m_auxarchives;          //!< depense par operation bancaire consolidée

public:
    enum m_auxarchives {NoLoSo, Oui, Non};
    //GETTER | SETTER
    int     id() const;
    int     iduser() const;
    QDate   date() const;
    QString reffiscale() const;
    QString objet() const;
    double  montant() const;
    QString famillefiscale() const;
    QString monnaie() const;
    int     idrecette() const;
    QString modepaiement() const;
    int     comptebancaire() const;
    int     nocheque();
    int     annee();
    int     isArchivee();
    void    setArchivee(bool arch);

    explicit Depense(QJsonObject data = {}, QObject *parent = nullptr);

    void setData(QJsonObject data);

};

#endif // CLS_DEPENSE_H
