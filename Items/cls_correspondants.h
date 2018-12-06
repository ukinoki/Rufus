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

#ifndef CLS_CORRESPONDANTS_H
#define CLS_CORRESPONDANTS_H

#include <QObject>
#include <cls_correspondant.h>

class Correspondants
{
    QMap<int, Correspondant*> *m_Correspondants;    //!<Collection de tous les correspondants
    QMap<int, Correspondant*> *m_MGCorrespondants;  //!<Collection de tous les généralistes

public:
    //GETTER
    QMap<int, Correspondant *> *getCorrespondants()     const;
    QMap<int, Correspondant *> *getMGCorrespondants()   const;


    Correspondants();

    bool addCorrespondant(Correspondant *Correspondant);
    bool addMGCorrespondant(Correspondant *Correspondant);
    Correspondant* getCorrespondantById(int id);
    void VideLesListes();
};


#endif // CLS_CORRESPONDANTS_H
