/* (C) 2018 LAINE SERGE
This file is part of Rufus.

Rufus is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rufus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rufus. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CLS_DEPENSES_H
#define CLS_DEPENSES_H

#include <QObject>
#include "cls_depense.h"

class Depenses : public QObject
{
public:
    QHash<int, Depense*> *m_Depenses;    //!< Collection de toutes les depenses pour unuser et une  année donnée - le QHash plutôt que le QMap pour garder le tri par date

public:
    //GETTER
    QHash<int, Depense *> *getDepenses()     const;

    Depenses(QObject *parent = Q_NULLPTR);

    bool addDepense(Depense *Depense);
    Depense* getDepenseById(int id);

};

#endif // CLS_DEPENSES_H
