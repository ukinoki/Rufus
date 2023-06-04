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

#ifndef MOTCLE_H
#define MOTCLE_H

#include "cls_item.h"

/*!
 * \brief The MotCle class
 * l'ensemble des mots cles
 */

class MotCle : public Item
{
    Q_OBJECT
private:
    QString m_motcle = "";            //!< motclé

public:
    MotCle(QJsonObject data = {}, QObject *parent = Q_NULLPTR);
    void setData(QJsonObject data);
    void resetdatas();

    QString motcle() const              { return m_motcle; }
    void setmotcle(QString motcle)      { m_motcle = motcle;
                                          m_data[CP_TEXT_MOTCLE] = motcle; }
};


#endif // MOTCLE_H
