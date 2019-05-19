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
    int m_id, m_idbanque, m_iduser;
    QString m_nom, m_iban, m_intitulecompte, m_nombanque;
    bool m_desactive, m_partage;
    double m_solde;

public:
    explicit Compte(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);
    void setSolde(double solde);

    int id() const;
    int idBanque() const;
    int idUser() const;                         //!< l'utilisateur qui a créé le compte
    QString iban() const;
    QString intitulecompte() const;
    QString nom() const;
    double solde() const;
    bool isPartage() const;
    bool isDesactive() const;
    QString nombanque() const;
};

#endif // CLS_COMPTE_H
