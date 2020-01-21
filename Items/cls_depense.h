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

#ifndef CLS_DEPENSE_H
#define CLS_DEPENSE_H

#include "cls_item.h"

/*!
 * \brief The depense class
 * l'ensemble des informations concernant une dépense
 */

class Depense : public Item
{


public:
    enum AuxArchives {NoLoSo, Oui, Non};    Q_ENUM(AuxArchives)
private: //Données de la dépense

    int m_iduser;               //!< id du user qui a fait la dépense (ce user est comptable)
    QDate m_datedepepense;      //!< date le la dépense
    QString m_rubriquefiscale;  //!< la rubriquee fiscale 2035 de la dépense
    int m_idrubriquefiscale;    //!< l'id de la rubrique fiscale 2035 de la dépense
    QString m_objetdepense;     //!< l'objet de la dépense
    double m_montant;           //!> le montant de la dépense
    QString m_famillefiscale;   //!< la famille de la référence fiscale de la dépense
    QString m_monnaie;          //!< monnaie utilisée (F ou €)
    int m_idRec;                //!< Ce champ sert à stocker l’idRec de LignesRecettes d’une commission sur la recette IdRec. Si on veut supprimer la recette idRec, on retrouve ainsi sans pb la commission Depensee.
    QString m_modepaiement;     //!< mode de paiement: V virement, C chèque, P prélèvement, T TIP, B carte de crédit, E espèces
    int m_compte;               //!< le compte bancaire concerné
    int m_nocheque;             //!< le no du chèque
    int m_idfacture;            //!< l'id de la facture ou de l'échéancier correspondant dans la table Facture
    QString m_lienfacture;      //!< l'emplacement de la facture ou de l'échéancier correspondant sur le disque dur
    bool m_echeancier;          //!< bool -> true = echeancier - false = facture
    AuxArchives m_auxarchives;  //!< depense par operation bancaire et dont l'operation bancaire est consolidée
    QString m_objetecheancier;  //!< l'intitule de l'échéancier correspondant sur le disque dur
    QString m_formatfacture;    //!< la facture est un jpg ou un pdf
    QByteArray m_blob;          //!< le contenu du fichier image de la facture

public:
    //GETTER | SETTER
    int     iduser() const;
    QDate   date() const;
    QString rubriquefiscale() const;
    int     idrubriquefiscale() const;
    QString objet() const;
    double  montant() const;
    QString famillefiscale() const;
    QString monnaie() const;
    int     idrecette() const;
    QString modepaiement() const;
    int     comptebancaire() const;
    int     nocheque() const;
    int     idfacture() const;
    QString lienfacture() const;
    bool    isecheancier() const;
    QString objetecheancier() const;
    int     annee() const;
    int     isArchivee() const;
    QString factureformat() const;
    QByteArray  factureblob() const;

    void    setiduser(int id)               { m_iduser = id; }
    void    setdate(QDate date)             { m_datedepepense = date; }
    void    setrubriquefiscale(QString txt) { m_rubriquefiscale = txt; }
    void    setidrubriquefiscale(int id)    { m_idrubriquefiscale = id; }
    void    setobjet(QString txt)           { m_objetdepense = txt; }
    void    setmontant(double montant)      { m_montant = montant; }
    void    setfamillefiscale(QString txt)  { m_famillefiscale = txt; }
    void    setmonnaie(QString txt)         { m_monnaie = txt; }
    void    setidrecette(int id)            { m_idRec = id; }
    void    setmodepaiement(QString txt)    { m_modepaiement = txt; }
    void    setidcomptebancaire(int id)     { m_compte = id; }
    void    setnocheque(int id)             { m_nocheque = id; }

    void    setarchivee(bool arch);
    void    setidfacture(int idfacture);
    void    setlienfacture(QString lien);
    void    setecheancier(bool ech);
    void    setobjetecheancier(QString obj);
    void    setfactureformat(QString typeimg);
    void    setfactureblob(QByteArray ba);

    explicit Depense(QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data);
};

#endif // CLS_DEPENSE_H
