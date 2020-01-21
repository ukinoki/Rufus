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

#ifndef CLS_COMPTE_H
#define CLS_COMPTE_H

#include "cls_item.h"

class Compte : public Item
{

private:
    int m_idbanque, m_iduser;
    QString m_nomabrege, m_iban, m_intitulecompte;
    bool m_desactive, m_partage;
    double m_solde;

public:
    explicit Compte(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int idBanque() const;
    int idUser() const;                         //!< l'utilisateur qui a créé le compte
    QString iban() const;
    QString intitulecompte() const;
    QString nomabrege() const;
    double solde() const;
    bool isPartage() const;
    bool isDesactive() const;

    void setid(int id)                  { m_id = id;
                                          m_data[CP_IDCOMPTE_COMPTES] = id; }
    void setidbanque(int id)            { m_idbanque = id;
                                          m_data[CP_IDBANQUE_COMPTES] = id; }
    void setiduser(int id)              { m_iduser = id;
                                          m_data[CP_IDUSER_COMPTES] = id; }
    void setiban(QString txt)           { m_iban = txt;
                                          m_data[CP_IBAN_COMPTES] = txt; }
    void setintitulecompte(QString txt) { m_intitulecompte = txt;
                                          m_data[CP_INTITULE_COMPTES] = txt; }
    void setnomabrege(QString txt)      { m_nomabrege = txt;
                                          m_data[CP_NOMABREGE_COMPTES] = txt; }
    void setsolde(double solde)         { m_solde = solde;
                                          m_data[CP_SOLDE_COMPTES] = solde; }
    void setpartage(bool logic)         { m_partage = logic;
                                          m_data[CP_PARTAGE_COMPTES] = logic; }
    void setdesactive(bool logic)       { m_desactive = logic;
                                          m_data[CP_DESACTIVE_COMPTES] = logic; }
};

#endif // CLS_COMPTE_H
