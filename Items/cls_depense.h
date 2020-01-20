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

    void    setid(int id)                   { m_id = id;
                                              m_data[CP_IDDEPENSE_DEPENSES] = id; }
    void    setiduser(int id)               { m_iduser = id;
                                              m_data[CP_IDUSER_DEPENSES] = id; }
    void    setdate(QDate date)             { m_datedepepense = date;
                                              m_data[CP_DATE_DEPENSES] = date.toString("yyyy-MM-dd"); }
    void    setrubriquefiscale(QString txt) { m_rubriquefiscale = txt;
                                              m_data[CP_REFFISCALE_DEPENSES] = txt; }
    void    setidrubriquefiscale(int id)    { m_idrubriquefiscale = id;
                                              m_data["idrubrique"] = id; }
    void    setobjet(QString txt)           { m_objetdepense = txt;
                                              m_data[CP_OBJET_DEPENSES] = txt; }
    void    setmontant(double montant)      { m_montant = montant;
                                              m_data[CP_MONTANT_DEPENSES] = montant; }
    void    setfamillefiscale(QString txt)  { m_famillefiscale = txt;
                                              m_data[CP_FAMILLEFISCALE_DEPENSES] = txt; }
    void    setmonnaie(QString txt)         { m_monnaie = txt;
                                              m_data[CP_MONNAIE_DEPENSES] = txt; }
    void    setidrecette(int id)            { m_idRec = id;
                                              m_data[CP_IDRECETTE_DEPENSES] = id; }
    void    setmodepaiement(QString txt)    { m_modepaiement = txt;
                                              m_data[CP_MODEPAIEMENT_DEPENSES] = txt; }
    void    setidcomptebancaire(int id)     { m_compte = id;
                                              m_data[CP_COMPTE_DEPENSES] = id; }
    void    setnocheque(int id)             { m_nocheque = id;
                                              m_data[CP_NUMCHEQUE_DEPENSES] = id; }
    void    setidfacture(int id)            { m_idfacture = id;
                                              m_data[CP_IDFACTURE_DEPENSES] = id; }
    void    setlienfacture(QString txt)     { m_lienfacture = txt;
                                              m_data[CP_LIENFICHIER_FACTURES] = txt; }
    void    setecheancier(bool logic)       { m_echeancier = logic;
                                              m_data[CP_ECHEANCIER_FACTURES] = logic; }
    void    setobjetecheancier(QString txt) { m_objetecheancier = txt;
                                              m_data[CP_INTITULE_FACTURES] = txt; }

    void    setarchivee(bool logic)         { m_auxarchives = (logic? Depense::Oui : Depense::Non);}
    void    setfactureformat(QString fmtimg){ m_formatfacture = fmtimg;}
    void    setfactureblob(QByteArray ba)   { m_blob = ba;}

    explicit Depense(QJsonObject data = {}, QObject *parent = Q_NULLPTR);

    void setData(QJsonObject data);
};

#endif // CLS_DEPENSE_H
