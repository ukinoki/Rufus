// COMMERCIAL_H
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

#ifndef COMMERCIAL_H
#define COMMERCIAL_H

#include "cls_item.h"
#include "macros.h"
/*!
 * \brief The Commercial class
 * correspond à un employé ou un représentant d'un fabricant: commercial, SAV,...etc...
 */
class Commercial : public Item
{
public:
    explicit Commercial(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    QString nom() const;
    QString prenom() const;
    QString statut() const;
    QString mail() const;
    QString telephone() const;
    QString tooltip() const;
    int idmanufacturer() const;

    void setnom(const QString &nom);
    void setprenom(const QString &corprenom);
    void setstatut(const QString &corstatut);
    void setmail(const QString &corstatut);
    void settelephone(const QString &cortelephone);
    void setidmanufactureur(const int id);
    void resetdatas();

    bool isnull() const   { return m_id == 0; }

private:
    QString m_nom               = "";
    QString m_prenom            = "";
    QString m_statut            = "";
    QString m_mail              = "";
    QString m_telephone         = "";
    int     m_idmanufacturer    = 0;
};

#endif // COMMERCIAL_H
