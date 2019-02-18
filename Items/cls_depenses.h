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

#ifndef CLS_DEPENSES_H
#define CLS_DEPENSES_H

#include <QObject>
#include "cls_depense.h"

class Depenses : public QObject
{
public:
    QMap<int, Depense*> *m_Depenses = Q_NULLPTR;    //!< Collection de toutes les depenses pour un user et une année donnée

public:
    //GETTER
    QMap<int, Depense *> *getDepenses()     const;

    Depenses(QObject *parent = Q_NULLPTR);

    bool        addDepense(Depense *Depense);
    Depense*    getDepenseById(int id);
    void        clearAll();
    void        removeDepense(Depense *dep);
};

#endif // CLS_DEPENSES_H
