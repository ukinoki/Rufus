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


#ifndef CLS_MOTIF_H
#define CLS_MOTIF_H

#include "cls_item.h"

/*!
 * \brief Motif class
 * l'ensemble des informations concernant un motif d'acte
 */


class Motif : public Item
{

private:
    int m_id, m_duree, m_noordre;
    QString m_motif, m_raccourci, m_couleur;
    bool m_pardefaut, m_utiliser;

public:
    explicit Motif(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);

    int id() const;
    QString motif() const;
    QString raccourci() const;
    QString couleur() const;
    int duree() const;
    bool pardefaut() const;
    bool utiliser() const;
    int noordre() const;
};

#endif // CLS_MOTIF_H
